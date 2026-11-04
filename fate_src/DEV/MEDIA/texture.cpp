#include "d3dx8.h"
/*
	texture.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	4/29/2004

	Change History

	Travis Baldree
	4/29/2004		-	texture and material were split, which is much more logical

	5/14/2004		-	made ConvertToFormat regenerate mipmaps - wasn't doing it before.

	7/13/2004		-	using D3DPOOL_SCRATCH for scratch textures when converting turned
						out to be a no-no, because the output format was not necessarily
						usable by the card. Switched to D3DPOOL_MANAGED

*/


#include <D3D8.h>
#include "D3DX8.h"

#include <assert.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/vistafileutilities.h"
#include "bitmap.h"

#include "texture.h"

#ifdef _USE_JPEG2000
#include "jasper/jasper.h"
#endif

void BlankTexture( D3DXVECTOR4 *Color,
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

#ifdef _USE_JPEG2000
// taken from the JASPER JPEG2000 samples -
// this assembles the final JPEG2000 SourceImage Data
static uint16 JPEG2000PutData( jas_stream_t* Target,		// target datastream
							   jas_image_t* SourceImage,	// source JAS image Data
							   uint16 Components )			// # of compents Stream the image
{
	uint16				ReturnValue( 0 );
	uint16				ComponentNumber( 0 );
	uint16				X( 0 );
	uint16				Y( 0 );
	jas_matrix_t*	Data[4];
	jas_seqent_t*	d[4];
	jas_seqent_t	v( 0 );
	uint16				LineLength( 0 );
	uint16				Width( 0 );
	uint16				Height( 0 );

	Width = jas_image_cmptwidth( SourceImage, 0 );
	Height = jas_image_cmptheight( SourceImage, 0 );

	ReturnValue = -1;

	Data[0] = 0;
	Data[1] = 0;
	Data[2] = 0;
	Data[3] = 0;
	for ( ComponentNumber = 0; ComponentNumber < Components; ++ComponentNumber )
	{
		if ( !( Data[ComponentNumber] = jas_matrix_create( 1, Width ) ) )
		{
			goto done;
		}
	}

	for ( Y = 0; Y < Height; Y++ )
	{
		for ( ComponentNumber = 0; ComponentNumber < Components; ++ComponentNumber )
		{
			if ( jas_image_readcmpt( SourceImage,
									 ComponentNumber,
									 0,
									 Y,
									 Width,
									 1,
									 Data[ComponentNumber] ) )
			{
				goto done;
			}
			d[ComponentNumber] = jas_matrix_getref( Data[ComponentNumber], 0, 0 );
		}
		LineLength = 0;
		for ( X = 0; X < Width; ++X )
		{
			for ( ComponentNumber = 0; ComponentNumber < Components; ++ComponentNumber )
			{
				v = *d[ComponentNumber];
				if ( v < 0 )
				{
					v = 0;
				}
				if ( v > 255 )
				{
					v = 255;
				}
				uint8 c;
				c = v;
				if ( jas_stream_putc( Target, c ) == EOF )
				{
					goto done;
				}
				++d[ComponentNumber];
			}
		}
		if ( jas_stream_error( Target ) )
		{
			goto done;
		}
	}

	jas_stream_flush( Target );
	ReturnValue = 0;

	done:

	for ( ComponentNumber = 0; ComponentNumber < Components; ++ComponentNumber )
	{
		if ( Data[ComponentNumber] )
		{
			jas_matrix_destroy( Data[ComponentNumber] );
		}
	}

	return ReturnValue;
}

#endif

CTexture::CTexture( bool AllowTextureCompression ) :	// allow DXT texture compression?
		m_AllowTextureCompression( AllowTextureCompression ),		
		m_Path( "" ),
		m_Valid( kFalse ),
		m_HasAlpha( kFalse ),
		m_HasColorKey( kFalse ),
		m_Width( 0 ),
		m_Height( 0 ),
		m_pTexture( NULL )
{
} // CTexture::CTexture()


CTexture::~CTexture()
{
	RELEASE_SAFELY( m_pTexture );
} // CTexture::~CTexture()

LPDIRECT3DTEXTURE8	CTexture::GetTexture( void )				
{	
	return m_pTexture;			
} // CTexture::GetTexture()

// create a basic texture
HRESULT CTexture::Create( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
						  std::string TextureName,			// absolute path to texture
						  bool ColorKey,					// use colorkey?
						  DWORD ColorKeyValue )				// defaults to green (0,255,0)
{
	std::string DDSTextureName	= "";
	bool		SaveDDS			= kFalse;
	// store the path for later comparison
	m_Path = StringUpper( TextureName );

	// clean up any previous texture and reset the material properties
	RELEASE_SAFELY( m_pTexture );
	m_Valid			= kFalse;
	m_HasAlpha		= kFalse;
	m_HasColorKey	= kFalse;


	D3DFORMAT ColorkeyFormat = D3DFMT_A8R8G8B8;
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
		else
		{
			m_AllowTextureCompression = kFalse;
		}
		if( !FAILED( pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
												D3DDEVTYPE_HAL,
												pMode.Format,
												0,
												D3DRTYPE_TEXTURE,
												D3DFMT_DXT3 ) ) )
		{
			ColorkeyFormat = D3DFMT_DXT3;
		}
		else
		{
			if( !FAILED( pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
													D3DDEVTYPE_HAL,
													pMode.Format,
													0,
													D3DRTYPE_TEXTURE,
													D3DFMT_DXT2 ) ) )
			{
				ColorkeyFormat = D3DFMT_DXT2;
			}
			else if( ColorKey )	// if we don't have dxt2, we can't do texture compression with alpha
			{
				m_AllowTextureCompression = kFalse;
			}
		}
	}
#endif
	HRESULT			hr = 0;
	std::string		Extension( "" );
	
	// parse the extension of the filename
	if ( TextureName.length() > 3 )
	{
		Extension = StringUpper( TextureName.substr( TextureName.length() - 3, 3 ) );
	}
	
#ifdef _USE_JPEG2000
	// if it is a PNG and doesn't exist, try checking to see if a WJ2(JPEG 2000) file exists,
	// and change the path accordingly
	if ( Extension == "PNG" && !FileExists( TextureName ) )
	{
		TextureName = TextureName.substr( 0, TextureName.length() - 3 );
		TextureName = TextureName + "WJ2";
		Extension = "WJ2";
	}

	// trying for a JPEG 2000 file
	if ( Extension == "JP2" || Extension == "J2K" || Extension == "WJ2" )
	{
		hr = CreateJP2( pD3DDevice,
						TextureName.c_str(),
						&m_pTexture,
						ColorKey,
						kFalse );
		if ( FAILED( hr ) )
		{
			return E_FAIL;
		}

		if ( ColorKey )
		{
			m_HasColorKey = kTrue;
		}

		hr = D3DXFilterTexture( m_pTexture, NULL, 0, D3DX_FILTER_BOX );
	}
	// otherwise we use the builtin D3DX texture loader
	else
#endif
	{
#ifdef CONVERT_TO_DDS
		// if it is a PNG and doesn't exist, try checking to see if a DDS file exists,
		// and change the path accordingly
		if ( m_AllowTextureCompression &&
			 Extension != "DDS" )
		{
			if( ColorKey )
			{
				DDSTextureName = CVistaFileUtilities::GetSharedDataPath( KFileLocation_SharedCache ) + "/" + TextureName.substr( 0, TextureName.length() - 4 );
				DDSTextureName = DDSTextureName + "_colorkey.DDS";
			}
			else
			{
				DDSTextureName = CVistaFileUtilities::GetSharedDataPath( KFileLocation_SharedCache ) + "/" + TextureName.substr( 0, TextureName.length() - 3 );
				DDSTextureName = DDSTextureName + "DDS";
			}

			if( FileExists( DDSTextureName ) )
			{
				// if the texture is colorkeyed, but has already been created, then
				// we no longer need colorkey, as alpha is embedded
				if( ColorKey )
				{
					ColorKey = kFalse;
				}
				Extension = "DDS";
				TextureName = DDSTextureName;
				// use whatever format is in the file if required
				ColorkeyFormat = D3DFMT_UNKNOWN;
				StandardFormat = D3DFMT_UNKNOWN;
			}
			else
			{
				SaveDDS = kTrue;
			}
		}
		else
		{
			ColorkeyFormat = D3DFMT_A8R8G8B8;
			StandardFormat = D3DFMT_UNKNOWN;
		}
#endif

		// if this material specifies a colorkey, create one with GREEN (0,255,0) as the colorkey
		if ( ColorKey )
		{
			m_HasColorKey = kTrue;
			hr = D3DXCreateTextureFromFileEx( pD3DDevice,
											  TextureName.c_str(),
											  D3DX_DEFAULT,
											  D3DX_DEFAULT,
											  1,
											  0,
											  ColorkeyFormat,
											  D3DPOOL_MANAGED,
											  D3DX_FILTER_LINEAR,
											  D3DX_FILTER_BOX,
											  ColorKeyValue,
											  NULL,
											  NULL,
											  &m_pTexture );
		}
		// otherwise, this is straight-up
		else
		{
			hr = D3DXCreateTextureFromFileEx( pD3DDevice,
											  TextureName.c_str(),
											  D3DX_DEFAULT,
											  D3DX_DEFAULT,
											  D3DX_DEFAULT,
											  0,
											  StandardFormat,
											  D3DPOOL_MANAGED,
											  D3DX_FILTER_LINEAR,
											  D3DX_FILTER_BOX,
											  0,
											  NULL,
											  NULL,
											  &m_pTexture );
		}
	}

	// if creation of the texture failed, return
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// now we check the texture's format, for which we need the surface description
	D3DSURFACE_DESC	desc;
	ZeroMemory ( &desc, sizeof ( desc ));
	hr = m_pTexture->GetLevelDesc( 0, &desc );
	// no surface description, we failed
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// store the image's width and height
	m_Width		= desc.Width;
	m_Height	= desc.Height;

	// these formats contain an Alpha channel. If the image had embedded Alpha, we need
	// to make sure we know we're an Alpha texture for later
	if ( desc.Format == 21 ||
		 desc.Format == 26 ||
		 desc.Format == 29 ||
		 desc.Format == 32 ||
		 desc.Format == 25 ||
		 desc.Format == 31 ||
		 desc.Format == 35 ||
		 desc.Format == 36 ||
		 desc.Format == 40 ||
		 desc.Format == D3DFMT_DXT2 ||
		 desc.Format == D3DFMT_DXT3 )
	{
		m_HasAlpha = kTrue;
	}
	if( m_HasColorKey )
	{
		m_HasAlpha = kTrue;
	}

	if( m_AllowTextureCompression &&
		StandardFormat == D3DFMT_DXT1 &&
		SaveDDS )
	{
		//create directory structure first..
		CVistaFileUtilities::CreateTempFile(DDSTextureName);
		D3DXSaveTextureToFile( DDSTextureName.c_str(),
							   D3DXIFF_DDS,
							   m_pTexture,
							   NULL );
	}
	// valid if completed load
	m_Valid = kTrue;

	return S_OK;
} // CTexture::Create()

// convert a texture to a new d3dfmt
HRESULT CTexture::ConvertToFormat( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
								   D3DFORMAT D3DFormat )				// desired format to convert to
{		
	assert( m_pTexture != NULL );

	D3DSURFACE_DESC	desc;
	HRESULT hr = m_pTexture->GetLevelDesc( 0, &desc );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// get the image's width and height
	uint32 Width = desc.Width;
	uint32 Height = desc.Height;

	LPDIRECT3DTEXTURE8	pScratchTexture	= NULL;
	// create a texture that has an alpha channel
	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							D3DX_DEFAULT,
							0,
							D3DFormat,
							D3DPOOL_MANAGED,
							&pScratchTexture );

	// if we couldn't, clear the data and leave
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	// we need to get a surface desc to copy things properly
	hr = pScratchTexture->GetLevelDesc( 0, &desc );
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	// now we're going to do a surface-to-surface copy from
	// our scratch texture to the target texture.
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	IDirect3DSurface8*	pSurfaceDest	= NULL;

	m_pTexture->GetSurfaceLevel( 0, &pSurfaceSrc );
	pScratchTexture->GetSurfaceLevel( 0, &pSurfaceDest );

	hr = D3DXLoadSurfaceFromSurface( pSurfaceDest,
									 NULL,
									 NULL,
									 pSurfaceSrc,
									 NULL,
									 NULL,
									 D3DX_FILTER_LINEAR,
									 0 );

	// on failure, clean up our mess
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pSurfaceDest );
		RELEASE_SAFELY( pSurfaceSrc );
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	// clean up our surfaces
	RELEASE_SAFELY( pSurfaceDest );
	RELEASE_SAFELY( pSurfaceSrc );

	
	// now we have to copy it back!
	RELEASE_SAFELY( m_pTexture );


	m_pTexture = pScratchTexture;
	// Why was I doing this? It doesn't make any sense!
/*
	// create a texture in the desired format
	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							D3DX_DEFAULT,
							0,
							D3DFormat,
							D3DPOOL_MANAGED,
							&m_pTexture );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	pScratchTexture->GetSurfaceLevel( 0, &pSurfaceSrc );
	m_pTexture->GetSurfaceLevel( 0, &pSurfaceDest );

	hr = D3DXLoadSurfaceFromSurface( pSurfaceDest,
									 NULL,
									 NULL,
									 pSurfaceSrc,
									 NULL,
									 NULL,
									 D3DX_FILTER_LINEAR,
									 0 );

	// on failure, clean up our mess
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pSurfaceDest );
		RELEASE_SAFELY( pSurfaceSrc );
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	RELEASE_SAFELY( pScratchTexture );
	
*/
	// clean up our surfaces
	RELEASE_SAFELY( pSurfaceDest );
	RELEASE_SAFELY( pSurfaceSrc );


	D3DXFilterTexture( m_pTexture, NULL, 0, D3DX_FILTER_BOX );

	return S_OK;
} // CTexture::ConvertToFormat()

// convert a texture to a new d3dfmt
HRESULT CTexture::ConvertToFormatSystem( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device 
										 D3DFORMAT D3DFormat )				// desired format to convert to
{		
	assert( m_pTexture != NULL );

	D3DSURFACE_DESC	desc;
	HRESULT hr = m_pTexture->GetLevelDesc( 0, &desc );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// get the image's width and height
	uint32 Width = desc.Width;
	uint32 Height = desc.Height;

	LPDIRECT3DTEXTURE8	pScratchTexture	= NULL;
	// create a texture that has an alpha channel
	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							D3DX_DEFAULT,
							0,
							D3DFormat,
							D3DPOOL_SYSTEMMEM,
							&pScratchTexture );

	// if we couldn't, clear the data and leave
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	// we need to get a surface desc to copy things properly
	hr = pScratchTexture->GetLevelDesc( 0, &desc );
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	// now we're going to do a surface-to-surface copy from
	// our scratch texture to the target texture.
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	IDirect3DSurface8*	pSurfaceDest	= NULL;

	m_pTexture->GetSurfaceLevel( 0, &pSurfaceSrc );
	pScratchTexture->GetSurfaceLevel( 0, &pSurfaceDest );

	hr = D3DXLoadSurfaceFromSurface( pSurfaceDest,
									 NULL,
									 NULL,
									 pSurfaceSrc,
									 NULL,
									 NULL,
									 D3DX_FILTER_LINEAR,
									 0 );

	// on failure, clean up our mess
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pSurfaceDest );
		RELEASE_SAFELY( pSurfaceSrc );
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}

	// clean up our surfaces
	RELEASE_SAFELY( pSurfaceDest );
	RELEASE_SAFELY( pSurfaceSrc );

	
	// now we have to copy it back!
	RELEASE_SAFELY( m_pTexture );
	m_pTexture = pScratchTexture;

	// Why was I doing this? It doesn't make any sense!
/*
	// create a texture in the desired format
	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							D3DX_DEFAULT,
							0,
							D3DFormat,
							D3DPOOL_SYSTEMMEM,
							&m_pTexture );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	pScratchTexture->GetSurfaceLevel( 0, &pSurfaceSrc );
	m_pTexture->GetSurfaceLevel( 0, &pSurfaceDest );

	hr = D3DXLoadSurfaceFromSurface( pSurfaceDest,
									 NULL,
									 NULL,
									 pSurfaceSrc,
									 NULL,
									 NULL,
									 D3DX_FILTER_LINEAR,
									 0 );

	// on failure, clean up our mess
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pSurfaceDest );
		RELEASE_SAFELY( pSurfaceSrc );
		RELEASE_SAFELY( pScratchTexture );
		return E_FAIL;
	}
	RELEASE_SAFELY( pScratchTexture );
*/
	// clean up our surfaces
	RELEASE_SAFELY( pSurfaceDest );
	RELEASE_SAFELY( pSurfaceSrc );


	D3DXFilterTexture( m_pTexture, NULL, 0, D3DX_FILTER_BOX );

	return S_OK;
} // CTexture::ConvertToFormatSystem()

HRESULT CTexture::CreateBlankTexture( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
									   uint32 Width,				// width in pixels
									   uint32 Height )				// height in pixels
{
	HRESULT hr;

	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							D3DX_DEFAULT,
							0,
							D3DFMT_A8R8G8B8,
							D3DPOOL_MANAGED,
							&m_pTexture );

							
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}
	
	hr = D3DXFillTexture( m_pTexture, BlankTexture, D3DXVECTOR4( 0, 0, 0, 1 ) );

	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	m_Valid = kTrue;

	return S_OK;
} // CTExture::CreateBlankTexture()

// create a texture with an opacity mask
HRESULT CTexture::MergeAlpha( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							  CTexture&	pAlphaTexture,		// alpha texture to merge
							  int32 MipLevels )				// # of mip levels to generate (D3DX_DEFAULT is default)
{
	assert( m_pTexture != NULL );
	assert( pAlphaTexture.GetTexture() != NULL );

	if( m_pTexture == NULL )
	{
		return E_FAIL;
	}

	if( pAlphaTexture.GetTexture() == NULL )
	{
		return E_FAIL;
	}

	if( !m_HasAlpha )
	{
		ConvertToFormat( pD3DDevice,
						 D3DFMT_A8R8G8B8 );
	}

	// get the surface description of the base texture

	D3DSURFACE_DESC	desc;
	HRESULT hr = m_pTexture->GetLevelDesc( 0, &desc );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// store the image's width and height
	m_Width = desc.Width;
	m_Height = desc.Height;

	// get the surface description of the alpha texture

	D3DSURFACE_DESC	descalpha;
	hr = pAlphaTexture.GetTexture()->GetLevelDesc( 0, &descalpha );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// make the alpha we're copying match the format of the
	// texture we're copying to, to minimize risk of error
	// during the bit merging
	if( descalpha.Format != desc.Format )
	{
		pAlphaTexture.ConvertToFormat( pD3DDevice,
									   desc.Format );
	}

	// then get the desc again in case something changed
	hr = pAlphaTexture.GetTexture()->GetLevelDesc( 0, &descalpha );
	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	if( desc.Width != descalpha.Width ||
		desc.Height != descalpha.Height )
	{
		return E_FAIL;
	}
	

	D3DLOCKED_RECT	Rect;
	D3DLOCKED_RECT	AlphaRect;

	// lock the base texture
	hr = m_pTexture->LockRect( 0, &Rect, NULL, 0 ); //This allows our debugger to keep running

	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}
	// lock the alpha texture
	hr = pAlphaTexture.GetTexture()->LockRect( 0, &AlphaRect, NULL, 0 ); //This allows our debugger to keep running

	if ( FAILED( hr ) )
	{
		return E_FAIL;
	}

	// based on our format, merge the alpha texture into the alpha channel of the base texture
	switch ( desc.Format )
	{
	case D3DFMT_A8R8G8B8:
		//32-bit ARGB, what we requested
		MergeAlphaA8R8G8B8( desc.Width,
							desc.Height,
							&Rect,
							&AlphaRect );
		break;
	case D3DFMT_A4R4G4B4:
		//16-bit ARGB, not ideal, but not bad
		MergeAlphaA4R4G4B4( desc.Width,
							desc.Height,
							&Rect,
							&AlphaRect );
		break;
	case D3DFMT_A1R5G5B5:
		//16-bit ARGB, not ideal, but not bad
		MergeAlphaA1R5G5B5( desc.Width,
							desc.Height,
							&Rect,
							&AlphaRect );
		break;
	case D3DFMT_R8G8B8:
	case D3DFMT_R3G3B2:
	case D3DFMT_R5G6B5:
		//These 3 formats have no Alpha channel
		break;
	default:
		//Default handler.
		break;
	};

	// unlock everthing
	m_pTexture->UnlockRect( 0 );
	pAlphaTexture.GetTexture()->UnlockRect( 0 );

	// if any miplevels are requested, make sure to filter the mips
	if ( MipLevels != 1 )
	{
		hr = D3DXFilterTexture( m_pTexture, NULL, 0, D3DX_FILTER_BOX );
	}

	// we now have a valid Alpha channel
	m_HasAlpha = kTrue;

	// get a pointer to the backbuffer so we can check its format -
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	pD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurfaceSrc );

	D3DSURFACE_DESC	Desc;
	pSurfaceSrc->GetDesc( &Desc );
	
	RELEASE_SAFELY( pSurfaceSrc );

	// now we're going to see about applying alphaed texture compression to this
	// get a pointer to the backbuffer so we can check its format -
	pD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pSurfaceSrc );

	pSurfaceSrc->GetDesc( &Desc );
	
	RELEASE_SAFELY( pSurfaceSrc );

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
												D3DFMT_DXT3 ) ) )
		{
			ConvertToFormat( pD3DDevice,
							D3DFMT_DXT3 );
		}
	}
#endif

	return S_OK;
} // CTexture::MergeAlpha()

#ifdef _USE_JPEG2000
// user JASPER to parse a JPG2000 file and copy it into a d3d texture
HRESULT CTexture::CreateJP2( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							  std::string TextureName,			// absolute path to WJ2
							  LPDIRECT3DTEXTURE8* TargetTexture,// D3D texture to fill
							  bool ColorKey,					// use colorkey?
							  DWORD ColorKeyValue )				// colorkey value
{
	HRESULT hr;

	// make sure it's there first
	if ( !FileExists( TextureName ) )
	{
		return E_FAIL;
	}
	jas_init();

	jas_stream_t*	Stream( jas_stream_fopen( TextureName.c_str(), "rb" ) );
	jas_image_t*	JImage( jas_image_decode( Stream, -1, "" ) ); // last is the option string whatto put there?
	if ( JImage == NULL )
	{
		return E_FAIL;
	}

	uint16	InternalFormat( JImage->numcmpts_ );

	uint16	Width( jas_image_width( JImage ) );
	uint16	Height( jas_image_height( JImage ) );

	unsigned char * Data = new unsigned char[InternalFormat * Width * Height];

	jas_stream_t*	Mem	= jas_stream_memopen( ( char* ) Data,
											  InternalFormat* Width * Height );

	JPEG2000PutData( Mem, JImage, InternalFormat );

	jas_image_destroy( JImage );
	jas_stream_close( Stream );
	jas_image_clearfmts();

	LPDIRECT3DTEXTURE8	pScratchTexture	= NULL;

	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							1,
							0,
							D3DFMT_A8R8G8B8,
							D3DPOOL_MANAGED,
							&pScratchTexture );

	// if we couldn't, clear the data and leave
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		DELETE_ARRAY_SAFELY( Data );
		return E_FAIL;
	}

	// ok, we know we can do it, so create a texture of the appropriate size
	// at the target texture we were passed
	hr = D3DXCreateTexture( pD3DDevice,
							Width,
							Height,
							D3DX_DEFAULT,
							0,
							D3DFMT_A8R8G8B8,
							D3DPOOL_MANAGED,
							TargetTexture );

	// conceivable we could fail this too, so kick out if we do
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		RELEASE_SAFELY( *TargetTexture );
		DELETE_ARRAY_SAFELY( Data );
		return E_FAIL;
	}


	// we need to get a surface desc to copy things properly
	D3DSURFACE_DESC	desc;
	hr = pScratchTexture->GetLevelDesc( 0, &desc );
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		RELEASE_SAFELY( *TargetTexture );
		DELETE_ARRAY_SAFELY( Data );
		return E_FAIL;
	}

	// lock our scratch texture
	D3DLOCKED_RECT	Rect;
	hr = pScratchTexture->LockRect( 0, &Rect, NULL, 0 ); //This allows our debugger to keep running

	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pScratchTexture );
		RELEASE_SAFELY( *TargetTexture );
		DELETE_ARRAY_SAFELY( Data );
		return E_FAIL;
	}

	// given the format of our JP2, copy the data into our scratch texture
	switch ( desc.Format )
	{
	case D3DFMT_A8R8G8B8:
		//32-bit ARGB, what we requested
		if ( InternalFormat == 4 )
		{
			FillAlphaTextureA8R8G8B8( Width, Height, &Rect, Data );
		}
		else
		{
			FillTextureA8R8G8B8( Width, Height, &Rect, Data );
		}
		break;
	case D3DFMT_R8G8B8:
		//32-bit ARGB, what we requested
		FillTextureR8G8B8( Width, Height, &Rect, Data );
		break;
	case D3DFMT_A4R4G4B4:
		//16-bit ARGB, not ideal, but not bad
		FillAlphaTextureA4R4G4B4( Width, Height, &Rect, Data );
		break;
	case D3DFMT_A1R5G5B5:
		//16-bit ARGB, not ideal, but not bad
		FillAlphaTextureA1R5G5B5( Width, Height, &Rect, Data );
		break;
	case D3DFMT_R5G6B5:
		//16-bit ARGB, not ideal, but not bad
		FillTextureR5G6B5( Width, Height, &Rect, Data );
		break;
	case D3DFMT_R3G3B2:
		// no alpha
		break;
	default:
		// no alpha
		break;
	};


	pScratchTexture->UnlockRect( 0 );
	DELETE_ARRAY_SAFELY( Data );


	// now we're going to do a surface-to-surface copy from
	// our scratch texture to the target texture.
	// why do this instead of creating it originally on the target texture?
	// this way, the mips will be properly generated! If you create it straight
	// to the texture, no mips.
	// also, this gives us the opportunity to specify a colorkey if need be
	IDirect3DSurface8*	pSurfaceSrc		= NULL;
	IDirect3DSurface8*	pSurfaceDest	= NULL;

	( *TargetTexture )->GetSurfaceLevel( 0, &pSurfaceDest );
	pScratchTexture->GetSurfaceLevel( 0, &pSurfaceSrc );

	// copy with a green colorkey
	if ( ColorKey )
	{
		hr = D3DXLoadSurfaceFromSurface( pSurfaceDest,
										 NULL,
										 NULL,
										 pSurfaceSrc,
										 NULL,
										 NULL,
										 D3DX_FILTER_LINEAR,
										 ColorKeyValue );
	}
	else	// or copy straight over
	{
		hr = D3DXLoadSurfaceFromSurface( pSurfaceDest,
										 NULL,
										 NULL,
										 pSurfaceSrc,
										 NULL,
										 NULL,
										 D3DX_FILTER_LINEAR,
										 0 );
	}

	// on failure, clean up our mess
	if ( FAILED( hr ) )
	{
		RELEASE_SAFELY( pSurfaceDest );
		RELEASE_SAFELY( pSurfaceSrc );
		RELEASE_SAFELY( pScratchTexture );
		RELEASE_SAFELY( *TargetTexture );
		return E_FAIL;
	}

	// clean up our surfaces
	RELEASE_SAFELY( pSurfaceDest );
	RELEASE_SAFELY( pSurfaceSrc );


	// and our scratch texture
	RELEASE_SAFELY( pScratchTexture );


	return S_OK;
} // CTexture::CreateJP2()

#endif


