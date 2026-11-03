/*
	newpetmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _NEWPETMENU_H
#define _NEWPETMENU_H

#include "../menu.h"

class CRefManager;
class CGameStateManager;
class CKeyHandler;
class CSettings;
class CSprite;
class CFontMetric;
class CNameGenerator;

enum ENewPetMenuCode
{
	KNewPetMenuBack,
	KNewPetMenuAccept,
	KNewPetMenuRandomName,
	KNewPetMenuDog,
	KNewPetMenuCat,
	KNewPetMenuCodes
};



class CNewPetMenu:
		public CMenu
{
public:
	CNewPetMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CMenuManager& pMenuManager );			// menu manager

	~CNewPetMenu( void );

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

	void	Begin( void );

	void	ProcessInput( HWND hWnd );								// hwnd of the active window

	FSOUND_SAMPLE*	m_pClickSound;

	// our logo texture
	CMaterial*	m_pLogoMaterial;

	// basic button material
	CMaterial*	m_pButtonMaterial;
	CMaterial*	m_pUIButtonMaterial2;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	std::string		m_Name;

	CText*			m_pNameText;

	float32			m_BlinkTimer;

	CNameGenerator*	m_pNameGenerator;

	bool			m_UpdatePlayer;

	bool			m_IsDog;

	CButton*		m_pCatButton;
	CButton*		m_pDogButton;

};

#endif