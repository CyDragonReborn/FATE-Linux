/*
	logomenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _LOGOMENU_H
#define _LOGOMENU_H

#include "../menu.h"

class CRefManager;
class CGameStateManager;
class CKeyHandler;
class CSettings;
class CSprite;
class CFontMetric;
class CScreenFade;

// logo menu

class CLogoMenu:
		public CMenu
{
public:
	CLogoMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
			   CSettings& pSettings,					//we need this for tracking user preferences
			   CGameStateManager& pGameStateManager,	// game manager
			   CMenuManager& pMenuManager );			// menu manager

	~CLogoMenu( void );

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
	bool	Has3DBackdrop( void )						{	return kFalse;					};


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

private:

	// these methods are overriding the methods in CMenu that this
	// class is derived from

	void	ProcessInput( HWND hWnd );								// hwnd of the active window

	CScreenFade*	m_pScreenFade;

	float32			m_FadeValue;
	float32			m_FadeDirection;

	// our logo texture
	CMaterial*		m_pLogoMaterial;
	CMaterial*		m_pWTLogoMaterial;

	float32			m_HoldTime;


};

#endif