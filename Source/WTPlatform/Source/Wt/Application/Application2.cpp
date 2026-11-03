//
//	Application2.cpp
//
//	Copyright (C) 2005-6 WildTangent, Inc. 
//	All Rights Reserved
//
#include "WT/stdafx.h"
#include "WT/System/Logging/Log.h"
#include "WT/System/Util/WinUtil.h"
#include "WT/Application/Application2.h"
#include "WT/Graphics/Material.h"
#include <strsafe.h>

namespace WT
{

using namespace System;

namespace Application2
{

#define _MIN_WINDOW_SIZE_X 200
#define _MIN_WINDOW_SIZE_Y 200

#define _ERR(str,hr)           (hr)
#define _ERR_MSGBOX(str,hr)    (hr)

IDirect3D9 * WINAPI _Dynamic_Direct3DCreate9(UINT SDKVersion);
int WINAPI _Dynamic_D3DPERF_BeginEvent( D3DCOLOR col, LPCSTR wszName );
int WINAPI _Dynamic_D3DPERF_EndEvent( void );
void WINAPI _Dynamic_D3DPERF_SetMarker( D3DCOLOR col, LPCSTR wszName );
void WINAPI _Dynamic_D3DPERF_SetRegion( D3DCOLOR col, LPCSTR wszName );
BOOL WINAPI _Dynamic_D3DPERF_QueryRepeatFrame( void );
void WINAPI _Dynamic_D3DPERF_SetOptions( DWORD dwOptions );
DWORD WINAPI _Dynamic_D3DPERF_GetStatus( void );
LPCSTR D3DFormatToString( D3DFORMAT format, bool bWithPrefix );
HMONITOR DynamicMonitorFromWindow( HWND hWnd, DWORD dwFlags );
BOOL     DynamicGetMonitorInfo( HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo );

//--------------------------------------------------------------------------------------
// Internal functions forward declarations
//--------------------------------------------------------------------------------------

typedef DECLSPEC_IMPORT UINT (WINAPI* LPTIMEBEGINPERIOD)( UINT uPeriod );

/*static*/ CApplication* CApplication::gm_pInstance = NULL;	

CApplication::CApplication()
{
	gm_pInstance = this;
	CreateState();
}

CApplication::~CApplication()
{
	DestroyState();
	// Invalidate instance pointer
	if (gm_pInstance == this)
	{
		gm_pInstance = NULL;
	}
}

void CApplication::CreateState()
{
    m_D3D = NULL;                     // the main D3D object

    m_D3DDevice = NULL;               // the D3D rendering device
    m_D3DEnumeration = NULL;          // CDeviceEnumeration object

    m_CurrentDeviceSettings = NULL;   // current device settings
    ZeroMemory ( &m_BackBufferSurfaceDesc, sizeof ( m_BackBufferSurfaceDesc));   // back buffer surface description
    ZeroMemory ( &m_Caps , sizeof ( m_Caps ));                    // D3D caps for current device

	m_HWNDFocus = NULL;                  // the main app focus window
    m_HWNDDeviceFullScreen = NULL;       // the main app device window in fullscreen mode
    m_HWNDDeviceWindowed = NULL;         // the main app device window in windowed mode
    m_AdapterMonitor = NULL;          // the monitor of the adapter 
    m_Menu = NULL;                       // handle to menu

    m_FullScreenBackBufferWidthAtModeChange = 0;  // back buffer size of fullscreen mode right before switching to windowed mode.  Used to restore to same resolution when toggling back to fullscreen
    m_FullScreenBackBufferHeightAtModeChange = 0; // back buffer size of fullscreen mode right before switching to windowed mode.  Used to restore to same resolution when toggling back to fullscreen
    m_WindowBackBufferWidthAtModeChange = 0;  // back buffer size of windowed mode right before switching to fullscreen mode.  Used to restore to same resolution when toggling back to windowed mode
    m_WindowBackBufferHeightAtModeChange = 0; // back buffer size of windowed mode right before switching to fullscreen mode.  Used to restore to same resolution when toggling back to windowed mode
    m_WindowedStyleAtModeChange = 0;  // window style
    ZeroMemory ( &m_WindowedPlacement , sizeof ( m_WindowedPlacement )); // record of windowed HWND position/show state/etc
    m_TopmostWhileWindowed = false;       // if true, the windowed HWND is topmost 
    m_Minimized = false;                  // if true, the HWND is minimized
    m_Maximized = false;                  // if true, the HWND is maximized
    m_MinimizedWhileFullscreen = false;   // if true, the HWND is minimized due to a focus switch away when fullscreen mode
    m_IgnoreSizeChange = false;           // if true,  won't reset the device upon HWND size change

    m_Time = 0;                      // current time in seconds
    m_ElapsedTime = 0;                // time elapsed since last frame

    m_HInstance = NULL;              // handle to the app instance
    m_LastStatsUpdateTime = 0;       // last time the stats were updated
    m_LastStatsUpdateFrames = 0;      // frames count since last time the stats were updated
    m_FPS = 0;                        // frames per second
    m_CurrentFrameNumber = 0;         // the current frame number
    m_KeyboardHook = NULL;               // handle to keyboard hook
    m_AllowShortcutKeysWhenFullscreen = false; // if true, when fullscreen enable shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
    m_AllowShortcutKeysWhenWindowed = false;   // if true, when windowed enable shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
    m_AllowShortcutKeys = false;          // if true, then shortcut keys are currently disabled (Windows key, etc)
    m_CallDefWindowProc = false;          // if true, StaticWndProc will call DefWindowProc for unhandled messages. Applications rendering to a dialog may need to set this to false.
    ZeroMemory ( &m_StartupStickyKeys, sizeof ( m_StartupStickyKeys ));     // StickyKey settings upon startup so they can be restored later
    ZeroMemory ( &m_StartupToggleKeys, sizeof ( m_StartupToggleKeys ));     // ToggleKey settings upon startup so they can be restored later
    ZeroMemory ( &m_StartupFilterKeys, sizeof ( m_StartupFilterKeys ));     // FilterKey settings upon startup so they can be restored later

    m_HandleDefaultHotkeys = false;       // if true, then  will handle some default hotkeys
    m_HandleAltEnter = false;             // if true, then  will handle Alt-Enter
    m_ShowMsgBoxOnError = false;          // if true, then msgboxes are displayed upon errors
    m_NoStats = false;                    // if true, then GetFrameStats() and GetDeviceStats() will return blank strings
    m_ClipCursorWhenFullScreen = false;   // if true, then  will keep the cursor from going outside the window when full screen
    m_ShowCursorWhenFullScreen = false;   // if true, then  will show a cursor when full screen
    m_ConstantFrameTime = false;          // if true, then elapsed frame time will always be 0.05f seconds which is good for debugging or automated capture
    m_TimePerFrame = 0;               // the constant time per frame in seconds, only valid if m_ConstantFrameTime==true
    m_WireframeMode = false;              // if true, then D3DRS_FILLMODE==D3DFILL_WIREFRAME else D3DRS_FILLMODE==D3DFILL_SOLID 
    m_AutoChangeAdapter = false;          // if true, then the adapter will automatically change if the window is different monitor
    m_WindowCreatedWithDefaultPositions = false; // if true, then CW_USEDEFAULT was used and the window should be moved to the right adapter
    m_ExitCode = 0;                   // the exit code to be returned to the command line

    m_Inited = false;                 // if true, then Init() has succeeded
    m_WindowCreated = false;              // if true, then CreateWindow() or SetWindow() has succeeded
    m_DeviceCreated = false;              // if true, then CreateDevice*() or SetDevice() has succeeded

    m_InitCalled = false;             // if true, then Init() was called
    m_WindowCreateCalled = false;         // if true, then CreateWindow() or SetWindow() was called
    m_DeviceCreateCalled = false;         // if true, then CreateDevice*() or SetDevice() was called

    m_DeviceObjectsCreated = false;       // if true, then DeviceCreated callback has been called (if non-NULL)
    m_DeviceObjectsReset = false;         // if true, then DeviceReset callback has been called (if non-NULL)
    m_InsideDeviceCallback = false;       // if true, then the framework is inside an app device callback
    m_InsideMainloop = false;             // if true, then the framework is inside the main loop
    m_Active = false;                     // if true, then the app is the active top level window
    m_TimePaused = false;                 // if true, then time is paused
    m_RenderingPaused = false;            // if true, then rendering is paused
    m_PauseRenderingCount = 0;        // pause rendering ref count
    m_PauseTimeCount = 0;             // pause time ref count
    m_DeviceLost = false;                 // if true, then the device is lost and needs to be reset
    m_NotifyOnMouseMove = false;          // if true, include WM_MOUSEMOVE in mousecallback
    m_Automation = false;                 // if true, automation is enabled
    m_InSizeMove = false;                 // if true, app is inside a WM_ENTERSIZEMOVE

    m_OverrideAdapterOrdinal = 0;     // if != -1, then override to use this adapter ordinal
    m_OverrideWindowed = false;           // if true, then force to start windowed
    m_OverrideFullScreen = false;         // if true, then force to start full screen
    m_OverrideStartX = 0;             // if != -1, then override to this X position of the window
    m_OverrideStartY = 0;             // if != -1, then override to this Y position of the window
    m_OverrideWidth = 0;              // if != 0, then override to this width
    m_OverrideHeight = 0;             // if != 0, then override to this height
    m_OverrideForceHAL = false;           // if true, then force to HAL device (failing if one doesn't exist)
    m_OverrideForceREF = false;           // if true, then force to REF device (failing if one doesn't exist)
    m_OverrideForcePureHWVP = false;      // if true, then force to use pure HWVP (failing if device doesn't support it)
    m_OverrideForceHWVP = false;          // if true, then force to use HWVP (failing if device doesn't support it)
    m_OverrideForceSWVP = false;          // if true, then force to use SWVP 
    m_OverrideConstantFrameTime = false;  // if true, then force to constant frame time
    m_OverrideConstantTimePerFrame = 0; // the constant time per frame in seconds if m_OverrideConstantFrameTime==true
    m_OverrideQuitAfterFrame = 0;     // if != 0, then it will force the app to quit after that frame
    m_OverrideForceVsync = 0;         // if == 0, then it will force the app to use D3DPRESENT_INTERVAL_IMMEDIATE, if == 1 force use of D3DPRESENT_INTERVAL_DEFAULT

	ZeroMemory ( m_Keys , sizeof ( m_Keys ));                      // array of key state
    ZeroMemory ( m_MouseButtons , sizeof ( m_MouseButtons ));                // array of mouse states

	m_TimerList = NULL;
    ZeroMemory ( m_StaticFrameStats, sizeof ( m_StaticFrameStats ));           // static part of frames stats 
    ZeroMemory ( m_FPSStats , sizeof ( m_FPSStats ));                   // fps stats
    ZeroMemory ( m_FrameStats , sizeof ( m_FrameStats ));               // frame stats (fps, width, etc)
    ZeroMemory ( m_DeviceStats , sizeof ( m_DeviceStats ));            // device stats (description, device type, etc)
    ZeroMemory ( m_WindowTitle , sizeof ( m_WindowTitle ));           // window title

	m_ShowStats = EShowStats::DontShow;
	m_pFrameStatsFont = NULL;
	m_pFrameStatsText = NULL;
    m_ThreadSafe = true; 
    InitializeCriticalSection( &m_CS ); 
    m_OverrideStartX = -1; 
    m_OverrideStartY = -1; 
    m_OverrideAdapterOrdinal = -1; 
	//TODO why is this set to 0 a few lines above and then -1 here? Also, is -1 a valid value - we test
	//for only 0 or 1 later?
    m_OverrideForceVsync = -1;
    m_AutoChangeAdapter = true; 
    m_ShowMsgBoxOnError = true;
    m_AllowShortcutKeysWhenWindowed = true;
    m_Active = true;
    m_CallDefWindowProc = true;
}

void CApplication::DestroyState()
{
	SafeDelete(m_TimerList);
	DeleteStatusDisplayObjects();
    DeleteCriticalSection( &m_CS ); 
}

// Init the application, Hotkeys handled by default are:
//          Alt-Enter           toggle between full screen & windowed (hotkey always enabled)
//          ESC                 exit app 
//          F3                  toggle HAL/REF
//          F8                  toggle wire-frame mode
//          Pause               pause time
HRESULT CApplication::Init( const std::string& SemaphoreName, bool bHandleDefaultHotkeys, bool bShowMsgBoxOnError, bool bHandleAltEnter )
{

	// Create a semaphore for the app - this keeps other copies from running
	// Each Application instance will make this check, and if the semaphore already exists it will exit immediately
	// The semaphore is released when this application closes.
	m_hInstanceSemaphore = ::CreateSemaphore( NULL, 
											  1, 
											  1, 
											  SemaphoreName.c_str() );
	if ( ::WaitForSingleObject( m_hInstanceSemaphore, 0 ) == WAIT_TIMEOUT )
	{
		::CloseHandle( m_hInstanceSemaphore );
		m_hInstanceSemaphore = NULL;
		return E_FAIL;
	}

    SetInitCalled( true );

    // Save the current sticky/toggle/filter key settings so  can restore them later
    STICKYKEYS sk = {sizeof(STICKYKEYS), 0};
    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
    SetStartupStickyKeys( sk );

    TOGGLEKEYS tk = {sizeof(TOGGLEKEYS), 0};
    SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
    SetStartupToggleKeys( tk );

    FILTERKEYS fk = {sizeof(FILTERKEYS), 0};
    SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);
    SetStartupFilterKeys( fk );

    // Increase the accuracy of Sleep() without needing to link to winmm.lib
    char wszPath[MAX_PATH+1];
    if( GetSystemDirectory( wszPath, MAX_PATH+1 ) )
    {
        StringCchCat( wszPath, MAX_PATH, "\\winmm.dll" );
        HINSTANCE hInstWinMM = LoadLibrary( wszPath );
        if( hInstWinMM ) 
        {
            LPTIMEBEGINPERIOD pTimeBeginPeriod = (LPTIMEBEGINPERIOD)GetProcAddress( hInstWinMM, "timeBeginPeriod" );
            if( NULL != pTimeBeginPeriod )
                pTimeBeginPeriod(1);

            FreeLibrary(hInstWinMM);
        }
    }

    SetShowMsgBoxOnError( bShowMsgBoxOnError );
    SetHandleDefaultHotkeys( bHandleDefaultHotkeys );
    SetHandleAltEnter( bHandleAltEnter );

    // Create a Direct3D object if one has not already been created
    IDirect3D9* pD3D = GetD3DObject();
    if( pD3D == NULL )
    {
        // This may fail if DirectX 9 isn't installed
        // This may fail if the DirectX headers are out of sync with the installed DirectX DLLs
        pD3D = _Dynamic_Direct3DCreate9( D3D_SDK_VERSION );
        SetD3D( pD3D );
    }

    if( pD3D == NULL )
    {
        // If still NULL, then something went wrong
        DisplayErrorMessage( ERR_NODIRECT3D );
        return _ERR( L"Direct3DCreate9", ERR_NODIRECT3D );
    }

    // Reset the timer
    GetTimer()->Reset();

    SetInited( true );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Creates a window with the specified window title, icon, menu, and 
// starting position.  If Init() has not already been called, it will
// call it with the default parameters.  Instead of calling this, you can 
// call SetWindow() to use an existing window.  
//--------------------------------------------------------------------------------------
HRESULT CApplication::CreateRenderWindow( const char* strWindowTitle, HINSTANCE hInstance, 
                                          HICON hIcon, HMENU hMenu, int x, int y )
{
    HRESULT hr;

    // Not allowed to call this from inside the device callbacks
    if( GetInsideDeviceCallback() )
        return _ERR_MSGBOX( "CreateWindow", E_FAIL );

    SetWindowCreateCalled( true );

    if( !GetInited() ) 
    {
        // If Init() was already called and failed, then fail.
        // Init() must first succeed for this function to succeed
        if( GetInitCalled() )
            return E_FAIL; 

        // If Init() hasn't been called, then automatically call it
        // with default params
        hr = Init();
        if( FAILED(hr) )
            return hr;
    }

    if( GetHWNDFocus() == NULL )
    {
        if( hInstance == NULL ) 
            hInstance = (HINSTANCE)GetModuleHandle(NULL);
        SetHInstance( hInstance );

        char szExePath[MAX_PATH];
        GetModuleFileName( NULL, szExePath, MAX_PATH );
        if( hIcon == NULL ) // If the icon is NULL, then use the first one found in the exe
            hIcon = ExtractIcon( hInstance, szExePath, 0 ); 

        // Register the windows class
        WNDCLASS wndClass;
        wndClass.style = CS_DBLCLKS;
        wndClass.lpfnWndProc = StaticWndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = hIcon;
        wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = "Direct3DWindowClass";

        if( !RegisterClass( &wndClass ) )
        {
            DWORD dwError = GetLastError();
            if( dwError != ERROR_CLASS_ALREADY_EXISTS )
                return _ERR_MSGBOX( "RegisterClass", HRESULT_FROM_WIN32(dwError) );
        }

        RECT rc;
        // Override the window's initial & size position if there were cmd line args
        if( GetOverrideStartX() != -1 )
            x = GetOverrideStartX();
        if( GetOverrideStartY() != -1 )
            y = GetOverrideStartY();

        SetWindowCreatedWithDefaultPositions( false );
        if( x == CW_USEDEFAULT && y == CW_USEDEFAULT )
            SetWindowCreatedWithDefaultPositions( true );

        // Find the window's initial size, but it might be changed later
        int nDefaultWidth = 640;
        int nDefaultHeight = 480;
        if( GetOverrideWidth() != 0 )
            nDefaultWidth = GetOverrideWidth();
        if( GetOverrideHeight() != 0 )
            nDefaultHeight = GetOverrideHeight();
        SetRect( &rc, 0, 0, nDefaultWidth, nDefaultHeight );        
        AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, ( hMenu != NULL ) ? true : false );

        char* strCachedWindowTitle = GetWindowTitle();
        StringCchCopy( strCachedWindowTitle, 256, strWindowTitle );

        // Create the render window
        HWND hWnd = CreateWindow( "Direct3DWindowClass", strWindowTitle, WS_OVERLAPPEDWINDOW,
                               x, y, (rc.right-rc.left), (rc.bottom-rc.top), 0,
                               hMenu, hInstance, 0 );
        if( hWnd == NULL )
        {
            DWORD dwError = GetLastError();
            return _ERR_MSGBOX( L"CreateWindow", HRESULT_FROM_WIN32(dwError) );
        }

        SetWindowCreated( true );
        SetHWNDFocus( hWnd );
        SetHWNDDeviceFullScreen( hWnd );
        SetHWNDDeviceWindowed( hWnd );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Sets a previously created window for the framework to use.  If Init() 
// has not already been called, it will call it with the default parameters.  
// Instead of calling this, you can call CreateWindow() to create a new window.  
//--------------------------------------------------------------------------------------
HRESULT CApplication::SetRenderWindow( HWND hWndFocus, HWND hWndDeviceFullScreen, HWND hWndDeviceWindowed, bool bHandleMessages )
{
    HRESULT hr;
 
    // Not allowed to call this from inside the device callbacks
    if( GetInsideDeviceCallback() )
        return _ERR_MSGBOX( L"CreateWindow", E_FAIL );

    SetWindowCreateCalled( true );

    // To avoid confusion, we do not allow any HWND to be NULL here.  The
    // caller must pass in valid HWND for all three parameters.  The same
    // HWND may be used for more than one parameter.
    if( hWndFocus == NULL || hWndDeviceFullScreen == NULL || hWndDeviceWindowed == NULL )
        return _ERR_MSGBOX( L"SetWindow", E_INVALIDARG );

    // If subclassing the window, set the pointer to the local window procedure
    if( bHandleMessages )
    {
        // Switch window procedures
#ifdef _WIN64
        LONG_PTR nResult = SetWindowLongPtr( hWndFocus, GWLP_WNDPROC, (LONG_PTR)StaticWndProc );
#else
        LONG_PTR nResult = SetWindowLongPtr( hWndFocus, GWLP_WNDPROC, (LONG)(LONG_PTR)StaticWndProc );
#endif 
 
        DWORD dwError = GetLastError();
        if( nResult == 0 )
            return _ERR_MSGBOX( L"SetWindowLongPtr", HRESULT_FROM_WIN32(dwError) );
    }
 
    if( !GetInited() ) 
    {
        // If Init() was already called and failed, then fail.
        // Init() must first succeed for this function to succeed
        if( GetInitCalled() )
            return E_FAIL; 
 
        // If Init() hasn't been called, then automatically call it
        // with default params
        hr = Init();
        if( FAILED(hr) )
            return hr;
    }
 
    char* strCachedWindowTitle = GetWindowTitle();
    GetWindowText( hWndFocus, strCachedWindowTitle, 255 );
    strCachedWindowTitle[255] = 0;
   
    HINSTANCE hInstance = (HINSTANCE) (LONG_PTR) GetWindowLongPtr( hWndFocus, GWLP_HINSTANCE ); 
    SetHInstance( hInstance );
    SetWindowCreatedWithDefaultPositions( false );
    SetWindowCreated( true );
    SetHWNDFocus( hWndFocus );
    SetHWNDDeviceFullScreen( hWndDeviceFullScreen );
    SetHWNDDeviceWindowed( hWndDeviceWindowed );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Creates a Direct3D device. If CreateWindow() or SetWindow() has not already 
// been called, it will call CreateWindow() with the default parameters.  
// Instead of calling this, you can call SetDevice() or CreateDeviceFromSettings() 
//--------------------------------------------------------------------------------------
HRESULT CApplication::CreateDevice( UINT AdapterOrdinal, bool bWindowed, int nSuggestedWidth, int nSuggestedHeight )
{
    HRESULT hr;

    // Not allowed to call this from inside the device callbacks
    if( GetInsideDeviceCallback() )
        return _ERR_MSGBOX( L"CreateWindow", E_FAIL );

    SetDeviceCreateCalled( true );

    // If CreateWindow() or SetWindow() has not already been called, 
    // then call CreateWindow() with the default parameters.         
    if( !GetWindowCreated() ) 
    {
        // If CreateWindow() or SetWindow() was already called and failed, then fail.
        // CreateWindow() or SetWindow() must first succeed for this function to succeed
        if( GetWindowCreateCalled() )
            return E_FAIL; 

        // If CreateWindow() or SetWindow() hasn't been called, then 
        // automatically call CreateWindow() with default params
        hr = CreateRenderWindow();
        if( FAILED(hr) )
            return hr;
    }

    // Force an enumeration with the new IsDeviceAcceptable callback
    PrepareEnumerationObject( true );

    MatchOptions matchOptions;
	matchOptions.AdapterOrdinal     = EMatchType::PreserveInput;
    matchOptions.DeviceType         = EMatchType::IgnoreInput;
    matchOptions.Windowed           = EMatchType::PreserveInput;
    matchOptions.AdapterFormat      = EMatchType::IgnoreInput;
    matchOptions.VertexProcessing   = EMatchType::IgnoreInput;
    if( bWindowed || (nSuggestedWidth != 0 && nSuggestedHeight != 0) )
        matchOptions.Resolution     = EMatchType::ClosestToInput;
    else
        matchOptions.Resolution     = EMatchType::IgnoreInput;
    matchOptions.BackBufferFormat   = EMatchType::IgnoreInput;
    matchOptions.BackBufferCount    = EMatchType::IgnoreInput;
    matchOptions.MultiSample        = EMatchType::IgnoreInput;
    matchOptions.SwapEffect         = EMatchType::IgnoreInput;
    matchOptions.DepthFormat        = EMatchType::IgnoreInput;
    matchOptions.StencilFormat      = EMatchType::IgnoreInput;
    matchOptions.PresentFlags       = EMatchType::IgnoreInput;
    matchOptions.RefreshRate        = EMatchType::IgnoreInput;
    matchOptions.PresentInterval    = EMatchType::IgnoreInput;

    DeviceSettings deviceSettings;
    ZeroMemory( &deviceSettings, sizeof(DeviceSettings) );
    deviceSettings.AdapterOrdinal      = AdapterOrdinal;
    deviceSettings.pp.Windowed         = bWindowed;
    deviceSettings.pp.BackBufferWidth  = nSuggestedWidth;
    deviceSettings.pp.BackBufferHeight = nSuggestedHeight;

    // Override with settings from the command line
    if( GetOverrideWidth() != 0 )
        deviceSettings.pp.BackBufferWidth = GetOverrideWidth();
    if( GetOverrideHeight() != 0 )
        deviceSettings.pp.BackBufferHeight = GetOverrideHeight();

    if( GetOverrideAdapterOrdinal() != -1 )
        deviceSettings.AdapterOrdinal = GetOverrideAdapterOrdinal();

    if( GetOverrideFullScreen() )
    {
        deviceSettings.pp.Windowed = FALSE;
        if( GetOverrideWidth() == 0 && GetOverrideHeight() == 0 )
            matchOptions.Resolution = EMatchType::IgnoreInput;
    }
    if( GetOverrideWindowed() )
        deviceSettings.pp.Windowed = TRUE;

    if( GetOverrideForceHAL() )
    {
        deviceSettings.DeviceType = D3DDEVTYPE_HAL;
        matchOptions.DeviceType = EMatchType::PreserveInput;
    }
    if( GetOverrideForceREF() )
    {
        deviceSettings.DeviceType = D3DDEVTYPE_REF;
        matchOptions.DeviceType = EMatchType::PreserveInput;
    }

    if( GetOverrideForcePureHWVP() )
    {
        deviceSettings.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
        matchOptions.VertexProcessing = EMatchType::PreserveInput;
    }
    else if( GetOverrideForceHWVP() )
    {
        deviceSettings.BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        matchOptions.VertexProcessing = EMatchType::PreserveInput;
    }
    else if( GetOverrideForceSWVP() )
    {
        deviceSettings.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        matchOptions.VertexProcessing = EMatchType::PreserveInput;
    }

    if( GetOverrideForceVsync() == 0 )
    {
        deviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        matchOptions.PresentInterval = EMatchType::PreserveInput;
    }
    else if( GetOverrideForceVsync() == 1 )
    {
        deviceSettings.pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
        matchOptions.PresentInterval = EMatchType::PreserveInput;
    }

    hr = FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( FAILED(hr) ) // the call will fail if no valid devices were found
    {
        DisplayErrorMessage( hr );
        return _ERR( L"FindValidDeviceSettings", hr );
    }

    // Change to a Direct3D device created from the new device settings.  
    // If there is an existing device, then either reset or recreated the scene
    hr = ChangeDevice( &deviceSettings, NULL, false, true );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Passes a previously created Direct3D device for use by the framework.  
// If CreateWindow() has not already been called, it will call it with the 
// default parameters.  Instead of calling this, you can call CreateDevice() or 
// CreateDeviceFromSettings() 
//--------------------------------------------------------------------------------------
HRESULT CApplication::SetDevice( IDirect3DDevice9* pd3dDevice )
{
    HRESULT hr;

    if( pd3dDevice == NULL )
        return _ERR_MSGBOX( L"SetDevice", E_INVALIDARG );

    // Not allowed to call this from inside the device callbacks
    if( GetInsideDeviceCallback() )
        return _ERR_MSGBOX( L"CreateWindow", E_FAIL );

    SetDeviceCreateCalled( true );

    // If CreateWindow() or SetWindow() has not already been called, 
    // then call CreateWindow() with the default parameters.         
    if( !GetWindowCreated() ) 
    {
        // If CreateWindow() or SetWindow() was already called and failed, then fail.
        // CreateWindow() or SetWindow() must first succeed for this function to succeed
        if( GetWindowCreateCalled() )
            return E_FAIL; 

        // If CreateWindow() or SetWindow() hasn't been called, then 
        // automatically call CreateWindow() with default params
        hr = CreateRenderWindow();
        if( FAILED(hr) )
            return hr;
    }

    DeviceSettings* pDeviceSettings = new DeviceSettings;
    if( pDeviceSettings == NULL )
        return E_OUTOFMEMORY;
    ZeroMemory( pDeviceSettings, sizeof(DeviceSettings) );

    // Get the present params from the swap chain
    IDirect3DSurface9* pBackBuffer = NULL;
    hr = pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    if( SUCCEEDED(hr) )
    {
        IDirect3DSwapChain9* pSwapChain = NULL;
        hr = pBackBuffer->GetContainer( IID_IDirect3DSwapChain9, (void**) &pSwapChain );
        if( SUCCEEDED(hr) )
        {
            pSwapChain->GetPresentParameters( &pDeviceSettings->pp );
            SafeRelease( pSwapChain );
        }

        SafeRelease( pBackBuffer );
    }

    D3DDEVICE_CREATION_PARAMETERS d3dCreationParams;
    pd3dDevice->GetCreationParameters( &d3dCreationParams );

    // Fill out the rest of the device settings struct
    pDeviceSettings->AdapterOrdinal = d3dCreationParams.AdapterOrdinal;
    pDeviceSettings->DeviceType     = d3dCreationParams.DeviceType;
    FindAdapterFormat( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, 
                           pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed, 
                           &pDeviceSettings->AdapterFormat );
    pDeviceSettings->BehaviorFlags  = d3dCreationParams.BehaviorFlags;

    // Change to the Direct3D device passed in
    hr = ChangeDevice( pDeviceSettings, pd3dDevice, false, false );

    delete pDeviceSettings;

    if( FAILED(hr) ) 
        return hr;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Tells the framework to change to a device created from the passed in device settings
// If CreateWindow() has not already been called, it will call it with the 
// default parameters.  Instead of calling this, you can call CreateDevice() 
// or SetDevice() 
//--------------------------------------------------------------------------------------
HRESULT CApplication::CreateDeviceFromSettings( DeviceSettings* pDeviceSettings, bool bPreserveInput, bool bClipWindowToSingleAdapter )
{
    HRESULT hr;

    SetDeviceCreateCalled( true );

    // If CreateWindow() or SetWindow() has not already been called, 
    // then call CreateWindow() with the default parameters.         
    if( !GetWindowCreated() ) 
    {
        // If CreateWindow() or SetWindow() was already called and failed, then fail.
        // CreateWindow() or SetWindow() must first succeed for this function to succeed
        if( GetWindowCreateCalled() )
            return E_FAIL; 

        // If CreateWindow() or SetWindow() hasn't been called, then 
        // automatically call CreateWindow() with default params
        hr = CreateRenderWindow();
        if( FAILED(hr) )
            return hr;
    }

    if( !bPreserveInput )
    {
        // If not preserving the input, then find the closest valid to it
        MatchOptions matchOptions;
        matchOptions.AdapterOrdinal     = EMatchType::ClosestToInput;
        matchOptions.DeviceType         = EMatchType::ClosestToInput;
        matchOptions.Windowed           = EMatchType::ClosestToInput;
        matchOptions.AdapterFormat      = EMatchType::ClosestToInput;
        matchOptions.VertexProcessing   = EMatchType::ClosestToInput;
        matchOptions.Resolution         = EMatchType::ClosestToInput;
        matchOptions.BackBufferFormat   = EMatchType::ClosestToInput;
        matchOptions.BackBufferCount    = EMatchType::ClosestToInput;
        matchOptions.MultiSample        = EMatchType::ClosestToInput;
        matchOptions.SwapEffect         = EMatchType::ClosestToInput;
        matchOptions.DepthFormat        = EMatchType::ClosestToInput;
        matchOptions.StencilFormat      = EMatchType::ClosestToInput;
        matchOptions.PresentFlags       = EMatchType::ClosestToInput;
        matchOptions.RefreshRate        = EMatchType::ClosestToInput;
        matchOptions.PresentInterval    = EMatchType::ClosestToInput;

        hr = FindValidDeviceSettings( pDeviceSettings, pDeviceSettings, &matchOptions );
        if( FAILED(hr) ) // the call will fail if no valid devices were found
        {
            DisplayErrorMessage( hr );
            return _ERR( L"FindValidDeviceSettings", hr );
        }
    }

    // Change to a Direct3D device created from the new device settings.  
    // If there is an existing device, then either reset or recreate the scene
    hr = ChangeDevice( pDeviceSettings, NULL, false, bClipWindowToSingleAdapter );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Toggle between full screen and windowed
//--------------------------------------------------------------------------------------
HRESULT CApplication::ToggleFullScreen()
{
    HRESULT hr;

    // Get the current device settings and flip the windowed state then
    // find the closest valid device settings with this change
    DeviceSettings deviceSettings = GetDeviceSettings();
    deviceSettings.pp.Windowed = !deviceSettings.pp.Windowed;

    MatchOptions matchOptions;
    matchOptions.AdapterOrdinal     = EMatchType::PreserveInput;
    matchOptions.DeviceType         = EMatchType::ClosestToInput;
    matchOptions.Windowed           = EMatchType::PreserveInput;
    matchOptions.AdapterFormat      = EMatchType::IgnoreInput;
    matchOptions.VertexProcessing   = EMatchType::ClosestToInput;
    matchOptions.BackBufferFormat   = EMatchType::IgnoreInput;
    matchOptions.BackBufferCount    = EMatchType::ClosestToInput;
    matchOptions.MultiSample        = EMatchType::ClosestToInput;
    matchOptions.SwapEffect         = EMatchType::ClosestToInput;
    matchOptions.DepthFormat        = EMatchType::ClosestToInput;
    matchOptions.StencilFormat      = EMatchType::ClosestToInput;
    matchOptions.PresentFlags       = EMatchType::ClosestToInput;
    matchOptions.RefreshRate        = EMatchType::IgnoreInput;
    matchOptions.PresentInterval    = EMatchType::ClosestToInput;

    // Go back to previous state

    UINT nWidth  = ( deviceSettings.pp.Windowed ) ? GetWindowBackBufferWidthAtModeChange() : GetFullScreenBackBufferWidthAtModeChange();
    UINT nHeight = ( deviceSettings.pp.Windowed ) ? GetWindowBackBufferHeightAtModeChange() : GetFullScreenBackBufferHeightAtModeChange();
    if( nWidth > 0 && nHeight > 0 )
    {
        matchOptions.Resolution = EMatchType::ClosestToInput;
        deviceSettings.pp.BackBufferWidth = nWidth;
        deviceSettings.pp.BackBufferHeight = nHeight;
    }
    else
    {
        // No previous data, so just switch to defaults
        matchOptions.Resolution = EMatchType::IgnoreInput;
    }
    
    hr = FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( SUCCEEDED(hr) ) 
    {
        // Create a Direct3D device using the new device settings.  
        // If there is an existing device, then it will either reset or recreate the scene.
        hr = ChangeDevice( &deviceSettings, NULL, false, false );

        // If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback so nothing changed
        if( FAILED(hr) && (hr != E_ABORT) )
        {
            // Failed creating device, try to switch back.
            deviceSettings.pp.Windowed = !deviceSettings.pp.Windowed;
            UINT nWidth  = ( deviceSettings.pp.Windowed ) ? GetWindowBackBufferWidthAtModeChange() : GetFullScreenBackBufferWidthAtModeChange();
            UINT nHeight = ( deviceSettings.pp.Windowed ) ? GetWindowBackBufferHeightAtModeChange() : GetFullScreenBackBufferHeightAtModeChange();
            if( nWidth > 0 && nHeight > 0 )
            {
                matchOptions.Resolution = EMatchType::ClosestToInput;
                deviceSettings.pp.BackBufferWidth = nWidth;
                deviceSettings.pp.BackBufferHeight = nHeight;
            }
            else
            {
                matchOptions.Resolution = EMatchType::IgnoreInput;
            }
            
            FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );

            HRESULT hr2 = ChangeDevice( &deviceSettings, NULL, false, false );
            if( FAILED(hr2) )
            {
                // If this failed, then shutdown
                Shutdown();
            }
        }
    }

    return hr;
}


//--------------------------------------------------------------------------------------
// Toggle between HAL and REF
//--------------------------------------------------------------------------------------
HRESULT CApplication::ToggleREF()
{
    HRESULT hr;

    DeviceSettings deviceSettings = GetDeviceSettings();
    if( deviceSettings.DeviceType == D3DDEVTYPE_HAL )
        deviceSettings.DeviceType = D3DDEVTYPE_REF;
    else if( deviceSettings.DeviceType == D3DDEVTYPE_REF )
        deviceSettings.DeviceType = D3DDEVTYPE_HAL;

    MatchOptions matchOptions;
    matchOptions.AdapterOrdinal     = EMatchType::PreserveInput;
    matchOptions.DeviceType         = EMatchType::PreserveInput;
    matchOptions.Windowed           = EMatchType::ClosestToInput;
    matchOptions.AdapterFormat      = EMatchType::ClosestToInput;
    matchOptions.VertexProcessing   = EMatchType::ClosestToInput;
    matchOptions.Resolution         = EMatchType::ClosestToInput;
    matchOptions.BackBufferFormat   = EMatchType::ClosestToInput;
    matchOptions.BackBufferCount    = EMatchType::ClosestToInput;
    matchOptions.MultiSample        = EMatchType::ClosestToInput;
    matchOptions.SwapEffect         = EMatchType::ClosestToInput;
    matchOptions.DepthFormat        = EMatchType::ClosestToInput;
    matchOptions.StencilFormat      = EMatchType::ClosestToInput;
    matchOptions.PresentFlags       = EMatchType::ClosestToInput;
    matchOptions.RefreshRate        = EMatchType::ClosestToInput;
    matchOptions.PresentInterval    = EMatchType::ClosestToInput;
    
    hr = FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
    if( SUCCEEDED(hr) ) 
    {
        // Create a Direct3D device using the new device settings.  
        // If there is an existing device, then it will either reset or recreate the scene.
        hr = ChangeDevice( &deviceSettings, NULL, false, false );

        // If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback so nothing changed
        if( FAILED( hr ) && (hr != E_ABORT) )
        {
            // Failed creating device, try to switch back.
            if( deviceSettings.DeviceType == D3DDEVTYPE_HAL )
                deviceSettings.DeviceType = D3DDEVTYPE_REF;
            else if( deviceSettings.DeviceType == D3DDEVTYPE_REF )
                deviceSettings.DeviceType = D3DDEVTYPE_HAL;

            FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );

            HRESULT hr2 = ChangeDevice( &deviceSettings, NULL, false, false );
            if( FAILED(hr2) )
            {
                // If this failed, then shutdown
                Shutdown();
            }
        }
    }

    return hr;
}


//--------------------------------------------------------------------------------------
// Internal helper function to prepare the enumeration object by creating it if it 
// didn't already exist and enumerating if desired.
//--------------------------------------------------------------------------------------
CDeviceEnumeration* CApplication::PrepareEnumerationObject( bool bEnumerate )
{
    // Create a new CDeviceEnumeration object and enumerate all devices unless its already been done
    CDeviceEnumeration* pd3dEnum = GetD3DEnumeration();
    if( pd3dEnum == NULL )
    {
		pd3dEnum = CDeviceEnumeration::GetEnumeration(); 
        SetD3DEnumeration( pd3dEnum );

        bEnumerate = true;
    }

    if( bEnumerate )
    {
        // Enumerate for each adapter all of the supported display modes, 
        // device types, adapter formats, back buffer formats, window/full screen support, 
        // depth stencil formats, multisampling types/qualities, and presentations intervals.
        //
        // For each combination of device type (HAL/REF), adapter format, back buffer format, and
        // IsWindowed it will call the app's ConfirmDevice callback.  This allows the app
        // to reject or allow that combination based on its caps/etc.  It also allows the 
        // app to change the BehaviorFlags.  The BehaviorFlags defaults non-pure HWVP 
        // if supported otherwise it will default to SWVP, however the app can change this 
        // through the ConfirmDevice callback.
        IDirect3D9* pD3D = GetD3DObject();
        pd3dEnum->Enumerate( pD3D, this );
    }
    
    return pd3dEnum;
}


//--------------------------------------------------------------------------------------
// This function tries to find valid device settings based upon the input device settings 
// struct and the match options.  For each device setting a match option in the 
// MatchOptions struct specifies how the function makes decisions.  For example, if 
// the caller wants a HAL device with a back buffer format of D3DFMT_A2B10G10R10 but the 
// HAL device on the system does not support D3DFMT_A2B10G10R10 however a REF device is 
// installed that does, then the function has a choice to either use REF or to change to 
// a back buffer format to compatible with the HAL device.  The match options lets the 
// caller control how these choices are made.
//
// Each match option must be one of the following types: 
//      EMatchType::IgnoreInput: Uses the closest valid value to a default 
//      EMatchType::PreserveInput: Uses the input without change, but may cause no valid device to be found
//      EMatchType::ClosestToInput: Uses the closest valid value to the input 
//
// If pMatchOptions is NULL then, all of the match options are assumed to be EMatchType::IgnoreInput.  
// The function returns failure if no valid device settings can be found otherwise 
// the function returns success and the valid device settings are written to pOut.
//--------------------------------------------------------------------------------------
HRESULT CApplication::FindValidDeviceSettings( DeviceSettings* pOut, DeviceSettings* pIn, 
                                     MatchOptions* pMatchOptions )
{
    if( pOut == NULL )
        return _ERR_MSGBOX( L"FindValidDeviceSettings", E_INVALIDARG );

    CDeviceEnumeration* pd3dEnum = PrepareEnumerationObject( false );
    IDirect3D9*      pD3D     = GetD3DObject();

    // Default to EMatchType::IgnoreInput for everything unless pMatchOptions isn't NULL
    MatchOptions defaultMatchOptions;
    if( NULL == pMatchOptions )
    {
        ZeroMemory( &defaultMatchOptions, sizeof(MatchOptions) );
        pMatchOptions = &defaultMatchOptions;
    }

    // Build an optimal device settings structure based upon the match 
    // options.  If the match option is set to ignore, then a optimal default value is used.
    // The default value may not exist on the system, but later this will be taken 
    // into account.
    DeviceSettings optimalDeviceSettings;
    BuildOptimalDeviceSettings( &optimalDeviceSettings, pIn, pMatchOptions );

    // Find the best combination of:
    //      Adapter Ordinal
    //      Device Type
    //      Adapter Format
    //      Back Buffer Format
    //      Windowed
    // given what's available on the system and the match options combined with the device settings input.
    // This combination of settings is encapsulated by the CDeviceEnumDeviceSettingsCombo class.
    float fBestRanking = -1.0f;
    CDeviceEnumDeviceSettingsCombo* pBestDeviceSettingsCombo = NULL;
    D3DDISPLAYMODE adapterDesktopDisplayMode;

	std::vector<CDeviceEnumAdapterInfo*>* pAdapterList = pd3dEnum->GetAdapterInfoList();
    for( uint32 iAdapter=0; iAdapter<pAdapterList->size(); iAdapter++ )
    {
        CDeviceEnumAdapterInfo* pAdapterInfo = (*pAdapterList)[iAdapter];

        // Get the desktop display mode of adapter 
        pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );

        // Enum all the device types supported by this adapter to find the best device settings
        for( uint32 iDeviceInfo=0; iDeviceInfo<pAdapterInfo->deviceInfoList.size(); iDeviceInfo++ )
        {
            CDeviceEnumDeviceInfo* pDeviceInfo = pAdapterInfo->deviceInfoList[iDeviceInfo];

            // Enum all the device settings combinations.  A device settings combination is 
            // a unique set of an adapter format, back buffer format, and IsWindowed.
            for( uint32 iDeviceCombo=0; iDeviceCombo<pDeviceInfo->deviceSettingsComboList.size(); iDeviceCombo++ )
            {
                CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList[iDeviceCombo];

                // If windowed mode the adapter format has to be the same as the desktop 
                // display mode format so skip any that don't match
                if (pDeviceSettingsCombo->Windowed && (pDeviceSettingsCombo->AdapterFormat != adapterDesktopDisplayMode.Format))
                    continue;

                // Skip any combo that doesn't meet the preserve match options
                if( false == DoesDeviceComboMatchPreserveOptions( pDeviceSettingsCombo, pIn, pMatchOptions ) )
                    continue;           

                // Get a ranking number that describes how closely this device combo matches the optimal combo
                float fCurRanking = RankDeviceCombo( pDeviceSettingsCombo, &optimalDeviceSettings, &adapterDesktopDisplayMode );

                // If this combo better matches the input device settings then save it
                if( fCurRanking > fBestRanking )
                {
                    pBestDeviceSettingsCombo = pDeviceSettingsCombo;
                    fBestRanking = fCurRanking;
                }                
            }
        }
    }

    // If no best device combination was found then fail
    if( pBestDeviceSettingsCombo == NULL ) 
        return ERR_NOCOMPATIBLEDEVICES;

    // Using the best device settings combo found, build valid device settings taking heed of 
    // the match options and the input device settings
    DeviceSettings validDeviceSettings;
    BuildValidDeviceSettings( &validDeviceSettings, pBestDeviceSettingsCombo, pIn, pMatchOptions );
    *pOut = validDeviceSettings;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Internal helper function to build a device settings structure based upon the match 
// options.  If the match option is set to ignore, then a optimal default value is used.
// The default value may not exist on the system, but later this will be taken 
// into account.
//--------------------------------------------------------------------------------------
void CApplication::BuildOptimalDeviceSettings( DeviceSettings* pOptimalDeviceSettings, 
                                     DeviceSettings* pDeviceSettingsIn, 
                                     MatchOptions* pMatchOptions )
{
    IDirect3D9* pD3D = GetD3DObject();
    D3DDISPLAYMODE adapterDesktopDisplayMode;

    ZeroMemory( pOptimalDeviceSettings, sizeof(DeviceSettings) ); 

    //---------------------
    // Adapter ordinal
    //---------------------    
    if( pMatchOptions->AdapterOrdinal == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->AdapterOrdinal = D3DADAPTER_DEFAULT; 
    else
        pOptimalDeviceSettings->AdapterOrdinal = pDeviceSettingsIn->AdapterOrdinal;      

    //---------------------
    // Device type
    //---------------------
    if( pMatchOptions->DeviceType == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->DeviceType = D3DDEVTYPE_HAL; 
    else
        pOptimalDeviceSettings->DeviceType = pDeviceSettingsIn->DeviceType;

    //---------------------
    // Windowed
    //---------------------
    if( pMatchOptions->Windowed == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->pp.Windowed = TRUE; 
    else
        pOptimalDeviceSettings->pp.Windowed = pDeviceSettingsIn->pp.Windowed;

    //---------------------
    // Adapter format
    //---------------------
    if( pMatchOptions->AdapterFormat == EMatchType::IgnoreInput )
    {
        // If windowed, default to the desktop display mode
        // If fullscreen, default to the desktop display mode for quick mode change or 
        // default to D3DFMT_X8R8G8B8 if the desktop display mode is < 32bit
        pD3D->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
        if( pOptimalDeviceSettings->pp.Windowed || ColorChannelBits(adapterDesktopDisplayMode.Format) >= 8 )
            pOptimalDeviceSettings->AdapterFormat = adapterDesktopDisplayMode.Format;
        else
            pOptimalDeviceSettings->AdapterFormat = D3DFMT_X8R8G8B8;
    }
    else
    {
        pOptimalDeviceSettings->AdapterFormat = pDeviceSettingsIn->AdapterFormat;
    }

    //---------------------
    // Vertex processing
    //---------------------
    if( pMatchOptions->VertexProcessing == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING; 
    else
        pOptimalDeviceSettings->BehaviorFlags = pDeviceSettingsIn->BehaviorFlags;

    //---------------------
    // Resolution
    //---------------------
    if( pMatchOptions->Resolution == EMatchType::IgnoreInput )
    {
        // If windowed, default to 640x480
        // If fullscreen, default to the desktop res for quick mode change
        if( pOptimalDeviceSettings->pp.Windowed )
        {
            pOptimalDeviceSettings->pp.BackBufferWidth = 640;
            pOptimalDeviceSettings->pp.BackBufferHeight = 480;
        }
        else
        {
            pD3D->GetAdapterDisplayMode( pOptimalDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
            pOptimalDeviceSettings->pp.BackBufferWidth = adapterDesktopDisplayMode.Width;
            pOptimalDeviceSettings->pp.BackBufferHeight = adapterDesktopDisplayMode.Height;
        }
    }
    else
    {
        pOptimalDeviceSettings->pp.BackBufferWidth = pDeviceSettingsIn->pp.BackBufferWidth;
        pOptimalDeviceSettings->pp.BackBufferHeight = pDeviceSettingsIn->pp.BackBufferHeight;
    }

    //---------------------
    // Back buffer format
    //---------------------
    if( pMatchOptions->BackBufferFormat == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->pp.BackBufferFormat = pOptimalDeviceSettings->AdapterFormat; // Default to match the adapter format
    else
        pOptimalDeviceSettings->pp.BackBufferFormat = pDeviceSettingsIn->pp.BackBufferFormat;

    //---------------------
    // Back buffer count
    //---------------------
    if( pMatchOptions->BackBufferCount == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->pp.BackBufferCount = 2; // Default to triple buffering for perf gain
    else
        pOptimalDeviceSettings->pp.BackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
   
    //---------------------
    // Multisample
    //---------------------
    if( pMatchOptions->MultiSample == EMatchType::IgnoreInput )
    {
        // Default to no multisampling 
        pOptimalDeviceSettings->pp.MultiSampleType = D3DMULTISAMPLE_NONE;
        pOptimalDeviceSettings->pp.MultiSampleQuality = 0; 
    }
    else
    {
        pOptimalDeviceSettings->pp.MultiSampleType = pDeviceSettingsIn->pp.MultiSampleType;
        pOptimalDeviceSettings->pp.MultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
    }

    //---------------------
    // Swap effect
    //---------------------
    if( pMatchOptions->SwapEffect == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->pp.SwapEffect = D3DSWAPEFFECT_DISCARD; 
    else
        pOptimalDeviceSettings->pp.SwapEffect = pDeviceSettingsIn->pp.SwapEffect;

    //---------------------
    // Depth stencil 
    //---------------------
    if( pMatchOptions->DepthFormat == EMatchType::IgnoreInput &&
        pMatchOptions->StencilFormat == EMatchType::IgnoreInput )
    {
        UINT nBackBufferBits = ColorChannelBits( pOptimalDeviceSettings->pp.BackBufferFormat );
        if( nBackBufferBits >= 8 )
            pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D32; 
        else
            pOptimalDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D16; 
    }
    else
    {
        pOptimalDeviceSettings->pp.AutoDepthStencilFormat = pDeviceSettingsIn->pp.AutoDepthStencilFormat;
    }

    //---------------------
    // Present flags
    //---------------------
    if( pMatchOptions->PresentFlags == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->pp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    else
        pOptimalDeviceSettings->pp.Flags = pDeviceSettingsIn->pp.Flags;

    //---------------------
    // Refresh rate
    //---------------------
    if( pMatchOptions->RefreshRate == EMatchType::IgnoreInput )
        pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = 0;
    else
        pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;

    //---------------------
    // Present interval
    //---------------------
    if( pMatchOptions->PresentInterval == EMatchType::IgnoreInput )
    {
        // For windowed and fullscreen, default to D3DPRESENT_INTERVAL_DEFAULT
        // which will wait for the vertical retrace period to prevent tearing.
        // For benchmarking, use D3DPRESENT_INTERVAL_DEFAULT  which will
        // will wait not for the vertical retrace period but may introduce tearing.
        pOptimalDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }
    else
    {
        pOptimalDeviceSettings->pp.PresentationInterval = pDeviceSettingsIn->pp.PresentationInterval;
    }
}


//--------------------------------------------------------------------------------------
// Returns false for any CDeviceEnumDeviceSettingsCombo that doesn't meet the preserve 
// match options against the input pDeviceSettingsIn.
//--------------------------------------------------------------------------------------
bool CApplication::DoesDeviceComboMatchPreserveOptions( CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo, 
                                                 DeviceSettings* pDeviceSettingsIn, 
                                                 MatchOptions* pMatchOptions )
{
    //---------------------
    // Adapter ordinal
    //---------------------
    if( pMatchOptions->AdapterOrdinal == EMatchType::PreserveInput && 
        (pDeviceSettingsCombo->AdapterOrdinal != pDeviceSettingsIn->AdapterOrdinal) )
        return false;

    //---------------------
    // Device type
    //---------------------
    if( pMatchOptions->DeviceType == EMatchType::PreserveInput && 
        (pDeviceSettingsCombo->DeviceType != pDeviceSettingsIn->DeviceType) )
        return false;

    //---------------------
    // Windowed
    //---------------------
    if( pMatchOptions->Windowed == EMatchType::PreserveInput && 
        (pDeviceSettingsCombo->Windowed != pDeviceSettingsIn->pp.Windowed) )
        return false;

    //---------------------
    // Adapter format
    //---------------------
    if( pMatchOptions->AdapterFormat == EMatchType::PreserveInput && 
        (pDeviceSettingsCombo->AdapterFormat != pDeviceSettingsIn->AdapterFormat) )
        return false;

    //---------------------
    // Vertex processing
    //---------------------
    // If keep VP and input has HWVP, then skip if this combo doesn't have HWTL 
    if( pMatchOptions->VertexProcessing == EMatchType::PreserveInput && 
        ((pDeviceSettingsIn->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0) && 
        ((pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0) )
        return false;

    //---------------------
    // Resolution
    //---------------------
    // If keep resolution then check that width and height supported by this combo
    if( pMatchOptions->Resolution == EMatchType::PreserveInput )
    {
        bool bFound = false;
        for( uint32 i=0; i< pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); i++ )
        {
            D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[i];
            if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
                continue; // Skip this display mode if it doesn't match the combo's adapter format

            if( displayMode.Width == pDeviceSettingsIn->pp.BackBufferWidth &&
                displayMode.Height == pDeviceSettingsIn->pp.BackBufferHeight )
            {
                bFound = true;
                break;
            }
        }

        // If the width and height are not supported by this combo, return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Back buffer format
    //---------------------
    if( pMatchOptions->BackBufferFormat == EMatchType::PreserveInput && 
        pDeviceSettingsCombo->BackBufferFormat != pDeviceSettingsIn->pp.BackBufferFormat )
        return false;

    //---------------------
    // Back buffer count
    //---------------------
    // No caps for the back buffer count

    //---------------------
    // Multisample
    //---------------------
    if( pMatchOptions->MultiSample == EMatchType::PreserveInput )
    {
        bool bFound = false;
        for( uint32 i=0; i<pDeviceSettingsCombo->multiSampleTypeList.size(); i++ )
        {
            D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList[i];
            DWORD msQuality  = pDeviceSettingsCombo->multiSampleQualityList[i];

            if( msType == pDeviceSettingsIn->pp.MultiSampleType &&
                msQuality >= pDeviceSettingsIn->pp.MultiSampleQuality )
            {
                bFound = true;
                break;
            }
        }

        // If multisample type/quality not supported by this combo, then return false
        if( !bFound )
            return false;
    }
        
    //---------------------
    // Swap effect
    //---------------------
    // No caps for swap effects

    //---------------------
    // Depth stencil 
    //---------------------
    // If keep depth stencil format then check that the depth stencil format is supported by this combo
    if( pMatchOptions->DepthFormat == EMatchType::PreserveInput &&
        pMatchOptions->StencilFormat == EMatchType::PreserveInput )
    {
        if( pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN &&
			!ContainerContains(pDeviceSettingsCombo->depthStencilFormatList, pDeviceSettingsIn->pp.AutoDepthStencilFormat ) )
            return false;
    }

    // If keep depth format then check that the depth format is supported by this combo
    if( pMatchOptions->DepthFormat == EMatchType::PreserveInput &&
        pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
    {
        bool bFound = false;
        UINT dwDepthBits = DepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
        for( uint32 i=0; i<pDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
        {
            D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList[i];
            UINT dwCurDepthBits = DepthBits( depthStencilFmt );
            if( dwCurDepthBits - dwDepthBits == 0)
                bFound = true;
        }

        if( !bFound )
            return false;
    }

    // If keep depth format then check that the depth format is supported by this combo
    if( pMatchOptions->StencilFormat == EMatchType::PreserveInput &&
        pDeviceSettingsIn->pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN )
    {
        bool bFound = false;
        UINT dwStencilBits = StencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );
        for( uint32 i=0; i<pDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
        {
            D3DFORMAT depthStencilFmt = pDeviceSettingsCombo->depthStencilFormatList[i];
            UINT dwCurStencilBits = StencilBits( depthStencilFmt );
            if( dwCurStencilBits - dwStencilBits == 0)
                bFound = true;
        }

        if( !bFound )
            return false;
    }

    //---------------------
    // Present flags
    //---------------------
    // No caps for the present flags

    //---------------------
    // Refresh rate
    //---------------------
    // If keep refresh rate then check that the resolution is supported by this combo
    if( pMatchOptions->RefreshRate == EMatchType::PreserveInput )
    {
        bool bFound = false;
        for( uint32 i=0; i<pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); i++ )
        {
            D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[i];
            if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
                continue;
            if( displayMode.RefreshRate == pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz )
            {
                bFound = true;
                break;
            }
        }

        // If refresh rate not supported by this combo, then return false
        if( !bFound )
            return false;
    }

    //---------------------
    // Present interval
    //---------------------
    // If keep present interval then check that the present interval is supported by this combo
    if( pMatchOptions->PresentInterval == EMatchType::PreserveInput &&
		!ContainerContains( pDeviceSettingsCombo->presentIntervalList, pDeviceSettingsIn->pp.PresentationInterval ) )
		return false;

    return true;
}


//--------------------------------------------------------------------------------------
// Returns a ranking number that describes how closely this device 
// combo matches the optimal combo based on the match options and the optimal device settings
//--------------------------------------------------------------------------------------
float CApplication::RankDeviceCombo( CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo, 
                           DeviceSettings* pOptimalDeviceSettings,
                           D3DDISPLAYMODE* pAdapterDesktopDisplayMode )
{
    float fCurRanking = 0.0f; 

    // Arbitrary weights.  Gives preference to the ordinal, device type, and windowed
    const float fAdapterOrdinalWeight   = 1000.0f;
    const float fDeviceTypeWeight       = 100.0f;
    const float fWindowWeight           = 10.0f;
    const float fAdapterFormatWeight    = 1.0f;
    const float fVertexProcessingWeight = 1.0f;
    const float fResolutionWeight       = 1.0f;
    const float fBackBufferFormatWeight = 1.0f;
    const float fMultiSampleWeight      = 1.0f;
    const float fDepthStencilWeight     = 1.0f;
    const float fRefreshRateWeight      = 1.0f;
    const float fPresentIntervalWeight  = 1.0f;

    //---------------------
    // Adapter ordinal
    //---------------------
    if( pDeviceSettingsCombo->AdapterOrdinal == pOptimalDeviceSettings->AdapterOrdinal )
        fCurRanking += fAdapterOrdinalWeight;

    //---------------------
    // Device type
    //---------------------
    if( pDeviceSettingsCombo->DeviceType == pOptimalDeviceSettings->DeviceType )
        fCurRanking += fDeviceTypeWeight;
    // Slightly prefer HAL 
    if( pDeviceSettingsCombo->DeviceType == D3DDEVTYPE_HAL )
        fCurRanking += 0.1f; 

    //---------------------
    // Windowed
    //---------------------
    if( pDeviceSettingsCombo->Windowed == pOptimalDeviceSettings->pp.Windowed )
        fCurRanking += fWindowWeight;

    //---------------------
    // Adapter format
    //---------------------
    if( pDeviceSettingsCombo->AdapterFormat == pOptimalDeviceSettings->AdapterFormat )
    {
        fCurRanking += fAdapterFormatWeight;
    }
    else
    {
        int nBitDepthDelta = abs( (long) ColorChannelBits(pDeviceSettingsCombo->AdapterFormat) -
                                  (long) ColorChannelBits(pOptimalDeviceSettings->AdapterFormat) );
        float fScale = __max(0.9f - (float)nBitDepthDelta*0.2f, 0.0f);
        fCurRanking += fScale * fAdapterFormatWeight;
    }

    if( !pDeviceSettingsCombo->Windowed )
    {
        // Slightly prefer when it matches the desktop format or is D3DFMT_X8R8G8B8
        bool bAdapterOptimalMatch;
        if( ColorChannelBits(pAdapterDesktopDisplayMode->Format) >= 8 )
            bAdapterOptimalMatch = (pDeviceSettingsCombo->AdapterFormat == pAdapterDesktopDisplayMode->Format);
        else
            bAdapterOptimalMatch = (pDeviceSettingsCombo->AdapterFormat == D3DFMT_X8R8G8B8);

        if( bAdapterOptimalMatch )
            fCurRanking += 0.1f;
    }

    //---------------------
    // Vertex processing
    //---------------------
    if( (pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 || 
        (pOptimalDeviceSettings->BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0 )
    {
        if( (pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
            fCurRanking += fVertexProcessingWeight;
    }
    // Slightly prefer HW T&L
    if( (pDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
        fCurRanking += 0.1f;

    //---------------------
    // Resolution
    //---------------------
    bool bResolutionFound = false;
    for( uint32 idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); idm++ )
    {
        D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[idm];
        if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
            continue;
        if( displayMode.Width == pOptimalDeviceSettings->pp.BackBufferWidth &&
            displayMode.Height == pOptimalDeviceSettings->pp.BackBufferHeight )
            bResolutionFound = true;
    }
    if( bResolutionFound )
        fCurRanking += fResolutionWeight;

    //---------------------
    // Back buffer format
    //---------------------
    if( pDeviceSettingsCombo->BackBufferFormat == pOptimalDeviceSettings->pp.BackBufferFormat )
    {
        fCurRanking += fBackBufferFormatWeight;
    }
    else
    {
        int nBitDepthDelta = abs( (long) ColorChannelBits(pDeviceSettingsCombo->BackBufferFormat) -
                                  (long) ColorChannelBits(pOptimalDeviceSettings->pp.BackBufferFormat) );
        float fScale = __max(0.9f - (float)nBitDepthDelta*0.2f, 0.0f);
        fCurRanking += fScale * fBackBufferFormatWeight;
    }

    // Check if this back buffer format is the same as 
    // the adapter format since this is preferred.
    bool bAdapterMatchesBB = (pDeviceSettingsCombo->BackBufferFormat == pDeviceSettingsCombo->AdapterFormat);
    if( bAdapterMatchesBB )
        fCurRanking += 0.1f;

    //---------------------
    // Back buffer count
    //---------------------
    // No caps for the back buffer count

    //---------------------
    // Multisample
    //---------------------
    bool bMultiSampleFound = false;
    for( uint32 i=0; i<pDeviceSettingsCombo->multiSampleTypeList.size(); i++ )
    {
        D3DMULTISAMPLE_TYPE msType = pDeviceSettingsCombo->multiSampleTypeList[i];
        DWORD msQuality  = pDeviceSettingsCombo->multiSampleQualityList[i];

        if( msType == pOptimalDeviceSettings->pp.MultiSampleType &&
            msQuality >= pOptimalDeviceSettings->pp.MultiSampleQuality )
        {
            bMultiSampleFound = true;
            break;
        }
    }
    if( bMultiSampleFound )
        fCurRanking += fMultiSampleWeight;
        
    //---------------------
    // Swap effect
    //---------------------
    // No caps for swap effects

    //---------------------
    // Depth stencil 
    //---------------------

	if( ContainerContains(pDeviceSettingsCombo->depthStencilFormatList, pOptimalDeviceSettings->pp.AutoDepthStencilFormat ) )
        fCurRanking += fDepthStencilWeight;

    //---------------------
    // Present flags
    //---------------------
    // No caps for the present flags

    //---------------------
    // Refresh rate
    //---------------------
    bool bRefreshFound = false;
    for( uint32 idm = 0; idm < pDeviceSettingsCombo->pAdapterInfo->displayModeList.size(); idm++ )
    {
        D3DDISPLAYMODE displayMode = pDeviceSettingsCombo->pAdapterInfo->displayModeList[idm];
        if( displayMode.Format != pDeviceSettingsCombo->AdapterFormat )
            continue;
        if( displayMode.RefreshRate == pOptimalDeviceSettings->pp.FullScreen_RefreshRateInHz )
            bRefreshFound = true;
    }
    if( bRefreshFound )
        fCurRanking += fRefreshRateWeight;

    //---------------------
    // Present interval
    //---------------------
    // If keep present interval then check that the present interval is supported by this combo
	if( ContainerContains ( pDeviceSettingsCombo->presentIntervalList , pOptimalDeviceSettings->pp.PresentationInterval ) )
        fCurRanking += fPresentIntervalWeight;

    return fCurRanking;
}


//--------------------------------------------------------------------------------------
// Builds valid device settings using the match options, the input device settings, and the 
// best device settings combo found.
//--------------------------------------------------------------------------------------
void CApplication::BuildValidDeviceSettings( DeviceSettings* pValidDeviceSettings, 
                                   CDeviceEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
                                   DeviceSettings* pDeviceSettingsIn, 
                                   MatchOptions* pMatchOptions )
{
    IDirect3D9* pD3D = GetD3DObject();
    D3DDISPLAYMODE adapterDesktopDisplayMode;
    pD3D->GetAdapterDisplayMode( pBestDeviceSettingsCombo->AdapterOrdinal, &adapterDesktopDisplayMode );

    // For each setting pick the best, taking into account the match options and 
    // what's supported by the device

    //---------------------
    // Adapter Ordinal
    //---------------------
    // Just using pBestDeviceSettingsCombo->AdapterOrdinal

    //---------------------
    // Device Type
    //---------------------
    // Just using pBestDeviceSettingsCombo->DeviceType

    //---------------------
    // Windowed 
    //---------------------
    // Just using pBestDeviceSettingsCombo->Windowed

    //---------------------
    // Adapter Format
    //---------------------
    // Just using pBestDeviceSettingsCombo->AdapterFormat

    //---------------------
    // Vertex processing
    //---------------------
    DWORD dwBestBehaviorFlags = 0;
    if( pMatchOptions->VertexProcessing == EMatchType::PreserveInput )   
    {
        dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
    }
    else if( pMatchOptions->VertexProcessing == EMatchType::IgnoreInput )    
    {
        // The framework defaults to HWVP if available otherwise use SWVP
        if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
            dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
        else
            dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
    else // if( pMatchOptions->VertexProcessing == EMatchType::ClosestToInput )    
    {
        // Default to input, and fallback to SWVP if HWVP not available 
        dwBestBehaviorFlags = pDeviceSettingsIn->BehaviorFlags;
        if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 && 
            ( (dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0 || 
              (dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) != 0) )
        {
            dwBestBehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
            dwBestBehaviorFlags &= ~D3DCREATE_MIXED_VERTEXPROCESSING;
            dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // One of these must be selected
        if( (dwBestBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) == 0 &&
            (dwBestBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING) == 0 &&
            (dwBestBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0 )
        {
            if ((pBestDeviceSettingsCombo->pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
                dwBestBehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
            else
                dwBestBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }
    }

    //---------------------
    // Resolution
    //---------------------
    D3DDISPLAYMODE bestDisplayMode;  
    if( pMatchOptions->Resolution == EMatchType::PreserveInput )   
    {
        bestDisplayMode.Width = pDeviceSettingsIn->pp.BackBufferWidth;
        bestDisplayMode.Height = pDeviceSettingsIn->pp.BackBufferHeight;
    }
    else 
    {
        D3DDISPLAYMODE displayModeIn;  
        if( pMatchOptions->Resolution == EMatchType::ClosestToInput &&
            pDeviceSettingsIn )   
        {
            displayModeIn.Width = pDeviceSettingsIn->pp.BackBufferWidth;
            displayModeIn.Height = pDeviceSettingsIn->pp.BackBufferHeight;
        }
        else // if( pMatchOptions->Resolution == EMatchType::IgnoreInput )   
        {
            if( pBestDeviceSettingsCombo->Windowed )
            {
                // The framework defaults to 640x480 for windowed
                displayModeIn.Width = 640;
                displayModeIn.Height = 480;
            }
            else
            {
                // The framework defaults to desktop resolution for fullscreen to try to avoid slow mode change
                displayModeIn.Width = adapterDesktopDisplayMode.Width;
                displayModeIn.Height = adapterDesktopDisplayMode.Height;
            }
        }

        // Call a helper function to find the closest valid display mode to the optimal 
        FindValidResolution( pBestDeviceSettingsCombo, displayModeIn, &bestDisplayMode );
    }

    //---------------------
    // Back Buffer Format
    //---------------------
    // Just using pBestDeviceSettingsCombo->BackBufferFormat

    //---------------------
    // Back buffer count
    //---------------------
    UINT bestBackBufferCount;
    if( pMatchOptions->BackBufferCount == EMatchType::PreserveInput )   
    {
        bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
    }
    else if( pMatchOptions->BackBufferCount == EMatchType::IgnoreInput )   
    {
        // The framework defaults to triple buffering 
        bestBackBufferCount = 2;
    }
    else // if( pMatchOptions->BackBufferCount == EMatchType::ClosestToInput )   
    {
        bestBackBufferCount = pDeviceSettingsIn->pp.BackBufferCount;
        if( bestBackBufferCount > 3 )
            bestBackBufferCount = 3;
        if( bestBackBufferCount < 1 )
            bestBackBufferCount = 1;
    }
    
    //---------------------
    // Multisample
    //---------------------
    D3DMULTISAMPLE_TYPE bestMultiSampleType;
    DWORD bestMultiSampleQuality;
    if( pDeviceSettingsIn && pDeviceSettingsIn->pp.SwapEffect != D3DSWAPEFFECT_DISCARD )
    {
        // Swap effect is not set to discard so multisampling has to off
        bestMultiSampleType = D3DMULTISAMPLE_NONE;
        bestMultiSampleQuality = 0;
    }
    else
    {
        if( pMatchOptions->MultiSample == EMatchType::PreserveInput )   
        {
            bestMultiSampleType    = pDeviceSettingsIn->pp.MultiSampleType;
            bestMultiSampleQuality = pDeviceSettingsIn->pp.MultiSampleQuality;
        }
        else if( pMatchOptions->MultiSample == EMatchType::IgnoreInput )   
        {
            // Default to no multisampling (always supported)
            bestMultiSampleType = D3DMULTISAMPLE_NONE;
            bestMultiSampleQuality = 0;
        }
        else if( pMatchOptions->MultiSample == EMatchType::ClosestToInput )   
        {
            // Default to no multisampling (always supported)
            bestMultiSampleType = D3DMULTISAMPLE_NONE;
            bestMultiSampleQuality = 0;

            for( uint32 i=0; i < pBestDeviceSettingsCombo->multiSampleTypeList.size(); i++ )
            {
                D3DMULTISAMPLE_TYPE type = pBestDeviceSettingsCombo->multiSampleTypeList[i];
                DWORD qualityLevels = pBestDeviceSettingsCombo->multiSampleQualityList[i];
                
                // Check whether supported type is closer to the input than our current best
                if( abs(type - pDeviceSettingsIn->pp.MultiSampleType) < abs(bestMultiSampleType - pDeviceSettingsIn->pp.MultiSampleType) )
                {
                    bestMultiSampleType = type; 
                    bestMultiSampleQuality = __min( qualityLevels-1, pDeviceSettingsIn->pp.MultiSampleQuality );
                }
            }
        }
        else
        {
            // Error case
            bestMultiSampleType = D3DMULTISAMPLE_NONE;
            bestMultiSampleQuality = 0;
        }
    }

    //---------------------
    // Swap effect
    //---------------------
    D3DSWAPEFFECT bestSwapEffect;
    if( pMatchOptions->SwapEffect == EMatchType::PreserveInput )   
    {
        bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;
    }
    else if( pMatchOptions->SwapEffect == EMatchType::IgnoreInput )   
    {
        bestSwapEffect = D3DSWAPEFFECT_DISCARD;
    }
    else // if( pMatchOptions->SwapEffect == EMatchType::ClosestToInput )   
    {
        bestSwapEffect = pDeviceSettingsIn->pp.SwapEffect;

        // Swap effect has to be one of these 3
        if( bestSwapEffect != D3DSWAPEFFECT_DISCARD &&
            bestSwapEffect != D3DSWAPEFFECT_FLIP &&
            bestSwapEffect != D3DSWAPEFFECT_COPY )
        {
            bestSwapEffect = D3DSWAPEFFECT_DISCARD;
        }
    }

    //---------------------
    // Depth stencil 
    //---------------------
    D3DFORMAT bestDepthStencilFormat;
    BOOL bestEnableAutoDepthStencil;

	std::vector< int > depthStencilRanking;
    depthStencilRanking.resize( pBestDeviceSettingsCombo->depthStencilFormatList.size() );

    UINT dwBackBufferBitDepth = ColorChannelBits( pBestDeviceSettingsCombo->BackBufferFormat );       
    UINT dwInputDepthBitDepth = 0;
    if( pDeviceSettingsIn )
        dwInputDepthBitDepth = DepthBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

    for( uint32 i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
    {
        D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList[i];
        DWORD dwCurDepthBitDepth = DepthBits( curDepthStencilFmt );
        int nRanking;

        if( pMatchOptions->DepthFormat == EMatchType::PreserveInput )
        {                       
            // Need to match bit depth of input
            if(dwCurDepthBitDepth == dwInputDepthBitDepth)
                nRanking = 0;
            else
                nRanking = 10000;
        }
        else if( pMatchOptions->DepthFormat == EMatchType::IgnoreInput )
        {
            // Prefer match of backbuffer bit depth
            nRanking = abs((int)dwCurDepthBitDepth - (int)dwBackBufferBitDepth*4);
        }
        else // if( pMatchOptions->DepthFormat == EMatchType::ClosestToInput )
        {
            // Prefer match of input depth format bit depth
            nRanking = abs((int)dwCurDepthBitDepth - (int)dwInputDepthBitDepth);
        }

        depthStencilRanking.push_back( nRanking );
    }

    UINT dwInputStencilBitDepth = 0;
    if( pDeviceSettingsIn )
        dwInputStencilBitDepth = StencilBits( pDeviceSettingsIn->pp.AutoDepthStencilFormat );

    for( uint32 i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
    {
        D3DFORMAT curDepthStencilFmt = pBestDeviceSettingsCombo->depthStencilFormatList[i];
        int nRanking = depthStencilRanking[i];
        DWORD dwCurStencilBitDepth = StencilBits( curDepthStencilFmt );

        if( pMatchOptions->StencilFormat == EMatchType::PreserveInput )
        {                       
            // Need to match bit depth of input
            if(dwCurStencilBitDepth == dwInputStencilBitDepth)
                nRanking += 0;
            else
                nRanking += 10000;
        }
        else if( pMatchOptions->StencilFormat == EMatchType::IgnoreInput )
        {
            // Prefer 0 stencil bit depth
            nRanking += dwCurStencilBitDepth;
        }
        else // if( pMatchOptions->StencilFormat == EMatchType::ClosestToInput )
        {
            // Prefer match of input stencil format bit depth
            nRanking += abs((int)dwCurStencilBitDepth - (int)dwInputStencilBitDepth);
        }

        depthStencilRanking[i] = nRanking;
    }

    int nBestRanking = 100000;
    int nBestIndex = -1;
    for( uint32 i=0; i<pBestDeviceSettingsCombo->depthStencilFormatList.size(); i++ )
    {
        int nRanking = depthStencilRanking[i];
        if( nRanking < nBestRanking )
        {
            nBestRanking = nRanking;
            nBestIndex = i;
        }
    }

    if( nBestIndex >= 0 )
    {
        bestDepthStencilFormat = pBestDeviceSettingsCombo->depthStencilFormatList[nBestIndex];
        bestEnableAutoDepthStencil = true;
    }
    else
    {
        bestDepthStencilFormat = D3DFMT_UNKNOWN;
        bestEnableAutoDepthStencil = false;
    }


    //---------------------
    // Present flags
    //---------------------
    DWORD dwBestFlags;
    if( pMatchOptions->PresentFlags == EMatchType::PreserveInput )   
    {
        dwBestFlags = pDeviceSettingsIn->pp.Flags;
    }
    else if( pMatchOptions->PresentFlags == EMatchType::IgnoreInput )   
    {
        dwBestFlags = 0;
        if( bestEnableAutoDepthStencil )
            dwBestFlags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;            
    }
    else // if( pMatchOptions->PresentFlags == EMatchType::ClosestToInput )   
    {
        dwBestFlags = pDeviceSettingsIn->pp.Flags;
        if( bestEnableAutoDepthStencil )
            dwBestFlags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    }

    //---------------------
    // Refresh rate
    //---------------------
    if( pBestDeviceSettingsCombo->Windowed )
    {
        // Must be 0 for windowed
        bestDisplayMode.RefreshRate = 0;
    }
    else
    {
        if( pMatchOptions->RefreshRate == EMatchType::PreserveInput )   
        {
            bestDisplayMode.RefreshRate = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
        }
        else 
        {
            UINT refreshRateMatch;
            if( pMatchOptions->RefreshRate == EMatchType::ClosestToInput )   
            {
                refreshRateMatch = pDeviceSettingsIn->pp.FullScreen_RefreshRateInHz;
            }
            else // if( pMatchOptions->RefreshRate == EMatchType::IgnoreInput )   
            {
                refreshRateMatch = adapterDesktopDisplayMode.RefreshRate;
            }

            bestDisplayMode.RefreshRate = 0;

            if( refreshRateMatch != 0 )
            {
                int nBestRefreshRanking = 100000;
				std::vector<D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
                for( uint32 iDisplayMode=0; iDisplayMode<pDisplayModeList->size(); iDisplayMode++ )
                {
                    D3DDISPLAYMODE displayMode = (*pDisplayModeList)[iDisplayMode];                
                    if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat || 
                        displayMode.Height != bestDisplayMode.Height ||
                        displayMode.Width != bestDisplayMode.Width )
                        continue; // Skip display modes that don't match 

                    // Find the delta between the current refresh rate and the optimal refresh rate 
                    int nCurRanking = abs((int)displayMode.RefreshRate - (int)refreshRateMatch);
                                        
                    if( nCurRanking < nBestRefreshRanking )
                    {
                        bestDisplayMode.RefreshRate = displayMode.RefreshRate;
                        nBestRefreshRanking = nCurRanking;

                        // Stop if perfect match found
                        if( nBestRefreshRanking == 0 )
                            break;
                    }
                }
            }
        }
    }

    //---------------------
    // Present interval
    //---------------------
    UINT bestPresentInterval;
    if( pMatchOptions->PresentInterval == EMatchType::PreserveInput )   
    {
        bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
    }
    else if( pMatchOptions->PresentInterval == EMatchType::IgnoreInput )   
    {
        // For windowed and fullscreen, default to D3DPRESENT_INTERVAL_DEFAULT
        // which will wait for the vertical retrace period to prevent tearing.
        // For benchmarking, use D3DPRESENT_INTERVAL_DEFAULT  which will
        // will wait not for the vertical retrace period but may introduce tearing.
        bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }
    else // if( pMatchOptions->PresentInterval == EMatchType::ClosestToInput )   
    {
		if( ContainerContains ( pBestDeviceSettingsCombo->presentIntervalList , pDeviceSettingsIn->pp.PresentationInterval ) )
        {
            bestPresentInterval = pDeviceSettingsIn->pp.PresentationInterval;
        }
        else
        {
            bestPresentInterval = D3DPRESENT_INTERVAL_DEFAULT;
        }
    }

    // Fill the device settings struct
    ZeroMemory( pValidDeviceSettings, sizeof(DeviceSettings) );
    pValidDeviceSettings->AdapterOrdinal                 = pBestDeviceSettingsCombo->AdapterOrdinal;
    pValidDeviceSettings->DeviceType                     = pBestDeviceSettingsCombo->DeviceType;
    pValidDeviceSettings->AdapterFormat                  = pBestDeviceSettingsCombo->AdapterFormat;
    pValidDeviceSettings->BehaviorFlags                  = dwBestBehaviorFlags;
    pValidDeviceSettings->pp.BackBufferWidth             = bestDisplayMode.Width;
    pValidDeviceSettings->pp.BackBufferHeight            = bestDisplayMode.Height;
    pValidDeviceSettings->pp.BackBufferFormat            = pBestDeviceSettingsCombo->BackBufferFormat;
    pValidDeviceSettings->pp.BackBufferCount             = bestBackBufferCount;
    pValidDeviceSettings->pp.MultiSampleType             = bestMultiSampleType;  
    pValidDeviceSettings->pp.MultiSampleQuality          = bestMultiSampleQuality;
    pValidDeviceSettings->pp.SwapEffect                  = bestSwapEffect;
    pValidDeviceSettings->pp.hDeviceWindow               = pBestDeviceSettingsCombo->Windowed ? GetHWNDDeviceWindowed() : GetHWNDDeviceFullScreen();
    pValidDeviceSettings->pp.Windowed                    = pBestDeviceSettingsCombo->Windowed;
    pValidDeviceSettings->pp.EnableAutoDepthStencil      = bestEnableAutoDepthStencil;  
    pValidDeviceSettings->pp.AutoDepthStencilFormat      = bestDepthStencilFormat;
    pValidDeviceSettings->pp.Flags                       = dwBestFlags;                   
    pValidDeviceSettings->pp.FullScreen_RefreshRateInHz  = bestDisplayMode.RefreshRate;
    pValidDeviceSettings->pp.PresentationInterval        = bestPresentInterval;
}


//--------------------------------------------------------------------------------------
// Internal helper function to find the closest allowed display mode to the optimal 
//--------------------------------------------------------------------------------------
HRESULT CApplication::FindValidResolution( CDeviceEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, 
                                D3DDISPLAYMODE displayModeIn, D3DDISPLAYMODE* pBestDisplayMode )
{
    D3DDISPLAYMODE bestDisplayMode;
    ZeroMemory( &bestDisplayMode, sizeof(D3DDISPLAYMODE) );
    
    if( pBestDeviceSettingsCombo->Windowed )
    {
        // In windowed mode, all resolutions are valid but restritions still apply 
        // on the size of the window.  See ChangeDevice() for details
        *pBestDisplayMode = displayModeIn;
    }
    else
    {
        int nBestRanking = 100000;
        int nCurRanking;
		std::vector<D3DDISPLAYMODE>* pDisplayModeList = &pBestDeviceSettingsCombo->pAdapterInfo->displayModeList;
        for( uint32 iDisplayMode=0; iDisplayMode<pDisplayModeList->size(); iDisplayMode++ )
        {
            D3DDISPLAYMODE displayMode = (*pDisplayModeList)[iDisplayMode];

            // Skip display modes that don't match the combo's adapter format
            if( displayMode.Format != pBestDeviceSettingsCombo->AdapterFormat )
                continue;

            // Find the delta between the current width/height and the optimal width/height
            nCurRanking = abs((int)displayMode.Width - (int)displayModeIn.Width) + 
                          abs((int)displayMode.Height- (int)displayModeIn.Height);

            if( nCurRanking < nBestRanking )
            {
                bestDisplayMode = displayMode;
                nBestRanking = nCurRanking;

                // Stop if perfect match found
                if( nBestRanking == 0 )
                    break;
            }
        }

        if( bestDisplayMode.Width == 0 )
        {
            *pBestDisplayMode = displayModeIn;
            return E_FAIL; // No valid display modes found
        }

        *pBestDisplayMode = bestDisplayMode;
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Internal helper function to return the adapter format from the first device settings 
// combo that matches the passed adapter ordinal, device type, backbuffer format, and windowed.  
//--------------------------------------------------------------------------------------
HRESULT CApplication::FindAdapterFormat( UINT AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT BackBufferFormat, 
                               BOOL Windowed, D3DFORMAT* pAdapterFormat )
{
    CDeviceEnumeration* pd3dEnum = PrepareEnumerationObject();
    CDeviceEnumDeviceInfo* pDeviceInfo = pd3dEnum->GetDeviceInfo( AdapterOrdinal, DeviceType );
    if( pDeviceInfo )
    {
        for( uint32 iDeviceCombo=0; iDeviceCombo<pDeviceInfo->deviceSettingsComboList.size(); iDeviceCombo++ )
        {
            CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo = pDeviceInfo->deviceSettingsComboList[iDeviceCombo];
            if( pDeviceSettingsCombo->BackBufferFormat == BackBufferFormat &&
                pDeviceSettingsCombo->Windowed == Windowed )
            {
                // Return the adapter format from the first match
                *pAdapterFormat = pDeviceSettingsCombo->AdapterFormat;
                return S_OK;
            }
        }
    }

    *pAdapterFormat = BackBufferFormat;
    return E_FAIL;
}


//--------------------------------------------------------------------------------------
// Change to a Direct3D device created from the device settings or passed in.
// The framework will only reset if the device is similar to the previous device 
// otherwise it will cleanup the previous device (if there is one) and recreate the 
// scene using the app's device callbacks.
//--------------------------------------------------------------------------------------
HRESULT CApplication::ChangeDevice( DeviceSettings* pNewDeviceSettings, IDirect3DDevice9* pd3dDeviceFromApp, bool bForceRecreate, bool bClipWindowToSingleAdapter )
{
    HRESULT hr;
    DeviceSettings* pOldDeviceSettings = GetCurrentDeviceSettings();

    if( GetD3DObject() == NULL )
        return S_FALSE;

    // Make a copy of the pNewDeviceSettings on the heap
    DeviceSettings* pNewDeviceSettingsOnHeap = new DeviceSettings;
    if( pNewDeviceSettingsOnHeap == NULL )
        return E_OUTOFMEMORY;
    memcpy( pNewDeviceSettingsOnHeap, pNewDeviceSettings, sizeof(DeviceSettings) );
    pNewDeviceSettings = pNewDeviceSettingsOnHeap;

    D3DCAPS9 caps;
    IDirect3D9* pD3D = GetD3DObject();
    pD3D->GetDeviceCaps( pNewDeviceSettings->AdapterOrdinal, pNewDeviceSettings->DeviceType, &caps );

	bool bContinue = OnModifyDeviceSettings ( pNewDeviceSettings, &caps );

    if( !bContinue )
    {
        // The app rejected the device change by returning false, so just use the current device if there is one.
        if( pOldDeviceSettings == NULL )
            DisplayErrorMessage( ERR_NOCOMPATIBLEDEVICES );
        SafeDelete( pNewDeviceSettings );
        return E_ABORT;
    }
    if( GetD3D() == NULL )
    {
        SafeDelete( pNewDeviceSettings );
        return S_FALSE;
    }
//    }

    SetCurrentDeviceSettings( pNewDeviceSettings );

    Pause( true, true );

    // When a WM_SIZE message is received, it calls CheckForWindowSizeChange().
    // A WM_SIZE message might be sent when adjusting the window, so tell 
    // CheckForWindowSizeChange() to ignore size changes temporarily
    SetIgnoreSizeChange( true );

    // Update thread safety on/off depending on Direct3D device's thread safety
	SetThreadSafe( (pNewDeviceSettings->BehaviorFlags & D3DCREATE_MULTITHREADED) != 0 );

    // Only apply the cmd line overrides if this is the first device created
    // and SetDevice() isn't used
    if( NULL == pd3dDeviceFromApp && NULL == pOldDeviceSettings )
    {
        // Updates the device settings struct based on the cmd line args.  
        // Warning: if the device doesn't support these new settings then CreateDevice() will fail.
        UpdateDeviceSettingsWithOverrides( pNewDeviceSettings );
    }

    // Take note if the backbuffer width & height are 0 now as they will change after pd3dDevice->Reset()
    bool bKeepCurrentWindowSize = false;
    if( pNewDeviceSettings->pp.BackBufferWidth == 0 && pNewDeviceSettings->pp.BackBufferHeight == 0 )
        bKeepCurrentWindowSize = true;

    //////////////////////////
    // Before reset
    /////////////////////////
    if( pNewDeviceSettings->pp.Windowed )
    {
        // Going to windowed mode

        if( pOldDeviceSettings && !pOldDeviceSettings->pp.Windowed )
        {
            // Going from fullscreen -> windowed
            SetFullScreenBackBufferWidthAtModeChange( pOldDeviceSettings->pp.BackBufferWidth );
            SetFullScreenBackBufferHeightAtModeChange( pOldDeviceSettings->pp.BackBufferHeight );

            // Restore windowed mode style
            SetWindowLong( GetHWNDDeviceWindowed(), GWL_STYLE, GetWindowedStyleAtModeChange() );
        }

        // If different device windows are used for windowed mode and fullscreen mode,
        // hide the fullscreen window so that it doesn't obscure the screen.
        if( GetHWNDDeviceFullScreen() != GetHWNDDeviceWindowed() )
            ShowWindow( GetHWNDDeviceFullScreen(), SW_HIDE );

        // If using the same window for windowed and fullscreen mode, reattach menu if one exists
        if( GetHWNDDeviceFullScreen() == GetHWNDDeviceWindowed() )
        {
            if( GetMenu() != NULL )
				::SetMenu( GetHWNDDeviceWindowed(), GetMenu() );
        }
    }
    else 
    {
        // Going to fullscreen mode

        if( pOldDeviceSettings == NULL || (pOldDeviceSettings && pOldDeviceSettings->pp.Windowed) )
        {
            // Transistioning to full screen mode from a standard window so 
            // save current window position/size/style now in case the user toggles to windowed mode later 
            WINDOWPLACEMENT* pwp = GetWindowedPlacement();
            ZeroMemory( pwp, sizeof(WINDOWPLACEMENT) );
            pwp->length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement( GetHWNDDeviceWindowed(), pwp );
            bool bIsTopmost = ( (GetWindowLong(GetHWNDDeviceWindowed(),GWL_EXSTYLE) & WS_EX_TOPMOST) != 0); 
            SetTopmostWhileWindowed( bIsTopmost );
            DWORD dwStyle = GetWindowLong( GetHWNDDeviceWindowed(), GWL_STYLE );
            dwStyle &= ~WS_MAXIMIZE & ~WS_MINIMIZE; // remove minimize/maximize style
            SetWindowedStyleAtModeChange( dwStyle );
            if( pOldDeviceSettings )
            {
                SetWindowBackBufferWidthAtModeChange( pOldDeviceSettings->pp.BackBufferWidth );
                SetWindowBackBufferHeightAtModeChange( pOldDeviceSettings->pp.BackBufferHeight );
            }
        }

        // Hide the window to avoid animation of blank windows
        ShowWindow( GetHWNDDeviceFullScreen(), SW_HIDE );

        // Set FS window style
        SetWindowLong( GetHWNDDeviceFullScreen(), GWL_STYLE, WS_POPUP|WS_SYSMENU );

        // If using the same window for windowed and fullscreen mode, save and remove menu 
        if( GetHWNDDeviceFullScreen() == GetHWNDDeviceWindowed() )
        {
			HMENU hMenu = ::GetMenu( GetHWNDDeviceFullScreen() );
            SetMenu( hMenu );
			::SetMenu( GetHWNDDeviceFullScreen(), NULL );
        }
      
        WINDOWPLACEMENT wpFullscreen;
        ZeroMemory( &wpFullscreen, sizeof(WINDOWPLACEMENT) );
        wpFullscreen.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement( GetHWNDDeviceFullScreen(), &wpFullscreen );
        if( (wpFullscreen.flags & WPF_RESTORETOMAXIMIZED) != 0 )
        {
            // Restore the window to normal if the window was maximized then minimized.  This causes the 
            // WPF_RESTORETOMAXIMIZED flag to be set which will cause SW_RESTORE to restore the 
            // window from minimized to maxmized which isn't what we want
            wpFullscreen.flags &= ~WPF_RESTORETOMAXIMIZED;
            wpFullscreen.showCmd = SW_RESTORE;
            SetWindowPlacement( GetHWNDDeviceFullScreen(), &wpFullscreen );
        }
    }

    // If AdapterOrdinal and DeviceType are the same, we can just do a Reset().
    // If they've changed, we need to do a complete device tear down/rebuild.
    // Also only allow a reset if pd3dDevice is the same as the current device 
    if( !bForceRecreate && 
        (pd3dDeviceFromApp == NULL || pd3dDeviceFromApp == GetD3DDevice()) && 
        GetD3DDevice() &&
        pOldDeviceSettings &&
        pOldDeviceSettings->AdapterOrdinal == pNewDeviceSettings->AdapterOrdinal &&
        pOldDeviceSettings->DeviceType == pNewDeviceSettings->DeviceType &&
        pOldDeviceSettings->BehaviorFlags == pNewDeviceSettings->BehaviorFlags )
    {
        // Reset the Direct3D device and call the app's device callbacks
        hr = Reset3DEnvironment();
        if( FAILED(hr) )
        {
            if( D3DERR_DEVICELOST == hr )
            {
                // The device is lost, just mark it as so and continue on with 
                // capturing the state and resizing the window/etc.
                SetDeviceLost( true );
            }
            else if( ERR_RESETTINGDEVICEOBJECTS == hr || 
                     ERR_MEDIANOTFOUND == hr )
            {
                // Something bad happened in the app callbacks
                SafeDelete( pOldDeviceSettings );
                DisplayErrorMessage( hr );
                Shutdown();
                return hr;
            }
            else // ERR_RESETTINGDEVICE
            {
                // Reset failed and the device wasn't lost and it wasn't the apps fault, 
                // so recreate the device to try to recover
                SetCurrentDeviceSettings( pOldDeviceSettings );
                if( FAILED( ChangeDevice( pNewDeviceSettings, pd3dDeviceFromApp, true, bClipWindowToSingleAdapter ) ) )
                {
                    // If that fails, then shutdown
                    SafeDelete( pOldDeviceSettings );
                    Shutdown();
                    return ERR_CREATINGDEVICE;
                }
                else
                {
                    SafeDelete( pOldDeviceSettings );
                    return S_OK;
                }
            }
        }
    }
    else
    {
        // Cleanup if not first device created
        if( pOldDeviceSettings )
            Cleanup3DEnvironment( false );

        // Create the D3D device and call the app's device callbacks
        hr = Create3DEnvironment( pd3dDeviceFromApp );
        if( FAILED(hr) )
        {
            SafeDelete( pOldDeviceSettings );
            Cleanup3DEnvironment();
            DisplayErrorMessage( hr );
            Pause( false, false );
            SetIgnoreSizeChange( false );
            return hr;
        }
    }

    // Enable/disable StickKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut, and Windows key 
    // to prevent accidental task switching
    AllowShortcutKeys( ( pNewDeviceSettings->pp.Windowed  ) ? GetAllowShortcutKeysWhenWindowed() : GetAllowShortcutKeysWhenFullscreen() );

    HMONITOR hAdapterMonitor = pD3D->GetAdapterMonitor( pNewDeviceSettings->AdapterOrdinal );
    SetAdapterMonitor( hAdapterMonitor );

    // Update the device stats text
    UpdateStaticFrameStats();

    if( pOldDeviceSettings && !pOldDeviceSettings->pp.Windowed && pNewDeviceSettings->pp.Windowed )
    {
        // Going from fullscreen -> windowed

        // Restore the show state, and positions/size of the window to what it was
        // It is important to adjust the window size 
        // after resetting the device rather than beforehand to ensure 
        // that the monitor resolution is correct and does not limit the size of the new window.
        WINDOWPLACEMENT* pwp = GetWindowedPlacement();
        SetWindowPlacement( GetHWNDDeviceWindowed(), pwp );

        // Also restore the z-order of window to previous state
        HWND hWndInsertAfter = GetTopmostWhileWindowed() ? HWND_TOPMOST : HWND_NOTOPMOST;
        SetWindowPos( GetHWNDDeviceWindowed(), hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOREDRAW|SWP_NOSIZE );
    }

    // Check to see if the window needs to be resized.  
    // Handle cases where the window is minimized and maxmimized as well.
    bool bNeedToResize = false;
    if( pNewDeviceSettings->pp.Windowed && // only resize if in windowed mode
        !bKeepCurrentWindowSize )          // only resize if pp.BackbufferWidth/Height were not 0
    {
        UINT nClientWidth;
        UINT nClientHeight;    
        if( IsIconic(GetHWNDDeviceWindowed()) )
        {
            // Window is currently minimized. To tell if it needs to resize, 
            // get the client rect of window when its restored the 
            // hard way using GetWindowPlacement()
            WINDOWPLACEMENT wp;
            ZeroMemory( &wp, sizeof(WINDOWPLACEMENT) );
            wp.length = sizeof(WINDOWPLACEMENT);
            GetWindowPlacement( GetHWNDDeviceWindowed(), &wp );

            if( (wp.flags & WPF_RESTORETOMAXIMIZED) != 0 && wp.showCmd == SW_SHOWMINIMIZED )
            {
                // WPF_RESTORETOMAXIMIZED means that when the window is restored it will
                // be maximized.  So maximize the window temporarily to get the client rect 
                // when the window is maximized.  GetSystemMetrics( SM_CXMAXIMIZED ) will give this 
                // information if the window is on the primary but this will work on multimon.
                ShowWindow( GetHWNDDeviceWindowed(), SW_RESTORE );
                RECT rcClient;
                GetClientRect( GetHWNDDeviceWindowed(), &rcClient );
                nClientWidth  = (UINT)(rcClient.right - rcClient.left);
                nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
                ShowWindow( GetHWNDDeviceWindowed(), SW_MINIMIZE );
            }
            else
            {
                // Use wp.rcNormalPosition to get the client rect, but wp.rcNormalPosition 
                // includes the window frame so subtract it
                RECT rcFrame = {0};
                AdjustWindowRect( &rcFrame, GetWindowedStyleAtModeChange(), GetMenu() != NULL );
                LONG nFrameWidth = rcFrame.right - rcFrame.left;
                LONG nFrameHeight = rcFrame.bottom - rcFrame.top;
                nClientWidth  = (UINT)(wp.rcNormalPosition.right - wp.rcNormalPosition.left - nFrameWidth);
                nClientHeight = (UINT)(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top - nFrameHeight);
            }
        }
        else
        {
            // Window is restored or maximized so just get its client rect
            RECT rcClient;
            GetClientRect( GetHWNDDeviceWindowed(), &rcClient );
            nClientWidth  = (UINT)(rcClient.right - rcClient.left);
            nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
        }

        // Now that we know the client rect, compare it against the back buffer size
        // to see if the client rect is already the right size
        if( nClientWidth  != pNewDeviceSettings->pp.BackBufferWidth ||
            nClientHeight != pNewDeviceSettings->pp.BackBufferHeight )
        {
            bNeedToResize = true;
        }       

        if( bClipWindowToSingleAdapter && !IsIconic(GetHWNDDeviceWindowed()) )
        {
            // Get the rect of the monitor attached to the adapter
            MONITORINFO miAdapter;
            miAdapter.cbSize = sizeof(MONITORINFO);
            HMONITOR hAdapterMonitor = GetD3DObject()->GetAdapterMonitor( pNewDeviceSettings->AdapterOrdinal );
            DynamicGetMonitorInfo( hAdapterMonitor, &miAdapter );
            HMONITOR hWindowMonitor = DynamicMonitorFromWindow( GetHWND(), MONITOR_DEFAULTTOPRIMARY );

            // Get the rect of the window
            RECT rcWindow;
            GetWindowRect( GetHWNDDeviceWindowed(), &rcWindow );

            // Check if the window rect is fully inside the adapter's vitural screen rect
            if( (rcWindow.left   < miAdapter.rcWork.left  ||
                 rcWindow.right  > miAdapter.rcWork.right ||
                 rcWindow.top    < miAdapter.rcWork.top   ||
                 rcWindow.bottom > miAdapter.rcWork.bottom) )
            {
                if( hWindowMonitor == hAdapterMonitor && IsZoomed(GetHWNDDeviceWindowed()) )
                {
                    // If the window is maximized and on the same monitor as the adapter, then 
                    // no need to clip to single adapter as the window is already clipped 
                    // even though the rcWindow rect is outside of the miAdapter.rcWork
                }
                else
                {
                    bNeedToResize = true;
                }
            }
        }
    }

    // Only resize window if needed 
    if( bNeedToResize ) 
    {
        // Need to resize, so if window is maximized or minimized then restore the window
        if( IsIconic(GetHWNDDeviceWindowed()) ) 
            ShowWindow( GetHWNDDeviceWindowed(), SW_RESTORE );
        if( IsZoomed(GetHWNDDeviceWindowed()) ) // doing the IsIconic() check first also handles the WPF_RESTORETOMAXIMIZED case
            ShowWindow( GetHWNDDeviceWindowed(), SW_RESTORE );

        if( bClipWindowToSingleAdapter )
        {
            // Get the rect of the monitor attached to the adapter
            MONITORINFO miAdapter;
            miAdapter.cbSize = sizeof(MONITORINFO);
            DynamicGetMonitorInfo( GetD3DObject()->GetAdapterMonitor( pNewDeviceSettings->AdapterOrdinal ), &miAdapter );

            // Get the rect of the monitor attached to the window
            MONITORINFO miWindow;
            miWindow.cbSize = sizeof(MONITORINFO);
            DynamicGetMonitorInfo( DynamicMonitorFromWindow( GetHWND(), MONITOR_DEFAULTTOPRIMARY ), &miWindow );

            // Do something reasonable if the BackBuffer size is greater than the monitor size
            int nAdapterMonitorWidth = miAdapter.rcWork.right - miAdapter.rcWork.left;
            int nAdapterMonitorHeight = miAdapter.rcWork.bottom - miAdapter.rcWork.top;

            int nClientWidth = pNewDeviceSettings->pp.BackBufferWidth;
            int nClientHeight = pNewDeviceSettings->pp.BackBufferHeight;

            // Get the rect of the window
            RECT rcWindow;
            GetWindowRect( GetHWNDDeviceWindowed(), &rcWindow );

            // Make a window rect with a client rect that is the same size as the backbuffer
            RECT rcResizedWindow;
            rcResizedWindow.left = 0;
            rcResizedWindow.right = nClientWidth;
            rcResizedWindow.top = 0;
            rcResizedWindow.bottom = nClientHeight;
            AdjustWindowRect( &rcResizedWindow, GetWindowLong( GetHWNDDeviceWindowed(), GWL_STYLE ), GetMenu() != NULL );

            int nWindowWidth = rcResizedWindow.right - rcResizedWindow.left;
            int nWindowHeight = rcResizedWindow.bottom - rcResizedWindow.top;

            if( nWindowWidth > nAdapterMonitorWidth )
                nWindowWidth = (nAdapterMonitorWidth - 0);
            if( nWindowHeight > nAdapterMonitorHeight )
                nWindowHeight = (nAdapterMonitorHeight - 0);

            if( rcResizedWindow.left < miAdapter.rcWork.left ||
                rcResizedWindow.top < miAdapter.rcWork.top ||
                rcResizedWindow.right > miAdapter.rcWork.right ||
                rcResizedWindow.bottom > miAdapter.rcWork.bottom )
            {
                int nWindowOffsetX = (nAdapterMonitorWidth - nWindowWidth) / 2;
                int nWindowOffsetY = (nAdapterMonitorHeight - nWindowHeight) / 2;

                rcResizedWindow.left = miAdapter.rcWork.left + nWindowOffsetX;
                rcResizedWindow.top = miAdapter.rcWork.top + nWindowOffsetY;
                rcResizedWindow.right = miAdapter.rcWork.left + nWindowOffsetX + nWindowWidth;
                rcResizedWindow.bottom = miAdapter.rcWork.top + nWindowOffsetY + nWindowHeight;
            }

            // Resize the window.  It is important to adjust the window size 
            // after resetting the device rather than beforehand to ensure 
            // that the monitor resolution is correct and does not limit the size of the new window.
            SetWindowPos( GetHWNDDeviceWindowed(), 0, rcResizedWindow.left, rcResizedWindow.top, nWindowWidth, nWindowHeight, SWP_NOZORDER );
        }        
        else
        {      
            // Make a window rect with a client rect that is the same size as the backbuffer
            RECT rcWindow = {0};
            rcWindow.right = (long)(pNewDeviceSettings->pp.BackBufferWidth);
            rcWindow.bottom = (long)(pNewDeviceSettings->pp.BackBufferHeight);
            AdjustWindowRect( &rcWindow, GetWindowLong( GetHWNDDeviceWindowed(), GWL_STYLE ), GetMenu() != NULL );

            // Resize the window.  It is important to adjust the window size 
            // after resetting the device rather than beforehand to ensure 
            // that the monitor resolution is correct and does not limit the size of the new window.
            int cx = (int)(rcWindow.right - rcWindow.left);
            int cy = (int)(rcWindow.bottom - rcWindow.top);
            SetWindowPos( GetHWNDDeviceWindowed(), 0, 0, 0, cx, cy, SWP_NOZORDER|SWP_NOMOVE );
        }

        // Its possible that the new window size is not what we asked for.  
        // No window can be sized larger than the desktop, so see see if the Windows OS resized the 
        // window to something smaller to fit on the desktop.  Also if WM_GETMINMAXINFO
        // will put a limit on the smallest/largest window size.
        RECT rcClient;
        GetClientRect( GetHWNDDeviceWindowed(), &rcClient );
        UINT nClientWidth  = (UINT)(rcClient.right - rcClient.left);
        UINT nClientHeight = (UINT)(rcClient.bottom - rcClient.top);
        if( nClientWidth  != pNewDeviceSettings->pp.BackBufferWidth ||
            nClientHeight != pNewDeviceSettings->pp.BackBufferHeight )
        {
            // If its different, then resize the backbuffer again.  This time create a backbuffer that matches the 
            // client rect of the current window w/o resizing the window.
            DeviceSettings deviceSettings = GetDeviceSettings();
            deviceSettings.pp.BackBufferWidth  = 0; 
            deviceSettings.pp.BackBufferHeight = 0;
            hr = ChangeDevice( &deviceSettings, NULL, false, bClipWindowToSingleAdapter );
            if( FAILED( hr ) )
            {
                SafeDelete( pOldDeviceSettings );
                Cleanup3DEnvironment();
                Pause( false, false );
                SetIgnoreSizeChange( false );
                return hr;
            }
        }
    }

    // Make the window visible
    if( !IsWindowVisible( GetHWND() ) )
        ShowWindow( GetHWND(), SW_SHOW );

    SafeDelete( pOldDeviceSettings );
    SetIgnoreSizeChange( false );
    Pause( false, false );
    SetDeviceCreated( true );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Low level keyboard hook to disable Windows key to prevent accidental task switching.  
//--------------------------------------------------------------------------------------
LRESULT CALLBACK CApplication::LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if (nCode < 0 || nCode != HC_ACTION)  // do not process message 
        return CallNextHookEx( GetApplication()->GetKeyboardHook(), nCode, wParam, lParam); 

    bool bEatKeystroke = false;
    KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
    switch (wParam) 
    {
        case WM_KEYDOWN:  
        case WM_KEYUP:    
        {
            bEatKeystroke = ( !GetApplication()->GetAllowShortcutKeys() && 
                            (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN) );
            break;
        }
    }

    if( bEatKeystroke )
        return 1;
    else
        return CallNextHookEx( GetApplication()->GetKeyboardHook(), nCode, wParam, lParam );
}



//--------------------------------------------------------------------------------------
// Controls how  behaves when fullscreen and windowed mode with regard to 
// shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
//--------------------------------------------------------------------------------------
void CApplication::SetShortcutKeySettings( bool bAllowWhenFullscreen, bool bAllowWhenWindowed )
{
    SetAllowShortcutKeysWhenWindowed( bAllowWhenWindowed );
    SetAllowShortcutKeysWhenFullscreen( bAllowWhenFullscreen );

    // Init() records initial accessibility states so don't change them until then
    if( GetInited() )
    {
        if( IsWindowed() )
            AllowShortcutKeys( GetAllowShortcutKeysWhenWindowed() );
        else
            AllowShortcutKeys( GetAllowShortcutKeysWhenFullscreen() );
    }
}


//--------------------------------------------------------------------------------------
// Enables/disables Windows keys, and disables or restores the StickyKeys/ToggleKeys/FilterKeys 
// shortcut to help prevent accidental task switching
//--------------------------------------------------------------------------------------
void CApplication::AllowShortcutKeys( bool bAllowKeys )
{
    SetAllowShortcutKeys( bAllowKeys );

    if( bAllowKeys )
    {
        // Restore StickyKeys/etc to original state and enable Windows key      
        STICKYKEYS sk = GetStartupStickyKeys();
        TOGGLEKEYS tk = GetStartupToggleKeys();
        FILTERKEYS fk = GetStartupFilterKeys();
        
        SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &sk, 0);
        SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tk, 0);
        SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fk, 0);

        // Remove the keyboard hoook when it isn't needed to prevent any slow down of other apps
        if( GetKeyboardHook() )
        {
            UnhookWindowsHookEx( GetKeyboardHook() );
            SetKeyboardHook( NULL );
        }                
    }
    else
    {
        // Set low level keyboard hook if haven't already
        if( GetKeyboardHook() == NULL )
        {
            // Set the low-level hook procedure.  Only works on Windows 2000 and above
            OSVERSIONINFO OSVersionInfo;
            OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVersionInfo);
            GetVersionEx(&OSVersionInfo);
            if( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && OSVersionInfo.dwMajorVersion > 4 )
            {
                HHOOK hKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0 );
                SetKeyboardHook( hKeyboardHook );
            }
        }

        // Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
        // then leave the settings alone as its probably being usefully used

        STICKYKEYS skOff = GetStartupStickyKeys();
        if( (skOff.dwFlags & SKF_STICKYKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
            skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
        }

        TOGGLEKEYS tkOff = GetStartupToggleKeys();
        if( (tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
            tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
        }

        FILTERKEYS fkOff = GetStartupFilterKeys();
        if( (fkOff.dwFlags & FKF_FILTERKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
            fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
        }
    }
}


//--------------------------------------------------------------------------------------
// Updates the device settings struct based on the cmd line args.  
//--------------------------------------------------------------------------------------
void CApplication::UpdateDeviceSettingsWithOverrides( DeviceSettings* pDeviceSettings )
{
    if( GetOverrideAdapterOrdinal() != -1 )
        pDeviceSettings->AdapterOrdinal = GetOverrideAdapterOrdinal();

    if( GetOverrideFullScreen() )
        pDeviceSettings->pp.Windowed = false;
    if( GetOverrideWindowed() )
        pDeviceSettings->pp.Windowed = true;

    if( GetOverrideForceREF() )
        pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
    else if( GetOverrideForceHAL() )
        pDeviceSettings->DeviceType = D3DDEVTYPE_HAL;

    if( GetOverrideWidth() != 0 )
        pDeviceSettings->pp.BackBufferWidth = GetOverrideWidth();
    if( GetOverrideHeight() != 0 )
        pDeviceSettings->pp.BackBufferHeight = GetOverrideHeight();

    if( GetOverrideForcePureHWVP() )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_PUREDEVICE;
    }
    else if( GetOverrideForceHWVP() )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }
    else if( GetOverrideForceSWVP() )
    {
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
}


//--------------------------------------------------------------------------------------
// Creates the 3D environment
//--------------------------------------------------------------------------------------
HRESULT CApplication::Create3DEnvironment( IDirect3DDevice9* pd3dDeviceFromApp )
{
    HRESULT hr = S_OK;

    IDirect3DDevice9* pd3dDevice = NULL;
    DeviceSettings* pNewDeviceSettings = GetCurrentDeviceSettings();

    // Only create a Direct3D device if one hasn't been supplied by the app
    if( pd3dDeviceFromApp == NULL )
    {
        // Try to create the device with the chosen settings
        IDirect3D9* pD3D = GetD3DObject();
        hr = pD3D->CreateDevice( pNewDeviceSettings->AdapterOrdinal, pNewDeviceSettings->DeviceType, 
                                 GetHWNDFocus(), pNewDeviceSettings->BehaviorFlags,
                                 &pNewDeviceSettings->pp, &pd3dDevice );
        if( hr == D3DERR_DEVICELOST ) 
        {
            SetDeviceLost( true );
            return S_OK;
        }
        else if( FAILED(hr) )
        {
            _ERR( L"CreateDevice", hr );
            return ERR_CREATINGDEVICE;
        }
    }
    else
    {
        pd3dDeviceFromApp->AddRef();
        pd3dDevice = pd3dDeviceFromApp;
    }

    SetD3DDevice( pd3dDevice );

    // If switching to REF, set the exit code to 11.  If switching to HAL and exit code was 11, then set it back to 0.
    if( pNewDeviceSettings->DeviceType == D3DDEVTYPE_REF && GetExitCode() == 0 )
        SetExitCode(11);
    else if( pNewDeviceSettings->DeviceType == D3DDEVTYPE_HAL && GetExitCode() == 11 )
        SetExitCode(0);

    // Update back buffer desc before calling app's device callbacks
    UpdateBackBufferDesc();

    // Setup cursor based on current settings (window/fullscreen mode, show cursor state, clip cursor state)
    SetupCursor();

    // Update GetState()'s copy of D3D caps 
    D3DCAPS9* pd3dCaps = GetCaps();
    GetD3DDevice()->GetDeviceCaps( pd3dCaps );

    // Update the device stats text
    CDeviceEnumeration* pd3dEnum = PrepareEnumerationObject();
    CDeviceEnumAdapterInfo* pAdapterInfo = pd3dEnum->GetAdapterInfo( pNewDeviceSettings->AdapterOrdinal );
    UpdateDeviceStats( pNewDeviceSettings->DeviceType, 
                        pNewDeviceSettings->BehaviorFlags, 
                        &pAdapterInfo->AdapterIdentifier );


	// create our frame stats objects
	CreateStatusDisplayObjects();

    const D3DSURFACE_DESC* pbackBufferSurfaceDesc = GetBackBufferSurfaceDesc();
    SetInsideDeviceCallback( true );
    hr = OnDeviceCreated( GetD3DDevice(), pbackBufferSurfaceDesc );
    SetInsideDeviceCallback( false );
    if( GetD3DDevice() == NULL ) // Handle Shutdown from inside callback
        return E_FAIL;
    if( FAILED(hr) )  
    {
        _ERR( L"DeviceCreated callback", hr );        
        return ( hr == ERR_MEDIANOTFOUND ) ? ERR_MEDIANOTFOUND : ERR_CREATINGDEVICEOBJECTS;
    }
    SetDeviceObjectsCreated( true );

    // Call the app's device reset callback if non-NULL
    SetInsideDeviceCallback( true );
    hr = OnDeviceReset ( GetD3DDevice(), pbackBufferSurfaceDesc );
    SetInsideDeviceCallback( false );
    if( GetD3DDevice() == NULL ) // Handle Shutdown from inside callback
        return E_FAIL;
    if( FAILED(hr) )
    {
        _ERR( L"DeviceReset callback", hr );
        return ( hr == ERR_MEDIANOTFOUND ) ? ERR_MEDIANOTFOUND : ERR_RESETTINGDEVICEOBJECTS;
    }
    SetDeviceObjectsReset( true );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Resets the 3D environment by:
//      - Calls the device lost callback 
//      - Resets the device
//      - Stores the back buffer description
//      - Sets up the full screen Direct3D cursor if requested
//      - Calls the device reset callback 
//--------------------------------------------------------------------------------------
HRESULT CApplication::Reset3DEnvironment()
{
    HRESULT hr;

    IDirect3DDevice9* pd3dDevice = GetD3DDevice();
    assert( pd3dDevice != NULL );

    // Call the app's device lost callback
    if( GetDeviceObjectsReset() == true )
    {
        SetInsideDeviceCallback( true );
		OnDeviceLost();
        SetDeviceObjectsReset( false );
        SetInsideDeviceCallback( false );
    }

    // Reset the device
    DeviceSettings* pDeviceSettings = GetCurrentDeviceSettings();
    hr = pd3dDevice->Reset( &pDeviceSettings->pp );
    if( FAILED(hr) )  
    {
        if( hr == D3DERR_DEVICELOST )
            return D3DERR_DEVICELOST; // Reset could legitimately fail if the device is lost
        else
            return _ERR( L"Reset", ERR_RESETTINGDEVICE );
    }

    // Update back buffer desc before calling app's device callbacks
    UpdateBackBufferDesc();

    // Setup cursor based on current settings (window/fullscreen mode, show cursor state, clip cursor state)
    SetupCursor();

    // Call the app's OnDeviceReset callback
    SetInsideDeviceCallback( true );
    const D3DSURFACE_DESC* pbackBufferSurfaceDesc = GetBackBufferSurfaceDesc();
    hr = OnDeviceReset( pd3dDevice, pbackBufferSurfaceDesc );
    SetInsideDeviceCallback( false );
    if( FAILED(hr) )
    {
        // If callback failed, cleanup
        _ERR( L"DeviceResetCallback", hr );
        if( hr != ERR_MEDIANOTFOUND )
            hr = ERR_RESETTINGDEVICEOBJECTS;

        SetInsideDeviceCallback( true );
		OnDeviceLost();
        SetInsideDeviceCallback( false );

        return hr;
    }

    // Success
    SetDeviceObjectsReset( true );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Pauses time or rendering.  Keeps a ref count so pausing can be layered
//--------------------------------------------------------------------------------------
void CApplication::Pause( bool bPauseTime, bool bPauseRendering )
{
    int nPauseTimeCount = GetPauseTimeCount();
    nPauseTimeCount += ( bPauseTime ? +1 : -1 );
    if( nPauseTimeCount < 0 )
        nPauseTimeCount = 0;
    SetPauseTimeCount( nPauseTimeCount );

    int nPauseRenderingCount = GetPauseRenderingCount();
    nPauseRenderingCount += ( bPauseRendering ? +1 : -1 );
    if( nPauseRenderingCount < 0 )
        nPauseRenderingCount = 0;
    SetPauseRenderingCount( nPauseRenderingCount );

    if( nPauseTimeCount > 0 )
    {
        // Stop the scene from animating
        GetTimer()->Stop();
    }
    else
    {
        // Restart the timer
        GetTimer()->Start();
    }

    SetRenderingPaused( nPauseRenderingCount > 0 );
    SetTimePaused( nPauseTimeCount > 0 );
}


//--------------------------------------------------------------------------------------
// Checks if the window client rect has changed and if it has, then reset the device
//--------------------------------------------------------------------------------------
void CApplication::CheckForWindowSizeChange()
{
    // Skip the check for various reasons
    if( GetIgnoreSizeChange() || 
        !GetDeviceCreated() || 
        !GetCurrentDeviceSettings()->pp.Windowed )
        return;

    RECT rcCurrentClient;
    GetClientRect( GetHWND(), &rcCurrentClient );
    
    if( (UINT)rcCurrentClient.right != GetCurrentDeviceSettings()->pp.BackBufferWidth ||
        (UINT)rcCurrentClient.bottom != GetCurrentDeviceSettings()->pp.BackBufferHeight )
    {
        // A new window size will require a new backbuffer size
        // size, so the device must be reset and the D3D structures updated accordingly.

        // Tell ChangeDevice and D3D to size according to the HWND's client rect
        DeviceSettings deviceSettings = GetDeviceSettings();
        deviceSettings.pp.BackBufferWidth  = 0; 
        deviceSettings.pp.BackBufferHeight = 0;
        ChangeDevice( &deviceSettings, NULL, false, false );
    }
}


//--------------------------------------------------------------------------------------
// Handles app's message loop and rendering when idle.  If CreateDevice*() or SetDevice() 
// has not already been called, it will call CreateWindow() with the default parameters.  
//--------------------------------------------------------------------------------------
HRESULT CApplication::MainLoop( HACCEL hAccel )
{
    HRESULT hr;

    // Not allowed to call this from inside the device callbacks or reenter
    if( GetInsideDeviceCallback() || GetInsideMainloop() )
    {
        if( (GetExitCode() == 0) || (GetExitCode() == 11) )
            SetExitCode(1);
        return _ERR_MSGBOX( L"MainLoop", E_FAIL );
    }

    SetInsideMainloop( true );

    // If CreateDevice*() or SetDevice() has not already been called, 
    // then call CreateDevice() with the default parameters.         
    if( !GetDeviceCreated() ) 
    {
        if( GetDeviceCreateCalled() )
        {
            if( (GetExitCode() == 0) || (GetExitCode() == 11) )
                SetExitCode(1);
            return E_FAIL; // CreateDevice() must first succeed for this function to succeed
        }

        hr = CreateDevice();
        if( FAILED(hr) )
        {
            if( (GetExitCode() == 0) || (GetExitCode() == 11) )
                SetExitCode(1);
            return hr;
        }
    }

    HWND hWnd = GetHWND();

    // Init() must have been called and succeeded for this function to proceed
    // CreateWindow() or SetWindow() must have been called and succeeded for this function to proceed
    // CreateDevice() or CreateDeviceFromSettings() or SetDevice() must have been called and succeeded for this function to proceed
    if( !GetInited() || !GetWindowCreated() || !GetDeviceCreated() )
    {
        if( (GetExitCode() == 0) || (GetExitCode() == 11) )
            SetExitCode(1);
        return _ERR_MSGBOX( L"MainLoop", E_FAIL );
    }

    // Now we're ready to receive and process Windows messages.
    bool bGotMsg;
    MSG  msg;
    msg.message = WM_NULL;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() so we can use idle time to render the scene. 
        bGotMsg = ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) != 0 );

        if( bGotMsg )
        {
            // Translate and dispatch the message
            if( hAccel == NULL || hWnd == NULL || 
                0 == TranslateAccelerator( hWnd, hAccel, &msg ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            // Render a frame during idle time (no messages are waiting)
            Render3DEnvironment();
        }
    }

    // Cleanup the accelerator table
    if( hAccel != NULL )
        DestroyAcceleratorTable( hAccel );

    SetInsideMainloop( false );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the 3D environment by:
//      - Checking if the device is lost and trying to reset it if it is
//      - Get the elapsed time since the last frame
//      - Calling the app's framemove and render callback
//      - Calling Present()
//--------------------------------------------------------------------------------------
void CApplication::Render3DEnvironment()
{
    HRESULT hr;
   
    if( GetDeviceLost() || IsRenderingPaused() || !IsActive() )
    {
        // Window is minimized or paused so yield CPU time to other processes
        Sleep( 50 ); 
    }

    IDirect3DDevice9* pd3dDevice = GetD3DDevice();
    if( NULL == pd3dDevice )
    {
        if( GetDeviceLost() )
        {
            DeviceSettings deviceSettings = GetDeviceSettings();
            ChangeDevice( &deviceSettings, NULL, false, true );
        }

        return;
    }
 
    if( GetDeviceLost() && !GetRenderingPaused() )
    {
        // Test the cooperative level to see if it's okay to render
        if( FAILED( hr = pd3dDevice->TestCooperativeLevel() ) )
        {
            if( D3DERR_DEVICELOST == hr )
            {
                // The device has been lost but cannot be reset at this time.  
				// So wait until it can be reset.
                return;
            }

            // If we are windowed, read the desktop format and 
            // ensure that the Direct3D device is using the same format 
            // since the user could have changed the desktop bitdepth 
            if( IsWindowed() )
            {
                D3DDISPLAYMODE adapterDesktopDisplayMode;
                IDirect3D9* pD3D = GetD3DObject();
                DeviceSettings* pDeviceSettings = GetCurrentDeviceSettings();
                pD3D->GetAdapterDisplayMode( pDeviceSettings->AdapterOrdinal, &adapterDesktopDisplayMode );
                if( pDeviceSettings->AdapterFormat != adapterDesktopDisplayMode.Format )
                {
                    MatchOptions matchOptions;
                    matchOptions.AdapterOrdinal     = EMatchType::PreserveInput;
                    matchOptions.DeviceType         = EMatchType::PreserveInput;
                    matchOptions.Windowed           = EMatchType::PreserveInput;
                    matchOptions.AdapterFormat      = EMatchType::PreserveInput;
                    matchOptions.VertexProcessing   = EMatchType::ClosestToInput;
                    matchOptions.Resolution         = EMatchType::ClosestToInput;
                    matchOptions.BackBufferFormat   = EMatchType::ClosestToInput;
                    matchOptions.BackBufferCount    = EMatchType::ClosestToInput;
                    matchOptions.MultiSample        = EMatchType::ClosestToInput;
                    matchOptions.SwapEffect         = EMatchType::ClosestToInput;
                    matchOptions.DepthFormat        = EMatchType::ClosestToInput;
                    matchOptions.StencilFormat      = EMatchType::ClosestToInput;
                    matchOptions.PresentFlags       = EMatchType::ClosestToInput;
                    matchOptions.RefreshRate        = EMatchType::ClosestToInput;
                    matchOptions.PresentInterval    = EMatchType::ClosestToInput;

                    DeviceSettings deviceSettings = GetDeviceSettings();
                    deviceSettings.AdapterFormat = adapterDesktopDisplayMode.Format;

                    hr = FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
                    if( FAILED(hr) ) // the call will fail if no valid devices were found
                    {
                        DisplayErrorMessage( ERR_NOCOMPATIBLEDEVICES );
                        Shutdown();
                    }

                    // Change to a Direct3D device created from the new device settings.  
                    // If there is an existing device, then either reset or recreate the scene
                    hr = ChangeDevice( &deviceSettings, NULL, false, false );
                    if( FAILED(hr) )  
                    {
                        // If this fails, try to go fullscreen and if this fails also shutdown.
                        if( FAILED(ToggleFullScreen()) )
                            Shutdown();
                    }

                    return;
                }
            }

            // Try to reset the device
            if( FAILED( hr = Reset3DEnvironment() ) )
            {
                if( D3DERR_DEVICELOST == hr )
                {
                    // The device was lost again, so continue waiting until it can be reset.
                    return;
                }
                else if( ERR_RESETTINGDEVICEOBJECTS == hr || 
                         ERR_MEDIANOTFOUND == hr )
                {
                    DisplayErrorMessage( hr );
                    Shutdown();
                    return;
                }
                else
                {
                    // Reset failed, but the device wasn't lost so something bad happened, 
                    // so recreate the device to try to recover
                    DeviceSettings* pDeviceSettings = GetCurrentDeviceSettings();
                    if( FAILED( ChangeDevice( pDeviceSettings, NULL, true, false ) ) )
                    {
                        Shutdown();
                        return;
                    }
                }
            }
        }

        SetDeviceLost( false );
    }

    // Get the app's time, in seconds. Skip rendering if no time elapsed
    double fTime        = GetTimer()->GetTime();
    float fElapsedTime  = static_cast<float>( GetTimer()->GetElapsedTime() );

    // Store the time for the app
    if( GetConstantFrameTime() )
    {        
        fElapsedTime = GetTimePerFrame();
        fTime     = GetTime() + fElapsedTime;
    }

    SetTime( fTime );
    SetElapsedTime( fElapsedTime );

    // Update the FPS stats
    UpdateFrameStats();

    HandleTimers();

    // Animate the scene by calling the app's frame move callback
	OnFrameMove( pd3dDevice, fTime, fElapsedTime );
	pd3dDevice = GetD3DDevice();
    if( NULL == pd3dDevice ) // Handle Shutdown from inside callback
		return;

    if( !GetRenderingPaused() )
    {
        // Render the scene by calling the app's render callback
        OnFrameRender( pd3dDevice, fTime, fElapsedTime );
		WT::Graphics::CMaterial::ResetCachedMaterial( WT::Graphics::CRenderContext(), m_D3DDevice );

		pd3dDevice = GetD3DDevice();
        if( NULL == pd3dDevice ) // Handle Shutdown from inside callback
            return;

		RenderFrameStats();
#if defined(DEBUG) || defined(_DEBUG)
        // The back buffer should always match the client rect 
        // if the Direct3D backbuffer covers the entire window
        RECT rcClient;
        GetClientRect( GetHWND(), &rcClient );
        if( !IsIconic( GetHWND() ) )
        {
            GetClientRect( GetHWND(), &rcClient );
            assert( GetBackBufferSurfaceDesc()->Width == (UINT)rcClient.right );
            assert( GetBackBufferSurfaceDesc()->Height == (UINT)rcClient.bottom );
        }        
#endif

        // Show the frame on the primary surface.
        hr = pd3dDevice->Present( NULL, NULL, NULL, NULL );
        if( FAILED(hr) )
        {
            if( D3DERR_DEVICELOST == hr )
            {
                SetDeviceLost( true );
            }
            else if( D3DERR_DRIVERINTERNALERROR == hr )
            {
                // When D3DERR_DRIVERINTERNALERROR is returned from Present(),
                // the application can do one of the following:
                // 
                // - End, with the pop-up window saying that the application cannot continue 
                //   because of problems in the display adapter and that the user should 
                //   contact the adapter manufacturer.
                //
                // - Attempt to restart by calling IDirect3DDevice9::Reset, which is essentially the same 
                //   path as recovering from a lost device. If IDirect3DDevice9::Reset fails with 
                //   D3DERR_DRIVERINTERNALERROR, the application should end immediately with the message 
                //   that the user should contact the adapter manufacturer.
                // 
                // The framework attempts the path of resetting the device
                // 
                SetDeviceLost( true );
            }
        }
    }

    // Update current frame #
    int nFrame = GetCurrentFrameNumber();
    nFrame++;
    SetCurrentFrameNumber( nFrame );

    // Check to see if the app should shutdown due to cmdline
    if( GetOverrideQuitAfterFrame() != 0 )
    {
        if( nFrame > GetOverrideQuitAfterFrame() )
            Shutdown();
    }

    return;
}

void CApplication::CreateStatusDisplayObjects()
{
	DeleteStatusDisplayObjects();

	using namespace Graphics::UI;

	// Create the font
	m_pFrameStatsFont = new CSystemFont();
	m_pFrameStatsFont->Initialize( "Arial", 8, ECharacterSet::Default, 256, 256, EFontWeight::Normal, 
								   false, false, false, true, false );
	wstring LowerChars( L"abcdefghijklmnopqrstuvwxyz1234567890-=,.;'/" );
	wstring UpperChars( L"ABCDEFGHIJKLMNOPQRSTUVWXYZ, <>?()*&^%$#@!:\"" );
	m_pFrameStatsFont->BeginUpdate( m_D3DDevice );
	m_pFrameStatsFont->AddChars( LowerChars );
	m_pFrameStatsFont->AddChars( UpperChars );
	m_pFrameStatsFont->EndUpdate();

	// Create the text rendering object
	m_pFrameStatsText = new CText( m_D3DDevice, m_pFrameStatsFont, 0xffffff00, CVector2f( 0.0f, 0.0f ) );
	m_pFrameStatsText->SetAlignment( ETextAlignment::MiddleLeft );
	m_pFrameStatsText->SetPosition ( 0,0);
}

void CApplication::DeleteStatusDisplayObjects()
{
	SafeDelete( m_pFrameStatsText );
	SafeDelete( m_pFrameStatsFont );
}

void CApplication::RenderFrameStats()
{
	if ( GetShowStats() == EShowStats::DontShow || !m_pFrameStatsText )
		return;

	WT::Graphics::CRenderContext Context( NULL );

	m_D3DDevice->BeginScene();
	m_pFrameStatsText->Render( m_D3DDevice, Context );
	m_D3DDevice->EndScene();
}

//--------------------------------------------------------------------------------------
// Updates the string which describes the device 
//--------------------------------------------------------------------------------------
void CApplication::UpdateDeviceStats( D3DDEVTYPE DeviceType, DWORD BehaviorFlags, D3DADAPTER_IDENTIFIER9* pAdapterIdentifier )
{
    if( GetNoStats() )
        return;

    // Store device description
    char* pstrDeviceStats = GetDeviceStats();
    if( DeviceType == D3DDEVTYPE_REF )
        StringCchCopy( pstrDeviceStats, 256, "REF" );
    else if( DeviceType == D3DDEVTYPE_HAL )
        StringCchCopy( pstrDeviceStats, 256, "HAL" );
    else if( DeviceType == D3DDEVTYPE_SW )
        StringCchCopy( pstrDeviceStats, 256, "SW" );

    if( BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING &&
        BehaviorFlags & D3DCREATE_PUREDEVICE )
    {
        if( DeviceType == D3DDEVTYPE_HAL )
            StringCchCat( pstrDeviceStats, 256, " (pure hw vp)" );
        else
            StringCchCat( pstrDeviceStats, 256, " (simulated pure hw vp)" );
    }
    else if( BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING )
    {
        if( DeviceType == D3DDEVTYPE_HAL )
            StringCchCat( pstrDeviceStats, 256, " (hw vp)" );
        else
            StringCchCat( pstrDeviceStats, 256, " (simulated hw vp)" );
    }
    else if( BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING )
    {
        if( DeviceType == D3DDEVTYPE_HAL )
            StringCchCat( pstrDeviceStats, 256, " (mixed vp)" );
        else
            StringCchCat( pstrDeviceStats, 256, " (simulated mixed vp)" );
    }
    else if( BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
    {
        StringCchCat( pstrDeviceStats, 256, " (sw vp)" );
    }

    if( DeviceType == D3DDEVTYPE_HAL )
    {
        // Be sure not to overflow m_strDeviceStats when appending the adapter 
        // description, since it can be long.  
        StringCchCat( pstrDeviceStats, 256, ": " );

        // Try to get a unique description from the CDeviceEnumDeviceSettingsCombo
        DeviceSettings* pDeviceSettings = GetCurrentDeviceSettings();
        CDeviceEnumeration* pd3dEnum = PrepareEnumerationObject();
        CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo = pd3dEnum->GetDeviceSettingsCombo( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, pDeviceSettings->AdapterFormat, pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed );
        if( pDeviceSettingsCombo )
        {
            StringCchCat( pstrDeviceStats, 256, pDeviceSettingsCombo->pAdapterInfo->szUniqueDescription );
        }
        else
        {
            const int cchDesc = sizeof(pAdapterIdentifier->Description);
            char szDescription[cchDesc];
			StringCchCopy ( szDescription , cchDesc , pAdapterIdentifier->Description );
            szDescription[cchDesc-1] = 0;
            StringCchCat( pstrDeviceStats, 256, szDescription );
        }
    }
}

//Update the actual text control (if available) with the text for the frame stats
void CApplication::UpdateFrameStatsText()
{
	if ( !m_pFrameStatsText )
		return;

	switch ( GetShowStats() )
	{
	case EShowStats::DontShow:
		m_pFrameStatsText->SetText(L"");
		return;
	case EShowStats::OnlyFPS:
		m_pFrameStatsText->SetText ( AnsiToWide(GetFPSStats()) );
		break;
	case EShowStats::FPSAndBuffer:
		m_pFrameStatsText->SetText ( AnsiToWide(GetFPSStats()) + L" " + AnsiToWide(GetStaticFrameStats()));
		break;
	case EShowStats::Full:
		m_pFrameStatsText->SetText ( AnsiToWide(GetFPSStats()) + L" " + AnsiToWide(GetStaticFrameStats())+L"\n"+
			                            AnsiToWide(GetDeviceStats()));
		break;
	}
}
//--------------------------------------------------------------------------------------
// Updates the frames/sec stat once per second
//--------------------------------------------------------------------------------------
void CApplication::UpdateFrameStats()
{
    if( GetNoStats() )
        return;

    // Keep track of the frame count
    double fLastTime = GetLastStatsUpdateTime();
    DWORD dwFrames  = GetLastStatsUpdateFrames();
    double fTime = GetTimer()->GetAbsoluteTime();
    dwFrames++;
    SetLastStatsUpdateFrames( dwFrames );

    // Update the scene stats once per second
    if( fTime - fLastTime > 1.0f )
    {
        float fFPS = (float) (dwFrames / (fTime - fLastTime));
        SetFPS( fFPS );
        SetLastStatsUpdateTime( fTime );
        SetLastStatsUpdateFrames( 0 );

        char* pstrFPS = GetFPSStats();
        StringCchPrintf( pstrFPS, 64, "%0.2f fps ", fFPS );
		UpdateFrameStatsText();
    }
}


//--------------------------------------------------------------------------------------
// Updates the static part of the frame stats so it doesn't have be generated every frame
//--------------------------------------------------------------------------------------
void CApplication::UpdateStaticFrameStats()
{
    if( GetNoStats() )
        return;

    DeviceSettings* pDeviceSettings = GetCurrentDeviceSettings();
    if( NULL == pDeviceSettings )
        return;
    CDeviceEnumeration* pd3dEnum = PrepareEnumerationObject();
    if( NULL == pd3dEnum )
        return;

    CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo = pd3dEnum->GetDeviceSettingsCombo( pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType, pDeviceSettings->AdapterFormat, pDeviceSettings->pp.BackBufferFormat, pDeviceSettings->pp.Windowed );
    if( NULL == pDeviceSettingsCombo )
        return;

    char strFmt[100];
    D3DPRESENT_PARAMETERS* pPP = &pDeviceSettings->pp;

    if( pDeviceSettingsCombo->AdapterFormat == pDeviceSettingsCombo->BackBufferFormat )
    {
        StringCchCopy( strFmt, 100, D3DFormatToString( pDeviceSettingsCombo->AdapterFormat, false ) );
    }
    else
    {
        StringCchPrintf( strFmt, 100, "backbuf %s, adapter %s", 
            D3DFormatToString( pDeviceSettingsCombo->BackBufferFormat, false ), 
            D3DFormatToString( pDeviceSettingsCombo->AdapterFormat, false ) );
    }

    char strDepthFmt[100];
    if( pPP->EnableAutoDepthStencil )
    {
        StringCchPrintf( strDepthFmt, 100, " (%s)", D3DFormatToString( pPP->AutoDepthStencilFormat, false ) );
    }
    else
    {
        // No depth buffer
        strDepthFmt[0] = 0;
    }

    char strMultiSample[100];
    switch( pPP->MultiSampleType )
    {
        case D3DMULTISAMPLE_NONMASKABLE: StringCchCopy( strMultiSample, 100, " (Nonmaskable Multisample)" ); break;
        case D3DMULTISAMPLE_NONE:        StringCchCopy( strMultiSample, 100, "" ); break;
        default:                         StringCchPrintf( strMultiSample, 100, " (%dx Multisample)", pPP->MultiSampleType ); break;
    }

    char* pstrStaticFrameStats = GetStaticFrameStats();
    StringCchPrintf( pstrStaticFrameStats, 256, "Vsync %s (%dx%d), %s%s%s", 
                ( pPP->PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ) ? "off" : "on", 
                pPP->BackBufferWidth, pPP->BackBufferHeight,
                strFmt, strDepthFmt, strMultiSample );
}


//--------------------------------------------------------------------------------------
LPCSTR CApplication::GetFrameStatsString( bool bShowFPS )                         
{ 
    char* pstrFrameStats = GetFrameStats();
    char* pstrFPS = ( bShowFPS ) ? GetFPSStats() : "";
    StringCchPrintf( pstrFrameStats, 256, GetStaticFrameStats(), pstrFPS );
    return pstrFrameStats;
}


//--------------------------------------------------------------------------------------
// Handles window messages 
//--------------------------------------------------------------------------------------

LRESULT CALLBACK CApplication::StaticWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return gm_pInstance->PrivateWndProc( hWnd , uMsg , wParam , lParam );
}

LRESULT CApplication::PrivateWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // Consolidate the keyboard messages and pass them to the app's keyboard callback
    if ( uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP )
    {
        bool bKeyDown = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN);
        DWORD dwMask = (1 << 29);
        bool bAltDown = ( (lParam & dwMask) != 0 );

        bool* bKeys = GetKeys();
        bKeys[ (BYTE) (wParam & 0xFF) ] = bKeyDown;

		OnKeyboardEvent ( (UINT)wParam, bKeyDown, bAltDown );
    }

    // Consolidate the mouse button messages and pass them to the app's mouse callback
    if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_LBUTTONDBLCLK || uMsg == WM_MBUTTONDOWN ||
        uMsg == WM_MBUTTONUP || uMsg == WM_MBUTTONDBLCLK ||uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP ||
        uMsg == WM_RBUTTONDBLCLK || uMsg == WM_XBUTTONDOWN || uMsg == WM_XBUTTONUP || uMsg == WM_XBUTTONDBLCLK ||
        uMsg == WM_MOUSEWHEEL || (GetNotifyOnMouseMove() && uMsg == WM_MOUSEMOVE) )
    {
        int xPos = (short)LOWORD(lParam);
        int yPos = (short)HIWORD(lParam);

        if( uMsg == WM_MOUSEWHEEL )
        {
            // WM_MOUSEWHEEL passes screen mouse coords
            // so convert them to client coords
            POINT pt;
            pt.x = xPos; pt.y = yPos;
            ScreenToClient( hWnd, &pt );
            xPos = pt.x; yPos = pt.y;
        }

        int nMouseWheelDelta = 0;
        if( uMsg == WM_MOUSEWHEEL ) 
            nMouseWheelDelta = (short) HIWORD(wParam);

        int nMouseButtonState = LOWORD(wParam);
        bool bLeftButton  = ((nMouseButtonState & MK_LBUTTON) != 0);
        bool bRightButton = ((nMouseButtonState & MK_RBUTTON) != 0);
        bool bMiddleButton = ((nMouseButtonState & MK_MBUTTON) != 0);
        bool bSideButton1 = ((nMouseButtonState & MK_XBUTTON1) != 0);
        bool bSideButton2 = ((nMouseButtonState & MK_XBUTTON2) != 0);

        bool* bMouseButtons = GetMouseButtons();
        bMouseButtons[0] = bLeftButton;
        bMouseButtons[1] = bMiddleButton;
        bMouseButtons[2] = bRightButton;
        bMouseButtons[3] = bSideButton1;
        bMouseButtons[4] = bSideButton2;

		OnMouseEvent ( bLeftButton, bRightButton, bMiddleButton, bSideButton1, bSideButton2, nMouseWheelDelta, xPos, yPos );
    }

	// Pass all messages to the app's WndProc callback, and don't 
    // process further messages if the apps says not to.
	bool NoFurtherProcessing = false;
	LRESULT Result = OnMessage ( hWnd, uMsg, wParam, lParam, NoFurtherProcessing );
    if( NoFurtherProcessing )
		return Result;

    switch( uMsg )
    {
        case WM_PAINT:
        {
            IDirect3DDevice9* pd3dDevice = GetD3DDevice();

            // Handle paint messages when the app is paused
            if( pd3dDevice && IsRenderingPaused() && 
                GetDeviceObjectsCreated() && GetDeviceObjectsReset() )
            {
                HRESULT hr;
                double fTime = GetTime();
                float fElapsedTime = GetElapsedTime();

                OnFrameRender( pd3dDevice, fTime, fElapsedTime );
				RenderFrameStats();

                hr = pd3dDevice->Present( NULL, NULL, NULL, NULL );
                if( D3DERR_DEVICELOST == hr )
                {
                    SetDeviceLost( true );
                }
                else if( D3DERR_DRIVERINTERNALERROR == hr )
                {
                    // When D3DERR_DRIVERINTERNALERROR is returned from Present(),
                    // the application can do one of the following:
                    // 
                    // - End, with the pop-up window saying that the application cannot continue 
                    //   because of problems in the display adapter and that the user should 
                    //   contact the adapter manufacturer.
                    //
                    // - Attempt to restart by calling IDirect3DDevice9::Reset, which is essentially the same 
                    //   path as recovering from a lost device. If IDirect3DDevice9::Reset fails with 
                    //   D3DERR_DRIVERINTERNALERROR, the application should end immediately with the message 
                    //   that the user should contact the adapter manufacturer.
                    // 
                    // The framework attempts the path of resetting the device
                    // 
                    SetDeviceLost( true );
                }
            }
            break;
        }

        case WM_SIZE:
            if( SIZE_MINIMIZED == wParam )
            {
                Pause( true, true ); // Pause while we're minimized

                SetMinimized( true );
                SetMaximized( false );
            }
            else
            {
                RECT rcCurrentClient;
                GetClientRect( GetHWND(), &rcCurrentClient );
                if( rcCurrentClient.top == 0 && rcCurrentClient.bottom == 0 )
                {
                    // Rapidly clicking the task bar to minimize and restore a window
                    // can cause a WM_SIZE message with SIZE_RESTORED when 
                    // the window has actually become minimized due to rapid change
                    // so just ignore this message
                }
                else if( SIZE_MAXIMIZED == wParam )
                {
                    if( GetMinimized() )
                        Pause( false, false ); // Unpause since we're no longer minimized
                    SetMinimized( false );
                    SetMaximized( true );
                    CheckForWindowSizeChange();
                    CheckForWindowChangingMonitors();
                }
                else if( SIZE_RESTORED == wParam )
                {      
                    if( GetMaximized() )
                    {
                        SetMaximized( false );
                        CheckForWindowSizeChange();
                        CheckForWindowChangingMonitors();
                    }
                    else if( GetMinimized() )
                    {
                        Pause( false, false ); // Unpause since we're no longer minimized
                        SetMinimized( false );
                        CheckForWindowSizeChange();
                        CheckForWindowChangingMonitors();
                    }
                    else if( GetInSizeMove() )
                    {
                        // If we're neither maximized nor minimized, the window size 
                        // is changing by the user dragging the window edges.  In this 
                        // case, we don't reset the device yet -- we wait until the 
                        // user stops dragging, and a WM_EXITSIZEMOVE message comes.
                    }
                    else
                    {
                        // This WM_SIZE come from resizing the window via an API like SetWindowPos() so 
                        // resize and reset the device now.
                        CheckForWindowSizeChange();
                        CheckForWindowChangingMonitors();
                    }
                }
            }
            break;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = _MIN_WINDOW_SIZE_X;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = _MIN_WINDOW_SIZE_Y;
            break;

        case WM_ENTERSIZEMOVE:
            // Halt frame movement while the app is sizing or moving
            Pause( true, true );
            SetInSizeMove( true );
            break;

        case WM_EXITSIZEMOVE:
            Pause( false, false );
            CheckForWindowSizeChange();
            CheckForWindowChangingMonitors();
            SetInSizeMove( false );
            break;

         case WM_MOUSEMOVE:
            if( IsActive() && !IsWindowed() )
            {
                IDirect3DDevice9* pd3dDevice = GetD3DDevice();
                if( pd3dDevice )
                {
                    POINT ptCursor;
                    GetCursorPos( &ptCursor );
                    pd3dDevice->SetCursorPosition( ptCursor.x, ptCursor.y, 0 );
                }
            }
            break;

        case WM_SETCURSOR:
            if( IsActive() && !IsWindowed() )
            {
                IDirect3DDevice9* pd3dDevice = GetD3DDevice();
                if( pd3dDevice && GetShowCursorWhenFullScreen() )
                    pd3dDevice->ShowCursor( true );
                return true; // prevent Windows from setting cursor to window class cursor
            }
            break;

       case WM_ACTIVATEAPP:
            if( wParam == TRUE && !IsActive() ) // Handle only if previously not active 
            {
                SetActive( true );

                // The GetMinimizedWhileFullscreen() varible is used instead of !IsWindowed()
                // to handle the rare case toggling to windowed mode while the fullscreen application 
                // is minimized and thus making the pause count wrong
                if( GetMinimizedWhileFullscreen() ) 
                {
                    Pause( false, false ); // Unpause since we're no longer minimized
                    SetMinimizedWhileFullscreen( false );
                }

                // Upon returning to this app, potentially disable shortcut keys 
                // (Windows key, accessibility shortcuts) 
                AllowShortcutKeys( ( IsWindowed() ) ? GetAllowShortcutKeysWhenWindowed() : 
                                                              GetAllowShortcutKeysWhenFullscreen() );

            }
            else if( wParam == FALSE && IsActive() ) // Handle only if previously active 
            {               
                SetActive( false );

                if( !IsWindowed() )
                {
                    // Going from full screen to a minimized state 
                    ClipCursor( NULL );      // don't limit the cursor anymore
                    Pause( true, true ); // Pause while we're minimized (take care not to pause twice by handling this message twice)
                    SetMinimizedWhileFullscreen( true ); 
                }

                // Restore shortcut keys (Windows key, accessibility shortcuts) to original state
                //
                // This is important to call here if the shortcuts are disabled, 
                // because if this is not done then the Windows key will continue to 
                // be disabled while this app is running which is very bad.
                // If the app crashes, the Windows key will return to normal.
                AllowShortcutKeys( true );
            }
            break;

       case WM_ENTERMENULOOP:
            // Pause the app when menus are displayed
            Pause( true, true );
            break;

        case WM_EXITMENULOOP:
            Pause( false, false );
            break;

        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key
            // So just ignore and don't beep
            return MAKELRESULT(0,MNC_CLOSE);
            break;

        case WM_NCHITTEST:
            // Prevent the user from selecting the menu in full screen mode
            if( !IsWindowed() )
                return HTCLIENT;
            break;

        case WM_POWERBROADCAST:
            switch( wParam )
            {
                #ifndef PBT_APMQUERYSUSPEND
                    #define PBT_APMQUERYSUSPEND 0x0000
                #endif
                case PBT_APMQUERYSUSPEND:
                    // At this point, the app should save any data for open
                    // network connections, files, etc., and prepare to go into
                    // a suspended mode.  The app can use the MsgProc callback
                    // to handle this if desired.
                    return true;

                #ifndef PBT_APMRESUMESUSPEND
                    #define PBT_APMRESUMESUSPEND 0x0007
                #endif
                case PBT_APMRESUMESUSPEND:
                    // At this point, the app should recover any data, network
                    // connections, files, etc., and resume running from when
                    // the app was suspended. The app can use the MsgProc callback
                    // to handle this if desired.
                   
                   // QPC may lose consistency when suspending, so reset the timer
                   // upon resume.
                   GetTimer()->Reset();                   
                   SetLastStatsUpdateTime( 0 );
                   return true;
            }
            break;

        case WM_SYSCOMMAND:
            // Prevent moving/sizing and power loss in full screen mode
            switch( wParam )
            {
                case SC_MOVE:
                case SC_SIZE:
                case SC_MAXIMIZE:
                case SC_KEYMENU:
                case SC_MONITORPOWER:
                    if( !IsWindowed() )
                        return 0;
                    break;
            }
            break;

        case WM_SYSKEYDOWN:
        {
            switch( wParam )
            {
                case VK_RETURN:
                {
                    if( GetHandleAltEnter() )
                    {
                        // Toggle full screen upon alt-enter 
                        DWORD dwMask = (1 << 29);
                        if( (lParam & dwMask) != 0 ) // Alt is down also
                        {
                            // Toggle the full screen/window mode
                            Pause( true, true );
                            ToggleFullScreen();
                            Pause( false, false );                        
                            return 0;
                        }
                    }
                }
            }
            break;
        }

        case WM_KEYDOWN:
        {
            if( GetHandleDefaultHotkeys() )
            {
                switch( wParam )
                {
					case VK_F1:
					{
						if ( GetShowStats() == EShowStats::Full )
							SetShowStats ( EShowStats::DontShow );
						else
							SetShowStats ( EShowStats::ShowStats ((int) GetShowStats() + 1) );
						UpdateFrameStatsText();
						break;
					}
                    case VK_F3:
                    {
                        Pause( true, true );
                        ToggleREF();
                        Pause( false, false );                        
                        break;
                    }

                    case VK_F8:
                    {
                        bool bWireFrame = GetWireframeMode();
                        bWireFrame = !bWireFrame; 
                        SetWireframeMode( bWireFrame );

                        IDirect3DDevice9* pd3dDevice = GetD3DDevice();
                        if( pd3dDevice )
                            pd3dDevice->SetRenderState( D3DRS_FILLMODE, (bWireFrame) ? D3DFILL_WIREFRAME : D3DFILL_SOLID ); 
                        break;
                    }

                    case VK_ESCAPE:
                    {
                        // Received key to exit app
                        SendMessage( hWnd, WM_CLOSE, 0, 0 );
                    }

                    case VK_PAUSE: 
                    {
                        bool bTimePaused = IsTimePaused();
                        bTimePaused = !bTimePaused;
                        if( bTimePaused ) 
                            Pause( true, false ); 
                        else
                            Pause( false, false ); 
                        break; 
                    }
                }
            }
            break;
        }

        case WM_CLOSE:
        {
            HMENU hMenu;
			hMenu = ::GetMenu(hWnd);
            if( hMenu != NULL )
                DestroyMenu( hMenu );
            DestroyWindow( hWnd );
            UnregisterClass( "Direct3DWindowClass", NULL );
            SetHWNDFocus( NULL );
            SetHWNDDeviceFullScreen( NULL );
            SetHWNDDeviceWindowed( NULL );
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    // Don't allow the F10 key to act as a shortcut to the menu bar
    // by not passing these messages to the DefWindowProc only when
    // there's no menu present
    if( !GetCallDefWindowProc() || GetMenu() == NULL && (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP) && wParam == VK_F10 )
        return 0;
    else
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


//--------------------------------------------------------------------------------------
// Resets the state associated with  
//--------------------------------------------------------------------------------------
void CApplication::ResetFrameworkState()
{
    DestroyState();
    CreateState();
}


//--------------------------------------------------------------------------------------
// Closes down the window.  When the window closes, it will cleanup everything
//--------------------------------------------------------------------------------------
void CApplication::Shutdown( int nExitCode )
{
    HWND hWnd = GetHWND();
    if( hWnd != NULL )
        SendMessage( hWnd, WM_CLOSE, 0, 0 );

    SetExitCode(nExitCode);

    Cleanup3DEnvironment( true );

    // Restore shortcut keys (Windows key, accessibility shortcuts) to original state
    // This is important to call here if the shortcuts are disabled, 
    // because accessibility setting changes are permanent.
    // This means that if this is not done then the accessibility settings 
    // might not be the same as when the app was started. 
    // If the app crashes without restoring the settings, this is also true so it
    // would be wise to backup/restore the settings from a file so they can be 
    // restored when the crashed app is run again.
    AllowShortcutKeys( true );
    
    SetD3DEnumeration( NULL );
    
    IDirect3D9* pD3D = GetD3DObject();
    SafeRelease( pD3D );
    SetD3D( NULL );

	// Release our semaphore so that another copy of the app can run later
	if( m_hInstanceSemaphore != NULL )
	{
		::ReleaseSemaphore( m_hInstanceSemaphore, 1, NULL );
		::CloseHandle( m_hInstanceSemaphore );
		m_hInstanceSemaphore = NULL;
	}

}

void WTCall CApplication::LaunchWordPadLog( std::string const& Name )
{
#ifdef WTLOG_ENABLED
	std::string const LogFileDir = System::Util::GetCommonAppDataPath() + "\\WildTangent\\WTPlatform";
	System::Util::CreateDirRecursive( LogFileDir );

	//Make sure the log file exists
	std::string LogFile = Name + ".rtf";

	std::string const LogPath = LogFileDir + "\\" + LogFile;

	HANDLE hFile = ::CreateFile( LogPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
	if (hFile != INVALID_HANDLE_VALUE)
	{
		//File successfully created
		::CloseHandle( hFile );
	}

	//Open the log in WordPad
	std::string CommandLine = "\"";
	CommandLine += System::Util::GetProgramFilesPath();
	CommandLine += "\\Windows NT\\Accessories\\wordpad.exe\" \"";
	CommandLine += LogPath.c_str();
	CommandLine += '"';

	STARTUPINFO StartupInfo;
	::ZeroMemory( &StartupInfo, sizeof StartupInfo );
	StartupInfo.cb = sizeof StartupInfo;

	PROCESS_INFORMATION ProcessInformation;
	if (::CreateProcess( NULL, const_cast<LPSTR>( CommandLine.c_str() ), 
		NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInformation ))
	{
		::WaitForInputIdle( ProcessInformation.hProcess, 5000 );
		::CloseHandle( ProcessInformation.hThread );
		::CloseHandle( ProcessInformation.hProcess );
	}

	//Log to the file
	LOG_SET_WORDPAD( true, LogFile.c_str() );
	LOG_GREEN( "Opening %s\n for logging", LogPath.c_str() );
#endif
}

//--------------------------------------------------------------------------------------
// Cleans up the 3D environment by:
//      - Calls the device lost callback 
//      - Calls the device destroyed callback 
//      - Releases the D3D device
//--------------------------------------------------------------------------------------
void CApplication::Cleanup3DEnvironment( bool bReleaseSettings )
{
    IDirect3DDevice9* pd3dDevice = GetD3DDevice();
    if( pd3dDevice != NULL )
    {
        SetInsideDeviceCallback( true );

        // Call the app's device lost callback
        if( GetDeviceObjectsReset() == true )
        {
			OnDeviceLost();
            SetDeviceObjectsReset( false );
        }

        // Call the app's device destroyed callback
        if( GetDeviceObjectsCreated() == true )
        {
            OnDeviceDestroyed();
            SetDeviceObjectsCreated( false );
        }

        SetInsideDeviceCallback( false );

		//delete our fonts and text objects used for status display
		DeleteStatusDisplayObjects();

        // Release the D3D device and in debug configs, displays a message box if there 
        // are unrelease objects.
        if( pd3dDevice )
        {
            if( pd3dDevice->Release() > 0 )  
            {
                DisplayErrorMessage( ERR_NONZEROREFCOUNT );
                _ERR( L"Cleanup3DEnvironment", ERR_NONZEROREFCOUNT );
            }
        }
        SetD3DDevice( NULL );

        if( bReleaseSettings )
        {
            DeviceSettings* pOldDeviceSettings = GetCurrentDeviceSettings();
            SafeDelete(pOldDeviceSettings);  
            SetCurrentDeviceSettings( NULL );
        }

        D3DSURFACE_DESC* pbackBufferSurfaceDesc = GetBackBufferSurfaceDesc();
        ZeroMemory( pbackBufferSurfaceDesc, sizeof(D3DSURFACE_DESC) );

        D3DCAPS9* pd3dCaps = GetCaps();
        ZeroMemory( pd3dCaps, sizeof(D3DCAPS9) );

        SetDeviceCreated( false );
    }
}


//--------------------------------------------------------------------------------------
// Stores back buffer surface desc in GetBackBufferSurfaceDesc()
//--------------------------------------------------------------------------------------
void CApplication::UpdateBackBufferDesc()
{
    HRESULT hr;
    IDirect3DSurface9* pBackBuffer;
    hr = GetD3DDevice()->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer );
    D3DSURFACE_DESC* pBBufferSurfaceDesc = GetBackBufferSurfaceDesc();
    ZeroMemory( pBBufferSurfaceDesc, sizeof(D3DSURFACE_DESC) );
    if( SUCCEEDED(hr) )
    {
        pBackBuffer->GetDesc( pBBufferSurfaceDesc );
        SafeRelease( pBackBuffer );
    }
}


//--------------------------------------------------------------------------------------
// Starts a user defined timer callback
//--------------------------------------------------------------------------------------
HRESULT CApplication::SetTimer ( CApplicationTimer::TriggerCallback* Callback, float fTimeoutInSecs , UINT* pnIDEvent )
{ 
    CApplicationTimer Timer;
    Timer.Event += Callback;
    Timer.TimeoutInSecs = fTimeoutInSecs;
    Timer.Countdown = fTimeoutInSecs;
    Timer.Enabled = true;

	std::vector<CApplicationTimer>* pTimerList = GetTimerList();
    if( pTimerList == NULL )
    {
		pTimerList = new std::vector<CApplicationTimer>;
        if( pTimerList == NULL )
            return E_OUTOFMEMORY; 
        SetTimerList( pTimerList );
    }

	pTimerList->push_back ( Timer );

    if( pnIDEvent )
        *pnIDEvent = (UINT) pTimerList->size();

    return S_OK; 
}


//--------------------------------------------------------------------------------------
// Stops a user defined timer callback
//--------------------------------------------------------------------------------------
HRESULT CApplication::KillTimer( UINT nIDEvent ) 
{ 
	std::vector<CApplicationTimer>* pTimerList = GetTimerList();
    if( pTimerList == NULL )
        return S_FALSE;

    if( nIDEvent < (UINT)pTimerList->size() )
    {
        CApplicationTimer& Timer = pTimerList->at(nIDEvent);
        Timer.Enabled = false;
    }
    else
    {
        return _ERR_MSGBOX( L"KillTimer", E_INVALIDARG );
    }

    return S_OK; 
}


//--------------------------------------------------------------------------------------
// Internal helper function to handle calling the user defined timer callbacks
//--------------------------------------------------------------------------------------
void CApplication::HandleTimers()
{
    float ElapsedTime = GetElapsedTime();

	std::vector<CApplicationTimer>* pTimerList = GetTimerList();
    if( pTimerList == NULL )
        return;

    // Walk through the list of timer callbacks
    for( uint32 i=0; i<pTimerList->size(); i++ )
    {
        CApplicationTimer& Timer = pTimerList->at(i);
        if( Timer.Enabled )
        {
            Timer.Countdown -= ElapsedTime;

            // Call the callback if count down expired
            if( Timer.Countdown < 0 )
            {
                Timer.Event ( i );
                Timer.Countdown = Timer.TimeoutInSecs;
            }
        }
    }
}

IDirect3D9* CApplication::GetD3DObject()
{
	return GetD3D();
}

const D3DCAPS9* CApplication::GetDeviceCaps()
{
	return GetCaps();
}

HWND CApplication::GetHWND()
{
	return IsWindowed() ? GetHWNDDeviceWindowed() : GetHWNDDeviceFullScreen();
}

RECT CApplication::GetWindowClientRect()
{
	RECT rc;
	GetClientRect( GetHWND(), &rc );
	return rc;
}

uint32 CApplication::GetWindowWidth()
{
	RECT rc;
	GetClientRect( GetHWND(), &rc );
	return rc.right-rc.left;
}

uint32 CApplication::GetWindowHeight()
{
	RECT rc;
	GetClientRect( GetHWND(), &rc );
	return rc.bottom-rc.top;
}

RECT CApplication::GetWindowClientRectAtModeChange()
{
	RECT rc = { 0, 0, GetWindowBackBufferWidthAtModeChange(), GetWindowBackBufferHeightAtModeChange() };
	return rc;
}

RECT CApplication::GetFullsceenClientRectAtModeChange()
{
	RECT rc = { 0, 0, GetFullScreenBackBufferWidthAtModeChange(), GetFullScreenBackBufferHeightAtModeChange() };
	return rc;
}

bool CApplication::IsRenderingPaused()
{
	return GetPauseRenderingCount() > 0;
}

bool CApplication::IsTimePaused()
{
	return GetPauseTimeCount() > 0;
}

bool CApplication::IsActive()
{
	return GetActive();
}

bool CApplication::IsKeyDown( BYTE vKey )
{ 
    bool* bKeys = GetKeys(); 
    if( vKey >= 0xA0 && vKey <= 0xA5 )  // VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL, VK_LMENU, VK_RMENU
        return GetAsyncKeyState( vKey ) != 0; // these keys only are tracked via GetAsyncKeyState()
    else if( vKey >= 0x01 && vKey <= 0x06 && vKey != 0x03 ) // mouse buttons (VK_*BUTTON)
        return IsMouseButtonDown(vKey);
    else
        return bKeys[vKey];
}
bool CApplication::IsMouseButtonDown( BYTE vButton )          
{ 
    bool* bMouseButtons = GetMouseButtons(); 
    int nIndex = MapButtonToArrayIndex(vButton); 
    return bMouseButtons[nIndex]; 
}
void CApplication::SetMultimonSettings( bool bAutoChangeAdapter )
{
    SetAutoChangeAdapter( bAutoChangeAdapter );
}
void CApplication::SetCursorSettings( bool bShowCursorWhenFullScreen, bool bClipCursorWhenFullScreen ) 
{ 
    SetClipCursorWhenFullScreen(bClipCursorWhenFullScreen); 
    SetShowCursorWhenFullScreen(bShowCursorWhenFullScreen); 
    SetupCursor();
}
void CApplication::SetWindowSettings( bool bCallDefWindowProc )
{
    SetCallDefWindowProc( bCallDefWindowProc );
}
void CApplication::SetConstantFrameTimeData( bool bEnabled, float fTimePerFrame ) 
{ 
    if( GetOverrideConstantFrameTime() ) 
    { 
        bEnabled = GetOverrideConstantFrameTime(); 
        fTimePerFrame = GetOverrideConstantTimePerFrame(); 
    } 
    SetConstantFrameTime(bEnabled); 
    SetTimePerFrame(fTimePerFrame); 
}


//--------------------------------------------------------------------------------------
// Return if windowed in the current device.  If no device exists yet, then returns false
//--------------------------------------------------------------------------------------
bool CApplication::IsWindowed()                               
{ 
    DeviceSettings* pDeviceSettings = GetCurrentDeviceSettings(); 
    if(pDeviceSettings) 
        return (pDeviceSettings->pp.Windowed != 0); 
    else 
        return false; 
}


//--------------------------------------------------------------------------------------
// Return the present params of the current device.  If no device exists yet, then
// return blank present params
//--------------------------------------------------------------------------------------
D3DPRESENT_PARAMETERS CApplication::GetPresentParameters()    
{ 
    DeviceSettings* pDS = GetCurrentDeviceSettings(); 
    if( pDS ) 
    {
        return pDS->pp; 
    }
    else 
    {
        D3DPRESENT_PARAMETERS pp;
        ZeroMemory( &pp, sizeof(D3DPRESENT_PARAMETERS) );
        return pp; 
    }
}


//--------------------------------------------------------------------------------------
// Return the device settings of the current device.  If no device exists yet, then
// return blank device settings 
//--------------------------------------------------------------------------------------
DeviceSettings CApplication::GetDeviceSettings()   
{ 
    DeviceSettings* pDS = GetCurrentDeviceSettings();
    if( pDS )
    {
        return *pDS;
    }
    else
    {
        DeviceSettings ds;
        ZeroMemory( &ds, sizeof(DeviceSettings) );
        return ds;
    }
} 

#ifndef SM_REMOTESESSION  // needs WINVER >= 0x0500
#define SM_REMOTESESSION  0x1000
#endif


//--------------------------------------------------------------------------------------
// Display an custom error msg box 
//--------------------------------------------------------------------------------------
void CApplication::DisplayErrorMessage( HRESULT hr )
{
    char strBuffer[512];

    int nExitCode;
    bool bFound = true; 
    switch( hr )
    {
        case ERR_NODIRECT3D:             nExitCode = 2; StringCchCopy( strBuffer, 512, "Could not initialize Direct3D. You may want to check that the latest version of DirectX is correctly installed on your system.  Also make sure that this program was compiled with header files that match the installed DirectX DLLs." ); break;
        case ERR_INCORRECTVERSION:       nExitCode = 10; StringCchCopy( strBuffer, 512, "Incorrect version of Direct3D and/or D3DX." ); break;
        case ERR_MEDIANOTFOUND:          nExitCode = 4; StringCchCopy( strBuffer, 512, "Could not find required media. Ensure that the DirectX SDK is correctly installed." ); break;
        case ERR_NONZEROREFCOUNT:        nExitCode = 5; StringCchCopy( strBuffer, 512, "The D3D device has a non-zero reference count, meaning some objects were not released." ); break;
        case ERR_CREATINGDEVICE:         nExitCode = 6; StringCchCopy( strBuffer, 512, "Failed creating the Direct3D device." ); break;
        case ERR_RESETTINGDEVICE:        nExitCode = 7; StringCchCopy( strBuffer, 512, "Failed resetting the Direct3D device." ); break;
        case ERR_CREATINGDEVICEOBJECTS:  nExitCode = 8; StringCchCopy( strBuffer, 512, "Failed creating Direct3D device objects." ); break;
        case ERR_RESETTINGDEVICEOBJECTS: nExitCode = 9; StringCchCopy( strBuffer, 512, "Failed resetting Direct3D device objects." ); break;
        case ERR_NOCOMPATIBLEDEVICES:    
            nExitCode = 3; 
            if( GetSystemMetrics(SM_REMOTESESSION) != 0 )
                StringCchCopy( strBuffer, 512, "Direct3D does not work over a remote session." ); 
            else
                StringCchCopy( strBuffer, 512, "Could not find any compatible Direct3D devices." ); 
            break;
        default: bFound = false; nExitCode = 1;break;
    }   

    SetExitCode(nExitCode);

    bool bShowMsgBoxOnError = GetShowMsgBoxOnError();
    if( bFound && bShowMsgBoxOnError )
    {
        if( GetWindowTitle()[0] == 0 )
            MessageBox( GetHWND(), strBuffer, "DirectX Application", MB_ICONERROR|MB_OK );
        else
            MessageBox( GetHWND(), strBuffer, GetWindowTitle(), MB_ICONERROR|MB_OK );
    }
}

//--------------------------------------------------------------------------------------
// Checks to see if the HWND changed monitors, and if it did it creates a device 
// from the monitor's adapter and recreates the scene.
//--------------------------------------------------------------------------------------
void CApplication::CheckForWindowChangingMonitors()
{
    // Skip this check for various reasons
    if( !GetAutoChangeAdapter() || 
         GetIgnoreSizeChange() ||
        !GetDeviceCreated() ||
        !GetCurrentDeviceSettings()->pp.Windowed )
    {
        return;
    }

    HRESULT hr;
    HMONITOR hWindowMonitor = DynamicMonitorFromWindow( GetHWND(), MONITOR_DEFAULTTOPRIMARY );
    HMONITOR hAdapterMonitor = GetAdapterMonitor();
    if( hWindowMonitor != hAdapterMonitor )
    {
        UINT newOrdinal;
        if( SUCCEEDED( GetAdapterOrdinalFromMonitor( hWindowMonitor, &newOrdinal ) ) )
        {
            // Find the closest valid device settings with the new ordinal
            DeviceSettings deviceSettings = GetDeviceSettings();
            deviceSettings.AdapterOrdinal = newOrdinal;
            
            MatchOptions matchOptions;
            matchOptions.AdapterOrdinal     = EMatchType::PreserveInput;
            matchOptions.DeviceType         = EMatchType::ClosestToInput;
            matchOptions.Windowed           = EMatchType::ClosestToInput;
            matchOptions.AdapterFormat      = EMatchType::ClosestToInput;
            matchOptions.VertexProcessing   = EMatchType::ClosestToInput;
            matchOptions.Resolution         = EMatchType::ClosestToInput;
            matchOptions.BackBufferFormat   = EMatchType::ClosestToInput;
            matchOptions.BackBufferCount    = EMatchType::ClosestToInput;
            matchOptions.MultiSample        = EMatchType::ClosestToInput;
            matchOptions.SwapEffect         = EMatchType::ClosestToInput;
            matchOptions.DepthFormat        = EMatchType::ClosestToInput;
            matchOptions.StencilFormat      = EMatchType::ClosestToInput;
            matchOptions.PresentFlags       = EMatchType::ClosestToInput;
            matchOptions.RefreshRate        = EMatchType::ClosestToInput;
            matchOptions.PresentInterval    = EMatchType::ClosestToInput;

            hr = FindValidDeviceSettings( &deviceSettings, &deviceSettings, &matchOptions );
            if( SUCCEEDED(hr) ) 
            {
                // Create a Direct3D device using the new device settings.  
                // If there is an existing device, then it will either reset or recreate the scene.
                hr = ChangeDevice( &deviceSettings, NULL, false, false );

                // If hr == E_ABORT, this means the app rejected the device settings in the ModifySettingsCallback
                if( hr == E_ABORT )
                {
                    // so nothing changed and keep from attempting to switch adapters next time
                    SetAutoChangeAdapter( false );
                }
                else if( FAILED(hr) )
                {
                    Shutdown();
                    Pause( false, false );
                    return;
                }
            }
        }
    }    
}


//--------------------------------------------------------------------------------------
// Look for an adapter ordinal that is tied to a HMONITOR
//--------------------------------------------------------------------------------------
HRESULT CApplication::GetAdapterOrdinalFromMonitor( HMONITOR hMonitor, UINT* pAdapterOrdinal )
{
    *pAdapterOrdinal = 0;

    CDeviceEnumeration* pd3dEnum = PrepareEnumerationObject();
    IDirect3D9*      pD3D     = GetD3DObject();

	std::vector<CDeviceEnumAdapterInfo*>* pAdapterList = pd3dEnum->GetAdapterInfoList();
    for( uint32 iAdapter=0; iAdapter<pAdapterList->size(); iAdapter++ )
    {
        CDeviceEnumAdapterInfo* pAdapterInfo = (*pAdapterList)[iAdapter];
        HMONITOR hAdapterMonitor = pD3D->GetAdapterMonitor( pAdapterInfo->AdapterOrdinal );
        if( hAdapterMonitor == hMonitor )
        {
            *pAdapterOrdinal = pAdapterInfo->AdapterOrdinal;
            return S_OK;
        }
    }

    return E_FAIL;
}


//--------------------------------------------------------------------------------------
// Internal function to map MK_* to an array index
//--------------------------------------------------------------------------------------
int CApplication::MapButtonToArrayIndex( BYTE vButton )
{
    switch( vButton )
    {
        case MK_LBUTTON: return 0;
        case VK_MBUTTON: 
        case MK_MBUTTON: return 1;
        case MK_RBUTTON: return 2;
        case VK_XBUTTON1:
        case MK_XBUTTON1: return 3;
        case VK_XBUTTON2:
        case MK_XBUTTON2: return 4;
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Setup cursor based on current settings (window/fullscreen mode, show cursor state, clip cursor state)
//--------------------------------------------------------------------------------------
void CApplication::SetupCursor()
{
	// Show the cursor again if returning to fullscreen 
	IDirect3DDevice9* pd3dDevice = GetD3DDevice();
	if( !IsWindowed() && pd3dDevice )
    {
        if( GetShowCursorWhenFullScreen() )
	    {
            SetCursor( NULL ); // Turn off Windows cursor in full screen mode
		    HCURSOR hCursor = (HCURSOR)(ULONG_PTR)GetClassLongPtr( GetHWNDDeviceFullScreen(), GCLP_HCURSOR );
		    SetDeviceCursor( pd3dDevice, hCursor, false );
            GetD3DDevice()->ShowCursor( true );
	    }
        else
        {
            SetCursor( NULL ); // Turn off Windows cursor in full screen mode
            GetD3DDevice()->ShowCursor( false );
        }
    }

	// Clip cursor if requested
	if( !IsWindowed() && GetClipCursorWhenFullScreen() )
	{
		// Confine cursor to full screen window
		RECT rcWindow;
		GetWindowRect( GetHWNDDeviceFullScreen(), &rcWindow );
		ClipCursor( &rcWindow );
	}
	else
	{
		ClipCursor( NULL );
	}
}

//--------------------------------------------------------------------------------------
// Gives the D3D device a cursor with image and hotspot from hCursor.
//--------------------------------------------------------------------------------------
HRESULT CApplication::SetDeviceCursor( IDirect3DDevice9* pd3dDevice, HCURSOR hCursor, bool bAddWatermark )
{
    HRESULT hr = E_FAIL;
    ICONINFO iconinfo;
    bool bBWCursor;
    LPDIRECT3DSURFACE9 pCursorSurface = NULL;
    HDC hdcColor = NULL;
    HDC hdcMask = NULL;
    HDC hdcScreen = NULL;
    BITMAP bm;
    DWORD dwWidth;
    DWORD dwHeightSrc;
    DWORD dwHeightDest;
    COLORREF crColor;
    COLORREF crMask;
    UINT x;
    UINT y;
    BITMAPINFO bmi;
    COLORREF* pcrArrayColor = NULL;
    COLORREF* pcrArrayMask = NULL;
    DWORD* pBitmap;
    HGDIOBJ hgdiobjOld;

    ZeroMemory( &iconinfo, sizeof(iconinfo) );
    if( !GetIconInfo( hCursor, &iconinfo ) )
        goto End;

    if (0 == GetObject((HGDIOBJ)iconinfo.hbmMask, sizeof(BITMAP), (LPVOID)&bm))
        goto End;
    dwWidth = bm.bmWidth;
    dwHeightSrc = bm.bmHeight;

    if( iconinfo.hbmColor == NULL )
    {
        bBWCursor = TRUE;
        dwHeightDest = dwHeightSrc / 2;
    }
    else 
    {
        bBWCursor = FALSE;
        dwHeightDest = dwHeightSrc;
    }

    // Create a surface for the fullscreen cursor
    if( FAILED( hr = pd3dDevice->CreateOffscreenPlainSurface( dwWidth, dwHeightDest, 
        D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pCursorSurface, NULL ) ) )
    {
        goto End;
    }

    pcrArrayMask = new DWORD[dwWidth * dwHeightSrc];

    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = dwWidth;
    bmi.bmiHeader.biHeight = dwHeightSrc;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hdcScreen = GetDC( NULL );
    hdcMask = CreateCompatibleDC( hdcScreen );
    if( hdcMask == NULL )
    {
        hr = E_FAIL;
        goto End;
    }
    hgdiobjOld = SelectObject(hdcMask, iconinfo.hbmMask);
    GetDIBits(hdcMask, iconinfo.hbmMask, 0, dwHeightSrc, 
        pcrArrayMask, &bmi, DIB_RGB_COLORS);
    SelectObject(hdcMask, hgdiobjOld);

    if (!bBWCursor)
    {
        pcrArrayColor = new DWORD[dwWidth * dwHeightDest];
        hdcColor = CreateCompatibleDC( hdcScreen );
        if( hdcColor == NULL )
        {
            hr = E_FAIL;
            goto End;
        }
        SelectObject(hdcColor, iconinfo.hbmColor);
        GetDIBits(hdcColor, iconinfo.hbmColor, 0, dwHeightDest, 
            pcrArrayColor, &bmi, DIB_RGB_COLORS);
    }

    // Transfer cursor image into the surface
    D3DLOCKED_RECT lr;
    pCursorSurface->LockRect( &lr, NULL, 0 );
    pBitmap = (DWORD*)lr.pBits;
    for( y = 0; y < dwHeightDest; y++ )
    {
        for( x = 0; x < dwWidth; x++ )
        {
            if (bBWCursor)
            {
                crColor = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
                crMask = pcrArrayMask[dwWidth*(dwHeightSrc-1-y) + x];
            }
            else
            {
                crColor = pcrArrayColor[dwWidth*(dwHeightDest-1-y) + x];
                crMask = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
            }
            if (crMask == 0)
                pBitmap[dwWidth*y + x] = 0xff000000 | crColor;
            else
                pBitmap[dwWidth*y + x] = 0x00000000;

            // It may be helpful to make the D3D cursor look slightly 
            // different from the Windows cursor so you can distinguish 
            // between the two when developing/testing code.  When
            // bAddWatermark is TRUE, the following code adds some
            // small grey "D3D" characters to the upper-left corner of
            // the D3D cursor image.
            if( bAddWatermark && x < 12 && y < 5 )
            {
                // 11.. 11.. 11.. .... CCC0
                // 1.1. ..1. 1.1. .... A2A0
                // 1.1. .1.. 1.1. .... A4A0
                // 1.1. ..1. 1.1. .... A2A0
                // 11.. 11.. 11.. .... CCC0

                const WORD wMask[5] = { 0xccc0, 0xa2a0, 0xa4a0, 0xa2a0, 0xccc0 };
                if( wMask[y] & (1 << (15 - x)) )
                {
                    pBitmap[dwWidth*y + x] |= 0xff808080;
                }
            }
        }
    }
    pCursorSurface->UnlockRect();

    // Set the device cursor
    if( FAILED( hr = pd3dDevice->SetCursorProperties( iconinfo.xHotspot, 
        iconinfo.yHotspot, pCursorSurface ) ) )
    {
        goto End;
    }

    hr = S_OK;

End:
    if( iconinfo.hbmMask != NULL )
        DeleteObject( iconinfo.hbmMask );
    if( iconinfo.hbmColor != NULL )
        DeleteObject( iconinfo.hbmColor );
    if( hdcScreen != NULL )
        ReleaseDC( NULL, hdcScreen );
    if( hdcColor != NULL )
        DeleteDC( hdcColor );
    if( hdcMask != NULL )
        DeleteDC( hdcMask );
    SafeDeleteArray( pcrArrayColor );
    SafeDeleteArray( pcrArrayMask );
    SafeRelease( pCursorSurface );
    return hr;
}

CTimer2* CApplication::GetTimer()
{
    return &m_Timer;
}

//--------------------------------------------------------------------------------------
// Direct3D9 dynamic linking support -- calls top-level D3D9 APIs with graceful
// failure if APIs are not present.
//--------------------------------------------------------------------------------------

// Function prototypes
typedef IDirect3D9* (WINAPI * LPDIRECT3DCREATE9) (UINT);
typedef INT         (WINAPI * LPD3DPERF_BEGINEVENT)(D3DCOLOR, LPCWSTR);
typedef INT         (WINAPI * LPD3DPERF_ENDEVENT)(void);
typedef VOID        (WINAPI * LPD3DPERF_SETMARKER)(D3DCOLOR, LPCWSTR);
typedef VOID        (WINAPI * LPD3DPERF_SETREGION)(D3DCOLOR, LPCWSTR);
typedef BOOL        (WINAPI * LPD3DPERF_QUERYREPEATFRAME)(void);
typedef VOID        (WINAPI * LPD3DPERF_SETOPTIONS)( DWORD dwOptions );
typedef DWORD       (WINAPI * LPD3DPERF_GETSTATUS)( void );

// Module and function pointers
static LPDIRECT3DCREATE9 s_DynamicDirect3DCreate9 = NULL;
static LPD3DPERF_BEGINEVENT s_DynamicD3DPERF_BeginEvent = NULL;
static LPD3DPERF_ENDEVENT s_DynamicD3DPERF_EndEvent = NULL;
static LPD3DPERF_SETMARKER s_DynamicD3DPERF_SetMarker = NULL;
static LPD3DPERF_SETREGION s_DynamicD3DPERF_SetRegion = NULL;
static LPD3DPERF_QUERYREPEATFRAME s_DynamicD3DPERF_QueryRepeatFrame = NULL;
static LPD3DPERF_SETOPTIONS s_DynamicD3DPERF_SetOptions = NULL;
static LPD3DPERF_GETSTATUS s_DynamicD3DPERF_GetStatus = NULL;

class CLibrary
{
public:
	CLibrary( char const LibFileName[] )
	{
		m_hModule = ::LoadLibrary( LibFileName );
		if (m_hModule == NULL)
		{
			throw std::runtime_error( "LoadLibrary failure" );
		}
	}

	~CLibrary()
	{
		assert( m_hModule != NULL );
		::FreeLibrary( m_hModule );
	}

	HMODULE GetModule() {return m_hModule;}

private:
	HMODULE m_hModule;
};

static std::auto_ptr<CLibrary> s_hModD3D9;

// Ensure function pointers are initialized
static bool _EnsureD3DAPIs( void )
{
    // If module is non-NULL, this function has already been called.  Note
    // that this doesn't guarantee that all D3D9 procaddresses were found.
    if (s_hModD3D9.get() != NULL)
        return true;

    // This may fail if DirectX 9 isn't installed
    char szPath[MAX_PATH+1];
    if( !::GetSystemDirectory( szPath, MAX_PATH+1 ) )
        return false;
    StringCchCat( szPath, MAX_PATH, "\\d3d9.dll" );

	try
	{
	    s_hModD3D9.reset( new CLibrary( szPath ) );
	} catch (...) {
		assert( s_hModD3D9.get() == NULL );
        return false;
	}

	HMODULE hModule = s_hModD3D9->GetModule();
	assert( hModule != NULL );
    s_DynamicDirect3DCreate9 = (LPDIRECT3DCREATE9)GetProcAddress( hModule, "Direct3DCreate9" );
    s_DynamicD3DPERF_BeginEvent = (LPD3DPERF_BEGINEVENT)GetProcAddress( hModule, "D3DPERF_BeginEvent" );
    s_DynamicD3DPERF_EndEvent = (LPD3DPERF_ENDEVENT)GetProcAddress( hModule, "D3DPERF_EndEvent" );
    s_DynamicD3DPERF_SetMarker = (LPD3DPERF_SETMARKER)GetProcAddress( hModule, "D3DPERF_SetMarker" );
    s_DynamicD3DPERF_SetRegion = (LPD3DPERF_SETREGION)GetProcAddress( hModule, "D3DPERF_SetRegion" );
    s_DynamicD3DPERF_QueryRepeatFrame = (LPD3DPERF_QUERYREPEATFRAME)GetProcAddress( hModule, "D3DPERF_QueryRepeatFrame" );
    s_DynamicD3DPERF_SetOptions = (LPD3DPERF_SETOPTIONS)GetProcAddress( hModule, "D3DPERF_SetOptions" );
    s_DynamicD3DPERF_GetStatus = (LPD3DPERF_GETSTATUS)GetProcAddress( hModule, "D3DPERF_GetStatus" );
    return true;
}

IDirect3D9 * WINAPI _Dynamic_Direct3DCreate9(UINT SDKVersion) 
{
    if( _EnsureD3DAPIs() && s_DynamicDirect3DCreate9 != NULL )
        return s_DynamicDirect3DCreate9( SDKVersion );
    else
        return NULL;
}

int WINAPI _Dynamic_D3DPERF_BeginEvent( D3DCOLOR col, LPCWSTR wszName )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_BeginEvent != NULL )
        return s_DynamicD3DPERF_BeginEvent( col, wszName );
    else
        return -1;
}

int WINAPI _Dynamic_D3DPERF_EndEvent( void )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_EndEvent != NULL )
        return s_DynamicD3DPERF_EndEvent();
    else
        return -1;
}

void WINAPI _Dynamic_D3DPERF_SetMarker( D3DCOLOR col, LPCWSTR wszName )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_SetMarker != NULL )
        s_DynamicD3DPERF_SetMarker( col, wszName );
}

void WINAPI _Dynamic_D3DPERF_SetRegion( D3DCOLOR col, LPCWSTR wszName )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_SetRegion != NULL )
        s_DynamicD3DPERF_SetRegion( col, wszName );
}

BOOL WINAPI _Dynamic_D3DPERF_QueryRepeatFrame( void )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_QueryRepeatFrame != NULL )
        return s_DynamicD3DPERF_QueryRepeatFrame();
    else
        return FALSE;
}

void WINAPI _Dynamic_D3DPERF_SetOptions( DWORD dwOptions )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_SetOptions != NULL )
        s_DynamicD3DPERF_SetOptions( dwOptions );
}

DWORD WINAPI _Dynamic_D3DPERF_GetStatus( void )
{
    if( _EnsureD3DAPIs() && s_DynamicD3DPERF_GetStatus != NULL )
        return s_DynamicD3DPERF_GetStatus();
    else
        return 0;
}


LPCSTR D3DFormatToString( D3DFORMAT format, bool bWithPrefix )
{
    char* pstr = NULL;
    switch( format )
    {
    case D3DFMT_UNKNOWN:         pstr = "D3DFMT_UNKNOWN"; break;
    case D3DFMT_R8G8B8:          pstr = "D3DFMT_R8G8B8"; break;
    case D3DFMT_A8R8G8B8:        pstr = "D3DFMT_A8R8G8B8"; break;
    case D3DFMT_X8R8G8B8:        pstr = "D3DFMT_X8R8G8B8"; break;
    case D3DFMT_R5G6B5:          pstr = "D3DFMT_R5G6B5"; break;
    case D3DFMT_X1R5G5B5:        pstr = "D3DFMT_X1R5G5B5"; break;
    case D3DFMT_A1R5G5B5:        pstr = "D3DFMT_A1R5G5B5"; break;
    case D3DFMT_A4R4G4B4:        pstr = "D3DFMT_A4R4G4B4"; break;
    case D3DFMT_R3G3B2:          pstr = "D3DFMT_R3G3B2"; break;
    case D3DFMT_A8:              pstr = "D3DFMT_A8"; break;
    case D3DFMT_A8R3G3B2:        pstr = "D3DFMT_A8R3G3B2"; break;
    case D3DFMT_X4R4G4B4:        pstr = "D3DFMT_X4R4G4B4"; break;
    case D3DFMT_A2B10G10R10:     pstr = "D3DFMT_A2B10G10R10"; break;
    case D3DFMT_A8B8G8R8:        pstr = "D3DFMT_A8B8G8R8"; break;
    case D3DFMT_X8B8G8R8:        pstr = "D3DFMT_X8B8G8R8"; break;
    case D3DFMT_G16R16:          pstr = "D3DFMT_G16R16"; break;
    case D3DFMT_A2R10G10B10:     pstr = "D3DFMT_A2R10G10B10"; break;
    case D3DFMT_A16B16G16R16:    pstr = "D3DFMT_A16B16G16R16"; break;
    case D3DFMT_A8P8:            pstr = "D3DFMT_A8P8"; break;
    case D3DFMT_P8:              pstr = "D3DFMT_P8"; break;
    case D3DFMT_L8:              pstr = "D3DFMT_L8"; break;
    case D3DFMT_A8L8:            pstr = "D3DFMT_A8L8"; break;
    case D3DFMT_A4L4:            pstr = "D3DFMT_A4L4"; break;
    case D3DFMT_V8U8:            pstr = "D3DFMT_V8U8"; break;
    case D3DFMT_L6V5U5:          pstr = "D3DFMT_L6V5U5"; break;
    case D3DFMT_X8L8V8U8:        pstr = "D3DFMT_X8L8V8U8"; break;
    case D3DFMT_Q8W8V8U8:        pstr = "D3DFMT_Q8W8V8U8"; break;
    case D3DFMT_V16U16:          pstr = "D3DFMT_V16U16"; break;
    case D3DFMT_A2W10V10U10:     pstr = "D3DFMT_A2W10V10U10"; break;
    case D3DFMT_UYVY:            pstr = "D3DFMT_UYVY"; break;
    case D3DFMT_YUY2:            pstr = "D3DFMT_YUY2"; break;
    case D3DFMT_DXT1:            pstr = "D3DFMT_DXT1"; break;
    case D3DFMT_DXT2:            pstr = "D3DFMT_DXT2"; break;
    case D3DFMT_DXT3:            pstr = "D3DFMT_DXT3"; break;
    case D3DFMT_DXT4:            pstr = "D3DFMT_DXT4"; break;
    case D3DFMT_DXT5:            pstr = "D3DFMT_DXT5"; break;
    case D3DFMT_D16_LOCKABLE:    pstr = "D3DFMT_D16_LOCKABLE"; break;
    case D3DFMT_D32:             pstr = "D3DFMT_D32"; break;
    case D3DFMT_D15S1:           pstr = "D3DFMT_D15S1"; break;
    case D3DFMT_D24S8:           pstr = "D3DFMT_D24S8"; break;
    case D3DFMT_D24X8:           pstr = "D3DFMT_D24X8"; break;
    case D3DFMT_D24X4S4:         pstr = "D3DFMT_D24X4S4"; break;
    case D3DFMT_D16:             pstr = "D3DFMT_D16"; break;
    case D3DFMT_L16:             pstr = "D3DFMT_L16"; break;
    case D3DFMT_VERTEXDATA:      pstr = "D3DFMT_VERTEXDATA"; break;
    case D3DFMT_INDEX16:         pstr = "D3DFMT_INDEX16"; break;
    case D3DFMT_INDEX32:         pstr = "D3DFMT_INDEX32"; break;
    case D3DFMT_Q16W16V16U16:    pstr = "D3DFMT_Q16W16V16U16"; break;
    case D3DFMT_MULTI2_ARGB8:    pstr = "D3DFMT_MULTI2_ARGB8"; break;
    case D3DFMT_R16F:            pstr = "D3DFMT_R16F"; break;
    case D3DFMT_G16R16F:         pstr = "D3DFMT_G16R16F"; break;
    case D3DFMT_A16B16G16R16F:   pstr = "D3DFMT_A16B16G16R16F"; break;
    case D3DFMT_R32F:            pstr = "D3DFMT_R32F"; break;
    case D3DFMT_G32R32F:         pstr = "D3DFMT_G32R32F"; break;
    case D3DFMT_A32B32G32R32F:   pstr = "D3DFMT_A32B32G32R32F"; break;
    case D3DFMT_CxV8U8:          pstr = "D3DFMT_CxV8U8"; break;
    default:                     pstr = "Unknown format"; break;
    }
    if( bWithPrefix || strstr( pstr, "D3DFMT_" )== NULL )
        return pstr;
    else
        return pstr + lstrlen( "D3DFMT_" );
}

//--------------------------------------------------------------------------------------
// Returns the number of depth bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
uint32 CApplication::DepthBits( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_D32F_LOCKABLE:
        case D3DFMT_D32:
            return 32;

        case D3DFMT_D24X8:
        case D3DFMT_D24S8:
        case D3DFMT_D24X4S4:
        case D3DFMT_D24FS8:
            return 24;

        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D16:
            return 16;

        case D3DFMT_D15S1:
            return 15;

        default:
            return 0;
    }
}




//--------------------------------------------------------------------------------------
// Returns the number of stencil bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
uint32 CApplication::StencilBits( D3DFORMAT fmt )
{
    switch( fmt )
    {
        case D3DFMT_D16_LOCKABLE:
        case D3DFMT_D16:
        case D3DFMT_D32F_LOCKABLE:
        case D3DFMT_D32:
        case D3DFMT_D24X8:
            return 0;

        case D3DFMT_D15S1:
            return 1;

        case D3DFMT_D24X4S4:
            return 4;

        case D3DFMT_D24S8:
        case D3DFMT_D24FS8:
            return 8;

        default:
            return 0;
    }
}

//--------------------------------------------------------------------------------------
// Returns the number of color channel bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
uint32 CApplication::ColorChannelBits( D3DFORMAT fmt )
{
    switch( fmt )
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
        case D3DFMT_A8B8G8R8:
            return 8;
        case D3DFMT_A2R10G10B10:
            return 10;
        case D3DFMT_A16B16G16R16:
            return 16;
        default:
            return 0;
    }
}




//--------------------------------------------------------------------------------------
// Returns the number of alpha channel bits in the specified D3DFORMAT
//--------------------------------------------------------------------------------------
uint32 CApplication::AlphaChannelBits( D3DFORMAT fmt )
{
    switch( fmt )
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
        case D3DFMT_A8B8G8R8:
            return 8;
        case D3DFMT_A2R10G10B10:
            return 2;
        case D3DFMT_A16B16G16R16:
            return 16;
        default:
            return 0;
    }
}

//--------------------------------------------------------------------------------------
// Multimon API handling for OSes with or without multimon API support
//--------------------------------------------------------------------------------------
#define _PRIMARY_MONITOR ((HMONITOR)0x12340042)
typedef HMONITOR (WINAPI* LPMONITORFROMWINDOW)(HWND, DWORD);
typedef BOOL     (WINAPI* LPGETMONITORINFO)(HMONITOR, LPMONITORINFO);

BOOL DynamicGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)
{
    static bool s_bInited = false;
    static LPGETMONITORINFO s_pFnGetMonitorInfo = NULL;
    if( !s_bInited )        
    {
        s_bInited = true;
        HMODULE hUser32 = GetModuleHandle( "USER32" );
        if (hUser32 ) 
        {
            OSVERSIONINFOA osvi = {0}; osvi.dwOSVersionInfoSize = sizeof(osvi); GetVersionExA((OSVERSIONINFOA*)&osvi);
            bool bNT = (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId);    
            s_pFnGetMonitorInfo = (LPGETMONITORINFO) (bNT ? GetProcAddress(hUser32,"GetMonitorInfoW") : GetProcAddress(hUser32,"GetMonitorInfoA"));
        }
    }

    if( s_pFnGetMonitorInfo ) 
        return s_pFnGetMonitorInfo(hMonitor, lpMonitorInfo);

    RECT rcWork;
    if ((hMonitor == _PRIMARY_MONITOR) && lpMonitorInfo && (lpMonitorInfo->cbSize >= sizeof(MONITORINFO)) && SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0))
    {
        lpMonitorInfo->rcMonitor.left = 0;
        lpMonitorInfo->rcMonitor.top  = 0;
        lpMonitorInfo->rcMonitor.right  = GetSystemMetrics(SM_CXSCREEN);
        lpMonitorInfo->rcMonitor.bottom = GetSystemMetrics(SM_CYSCREEN);
        lpMonitorInfo->rcWork = rcWork;
        lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;
        return TRUE;
    }
    return FALSE;
}


HMONITOR DynamicMonitorFromWindow(HWND hWnd, DWORD dwFlags)
{
    static bool s_bInited = false;
    static LPMONITORFROMWINDOW s_pFnGetMonitorFronWindow = NULL;
    if( !s_bInited )        
    {
        s_bInited = true;
        HMODULE hUser32 = GetModuleHandle( "USER32" );
        if (hUser32 ) s_pFnGetMonitorFronWindow = (LPMONITORFROMWINDOW) GetProcAddress(hUser32,"MonitorFromWindow");
    }

    if( s_pFnGetMonitorFronWindow ) 
        return s_pFnGetMonitorFronWindow(hWnd, dwFlags);
    if (dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST))
        return _PRIMARY_MONITOR;
    return NULL;
}


//--------------------------------------------------------------------------------------
// Get the desktop resolution of an adapter. This isn't the same as the current resolution 
// from GetAdapterDisplayMode since the device might be fullscreen 
//--------------------------------------------------------------------------------------
void CApplication::GetDesktopResolution( UINT AdapterOrdinal, UINT* pWidth, UINT* pHeight )
{
    CDeviceEnumeration* pd3dEnum = CDeviceEnumeration::GetEnumeration();
    CDeviceEnumAdapterInfo* pAdapterInfo = pd3dEnum->GetAdapterInfo( AdapterOrdinal );                       
    DEVMODE devMode;
    ZeroMemory( &devMode, sizeof(DEVMODE) );
    devMode.dmSize = sizeof(DEVMODE);
    char strDeviceName[256];
	strncpy (strDeviceName , pAdapterInfo->AdapterIdentifier.DeviceName , 256 );
    strDeviceName[255] = 0;
    EnumDisplaySettings( strDeviceName, ENUM_REGISTRY_SETTINGS, &devMode );
    
    if( pWidth )
        *pWidth = devMode.dmPelsWidth;
    if( pHeight )
        *pHeight = devMode.dmPelsHeight;
}

bool CApplication::OnIsDeviceAcceptable ( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool Windowed )
{
    // Typically want to skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = GetD3DObject(); 
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                    AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
                    D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}

bool CApplication::OnModifyDeviceSettings ( DeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps )
{
	return true;
}

HRESULT CApplication::OnDeviceCreated ( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
	return S_OK;
}

HRESULT CApplication::OnDeviceReset ( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc )
{
	return S_OK;
}

void CApplication::OnDeviceDestroyed ()
{
	WT::Graphics::CTexture::FlushTextureCache(); //Flush any cached textures for this device
	WT::Graphics::CMaterial::ResetCachedMaterial( WT::Graphics::CRenderContext(), m_D3DDevice );
}

void CApplication::OnDeviceLost ()
{
}

void CApplication::OnFrameMove ( IDirect3DDevice9* /*pDevice*/, double /*TotalTime*/, float ElapsedTime )
{
	//Update global simulation time - used for scrolling textures
	WT::Graphics::gSimulationTimeSeconds += ElapsedTime;
}

void CApplication::OnFrameRender ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
{
}

void CApplication::OnKeyboardEvent ( UINT Char, bool KeyDown, bool AltDown )
{
}

void CApplication::OnMouseEvent ( bool LeftButtonDown, bool RightButtonDown, bool MiddleButtonDown,
								  bool SideButton1Down, bool SideButton2Down, int MouseWheelDelta,
								  int XPos, int YPos )
{
}

void CApplication::OnTimer ( UINT TimerID )
{
}

LRESULT CApplication::OnMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam , bool& NoFurtherProcessing )
{
	return 0;
}

}
}
