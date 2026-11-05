/*
	buysellmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _BUYSELLMENU_H
#define _BUYSELLMENU_H

#include "d3dx8_shim.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../GAME/gamestatemanager.h"
#include "../CHARACTER/inventory.h"

enum EBuySellMenuButtons
{
	KBuySellButtonStats,
	KBuySellButtonBuySell,
	KBuySellButtonMap,
	KBuySellButtonOptions,
	KBuySellButtons
};

class CKeyHandler;
class CSprite;
class CMaterial;
class CGameStateManager;
class CCharacter;
class CButton;
class CText;
class CItem;

const int32 KBuySellButton( 5000 );

// manage the ingame ui and events

class CBuySellMenu
{
public:
				CBuySellMenu( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							  CRefManager& pRefManager,		// the reference manager, for tracking media we create
							  CSettings& pSettings,			//we need this for tracking user preferences
							  CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CBuySellMenu( void );

	void	Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds


	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
				    const D3DXMATRIX& ViewportMatri,		// viewport matrix ( for bringing 3d objects to screen space
					const D3DXMATRIX& ProjectionMatrix,		// projection matrix  ( for bringing 3d objects to screen space
					CItem* pMouseOverItem,					// mouseover item
					bool ShowSockets );						// show sockets for items?


	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info


////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	
	bool	Open( void )			{	return m_Open;		};

	bool	MouseOver( void );

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	FlushAll( void );

	void	SetMerchantCharacter( CCharacter* pCharacter )		{	m_pMerchantCharacter = pCharacter;		};

	void	SetPlayerCharacter( CCharacter* pCharacter )		{	m_pPlayerCharacter = pCharacter;		};

	void	SetOpen( bool Open )	{	m_Open = Open;		};

	int32	UpdateButtonPress( bool ButtonPressed );	// is the left mouse button pressed?

	int32	UpdateButtonOver( void );

	int32	UpdateButtonOver( std::string& TooltipText );	// tooltip text to fill out

private:
	void	CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void	ProcessInput( HWND hWnd );						// hwnd of the active window

	void			BringToFront( CSprite* pSprite );			// pointer to sprite in the internal list

	CSprite*		AddSprite( CSprite* pSprite );				// pointer to sprite to add to internal list

	CSprite*		AddSpriteFront( CSprite* pSprite );			// pointer to sprite to add to internal list

	void			RemoveSprite( CSprite* pSprite );			// pointer to sprite to remove from internal list

	void			DestroySprites( void );

	void			RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CButton*		AddButton( CButton* pButton );			// pointer to button to add to internal list

	void			RemoveButton( CButton* pButton );		// pointer to button to remove from internal list

	void			UpdateButtons( float32 TimeElapsed );		// time elapsed in seconds

	void			DestroyButtons();

	void			RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CText*			AddText( CText* pText );			// pointer to text to add to internal list

	void			RemoveText( CText* pText );			// pointer to text to remove from internal list

	void			DestroyText();

	void			RenderText( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void			RenderBuySellIcons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void			RenderBuySellIconsSockets( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
												CItem* pMouseOverItem,					// mouseover item
												bool ShowSockets );						// show sockets for items?

	CKeyHandler				m_KeyHandler;
	CMouseHandler			m_MouseHandler;

	CSettings&				m_pSettings;		
	CRefManager&			m_pRefManager;
	CGameStateManager&		m_pGameStateManager;

	CLList<CButton>*		m_pButtonList;
	CLList<CSprite>*		m_pSpriteList;
	CLList<CText>*			m_pTextList;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CFontMetric*	m_pFontMetric;

	CSprite*		m_pSocketSprite;
	CMaterial*		m_pSocketMaterial;

	CSprite*		m_pMiniSprite;

	CCharacter*		m_pMerchantCharacter;
	CCharacter*		m_pPlayerCharacter;

	// BuySell menu
	CMaterial*		m_pBuySellMenuTop;
	CMaterial*		m_pBuySellMenuCenter;
	CMaterial*		m_pBuySellMenuBottom;

	CMaterial*		m_pErrorIconMaterial;
	CMaterial*		m_pMagicIconMaterial;
	CMaterial*		m_pUniqueIconMaterial;
	CMaterial*		m_pArtifactIconMaterial;
	CMaterial*		m_pQuestItemMaterial;
	CMaterial*		m_pGradedItemMaterial;
	CMaterial*		m_pEliteItemMaterial;

	CMaterial*		m_pUIButtons;

	bool			m_Open;

	FSOUND_SAMPLE*	m_pClickSound;
};

#endif