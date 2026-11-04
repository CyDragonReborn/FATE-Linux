/*
	d3denumeration.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _D3DENUMERATION_H
#define _D3DENUMERATION_H


#include <D3DX8.h>
#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"

// types of vertex processing possible
enum EVertexProcessingType
{
	KVPSoftware,
	KVPMixed,
	KVPHardware,
	KVPPureHardware
};

// formats and capabilities available for a particular device
class D3DDeviceCombo
{
public:

	uint32		m_AdapterOrdinal;		// which adapter is this?
	D3DDEVTYPE	m_DeviceType;
	D3DFORMAT	m_AdapterFormat;
	D3DFORMAT	m_BackBufferFormat;
	bool		m_IsWindowed;

	std::vector< D3DFORMAT >				m_pDepthStencilFormatList; 
	std::vector< D3DMULTISAMPLE_TYPE >		m_pMultiSampleTypeList; 
	std::vector< EVertexProcessingType >	m_pVertexProcessingTypeList;
	std::vector< UINT >						m_pPresentIntervalList; 
};

// one of the devices available for an adapter -
// usually hsa several 'combos' (combinations of formats and capabilities)
class D3DDeviceInfo
{
public:
	~D3DDeviceInfo( void )
	{
		for( uint32 i = 0; i < m_pDeviceComboList.size(); i ++ )
		{
			DELETE_SAFELY( m_pDeviceComboList[i] );
		}
			
	}

	uint32							m_AdapterOrdinal;	// which adapter is this for?
	D3DDEVTYPE						m_DeviceType;
	D3DCAPS8						m_Caps;
	std::vector< D3DDeviceCombo* >	m_pDeviceComboList; 
};

// adapters contain devices
class D3DAdapterInfo
{
public:
	~D3DAdapterInfo( void )
	{
		for( uint32 i = 0; i < m_pDeviceInfoList.size(); i ++ )
		{
			DELETE_SAFELY( m_pDeviceInfoList[i] );
		}
			
	}

	uint32							m_AdapterOrdinal;		// which adapter is this?
	D3DADAPTER_IDENTIFIER8			m_AdapterIdentifier;

	std::vector< D3DDISPLAYMODE >	m_pDisplayModeList; // List of D3DDISPLAYMODEs
	std::vector< D3DDeviceInfo* >	m_pDeviceInfoList; // List of D3DDISPLAYMODEs
};

// the D3DEnumeration class finds all devices that conform to the standards it
// is supplied with at creation time.  The application can then select from these
// devices when creating the d3ddevice
class CD3DEnumeration
{
public:

					CD3DEnumeration( IDirect3D8* pD3D,						// direct3d object
									 uint32 MinimumScreenWidth = 320,		// minimum screen width (default 320)
									 uint32 MinimumScreenHeight = 240,		// minimum screen height (default 240)
									 uint32 MinimumColorBits = 5,			// minimum color bits (default 5)
									 uint32 MinimumAlphaBits = 0,			// minimum alpha bits (default 0)
						  			 uint32 MinimumDepthBufferBits = 16,	// minimum depth(z) buffer bits (default 16)
						  			 uint32 MinimumStencilBufferBits = 4);	// minimum stencil buffer bits (default 4)
					~CD3DEnumeration( void );
						
	HRESULT			Enumerate( void );
								  				  			
	std::vector< D3DAdapterInfo* >&		GetAdapterList( void )			{	return m_pAdapterInfoList;	};

	uint32			MinFullscreenWidth( void )							{	return m_MinFullscreenWidth;	};
	uint32			MinFullscreenHeight( void )							{	return m_MinFullscreenHeight;	};
	uint32			MinColorChannelBits( void )							{	return m_MinColorChannelBits;	};
	uint32			MinAlphaChannelBits( void )							{	return m_MinAlphaChannelBits;	};
	uint32			MinStencilBits( void )								{	return m_MinStencilBits;		};
	uint32			MinDepthBits( void )								{	return m_MinDepthBits;			};

	void			SetMinFullscreenWidth( uint32 Value )				{	m_MinFullscreenWidth = Value;	};
	void			SetMinFullscreenHeight( uint32 Value )				{	m_MinFullscreenHeight = Value;	};
	void			SetMinColorChannelBits( uint32 Value )				{	m_MinColorChannelBits = Value;	};
	void			SetMinAlphaChannelBits( uint32 Value )				{	m_MinAlphaChannelBits = Value;	};
	void			SetMinStencilBits( uint32 Value )					{	m_MinStencilBits = Value;		};
	void			SetMinDepthBits( uint32 Value )						{	m_MinDepthBits = Value;			};

private:
	IDirect3D8*				m_pD3D;

	std::vector< D3DAdapterInfo* >		m_pAdapterInfoList;		// information about available adapters

	uint32					m_MinFullscreenWidth;
	uint32					m_MinFullscreenHeight;
	uint32					m_MinColorChannelBits;				// min color bits per channel in adapter format
	uint32					m_MinAlphaChannelBits;				// min alpha bits per pixel in back buffer format
	uint32					m_MinDepthBits;
	uint32					m_MinStencilBits;

	bool					m_UsesMixedVertexProcessing;		// whether app can take advantage of mixed vp mode



	HRESULT					EnumerateDevices( D3DAdapterInfo* pAdapterInfo,						// adapter to enumerate devices for
											  std::vector < D3DFORMAT >& pAdapterFormatList );	// list of compatible adapter formats

	HRESULT					EnumerateDeviceCombos( D3DDeviceInfo* pDeviceInfo,							// device to find combos for
												   std::vector < D3DFORMAT >& pAdapterFormatList );		// list of compatible adapter formats

	void					BuildDepthStencilFormatList( D3DDeviceCombo* pDeviceCombo );	// device combo to find types for

	void					BuildMultiSampleTypeList( D3DDeviceCombo* pDeviceCombo );		// device combo to find types for

	void					BuildVertexProcessingTypeList( D3DDeviceInfo* pDeviceInfo,		// device to check availability on
														   D3DDeviceCombo* pDeviceCombo );	// device combo to check availability on

	void					BuildPresentIntervalList( D3DDeviceInfo* pDeviceInfo,			// device to check availability on
													  D3DDeviceCombo* pDeviceCombo );		// device combo to check availability on


};



#endif
