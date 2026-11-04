/*
	menu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _MENU_H
#define _MENU_H

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../UTILITIES/llist.h"

#include "../GAME/gamestatemanager.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "dropdown.h"

#include "menumanager.h"

const std::string KSetOnOff[] = 
{
	"OFF",
	"ON"
};

const std::string KSetHighLowOff[] = 
{
	"OFF",
	"LOW",
	"HIGH"
};

const std::string KSetReflections[] = 
{
	"OFF",
	"ON"
};

const std::string KSetNoneSomeLots[] = 
{
	"NONE",
	"SOME",
	"LOTS"
};

// base class for all menus - individual menus extend CMenu.

class CMenu
{
public:
	CMenu( CRefManager& pRefManager,				// the reference manager, for tracking media we create
		   CSettings& pSettings,					//we need this for tracking user preferences
		   CGameStateManager& pGameStateManager,	// game manager
		   CMenuManager& pMenuManager );			// menu manager

	virtual	~CMenu( void );

	virtual void	Activate( void );

	virtual void	Deactivate( void );

	virtual void	Create( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	virtual void	Update( HWND hWnd,								// hwnd of the active window
							float32 TimeElapsed );					// time elapsed, in seconds


	virtual void	Render( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	virtual void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	virtual void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							  WPARAM KeyCode );	// the wparam which will be the keycode

	virtual void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONDOWN, etc.
								WPARAM Param );		// the wparam which will be the button code

	virtual void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	{	};// direct3d device

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	virtual bool	Has3DBackdrop( void )					{	return kTrue;					};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	
	void			UpdateElements()						{	m_UpdateElements = kTrue;		};

protected:

	virtual void	ProcessInput( HWND hWnd );								// hwnd of the active window

	CSprite*		AddSprite( CSprite* pSprite );				// pointer to sprite to add to internal list

	CSprite*		AddSpriteFront( CSprite* pSprite );			// pointer to sprite to add to internal list

	void			RemoveSprite( CSprite* pSprite );			// pointer to sprite to remove from internal list

	void			DestroySprites( void );

	void			RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CButton*		AddButton( CButton* pButton );			// pointer to button to add to internal list

	void			RemoveButton( CButton* pButton );		// pointer to button to remove from internal list

	void			UpdateButtons( float32 TimeElapsed );		// time elapsed in seconds

	int32			UpdateButtonPress( bool ButtonPressed,			// is the left mouse button pressed?
									   bool PlaySounds = kTrue );	// play sounds?

	void			DestroyButtons();

	void			RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CText*			AddText( CText* pText );			// pointer to text to add to internal list

	void			RemoveText( CText* pText );			// pointer to text to remove from internal list

	void			DestroyText();

	void			RenderText( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CDropdown*		AddDropdown( CDropdown* pDropdown );				// pointer to dropdown to add to internal list

	CDropdown*		AddDropdownFront( CDropdown* pDropdown );			// pointer to dropdown to add to internal list

	void			RemoveDropdown( CDropdown* pDropdown );			// pointer to dropdown to remove from internal list

	void			DestroyDropdowns( void );

	void			RenderDropdowns( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void			CloseDropdowns( void );

	CKeyHandler				m_KeyHandler;
	CMouseHandler			m_MouseHandler;

	CSettings&				m_pSettings;		
	CRefManager&			m_pRefManager;
	CGameStateManager&		m_pGameStateManager;
	CMenuManager&			m_pMenuManager;

	CLList<CButton>*		m_pButtonList;
	CLList<CSprite>*		m_pSpriteList;
	CLList<CText>*			m_pTextList;
	CLList<CDropdown>*		m_pDropdownList;


	bool					m_UpdateElements;		// onscreen info has been changed, and text/ui should be recreated
};

#endif