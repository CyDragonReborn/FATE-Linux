/*
	dx8app.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _DX8APP_H
#define _DX8APP_H
#include <string>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "d3denumeration.h"
#include "d3dsettings.h"

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


// CDX8App is base class for creating D3D applications. To create a
// Direct3D application, simply derive this class into a class (such as
// class CMyApp) and override the following functions, as needed:
//  		OneTimeSceneInit()    - To initialize app data (alloc mem, etc.)
//  		InitDeviceObjects()   - To initialize the 3D scene objects
//  		FrameMove() 		  - To animate the scene
//  		Render()			  - To render the scene
//  		DeleteDeviceObjects() - To cleanup the 3D scene objects
//  		FinalCleanup()  	  - To cleanup app data (for exitting the app)
//  		MsgProc()   		  - To handle Windows messages


class CDX8App
{

public:
	// Functions to create, run, pause, and clean up the application
	virtual HRESULT			Create( HINSTANCE hInstance );
	virtual int32			Run();
	virtual LRESULT			MsgProc( HWND hWnd, 
									 UINT uMsg, 
									 WPARAM wParam, 
									 LPARAM lParam );
	virtual ~CDX8App()
	{
	}

	// Internal constructor
	CDX8App();

	CD3DSettings			m_D3DSettings;
	D3DPRESENT_PARAMETERS	m_D3Dpp;  			// Parameters for CreateDevice/Reset
	LPDIRECT3D8				m_pD3D; 			// The main D3D object
	LPDIRECT3DDEVICE8		m_pD3DDevice; 		// The D3D rendering device

	void					SetCursorVisibility( bool Visible )	{	m_ShowCursor = Visible;	};

	void					SetStartupResolution( uint32 Width,		// screen width
												  uint32 Height );	// screen height

	void					SetStartupFullscreen( bool Fullscreen )	{	m_StartFullscreen = Fullscreen;	};

	void					SetWindowTitle( const std::string& Title );	// title to set for window

	bool					DeviceLost( void )				{	return m_DeviceLost;		};

	bool					Minimized( void )				{	return m_Minimized;			};
	bool					Maximized( void )				{	return m_Maximized;			};

	bool					Windowed( void )				{	return m_Windowed;			};
	bool					Fullscreen( void )				{	return !m_Windowed;			};

	bool					WindowedFSAA( void )			{	return m_WindowedFSAA;		};
	bool					FullscreenFSAA( void )			{	return m_FullscreenFSAA;	};

	bool					SupportsWindowedFSAA( void )	{	return m_SupportsWindowedFSAA;		};
	bool					SupportsFullscreenFSAA( void )	{	return m_SupportsFullscreenFSAA;	};

	bool					CanSwitchFSAA( void )			{	return m_CanSwitchFSAA;	};

	bool					CanStartFullscreen( void )		{	return m_CanStartFullscreen;		};

	bool					AllowFSAA( void )				{	return m_AllowFSAA;		};

	void					SetAllowFSAA( bool Allow )		{	m_AllowFSAA = Allow;	};

	void					SetRequiresVSYNC( bool State )	{	m_RequiresVSYNC = State;	};

	bool					FullscreenResolutionExists( uint32 Width,		// screen width
														uint32 Height );	// screen height

	bool					RequiresVSYNC( void )			{	return m_RequiresVSYNC;	};

	bool					IsVSYNCed( void )				{	return m_IsVSYNCed;		};

	// tell us whether the application is active, and not changing devices, etc.
	bool					ApplicationActive( void )	{	return m_Active;		};

	float32					FPS( void )				{	return m_FPS;				};
	std::string				FPSText( void )			{	return m_FrameStats;		};

	bool					Focused( void )			{	return m_Focused;			};


protected:
	HANDLE					m_Semaphore;

	CD3DEnumeration*		m_pEnumeration;		// enumerator for devices

	D3DCAPS8				m_D3DCaps; 			// Caps for the device


	D3DSURFACE_DESC			m_BackBufferSurface;// Surface desc of the backbuffer

	HWND					m_hWnd;				// The main app window
	HMENU					m_hMenu;  			// App menu bar (stored here when fullscreen)
	
	DWORD					m_WindowStyle;		// Saved window style for mode switches
	std::string				m_WindowTitle;		// Title for the app's window
	uint32					m_WindowWidth;		// Width used to create window
	uint32					m_WindowHeight;		// Height used to create window

	RECT					m_WindowBounds;		// Saved window bounds for mode switches
	RECT					m_WindowClient;		// Saved client area size for mode switches

	// Internal variables for the state of the app
	bool					m_Focused;
	bool					m_RequiresVSYNC;
	bool					m_IsVSYNCed;
	bool					m_ShuttingDown;
	bool					m_Windowed;
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
	bool					m_FullscreenFSAA;			// Whether to show cursor when fullscreen
	bool					m_WindowedFSAA;				// Whether to show cursor when fullscreen
	bool					m_SupportsFullscreenFSAA;	// Whether to show cursor when fullscreen
	bool					m_SupportsWindowedFSAA;		// Whether to show cursor when fullscreen
	bool					m_CanSwitchFSAA;			// Whether this device can switch FSAA on the fly.
	bool					m_CanStartFullscreen;		// Whether this device can start fullscreen successfully

	bool					m_AllowFSAA;				// whether to allow FSAA at creation time

	int32					m_RenderFailures;

	// Variables for timing
	float32					m_FPS;   					// frames per second
	TCHAR					m_FrameStats[90];			// String to hold frame stats

	// Internal error handling function
	HRESULT			DisplayErrorMsg( HRESULT hr, 
									 DWORD dwType );

	void			BuildPresentParamsFromSettings();
	bool			FindBestWindowedMode( );
	bool			IsFullscreenResolutionAvailable( uint32 Width,		// screen width
													 uint32 Height );	// screen height
	bool			FindBestFullscreenMode( );
	HRESULT			ChooseInitialD3DSettings();
	HRESULT			Initialize3DEnvironment();
	HRESULT			HandlePossibleSizeChange();
	HRESULT			ToggleFullscreen();
	HRESULT			ChangeResolution( uint32 NewWidth,			// width to change to
									  uint32 NewHeight );		// height to change to
	void			Cleanup3DEnvironment();
	HRESULT			Render3DEnvironment();
	HRESULT			Reset3DEnvironment();
	virtual HRESULT	AdjustWindowForChange();


	virtual HRESULT OneTimeSceneInit()			{	return S_OK;	};
	virtual HRESULT PostInitialization()		{	return S_OK;	};
	virtual HRESULT InitDeviceObjects()			{	return S_OK;	};
	virtual HRESULT RestoreDeviceObjects()		{	return S_OK;	};
	virtual HRESULT FrameMove()					{	return S_OK;	};
	virtual HRESULT Render()					{	return S_OK;	};
	virtual HRESULT InvalidateDeviceObjects()	{	return S_OK;	};
	virtual HRESULT DeleteDeviceObjects()		{	return S_OK;	};
	virtual HRESULT FinalCleanup()				{	return S_OK;	};
	
	virtual void	LButtonDown()				{					};
	virtual void	LButtonUp()					{					};				

	virtual bool ShouldClose()					{	return kFalse;	};
	virtual HRESULT UnFocus()					{	m_Focused = kFalse;	return S_OK;	};
	virtual HRESULT Focus()						{	m_Focused = kTrue;	return S_OK;	};

};




#endif



