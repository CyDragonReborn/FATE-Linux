/*
	gameui.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _GAMEUI_H
#define _GAMEUI_H

#include "d3dx8_shim.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../GAME/gamestatemanager.h"
#include "inventorymenu.h"
#include "petinventorymenu.h"
#include "petsellinventorymenu.h"
#include "statsmenu.h"
#include "spellmenu.h"
#include "petstatsmenu.h"
#include "buysellmenu.h"
#include "skillmenu.h"
#include "questmenu.h"
#include "journalmenu.h"
#include "fatemenu.h"
#include "gameoptionsmenu.h"
#include "confirmmenu.h"
#include "enchantmenu.h"
#include "helpmenu.h"
#include "messagemenu.h"
#include "tipmenu.h"
#include "../GAMEOBJECTS/camera.h"
#include "../UTILITIES/frustum.h"
#include "bottommenu.h"


const enum EMenuButtons
{
	KBottomButtonStats,
	KBottomButtonInventory,
	KBottomButtonMap,
	KBottomButtonJournal,
	KBottomButtonQuests,
	KBottomButtonOptions,
	KBottomButtonLevelUp,
	KBottomButtonSkills,
	KBottomButtonMagic,
	KButtonSaveAndExit,
	KButtonSaveAndContinue,
	KButtonQuitNoSave,
	KButtonFate,
	KButtonResume,
	KButtonPetUse,
	KButtonPetTown,
	KButtonPetInventory,
	KButtonPetStats,
	KButtonSwapSpell,
	KButtonAcceptQuest,
	KButtonDeclineQuest,
	KButtonChangeLevels,
	KButtonCancelQuestYes,
	KButtonCancelQuestNo,
	KButtonToggleRun,
	KButtonToggleShowItems,
	KButtonYes,
	KButtonNo,
	KButtonFate1,
	KButtonFate2,
	KButtonFate3,
	KButtonCloseHelp,
	KButtonOpenHelp,
	KButtonEventYes,
	KButtonEventNo,
	KButtonEventBreakGems,
	KButtonEventBreakWeapon,
	KButtonEventBreakCancel,
	KButtonEventEnchantItem,
	KButtonEventHeirloom,
	KButtonTipCheck,
	KButtonCancelFishing,
	KButtonSetHook,
	KStatButtonAddStrength,
	KStatButtonAddDexterity,
	KStatButtonAddVitality,
	KStatButtonAddMagic,
	KSkillButtonAdd,
	KButtons
};

const float32 KMiniMapDolly[KMiniMapHeights] = 
{
	200,
	300,
	400,
	600
};

const static uint32		KMaxHotkeys( 12 );

const static uint32		KMiniMapWidth( 256 );
const static uint32		KMiniMapHeight( 256 );

const static uint32		KMiniMapTopLeftX( 16 );
const static uint32		KMiniMapTopLeftY( 384 );

class CKeyHandler;
class CSprite;
class CMaterial;
class CGameStateManager;
class CCharacter;
class CButton;
class CText;
class CItem;
class CBottomMenu;
class CGameOptionsMenu;
class CConfirmMenu;
class CEnchantMenu;
class CHelpMenu;
class CFateMenu;
class CGameClient;
class CEventText;
class CSpellDescription;

// manage the ingame ui and events

class CGameUI
{
public:
				CGameUI( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						 CRefManager& pRefManager,		// the reference manager, for tracking media we create
						 CSettings& pSettings,			//we need this for tracking user preferences
						 CParticleManager& pParticleManager,	// particle manager
						 CGlowManager& pGlowManager,			// glow manager
						 CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CGameUI( void );

	void	Update( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					CGameClient& pClient,					// active game client
					CLevel& pLevel,							// level to create pathfinder for
					HWND hWnd,								// hwnd of the active window
					EViewType ViewType,						// type of view (split left, right or full?
					float32 TimeElapsed );					// time elapsed, in seconds


	void	Reset( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	ResetMouse( void );

	void	Render3D( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					  CGameClient& pClient,					// game client
					  const D3DXMATRIX& ViewportMatri,		// viewport matrix ( for bringing 3d objects to screen space
					  const D3DXMATRIX& ProjectionMatrix );	// projection matrix  ( for bringing 3d objects to screen space

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					CGameClient& pClient,					// game client
				    const D3DXMATRIX& ViewportMatri,		// viewport matrix ( for bringing 3d objects to screen space
					const D3DXMATRIX& ProjectionMatrix );	// projection matrix  ( for bringing 3d objects to screen space

	void	RenderTooltips( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device to use
							CGameClient& pClient );				// game client

	void	RenderMiniMap( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
						   CGameClient& pClient );				// game client


	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info


	void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	int32	ConfirmMenuResult( void )							{	return m_ConfirmMenuResult;				};

	int32	EnchantMenuResult( void )							{	return m_EnchantMenuResult;				};

	CItem*	EnchantMenuItem( void )								{	return m_pEnchantMenu->Item();			};

	CCharacter*	EnchantMenuOwner( void )						{	return m_pEnchantMenu->Owner();			};

	bool	MapVisible( void )									{	return m_MapVisible;					};

	EMiniMapDolly	MapDolly( void )							{	return m_MiniMapDolly;					};

	bool	InventoryOpen( void )								{	return m_pInventoryMenu->Open();		};

	bool	PetInventoryOpen( void )							{	return m_pPetInventoryMenu->Open();		};

	bool	PetSellInventoryOpen( void )						{	return m_pPetSellInventoryMenu->Open();	};

	bool	StatsOpen( void )									{	return m_pStatsMenu->Open();			};

	bool	SpellsOpen( void )									{	return m_pSpellMenu->Open();			};

	bool	PetStatsOpen( void )								{	return m_pPetStatsMenu->Open();			};

	bool	FateOpen( void )									{	return m_pFateMenu->Open();			};

	bool	SkillsOpen( void )									{	return m_pSkillMenu->Open();			};

	bool	QuestsOpen( void )									{	return m_pQuestMenu->Open();			};

	bool	JournalOpen( void )									{	return m_pJournalMenu->Open();			};

	bool	BuySellOpen( void )									{	return m_pBuySellMenu->Open();			};

	bool	EnchantOpen( void )									{	return m_pEnchantMenu->Open();			};

	CItem*	DraggedItem( void )									{	return m_pDraggingItem;					};

	bool	DraggingItem( void )								{	return m_pDraggingItem != NULL;			};

	bool	MouseOverMenus( void );

	bool	ConsoleOpen( void )									{	return m_ConsoleOpen;					};

	bool	Targeting( void )									{	return m_pItemInUse != NULL || m_pSpellInUse != NULL;			};

	bool	LeftCovered( void )									{	return m_LeftCovered;					};

	bool	RightCovered( void )								{	return m_RightCovered;					};

	bool	BothCovered( void )									{	return m_LeftCovered && m_RightCovered;	};

	bool	Paused( void )										{	return m_pGameOptionsMenu->Open() ||
																		   m_pConfirmMenu->Open() ||
																		   m_pMessageMenu->Open() ||
																		   m_pHelpMenu->Open() ||
																		   m_pFateMenu->Open() ||
																		   m_pTipMenu->Open();			};

	// allows menu usage, but no interaction ingame
	bool	ActionPaused( void )								{	return m_pEnchantMenu->Open();		};

	bool	AnyMenusOpen( void );	

	bool	AlwaysRunning( void )								{	return m_AlwaysRunning;				};

	bool	AlwaysShowingItems( void )							{	return m_AlwaysShowingItems;		};

	int32			HotkeyKeyAssignment( const std::string& SpellName );	// name of spell to clear hotkey assignment for

	const std::string&	HotkeyAssignment( uint32 Hotkey );	// hotkey to get assignment for

	bool	HotkeysChanged( void )								{	return m_HotkeysChanged;			};

	void	SetHotkeyChangeComplete( void )						{	m_HotkeysChanged = kFalse;			};

	void	SetHotkeysChanged( void )							{	m_HotkeysChanged = kTrue;			};

	void	SetHotkeyAssignment( uint32 Hotkey,						// hotkey to use
								 const std::string& SpellName );	// spell to set on it

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	SetEnchantMenuItem( CCharacter* pOwner,
								CItem* pItem )						{	m_pEnchantMenu->SetItem( pOwner, pItem );		};

	void	ClearConfirmMenuResult( void )							{	m_ConfirmMenuResult = -1;				};

	void	ClearEnchantMenuResult( void )							{	m_EnchantMenuResult = -1;				};

	void	Pause( CLevel& pLevel );					// level we are in

	void	OpenQuestMenu( void )		{	if( !m_pQuestMenu->Open() )ToggleQuestMenu();		};

	void	OpenConfirmation( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
							  const std::string& Message,	// message to be displayed
							  const std::string& YesMessage,// yes button text
							  int32 YesCode,				// code for yes return
							  const std::string& NoMessage,	// no button text
							  int32 NoCode,					// code for no return
							  float32 Scale = 1 );			// text scale

	void	OpenEnchant( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
						  const D3DXVECTOR3& Position,	// position of enchanting item
						  const std::string& Message,	// message to be displayed
						  const std::string& YesMessage,// yes button text
						  int32 YesCode,				// code for yes return
						  const std::string& NoMessage,	// no button text
						  int32 NoCode,					// code for no return
						  float32 Scale = 1 );			// text scale

	void	OpenTip( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
					 const std::string& Message );	// message to be displayed

	void	OpenMessage( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
						 const std::string& Message,	// message to be displayed
						 const std::string& ButtonMessage = LoadLocalizedString(915),
						 int32 ButtonValue = -1 );	// button value

	void	OpenFate( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
					  CLevel& pLevel );				// level to execute into

	void	CloseConfirmation( void )							{	m_pConfirmMenu->SetOpen( kFalse );
																	m_ConfirmMenuResult = -1;		};

	void	CloseEnchant( CLevel& pLevel )						{	m_pEnchantMenu->SetOpen( pLevel, kFalse );
																	m_EnchantMenuResult = -1;		};

	void	FlushAll( void );

	void	SetDisplayAllItems( bool State )					{	m_DisplayAllItems = State;				};

	void	SetPlayerCharacter( CCharacter* pCharacter );		// character to set

	void	SetTargetCharacter( CCharacter* pCharacter )		{	m_pTargetCharacter = pCharacter;		};

	void	SetTargetItem( CItem* pItem )						{	m_pTargetItem = pItem;					};

	void	SetMapVisible( bool State )							{	m_MapVisible = State;					};

	void	SetMapDolly( EMiniMapDolly Dolly )					{	m_MiniMapDolly = Dolly;					};

	void	SetHelpButtonVisible( bool State )					{	m_pBottomMenu->SetHelpButtonVisible( State );			};

	void	CloseAllMenus( CLevel& pLevel );	// level to drop items to

	void	ToggleInventoryMenu( CLevel& pLevel );			// level we are in

	void	SetAlwaysRunning( bool State )						{	m_AlwaysRunning = State;				};

	void	SetAlwaysShowingItems( bool State )					{	m_AlwaysShowingItems = State;			};

private:

	void	ToggleMiniMap( void );

	void			CloseAll( CLevel& pLevel );	// level to drop items to

	void			TogglePetInventoryMenu(  CLevel& pLevel );			// level we are in
	void			ToggleStatsMenu( CLevel& pLevel );					// level we are in
	void			TogglePetStatsMenu( void );
	void			ToggleSkillMenu( void );
	void			ToggleQuestMenu( void );
	void			ToggleJournalMenu( void );
	void			ToggleSpellMenu( void );

	void			CastActiveSpell( CLevel& pLevel,				// level to drop to
  									 CGameClient& pClient );		// game client

	void			CastSpell( CLevel& pLevel,						// level to drop to
							   CSpellDescription* pSpell,			// spell to cast
   							   CGameClient& pClient );				// game client


	
	void			ExecuteConsole( CGameClient& pClient,			// active game client
									CLevel& pLevel );				// level to execute into

	void			UpdateItemDescription( void );

	void			DropItem( CLevel& pLevel,						// level to drop to
							  CCharacter* pInventoryCharacter );	// character whose inventory this is from

	void			UseItem( CLevel& pLevel,						// level to post effects to
							 uint32 ButtonPressed );				// index of button pressed to pick up 

	void			PerformItemUse( CLevel& pLevel,					// level to post effects to
								    CItem* pItem,					// item to use
								    CCharacter* pOwningCharacter,	// owner
								    CCharacter* pUsingCharacter,	// user
								    CCharacter* pTargetCharacter ); // target

	void			UseItemOnPet( CLevel& pLevel );	// level to post effects to

	void			UseOnSlot( CLevel& pLevel,					// level to drop to
							   CCharacter* pInventoryCharacter,	// character whose inventory this is from
							   uint32 ButtonPressed );			// index of button pressed to pick up 

	void			TakeItemFromSlot( CLevel& pLevel,					// level to drop to
									  CCharacter* pInventoryCharacter,	// character whose inventory this is from
									  uint32 ButtonPressed );			// index of button pressed to pick up 

	void			PlaceItem( CLevel& pLevel,						// level to drop to
							   CCharacter* pInventoryCharacter,		// character whose inventory this is from
							   int32 SlotIndex = -1 );				// slot to place item into

	void			ReturnItem( CLevel& pLevel,					// level to drop to
								CCharacter* pInventoryCharacter );	// character whose inventory this is from

	void			UpdateTextEvents( float32 TimeElapsed );					// time elapsed in seconds

	void			RenderTextEvents( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
									  const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
									  const D3DXMATRIX& ProjectionMatrix );	// projection matrix  ( for bringing 3d objects to screen space

	void			FindTextEvents( LPDIRECT3DDEVICE8 pD3DDevice,					// d3d device
									CGameClient& pClient );							// game client

	void							CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice,				// d3d device
												 CParticleManager& pParticleManager,		// particle manager
												 CGlowManager& pGlowManager );				// glow manager

	void			ProcessInput( HWND hWnd );						// hwnd of the active window

	void			BringToFront( CSprite* pSprite );			// pointer to sprite in the internal list

	CSprite*		AddSprite( CSprite* pSprite );				// pointer to sprite to add to internal list

	CSprite*		AddSpriteFront( CSprite* pSprite );			// pointer to sprite to add to internal list

	void			RemoveSprite( CSprite* pSprite );			// pointer to sprite to remove from internal list

	void			DestroySprites( void );

	void			RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CButton*		AddButton( CButton* pButton );			// pointer to button to add to internal list

	void			RemoveButton( CButton* pButton );		// pointer to button to remove from internal list

	void			UpdateButtons( float32 TimeElapsed );		// time elapsed in seconds

	int32			UpdateButtonPress( bool ButtonPressed );	// is the left mouse button pressed?

	int32			UpdateButtonOver( void );
	int32			UpdateButtonOver( std::string& TooltipText );	// tooltip text to fill out

	void			DestroyButtons();

	void			RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	CText*			AddText( CText* pText );			// pointer to text to add to internal list

	void			RemoveText( CText* pText );			// pointer to text to remove from internal list

	void			DestroyText();

	void			RenderText( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void			ExecuteConsole( void );

	CItem*			GetItemFromSlot( uint32 ButtonPressed );	// index of button pressed to pick up 

	void			ClearHotkeyAssignments( void );

	void			ClearHotkeyAssignment( const std::string& SpellName );	// name of spell to clear hotkey assignment for

	void			VerifyHotkeyAssignments( void );

	CKeyHandler				m_KeyHandler;
	CMouseHandler			m_MouseHandler;

	CSettings&				m_pSettings;		
	CRefManager&			m_pRefManager;
	CGameStateManager&		m_pGameStateManager;

	CLList<CButton>*		m_pButtonList;
	CLList<CSprite>*		m_pSpriteList;
	CLList<CText>*			m_pTextList;

	LPDIRECT3DSURFACE8	m_pMouseSurface;
	LPDIRECT3DSURFACE8	m_pMouseSurfaceSelect;
	LPDIRECT3DSURFACE8	m_pBlankMouseSurface;

	CSprite*	m_pMousePointer;
	CMaterial*	m_pPointerMaterial;
	CMaterial*	m_pPointerSelectMaterial;

	CSprite*	m_pExclamation;
	CMaterial*	m_pExclamationMaterial;

	CSprite*	m_pExclamation2;
	CMaterial*	m_pExclamationMaterial2;

	// font texture and metric
	CMaterial*		m_pFontMaterial;
	CMaterial*		m_pFontMaterial2;
	CMaterial*		m_pFontHighlightMaterial;
	CFontMetric*	m_pFontMetric;

	CCharacter*		m_pTargetCharacter;
	CCharacter*		m_pPlayerCharacter;
	CCharacter*		m_pItemOwner;
	CItem*			m_pTargetItem;

	CItem*			m_pItemInUse;
	CCharacter*		m_pOwnerOfItemInUse;
	CCharacter*		m_pUserOfItemInUse;

	CSpellDescription*	m_pSpellInUse;
	CCharacter*			m_pOwnerOfSpellInUse;

	CItem*				m_pMouseOverItem;
	CSpellDescription*	m_pMouseOverSpell;

	CItem*			m_pDraggingItem;
	uint32			m_DragItemSlot;

	CText*			m_pConsoleText;

	bool			m_shouldOpenConsole;
	bool			m_consoleConfirmation;

	CText*			m_pTargetName;
	CText*			m_pTargetLevel;
	CText*			m_pTargetResistances;
	CText*			m_pToolTipText;

	CText*			m_pInventoryItemDescription;

	CMaterial*		m_pHealthBarMaterial;
	CMaterial*		m_pEliteMaterial;

	CMaterial*		m_pMiniMapMaterial;

	CMaterial*		m_pDescriptionShadowMaterial;
	CMaterial*		m_pDescriptionHighlightMaterial;

	CSprite*		m_pMiniMapSprite;

	CSprite*		m_pTargetHealthBarBack;
	CSprite*		m_pTargetHealthBarFront;
	CSprite*		m_pTargetEliteIcon;

	CSprite*		m_pDescriptionShadow;

	CBottomMenu*		m_pBottomMenu;
	CGameOptionsMenu*	m_pGameOptionsMenu;
	CConfirmMenu*		m_pConfirmMenu;
	CEnchantMenu*		m_pEnchantMenu;
	CMessageMenu*		m_pMessageMenu;
	CTipMenu*			m_pTipMenu;
	CInventoryMenu*		m_pInventoryMenu;
	CPetInventoryMenu*	m_pPetInventoryMenu;
	CPetSellInventoryMenu*	m_pPetSellInventoryMenu;
	CStatsMenu*			m_pStatsMenu;
	CSpellMenu*			m_pSpellMenu;
	CPetStatsMenu*		m_pPetStatsMenu;
	CBuySellMenu*		m_pBuySellMenu;
	CSkillMenu*			m_pSkillMenu;
	CQuestMenu*			m_pQuestMenu;
	CJournalMenu*		m_pJournalMenu;
	CHelpMenu*			m_pHelpMenu;
	CFateMenu*			m_pFateMenu;

	EViewType		m_ViewType;
	
	CText*			m_pDungeonLevel;
	CText*			m_pSummonCount;

	bool			m_MapVisible;
	CFrustum		m_MiniMapFrustum;

	bool			m_DisplayAllItems;

	CSprite*		m_pHighlight[KHighlightColors];

	CLList<CEventText>*		m_pEventTextList;	// list of pointers to all floating text events

	FSOUND_SAMPLE*	m_pErrorSound;
	FSOUND_SAMPLE*	m_pLowManaSound;
	FSOUND_SAMPLE*	m_pPurchaseSound;
	FSOUND_SAMPLE*	m_pSelectSpellSound;

	bool			m_ConsoleOpen;
	std::string		m_ConsoleText;

	bool			m_LeftCovered;
	bool			m_RightCovered;

	int32			m_ConfirmMenuResult;
	int32			m_EnchantMenuResult;

	EMiniMapDolly	m_MiniMapDolly;


	bool			m_AlwaysRunning;
	bool			m_AlwaysShowingItems;

	bool			m_HotkeysChanged;

	std::string		m_HotkeySpellAssignment[12];

	CButton*		m_pPlayerEffects[10];
	CButton*		m_pTargetEffects[10];

	bool			m_CursorIsTarget;
	bool			m_CursorIsVisible;
};

#endif