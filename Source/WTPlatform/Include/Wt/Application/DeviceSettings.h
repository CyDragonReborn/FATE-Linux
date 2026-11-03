/*
	DeviceSettings.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	06/22/2005

	Based on original work by Travis Baldree
*/

#pragma once

#include "WT/System/Platform.h"
#include "WT/Application/DeviceEnumeration.h"
#include <d3d9.h>

namespace WT
{
namespace Application
{

class CDeviceSettings
{
public:	// attributes

	bool					IsWindowed;

	AdapterInfo*			Windowed_AdapterInfo;
	DeviceInfo*				Windowed_DeviceInfo;
	DeviceCombo*			Windowed_DeviceCombo;

	D3DDISPLAYMODE			Windowed_DisplayMode;				// not changeable by the user
	D3DFORMAT				Windowed_DepthStencilBufferFormat;
	D3DMULTISAMPLE_TYPE		Windowed_MultisampleType;
	uint32					Windowed_MultisampleQuality;
	EVertexProcessingType::Type	Windowed_VertexProcessingType;
	uint32					Windowed_PresentInterval;
	int32					Windowed_Width;
	int32					Windowed_Height;

	AdapterInfo*			Fullscreen_AdapterInfo;
	DeviceInfo*				Fullscreen_DeviceInfo;
	DeviceCombo*			Fullscreen_DeviceCombo;

	D3DDISPLAYMODE			Fullscreen_DisplayMode; 
	D3DFORMAT				Fullscreen_DepthStencilBufferFormat;
	D3DMULTISAMPLE_TYPE		Fullscreen_MultisampleType;
	EVertexProcessingType::Type	Fullscreen_VertexProcessingType;
	uint32					Fullscreen_PresentInterval;

	AdapterInfo* GetAdapterInfo() const
	{
		return IsWindowed ? Windowed_AdapterInfo : Fullscreen_AdapterInfo;
	}

	DeviceInfo* GetDeviceInfo() const
	{
		return IsWindowed ? Windowed_DeviceInfo : Fullscreen_DeviceInfo;
	}

	DeviceCombo* GetDeviceCombo() const
	{
		return IsWindowed ? Windowed_DeviceCombo : Fullscreen_DeviceCombo;
	}

	uint32 GetAdapterOrdinal() const
	{
		return GetDeviceCombo()->AdapterOrdinal;
	}

	D3DDEVTYPE GetDeviceType() const
	{
		return GetDeviceCombo()->DeviceType;
	}

	D3DFORMAT GetBackBufferFormat() const
	{
		return GetDeviceCombo()->BackBufferFormat;
	}

	D3DDISPLAYMODE GetDisplayMode() const
	{
		return IsWindowed ? Windowed_DisplayMode : Fullscreen_DisplayMode;
	}

	void SetDisplayMode( D3DDISPLAYMODE DisplayMode )
	{
		if ( IsWindowed )
		{
			Windowed_DisplayMode = DisplayMode;
		}
		else
		{
			Fullscreen_DisplayMode = DisplayMode;
		}
	}

	D3DFORMAT GetDepthStencilBufferFormat() const
	{
		return IsWindowed ? Windowed_DepthStencilBufferFormat : Fullscreen_DepthStencilBufferFormat;
	}

	void SetDepthStencilBufferFormat( D3DFORMAT DepthStencilBufferFormat )
	{
		if ( IsWindowed )
		{
			Windowed_DepthStencilBufferFormat = DepthStencilBufferFormat;
		}
		else
		{
			Fullscreen_DepthStencilBufferFormat = DepthStencilBufferFormat;
		}
	}

	D3DMULTISAMPLE_TYPE GetMultisampleType() const
	{
		return IsWindowed ? Windowed_MultisampleType : Fullscreen_MultisampleType;
	}

	uint32 GetPresentInterval() const
	{
		return IsWindowed ? Windowed_PresentInterval : Fullscreen_PresentInterval;
	}

	void SetMultisampleType( D3DMULTISAMPLE_TYPE MultisampleType )
	{
		if ( IsWindowed )
		{
			Windowed_MultisampleType = MultisampleType;
		}
		else
		{
			Fullscreen_MultisampleType = MultisampleType;
		}
	}

	EVertexProcessingType::Type GetVertexProcessingType() const
	{
		return IsWindowed ? Windowed_VertexProcessingType : Fullscreen_VertexProcessingType;
	}
};

}
}


