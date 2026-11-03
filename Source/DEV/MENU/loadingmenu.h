/*
	loadingmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _LOADINGMENU_H
#define _LOADINGMENU_H

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

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "dropdown.h"

class CTips;

// generic intermediate loading menu

class CLoadingMenu
{
public:
	CLoadingMenu( CRefManager& pRefManager,				// the reference manager, for tracking media we create
				  CSettings& pSettings );				//we need this for tracking user preferences

	virtual	~CLoadingMenu( void );

	virtual void	Activate( void );

	virtual void	Deactivate( void );

	virtual void	Create( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	virtual void	Update( HWND hWnd,								// hwnd of the active window
							float32 TimeElapsed );					// time elapsed, in seconds


	virtual void	Render( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	virtual void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
							  bool Flip );						// flip immediately?

	virtual void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							  WPARAM KeyCode );	// the wparam which will be the keycode

	virtual void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONDOWN, etc.
								WPARAM Param );		// the wparam which will be the button code
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	
	void			SetDisplayTips( bool State )			{	m_DisplayTips = State;			};

	void			UpdateElements()						{	m_UpdateElements = kTrue;		};

	void			SetProgressPercentage( float32 Value )	{	m_ProgressPercentage = Value;	};

	void			SetMessageText( const std::string& Text )	{	m_Text = Text;				};

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

	CLList<CButton>*		m_pButtonList;
	CLList<CSprite>*		m_pSpriteList;
	CLList<CText>*			m_pTextList;
	CLList<CDropdown>*		m_pDropdownList;

	CMaterial*				m_pPopupMenu;
	CMaterial*				m_pBottomMenuCenter;

	CMaterial*				m_pDPLogoMaterial;

	CSprite*				m_pProgressBar;

	float32					m_ProgressPercentage;

	CText*					m_pMessageText;

	std::string				m_Text;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	CTips*			m_pTips;

	bool			m_DisplayTips;

	bool			m_UpdateElements;		// onscreen info has been changed, and text/ui should be recreated

	float32			m_LastTipTime;
};

#endif