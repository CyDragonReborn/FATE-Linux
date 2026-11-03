//
//	Application2.h
//
//	Copyright (C) 2005-6 WildTangent, Inc. 
//	All Rights Reserved
//
// Overview:
//	This class provides global application management for a direct3d based application
//
#pragma once

#include "d3dx9.h"

#include "WT/System/Object.h"
#include "WT/System/Callback.h"
#include "WT/System/Timer2.h"
#include "WT/Application/DeviceEnumeration2.h"
#include "WT/Graphics/UI/SystemFont.h"
#include "WT/Graphics/UI/Text.h"

#include <atlbase.h>

namespace WT
{
namespace Application2
{

//--------------------------------------------------------------------------------------
// forward declerations
//--------------------------------------------------------------------------------------

class CDeviceEnumeration;
class CApplication;

//--------------------------------------------------------------------------------------
// Error codes
//--------------------------------------------------------------------------------------
#define ERR_NODIRECT3D              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0901)
#define ERR_NOCOMPATIBLEDEVICES     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0902)
#define ERR_MEDIANOTFOUND           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0903)
#define ERR_NONZEROREFCOUNT         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0904)
#define ERR_CREATINGDEVICE          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0905)
#define ERR_RESETTINGDEVICE         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0906)
#define ERR_CREATINGDEVICEOBJECTS   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0907)
#define ERR_RESETTINGDEVICEOBJECTS  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0908)
#define ERR_INCORRECTVERSION        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0909)


//--------------------------------------------------------------------------------------
// Automatically enters & leaves the CS upon object creation/deletion
//--------------------------------------------------------------------------------------
class CApplicationLock
{
public:
    inline CApplicationLock( bool ThreadSafe , CRITICAL_SECTION* CS)
	{
		m_ThreadSafe = ThreadSafe;
		if ( ThreadSafe )
		{
			m_CS = CS;
			EnterCriticalSection( CS );
		}
	}
    inline ~CApplicationLock()
	{
		if ( m_ThreadSafe )
			LeaveCriticalSection( m_CS );
	}

private:

	bool m_ThreadSafe;
	CRITICAL_SECTION* m_CS;
};

//--------------------------------------------------------------------------------------
// Helper macros to build member functions that access member variables with thread safety
//--------------------------------------------------------------------------------------

#define SET_ACCESSOR( x, y )       inline void Set##y( x t )  { CApplicationLock Lock(m_ThreadSafe , &m_CS); m_##y = t; };
#define GET_ACCESSOR( x, y )       inline x Get##y() { CApplicationLock Lock(m_ThreadSafe , &m_CS); return m_##y; };
#define GET_SET_ACCESSOR( x, y )   SET_ACCESSOR( x, y ) GET_ACCESSOR( x, y )

#define SETP_ACCESSOR( x, y )      inline void Set##y( x* t )  { CApplicationLock Lock(m_ThreadSafe , &m_CS); m_##y = *t; };
#define GETP_ACCESSOR( x, y )      inline x* Get##y() { CApplicationLock Lock(m_ThreadSafe , &m_CS); return &m_##y; };
#define GETP_SETP_ACCESSOR( x, y ) SETP_ACCESSOR( x, y ) GETP_ACCESSOR( x, y )


//--------------------------------------------------------------------------------------
// Stores timer callback info
//--------------------------------------------------------------------------------------

struct CApplicationTimer
{
	typedef System::CCallback1<int>	TriggerCallback;
	typedef TriggerCallback::Event	TriggerEvent;

	TriggerEvent	Event;
    float32			TimeoutInSecs;
    float32			Countdown;
    bool			Enabled;
};

namespace EShowStats
{
	enum ShowStats
	{
		DontShow,
		OnlyFPS,
		FPSAndBuffer,
		Full
	};
}
class CApplication : public WT::System::CObject
{
public:
	CApplication();
	virtual ~CApplication();

	static CApplication* GetApplication() { return gm_pInstance; }
	//--------------------------------------------------------------------------------------
	// Initialization
	//--------------------------------------------------------------------------------------
	HRESULT Init( const std::string& SemaphorName = "WTPlatfrom App", bool bHandleDefaultHotkeys = true, bool bShowMsgBoxOnError = true, bool bHandleAltEnter = true );

	// Choose either CreateRenderWindow or SetRenderWindow.  If using SetRenderWindow, consider using StaticWndProc
	HRESULT CreateRenderWindow( const char* strWindowTitle = "WildTangent Platform Window", 
							HINSTANCE hInstance = NULL, HICON hIcon = NULL, HMENU hMenu = NULL,
							int x = CW_USEDEFAULT, int y = CW_USEDEFAULT );
	HRESULT SetRenderWindow( HWND hWndFocus, HWND hWndDeviceFullScreen, HWND hWndDeviceWindowed, bool bHandleMessages = true );

	// Choose either CreateDevice or SetDevice or CreateDeviceFromSettings
	HRESULT CreateDevice( UINT AdapterOrdinal = D3DADAPTER_DEFAULT, bool bWindowed = true, 
							int nSuggestedWidth = 0, int nSuggestedHeight = 0 );
	HRESULT CreateDeviceFromSettings( DeviceSettings* pDeviceSettings, bool bPreserveInput = false, bool bClipWindowToSingleAdapter = true );
	HRESULT SetDevice( IDirect3DDevice9* pd3dDevice );

	// Choose either MainLoop or implement your own main loop 
	HRESULT MainLoop( HACCEL hAccel = NULL );

	// If not using MainLoop consider using Render3DEnvironment
	void Render3DEnvironment(); 

	//--------------------------------------------------------------------------------------
	// virtual functions that can be overrided to provide rendering and animation
	//--------------------------------------------------------------------------------------

	virtual bool	OnIsDeviceAcceptable ( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool Windowed );
	virtual bool	OnModifyDeviceSettings ( DeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps );
	virtual HRESULT	OnDeviceCreated ( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
	virtual HRESULT OnDeviceReset ( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc );
	virtual void	OnDeviceDestroyed ();
	virtual void	OnDeviceLost ();
	virtual void	OnFrameMove ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTimeSeconds );
	virtual void	OnFrameRender ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTimeSeconds );
	virtual LRESULT OnMessage ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam , bool& NoFurtherProcessing );
	virtual void	OnKeyboardEvent ( UINT Char, bool KeyDown, bool AltDown );
	virtual void	OnMouseEvent ( bool LeftButtonDown, bool RightButtonDown, bool MiddleButtonDown, bool SideButton1Down,
	                               bool SideButton2Down, int MouseWheelDelta, int XPos, int YPos );
	virtual void	OnTimer ( UINT TimerID );

	//--------------------------------------------------------------------------------------
	// Common Tasks 
	//--------------------------------------------------------------------------------------

	void    SetCursorSettings( bool bShowCursorWhenFullScreen, bool bClipCursorWhenFullScreen );
	void    SetMultimonSettings( bool bAutoChangeAdapter );
	void    SetShortcutKeySettings( bool bAllowWhenFullscreen = false, bool bAllowWhenWindowed = true ); // Controls the Windows key, and accessibility shortcut keys
	void    SetWindowSettings( bool bCallDefWindowProc = true );
	void    SetConstantFrameTimeData( bool bConstantFrameTime, float fTimePerFrame = 0.0333f );
	HRESULT SetTimer ( CApplicationTimer::TriggerCallback* Callback, float fTimeoutInSecs = 1.0f, UINT* pnIDEvent = NULL );
	HRESULT KillTimer( UINT nIDEvent );
	HRESULT ToggleFullScreen();
	HRESULT ToggleREF();
	void    Pause( bool bPauseTime, bool bPauseRendering );
	void    ResetFrameworkState();
	void    Shutdown( int nExitCode = 0 );

	//This function chooses a log file based on the specified name, creates the file if necessary,
	//and opens the log file in WordPad. This function does nothing unless you are using WTPlatform1(D).lib, 
	//which is built with WTLOG_ENABLED.
	static void	WTCall LaunchWordPadLog( std::string const& Name );

	HRESULT FindValidDeviceSettings( DeviceSettings* pOut, DeviceSettings* pIn = NULL, MatchOptions* pMatchOptions = NULL );

	//--------------------------------------------------------------------------------------
	// State Retrieval  
	//--------------------------------------------------------------------------------------

	IDirect3D9*             GetD3DObject(); // Does not addref unlike typical Get* APIs
	DeviceSettings			GetDeviceSettings(); 
	D3DPRESENT_PARAMETERS   GetPresentParameters();
	const D3DCAPS9*         GetDeviceCaps();
	HINSTANCE               GetHINSTANCE();
	HWND                    GetHWND();
	uint32					GetWindowWidth();
	uint32					GetWindowHeight();
	RECT                    GetWindowClientRect();
	RECT                    GetWindowClientRectAtModeChange(); // Useful for returning to windowed mode with the same resolution as before toggle to full screen mode
	RECT                    GetFullsceenClientRectAtModeChange(); // Useful for returning to full screen mode with the same resolution as before toggle to windowed mode
	bool                    IsWindowed();
	LPCSTR                  GetFrameStatsString( bool bIncludeFPS = false );
	bool                    IsRenderingPaused();
	bool                    IsTimePaused();
	bool                    IsActive();
	bool                    IsKeyDown( BYTE vKey ); // Pass a virtual-key code, ex. VK_F1, 'A', VK_RETURN, VK_LSHIFT, etc
	bool                    IsMouseButtonDown( BYTE vButton ); // Pass a virtual-key code: VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2
	System::CTimer2*		GetTimer();
	static void				GetDesktopResolution( UINT AdapterOrdinal, UINT* pWidth, UINT* pHeight );

private:

	System::CTimer2			m_Timer;					//Our global application timer
	static CApplication*	gm_pInstance;				// provides global access to the app (needed for the global WndProc())
	typedef IDirect3D9* (WINAPI* LPDIRECT3DCREATE9)(UINT SDKVersion);
	int     MapButtonToArrayIndex( BYTE vButton );
	CDeviceEnumeration* PrepareEnumerationObject( bool bEnumerate = false );
	void    BuildOptimalDeviceSettings( DeviceSettings* pOptimalDeviceSettings, DeviceSettings* pDeviceSettingsIn, MatchOptions* pMatchOptions );
	bool    DoesDeviceComboMatchPreserveOptions( CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo, DeviceSettings* pDeviceSettingsIn, MatchOptions* pMatchOptions );
	float   RankDeviceCombo( CDeviceEnumDeviceSettingsCombo* pDeviceSettingsCombo, DeviceSettings* pDeviceSettingsIn, D3DDISPLAYMODE* pAdapterDesktopDisplayMode );
	void    BuildValidDeviceSettings( DeviceSettings* pDeviceSettings, CDeviceEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, DeviceSettings* pDeviceSettingsIn, MatchOptions* pMatchOptions );
	HRESULT FindValidResolution( CDeviceEnumDeviceSettingsCombo* pBestDeviceSettingsCombo, D3DDISPLAYMODE displayModeIn, D3DDISPLAYMODE* pBestDisplayMode );
	HRESULT FindAdapterFormat( UINT AdapterOrdinal, D3DDEVTYPE DeviceType, D3DFORMAT BackBufferFormat, BOOL Windowed, D3DFORMAT* pAdapterFormat );
	HRESULT ChangeDevice( DeviceSettings* pNewDeviceSettings, IDirect3DDevice9* pd3dDeviceFromApp, bool bForceRecreate, bool bClipWindowToSingleAdapter );
	void    UpdateDeviceSettingsWithOverrides( DeviceSettings* pNewDeviceSettings );
	HRESULT Create3DEnvironment( IDirect3DDevice9* pd3dDeviceFromApp );
	HRESULT Reset3DEnvironment();
	void    Cleanup3DEnvironment( bool bReleaseSettings = true );
	void    UpdateFrameStats();
	void	UpdateFrameStatsText();
	void    UpdateDeviceStats( D3DDEVTYPE DeviceType, DWORD BehaviorFlags, D3DADAPTER_IDENTIFIER9* pAdapterIdentifier );
	void    UpdateStaticFrameStats();
	void    HandleTimers();
	void    DisplayErrorMessage( HRESULT hr );
	static LRESULT CALLBACK StaticWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual LRESULT PrivateWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void    CheckForWindowSizeChange();
	void    CheckForWindowChangingMonitors();
	static uint32    ColorChannelBits( D3DFORMAT fmt );
	static uint32    AlphaChannelBits( D3DFORMAT fmt );
	static uint32    StencilBits( D3DFORMAT fmt );
	static uint32    DepthBits( D3DFORMAT fmt );
	HRESULT GetAdapterOrdinalFromMonitor( HMONITOR hMonitor, UINT* pAdapterOrdinal );
	void    AllowShortcutKeys( bool bAllowKeys );
	void    UpdateBackBufferDesc();
	void    SetupCursor();
	void	RenderFrameStats();
	HRESULT SetDeviceCursor( IDirect3DDevice9* pd3dDevice, HCURSOR hCursor, bool bAddWatermark );
	static LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );

protected:

	CRITICAL_SECTION m_CS;
	bool m_ThreadSafe;

    IDirect3D9*		        m_D3D;						// the main D3D object
    IDirect3DDevice9*		m_D3DDevice;					// the D3D rendering device
    CDeviceEnumeration*     m_D3DEnumeration;			// CDeviceEnumeration object
    DeviceSettings*			m_CurrentDeviceSettings;	// current device settings
    D3DSURFACE_DESC			m_BackBufferSurfaceDesc;	// back buffer surface description
    D3DCAPS9				m_Caps;						// D3D caps for current device
    HWND					m_HWNDFocus;				// the main app focus window
    HWND					m_HWNDDeviceFullScreen;		// the main app device window in fullscreen mode
    HWND					m_HWNDDeviceWindowed;		// the main app device window in windowed mode
    HMONITOR				m_AdapterMonitor;			// the monitor of the adapter 
    HMENU					m_Menu;						// handle to menu

    UINT m_FullScreenBackBufferWidthAtModeChange;		// back buffer size of fullscreen mode right before switching to windowed mode.  Used to restore to same resolution when toggling back to fullscreen
    UINT m_FullScreenBackBufferHeightAtModeChange;		// back buffer size of fullscreen mode right before switching to windowed mode.  Used to restore to same resolution when toggling back to fullscreen
    UINT m_WindowBackBufferWidthAtModeChange;			// back buffer size of windowed mode right before switching to fullscreen mode.  Used to restore to same resolution when toggling back to windowed mode
    UINT m_WindowBackBufferHeightAtModeChange;			// back buffer size of windowed mode right before switching to fullscreen mode.  Used to restore to same resolution when toggling back to windowed mode
    DWORD m_WindowedStyleAtModeChange;					// window style
    WINDOWPLACEMENT m_WindowedPlacement; // record of windowed HWND position/show state/etc
    bool  m_TopmostWhileWindowed;       // if true, the windowed HWND is topmost 
    bool  m_Minimized;                  // if true, the HWND is minimized
    bool  m_Maximized;                  // if true, the HWND is maximized
    bool  m_MinimizedWhileFullscreen;   // if true, the HWND is minimized due to a focus switch away when fullscreen mode
    bool  m_IgnoreSizeChange;           // if true,  won't reset the device upon HWND size change

    double m_Time;                      // current time in seconds
    float m_ElapsedTime;                // time elapsed since last frame in seconds

    HINSTANCE m_HInstance;              // handle to the app instance
    double m_LastStatsUpdateTime;       // last time the stats were updated
    DWORD m_LastStatsUpdateFrames;      // frames count since last time the stats were updated
    float m_FPS;                        // frames per second
    int   m_CurrentFrameNumber;         // the current frame number
    HHOOK m_KeyboardHook;               // handle to keyboard hook
    bool  m_AllowShortcutKeysWhenFullscreen; // if true, when fullscreen enable shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
    bool  m_AllowShortcutKeysWhenWindowed;   // if true, when windowed enable shortcut keys (Windows keys, StickyKeys shortcut, ToggleKeys shortcut, FilterKeys shortcut) 
    bool  m_AllowShortcutKeys;          // if true, then shortcut keys are currently disabled (Windows key, etc)
    bool  m_CallDefWindowProc;          // if true, StaticWndProc will call DefWindowProc for unhandled messages. Applications rendering to a dialog may need to set this to false.
    STICKYKEYS m_StartupStickyKeys;     // StickyKey settings upon startup so they can be restored later
    TOGGLEKEYS m_StartupToggleKeys;     // ToggleKey settings upon startup so they can be restored later
    FILTERKEYS m_StartupFilterKeys;     // FilterKey settings upon startup so they can be restored later

    bool  m_HandleDefaultHotkeys;       // if true, then  will handle some default hotkeys
    bool  m_HandleAltEnter;             // if true, then  will handle Alt-Enter
    bool  m_ShowMsgBoxOnError;          // if true, then msgboxes are displayed upon errors
    bool  m_NoStats;                    // if true, then GetFrameStats() and GetDeviceStats() will return blank strings
    bool  m_ClipCursorWhenFullScreen;   // if true, then  will keep the cursor from going outside the window when full screen
    bool  m_ShowCursorWhenFullScreen;   // if true, then  will show a cursor when full screen
    bool  m_ConstantFrameTime;          // if true, then elapsed frame time will always be 0.05f seconds which is good for debugging or automated capture
    float m_TimePerFrame;               // the constant time per frame in seconds, only valid if m_ConstantFrameTime==true
    bool  m_WireframeMode;              // if true, then D3DRS_FILLMODE==D3DFILL_WIREFRAME else D3DRS_FILLMODE==D3DFILL_SOLID 
    bool  m_AutoChangeAdapter;          // if true, then the adapter will automatically change if the window is different monitor
    bool  m_WindowCreatedWithDefaultPositions; // if true, then CW_USEDEFAULT was used and the window should be moved to the right adapter
    int   m_ExitCode;                   // the exit code to be returned to the command line

    bool  m_Inited;						// if true, then Init() has succeeded
    bool  m_WindowCreated;              // if true, then CreateWindow() or SetWindow() has succeeded
    bool  m_DeviceCreated;              // if true, then CreateDevice*() or SetDevice() has succeeded

    bool  m_InitCalled;					// if true, then Init() was called
    bool  m_WindowCreateCalled;         // if true, then CreateWindow() or SetWindow() was called
    bool  m_DeviceCreateCalled;         // if true, then CreateDevice*() or SetDevice() was called

    bool  m_DeviceObjectsCreated;       // if true, then DeviceCreated callback has been called (if non-NULL)
    bool  m_DeviceObjectsReset;         // if true, then DeviceReset callback has been called (if non-NULL)
    bool  m_InsideDeviceCallback;       // if true, then the framework is inside an app device callback
    bool  m_InsideMainloop;             // if true, then the framework is inside the main loop
    bool  m_Active;                     // if true, then the app is the active top level window
    bool  m_TimePaused;                 // if true, then time is paused
    bool  m_RenderingPaused;            // if true, then rendering is paused
    int   m_PauseRenderingCount;        // pause rendering ref count
    int   m_PauseTimeCount;             // pause time ref count
    bool  m_DeviceLost;                 // if true, then the device is lost and needs to be reset
    bool  m_NotifyOnMouseMove;          // if true, include WM_MOUSEMOVE in mousecallback
    bool  m_Automation;                 // if true, automation is enabled
    bool  m_InSizeMove;                 // if true, app is inside a WM_ENTERSIZEMOVE

    int   m_OverrideAdapterOrdinal;     // if != -1, then override to use this adapter ordinal
    bool  m_OverrideWindowed;           // if true, then force to start windowed
    bool  m_OverrideFullScreen;         // if true, then force to start full screen
    int   m_OverrideStartX;             // if != -1, then override to this X position of the window
    int   m_OverrideStartY;             // if != -1, then override to this Y position of the window
    int   m_OverrideWidth;              // if != 0, then override to this width
    int   m_OverrideHeight;             // if != 0, then override to this height
    bool  m_OverrideForceHAL;           // if true, then force to HAL device (failing if one doesn't exist)
    bool  m_OverrideForceREF;           // if true, then force to REF device (failing if one doesn't exist)
    bool  m_OverrideForcePureHWVP;      // if true, then force to use pure HWVP (failing if device doesn't support it)
    bool  m_OverrideForceHWVP;          // if true, then force to use HWVP (failing if device doesn't support it)
    bool  m_OverrideForceSWVP;          // if true, then force to use SWVP 
    bool  m_OverrideConstantFrameTime;  // if true, then force to constant frame time
    float m_OverrideConstantTimePerFrame; // the constant time per frame in seconds if m_OverrideConstantFrameTime==true
    int   m_OverrideQuitAfterFrame;     // if != 0, then it will force the app to quit after that frame
    int   m_OverrideForceVsync;         // if == 0, then it will force the app to use D3DPRESENT_INTERVAL_IMMEDIATE, if == 1 force use of D3DPRESENT_INTERVAL_DEFAULT
	EShowStats::ShowStats m_ShowStats;	// show frame stats options

    bool	m_Keys[256];							// array of key state
    bool	m_MouseButtons[5];						// array of mouse states

	std::vector<CApplicationTimer>*	m_TimerList;	// list of CApplicationTimer structs
    char	m_StaticFrameStats[256];				// static part of frames stats 
    char	m_FPSStats[64];							// fps stats
    char	m_FrameStats[256];						// frame stats (fps, width, etc)
    char	m_DeviceStats[256];						// device stats (description, device type, etc)
    char	m_WindowTitle[256];						// window title

	Graphics::UI::CSystemFont*	m_pFrameStatsFont;	// Frame stats display font
	Graphics::UI::CText*		m_pFrameStatsText;	// Frame stats display text object

	HANDLE	m_hInstanceSemaphore;
	void	CreateStatusDisplayObjects();
	void	DeleteStatusDisplayObjects();
    void	CreateState();
    void	DestroyState();

public:

	void SetThreadSafe ( bool ThreadSafe ) { m_ThreadSafe = ThreadSafe; }
	bool GetThreadSafe () { return m_ThreadSafe; }

    // Macros to define access functions for thread safe access
    GET_SET_ACCESSOR( IDirect3D9*, D3D );

    GET_SET_ACCESSOR( IDirect3DDevice9*, D3DDevice );
    GET_SET_ACCESSOR( CDeviceEnumeration*, D3DEnumeration );   
    GET_SET_ACCESSOR( DeviceSettings*, CurrentDeviceSettings );   
    GETP_SETP_ACCESSOR( D3DSURFACE_DESC, BackBufferSurfaceDesc );
    GETP_SETP_ACCESSOR( D3DCAPS9, Caps );

    GET_SET_ACCESSOR( HWND, HWNDFocus );
    GET_SET_ACCESSOR( HWND, HWNDDeviceFullScreen );
    GET_SET_ACCESSOR( HWND, HWNDDeviceWindowed );
    GET_SET_ACCESSOR( HMONITOR, AdapterMonitor );
    GET_SET_ACCESSOR( HMENU, Menu );   

    GET_SET_ACCESSOR( UINT, FullScreenBackBufferWidthAtModeChange );
    GET_SET_ACCESSOR( UINT, FullScreenBackBufferHeightAtModeChange );
    GET_SET_ACCESSOR( UINT, WindowBackBufferWidthAtModeChange );
    GET_SET_ACCESSOR( UINT, WindowBackBufferHeightAtModeChange );
    GETP_SETP_ACCESSOR( WINDOWPLACEMENT, WindowedPlacement );
    GET_SET_ACCESSOR( DWORD, WindowedStyleAtModeChange );
    GET_SET_ACCESSOR( bool, TopmostWhileWindowed );
    GET_SET_ACCESSOR( bool, Minimized );
    GET_SET_ACCESSOR( bool, Maximized );
    GET_SET_ACCESSOR( bool, MinimizedWhileFullscreen );
    GET_SET_ACCESSOR( bool, IgnoreSizeChange );   

    GET_SET_ACCESSOR( double, Time );
    GET_SET_ACCESSOR( float, ElapsedTime );

    GET_SET_ACCESSOR( HINSTANCE, HInstance );
    GET_SET_ACCESSOR( double, LastStatsUpdateTime );   
    GET_SET_ACCESSOR( DWORD, LastStatsUpdateFrames );   
    GET_SET_ACCESSOR( float, FPS );    
    GET_SET_ACCESSOR( int, CurrentFrameNumber );
    GET_SET_ACCESSOR( HHOOK, KeyboardHook );
    GET_SET_ACCESSOR( bool, AllowShortcutKeysWhenFullscreen );
    GET_SET_ACCESSOR( bool, AllowShortcutKeysWhenWindowed );
    GET_SET_ACCESSOR( bool, AllowShortcutKeys );
    GET_SET_ACCESSOR( bool, CallDefWindowProc );
    GET_SET_ACCESSOR( STICKYKEYS, StartupStickyKeys );
    GET_SET_ACCESSOR( TOGGLEKEYS, StartupToggleKeys );
    GET_SET_ACCESSOR( FILTERKEYS, StartupFilterKeys );

    GET_SET_ACCESSOR( bool, HandleDefaultHotkeys );
    GET_SET_ACCESSOR( bool, HandleAltEnter );
    GET_SET_ACCESSOR( bool, ShowMsgBoxOnError );
    GET_SET_ACCESSOR( bool, NoStats );
    GET_SET_ACCESSOR( bool, ClipCursorWhenFullScreen );   
    GET_SET_ACCESSOR( bool, ShowCursorWhenFullScreen );
    GET_SET_ACCESSOR( bool, ConstantFrameTime );
    GET_SET_ACCESSOR( float, TimePerFrame );
    GET_SET_ACCESSOR( bool, WireframeMode );   
    GET_SET_ACCESSOR( bool, AutoChangeAdapter );
    GET_SET_ACCESSOR( bool, WindowCreatedWithDefaultPositions );
    GET_SET_ACCESSOR( int, ExitCode );

    GET_SET_ACCESSOR( bool, Inited );
    GET_SET_ACCESSOR( bool, WindowCreated );
    GET_SET_ACCESSOR( bool, DeviceCreated );
    GET_SET_ACCESSOR( bool, InitCalled );
    GET_SET_ACCESSOR( bool, WindowCreateCalled );
    GET_SET_ACCESSOR( bool, DeviceCreateCalled );
    GET_SET_ACCESSOR( bool, InsideDeviceCallback );
    GET_SET_ACCESSOR( bool, InsideMainloop );
    GET_SET_ACCESSOR( bool, DeviceObjectsCreated );
    GET_SET_ACCESSOR( bool, DeviceObjectsReset );
    GET_SET_ACCESSOR( bool, Active );
    GET_SET_ACCESSOR( bool, RenderingPaused );
    GET_SET_ACCESSOR( bool, TimePaused );
    GET_SET_ACCESSOR( int, PauseRenderingCount );
    GET_SET_ACCESSOR( int, PauseTimeCount );
    GET_SET_ACCESSOR( bool, DeviceLost );
    GET_SET_ACCESSOR( bool, NotifyOnMouseMove );
    GET_SET_ACCESSOR( bool, Automation );
    GET_SET_ACCESSOR( bool, InSizeMove );

    GET_SET_ACCESSOR( int, OverrideAdapterOrdinal );
    GET_SET_ACCESSOR( bool, OverrideWindowed );
    GET_SET_ACCESSOR( bool, OverrideFullScreen );
    GET_SET_ACCESSOR( int, OverrideStartX );
    GET_SET_ACCESSOR( int, OverrideStartY );
    GET_SET_ACCESSOR( int, OverrideWidth );
    GET_SET_ACCESSOR( int, OverrideHeight );
    GET_SET_ACCESSOR( bool, OverrideForceHAL );
    GET_SET_ACCESSOR( bool, OverrideForceREF );
    GET_SET_ACCESSOR( bool, OverrideForcePureHWVP );
    GET_SET_ACCESSOR( bool, OverrideForceHWVP );
    GET_SET_ACCESSOR( bool, OverrideForceSWVP );
    GET_SET_ACCESSOR( bool, OverrideConstantFrameTime );
    GET_SET_ACCESSOR( float, OverrideConstantTimePerFrame );
    GET_SET_ACCESSOR( int, OverrideQuitAfterFrame );
    GET_SET_ACCESSOR( int, OverrideForceVsync );

	GET_SET_ACCESSOR( std::vector<CApplicationTimer>*, TimerList );   
    GET_ACCESSOR( bool*, Keys );
    GET_ACCESSOR( bool*, MouseButtons );
    GET_ACCESSOR( char*, StaticFrameStats );
    GET_ACCESSOR( char*, FPSStats );
    GET_ACCESSOR( char*, FrameStats );
    GET_ACCESSOR( char*, DeviceStats );    
    GET_ACCESSOR( char*, WindowTitle );

	GET_SET_ACCESSOR(EShowStats::ShowStats, ShowStats);
};

const GUID ApplicationLibraryID = {0x2DC1CA2F,0x0FBE,0x4D2D,0xB9,0x4B,0x5B,0x6A,0xA5,0xD5,0x9B,0x46};

class CAtlApplicationModule : public CAtlExeModuleT< CAtlApplicationModule >
{
public :
	DECLARE_LIBID(ApplicationLibraryID)
};

template<typename T>
inline int APIENTRY WinMainImpl ( const std::string& WindowTitle , const std::string& SemaphoreName = "WTPlatform Application" )
{
	T App;
	if ( FAILED(App.Init( SemaphoreName, true, true, true )))		// handle the default hotkeys, and show msgboxes
		return -1;									// App failed to create, meaning there was another instance running
													// so exit.

	App.SetCursorSettings( true, true );			// Show the cursor and clip it when in full screen
	App.CreateRenderWindow( WindowTitle.c_str() );
	App.CreateDevice( D3DADAPTER_DEFAULT, true, 640, 480 );

	// Start the render loop
	App.MainLoop();

	// Shut down the application after main loop is done
	App.Shutdown();

	//return the error code to the calling winmain
	return App.GetExitCode();
}


}
}
