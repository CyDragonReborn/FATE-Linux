/*
	bottommenu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/10/2004

*/


#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"

#include "../UTILITIES/loadlocalizedstring.h"

#include "../GAME/gamestatemanager.h"

#ifdef ONLINEVERSION
#include "../CDA/cdamanager.h"
#endif

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../IO/keyhandler.h"

#include "../CHARACTER/character.h"
#include "../ITEM/item.h"
#include "../CHARACTER/inventory.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "../ITEM/spelltemplate.h"

#include "tips.h"
#include "gameui.h"
#include "inventorymenu.h"
#include "bottommenu.h"

CBottomMenu::CBottomMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  CRefManager& pRefManager,					// the reference manager, for tracking media we create
						  CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
						  CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																	m_pGameStateManager( pGameStateManager ),
																	m_pRefManager( pRefManager ),
																	m_pSettings( pSettings ),
																	m_pPlayerCharacter( NULL ),
																	m_pLevelUpButton( NULL ),
																	m_pLevelUpText( NULL ),
																	m_pHelpButton( NULL ),
																	m_pHelpButtonText( NULL ),
																	m_pPetPanel( NULL ),
																	m_pPetHealthBar( NULL ),
																	m_pPetFleeing( NULL ),
																	m_pIconSprite( NULL ),
																	m_pBaseSpellButton( NULL ),
																	m_PetPanelOnRight( kFalse ),
																	m_PetPanelVisible( kFalse ),
																	m_LevelUpButtonsVisible( kFalse ),
																	m_HelpButtonVisible( kFalse ),
																	m_HelpButtonAdded( kFalse ),
																	m_StatsMenuOpen( kFalse ),
																	m_SpellsMenuOpen( kFalse ),
																	m_SkillMenuOpen( kFalse ),
																	m_BuyMenuOpen( kFalse ),
																	m_InventoryMenuOpen( kFalse ),
																	m_PetInventoryMenuOpen( kFalse ),
																	m_SpellsRolledOut( kFalse ),
																	m_RequestSpellRollout( kFalse ),
																	m_HeartbeatTimer( 0 ),
																	m_FishingButtonsVisible( kFalse ),
																	m_PetTimeString( "" ),
																	m_pButtonList	( new CLList<CButton>() ),
																	m_pTextList		( new CLList<CText>() ),
																	m_pSpriteList	( new CLList<CSprite>() ),
																	m_pFontMetric( NULL ),
																	m_pPlayerHealthBar( NULL ),
																	m_pPlayerManaBar( NULL ),
																	m_pPlayerStaminaBar( NULL ),
																	m_pPlayerExperienceBar( NULL ),
																	m_pPetCharacter( NULL ),
																	m_pFontMaterial( NULL ),
																	m_pUIButtons( NULL ),
																	m_pButtons( NULL ),
																	m_pBottomMenuLeft( NULL ),
																	m_pBottomMenuCenter( NULL ),
																	m_pBottomMenuRight( NULL ),
																	m_pPetPanelMaterial( NULL ),
																	m_pSpellButtonMaterial( NULL ),
																	m_pGoldBag( NULL ),
																	m_HPPercentage( 0.0f ),
																	m_ExperiencePercentage( 0.0f ),
																	m_ManaPercentage( 0.0f ),
																	m_StaminaPercentage( 0.0f ),
																	m_PetHPPercentage( 0.0f ),
																	m_HP( 0 ),
																	m_MaxHP( 0 ),
																	m_PetHP( 0 ),
																	m_PetMaxHP( 0 ),
																	m_Experience( 0 ),
																	m_NextExperience( 0 ),
																	m_Mana( 0 ),
																	m_MaxMana( 0 ),
																	m_Stamina( 0 ),
																	m_MaxStamina( 0 ),
																	m_pGold( NULL ),
																	m_pPetTimeText( NULL ),
																	m_pFishingText( NULL ),
																	m_pRunButton( NULL ),
																	m_pShowItemsButton( NULL ),
																	m_pHealthBarTooltip( NULL ),
																	m_pPetHealthBarTooltip( NULL ),
																	m_pManaBarTooltip( NULL ),
																	m_pStaminaBarTooltip( NULL ),
																	m_pExperienceBarTooltip( NULL ),
																	m_pPetUseButton( NULL ),
																	m_pPetTownButton( NULL ),
																	m_pPetInventoryButton( NULL ),
																	m_pPetStatsButton( NULL ),
																	m_pSetHookButton( NULL ),
																	m_pCancelFishButton( NULL ),
																	m_pLevelUpSound( NULL ),
																	m_pHeartbeatSound( NULL ),
																	m_pClickSound( NULL ),
																	m_pSelectSpellSound( NULL )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));

	m_pIconSprite = new CSprite( pD3DDevice,
								 NULL,
								 0,
								 0,
								 48,
								 48,
								 0,
								 0,
								 1,
								 1,
								 KSpriteTopLeft );


	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CBottomMenu::CBottomMenu()

CBottomMenu::~CBottomMenu( void )
{
	HideSpellRollout();

	RemoveButton( m_pBaseSpellButton );
	DELETE_SAFELY( m_pBaseSpellButton );
	
	RemoveButton( m_pSetHookButton );
	DELETE_SAFELY( m_pSetHookButton );

	RemoveButton( m_pCancelFishButton );
	DELETE_SAFELY( m_pCancelFishButton );

	RemoveText( m_pFishingText );
	DELETE_SAFELY( m_pFishingText );

	DELETE_SAFELY( m_pPetTimeText );

	m_pPlayerCharacter = NULL;
	RemoveLevelUpButtons();
	RemoveHelpButton();
	RemovePetPanel();

	DELETE_SAFELY( m_pIconSprite );

	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	DELETE_SAFELY( m_pPetFleeing );

	m_pRefManager.RemoveSample( m_pHeartbeatSound );

	m_pRefManager.RemoveSample( m_pLevelUpSound );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveSample( m_pSelectSpellSound );

	m_pRefManager.RemoveMaterial( m_pUIButtons );
	m_pRefManager.RemoveMaterial( m_pButtons );

	m_pRefManager.RemoveMaterial( m_pSpellButtonMaterial );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pGoldBag );

	m_pRefManager.RemoveMaterial( m_pPetPanelMaterial );

	m_pRefManager.RemoveMaterial( m_pBottomMenuLeft );
	m_pRefManager.RemoveMaterial( m_pBottomMenuRight );
	m_pRefManager.RemoveMaterial( m_pBottomMenuCenter );

} // CBottomMenu::~CBottomMenu()

void CBottomMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CBottomMenu::FlushAll()

void CBottomMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	m_pSelectSpellSound = m_pRefManager.AddSample( "SOUNDS/UI/levelup.wav", kFalse );

	m_pLevelUpSound = m_pRefManager.AddSample( "SOUNDS/UI/levelup.wav", kFalse );

	m_pHeartbeatSound = m_pRefManager.AddSample( "SOUNDS/UI/heartbeat.wav", kFalse );

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pSpellButtonMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
															  "MENUS/spellbutton.png",
															  "MENUS/spellbuttona.png" );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												 "MENUS/buttons.png",
												 "MENUS/buttonsa.png" );

	m_pGoldBag = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												 "MENUS/goldbag.png",
												 "MENUS/goldbaga.png" );

	m_pPetPanelMaterial = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/petpanel.png" );

	m_pBottomMenuLeft = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														"MENUS/bottombarleft.png",
														"MENUS/bottombarlefta.png" );

	m_pBottomMenuRight = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														"MENUS/bottombarright.png",
														"MENUS/bottombarrighta.png" );

	m_pBottomMenuCenter = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														"MENUS/bottombarcenter.png",
														"MENUS/bottombarcentera.png" );

	m_pPlayerHealthBar = AddSprite( new CSprite( pD3DDevice,
												 m_pBottomMenuCenter,
												 223,
												 736,
												 128,
												 23,
												 m_pBottomMenuCenter->PixelToU() * 144,
												 m_pBottomMenuCenter->PixelToV() * 102,
												 m_pBottomMenuCenter->PixelToU() * 156,
												 m_pBottomMenuCenter->PixelToV() * 125,
												 KSpriteTopLeft ) );

	m_pHealthBarTooltip = AddButton( new CButton( pD3DDevice,
												  NULL,
												  NULL,
												  NULL,
												  223,
												  736,
												  128,
												  23,
												  0,
												  0,
												  1,
												  1,
												  0,
												  0,
												  1,
												  1,
												  KButtonTopLeft,
												  -999 ) );
	m_pHealthBarTooltip->SetToolTip( "0/0" );

	m_pPlayerManaBar = AddSprite( new CSprite( pD3DDevice,
												 m_pBottomMenuCenter,
												 677,
												 736,
												 128,
												 23,
												 m_pBottomMenuCenter->PixelToU() * 164,
												 m_pBottomMenuCenter->PixelToV() * 102,
												 m_pBottomMenuCenter->PixelToU() * 176,
												 m_pBottomMenuCenter->PixelToV() * 125,
												 KSpriteTopLeft ) );

	m_pManaBarTooltip = AddButton( new CButton( pD3DDevice,
												  NULL,
												  NULL,
												  NULL,
												  677,
												  736,
												  128,
												  23,
												  0,
												  0,
												  1,
												  1,
												  0,
												  0,
												  1,
												  1,
												  KButtonTopLeft,
												  -999 ) );
	m_pManaBarTooltip->SetToolTip( "0/0" );

	m_pPlayerStaminaBar = AddSprite( new CSprite( pD3DDevice,
												  m_pBottomMenuCenter,
												  451,
												  694,
												  124,
												  16,
												  m_pBottomMenuCenter->PixelToU() * 184,
												  m_pBottomMenuCenter->PixelToV() * 102,
												  m_pBottomMenuCenter->PixelToU() * 196,
												  m_pBottomMenuCenter->PixelToV() * 125,
												  KSpriteTopLeft ) );

	m_pStaminaBarTooltip = AddButton( new CButton( pD3DDevice,
												  NULL,
												  NULL,
												  NULL,
												  451,
												  694,
												  124,
												  16,
												  0,
												  0,
												  1,
												  1,
												  0,
												  0,
												  1,
												  1,
												  KButtonTopLeft,
												  -999 ) );
	m_pManaBarTooltip->SetToolTip( "0/0" );

	m_pPlayerExperienceBar = AddSprite( new CSprite( pD3DDevice,
													  m_pBottomMenuCenter,
													  451,
													  676,
													  124,
													  8,
													  m_pBottomMenuCenter->PixelToU() * 204,
													  m_pBottomMenuCenter->PixelToV() * 102,
													  m_pBottomMenuCenter->PixelToU() * 210,
													  m_pBottomMenuCenter->PixelToV() * 125,
													  KSpriteTopLeft ) );

	m_pExperienceBarTooltip = AddButton( new CButton( pD3DDevice,
												  NULL,
												  NULL,
												  NULL,
												  451,
												  674,
												  124,
												  8,
												  0,
												  0,
												  1,
												  1,
												  0,
												  0,
												  1,
												  1,
												  KButtonTopLeft,
												  -999 ) );
	AddSprite( new CSprite( pD3DDevice,
							m_pBottomMenuLeft,
							191,
							641,
							256,
							128,
							0.01f,
							0.01f,
							.99f,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pBottomMenuCenter,
							447,
							641,
							135,
							128,
							0.00f,
							0.01f,
							m_pBottomMenuCenter->PixelToU() * 135.99f,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pBottomMenuRight,
							582,
							641,
							256,
							128,
							0.0015f,
							0.01f,
							.99f,
							1,
							KSpriteTopLeft ) );

	CButton* pButton;
	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									234,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 103,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 153,
									KButtonTopLeft,
									KBottomButtonStats ) );
	pButton->SetToolTip( LoadLocalizedString(144) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									286,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 154,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 204,
									KButtonTopLeft,
									KBottomButtonInventory ) );
	pButton->SetToolTip( LoadLocalizedString(145) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									338,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 205,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 255,
									KButtonTopLeft,
									KBottomButtonMagic ) );
	pButton->SetToolTip( LoadLocalizedString(146) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									390,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 103,
									m_pUIButtons->PixelToV() * 154,
									m_pUIButtons->PixelToU() * 153,
									m_pUIButtons->PixelToV() * 203,
									KButtonTopLeft,
									KBottomButtonSkills ) );

	pButton->SetToolTip( LoadLocalizedString(147) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									586,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 52,
									m_pUIButtons->PixelToV() * 103,
									m_pUIButtons->PixelToU() * 102,
									m_pUIButtons->PixelToV() * 153,
									KButtonTopLeft,
									KBottomButtonJournal ) );
	
	pButton->SetToolTip( LoadLocalizedString(148) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									638,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 103,
									m_pUIButtons->PixelToV() * 104,
									m_pUIButtons->PixelToU() * 153,
									m_pUIButtons->PixelToV() * 153,
									KButtonTopLeft,
									KBottomButtonQuests ) );
	pButton->SetToolTip( LoadLocalizedString(149) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									690,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 52,
									m_pUIButtons->PixelToV() * 154,
									m_pUIButtons->PixelToU() * 102,
									m_pUIButtons->PixelToV() * 204,
									KButtonTopLeft,
									KBottomButtonMap ) );
	pButton->SetToolTip( LoadLocalizedString(150) );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									742,
									666,
									51,
									51,
									m_pUIButtons->PixelToU() * 253,
									m_pUIButtons->PixelToV() * 250,
									m_pUIButtons->PixelToU() * 254,
									m_pUIButtons->PixelToV() * 251,
									m_pUIButtons->PixelToU() * 52,
									m_pUIButtons->PixelToV() * 205,
									m_pUIButtons->PixelToU() * 102,
									m_pUIButtons->PixelToV() * 255,
									KButtonTopLeft,
									KBottomButtonOptions ) );
	pButton->SetToolTip( LoadLocalizedString(151) );


	// add invisible buttons for the quickitem slots
	for( uint32 i = 0; i < KQuickSlots - KEquipmentSlots; i++ )
	{
		CButton* pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													371 + i * 48.0f,
													720,
													48,
													48,
													0,
													0,
													1,
													1,
													0,
													0,
													1,
													1,
													KButtonTopLeft,
													KInventoryButton + KEquipmentSlots + i ) );
		pButton->SetToolTip( LoadLocalizedString(152) );
	}

	// gold bag sprite
	AddSprite( new CSprite( pD3DDevice,
							m_pGoldBag,
							895,
							660,
							64,
							64,
							0,
							0,
							1,
							1,
							KSpriteTopLeft ) );

	m_pGold = AddText( new CText( pD3DDevice,
								  m_pFontMaterial,
								  m_pFontMetric,
								  "0 GP",
								   930,
								   725,
								   .8f,
								   kFalse,
								   KJustifyCenter ) );

	m_pPetTimeText = new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"",
								100,
								50,
								.8f,
								kFalse,
								KJustifyCenter );

	m_pPetFleeing = new CText( pD3DDevice,
							   m_pFontMaterial,
							   m_pFontMetric,
							   LoadLocalizedString(153),
							   78,
							   142,
							   .8f,
							   kFalse,
							   KJustifyCenter );

	m_pFishingText = new CText( pD3DDevice,
							   m_pFontMaterial,
							   m_pFontMetric,
							   LoadLocalizedString(154),
							   512,
							   200,
							   1.0f,
							   kFalse,
							   KJustifyCenter );

	m_pSetHookButton = new CButton( pD3DDevice,
									m_pButtons,
									NULL,
									m_pClickSound,
									512,
									344,
									256,
									48,
									0 * m_pButtons->PixelToU(),
									49 * m_pButtons->PixelToV(),
									256 * m_pButtons->PixelToU(),
									97 * m_pButtons->PixelToV(),
									0 * m_pButtons->PixelToU(),
									0 * m_pButtons->PixelToV(),
									256 * m_pButtons->PixelToU(),
								    48 * m_pButtons->PixelToV(),
									KButtonCenter,
									KButtonSetHook );

	m_pSetHookButton->AddText( pD3DDevice,
					  LoadLocalizedString(155),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	m_pSetHookButton->AddOverText( pD3DDevice,
						LoadLocalizedString(155),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );

	m_pCancelFishButton = new CButton( pD3DDevice,
									m_pButtons,
									NULL,
									m_pClickSound,
									512,
									344,
									256,
									48,
									0 * m_pButtons->PixelToU(),
									49 * m_pButtons->PixelToV(),
									256 * m_pButtons->PixelToU(),
									97 * m_pButtons->PixelToV(),
									0 * m_pButtons->PixelToU(),
									0 * m_pButtons->PixelToV(),
									256 * m_pButtons->PixelToU(),
								    48 * m_pButtons->PixelToV(),
									KButtonCenter,
									KButtonCancelFishing );

	m_pCancelFishButton->AddText( pD3DDevice,
					  LoadLocalizedString(156),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	m_pCancelFishButton->AddOverText( pD3DDevice,
						LoadLocalizedString(156),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );


	m_pRunButton = AddButton( new CButton( pD3DDevice,
									m_pBottomMenuCenter,
									NULL,
									m_pClickSound,
									478,
									648,
									24,
									24,
									m_pBottomMenuCenter->PixelToU() * 143,
									m_pBottomMenuCenter->PixelToV() * 0,
									m_pBottomMenuCenter->PixelToU() * 166,
									m_pBottomMenuCenter->PixelToV() * 23,
									m_pBottomMenuCenter->PixelToU() * 143,
									m_pBottomMenuCenter->PixelToV() * 24,
									m_pBottomMenuCenter->PixelToU() * 166,
									m_pBottomMenuCenter->PixelToV() * 47,
									KButtonTopLeft,
									KButtonToggleRun ) );
	m_pRunButton->SetToolTip( LoadLocalizedString(157) );

	m_pShowItemsButton = AddButton( new CButton( pD3DDevice,
									m_pBottomMenuCenter,
									NULL,
									m_pClickSound,
									523,
									648,
									24,
									24,
									m_pBottomMenuCenter->PixelToU() * 168,
									m_pBottomMenuCenter->PixelToV() * 0,
									m_pBottomMenuCenter->PixelToU() * 191,
									m_pBottomMenuCenter->PixelToV() * 23,
									m_pBottomMenuCenter->PixelToU() * 168,
									m_pBottomMenuCenter->PixelToV() * 24,
									m_pBottomMenuCenter->PixelToU() * 191,
									m_pBottomMenuCenter->PixelToV() * 47,
									KButtonTopLeft,
									KButtonToggleShowItems ) );
	m_pShowItemsButton->SetToolTip( LoadLocalizedString(158) );

} // CBottomMenu::CreateMenus()

bool CBottomMenu::MouseOver( void )
{
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	if( MousePosition.x >= 193 &&
		MousePosition.y >= 700 &&
		MousePosition.x <= 836 &&
		MousePosition.y <= 768 )
	{
		return kTrue;
	}
	if( MousePosition.x >= 210 &&
		MousePosition.y >= 642 &&
		MousePosition.x <= 811 &&
		MousePosition.y <= 768 )
	{
		return kTrue;
	}

	if( m_PetPanelVisible && 
		!m_PetPanelOnRight &&
		MousePosition.x >=10 &&
		MousePosition.y >= 42 &&
		MousePosition.x <= 138 &&
		MousePosition.y <= 170 )
	{
		return kTrue;
	}
	else if( m_PetPanelVisible && 
		m_PetPanelOnRight &&
		MousePosition.x >= 10 + 512 &&
		MousePosition.y >= 42 &&
		MousePosition.x <= 138 + 512 &&
		MousePosition.y <= 170 )
	{
		return kTrue;
	}
	if( UpdateButtonOver() != -1 )
	{
		return kTrue;
	}
	return kFalse;

} // CBottomMenu::MouseOver()

void CBottomMenu::SetHP( int32 MaxHP,		// max hp value
						 float32 HP )			// actual hp value
{
	m_HP = (int32)HP;
	m_MaxHP = MaxHP;
	m_HPPercentage = (float32)HP / (float32)MaxHP;
	if( m_HPPercentage < 0 )
	{
		m_HPPercentage = 0;
	}
	if( m_HPPercentage > 1 )
	{
		m_HPPercentage = 1;
	}

} // CBottomMenu::SetHP()

void CBottomMenu::SetPetHP( int32 PetMaxHP,		// max PetHP value
							float32 PetHP )			// actual PetHP value
{
	m_PetHP = (int32)PetHP;
	m_PetMaxHP = PetMaxHP;
	m_PetHPPercentage = (float32)PetHP / (float32)PetMaxHP;
	if( m_PetHPPercentage < 0 )
	{
		m_PetHPPercentage = 0;
	}
	if( m_PetHPPercentage > 1 )
	{
		m_PetHPPercentage = 1;
	}

} // CBottomMenu::SetPetHP()

void CBottomMenu::SetMana( int32 MaxMana,		// max Mana value
						   float32 Mana )		// actual Mana value
{
	m_Mana = (int32)Mana;
	m_MaxMana = MaxMana;
	if( m_MaxMana == 0 )
	{
		m_ManaPercentage = 0;
	}
	else
	{
		m_ManaPercentage = (float32)Mana / (float32)MaxMana;
		if( m_ManaPercentage < 0 )
		{
			m_ManaPercentage = 0;
		}
		if( m_ManaPercentage > 1 )
		{
			m_ManaPercentage = 1;
		}
	}

} // CBottomMenu::SetMana()

void CBottomMenu::SetStamina( int32 MaxStamina,		// max Stamina value
							  float32 Stamina )			// actual Stamina value
{
	m_Stamina = (int32)Stamina;
	m_MaxStamina = MaxStamina;
	m_StaminaPercentage = (float32)Stamina / (float32)MaxStamina;
	if( m_StaminaPercentage < 0 )
	{
		m_StaminaPercentage = 0;
	}
	if( m_StaminaPercentage > 1 )
	{
		m_StaminaPercentage = 1;
	}

} // CBottomMenu::SetStamina()

void CBottomMenu::SetExperience( int32 NextExperience,		// next Experience value
								 int32 PreviousExperience,	// previous experience value
								 int32 Experience )			// actual Experience value
{
	m_Experience = Experience;
	m_NextExperience = NextExperience;

	Experience -= PreviousExperience;
	NextExperience -= PreviousExperience;
	m_ExperiencePercentage = (float32)Experience / (float32)NextExperience;
	if( m_ExperiencePercentage < 0 )
	{
		m_ExperiencePercentage = 0;
	}
	if( m_ExperiencePercentage > 1 )
	{
		m_ExperiencePercentage = 1;
	}

} // CBottomMenu::SetExperience()

void CBottomMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{

	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CBottomMenu::KeyEvent()

void CBottomMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{

	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CBottomMenu::KeyEvent()

void CBottomMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CBottomMenu::ProcessInput()

void CBottomMenu::Update( HWND hWnd,						// hwnd of the active window
						  float32 TimeElapsed )				// time elapsed, in seconds
{
	if( m_HPPercentage < .3f &&
		m_HPPercentage > 0 )
	{
		m_HeartbeatTimer += TimeElapsed;
		float32 HeartbeatTimeGate = m_HPPercentage - .1f;
		if( HeartbeatTimeGate < 0 )
		{
			HeartbeatTimeGate = 0;
		}
		HeartbeatTimeGate = .85f + ( HeartbeatTimeGate ) * 4;
		if( m_HeartbeatTimer >= HeartbeatTimeGate )
		{
			m_HeartbeatTimer = 0;
			FSOUND_PlaySound( FSOUND_FREE, m_pHeartbeatSound );
		}
	}

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );

	int32 ButtonPressed = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
											 m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );

	if( ButtonPressed == KButtonSwapSpell )
	{
		if( m_SpellsRolledOut )
		{
			HideSpellRollout();
		}
		else
		{
			m_RequestSpellRollout = kTrue;
		}
	}
	else if( ButtonPressed >= KSwapSpellSelectButton &&
			 ButtonPressed <= KSwapSpellSelectButton + KMagicSpheres * KMaximumSpellsPerSphere )
	{
		int32 Sphere( 0 );
		int32 Slot( ButtonPressed - KSwapSpellSelectButton );
		while( Slot >= KMaximumSpellsPerSphere )
		{
			Slot -= KMaximumSpellsPerSphere;
			Sphere++;
		}
		CSpellDescription* pDescription = m_pPlayerCharacter->Spell( (EMagicSphere)Sphere, Slot );

		if( pDescription != NULL )
		{
			FSOUND_PlaySound( FSOUND_FREE, m_pSelectSpellSound );
			m_pPlayerCharacter->SetActiveSpell( pDescription );
			HideSpellRollout();
		}
	}
	else if( ( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
			   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) ||
			   m_MouseHandler.ButtonPressed( KButtonRight ) ||
			   m_MouseHandler.ButtonDoubleClick( KButtonRight ) ) )
	{
		HideSpellRollout();
	}


	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CBottomMenu::Update()

// render any 3d elements
void CBottomMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						  CGameUI& pGameUI,						// game ui
						  const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						  const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( pGameUI.FateOpen() )
	{
		return;
	}
	if( pGameUI.AlwaysRunning() )
	{
		m_pRunButton->SetUVs( m_pBottomMenuCenter->PixelToU() * 143,
							  m_pBottomMenuCenter->PixelToV() * 48,
							  m_pBottomMenuCenter->PixelToU() * 166,
							  m_pBottomMenuCenter->PixelToV() * 71 );
	}
	else
	{
		m_pRunButton->SetUVs( m_pBottomMenuCenter->PixelToU() * 143,
							  m_pBottomMenuCenter->PixelToV() * 0,
							  m_pBottomMenuCenter->PixelToU() * 166,
							  m_pBottomMenuCenter->PixelToV() * 23 );
	}

	if( pGameUI.AlwaysShowingItems() )
	{
		m_pShowItemsButton->SetUVs(	m_pBottomMenuCenter->PixelToU() * 168,
									m_pBottomMenuCenter->PixelToV() * 48,
									m_pBottomMenuCenter->PixelToU() * 191,
									m_pBottomMenuCenter->PixelToV() * 71 );
	}
	else
	{
		m_pShowItemsButton->SetUVs(	m_pBottomMenuCenter->PixelToU() * 168,
									m_pBottomMenuCenter->PixelToV() * 0,
									m_pBottomMenuCenter->PixelToU() * 191,
									m_pBottomMenuCenter->PixelToV() * 23 );
	}

	RenderSprites( pD3DDevice );

	RenderSpellIcons( pD3DDevice, pGameUI );

	RenderButtons( pD3DDevice );

	RenderInventoryIcons( pD3DDevice );

	if( m_RequestSpellRollout || 
		( pGameUI.HotkeysChanged() &&
		  m_SpellsRolledOut ) )
	{
		if( pGameUI.HotkeysChanged() )
		{
			HideSpellRollout();
			if( m_pBaseSpellButton != NULL )
			{
				RemoveButton( m_pBaseSpellButton );
				DELETE_SAFELY( m_pBaseSpellButton );
				m_pBaseSpellButton = NULL;
			}
		}
		ShowSpellRollout( pD3DDevice, pGameUI );
		m_RequestSpellRollout = kFalse;
		pGameUI.SetHotkeyChangeComplete();
	}


	if( m_pPlayerCharacter != NULL )
	{
		//m_pGold->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Inventory()->Gold() ) + 
		//							  " GP" );
		m_pGold->SetText( pD3DDevice, LoadLocalizedString1Variable(1162, IntegerToString( m_pPlayerCharacter->Inventory()->Gold() ) ) );
	}

	RenderText( pD3DDevice );

	if( m_pPlayerCharacter != NULL &&
		m_pPlayerCharacter->HasPets() && 
		!m_StatsMenuOpen &&
		!m_BuyMenuOpen &&
		!m_SpellsMenuOpen &&
		( m_pPlayerCharacter->Pet( 0 )->AIState() == KAIInTown || 
		  m_pPlayerCharacter->Pet( 0 )->AIState() == KAIReturningToTown ) )
	{
		m_pPetTimeText->SetText( pD3DDevice, m_PetTimeString );
		m_pPetTimeText->Render( pD3DDevice );
	}


	float32 Percentage = m_HPPercentage;
	if( Percentage < 0 )
	{
		Percentage = 0;
	}
	m_pPlayerHealthBar->SetSize( 128 * Percentage, 23 );
	if( m_pPlayerCharacter->Poisoned() )
	{
		 m_pPlayerHealthBar->SetUVs( m_pBottomMenuCenter->PixelToU() * 220,
									 m_pBottomMenuCenter->PixelToV() * 102,
									 m_pBottomMenuCenter->PixelToU() * 232,
									 m_pBottomMenuCenter->PixelToV() * 125 );
	}
	else
	{
		 m_pPlayerHealthBar->SetUVs( m_pBottomMenuCenter->PixelToU() * 144,
									 m_pBottomMenuCenter->PixelToV() * 102,
									 m_pBottomMenuCenter->PixelToU() * 156,
									 m_pBottomMenuCenter->PixelToV() * 125 );
	}



	m_pPlayerHealthBar->Render( pD3DDevice );

	std::string ResistanceText( "" );
	if( m_pPlayerCharacter != NULL )
	{
		for( uint32 r = 0; r < KDamageTypes; r++ )
		{
			if( r != KDamageUndead &&
				m_pPlayerCharacter->DamageResistance( (EDamageType)r ) > 0 )
			{
				ResistanceText = ResistanceText + 
								 "\b" + IntegerToString( m_pPlayerCharacter->DamageResistance( (EDamageType)r ) ) + "%\b " +
								 KDamageResistanceDescription[r] + "\n";
			}
			else if( m_pPlayerCharacter->DamageResistance( (EDamageType)r ) < 0 )
			{
				ResistanceText = ResistanceText + "\b" +
								 "\b" + IntegerToString( abs( m_pPlayerCharacter->DamageResistance( (EDamageType)r ) ) ) + "%\b " +
								 KDamageWeaknessDescription[r] + "\b\n";
			}
			else if( r == KDamageUndead &&
					m_pPlayerCharacter->DamageResistance( (EDamageType)r ) == 0 )
			{
				ResistanceText = ResistanceText + "\b" +
								 KDamageWeaknessDescription[r] + "\b\n";
			}
		}
	}

	m_pHealthBarTooltip->SetToolTip( LoadLocalizedString2Variables(1142, IntegerToString( m_HP ), IntegerToString( m_MaxHP ) ) + "\n" + ResistanceText );

	Percentage = m_ManaPercentage;
	if( Percentage < 0 )
	{
		Percentage = 0;
	}
	m_pPlayerManaBar->SetSize( 128 * Percentage, 23 );
	m_pPlayerManaBar->Render( pD3DDevice );
	m_pManaBarTooltip->SetToolTip( LoadLocalizedString2Variables(1143, IntegerToString( m_Mana ), IntegerToString( m_MaxMana ) ) );

	Percentage = m_StaminaPercentage;
	if( Percentage < 0 )
	{
		Percentage = 0;
	}
	m_pPlayerStaminaBar->SetSize( 124 * Percentage, 16 );
	m_pPlayerStaminaBar->Render( pD3DDevice );
	m_pStaminaBarTooltip->SetToolTip( LoadLocalizedString2Variables(1144, IntegerToString( m_Stamina ), IntegerToString( m_MaxStamina ) ) );

	Percentage = m_ExperiencePercentage;
	if( Percentage < 0 )
	{
		Percentage = 0;
	}
	m_pPlayerExperienceBar->SetSize( 124 * Percentage, 9 );
	m_pPlayerExperienceBar->Render( pD3DDevice );
	m_pExperienceBarTooltip->SetToolTip( LoadLocalizedString2Variables(1145, IntegerToString( m_Experience ), IntegerToString( m_NextExperience ) ) );

	if( m_pPlayerCharacter != NULL &&
		m_pPlayerCharacter->HasPets() && 
		!pGameUI.BothCovered() &&
		!pGameUI.EnchantOpen() &&
		!pGameUI.FateOpen() &&
		m_pPlayerCharacter->Pet( 0 )->AIState() != KAIInTown && 
		m_pPlayerCharacter->Pet( 0 )->AIState() != KAIReturningToTown )
	{
		ShowPetPanel( pD3DDevice, pGameUI.LeftCovered() );
	}
	else
	{
		RemovePetPanel( );
		if( m_pPlayerCharacter != NULL &&
			m_pPlayerCharacter->HasPets() && 
			!m_StatsMenuOpen &&
			!m_BuyMenuOpen &&
			!m_SpellsMenuOpen &&
			( m_pPlayerCharacter->Pet( 0 )->AIState() == KAIInTown || 
			  m_pPlayerCharacter->Pet( 0 )->AIState() == KAIReturningToTown ) )
		{
			float32 Seconds = m_pPlayerCharacter->Pet( 0 )->TownTimer();
			uint32 Minutes = 0;
			uint32 Hours = 0;
			while( Seconds >= 60 )
			{
				Seconds -= 60;
				Minutes++;
			}
			while( Minutes >= 60 )
			{
				Minutes -= 60;
				Hours++;
			}
			std::string MinuteString;
			if( Minutes < 10 )
			{
				MinuteString = "0" + IntegerToString( (int32)Minutes );
			}
			else
			{
				MinuteString = IntegerToString( (int32)Minutes );
			}

			std::string SecondString;
			if( Seconds < 10 )
			{
				SecondString = "0" + IntegerToString( (int32)Seconds );
			}
			else
			{
				SecondString = IntegerToString( (int32)Seconds );
			}
			m_PetTimeString = "\b" + IntegerToString( Hours ) + ":" + MinuteString +
							   ":" + SecondString + LoadLocalizedString(159);

		}
	}

	if( m_PetPanelVisible )
	{
		Percentage = m_PetHPPercentage;
		if( Percentage < 0 )
		{
			Percentage = 0;
		}
		m_pPetHealthBar->SetSize( 120 * Percentage, 16 );
		if( m_pPetCharacter->Poisoned() )
		{
			 m_pPetHealthBar->SetUVs( m_pBottomMenuCenter->PixelToU() * 220,
									  m_pBottomMenuCenter->PixelToV() * 102,
									  m_pBottomMenuCenter->PixelToU() * 232,
									  m_pBottomMenuCenter->PixelToV() * 125 );
		}
		else
		{
			 m_pPetHealthBar->SetUVs( m_pBottomMenuCenter->PixelToU() * 144,
									  m_pBottomMenuCenter->PixelToV() * 102,
									  m_pBottomMenuCenter->PixelToU() * 156,
									  m_pBottomMenuCenter->PixelToV() * 125 );
		}

		m_pPetHealthBar->Render( pD3DDevice );
		m_pPetHealthBarTooltip->SetToolTip( LoadLocalizedString(160) + IntegerToString( m_PetHP ) + "/" + IntegerToString( m_PetMaxHP ) );


		if( m_pPlayerCharacter->Pet( 0 )->IsPetNearDeath() )
		{
			m_pPetFleeing->Render( pD3DDevice );
		}

	}


	if( ( ( !m_StatsMenuOpen &&
		    m_pPlayerCharacter->UnusedStatPoints() > 0 ) ||
		  ( !m_SkillMenuOpen &&
  		    m_pPlayerCharacter->UnusedSkillPoints() > 0 ) ) &&
		!pGameUI.FateOpen()  )
	{

		ShowLevelUpButtons( pD3DDevice );
		m_HelpButtonVisible = kFalse;
	}
	else
	{
		RemoveLevelUpButtons();
	}

	if( m_HelpButtonVisible &&
		!pGameUI.FateOpen() )
	{
		ShowHelpButton( pD3DDevice );
	}
	else
	{
		RemoveHelpButton();
	}

	if( m_pPlayerCharacter->AIState() == KAIFish )
	{
		if( !m_FishingButtonsVisible )
		{
			AddText( m_pFishingText );
			AddButton( m_pSetHookButton );
			AddButton( m_pCancelFishButton );
			m_FishingButtonsVisible = kTrue;
		}
		float32 XPos = 512;
		if( pGameUI.RightCovered() )
		{
			XPos = 256;

		}
		else if( pGameUI.LeftCovered() )
		{
			XPos = 768;
		}
		m_pFishingText->SetPosition( XPos, 200 );
		m_pSetHookButton->SetPosition( XPos, 540 );
		m_pCancelFishButton->SetPosition( XPos, 600 );
	}
	else
	{
		if( m_FishingButtonsVisible )
		{
			RemoveText( m_pFishingText );
			RemoveButton( m_pSetHookButton );
			RemoveButton( m_pCancelFishButton );
			m_FishingButtonsVisible = kFalse;
		}
	}

} // CBottomMenu::Render()

void CBottomMenu::ShowLevelUpButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_LevelUpButtonsVisible )
	{
		FSOUND_PlaySound( FSOUND_FREE, m_pLevelUpSound );

		m_pLevelUpButton = AddButton( new CButton( pD3DDevice,
												   m_pUIButtons,
												   60,
												   660,
												   53,
												   51,
												   m_pUIButtons->PixelToU() * 52,
												   m_pUIButtons->PixelToV() * 0,
												   m_pUIButtons->PixelToU() * 104,
												   m_pUIButtons->PixelToV() * 50,
												   m_pUIButtons->PixelToU() * 52,
												   m_pUIButtons->PixelToV() * 51,
												   m_pUIButtons->PixelToU() * 104,
												   m_pUIButtons->PixelToV() * 101,
												   KButtonTopLeft,
												   KBottomButtonLevelUp ) );

		m_pLevelUpButton->SetToolTip( LoadLocalizedString(161) );

		m_pLevelUpText = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											LoadLocalizedString(162),
											86,
											725,
											.8f,
											kFalse,
											KJustifyCenter ) );

		m_LevelUpButtonsVisible = kTrue;
	}
} // CBottomMenu::ShowLevelUpButtons()

void CBottomMenu::RemoveLevelUpButtons( void )
{
	if( m_LevelUpButtonsVisible )
	{
		RemoveButton( m_pLevelUpButton );
		DELETE_SAFELY( m_pLevelUpButton );

		RemoveText( m_pLevelUpText );
		DELETE_SAFELY( m_pLevelUpText );

		m_LevelUpButtonsVisible = kFalse;
	}
} // CBottomMenu::RemoveLevelUpButtons()

void CBottomMenu::ShowHelpButton( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_HelpButtonAdded )
	{
		FSOUND_PlaySound( FSOUND_FREE, m_pLevelUpSound );

		m_pHelpButton = AddButton( new CButton( pD3DDevice,
												   m_pUIButtons,
												   60,
												   660,
												   53,
												   51,
												   m_pUIButtons->PixelToU() * 52,
												   m_pUIButtons->PixelToV() * 0,
												   m_pUIButtons->PixelToU() * 104,
												   m_pUIButtons->PixelToV() * 50,
												   m_pUIButtons->PixelToU() * 52,
												   m_pUIButtons->PixelToV() * 51,
												   m_pUIButtons->PixelToU() * 104,
												   m_pUIButtons->PixelToV() * 101,
												   KButtonTopLeft,
												   KButtonOpenHelp ) );

		m_pHelpButton->SetToolTip( LoadLocalizedString(1161) );

		m_pHelpButtonText = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											LoadLocalizedString(163),
											86,
											725,
											.8f,
											kFalse,
											KJustifyCenter ) );

		m_HelpButtonAdded = kTrue;
	}
} // CBottomMenu::ShowHelpButton()

void CBottomMenu::RemoveHelpButton( void )
{
	if( m_HelpButtonAdded )
	{
		RemoveButton( m_pHelpButton );
		DELETE_SAFELY( m_pHelpButton );

		RemoveText( m_pHelpButtonText );
		DELETE_SAFELY( m_pHelpButtonText );

		m_HelpButtonAdded = kFalse;
	}
} // CBottomMenu::RemoveHelpButton()

void CBottomMenu::ShowPetPanel( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
							    bool LeftCovered )					// left side of screen covered?
{
	if( !m_PetPanelVisible )
	{
		m_PetPanelVisible = kTrue;

		m_pPetPanel = AddSprite( new CSprite( pD3DDevice,
											  m_pPetPanelMaterial,
											  10,
											  42,
											  128,
											  128,
											  0,
											  0,
											  1,
											  1,
											  KSpriteTopLeft ) );

		m_pPetHealthBarTooltip = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													15,
													150,
													118,
													16,
													0,
													0,
													1,
													1,
													0,
													0,
													1,
													1,
													KButtonTopLeft,
													-999 ) );
		m_pPetHealthBarTooltip->SetToolTip( "0/0" );

		m_pPetHealthBar = AddSprite( new CSprite( pD3DDevice,
												  m_pBottomMenuCenter,
												  15,
												  149,
												  118,
												  16,
												  m_pBottomMenuCenter->PixelToU() * 144,
												  m_pBottomMenuCenter->PixelToV() * 102,
												  m_pBottomMenuCenter->PixelToU() * 156,
												  m_pBottomMenuCenter->PixelToV() * 125,
												  KSpriteTopLeft ) );

		m_pPetUseButton = AddButton( new CButton( pD3DDevice,
												  m_pUIButtons,
												  10,
												  42,
												  74,
												  78,
												  m_pUIButtons->PixelToU() * 250,
												  m_pUIButtons->PixelToV() * 250,
												  m_pUIButtons->PixelToU() * 251,
												  m_pUIButtons->PixelToV() * 251,
												  m_pUIButtons->PixelToU() * 106,
												  m_pUIButtons->PixelToV() * 2,
												  m_pUIButtons->PixelToU() * 180,
												  m_pUIButtons->PixelToV() * 80,
												  KButtonTopLeft,
												  KButtonPetUse ) );

		m_pPetUseButton->SetToolTip( LoadLocalizedString(164) );

		m_pPetStatsButton = AddButton( new CButton( pD3DDevice,
												  m_pUIButtons,
												  85,
												  43,
												  50,
												  50,
												  m_pUIButtons->PixelToU() * 253,
												  m_pUIButtons->PixelToV() * 250,
												  m_pUIButtons->PixelToU() * 254,
												  m_pUIButtons->PixelToV() * 251,
												  m_pUIButtons->PixelToU() * 1,
												  m_pUIButtons->PixelToV() * 103,
												  m_pUIButtons->PixelToU() * 51,
												  m_pUIButtons->PixelToV() * 153,
												  KButtonTopLeft,
												  KButtonPetStats ) );

		m_pPetStatsButton->SetToolTip( LoadLocalizedString(165) );

		m_pPetInventoryButton = AddButton( new CButton( pD3DDevice,
												  m_pUIButtons,
												  85,
												  95,
												  50,
												  50,
												  m_pUIButtons->PixelToU() * 253,
												  m_pUIButtons->PixelToV() * 250,
												  m_pUIButtons->PixelToU() * 254,
												  m_pUIButtons->PixelToV() * 251,
												  m_pUIButtons->PixelToU() * 1,
												  m_pUIButtons->PixelToV() * 154,
												  m_pUIButtons->PixelToU() * 51,
												  m_pUIButtons->PixelToV() * 204,
												  KButtonTopLeft,
												  KButtonPetInventory ) );

		m_pPetInventoryButton->SetToolTip( LoadLocalizedString(166) );


		m_pPetTownButton = AddButton( new CButton( pD3DDevice,
												  m_pUIButtons,
												  10,
												  122,
												  73,
												  23,
												  m_pUIButtons->PixelToU() * 104,
												  m_pUIButtons->PixelToV() * 231,
												  m_pUIButtons->PixelToU() * 176,
												  m_pUIButtons->PixelToV() * 254,
												  m_pUIButtons->PixelToU() * 104,
												  m_pUIButtons->PixelToV() * 206,
												  m_pUIButtons->PixelToU() * 176,
												  m_pUIButtons->PixelToV() * 229,
												  KButtonTopLeft,
												  KButtonPetTown ) );

		m_pPetTownButton->SetToolTip( LoadLocalizedString(167) );
	}

	if( LeftCovered )
	{
		m_PetPanelOnRight = kTrue;
		m_pPetTownButton->SetPosition( 10 + 512, 122 );
		m_pPetInventoryButton->SetPosition( 85 + 512, 95 );
		m_pPetStatsButton->SetPosition( 85 + 512, 43 );
		m_pPetUseButton->SetPosition( 10 + 512, 42 );
		m_pPetHealthBar->SetPosition( 15 + 512, 149 );
		m_pPetHealthBarTooltip->SetPosition( 15 + 512, 150 );
		m_pPetPanel->SetPosition( 10 + 512, 42 );
	}
	else
	{
		m_PetPanelOnRight = kFalse;
		m_pPetTownButton->SetPosition( 10, 122 );
		m_pPetInventoryButton->SetPosition( 85, 95 );
		m_pPetStatsButton->SetPosition( 85, 43 );
		m_pPetUseButton->SetPosition( 10, 42 );
		m_pPetHealthBar->SetPosition( 15, 149 );
		m_pPetHealthBarTooltip->SetPosition( 15, 150 );
		m_pPetPanel->SetPosition( 10, 42 );
	}
} // CBottomMenu::ShowPetPanel()

void CBottomMenu::RemovePetPanel( void )
{
	if( m_PetPanelVisible )
	{
		RemoveSprite( m_pPetPanel );
		DELETE_SAFELY( m_pPetPanel );

		RemoveSprite( m_pPetHealthBar );
		DELETE_SAFELY( m_pPetHealthBar );

		RemoveButton( m_pPetHealthBarTooltip );
		DELETE_SAFELY( m_pPetHealthBarTooltip );

		RemoveButton( m_pPetUseButton );
		DELETE_SAFELY( m_pPetUseButton );

		RemoveButton( m_pPetTownButton );
		DELETE_SAFELY( m_pPetTownButton );

		RemoveButton( m_pPetInventoryButton );
		DELETE_SAFELY( m_pPetInventoryButton );

		RemoveButton( m_pPetStatsButton );
		DELETE_SAFELY( m_pPetStatsButton );

		m_PetPanelVisible = kFalse;
	}
} // CBottomMenu::RemovePetPanel()

// render active spell icons
void CBottomMenu::RenderSpellIcons( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
				   					CGameUI& pGameUI )				// game ui

{
	if( pGameUI.BothCovered() ||
		m_pPlayerCharacter == NULL ||
		m_pPlayerCharacter->ActiveSpell() == NULL )
	{
		if( m_pBaseSpellButton != NULL )
		{
			RemoveButton( m_pBaseSpellButton );
			DELETE_SAFELY( m_pBaseSpellButton );
			m_pBaseSpellButton = NULL;
		}
		return;
	}

	float32 XPos( 955 );
	float32 YPos( 565 );
	if( pGameUI.RightCovered() )
	{
		XPos = 443;

	}

	if( m_pBaseSpellButton == NULL )
	{
		m_pBaseSpellButton = AddButton( new CButton( pD3DDevice,
													m_pSpellButtonMaterial,
													NULL,
													m_pClickSound,
													XPos,
													YPos,
													64,
													64,
													0,
													0,
													.5f,
													.5f,
													0,
													.5f,
													.5f,
													1,
													KButtonTopLeft,
													KButtonSwapSpell ) );
		m_pBaseSpellButton->SetToolTip( LoadLocalizedString(168) );
		int32 HotkeyAssignment = -1;
		if( m_pPlayerCharacter->ActiveSpell() != NULL )
		{
			HotkeyAssignment = pGameUI.HotkeyKeyAssignment( m_pPlayerCharacter->ActiveSpell()->Name() );
		}
		std::string HotkeyText = "";
		if( HotkeyAssignment >= 0 )
		{
			HotkeyText = "F" + IntegerToString( HotkeyAssignment + 1 );
		}
		m_pBaseSpellButton->AddText( pD3DDevice,
									HotkeyText,
									m_pFontMaterial,
									m_pFontMetric,
									7,
									1,
									.8f,
									kFalse,
									KJustifyLeft );
		return;
	}
	else
	{
		int32 HotkeyAssignment = -1;
		if( m_pPlayerCharacter->ActiveSpell() != NULL )
		{
			HotkeyAssignment = pGameUI.HotkeyKeyAssignment( m_pPlayerCharacter->ActiveSpell()->Name() );
		}
		std::string HotkeyText = "";
		if( HotkeyAssignment >= 0 )
		{
			HotkeyText = "F" + IntegerToString( HotkeyAssignment + 1 );
		}
		m_pBaseSpellButton->SetText( pD3DDevice,
									 HotkeyText );
	}


	// we hide everything when enchanted
	if( pGameUI.EnchantOpen() ||
		pGameUI.FateOpen() )
	{
		HideSpellRollout();
		m_pBaseSpellButton->SetPosition( -200, -200 );
		return;
	}

	m_pBaseSpellButton->SetPosition( XPos, YPos );

	pD3DDevice->SetTexture( 0, m_pPlayerCharacter->ActiveSpell()->IconMaterial()->GetTexture() );
	m_pIconSprite->RenderNoTexture( pD3DDevice,
									XPos + 8,
									YPos + 8);		   

	if( m_SpellsRolledOut )
	{
		float32 XPos( 955 );
		float32 YPos( 373 );
		float32 HorizontalStep( -64 );
		if( pGameUI.RightCovered() )
		{
			XPos = 443;
		}

		uint32 CurrentButton( 0 );
		for( uint32 i = 0; i < KMagicSpheres; i++ )
		{
			float32 IconX = XPos;
			float32 IconY = YPos + i * 64;
			for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				CSpellDescription* pDescription = m_pPlayerCharacter->Spell( (EMagicSphere)i, j );
				if( pDescription != NULL &&
					pDescription->RequirementsMet( *m_pPlayerCharacter ) )
				{
					if( CurrentButton < m_pSpellButtons.size() )
					{
						m_pSpellButtons[CurrentButton]->SetPosition( IconX, IconY );
					}
					pD3DDevice->SetTexture( 0, pDescription->IconMaterial()->GetTexture() );
					m_pIconSprite->RenderNoTexture( pD3DDevice,
													IconX + 8,
													IconY + 8);		   

					IconX += HorizontalStep;
					CurrentButton++;
				}
			}

		}

		m_SpellsRolledOut = kTrue;
	}

} // CBottomMenu::RenderSpellIcons()

void CBottomMenu::ShowSpellRollout( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
				   					CGameUI& pGameUI )				// game ui
{
	if( !m_SpellsRolledOut )
	{
		float32 XPos( 955 );
		float32 YPos( 373 );
		float32 HorizontalStep( -64 );
		if( pGameUI.RightCovered() )
		{
			XPos = 443;
		}

		for( uint32 i = 0; i < KMagicSpheres; i++ )
		{
			float32 IconX = XPos;
			float32 IconY = YPos + i * 64;
			for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				CSpellDescription* pDescription = m_pPlayerCharacter->Spell( (EMagicSphere)i, j );
				if( pDescription != NULL  &&
					pDescription->RequirementsMet( *m_pPlayerCharacter ) )
				{
					CButton* pButton = AddButton( new CButton( pD3DDevice,
																m_pSpellButtonMaterial,
																NULL,
																m_pClickSound,
																IconX,
																IconY,
																64,
																64,
																0,
																0,
																.5f,
																.5f,
																0,
																.5f,
																.5f,
																1,
																KButtonTopLeft,
																KSwapSpellSelectButton + i * KMaximumSpellsPerSphere + j ) );
					pButton->SetToolTip( LoadLocalizedString1Variable(169, pDescription->Name()));
					m_pSpellButtons.push_back( pButton );
					IconX += HorizontalStep;
					int32 HotkeyAssignment = pGameUI.HotkeyKeyAssignment( pDescription->Name() );
					std::string HotkeyText = "";
					if( HotkeyAssignment >= 0 )
					{
						HotkeyText = "F" + IntegerToString( HotkeyAssignment + 1 );
					}
					pButton->AddText( pD3DDevice,
									  HotkeyText,
									  m_pFontMaterial,
									  m_pFontMetric,
									  7,
									  1,
									  .8f,
									  kFalse,
									  KJustifyLeft );
				}
			}

		}

		m_SpellsRolledOut = kTrue;
	}
} // CBottomMenu::ShowSpellRollout()

void CBottomMenu::HideSpellRollout( void )
{
	if( m_SpellsRolledOut )
	{
		for( uint32 i = 0; i < m_pSpellButtons.size(); i++ )
		{
			RemoveButton( m_pSpellButtons[i] );
			DELETE_SAFELY( m_pSpellButtons[i] );
		}
		m_pSpellButtons.clear();
		m_SpellsRolledOut = kFalse;
	}
} // CBottomMenu::HideSpellRollout()

// render quick-item slots
void CBottomMenu::RenderInventoryIcons( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	CLLNode<CItemRef>* Root = m_pPlayerCharacter->Inventory()->ItemList()->GetFirst();
	while( Root != NULL )
	{
		int32 Index = Root->Data()->m_SlotIndex;

		if( Index >= KEquipmentSlots  && Index < KQuickSlots)
		{
			float32 Column = (float32)Index - KEquipmentSlots;

			if( Root->Data()->m_pItem->HasIcon() )
			{
				pD3DDevice->SetTexture( 0, Root->Data()->m_pItem->IconMaterial()->GetTexture() );
				m_pIconSprite->RenderNoTexture( pD3DDevice,
												371 + Column * 48,
												720 );		   
			}
		}
		Root = Root->Next();
	}

} // CBottomMenu::RenderInventoryIcons()


void CBottomMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CMaterial* pMaterial = NULL;
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		if( pNode->Data()->Material() != pMaterial )
		{
			pNode->Data()->Render( pD3DDevice );
			pMaterial = pNode->Data()->Material();
		}
		else
		{
			pNode->Data()->RenderNoTexture( pD3DDevice );
		}
		pNode = pNode->Next();
	}
} // CBottomMenu::RenderText()

void CBottomMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CMaterial* pMaterial = NULL;
	CLLNode<CSprite>* pNode = m_pSpriteList->GetFirst();
	while( pNode != NULL )
	{
		if( pNode->Data()->Material() != pMaterial )
		{
			pNode->Data()->Render( pD3DDevice );
			pMaterial = pNode->Data()->Material();
		}
		else
		{
			pNode->Data()->RenderNoTexture( pD3DDevice );
		}
		pNode = pNode->Next();
	}
} // CBottomMenu::RenderSprites()

void CBottomMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CBottomMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CBottomMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
{
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Update( MousePosition );
		pNode = pNode->Next();
	}
} // CBottomMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CBottomMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
{
	if( !ButtonPressed )
	{
		return -1;
	}
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		int32 ReturnCode = pNode->Data()->IsOver( MousePosition );
		if( ReturnCode != -1 )
		{
			pNode->Data()->Press();
			return ReturnCode;
		}
		pNode = pNode->Next();
	}

	return -1;
} // CBottomMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CBottomMenu::UpdateButtonOver( void )
{
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		int32 ReturnCode = pNode->Data()->IsOver( MousePosition );
		if( ReturnCode != -1 )
		{
			return ReturnCode;
		}
		pNode = pNode->Next();
	}

	return -1;
} // CBottomMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CBottomMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
{
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		int32 ReturnCode = pNode->Data()->IsOver( MousePosition );
		if( ReturnCode != -1 )
		{
			TooltipText = pNode->Data()->ToolTip();
			return ReturnCode;
		}
		pNode = pNode->Next();
	}

	return -1;
} // CBottomMenu::UpdateButtonOver()

CButton* CBottomMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CBottomMenu::AddButton()

void CBottomMenu::RemoveButton( CButton* pButton )	// button to remove from the list
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		if( pNode->Data() == pButton )
		{
			m_pButtonList->RemoveNoDelete( pNode );
			return;
		}
		pNode = pNode->Next();
	}
} // CBottomMenu::RemoveButton()

void CBottomMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CBottomMenu::DestroyButtons()

CText* CBottomMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CBottomMenu::AddText()

void CBottomMenu::RemoveText( CText* pText )	// Text to remove from the list
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		if( pNode->Data() == pText )
		{
			m_pTextList->RemoveNoDelete( pNode );
			return;
		}
		pNode = pNode->Next();
	}
} // CBottomMenu::RemoveText()

void CBottomMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CBottomMenu::DestroyText()

CSprite* CBottomMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CBottomMenu::AddSprite()

CSprite* CBottomMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CBottomMenu::AddSpriteFront()

void CBottomMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
{
	CLLNode<CSprite>* pNode = m_pSpriteList->GetFirst();
	while( pNode != NULL )
	{
		if( pNode->Data() == pSprite )
		{
			m_pSpriteList->RemoveNoDelete( pNode );
			return;
		}
		pNode = pNode->Next();
	}
} // CBottomMenu::RemoveSprite()

void CBottomMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CBottomMenu::DestroySprite()

