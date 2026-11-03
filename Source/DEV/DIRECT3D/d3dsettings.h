/*
	d3dsettings.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _D3DSETTINGS_H
#define _D3DSETTINGS_H

#include "../UTILITIES/constants.h"

// based originally on the D3D SDK framework, the d3dsettings class really just maintains
// data about the currently selected device, for use in day-to-day d3d tasks.
// always maintains data for both a windowed and fullscreen device, so that we
// can switch between them
class CD3DSettings
{
public:
	bool					m_IsWindowed;

	D3DAdapterInfo*			m_pWindowed_AdapterInfo;
	D3DDeviceInfo*			m_pWindowed_DeviceInfo;
	D3DDeviceCombo*			m_pWindowed_DeviceCombo;

	D3DDISPLAYMODE			m_Windowed_DisplayMode; // not changable by the user
	D3DFORMAT				m_Windowed_DepthStencilBufferFormat;
	D3DMULTISAMPLE_TYPE		m_Windowed_MultisampleType;
	DWORD					m_Windowed_MultisampleQuality;
	EVertexProcessingType	m_Windowed_VertexProcessingType;
	uint32					m_Windowed_PresentInterval;
	int32					m_Windowed_Width;
	int32					m_Windowed_Height;

	D3DAdapterInfo*			m_pFullscreen_AdapterInfo;
	D3DDeviceInfo*			m_pFullscreen_DeviceInfo;
	D3DDeviceCombo*			m_pFullscreen_DeviceCombo;

	D3DDISPLAYMODE			m_Fullscreen_DisplayMode; 
	D3DFORMAT				m_Fullscreen_DepthStencilBufferFormat;
	D3DMULTISAMPLE_TYPE		m_Fullscreen_MultisampleType;
	EVertexProcessingType	m_Fullscreen_VertexProcessingType;
	uint32					m_Fullscreen_PresentInterval;

	CD3DSettings() :
					 m_IsWindowed( kFalse ),
					 m_pWindowed_AdapterInfo( NULL ),
					 m_pWindowed_DeviceInfo( NULL ),
					 m_pWindowed_DeviceCombo( NULL ),
					 m_pFullscreen_AdapterInfo( NULL ),
					 m_pFullscreen_DeviceInfo( NULL ),
					 m_pFullscreen_DeviceCombo( NULL ),
					 m_Windowed_MultisampleQuality(0),
					 m_Windowed_VertexProcessingType(KVPPureHardware),
					 m_Fullscreen_VertexProcessingType(KVPPureHardware),
					 m_Windowed_PresentInterval(0),
					 m_Windowed_Width(0),
					 m_Windowed_Height(0)
	{
		ZeroMemory (&m_Windowed_DisplayMode,				sizeof( m_Windowed_DisplayMode ));
		ZeroMemory (&m_Windowed_DepthStencilBufferFormat,	sizeof( m_Windowed_DepthStencilBufferFormat ));
		ZeroMemory (&m_Windowed_MultisampleType,			sizeof( m_Windowed_MultisampleType ));
		ZeroMemory (&m_Fullscreen_DisplayMode,				sizeof( m_Fullscreen_DisplayMode ));
		ZeroMemory (&m_Fullscreen_DepthStencilBufferFormat,	sizeof( m_Fullscreen_DepthStencilBufferFormat ));
		ZeroMemory (&m_Fullscreen_MultisampleType,			sizeof( m_Fullscreen_MultisampleType ));
	}

	D3DAdapterInfo* AdapterInfo()
	{
		return m_IsWindowed ? m_pWindowed_AdapterInfo : m_pFullscreen_AdapterInfo;
	} // AdapterInfo()

	D3DDeviceInfo* DeviceInfo()
	{
		return m_IsWindowed ? m_pWindowed_DeviceInfo : m_pFullscreen_DeviceInfo;
	} // DeviceInfo()

	D3DDeviceCombo* DeviceCombo()
	{
		return m_IsWindowed ? m_pWindowed_DeviceCombo : m_pFullscreen_DeviceCombo;
	} // DeviceCombo()

	int32 AdapterOrdinal()
	{
		return DeviceCombo()->m_AdapterOrdinal;
	} // AdapterOrdinal()

	D3DDEVTYPE DeviceType()
	{
		return DeviceCombo()->m_DeviceType;
	} // DeviceType()

	D3DFORMAT BackBufferFormat()
	{
		return DeviceCombo()->m_BackBufferFormat;
	} // BackBufferFormat()

	D3DDISPLAYMODE DisplayMode()
	{
		return m_IsWindowed ? m_Windowed_DisplayMode : m_Fullscreen_DisplayMode;
	} // DisplayMode()

	void SetDisplayMode( D3DDISPLAYMODE Value )
	{
		if ( m_IsWindowed )
		{
			m_Windowed_DisplayMode = Value;
		}
		else
		{
			m_Fullscreen_DisplayMode = Value;
		}
	} // SetDisplayMode()

	D3DFORMAT DepthStencilBufferFormat()
	{
		return m_IsWindowed ?
			   m_Windowed_DepthStencilBufferFormat :
			   m_Fullscreen_DepthStencilBufferFormat;
	} // DepthStencilBufferFormat()

	void SetDepthStencilBufferFormat( D3DFORMAT Value )
	{
		if ( m_IsWindowed )
		{
			m_Windowed_DepthStencilBufferFormat = Value;
		}
		else
		{
			m_Fullscreen_DepthStencilBufferFormat = Value;
		}
	} // SetDepthStencilBufferFormat()

	D3DMULTISAMPLE_TYPE MultisampleType()
	{
		return m_IsWindowed ? m_Windowed_MultisampleType : m_Fullscreen_MultisampleType;
	} // MultisampleType()

	uint32 PresentInterval()
	{
		return m_IsWindowed ? m_Windowed_PresentInterval : m_Fullscreen_PresentInterval;
	} // PresentInterval()

	void SetMultisampleType( D3DMULTISAMPLE_TYPE Value )
	{
		if ( m_IsWindowed )
		{
			m_Windowed_MultisampleType = Value;
		}
		else
		{
			m_Fullscreen_MultisampleType = Value;
		}
	} // SetMultisampleType()

	EVertexProcessingType GetVertexProcessingType()
	{
		return m_IsWindowed ?
			   m_Windowed_VertexProcessingType :
			   m_Fullscreen_VertexProcessingType;
	} // GetVertexProcessingType()
};



#endif



