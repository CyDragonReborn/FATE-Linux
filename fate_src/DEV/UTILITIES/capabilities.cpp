/*
	capabilities.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#include <windows.h>
#include <string>
#include <D3D8.h>
#include "D3DX8.h"

using namespace std;

#include "utilities.h"
#include "constants.h"
#include "capabilities.h"
#include "macros.h"

#include "vertexdefinitions.h"

CCapabilities::CCapabilities( void ) :
							  m_DetectionComplete( kFalse ),
							  m_IsIntelCard( kFalse ),
							  m_IsIntegratedCard( kFalse ),
							  m_VRAM( 0 ),
							  m_AdapterName( "UNKNOWN" )

{
	// reset our capability tables
	ResetCapabilityTables();
} // CCapabilities::CCapabilities()

CCapabilities::~CCapabilities( void )
{
} // CCapabilities::~CCapabilities()

// return the amount of physical RAM available in MB

uint32 CCapabilities::GetAvailableRAM( void )
{
	MEMORYSTATUS	MemoryStatus;

	memset( &MemoryStatus, sizeof( MEMORYSTATUS ), 0 );
	MemoryStatus.dwLength = sizeof( MEMORYSTATUS );

	GlobalMemoryStatus( &MemoryStatus );

	return MemoryStatus.dwTotalPhys / 1024 / 1024;
} // CCapabilities::GetAvailableRAM()

// return the amount of Video RAM available in MB

uint32 CCapabilities::GetAvailableVRAM( void )
{
	if( !m_DetectionComplete )
	{
		MessageBox( NULL, 
					"VRAM requested before D3D setup completed", 
					"DETECTION ERROR", 
					MB_ICONERROR|MB_OK );


	}

	return m_VRAM;
} // CCapabilities::GetAvailableVRAM()

// return the processor speed, in MHz
// -1 if no speed could be determined

int32 CCapabilities::GetProcessorSpeed( void )
{
	LONG		Result;
	HKEY		RegistryKey;
	DWORD		Data;
	DWORD		DataSize;

	// Get the processor speed info.

	Result = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
							 "Hardware\\Description\\System\\CentralProcessor\\0",
							 0,
							 KEY_QUERY_VALUE,
							 &RegistryKey );

	// Check if the function has succeeded.

	if ( Result == ERROR_SUCCESS )
	{
		Result = ::RegQueryValueEx( RegistryKey,
									"~MHz",
									NULL,
									NULL,
									( LPBYTE ) & Data,
									&DataSize );

		return Data;
	}

	// Make sure to close the reg key

	RegCloseKey( RegistryKey );

	return -1;
} // CCapabilities::GetProcessorSpeed()

// return the current OS of this machine
EOSVersions CCapabilities::GetWindowsVersion( void )
{
	OSVERSIONINFO	OSVersion ;
	ZeroMemory( &OSVersion, sizeof( OSVersion ) );
	OSVersion.dwOSVersionInfoSize = sizeof( OSVersion );

	if ( GetVersionEx( &OSVersion ) )
	{
		switch( OSVersion.dwMajorVersion )
		{
			// 95 based OS'es
		case 4:

			switch( OSVersion.dwMinorVersion )
			{
			case 0:
				return KWindows95;

			case 10:
				return KWindows98;

			case 90:
				return KWindowsME;

			case 51:
				return KWindowsNT40;

			default:
				return KOSUnknown;
			}

			// NT 3.51
		case 3:
			return KWindowsNT35;

			// 2000 based OSes
		case 5:
			
			switch( OSVersion.dwMinorVersion )
			{
			case 0:
				return KWindows2000;

			case 1:
				return KWindowsXP;

			case 2:
				return KWindows2004Server;

			default:
				return KOSUnknown;
			}

		default:
			return KOSUnknown;
		}
	}
	return KOSUnknown;
} // CCapabilities::GetWindowsVersion()

void CCapabilities::ResetCapabilityTables( void )
{
	for( uint32 i = 0; i < KCapabilitiesAvailableCount; i++ )
	{
		m_Capable[i] = kTrue;
	}
	for( int32 i = 0; i < KCapabilitiesValueCount; i++ )
	{
		m_CapabilityValue[i] = 0;
	}
} // CCapabilities::ResetCapabilityTables()

// determine base D3D capabilities
void CCapabilities::DetectD3DCapabilities( LPDIRECT3D8	pD3D,				// d3d object
										   LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
									       D3DPRESENT_PARAMETERS* pD3Dpp )	// d3d presentation parameters

{
	// reset our capability tables
	ResetCapabilityTables();

	// we set this now so that the followind detections can reference other detections.
	m_DetectionComplete = kTrue;

	// get the device capabilities
	D3DCAPS8	d3dCaps;
	pD3DDevice->GetDeviceCaps( &d3dCaps );

	// retrieve the adapter's name, for any hacks we may need to do
	D3DADAPTER_IDENTIFIER8 AdapterIdentifier;
	
	pD3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT,
								0,
								&AdapterIdentifier );

	m_AdapterName = AdapterIdentifier.Description;

	// put it in uppercase for easier compare later
	m_AdapterName = StringUpper( m_AdapterName );

	// calculate video memory
	m_VRAM = pD3DDevice->GetAvailableTextureMem() / 1024 / 1024;

	if ( m_AdapterName.length() >= 5 &&
		 m_AdapterName.substr( 0, 5 ) == "INTEL" )
	{
		m_IsIntelCard = kTrue;
	}

	if( StringUpper( m_AdapterName ).find( "INTEGRATED" ) != -1 )
	{
		m_IsIntegratedCard = kTrue;
	}


	// how many texture layers can be rendered at once?
	SetCapabilityValue( KValueMaxSimultaneousTextures, d3dCaps.MaxSimultaneousTextures );

	// how many texture layers can be rendered at once?
	SetCapabilityValue( KValueMaxSimultaneousBlendStages, d3dCaps.MaxTextureBlendStages );

	// how many simultaneous lights are possible per-mesh
	SetCapabilityValue( KValueMaxLights, d3dCaps.MaxActiveLights );
	// sometimes we get funky values, so cap them
	// we always want at least 1 light, even if it's not hardware,
	// so that we can manage SOME sort of scene illumination
	if ( GetCapabilityValue( KValueMaxLights ) < 1 )
	{
		SetCapabilityValue( KValueMaxLights, 1 );
	}


	// what is the best MIP filtering we can hope for?
	SetCapabilityValue( KValueMipFilter, D3DTEXF_POINT );
	if( !( d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR ) )
	{
		SetCapabilityValue( KValueMipFilter, D3DTEXF_POINT );
	}
	else
	{
		SetCapabilityValue( KValueMipFilter, D3DTEXF_LINEAR );
	}

	// ok, detection of whether we can render to a texture takes a bit. There's lots of exceptions
	// and screw-ups in drivers. So we start assuming we can, and knock off the impossibilites
	// one by one.
	SetCapabilityValue( KValueMaxRenderToTextureSize, 1024 );

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

	HRESULT hr =  pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
												D3DDEVTYPE_HAL,
												Desc.Format,
												D3DUSAGE_RENDERTARGET,
												D3DRTYPE_TEXTURE,
												Desc.Format );
	if( FAILED( hr ) )
	{
		SetCapable( KCanRenderToTexture, kFalse );
	}

	// get a pointer to the backbuffer so we can check its format -
	// we're going to see if we support texture compression, which will
	// save us MUCHO MEMORY!
	hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
								  D3DDEVTYPE_HAL,
								  Desc.Format,
								  0,
								  D3DRTYPE_TEXTURE,
								  D3DFMT_DXT1 );
	if( FAILED( hr ) )
	{
		SetCapable( KCanDoDXT1, kFalse );
	}

	hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
								  D3DDEVTYPE_HAL,
								  Desc.Format,
								  0,
								  D3DRTYPE_TEXTURE,
								  D3DFMT_DXT2 );
	if( FAILED( hr ) )
	{
		SetCapable( KCanDoDXT2, kFalse );
	}

	hr = pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
							      D3DDEVTYPE_HAL,
							 	  Desc.Format,
								  0,
								  D3DRTYPE_TEXTURE,
								  D3DFMT_DXT3 );
	if( FAILED( hr ) )
	{
		SetCapable( KCanDoDXT3, kFalse );
	}

/*
	if ( !( d3dCaps.Caps2 & D3DCAPS2_DYNAMICTEXTURES ) )
	{
		SetCapable( KCanRenderToTexture, kFalse );
	}*/

	if( StringUpper( m_AdapterName ).find( "MATROX MILLENNIUM G450" ) != -1 )
	{
		SetCapable( KCanRenderToTexture, kFalse );
	}

	if ( m_AdapterName.find( "VIPER" ) != -1 ||
		 m_AdapterName.find( "VOODOO" ) != -1  )
	{
		SetCapable( KCanDoDXT1, kFalse );
		SetCapable( KCanDoDXT2, kFalse );
		SetCapable( KCanDoDXT3, kFalse );
	}

	// these savage cards act like they can - but they can't. Trust me.
	if ( m_AdapterName.length() >= 21 &&
		 m_AdapterName.substr( 0, 21 ) ==
		 "S3 GRAPHICS SAVAGE/IX" )
	{
		SetCapable( KCanRenderToTexture, kFalse );
	}
	if ( m_AdapterName.length() >= 26 &&
		 m_AdapterName.substr( 0, 26 ) ==
		 "S3 GRAPHICS SUPERSAVAGE/IX" )
	{
		SetCapable( KCanRenderToTexture, kFalse );
	}


	// some cards can't render a large texture well though, so we knock them down to a smaller max size
	if( IsCapable( KCanRenderToTexture ) )
	{
		if ( m_AdapterName.length() >= 8 &&
			m_AdapterName.substr( 0, 8 ) ==
			"MOBILITY" )
		{
			SetCapabilityValue( KValueMaxRenderToTextureSize, 256 );
		}
	}

	// can we do stencil shadows?
	// some cards sey they can, but they can't
	if ( m_AdapterName.length() >= 21 &&
		 m_AdapterName.substr( 0, 21 ) ==
		 "DIAMOND VIPER II Z200" )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}

	if ( m_AdapterName.length() >= 18 &&
		 m_AdapterName.substr( 0, 18 ) ==
		 "TRIDENT CYBERBLADE" )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}

	// we need these stencilcaps to actually do the shadow casting
	if ( !( d3dCaps.StencilCaps & D3DSTENCILCAPS_KEEP ) )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}
	if ( !( d3dCaps.StencilCaps & D3DSTENCILCAPS_INCR ) )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}
	if ( !( d3dCaps.StencilCaps & D3DSTENCILCAPS_INCRSAT ) )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}
	if ( !( d3dCaps.StencilCaps & D3DSTENCILCAPS_DECR ) )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}
	if ( !( d3dCaps.ZCmpCaps & D3DPCMPCAPS_LESSEQUAL ) )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}
	if ( !( d3dCaps.ZCmpCaps & D3DPCMPCAPS_ALWAYS ) )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}

	// and our current device has to have a workable depth stencil format
	uint32 StencilDepth( 0 );
	switch ( pD3Dpp->AutoDepthStencilFormat )
	{
	case D3DFMT_D16:
		StencilDepth = 0;
		break;
	case D3DFMT_D15S1:
		StencilDepth = 1;
		break;
	case D3DFMT_D24X8:
		StencilDepth = 0;
		break;
	case D3DFMT_D24S8:
		StencilDepth = 8;
		break;
	case D3DFMT_D24X4S4:
		StencilDepth = 4;
		break;
	case D3DFMT_D32:
		StencilDepth = 0;
		break;
	default:
		StencilDepth = 0;
		break;
	}

	if ( StencilDepth <= 0 )
	{
		SetCapable( KCanDoStencilShadows, kFalse );
	}

	// support modalphaadd? ( used for reflection mapping - the alpha mods the added reflection map )
	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR ) )
	{
		SetCapable( KCanDoModAlphaAdd, kFalse );
	}

	// cube mapping supported?
	if ( !( d3dCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP ) )
	{
		SetCapable( KCanDoCubeMap, kFalse );
	}

	// got to be able to have a second layer to do it
	if( d3dCaps.MaxSimultaneousTextures < 2 )
	{
		SetCapable( KCanDoCubeMap, kFalse );
	}

	// some cards that say they can really can't
	if ( m_AdapterName.length() >=8 &&
		 m_AdapterName.substr( 0, 8 ) ==
		 "MOBILITY" )
	{
		SetCapable( KCanDoCubeMap, kFalse );
	}

	if ( m_AdapterName.length() >= 18 &&
		 m_AdapterName.substr( 0, 18 ) ==
		 "TRIDENT CYBERBLADE" )
	{
		SetCapable( KCanDoCubeMap, kFalse );
	}

	// let's also validate that the card can actually perform the texcoord operations to do a cubemap
	pD3DDevice->SetTextureStageState( 1,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_DISABLE );
	pD3DDevice->SetTextureStageState( 1,
									  D3DTSS_TEXCOORDINDEX,
									  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
	
	DWORD	NumPasses;
	hr =  pD3DDevice->ValidateDevice( &NumPasses );

	if ( !( ( D3D_OK == hr ) || ( D3DERR_DEVICELOST == hr ) ) )
	{
		SetCapable( KCanDoCubeMap, kFalse );
	}

	// can we do specular reflection mapping?
	if( !IsCapable( KCanDoCubeMap ) || !IsCapable( KCanDoModAlphaAdd ) )
	{
		SetCapable( KCanDoReflectionSpecularMapping, kFalse );
	}
	else
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 1,
										D3DTSS_TEXCOORDINDEX,
										D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 1,
										D3DTSS_COLOROP,
										D3DTOP_MODULATEALPHA_ADDCOLOR );

		pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
		pD3DDevice->SetTextureStageState( 2,
										D3DTSS_COLOROP,
										D3DTOP_MODULATEALPHA_ADDCOLOR );

		hr =  pD3DDevice->ValidateDevice( &NumPasses );
		
		if ( !( ( D3D_OK == hr ) || ( D3DERR_DEVICELOST == hr ) ) )
		{
			SetCapable( KCanDoReflectionSpecularMapping, kFalse );
		}
	}
	// can we do plain specular mapping?
	if( !IsCapable( KCanDoModAlphaAdd ) )
	{
		SetCapable( KCanDoSpecularMapping, kFalse );
	}
	else
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
		pD3DDevice->SetTextureStageState( 1,
										  D3DTSS_COLOROP,
										  D3DTOP_MODULATEALPHA_ADDCOLOR );
	
		pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kFalse );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
		pD3DDevice->SetTextureStageState( 2,
										D3DTSS_COLOROP,
										D3DTOP_MODULATEALPHA_ADDCOLOR );

		hr =  pD3DDevice->ValidateDevice( &NumPasses );
		
		if ( !( ( D3D_OK == hr ) || ( D3DERR_DEVICELOST == hr ) ) )
		{
			SetCapable( KCanDoSpecularMapping, kFalse );
		}
	}

	// TRASH render
	pD3DDevice->BeginScene();
	D3DSIMPLEVERTEX verts[ 3 ];
	pD3DDevice->SetVertexShader( D3DFVF_D3DSIMPLEVERTEX );
	pD3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, verts, sizeof( D3DSIMPLEVERTEX ) );
	pD3DDevice->EndScene();

	// can we do texture projection?
	if ( !( d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED ) )
	{
		SetCapable( KCanDoTextureProjection, kFalse );
	}
	else
	{
		
		pD3DDevice->SetTextureStageState( 1,
			D3DTSS_TEXTURETRANSFORMFLAGS,
			D3DTTFF_COUNT3 |
			D3DTTFF_PROJECTED );
		pD3DDevice->SetTextureStageState( 1,
			D3DTSS_TEXCOORDINDEX,
			D3DTSS_TCI_CAMERASPACEPOSITION |
			2 );
		
		hr =  pD3DDevice->ValidateDevice( &NumPasses );
		
		if ( !( ( D3D_OK == hr ) || ( D3DERR_DEVICELOST == hr ) ) )
		{
			SetCapable( KCanDoTextureProjection, kFalse );
		}
		
		// some cards say they can, but can't
		if ( m_AdapterName.length() >= 8 &&
			m_AdapterName.substr( 0, 8 ) ==
			"AG315-64" )
		{
			SetCapable( KCanDoTextureProjection, kFalse );
		}
		
		// if we can't clamp the border UV's in some way, we don't want to do texture
		// projection either, otherwise we get nasty 'hall of mirrors' effects
		
		SetCapabilityValue( KValueClampUVs, D3DTADDRESS_BORDER );
		if ( !( d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER ) )
		{
			if ( !( d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP ) )
			{
				SetCapable( KCanDoTextureProjection, kFalse );
			}
			else
			{
				SetCapabilityValue( KValueClampUVs, D3DTADDRESS_CLAMP );
			}
		}
		else
		{
			SetCapabilityValue( KValueClampUVs, D3DTADDRESS_BORDER );
		}
	}


	// can we do texture projection?
	if ( !( d3dCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED ) )
	{
		SetCapable( KCanDoShadowTextureProjection, kFalse );
	}

	// might REALLY need to do device validation here - but fails often!

	// HW TNL required for shadows, period
	if ( !( d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) )
	{
		/// rrr! Maybe not!
	//	SetCapable( KCanDoShadowTextureProjection, kFalse );
	//	SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}


	// this card crashes if you attempt to do texture addressing
	if( StringUpper( m_AdapterName ).find( "INTEL(R) 82810" ) != -1 )
	{
		SetCapable( KCanRenderToTexture, kFalse );
		SetCapable( KCanDoShadowTextureProjection, kFalse );
		SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}
	if( StringUpper( m_AdapterName ).find( "MATROX MILLENNIUM G400" ) != -1 )
	{
		SetCapable( KCanDoShadowTextureProjection, kFalse );
		SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}
	if( StringUpper( m_AdapterName ).find( "MATROX MILLENNIUM G450" ) != -1 )
	{
		SetCapable( KCanDoShadowTextureProjection, kFalse );
		SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}
	if( StringUpper( m_AdapterName ).find( "3DFX VOODOO" ) != -1 )
	{
		SetCapable( KCanDoShadowTextureProjection, kFalse );
		SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}
	if( StringUpper( m_AdapterName ).find( "VOODOO3" ) != -1 )
	{
		SetCapable( KCanDoShadowTextureProjection, kFalse );
		SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}
	if( StringUpper( m_AdapterName ).find( "DIAMOND VIPER II Z200" ) != -1 )
	{
		SetCapable( KCanDoShadowTextureProjection, kFalse );
		SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
	}

	// if we can't clamp the border UV's in some way, we don't want to do texture
	// projection either, otherwise we get nasty 'hall of mirrors' effects
		
	SetCapabilityValue( KValueClampUVs, D3DTADDRESS_BORDER );
	if ( !( d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER ) )
	{
		if ( !( d3dCaps.TextureAddressCaps & D3DPTADDRESSCAPS_CLAMP ) )
		{
			SetCapable( KCanDoShadowTextureProjection, kFalse );
			SetCapable( KCanDoOrthoShadowTextureProjection, kFalse );
		}
		else
		{
			SetCapabilityValue( KValueClampUVs, D3DTADDRESS_CLAMP );
		}
	}
	else
	{
		SetCapabilityValue( KValueClampUVs, D3DTADDRESS_BORDER );
	}

	// do we support additive blending?
	if ( !( d3dCaps.SrcBlendCaps & D3DPBLENDCAPS_ONE ) )
	{
		SetCapable( KCanDoAdditive, kFalse );
	}
	if ( !( d3dCaps.DestBlendCaps & D3DPBLENDCAPS_ONE ) )
	{
		SetCapable( KCanDoAdditive, kFalse );
	}

	// support AddSigned 2x mapping
	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADDSIGNED2X ) )
	{
		SetCapable( KCanDoAdditiveSigned2X, kFalse );
	}

	// support addsmooth
	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_ADDSMOOTH ) )
	{
		SetCapable( KCanDoAdditiveSmooth, kFalse );
	}

	// support Modulate 2x mapping
	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATE2X ) )
	{
		SetCapable( KCanDoModulate2X, kFalse );
	}

	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATE4X ) )
	{
		SetCapable( KCanDoModulate4X, kFalse );
	}

	// can we fade out textures with alpha?
	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BLENDFACTORALPHA ) )
	{
		SetCapable( KCanFadeAlpha, kFalse );
	}

	// support DOT3 mapping
	if ( !( d3dCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3 ) )
	{
		SetCapable( KCanDoDot3, kFalse );
	}

	// does the card support fogtable?
	if ( !( d3dCaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE ) )
	{
		SetCapable( KCanDoFogTable, kFalse );
	}
	if ( !( d3dCaps.RasterCaps & D3DPRASTERCAPS_ZFOG ) )
	{
		SetCapable( KCanDoFogTable, kFalse );
	}

	// can we use pointsprites?
	if ( d3dCaps.MaxPointSize <= 1.0f )
	{
		SetCapable( KCanDoPointSprites, kFalse );
	}


	// can we write with alpha to the zbuffer?
	if ( !( d3dCaps.AlphaCmpCaps & D3DPCMPCAPS_GREATER ) )
	{
		SetCapable( KCanAlphaTest, kFalse );
	}

	// do we support user-defined clipplanes?
	// is this a geforce 4?
	bool IsGEForce4 = kFalse;

	if( m_AdapterName.length() >= 15 && 
		m_AdapterName.substr( 0, 15 ) ==
		"NVIDIA GEFORCE4" )
	{
		IsGEForce4 = kTrue;
	}

	// for some reason, some GEFORCE4's say they have no clip planes, but they do.
	if ( d3dCaps.MaxUserClipPlanes <= 0 && !IsGEForce4 )
		
	{
		SetCapable( KCanDoClipPlanes, kFalse );
	}

	// some cards say they can, but can't -

	if ( m_AdapterName.length() >= 16 &&
		 m_AdapterName.substr( 0, 16 ) ==
		 "NVIDIA QUADRO FX" )
	{
		SetCapable( KCanDoClipPlanes, kFalse );
	}
	if ( m_AdapterName.length() >= 17 &&
		 m_AdapterName.substr( 0, 17 ) ==
		 "NVIDIA GEFORCE FX" )
	{
		SetCapable( KCanDoClipPlanes, kFalse );
	}

} // CCapabilities::DetectD3DCapabilities()

// get the name of the video card
std::string CCapabilities::GetAdapterName( void )
{
	if( !m_DetectionComplete )
	{
		MessageBox( NULL, 
					"Adapter name requested before D3D setup completed", 
					"DETECTION ERROR", 
					MB_ICONERROR|MB_OK );


	}

	return m_AdapterName;
} // CCapabilities::GetAdapterName()

int32 CCapabilities::GetCapabilityValue( ECapabilitiesValues Capability )	// which capability to check from ECapabilitiesCapabilitys
{
	if( !m_DetectionComplete )
	{
		MessageBox( NULL, 
					"D3D Capability Value requested before D3D setup completed", 
					"DETECTION ERROR", 
					MB_ICONERROR|MB_OK );
	}
	return m_CapabilityValue[Capability];
} // CCapabilities::GetCapabilityValue()


bool CCapabilities::IsCapable( ECapabilitiesAvailable Capability )	// which capability to check from ECapabilitiesAvailable
{
	if( !m_DetectionComplete )
	{
		MessageBox( NULL, 
					"D3D Capability requested before D3D setup completed", 
					"DETECTION ERROR", 
					MB_ICONERROR|MB_OK );
	}

	return m_Capable[Capability];

} // CCapabilities::IsCapable()

void CCapabilities::SetCapable( ECapabilitiesAvailable Capability,	// which capability to set
								bool Value )						// value to set 
{
	m_Capable[Capability] = Value;
} // CCapabilities::SetCapable()

void CCapabilities::SetCapabilityValue( ECapabilitiesValues Capability,		// which capability to set
										int32 Value )						// value to set 
{
	m_CapabilityValue[Capability] = Value;
} // CCapabilities::SetCapable()