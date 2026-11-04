/*
	retiremenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _RETIREMENU_H
#define _RETIREMENU_H

#include "../menu.h"

class CRefManager;
class CGameStateManager;
class CKeyHandler;
class CSettings;
class CSprite;
class CFontMetric;
class CNameGenerator;
class CSoundManager;
class CScreenFade;
class CGameClient;

enum ERetireMenuCode
{
	KRetireMenuAccept,
	KRetireMenuContinue,
	KRetireMenuCodes
};


class CRetireMenu:
		public CMenu
{
public:
	CRetireMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CSoundManager& pSoundManager,			// sound manager
			   CGameClient& pGameClient,				// game client
			   CMenuManager& pMenuManager );			// menu manager

	~CRetireMenu( void );

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	Create( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds

	void	Activate( void );

	void	Deactivate( void );

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage );	// windows message - WM_LBUTTONUP, etc.

	void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool	Has3DBackdrop( void )						{	return kFalse;					};


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

private:

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	Begin( void );

	void	ProcessInput( HWND hWnd );								// hwnd of the active window

	FSOUND_SAMPLE*	m_pClickSound;

	// our intro texture
	CMaterial*	m_pIntroMaterial;

	// basic button material
	CMaterial*	m_pButtonMaterial;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	CSoundManager&	m_pSoundManager;

	CGameClient&	m_pGameClient;

	bool			m_UpdateText;

	CScreenFade*	m_pScreenFade;

	float32			m_FadeValue;
	float32			m_FadeDirection;

	CButton*		m_pContinueButton;
	CButton*		m_pRetireButton;

	CText*			m_pText;

	bool			m_Continue;
};

#endif