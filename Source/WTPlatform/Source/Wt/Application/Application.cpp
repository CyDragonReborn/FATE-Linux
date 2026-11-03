/*
	Application.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

*/

#include "WT/stdafx.h"

#define STRICT
#include <string>
#include <tchar.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include "WT/Application/Application.h"
#include "WT/Application/DirectXVersionUtil.h"
#include "WT/System/SystemException.h"

#pragma comment( lib, "version" )

using namespace std;

namespace WT
{
namespace Application
{

/*static*/ std::string CApplication::gm_DefaultSemaphoreName = "WTPlatform Semaphore";
/*static*/ std::string CApplication::gm_DefaultClassName = "WTPlatform Class";
/*static*/ CApplication* CApplication::gm_pInstance = NULL;	

// Retrieves the color depth of the desktop
uint32 GetDesktopBPP( HWND hWnd )
{
	uint32 Bpp = 16;
	HDC	hDC	= ::GetDC( hWnd );
	if ( hDC )
	{
		Bpp = ::GetDeviceCaps( hDC, BITSPIXEL );
		::ReleaseDC( hWnd, hDC );
	}
	return Bpp;
}

// Windows message handler 
static LRESULT CALLBACK WndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	// Forward the message to the application class
	if( CApplication::gm_pInstance != NULL )
	{
		return CApplication::gm_pInstance->MsgProc( hWnd, Msg, wParam, lParam );
	}
	return DefWindowProc( hWnd, Msg, wParam, lParam );
}

// Frame windows message handler 
static LRESULT CALLBACK FrameWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	// Forward the message to the application class
	if( CApplication::gm_pInstance != NULL )
	{
		return CApplication::gm_pInstance->FrameMsgProc( hWnd, Msg, wParam, lParam );
	}
	return DefWindowProc( hWnd, Msg, wParam, lParam );
}


CApplication::CApplication()
	: m_ShuttingDown( false )
	, m_Windowed( true )
	, m_DeviceLost( false )
	, m_Minimized( false )
	, m_Maximized( false )
	, m_IgnoreSizeChange( false )
	, m_DeviceObjectsInitialized( false )
	, m_DeviceObjectsRestored( false )
	, m_FullscreenFSAA( false )
	, m_WindowedFSAA( false )
	, m_SupportsFullscreenFSAA( false )
	, m_SupportsWindowedFSAA( false )
	, m_StartFullscreen( false )
	, m_StartCentered( true )
	, m_Created( false )
	, m_Active( false )
	, m_ShowCursor( false )
	, m_CreateMultithreadedDevice( false )
	, m_ProcessKeyEvents( true )
	, m_ProcessMouseEvents( true )
	, m_Focused( false )
	, m_AllowFSAA( true )
	, m_CanSwitchFSAA( false )
	, m_RequiresVerticalSync( false )
	, m_IsVerticalSynced( false )
	, m_CanStartFullscreen( true )
	, m_DisplayFramerate( false )
	, m_DisplayExtraStats( false )
	, m_WindowTitle( "WildTangent Platform Application" )
	, m_Framerate( 0 )
	, m_pStatusFont( NULL )
	, m_pFRText( NULL )
	, m_pStatsText( NULL )
	, m_WindowWidth( 640 )
	, m_WindowHeight( 480 )
	, m_RenderFailures( 0 )
	, m_pD3D( NULL )
	, m_pD3DDevice( NULL )
	, m_hWnd( NULL )
	, m_hFrameWnd ( NULL )
	, m_hMenu( NULL )
	, m_pDefaultAllState( NULL )
	, m_pDefaultPixelState( NULL )
	, m_pDefaultVertexState( NULL )
	, m_WindowStyle( 0 )
	, m_LeftFrameBorder( 0 )
	, m_RightFrameBorder( 0 )
	, m_TopFrameBorder( 0 )
	, m_BottomFrameBorder ( 0 )
	, m_FrameStyle( 0 )
{
	m_FrameStats[0] = _T( '\0' );

	// When m_ClipCursorWhenFullscreen is true, the cursor is limited to
	// the device window when the app goes fullscreen.  This prevents users
	// from accidentally clicking outside the app window on a multimon system.
	// This flag is turned off by default for debug builds, since it makes 
	// multimon debugging difficult.
#if defined(_DEBUG) || defined(DEBUG)
	m_ClipCursorWhenFullscreen = false;
#else
	m_ClipCursorWhenFullscreen = true;
#endif

	memset( &m_FrameBounds, 0, sizeof m_FrameBounds );
	memset( &m_WindowBounds, 0, sizeof m_FrameBounds );
	memset( &m_WindowClient, 0, sizeof m_FrameBounds );
	memset( &m_D3DCaps, 0, sizeof m_D3DCaps );
	memset( &m_BackBufferSurfaceDesc, 0, sizeof m_BackBufferSurfaceDesc );

	gm_pInstance = this;
}

CApplication::~CApplication()
{
	// Invalidate instance pointer
	if (gm_pInstance == this)
	{
		gm_pInstance = NULL;
	}
}

// Sets command-line parameters passed to WinMain
void CApplication::SetCommandLine( const std::string& CommandLine )
{
	m_CommandLine = CommandLine;
}

// Retrieves the original command-line parameters passed to WinMain
std::string CApplication::GetCommandLine() const
{
	return m_CommandLine;
}

// Creates the application window and initial Direct3D objects
HRESULT CApplication::Create( HINSTANCE hInstance, HICON hIcon , HMENU hMenu , bool UseFrame )
{
	// Create a semaphore for the game - this keeps other copies from running
	// Each Application instance will make this check, and if the semaphore already exists it will exit immediately
	// The semaphore is released when this application closes.
	std::string SemaphoreName = GetSemaphoreName();
	m_hInstanceSemaphore = ::CreateSemaphore( NULL, 
											  1, 
											  1, 
											  SemaphoreName.c_str() );
	if ( ::WaitForSingleObject( m_hInstanceSemaphore, 0 ) == WAIT_TIMEOUT )
	{
		::CloseHandle( m_hInstanceSemaphore );
		m_hInstanceSemaphore = NULL;

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
		return E_FAIL;
	}

	HRESULT	hr;

	m_hInstance = hInstance;
	OSVERSIONINFO Osvi;
	ZeroMemory( &Osvi, sizeof Osvi );
	Osvi.dwOSVersionInfoSize = sizeof Osvi;

	bool IsXP( false );
	if ( ::GetVersionEx( &Osvi ) )
	{
		switch( Osvi.dwMajorVersion )
		{
			// NT 3.51
		case 3:
				IsXP = false;

			// 2000 based OSes
		case 5:
			
			switch( Osvi.dwMinorVersion )
			{
			case 1:
				IsXP = true;
			default:
				IsXP = false;
			}

		default:
			IsXP = false;
		}

	}

	DWORD DirectXVersion = 0;
    TCHAR DirectXVersionString[10];

	hr = CDirectXVersionUtil::GetVersion( &DirectXVersion, DirectXVersionString, 10 );
	
	std::string LocalPath = "";
	if( DirectXVersion < 589824 && !IsXP )
	{
		m_CanStartFullscreen = false;
	}

	// Find the desktop color depth
	// If it isn't 16 or 32 bit color, we're going to exit too - no hardware acceleration
	uint32 Bpp = GetDesktopBPP( m_hWnd );
	if ( ( Bpp < 16 || Bpp == 24 ) )
	{
		// TODO: wtGameEngine had a feature that could handle this -- need to integrate
		// TODO: Also need to make use of the other "user friendliness" utilities this feature provides

		::MessageBox( NULL,
					  "This game must be run in 16- or 32-bit color mode. Please change your desktop color depth and try again.",
					  m_WindowTitle.c_str(),
					  MB_ICONERROR | MB_OK );

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}

		return E_FAIL;
	}

	/// TODO: Doesn't Direct3DCreate9 pop up a message box on failure (if the .dll's are not installed)?
	/// If so, there is a global Win32 function that can suspend this message box -- need to investigate
	try
	{
		// Create the Direct3D object
		m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

		// if we didn't succeed, shut down
		if ( m_pD3D == NULL )
		{
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		}

		// Check to see if this card is happy starting fullscreen
		// Retrieve the adapter's name, for any hacks we may need to do
		// TODO: Should this be externally controlled, or don't we care anymore?
		D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
		
		m_pD3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &AdapterIdentifier );

		std::string AdapterName = AdapterIdentifier.Description;		
		AdapterName = StringUpper( AdapterName );					// Put it in uppercase for easier compare later

		if( StringUpper( AdapterName ).find( "MATROX MILLENNIUM G400" ) != -1 )
		{
			m_CanStartFullscreen = false;
		}
		if( StringUpper( AdapterName ).find( "DIAMOND VIPER II Z200" ) != -1 )
		{
			m_CanStartFullscreen = false;
		}
		if( StringUpper( AdapterName ).find( "3DFX VOODOO" ) != -1 )
		{
			m_CanStartFullscreen = false;
		}
		if( StringUpper( AdapterName ).find( "VOODOO3" ) != -1 )
		{
			m_CanStartFullscreen = false;
		}

		if( !m_CanStartFullscreen )
		{
			m_StartFullscreen = false;
		}

		// Create the enumerator to find out what devices and device combos are supported on this machine
		m_pDeviceEnumeration = new CDeviceEnumeration( m_pD3D );

		// Perform the enumeration
		if ( FAILED( hr = m_pDeviceEnumeration->Enumerate() ) )
		{
			// Clean up if it didn't work
			SafeRelease( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}

		RECT WindowRect;
		::SetRect( &WindowRect, 0, 0, m_WindowWidth, m_WindowHeight );

		if ( m_hWnd == NULL )
		{
			// Set the window's initial style if it hasn't already been set by the user
			// if we have a frame window around the main rendering window, don't decorate the
			// rendering window
			if ( m_WindowStyle == 0 )
			{
				m_WindowStyle = UseFrame ? WS_POPUP :
								WS_OVERLAPPED |
								WS_CAPTION |
								WS_SYSMENU |
								WS_MINIMIZEBOX |
								WS_VISIBLE |
								WS_DLGFRAME;
			}

			::AdjustWindowRect( &WindowRect, m_WindowStyle, hMenu != NULL ? TRUE : FALSE );
		}

		// first create the frame window if requested

		if ( UseFrame )
		{
			// Register the windows class
			std::string FrameClassName = m_WindowTitle + "Frame";
			WNDCLASS FrameWndClass =
			{
				0,
				(WNDPROC)FrameWndProc,
				0,
				0,
				hInstance,
				hIcon,
				::LoadCursor( NULL, IDC_ARROW ),
				reinterpret_cast<HBRUSH>(::GetStockObject( WHITE_BRUSH )),
				NULL,
				FrameClassName.c_str()
			};
			::RegisterClass( &FrameWndClass );

			// Set the window's initial style if it hasn't already been set by the user
			if ( m_FrameStyle == 0 )
			{
				m_FrameStyle = WS_POPUP |
								WS_CAPTION |
								WS_SYSMENU |
								WS_MINIMIZEBOX |
								WS_VISIBLE |
								WS_SIZEBOX;
			}

			// Set the window's initial width
			::SetRect( &m_FrameBounds, 0, 0,
				       WindowRect.right-WindowRect.left + m_LeftFrameBorder + m_RightFrameBorder ,
					   WindowRect.bottom-WindowRect.top + m_TopFrameBorder + m_BottomFrameBorder );

			// Create the window
			m_hFrameWnd = ::CreateWindowEx ( WS_EX_LAYERED,
				                             FrameClassName.c_str(),
									         m_WindowTitle.c_str(),
									         m_FrameStyle,
									         CW_USEDEFAULT,
									         CW_USEDEFAULT,
									         m_FrameBounds.right-m_FrameBounds.left,
									         m_FrameBounds.bottom-m_FrameBounds.top,
									         0,
									         NULL,
									         hInstance,
									         0 );
			::GetWindowRect( m_hFrameWnd, &m_FrameBounds );
		}

		// Unless a substitute hWnd has been specified, create a window to render into
		if ( m_hWnd == NULL )
		{
			// Register the windows class
			std::string ClassName( GetClassName() );
			WNDCLASS wndClass =
			{
				CS_DBLCLKS,
				WndProc,
				0,
				0,
				hInstance,
				hIcon,
				::LoadCursor( NULL, IDC_ARROW ),
				reinterpret_cast<HBRUSH>(::GetStockObject( WHITE_BRUSH )),
				NULL,
				ClassName.c_str()
			};
			::RegisterClass( &wndClass );

			// Create the window
			m_hWnd = ::CreateWindow( ClassName.c_str(),
									 m_WindowTitle.c_str(),
									 m_hFrameWnd ? WS_POPUP : m_WindowStyle,
									 m_hFrameWnd ? m_FrameBounds.left+m_LeftFrameBorder : CW_USEDEFAULT,
									 m_hFrameWnd ? m_FrameBounds.top+m_TopFrameBorder : CW_USEDEFAULT,
									 WindowRect.right-WindowRect.left,
									 WindowRect.bottom-WindowRect.top,
									 m_hFrameWnd,
									 hMenu,
									 hInstance,
									 0 );
		}

		// Save window properties for use when we change resolutions, etc. later
		m_WindowStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
		::GetWindowRect( m_hWnd, &m_WindowBounds );
		::GetClientRect( m_hWnd, &m_WindowClient );

		// Find devices for windowed and fullscreen that match our requested resolution
		if ( FAILED( hr = ChooseInitialD3DSettings() ) )
		{
			SafeRelease( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}


		// Initialize any application specific data ( OneTimeSceneInit should be overridden )
		if ( FAILED( hr = OneTimeSceneInit() ) )
		{
			SafeRelease( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}

		// Initialize the 3D environment for the app
		if ( FAILED( hr = Initialize3DEnvironment() ) )
		{
			SafeRelease( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}

		m_Created = true;
		m_Active = true;
	}
	catch(...)
	{
		if( IsFullscreen() )
		{
			ToggleFullscreen();
		}
		Cleanup3DEnvironment();
		SafeRelease( m_pD3D );

		SafeDelete( m_pDeviceEnumeration );

		if( DirectXVersion < 589824 )
		{
			MessageBox( NULL, "\nTo play this game, you need DirectX 9.0 or higher.\nDownload a new version of DirectX from http://www.microsoft.com/directx and launch the game again.", "DirectX Upgrade", MB_OK | MB_ICONINFORMATION );
			// Close the window, which shuts down the app
			if ( m_hWnd )
			{
				::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
			}
			return E_FAIL;
		}
		else
		{
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		}
	}

	// Everything has been created and initialized.  Announce this by calling PostCreate.
	return PostCreate();
}

// Assembles the settings for fullscreen and windowed modes given what is known about the hardware from device enumeration
HRESULT CApplication::ChooseInitialD3DSettings()
{
	const bool FoundFullscreen = FindBestFullscreenMode();
	const bool FoundWindowed   = FindBestWindowedMode();

	if ( m_StartFullscreen && FoundFullscreen )
	{
		m_DeviceSettings.IsWindowed = false;
	}
	// If there is no acceptable windowed mode, we can only do fullscreen
	if ( !FoundWindowed && FoundFullscreen )
	{
		m_DeviceSettings.IsWindowed = false;
	}

	// If we didn't find anything at all...
	if ( ( !FoundFullscreen && !FoundWindowed ) ||
		 ( !m_StartFullscreen && !FoundWindowed ) )
	{
		// And we're still looking for a device with stencil capability...
		if ( m_pDeviceEnumeration->GetMinStencilBits() > 0 )
		{
			// try enumerating for devices with no stencil requirements -
			// no stencil shadows for us!
			m_pDeviceEnumeration->SetMinStencilBits( 0 );
			m_pDeviceEnumeration->Enumerate();
			return ChooseInitialD3DSettings();
		}
		return D3DAPPERR_NOCOMPATIBLEDEVICES;
	}

	return S_OK;
}

// Choose the best available display mode for windowed mode
bool CApplication::FindBestWindowedMode()
{
	// Get display mode of primary adapter (which is assumed to be where the window 
	// will appear)

	D3DDISPLAYMODE	PrimaryDesktopDisplayMode;
	m_pD3D->GetAdapterDisplayMode( 0, &PrimaryDesktopDisplayMode );

	AdapterInfo* pBestAdapterInfo = NULL;
	DeviceInfo* pBestDeviceInfo = NULL;
	DeviceCombo* pBestDeviceCombo = NULL;

	// Go through all the adapters
	const uint32 AdapterCount = (uint32)m_pDeviceEnumeration->GetAdapterList().size();
	for ( uint32 CurrentAdapter = 0; CurrentAdapter < AdapterCount; ++CurrentAdapter )
	{
		AdapterInfo* pAdapterInfo = m_pDeviceEnumeration->GetAdapterList()[CurrentAdapter];

		// And through all the devices for each adapter
		const uint32 DeviceInfoCount = (uint32)pAdapterInfo->DeviceInfoList.size();
		for ( uint32 CurrentDeviceInfo = 0; CurrentDeviceInfo < DeviceInfoCount; ++CurrentDeviceInfo )
		{
			DeviceInfo*	pDeviceInfo	= pAdapterInfo->DeviceInfoList[CurrentDeviceInfo];
			
			// If it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}

			// Go through all the device combos
			const uint32 DeviceComboCount = (uint32)pDeviceInfo->DeviceComboList.size();
			for ( uint32 CurrentDeviceCombo = 0; CurrentDeviceCombo < DeviceComboCount; ++CurrentDeviceCombo )
			{
				DeviceCombo* pDeviceCombo = pDeviceInfo->DeviceComboList[CurrentDeviceCombo];
				
				// See if the backbuffer matches the adapter format
				const bool AdapterMatchesBB = ( pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat );
				
				if ( !pDeviceCombo->IsWindowed )
				{
					// Not windowed, we don't care
					continue;
				}

				// If it doesn't match the desktop, we can't use it
				if ( pDeviceCombo->AdapterFormat != PrimaryDesktopDisplayMode.Format )
				{
					continue;
				}

				// If we haven't found a compatible DeviceCombo yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceCombo->DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesBB ) )
				{
					// keep this as our best combination

					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					// if this is exactly what we want, finish now
					if ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
						 AdapterMatchesBB )
					{
						// This windowed device combo looks great -- take it
						goto EndWindowedDeviceComboSearch;
					}
					// Otherwise keep looking for a better windowed device combo
				}
			}
		}
	}

EndWindowedDeviceComboSearch:

	// If no device was found, we're done
	if ( pBestDeviceCombo == NULL )
	{
		return false;
	}

	// update our settings to match this device
	m_DeviceSettings.Windowed_AdapterInfo = pBestAdapterInfo;
	m_DeviceSettings.Windowed_DeviceInfo  = pBestDeviceInfo;
	m_DeviceSettings.Windowed_DeviceCombo = pBestDeviceCombo;
	m_DeviceSettings.IsWindowed = true;
	m_DeviceSettings.Windowed_DisplayMode = PrimaryDesktopDisplayMode;
	m_DeviceSettings.Windowed_Width  = m_WindowClient.right-m_WindowClient.left;
	m_DeviceSettings.Windowed_Height = m_WindowClient.bottom-m_WindowClient.top;

	m_DeviceSettings.Windowed_DepthStencilBufferFormat = pBestDeviceCombo->DepthStencilFormatList[0];

	m_DeviceSettings.Windowed_MultisampleType = pBestDeviceCombo->MultiSampleTypeList[0];

	m_WindowedFSAA = false;
	m_CanSwitchFSAA = true;
	
	/* TODO:
	if ( bestDeviceInfo->m_caps.RasterCaps & D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE )
	{
		m_CanSwitchFSAA = false;
	}*/

	// Go through the multisample types available for the device
	for ( uint32 i = 1; i < pBestDeviceCombo->MultiSampleTypeList.size(); ++i )
	{
		// If it supports it, then we can do FSAA in this Windowed mode
		if ( pBestDeviceCombo->MultiSampleTypeList[i] == D3DMULTISAMPLE_2_SAMPLES )
		{
			m_SupportsWindowedFSAA = true;
			if( m_AllowFSAA )
			{
				m_WindowedFSAA = true;
				m_DeviceSettings.Windowed_MultisampleType = pBestDeviceCombo->MultiSampleTypeList[i];
			}
			break;
		}
	}
	m_DeviceSettings.Windowed_MultisampleQuality   = 0;
	m_DeviceSettings.Windowed_VertexProcessingType = pBestDeviceCombo->VertexProcessingTypeList[0];
	m_DeviceSettings.Windowed_PresentInterval      = 0;	// present interval is always 0 for windowed

	return true;
}

bool CApplication::FullscreenResolutionExists( uint32 Width, uint32 Height ) const
{
	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE	AdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestDisplayMode;

	BestAdapterDesktopDisplayMode.Width  = 0;
	BestAdapterDesktopDisplayMode.Height = 0;
	BestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	BestAdapterDesktopDisplayMode.RefreshRate = 0;

	AdapterInfo* pBestAdapterInfo = NULL;
	DeviceInfo*	pBestDeviceInfo = NULL;
	DeviceCombo* pBestDeviceCombo = NULL;

	// Go through all the adapters
	const uint32 AdapterCount = (uint32)m_pDeviceEnumeration->GetAdapterList().size();
	for ( uint32 CurrentAdapter = 0; CurrentAdapter < AdapterCount; ++CurrentAdapter )
	{
		AdapterInfo* pAdapterInfo = m_pDeviceEnumeration->GetAdapterList()[CurrentAdapter];
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &AdapterDesktopDisplayMode );

		// And through all the devices for each adapter
		const uint32 DeviceInfoCount = (uint32)pAdapterInfo->DeviceInfoList.size();
		for ( uint32 CurrentDeviceInfo = 0; CurrentDeviceInfo < DeviceInfoCount; ++CurrentDeviceInfo )
		{
			DeviceInfo*	pDeviceInfo = pAdapterInfo->DeviceInfoList[CurrentDeviceInfo];

			// If it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// Go through all the device combos
			const uint32 DeviceComboCount = (uint32)pDeviceInfo->DeviceComboList.size();
			for ( uint32 CurrentDeviceCombo = 0; CurrentDeviceCombo < DeviceComboCount; ++CurrentDeviceCombo )
			{
				DeviceCombo* pDeviceCombo = pDeviceInfo->DeviceComboList[CurrentDeviceCombo];
				
				// See if the backbuffer matches the adapter format
				const bool AdapterMatchesBB = ( pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat );

				// See if the adapter format matches the desktop
				const bool AdapterMatchesDesktop = ( pDeviceCombo->AdapterFormat == AdapterDesktopDisplayMode.Format );
				
				// If it is windowed, we don't want it
				if ( pDeviceCombo->IsWindowed )
				{
					continue;
				}

				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better), save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceInfo->DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   pBestDeviceCombo->AdapterFormat != AdapterDesktopDisplayMode.Format &&
					   AdapterMatchesDesktop) || 
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesDesktop &&
					   AdapterMatchesBB ) )
				{
					// Keep this as our best combination

					BestAdapterDesktopDisplayMode = AdapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					if ( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL && AdapterMatchesDesktop && AdapterMatchesBB )
					{
						// This fullscreen device combo looks great -- take it
						goto EndFullscreenDeviceComboSearch;
					}
					// Otherwise keep looking for a better fullscreen device combo
				}
			}
		}
	}

EndFullscreenDeviceComboSearch:

	// If we didn't come up with anything, we're done
	if ( pBestDeviceCombo == NULL )
	{
		return false;
	}

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's resolution as possible
	BestDisplayMode.Width  = 0;
	BestDisplayMode.Height = 0;
	BestDisplayMode.Format = D3DFMT_UNKNOWN;
	BestDisplayMode.RefreshRate = 0;

	// Go through all the available modes
	const uint32 DisplayModeCount = (uint32)pBestAdapterInfo->DisplayModeList.size();
	for ( uint32 CurrentDisplayMode = 0; CurrentDisplayMode < DisplayModeCount; ++CurrentDisplayMode )
	{
		D3DDISPLAYMODE DisplayMode	= pBestAdapterInfo->DisplayModeList[CurrentDisplayMode];

		// If the mode's format is not the same as our device combo, we don't want it
		if ( DisplayMode.Format != pBestDeviceCombo->AdapterFormat )
		{
			continue;
		}
		
		// If it exactly matches what we've requested, we're done
		if ( DisplayMode.Width == m_WindowWidth && DisplayMode.Height == m_WindowHeight &&
			 DisplayMode.RefreshRate == BestAdapterDesktopDisplayMode.RefreshRate )
		{
			// Found a perfect match, so stop
			BestDisplayMode = DisplayMode;
			break;
		}
		// If it matches the res, but has a better refresh, we'll take it too
		else if ( DisplayMode.Width == m_WindowWidth && DisplayMode.Height == m_WindowHeight &&
				  DisplayMode.RefreshRate > BestDisplayMode.RefreshRate )
		{
			// Refresh rate doesn't match, but width/height match, so keep this and keep looking
			BestDisplayMode = DisplayMode;
		}
		else if ( DisplayMode.Width == m_WindowWidth )
		{
			// Width matches, so keep this and keep looking
			BestDisplayMode = DisplayMode;
		}
		else if ( BestDisplayMode.Width == 0 )
		{
		 	// We don't have anything better yet, so keep this and keep looking
		 	BestDisplayMode = DisplayMode;
		}
	}
	if( BestDisplayMode.Width == Width && BestDisplayMode.Height == Height )
	{
		return true;
	}
	return false;
}

// Chooses the best available display mode for fullscreen
bool CApplication::FindBestFullscreenMode()
{
	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE	AdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestDisplayMode;

	BestAdapterDesktopDisplayMode.Width  = 0;
	BestAdapterDesktopDisplayMode.Height = 0;
	BestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	BestAdapterDesktopDisplayMode.RefreshRate = 0;

	AdapterInfo* pBestAdapterInfo = NULL;
	DeviceInfo* pBestDeviceInfo = NULL;
	DeviceCombo* pBestDeviceCombo = NULL;

	// go through all the adapters
	const uint32 AdapterCount = (uint32)m_pDeviceEnumeration->GetAdapterList().size();
	for ( uint32 CurrentAdapter = 0; CurrentAdapter < AdapterCount; ++CurrentAdapter )
	{
		AdapterInfo* pAdapterInfo = m_pDeviceEnumeration->GetAdapterList()[CurrentAdapter];
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &AdapterDesktopDisplayMode );

		// And through all the devices for each adapter
		const uint32 DeviceInfoCount = (uint32)pAdapterInfo->DeviceInfoList.size();
		for ( uint32 CurrentDeviceInfo = 0; CurrentDeviceInfo < DeviceInfoCount; ++CurrentDeviceInfo )
		{
			DeviceInfo* pDeviceInfo	= pAdapterInfo->DeviceInfoList[CurrentDeviceInfo];

			// If it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// Go through all the device combos
			const uint32 DeviceComboCount = (uint32)pDeviceInfo->DeviceComboList.size();
			for ( uint32 CurrentDeviceCombo = 0; CurrentDeviceCombo < DeviceComboCount; ++CurrentDeviceCombo )
			{
				DeviceCombo* pDeviceCombo = pDeviceInfo->DeviceComboList[CurrentDeviceCombo];
				
				// See if the backbuffer matches the adapter format
				const bool AdapterMatchesBB = ( pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat );

				// See if the adapter format matches the desktop
				const bool AdapterMatchesDesktop = ( pDeviceCombo->AdapterFormat == AdapterDesktopDisplayMode.Format );
				
				// If it is windowed, we don't want it
				if ( pDeviceCombo->IsWindowed )
				{
					continue;
				}

				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better), save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceInfo->DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   pBestDeviceCombo->AdapterFormat != AdapterDesktopDisplayMode.Format &&
					   AdapterMatchesDesktop) || 
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesDesktop &&
					   AdapterMatchesBB ) )
				{
					// Keep this as our best combination

					BestAdapterDesktopDisplayMode = AdapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					if ( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL &&
						 AdapterMatchesDesktop && AdapterMatchesBB )
					{
						// This fullscreen device combo looks great -- take it
						goto EndFullscreenDeviceComboSearch;
					}
					// Otherwise keep looking for a better fullscreen device combo
				}
			}
		}
	}

EndFullscreenDeviceComboSearch:

	// if we didn't come up with anything, we're done
	if ( pBestDeviceCombo == NULL )
	{
		return false;
	}

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's resolution as possible
	BestDisplayMode.Width  = 0;
	BestDisplayMode.Height = 0;
	BestDisplayMode.Format = D3DFMT_UNKNOWN;
	BestDisplayMode.RefreshRate = 0;

	// Go through all the available modes
	const uint32 DisplayModeCount = (uint32)pBestAdapterInfo->DisplayModeList.size();
	for ( uint32 CurrentDisplayMode = 0; CurrentDisplayMode < DisplayModeCount; ++CurrentDisplayMode )
	{
		D3DDISPLAYMODE DisplayMode = pBestAdapterInfo->DisplayModeList[CurrentDisplayMode];

		// If the mode's format is not the same as our device combo, we don't want it
		if ( DisplayMode.Format != pBestDeviceCombo->AdapterFormat )
		{
			continue;
		}
		// If it exactly matches what we've requested, we're done
		if ( DisplayMode.Width == m_WindowWidth && DisplayMode.Height == m_WindowHeight &&
			 DisplayMode.RefreshRate == BestAdapterDesktopDisplayMode.RefreshRate )
		{
			// Found a perfect match, so stop
			BestDisplayMode = DisplayMode;
			break;
		}
		// If it matches the res, but has a better refresh, we'll take it too
		else if ( DisplayMode.Width == m_WindowWidth && DisplayMode.Height == m_WindowHeight &&
				  DisplayMode.RefreshRate > BestDisplayMode.RefreshRate )
		{
			// Refresh rate doesn't match, but width/height match, so keep this
			// and keep looking
			BestDisplayMode = DisplayMode;
		}
		else if ( DisplayMode.Width == m_WindowWidth )
		{
			// Width matches, so keep this and keep looking
			BestDisplayMode = DisplayMode;
		}
		else if ( BestDisplayMode.Width == 0 )
		{
		 	// We don't have anything better yet, so keep this and keep looking
		 	BestDisplayMode = DisplayMode;
		}
	}

	// Fill in our settings to match this adapter/device
	m_DeviceSettings.Fullscreen_AdapterInfo = pBestAdapterInfo;
	m_DeviceSettings.Fullscreen_DeviceInfo = pBestDeviceInfo;
	m_DeviceSettings.Fullscreen_DeviceCombo = pBestDeviceCombo;
	m_DeviceSettings.IsWindowed = false;
	m_DeviceSettings.Fullscreen_DisplayMode = BestDisplayMode;
	m_DeviceSettings.Fullscreen_DepthStencilBufferFormat = pBestDeviceCombo->DepthStencilFormatList[0];

	m_DeviceSettings.Fullscreen_MultisampleType = pBestDeviceCombo->MultiSampleTypeList[0];

	m_CanSwitchFSAA = true;
	m_FullscreenFSAA = false;

	/* TODO:
	if ( bestDeviceInfo->m_caps.RasterCaps & D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE )
	{
		m_CanSwitchFSAA = false;
	}*/

	// Go through the multisample types available for the device
	const uint32 MultiSampleTypeCount = (uint32)pBestDeviceCombo->MultiSampleTypeList.size();
	for ( uint32 i = 1; i < MultiSampleTypeCount; ++i )
	{
		// If it supports it, then we can do FSAA in this Windowed mode
		if ( pBestDeviceCombo->MultiSampleTypeList[i] == D3DMULTISAMPLE_2_SAMPLES )
		{
			m_SupportsFullscreenFSAA = true;
			if( m_AllowFSAA )
			{
				m_FullscreenFSAA = true;
				m_DeviceSettings.Fullscreen_MultisampleType = pBestDeviceCombo->MultiSampleTypeList[i];
			}
			break;
		}
	}

	m_DeviceSettings.Fullscreen_VertexProcessingType = pBestDeviceCombo->VertexProcessingTypeList[0];
	m_DeviceSettings.Fullscreen_PresentInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // we flip right away

	return true;
}

// Determines whether a given resolution is available
bool CApplication::IsFullscreenResolutionAvailable( uint32 Width, uint32 Height )
{
	bool ResolutionAvailable = false;

	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE	AdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestDisplayMode;

	BestAdapterDesktopDisplayMode.Width  = 0;
	BestAdapterDesktopDisplayMode.Height = 0;
	BestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	BestAdapterDesktopDisplayMode.RefreshRate = 0;

	AdapterInfo* pBestAdapterInfo = NULL;
	DeviceInfo*	pBestDeviceInfo = NULL;
	DeviceCombo* pBestDeviceCombo = NULL;

	// Go through all the adapters
	const uint32 AdapterCount = (uint32)m_pDeviceEnumeration->GetAdapterList().size();
	for ( uint32 CurrentAdapter = 0; CurrentAdapter < AdapterCount; ++CurrentAdapter )
	{
		AdapterInfo* pAdapterInfo = m_pDeviceEnumeration->GetAdapterList()[CurrentAdapter];
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &AdapterDesktopDisplayMode );

		// And through all the devices for each adapter
		const uint32 DeviceInfoCount = (uint32)pAdapterInfo->DeviceInfoList.size();
		for ( uint32 CurrentDeviceInfo = 0; CurrentDeviceInfo < DeviceInfoCount; ++CurrentDeviceInfo )
		{
			DeviceInfo* pDeviceInfo = pAdapterInfo->DeviceInfoList[CurrentDeviceInfo];

			// If it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// Go through all the device combos
			const uint32 DeviceComboCount = (uint32)pDeviceInfo->DeviceComboList.size();
			for ( uint32 CurrentDeviceCombo = 0; CurrentDeviceCombo < DeviceComboCount; ++CurrentDeviceCombo )
			{
				DeviceCombo* pDeviceCombo = pDeviceInfo->DeviceComboList[CurrentDeviceCombo];

				// See if the backbuffer matches the adapter format
				const bool AdapterMatchesBB = ( pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat );

				// See if the adapter format matches the desktop
				const bool AdapterMatchesDesktop = ( pDeviceCombo->AdapterFormat == AdapterDesktopDisplayMode.Format );

				// If it is windowed, we don't want it
				if ( pDeviceCombo->IsWindowed )
				{
					continue;
				}

				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better), save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceInfo->DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   pBestDeviceCombo->AdapterFormat != AdapterDesktopDisplayMode.Format &&
					   AdapterMatchesDesktop) || 
					 ( pDeviceCombo->DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesDesktop &&
					   AdapterMatchesBB ) )
				{
					// Keep this as our best combination

					BestAdapterDesktopDisplayMode = AdapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					if ( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL && 
						 AdapterMatchesDesktop && AdapterMatchesBB )
					{
						// This fullscreen device combo looks great -- take it
						goto EndFullscreenDeviceComboSearch;
					}
					// Otherwise keep looking for a better fullscreen device combo
				}
			}
		}
	}

EndFullscreenDeviceComboSearch:

	// If we didn't come up with anything, we're done
	if ( pBestDeviceCombo == NULL )
	{
		return false;
	}

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's resolution as possible
	BestDisplayMode.Width  = 0;
	BestDisplayMode.Height = 0;
	BestDisplayMode.Format = D3DFMT_UNKNOWN;
	BestDisplayMode.RefreshRate = 0;

	// Go through all the available modes
	const uint32 DisplayModeCount = (uint32)pBestAdapterInfo->DisplayModeList.size();
	for ( uint32 CurrentDisplayMode = 0; CurrentDisplayMode < DisplayModeCount; ++CurrentDisplayMode )
	{
		D3DDISPLAYMODE DisplayMode = pBestAdapterInfo->DisplayModeList[CurrentDisplayMode];

		// If the mode's format is not the same as our device combo, we don't want it
		if ( DisplayMode.Format != pBestDeviceCombo->AdapterFormat )
		{
			continue;
		}

		// If it exactly matches what we've requested, we're done
		if ( DisplayMode.Width == Width && DisplayMode.Height == Height &&
			 DisplayMode.RefreshRate == BestAdapterDesktopDisplayMode.RefreshRate )
		{
			ResolutionAvailable = true;
			break;
		}
		// If it matches the res, but has a better refresh, we'll take it too
		else if ( DisplayMode.Width == Width && DisplayMode.Height == Height &&
				  DisplayMode.RefreshRate > BestDisplayMode.RefreshRate )
		{
			ResolutionAvailable = true;
		}
		else if ( DisplayMode.Width == Width )
		{
			// Width matches, so keep this and keep looking
			BestDisplayMode = DisplayMode;
		}
		else if ( BestDisplayMode.Width == 0 )
		{
		 	// We don't have anything better yet, so keep this and keep looking
		 	BestDisplayMode = DisplayMode;
		}
	}

	return ResolutionAvailable;
}

// Processes messages received from the redner window message pump
LRESULT CApplication::MsgProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	switch ( Msg )
	{
	case WM_PAINT:
		// We render on the paint message
		if ( m_pD3DDevice && !m_Active && m_Windowed && m_DeviceObjectsInitialized && m_DeviceObjectsRestored &&
			 !m_ShuttingDown )
		{
			Render();
			m_pD3DDevice->Present( NULL, NULL, NULL, NULL );
		}
		break;

	case WM_ACTIVATEAPP:
		m_Active = ( m_Created && (wParam != 0) );
		return 0;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		if( m_ProcessMouseEvents )
		{
			OnMouseButton( wParam, (Msg == WM_LBUTTONDOWN), Math::CVector2i( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) ) );
			return 0;
		}	
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
		if( m_ProcessKeyEvents )
		{
			if( Msg == WM_KEYDOWN )
				OnKeyDown( static_cast<uint8>(wParam) );
			else
				OnKeyUp( static_cast<uint8>(wParam) );

			return 0;
		}	
		break;
		
	case WM_SIZE:
		// Pick up possible changes to window style due to maximize, etc.
		if ( m_Windowed && m_hWnd != NULL )
		{
			m_WindowStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
		}

		if ( SIZE_MINIMIZED == wParam )
		{
			// if we're in fullscreen, clip the cursor
			if ( m_ClipCursorWhenFullscreen && !m_Windowed )
			{
				::ClipCursor( NULL );
			}
			// if we're in fullscreen, and trying to minimized -
			if ( !m_Windowed )
			{
				/*// get out of fullscreen! we want to be windowed
				if ( FAILED( ToggleFullscreen() ) )
				{
					DisplayErrorMsg( D3DAPPERR_RESETFAILED,
									 MSGERR_APPMUSTEXIT );
				}*/
			}
			m_Minimized = true;
			m_Maximized = false;
			UnFocus();
		}
		else if ( SIZE_MAXIMIZED == wParam )
		{
			::ShowWindow( m_hWnd, SW_SHOWNORMAL );
			m_Minimized = false;
			m_Maximized = true;
			Focus();

			HandlePossibleSizeChange();
		}
		else if ( SIZE_RESTORED == wParam )
		{
			if ( m_Maximized )
			{
				m_Maximized = false;
			}
			else if ( m_Minimized )
			{
				m_Minimized = false;
				::ShowWindow( m_hWnd, SW_SHOWNORMAL );
				Focus();
			}

			HandlePossibleSizeChange();
		}
		break;

	case WM_SETCURSOR:
		// Turn off Windows cursor
		if ( m_Active && ( HTCLIENT == LOWORD( lParam ) ) )
		{
			// If we want a visible cursor invisible over our app, hide it here
			if( m_Windowed )
			{
				if ( !m_ShowCursor )
				{
					::SetCursor( NULL );
				}
				else
				{
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
				}
			}
			else 
			{
				if ( !m_ShowCursor )
				{
					::SetCursor( NULL );
				}
				else
				{
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
				}
			}
			return false; // allow Windows to set cursor to window class cursor
		}
		break;

	case WM_KILLFOCUS:
		{
			UnFocus();// override this in your application to receive a lost focus event

			// If we are in fullscreen then try to refocus!
			if ( !m_Windowed && !m_Minimized )
			{
				//m_IsVerticalSynced = !m_RequiresVerticalSync;
				//m_Minimized = true;
				//::SetFocus( m_hWnd ); 	
			}
		}
		break;

	case WM_SETFOCUS:
			Focus();	// override this in your application to receive a focus event
		break;

	case WM_MOUSEMOVE:
		if( m_Active )
		{
			POINT ptCursor;
			::GetCursorPos( &ptCursor );
			// if fullscreen, bring the mouse cursor into client space
			if ( !m_Windowed )
			{
				::ScreenToClient( m_hWnd, &ptCursor );
			}

			// Set the mouse's cursor position in the D3D device
			if( m_pD3DDevice != NULL )
			{
				m_pD3DDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0 );
			}

			if( m_ProcessMouseEvents )
			{
				OnMouseMove( *reinterpret_cast<Math::CVector2i*>( &ptCursor ) );
			}
		}
		break;


	case WM_NCHITTEST:
		// Prevent the user from selecting the menu in fullscreen mode
		if ( !m_Windowed )
		{
			return HTCLIENT;
		}
		break;

	case WM_SYSCOMMAND:
		// Prevent moving/sizing and power loss in fullscreen mode
		switch ( wParam )
		{
		case SC_MOVE:
		case SC_SIZE:
		case SC_MAXIMIZE:
		case SC_KEYMENU:
		case SC_MONITORPOWER:
			if ( !m_Windowed )
			{
				return 1;
			}
			break;
		}
		break;

	case WM_SETTEXT:
		if ( m_hFrameWnd )
			SetWindowText ( m_hFrameWnd , (LPCSTR) lParam );
		break;

	case WM_COMMAND:
		/*
		switch ( LOWORD( wParam ) )
		{			
		case IDM_EXIT:
			// Recieved key/menu command to exit app
			if ( m_hWnd )
			{
				::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
			}
			return 0;
		}
		*/
		break;

	case WM_CLOSE:
		HMENU	hMenu;
		hMenu = ::GetMenu( hWnd );
		if ( hMenu != NULL )
		{
			::DestroyMenu( hMenu );
		}

		// clear up our d3d stuff
		Cleanup3DEnvironment();
		SafeRelease( m_pD3D );
		SafeDelete( m_pDeviceEnumeration );

		// Do any final application cleanup
		FinalCleanup();

		// Release our semaphore so that another copy of the app can run later
		if( m_hInstanceSemaphore != NULL )
		{
			::ReleaseSemaphore( m_hInstanceSemaphore, 1, NULL );
			::CloseHandle( m_hInstanceSemaphore );
			m_hInstanceSemaphore = NULL;
		}

		// Posting this quit message does the actual final shutdown
		::PostQuitMessage( 0 );

		return 0;
	}

	return DefWindowProc( hWnd, Msg, wParam, lParam );

}

void CApplication::UpdateFrameWindow( HWND hWnd )
{
}

LRESULT CApplication::FrameWindowHitTest( HWND hWnd , int X , int Y )
{
	return DefWindowProc ( hWnd , WM_NCHITTEST , 0 , MAKELPARAM ( X , Y ));
}

void CApplication::FrameWindowMove ( HWND hWnd )
{
	GetWindowRect ( m_hFrameWnd , &m_FrameBounds );
	SetWindowPos ( m_hWnd , NULL , m_FrameBounds.left + m_LeftFrameBorder , m_FrameBounds.top + m_TopFrameBorder,
		           0 , 0 , SWP_NOSIZE | SWP_NOZORDER );
}

void CApplication::FrameWindowResize ( HWND hWnd )
{
	GetWindowRect ( m_hFrameWnd , &m_FrameBounds );
	SetWindowPos ( m_hWnd , NULL , m_FrameBounds.left + m_LeftFrameBorder , m_FrameBounds.top + m_TopFrameBorder,
		           m_FrameBounds.right - m_FrameBounds.left - m_RightFrameBorder - m_LeftFrameBorder ,
				   m_FrameBounds.bottom - m_FrameBounds.top - m_BottomFrameBorder - m_TopFrameBorder , SWP_NOZORDER );
}

// Retrieves the top-level window associated with the Application instance
HWND CApplication::GetHWND()
{
	return m_hWnd;
}

// Retrieves the DeviceSettings structure associated with the Application instance
CDeviceSettings& CApplication::GetDeviceSettings()
{
	return m_DeviceSettings;
}

// Retrieves the Direct3D D3DPRESENT_PARAMETERS structure associated with the Application instance
D3DPRESENT_PARAMETERS& CApplication::GetPresentParameters()
{
	return m_PresentParameters;
}

// Associates the given IDirect3D9 pointer with the Application instance
void CApplication::SetD3D( LPDIRECT3D9 pD3D )
{
	if( pD3D != NULL )
	{
		pD3D->AddRef();
	}
	SafeRelease( m_pD3D );
	m_pD3D = pD3D;
}

// Retrieves the IDirect3D9 pointer associated with the Application instance
LPDIRECT3D9 CApplication::GetD3D()
{
	return m_pD3D;
}

// Associates the given IDirect3DDevice9 pointer with the Application instance
void CApplication::SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice )
{
	m_pD3DDevice = pD3DDevice;
}

// Retrieves the IDirect3DDevice9 pointer associated with the Application instance
LPDIRECT3DDEVICE9 CApplication::GetD3DDevice() const
{
	return m_pD3DDevice;
}

IDirect3DStateBlock9* CApplication::GetDefaultRenderStateAll() const
{
	return m_pDefaultAllState;
}

IDirect3DStateBlock9* CApplication::GetDefaultRenderStatePixel() const
{
	return m_pDefaultPixelState;
}

IDirect3DStateBlock9* CApplication::GetDefaultRenderStateVertex() const
{
	return m_pDefaultVertexState;
}

void CApplication::SetCursorVisibility( bool ShowCursor )	
{	
	m_ShowCursor = ShowCursor;	
}

void CApplication::SetStartupResolution( uint32 Width, uint32 Height )
{
	m_WindowWidth = Width;
	m_WindowHeight = Height;
}

void CApplication::SetStartupFullscreen( bool StartupFullscreen )	
{	
	m_StartFullscreen = StartupFullscreen;	
}

void CApplication::SetWindowTitle( const std::string& Title )
{
	m_WindowTitle = Title;	
	if( m_hWnd != NULL )
	{
		::SetWindowText( m_hWnd, m_WindowTitle.c_str() );
	}
}

void CApplication::SetWindowStyle( DWORD WindowStyle )
{
	m_WindowStyle = WindowStyle;
}

void CApplication::SetStartCentered( bool StartCentered )
{
	m_StartCentered = StartCentered;
}

bool CApplication::IsDeviceLost() const
{
	return m_DeviceLost;
}

bool CApplication::IsMinimized() const
{
	return m_Minimized;			
}

bool CApplication::IsMaximized() const
{	
	return m_Maximized;			
}

bool CApplication::IsWindowed() const
{	
	return m_Windowed;			
}

bool CApplication::IsFullscreen() const
{	
	return !IsWindowed();
}

bool CApplication::IsWindowedFSAA() const
{	
	return m_WindowedFSAA;		
}

bool CApplication::IsFullscreenFSAA() const
{	
	return m_FullscreenFSAA;
}

bool CApplication::SupportsWindowedFSAA() const
{	
	return m_SupportsWindowedFSAA;
}

bool CApplication::SupportsFullscreenFSAA() const
{	
	return m_SupportsFullscreenFSAA;
}

bool CApplication::CanSwitchFSAA() const
{	
	return m_CanSwitchFSAA;
}

bool CApplication::CanStartFullscreen() const
{	
	return m_CanStartFullscreen;		
}

bool CApplication::AllowFSAA() const
{	
	return m_AllowFSAA;		
}

void CApplication::SetAllowFSAA( bool AllowFSAA )
{	
	m_AllowFSAA = AllowFSAA;
}

void CApplication::SetRequiresVerticalSync( bool RequiresVerticalSync )
{	
	m_RequiresVerticalSync = RequiresVerticalSync;	
}

bool CApplication::RequiresVerticalSync()
{	
	return m_RequiresVerticalSync;	
}

bool CApplication::IsVerticalSynced()				
{	
	return m_IsVerticalSynced;		
}

// Determines whether the application is active, and not changing devices
bool CApplication::IsActive() const
{	
	return m_Active;		
}

float32 CApplication::GetFramerate() const
{	
	return m_Framerate;				
}

std::string CApplication::GetFramerateText() const
{	
	return m_FrameStats;		
}

std::string CApplication::FillFrameStats()
{
	std::ostringstream o;
	o << "FPS : " << setw( 6 ) << setprecision( 4 ) << GetFramerate();
	return o.str();
}

bool CApplication::IsFocused() const
{	
	return m_Focused;			
}

bool CApplication::IsFramerateDisplayed() const
{
	return m_DisplayFramerate;
}

void CApplication::SetDisplayFramerate( bool DisplayFramerate )
{
	m_DisplayFramerate = DisplayFramerate;
}

LRESULT CApplication::FrameMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		FrameWindowResize ( hWnd );
		UpdateFrameWindow( hWnd );
		break;
	case WM_MOVE:
		FrameWindowMove( hWnd );
		break;
	case WM_SETTEXT:
		UpdateFrameWindow(hWnd);
		break;
	case WM_NCHITTEST:
		return FrameWindowHitTest ( hWnd , GET_X_LPARAM(lParam) , GET_Y_LPARAM(lParam) );
	case WM_NCLBUTTONDOWN:
		if ( wParam == HTCLOSE)
			SendMessage ( hWnd , WM_SYSCOMMAND , SC_CLOSE , 0 );
		else if ( wParam == HTMINBUTTON )
			SendMessage ( hWnd , WM_SYSCOMMAND , SC_MINIMIZE , 0 );
		else if ( wParam == HTMAXBUTTON )
			SendMessage ( hWnd , WM_SYSCOMMAND , IsZoomed(hWnd) ? SC_RESTORE : SC_MAXIMIZE , 0 );
	}
	return DefWindowProc( hWnd, msg, wParam, lParam );
}

// Initializes the Direct3D device using previously cached settings
HRESULT CApplication::Initialize3DEnvironment()
{
	HRESULT hr;

	// get info about the adapter and device we want to use from our settings object
	DeviceInfo*	pDeviceInfo		= m_DeviceSettings.GetDeviceInfo();

	m_Windowed = m_DeviceSettings.IsWindowed;

	// Prepare window for possible windowed/fullscreen change
	AdjustWindowForChange();

	// Set up the presentation parameters
	BuildPresentParamsFromSettings();

	// if we somehow ended up with a ref device, error out
	if ( pDeviceInfo->Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
	{
		DisplayErrorMsg( D3DAPPERR_NULLREFDEVICE, 0 );
	}

	DWORD BehaviorFlags = 0L;

	// set vertex processing flags
	if ( m_DeviceSettings.GetVertexProcessingType() == EVertexProcessingType::Software )
	{
		BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else if ( m_DeviceSettings.GetVertexProcessingType() == EVertexProcessingType::Mixed )
	{
		BehaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
	}
	else if ( m_DeviceSettings.GetVertexProcessingType() == EVertexProcessingType::Hardware )
	{
		BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else if ( m_DeviceSettings.GetVertexProcessingType() == EVertexProcessingType::PureHardware )
	{
		BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING /*| D3DCREATE_PUREDEVICE*/;
	}

	// Append D3DCREATE_MULTITHREADED if application requests synchronized access to D3D device
	if( m_CreateMultithreadedDevice )
	{
		BehaviorFlags |= D3DCREATE_MULTITHREADED;
	}

	// Create the device with the presentation parameters we constructed
	hr = m_pD3D->CreateDevice( m_DeviceSettings.GetAdapterOrdinal(),
							   pDeviceInfo->DeviceType,
							   m_hWnd,
							   BehaviorFlags,
							   &m_PresentParameters,
							   &m_pD3DDevice );

	// If it worked...
	if ( SUCCEEDED( hr ) )
	{
		if( pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
        {
			m_DeviceDescription = m_DeviceSettings.GetAdapterInfo()->AdapterIdentifier.Description;
        }

		// Set up the window for windowed mode
		if ( m_Windowed )
		{
			// Center the window on the desktop if requested by the application
			if( m_StartCentered )
			{
				RECT rcDesktop;
				if( ::GetWindowRect( ::GetDesktopWindow(), &rcDesktop ) )
				{
					const LONG DesktopWidth = rcDesktop.right-rcDesktop.left;
					const LONG DesktopHeight= rcDesktop.bottom-rcDesktop.top;

					int WindowWidth;
					int WindowHeight;
					WindowWidth  = m_WindowBounds.right-m_WindowBounds.left;
					WindowHeight = m_WindowBounds.bottom-m_WindowBounds.top;
					m_WindowBounds.left   = (DesktopWidth/2) - (WindowWidth/2);
					m_WindowBounds.right  = m_WindowBounds.left + WindowWidth;
					m_WindowBounds.top    = (DesktopHeight/2) - (WindowHeight/2);
					m_WindowBounds.bottom = m_WindowBounds.top + WindowHeight;

				}				
			}

			if ( m_hFrameWnd )
			{
 				::SetWindowPos( m_hFrameWnd,
								HWND_NOTOPMOST,
								m_FrameBounds.left,
								m_FrameBounds.top,
								m_FrameBounds.right-m_FrameBounds.left,
								m_FrameBounds.bottom-m_FrameBounds.top,
								SWP_SHOWWINDOW );
				ShowWindow ( m_hFrameWnd , SW_SHOW );
				ShowWindow ( m_hWnd , SW_SHOW );
				// TODO : Window styles defines whether the window is visible, which shouldn't be overriden here.
			}
			else
			{
 				::SetWindowPos( m_hWnd,
								HWND_NOTOPMOST,
								m_WindowBounds.left,
								m_WindowBounds.top,
								m_WindowBounds.right-m_WindowBounds.left,
								m_WindowBounds.bottom-m_WindowBounds.top,
								0);		// SWP_SHOWWINDOW );
								// Window styles defines whether the window is visible, which shouldn't be overriden here.
			}
		}

		// Store device Caps
		m_pD3DDevice->GetDeviceCaps( &m_D3DCaps );

		// Store backbuffer surface description
		LPDIRECT3DSURFACE9 pBackBuffer = NULL;
		m_pD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
		pBackBuffer->GetDesc( &m_BackBufferSurfaceDesc );
		pBackBuffer->Release();

		// Set up the cursor if visible
		if ( m_ShowCursor )
		{
			HCURSOR	hCursor;
#ifdef _WIN64
			hCursor = ( HCURSOR ) ::GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
			hCursor = ( HCURSOR ) ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
		}

		// Confine cursor to fullscreen window
		if ( m_ClipCursorWhenFullscreen )
		{
			if ( !m_Windowed )
			{
				RECT rcWindow;
				::GetWindowRect( m_hWnd, &rcWindow );
				::ClipCursor( &rcWindow );
			}
			else
			{
				::ClipCursor( NULL );
			}
		}

		// Initialize the framework framerate display font and text objects
		InitStatusDisplayObjects();

		// Save device default render states
		SaveDeviceStates();

		// Initialize the app's device-dependent objects
		hr = InitDeviceObjects();

		if ( FAILED( hr ) )
		{
			DeleteDeviceObjects();
			DeleteStatusDisplayObjects();
			DeleteDeviceStates();
		}
		else
		{
			m_DeviceObjectsInitialized = true;
			hr = RestoreDeviceObjects();

			if ( FAILED( hr ) )
			{
				InvalidateDeviceObjects();
			}
			else
			{
				m_DeviceObjectsRestored = true;
				return S_OK;
			}
		}

		// Cleanup 
		Cleanup3DEnvironment();
	}

	// if that failed, we have no hardware device, and we have to quit
	if ( hr != D3DAPPERR_MEDIANOTFOUND &&
		 //hr != HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) &&
		 pDeviceInfo->DeviceType == D3DDEVTYPE_HAL )
	{
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );

	}

	return hr;
}

// Builds presentation parameters from the settings assembled based upon initial device enumeration
void CApplication::BuildPresentParamsFromSettings()
{
	// TODO: Fill in MSQuality level in m_PresentParameters
	::memset( &m_PresentParameters, 0, sizeof m_PresentParameters );

	m_PresentParameters.hDeviceWindow   = m_hWnd;
	m_PresentParameters.Windowed		= m_DeviceSettings.IsWindowed;
	m_PresentParameters.BackBufferCount = 1;
	m_PresentParameters.MultiSampleType = m_DeviceSettings.GetMultisampleType();
	m_PresentParameters.SwapEffect		= D3DSWAPEFFECT_DISCARD;

	if( m_RequiresVerticalSync )
	{
		m_IsVerticalSynced = true;
	}
	else
	{
		m_IsVerticalSynced = false;
	}

	m_PresentParameters.EnableAutoDepthStencil = true;
	m_PresentParameters.AutoDepthStencilFormat = m_DeviceSettings.GetDepthStencilBufferFormat();
	m_PresentParameters.Flags = 0;

	if ( m_Windowed )
	{
		m_PresentParameters.BackBufferWidth  = m_WindowClient.right-m_WindowClient.left;
		m_PresentParameters.BackBufferHeight = m_WindowClient.bottom-m_WindowClient.top;
		m_PresentParameters.BackBufferFormat = m_DeviceSettings.GetBackBufferFormat();
		m_PresentParameters.FullScreen_RefreshRateInHz = 0;
		m_PresentParameters.PresentationInterval = m_IsVerticalSynced ? m_DeviceSettings.GetPresentInterval() : D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	else
	{
		m_PresentParameters.BackBufferWidth  = m_DeviceSettings.GetDisplayMode().Width;
		m_PresentParameters.BackBufferHeight = m_DeviceSettings.GetDisplayMode().Height;
		m_PresentParameters.BackBufferFormat = m_DeviceSettings.GetBackBufferFormat();
		m_PresentParameters.FullScreen_RefreshRateInHz = m_DeviceSettings.Fullscreen_DisplayMode.RefreshRate;
		m_PresentParameters.PresentationInterval = m_IsVerticalSynced ? m_DeviceSettings.GetPresentInterval() : D3DPRESENT_INTERVAL_IMMEDIATE;
		
		if( m_IsVerticalSynced &&
			m_DeviceSettings.GetDeviceInfo()->Caps.RasterCaps & D3DPRESENT_INTERVAL_ONE )
		{
			m_PresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
	}
}

// Resets the Direct3D device
HRESULT CApplication::Reset3DEnvironment()
{
	HRESULT	hr;

	// Release all video memory objects
	if ( m_DeviceObjectsRestored )
	{
		m_DeviceObjectsRestored = false;
		InvalidateDeviceObjects();
		DeleteDeviceStates();
	}
	// Reset the device
	if ( FAILED( hr = m_pD3DDevice->Reset( &m_PresentParameters ) ) )
	{
		return hr;
	}

	// Store backbuffer description again - it might have changed
	LPDIRECT3DSURFACE9 pBackBuffer;
	m_pD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	pBackBuffer->GetDesc( &m_BackBufferSurfaceDesc );
	pBackBuffer->Release();

	// Save default device states
	SaveDeviceStates();

	// Set up the cursor if it should be visible
	if ( m_ShowCursor )
	{
		HCURSOR	hCursor;
#ifdef _WIN64
		hCursor = ( HCURSOR ) ::GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
		hCursor = ( HCURSOR ) ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
		m_pD3DDevice->ShowCursor( true );
	}

	// Confine cursor to fullscreen window
	if ( m_ClipCursorWhenFullscreen )
	{
		if ( !m_Windowed )
		{
			RECT WindowRect;
			::GetWindowRect( m_hWnd, &WindowRect );
			::ClipCursor( &WindowRect );
		}
		else 
		{
			// Don't clip in windowed mode
			::ClipCursor( NULL );
		}
	}

	// Initialize the app's device-dependent objects
	hr = RestoreDeviceObjects();

	// If we couldn't restore the objects, invalidate the m and fail
	if ( FAILED( hr ) )
	{
		::MessageBox( NULL, "FAILED RESTORE", "Load Error", MB_ICONERROR | MB_OK );
		InvalidateDeviceObjects();
		return hr;
	}
	m_DeviceObjectsRestored = true;

	return S_OK;
}


// Toggles the application between fullscreen and windowed modes
HRESULT CApplication::ToggleFullscreen()
{
	HRESULT		hr;
	uint32		AdapterOrdinalOld	= m_DeviceSettings.GetAdapterOrdinal();
	D3DDEVTYPE	DevTypeOld			= m_DeviceSettings.GetDeviceType();

	m_Active = false;

	// We're in the middle of changing the device over, so we make sure to let
	// other events know not to fiddle with us right now
	m_IgnoreSizeChange = true;

	if ( m_Windowed )
	{
		// If we are in windowed mode then save the window properties
		m_WindowStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
		::GetWindowRect( m_hWnd, &m_WindowBounds );
		::GetClientRect( m_hWnd, &m_WindowClient );
	}

	// Toggle the windowed state
	m_Windowed = !m_Windowed;

	// Keep our settings up to date
	m_DeviceSettings.IsWindowed = m_Windowed;

	m_StartFullscreen = !m_Windowed;

	// Prepare window for windowed/fullscreen change
	AdjustWindowForChange();

	// If we're switching to the same adapter and the same device, we can just reset,
	// and don't have to rebuild everything from scratch
	if ( m_DeviceSettings.GetAdapterOrdinal() == AdapterOrdinalOld &&
		 m_DeviceSettings.GetDeviceType() == DevTypeOld )
	{
		if ( !FAILED( hr = ChooseInitialD3DSettings() ) )
		{
			// Reset the 3D device
			BuildPresentParamsFromSettings();
			hr = Reset3DEnvironment();
		}
	}
	else
	{
		// Otherwise we gotta redo everything
		Cleanup3DEnvironment();
		hr = Initialize3DEnvironment();
	}


	if ( FAILED( hr ) )
	{
		// Out of video memory
		if( hr != D3DERR_OUTOFVIDEOMEMORY )
		{
			hr = D3DAPPERR_RESETFAILED;
		}

		// We've got to let things adjust again
		m_IgnoreSizeChange = false;

		// If we're in fullscreen, get out now - we obviously didn't succeed
		if ( !m_Windowed )
		{
			// Restore window type to windowed mode
			m_Windowed = !m_Windowed;
			m_DeviceSettings.IsWindowed = m_Windowed;
			AdjustWindowForChange();
			::SetWindowPos( m_hWnd,
						    HWND_NOTOPMOST,
						    m_WindowBounds.left,
						    m_WindowBounds.top,
						    m_WindowBounds.right-m_WindowBounds.left,
						    m_WindowBounds.bottom-m_WindowBounds.top,
						    SWP_SHOWWINDOW );
		}

		// We'll have to close now
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
	}

	// We've got to let things adjust again
	m_IgnoreSizeChange = false;

	// When moving from fullscreen to windowed mode, it is important to
	// adjust the window size after resetting the device rather than
	// beforehand to ensure that you get the window size you want.  For
	// example, when switching from 640x480 fullscreen to windowed with
	// a 1000x600 window on a 1024x768 desktop, it is impossible to set
	// the window size to 1000x600 until after the display mode has
	// changed to 1024x768, because windows cannot be larger than the
	// desktop.
	if ( m_Windowed )
	{
		// Heading back to windowed mode, so set up the rect and adjust the window
		RECT WindowRect;
		::SetRect( &WindowRect, 0, 0, m_WindowWidth, m_WindowHeight );
		::AdjustWindowRect( &WindowRect, m_WindowStyle, false );

		// Adjust client window rectangle
		::GetClientRect( m_hWnd, &m_WindowClient );
		m_WindowClient.right = m_WindowClient.left + m_WindowWidth;
		m_WindowClient.bottom = m_WindowClient.top + m_WindowHeight;

		// Keep window position but adjust size to meet current window client area
		m_WindowBounds.right = m_WindowBounds.left + ( WindowRect.right - WindowRect.left );
		m_WindowBounds.bottom = m_WindowBounds.top + ( WindowRect.bottom - WindowRect.top );

		// reposition the window
		::SetWindowPos( m_hWnd,
 					    HWND_NOTOPMOST,
					    m_WindowBounds.left,
					    m_WindowBounds.top,
					    m_WindowBounds.right - m_WindowBounds.left,
					    m_WindowBounds.bottom - m_WindowBounds.top,
					    SWP_SHOWWINDOW );
	}

	// Refocus
	::SetFocus( m_hWnd );
	m_Active = true;

	return S_OK;
}

// Performs a resolution change from one size to another
// Note this is not the same as toggling fullscreen
HRESULT CApplication::ChangeResolution( uint32 NewWidth, uint32 NewHeight )
{
	HRESULT	hr;
	uint32 AdapterOrdinalOld = m_DeviceSettings.GetAdapterOrdinal();
	D3DDEVTYPE DevTypeOld	 = m_DeviceSettings.GetDeviceType();

	m_WindowWidth = NewWidth;
	m_WindowHeight = NewHeight;

	// if we're in windowed
	if ( m_Windowed )
	{
		// Set the window's new rect size
		RECT WindowRect;
		::SetRect( &WindowRect, 0, 0, m_WindowWidth, m_WindowHeight );
		::AdjustWindowRect( &WindowRect, m_WindowStyle, false );

		// Save window properties
		m_WindowStyle = ::GetWindowLong( m_hWnd, GWL_STYLE );
		::GetWindowRect( m_hWnd, &m_WindowBounds );
		::GetClientRect( m_hWnd, &m_WindowClient );

		// Make sure the client and bounds are ok
		m_WindowClient.right	= m_WindowClient.left + m_WindowWidth;
		m_WindowClient.bottom	= m_WindowClient.top + m_WindowHeight;

		m_WindowBounds.right	= m_WindowBounds.left + ( WindowRect.right - WindowRect.left );
		m_WindowBounds.bottom	= m_WindowBounds.top + ( WindowRect.bottom - WindowRect.top );
	}

	// Prepare window for change
	AdjustWindowForChange();

	// If we're switching to the same adapter and the same device, we can just reset,
	// and don't have to rebuild everything from scratch
	if ( m_DeviceSettings.GetAdapterOrdinal() == AdapterOrdinalOld &&
		 m_DeviceSettings.GetDeviceType() == DevTypeOld )
	{
		if ( !FAILED( hr = ChooseInitialD3DSettings() ) )
		{
			// Reset the 3D device
			BuildPresentParamsFromSettings();
			hr = Reset3DEnvironment();
		}
	}
	else
	{
		// Otherwise we gotta start over
		Cleanup3DEnvironment();
		hr = Initialize3DEnvironment();
	}

	// Couldn't change resolution
	if ( FAILED( hr ) )
	{
		if ( hr != D3DERR_OUTOFVIDEOMEMORY )
		{
			hr = D3DAPPERR_RESETFAILED;
		}
		// If we're in fullscreen, drop back to desktop
		if ( !m_Windowed )
		{
			// Restore window type to windowed mode
			m_Windowed = !m_Windowed;
			m_DeviceSettings.IsWindowed = m_Windowed;
			AdjustWindowForChange();
			::SetWindowPos( m_hWnd,
						    HWND_NOTOPMOST,
						    m_WindowBounds.left,
						    m_WindowBounds.top,
						    m_WindowBounds.right - m_WindowBounds.left,
						    m_WindowBounds.bottom - m_WindowBounds.top,
						    SWP_SHOWWINDOW );
		}
		// And error-out
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
	}

	// Reposition the window in windowed mode
	if ( m_Windowed )
	{
		::SetWindowPos( m_hWnd,
					    HWND_NOTOPMOST,
					    m_WindowBounds.left,
					    m_WindowBounds.top,
					    m_WindowBounds.right - m_WindowBounds.left,
					    m_WindowBounds.bottom - m_WindowBounds.top,
					    SWP_SHOWWINDOW );
	}

	// Refocus
	::SetFocus( m_hWnd );
	
	// Handle any window widget changes that need to be done
	HandlePossibleSizeChange();

	return S_OK;
}

// Resets the device if the client area has changed
HRESULT CApplication::HandlePossibleSizeChange()
{
	HRESULT	hr = S_OK;

	if (m_hWnd == NULL)
	{
		//We're not ready to handle this message yet.
		//Note: I added this test to prevent access to m_WindowClient, which 
		//used to be uninitialized at this point, as well as use of the NULL
		//window handle. m_hWnd and m_WindowClient won't be usable until after
		//our window is actually created. - GWH 10/3/05
		return hr;
	}

	RECT OldClientRect( m_WindowClient );

	// if we're in the middle of an operation on the device, we can't do this 
	if ( m_IgnoreSizeChange )
	{
		return S_OK;
	}

	// Update window properties
	::GetWindowRect( m_hWnd, &m_WindowBounds );
	::GetClientRect( m_hWnd, &m_WindowClient );

	// if the size of the window has changed
	if ( OldClientRect.right - OldClientRect.left != m_WindowClient.right - m_WindowClient.left ||
		 OldClientRect.bottom - OldClientRect.top != m_WindowClient.bottom - m_WindowClient.top )
	{
		m_Active = false;

		m_WindowWidth = m_WindowClient.right - m_WindowClient.left;
		m_WindowHeight = m_WindowClient.bottom - m_WindowClient.top;

		// A new window size will require a new backbuffer
		// size, so the 3D structures must be changed accordingly.
		m_PresentParameters.BackBufferWidth = m_WindowClient.right - m_WindowClient.left;
		m_PresentParameters.BackBufferHeight = m_WindowClient.bottom - m_WindowClient.top;

		if ( m_pD3DDevice != NULL )
		{
			// Reset the 3D environment
			if ( FAILED( hr = Reset3DEnvironment() ) )
			{
				if ( hr != D3DERR_OUTOFVIDEOMEMORY )
				{
					hr = D3DAPPERR_RESETFAILED;
				}
				DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
			}
		}

		m_Active = true;
	}
	return hr;
}

// Prepares the application window for a resolution or windowed/fullscreen change
HRESULT CApplication::AdjustWindowForChange()
{
	// This method basically just sets up the menus and window properties so that
	// things look correct for windowed/fullscreen

	if ( m_Windowed )
	{
		// Set windowed-mode style
		::SetWindowLong( m_hWnd, GWL_STYLE, m_WindowStyle );
		if ( m_hMenu != NULL )
		{
			::SetMenu( m_hWnd, m_hMenu );
			m_hMenu = NULL;
		}
	}
	else
	{
		// Set fullscreen-mode style
		::SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE );
		if ( m_hMenu == NULL )
		{
			m_hMenu = GetMenu( m_hWnd );
			::SetMenu( m_hWnd, NULL );
		}
	}
	return S_OK;
}

bool CApplication::ShouldClose()
{	
	return false;	
}

HRESULT CApplication::UnFocus()
{	
	m_Focused = false;	
	return S_OK;	
}

HRESULT CApplication::Focus()
{	
	m_Focused = true;	
	return S_OK;	
}

// Allows the application to process key down events
// Called if the application class is configured to process key events
void CApplication::OnKeyDown( uint8 Key )
{
	Key;
}

// Allows the application to process key up events
// Called if the application class is configured to process key events
void CApplication::OnKeyUp( uint8 Key )
{
	Key;
}

// Allows the application to process mouse button events
// Called if the application class is configured to process mouse events
void CApplication::OnMouseButton( UINT_PTR VirtualKeyMask, bool Pressed, const Math::CVector2i& MousePos )
{
	VirtualKeyMask;
	Pressed;
	MousePos;
}

// Allows the application to process mouse movement
// Called if the application class is configured to process mouse events
void CApplication::OnMouseMove( const Math::CVector2i& MousePos )
{
	MousePos;
}

// Runs the Windows message pump for the application and renders the scene during idle time
int32 CApplication::Run()
{
	// Now we're ready to recieve and process Windows messages.
	bool GotMessage = false;
	MSG Msg;
	Msg.message = WM_NULL;
	::PeekMessage( &Msg, NULL, 0U, 0U, PM_NOREMOVE );

	// Start the game timer
	m_GameTimer.Start();

	try
	{

	// Keep looping until we get a quit message
	while ( WM_QUIT != Msg.message )
	{
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		if ( m_Active )
		{
			GotMessage = ( ::PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );
		}
		else
		{
			GotMessage = ( ::GetMessage( &Msg, NULL, 0U, 0U ) != 0 );
		}

		// If we got a message
		if ( GotMessage )
		{
			// Quit knocks us out
			if ( Msg.message == WM_QUIT )
			{
				break;
			}
			
			// Otherwise we ferry them on
			TranslateMessage( &Msg );
			DispatchMessage( &Msg );			
		}
		else	// no message, so we do our business
		{
			// If we aren't in the process of shutting down...
			if ( !m_ShuttingDown )
			{
				// If we have no device, we just sleep and let the CPU roll on
				if ( m_DeviceLost )
				{
					// Yield some CPU time to other processes
					::Sleep( 100 ); 
				}
				// Render a frame during idle time (since no messages are waiting)
				if ( m_Active )
				{
					// Try to render everything
					if ( FAILED( Render3DEnvironment() ) )
					{
						++m_RenderFailures;
						if( m_RenderFailures > 2 )
						{
							m_ShuttingDown = true;
							::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
						}
					}
					else
					{
						m_RenderFailures = 0;
					}
					if ( ShouldClose() ) // ShouldClose can be overridden so that you can notify the game to exit
					{
						m_ShuttingDown = true;
						::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
					}
				}
			}
		}
	} // end message loop

	} // end try block
	catch( System::CSystemException& e )
	{
		// Top level exception handler for WTPlatform type exceptions.
		// Display text message and exit application.
		if ( IsFullscreen() )
		{
			ToggleFullscreen();
		}

		::MessageBox( m_hWnd, e.GetMessage().c_str(), m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );
		::MessageBox( m_hWnd, "This application must now exit", m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );

		return -1;
	}
	catch( ... )
	{
		// Top level exception handler for all other exception types
		// Display text message and exit application.
		if ( IsFullscreen() )
		{
			ToggleFullscreen();
		}

		::MessageBox( m_hWnd, "An unknown error has occurred.  This application must now exit", m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );

		return -1;
	} // end catch block

	return ( int32 ) Msg.wParam;
}

// Renders the current scene
HRESULT CApplication::Render3DEnvironment()
{
	HRESULT	hr;

	// if our device has been lost, we need to recreate it
	if ( m_DeviceLost )
	{
		// Test the cooperative level to see if it's okay to render
		if ( FAILED( hr = m_pD3DDevice->TestCooperativeLevel() ) )
		{
			// If the device was lost, do not render until we get it back
			if ( D3DERR_DEVICELOST == hr )
			{
				UnFocus();
				return S_OK;
			}

			// Check if the device needs to be reset.
			if ( D3DERR_DEVICENOTRESET == hr )
			{
				// If we are windowed, read the desktop mode and use the same format for
				// the back buffer
				if ( m_Windowed )
				{
					AdapterInfo* pAdapterInfo = m_DeviceSettings.GetAdapterInfo();
					m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &m_DeviceSettings.Windowed_DisplayMode );
					m_PresentParameters.BackBufferFormat = m_DeviceSettings.Windowed_DisplayMode.Format;
				}

				// try to reset the device
				if ( FAILED( hr = Reset3DEnvironment() ) )
				{
					return hr;
				}
			}
			return hr;
		}
		m_DeviceLost = false;
	}

	// Frame move the scene
	if ( FAILED( hr = FrameMove() ) ) // override frame move to perform application logic
	{
		return hr;
	}

	if ( !m_Minimized )
	{
		// Render the scene as normal
		if ( FAILED( hr = Render() ) )	// override render to do your actual rendering
		{
			return hr;
		}
	}

	// Render the framerate text if requested
	if ( (m_DisplayFramerate && m_pFRText) || (m_DisplayExtraStats && m_pStatsText) )
	{
		// Make sure we are in a known render state
		if ( m_pDefaultAllState )
			m_pDefaultAllState->Apply();

		float32 Y = 0.0f;
		if( m_pFRText )
		{
			m_pFRText->SetPosition( 0.0f, Y );
			Y += 12.0f;
		}
		if( m_pStatsText )
		{
			m_pStatsText->SetPosition( 0.0f, Y );
		}

		if( m_pFRText != NULL || m_pStatsText != NULL )
		{
			WT::Graphics::CCamera Camera;
			Camera.SetRenderTarget( m_pD3DDevice );
			WT::Graphics::CRenderContext Context( &Camera );

			m_pD3DDevice->BeginScene();

			if (m_pFRText != NULL)
			{
				m_pFRText->Render( m_pD3DDevice, Context );
			}
			if( m_pStatsText != NULL )
			{
				m_pStatsText->Render( m_pD3DDevice, Context );
			}

			m_pD3DDevice->EndScene();
		}
	}

	// Calculate elapsed time
	static float32 LastTime	= 0.0f;
	static uint32 Frames	= 0;

	float32	Time 			= m_GameTimer.GetTotalTimeSeconds();
	++Frames;

	// Update the scene stats once per second
	if ( Time - LastTime > 1.0f )
	{
		m_Framerate = Frames / ( Time - LastTime );
		LastTime = Time;
		Frames = 0;

		m_FrameStats = FillFrameStats();
		
		WCHAR Buffer[256];
		swprintf( Buffer, L"%S", m_FrameStats.c_str() );

		if ( m_DisplayFramerate && m_pFRText )
			m_pFRText->SetText ( Buffer );
	}

	// Show the frame on the primary surface.
	hr = m_pD3DDevice->Present( NULL, NULL, NULL, NULL );

	// If we lost the device, we'll need to try to recreate it next go-round
	if ( D3DERR_DEVICELOST == hr )
	{
		m_DeviceLost = true;
	}

	// Update our present parameters if the vertical sync setting has changed
	if( !m_DeviceLost &&
		m_RequiresVerticalSync != m_IsVerticalSynced &&
		m_DeviceSettings.GetDeviceInfo()->Caps.RasterCaps & D3DPRESENT_INTERVAL_ONE )
	{
		BuildPresentParamsFromSettings();

		// Try to reset the device
		if ( FAILED( hr = Reset3DEnvironment() ) )
		{
			return hr;
		}
	}

	return S_OK;
}

// Frees the Direct3D device any any allocated Direct3D objects
void CApplication::Cleanup3DEnvironment()
{
	if ( m_pD3DDevice != NULL )
	{
		if ( m_DeviceObjectsRestored )
		{
			m_DeviceObjectsRestored = false;
			InvalidateDeviceObjects();
		}
		if ( m_DeviceObjectsInitialized )
		{
			m_DeviceObjectsInitialized = false;
			DeleteDeviceObjects();
			DeleteStatusDisplayObjects();
			DeleteDeviceStates();
		}

		ULONG RefCount = m_pD3DDevice->Release();
		if( RefCount > 0 )
		{
			DisplayErrorMsg( D3DAPPERR_NONZEROREFCOUNT, MSGERR_APPMUSTEXIT );
		}

		m_pD3DDevice = NULL;
	}
}

// Sets up Font/Text objects for framerate display
bool CApplication::InitStatusDisplayObjects()
{
	DeleteStatusDisplayObjects();

	using namespace Graphics::UI;

	// Create the font
	m_pStatusFont = new CSystemFont();
	m_pStatusFont->Initialize( "Arial", 10, ECharacterSet::Default, 256, 256, EFontWeight::Bold, 
						   false, false, false, false, true );
	wstring LowerChars( L"abcdefghijklmnopqrstuvwxyz1234567890-=,.;'/" );
	wstring UpperChars( L"ABCDEFGHIJKLMNOPQRSTUVWXYZ, <>?()*&^%$#@!:\"" );
	m_pStatusFont->BeginUpdate( m_pD3DDevice );
	m_pStatusFont->AddChars( LowerChars );
	m_pStatusFont->AddChars( UpperChars );
	m_pStatusFont->EndUpdate();

	// Create the text rendering object
	m_pFRText = new CText( m_pD3DDevice, m_pStatusFont, 0xffffff00, CVector2f( 0.0f, 0.0f ) );
	m_pFRText->SetAlignment( ETextAlignment::MiddleLeft );

	// Create the text rendering object
	m_pStatsText = new CText( m_pD3DDevice, m_pStatusFont, 0xffffff00, CVector2f( 0.0f, 0.0f ) );
	m_pStatsText->SetAlignment( ETextAlignment::MiddleLeft );
	// Give stats text an initial value (it may be overridden by the client application)
	m_pStatsText->SetText( WT::AnsiToWide( m_DeviceDescription, StringEncoding::UTF8 ) );
	
	return true;
}

// Cleans up Font/Text objects for framerate display
void CApplication::DeleteStatusDisplayObjects()
{
	SafeDelete( m_pFRText );
	SafeDelete( m_pStatsText );
	SafeDelete( m_pStatusFont );
}

// Saves the default render state of current device into state blocks
void CApplication::SaveDeviceStates()
{
	DeleteDeviceStates();

	HRESULT hr = m_pD3DDevice->CreateStateBlock( D3DSBT_ALL, &m_pDefaultAllState );
	if ( !SUCCEEDED(hr) )
	{
		SafeRelease( m_pDefaultAllState );
	}

	hr = m_pD3DDevice->CreateStateBlock( D3DSBT_PIXELSTATE, &m_pDefaultPixelState );
	if ( !SUCCEEDED(hr) )
	{
		SafeRelease( m_pDefaultPixelState );
	}

	hr = m_pD3DDevice->CreateStateBlock( D3DSBT_VERTEXSTATE, &m_pDefaultVertexState );
	if ( !SUCCEEDED(hr) )
	{
		SafeRelease( m_pDefaultVertexState );
	}
}

void CApplication::DeleteDeviceStates()
{
	SafeRelease( m_pDefaultAllState );
	SafeRelease( m_pDefaultPixelState );
	SafeRelease( m_pDefaultVertexState );
}

// Internal error handling function; displays error messages in a message box
HRESULT CApplication::DisplayErrorMsg( HRESULT hr, DWORD dwType )
{
	// TODO: Most of this is not appropriate for shipping production titles
	// TODO: Consider showing only in Debug builds, and choosing a single default error string
	// TODO: for Release builds

	static bool FatalErrorReported = false;

	static const TCHAR NoDirect3D[] = _T( "Could not initialize Direct3D. You may\nwant to check that the latest version of\nDirectX is correctly installed on your\nsystem." );
	static const TCHAR NoCompatibleDevices[] = _T( "Could not find any compatible Direct3D\ndevices" );
	static const TCHAR NoWindowableDevices[] = _T( "This game cannot run in a desktop\nwindow with the current display settings.\nPlease change your desktop settings to a\n16- or 32-bit display mode and re-run this\nsample." );
	static const TCHAR NoHardwareDevice[] = _T( "No hardware-accelerated Direct3D devices\nwere found." );
	static const TCHAR HalNotCompatible[] =  _T( "This game requires functionality that is\nnot available on your Direct3D hardware\naccelerator." );
	static const TCHAR NoWindowedHal[] = _T( "Your Direct3D hardware accelerator cannot\nrender into a window.\nPress F2 while the app is running to see a\nlist of available devices and modes." );
	static const TCHAR NoDesktopHal[] = _T( "Your Direct3D hardware accelerator cannot\nrender into a window with the current\ndesktop display settings.\n" );
	static const TCHAR NoHalThisMode[] = _T( "This game requires functionality that is\nnot available on your Direct3D hardware\naccelerator with the current desktop display\nsettings.\n" );
	static const TCHAR MediaNotFound[] = _T( "Could not load required media." );
	static const TCHAR ResetFailed[] = _T( "Could not reset the Direct3D device." );
	static const TCHAR NonZeroRefCount[] = _T( "A D3D object has a non-zero reference\ncount (meaning things were not properly\ncleaned up)." );
	static const TCHAR NullRefDevice[] = _T( "Warning: Nothing will be rendered.\nThe reference rendering device was selected, but your\ncomputer only has a reduced-functionality reference device\ninstalled.\n" );
	static const TCHAR OutOfMemory[] = _T( "Not enough memory." );
	static const TCHAR OutOfVideoMemory[] = _T( "Not enough video memory." );
	static const TCHAR InternalDriverError[] =  _T( "A serious problem occured inside the display driver." );
	static const TCHAR DefaultError[] = _T( "Application Error" );
		

	TCHAR strMsg[512];

	// If a fatal error message has already been reported, the app
	// is already shutting down, so don't show more error messages.
	if ( FatalErrorReported )
	{
		return hr;
	}

	switch ( hr )
	{
	case D3DAPPERR_NODIRECT3D:
		_tcscpy( strMsg, NoDirect3D );
		break;

	case D3DAPPERR_NOCOMPATIBLEDEVICES:
		_tcscpy( strMsg, NoCompatibleDevices );
		break;

	case D3DAPPERR_NOWINDOWABLEDEVICES:
		_tcscpy( strMsg, NoWindowableDevices );
		break;

	case D3DAPPERR_NOHARDWAREDEVICE:
		_tcscpy( strMsg, NoHardwareDevice );
		break;

	case D3DAPPERR_HALNOTCOMPATIBLE:
		_tcscpy( strMsg, HalNotCompatible );
		break;

	case D3DAPPERR_NOWINDOWEDHAL:
		_tcscpy( strMsg, NoWindowedHal );
		break;

	case D3DAPPERR_NODESKTOPHAL:
		_tcscpy( strMsg, NoDesktopHal );
		break;

	case D3DAPPERR_NOHALTHISMODE:
		_tcscpy( strMsg, NoHalThisMode );
		break;

	case D3DAPPERR_MEDIANOTFOUND:
	case HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ):
		_tcscpy( strMsg, MediaNotFound );
		break;

	case D3DAPPERR_RESETFAILED:
		_tcscpy( strMsg, ResetFailed );		
		break;

	case D3DAPPERR_NONZEROREFCOUNT:
		_tcscpy( strMsg, NonZeroRefCount );		
		break;

	case D3DAPPERR_NULLREFDEVICE:
		_tcscpy( strMsg, NullRefDevice );
		break;

	case E_OUTOFMEMORY:
		_tcscpy( strMsg, OutOfMemory );		
		break;

	case D3DERR_OUTOFVIDEOMEMORY:
		_tcscpy( strMsg, OutOfVideoMemory );		
		break;

	case D3DERR_DRIVERINTERNALERROR:
		_tcscpy( strMsg, InternalDriverError );				  
		dwType = MSGERR_APPMUSTEXIT;
		break;

	default:
		_tcscpy( strMsg, DefaultError );
	}

	if ( MSGERR_APPMUSTEXIT == dwType )
	{
		FatalErrorReported = true;
		_tcscat( strMsg, _T( "\n\nThis game will now exit." ) );
		::MessageBox( NULL, strMsg, m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
		hr = E_FAIL;
	}
	else
	{
		FatalErrorReported = true;
		_tcscat( strMsg, _T( "\n\nThis game will now exit." ) );
		::MessageBox( NULL, strMsg, m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			::SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
		hr = E_FAIL;
	}

	return hr;
}

}
}