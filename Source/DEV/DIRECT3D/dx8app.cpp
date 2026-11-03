/*
	dx8app.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	CHANGE HISTORY

	8/3/2004	-	Added functionality for disallowing creation
					of d3d device with FSAA support, and detection
					of FSAA switching capability from the caps,
					for cards that cannot switch FSAA on the fly
*/

#define STRICT
#include <string>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <D3D8.h>

using namespace std;

#include "../UTILITIES/vistafileutilities.h"
#include "../RESOURCES/resources.h"
#include "d3dEnumeration.h"
#include "d3dsettings.h"
#include "dx8app.h"
#include "getdxver.h"


static std::string		g_SemaphoreName	= "FateTheGame";

// Global access to the app (needed for the global WndProc())

static CDX8App*	g_pD3DApp	= NULL;

// Static msg handler which passes messages to the application class.
LRESULT CALLBACK WndProc( HWND hWnd, 
						  UINT uMsg, 
						  WPARAM wParam, 
						  LPARAM lParam )
{
	return g_pD3DApp->MsgProc( hWnd, uMsg, wParam, lParam );
} // WndProc()


// constructor for the d3d app
CDX8App::CDX8App( void ) :
					m_ShuttingDown( kFalse ),
					m_Windowed( kTrue ),
					m_DeviceLost( kFalse ),
					m_Minimized( kFalse ),
					m_Maximized( kFalse ),
					m_IgnoreSizeChange( kFalse ),
					m_DeviceObjectsInitialized( kFalse ),
					m_DeviceObjectsRestored( kFalse ),
					m_FullscreenFSAA( kFalse ),
					m_WindowedFSAA( kFalse ),
					m_SupportsFullscreenFSAA( kFalse ),
					m_SupportsWindowedFSAA( kFalse ),
					m_StartFullscreen( kFalse ),
					m_Active( kFalse ),
					m_ShowCursor( kFalse ),
					m_Focused( kFalse ),
					m_AllowFSAA( kTrue ),
					m_CanSwitchFSAA( kFalse ),
					m_RequiresVSYNC( kFalse ),
					m_IsVSYNCed( kFalse ),
					m_CanStartFullscreen( kTrue ),
					m_WindowTitle( "GAME" ),
					m_FPS( 0 ),
					m_WindowWidth( 640 ),
					m_WindowHeight( 480 ),
					m_RenderFailures( 0 ),
					m_pD3D( NULL ),
					m_pD3DDevice( NULL ),
					m_hWnd( NULL ),
					m_hMenu( NULL ),
					m_pEnumeration( NULL ),
					m_Semaphore( NULL ),
					m_WindowStyle( 0 ),
					m_ClipCursorWhenFullscreen ( kFalse )
{
	ZeroMemory ( &m_D3DSettings,		sizeof ( m_D3DSettings ));
	ZeroMemory ( &m_D3Dpp,				sizeof ( m_D3Dpp ));
	ZeroMemory ( &m_D3DCaps,			sizeof ( m_D3DCaps ));
	ZeroMemory ( &m_BackBufferSurface,	sizeof ( m_BackBufferSurface ));
	ZeroMemory ( &m_WindowBounds,		sizeof ( m_WindowBounds ));
	ZeroMemory ( &m_WindowClient,		sizeof ( m_WindowClient ));

	g_pD3DApp = this;

	m_FrameStats[0] = _T( '\0' );

	// When m_ClipCursorWhenFullscreen is kTrue, the cursor is limited to
	// the device window when the app goes fullscreen.  This prevents users
	// from accidentally clicking outside the app window on a multimon system.
	// This flag is turned off by default for debug builds, since it makes 
	// multimon debugging difficult.
#if defined(_DEBUG) || defined(DEBUG)
	m_ClipCursorWhenFullscreen = kFalse;
#else
	m_ClipCursorWhenFullscreen = kTrue;
#endif
} // CDX8App::CDX8App()

void	CDX8App::SetStartupResolution( uint32 Width,		// screen width
									   uint32 Height )		// screen height
{
	m_WindowWidth = Width;
	m_WindowHeight = Height;
} // CDX8App::SetStartupResolution()

// creates the window and d3d objects
HRESULT CDX8App::Create( HINSTANCE hInstance )	// HInstance
{
	HRESULT	hr;


	OSVERSIONINFO	OSVersion ;
	ZeroMemory( &OSVersion, sizeof( OSVERSIONINFO ) );
	OSVersion.dwOSVersionInfoSize = sizeof( OSVersion );

	GetVersionEx( &OSVersion );
	bool IsWindowsXPorLater = ( ( OSVersion.dwMajorVersion > 5 ) || ((OSVersion.dwMajorVersion == 5) && (OSVersion.dwMinorVersion >= 1 )) );



	DWORD dwDirectXVersion = 0;
    TCHAR strDirectXVersion[10];

    hr = GetDXVersion( &dwDirectXVersion, strDirectXVersion, 10 );
	//check in getDXVER for definition
	std::string LocalPath = "";

	if( dwDirectXVersion < 589824 && !IsWindowsXPorLater )
	{
		m_CanStartFullscreen = kFalse;
	}

	// create a semaphore for the game - this keeps other copies from running!
	// each d3dapplication will make this check, and if the semaphore already exists
	// it will exit immediately.
	// the semaphore is released when this application closes.
	m_Semaphore = CreateSemaphore( NULL, 1, 1, g_SemaphoreName.c_str() );
	if ( WaitForSingleObject( m_Semaphore, 0 ) == WAIT_TIMEOUT )
	{
		CloseHandle( m_Semaphore );
		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
		return E_FAIL;
	} 

	// find the desktop color depth -
	// if it isn't 16 or 32 bit color, we're going to exit too - no hardware acceleration
	uint32 BPP = GetDesktopBPP( m_hWnd );
	if ( ( BPP < 16 || BPP == 24 ) )
	{
		MessageBox( NULL,
					"This game must be run in 16- or 32-bit color mode. Please change your desktop color depth and try again.", // 101
					m_WindowTitle.c_str(),
					MB_ICONERROR |
					MB_OK );

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}

		return E_FAIL;
	}

	try
	{

		/* m_pD3D = Direct3DCreate8( D3D_SDK_VERSION ); */

		// Create the Direct3D object
		// regardless of SDK version, we want an 8.0 direct3d object
		// for widest support
		m_pD3D = Direct3DCreate8( 120 );

		// if we didn't succeed, shut down
		if ( m_pD3D == NULL )
		{
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		}

		// check to see if this card is happy starting fullscreen - 
		// some aren't!
		// retrieve the adapter's name, for any hacks we may need to do
		D3DADAPTER_IDENTIFIER8 AdapterIdentifier;
		
		m_pD3D->GetAdapterIdentifier( D3DADAPTER_DEFAULT,
									0,
									&AdapterIdentifier );

		std::string AdapterName = AdapterIdentifier.Description;

		// put it in uppercase for easier compare later
		AdapterName = StringUpper( AdapterName );

		if( StringUpper( AdapterName ).find( "MATROX MILLENNIUM G400" ) != -1 )
		{
			m_CanStartFullscreen = kFalse;
		}
		if( StringUpper( AdapterName ).find( "DIAMOND VIPER II Z200" ) != -1 )
		{
			m_CanStartFullscreen = kFalse;
		}
		if( StringUpper( AdapterName ).find( "3DFX VOODOO" ) != -1 )
		{
			m_CanStartFullscreen = kFalse;
		}
		if( StringUpper( AdapterName ).find( "VOODOO3" ) != -1 )
		{
			m_CanStartFullscreen = kFalse;
		}

		if( !m_CanStartFullscreen )
		{
			m_StartFullscreen = kFalse;
		}

		// create the enumerator - it will find out what devices and device combos are
		// supported on this machine
		m_pEnumeration = new CD3DEnumeration( m_pD3D );

		// perform the enumeration
		if ( FAILED( hr = m_pEnumeration->Enumerate() ) )
		{
			// clean up if it didn't work
			RELEASE_SAFELY( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}

		// Unless a substitute hWnd has been specified, create a window to
		// render into
		if ( m_hWnd == NULL )
		{
			// Register the windows class
			WNDCLASS	wndClass	=
			{
				CS_DBLCLKS,
				WndProc,
				0,
				0,
				hInstance,
				LoadIcon( hInstance, MAKEINTRESOURCE( IDI_MAIN_ICON ) ),
				LoadCursor( NULL, IDC_ARROW ),
				( HBRUSH )GetStockObject( WHITE_BRUSH ),
				NULL,
				m_WindowTitle.c_str()
			};
			RegisterClass( &wndClass );

			// Set the window's initial style
			m_WindowStyle = WS_OVERLAPPED |
							WS_CAPTION |
							WS_SYSMENU |
							WS_MINIMIZEBOX |
							WS_VISIBLE |
							WS_DLGFRAME ;

			// Set the window's initial width
			RECT	WindowRect;
			SetRect( &WindowRect, 0, 0, m_WindowWidth, m_WindowHeight );
			AdjustWindowRect( &WindowRect, m_WindowStyle, kFalse );

			// Create the window
			m_hWnd = CreateWindow( m_WindowTitle.c_str(),
								m_WindowTitle.c_str(),
								m_WindowStyle,
								CW_USEDEFAULT,
								CW_USEDEFAULT,
								( WindowRect.right - WindowRect.left ),
								( WindowRect.bottom - WindowRect.top ),
								0,
								LoadMenu( hInstance, MAKEINTRESOURCE( IDR_MENU ) ),
								hInstance,
								0 );
		}


		// Save window properties for use when we change resolutions, etc. later
		m_WindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
		GetWindowRect( m_hWnd, &m_WindowBounds );
		GetClientRect( m_hWnd, &m_WindowClient );


		// find devices for windowed and fullscreen that match our requested resolution
		if ( FAILED( hr = ChooseInitialD3DSettings() ) )
		{
			RELEASE_SAFELY( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}


		// Initialize any application specific data ( OneTimeSceneInit should be overridden )
		if ( FAILED( hr = OneTimeSceneInit() ) )
		{
			RELEASE_SAFELY( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}

		// Initialize the 3D environment for the app
		if ( FAILED( hr = Initialize3DEnvironment() ) )
		{
			RELEASE_SAFELY( m_pD3D );
			return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
		}


		m_Active = kTrue;
	}
	catch(...)
	{
		if( Fullscreen() )
		{
			ToggleFullscreen();
		}
		Cleanup3DEnvironment();
		RELEASE_SAFELY( m_pD3D );

		DELETE_SAFELY( m_pEnumeration );

		if( dwDirectXVersion < 589824 )
		{
			MessageBox( NULL, "\nTo play this game, you need DirectX 9.0 or higher.\nDownload a new version of DirectX from http://www.microsoft.com/directx and launch the game again.", "DirectX Upgrade", MB_OK | MB_ICONINFORMATION ); //ID 102
			// Close the window, which shuts down the app
			if ( m_hWnd )
			{
				SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
			}
			return E_FAIL;
		}
		else
		{
			return DisplayErrorMsg( D3DAPPERR_NODIRECT3D, MSGERR_APPMUSTEXIT );
		}
	}

	if ( !m_ShowCursor )
	{
		SetCursor( NULL );
	}
	else
	{
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	}

	return S_OK;
} // CDX8App::Create()

void CDX8App::SetWindowTitle( const std::string& Title )	// title to set for window	
{	 
	m_WindowTitle = Title;	
	if( m_hWnd != NULL )
	{
		::SetWindowText( m_hWnd, m_WindowTitle.c_str() );
	}
} // CDX8App::SetWindowTitle()


// assemble the settings for fullscreen and windowed modes given what
// we know about the hardware from our enumeration
// we will use these settings when the device is created
HRESULT CDX8App::ChooseInitialD3DSettings( void )
{
	bool	FoundFullscreen		= FindBestFullscreenMode( );
	bool	FoundWindowed		= FindBestWindowedMode( );

	if ( m_StartFullscreen && FoundFullscreen )
	{
		m_D3DSettings.m_IsWindowed = kFalse;
	}
	//  if there is no acceptable windowed mode, we can only do fullscreen
	if ( !FoundWindowed && FoundFullscreen )
	{
		m_D3DSettings.m_IsWindowed = kFalse;
	}

	// if we didn't find anything at all
	if ( ( !FoundFullscreen && !FoundWindowed ) ||
		 ( !m_StartFullscreen && !FoundWindowed ) )
	{
		// and we're still looking for a device with stencil capability
		if ( m_pEnumeration->MinStencilBits() > 0 )
		{
			// try enumerating for devices with no stencil requirements -
			// no stencil shadows for us!
			m_pEnumeration->SetMinStencilBits( 0 );
			m_pEnumeration->Enumerate();
			return ChooseInitialD3DSettings();
		}
		return D3DAPPERR_NOCOMPATIBLEDEVICES;
	}

	return S_OK;
} // CDX8App::ChooseInitialD3DSettings()

// choose the best available display mode for windowed
bool CDX8App::FindBestWindowedMode( void )
{
	// Get display mode of primary adapter (which is assumed to be where the window 
	// will appear)

	D3DDISPLAYMODE	primaryDesktopDisplayMode;
	m_pD3D->GetAdapterDisplayMode( 0, &primaryDesktopDisplayMode );

	D3DAdapterInfo*	pBestAdapterInfo = NULL;
	D3DDeviceInfo*	pBestDeviceInfo = NULL;
	D3DDeviceCombo*	pBestDeviceCombo = NULL;



	// go through all the adapters
	for ( UINT CurrentAdapter = 0;
		  CurrentAdapter < m_pEnumeration->GetAdapterList().size();
		  CurrentAdapter++ )
	{
		D3DAdapterInfo*	pAdapterInfo	=  m_pEnumeration->GetAdapterList()[CurrentAdapter];
		// and through all the devices for each adapter
		for ( UINT CurrentDeviceInfo = 0;
			  CurrentDeviceInfo < pAdapterInfo->m_pDeviceInfoList.size();
			  CurrentDeviceInfo++ )
		{
			D3DDeviceInfo*	pDeviceInfo	= pAdapterInfo->m_pDeviceInfoList[CurrentDeviceInfo];
			// if it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->m_DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// go through all the device combos
			for ( UINT CurrentDeviceCombo = 0;
				  CurrentDeviceCombo < pDeviceInfo->m_pDeviceComboList.size();
				  CurrentDeviceCombo++ )
			{
				D3DDeviceCombo*	pDeviceCombo		= pDeviceInfo->m_pDeviceComboList[CurrentDeviceCombo];
				// see if the backbuffer matches the adapter format
				bool AdapterMatchesBB	= ( pDeviceCombo->m_BackBufferFormat ==
											pDeviceCombo->m_AdapterFormat );
				// not windowed, we don't care
				if ( !pDeviceCombo->m_IsWindowed )
				{
					continue;
				}

				// if it doesn't match the desktop, we can't use it
				if ( pDeviceCombo->m_AdapterFormat !=
					 primaryDesktopDisplayMode.Format )
				{
					continue;
				}
				// If we haven't found a compatible DeviceCombo yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->m_DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesBB ) )
				{
					// keep this as our best combination

					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;
					// if this is exactly what we want, finish now
					if ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
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
	// if no device was found, we're done
	if ( pBestDeviceCombo == NULL )
	{
		return kFalse;
	}

	// update our settings to match this device
	m_D3DSettings.m_pWindowed_AdapterInfo = pBestAdapterInfo;
	m_D3DSettings.m_pWindowed_DeviceInfo = pBestDeviceInfo;
	m_D3DSettings.m_pWindowed_DeviceCombo = pBestDeviceCombo;
	m_D3DSettings.m_IsWindowed = kTrue;
	m_D3DSettings.m_Windowed_DisplayMode = primaryDesktopDisplayMode;
	m_D3DSettings.m_Windowed_Width = m_WindowClient.right - m_WindowClient.left;
	m_D3DSettings.m_Windowed_Height = m_WindowClient.bottom - m_WindowClient.top;

	m_D3DSettings.m_Windowed_DepthStencilBufferFormat = pBestDeviceCombo->m_pDepthStencilFormatList[0];


	m_D3DSettings.m_Windowed_MultisampleType = pBestDeviceCombo->m_pMultiSampleTypeList[0];

	m_WindowedFSAA = kFalse;
	m_CanSwitchFSAA = kTrue;
	if ( pBestDeviceInfo->m_Caps.RasterCaps & D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE )
	{
		m_CanSwitchFSAA = kFalse;
	}
	// go through the multisample types available for the device
	for ( uint32 i = 1; i < pBestDeviceCombo->m_pMultiSampleTypeList.size(); i++ )
	{
		// if it supports it, then we can do FSAA in this Windowed mode
		if ( pBestDeviceCombo->m_pMultiSampleTypeList[i] ==
			 D3DMULTISAMPLE_2_SAMPLES )
		{
			m_SupportsWindowedFSAA = kTrue;
			if( m_AllowFSAA )
			{
				m_WindowedFSAA = kTrue;
				m_D3DSettings.m_Windowed_MultisampleType = pBestDeviceCombo->m_pMultiSampleTypeList[i];
			}
			break;
		}
	}
	m_D3DSettings.m_Windowed_MultisampleQuality = 0;
	m_D3DSettings.m_Windowed_VertexProcessingType = pBestDeviceCombo->m_pVertexProcessingTypeList[0];
	m_D3DSettings.m_Windowed_PresentInterval = 0;	// present interval is always 0 for windowed

	return kTrue;
} //  CDX8App::FindBestWindowedMode()


bool CDX8App::FullscreenResolutionExists( uint32 Width,		// screen width
										  uint32 Height )	// screen height
{
	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE	adapterDesktopDisplayMode;
	D3DDISPLAYMODE	bestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestDisplayMode;

	bestAdapterDesktopDisplayMode.Width = 0;
	bestAdapterDesktopDisplayMode.Height = 0;
	bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	bestAdapterDesktopDisplayMode.RefreshRate = 0;

	D3DAdapterInfo*	pBestAdapterInfo = NULL;
	D3DDeviceInfo*	pBestDeviceInfo = NULL;
	D3DDeviceCombo*	pBestDeviceCombo = NULL;

	// go through all the adapters
	for ( UINT CurrentAdapter = 0;
		  CurrentAdapter < m_pEnumeration->GetAdapterList().size();
		  CurrentAdapter++ )
	{
		D3DAdapterInfo*	pAdapterInfo	= m_pEnumeration->GetAdapterList()[CurrentAdapter];
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->m_AdapterOrdinal,
									   &adapterDesktopDisplayMode );

		// and through all the devices for each adapter
		for ( UINT CurrentDeviceInfo = 0;
			  CurrentDeviceInfo < pAdapterInfo->m_pDeviceInfoList.size();
			  CurrentDeviceInfo++ )
		{
			D3DDeviceInfo*	pDeviceInfo	= pAdapterInfo->m_pDeviceInfoList[CurrentDeviceInfo];

			// if it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->m_DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// go through all the device combos
			for ( UINT CurrentDeviceCombo = 0;
				  CurrentDeviceCombo < pDeviceInfo->m_pDeviceComboList.size();
				  CurrentDeviceCombo++ )
			{
				D3DDeviceCombo*	pDeviceCombo			= pDeviceInfo->m_pDeviceComboList[CurrentDeviceCombo];
				// see if the backbuffer matches the adapter format
				bool AdapterMatchesBB		= ( pDeviceCombo->m_BackBufferFormat ==
												pDeviceCombo->m_AdapterFormat );

				// see if the adapter format matches the desktop
				bool AdapterMatchesDesktop	= ( pDeviceCombo->m_AdapterFormat ==
												adapterDesktopDisplayMode.Format );
				// if it is windowed, we don't want it
				if ( pDeviceCombo->m_IsWindowed )
				{
					continue;
				}
				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->m_DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   pBestDeviceCombo->m_AdapterFormat != adapterDesktopDisplayMode.Format &&
					   AdapterMatchesDesktop) || 
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesDesktop &&
					   AdapterMatchesBB ) )
				{
					// keep this as our best combination

					bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					if ( pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL &&
						 AdapterMatchesDesktop &&
						 AdapterMatchesBB )
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
		return kFalse;
	}

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's resolution as possible
	BestDisplayMode.Width = 0;
	BestDisplayMode.Height = 0;
	BestDisplayMode.Format = D3DFMT_UNKNOWN;
	BestDisplayMode.RefreshRate = 0;
	// go through all the available modes
	for ( UINT CurrentDisplayMode = 0; CurrentDisplayMode < pBestAdapterInfo->m_pDisplayModeList.size(); CurrentDisplayMode++ )
	{
		D3DDISPLAYMODE pDisplayMode	= pBestAdapterInfo->m_pDisplayModeList[CurrentDisplayMode];
		// if the mode's format is not the same as our device combo, we don't want it
		if ( pDisplayMode.Format != pBestDeviceCombo->m_AdapterFormat )
		{
			continue;
		}
		// if it exactly matches what we've requested, we're done
		if ( pDisplayMode.Width == m_WindowWidth &&
			 pDisplayMode.Height == m_WindowHeight &&
			 pDisplayMode.RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
		{
			// found a perfect match, so stop
			BestDisplayMode = pDisplayMode;
			break;
		}
		// if it matches the res, but has a better refresh, we'll take it too
		else if ( pDisplayMode.Width == m_WindowWidth &&
				  pDisplayMode.Height == m_WindowHeight &&
				  pDisplayMode.RefreshRate > BestDisplayMode.RefreshRate )
		{
			// refresh rate doesn't match, but width/height match, so keep this
			// and keep looking
			BestDisplayMode = pDisplayMode;
		}
		else if ( pDisplayMode.Width == m_WindowWidth )
		{
			// width matches, so keep this and keep looking
			BestDisplayMode = pDisplayMode;
		}
		else if ( BestDisplayMode.Width == 0 )
		{
			 	// we don't have anything better yet, so keep this and keep looking
			 	BestDisplayMode = pDisplayMode;
		}
	}
	if( BestDisplayMode.Width == Width &&
		BestDisplayMode.Height == Height )
	{
		return kTrue;
	}
	return kFalse;
} // CDX8App::FullscreenResolutionExists()

// choose the best available display mode for fullscreen
bool CDX8App::FindBestFullscreenMode( void )
{
	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE	adapterDesktopDisplayMode;
	D3DDISPLAYMODE	bestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestDisplayMode;

	bestAdapterDesktopDisplayMode.Width = 0;
	bestAdapterDesktopDisplayMode.Height = 0;
	bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	bestAdapterDesktopDisplayMode.RefreshRate = 0;

	D3DAdapterInfo*	pBestAdapterInfo = NULL;
	D3DDeviceInfo*	pBestDeviceInfo = NULL;
	D3DDeviceCombo*	pBestDeviceCombo = NULL;

	// go through all the adapters
	for ( UINT CurrentAdapter = 0;
		  CurrentAdapter < m_pEnumeration->GetAdapterList().size();
		  CurrentAdapter++ )
	{
		D3DAdapterInfo*	pAdapterInfo	= m_pEnumeration->GetAdapterList()[CurrentAdapter];
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->m_AdapterOrdinal,
									   &adapterDesktopDisplayMode );

		// and through all the devices for each adapter
		for ( UINT CurrentDeviceInfo = 0;
			  CurrentDeviceInfo < pAdapterInfo->m_pDeviceInfoList.size();
			  CurrentDeviceInfo++ )
		{
			D3DDeviceInfo*	pDeviceInfo	= pAdapterInfo->m_pDeviceInfoList[CurrentDeviceInfo];

			// if it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->m_DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// go through all the device combos
			for ( UINT CurrentDeviceCombo = 0;
				  CurrentDeviceCombo < pDeviceInfo->m_pDeviceComboList.size();
				  CurrentDeviceCombo++ )
			{
				D3DDeviceCombo*	pDeviceCombo			= pDeviceInfo->m_pDeviceComboList[CurrentDeviceCombo];
				// see if the backbuffer matches the adapter format
				bool AdapterMatchesBB		= ( pDeviceCombo->m_BackBufferFormat ==
												pDeviceCombo->m_AdapterFormat );

				// see if the adapter format matches the desktop
				bool AdapterMatchesDesktop	= ( pDeviceCombo->m_AdapterFormat ==
												adapterDesktopDisplayMode.Format );
				// if it is windowed, we don't want it
				if ( pDeviceCombo->m_IsWindowed )
				{
					continue;
				}
				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->m_DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   pBestDeviceCombo->m_AdapterFormat != adapterDesktopDisplayMode.Format &&
					   AdapterMatchesDesktop) || 
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesDesktop &&
					   AdapterMatchesBB ) )
				{
					// keep this as our best combination

					bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					if ( pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL &&
						 AdapterMatchesDesktop &&
						 AdapterMatchesBB )
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
		return kFalse;
	}

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's resolution as possible
	BestDisplayMode.Width = 0;
	BestDisplayMode.Height = 0;
	BestDisplayMode.Format = D3DFMT_UNKNOWN;
	BestDisplayMode.RefreshRate = 0;
	// go through all the available modes
	for ( UINT CurrentDisplayMode = 0; CurrentDisplayMode < pBestAdapterInfo->m_pDisplayModeList.size(); CurrentDisplayMode++ )
	{
		D3DDISPLAYMODE pDisplayMode	= pBestAdapterInfo->m_pDisplayModeList[CurrentDisplayMode];
		// if the mode's format is not the same as our device combo, we don't want it
		if ( pDisplayMode.Format != pBestDeviceCombo->m_AdapterFormat )
		{
			continue;
		}
		// if it exactly matches what we've requested, we're done
		if ( pDisplayMode.Width == m_WindowWidth &&
			 pDisplayMode.Height == m_WindowHeight &&
			 pDisplayMode.RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
		{
			// found a perfect match, so stop
			BestDisplayMode = pDisplayMode;
			break;
		}
		// if it matches the res, but has a better refresh, we'll take it too
		else if ( pDisplayMode.Width == m_WindowWidth &&
				  pDisplayMode.Height == m_WindowHeight &&
				  pDisplayMode.RefreshRate > BestDisplayMode.RefreshRate )
		{
			// refresh rate doesn't match, but width/height match, so keep this
			// and keep looking
			BestDisplayMode = pDisplayMode;
		}
		else if ( pDisplayMode.Width == m_WindowWidth )
		{
			// width matches, so keep this and keep looking
			BestDisplayMode = pDisplayMode;
		}
		else if ( BestDisplayMode.Width == 0 )
		{
			 	// we don't have anything better yet, so keep this and keep looking
			 	BestDisplayMode = pDisplayMode;
		}
	}

	// fill in our settings to match this adapter/device
	m_D3DSettings.m_pFullscreen_AdapterInfo = pBestAdapterInfo;
	m_D3DSettings.m_pFullscreen_DeviceInfo = pBestDeviceInfo;
	m_D3DSettings.m_pFullscreen_DeviceCombo = pBestDeviceCombo;
	m_D3DSettings.m_IsWindowed = kFalse;
	m_D3DSettings.m_Fullscreen_DisplayMode = BestDisplayMode;
	m_D3DSettings.m_Fullscreen_DepthStencilBufferFormat = pBestDeviceCombo->m_pDepthStencilFormatList[0];

	m_D3DSettings.m_Fullscreen_MultisampleType = pBestDeviceCombo->m_pMultiSampleTypeList[0];

	m_CanSwitchFSAA = kTrue;
	m_FullscreenFSAA = kFalse;
	if ( pBestDeviceInfo->m_Caps.RasterCaps & D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE )
	{
		m_CanSwitchFSAA = kFalse;
	}
	// go through the multisample types available for the device
	for ( uint32 i = 1; i < pBestDeviceCombo->m_pMultiSampleTypeList.size(); i++ )
	{
		// if it supports it, then we can do FSAA in this Windowed mode
		if ( pBestDeviceCombo->m_pMultiSampleTypeList[i] ==
			 D3DMULTISAMPLE_2_SAMPLES )
		{
			m_SupportsFullscreenFSAA = kTrue;
			if( m_AllowFSAA )
			{
				m_FullscreenFSAA = kTrue;
				m_D3DSettings.m_Fullscreen_MultisampleType = pBestDeviceCombo->m_pMultiSampleTypeList[i];
			}
			break;
		}
	}

	m_D3DSettings.m_Fullscreen_VertexProcessingType = pBestDeviceCombo->m_pVertexProcessingTypeList[0];
	m_D3DSettings.m_Fullscreen_PresentInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // we flip right away

	return kTrue;
} // CDX8App::FindBestFullscreenMode()

// find out if a given resolution is available
bool CDX8App::IsFullscreenResolutionAvailable( uint32 Width,	// screen width
											   uint32 Height )	// screen height
{
	bool ResolutionAvailable( kFalse );
	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE	adapterDesktopDisplayMode;
	D3DDISPLAYMODE	bestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE	BestDisplayMode;

	bestAdapterDesktopDisplayMode.Width = 0;
	bestAdapterDesktopDisplayMode.Height = 0;
	bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	bestAdapterDesktopDisplayMode.RefreshRate = 0;

	D3DAdapterInfo*	pBestAdapterInfo = NULL;
	D3DDeviceInfo*	pBestDeviceInfo = NULL;
	D3DDeviceCombo*	pBestDeviceCombo = NULL;

	// go through all the adapters
	for ( UINT CurrentAdapter = 0;
		  CurrentAdapter < m_pEnumeration->GetAdapterList().size();
		  CurrentAdapter++ )
	{
		D3DAdapterInfo*	pAdapterInfo	= m_pEnumeration->GetAdapterList()[CurrentAdapter];
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->m_AdapterOrdinal,
									   &adapterDesktopDisplayMode );

		// and through all the devices for each adapter
		for ( UINT CurrentDeviceInfo = 0;
			  CurrentDeviceInfo < pAdapterInfo->m_pDeviceInfoList.size();
			  CurrentDeviceInfo++ )
		{
			D3DDeviceInfo*	pDeviceInfo	= pAdapterInfo->m_pDeviceInfoList[CurrentDeviceInfo];

			// if it's not hardware accelerated, we don't want it!
			if ( pDeviceInfo->m_DeviceType != D3DDEVTYPE_HAL )
			{
				continue;
			}
			// go through all the device combos
			for ( UINT CurrentDeviceCombo = 0;
				  CurrentDeviceCombo < pDeviceInfo->m_pDeviceComboList.size();
				  CurrentDeviceCombo++ )
			{
				D3DDeviceCombo*	pDeviceCombo			= pDeviceInfo->m_pDeviceComboList[CurrentDeviceCombo];
				// see if the backbuffer matches the adapter format
				bool AdapterMatchesBB		= ( pDeviceCombo->m_BackBufferFormat ==
												pDeviceCombo->m_AdapterFormat );

				// see if the adapter format matches the desktop
				bool AdapterMatchesDesktop	= ( pDeviceCombo->m_AdapterFormat ==
												adapterDesktopDisplayMode.Format );
				// if it is windowed, we don't want it
				if ( pDeviceCombo->m_IsWindowed )
				{
					continue;
				}
				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if ( pBestDeviceCombo == NULL ||
					 ( pBestDeviceCombo->m_DeviceType != D3DDEVTYPE_HAL &&
					   pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL ) ||
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   pBestDeviceCombo->m_AdapterFormat != adapterDesktopDisplayMode.Format &&
					   AdapterMatchesDesktop) || 
					 ( pDeviceCombo->m_DeviceType == D3DDEVTYPE_HAL &&
					   AdapterMatchesDesktop &&
					   AdapterMatchesBB ) )
				{
					// keep this as our best combination

					bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;

					if ( pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL &&
						 AdapterMatchesDesktop &&
						 AdapterMatchesBB )
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
		return kFalse;
	}

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's resolution as possible
	BestDisplayMode.Width = 0;
	BestDisplayMode.Height = 0;
	BestDisplayMode.Format = D3DFMT_UNKNOWN;
	BestDisplayMode.RefreshRate = 0;
	// go through all the available modes
	for ( UINT CurrentDisplayMode = 0; CurrentDisplayMode < pBestAdapterInfo->m_pDisplayModeList.size(); CurrentDisplayMode++ )
	{
		D3DDISPLAYMODE pDisplayMode	= pBestAdapterInfo->m_pDisplayModeList[CurrentDisplayMode];
		// if the mode's format is not the same as our device combo, we don't want it
		if ( pDisplayMode.Format != pBestDeviceCombo->m_AdapterFormat )
		{
			continue;
		}
		// if it exactly matches what we've requested, we're done
		if ( pDisplayMode.Width == Width &&
			 pDisplayMode.Height == Height &&
			 pDisplayMode.RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
		{
		
			ResolutionAvailable = kTrue;
			break;
		}
		// if it matches the res, but has a better refresh, we'll take it too
		else if ( pDisplayMode.Width == Width &&
				  pDisplayMode.Height == Height &&
				  pDisplayMode.RefreshRate > BestDisplayMode.RefreshRate )
		{
			ResolutionAvailable = kTrue;
		}
		else if ( pDisplayMode.Width == Width )
		{
			// width matches, so keep this and keep looking
			BestDisplayMode = pDisplayMode;
		}
		else if ( BestDisplayMode.Width == 0 )
		{
			 	// we don't have anything better yet, so keep this and keep looking
			 	BestDisplayMode = pDisplayMode;
		}
	}


	return ResolutionAvailable;
} // CDX8App::IsFullscreenResolutionAvailable()

// handle messages as received
LRESULT CDX8App::MsgProc( HWND hWnd,			// the HWND
								  UINT uMsg,			// message to process
								  WPARAM wParam,		// param for the message
								  LPARAM lParam )		// param for the message
{
	switch ( uMsg )
	{
	case WM_PAINT:
		// we render on the paint
		if ( m_pD3DDevice &&
			 !m_Active &&
			 m_Windowed &&
			 m_DeviceObjectsInitialized &&
			 m_DeviceObjectsRestored &&
			 !m_ShuttingDown )
		{
			Render();
			m_pD3DDevice->Present( NULL, NULL, NULL, NULL );
		}
		break;

	case WM_ACTIVATEAPP:
		return 0;

	case WM_LBUTTONDOWN:
			LButtonDown();	// override this to receive the event
		return 0;

	case WM_LBUTTONUP:
			LButtonUp();	// override this to receive the event
		return 0;

	case WM_SIZE:
		// Pick up possible changes to window style due to maximize, etc.
		if ( m_Windowed && m_hWnd != NULL )
		{
			m_WindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
		}

		if ( SIZE_MINIMIZED == wParam )
		{
			// if we're in fullscreen, clip the cursor
			if ( m_ClipCursorWhenFullscreen && !m_Windowed )
			{
				ClipCursor( NULL );
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
			m_Minimized = kTrue;
			m_Maximized = kFalse;
			UnFocus();
		}
		else if ( SIZE_MAXIMIZED == wParam )
		{
			ShowWindow( m_hWnd, SW_SHOWNORMAL );
			m_Minimized = kFalse;
			m_Maximized = kTrue;
			Focus();
		}
		else if ( SIZE_RESTORED == wParam )
		{
			if ( m_Maximized )
			{
				m_Maximized = kFalse;
			}
			else if ( m_Minimized )
			{

				m_Minimized = kFalse;
				ShowWindow( m_hWnd, SW_SHOWNORMAL );
				Focus();
			}
		}
		break;

	case WM_SETCURSOR:
		// Turn off Windows cursor
		if ( m_Active && ( HTCLIENT == LOWORD( lParam ) ) )
		{
			// if we want a visible cursor invisible over our app, hide it here
			if( m_Windowed )
			{
				if ( !m_ShowCursor )
				{
					SetCursor( NULL );
				}
				else
				{
					//SetCursor( NULL );
					//m_pD3DDevice->ShowCursor( TRUE ); 
					return kTrue;
				}
			}
			else 
			{
				if ( !m_ShowCursor )
				{
					SetCursor( NULL );
				}
				else
				{
					//SetCursor( NULL );
					//m_pD3DDevice->ShowCursor( TRUE ); 
					return kTrue;
				}
			}
			return kFalse; // allow Windows to set cursor to window class cursor
		}
		break;

	case WM_KILLFOCUS:
		{
			UnFocus();// override this in your application to receive a lost focus event
			//  if we are in fullscreen then try to refocus!
			if ( !m_Windowed && !m_Minimized )
			{
				//m_IsVSYNCed = !m_RequiresVSYNC;
				//m_Minimized = kTrue;
			//	::SetFocus( m_hWnd ); 	
			}
		}
		break;
	case WM_SETFOCUS:
			Focus();	// override this in your application to receive a focus event
		break;

	case WM_MOUSEMOVE:
		if ( m_Active && m_pD3DDevice != NULL )
		{
			POINT	ptCursor;
			GetCursorPos( &ptCursor );
			// if fullscreen, bring the mouse cursor into client space
			if ( !m_Windowed )
			{
				ScreenToClient( m_hWnd, &ptCursor );
			}
			// set the mouse's cursor position in the d3ddevice
			m_pD3DDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0 );
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

	case WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case IDM_EXIT:
			// Recieved key/menu command to exit app
			if ( m_hWnd )
			{
				SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
			}
			return 0;
		}
		break;

	case WM_CLOSE:
		HMENU	hMenu;
		hMenu = GetMenu( hWnd );
		if ( hMenu != NULL )
		{
			DestroyMenu( hMenu );
		}

		// clear up our d3d stuff
		Cleanup3DEnvironment();
		RELEASE_SAFELY( m_pD3D );

		DELETE_SAFELY( m_pEnumeration );

		// do any application cleanup
		FinalCleanup();


		// release our semaphore so that another copy of the app can run later
		ReleaseSemaphore( m_Semaphore, 1, NULL );
		CloseHandle( m_Semaphore );

		// posting this quit message does the actual final shutdown
		PostQuitMessage( 0 );

		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );

} // CDX8App::MsgProc()



// initialize the d3d device based upon the settings we request
HRESULT CDX8App::Initialize3DEnvironment( void )
{
	HRESULT			hr;

	// get info about the adapter and device we want to use from our settings object
	D3DAdapterInfo*	pAdapterInfo	= m_D3DSettings.AdapterInfo();
	D3DDeviceInfo*	pDeviceInfo		= m_D3DSettings.DeviceInfo();

	m_Windowed = m_D3DSettings.m_IsWindowed;

	// Prepare window for possible windowed/fullscreen change
	AdjustWindowForChange();

	// Set up the presentation parameters
	BuildPresentParamsFromSettings();

	// if we somehow ended up with a ref device, error out
	if ( pDeviceInfo->m_Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE )
	{
		DisplayErrorMsg( D3DAPPERR_NULLREFDEVICE, 0 );
	}

	DWORD	BehaviorFlags;

	// set vertex processing flags
	if ( m_D3DSettings.GetVertexProcessingType() == KVPSoftware )
	{
		BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else if ( m_D3DSettings.GetVertexProcessingType() == KVPMixed )
	{
		BehaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
	}
	else if ( m_D3DSettings.GetVertexProcessingType() == KVPHardware )
	{
		BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}
	else if ( m_D3DSettings.GetVertexProcessingType() == KVPPureHardware )
	{
		BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING |
		 				D3DCREATE_PUREDEVICE;
	}
	else
	{
		BehaviorFlags = 0;
	}

	// Create the device with the presentation parameters we constructed
	hr = m_pD3D->CreateDevice( m_D3DSettings.AdapterOrdinal(),
							   pDeviceInfo->m_DeviceType,
							   m_hWnd,
							   BehaviorFlags,
							   &m_D3Dpp,
							   &m_pD3DDevice );

	// if it worked
	if ( SUCCEEDED( hr ) )
	{
		// set up the window for windowed mode
		if ( m_Windowed )
		{
			SetWindowPos( m_hWnd,
						  HWND_NOTOPMOST,
						  m_WindowBounds.left,
						  m_WindowBounds.top,
						  ( m_WindowBounds.right - m_WindowBounds.left ),
						  ( m_WindowBounds.bottom - m_WindowBounds.top ),
						  SWP_SHOWWINDOW );
		}

		// Store device Caps
		m_pD3DDevice->GetDeviceCaps( &m_D3DCaps );


		// Store backbuffer surface description
		LPDIRECT3DSURFACE8	pBackBuffer	= NULL;
		m_pD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
		pBackBuffer->GetDesc( &m_BackBufferSurface );
		pBackBuffer->Release();

		// Set up the cursor if visible
		if ( m_ShowCursor )
		{
			HCURSOR	hCursor;
#ifdef _WIN64
			hCursor = ( HCURSOR ) GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
			hCursor = ( HCURSOR )
					  ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
		}

		// Confine cursor to fullscreen window
		if ( m_ClipCursorWhenFullscreen )
		{
			if ( !m_Windowed )
			{
				RECT	rcWindow;
				GetWindowRect( m_hWnd, &rcWindow );
				ClipCursor( &rcWindow );
			}
			else
			{
				ClipCursor( NULL );
			}
		}

		// Initialize the app's device-dependent objects
		hr = InitDeviceObjects();



		if ( FAILED( hr ) )
		{
			DeleteDeviceObjects();
		}
		else
		{
			m_DeviceObjectsInitialized = kTrue;
			hr = RestoreDeviceObjects();

			if ( FAILED( hr ) )
			{
				InvalidateDeviceObjects();
			}
			else
			{
				m_DeviceObjectsRestored = kTrue;
				return S_OK;
			}
		}

		// Cleanup 
		Cleanup3DEnvironment();
	}

	// if that failed, we have no hardware device, and we have to quit
	if ( hr != D3DAPPERR_MEDIANOTFOUND &&
		 //hr != HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) &&
		 pDeviceInfo->m_DeviceType == D3DDEVTYPE_HAL )
	{
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );

	}
	return hr;
} // CDX8App::Initialize3DEnvironment()

// build our presentation parameters from the settings we assembled based upon our enumeration
// most of this is just copying from one structure to the other
// the presentation parameters are what we'll use to actually construct the device, though
void CDX8App::BuildPresentParamsFromSettings( void )
{
	ZeroMemory(&m_D3Dpp, sizeof(m_D3Dpp));
	m_D3Dpp.hDeviceWindow			= m_hWnd;

	m_D3Dpp.Windowed		= m_D3DSettings.m_IsWindowed;
	m_D3Dpp.BackBufferCount = 1;
	m_D3Dpp.MultiSampleType = m_D3DSettings.MultisampleType();
	m_D3Dpp.SwapEffect		= D3DSWAPEFFECT_DISCARD;

	if( m_RequiresVSYNC )
	{
		m_IsVSYNCed = kTrue;
	}
	else
	{
		m_IsVSYNCed = kFalse;
	}

	m_D3Dpp.EnableAutoDepthStencil	= kTrue;
	m_D3Dpp.AutoDepthStencilFormat = m_D3DSettings.DepthStencilBufferFormat();

	m_D3Dpp.Flags = 0;

	if ( m_Windowed )
	{
		m_D3Dpp.BackBufferWidth = m_WindowClient.right - m_WindowClient.left;
		m_D3Dpp.BackBufferHeight = m_WindowClient.bottom - m_WindowClient.top;
		m_D3Dpp.BackBufferFormat = m_D3DSettings.BackBufferFormat();
		m_D3Dpp.FullScreen_RefreshRateInHz = 0;
		m_D3Dpp.FullScreen_PresentationInterval = m_D3DSettings.PresentInterval();
	}
	else
	{
		m_D3Dpp.BackBufferWidth = m_D3DSettings.DisplayMode().Width;
		m_D3Dpp.BackBufferHeight = m_D3DSettings.DisplayMode().Height;
		m_D3Dpp.BackBufferFormat = m_D3DSettings.BackBufferFormat();
		m_D3Dpp.FullScreen_RefreshRateInHz = m_D3DSettings.m_Fullscreen_DisplayMode.RefreshRate;
		m_D3Dpp.FullScreen_PresentationInterval = m_D3DSettings.PresentInterval();
		
		if( m_IsVSYNCed &&
			m_D3DSettings.DeviceInfo()->m_Caps.RasterCaps & D3DPRESENT_INTERVAL_ONE )
		{
			m_D3Dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
	}
} // CDX8App::BuildPresentParamsFromSettings()

// reset the 3d device
HRESULT CDX8App::Reset3DEnvironment( void )
{
	HRESULT	hr;

	// Release all video memory objects
	if ( m_DeviceObjectsRestored )
	{
		m_DeviceObjectsRestored = kFalse;
		InvalidateDeviceObjects();
	}
	
	if ( ::GetActiveWindow() != m_hWnd || ::GetFocus() != m_hWnd || ::GetForegroundWindow() != m_hWnd )
	{
		ShowWindow( m_hWnd, SW_SHOWNORMAL );
	}

	// Reset the device
	if ( FAILED( hr = m_pD3DDevice->Reset( &m_D3Dpp ) ) )
	{
		return hr;
	}

	// Store backbuffer description again - it might have changed
	LPDIRECT3DSURFACE8	pBackBuffer;
	m_pD3DDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
	pBackBuffer->GetDesc( &m_BackBufferSurface );
	pBackBuffer->Release();

	// Set up the cursor if it should be visible
	if ( m_ShowCursor )
	{
		HCURSOR	hCursor;
#ifdef _WIN64
		hCursor = ( HCURSOR ) GetClassLongPtr( m_hWnd, GCLP_HCURSOR );
#else
		hCursor = ( HCURSOR )
				  ULongToHandle( GetClassLong( m_hWnd, GCL_HCURSOR ) );
#endif
		m_pD3DDevice->ShowCursor( kTrue );
	}

	// Confine cursor to fullscreen window
	if ( m_ClipCursorWhenFullscreen )
	{
		if ( !m_Windowed )
		{
			RECT	WindowRect;
			GetWindowRect( m_hWnd, &WindowRect );
			ClipCursor( &WindowRect );
		}
		else // don't clip in windowed
		{
			ClipCursor( NULL );
		}
	}

	// Initialize the app's device-dependent objects
	hr = RestoreDeviceObjects();
	// if we couldn't restore the objects, invalidate the m and fail
	if ( FAILED( hr ) )
	{
		MessageBox( NULL,
					"FAILED RESTORE",
					"Load Error",
					MB_ICONERROR | MB_OK );
		InvalidateDeviceObjects();
		return hr;
	}
	m_DeviceObjectsRestored = kTrue;

	return S_OK;
} // CDX8App::Reset3DEnvironment()


// toggle fullscreen/windowed state
HRESULT CDX8App::ToggleFullscreen( void )
{
	HRESULT		hr;
	uint32		AdapterOrdinalOld	= m_D3DSettings.AdapterOrdinal();
	D3DDEVTYPE	DevTypeOld			= m_D3DSettings.DeviceType();


	m_Active = kFalse;
	// we're in the middle of changing the device over, so we make sure to let
	// other events know not to fiddle with us right now
	m_IgnoreSizeChange = kTrue;

	// Toggle the windowed state
	m_Windowed = !m_Windowed;
	// keep our settings up to date
	m_D3DSettings.m_IsWindowed = m_Windowed;

	m_StartFullscreen = !m_Windowed;
	// Prepare window for windowed/fullscreen change
	AdjustWindowForChange();

	// if we're switching to the same adapter and the same device, we can just reset,
	// and don't have to rebuild everything from scratch
	if ( m_D3DSettings.AdapterOrdinal() == AdapterOrdinalOld &&
		 m_D3DSettings.DeviceType() == DevTypeOld )
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
		// otherwise we gotta redo everything
		Cleanup3DEnvironment();
		hr = Initialize3DEnvironment();
	}


	if ( FAILED( hr ) )
	{
		// out of video memory
		if( hr != D3DERR_OUTOFVIDEOMEMORY )
		{
			hr = D3DAPPERR_RESETFAILED;
		}
		// we've got to let things adjust again
		m_IgnoreSizeChange = kFalse;
		// if we're in fullscreen, get out now - we obviously didn't succeed
		if ( !m_Windowed )
		{
			// Restore window type to windowed mode
			m_Windowed = !m_Windowed;
			m_D3DSettings.m_IsWindowed = m_Windowed;
			AdjustWindowForChange();
			SetWindowPos( m_hWnd,
						  HWND_NOTOPMOST,
						  m_WindowBounds.left,
						  m_WindowBounds.top,
						  ( m_WindowBounds.right - m_WindowBounds.left ),
						  ( m_WindowBounds.bottom - m_WindowBounds.top ),
						  SWP_SHOWWINDOW );
		}
		// we'll have to close now
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
	}

	// we've got to let things adjust again
	m_IgnoreSizeChange = kFalse;

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
		// heading back to windowed mode, so set up the rect and adjust the window
		RECT	WindowRect;
		SetRect( &WindowRect, 0, 0, m_WindowWidth, m_WindowHeight );
		AdjustWindowRect( &WindowRect, m_WindowStyle, kFalse );

		// Save window properties
		m_WindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
		GetWindowRect( m_hWnd, &m_WindowBounds );
		GetClientRect( m_hWnd, &m_WindowClient );
		// make sure the client and bounds rects are OK
		m_WindowClient.right = m_WindowClient.left + m_WindowWidth;
		m_WindowClient.bottom = m_WindowClient.top + m_WindowHeight;

		m_WindowBounds.right	= m_WindowBounds.left +
								  ( WindowRect.right - WindowRect.left );
		m_WindowBounds.bottom	= m_WindowBounds.top +
								  ( WindowRect.bottom - WindowRect.top );

		// reposition the window
		SetWindowPos( m_hWnd,
					  HWND_NOTOPMOST,
					  m_WindowBounds.left,
					  m_WindowBounds.top,
					  ( m_WindowBounds.right - m_WindowBounds.left ),
					  ( m_WindowBounds.bottom - m_WindowBounds.top ),
					  SWP_SHOWWINDOW );
	}

	// refocus
	::SetFocus( m_hWnd );
	m_Active = kTrue;

	return S_OK;
}

// perform a resolution change from one size to another - NOT THE SAME AS TOGGLING FULLSCREEN!
HRESULT CDX8App::ChangeResolution( uint32 NewWidth,			// width to change to
								   uint32 NewHeight )		// height to change to
{
	HRESULT		hr;
	uint32			AdapterOrdinalOld	= m_D3DSettings.AdapterOrdinal();
	D3DDEVTYPE		DevTypeOld			= m_D3DSettings.DeviceType();

	m_WindowWidth = NewWidth;
	m_WindowHeight = NewHeight;

	// if we're in windowed
	if ( m_Windowed )
	{
		// Set the window's new rect size
		RECT	WindowRect;
		SetRect( &WindowRect, 0, 0, m_WindowWidth, m_WindowHeight );
		AdjustWindowRect( &WindowRect, m_WindowStyle, kFalse );

		// Save window properties
		m_WindowStyle = GetWindowLong( m_hWnd, GWL_STYLE );
		GetWindowRect( m_hWnd, &m_WindowBounds );
		GetClientRect( m_hWnd, &m_WindowClient );

		// make sure the client and bounds are ok
		m_WindowClient.right	= m_WindowClient.left + m_WindowWidth;
		m_WindowClient.bottom	= m_WindowClient.top + m_WindowHeight;

		m_WindowBounds.right	= m_WindowBounds.left +
								  ( WindowRect.right - WindowRect.left );
		m_WindowBounds.bottom	= m_WindowBounds.top +
								  ( WindowRect.bottom - WindowRect.top );
	}

	// Prepare window for change
	AdjustWindowForChange();

	// if we're switching to the same adapter and the same device, we can just reset,
	// and don't have to rebuild everything from scratch
	if ( m_D3DSettings.AdapterOrdinal() == AdapterOrdinalOld &&
		 m_D3DSettings.DeviceType() == DevTypeOld )
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
		// otherwise we gotta start over
		Cleanup3DEnvironment();
		hr = Initialize3DEnvironment();
	}

	// couldn't change resolution
	if ( FAILED( hr ) )
	{
		if ( hr != D3DERR_OUTOFVIDEOMEMORY )
		{
			hr = D3DAPPERR_RESETFAILED;
		}
		// if we're in fullscreen, drop back to desktop
		if ( !m_Windowed )
		{
			// Restore window type to windowed mode
			m_Windowed = !m_Windowed;
			m_D3DSettings.m_IsWindowed = m_Windowed;
			AdjustWindowForChange();
			SetWindowPos( m_hWnd,
						HWND_NOTOPMOST,
						m_WindowBounds.left,
						m_WindowBounds.top,
						( m_WindowBounds.right - m_WindowBounds.left ),
						( m_WindowBounds.bottom - m_WindowBounds.top ),
						SWP_SHOWWINDOW );
		}
		// and error out
		return DisplayErrorMsg( hr, MSGERR_APPMUSTEXIT );
	}


	// reposition the window in windowed mode
	if ( m_Windowed )
	{
		SetWindowPos( m_hWnd,
					  HWND_NOTOPMOST,
					  m_WindowBounds.left,
					  m_WindowBounds.top,
					  ( m_WindowBounds.right - m_WindowBounds.left ),
					  ( m_WindowBounds.bottom - m_WindowBounds.top ),
					  SWP_SHOWWINDOW );
	}
	// refocus
	::SetFocus( m_hWnd );
	// handle any window widget changes that need to be done
	HandlePossibleSizeChange();
	return S_OK;
} // CDX8App::ChangeResolution()

// if the client area has changed, reset the device if required
HRESULT CDX8App::HandlePossibleSizeChange( void )
{
	HRESULT	hr	= S_OK;
	RECT	OldClientRect;
	OldClientRect = m_WindowClient;

	// if we're in the middle of an operation on the device, we can't do this 
	if ( m_IgnoreSizeChange )
	{
		return S_OK;
	}

	// Update window properties
	GetWindowRect( m_hWnd, &m_WindowBounds );
	GetClientRect( m_hWnd, &m_WindowClient );

	// if the size of the window has changed
	if ( OldClientRect.right - OldClientRect.left !=
		 m_WindowClient.right - m_WindowClient.left ||
		 OldClientRect.bottom - OldClientRect.top !=
		 m_WindowClient.bottom - m_WindowClient.top )
	{
		m_Active = kFalse;

		// A new window size will require a new backbuffer
		// size, so the 3D structures must be changed accordingly.
		m_D3Dpp.BackBufferWidth = m_WindowClient.right - m_WindowClient.left;
		m_D3Dpp.BackBufferHeight = m_WindowClient.bottom - m_WindowClient.top;

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

		m_Active = kTrue;
	}
	return hr;
} // CDX8App::HandlePossibleSizeChange()


// get ready for a resolution or windowed/fullscreen change
// basically just set up the menus and window properties so that
// things look correct for windowed/fullscreen
HRESULT CDX8App::AdjustWindowForChange( void )
{
	if ( m_Windowed )
	{
		// Set windowed-mode style
		SetWindowLong( m_hWnd, GWL_STYLE, m_WindowStyle );
		if ( m_hMenu != NULL )
		{
			SetMenu( m_hWnd, m_hMenu );
			m_hMenu = NULL;
		}
	}
	else
	{
		// Set fullscreen-mode style
		SetWindowLong( m_hWnd, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_VISIBLE );
		if ( m_hMenu == NULL )
		{
			m_hMenu = GetMenu( m_hWnd );
			SetMenu( m_hWnd, NULL );
		}
	}
	return S_OK;
} // CDX8App::AdjustWindowForChange()


// begins and maintains our 'render loop' until it receives a WM_QUIT message
int32 CDX8App::Run( void )
{


	// Now we're ready to recieve and process Windows messages.
	bool	GotMessage;
	MSG		Message;
	Message.message = WM_NULL;
	PeekMessage( &Message, NULL, 0U, 0U, PM_NOREMOVE );

	// we keep looping until we get a quit message
	while ( WM_QUIT != Message.message )
	{
		// Use PeekMessage() if the app is active, so we can use idle time to
		// render the scene. Else, use GetMessage() to avoid eating CPU time.
		if ( m_Active )
		{
			GotMessage = ( PeekMessage( &Message, NULL, 0U, 0U, PM_REMOVE ) != 0 );
		}


		// if we got a message
		if ( GotMessage )
		{
			// quit knocks us out
			if ( Message.message == WM_QUIT )
			{
				break;
			}
			
			// otherwise we ferry them on
			TranslateMessage( &Message );
			DispatchMessage( &Message );
			
		}
		else	// no message, so we do our business
		{
			// if we aren't in the process of shutting down
			if ( !m_ShuttingDown )
			{
				// if we have no device, we just sleep and let the CPU roll on
				if ( m_DeviceLost )
				{
					// Yield some CPU time to other processes
					Sleep( 100 ); 
				}
				// Render a frame during idle time (no messages are waiting)
				if ( m_Active )
				{
					// try to render everything
					if ( FAILED( Render3DEnvironment() ) )
					{
						m_RenderFailures++;
						if( m_RenderFailures > 2 )
						{
							m_ShuttingDown = kTrue;
							SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
						}
					}
					else
					{
						m_RenderFailures = 0;
					}
					if ( ShouldClose() ) // shouldclose can be overridden so that you can notify the game to exit
					{
						m_ShuttingDown = kTrue;
						SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
					}
				}
			}
		}
	}

	return ( int32 ) Message.wParam;
} // CDX8App::Run()



// render the scene
HRESULT CDX8App::Render3DEnvironment( void )
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
					D3DAdapterInfo*	pAdapterInfo	= m_D3DSettings.AdapterInfo();
					m_pD3D->GetAdapterDisplayMode( pAdapterInfo->m_AdapterOrdinal,
												   &m_D3DSettings.m_Windowed_DisplayMode );
					m_D3Dpp.BackBufferFormat = m_D3DSettings.m_Windowed_DisplayMode.Format;
				}

				// try to reset the device
				if ( FAILED( hr = Reset3DEnvironment() ) )
				{
					return hr;
				}
			}
			return hr;
		}
		m_DeviceLost = kFalse;
	}


	// Frame move the scene
	if ( FAILED( hr = FrameMove() ) )	// override frame move to perform app logic
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

	// calculate elapsed time
	static float32	LastTime	= 0.0f;
	static uint32	Frames		= 0;
	float32			Time		= (float32)Game_Timer( KTimerGetAbsoluteTime );
	++Frames;

	// Update the scene stats once per second
	if ( Time - LastTime > 1.0f )
	{
		m_FPS = Frames / ( Time - LastTime );
		LastTime = Time;
		Frames = 0;

		_stprintf( m_FrameStats, _T( "FPS : %.02f" ), m_FPS );
	}

	// Show the frame on the primary surface.
	hr = m_pD3DDevice->Present( NULL, NULL, NULL, NULL );
	// if we lost the device, we'll need to try to recreate it next go-round
	if ( D3DERR_DEVICELOST == hr )
	{
		m_DeviceLost = kTrue;
	}

	// update our pp if the vsync setting has changed
	if( !m_DeviceLost &&
		m_RequiresVSYNC != m_IsVSYNCed &&
		m_D3DSettings.DeviceInfo()->m_Caps.RasterCaps & D3DPRESENT_INTERVAL_ONE )
	{
		BuildPresentParamsFromSettings();
		// try to reset the device
		if ( FAILED( hr = Reset3DEnvironment() ) )
		{
			return hr;
		}
	}

	return S_OK;
} // CDX8App::Render3DEnvironment()

// clean up our d3d device and objects
void CDX8App::Cleanup3DEnvironment( void )
{
	if ( m_pD3DDevice != NULL )
	{
		if ( m_DeviceObjectsRestored )
		{
			m_DeviceObjectsRestored = kFalse;
			InvalidateDeviceObjects();
		}
		if ( m_DeviceObjectsInitialized )
		{
			m_DeviceObjectsInitialized = kFalse;
			DeleteDeviceObjects();
		}

		if( m_pD3DDevice->Release() > 0 )
		{
			DisplayErrorMsg( D3DAPPERR_NONZEROREFCOUNT, MSGERR_APPMUSTEXIT );
		}

		m_pD3DDevice = NULL;
	}
} // CDX8App::Cleanup3DEnvironment()




//-----------------------------------------------------------------------------
// Name: DisplayErrorMsg()
// Desc: Displays error messages in a message box
//-----------------------------------------------------------------------------
HRESULT CDX8App::DisplayErrorMsg( HRESULT hr, 
										  DWORD dwType )
{
	static bool	s_bFatalErrorReported	= kFalse;
	TCHAR		strMsg[512];

	// If a fatal error message has already been reported, the app
	// is already shutting down, so don't show more error messages.
	if ( s_bFatalErrorReported )
	{
		return hr;
	}

	switch ( hr )
	{
	case D3DAPPERR_NODIRECT3D:
		_tcscpy( strMsg,
				 _T( "Could not initialize Direct3D. You may\n" ) // 103-106
							 _T( "want to check that the latest version of\n" )
							 _T( "DirectX is correctly installed on your\n" )
							 _T( "system." ) );
		break;

	case D3DAPPERR_NOCOMPATIBLEDEVICES:
		_tcscpy( strMsg, _T( "Could not find any compatible Direct3D\n" ) // 107 and 108
							 _T( "devices." ) );
		break;

	case D3DAPPERR_NOWINDOWABLEDEVICES:
		_tcscpy( strMsg,
				 _T( "This game cannot run in a desktop\n" ) // 109-112
							 _T( "window with the current display settings.\n" )
							 _T( "Please change your desktop settings to a\n" )
							 _T( "16- or 32-bit display mode and re-run this\n" )
							 _T( "sample." ) );
		break;

	case D3DAPPERR_NOHARDWAREDEVICE:
		_tcscpy( strMsg, _T( "No hardware-accelerated Direct3D devices\n" ) // 112 and 113
							 _T( "were found." ) );
		break;

	case D3DAPPERR_HALNOTCOMPATIBLE:
		_tcscpy( strMsg, _T( "This game requires functionality that is\n" ) // 114-117
							 _T( "not available on your Direct3D hardware\n" )
							 _T( "accelerator." ) );
		break;

	case D3DAPPERR_NOWINDOWEDHAL:
		_tcscpy( strMsg,
				 _T( "Your Direct3D hardware accelerator cannot\n" ) // 118-121
							 _T( "render into a window.\n" )
							 _T( "Press F2 while the app is running to see a\n" )
							 _T( "list of available devices and modes." ) );
		break;

	case D3DAPPERR_NODESKTOPHAL:
		_tcscpy( strMsg, _T( "Your Direct3D hardware accelerator cannot\n" ) // 122-124
							 _T( "render into a window with the current\n" )
							 _T( "desktop display settings.\n" ) );
		break;

	case D3DAPPERR_NOHALTHISMODE:
		_tcscpy( strMsg,
				 _T( "This game requires functionality that is\n" ) // 125-128
							 _T( "not available on your Direct3D hardware\n" )
							 _T( "accelerator with the current desktop display\n" )
							 _T( "settings.\n" ) );
		break;

	case D3DAPPERR_MEDIANOTFOUND:
	case HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ):
		_tcscpy( strMsg, _T( "Could not load required media." ) ); // 129
		break;

	case D3DAPPERR_RESETFAILED:
		_tcscpy( strMsg, _T( "Could not reset the Direct3D device." ) ); // 130
		break;

	case D3DAPPERR_NONZEROREFCOUNT:
		_tcscpy( strMsg, _T( "A D3D object has a non-zero reference\n" ) // 131-133
							 _T( "count (meaning things were not properly\n" )
							 _T( "cleaned up)." ) );
		break;

	case D3DAPPERR_NULLREFDEVICE:
		_tcscpy( strMsg,
				 _T( "Warning: Nothing will be rendered.\n" ) // 134-137
							 _T( "The reference rendering device was selected, but your\n" )
							 _T( "computer only has a reduced-functionality reference device\n" )
							 _T( "installed.  \n" ) );
		break;

	case E_OUTOFMEMORY:
		_tcscpy( strMsg, _T( "Not enough memory." ) ); // 138
		break;

	case D3DERR_OUTOFVIDEOMEMORY:
		_tcscpy( strMsg, _T( "Not enough video memory." ) ); // 139
		break;

	case D3DERR_DRIVERINTERNALERROR:
		_tcscpy( strMsg,
				 _T( "A serious problem occured inside the display driver." ) ); // 140
		dwType = MSGERR_APPMUSTEXIT;
		break;

	default:
		_tcscpy( strMsg, _T( "Application Error" ) ); // 141
	}

	if ( MSGERR_APPMUSTEXIT == dwType )
	{
		s_bFatalErrorReported = kTrue;
		_tcscat( strMsg, _T( "\n\nThis game will now exit." ) ); // 142
		MessageBox( NULL, strMsg, m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
		hr = E_FAIL;
	}
	else
	{

		s_bFatalErrorReported = kTrue;
		_tcscat( strMsg, _T( "\n\nThis game will now exit." ) ); // 142 (repeat)
		MessageBox( NULL, strMsg, m_WindowTitle.c_str(), MB_ICONERROR | MB_OK );

		// Close the window, which shuts down the app
		if ( m_hWnd )
		{
			SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		}
		hr = E_FAIL;
	}

	return hr;
}
