/*
	enchantmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _ENCHANTMENU_H
#define _ENCHANTMENU_H

#include "d3dx8_shim.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../GAME/gamestatemanager.h"
class CKeyHandler;
class CSprite;
class CMaterial;
class CGameStateManager;
class CCharacter;
class CButton;
class CText;
class CItem;
const int32 KEnchantItemButton( 999 );
// manage the ingame ui and events

class CEnchantMenu
{
public:
				CEnchantMenu( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							CRefManager& pRefManager,		// the reference manager, for tracking media we create
							CSettings& pSettings,			//we need this for tracking user preferences
							CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CEnchantMenu( void );

	void	Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds


	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
				    const D3DXMATRIX& ViewportMatri,		// viewport matrix ( for bringing 3d objects to screen space
					const D3DXMATRIX& ProjectionMatrix );	// projection matrix  ( for bringing 3d objects to screen space


	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info


////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	
	bool	Open( void )			{	return m_Open;		};

	bool	MouseOver( void );

	CItem*	Item( void )			{	return m_pItem;		};

	CCharacter*	Owner( void )		{	return m_pOwner;	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	SetText( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
					 const std::string& Message );	// text to set

	void	SetItem( CCharacter* pOwner,
					 CItem* pItem )	
			{	
				m_pItem = pItem;	
				m_pOwner = pOwner;
			};

	void	FlushAll( void );

	void	SetPlayerCharacter( CCharacter* pCharacter )		{	m_pPlayerCharacter = pCharacter;		};

	void	SetOpen( CLevel& pLevel,	// level to drop items to
					 bool Open );		// open state

	void	Open( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
				  const D3DXVECTOR3& Position,	// position of enchanting item
				  const std::string& Message,	// message to be displayed
				  const std::string& YesMessage,// yes button text
				  int32 YesCode,				// code for yes return
				  const std::string& NoMessage,	// no button text
				  int32 NoCode,					// code for no return
				  float32 Scale = 1 );			// text scale

	int32	UpdateButtonPress( bool ButtonPressed );	// is the left mouse button pressed?

	int32	UpdateButtonOver( void );

	int32	UpdateButtonOver( std::string& ToolmessageText );	// toolmessage text to fill out

private:

	void	RenderInventoryIcons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

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


	CCharacter*		m_pPlayerCharacter;

	D3DXVECTOR3		m_WorldPosition;

	// Skill menu
	CMaterial*		m_pPopupMenu;

	CMaterial*		m_pUIButtons;

	CText*			m_pMessageText;

	CButton*		m_pYesButton;
	CButton*		m_pNoButton;

	bool			m_Open;

	FSOUND_SAMPLE*	m_pClickSound;

	std::string		m_Message;

	CItem*			m_pItem;
	CCharacter*		m_pOwner;
};

#endif