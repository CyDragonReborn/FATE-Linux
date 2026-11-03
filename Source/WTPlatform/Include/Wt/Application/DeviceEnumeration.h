/*
	DeviceEnumeration.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	06/22/2005

	Based on original work by Travis Baldree
*/

#pragma once

#include "WT/System/Object.h"
#include <vector>
#include <d3d9.h>

namespace WT
{
namespace Application
{

// Describes various types of possible vertex processing
namespace EVertexProcessingType
{
	enum Type
	{
		Software,
		Mixed,
		Hardware,
		PureHardware
	};
}

// Describes formats and capabilities available for a particular device
struct DeviceCombo
{
	uint32		AdapterOrdinal;
	D3DDEVTYPE	DeviceType;
	D3DFORMAT	AdapterFormat;
	D3DFORMAT	BackBufferFormat;
	bool		IsWindowed;

	std::vector< D3DFORMAT >					DepthStencilFormatList; 
	std::vector< D3DMULTISAMPLE_TYPE >			MultiSampleTypeList; 
	std::vector< EVertexProcessingType::Type >	VertexProcessingTypeList;
	std::vector< UINT >							PresentIntervalList; 
};

// Describes one of the devices available for an adapter
// An instance usually has several combinations of formats and capabilities
struct DeviceInfo
{
	uint32		AdapterOrdinal;	
	D3DDEVTYPE	DeviceType;
	D3DCAPS9	Caps;
	std::vector<DeviceCombo*> DeviceComboList; 

	virtual ~DeviceInfo( )
	{
		for( uint32 i = 0; i < DeviceComboList.size(); ++i )
		{
			SafeDelete( DeviceComboList[i] );
		}			
	}
};

// Describes display modes available to a single adapter
struct AdapterInfo
{	
	uint32						AdapterOrdinal;
	D3DADAPTER_IDENTIFIER9		AdapterIdentifier;

	std::vector<D3DDISPLAYMODE>	DisplayModeList;
	std::vector<DeviceInfo*>	DeviceInfoList;

	virtual ~AdapterInfo()
	{
		for( uint32 i = 0; i < DeviceInfoList.size(); ++i )
		{
			SafeDelete( DeviceInfoList[i] );
		}			
	}
};

// The D3DEnumeration class finds all devices that conform to the standards it 
// is supplied with at creation time. The application can then select from 
// these devices when creating the Direct3D device
class CDeviceEnumeration : public WT::System::CObject
{
public:
					
	// Enumerate all available devices that meet our minimum requirements
	// Only HAL devices are enumerated (REF and software devices are not)
	HRESULT	Enumerate();
								  				  			
	inline std::vector<AdapterInfo*>& GetAdapterList()
	{
		return m_AdapterInfoList;	
	};

	inline uint32 GetMinFullscreenWidth() const
	{
		return m_MinFullscreenWidth;	
	}
	inline uint32 GetMinFullscreenHeight() const
	{
		return m_MinFullscreenHeight;	
	}
	inline uint32 GetMinColorChannelBits() const
	{
		return m_MinColorChannelBits;	
	}
	inline uint32 GetMinAlphaChannelBits() const
	{
		return m_MinAlphaChannelBits;	
	}
	inline uint32 GetMinStencilBits() const
	{
		return m_MinStencilBits;		
	}
	inline uint32 GetMinDepthBits() const
	{
		return m_MinDepthBits;			
	}

	inline void SetMinFullscreenWidth( uint32 MinFullscreenWidth )
	{	
		m_MinFullscreenWidth = MinFullscreenWidth;
	}
	inline void SetMinFullscreenHeight( uint32 MinFullscreenHeight )				
	{
		m_MinFullscreenHeight = MinFullscreenHeight;	
	}
	inline void SetMinColorChannelBits( uint32 MinColorChannelBits )
	{	
		m_MinColorChannelBits = MinColorChannelBits;
	}
	inline void SetMinAlphaChannelBits( uint32 MinAlphaChannelBits )				
	{	
		m_MinAlphaChannelBits = MinAlphaChannelBits;	
	}
	inline void SetMinStencilBits( uint32 MinStencilBits )					
	{	
		m_MinStencilBits = MinStencilBits;		
	}
	inline void SetMinDepthBits( uint32 MinDepthBits )						
	{	
		m_MinDepthBits = MinDepthBits;			
	}

	CDeviceEnumeration( IDirect3D9* pD3D,					  // Direct3D object
					   uint32 MinimumScreenWidth = 320,		  // minimum screen width (default 320)
					   uint32 MinimumScreenHeight = 240,	  // minimum screen height (default 240)
					   uint32 MinimumColorBits = 5,			  // minimum color bits (default 5)
					   uint32 MinimumAlphaBits = 0,			  // minimum alpha bits (default 0)
					   uint32 MinimumDepthBufferBits = 16,	  // minimum depth(z) buffer bits (default 16)
					   uint32 MinimumStencilBufferBits = 4 ); // minimum stencil buffer bits (default 4)

	virtual ~CDeviceEnumeration();

private:	// methods

	// Enumerates available devices on this adapter 
	HRESULT EnumerateDevices( AdapterInfo* pAdapterInfo, std::vector<D3DFORMAT>& AdapterFormatList );

	// Determines which formats and capabilities are available for a specific device
	HRESULT EnumerateDeviceCombos( DeviceInfo* pDeviceInfo, std::vector<D3DFORMAT>& AdapterFormatList );

	// Gather the stencil formats available for a given device combo
	void BuildDepthStencilFormatList( DeviceCombo* pDeviceCombo );

	// Gathers the multisample types available for a given device combo
	void BuildMultiSampleTypeList( DeviceCombo* pDeviceCombo );

	// Gathers the vertex processing types available for a given device and device combo
	void BuildVertexProcessingTypeList( DeviceInfo* pDeviceInfo, DeviceCombo* pDeviceCombo );

	// Gathers the presentation intervals available for a given device and device combo
	void BuildPresentIntervalList( DeviceInfo* pDeviceInfo, DeviceCombo* pDeviceCombo );

private:	// data

	IDirect3D9*					m_pD3D;

	std::vector<AdapterInfo*>	m_AdapterInfoList;	// information about available adapters

	uint32	m_MinFullscreenWidth;
	uint32	m_MinFullscreenHeight;
	uint32	m_MinColorChannelBits;					// min color bits per channel in adapter format
	uint32	m_MinAlphaChannelBits;					// min alpha bits per pixel in back buffer format
	uint32	m_MinDepthBits;
	uint32	m_MinStencilBits;

	bool m_UsesMixedVertexProcessing;			// whether app can take advantage of mixed vp mode
};


}
}