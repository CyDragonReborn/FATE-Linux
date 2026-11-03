/*
	d3denumeration.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#define STRICT


#include <windows.h>
#include <D3D8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "d3dEnumeration.h"

// return the number of color bits in the specified format
static uint32 ColorChannelBits( D3DFORMAT Format )	// format to determine bits for
{
	switch ( Format )
	{
	case D3DFMT_R8G8B8:
		return 8;
	case D3DFMT_A8R8G8B8:
		return 8;
	case D3DFMT_X8R8G8B8:
		return 8;
	case D3DFMT_R5G6B5:
		return 5;
	case D3DFMT_X1R5G5B5:
		return 5;
	case D3DFMT_A1R5G5B5:
		return 5;
	case D3DFMT_A4R4G4B4:
		return 4;
	case D3DFMT_R3G3B2:
		return 2;
	case D3DFMT_A8R3G3B2:
		return 2;
	case D3DFMT_X4R4G4B4:
		return 4;
	case D3DFMT_A2B10G10R10:
		return 10;
	default:
		return 0;
	}
} // ColorChannelBits()

// return the number of alpha bits in the specified format
static uint32 AlphaChannelBits( D3DFORMAT Format )	// format to determine bits for
{
	switch ( Format )
	{
	case D3DFMT_R8G8B8:
		return 0;
	case D3DFMT_A8R8G8B8:
		return 8;
	case D3DFMT_X8R8G8B8:
		return 0;
	case D3DFMT_R5G6B5:
		return 0;
	case D3DFMT_X1R5G5B5:
		return 0;
	case D3DFMT_A1R5G5B5:
		return 1;
	case D3DFMT_A4R4G4B4:
		return 4;
	case D3DFMT_R3G3B2:
		return 0;
	case D3DFMT_A8R3G3B2:
		return 8;
	case D3DFMT_X4R4G4B4:
		return 0;
	case D3DFMT_A2B10G10R10:
		return 2;
	default:
		return 0;
	}
} // AlphaChannelBits()

// return the number of depth bits in the specified depth format
static uint32 DepthBits( D3DFORMAT Format )	// format to determine bits for
{
	switch ( Format )
	{
	case D3DFMT_D16:
		return 16;
	case D3DFMT_D15S1:
		return 15;
	case D3DFMT_D24X8:
		return 24;
	case D3DFMT_D24S8:
		return 24;
	case D3DFMT_D24X4S4:
		return 24;
	case D3DFMT_D32:
		return 32;
	default:
		return 0;
	}
} // DepthBits()


// return the number of stencil bits in the specified depth format
static uint32 StencilBits( D3DFORMAT Format )	// format to determine bits for
{
	switch ( Format )
	{
	case D3DFMT_D16:
		return 0;
	case D3DFMT_D15S1:
		return 1;
	case D3DFMT_D24X8:
		return 0;
	case D3DFMT_D24S8:
		return 8;
	case D3DFMT_D24X4S4:
		return 4;
	case D3DFMT_D32:
		return 0;
	default:
		return 0;
	}
} // StencilBits()



CD3DEnumeration::CD3DEnumeration( IDirect3D8* pD3D,						// direct3d object
								  uint32 MinimumScreenWidth,						// minimum screen width (default 320)
								  uint32 MinimumScreenHeight,						// minimum screen height (default 240)
								  uint32 MinimumColorBits,							// minimum color bits (default 5)
								  uint32 MinimumAlphaBits,							// minimum alpha bits (default 0)
								  uint32 MinimumDepthBufferBits,					// minimum depth(z) buffer bits (default 16)
								  uint32 MinimumStencilBufferBits ) :				// minimum stencil buffer bits (default 4)
								  m_pD3D( pD3D ),
								  m_MinFullscreenWidth( MinimumScreenWidth ),
								  m_MinFullscreenHeight( MinimumScreenHeight ),
								  m_MinColorChannelBits( MinimumColorBits ),
								  m_MinAlphaChannelBits( MinimumAlphaBits ),
								  m_MinDepthBits( MinimumDepthBufferBits ),
								  m_MinStencilBits( MinimumStencilBufferBits ),
								  m_UsesMixedVertexProcessing( kFalse )
{
} // CD3DEnumeration::CD3DEnumeration()

CD3DEnumeration::~CD3DEnumeration()
{
	for( uint32 i = 0; i < m_pAdapterInfoList.size(); i ++ )
	{
		DELETE_SAFELY( m_pAdapterInfoList[i] );
	}		
} // CD3DEnumeration::~CD3DEnumeration()

// enumerate all available devices that meet our minimum requirements
// we are ONLY enumerating HAL devices. We don't care about REF or software
HRESULT CD3DEnumeration::Enumerate( void )
{
	HRESULT		hr;	
	// we can only do this if we have a D3D device
	if ( m_pD3D == NULL )
	{
		return E_FAIL;
	}

	std::vector < D3DFORMAT > AdapterFormatList;

	uint32			Adapters		= m_pD3D->GetAdapterCount();

	D3DAdapterInfo*	pAdapterInfo = NULL;

	// we want to check the devices for every adapter
	for ( uint32 AdapterOrdinal = 0;
		  AdapterOrdinal < Adapters;
		  AdapterOrdinal++ )
	{
		// get info about this adapter
		pAdapterInfo = new D3DAdapterInfo();

		pAdapterInfo->m_AdapterOrdinal = AdapterOrdinal;
		m_pD3D->GetAdapterIdentifier( AdapterOrdinal,
									  0,
									  &pAdapterInfo->m_AdapterIdentifier );

		// clear our temporary list of adapter formats
		AdapterFormatList.resize( 0 );


		// find out how many display modes this adapter has
		uint32		AdapterModes			= m_pD3D->GetAdapterModeCount( AdapterOrdinal );
		for ( uint32 Mode = 0; Mode < AdapterModes; Mode++ )
		{
			D3DDISPLAYMODE	DisplayMode;

			// get display information for this mode
			m_pD3D->EnumAdapterModes( AdapterOrdinal,
									  Mode,
									  &DisplayMode );

			// if it doesn't meet our minimum requirements, we skip it
			if ( DisplayMode.Width < m_MinFullscreenWidth ||
				 DisplayMode.Height < m_MinFullscreenHeight ||
				 ColorChannelBits( DisplayMode.Format ) < m_MinColorChannelBits )
			{
				continue;
			}
			
			// otherwise we  add it to our list of available modes
			pAdapterInfo->m_pDisplayModeList.push_back( DisplayMode );
			
			// and add this display format to our format list
			AdapterFormatList.push_back( DisplayMode.Format );
		}



		// Get info for each device on this adapter based on our adapter info
		if ( FAILED( hr = EnumerateDevices( pAdapterInfo, AdapterFormatList ) ) )
		{
			DELETE_SAFELY( pAdapterInfo );
			return hr;
		}

		// If at least one device on this adapter is available and compatible
		// with the app, add the adapterInfo to the list
		if ( pAdapterInfo->m_pDeviceInfoList.size() != 0 )
		{
			m_pAdapterInfoList.push_back( pAdapterInfo );
		}
		else
		{
			DELETE_SAFELY( pAdapterInfo );
		}
	}
	return S_OK;
} // CD3DEnumeration::Enumerate()

// enumerate available devices on this adapter 
//
HRESULT CD3DEnumeration::EnumerateDevices( D3DAdapterInfo* pAdapterInfo,					// adapter to enumerate devices for
										   std::vector < D3DFORMAT >& pAdapterFormatList )	// list of compatible adapter formats
{

	HRESULT				hr;

	D3DDeviceInfo*	pDeviceInfo = NULL;
	
	pDeviceInfo = new D3DDeviceInfo;
	
	pDeviceInfo->m_AdapterOrdinal = pAdapterInfo->m_AdapterOrdinal;
	// only looking for hardware accelerated devices
	pDeviceInfo->m_DeviceType = D3DDEVTYPE_HAL;			

	// try to get the caps for this device
	if ( FAILED( hr = m_pD3D->GetDeviceCaps( pAdapterInfo->m_AdapterOrdinal,
											 pDeviceInfo->m_DeviceType,
											 &pDeviceInfo->m_Caps ) ) )
	{
		DELETE_SAFELY( pDeviceInfo );
		return hr;
	}
	
	// Get info for each devicecombo on this device
	// this finds the formats and capabilities available for this particular device
	if ( FAILED( hr = EnumerateDeviceCombos( pDeviceInfo,
											 pAdapterFormatList ) ) )
	{
		DELETE_SAFELY( pDeviceInfo );
		return hr;
	}
	
	// If at least one devicecombo for this device is found, 
	// add the deviceInfo to the list
	if ( pDeviceInfo->m_pDeviceComboList.size() == 0 )
	{
		DELETE_SAFELY( pDeviceInfo );
		return S_OK;
	}
	pAdapterInfo->m_pDeviceInfoList.push_back( pDeviceInfo );
	

	return S_OK;
} // CD3DEnumeration::EnumerateDevices()


// find out which formats and capabilities are available for a specific device
HRESULT CD3DEnumeration::EnumerateDeviceCombos( D3DDeviceInfo* pDeviceInfo,							// device to find combos for
												std::vector < D3DFORMAT >& pAdapterFormatList )		// list of compatible adapter formats
{
	// backbuffer formats to check
	const D3DFORMAT	BackBufferFormatsArray[]		=
	{
		D3DFMT_A8R8G8B8, 
		D3DFMT_X8R8G8B8,
		D3DFMT_R5G6B5, 
		D3DFMT_A1R5G5B5, 
		D3DFMT_X1R5G5B5
	};
	const uint32		BackBufferFormatCount	= sizeof( BackBufferFormatsArray ) /
												  sizeof( BackBufferFormatsArray[0] );
	
	bool			WindowedArray[]			=
	{
		kFalse, kTrue
	};

	// See which adapter formats are supported by this device
	D3DFORMAT		AdapterFormat;
	// go through all our compatible adapter formats
	for ( uint32 AdapterFormats = 0; AdapterFormats < pAdapterFormatList.size(); AdapterFormats++ )
	{
		// get a compatible format
		AdapterFormat = pAdapterFormatList[ AdapterFormats ];
		D3DFORMAT	BackBufferFormat;
		// go through all the formats we can try
		for ( uint32 CurrentFormat = 0; CurrentFormat < BackBufferFormatCount; CurrentFormat++ )
		{
			// current format to try
			BackBufferFormat = BackBufferFormatsArray[CurrentFormat];
			// make sure this format meets our minimum alpha bit requirements
			if ( AlphaChannelBits( BackBufferFormat ) < m_MinAlphaChannelBits )
			{
				continue;
			}

			// check whether the device is available for both fullscreen and windowed
			bool	IsWindowed = kFalse;
			for ( uint32 WindowedTypes = 0; WindowedTypes < 2; WindowedTypes++ )
			{
				IsWindowed = WindowedArray[WindowedTypes];

				// see if this device is available
				if ( FAILED( m_pD3D->CheckDeviceType( pDeviceInfo->m_AdapterOrdinal,
													  pDeviceInfo->m_DeviceType,
													  AdapterFormat,
													  BackBufferFormat,
													  IsWindowed ) ) )
				{
					continue;
				}

				// At this point, we have a DeviceCombo that is supported by the system.  
				// We still need to confirm that it's  compatible with the app, and find
				// one or more suitable depth/stencil buffer formats,
				// multisample type, vertex processing type, and presentation interval.
				
				// create a new device combo
				D3DDeviceCombo* pDeviceCombo = new D3DDeviceCombo();

				// insert the info pertaining to this device
				pDeviceCombo->m_AdapterOrdinal	= pDeviceInfo->m_AdapterOrdinal;
				pDeviceCombo->m_DeviceType			= pDeviceInfo->m_DeviceType;
				pDeviceCombo->m_AdapterFormat		= AdapterFormat;
				pDeviceCombo->m_BackBufferFormat		= BackBufferFormat;
				pDeviceCombo->m_IsWindowed		= IsWindowed;
				//  gather stencil formats available
				BuildDepthStencilFormatList( pDeviceCombo );
				// if we have no stencil formats, we fail
				if ( pDeviceCombo->m_pDepthStencilFormatList.size() == 0 )
				{
					DELETE_SAFELY( pDeviceCombo );
					continue;
				}
				// gather the multisample types available
				BuildMultiSampleTypeList( pDeviceCombo );
				// if we have none, we fail
				if ( pDeviceCombo->m_pMultiSampleTypeList.size() == 0 )
				{
					DELETE_SAFELY( pDeviceCombo );
					continue;
				}
				// gather the vertex processing types available
				BuildVertexProcessingTypeList( pDeviceInfo, pDeviceCombo );
				// if we have none, we fail
				if ( pDeviceCombo->m_pVertexProcessingTypeList.size() == 0 )
				{
					DELETE_SAFELY( pDeviceCombo );
					continue;
				}
				// gather presentation intervals available
				BuildPresentIntervalList( pDeviceInfo, pDeviceCombo );

				// and put it in the list, since we've succeeded
				pDeviceInfo->m_pDeviceComboList.push_back( pDeviceCombo );
			}
		}
	}
	return S_OK;
} // CD3DEnumeration::EnumerateDeviceCombos()

// gather the stencil formats available for a given device combo
void CD3DEnumeration::BuildDepthStencilFormatList( D3DDeviceCombo* pDeviceCombo )	// device combo to find types for
{
	const D3DFORMAT	DepthStencilFormatArray[]		=
	{
		D3DFMT_D16,
		D3DFMT_D15S1,
		D3DFMT_D24X8,
		D3DFMT_D24S8,
		D3DFMT_D24X4S4,
		D3DFMT_D32,
	};
	const uint32		DepthStencilFormatArrayCount	=	sizeof( DepthStencilFormatArray ) /
															sizeof( DepthStencilFormatArray[0] );

	D3DFORMAT		DepthStencilFormat;

	// for each of the possible formats, we check to see if it's available on this device
	for ( uint32 CurrentFormat = 0; CurrentFormat < DepthStencilFormatArrayCount; CurrentFormat++ )
	{
		DepthStencilFormat = DepthStencilFormatArray[CurrentFormat];
		// if the depth buffer doesn't meet our minimum requirements, we can't use it
		if ( DepthBits( DepthStencilFormat ) < m_MinDepthBits )
		{
			continue;
		}
		// if the stencil buffer doesn't meet our minimum requirements, we can't use it
		if ( StencilBits( DepthStencilFormat ) < m_MinStencilBits )
		{
			continue;
		}

		// if this format exists for the device
		if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( pDeviceCombo->m_AdapterOrdinal,
												   pDeviceCombo->m_DeviceType,
												   pDeviceCombo->m_AdapterFormat,
												   D3DUSAGE_DEPTHSTENCIL,
												   D3DRTYPE_SURFACE,
												   DepthStencilFormat ) ) )
		{
			// and the stencil format is also available
			if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( pDeviceCombo->m_AdapterOrdinal,
															pDeviceCombo->m_DeviceType,
															pDeviceCombo->m_AdapterFormat,
															pDeviceCombo->m_BackBufferFormat,
															DepthStencilFormat ) ) )
			{
				// we add it to our list of available stencil formats
				pDeviceCombo->m_pDepthStencilFormatList.push_back( DepthStencilFormat );
			}
		}
	}
} // CD3DEnumeration::BuildDepthStencilFormatList()




// gather the multisample types available for a given device combo
void CD3DEnumeration::BuildMultiSampleTypeList( D3DDeviceCombo* pDeviceCombo )	// device combo to find types for
{
	// possible types
	const D3DMULTISAMPLE_TYPE	MultiSampleTypeArray[]		=
	{
		D3DMULTISAMPLE_NONE,
		D3DMULTISAMPLE_2_SAMPLES,
		D3DMULTISAMPLE_3_SAMPLES,
		D3DMULTISAMPLE_4_SAMPLES,
		D3DMULTISAMPLE_5_SAMPLES,
		D3DMULTISAMPLE_6_SAMPLES,
		D3DMULTISAMPLE_7_SAMPLES,
		D3DMULTISAMPLE_8_SAMPLES,
		D3DMULTISAMPLE_9_SAMPLES,
		D3DMULTISAMPLE_10_SAMPLES,
		D3DMULTISAMPLE_11_SAMPLES,
		D3DMULTISAMPLE_12_SAMPLES,
		D3DMULTISAMPLE_13_SAMPLES,
		D3DMULTISAMPLE_14_SAMPLES,
		D3DMULTISAMPLE_15_SAMPLES,
		D3DMULTISAMPLE_16_SAMPLES,
	};
	const uint32					MultiSampleTypeArrayCount	= sizeof( MultiSampleTypeArray ) /
																  sizeof( MultiSampleTypeArray[0] );

	D3DMULTISAMPLE_TYPE			MultiSampleType;

	for ( uint32 CurrentSample = 0; CurrentSample < MultiSampleTypeArrayCount; CurrentSample++ )
	{
		MultiSampleType = MultiSampleTypeArray[CurrentSample];
		// if this multisample type exists on the device
		if ( SUCCEEDED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->m_AdapterOrdinal,
															pDeviceCombo->m_DeviceType,
															pDeviceCombo->m_BackBufferFormat,
															pDeviceCombo->m_IsWindowed,
															MultiSampleType ) ) )
		{
			// add it to the list of available ones
			pDeviceCombo->m_pMultiSampleTypeList.push_back( MultiSampleType );
		}
	}
} // CD3DEnumeration::BuildMultiSampleTypeList()

// gather the vertex processing types available for a given device and device combo
void CD3DEnumeration::BuildVertexProcessingTypeList( D3DDeviceInfo* pDeviceInfo,		// device to check availability on
													 D3DDeviceCombo* pDeviceCombo )		// device combo to check availability on
{
	// if the device is capable of Hardware T&L
	if ( ( pDeviceInfo->m_Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0 )
	{
		// and can do pure hardware Vertex Processing
		// add a pure vp type to the list
		if ( ( pDeviceInfo->m_Caps.DevCaps & D3DDEVCAPS_PUREDEVICE ) != 0 )
		{
			pDeviceCombo->m_pVertexProcessingTypeList.push_back( KVPPureHardware );
		}

		// we can at least do hardware
		pDeviceCombo->m_pVertexProcessingTypeList.push_back( KVPHardware );

		// and if required, mixed vp
		if ( m_UsesMixedVertexProcessing )
		{
			pDeviceCombo->m_pVertexProcessingTypeList.push_back( KVPMixed );
		}
	}

	// and of course, everbody can do software
	pDeviceCombo->m_pVertexProcessingTypeList.push_back( KVPSoftware );

} // CD3DEnumeration::BuildVertexProcessingTypeList()

// gather the presentation intervals available for a given device and device combo
void CD3DEnumeration::BuildPresentIntervalList( D3DDeviceInfo* pDeviceInfo,		// device to check availability on
												D3DDeviceCombo* pDeviceCombo )	// device combo to check availability on
{
	// available intervals
	const uint32	PresentationIntervalArray[]		=
	{
		D3DPRESENT_INTERVAL_IMMEDIATE,
		D3DPRESENT_INTERVAL_DEFAULT,
		D3DPRESENT_INTERVAL_ONE,
		D3DPRESENT_INTERVAL_TWO,
		D3DPRESENT_INTERVAL_THREE,
		D3DPRESENT_INTERVAL_FOUR,
	};
	const uint32	PresentationIntervalArrayCount	= sizeof( PresentationIntervalArray ) / sizeof( PresentationIntervalArray[0] );

	uint32		PresentationInterval;
	// check all the possible presentation intervals
	for ( uint32 CurrentInterval = 0; CurrentInterval < PresentationIntervalArrayCount; CurrentInterval++ )
	{
		PresentationInterval = PresentationIntervalArray[CurrentInterval];
		// if windowed, the following three intervals are not supported
		if ( pDeviceCombo->m_IsWindowed )
		{
			if ( PresentationInterval == D3DPRESENT_INTERVAL_TWO ||
				 PresentationInterval == D3DPRESENT_INTERVAL_THREE ||
				 PresentationInterval == D3DPRESENT_INTERVAL_FOUR )
			{
				continue;
			}
		}
		// if the device is capable of it, or it is the default (0), put it in the list
		if ( PresentationInterval == D3DPRESENT_INTERVAL_DEFAULT ||
			 ( pDeviceInfo->m_Caps.PresentationIntervals & PresentationInterval ) )
		{
			pDeviceCombo->m_pPresentIntervalList.push_back( PresentationInterval );
		}
	}
} // CD3DEnumeration::BuildPresentIntervalList()
