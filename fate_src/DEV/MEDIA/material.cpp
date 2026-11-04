#include "d3dx8.h"
/*
	material.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History

	Travis Baldree
	4/21/2004		-	added support for AVIs as textures

	Travis Baldree
	4/29/2004		-	texture and material were split, which is much more logical

	5/27/2004		-	it looks like dynamic rendertotextures were being created for ALL textures
						instead of just dynamic DynamicTexture textures! fixed.

	6/29/2004		-	texture paths were not being properly stored for comparison later. Fixed.

	7/14/2004		-	renamed 'reflection' to 'dynamic texture' since it turned out to be much
						more general-purpose than originally thought.

	7/24/2004		-	allows creation of non-square dynamic textures now.

	8/24/2004		-	added support for material-specific cubemaps
*/


#include <D3D8.h>
#include "D3DX8.h"

#include <assert.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/vistafileutilities.h"

#include "refmanager.h"
#include "cubemap.h"
#include "movie.h"
#include "material.h"
#include "texture.h"

void BlackTexture( D3DXVECTOR4 *Color,
				   D3DXVECTOR2 *Input,
				   D3DXVECTOR2 *SampleSize,
				   void* pColor )
{
	//Color = (D3DXVECTOR4*)pColor;
	Color->x = 0;
	Color->y = 0;
	Color->z = 0;
	Color->w = 1;
}

void WhiteTexture( D3DXVECTOR4 *Color,
				   D3DXVECTOR2 *Input,
				   D3DXVECTOR2 *SampleSize,
				   void* pColor )
{
	//Color = (D3DXVECTOR4*)pColor;
	Color->x = 1;
	Color->y = 1;
	Color->z = 1;
	Color->w = 1;
}

CMaterial::CMaterial( CRefManager& pRefManager,			// reference manager(primarily for use in preventing duplicate movies)
					  bool AllowTextureCompression ) :	// allow DXT texture compression?
		m_pRefManager( pRefManager ),
		m_IlluminationType( KMaterialIllumination ),	// default to additive illumination maps
		m_AllowTextureCompression( AllowTextureCompression ),
		m_Valid( kFalse ),
		m_DynamicTextureValid( kFalse ),
		m_HasAlpha( kFalse ),
		m_HasSphereMap( kFalse ),
		m_HasAlphaTexture( kFalse ),
		m_HasBaseTexture( kFalse ),
		m_HasColorKey( kFalse ),
		m_HasIllumination( kFalse ),
		m_HasReflection( kFalse ),
		m_HasDynamicTexture( kFalse ),
		m_HasNormalMap( kFalse ),
		m_HasCubeMap( kFalse ),
		m_TransparentReflection( kFalse ),
		m_AdditiveReflection( kFalse ),
		m_BlankBaseTexture( kFalse ),
		m_AllowZWrite( kTrue ),
		m_DynamicTextureWidth( 128 ),
		m_DynamicTextureHeight( 128 ),
		m_DataID( 0 ),
		m_pTexture( NULL ),
		m_pAlphaTexture( NULL ),
		m_pSphereMapTexture( NULL ),
		m_pIlluminationTexture( NULL ),
		m_pNormalTexture( NULL ),
		m_pMovie( NULL ),
		m_pIlluminationMovie( NULL ),
		m_pDynamicTextureMap( NULL ),
		m_pRenderToDynamicTextureMap( NULL ),
		m_pCubeMap( NULL ),
		m_Path( "" ),
		m_CubeMapPath( "" ),
		m_AlphaPath( "" ),
		m_IlluminationMapPath( "" ),
		m_NormalMapPath( "" ),
		m_SphereMapPath( "" )
{
	ZeroMemory( &m_DynamicTexturePlane, sizeof( m_DynamicTexturePlane ));
	ZeroMemory( &m_DynamicTextureNormal, sizeof( m_DynamicTextureNormal ));
	ZeroMemory( &m_D3DMaterial, sizeof( D3DMATERIAL8 ));
	
	ResetAnimationProperties();

	
	m_D3DMaterial.Diffuse.r	= 1.0f;					
	m_D3DMaterial.Diffuse.g	= 1.0f;
	m_D3DMaterial.Diffuse.b	= 1.0f;
	m_D3DMaterial.Diffuse.a	= 1.0f;

	m_D3DMaterial.Specular.r	= 0.4F;
	m_D3DMaterial.Specular.g	= 0.4F;
	m_D3DMaterial.Specular.b	= 0.4F;

	m_D3DMaterial.Power		= .0001f;

	m_D3DMaterial.Ambient.r	= 0.15f;						
	m_D3DMaterial.Ambient.g	= 0.15f;
	m_D3DMaterial.Ambient.b	= 0.15f;
	m_D3DMaterial.Ambient.a	= 1.0f;

} // CMaterial::CMaterial()


CMaterial::~CMaterial()
{
	m_pRefManager.RemoveMovie( m_pMovie );
	m_pRefManager.RemoveMovie( m_pIlluminationMovie );

	DELETE_SAFELY( m_pTexture );
	DELETE_SAFELY( m_pAlphaTexture );
	DELETE_SAFELY( m_pSphereMapTexture );
	DELETE_SAFELY( m_pIlluminationTexture );
	DELETE_SAFELY( m_pNormalTexture );

	RELEASE_SAFELY( m_pDynamicTextureMap );
	RELEASE_SAFELY( m_pRenderToDynamicTextureMap );
} // CMaterial::~CMaterial()

void CMaterial::ResetAnimationProperties( void )
{
	for( uint32 i = 0; i < KMaxLayers; i++ )
	{
		m_FlipbookAnimation[i] = kFalse;
		m_ShifterAnimation[i] = kFalse;
		m_AnimationTimer[i] = 0;
		m_ShiftSpeedU[i] = 0;
		m_ShiftSpeedV[i] = 0;
		m_ShiftPositionU[i] = 0;
		m_ShiftPositionV[i] = 0;
		m_FramesWide[i] = 1;
		m_FramesHigh[i] = 1;
		m_CurrentFrame[i] = 0;
		m_FrameTime[i] = 0;
		m_FrameHeight[i] = 0;
		m_FrameWidth[i] = 0;
	}

} // CMaterial::ResetAnimationProperties()

void CMaterial::InvalidateDeviceObjects( void )
{
	if( m_DynamicTextureValid )
	{
		RELEASE_SAFELY( m_pDynamicTextureMap );
		RELEASE_SAFELY( m_pRenderToDynamicTextureMap );
		m_DynamicTextureValid = kFalse;
	}
} // CMaterial::InvalidateDeviceObjects()

void CMaterial::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
									  
{
	if( m_DynamicTextureValid || 
		!m_HasDynamicTexture )
	{
		return;
	}

	// get a pointer to the backbuffer so we can check its format -
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	pD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurfaceSrc );

	D3DSURFACE_DESC	Desc;
	pSurfaceSrc->GetDesc( &Desc );
	
	RELEASE_SAFELY( pSurfaceSrc );


	// get a pointer to the depthbuffer so we can check its format -
	pD3DDevice->GetDepthStencilSurface( &pSurfaceSrc );

	D3DSURFACE_DESC	DepthDesc;
	pSurfaceSrc->GetDesc( &DepthDesc );
	
	RELEASE_SAFELY( pSurfaceSrc );


	// we create our DynamicTexture surfaces here
	// try to create a surface to render to
	if ( FAILED( D3DXCreateTexture( pD3DDevice,
									m_DynamicTextureWidth,
									m_DynamicTextureHeight,
									1,
									D3DUSAGE_RENDERTARGET,
									Desc.Format,
									D3DPOOL_DEFAULT,
									&m_pDynamicTextureMap ) ) )
	{
		return;
	}
	// the RenderToSurface allows us to draw to the actual surface
	else
	{
		D3DSURFACE_DESC	TextureDesc;
		m_pDynamicTextureMap->GetLevelDesc( 0, &TextureDesc );

		if ( FAILED( D3DXCreateRenderToSurface( pD3DDevice,
												TextureDesc.Width,
												TextureDesc.Height,
												TextureDesc.Format,
												kTrue,
												DepthDesc.Format,
												&m_pRenderToDynamicTextureMap ) ) )
		{
			RELEASE_SAFELY( m_pDynamicTextureMap );
			return;
		}
	}

	D3DXFillTexture( m_pDynamicTextureMap, WhiteTexture, D3DXVECTOR4( 1, 1, 1, 1 ) );

	m_DynamicTextureValid = kTrue;
} // CMaterial::RestoreDeviceObjects()

void CMaterial::EnableDynamicTexture( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									  uint32 Width,					// pixel width of DynamicTexture
									  uint32 Height )					// pixel height of DynamicTexture
{
	m_HasDynamicTexture = kTrue;
	m_DynamicTextureWidth = Width;
	m_DynamicTextureHeight = Height;
	if( Height == 0 )
	{
		m_DynamicTextureHeight = Width;
	}
	RestoreDeviceObjects( pD3DDevice );
} // CMaterial::EnableDynamicTexture()

HRESULT	CMaterial::ConvertToFormatSystem( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
										   D3DFORMAT D3DFormat )				// desired format to convert to	
{
	if( m_pTexture != NULL )
	{
		return m_pTexture->ConvertToFormatSystem( pD3DDevice, D3DFormat );
	}
	return S_OK;
} // CTexture::ConvertToFormatSystem()

LPDIRECT3DTEXTURE8	CMaterial::GetTexture( void )				
{	
	if( m_pMovie )
	{
		return m_pMovie->GetTexture();
	}
	return m_pTexture->GetTexture();			
} // CMaterial::GetTexture()

LPDIRECT3DTEXTURE8	CMaterial::GetIlluminationTexture( void )	
{	
	if( m_pIlluminationMovie )
	{
		return m_pIlluminationMovie->GetTexture();
	}
	return m_pIlluminationTexture->GetTexture();	
} // CMaterial::GetIlluminationTexture()

LPDIRECT3DTEXTURE8	CMaterial::GetNormalTexture( void )	
{	
	return m_pNormalTexture->GetTexture();	
} // CMaterial::GetNormalTexture()

LPDIRECT3DTEXTURE8	CMaterial::GetAlphaTexture( void )	
{	
	return m_pAlphaTexture->GetTexture();	
} // CMaterial::GetAlphaTexture()

LPDIRECT3DTEXTURE8	CMaterial::GetSphereMapTexture( void )	
{	
	return m_pSphereMapTexture->GetTexture();	
} // CMaterial::GetSphereMapTexture()

LPDIRECT3DTEXTURE8	CMaterial::GetDynamicTexture( void )	
{	
	return m_pDynamicTextureMap;	
} // CMaterial::GetAlphaTexture()

CCubeMap* CMaterial::GetCubeMap( void )	
{	
	return m_pCubeMap;	
} // CMaterial::GetCubeMap()


// create a basic texture
HRESULT CMaterial::Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						   const std::string& TextureName,		// absolute path to texture
						   bool ColorKey,						// use colorkey?
						   DWORD ColorKeyValue )				// defaults to green (0,255,0)
{
	// store the path for later comparison
	m_Path = StringUpper( TextureName );

	// reset the material properties
	m_Valid = kFalse;
	m_HasAlpha = kFalse;
	m_HasColorKey = kFalse;


	std::string		Extension( "" );
	
	// parse the extension of the filename
	if ( TextureName.length() > 3 )
	{
		Extension = StringUpper( TextureName.substr( TextureName.length() - 3, 3 ) );
	}
	
	// load an avi if specified
	if( Extension == "AVI" && FileExists( TextureName ) )
	{
		m_pMovie = m_pRefManager.AddMovie( pD3DDevice, TextureName, kTrue );
		m_Valid = m_pMovie->IsValid();

		if( m_Valid )
		{
			m_HasBaseTexture = kTrue;
		}
		else
		{
			 m_pRefManager.RemoveMovie( m_pMovie );
			 m_pMovie = NULL;

			m_pTexture = new CTexture( kFalse );
			m_pTexture->CreateBlankTexture( pD3DDevice,
											32,
											32 );

			if( m_pTexture->IsValid() )
			{
				m_BlankBaseTexture = kTrue;
				m_Valid = kTrue;
				m_HasBaseTexture = kTrue;
			}

		}
		return S_OK;
	}


	m_pTexture = new CTexture( m_AllowTextureCompression );
	m_pTexture->Create( pD3DDevice,
						TextureName,
						ColorKey,
						ColorKeyValue );

	// valid if completed load
	m_Valid = m_pTexture->IsValid();
	if( m_Valid )
	{
		m_HasBaseTexture = kTrue;
		m_HasAlpha = m_pTexture->HasAlpha();
		m_HasColorKey = m_pTexture->HasColorKey();
	}


	return S_OK;
} // CMaterial::Create()


// create a texture with an opacity mask
HRESULT CMaterial::Create( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   std::string TextureName,	// absolute path to base texture
						   const std::string& OpacityName,	// absolute path to opacity mask
						   int32 MipLevels )				// # of mip levels to generate (D3DX_DEFAULT is default)
{
	// store the path for later comparison
	m_Path = StringUpper( TextureName );
	// store the path for later comparison
	m_AlphaPath = StringUpper( OpacityName );

	// reset material properties
	m_Valid = kFalse;
	m_HasAlpha = kFalse;
	m_HasColorKey = kFalse;

	bool				MergedDDS( kFalse );
	bool				SaveDDS( kFalse );
	std::string			Extension( "" );
	std::string			DDSTextureName( "" );

	// parse the extension of the filename
	if ( TextureName.length() > 3 )
	{
		Extension = StringUpper( TextureName.substr( TextureName.length() - 3, 3 ) );
	}

	// load an avi if specified - we don't get an alph
	if( Extension == "AVI" && FileExists( TextureName ) )
	{
		return Create( pD3DDevice, TextureName );
	}

	D3DFORMAT StandardFormat = D3DFMT_UNKNOWN;
#ifdef CONVERT_TO_DDS
	IDirect3D8 *pD3D = NULL;
	D3DDISPLAYMODE pMode;
	ZeroMemory ( &pMode, sizeof ( pMode ));

	pD3DDevice->GetDisplayMode( &pMode );
	if( m_AllowTextureCompression &&
		!FAILED( pD3DDevice->GetDirect3D( &pD3D ) ) )
	{
		if( !FAILED( pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
												D3DDEVTYPE_HAL,
												pMode.Format,
												0,
												D3DRTYPE_TEXTURE,
												D3DFMT_DXT1 ) ) )
		{
			StandardFormat = D3DFMT_DXT1;
		}
	}

	if( Extension != "DDS" &&
		StandardFormat == D3DFMT_DXT1 )
	{
		DDSTextureName = CVistaFileUtilities::GetSharedDataPath( KFileLocation_SharedCache ) + "/" + TextureName.substr( 0, TextureName.length() - 4 );
		DDSTextureName = DDSTextureName + "_merged.DDS";
		if( FileExists( DDSTextureName ) )
		{
			TextureName = DDSTextureName;
			MergedDDS = kTrue;
			Extension = "DDS";
		}
		else
		{
			SaveDDS = kTrue;
		}
	}
#endif

	m_pTexture = new CTexture( m_AllowTextureCompression );
	m_pTexture->Create( pD3DDevice,
						TextureName );

	// valid if completed load - we now have a base texture
	// even if the Alpha fails to load/merge, we can still use this
	if( m_pTexture->IsValid() )
	{
		m_Valid = kTrue;
		m_HasBaseTexture = kTrue;
	}

	// don't bother to create the alpha if we have a premade DDS
	if( !MergedDDS )
	{
		m_pAlphaTexture = new CTexture( m_AllowTextureCompression );
		m_pAlphaTexture->Create( pD3DDevice,
								OpacityName );

		// if we have an alpha, but no base texture,
		// just create a blank black one of the 
		// appropriate size
		if( m_pAlphaTexture->IsValid() &&
			!m_HasBaseTexture )
		{
			DELETE_SAFELY( m_pTexture );
			m_pTexture = new CTexture( m_AllowTextureCompression );
			m_pTexture->CreateBlankTexture( pD3DDevice,
											m_pAlphaTexture->Width(),
											m_pAlphaTexture->Height() );

			if( m_pTexture->IsValid() )
			{
				m_BlankBaseTexture = kTrue;
				m_Valid = kTrue;
				m_HasBaseTexture = kTrue;
			}
		}
		if( m_pAlphaTexture->IsValid() &&
			m_HasBaseTexture )
		{
			m_HasAlphaTexture = kTrue;
			m_pTexture->MergeAlpha( pD3DDevice,
									*m_pAlphaTexture );
		}

	}
	// we now have a valid Alpha channel
	m_HasAlpha = m_pTexture->HasAlpha();

	if( SaveDDS &&
		StandardFormat == D3DFMT_DXT1 )
	{
		CVistaFileUtilities::CreateTempFile(DDSTextureName);
		D3DXSaveTextureToFile( DDSTextureName.c_str(),
							   D3DXIFF_DDS,
							   m_pTexture->GetTexture(),
							   NULL );
	}

	return S_OK;
} // CMaterial::Create()

// create an illumination map for the base texture
HRESULT CMaterial::CreateIllumination( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									   const std::string& TextureName )		// absolute path to texture
{
	// store the path for later comparison
	m_IlluminationMapPath = StringUpper( TextureName );

	std::string		Extension( "" );
	
	// parse the extension of the filename
	if ( TextureName.length() > 3 )
	{
		Extension = StringUpper( TextureName.substr( TextureName.length() - 3, 3 ) );
	}
	// load an avi if specified
	if( Extension == "AVI" && FileExists( TextureName ) )
	{
		m_pIlluminationMovie = m_pRefManager.AddMovie( pD3DDevice, TextureName, kTrue );
		m_Valid = m_pIlluminationMovie->IsValid();

		if( m_Valid )
		{
			m_HasIllumination = kTrue;
		}
		else
		{
			 m_pRefManager.RemoveMovie( m_pIlluminationMovie );
			 m_pIlluminationMovie = NULL;


			m_pIlluminationTexture = new CTexture( kFalse );
			m_pIlluminationTexture->CreateBlankTexture( pD3DDevice,
											32,
											32 );

			if( m_pIlluminationTexture->IsValid() )
			{
				// valid if completed load
				m_Valid = kTrue;
				m_HasIllumination = kTrue;
			}
		}
		return S_OK;
	}


	m_pIlluminationTexture = new CTexture( m_AllowTextureCompression );
	m_pIlluminationTexture->Create( pD3DDevice,
									TextureName );

	if( m_pIlluminationTexture->IsValid() )
	{
		// valid if completed load
		m_Valid = kTrue;
		m_HasIllumination = kTrue;
	}

	return S_OK;
} // CMaterial::CreateIllumination()

// create a spherical map for the base texture to use
HRESULT CMaterial::CreateSphereMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									const std::string& TextureName )	// absolute path to texture
{
	// store the path for later comparison
	m_SphereMapPath = StringUpper( TextureName );

	std::string		Extension( "" );
	
	// parse the extension of the filename
	if ( TextureName.length() > 3 )
	{
		Extension = StringUpper( TextureName.substr( TextureName.length() - 3, 3 ) );
	}
	// if we tried to load a cheap reflection map, and it
	// turns out to be a cubemap dds, just load the cubemap
	if( Extension == "DDS" )
	{
		return CreateCubeMap( pD3DDevice, TextureName );
	}

	m_pSphereMapTexture = new CTexture( m_AllowTextureCompression );
	m_pSphereMapTexture->Create( pD3DDevice,
								 TextureName );

	if( m_pSphereMapTexture->IsValid() )
	{
		// valid if completed load
		m_Valid = kTrue;
		m_HasSphereMap = kTrue;
	}

	return S_OK;
} // CMaterial::CreateSphereMap()

// create a cube map for use in reflections -- assumes DDS CUBEMAP!
HRESULT CMaterial::CreateCubeMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								  const std::string& TextureName )	// absolute path to texture
{
	// store the path for later comparison
	m_CubeMapPath = StringUpper( TextureName );

	std::string		Extension( "" );
	
	// parse the extension of the filename
	if ( TextureName.length() > 3 )
	{
		Extension = StringUpper( TextureName.substr( TextureName.length() - 3, 3 ) );		
	}
	if( Extension != "DDS" )
	{
		return S_OK;
	}

	m_pCubeMap = m_pRefManager.AddCubeMap( pD3DDevice, m_CubeMapPath );

	
	if( m_pCubeMap->IsValid() )
	{
		// valid if completed load
		m_Valid = kTrue;
		m_HasCubeMap = kTrue;
	}

	return S_OK;
} // CMaterial::CreateCubeMap()

// create a normal map for the base texture
HRESULT CMaterial::CreateNormalMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									const std::string& TextureName )	// absolute path to texture
{
	// store the path for later comparison
	m_NormalMapPath = StringUpper( TextureName );

	m_pNormalTexture = new CTexture( m_AllowTextureCompression );
	m_pNormalTexture->Create( pD3DDevice,
							  TextureName );

	if( m_pNormalTexture->IsValid() )
	{
		// valid if completed load
		m_Valid = kTrue;
		m_HasNormalMap = kTrue;
	}

	return S_OK;
} // CMaterial::CreateNormalMap()

// set this material as 'reflective' This property can only be set if the
// material has an Alpha channel, and is only used by certain objects (scene objects and models, notably)
void CMaterial::SetReflective( bool State )	// reflective state
{
	if ( !State )
	{
		m_HasReflection = kFalse;
	}
	else 
	{
		m_HasReflection = kTrue;
	}
} // CMaterial::SetReflective()

// set this material as normal mapped This property can only be set if the
// material has a Normal Map
void CMaterial::SetNormalMapped( bool State )	// normal mapped state
{
	if ( !State )
	{
		m_HasNormalMap = kFalse;
	}
	else if ( m_HasNormalMap )
	{
		m_HasNormalMap = kTrue;
	}
} // CMaterial::SetReflective()

// enable shifting animations
void CMaterial::SetAnimationShifting( uint32 Layer,		// layer to perform operation on
									  float32 U,		// how far to scroll in U per second
									  float32 V )		// how far to scroll in V per second
{
	m_ShiftSpeedU[Layer] = U;
	m_ShiftSpeedV[Layer] = V;
	m_ShifterAnimation[Layer] = kTrue;
} // CMaterial::SetUVShift()

// enable flipbook animations
void CMaterial::SetAnimationFlipbook( uint32 Layer,		// layer to perform operation on
									  int32 FramesWide,	// how many frames wide
									  int32 FramesHigh,	// how many frames high
									  float32 FrameTime )	// seconds per frame
{
	m_FramesWide[Layer] = FramesWide;
	m_FramesHigh[Layer] = FramesHigh;
	m_FrameTime[Layer] = FrameTime;
	m_FrameHeight[Layer] = 1.0f / FramesHigh;
	m_FrameWidth[Layer] = 1.0f / FramesWide;
	m_FlipbookAnimation [Layer]= kTrue;

} // CMaterial::SetUVShiftAnim()

bool CMaterial::IsAnimated( uint32 Layer )			
{	
	assert( Layer < KMaxLayers );
	if( m_FlipbookAnimation[Layer] || m_ShifterAnimation[Layer] )
	{
		return kTrue;			
	}
	return kFalse;
} // CMaterial::IsAnimated()

// update any animations for this material
void CMaterial::Update( float32 TimeElapsed )		// seconds elapsed
{
	for( uint32 i = 0; i < KMaxLayers; i++ )
	{
		if ( m_FlipbookAnimation[i] )
		{
			m_AnimationTimer[i] += TimeElapsed;
			if ( m_AnimationTimer[i] > m_FrameTime[i] )
			{
				m_ShiftPositionU[i] += m_FrameWidth[i];
				m_CurrentFrame[i] ++;
				if( m_CurrentFrame[i] >= m_FramesWide[i] )
				{
					m_ShiftPositionV[i] += m_FrameHeight[i];
					m_ShiftPositionU[i] -= 1.0f;
					m_CurrentFrame[i] = 0;
				}
				m_AnimationTimer[i] = 0;
			}
		}
		if( m_ShifterAnimation[i] )
		{
			m_ShiftPositionU[i] += m_ShiftSpeedU[i] * TimeElapsed;
			m_ShiftPositionV[i] += m_ShiftSpeedV[i] * TimeElapsed;
		}
	}
} // CMaterial::Update()

void CMaterial::SetAnimationStates( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	for( uint32 i = 0; i < KMaxLayers; i++ )
	{
		if( IsAnimated( i ) )
		{
			SetAnimationLayerStates( pD3DDevice, i );
		}
	}
} // CMaterial::SetAnimationStates()

void CMaterial::SetAnimationLayerStates( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
										 uint32 Layer )					// layer to set states for
{
	assert( Layer < KMaxLayers );

	pD3DDevice->SetTextureStageState( Layer,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_COUNT2 );

	D3DXMATRIX	matShift;
	D3DXMatrixIdentity( &matShift );

	// set the matrix using the shifting component of the material
	matShift._31 = GetU( Layer );
	matShift._32 = GetV( Layer );

	pD3DDevice->SetTransform( D3DTS_TEXTURE0, &matShift );
} // CMaterial::SetAnimationLayerStates()

void CMaterial::UnSetAnimationStates( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	for( uint32 i = 0; i < KMaxLayers; i++ )
	{
		if( IsAnimated( i ) )
		{
			UnSetAnimationLayerStates( pD3DDevice, i );
		}
	}
} // CMaterial::SetAnimationStates()

void CMaterial::UnSetAnimationLayerStates( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
										   uint32 Layer )					// layer to set states for
{
	assert( Layer < KMaxLayers );

	pD3DDevice->SetTextureStageState( Layer,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_DISABLE );

} // CMaterial::UnSetAnimationLayerStates()

uint32 CMaterial::Width( void )
{	
	assert( m_pTexture != NULL ||
			m_pAlphaTexture != NULL ||
			m_pIlluminationTexture != NULL ||
			m_pMovie != NULL ||
			m_pIlluminationMovie != NULL );

	if( m_pTexture != NULL )
	{
		return m_pTexture->Width();
	}
	else if( m_pAlphaTexture != NULL )
	{
		return m_pAlphaTexture->Width();
	}
	else if( m_pIlluminationTexture != NULL )
	{
		return m_pIlluminationTexture->Width();
	}
	else if( m_pMovie != NULL )
	{
		return m_pMovie->Width();
	}
	else if( m_pIlluminationMovie != NULL )
	{
		return m_pIlluminationMovie->Width();
	}
	return 0;				
};
uint32 CMaterial::Height( void )				
{	
	assert( m_pTexture != NULL ||
			m_pAlphaTexture != NULL ||
			m_pIlluminationTexture != NULL ||
			m_pMovie != NULL ||
			m_pIlluminationMovie != NULL );

	if( m_pTexture != NULL )
	{
		return m_pTexture->Height();
	}
	else if( m_pAlphaTexture != NULL )
	{
		return m_pAlphaTexture->Height();
	}
	else if( m_pIlluminationTexture != NULL )
	{
		return m_pIlluminationTexture->Height();
	}
	else if( m_pMovie != NULL )
	{
		return m_pMovie->Height();
	}
	else if( m_pIlluminationMovie != NULL )
	{
		return m_pIlluminationMovie->Height();
	}
	return 0;				
};

// begins rendering to DynamicTexture map
HRESULT CMaterial::BeginRender( void )
{

	// get the surface of our DynamicTexture map
	LPDIRECT3DSURFACE8	pSurface;
	m_pDynamicTextureMap->GetSurfaceLevel( 0, &pSurface );

	// and begin a scene on it
	HRESULT hr = m_pRenderToDynamicTextureMap->BeginScene( pSurface, NULL );

	// and release the surface
	RELEASE_SAFELY( pSurface );

	return hr;
} // CMaterial::BeginRender()

// end an active render to the DynamicTexture map
void CMaterial::EndRender( void )	
{
	assert( m_pRenderToDynamicTextureMap != NULL );

	// close the render
	m_pRenderToDynamicTextureMap->EndScene();
} // CMaterial::EndRender()

void CMaterial::SetDynamicTexturePlane( const D3DXVECTOR3& CenterPoint, 
								    const D3DXVECTOR3& Normal )
{
	m_DynamicTextureNormal = Normal;
	D3DXPlaneFromPointNormal( &m_DynamicTexturePlane, &CenterPoint, &Normal );
} // CMaterial::SetDynamicTexturePlane()