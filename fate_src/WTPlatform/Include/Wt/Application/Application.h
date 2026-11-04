//
// Application.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// The Application class serves as a base class for creating Direct3D-based
// applications.  The primary responsibilities of this class are to create 
// the application window, create the Direct3D device object, run the Windows 
// message pump associated with the application window, and dispatch frame 
// update and render calls to the derived application class.   
//

#pragma once

#undef     IsMinimized
#undef     IsMaximized

#include <string>
#include <queue>
#include <sstream>
#include <iomanip>
#include "WT/System/Object.h"
#include "WT/System/Timer.h"
#include "WT/Math/Math.h"
#include "WT/Application/DeviceSettings.h"
#include "WT/Graphics/UI/SystemFont.h"
#include "WT/Graphics/UI/Text.h"
#include <d3d9.h>

namespace WT
{
namespace Application
{

//-----------------------------------------------------------------------------
// Error codes
//-----------------------------------------------------------------------------
enum APPMSGTYPE
{
	MSG_NONE,
	MSGERR_APPMUSTEXIT,
	MSGWARN_SWITCHEDTOREF
};

#define D3DAPPERR_NODIRECT3D		  0x82000001
#define D3DAPPERR_NOWINDOW  		  0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL 	  0x82000007
#define D3DAPPERR_NODESKTOPHAL  	  0x82000008
#define D3DAPPERR_NOHALTHISMODE 	  0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND 	  0x8200000b
#define D3DAPPERR_RESETFAILED   	  0x8200000c
#define D3DAPPERR_NULLREFDEVICE 	  0x8200000d


// Application is base class for creating D3D applications
// To create a Direct3D application, simply derive from this class 
// override the following functions as needed:
//  	OneTimeSceneInit()    - To initialize application data (memory allocation, etc.)
//  	InitDeviceObjects()   - To initialize the 3D scene objects
//		PostCreate()		  - Called after window has been created and 3D environment initialized
//  	FrameMove() 		  - To animate the scene
//  	Render()			  - To render the scene
//  	DeleteDeviceObjects() - To cleanup the 3D scene objects
//  	FinalCleanup()  	  - To cleanup application data (when exitting the app)
//  	MsgProc()   		  - To handle Windows messages

class CApplication : public WT::System::CObject
{
public:

	// Sets command-line parameters passed to WinMain
	virtual void SetCommandLine( const std::string& CommandLine );

	// Retrieves the original command-line parameters passed to WinMain
	virtual std::string GetCommandLine() const;

	// Creates the application window and initial Direct3D objects
	virtual HRESULT Create( HINSTANCE hInstance, HICON hIcon , HMENU hMenu , bool UseFrame );

	// Runs the Windows message pump for the application and renders the scene during idle time
	virtual int32 Run();

	// Processes messages received from the main windows message pump
	virtual LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	// Processes messages received from the frame window message pump
	virtual LRESULT FrameMsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

	// Override this to rebuild the framework when needed (especially if using layered window for the frame)
	virtual void UpdateFrameWindow( HWND hWnd );

	// Override to provide custom hit testing for the frame window
	virtual LRESULT FrameWindowHitTest( HWND hWnd , int X , int Y );

	// Override to handle movement of the frame window yourself. default is to reposiotn the
	// rendering window within the frame given the border sizes
	virtual void FrameWindowMove ( HWND hWnd );

	// Override to handle resize of the frame window yourself. default is to resize the
	// rendering window within the frame given the border sizes
	virtual void FrameWindowResize ( HWND hWnd );

	// Retrieves the top-level window associated with the Application instance
	HWND GetHWND();

	// Retrieves the DeviceSettings structure associated with the Application instance
	CDeviceSettings& GetDeviceSettings();

	// Retrieves the Direct3D D3DPRESENT_PARAMETERS structure associated with the Application instance
	D3DPRESENT_PARAMETERS& GetPresentParameters();

	// Associates the given IDirect3D9 pointer with the Application instance
	void SetD3D( LPDIRECT3D9 pD3D );

	// Retrieves the IDirect3D9 pointer associated with the Application instance
	LPDIRECT3D9 GetD3D();

	// Associates the given IDirect3DDevice9 pointer with the Application instance
	void SetD3DDevice( LPDIRECT3DDEVICE9 pD3DDevice );

	// Retrieves the IDirect3DDevice9 pointer associated with the Application instance
	LPDIRECT3DDEVICE9 GetD3DDevice() const;

	IDirect3DStateBlock9* GetDefaultRenderStateAll() const;
	
	IDirect3DStateBlock9* GetDefaultRenderStatePixel() const;

	IDirect3DStateBlock9* GetDefaultRenderStateVertex() const;

	void SetCursorVisibility( bool ShowCursor );

	void SetStartupResolution( uint32 Width, uint32 Height );

	void SetStartupFullscreen( bool StartupFullscreen );

	void SetWindowTitle( const std::string& Title );

	void SetWindowStyle( DWORD WindowStyle );

	void SetStartCentered( bool StartCentered );

	bool IsDeviceLost() const;

	bool IsMinimized() const;

	bool IsMaximized() const;

	bool IsWindowed() const;

	bool IsFullscreen() const;

	bool IsWindowedFSAA() const;

	bool IsFullscreenFSAA() const;

	bool SupportsWindowedFSAA() const;

	bool SupportsFullscreenFSAA() const;

	bool CanSwitchFSAA() const;

	bool CanStartFullscreen() const;

	bool AllowFSAA() const;

	void SetAllowFSAA( bool AllowFSAA );

	void SetRequiresVerticalSync( bool RequiresVerticalSync );

	bool FullscreenResolutionExists( uint32 Width, uint32 Height ) const;

	bool RequiresVerticalSync();

	bool IsVerticalSynced();

	// Determines whether the application is active, and not changing devices
	bool IsActive() const;

	float32 GetFramerate() const;

	std::string GetFramerateText() const;

	virtual std::string FillFrameStats();

	bool IsFocused() const;

	bool IsFramerateDisplayed() const;

	void SetDisplayFramerate( bool DisplayFramerate );

	CApplication();
	virtual ~CApplication();

	virtual std::string GetSemaphoreName() const {return gm_DefaultSemaphoreName;}
	virtual std::string GetClassName() const {return gm_DefaultClassName;}

protected:	// methods

	// Internal error handling function; displays error messages in a message box
	HRESULT DisplayErrorMsg( HRESULT hr, DWORD dwType );

	// Builds presentation parameters from the settings assembled based upon initial device enumeration
	void BuildPresentParamsFromSettings();

	// Choose the best available display mode for windowed mode
    bool FindBestWindowedMode();

	// Determines whether a given resolution is available
	bool IsFullscreenResolutionAvailable( uint32 width, uint32 height );

	// Chooses the best available display mode for fullscreen
	bool FindBestFullscreenMode();

	// Assembles the settings for fullscreen and windowed modes given what is known about the hardware from device enumeration
	HRESULT ChooseInitialD3DSettings();

	// Initializes the Direct3D device using previously cached settings
	HRESULT Initialize3DEnvironment();

	// Resets the device if the client area has changed
	HRESULT HandlePossibleSizeChange();

	// Toggles the application between fullscreen and windowed modes
	HRESULT ToggleFullscreen();

	// Performs a resolution change from one size to another
	// Note this is nto the same as toggling fullscreen
	HRESULT ChangeResolution( uint32 newWidth, uint32 newHeight );

	// Frees the Direct3D device any any allocated Direct3D objects
	void Cleanup3DEnvironment();

	// Renders the current scene
	HRESULT Render3DEnvironment();

	// Resets the Direct3D device
	HRESULT Reset3DEnvironment();

	// Prepares the application window for a resolution or windowed/fullscreen change
	virtual HRESULT	AdjustWindowForChange();

	virtual HRESULT OneTimeSceneInit()			{	return S_OK;	}
	virtual HRESULT InitDeviceObjects()			{	return S_OK;	}
	virtual HRESULT PostCreate()				{	return S_OK;	}
	virtual HRESULT RestoreDeviceObjects()		{	return S_OK;	}
	virtual HRESULT FrameMove()					{	return S_OK;	}
	virtual HRESULT Render()					{	return S_OK;	}
	virtual HRESULT InvalidateDeviceObjects()	{	return S_OK;	}
	virtual HRESULT DeleteDeviceObjects()		{	return S_OK;	}
	virtual HRESULT FinalCleanup()				{	return S_OK;	}

	virtual bool ShouldClose();
	
	virtual HRESULT UnFocus();

	virtual HRESULT Focus();

	// Allows the application to process key down events
	// Called if the application class is configured to process key events
	virtual void OnKeyDown( uint8 Key );

	// Allows the application to process key up events
	// Called if the application class is configured to process key events
	virtual void OnKeyUp( uint8 Key );

	// Allows the application to process mouse button events
	// Called if the application class is configured to process mouse events
	virtual void OnMouseButton( UINT_PTR VirtualKeyMask, bool Pressed, const Math::CVector2i& MousePos );

	// Allows the application to process mouse movement
	// Called if the application class is configured to process mouse events
	virtual void OnMouseMove( const Math::CVector2i& MousePos );

	// Sets up and cleans up Font/Text objects for framerate display
	bool InitStatusDisplayObjects();
	void DeleteStatusDisplayObjects();

	// Saves the default render state of current device into state blocks
	void SaveDeviceStates();
	void DeleteDeviceStates();

protected:	// data

	std::string				m_CommandLine;			// Command-line string passed to WinMain
	HANDLE					m_hInstanceSemaphore;
	HINSTANCE				m_hInstance;
	HWND					m_hWnd;					// The main app window
	HWND					m_hFrameWnd;			// The frame window that wraps the DirectX window (if exists)
	HMENU					m_hMenu;  				// App menu bar (stored here when fullscreen)
	DWORD					m_WindowStyle;			// Saved window style for mode switches
	DWORD					m_FrameStyle;			// Saved window style for mode switches
	std::string				m_WindowTitle;			// Title for the app's window
	uint32					m_WindowWidth;			// Width used to create window
	uint32					m_WindowHeight;			// Height used to create window
	int32					m_LeftFrameBorder;		// Width of the left frame border
	int32					m_RightFrameBorder;		// Width of the right frame border
	int32					m_TopFrameBorder;		// Height of the Top frame border
	int32					m_BottomFrameBorder;	// Height of the Bottom frame border

	RECT					m_FrameBounds;
	RECT					m_WindowBounds;			// Saved window bounds for mode switches
	RECT					m_WindowClient;			// Saved client area size for mode switches

	CDeviceSettings			m_DeviceSettings;
	D3DPRESENT_PARAMETERS	m_PresentParameters;  	// parameters for CreateDevice/Reset
	LPDIRECT3D9				m_pD3D;					// main Direct3D object
	LPDIRECT3DDEVICE9		m_pD3DDevice; 			// Direct3D rendering device
	D3DCAPS9				m_D3DCaps; 				// Caps for the device
	D3DSURFACE_DESC			m_BackBufferSurfaceDesc;// surface description of the backbuffer

	CDeviceEnumeration*		m_pDeviceEnumeration;	// enumerator for devices
	
	bool					m_CreateMultithreadedDevice; // informs framework to create D3D devicse using D3DCREATE_MULTITHREADED flag
	bool					m_ProcessKeyEvents;		// whether to handle key events in the Windows message proc 
	bool					m_ProcessMouseEvents;	// whether to handle mouse events in the Windows message proc 
	bool					m_Focused;
	bool					m_RequiresVerticalSync;
	bool					m_IsVerticalSynced;
	bool					m_ShuttingDown;
	bool					m_Windowed;
	bool					m_Created;
	bool					m_Active;
	bool					m_DeviceLost;
	bool					m_Minimized;
	bool					m_Maximized;
	bool					m_IgnoreSizeChange;
	bool					m_DeviceObjectsInitialized;
	bool					m_DeviceObjectsRestored;
	bool					m_ShowCursor;				// Whether to show cursor 
	bool					m_ClipCursorWhenFullscreen; // Whether to limit cursor pos when fullscreen
	bool					m_StartFullscreen;			// Whether to start up the app in fullscreen mode
	bool					m_StartCentered;			// If starting windowed (not fullscreen); whether to center the window on the desktop
	bool					m_FullscreenFSAA;			// Whether to show cursor when fullscreen
	bool					m_WindowedFSAA;				// Whether to show cursor when fullscreen
	bool					m_SupportsFullscreenFSAA;	// Whether to show cursor when fullscreen
	bool					m_SupportsWindowedFSAA;		// Whether to show cursor when fullscreen
	bool					m_CanSwitchFSAA;			// Whether this device can switch FSAA on the fly.
	bool					m_CanStartFullscreen;		// Whether this device can start fullscreen successfully

	bool					m_AllowFSAA;				// whether to allow FSAA at creation time

	int32					m_RenderFailures;			// counts render failures

	// Render state blocks
	IDirect3DStateBlock9*	m_pDefaultAllState;			// All device render states (Vertex, Pixel)
	IDirect3DStateBlock9*	m_pDefaultPixelState;		// Pixel only device render states
	IDirect3DStateBlock9*	m_pDefaultVertexState;		// Vertex only device render states

	// Status display
	bool					  m_DisplayFramerate;		// Display m_FrameStats in upper left corner of screen
	bool					  m_DisplayExtraStats;		// Display additional stats in upper left corner of screen
	Graphics::UI::CSystemFont* m_pStatusFont;			// Framerate stats display font
	Graphics::UI::CText*	  m_pFRText;				// Framerate stats display text
	Graphics::UI::CText*	  m_pStatsText;				// Extra stats display text
	std::string				  m_DeviceDescription;		// String device description

	// Variables for timing
	System::CTimer			m_GameTimer;
	float32					m_Framerate;   				// frames per second
	std::string				m_FrameStats;				// buffer to hold frame statistics

public:	// global framework data

	static std::string		gm_DefaultSemaphoreName;
	static std::string		gm_DefaultClassName;
	static CApplication*	gm_pInstance;				// provides global access to the app (needed for the global WndProc())
};

template<typename T>
inline int APIENTRY WinMainImpl( HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow, 
	const std::string& WindowTitle, WORD wIconRes = (WORD)-1 , WORD MenuRes = (WORD)-1 , bool UseFrame = false )
{
	nCmdShow;

	T App;

	App.SetCommandLine( lpCmdLine );
	App.SetWindowTitle( WindowTitle );

	HICON hIcon = NULL;
	if( IS_INTRESOURCE( wIconRes ) )
	{
		hIcon = ::LoadIcon( hInstance, MAKEINTRESOURCE( wIconRes ) );
	}

	HMENU hMenu = NULL;
	if( IS_INTRESOURCE( MenuRes ) )
	{
		hMenu = ::LoadMenu( hInstance, MAKEINTRESOURCE( MenuRes ) );
	}

	if ( FAILED( App.Create( hInstance, hIcon , hMenu , UseFrame ) ) )
	{
		return -2;
	}

	return App.Run();
}

}
}
