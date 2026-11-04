/*
	capabilities.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#ifndef _CAPABILITIES_H
#define _CAPABILITIES_H

// versions of the windows OS
enum EOSVersions
{
	KWindows95,
	KWindows98,
	KWindowsME,
	KWindowsNT35,
	KWindowsNT40,
	KWindows2000,
	KWindowsXP,
	KWindows2004Server,
	KOSUnknown,
	KOSVersionsCount
		
};

// device capabilites with a boolean value
enum	ECapabilitiesAvailable
{
	KCanRenderToTexture,
	KCanDoStencilShadows,
	KCanDoCubeMap,
	KCanDoTextureProjection,
	KCanDoShadowTextureProjection,
	KCanDoOrthoShadowTextureProjection,
	KCanFadeAlpha,
	KCanDoPointSprites,
	KCanDoModAlphaAdd,
	KCanDoAdditive,
	KCanDoAdditiveSmooth,
	KCanDoAdditiveSigned2X,
	KCanDoModulate2X,
	KCanDoModulate4X,
	KCanDoDot3,
	KCanAlphaTest,
	KCanDoFogTable,
	KCanDoClipPlanes,
	KCanDoReflectionSpecularMapping,
	KCanDoSpecularMapping,
	KCanDoDXT1,
	KCanDoDXT2,
	KCanDoDXT3,
	KCapabilitiesAvailableCount
};

// device capabilities/settings with an integer value
enum	ECapabilitiesValues
{
	KValueMipFilter,
	KValueClampUVs,
	KValueMaxLights,
	KValueMaxSimultaneousTextures,
	KValueMaxSimultaneousBlendStages,
	KValueMaxRenderToTextureSize,
	KCapabilitiesValueCount
};


// the capabilities class is used to find out information about your PC,
// including speed, memory, and the capabilities of your video card

class CCapabilities
{
public:
	
	
		CCapabilities( void );
		~CCapabilities( void );

		void DetectD3DCapabilities( LPDIRECT3D8	pD3D,				// d3d object
									LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
									D3DPRESENT_PARAMETERS* pD3Dpp );// d3d presentation parameters


////////////////////////////////////////////////////////////////////////////////////////////
//ACCESSORS
////////////////////////////////////////////////////////////////////////////////////////////

		int32		GetProcessorSpeed( void );

		uint32		GetAvailableRAM( void );

		uint32		GetAvailableVRAM( void );

		std::string GetAdapterName( void );

		bool		IsIntelCard( void )				{	return m_IsIntelCard;		};

		bool		IsIntegratedCard( void )		{	return m_IsIntegratedCard;	};

		int32		GetCapabilityValue( ECapabilitiesValues Capability );	// which capability to check from ECapabilitiesValues

		bool		IsCapable( ECapabilitiesAvailable Capability );			// which capability to check from ECapabilitiesAvailable


		EOSVersions	GetWindowsVersion( void );

////////////////////////////////////////////////////////////////////////////////////////////
//MUTATORS
////////////////////////////////////////////////////////////////////////////////////////////

		void		SetCapable( ECapabilitiesAvailable Capability,	// which capability to set
								bool Value );						// value to set 
		void		SetCapabilityValue( ECapabilitiesValues Capability,		// which capability to set
										int32 Value );						// value to set 

private:

		void		ResetCapabilityTables();

		std::string	m_AdapterName;

		uint32		m_VRAM;

		int32		m_CapabilityValue[KCapabilitiesValueCount];

		bool		m_Capable[KCapabilitiesAvailableCount];

		bool		m_DetectionComplete;

		bool		m_IsIntelCard;

		bool		m_IsIntegratedCard;
};

#endif