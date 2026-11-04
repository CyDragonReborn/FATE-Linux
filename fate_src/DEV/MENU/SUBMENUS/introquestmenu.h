/*
	introquestmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _INTROQUESTMENU_H
#define _INTROQUESTMENU_H

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
class CQuest;

enum EIntroQuestMenuCode
{
	KIntroQuestMenuAccept,
	KIntroQuestMenuCodes
};



class CIntroQuestMenu:
		public CMenu
{
public:
	CIntroQuestMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CSoundManager& pSoundManager,			// sound manager
			   CMenuManager& pMenuManager );			// menu manager

	~CIntroQuestMenu( void );

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
	CMaterial*	m_pIntroMaterial2;

	// basic button material
	CMaterial*	m_pButtonMaterial;

	CMaterial*	m_pGradientMaterial;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	CSoundManager&	m_pSoundManager;

	CSprite*		m_pIntro1;
	CSprite*		m_pIntro2;

	CScreenFade*	m_pScreenFade;

	float32			m_FadeValue;
	float32			m_FadeDirection;

	CButton*		m_pContinueButton;

	CQuest*			m_pQuest;

	CText*			m_pText;

	float32			m_SpriteYPosition;
};

#endif