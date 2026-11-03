/*
	mainmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _MAINMENU_H
#define _MAINMENU_H

#include "../menu.h"
#include "../../CHARACTER/gamesave.h"

class CRefManager;
class CGameStateManager;
class CKeyHandler;
class CSettings;
class CSprite;
class CFontMetric;

enum EMainMenuCode
{
	KMainMenuNewGame,
	KMainMenuContinueGame,
	KMainMenuOptions,
	KMainMenuHelp,
	KMainMenuCredits,
	KMainMenuQuit,
	KMainMenuWebsite,
	KMainMenuCodes
};

// splash menu

class CMainMenu:
		public CMenu
{
public:
	CMainMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CMenuManager& pMenuManager );			// menu manager

	~CMainMenu( void );

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	Create( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds

	void	Activate( void );

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage );	// windows message - WM_LBUTTONUP, etc.

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

private:

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	ProcessInput( HWND hWnd );								// hwnd of the active window


	FSOUND_SAMPLE*	m_pClickSound;

	// our logo texture
	CMaterial*	m_pLogoMaterial;

	CMaterial*	m_pDPLogoMaterial;

	// basic button material
	CMaterial*	m_pButtonMaterial;

	CButton*	m_pContinueButton;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CMaterial*		m_pFontMaterial2;
	CFontMetric*	m_pFontMetric;

	CText*			m_pVersionText;

	CGameSave*		m_pGameSave;
};

#endif