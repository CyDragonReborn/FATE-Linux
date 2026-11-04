//
//	DeviceEnumeration2.h
//
//	Copyright (C) 2005-6 WildTangent, Inc. 
//	All Rights Reserved
//
// Overview:
//	This class provides services for enumarting direct3d resource and devices.
//
#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Application2
{

class CApplication;

//--------------------------------------------------------------------------------------
// Finding valid device settings
//--------------------------------------------------------------------------------------
namespace EMatchType
{
enum MatchType
{
	IgnoreInput = 0,	// Use the closest valid value to a default 
	PreserveInput,		// Use input without change, but may cause no valid device to be found
	ClosestToInput		// Use the closest valid value to the input 
};
}

struct MatchOptions
{
	EMatchType::MatchType AdapterOrdinal;
    EMatchType::MatchType DeviceType;
    EMatchType::MatchType Windowed;
    EMatchType::MatchType AdapterFormat;
    EMatchType::MatchType VertexProcessing;
    EMatchType::MatchType Resolution;
    EMatchType::MatchType BackBufferFormat;
    EMatchType::MatchType BackBufferCount;
    EMatchType::MatchType MultiSample;
    EMatchType::MatchType SwapEffect;
    EMatchType::MatchType DepthFormat;
    EMatchType::MatchType StencilFormat;
    EMatchType::MatchType PresentFlags;
    EMatchType::MatchType RefreshRate;
    EMatchType::MatchType PresentInterval;
};

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
class CDeviceEnumAdapterInfo;
class CDeviceEnumDeviceInfo;
struct CDeviceEnumDeviceSettingsCombo;
struct CDeviceEnumDSMSConflict;


struct DeviceSettings
{
    UINT AdapterOrdinal;
    D3DDEVTYPE DeviceType;
    D3DFORMAT AdapterFormat;
    DWORD BehaviorFlags;
    D3DPRESENT_PARAMETERS pp;
};

//--------------------------------------------------------------------------------------
// Enumerates available Direct3D adapters, devices, modes, etc.
// Use GetEnumeration() to access global instance
//--------------------------------------------------------------------------------------
class CDeviceEnumeration
{
public:
    // These should be called before Enumerate(). 
    //
    // Use these calls and the IsDeviceAcceptable to control the contents of 
    // the enumeration object, which affects the device selection and the device settings dialog.
    void SetRequirePostPixelShaderBlending( bool bRequire ) { m_bRequirePostPixelShaderBlending = bRequire; }
    void SetResolutionMinMax( uint32 nMinWidth, uint32 nMinHeight, uint32 nMaxWidth, uint32 nMaxHeight );  
    void SetRefreshMinMax( uint32 nMin, uint32 nMax );
    void SetMultisampleQualityMax( uint32 nMax );    
    void GetPossibleVertexProcessingList( bool* pbSoftwareVP, bool* pbHardwareVP, bool* pbPureHarewareVP, bool* pbMixedVP );
    void SetPossibleVertexProcessingList( bool bSoftwareVP, bool bHardwareVP, bool bPureHarewareVP, bool bMixedVP );
	std::vector<D3DFORMAT>* GetPossibleDepthStencilFormatList();   
	std::vector<D3DMULTISAMPLE_TYPE>* GetPossibleMultisampleTypeList();   
	std::vector<uint32>* GetPossiblePresentIntervalList();
    void ResetPossibleDepthStencilFormats();
    void ResetPossibleMultisampleTypeList();
    void ResetPossiblePresentIntervalList();

    // Call Enumerate() to enumerate available D3D adapters, devices, modes, etc.
    HRESULT Enumerate( IDirect3D9* pD3D, CApplication* pApplication );

	// These should be called after Enumerate() is called
	std::vector<CDeviceEnumAdapterInfo*>*		GetAdapterInfoList();  
    CDeviceEnumAdapterInfo*                    GetAdapterInfo( uint32 AdapterOrdinal );  
    CDeviceEnumDeviceInfo*                     GetDeviceInfo( uint32 AdapterOrdinal, D3DDEVTYPE DeviceType );    
    CDeviceEnumDeviceSettingsCombo*            GetDeviceSettingsCombo( DeviceSettings* pDeviceSettings ) { return GetDeviceSettingsCombo( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, pDeviceSettings->AdapterFormat, pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed ); }
    CDeviceEnumDeviceSettingsCombo*            GetDeviceSettingsCombo( uint32 AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL Windowed );  

    ~CDeviceEnumeration();

	//get access to the singleton instance of the enumeration object
	static CDeviceEnumeration* GetEnumeration(); 

private:
    // Use GetEnumeration() to access global instance
    CDeviceEnumeration();

    IDirect3D9* m_pD3D;                 
	CApplication* m_pApplication;
    bool m_bRequirePostPixelShaderBlending;
	std::vector<D3DFORMAT> m_DepthStecilPossibleList;
    std::vector<D3DMULTISAMPLE_TYPE> m_MultiSampleTypeList;
	std::vector<uint32> m_PresentIntervalList;

    bool m_bSoftwareVP;
    bool m_bHardwareVP;
    bool m_bPureHarewareVP;
    bool m_bMixedVP;

    uint32 m_nMinWidth;
    uint32 m_nMaxWidth;
    uint32 m_nMinHeight;
    uint32 m_nMaxHeight;
    uint32 m_nRefreshMin;
    uint32 m_nRefreshMax;
    uint32 m_nMultisampleQualityMax;

    // Array of CDeviceEnumAdapterInfo* with unique AdapterOrdinals
	std::vector<CDeviceEnumAdapterInfo*> m_AdapterInfoList;  

	HRESULT EnumerateDevices( CDeviceEnumAdapterInfo* pAdapterInfo, std::vector<D3DFORMAT>* pAdapterFormatList );
	HRESULT EnumerateDeviceCombos( CDeviceEnumAdapterInfo* pAdapterInfo, CDeviceEnumDeviceInfo* pDeviceInfo, std::vector<D3DFORMAT>* pAdapterFormatList );
    void BuildDepthStencilFormatList( CDeviceEnumDeviceSettingsCombo* pDeviceCombo );
    void BuildMultiSampleTypeList( CDeviceEnumDeviceSettingsCombo* pDeviceCombo );
    void BuildDSMSConflictList( CDeviceEnumDeviceSettingsCombo* pDeviceCombo );
    void BuildPresentIntervalList( CDeviceEnumDeviceInfo* pDeviceInfo, CDeviceEnumDeviceSettingsCombo* pDeviceCombo );
    void ClearAdapterInfoList();
};

//--------------------------------------------------------------------------------------
// A class describing an adapter which contains a unique adapter ordinal 
// that is installed on the system
//--------------------------------------------------------------------------------------
class CDeviceEnumAdapterInfo
{
public:
    ~CDeviceEnumAdapterInfo();

    uint32 AdapterOrdinal;
    D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
    char szUniqueDescription[256];

	std::vector<D3DDISPLAYMODE> displayModeList; // Array of supported D3DDISPLAYMODEs
	std::vector<CDeviceEnumDeviceInfo*> deviceInfoList; // Array of CDeviceEnumDeviceInfo* with unique supported DeviceTypes
};


//--------------------------------------------------------------------------------------
// A class describing a Direct3D device that contains a 
//       unique supported device type 
//--------------------------------------------------------------------------------------
class CDeviceEnumDeviceInfo
{
public:
    ~CDeviceEnumDeviceInfo();

    uint32 AdapterOrdinal;
    D3DDEVTYPE DeviceType;
    D3DCAPS9 Caps;

    // List of CDeviceEnumDeviceSettingsCombo* with a unique set 
    // of AdapterFormat, BackBufferFormat, and Windowed
	std::vector<CDeviceEnumDeviceSettingsCombo*> deviceSettingsComboList; 
};


//--------------------------------------------------------------------------------------
// A struct describing device settings that contains a unique combination of 
// adapter format, back buffer format, and windowed that is compatible with a 
// particular Direct3D device and the app.
//--------------------------------------------------------------------------------------
struct CDeviceEnumDeviceSettingsCombo
{
    uint32 AdapterOrdinal;
    D3DDEVTYPE DeviceType;
    D3DFORMAT AdapterFormat;
    D3DFORMAT BackBufferFormat;
    BOOL Windowed;

	std::vector<D3DFORMAT> depthStencilFormatList;			// List of D3DFORMATs
    std::vector<D3DMULTISAMPLE_TYPE> multiSampleTypeList;	// List of D3DMULTISAMPLE_TYPEs
	std::vector<DWORD> multiSampleQualityList;				// List of number of quality levels for each multisample type
    std::vector<UINT> presentIntervalList;					// List of D3DPRESENT flags
	std::vector<CDeviceEnumDSMSConflict> DSMSConflictList;	// List of CDeviceEnumDSMSConflict

    CDeviceEnumAdapterInfo* pAdapterInfo;
    CDeviceEnumDeviceInfo* pDeviceInfo;
};


//--------------------------------------------------------------------------------------
// A depth/stencil buffer format that is incompatible with a
// multisample type.
//--------------------------------------------------------------------------------------
struct CDeviceEnumDSMSConflict
{
    D3DFORMAT DSFormat;
    D3DMULTISAMPLE_TYPE MSType;
};


}
}