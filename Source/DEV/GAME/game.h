/*
	game.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/


#ifndef _GAME_H
#define _GAME_H

#include "../DIRECT3D/Dx8App.h"
#include "../UTILITIES/UserLocaleInfo.h"
#include "steam_api.h"

class CSettings;
class CCapabilities;
class CSoundManager;
class CRefManager;
class CFontMetric;
class CText;
class CMaterial;
class CGameClient;
class CGameStateManager;
class CMenuManager;
class CLoadingMenu;
class CStatsAndAchievements;

// this is a generic template - the class is derived from CDX8App,
// which manages the D3D device and the render loop, and provides methods
// which are overrided by the 'game' class
class CGame:
	public CDX8App
{
public:
							CGame( std::string Params );	// command line parameters
							~CGame( void );
	HWND					GetHWND(){ return m_hWnd; } //marsh - PAL needs the window handle
	void					SetFullscreenAfterPal(); //marsh - since we want the game to go into fullscreen, we need to make it after PAL is done.
	
	void					DropFromFullScreenPal(); //marsh

	static CUserLocaleInfo theUserLocaleInfo;  // TDC
	static std::string		GetLocaleString(); // TDC
	static HMODULE			GetLangDLL(); // TDC

	bool					InitializeSteam(); //Initializes Steam Library and Process
	void					ShutDownSteam(); //Initializes Steam Library and Process

	STEAM_CALLBACK( CGame, OnGameOverlayActivated, GameOverlayActivated_t, m_CallbackGameOverlayActivated );

private:
	static HMODULE localizedDLL;  //TDC
	static std::string localeString;  //TDC
	static std::wstring wlocaleString;  //TDC
	static HMODULE DetermineLocaleDLL();  //TDC
	static std::string DetermineLocaleString();  //TDC

	void					PreRender( void );

	void					RenderUIElements( void );

	void					RenderLoadingMenu( bool Flip );	// force a flip?

	void					Initialize( void );

	void					PostInitialize( void );

	void					FirstFrame( void );

	void					ClearOldDDS( std::string Path );		// path to the folder to look in

	void					ConvertToDDS( std::string Path );		// path to the folder to look in

	void					GetTexturesFromFolder( std::string Path,						// path to the folder to look in
													std::vector< std::string >& Paths,	// list of paths to add to
													uint32& Depth,						// subfolder depth
													uint32& FilesChecked );				// number of files checked

	std::string				SelectEmptyScreenshot( std::string Path );						// path to the music folder to look in

	void					TakeScreenshot( void );


	// these methods are overriding the methods in CDX8App that this
	// class is derived from

	HRESULT					OneTimeSceneInit( void );
	HRESULT					InitDeviceObjects( void );

	HRESULT					FrameMove( void );

	HRESULT					Focus( void );

	HRESULT					UnFocus( void );

	HRESULT					Render( void );
	
	HRESULT					InvalidateDeviceObjects( void );
	HRESULT					RestoreDeviceObjects( void );
	
	HRESULT					DeleteDeviceObjects( void );

	HRESULT					FinalCleanup( void );

	LRESULT					MsgProc( HWND hWnd, 
									 UINT uMsg, 
									 WPARAM wParam, 
									 LPARAM lParam );	

	bool					ShouldClose( void )			{	return m_ShouldClose;	};	// return true when we're ready to shut down

	CMenuManager*			m_pMenuManager;

	CGameStateManager*		m_pGameStateManager;
	CGameClient*			m_pGameClient;

	CSettings*				m_pSettings;
	CCapabilities*			m_pCapabilities;

	CSoundManager*			m_pSoundManager;

	CRefManager*			m_pRefManager;

	CFontMetric*			m_pFontMetric;
	CText*					m_pFPSText;

	bool					m_ShouldClose;			// set to true when you want to shut down

	bool					m_Initialized;
	bool					m_PostInitialized;

	float32					m_VersionNumber;

	CLoadingMenu*			m_pLoadingMenu;

	bool					m_FirstFrameRendered;
	bool					m_FirstFrameInitialized;

	//[BLG] IF (10-18-2013) Steam Support
	bool		m_bSteamInitialized;

};

#endif