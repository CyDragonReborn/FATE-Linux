//
//	Application2.h
//
//	Copyright (C) 2005-6 WildTangent, Inc. 
//	All Rights Reserved
//
#include "WT/stdafx.h"
#include "WT/Application/DeviceEnumeration2.h"
#include "WT/Application/Application2.h"
#include <strsafe.h>

namespace WT
{
namespace Application2
{

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
static bool SortModesCallback(D3DDISPLAYMODE pdm1, D3DDISPLAYMODE pdm2 );

CDeviceEnumeration* CDeviceEnumeration::GetEnumeration()
{
    // Using an accessor function gives control of the construction order
    static CDeviceEnumeration d3denum;
    return &d3denum;
}


//--------------------------------------------------------------------------------------
CDeviceEnumeration::CDeviceEnumeration()
{
    m_pD3D = NULL;
    m_bRequirePostPixelShaderBlending = true;

    m_nMinWidth = 640;
    m_nMinHeight = 480;
    m_nMaxWidth = UINT_MAX;
    m_nMaxHeight = UINT_MAX;

    m_nRefreshMin = 0;
    m_nRefreshMax = UINT_MAX;

    m_nMultisampleQualityMax = 0xFFFF;

    ResetPossibleDepthStencilFormats();
    ResetPossibleMultisampleTypeList();                                   
    ResetPossiblePresentIntervalList();
    SetPossibleVertexProcessingList( true, true, true, false );
}


//--------------------------------------------------------------------------------------
CDeviceEnumeration::~CDeviceEnumeration()
{
    ClearAdapterInfoList();
}



//--------------------------------------------------------------------------------------
// Enumerates available D3D adapters, devices, modes, etc.
//--------------------------------------------------------------------------------------
HRESULT CDeviceEnumeration::Enumerate( IDirect3D9* pD3D, CApplication* pApplication )
{
    m_pD3D = pD3D;
	m_pApplication = pApplication;

    ClearAdapterInfoList();
	std::vector<D3DFORMAT> adapterFormatList;

    const D3DFORMAT allowedAdapterFormatArray[] = 
    {   
        D3DFMT_X8R8G8B8, 
        D3DFMT_X1R5G5B5, 
        D3DFMT_R5G6B5, 
        D3DFMT_A2R10G10B10
    };
    const uint32 allowedAdapterFormatArrayCount  = sizeof(allowedAdapterFormatArray) / sizeof(allowedAdapterFormatArray[0]);

    uint32 numAdapters = pD3D->GetAdapterCount();
    for (uint32 adapterOrdinal = 0; adapterOrdinal < numAdapters; adapterOrdinal++)
    {
        CDeviceEnumAdapterInfo* pAdapterInfo = new CDeviceEnumAdapterInfo;
        if( pAdapterInfo == NULL )
            return E_OUTOFMEMORY;

        pAdapterInfo->AdapterOrdinal = adapterOrdinal;
        pD3D->GetAdapterIdentifier(adapterOrdinal, 0, &pAdapterInfo->AdapterIdentifier);

        // Get list of all display modes on this adapter.  
        // Also build a temporary list of all display adapter formats.
        adapterFormatList.clear();

        for( uint32 iFormatList = 0; iFormatList < allowedAdapterFormatArrayCount; iFormatList++ )
        {
            D3DFORMAT allowedAdapterFormat = allowedAdapterFormatArray[iFormatList];
            uint32 numAdapterModes = pD3D->GetAdapterModeCount( adapterOrdinal, allowedAdapterFormat );
            for (uint32 mode = 0; mode < numAdapterModes; mode++)
            {
                D3DDISPLAYMODE displayMode;
                pD3D->EnumAdapterModes( adapterOrdinal, allowedAdapterFormat, mode, &displayMode );

                if( displayMode.Width < m_nMinWidth ||
                    displayMode.Height < m_nMinHeight || 
                    displayMode.Width > m_nMaxWidth ||
                    displayMode.Height > m_nMaxHeight || 
                    displayMode.RefreshRate < m_nRefreshMin ||
                    displayMode.RefreshRate > m_nRefreshMax )
                {
                    continue;
                }

                pAdapterInfo->displayModeList.push_back( displayMode );
                
				if( !ContainerContains(adapterFormatList,displayMode.Format ))
                    adapterFormatList.push_back( displayMode.Format );
            }

        }

        D3DDISPLAYMODE displayMode;
        pD3D->GetAdapterDisplayMode( adapterOrdinal, &displayMode );
		if( !ContainerContains(adapterFormatList,displayMode.Format) )
            adapterFormatList.push_back( displayMode.Format );

        // Sort displaymode list
		std::sort ( pAdapterInfo->displayModeList.begin(), pAdapterInfo->displayModeList.end() , SortModesCallback );

        // Get info for each device on this adapter
        if( FAILED( EnumerateDevices( pAdapterInfo, &adapterFormatList ) ) )
        {
            delete pAdapterInfo;
            continue;
        }

        // If at least one device on this adapter is available and compatible
        // with the app, add the adapterInfo to the list
        if( pAdapterInfo->deviceInfoList.size() > 0 )
        {
            m_AdapterInfoList.push_back( pAdapterInfo );
        }
		else
            delete pAdapterInfo;
    }

    bool bUniqueDesc = true;
    CDeviceEnumAdapterInfo* pAdapterInfo;
    for( uint32 i=0; i<m_AdapterInfoList.size(); i++ )
    {
        CDeviceEnumAdapterInfo* pAdapterInfo1 = m_AdapterInfoList[i];

        for( uint32 j=i+1; j<m_AdapterInfoList.size(); j++ )
        {
            CDeviceEnumAdapterInfo* pAdapterInfo2 = m_AdapterInfoList[j];
            if( _stricmp( pAdapterInfo1->AdapterIdentifier.Description, 
                          pAdapterInfo2->AdapterIdentifier.Description ) == 0 )
            {
                bUniqueDesc = false;
                break;
            }
        }

        if( !bUniqueDesc )
            break;
    }

    for( uint32 i=0; i<m_AdapterInfoList.size(); i++ )
    {
        pAdapterInfo = m_AdapterInfoList[i];

        StringCchCopy( pAdapterInfo->szUniqueDescription, 100, pAdapterInfo->AdapterIdentifier.Description ); 
        pAdapterInfo->szUniqueDescription[100] = 0;

        if( !bUniqueDesc )
        {
            char sz[100];
            StringCchPrintf( sz, 100, " (#%d)", pAdapterInfo->AdapterOrdinal );
            StringCchCat( pAdapterInfo->szUniqueDescription, 256, sz );

        }
    }

    return S_OK;
}



//--------------------------------------------------------------------------------------
// Enumerates D3D devices for a particular adapter.
//--------------------------------------------------------------------------------------
HRESULT CDeviceEnumeration::EnumerateDevices( CDeviceEnumAdapterInfo* pAdapterInfo, std::vector<D3DFORMAT>* pAdapterFormatList )
{
    HRESULT hr;

    const D3DDEVTYPE devTypeArray[] = 
    { 
        D3DDEVTYPE_HAL, 
        D3DDEVTYPE_SW, 
        D3DDEVTYPE_REF 
    };
    const uint32 devTypeArrayCount = sizeof(devTypeArray) / sizeof(devTypeArray[0]);

    // Enumerate each Direct3D device type
    for( uint32 iDeviceType = 0; iDeviceType < devTypeArrayCount; iDeviceType++ )
    {
        CDeviceEnumDeviceInfo* pDeviceInfo = new CDeviceEnumDeviceInfo;
        if( pDeviceInfo == NULL )
            return E_OUTOFMEMORY;

        // Fill struct w/ AdapterOrdinal and D3DDEVTYPE
        pDeviceInfo->DeviceType = devTypeArray[iDeviceType];

        // Store device caps
        if( FAILED( hr = m_pD3D->GetDeviceCaps( pAdapterInfo->AdapterOrdinal, pDeviceInfo->DeviceType, 
                                                &pDeviceInfo->Caps ) ) )
        {
            delete pDeviceInfo;
            continue;
        }

        // Create a dummy device to verify that we really can create of this type.
        D3DDISPLAYMODE Mode;
        m_pD3D->GetAdapterDisplayMode(0, &Mode);
        D3DPRESENT_PARAMETERS pp;
        ZeroMemory( &pp, sizeof(D3DPRESENT_PARAMETERS) );
        pp.BackBufferWidth  = 1;
        pp.BackBufferHeight = 1;
        pp.BackBufferFormat = Mode.Format;
        pp.BackBufferCount  = 1;
        pp.SwapEffect       = D3DSWAPEFFECT_COPY;
        pp.Windowed         = TRUE;
        pp.hDeviceWindow    = m_pApplication->GetHWNDFocus();
        IDirect3DDevice9 *pDevice;
        if( FAILED( hr = m_pD3D->CreateDevice( pAdapterInfo->AdapterOrdinal, pDeviceInfo->DeviceType, m_pApplication->GetHWNDFocus(),
                                          D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &pDevice ) ) )
        {
            if( hr == D3DERR_NOTAVAILABLE )
            {
                delete pDeviceInfo;
                continue;
            }
        }
        SafeRelease( pDevice );

        // Get info for each devicecombo on this device
        if( FAILED( hr = EnumerateDeviceCombos( pAdapterInfo, pDeviceInfo, pAdapterFormatList ) ) )
        {
            delete pDeviceInfo;
            continue;
        }

        // If at least one devicecombo for this device is found, 
        // add the deviceInfo to the list
        if (pDeviceInfo->deviceSettingsComboList.size() > 0 )
            pAdapterInfo->deviceInfoList.push_back( pDeviceInfo );
        else
            delete pDeviceInfo;
    }

    return S_OK;
}



//--------------------------------------------------------------------------------------
// Enumerates DeviceCombos for a particular device.
//--------------------------------------------------------------------------------------
HRESULT CDeviceEnumeration::EnumerateDeviceCombos( CDeviceEnumAdapterInfo* pAdapterInfo, CDeviceEnumDeviceInfo* pDeviceInfo, std::vector<D3DFORMAT>* pAdapterFormatList )
{
    const D3DFORMAT backBufferFormatArray[] = 
    {   
        D3DFMT_A8R8G8B8, 
        D3DFMT_X8R8G8B8, 
        D3DFMT_A2R10G10B10, 
        D3DFMT_R5G6B5, 
        D3DFMT_A1R5G5B5, 
        D3DFMT_X1R5G5B5 
    };
    const uint32 backBufferFormatArrayCount = sizeof(backBufferFormatArray) / sizeof(backBufferFormatArray[0]);

    // See which adapter formats are supported by this device
    for( uint32 iFormat=0; iFormat<pAdapterFormatList->size(); iFormat++ )
    {
        D3DFORMAT adapterFormat = (*pAdapterFormatList)[iFormat];

        for( uint32 iBackBufferFormat = 0; iBackBufferFormat < backBufferFormatArrayCount; iBackBufferFormat++ )
        {
            D3DFORMAT backBufferFormat = backBufferFormatArray[iBackBufferFormat];

            for( int nWindowed = 0; nWindowed < 2; nWindowed++)
            {
                if( !nWindowed && pAdapterInfo->displayModeList.size() == 0 )
                    continue;

                if (FAILED( m_pD3D->CheckDeviceType( pAdapterInfo->AdapterOrdinal, pDeviceInfo->DeviceType, 
                                                     adapterFormat, backBufferFormat, nWindowed )))
                {
                    continue;
                }

                if( m_bRequirePostPixelShaderBlending )
                {
                    // If the backbuffer format doesn't support D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING
                    // then alpha test, pixel fog, render-target blending, color write enable, and dithering. 
                    // are not supported.
                    if( FAILED( m_pD3D->CheckDeviceFormat( pAdapterInfo->AdapterOrdinal, pDeviceInfo->DeviceType,
                                                           adapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                                                           D3DRTYPE_TEXTURE, backBufferFormat ) ) )
                    {
                        continue;
                    }
                }

                // If an application callback function has been provided, make sure this device
                // is acceptable to the app.
				if ( m_pApplication != NULL )
				{
					if ( !m_pApplication->OnIsDeviceAcceptable ( &pDeviceInfo->Caps, adapterFormat, backBufferFormat, FALSE != nWindowed ))
						continue;
                }
                
                // At this point, we have an adapter/device/adapterformat/backbufferformat/iswindowed
                // DeviceCombo that is supported by the system and acceptable to the app. We still 
                // need to find one or more suitable depth/stencil buffer format,
                // multisample type, and present interval.
                CDeviceEnumDeviceSettingsCombo* pDeviceCombo = new CDeviceEnumDeviceSettingsCombo;
                if( pDeviceCombo == NULL )
                    return E_OUTOFMEMORY;

                pDeviceCombo->AdapterOrdinal = pAdapterInfo->AdapterOrdinal;
                pDeviceCombo->DeviceType = pDeviceInfo->DeviceType;
                pDeviceCombo->AdapterFormat = adapterFormat;
                pDeviceCombo->BackBufferFormat = backBufferFormat;
                pDeviceCombo->Windowed = (nWindowed != 0);
               
                BuildDepthStencilFormatList( pDeviceCombo );
                BuildMultiSampleTypeList( pDeviceCombo );
                if (pDeviceCombo->multiSampleTypeList.size() == 0)
                {
                    delete pDeviceCombo;
                    continue;
                }
                BuildDSMSConflictList( pDeviceCombo );
                BuildPresentIntervalList(pDeviceInfo, pDeviceCombo );
                pDeviceCombo->pAdapterInfo = pAdapterInfo;
                pDeviceCombo->pDeviceInfo = pDeviceInfo;

                pDeviceInfo->deviceSettingsComboList.push_back( pDeviceCombo );
            
            }
        }
    }

    return S_OK;
}



//--------------------------------------------------------------------------------------
// Adds all depth/stencil formats that are compatible with the device 
//       and app to the given D3DDeviceCombo.
//--------------------------------------------------------------------------------------
void CDeviceEnumeration::BuildDepthStencilFormatList( CDeviceEnumDeviceSettingsCombo* pDeviceCombo )
{
    D3DFORMAT depthStencilFmt;
    for( uint32 idsf = 0; idsf < m_DepthStecilPossibleList.size(); idsf++ )
    {
        depthStencilFmt = m_DepthStecilPossibleList[idsf];
        if (SUCCEEDED(m_pD3D->CheckDeviceFormat(pDeviceCombo->AdapterOrdinal, 
                pDeviceCombo->DeviceType, pDeviceCombo->AdapterFormat, 
                D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthStencilFmt)))
        {
            if (SUCCEEDED(m_pD3D->CheckDepthStencilMatch(pDeviceCombo->AdapterOrdinal, 
                    pDeviceCombo->DeviceType, pDeviceCombo->AdapterFormat, 
                    pDeviceCombo->BackBufferFormat, depthStencilFmt)))
            {
                pDeviceCombo->depthStencilFormatList.push_back( depthStencilFmt );
            }
        }
    }
}




//--------------------------------------------------------------------------------------
// Adds all multisample types that are compatible with the device and app to
//       the given D3DDeviceCombo.
//--------------------------------------------------------------------------------------
void CDeviceEnumeration::BuildMultiSampleTypeList( CDeviceEnumDeviceSettingsCombo* pDeviceCombo )
{
    D3DMULTISAMPLE_TYPE msType;
    DWORD msQuality;
    for( uint32 imst = 0; imst < m_MultiSampleTypeList.size(); imst++ )
    {
        msType = m_MultiSampleTypeList[imst];
        if( SUCCEEDED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->AdapterOrdinal, 
                pDeviceCombo->DeviceType, pDeviceCombo->BackBufferFormat, 
                pDeviceCombo->Windowed, msType, &msQuality ) ) )
        {
            pDeviceCombo->multiSampleTypeList.push_back( msType );
            if( msQuality > m_nMultisampleQualityMax+1 )
                msQuality = m_nMultisampleQualityMax+1;
            pDeviceCombo->multiSampleQualityList.push_back( msQuality );
        }
    }
}




//--------------------------------------------------------------------------------------
// Find any conflicts between the available depth/stencil formats and
//       multisample types.
//--------------------------------------------------------------------------------------
void CDeviceEnumeration::BuildDSMSConflictList( CDeviceEnumDeviceSettingsCombo* pDeviceCombo )
{
    CDeviceEnumDSMSConflict DSMSConflict;

    for( uint32 iDS=0; iDS<pDeviceCombo->depthStencilFormatList.size(); iDS++ )
    {
        D3DFORMAT dsFmt = pDeviceCombo->depthStencilFormatList[iDS];

        for( uint32 iMS=0; iMS<pDeviceCombo->multiSampleTypeList.size(); iMS++ )
        {
            D3DMULTISAMPLE_TYPE msType = pDeviceCombo->multiSampleTypeList[iMS];

            if( FAILED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->AdapterOrdinal, pDeviceCombo->DeviceType,
                                                            dsFmt, pDeviceCombo->Windowed, msType, NULL ) ) )
            {
                DSMSConflict.DSFormat = dsFmt;
                DSMSConflict.MSType = msType;
                pDeviceCombo->DSMSConflictList.push_back( DSMSConflict );
            }
        }
    }
}



//--------------------------------------------------------------------------------------
// Adds all present intervals that are compatible with the device and app 
//       to the given D3DDeviceCombo.
//--------------------------------------------------------------------------------------
void CDeviceEnumeration::BuildPresentIntervalList( CDeviceEnumDeviceInfo* pDeviceInfo, 
                                                CDeviceEnumDeviceSettingsCombo* pDeviceCombo )
{
    uint32 pi;
    for( uint32 ipi = 0; ipi < m_PresentIntervalList.size(); ipi++ )
    {
        pi = m_PresentIntervalList[ipi];
        if( pDeviceCombo->Windowed )
        {
            if( pi == D3DPRESENT_INTERVAL_TWO ||
                pi == D3DPRESENT_INTERVAL_THREE ||
                pi == D3DPRESENT_INTERVAL_FOUR )
            {
                // These intervals are not supported in windowed mode.
                continue;
            }
        }
        // Note that D3DPRESENT_INTERVAL_DEFAULT is zero, so you
        // can't do a caps check for it -- it is always available.
        if( pi == D3DPRESENT_INTERVAL_DEFAULT ||
            (pDeviceInfo->Caps.PresentationIntervals & pi) )
        {
            pDeviceCombo->presentIntervalList.push_back( pi );
        }
    }
}



//--------------------------------------------------------------------------------------
// Release all the allocated CDeviceEnumAdapterInfo objects and empty the list
//--------------------------------------------------------------------------------------
void CDeviceEnumeration::ClearAdapterInfoList()
{
    CDeviceEnumAdapterInfo* pAdapterInfo;
    for( uint32 i=0; i<m_AdapterInfoList.size(); i++ )
    {
        pAdapterInfo = m_AdapterInfoList[i];
        delete pAdapterInfo;
    }

    m_AdapterInfoList.clear();
}



//--------------------------------------------------------------------------------------
// Call GetAdapterInfoList() after Enumerate() to get a STL vector of 
//       CDeviceEnumAdapterInfo* 
//--------------------------------------------------------------------------------------
std::vector<CDeviceEnumAdapterInfo*>* CDeviceEnumeration::GetAdapterInfoList()
{
    return &m_AdapterInfoList;
}



//--------------------------------------------------------------------------------------
CDeviceEnumAdapterInfo* CDeviceEnumeration::GetAdapterInfo( uint32 AdapterOrdinal )
{
    for( uint32 iAdapter=0; iAdapter<m_AdapterInfoList.size(); iAdapter++ )
    {
        CDeviceEnumAdapterInfo* pAdapterInfo = m_AdapterInfoList[iAdapter];
        if( pAdapterInfo->AdapterOrdinal == AdapterOrdinal )
            return pAdapterInfo;
    }

    return NULL;
}


//--------------------------------------------------------------------------------------
CDeviceEnumDeviceInfo* CDeviceEnumeration::GetDeviceInfo( uint32 AdapterOrdinal, D3DDEVTYPE DeviceType )
{
    CDeviceEnumAdapterInfo* pAdapterInfo = GetAdapterInfo( AdapterOrdinal );
    if( pAdapterInfo )
    {
        for( uint32 iDeviceInfo=0; iDeviceInfo<pAdapterInfo->deviceInfoList.size(); iDeviceInfo++ )
        {
            CDeviceEnumDeviceInfo* pDeviceInfo = pAdapterInfo->deviceInfoList[iDeviceInfo];
            if( pDeviceInfo->DeviceType == DeviceType )
                return pDeviceInfo;
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
CDeviceEnumDeviceSettingsCombo* CDeviceEnumeration::GetDeviceSettingsCombo( uint32 AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed )
{
    CDeviceEnumDeviceInfo* pDeviceInfo = GetDeviceInfo( AdapterOrdinal, DeviceType );
    if( pDeviceInfo )
    {
        for( uint32 iDeviceCombo=0; iDeviceCombo<pDeviceInfo->deviceSettingsComboList.size(); iDeviceCombo++ )
        {
            CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList[iDeviceCombo];
            if( pDeviceSettingsCombo->AdapterFormat == AdapterFormat &&
                pDeviceSettingsCombo->BackBufferFormat == BackBufferFormat &&
                pDeviceSettingsCombo->Windowed == bWindowed )
                return pDeviceSettingsCombo;
        }
    }

    return NULL;
}



//--------------------------------------------------------------------------------------
// Used to sort D3DDISPLAYMODEs
//--------------------------------------------------------------------------------------

static bool SortModesCallback(D3DDISPLAYMODE pdm1, D3DDISPLAYMODE pdm2 )
{
    if (pdm1.Width > pdm2.Width)
        return true;
    if (pdm1.Width < pdm2.Width)
        return false;
    if (pdm1.Height > pdm2.Height)
        return true;
    if (pdm1.Height < pdm2.Height)
        return false;
    if (pdm1.Format > pdm2.Format)
        return true;
    if (pdm1.Format < pdm2.Format)
        return false;
    if (pdm1.RefreshRate > pdm2.RefreshRate)
        return true;
    if (pdm1.RefreshRate < pdm2.RefreshRate)
        return false;
    return false;
}


//--------------------------------------------------------------------------------------
CDeviceEnumAdapterInfo::~CDeviceEnumAdapterInfo( void )
{
    CDeviceEnumDeviceInfo* pDeviceInfo;
    for( uint32 i=0; i<deviceInfoList.size(); i++ )
    {
        pDeviceInfo = deviceInfoList[i];
        delete pDeviceInfo;
    }
    deviceInfoList.clear();
}




//--------------------------------------------------------------------------------------
CDeviceEnumDeviceInfo::~CDeviceEnumDeviceInfo( void )
{
    CDeviceEnumDeviceSettingsCombo* pDeviceCombo;
    for( uint32 i=0; i<deviceSettingsComboList.size(); i++ )
    {
        pDeviceCombo = deviceSettingsComboList[i];
        delete pDeviceCombo;
    }
    deviceSettingsComboList.clear();
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::ResetPossibleDepthStencilFormats()
{
    m_DepthStecilPossibleList.clear();
    m_DepthStecilPossibleList.push_back( D3DFMT_D16 );
    m_DepthStecilPossibleList.push_back( D3DFMT_D15S1 );
    m_DepthStecilPossibleList.push_back( D3DFMT_D24X8 );
    m_DepthStecilPossibleList.push_back( D3DFMT_D24S8 );
    m_DepthStecilPossibleList.push_back( D3DFMT_D24X4S4 );
    m_DepthStecilPossibleList.push_back( D3DFMT_D32 );
}


//--------------------------------------------------------------------------------------
std::vector<D3DFORMAT>* CDeviceEnumeration::GetPossibleDepthStencilFormatList() 
{
    return &m_DepthStecilPossibleList;
}


//--------------------------------------------------------------------------------------
std::vector<D3DMULTISAMPLE_TYPE>* CDeviceEnumeration::GetPossibleMultisampleTypeList()
{
    return &m_MultiSampleTypeList;
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::ResetPossibleMultisampleTypeList()
{
    m_MultiSampleTypeList.clear();
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_NONE );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_NONMASKABLE );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_2_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_3_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_4_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_5_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_6_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_7_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_8_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_9_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_10_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_11_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_12_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_13_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_14_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_15_SAMPLES );
    m_MultiSampleTypeList.push_back( D3DMULTISAMPLE_16_SAMPLES );
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::GetPossibleVertexProcessingList( bool* pbSoftwareVP, bool* pbHardwareVP, bool* pbPureHarewareVP, bool* pbMixedVP )
{
    *pbSoftwareVP = m_bSoftwareVP;
    *pbHardwareVP = m_bHardwareVP;
    *pbPureHarewareVP = m_bPureHarewareVP;
    *pbMixedVP = m_bMixedVP;
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::SetPossibleVertexProcessingList( bool bSoftwareVP, bool bHardwareVP, bool bPureHarewareVP, bool bMixedVP )
{
    m_bSoftwareVP = bSoftwareVP;
    m_bHardwareVP = bHardwareVP;
    m_bPureHarewareVP = bPureHarewareVP;
    m_bMixedVP = bMixedVP;
}


//--------------------------------------------------------------------------------------
std::vector<uint32>* CDeviceEnumeration::GetPossiblePresentIntervalList()
{
    return &m_PresentIntervalList;
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::ResetPossiblePresentIntervalList()
{
    m_PresentIntervalList.clear();
    m_PresentIntervalList.push_back( D3DPRESENT_INTERVAL_IMMEDIATE );
    m_PresentIntervalList.push_back( D3DPRESENT_INTERVAL_DEFAULT );
    m_PresentIntervalList.push_back( D3DPRESENT_INTERVAL_ONE );
    m_PresentIntervalList.push_back( D3DPRESENT_INTERVAL_TWO );
    m_PresentIntervalList.push_back( D3DPRESENT_INTERVAL_THREE );
    m_PresentIntervalList.push_back( D3DPRESENT_INTERVAL_FOUR );
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::SetResolutionMinMax( uint32 nMinWidth, uint32 nMinHeight, 
                                           uint32 nMaxWidth, uint32 nMaxHeight )
{
    m_nMinWidth = nMinWidth;
    m_nMinHeight = nMinHeight;
    m_nMaxWidth = nMaxWidth;
    m_nMaxHeight = nMaxHeight;
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::SetRefreshMinMax( uint32 nMin, uint32 nMax )
{
    m_nRefreshMin = nMin;
    m_nRefreshMax = nMax;
}


//--------------------------------------------------------------------------------------
void CDeviceEnumeration::SetMultisampleQualityMax( uint32 nMax )
{
    if( nMax > 0xFFFF )
        nMax = 0xFFFF;
    m_nMultisampleQualityMax = nMax;
}

}
}