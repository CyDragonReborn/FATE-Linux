/*
	DeviceEnumeration.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	06/22/2005

	Based on original work by Travis Baldree
*/

#include "WT/stdafx.h"

#define STRICT

#include <windows.h>
#include <d3d9.h>
#include "WT/Application/DeviceEnumeration.h"

namespace WT
{
namespace Application
{

// Retrieves the number of color bits in the specified format
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
}

// Retrieves the number of alpha bits in the specified format
static uint32 AlphaChannelBits( D3DFORMAT Format )
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
}

// Retrieves the number of depth bits in the specified depth format
static uint32 DepthBits( D3DFORMAT Format )
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
}

// Retrieves the number of stencil bits in the specified depth format
static uint32 StencilBits( D3DFORMAT Format )
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
} 


CDeviceEnumeration::CDeviceEnumeration( IDirect3D9* pD3D,						// Direct3D object
									  uint32 MinimumScreenWidth,			// minimum screen width (default 320)
									  uint32 MinimumScreenHeight,			// minimum screen height (default 240)
									  uint32 MinimumColorBits,				// minimum color bits (default 5)
									  uint32 MinimumAlphaBits,				// minimum alpha bits (default 0)
									  uint32 MinimumDepthBufferBits,		// minimum depth(z) buffer bits (default 16)
									  uint32 MinimumStencilBufferBits ) 	// minimum stencil buffer bits (default 4)
	: m_pD3D( pD3D )
	, m_MinFullscreenWidth( MinimumScreenWidth )
	, m_MinFullscreenHeight( MinimumScreenHeight )
	, m_MinColorChannelBits( MinimumColorBits )
	, m_MinAlphaChannelBits( MinimumAlphaBits )
	, m_MinDepthBits( MinimumDepthBufferBits )
	, m_MinStencilBits( MinimumStencilBufferBits )
	, m_UsesMixedVertexProcessing( false )
{
}

CDeviceEnumeration::~CDeviceEnumeration()
{
	for( uint32 i = 0; i < m_AdapterInfoList.size(); ++i )
	{
		SafeDelete( m_AdapterInfoList[i] );
	}		
}

// Enumerate all available devices that meet our minimum requirements
// Only HAL devices are enumerated (REF and software devices are not)
HRESULT CDeviceEnumeration::Enumerate()
{
	HRESULT hr;	

	// we can only do this if we have a D3D device
	if ( m_pD3D == NULL )
	{
		return E_FAIL;
	}

	std::vector < D3DFORMAT > AdapterFormatList;
	AdapterInfo* pAdapterInfo = NULL;

	// we want to check the devices for every adapter
	const uint32 AdapterCount = m_pD3D->GetAdapterCount();
	for ( uint32 AdapterOrdinal = 0; AdapterOrdinal < AdapterCount; ++AdapterOrdinal )
	{
		// get info about this adapter
		pAdapterInfo = new AdapterInfo();
		pAdapterInfo->AdapterOrdinal = AdapterOrdinal;
		m_pD3D->GetAdapterIdentifier( AdapterOrdinal, 0, &pAdapterInfo->AdapterIdentifier );

		// clear our temporary list of adapter formats
		AdapterFormatList.resize( 0 );

		D3DFORMAT AllowedAdapterFormats[ ] = { D3DFMT_A1R5G5B5,
											   D3DFMT_A2B10G10R10,
											   D3DFMT_A8R8G8B8,
											   D3DFMT_R5G6B5,
											   D3DFMT_X1R5G5B5,
											   D3DFMT_X8R8G8B8,
                                             };

		const uint32 AllowedAdapterFormatsCount = sizeof(AllowedAdapterFormats) /
												  sizeof(AllowedAdapterFormats[0]);

		for( uint32 AdapterFormat = 0; AdapterFormat < AllowedAdapterFormatsCount; ++AdapterFormat )
		{

			// find out how many display modes this adapter has
			const uint32 AdapterModes = m_pD3D->GetAdapterModeCount( AdapterOrdinal, AllowedAdapterFormats[AdapterFormat] );
			for ( uint32 Mode = 0; Mode < AdapterModes; ++Mode )
			{
				D3DDISPLAYMODE DisplayMode;
			
				// get display information for this mode
				if( FAILED( m_pD3D->EnumAdapterModes( AdapterOrdinal,
													  AllowedAdapterFormats[AdapterFormat],
													  Mode,
													  &DisplayMode ) ) )
				{
					continue;
				}

				// if it doesn't meet our minimum requirements, we skip it
				if ( DisplayMode.Width < m_MinFullscreenWidth ||
					 DisplayMode.Height < m_MinFullscreenHeight ||
					 ColorChannelBits( DisplayMode.Format ) < m_MinColorChannelBits )
				{
					continue;
				}
			
				// otherwise we  add it to our list of available modes
				pAdapterInfo->DisplayModeList.push_back( DisplayMode );
			
				// and add this display format to our format list
				AdapterFormatList.push_back( DisplayMode.Format );
			}
		}

		// Get info for each device on this adapter based on our adapter info
		if ( FAILED( hr = EnumerateDevices( pAdapterInfo, AdapterFormatList ) ) )
		{
			SafeDelete( pAdapterInfo );
			return hr;
		}

		// If at least one device on this adapter is available and compatible
		// with the app, add the AdapterInfo to the list
		if ( pAdapterInfo->DeviceInfoList.size() != 0 )
		{
			m_AdapterInfoList.push_back( pAdapterInfo );
		}
		else
		{
			SafeDelete( pAdapterInfo );
		}
	}
	return S_OK;
}

// Enumerates available devices on this adapter 
HRESULT CDeviceEnumeration::EnumerateDevices( AdapterInfo* pAdapterInfo,						// adapter to enumerate devices for
										     std::vector<D3DFORMAT>& AdapterFormatList )	// list of compatible adapter formats
{

	HRESULT	hr;

	DeviceInfo* pDeviceInfo = new DeviceInfo;
	
	pDeviceInfo->AdapterOrdinal = pAdapterInfo->AdapterOrdinal;
	// only looking for hardware accelerated devices
	pDeviceInfo->DeviceType = D3DDEVTYPE_HAL;			

	// try to get the caps for this device
	if ( FAILED( hr = m_pD3D->GetDeviceCaps( pAdapterInfo->AdapterOrdinal,
											 pDeviceInfo->DeviceType,
											 &pDeviceInfo->Caps ) ) )
	{
		SafeDelete( pDeviceInfo );
		return hr;
	}
	
	// Get info for each devicecombo on this device
	// this finds the formats and capabilities available for this particular device
	if ( FAILED( hr = EnumerateDeviceCombos( pDeviceInfo,
											 AdapterFormatList ) ) )
	{
		SafeDelete( pDeviceInfo );
		return hr;
	}
	
	// If at least one devicecombo for this device is found, 
	// add the DeviceInfo to the list
	if ( pDeviceInfo->DeviceComboList.size() == 0 )
	{
		SafeDelete( pDeviceInfo );
		return S_OK;
	}
	pAdapterInfo->DeviceInfoList.push_back( pDeviceInfo );

	return S_OK;
}

// Determines which formats and capabilities are available for a specific device
HRESULT CDeviceEnumeration::EnumerateDeviceCombos( DeviceInfo* pDeviceInfo,							// device to find combos for
												  std::vector<D3DFORMAT>& AdapterFormatList )		// list of compatible adapter formats
{
	// backbuffer formats to check
	const D3DFORMAT	BackBufferFormatsArray[] =
	{
		D3DFMT_A8R8G8B8, 
		D3DFMT_X8R8G8B8,
		D3DFMT_R5G6B5, 
		D3DFMT_A1R5G5B5, 
		D3DFMT_X1R5G5B5
	};
	const uint32 BackBufferFormatCount = sizeof( BackBufferFormatsArray ) /
									     sizeof( BackBufferFormatsArray[0] );
	
	const bool WindowedArray[] =
	{
		false, true
	};
	const uint32 WindowedArrayCount = sizeof( WindowedArray ) /
									  sizeof( WindowedArray[0] );

	// See which adapter formats are supported by this device
	D3DFORMAT AdapterFormat;
	// go through all our compatible adapter formats
	for ( uint32 AdapterFormats = 0; AdapterFormats < AdapterFormatList.size(); ++AdapterFormats )
	{
		// get a compatible format
		AdapterFormat = AdapterFormatList[AdapterFormats];
		
		D3DFORMAT BackBufferFormat;

		// go through all the formats we can try
		for ( uint32 CurrentFormat = 0; CurrentFormat < BackBufferFormatCount; ++CurrentFormat )
		{
			// current format to try
			BackBufferFormat = BackBufferFormatsArray[CurrentFormat];
		
			// make sure this format meets our minimum alpha bit requirements
			if ( AlphaChannelBits( BackBufferFormat ) < m_MinAlphaChannelBits )
			{
				continue;
			}

			// check whether the device is available for both fullscreen and windowed
			bool IsWindowed = false;
			for ( uint32 WindowedTypes = 0; WindowedTypes < WindowedArrayCount; ++WindowedTypes )
			{
				IsWindowed = WindowedArray[WindowedTypes];

				// see if this device is available
				if ( FAILED( m_pD3D->CheckDeviceType( pDeviceInfo->AdapterOrdinal,
													  pDeviceInfo->DeviceType,
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
				DeviceCombo* pDeviceCombo = new DeviceCombo;

				// insert the info pertaining to this device
				pDeviceCombo->AdapterOrdinal	  = pDeviceInfo->AdapterOrdinal;
				pDeviceCombo->DeviceType		  = pDeviceInfo->DeviceType;
				pDeviceCombo->AdapterFormat		  = AdapterFormat;
				pDeviceCombo->BackBufferFormat	  = BackBufferFormat;
				pDeviceCombo->IsWindowed		  = IsWindowed;

				// gather stencil formats available
				BuildDepthStencilFormatList( pDeviceCombo );
				// if we have no stencil formats, we fail
				if ( pDeviceCombo->DepthStencilFormatList.size() == 0 )
				{
					SafeDelete( pDeviceCombo );
					continue;
				}

				// gather the multisample types available
				BuildMultiSampleTypeList( pDeviceCombo );
				// if we have none, we fail
				if ( pDeviceCombo->MultiSampleTypeList.size() == 0 )
				{
					SafeDelete( pDeviceCombo );
					continue;
				}

				// gather the vertex processing types available
				BuildVertexProcessingTypeList( pDeviceInfo, pDeviceCombo );
				// if we have none, we fail
				if ( pDeviceCombo->VertexProcessingTypeList.size() == 0 )
				{
					SafeDelete( pDeviceCombo );
					continue;
				}
				// gather presentation intervals available
				BuildPresentIntervalList( pDeviceInfo, pDeviceCombo );

				// and put it in the list, since we've succeeded
				pDeviceInfo->DeviceComboList.push_back( pDeviceCombo );
			}
		}
	}
	return S_OK;
}

// Gather the stencil formats available for a given device combo
void CDeviceEnumeration::BuildDepthStencilFormatList( DeviceCombo* pDeviceCombo )	// device combo to find types for
{
	const D3DFORMAT	DepthStencilFormatArray[] =
	{
		D3DFMT_D16,
		D3DFMT_D15S1,
		D3DFMT_D24X8,
		D3DFMT_D24S8,
		D3DFMT_D24X4S4,
		D3DFMT_D32,
	};
	const uint32 DepthStencilFormatArrayCount =	sizeof( DepthStencilFormatArray ) /
												sizeof( DepthStencilFormatArray[0] );

	D3DFORMAT DepthStencilFormat;

	// for each of the possible formats, we check to see if it's available on this device
	for ( uint32 CurrentFormat = 0; CurrentFormat < DepthStencilFormatArrayCount; ++CurrentFormat )
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
		if ( SUCCEEDED( m_pD3D->CheckDeviceFormat( pDeviceCombo->AdapterOrdinal,
												   pDeviceCombo->DeviceType,
												   pDeviceCombo->AdapterFormat,
												   D3DUSAGE_DEPTHSTENCIL,
												   D3DRTYPE_SURFACE,
												   DepthStencilFormat ) ) )
		{
			// and the stencil format is also available
			if ( SUCCEEDED( m_pD3D->CheckDepthStencilMatch( pDeviceCombo->AdapterOrdinal,
														    pDeviceCombo->DeviceType,
														    pDeviceCombo->AdapterFormat,
														    pDeviceCombo->BackBufferFormat,
														    DepthStencilFormat ) ) )
			{
				// we add it to our list of available stencil formats
				pDeviceCombo->DepthStencilFormatList.push_back( DepthStencilFormat );
			}
		}
	}
}

// Gathers the multisample types available for a given device combo
void CDeviceEnumeration::BuildMultiSampleTypeList( DeviceCombo* pDeviceCombo )	// device combo to find types for
{
	// possible types
	const D3DMULTISAMPLE_TYPE MultiSampleTypeArray[] =
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
	const uint32 MultiSampleTypeArrayCount = sizeof( MultiSampleTypeArray ) /
										     sizeof( MultiSampleTypeArray[0] );

	D3DMULTISAMPLE_TYPE	MultiSampleType;

	for ( uint32 CurrentSample = 0; CurrentSample < MultiSampleTypeArrayCount; ++CurrentSample )
	{
		MultiSampleType = MultiSampleTypeArray[CurrentSample];

		// if this multisample type exists on the device
		if ( SUCCEEDED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->AdapterOrdinal,
														    pDeviceCombo->DeviceType,
														    pDeviceCombo->BackBufferFormat,
														    pDeviceCombo->IsWindowed,
														    MultiSampleType,
														    NULL	// TODO: Need MS Quality parameter/list
														    ) ) )
		{
			// add it to the list of available ones
			pDeviceCombo->MultiSampleTypeList.push_back( MultiSampleType );
		}
	}
}

// Gathers the vertex processing types available for a given device and device combo
void CDeviceEnumeration::BuildVertexProcessingTypeList( DeviceInfo* pDeviceInfo,		// device to check availability on
													   DeviceCombo* pDeviceCombo )	// device combo to check availability on
{
	// if the device is capable of Hardware T&L
	if ( ( pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) != 0 )
	{
		// and can do pure hardware Vertex Processing
		// add a pure vp type to the list
		if ( ( pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE ) != 0 )
		{
			pDeviceCombo->VertexProcessingTypeList.push_back( EVertexProcessingType::PureHardware );
		}

		// we can at least do hardware
		pDeviceCombo->VertexProcessingTypeList.push_back( EVertexProcessingType::Hardware );

		// and if required, mixed vp
		if ( m_UsesMixedVertexProcessing )
		{
			pDeviceCombo->VertexProcessingTypeList.push_back( EVertexProcessingType::Mixed );
		}
	}

	// and of course, everbody can do software
	pDeviceCombo->VertexProcessingTypeList.push_back( EVertexProcessingType::Software );
}

// Gathers the presentation intervals available for a given device and device combo
void CDeviceEnumeration::BuildPresentIntervalList( DeviceInfo* pDeviceInfo,		// device to check availability on
												  DeviceCombo* pDeviceCombo )	// device combo to check availability on
{
	// available intervals
	const uint32 PresentationIntervalArray[] =
	{
		D3DPRESENT_INTERVAL_IMMEDIATE,
		D3DPRESENT_INTERVAL_DEFAULT,
		D3DPRESENT_INTERVAL_ONE,
		D3DPRESENT_INTERVAL_TWO,
		D3DPRESENT_INTERVAL_THREE,
		D3DPRESENT_INTERVAL_FOUR,
	};
	const uint32 PresentationIntervalArrayCount	= sizeof( PresentationIntervalArray ) / 
												  sizeof( PresentationIntervalArray[0] );

	uint32 PresentationInterval;
	// check all the possible presentation intervals
	for ( uint32 CurrentInterval = 0; CurrentInterval < PresentationIntervalArrayCount; ++CurrentInterval )
	{
		PresentationInterval = PresentationIntervalArray[CurrentInterval];
		// if windowed, the following three intervals are not supported
		if ( pDeviceCombo->IsWindowed )
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
			 ( pDeviceInfo->Caps.PresentationIntervals & PresentationInterval ) )
		{
			pDeviceCombo->PresentIntervalList.push_back( PresentationInterval );
		}
	}
}

}
}