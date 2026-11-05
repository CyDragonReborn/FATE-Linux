/*
	petinventorymenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _PETINVENTORYMENU_H
#define _PETINVENTORYMENU_H

#include "d3dx8_shim.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../GAME/gamestatemanager.h"
#include "../CHARACTER/inventory.h"

enum EPetInventoryMenuButtons
{
	KPetInventoryButtonStats,
	KPetInventoryButtonInventory,
	KPetInventoryButtonMap,
	KPetInventoryButtonOptions,
	KPetInventoryButtons
};

class CKeyHandler;
class CSprite;
class CMaterial;
class CGameStateManager;
class CCharacter;
class CButton;
class CText;
class CItem;

const int32 KPetInventoryButton( 2000 );

const std::string KPetEquipmentIconPath[KEquipmentSlots] = 
{
	"",
	"",
	"",
	"",
	"",
	"",
	"MENUS/necklaceicon.png",
	"",
	"MENUS/ringicon.png",
	"MENUS/ringicon.png",
	""
};

const std::string KPetEquipmentIconPathAlpha[KEquipmentSlots] = 
{
	"",
	"",
	"",
	"",
	"",
	"",
	"MENUS/necklaceicona.png",
	"",
	"MENUS/ringicona.png",
	"MENUS/ringicona.png",
	""
};
const float32 KPetEquipmentIconWidth[KEquipmentSlots] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	48,
	0,
	48,
	48,
	0
};

const float32 KPetEquipmentIconHeight[KEquipmentSlots] =
{
	0,
	0,
	0,
	0,
	0,
	0,
	48,
	0,
	48,
	48,
	0
};


const float32 KPetEquipmentSlotX[KEquipmentSlots] =
{
	-724,
	-724,
	-899,
	-550,
	-899,
	-550,
	236,
	-899,
	149,
	323,
	-725
};


const float32 KPetEquipmentSlotY[KEquipmentSlots] =
{
	-75,
	-177,
	-75,
	-75,
	-75,
	-273,
	128,
	-273,
	128,
	128,
	-321
};

const float32 KPetEquipmentSlotWidth[KEquipmentSlots] =
{
	96,
	96,
	96,
	96,
	96,
	96,
	48,
	96,
	48,
	48,
	96
};

const float32 KPetEquipmentSlotHeight[KEquipmentSlots] =
{
	96,
	144,
	192,
	192,
	192,
	96,
	48,
	96,
	48,
	48,
	48
};

// manage the ingame ui and events

class CPetInventoryMenu
{
public:
				CPetInventoryMenu( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							  CRefManager& pRefManager,		// the reference manager, for tracking media we create
							  CSettings& pSettings,			//we need this for tracking user preferences
							  CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CPetInventoryMenu( void );

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
	
	bool	Open( void )				{	return m_Open;						};

	bool	MouseOver( void );

	CCharacter*	PetCharacter( void )	{	return m_pPetCharacter;				};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	FlushAll( void );

	void	SetPetCharacter( CCharacter* pCharacter )		{	m_pPetCharacter = pCharacter;		};

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

	void			RenderInventoryIcons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void			RenderInventoryIconSockets( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
												CItem* pMouseOverItem,			// mouseover item
												bool ShowSockets );				// show sockets for items?

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

	CCharacter*		m_pPetCharacter;

	// Inventory menu
	CMaterial*		m_pInventoryMenuTop;
	CMaterial*		m_pInventoryMenuCenter;
	CMaterial*		m_pInventoryMenuBottom;

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

	CSprite*		m_pEquipmentIcon[KEquipmentSlots];
	CButton*		m_pEquipmentButton[KEquipmentSlots];
	CMaterial*		m_pEquipmentMaterial[KEquipmentSlots];

};

#endif