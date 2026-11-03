/*
	petstatsmenu.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _PETSTATSMENU_H
#define _PETSTATSMENU_H

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../GAME/gamestatemanager.h"
#include "gamerules.h"
class CKeyHandler;
class CSprite;
class CMaterial;
class CGameStateManager;
class CCharacter;
class CButton;
class CText;
class CItem;

// manage the ingame ui and events

class CPetStatsMenu
{
public:
				CPetStatsMenu( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							  CRefManager& pRefManager,		// the reference manager, for tracking media we create
							  CSettings& pSettings,			//we need this for tracking user preferences
							  CGameStateManager& GameStateManager );	// to determine whether needed media exists


				~CPetStatsMenu( void );

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

	void	ShowUpgradeButtons( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device
	
	void	RemoveUpgradeButtons( void );

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


	CCharacter*		m_pPetCharacter;

	// PetStats menu
	CMaterial*		m_pPetStatsMenuTop;
	CMaterial*		m_pPetStatsMenuCenter;
	CMaterial*		m_pPetStatsMenuBottom;

	CMaterial*		m_pUIButtons;
	CMaterial*		m_pFontHighlightMaterial;

	CText*			m_pName;
	CText*			m_pLevel;
	CText*			m_pExperience;
	CText*			m_pNextExperience;

	CText*			m_pStrength;
	CText*			m_pDexterity;
	CText*			m_pVitality;
	CText*			m_pMagic;

	CText*			m_pDamage;

	CText*			m_pToHit;

	CText*			m_pDefense;

	CText*			m_pLife;
	CText*			m_pStamina;

	CText*			m_pMana;

	bool			m_Open;

	bool			m_UpgradeButtonsVisible;

	CSprite*		m_pHighlight[KHighlightColors];

	CButton*		m_pAddStrengthButton;
	CButton*		m_pAddDexterityButton;
	CButton*		m_pAddVitalityButton;
	CButton*		m_pAddMagicButton;

	CButton*		m_pStrengthButton;
	CButton*		m_pDexterityButton;
	CButton*		m_pVitalityButton;
	CButton*		m_pMagicButton;

	CButton*		m_pDamageButton;
	CButton*		m_pAttackButton;
	CButton*		m_pDefenseButton;
	CButton*		m_pStaminaButton;
	CButton*		m_pLifeButton;
	CButton*		m_pManaButton;

	FSOUND_SAMPLE*	m_pGivePointSound;

	FSOUND_SAMPLE*	m_pClickSound;

	std::string		m_LastDisplayedName;
};

#endif