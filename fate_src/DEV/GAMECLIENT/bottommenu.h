/*
	BottomMenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _BOTTOMMENU_H
#define _BOTTOMMENU_H

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
class CGameUI;

const int32 KSwapSpellSelectButton( 200 );

// manage the ingame ui and events

class CBottomMenu
{
public:
				CBottomMenu( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							  CRefManager& pRefManager,		// the reference manager, for tracking media we create
							  CSettings& pSettings,			//we need this for tracking user preferences
							  CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CBottomMenu( void );

	void	Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds


	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					CGameUI& pGameUI,						// game ui
				    const D3DXMATRIX& ViewportMatri,		// viewport matrix ( for bringing 3d objects to screen space
					const D3DXMATRIX& ProjectionMatrix );	// projection matrix  ( for bringing 3d objects to screen space

	void	RenderSpellIcons( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
				   			  CGameUI& pGameUI );			// game ui

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info


////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	
	bool	MouseOver( void );

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	FlushAll( void );

	void	SetSpellsMenuOpen( bool State )						{	m_SpellsMenuOpen = State;				};

	void	SetPetInventoryMenuOpen( bool State )				{	m_PetInventoryMenuOpen = State;				};

	void	SetStatsMenuOpen( bool State )						{	m_StatsMenuOpen = State;				};

	void	SetSkillMenuOpen( bool State )						{	m_SkillMenuOpen = State;				};

	void	SetBuyMenuOpen( bool State )						{	m_BuyMenuOpen = State;					};

	void	SetInventoryMenuOpen( bool State )					{	m_InventoryMenuOpen = State;			};

	int32	UpdateButtonPress( bool ButtonPressed );	// is the left mouse button pressed?

	int32	UpdateButtonOver( void );

	int32	UpdateButtonOver( std::string& TooltipText );	// tooltip text to fill out


	void	SetHP( int32 MaxHP,		// max hp value
				   float32 HP );		// actual hp value

	void	SetPetHP( int32 MaxPetHP,		// max hp value
					  float32 PetHP );		// actual hp value

	void	SetMana( int32 MaxMana,		// max mana value
				     float32 Mana );		// actual mana value

	void	SetStamina( int32 MaxStamina,		// max stamina value
						float32 Stamina );		// actual stamina value

	void	SetExperience( int32 MaxHP,					// max exp value
						   int32 PreviousExperience,	// previous experience value
						   int32 HP );					// actual exp value

	void	SetPlayerCharacter( CCharacter* pCharacter )		{	m_pPlayerCharacter = pCharacter;		};

	void	SetPetCharacter( CCharacter* pCharacter )			{	m_pPetCharacter = pCharacter;			};

	void	SetHelpButtonVisible( bool State )					{	m_HelpButtonVisible = State;				};

	bool	SpellsRolledOut( void )								{	return m_SpellsRolledOut;					};

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

	void			ShowLevelUpButtons( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device
	
	void			RemoveLevelUpButtons( void );

	void			ShowHelpButton( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device
	
	void			RemoveHelpButton( void );

	void			ShowPetPanel( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
								  bool LeftCovered );					// left side of screen covered?
	void			RemovePetPanel( void );

	void			ShowSpellRollout( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
				   					  CGameUI& pGameUI );			// game ui

	void			HideSpellRollout( void );

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

	CMaterial*		m_pUIButtons;
	CMaterial*		m_pButtons;

	CSprite*		m_pPlayerHealthBar;
	CSprite*		m_pPlayerManaBar;
	CSprite*		m_pPlayerStaminaBar;
	CSprite*		m_pPlayerExperienceBar;
	CSprite*		m_pPetHealthBar;

	CSprite*		m_pPetPanel;

	CCharacter*		m_pPlayerCharacter;
	CCharacter*		m_pPetCharacter;

	// bottom menu
	CMaterial*		m_pBottomMenuLeft;
	CMaterial*		m_pBottomMenuCenter;
	CMaterial*		m_pBottomMenuRight;
	CMaterial*		m_pPetPanelMaterial;

	CMaterial*		m_pSpellButtonMaterial;

	CMaterial*		m_pGoldBag;
	CText*			m_pGold;

	float32			m_HPPercentage;
	int32			m_HP;
	int32			m_MaxHP;

	float32			m_PetHPPercentage;
	int32			m_PetHP;
	int32			m_PetMaxHP;

	float32			m_ExperiencePercentage;
	int32			m_Experience;
	int32			m_NextExperience;

	float32			m_ManaPercentage;
	int32			m_Mana;
	int32			m_MaxMana;

	float32			m_StaminaPercentage;
	int32			m_Stamina;
	int32			m_MaxStamina;

	CSprite*		m_pIconSprite;

	bool			m_LevelUpButtonsVisible;
	bool			m_HelpButtonVisible;
	bool			m_HelpButtonAdded;

	CButton*		m_pRunButton;
	CButton*		m_pShowItemsButton;

	CButton*		m_pLevelUpButton;
	CText*			m_pLevelUpText;

	CButton*		m_pHelpButton;
	CText*			m_pHelpButtonText;

	CText*			m_pPetTimeText;
	std::string		m_PetTimeString;

	CText*			m_pPetFleeing;

	bool			m_SpellsMenuOpen;
	bool			m_StatsMenuOpen;
	bool			m_SkillMenuOpen;
	bool			m_BuyMenuOpen;
	bool			m_InventoryMenuOpen;
	bool			m_PetInventoryMenuOpen;

	bool			m_PetPanelVisible;

	FSOUND_SAMPLE*	m_pLevelUpSound;

	FSOUND_SAMPLE*	m_pHeartbeatSound;

	FSOUND_SAMPLE*	m_pClickSound;

	FSOUND_SAMPLE*	m_pSelectSpellSound;

	CButton*		m_pHealthBarTooltip;
	CButton*		m_pPetHealthBarTooltip;
	CButton*		m_pManaBarTooltip;
	CButton*		m_pStaminaBarTooltip;
	CButton*		m_pExperienceBarTooltip;

	CButton*		m_pPetUseButton;
	CButton*		m_pPetTownButton;
	CButton*		m_pPetInventoryButton;
	CButton*		m_pPetStatsButton;

	float32			m_HeartbeatTimer;

	std::vector< CButton* >	m_pSpellButtons;
	bool			m_SpellsRolledOut;
	CButton*		m_pBaseSpellButton;

	bool			m_RequestSpellRollout;

	CButton*		m_pSetHookButton;
	CButton*		m_pCancelFishButton;
	CText*			m_pFishingText;

	bool			m_FishingButtonsVisible;

	bool			m_PetPanelOnRight;
};

#endif