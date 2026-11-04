/*
	gameui.cpp

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
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../GAME/gamestatemanager.h"

#ifdef ONLINEVERSION
#include "../CDA/cdamanager.h"
#endif

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../IO/keyhandler.h"

#include "../GAMEOBJECTS/camera.h"

#include "../CHARACTER/character.h"
#include "../CHARACTER/inventory.h"
#include "../ITEM/item.h"
#include "../ITEM/spelltemplate.h"

#include "../LEVEL/level.h"
#include "../EFFECTS/eventtext.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"
#include "bottommenu.h"
#include "inventorymenu.h"
#include "skillmenu.h"
#include "questmenu.h"
#include "journalmenu.h"
#include "petinventorymenu.h"
#include "petsellinventorymenu.h"
#include "statsmenu.h"
#include "spellmenu.h"
#include "petstatsmenu.h"
#include "gameoptionsmenu.h"
#include "confirmmenu.h"
#include "enchantmenu.h"
#include "helpmenu.h"
#include "fatemenu.h"
#include "tipmenu.h"
#include "messagemenu.h"
#include "buysellmenu.h"
#include "gameui.h"
#include "gameclient.h"
#include "tips.h"
#include "quest.h"

#include "demorules.h" //marsh

CGameUI::CGameUI( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
				  CRefManager& pRefManager,			// the reference manager, for tracking media we create
				  CSettings& pSettings,				// the settings manager, for keeping track of card/machine capabilities
			  	  CParticleManager& pParticleManager,		// particle manager
				  CGlowManager& pGlowManager,				// glow manager
				  CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
															m_pGameStateManager( pGameStateManager ),
															m_pRefManager( pRefManager ),
															m_pSettings( pSettings ),
															m_pPointerMaterial( NULL ),
															m_pPointerSelectMaterial( NULL ),
															m_pMousePointer( NULL ),
															m_pMiniMapSprite( NULL ),
															m_pPlayerCharacter( NULL ),
															m_pTargetCharacter( NULL ),
															m_pTargetItem( NULL ),
															m_pToolTipText( NULL ),
															m_pTargetHealthBarFront( NULL ),
															m_pTargetHealthBarBack( NULL ),
															m_pDescriptionShadow( NULL ),
															m_pInventoryItemDescription( NULL ),
															m_pTargetName( NULL ),
															m_pConsoleText( NULL ),
															m_pTargetLevel( NULL ),
															m_pTargetResistances( NULL ),
															m_pBottomMenu( NULL ),
															m_pGameOptionsMenu( NULL ),
															m_pConfirmMenu( NULL ),
															m_pEnchantMenu( NULL ),
															m_pHelpMenu( NULL ),
															m_pFateMenu( NULL ),
															m_pTipMenu( NULL ),
															m_pMessageMenu( NULL ),
															m_pInventoryMenu( NULL ),
															m_pSkillMenu( NULL ),
															m_pQuestMenu( NULL ),
															m_pJournalMenu( NULL ),
															m_pPetInventoryMenu( NULL ),
															m_pPetSellInventoryMenu( NULL ),
															m_pStatsMenu( NULL ),
															m_pPetStatsMenu( NULL ),
															m_pSpellMenu( NULL ),
															m_pBuySellMenu( NULL ),
															m_pDraggingItem( NULL ),
															m_pMouseOverItem( NULL ),
															m_pMouseOverSpell( NULL ),
															m_pOwnerOfItemInUse( NULL ),
															m_pUserOfItemInUse( NULL ),
															m_pItemInUse( NULL ),
															m_pBlankMouseSurface( NULL ),
															m_pMouseSurface( NULL ),
															m_pMouseSurfaceSelect( NULL ),
															m_pOwnerOfSpellInUse( NULL ),
															m_pSpellInUse( NULL ),
															m_CursorIsTarget( kFalse ),
															m_CursorIsVisible( kTrue ),
															m_MapVisible( kFalse ),
															m_ConsoleOpen( kFalse ),
															m_DisplayAllItems( kFalse ),
															m_LeftCovered( kFalse ),
															m_RightCovered( kFalse ),
															m_AlwaysRunning( kFalse ),
															m_AlwaysShowingItems( kFalse ),
															m_MiniMapDolly( KMapNormal ),
															m_ConfirmMenuResult( -1 ),
															m_EnchantMenuResult( -1 ),
															m_pButtonList	( new CLList<CButton>() ),
															m_pTextList		( new CLList<CText>() ),
															m_pSpriteList	( new CLList<CSprite>() ),
															m_pEventTextList( new CLList< CEventText >() ),
															m_pExclamation( NULL ),
															m_pExclamationMaterial( NULL ),
															m_pExclamation2( NULL ),
															m_pExclamationMaterial2( NULL ),
															m_pFontMaterial( NULL ),
															m_pFontMaterial2( NULL ),
															m_pFontHighlightMaterial( NULL ),
															m_pFontMetric( NULL ),
															m_pItemOwner( NULL ),
															m_pHealthBarMaterial( NULL ),
															m_pEliteMaterial( NULL ),
															m_pMiniMapMaterial( NULL ),
															m_pDescriptionShadowMaterial( NULL ),
															m_pDescriptionHighlightMaterial( NULL ),
															m_pTargetEliteIcon( NULL ),
															m_pDungeonLevel( NULL ),
															m_pSummonCount( NULL ),
															m_pErrorSound( NULL ),
															m_pLowManaSound( NULL ),
															m_pPurchaseSound( NULL ),
															m_pSelectSpellSound( NULL ),
															m_DragItemSlot ( 0 ),
															m_ViewType (KViewFull),
															m_ConsoleText ( "" ),
															m_HotkeysChanged ( kFalse ),
															m_shouldOpenConsole(kFalse),
															m_consoleConfirmation(kFalse)
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));
	ZeroMemory ( &m_MiniMapFrustum, sizeof( m_MiniMapFrustum ));

	if( !m_pSettings.GetSettings( KSetCustomCursor ) )
	{
		m_pPointerMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															"MENUS/pointersmall.png",
															"MENUS/pointersmallalpha.png",
															kTrue,
															kFalse );

		m_pPointerSelectMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
																	"MENUS/pointerselectsmall.png",
																	"MENUS/pointerselectsmallalpha.png",
																	kTrue,
																	kFalse );
		// make this a system texture now, for the windows cursor
		m_pPointerMaterial->ConvertToFormatSystem( pD3DDevice, 
												   D3DFMT_A8R8G8B8 );
		// grab the surface
		m_pPointerMaterial->GetTexture()->GetSurfaceLevel( 0, &m_pMouseSurface );

		// make this a system texture now, for the windows cursor
		m_pPointerSelectMaterial->ConvertToFormatSystem( pD3DDevice, 
														 D3DFMT_A8R8G8B8 );
		// grab the surface
		m_pPointerSelectMaterial->GetTexture()->GetSurfaceLevel( 0, &m_pMouseSurfaceSelect );

		CMaterial* pBlankPointerMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
																		   "MENUS/black.png",
																		   "MENUS/black.png", 
																		   kTrue,
																		   kFalse );

		// make this a system texture now, for the windows cursor
		pBlankPointerMaterial->ConvertToFormatSystem( pD3DDevice, 
															D3DFMT_A8R8G8B8 );
		// grab the surface
		pBlankPointerMaterial->GetTexture()->GetSurfaceLevel( 0, &m_pBlankMouseSurface );

	}
	else
	{
		m_pPointerMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															"MENUS/pointer.png",
															"MENUS/pointeralpha.png" );

		m_pPointerSelectMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
																"MENUS/pointerselect.png",
																"MENUS/pointerselectalpha.png" );
	}

	m_pMousePointer = new CSprite( pD3DDevice,
								   m_pPointerMaterial,
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

	m_pFontMaterial2	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font2.png",
													  "FONT/fonta.png" );

	// initialize font texture and metric
	m_pFontHighlightMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
																  "FONT/highlights.png",
																  "FONT/highlightsa.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );
	m_pFontMetric->SetKerning( -4 );
	m_pFontMetric->Pad( 4 );

	CreateMenus( pD3DDevice, pParticleManager, pGlowManager );

	m_pErrorSound = m_pRefManager.AddSample( "SOUNDS/UI/error.wav", kFalse );

	m_pLowManaSound = m_pRefManager.AddSample( "SOUNDS/UI/lowmana.wav", kFalse );

	m_pSelectSpellSound = m_pRefManager.AddSample( "SOUNDS/UI/levelup.wav", kFalse );

	m_pPurchaseSound = m_pRefManager.AddSample( "SOUNDS/ITEMS/gold.wav", kFalse );

	ClearHotkeyAssignments();
} // CGameUI::CGameUI()

CGameUI::~CGameUI( void )
{
	RELEASE_SAFELY( m_pBlankMouseSurface );
	RELEASE_SAFELY( m_pMouseSurface );
	RELEASE_SAFELY( m_pMouseSurfaceSelect );
	for( uint32 i = 0; i < 10; i++ )
	{
		RemoveButton( m_pPlayerEffects[i] );
		DELETE_SAFELY( m_pPlayerEffects[i] );
		RemoveButton( m_pTargetEffects[i] );
		DELETE_SAFELY( m_pTargetEffects[i] );
	}

	DELETE_SAFELY( m_pExclamation );
	DELETE_SAFELY( m_pExclamation2 );
	DELETE_SAFELY( m_pEventTextList );

	for( i = 0; i < KHighlightColors; i++ )
	{
		DELETE_SAFELY( m_pHighlight[i] );
	}

	DELETE_SAFELY( m_pDraggingItem );

	DELETE_SAFELY( m_pMiniMapSprite );

	DELETE_SAFELY( m_pDescriptionShadow );

	DELETE_SAFELY( m_pBottomMenu );
	DELETE_SAFELY( m_pGameOptionsMenu );
	DELETE_SAFELY( m_pConfirmMenu );
	DELETE_SAFELY( m_pEnchantMenu );
	DELETE_SAFELY( m_pHelpMenu );
	DELETE_SAFELY( m_pFateMenu );
	DELETE_SAFELY( m_pTipMenu );
	DELETE_SAFELY( m_pMessageMenu );
	DELETE_SAFELY( m_pInventoryMenu );
	DELETE_SAFELY( m_pSkillMenu );
	DELETE_SAFELY( m_pQuestMenu );
	DELETE_SAFELY( m_pJournalMenu );
	DELETE_SAFELY( m_pPetInventoryMenu );
	DELETE_SAFELY( m_pPetSellInventoryMenu );
	DELETE_SAFELY( m_pStatsMenu );
	DELETE_SAFELY( m_pSpellMenu );
	DELETE_SAFELY( m_pPetStatsMenu );
	DELETE_SAFELY( m_pBuySellMenu );

	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );


	DELETE_SAFELY( m_pConsoleText );
	DELETE_SAFELY( m_pTargetName );
	DELETE_SAFELY( m_pTargetLevel );
	DELETE_SAFELY( m_pTargetResistances );
	DELETE_SAFELY( m_pToolTipText );
	DELETE_SAFELY( m_pInventoryItemDescription );

	m_pRefManager.RemoveSample( m_pPurchaseSound );
	m_pRefManager.RemoveSample( m_pErrorSound );
	m_pRefManager.RemoveSample( m_pLowManaSound );
	m_pRefManager.RemoveSample( m_pSelectSpellSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pExclamationMaterial );
	m_pRefManager.RemoveMaterial( m_pExclamationMaterial2 );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pFontMaterial2 );
	m_pRefManager.RemoveMaterial( m_pFontHighlightMaterial );
	m_pRefManager.RemoveMaterial( m_pHealthBarMaterial );
	m_pRefManager.RemoveMaterial( m_pEliteMaterial );
	m_pRefManager.RemoveMaterial( m_pDescriptionShadowMaterial );
	m_pRefManager.RemoveMaterial( m_pDescriptionHighlightMaterial );
	m_pRefManager.RemoveMaterial( m_pMiniMapMaterial );

	m_pRefManager.RemoveMaterial( m_pPointerMaterial );
	m_pRefManager.RemoveMaterial( m_pPointerSelectMaterial );

	DELETE_SAFELY( m_pMousePointer );

	DELETE_SAFELY( m_pTargetHealthBarBack );
	DELETE_SAFELY( m_pTargetHealthBarFront );
	DELETE_SAFELY( m_pTargetEliteIcon );

} // CGameUI::~CGameUI()

void CGameUI::Reset( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pItemInUse = NULL;
	m_pSpellInUse = NULL;
	m_CursorIsTarget = kFalse;
	m_CursorIsVisible = kTrue;

	ResetMouse();
} // CGameUI::Reset()

void CGameUI::ResetMouse( void )
{

	if( !m_pSettings.GetSettings( KSetCustomCursor ) )
	{
		if( Targeting() )
		{
			SetCursor( LoadCursor( NULL, IDC_CROSS ) );
			//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurfaceSelect );
			m_CursorIsTarget = kTrue;
			m_CursorIsVisible = kTrue;
		}
		else
		{
			SetCursor( LoadCursor( NULL, IDC_ARROW ) );
			//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurface );
			m_CursorIsTarget = kFalse;
			m_CursorIsVisible = kTrue;
		}
		//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurface );
	}
} // CGameUI::ResetMouse()

void CGameUI::ClearHotkeyAssignments( void )
{
	for( uint32 i = 0; i < KMaxHotkeys; i++ )
	{
		m_HotkeySpellAssignment[i] = "";
	}
} // CGameUI::ClearHotkeyAssignments()

const std::string& CGameUI::HotkeyAssignment( uint32 Hotkey )	// hotkey to retrieve
{
	return m_HotkeySpellAssignment[Hotkey];
} // CGameUI::HotkeyAssignment()

int32 CGameUI::HotkeyKeyAssignment( const std::string& SpellName )	// spell to check for
{
	for( uint32 i = 0; i < KMaxHotkeys; i++ )
	{
		if( m_HotkeySpellAssignment[i] == SpellName )
		{
			return i;
		}
	}
	return -1;
} // CGameUI::HotkeyKeyAssignment()

void CGameUI::SetHotkeyAssignment( uint32 Hotkey,					// hotkey to use
								   const std::string& SpellName )	// spell to set on it
{
	// clear out previous mapping
	ClearHotkeyAssignment( SpellName );
	ClearHotkeyAssignment( m_HotkeySpellAssignment[Hotkey] );

	m_HotkeySpellAssignment[Hotkey] = SpellName;
	m_HotkeysChanged = kTrue;
} // CGameUI::SetHotkeyAssignment()

void CGameUI::ClearHotkeyAssignment( const std::string& SpellName )	// name of spell to clear hotkey assignment for
{
	for( uint32 i = 0; i < KMaxHotkeys; i++ )
	{
		if( m_HotkeySpellAssignment[i] == SpellName )
		{
			m_HotkeySpellAssignment[i] = "";
			m_HotkeysChanged = kTrue;
		}
	}
} // CGameUI::ClearHotkeyAssignment()

void CGameUI::VerifyHotkeyAssignments( void )
{
	for( uint32 i = 0; i < KMaxHotkeys; i++ )
	{
		if( m_HotkeySpellAssignment[i].length() > 0 )
		{
			if( !m_pPlayerCharacter->HasSpell( m_HotkeySpellAssignment[i] ) )
			{
				m_HotkeySpellAssignment[i] = "";
				m_HotkeysChanged = kTrue;
			}
		}
	}
} // CGameUI::VerifyHotkeyAssignments()

void CGameUI::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pFateMenu->RestoreDeviceObjects( pD3DDevice );
	m_pHelpMenu->RestoreDeviceObjects( pD3DDevice );

	if( !m_pSettings.GetSettings( KSetCustomCursor ) )
	{
		if( Targeting() ||
			!m_CursorIsVisible )
		{
			if( !m_CursorIsTarget )
			{
				SetCursor( LoadCursor( NULL, IDC_CROSS ) );
				//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurfaceSelect );
				m_CursorIsTarget = kTrue;
				m_CursorIsVisible = kTrue;
			}
		}
		else
		{
			if( m_CursorIsTarget||
				!m_CursorIsVisible )
			{
				SetCursor( LoadCursor( NULL, IDC_ARROW ) );
				//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurface );
				m_CursorIsTarget = kFalse;
				m_CursorIsVisible = kTrue;
			}
		}
	}

} // CGameUI::RestoreDeviceObjects()

void CGameUI::FlushAll( void )
{
	m_pBottomMenu->FlushAll();
	m_pGameOptionsMenu->FlushAll();
	m_pConfirmMenu->FlushAll();
	m_pEnchantMenu->FlushAll();
	m_pHelpMenu->FlushAll();
	m_pFateMenu->FlushAll();
	m_pTipMenu->FlushAll();
	m_pMessageMenu->FlushAll();
	m_pInventoryMenu->FlushAll();
	m_pSkillMenu->FlushAll();
	m_pQuestMenu->FlushAll();
	m_pJournalMenu->FlushAll();
	m_pPetInventoryMenu->FlushAll();
	m_pPetSellInventoryMenu->FlushAll();
	m_pStatsMenu->FlushAll();
	m_pSpellMenu->FlushAll();
	m_pPetStatsMenu->FlushAll();
	m_pBuySellMenu->FlushAll();

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CGameUI::FlushAll()

void CGameUI::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
  						   CParticleManager& pParticleManager,		// particle manager
						   CGlowManager& pGlowManager )				// glow manager

{

	m_pExclamationMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														     "MENUS/exclamation.png",
														     "MENUS/exclamationa.png" );
	m_pExclamationMaterial2 = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														     "MENUS/exclamation2.png",
														     "MENUS/exclamationa.png" );

	m_pHealthBarMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/healthbar.png",
														   "MENUS/healthbara.png" );

	m_pEliteMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													   "MENUS/elite.png",
													   "MENUS/elitea.png" );

	m_pMiniMapMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														 "MENUS/mapbackground.png",
													     "MENUS/mapbackgrounda.png" );

	m_pDescriptionShadowMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
																	"MENUS/shadow.png",
																	"MENUS/shadowa.png" );

	m_pDescriptionHighlightMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
																	"MENUS/highlight.png",
																	"MENUS/highlighta.png" );

	m_pMiniMapSprite = new CSprite( pD3DDevice,
								   m_pMiniMapMaterial,
								   (float32)KMiniMapTopLeftX,
								   (float32)KMiniMapTopLeftY,
								   (float32)KMiniMapWidth,
								   (float32)KMiniMapHeight,
								   0,
								   0,
								   1,
								   1,
								   KSpriteTopLeft );

	m_pExclamation = new CSprite( pD3DDevice,
								  m_pExclamationMaterial,
								  0,
								  0,
								  64,
								  64,
								  0,
								  0,
								  1,
								  1,
								  KSpriteCenter );
	m_pExclamation2 = new CSprite( pD3DDevice,
								  m_pExclamationMaterial2,
								  0,
								  0,
								  64,
								  64,
								  0,
								  0,
								  1,
								  1,
								  KSpriteCenter );

	m_pDescriptionShadow = new CSprite( pD3DDevice,
										m_pDescriptionShadowMaterial,
										0,
										0,
										48,
										48,
										0,
										0,
										1,
										1,
										KSpriteCenter );

	m_pInventoryItemDescription = new CText( pD3DDevice,
											 m_pFontMaterial,
											 m_pFontMetric,
											 "",
											 512,
											 48,
											 1,
											 kFalse,
											 KJustifyCenter );


	m_pConsoleText = new CText( pD3DDevice,
							   m_pFontMaterial,
							   m_pFontMetric,
							   "",
							   2,
							   384,
							   1.5f,
							   kFalse,
							   KJustifyLeft );

	m_pTargetName = new CText( pD3DDevice,
							   m_pFontMaterial,
							   m_pFontMetric,
							   "",
							   512,
							   48,
							   1,
							   kFalse,
							   KJustifyCenter );

	m_pTargetLevel = new CText( pD3DDevice,
							   m_pFontMaterial,
							   m_pFontMetric,
							   "",
							   512,
							   25,
							   .75f,
							   kFalse,
							   KJustifyCenter );

	m_pTargetResistances = new CText( pD3DDevice,
									  m_pFontMaterial,
									  m_pFontMetric,
									  "",
									  512,
									  70,
									  .75f,
									  kTrue,
									  KJustifyCenter );

	m_pToolTipText = new CText( pD3DDevice,
								   m_pFontMaterial,
								   m_pFontMetric,
								   "",
								   512,
								   48,
								   .8f,
							       kFalse,
							       KJustifyCenter );


	m_pTargetHealthBarBack = new CSprite( pD3DDevice,
										  m_pHealthBarMaterial,
										  312,
										  48,
										  400,
										  32,
										  0,
										  .3f,
										  1,
										  .4f,
										  KSpriteTopLeft );

	m_pTargetHealthBarFront = new CSprite( pD3DDevice,
										   m_pHealthBarMaterial,
										   312,
										   48,
										   400,
										   32,
										   0,
										   .1f,
										   1,
										   .2f,
										   KSpriteTopLeft );

	m_pTargetEliteIcon = new CSprite( pD3DDevice,
								m_pEliteMaterial,
								0,
								0,
								32,
								32,
								0,
								0,
								1,
								1,
								KSpriteTopLeft );

	m_pBottomMenu = new CBottomMenu( pD3DDevice,
									 m_pRefManager,
									 m_pSettings,
									 m_pGameStateManager );

	m_pGameOptionsMenu = new CGameOptionsMenu( pD3DDevice,
												m_pRefManager,
												m_pSettings,
												m_pGameStateManager );

	m_pConfirmMenu = new CConfirmMenu( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pGameStateManager );

	m_pEnchantMenu = new CEnchantMenu( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pGameStateManager );

	m_pHelpMenu = new CHelpMenu( pD3DDevice,
							     m_pRefManager,
								 m_pSettings,
								 m_pGameStateManager );

	m_pFateMenu = new CFateMenu( pD3DDevice,
							     m_pRefManager,
								 m_pSettings,
								 pParticleManager,
								 pGlowManager,
								 m_pGameStateManager );

	m_pTipMenu = new CTipMenu( pD3DDevice,
							   m_pRefManager,
							   m_pSettings,
							   m_pGameStateManager );

	m_pMessageMenu = new CMessageMenu( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pGameStateManager );

	m_pInventoryMenu = new CInventoryMenu( pD3DDevice,
										   m_pRefManager,
										   m_pSettings,
										   m_pGameStateManager );

	m_pSkillMenu = new CSkillMenu( pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   m_pGameStateManager );

	m_pQuestMenu = new CQuestMenu( pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   m_pGameStateManager );

	m_pJournalMenu = new CJournalMenu( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pGameStateManager );

	m_pPetInventoryMenu = new CPetInventoryMenu( pD3DDevice,
												 m_pRefManager,
												 m_pSettings,
												 m_pGameStateManager );

	m_pPetSellInventoryMenu = new CPetSellInventoryMenu( pD3DDevice,
													 m_pRefManager,
													 m_pSettings,
													 m_pGameStateManager );

	m_pStatsMenu = new CStatsMenu( pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   m_pGameStateManager );

	m_pSpellMenu = new CSpellMenu( pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   m_pGameStateManager );

	m_pPetStatsMenu = new CPetStatsMenu( pD3DDevice,
									     m_pRefManager,
										 m_pSettings,
										 m_pGameStateManager );

	m_pBuySellMenu = new CBuySellMenu( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pGameStateManager );

	m_pDungeonLevel = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"Level 1",
											1016,
											8,
											1.0f,
											kFalse,
											KJustifyRight ) );

	m_pSummonCount = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"",
											10,
											108,
											1.0f,
											kFalse,
											KJustifyLeft ) );

	float32 StepSize = 1.0f / (float32)KHighlightColors;
	for( uint32 i = 0; i < KHighlightColors; i++ )
	{
		m_pHighlight[i] = new CSprite( pD3DDevice,
									   m_pFontHighlightMaterial,
									   0,
									   0,
									   32,
									   32,
									   0,
									   i * StepSize + StepSize *.4f,
									   1,
									   i * StepSize + StepSize *.6f,
									   KSpriteCenter );
	}

	for( i = 0; i < 10; i++ )
	{
		m_pPlayerEffects[i] = new CButton( pD3DDevice,
													  m_pFontHighlightMaterial,
													  NULL,
													  NULL,
													  20,
													  180 + (float32)i * 32,
													  24,
													  24,
													  0,
													  0,
													  1,
													  1,
													  0,
													  0,
													  1,
													  1,
													  KButtonTopLeft,
													  -10 );

		m_pTargetEffects[i] = new CButton( pD3DDevice,
													  m_pFontHighlightMaterial,
													  NULL,
													  NULL,
													  20,
													  180 + (float32)i * 32,
													  24,
													  24,
													  0,
													  0,
													  1,
													  1,
													  0,
													  0,
													  1,
													  1,
													  KButtonTopLeft,
													  -10 );
	}

} // CGameUI::CreateMenus()

void CGameUI::SetPlayerCharacter( CCharacter* pCharacter )		// character to set
{
	m_pPlayerCharacter = pCharacter;
	m_pFateMenu->SetPlayerCharacter( pCharacter );
	m_pInventoryMenu->SetPlayerCharacter( pCharacter );
	m_pBuySellMenu->SetPlayerCharacter( pCharacter );
	m_pSpellMenu->SetPlayerCharacter( pCharacter );
	m_pStatsMenu->SetPlayerCharacter( pCharacter );
	m_pSkillMenu->SetPlayerCharacter( pCharacter );
	m_pQuestMenu->SetPlayerCharacter( pCharacter );
	m_pJournalMenu->SetPlayerCharacter( pCharacter );
	if( pCharacter->HasPets() )
	{
		m_pPetStatsMenu->SetPetCharacter( pCharacter->Pet( 0) );
		m_pPetInventoryMenu->SetPetCharacter( pCharacter->Pet( 0 ) );
		m_pPetSellInventoryMenu->SetPetCharacter( pCharacter->Pet( 0 ) );
		m_pBottomMenu->SetPetCharacter( pCharacter->Pet( 0 ) );
	}
	else
	{
		m_pPetStatsMenu->SetPetCharacter( NULL );
		m_pPetInventoryMenu->SetPetCharacter( NULL );
		m_pPetSellInventoryMenu->SetPetCharacter( NULL );
		m_pBottomMenu->SetPetCharacter( NULL );
	}
	m_pBottomMenu->SetPlayerCharacter( pCharacter );
} // CGameUI::SetPlayerCharacter()

bool CGameUI::MouseOverMenus( void )
{
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( m_pGameOptionsMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pConfirmMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pEnchantMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pHelpMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pFateMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pTipMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pMessageMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pBottomMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pInventoryMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pSkillMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pQuestMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pJournalMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pPetInventoryMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pPetSellInventoryMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pStatsMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pSpellMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pPetStatsMenu->MouseOver() )
	{
		return kTrue;
	}
	if( m_pBuySellMenu->MouseOver() )
	{
		return kTrue;
	}

	return kFalse;
} // CGameUI::MouseOverMenus()

void CGameUI::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );

	m_pGameOptionsMenu->KeyEvent( WMMessage,
								  KeyCode );
	m_pConfirmMenu->KeyEvent( WMMessage,
							  KeyCode );
	m_pEnchantMenu->KeyEvent( WMMessage,
							  KeyCode );
	m_pHelpMenu->KeyEvent( WMMessage,
							  KeyCode );
	m_pFateMenu->KeyEvent( WMMessage,
							  KeyCode );
	m_pTipMenu->KeyEvent( WMMessage,
						  KeyCode );
	m_pMessageMenu->KeyEvent( WMMessage,
							  KeyCode );
	// options menu precludes everything else
	if( Paused() )
	{
		return;
	}
	if( m_ConsoleOpen )
	{
		if( WMMessage == WM_KEYDOWN )
		{

			bool Shifted( m_KeyHandler.KeyHeld( VK_SHIFT ) );
			if( m_KeyHandler.CapsLock() )
			{
				Shifted = !Shifted;
			}

			char Code = m_KeyHandler.ShiftCharacter( KeyCode, Shifted );

			//  if valid characters, and we have space for them,
			// add them to the unlock code
			if( ( ( Code >= 65 && Code <= 90 ) || 
				( Code >= 48 && Code <= 57 ) ||
				//attempting to add support for extended characters with these
				( Code >= 192 && Code <= 221 ) ||
				( Code >= 224 && Code <= 253 ) ||
				//
				( Code >= 97 && Code <= 122 ) ||
				Code == '_' ||
				Code == '-' ||
				Code == '\'' ||
				Code == '`' ||
				Code == 32 )  )
			{

				 m_ConsoleText =  m_ConsoleText + Code;
			}
			// if backspace, delete a character
			else if( KeyCode == 8 && m_ConsoleText.length() > 0 )
			{
				 m_ConsoleText.resize(  m_ConsoleText.length() - 1 );
			}

		}
	}

	m_pBottomMenu->KeyEvent( WMMessage,
							 KeyCode );
	m_pInventoryMenu->KeyEvent( WMMessage,
								KeyCode );
	m_pSkillMenu->KeyEvent( WMMessage,
							KeyCode );
	m_pQuestMenu->KeyEvent( WMMessage,
							KeyCode );
	m_pJournalMenu->KeyEvent( WMMessage,
								KeyCode );
	m_pPetInventoryMenu->KeyEvent( WMMessage,
								   KeyCode );
	m_pPetSellInventoryMenu->KeyEvent( WMMessage,
								   KeyCode );
	m_pStatsMenu->KeyEvent( WMMessage,
							KeyCode );
	m_pSpellMenu->KeyEvent( WMMessage,
							KeyCode );
	m_pPetStatsMenu->KeyEvent( WMMessage,
							KeyCode );
	m_pBuySellMenu->KeyEvent( WMMessage,
							  KeyCode );
} // CGameUI::KeyEvent()

void CGameUI::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	m_MouseHandler.MouseEvent( WMMessage, WMParam );

	m_pGameOptionsMenu->MouseEvent( WMMessage,
					 			    WMParam );
	m_pConfirmMenu->MouseEvent( WMMessage,
				 			    WMParam );
	m_pEnchantMenu->MouseEvent( WMMessage,
				 			    WMParam );
	m_pHelpMenu->MouseEvent( WMMessage,
				 			 WMParam );
	m_pFateMenu->MouseEvent( WMMessage,
				 			 WMParam );
	m_pTipMenu->MouseEvent( WMMessage,
			 			    WMParam );
	m_pMessageMenu->MouseEvent( WMMessage,
							    WMParam );
	// options menu precludes everything else
	if( Paused() )
	{
		return;
	}

	m_pBottomMenu->MouseEvent( WMMessage,
							   WMParam );

	m_pInventoryMenu->MouseEvent( WMMessage,
								  WMParam );
	m_pSkillMenu->MouseEvent( WMMessage,
							  WMParam );
	m_pQuestMenu->MouseEvent( WMMessage,
							  WMParam );
	m_pJournalMenu->MouseEvent( WMMessage,
								  WMParam );
	m_pPetInventoryMenu->MouseEvent( WMMessage,
									 WMParam );
	m_pPetSellInventoryMenu->MouseEvent( WMMessage,
									 WMParam );
	m_pStatsMenu->MouseEvent( WMMessage,
							  WMParam );
	m_pSpellMenu->MouseEvent( WMMessage,
							  WMParam );
	m_pPetStatsMenu->MouseEvent( WMMessage,
							  WMParam );
	m_pBuySellMenu->MouseEvent( WMMessage,
								WMParam );
} // CGameUI::KeyEvent()

void CGameUI::OpenConfirmation( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
								const std::string& Message,		// message to be displayed
								const std::string& YesMessage,	// yes button text
								int32 YesCode,					// code for yes return
								const std::string& NoMessage,	// no button text
								int32 NoCode,					// code for no return
								float32 Scale )					// text scale
{
	m_ConfirmMenuResult = -1;
	m_pConfirmMenu->Open( pD3DDevice,
						  Message,
						  YesMessage,
						  YesCode,
						  NoMessage,
						  NoCode,
						  Scale );
} // CGameUI::OpenConfirmation()

void CGameUI::OpenEnchant( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
						   const D3DXVECTOR3& Position,		// position of enchanting item
						   const std::string& Message,		// message to be displayed
						   const std::string& YesMessage,	// yes button text
						   int32 YesCode,					// code for yes return
						   const std::string& NoMessage,	// no button text
						   int32 NoCode,					// code for no return
						   float32 Scale )					// text scale
{
	m_EnchantMenuResult = -1;
	m_pEnchantMenu->Open( pD3DDevice,
						  Position,
						  Message,
						  YesMessage,
						  YesCode,
						  NoMessage,
						  NoCode,
						  Scale );
} // CGameUI::OpenEnchant()

void CGameUI::OpenTip( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
					   const std::string& Message )		// message to be displayed
{
	m_pTipMenu->Open( pD3DDevice,
					  Message );
} // CGameUI::OpenTip()


void CGameUI::OpenMessage( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
						   const std::string& Message,		// message to be displayed
						   const std::string& ButtonMessage,// message to be displayed
						   int32 ButtonValue )				// button code
{
	m_pMessageMenu->Open( pD3DDevice,
						  Message,
						  ButtonMessage,
						  ButtonValue );
} // CGameUI::OpenMessage()

void CGameUI::OpenFate( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
						CLevel& pLevel )				// level to execute into
{
	CloseAll( pLevel );
	m_pFateMenu->Open( pD3DDevice, pLevel );
} // CGameUI::OpenFate()

void CGameUI::ExecuteConsole( CGameClient& pClient,
							  CLevel& pLevel )				// level to execute into
{
	if( !IsGameUnlocked() ) return; //marsh
//#ifdef USES_PAL
//	if( kTrue ) return; //marsh so the compiler won't give you an error
//#endif
	if( StringUpper( m_ConsoleText ) == "LEVELUP" )
	{
		m_pPlayerCharacter->AwardExperience( pLevel,
											 NULL,
											 ExperienceGate( m_pPlayerCharacter->Level() ) -
											 m_pPlayerCharacter->Experience() );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "GOD" )
	{
		for( uint32 i = 0; i < 10; i++ )
		{
			m_pPlayerCharacter->AwardExperience( pLevel,
												 NULL,
												 ExperienceGate( m_pPlayerCharacter->Level() ) -
												 m_pPlayerCharacter->Experience() );
		}
		return;
	}
	if( m_ConsoleText.length() >= 7 &&
		StringUpper( m_ConsoleText.substr( 0, 7 ) ) == "DESCEND" )
	{
		int32 Levels( 1 );
		if( m_ConsoleText.length() > 8 )
		{
			Levels = atoi( m_ConsoleText.substr( 8, m_ConsoleText.length() - 8 ).c_str() );
		}
		pLevel.RequestLevelChange( Levels );
	}
	if( m_ConsoleText.length() >= 6 &&
		StringUpper( m_ConsoleText.substr( 0, 6 ) ) == "ASCEND" )
	{
		int32 Levels( 1 );
		if( m_ConsoleText.length() > 7 )
		{
			Levels = atoi( m_ConsoleText.substr( 7, m_ConsoleText.length() - 7 ).c_str() );
		}
		pLevel.RequestLevelChange( -Levels );
	}
	if( StringUpper( m_ConsoleText ) == "EXPERIENCE" )
	{
		m_pPlayerCharacter->AwardExperience( pLevel, NULL, 5000 );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "BLOOD" )
	{
		m_pSettings.SetSettings( KSetRedBlood, kTrue );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "NOBLOOD" )
	{
		m_pSettings.SetSettings( KSetRedBlood, kFalse );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "GOLD" )
	{
		m_pPlayerCharacter->GiveGold( 500000 );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "FAME" )
	{
		m_pPlayerCharacter->AwardFame( pLevel, NULL, 1000 );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "DISCOVERALL" )
	{
		pClient.Level()->DiscoverAll();
		return;
	}
	if( StringUpper( m_ConsoleText ) == "HEAL" )
	{
		m_pPlayerCharacter->ModifyHP( 100 );
		return;
	}
	if( StringUpper( m_ConsoleText ) == "DUMPMAP" )
	{
		pClient.Level()->DumpToFile();
		return;
	}
	CCharacterDescription* pDescription = pClient.CharacterManifest()->CharacterDescription( m_ConsoleText );
	if( pDescription != NULL )
	{
		CCharacter* pNewMonster =	pLevel.AddMonster( *pDescription,
													   pLevel.RandomOpenPosition( m_pPlayerCharacter->Position(), 12 ) );

		pLevel.OutfitCharacter( *pDescription,
								*pNewMonster );

		pLevel.GenerateMinions( *pDescription,
								*pNewMonster );

	}
	else
	{
		CItemDescription* pDescription = pClient.ItemManifest()->ItemDescriptionEnterableName( m_ConsoleText );
		if( pDescription != NULL )
		{
			CItem* pItem = pLevel.AddItem( *pDescription,
										   pLevel.RandomOpenPosition( m_pPlayerCharacter->Position(), 12 ) );

			if( pItem != NULL )
			{
				pItem->Drop( pLevel );
			}

		}
	}

	m_ConsoleText = "";
} // CGameUI::ExecuteConsole()

void CGameUI::ToggleSpellMenu( void )
{
	if( !m_pBuySellMenu->Open() )
	{
		m_pSpellMenu->SetOpen( !m_pSpellMenu->Open() );
		if( m_pSpellMenu->Open() && m_pPetInventoryMenu->Open() )
		{
			m_pPetInventoryMenu->SetOpen( kFalse );
		}
		if( m_pSpellMenu->Open() && m_pStatsMenu->Open() )
		{
			m_pStatsMenu->SetOpen( kFalse );
		}
	}
} // CGameUI::ToggleSpellMenu()

void CGameUI::ToggleStatsMenu( CLevel& pLevel )					// level we are in
{
	if( !m_pBuySellMenu->Open() )
	{
		m_pStatsMenu->SetOpen( !m_pStatsMenu->Open() );
		if( m_pStatsMenu->Open() && m_pPetInventoryMenu->Open() )
		{
			m_pPetInventoryMenu->SetOpen( kFalse );
		}
		if( m_pStatsMenu->Open() && m_pSpellMenu->Open() )
		{
			m_pSpellMenu->SetOpen( kFalse );
		}
	}

	if( m_pStatsMenu->Open() &&
		m_pPlayerCharacter->UnusedStatPoints() > 0 )
	{
		// pop up a 'level up' tip?
		if( m_pSettings.GetSettings( KSetShowTips ) &&
			!pLevel.Tips().TipSeen( KTipLevelUp2 ) )
		{
			pLevel.RequestTip( pLevel.Tips().ContextTip( KTipLevelUp2 ) );
			pLevel.Tips().SetTipSeen( KTipLevelUp2, kTrue );
		}
	}
} // CGameUI::ToggleStatsMenu()

void CGameUI::TogglePetStatsMenu( void )
{
	if( !m_pBuySellMenu->Open() )
	{
		m_pPetStatsMenu->SetOpen( !m_pPetStatsMenu->Open() );
		if( m_pPetStatsMenu->Open() && m_pInventoryMenu->Open() )
		{
			m_pInventoryMenu->SetOpen( kFalse );
		}
		if( m_pPetStatsMenu->Open() && m_pSkillMenu->Open() )
		{
			m_pSkillMenu->SetOpen( kFalse );
		}
		if( m_pPetStatsMenu->Open() && m_pQuestMenu->Open() )
		{
			m_pQuestMenu->SetOpen( kFalse );
		}
		if( m_pPetStatsMenu->Open() && m_pJournalMenu->Open() )
		{
			m_pJournalMenu->SetOpen( kFalse );
		}
	}
} // CGameUI::TogglePetStatsMenu()

void CGameUI::ToggleInventoryMenu( CLevel& pLevel )					// level we are in
{
	if( m_pBuySellMenu->Open() &&
		m_pPetSellInventoryMenu->Open() )
	{
		if( m_pDraggingItem != NULL )
		{
			ReturnItem( pLevel, m_pItemOwner );
		}
		m_pBuySellMenu->SetOpen( kFalse );
		m_pPetSellInventoryMenu->SetOpen( kFalse );
		if( m_pPlayerCharacter->Pet( 0 ) != NULL &&
			m_pPlayerCharacter->Pet( 0 )->Alive() )
		{
			m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIInactive );
		}
		return;
	}

	m_pInventoryMenu->SetOpen( !m_pInventoryMenu->Open() );
	// stop buying or selling when we close the inventory
	if( !m_pInventoryMenu->Open() &&
		m_pBuySellMenu->Open() )
	{
		if( m_pDraggingItem != NULL )
		{
			ReturnItem( pLevel, m_pItemOwner );
		}
		m_pBuySellMenu->SetOpen( kFalse );
		m_pPetSellInventoryMenu->SetOpen( kFalse );
		if( m_pPlayerCharacter != NULL &&
			m_pPlayerCharacter->Alive() )
		{
			m_pPlayerCharacter->SetAIState( KAIInactive );
		}
	}
	else if( m_pInventoryMenu->Open() )
	{
		// inventory tip
		if( m_pSettings.GetSettings( KSetShowTips ) &&
			!pLevel.Tips().TipSeen( KTipInventory ) )
		{
			pLevel.RequestTip( pLevel.Tips().ContextTip( KTipInventory ) );
			pLevel.Tips().SetTipSeen( KTipInventory, kTrue );
		}

		if( m_pInventoryMenu->Open() && m_pPetStatsMenu->Open() )
		{
			m_pPetStatsMenu->SetOpen( kFalse );
		}
		if( m_pInventoryMenu->Open() && m_pSkillMenu->Open() )
		{
			m_pSkillMenu->SetOpen( kFalse );
		}
		if( m_pInventoryMenu->Open() && m_pQuestMenu->Open() )
		{
			m_pQuestMenu->SetOpen( kFalse );
		}
		if( m_pInventoryMenu->Open() && m_pJournalMenu->Open() )
		{
			m_pJournalMenu->SetOpen( kFalse );
		}
	}
} // CGameUI::ToggleInventoryMenu()

void CGameUI::TogglePetInventoryMenu( CLevel& pLevel )					// level we are in
{
	if( !m_pBuySellMenu->Open() &&
		!m_pEnchantMenu->Open() )
	{
		if( m_pPetInventoryMenu->PetCharacter() != NULL )
		{
			m_pPetInventoryMenu->SetOpen( !m_pPetInventoryMenu->Open() );
			if( m_pPetInventoryMenu->Open() && m_pStatsMenu->Open() )
			{
				m_pStatsMenu->SetOpen( kFalse );
			}
			if( m_pPetInventoryMenu->Open() && m_pSpellMenu->Open() )
			{
				m_pSpellMenu->SetOpen( kFalse );
			}
		}

		if( m_pPetInventoryMenu->Open() )
		{
			// inventory tip
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipPetInventory ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipPetInventory ) );
				pLevel.Tips().SetTipSeen( KTipPetInventory, kTrue );
			}
		}
	}
	else if( ( m_pBuySellMenu->Open() ||
			   m_pEnchantMenu->Open() )&&
			 m_pPetSellInventoryMenu->Open() )
	{
		if( m_pDraggingItem != NULL )
		{
			ReturnItem( pLevel, m_pItemOwner );
		}
		m_pBuySellMenu->SetOpen( kFalse );
		m_pEnchantMenu->SetOpen( pLevel, kFalse );
		m_pPetSellInventoryMenu->SetOpen( kFalse );
		if( m_pPlayerCharacter->Pet( 0 ) != NULL &&
			m_pPlayerCharacter->Pet( 0 )->Alive() )
		{
			m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIInactive );
		}
	}
} // CGameUI::TogglePetInventoryMenu()

void CGameUI::ToggleSkillMenu( void )
{
	if( !m_pBuySellMenu->Open() )
	{
		m_pSkillMenu->SetOpen( !m_pSkillMenu->Open() );
		if( m_pSkillMenu->Open() && m_pInventoryMenu->Open() )
		{
			m_pInventoryMenu->SetOpen( kFalse );
		}
		if( m_pSkillMenu->Open() && m_pPetStatsMenu->Open() )
		{
			m_pPetStatsMenu->SetOpen( kFalse );
		}
		if( m_pSkillMenu->Open() && m_pQuestMenu->Open() )
		{
			m_pQuestMenu->SetOpen( kFalse );
		}
		if( m_pSkillMenu->Open() && m_pJournalMenu->Open() )
		{
			m_pJournalMenu->SetOpen( kFalse );
		}
	}
} // CGameUI::ToggleSkillMenu()

void CGameUI::ToggleQuestMenu( void )
{
	if( !m_pBuySellMenu->Open() )
	{
		m_pQuestMenu->SetOpen( !m_pQuestMenu->Open() );
		if( m_pQuestMenu->Open() && m_pInventoryMenu->Open() )
		{
			m_pInventoryMenu->SetOpen( kFalse );
		}
		if( m_pQuestMenu->Open() && m_pPetStatsMenu->Open() )
		{
			m_pPetStatsMenu->SetOpen( kFalse );
		}
		if( m_pQuestMenu->Open() && m_pSkillMenu->Open() )
		{
			m_pSkillMenu->SetOpen( kFalse );
		}
		if( m_pQuestMenu->Open() && m_pJournalMenu->Open() )
		{
			m_pJournalMenu->SetOpen( kFalse );
		}
	}
} // CGameUI::ToggleQuestMenu()

void CGameUI::ToggleJournalMenu( void )
{
	if( !m_pBuySellMenu->Open() )
	{
		m_pJournalMenu->SetOpen( !m_pJournalMenu->Open() );
		if( m_pJournalMenu->Open() && m_pInventoryMenu->Open() )
		{
			m_pInventoryMenu->SetOpen( kFalse );
		}
		if( m_pJournalMenu->Open() && m_pPetStatsMenu->Open() )
		{
			m_pPetStatsMenu->SetOpen( kFalse );
		}
		if( m_pJournalMenu->Open() && m_pSkillMenu->Open() )
		{
			m_pSkillMenu->SetOpen( kFalse );
		}
		if( m_pJournalMenu->Open() && m_pQuestMenu->Open() )
		{
			m_pQuestMenu->SetOpen( kFalse );
		}
	}
} // CGameUI::ToggleJournalMenu()

void CGameUI::CloseAll( CLevel& pLevel )	// level to drop items to
{
	CloseAllMenus( pLevel );
	if( m_pPlayerCharacter != NULL &&
		m_pPlayerCharacter->Alive() )
	{
		if( m_pPlayerCharacter->AIState() != KAIFish &&
			m_pPlayerCharacter->AIState() != KAIPullFish &&
			m_pPlayerCharacter->AIState() != KAICancelFish &&
			m_pPlayerCharacter->AIState() != KAIReceiveFish )
		{
			m_pPlayerCharacter->SetAIState( KAIInactive );
		}
	}
	if( m_pPlayerCharacter != NULL &&
		m_pPlayerCharacter->Pet( 0 ) != NULL && 
		m_pPlayerCharacter->Pet( 0 )->AvailableForCommand() )
	{
		m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIInactive );
	}
}  // CGameUI::CloseAll()

void CGameUI::CloseAllMenus( CLevel& pLevel )	// level to drop items to
{
	ReturnItem( pLevel,
				m_pItemOwner );

	m_ConsoleOpen = kFalse;
	m_pGameOptionsMenu->SetOpen( kFalse );
	m_pConfirmMenu->SetOpen( kFalse );
	m_pEnchantMenu->SetOpen( pLevel, kFalse );
	m_pHelpMenu->SetOpen( kFalse );
	m_pFateMenu->SetOpen( kFalse );
	m_pTipMenu->SetOpen( kFalse );
	m_pMessageMenu->SetOpen( kFalse );
	m_pInventoryMenu->SetOpen( kFalse );
	m_pPetStatsMenu->SetOpen( kFalse );
	m_pPetInventoryMenu->SetOpen( kFalse );
	m_pPetSellInventoryMenu->SetOpen( kFalse );
	m_pStatsMenu->SetOpen( kFalse );
	m_pBuySellMenu->SetOpen( kFalse );
	m_pSpellMenu->SetOpen( kFalse );
	m_pSkillMenu->SetOpen( kFalse );
	m_pQuestMenu->SetOpen( kFalse );
	m_pJournalMenu->SetOpen( kFalse );
}  // CGameUI::CloseAllMenus()

bool CGameUI::AnyMenusOpen( void )
{
	return( m_pGameOptionsMenu->Open() ||
		    m_pConfirmMenu->Open() ||
		    m_pEnchantMenu->Open() ||
		    m_pHelpMenu->Open() ||
		    m_pFateMenu->Open() ||
			m_pTipMenu->Open() ||
			m_pMessageMenu->Open() ||
			m_pInventoryMenu->Open() ||
			m_pPetStatsMenu->Open() ||
			m_pPetInventoryMenu->Open() ||
			m_pPetSellInventoryMenu->Open() ||
			m_pStatsMenu->Open() ||
			m_pBuySellMenu->Open() ||
			m_pSpellMenu->Open() ||
			m_pQuestMenu->Open() ||
			m_pJournalMenu->Open() ||
			m_pSkillMenu->Open() );

} // CGameUI::AnyMenusOpen()

void CGameUI::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );

	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	m_pMousePointer->SetPosition( (float32)MousePosition.x,
								  (float32)MousePosition.y );

	if( Paused() )
	{
		return;
	}

	if( m_pPlayerCharacter->Alive() )
	{
		// square brackets cycle spells
		if( m_KeyHandler.KeyPressed( 219 ) )
		{
			FSOUND_PlaySound( FSOUND_FREE, m_pSelectSpellSound );
			m_pPlayerCharacter->CycleSpell( -1 );
		}
		if( m_KeyHandler.KeyPressed( 221 ) )
		{
			FSOUND_PlaySound( FSOUND_FREE, m_pSelectSpellSound );
			m_pPlayerCharacter->CycleSpell( 1 );
		}
	}

	// if we press ~, we start console entry!
	if( m_KeyHandler.KeyPressed( 192 ) &&
		m_KeyHandler.KeyHeld( VK_CONTROL ) &&
		m_KeyHandler.KeyHeld( VK_SHIFT ) )
	{

		if(m_pPlayerCharacter->GetDisabledCheatAchievements())
		{
			m_ConsoleOpen = !m_ConsoleOpen;
			m_ConsoleText = "";
		}
		else
		{
			m_consoleConfirmation = kTrue;
		}
	}
	
	// check for hotkey assignment
	int32 ButtonPressed = m_pBottomMenu->UpdateButtonOver();

	if( ButtonPressed == KButtonSwapSpell ||
		( ButtonPressed >= KSwapSpellSelectButton &&
		  ButtonPressed <= KSwapSpellSelectButton + KMagicSpheres * KMaximumSpellsPerSphere ) )
	{
		int32 Sphere( 0 );
		int32 Slot( 0 );
		CSpellDescription* pDescription;
		if( ButtonPressed == KButtonSwapSpell )
		{
			pDescription = m_pPlayerCharacter->ActiveSpell();
		}
		else
		{
			int32 Slot( ButtonPressed - KSwapSpellSelectButton );
			while( Slot >= KMaximumSpellsPerSphere )
			{
				Slot -= KMaximumSpellsPerSphere;
				Sphere++;
			}
			pDescription = m_pPlayerCharacter->Spell( (EMagicSphere)Sphere, Slot );
		}

		if( pDescription != NULL )
		{
			for( uint32 i = 0; i < KMaxHotkeys; i++ )
			{
				if( m_KeyHandler.KeyPressed( VK_F1 + i ) )
				{
					SetHotkeyAssignment( i, pDescription->Name() );
					FSOUND_PlaySound( FSOUND_FREE, m_pSelectSpellSound );
				}
			}
		}
	}
	else
	{
		for( uint32 i = 0; i < KMaxHotkeys; i++ )
		{
			if( m_KeyHandler.KeyPressed( VK_F1 + i ) )
			{
				if( HotkeyAssignment( i ).length() > 0 )
				{
					m_pPlayerCharacter->SetActiveSpell( HotkeyAssignment( i ) );
				}
			}
		}
	}


	if( m_ConsoleOpen )
	{
		return;
	}

} // CGameUI::ProcessInput()

void CGameUI::Pause( CLevel& pLevel )					// level we are in
{
	if( !Paused() )
	{
		if( !BothCovered() )
		{
			CloseAll( pLevel );
			m_pGameOptionsMenu->SetOpen( kTrue );
			if( m_pDraggingItem != NULL )
			{
				ReturnItem( pLevel,
							m_pItemOwner );
			}
		}
	}
} // CGameUI::Pause()

void CGameUI::Update( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					  CGameClient& pClient,				// active game client
					  CLevel& pLevel,					// level we are in
					  HWND hWnd,						// hwnd of the active window
					  EViewType ViewType,				// type of view (split left, right or full?
					  float32 TimeElapsed )				// time elapsed, in seconds
{
	if( m_pPlayerCharacter == NULL
		|| !m_pPlayerCharacter->Alive() )
	{
		if( m_pDraggingItem != NULL )
		{
			ReturnItem( pLevel, m_pItemOwner );
		}
	}

	if( Targeting() )
	{
		if( m_MouseHandler.ButtonPressed( KButtonRight ) ||
			m_pPlayerCharacter == NULL ||
			!m_pPlayerCharacter->Alive() )
		{
			m_pItemInUse = NULL;
			m_pSpellInUse = NULL;
			m_MouseHandler.FlushAll();
		}
	}

	if( EnchantOpen() )
	{
		// close the enchant menu if any other menus open
		if( ( !InventoryOpen() &&
			  !PetSellInventoryOpen() ) ||
			PetStatsOpen() ||
			QuestsOpen() ||
			JournalOpen() ||
			SkillsOpen() ||
			PetInventoryOpen() ||
			BuySellOpen() ||
			StatsOpen() ||
			SpellsOpen() )
		{
			m_pEnchantMenu->SetOpen( pLevel, kFalse );
		}
	}

	// pressing enter will save it
	if( m_KeyHandler.KeyPressed( 13 ) &&
		m_ConsoleText.length() > 0 )
	{
		ExecuteConsole( pClient, pLevel );
		m_ConsoleOpen = kFalse;
		m_ConsoleText = "";
	}

	if( !m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
	{
		if( m_pPetStatsMenu->Open() )
		{
			TogglePetStatsMenu();
		}
		if( m_pPetInventoryMenu->Open() )
		{
			TogglePetInventoryMenu( pLevel );
		}
	}

	// H opens help menu
	if ( m_KeyHandler.KeyPressed( 72 ) &&
		!ConsoleOpen() )
	{

		if( !Paused() && !m_pHelpMenu->Open() )
		{
			CloseAll( pLevel );
			m_pHelpMenu->SetOpen( kTrue );
			m_pBottomMenu->SetHelpButtonVisible( kFalse );
		}
		else if( m_pHelpMenu->Open() )
		{
			m_pHelpMenu->SetOpen( kFalse );
		}
	}
	// spacebar closes out menus
	if ( !ConsoleOpen() &&
		 m_KeyHandler.KeyPressed( VK_SPACE ) )
	{
		if( !Paused() &&
			m_pPlayerCharacter->Alive() )
		{
			CloseAll( pLevel );
		}
	}

	// esc requests a shutdown
	if ( m_KeyHandler.KeyPressed( 27 ) )
	{
		if( !Paused() &&
			m_pPlayerCharacter->Alive() )
		{
			if( !AnyMenusOpen() )
			{
				CloseAll( pLevel );
				m_pGameOptionsMenu->SetOpen( kTrue );
				if( m_pDraggingItem != NULL )
				{
					ReturnItem( pLevel,
							m_pItemOwner );
				}
			}
			else
			{
				CloseAll( pLevel );
			}
		}
		else
		{
			if( m_pGameOptionsMenu->Open() )
			{
				m_pGameOptionsMenu->SetOpen( kFalse );
			}
		}
	}

	// update floating text items
	if( !Paused() &&
		!BothCovered() )
	{
		UpdateTextEvents( TimeElapsed );
	}
	m_ViewType = ViewType;

	if( m_pPlayerCharacter != NULL )
	{
		m_pBottomMenu->SetHP( m_pPlayerCharacter->MaxHP(),
							  m_pPlayerCharacter->HPFloat() );
		m_pBottomMenu->SetStamina( m_pPlayerCharacter->MaxStamina(),
								   m_pPlayerCharacter->StaminaFloat() );
		m_pBottomMenu->SetMana( m_pPlayerCharacter->MaxMana(),
								m_pPlayerCharacter->ManaFloat() );
		m_pBottomMenu->SetExperience( ExperienceGate( m_pPlayerCharacter->Level() ),
									  ExperienceGate( m_pPlayerCharacter->Level() - 1 ),
									  m_pPlayerCharacter->Experience() );

		if( m_pPlayerCharacter->HasPets() )
		{
			m_pBottomMenu->SetPetHP( m_pPlayerCharacter->Pet( 0 )->MaxHP(),
								     m_pPlayerCharacter->Pet( 0 )->HPFloat() );
		}
		else
		{
			m_pPetInventoryMenu->SetOpen( kFalse );
			m_pPetSellInventoryMenu->SetOpen( kFalse );
		}

		// update any special enchant menu text
		if( m_pEnchantMenu->Open() &&
			( ( m_pPlayerCharacter->Target() != NULL &&
			    m_pPlayerCharacter->Target()->MerchantType() == KMerchantEnchanter ) || 
				( m_pPlayerCharacter->Pet( 0 )->Target() != NULL &&
			     m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantEnchanter ) ) )
		{
			if( m_pEnchantMenu->Item() != NULL )
			{
				m_pEnchantMenu->SetText( pD3DDevice,
										 LoadLocalizedString1Variable(805, IntegerToString( m_pEnchantMenu->Item()->EnchantValue() ) ));
			}
			else
			{
				m_pEnchantMenu->SetText( pD3DDevice,
										 LoadLocalizedString(806));
			}
		}
		// speak to merchant....
		if( m_pPlayerCharacter->AIState() == KAISpeakMerchant &&
			m_pPlayerCharacter->Target() != NULL )
		{
			if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantStash ||
				!m_pPlayerCharacter->Target()->HasGreetings() )
			{
				m_pPlayerCharacter->SetAIState( KAIBuySell );
			}
			else
			{
				CloseAll( pLevel );
				m_pPlayerCharacter->SetAIState( KAISpeakMerchant );
				std::string Greeting = m_pPlayerCharacter->Target()->Greeting();

				if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantMinstrel )
				{
					if( m_pPlayerCharacter->FameRank() >= KFameLevels - 1 )
					{
						Greeting = LoadLocalizedString(807);
						OpenMessage( pD3DDevice, Greeting, LoadLocalizedString(185), KButtonYes );
						m_pPlayerCharacter->SetAIState( KAIInactive );

					}
					else
					{
						int32 Price = KFameGate[m_pPlayerCharacter->FameRank() + 1] * 5;
						float32 Fame = (float32)m_pPlayerCharacter->Fame();
						if( Fame < (float32)KFameGate[m_pPlayerCharacter->FameRank() - 1] )
						{
							Fame = (float32)KFameGate[m_pPlayerCharacter->FameRank() - 1] ;
						}
						float32 Delta = Fame - (float32)KFameGate[m_pPlayerCharacter->FameRank() - 1];
						Delta /= KFameGate[m_pPlayerCharacter->FameRank()] - KFameGate[m_pPlayerCharacter->FameRank() - 1];
						Price = Price - (int32)( Delta * (float32)Price );
						Greeting = Greeting + LoadLocalizedString3Variables(808, IntegerToString( Price ), KFameName[m_pPlayerCharacter->FameRank() + 1], IntegerToString( m_pPlayerCharacter->FameRank() + 1 ) );
						OpenConfirmation( pD3DDevice,
										Greeting,
										LoadLocalizedString(171),
										KButtonYes,
										LoadLocalizedString(172),
										KButtonNo );
					}
				}
				else
				{
					OpenConfirmation( pD3DDevice,
									Greeting,
									LoadLocalizedString(171),
									KButtonYes,
									LoadLocalizedString(172),
									KButtonNo );
				}
			}
		}
		else if( m_pPlayerCharacter->HasPets() &&
				m_pPlayerCharacter->Pet( 0 )->AIState() == KAISpeakMerchant &&
				m_pPlayerCharacter->Pet( 0 )->Target() != NULL )
		{
			if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantStash ||
				!m_pPlayerCharacter->Pet( 0 )->Target()->HasGreetings() )
			{
				m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIBuySell );
			}
			else
			{
				CloseAll( pLevel );
				m_pPlayerCharacter->Pet( 0 )->SetAIState( KAISpeakMerchant );
				std::string Greeting = m_pPlayerCharacter->Pet( 0 )->Target()->Greeting();

				if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantMinstrel )
				{
					if( m_pPlayerCharacter->FameRank() >= KFameLevels - 1 )
					{
						Greeting = LoadLocalizedString(809);
						OpenMessage( pD3DDevice, Greeting, LoadLocalizedString(185), KButtonYes );
						m_pPlayerCharacter->SetAIState( KAIInactive );

					}
					else
					{
						int32 Price = KFameGate[m_pPlayerCharacter->FameRank() + 1] * 5;
						float32 Fame = (float32)m_pPlayerCharacter->Fame();
						if( Fame < (float32)KFameGate[m_pPlayerCharacter->FameRank() - 1] )
						{
							Fame = (float32)KFameGate[m_pPlayerCharacter->FameRank() - 1] ;
						}
						float32 Delta = Fame - (float32)KFameGate[m_pPlayerCharacter->FameRank() - 1];
						Delta /= KFameGate[m_pPlayerCharacter->FameRank()] - KFameGate[m_pPlayerCharacter->FameRank() - 1];
						Price = Price - (int32)( Delta * (float32)Price );
						Greeting = Greeting + LoadLocalizedString3Variables(808, IntegerToString( Price ), KFameName[m_pPlayerCharacter->FameRank() + 1], IntegerToString( m_pPlayerCharacter->FameRank() + 1 ) );
						OpenConfirmation( pD3DDevice,
										Greeting,
										LoadLocalizedString(171),
										KButtonYes,
										LoadLocalizedString(172),
										KButtonNo );
					}
				}
				else
				{
					OpenConfirmation( pD3DDevice,
									Greeting,
									LoadLocalizedString(171),
									KButtonYes,
									LoadLocalizedString(172),
									KButtonNo );
				}
			}
		}
		else if( m_pPlayerCharacter->AIState() == KAIRetire &&
				 m_pPlayerCharacter->Target() != NULL )
		{
			CloseAll( pLevel );
			m_pPlayerCharacter->SetAIState( KAIRetire );
			OpenConfirmation( pD3DDevice,
							  m_pPlayerCharacter->Target()->Greeting(),
							  LoadLocalizedString(171),
							  KButtonYes,
							  LoadLocalizedString(172),
							  KButtonNo );
		}		
		if( m_pPlayerCharacter->AIState() == KAIBuySell &&
			m_pPlayerCharacter->Target() != NULL )
		{
			m_pSpellMenu->SetOpen( kFalse );
			m_pStatsMenu->SetOpen( kFalse );
			m_pPetStatsMenu->SetOpen( kFalse );
			m_pPetInventoryMenu->SetOpen( kFalse );
			m_pSkillMenu->SetOpen( kFalse );
			m_pQuestMenu->SetOpen( kFalse );
			m_pJournalMenu->SetOpen( kFalse );
			m_pInventoryMenu->SetOpen( kTrue );
			m_pBuySellMenu->SetMerchantCharacter( m_pPlayerCharacter->Target() );

			if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantBreakGems )
			{
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Position(),
							 LoadLocalizedString(180),
							 LoadLocalizedString(811),
							 KButtonEventBreakGems,
							 LoadLocalizedString(812),
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );
			}
			else if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantBreakWeapons )
			{
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Position(),
							 LoadLocalizedString(813),
							 LoadLocalizedString(814),
							 KButtonEventBreakWeapon,
							 LoadLocalizedString(812),
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );
			}
			else if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantEnchanter )
			{
				if( m_pEnchantMenu->Item() != NULL )
				{
					m_pEnchantMenu->SetText( pD3DDevice,
											LoadLocalizedString1Variable(805, IntegerToString( m_pEnchantMenu->Item()->EnchantValue() ) ));
				}
				else
				{
					m_pEnchantMenu->SetText( pD3DDevice,
											LoadLocalizedString(806) );
				}
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Position(),
							 LoadLocalizedString(806),
							 LoadLocalizedString(815),
							 KButtonEventEnchantItem,
							 LoadLocalizedString(812),
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );
			}			
			else if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantStash )
			{
				m_pBuySellMenu->SetOpen( kTrue );
				if( m_pSettings.GetSettings( KSetShowTips ) &&
					!pClient.Tips()->TipSeen( KTipStash ) )
				{
					pLevel.RequestTip( pClient.Tips()->ContextTip( KTipStash ) );
					pClient.Tips()->SetTipSeen( KTipStash, kTrue );
				}
			}
			else if( m_pPlayerCharacter->Target()->MerchantType() == KMerchantGambler )
			{
				m_pBuySellMenu->SetOpen( kTrue );
				if( m_pSettings.GetSettings( KSetShowTips ) &&
					!pClient.Tips()->TipSeen( KTipGamble ) )
				{
					pLevel.RequestTip( pClient.Tips()->ContextTip( KTipGamble ) );
					pClient.Tips()->SetTipSeen( KTipGamble, kTrue );
				}
			}
			else
			{
				m_pBuySellMenu->SetOpen( kTrue );
				if( m_pSettings.GetSettings( KSetShowTips ) &&
					!pClient.Tips()->TipSeen( KTipShopping ) )
				{
					pLevel.RequestTip( pClient.Tips()->ContextTip( KTipShopping ) );
					pClient.Tips()->SetTipSeen( KTipShopping, kTrue );
				}
			}
		}
		else if( m_pPlayerCharacter->HasPets() &&
				m_pPlayerCharacter->Pet( 0 )->AIState() == KAIBuySell &&
				m_pPlayerCharacter->Pet( 0 )->Target() != NULL )
		{
			m_pSpellMenu->SetOpen( kFalse );
			m_pStatsMenu->SetOpen( kFalse );
			m_pPetStatsMenu->SetOpen( kFalse );
			m_pPetInventoryMenu->SetOpen( kFalse );
			m_pPetSellInventoryMenu->SetOpen( kTrue );
			m_pSkillMenu->SetOpen( kFalse );
			m_pQuestMenu->SetOpen( kFalse );
			m_pJournalMenu->SetOpen( kFalse );
			m_pInventoryMenu->SetOpen( kFalse );
			m_pBuySellMenu->SetMerchantCharacter( m_pPlayerCharacter->Pet( 0 )->Target() );

			if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantBreakGems )
			{
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Pet( 0 )->Position(),
							 LoadLocalizedString(810),
							 LoadLocalizedString(811),
							 KButtonEventBreakGems,
							 LoadLocalizedString(812),
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );
				m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIInactive );
			}
			else if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantBreakWeapons )
			{
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Pet( 0 )->Position(),
							 LoadLocalizedString(813),
							 LoadLocalizedString(814),
							 KButtonEventBreakWeapon,
							 LoadLocalizedString(812),
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );
				m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIInactive );
			}
			else if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantEnchanter )
			{
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				if( m_pEnchantMenu->Item() != NULL )
				{
					m_pEnchantMenu->SetText( pD3DDevice,
											LoadLocalizedString1Variable(805, IntegerToString( m_pEnchantMenu->Item()->EnchantValue() ) ));
				}
				else
				{
					m_pEnchantMenu->SetText( pD3DDevice,
											LoadLocalizedString(806) );
				}
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Pet( 0 )->Position(),
							 LoadLocalizedString(806),
							 LoadLocalizedString(815),
							 KButtonEventEnchantItem,
							 LoadLocalizedString(812),
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );
				m_pPlayerCharacter->Pet( 0 )->SetAIState( KAIInactive );
				/*
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				OpenEnchant( pD3DDevice,
							 m_pPlayerCharacter->Pet( 0 )->Position(),
							 "Place \bitem\b here to \benchant it\b",
							 "Enchant",
							 KButtonEventEnchantItem,
							 "Cancel",
							 KButtonEventBreakCancel );
				m_pPlayerCharacter->SetAIState( KAIInEvent );*/
			}
			else if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantStash )
			{
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				m_pBuySellMenu->SetOpen( kTrue );
				if( m_pSettings.GetSettings( KSetShowTips ) &&
					!pClient.Tips()->TipSeen( KTipStash ) )
				{
					pLevel.RequestTip( pClient.Tips()->ContextTip( KTipStash ) );
					pClient.Tips()->SetTipSeen( KTipStash, kTrue );
				}
			}
			else if( m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() == KMerchantGambler )
			{
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				m_pBuySellMenu->SetOpen( kTrue );
				if( m_pSettings.GetSettings( KSetShowTips ) &&
					!pClient.Tips()->TipSeen( KTipGamble ) )
				{
					pLevel.RequestTip( pClient.Tips()->ContextTip( KTipGamble ) );
					pClient.Tips()->SetTipSeen( KTipGamble, kTrue );
				}
			}
			else
			{
				m_pPlayerCharacter->SetTarget( m_pPlayerCharacter->Pet( 0 )->Target() );
				m_pBuySellMenu->SetOpen( kTrue );
				if( m_pSettings.GetSettings( KSetShowTips ) &&
					!pClient.Tips()->TipSeen( KTipShopping ) )
				{
					pLevel.RequestTip( pClient.Tips()->ContextTip( KTipShopping ) );
					pClient.Tips()->SetTipSeen( KTipShopping, kTrue );
				}
			}
		}
		if( m_pPlayerCharacter->AIState() == KAICompleteRetire &&
			m_pPlayerCharacter->Target() != NULL )
		{
			m_pSpellMenu->SetOpen( kFalse );
			m_pStatsMenu->SetOpen( kFalse );
			m_pPetStatsMenu->SetOpen( kFalse );
			m_pPetInventoryMenu->SetOpen( kFalse );
			m_pSkillMenu->SetOpen( kFalse );
			m_pQuestMenu->SetOpen( kFalse );
			m_pJournalMenu->SetOpen( kFalse );
			m_pInventoryMenu->SetOpen( kTrue );

			OpenEnchant( pD3DDevice,
						 m_pPlayerCharacter->Position(),
						 LoadLocalizedString(816),
						 LoadLocalizedString(817),
						 KButtonEventHeirloom,
						 LoadLocalizedString(812),
						 KButtonEventBreakCancel );
			m_pPlayerCharacter->SetAIState( KAIInEvent );
		}
	}

	m_pGameOptionsMenu->Update( hWnd,
							    TimeElapsed );

	m_pConfirmMenu->Update( hWnd,
						    TimeElapsed );

	m_pEnchantMenu->Update( hWnd,
						    TimeElapsed );

	m_pTipMenu->Update( hWnd,
						*this,
					    TimeElapsed );

	m_pMessageMenu->Update( hWnd,
							*this,
						    TimeElapsed );

	m_pHelpMenu->Update( hWnd,
					     TimeElapsed );

	m_pFateMenu->Update( hWnd,
						 pLevel,
					     TimeElapsed );

	// we don't update any other menus if game options is open
	if( !Paused() )
	{
		VerifyHotkeyAssignments();

		m_pBottomMenu->Update( hWnd,
							TimeElapsed );

		m_pInventoryMenu->Update( hWnd,
								TimeElapsed );

		m_pSkillMenu->Update( hWnd,
							TimeElapsed );
		if( m_pSkillMenu->MessagePending() ) //marsh - tell the player they are in demo mode
		{			
			OpenMessage( pD3DDevice, m_pSkillMenu->PendingMessageText(), LoadLocalizedString(185), KButtonYes );
		}

		m_pQuestMenu->Update( hWnd,
							TimeElapsed );

		m_pJournalMenu->Update( hWnd,
								TimeElapsed );

		m_pPetInventoryMenu->Update( hWnd,
									TimeElapsed );

		m_pPetSellInventoryMenu->Update( hWnd,
										TimeElapsed );

		m_pStatsMenu->Update( hWnd,
							TimeElapsed );
		if( m_pStatsMenu->MessagePending() ) //marsh - tell the player they are in demo mode
		{			
			OpenMessage( pD3DDevice, m_pStatsMenu->PendingMessageText(), LoadLocalizedString(185), KButtonYes );
		}

		m_pSpellMenu->Update( hWnd,
							TimeElapsed );

		m_pPetStatsMenu->Update( hWnd,
								TimeElapsed );

		m_pBuySellMenu->Update( hWnd,
								TimeElapsed );
	}

	ProcessInput( hWnd );

	m_pBottomMenu->SetBuyMenuOpen( m_pBuySellMenu->Open() );
	m_pBottomMenu->SetStatsMenuOpen( m_pStatsMenu->Open() );
	m_pBottomMenu->SetSkillMenuOpen( m_pSkillMenu->Open() );
	m_pBottomMenu->SetInventoryMenuOpen( m_pInventoryMenu->Open() );
	m_pBottomMenu->SetPetInventoryMenuOpen( m_pPetInventoryMenu->Open() );

	if( !Paused() )
	{
		if( !m_ConsoleOpen )
		{

			// pressing 'i' toggles inventory
			if( m_KeyHandler.KeyPressed( 73 ) )
			{
				ToggleInventoryMenu( pLevel );
			}
			// toggle quests 'q'
			if ( m_KeyHandler.KeyPressed( 81 ) )
			{
				ToggleQuestMenu();
			}
			// toggle quests 'j'
			if ( m_KeyHandler.KeyPressed( 74 ) )
			{
				ToggleJournalMenu();
			}
			// pressing 'v' opens Save menu
			if( m_KeyHandler.KeyPressed( 86 ) )
			{
				if( !Paused() &&
					m_pPlayerCharacter->Alive() )
				{
					CloseAll( pLevel );
					m_pGameOptionsMenu->SetOpen( kTrue );
					if( m_pDraggingItem != NULL )
					{
						ReturnItem( pLevel,
								m_pItemOwner );
					}
				}
			}
			// pressing 'p' toggles pet inventory
			if( m_KeyHandler.KeyPressed( 80 ) &&
				m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
			{
				TogglePetInventoryMenu( pLevel );
			}
			// pressing 'c' toggles stats
			if( m_KeyHandler.KeyPressed( 67 ) )
			{
				ToggleStatsMenu( pLevel );
			}
			// pressing 'k' toggles stats
			if( m_KeyHandler.KeyPressed( 75 ) )
			{
				ToggleSkillMenu();
			}
			// pressing 'b' toggles spellbook
			if( m_KeyHandler.KeyPressed( 66 ) )
			{
				ToggleSpellMenu();
			}
			// pressing 't' toggles pet stats
			if( m_KeyHandler.KeyPressed( 84 ) &&
				m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
			{
				TogglePetStatsMenu();
			}
			// pressing 'm' or 'TAB' toggles minimap
			if( m_KeyHandler.KeyPressed( 9 ) ||
				m_KeyHandler.KeyPressed( 77 ) )
			{
				ToggleMiniMap();
				//m_MapVisible = !m_MapVisible;
			}
		}

		bool ShowItems = AlwaysShowingItems();
		if( m_KeyHandler.KeyHeld( 18 ) )
		{
			ShowItems = !ShowItems;
		}
		// ALT shows all items on the ground
		if( !Paused() &&
			ShowItems )
		{
			SetDisplayAllItems( kTrue );
		}
		else
		{
			SetDisplayAllItems( kFalse );
		}



		// only use when alive
		if( m_pPlayerCharacter->Alive() )
		{
			// check the quickslots for usage
			for( uint32 i = 0; i < KQuickSlots - KEquipmentSlots ; i++ )
			{
				uint32 ActualSlot( 0 );
				if( !ConsoleOpen() &&
					m_KeyHandler.KeyPressed( i + 49 ) )
				{
					CItem* pItem = m_pPlayerCharacter->Inventory()->GetItemFromSlot( i + KEquipmentSlots, ActualSlot );

					// try to use it
					if( pItem != NULL &&
						pItem->Useable() )
					{
						if( m_KeyHandler.KeyHeld( VK_SHIFT ) &&
							m_pPlayerCharacter->Pets() > 0 &&
							m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
						{
							PerformItemUse( pLevel,
											pItem,
											m_pPlayerCharacter,
											m_pPlayerCharacter,
											m_pPlayerCharacter->Pet( 0 ) );
						}
						else
						{
							PerformItemUse( pLevel,
											pItem,
											m_pPlayerCharacter,
											m_pPlayerCharacter,
											m_pPlayerCharacter );
						}
					}
				}
			}
		}

		UpdateButtons( TimeElapsed );

	}
	else
	{
		SetDisplayAllItems( kFalse );
	}

	// left mouse button first
	int32 ButtonPressed = m_pGameOptionsMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															     m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
	if( ButtonPressed == -1 )
	{
		ButtonPressed = m_pConfirmMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
														   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		if( ButtonPressed != -1 )
		{
			if( ButtonPressed == KButtonCancelQuestYes ||
				ButtonPressed == KButtonCancelQuestNo )
			{
				m_pQuestMenu->ProcessConfirmation( ButtonPressed );
				m_pConfirmMenu->SetOpen( kFalse );
				FlushAll();
			}
			m_ConfirmMenuResult = ButtonPressed;
		}
	}
	if( ButtonPressed == -1 )
	{
		ButtonPressed = m_pEnchantMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
														   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		if( ButtonPressed != -1 )
		{
			m_EnchantMenuResult = ButtonPressed;
		}
	}
	if( ButtonPressed == -1 )
	{
		ButtonPressed = m_pMessageMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
														   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
	}

	if( ButtonPressed == -1 )
	{
		ButtonPressed = m_pHelpMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
													    m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
	}

	if( ButtonPressed == -1 )
	{
		ButtonPressed = m_pFateMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
													    m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
	}

	if( !Paused() )
	{
		if( ButtonPressed == -1 )
		{
			ButtonPressed = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
											   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pBottomMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
																m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pSkillMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pQuestMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pJournalMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
																	m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetSellInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
																		m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pStatsMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pSpellMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetStatsMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
																m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pBuySellMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
															m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
	}

	switch( ButtonPressed )
	{
	case KButtonCloseHelp :
		m_pHelpMenu->SetOpen( kFalse );
		break;
	case KButtonOpenHelp :
		CloseAll( pLevel );
		m_pHelpMenu->SetOpen( kTrue );
		m_pBottomMenu->SetHelpButtonVisible( kFalse );
		break;
	case KButtonFate :
		m_pMessageMenu->SetOpen( kFalse );
		pClient.StopMusic();
		OpenFate( pD3DDevice, pLevel );
		break;
	case KButtonSaveAndExit :
		// make sure any dying characters finish doing so
		if( pClient.Level() != NULL )
		{
			pClient.Level()->UpdateCharacterDeaths();
		}
		pClient.SaveCharacter();
		m_pGameStateManager.SetTargetMenu( KMainMenu );
		CloseAll( pLevel );
		break;
	case KButtonSaveAndContinue :
		//pClient.SaveCharacter();
		m_pGameOptionsMenu->SetOpen( kFalse );
		break;
	case KButtonResume :
		m_pGameOptionsMenu->SetOpen( kFalse );
		break;
	case KButtonCancelFishing :
		m_pPlayerCharacter->CancelFish();
		break;
	case KButtonSetHook :
		m_pPlayerCharacter->PullFish();
		break;
	case KBottomButtonInventory :
		ToggleInventoryMenu( pLevel );
		break;
	case KButtonPetInventory :
		if( m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() &&
			m_pDraggingItem == NULL )
		{
			TogglePetInventoryMenu( pLevel );
		}
		break;
	case KBottomButtonStats :
		ToggleStatsMenu( pLevel );
		break;
	case KBottomButtonMagic :
		ToggleSpellMenu();
		break;
	case KButtonPetStats :
		if( m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
		{
			TogglePetStatsMenu();
		}
		break;
	case KBottomButtonSkills :
		ToggleSkillMenu();
		break;
	case KBottomButtonQuests :
		ToggleQuestMenu();
		break;
	case KBottomButtonJournal :
		ToggleJournalMenu();
		break;
	case KBottomButtonLevelUp :
		if( !m_pBuySellMenu->Open() )
		{
			if( !m_pStatsMenu->Open() )
			{
				ToggleStatsMenu( pLevel );
			}
			if( !m_pSkillMenu->Open() )
			{
				ToggleSkillMenu();
			}
			m_MouseHandler.FlushAll();
		}
		break;
	case KButtonPetTown :
		if( m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
		{
			m_pPlayerCharacter->SetAIState( KAISendPetToTown );
		}
		break;
	case KBottomButtonMap :
		m_MapVisible = !m_MapVisible;
		break;
	case KBottomButtonOptions :
		if( !Paused() )
		{
			CloseAll( pLevel );
			if( m_pDraggingItem != NULL )
			{
				ReturnItem( pLevel,
						m_pItemOwner );
			}
			m_pGameOptionsMenu->SetOpen( kTrue );
		}
		break;
	case KButtonToggleRun :
		SetAlwaysRunning( !AlwaysRunning() );
		break;
	case KButtonToggleShowItems :
		SetAlwaysShowingItems( !AlwaysShowingItems() );
		break;
	}


	CCharacter* pInventoryCharacter = m_pPlayerCharacter;
	if( ButtonPressed >= KBuySellButton )
	{
		if( PetSellInventoryOpen() )
		{
			pInventoryCharacter = m_pPlayerCharacter->Pet( 0 )->Target();
		}
		else
		{
			pInventoryCharacter = m_pPlayerCharacter->Target();
		}
	}
	else if( ButtonPressed >= KPetInventoryButton )
	{
		pInventoryCharacter = m_pPlayerCharacter->Pet( 0 );
	}
	// try to use an item on a pet, if requested
	if( m_pPlayerCharacter->HasPets() &&
		ButtonPressed == KButtonPetUse &&
		m_pDraggingItem != NULL &&
		m_pDraggingItem->Useable() &&
		( m_pItemOwner == m_pPlayerCharacter ||
		  ( m_pPlayerCharacter->Pets() > 0 && m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) ) ) )
	{
		UseItemOnPet( pLevel );
	}
	else if( ButtonPressed >= KQuestButton )
	{
	}
	else if( ButtonPressed == KEnchantItemButton )
	{
		if( m_pDraggingItem != NULL )
		{
			CItem* pItem = m_pEnchantMenu->Item();
			m_pDraggingItem->PlayRemoveSound();
			m_pEnchantMenu->SetItem( m_pItemOwner, m_pDraggingItem );
			m_pDraggingItem = pItem;
			if( m_pDraggingItem != NULL )
			{
				m_pItemOwner = m_pPlayerCharacter;
			}
		}
		else
		{
			// scoot it back
			if( m_KeyHandler.KeyHeld( VK_SHIFT ) &&
				m_pEnchantMenu->Item() != NULL )
			{
				m_pEnchantMenu->Item()->SetPosition( pLevel, m_pPlayerCharacter->Position() );
				m_pEnchantMenu->Item()->PlayRemoveSound();
				m_pEnchantMenu->Item()->PlayTakeSound();

				EnchantMenuOwner()->GiveItem( pLevel,
											   m_pEnchantMenu->Item() );
				SetEnchantMenuItem( NULL, NULL );

			}
			else
			{
				m_pDraggingItem = m_pEnchantMenu->Item();
				if( m_pDraggingItem != NULL )
				{
					m_pDraggingItem->PlayTakeSound();
				}
				m_pEnchantMenu->SetItem( NULL, NULL );
				if( m_pDraggingItem != NULL )
				{
					m_pItemOwner = m_pPlayerCharacter;
				}
			}
		}
	}
	// if we clicked on a spell -
	else if( ButtonPressed >= KAttackSpellButton )
	{
		if( m_pDraggingItem != NULL )
		{
			PlaceItem( pLevel, m_pPlayerCharacter );
		}
		else
		{
			EMagicSphere Sphere = KMagicAttack;
			bool SelectedCast = kFalse;
			int32 SlotIndex = ButtonPressed;
			if( SlotIndex >= KActiveSpellButton )
			{
				SelectedCast = kTrue;
			}
			else if( SlotIndex >= KCharmSpellButton )
			{
				Sphere = KMagicCharm;
				SlotIndex -= KCharmSpellButton;
			}
			else if( SlotIndex >= KDefenseSpellButton )
			{
				Sphere = KMagicDefense;
				SlotIndex -= KDefenseSpellButton;
			}
			else
			{
				SlotIndex -= KAttackSpellButton;
			}
			if( SelectedCast )
			{
				m_pTargetCharacter = NULL;
				CastActiveSpell( pLevel, pClient );
			}
			else
			{
				if( m_pPlayerCharacter->Spell( Sphere, SlotIndex ) != NULL )
				{
					if( m_KeyHandler.KeyHeld( VK_CONTROL ) )
					{
						m_pPlayerCharacter->UnLearnSpell( Sphere, SlotIndex );
						FSOUND_PlaySound( FSOUND_FREE, m_pSelectSpellSound );
					}
					else
					{
						CSpellDescription* pSpell = m_pPlayerCharacter->Spell( Sphere, SlotIndex );
						bool RequirementsMet( kTrue );
						if( pSpell->RequirementsMet( *m_pPlayerCharacter ) )
						{
							m_pPlayerCharacter->SetActiveSpell( pSpell );
							FSOUND_PlaySound( FSOUND_FREE, m_pSelectSpellSound );
						}
						else
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
						}
					}
				}
			}
		}
	}
	// if we clicked on an inventory box, try to pick the item up
	else if( ButtonPressed >= KInventoryButton &&
			 m_pDraggingItem == NULL )
	{
		if( Targeting() )
		{
			if( ( m_pItemInUse != NULL &&
				  m_pItemInUse->Target() == KTargetItem ) ||
				( m_pSpellInUse != NULL &&
				  m_pSpellInUse->Target() == KTargetItem ) )
			{
				UseOnSlot( pLevel, pInventoryCharacter, ButtonPressed );
			}
			else
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );

				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
			}
		}
		else
		{
			TakeItemFromSlot( pLevel, pInventoryCharacter, ButtonPressed );
		}
	}
	// if we are dragging something, and
	// let go of the mouse, drop it, equip it, or put it back
	else if( m_pDraggingItem != NULL &&
			 ( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
			   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) ) )
	{
		if( !MouseOverMenus() )
		{
			DropItem( pLevel,
					  pInventoryCharacter );
		}
		else
		{
			PlaceItem( pLevel,
					   pInventoryCharacter );
		}
		if( m_pPlayerCharacter->Fishing() &&
			!m_pPlayerCharacter->Inventory()->HasItems( KItemFishingPole, 1 ) )
		{
			m_pPlayerCharacter->CancelFish();
		}
	}

	// check the right mouse button and double-clicks for usage

	if( !Paused() )
	{
		ButtonPressed = m_pBottomMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonRight ) );
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonRight ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonRight ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetSellInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonRight ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pSpellMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonRight ) );
		}
		/*if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pBottomMenu->UpdateButtonPress( m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pPetSellInventoryMenu->UpdateButtonPress( m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
		if( ButtonPressed == -1 )
		{
			ButtonPressed = m_pBuySellMenu->UpdateButtonPress( m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
		}
*/
		if( ButtonPressed >= KAttackSpellButton )
		{
			pInventoryCharacter = m_pPlayerCharacter;
		}
		if( ButtonPressed >= KBuySellButton )
		{
			if( PetSellInventoryOpen() )
			{
				pInventoryCharacter = m_pPlayerCharacter->Pet( 0 )->Target();
			}
			else
			{
				pInventoryCharacter = m_pPlayerCharacter->Target();
			}
		}
		else if( ButtonPressed >= KPetInventoryButton )
		{
			pInventoryCharacter = m_pPlayerCharacter->Pet( 0 );
		}


		// if we right-click in the world, or over a menu for item-targeted spells , then we cast a spell
		
		if( m_MouseHandler.ButtonPressed( KButtonRight ) &&
			!Targeting() &&
			( ButtonPressed == -1 ||
			!GetItemFromSlot( ButtonPressed ) ) )
		{
			if( m_pPlayerCharacter->ActiveSpell() != NULL )
			{
				if( !MouseOverMenus() ||
					m_pPlayerCharacter->ActiveSpell()->Target() == KTargetItem )
				{
					CastActiveSpell( pLevel, pClient );
				}
			}
		}

		// if we clicked on an inventory box, try to use the item - doesn't work in merchant inventory
		if( ButtonPressed >= KAttackSpellButton )
		{
			if( m_pDraggingItem == NULL )
			{
				EMagicSphere Sphere = KMagicAttack;
				bool SelectedCast = kFalse;
				int32 SlotIndex = ButtonPressed;
				if( SlotIndex >= KActiveSpellButton )
				{
					SelectedCast = kTrue;
				}
				else if( SlotIndex >= KCharmSpellButton )
				{
					Sphere = KMagicCharm;
					SlotIndex -= KCharmSpellButton;
				}
				else if( SlotIndex >= KDefenseSpellButton )
				{
					Sphere = KMagicDefense;
					SlotIndex -= KDefenseSpellButton;
				}
				else
				{
					SlotIndex -= KAttackSpellButton;
				}
				if( SelectedCast )
				{
					m_pTargetCharacter = NULL;
					CastActiveSpell( pLevel, pClient );
				}
				else
				{
					if( m_pPlayerCharacter->Spell( Sphere, SlotIndex ) != NULL )
					{
						m_pTargetCharacter = NULL;
						CastSpell( pLevel,
								   m_pPlayerCharacter->Spell( Sphere, SlotIndex ),
								   pClient );
					}
				}
			}
		}
		else if( ButtonPressed >= KInventoryButton && 
				ButtonPressed < KBuySellButton &&
				m_pDraggingItem == NULL &&
				!Targeting() )
		{
			UseItem( pLevel, ButtonPressed );
		}

		m_pMouseOverItem = NULL;
		m_pMouseOverSpell = NULL;
		// if the mouse is over an item, and the button is not clicked, and we aren't dragging,
		// show a rollout of the item's description
		if( !m_MouseHandler.ButtonHeld( KButtonLeft ) &&
			m_pDraggingItem == NULL )
		{
			UpdateItemDescription();
		}


	}
	else
	{
		m_pMouseOverItem = NULL;
		m_pMouseOverSpell = NULL;
	}

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

	m_LeftCovered = kFalse;
	m_RightCovered = kFalse;
	if( InventoryOpen() ||
		PetSellInventoryOpen() ||
		PetStatsOpen() ||
		FateOpen() ||
		QuestsOpen() ||
		JournalOpen() ||
		SkillsOpen() )
	{
		m_RightCovered = kTrue;
	}
	if( PetInventoryOpen() ||
		BuySellOpen() ||
		FateOpen() ||
		StatsOpen() ||
		SpellsOpen() )
	{
		m_LeftCovered = kTrue;
	}

	//Disabling achivements when cheating
	if(m_consoleConfirmation)
	{
		OpenConfirmation( pD3DDevice,
										LoadLocalizedString(1500),
										LoadLocalizedString(171),
										KButtonYes,
										LoadLocalizedString(172),
										KButtonNo );

		m_consoleConfirmation = kFalse;
		m_shouldOpenConsole = kTrue;
	}

	if(m_shouldOpenConsole)
	{
		if(ConfirmMenuResult() == KButtonYes)
		{
			#ifndef _DEBUG
			if( m_pPlayerCharacter )
			{
				m_pPlayerCharacter->SetDisabledCheatAchievements(kTrue);
			}
			#endif
			m_ConsoleOpen = !m_ConsoleOpen;
			m_ConsoleText = "";
			CloseConfirmation();
			m_shouldOpenConsole = kFalse;
		}
		else if(ConfirmMenuResult() == KButtonNo)
		{
			CloseConfirmation();
			m_shouldOpenConsole = kFalse;
		}
	}

} // CGameUI::Update()

void CGameUI::ToggleMiniMap( void )
{
	if( !m_MapVisible )
	{
		m_MapVisible = kTrue;
	}
	else
	{
		int32 Dolly = (int32)m_MiniMapDolly;
		Dolly++;
		m_MiniMapDolly = (EMiniMapDolly)Dolly;
		if( m_MiniMapDolly >= KMiniMapHeights )
		{
			m_MiniMapDolly = KMapNormal;
			m_MapVisible = kFalse;
		}
	}
} // CGameUI::ToggleMiniMap()

void CGameUI::CastActiveSpell( CLevel& pLevel,					// level to drop to
  							   CGameClient& pClient )			// game client

{
	if( m_pPlayerCharacter == NULL ||
		!m_pPlayerCharacter->Alive() )
	{
		return;
	}
	CastSpell( pLevel, m_pPlayerCharacter->ActiveSpell(), pClient );
} // CGameUI::CastActiveSpell()

void CGameUI::CastSpell( CLevel& pLevel,						// level to drop to
						 CSpellDescription* pSpell,				// spell to cast
						 CGameClient& pClient )					// game client
{
	if( m_pPlayerCharacter == NULL ||
		!m_pPlayerCharacter->Alive() )
	{
		return;
	}
	if( m_pPlayerCharacter->Fishing() )
	{
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpSpellMiscast );
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		return;
	}
	if( pSpell != NULL )
	{
		if( pSpell->ManaCost() > m_pPlayerCharacter->Mana() )
		{
	
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpManaLow );
			FSOUND_PlaySound( FSOUND_FREE, m_pLowManaSound );
			return;
		}
		if( m_pPlayerCharacter->PerformingCast() )
		{
			FSOUND_PlaySound( FSOUND_FREE, m_pLowManaSound );
			return;
		}
		if( pSpell->Target() == KTargetCharacter )
		{
			if( m_pTargetCharacter == NULL )
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpSpellMiscast );
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
				pLevel.AddEventText( m_pPlayerCharacter,
									 m_pPlayerCharacter,
									 m_pPlayerCharacter->Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(818),
									 .75f,
									 kFalse,
									 .6f );

				return;
			}
			if( m_pTargetCharacter->Alignment() != pSpell->TargetAlignment() &&
				pSpell->TargetAlignment() != KAllAlignments )
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpSpellMiscast );
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
				pLevel.AddEventText( m_pPlayerCharacter,
									 m_pPlayerCharacter,
									 m_pPlayerCharacter->Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(819),
									 .75f,
									 kFalse,
									 .6f );
				return;
			}
		}
		
		// if it is a targeted spell, we need to queue it up for usage on an item
		if( pSpell->Target() == KTargetItem )
		{
			m_pSpellInUse = pSpell;
			m_pOwnerOfSpellInUse = m_pPlayerCharacter;
			m_pItemInUse = NULL;
		}
		else
		{
			D3DXVECTOR3 Position;
			pClient.FindWorldLocation( Position );
			m_pPlayerCharacter->CastSpell( pLevel,
										   pSpell,
										   NULL,
										   m_pTargetCharacter,
										   Position );
		}
	}
} // CGameUI::CastSpell()

void CGameUI::UpdateItemDescription( void )
{
	CCharacter* pInventoryCharacter = m_pPlayerCharacter;
	int32 SlotIndex = m_pBottomMenu->UpdateButtonOver();
	if( SlotIndex == -1 && InventoryOpen() )
	{
		SlotIndex = m_pInventoryMenu->UpdateButtonOver();
	}
	if( SlotIndex == -1 && PetInventoryOpen() )
	{
		SlotIndex = m_pPetInventoryMenu->UpdateButtonOver();
	}
	if( SlotIndex == -1 && PetSellInventoryOpen() )
	{
		SlotIndex = m_pPetSellInventoryMenu->UpdateButtonOver();
	}
	if( SlotIndex == -1 && BuySellOpen() )
	{
		SlotIndex = m_pBuySellMenu->UpdateButtonOver();
	}
	if( SlotIndex == -1 && SpellsOpen() )
	{
		SlotIndex = m_pSpellMenu->UpdateButtonOver();
	}
	if( SlotIndex == -1 && EnchantOpen() )
	{
		SlotIndex = m_pEnchantMenu->UpdateButtonOver();
	}

	if( SlotIndex >= KAttackSpellButton )
	{
		pInventoryCharacter = m_pPlayerCharacter;
	}
	else if( SlotIndex >= KBuySellButton )
	{
		if( PetSellInventoryOpen() )
		{
			pInventoryCharacter = m_pPlayerCharacter->Pet( 0 )->Target();
		}
		else
		{
			pInventoryCharacter = m_pPlayerCharacter->Target();
		}
	}
	else if( SlotIndex >= KPetInventoryButton )
	{
		pInventoryCharacter = m_pPlayerCharacter->Pet( 0 );
	}

	// if we're mousing over a spell
	if( SlotIndex >= KAttackSpellButton )
	{
		EMagicSphere Sphere = KMagicAttack;
		if( SlotIndex >= KActiveSpellButton )
		{
			m_pMouseOverSpell = pInventoryCharacter->ActiveSpell();
			return;
		}
		if( SlotIndex >= KCharmSpellButton )
		{
			Sphere = KMagicCharm;
			SlotIndex -= KCharmSpellButton;
		}
		else if( SlotIndex >= KDefenseSpellButton )
		{
			Sphere = KMagicDefense;
			SlotIndex -= KDefenseSpellButton;
		}
		else
		{
			SlotIndex -= KAttackSpellButton;
		}

		assert( SlotIndex >= 0 && SlotIndex < KMaximumSpellsPerSphere );
		m_pMouseOverSpell = pInventoryCharacter->Spell( Sphere, SlotIndex );
	} // if we're mousing over an inventory item
	else if( SlotIndex >= KInventoryButton )
	{
		if( SlotIndex >= KBuySellButton )
		{
			SlotIndex -= KBuySellButton;
		}
		else if( SlotIndex >= KPetInventoryButton )
		{
			SlotIndex -= KPetInventoryButton;
		}
		else
		{
			SlotIndex -= KInventoryButton;
		}
		m_pItemOwner = pInventoryCharacter;
		m_pMouseOverItem = pInventoryCharacter->Inventory()->GetItemFromSlot( SlotIndex );

		// if mousing over the shield panel, check to see if a dual-wield ( or bow ) weapon is equipped
		if( m_pMouseOverItem == NULL &&
			SlotIndex == KSlotLeftArm )
		{
			SlotIndex = KSlotLeftHand;
			m_pMouseOverItem = pInventoryCharacter->Inventory()->GetItemFromSlot( SlotIndex );
		}
	}
	else if( SlotIndex == KEnchantItemButton )
	{
		m_pMouseOverItem = m_pEnchantMenu->Item();
	}
} // CGameUI::UpdateItemDescription()

void CGameUI::DropItem( CLevel& pLevel,						// level to drop to
						CCharacter* pInventoryCharacter )	// character whose inventory this is from
{
	// can't drop items from a merchant
	if( m_pItemOwner != m_pPlayerCharacter->Target() ||
		( m_pItemOwner != NULL && m_pItemOwner->MerchantType() == KMerchantStash ) )
	{
		pLevel.AddItem( m_pDraggingItem, pInventoryCharacter->Position() );
		m_pDraggingItem->Drop( pLevel );
		m_pDraggingItem = NULL;
	}
	else
	{
		ReturnItem( pLevel, m_pItemOwner );
	}
} // CGameUI::DropItem()

CItem* CGameUI::GetItemFromSlot( uint32 ButtonPressed )	// index of button pressed to pick up 
{
	if( !m_pPlayerCharacter->Alive() )
	{
		return NULL;
	}
	uint32 ActualSlot( 0 );
	int32 InventorySlot = ButtonPressed - KInventoryButton;

	CCharacter* pInventoryCharacter = m_pPlayerCharacter;
	if( ButtonPressed >= KPetInventoryButton )
	{
		InventorySlot = ButtonPressed - KPetInventoryButton;
		pInventoryCharacter = m_pPlayerCharacter->Pet( 0 );
	}
	return pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot, ActualSlot );
}

void CGameUI::UseItem( CLevel& pLevel,						// level to drop to
					   uint32 ButtonPressed )				// index of button pressed to pick up 
{
	if( !m_pPlayerCharacter->Alive() )
	{
		return;
	}
	uint32 ActualSlot( 0 );
	int32 InventorySlot = ButtonPressed - KInventoryButton;

	CCharacter* pInventoryCharacter = m_pPlayerCharacter;
	if( ButtonPressed >= KPetInventoryButton )
	{
		InventorySlot = ButtonPressed - KPetInventoryButton;
		pInventoryCharacter = m_pPlayerCharacter->Pet( 0 );
	}
	CItem* pItem = pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot, ActualSlot );

	// try to use it
	if( pItem != NULL &&
		pItem->Useable() )
	{
		if( m_KeyHandler.KeyHeld( VK_SHIFT ) &&
			m_pPlayerCharacter->Pets() > 0 &&
			m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() &&
			( pItem->Type() == KItemPotion ||
			  pItem->Type() == KItemPetFood ) )
		{
			PerformItemUse( pLevel,
							pItem,
							pInventoryCharacter,
							m_pPlayerCharacter,
							m_pPlayerCharacter->Pet( 0 ) );
		}
		else
		{
			PerformItemUse( pLevel,
							pItem,
							pInventoryCharacter,
							m_pPlayerCharacter,
							m_pPlayerCharacter );
		}

	}
	else if( pItem != NULL )
	{
		// keep it at the player's position, for full volume sound
		pItem->SetPosition( pLevel, pInventoryCharacter->Position() );
		pItem->PlayRemoveSound();

		pInventoryCharacter->Inventory()->EquipItem( pLevel, ActualSlot );
	}
} // CGameUI::UseItem()

void CGameUI::UseItemOnPet( CLevel& pLevel )	// level to post effects to
{
	if( !m_pPlayerCharacter->Alive() )
	{
		return;
	}

	if( m_pDraggingItem != NULL &&
		( m_pDraggingItem->Target() == KTargetUser ||
		  m_pDraggingItem->Target() == KTargetCharacter ) )
	{
		PerformItemUse( pLevel,
						m_pDraggingItem,
						m_pItemOwner,
						m_pPlayerCharacter,
						m_pPlayerCharacter->Pet( 0 ) );
	}
	else
	{
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
	}

} // CGameUI::UseItemOnPet()

void CGameUI::PerformItemUse( CLevel& pLevel,				// level to post effects to
							  CItem* pItem,					// item to use
							  CCharacter* pOwningCharacter,	// user
							  CCharacter* pUsingCharacter,	// user
							  CCharacter* pTargetCharacter )// target
{
	if( !pItem->RequirementsMet( *pUsingCharacter ) )
	{
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpLowPower );
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		return;
	}
	if( pItem->Type() == KItemPetFood &&
		pTargetCharacter != m_pPlayerCharacter->Pet( 0 ) )
	{
		pTargetCharacter = m_pPlayerCharacter->Pet( 0 );
		//m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
		//FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		//return;
	}

	if( pItem->Type() == KItemPetFood )
	{
		if( !pTargetCharacter->AvailableForKeyCommand() )
		{
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
			FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
			return;
		}
		else
		{
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpPetTransform, kTrue );
		}
	}

	bool IsTownPortal( kFalse );
	for( uint32 e = 0; e < pItem->Effects( KActivationUsage ); e++ )
	{
		if( pItem->Effect( KActivationUsage, e )->Type() == KEffectOpenPortal )
		{
			IsTownPortal = kTrue;
		}
	}
	if( IsTownPortal && pLevel.LevelDepth() == 0 )
	{
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpSpellMiscast );
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		return;
	}

	if( pItem->Type() == KItemPotion )
	{
		m_pPlayerCharacter->IncrementStat( KJournalStatPotionsUsed );
	}

	switch( pItem->Target() )
	{
	case KTargetUser :
		// use it
		if( !pItem->CanUse( pLevel, pUsingCharacter, pTargetCharacter ) )
		{
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
			FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		}
		else
		{
			if( pItem->Type() == KItemSpell &&
				!m_pSpellMenu->Open() )
			{
				ToggleSpellMenu();
			}
			pItem->Use( pLevel, pUsingCharacter, pTargetCharacter );
			// if used up, discard it
			if( pItem->UsedUp() )
			{
				pOwningCharacter->Inventory()->RemoveItem( pItem );
				if( m_pDraggingItem == pItem )
				{
					m_pDraggingItem = NULL;
				}
				DELETE_SAFELY( pItem );
			}
		}
		break;
	case KTargetItem :
		m_pItemInUse = pItem;
		m_pSpellInUse = NULL;
		m_pOwnerOfItemInUse = pOwningCharacter;
		m_pUserOfItemInUse = pUsingCharacter;
		break;
	}
} // CGameUI::PerformItemUse()

void CGameUI::UseOnSlot( CLevel& pLevel,					// level to drop to
					 	 CCharacter* pInventoryCharacter,	// character whose inventory this is slot is from
						 uint32 ButtonPressed )				// index of button pressed to pick up 
{
	// we can only apply effects to our own items
	if( pInventoryCharacter != m_pPlayerCharacter &&
		!( m_pPlayerCharacter->Pets() > 0 &&
		   pInventoryCharacter == m_pPlayerCharacter->Pet( 0 ) ) &&
		   !( pInventoryCharacter->IsMerchant() && pInventoryCharacter->MerchantType() == KMerchantStash ) )
	{
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		return;
	}
	uint32 InventorySlot = ButtonPressed - KInventoryButton;
	if( ButtonPressed >= KBuySellButton )
	{
		InventorySlot = ButtonPressed - KBuySellButton;
	}
	else if( ButtonPressed >= KPetInventoryButton )
	{
		InventorySlot = ButtonPressed - KPetInventoryButton;
	}
	uint32 ActualSlot;
	// left arm/hand slot doubles for shields and weapons - but they are two DIFFERENT
	// attachments
	if( InventorySlot == KSlotLeftArm &&
		pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot ) == NULL )
	{
		InventorySlot = KSlotLeftHand;
	}
	else if( InventorySlot == KSlotLeftHand &&
			 pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot ) == NULL )
	{
		InventorySlot = KSlotLeftArm;
	}
	// see if there's an item in the slot
	CItem* pItem = pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot, ActualSlot );

	if( pItem != NULL )
	{
		if( m_pSpellInUse != NULL )
		{
			if( m_pSpellInUse->CanCastOnItem( pItem ) &&
				m_pSpellInUse->SufficientLevelToCastOnItem( m_pOwnerOfSpellInUse,
															pItem ) )
			{
				m_pOwnerOfSpellInUse->CastSpell( pLevel,
												 m_pSpellInUse,
												 pItem,
												 m_pTargetCharacter,
												 D3DXVECTOR3( 0, 0, 0 ) );
				m_pSpellInUse = NULL;
			}
			else
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpSpellMiscast );
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
			}
		}
		// use it
		else if( m_pItemInUse != NULL )
		{
			if( m_pItemInUse->CanUseOnItem( pItem ) )
			{
				m_pItemInUse->Use( pLevel, m_pUserOfItemInUse, pItem );
				// if used up, discard it
				if( m_pItemInUse->UsedUp() )
				{
					m_pOwnerOfItemInUse->Inventory()->RemoveItem( m_pItemInUse );
					if( m_pDraggingItem == m_pItemInUse )
					{
						m_pDraggingItem = NULL;
					}
					DELETE_SAFELY( m_pItemInUse );
				}
				m_pItemInUse = NULL;
			}
			else
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
			}
		}
	}
	else
	{
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
	}
} // CGameUI::UseOnSlot( )


void CGameUI::TakeItemFromSlot( CLevel& pLevel,						// level to drop to
								CCharacter* pInventoryCharacter,	// character whose inventory this is from
								uint32 ButtonPressed )				// index of button pressed to pick up 
{
	uint32 InventorySlot = ButtonPressed - KInventoryButton;
	if( ButtonPressed >= KBuySellButton )
	{
		InventorySlot = ButtonPressed - KBuySellButton;
	}
	else if( ButtonPressed >= KPetInventoryButton )
	{
		InventorySlot = ButtonPressed - KPetInventoryButton;
	}
	uint32 ActualSlot;
	// left arm/hand slot doubles for shields and weapons - but they are two DIFFERENT
	// attachments
	if( InventorySlot == KSlotLeftArm &&
		pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot ) == NULL )
	{
		InventorySlot = KSlotLeftHand;
	}
	else if( InventorySlot == KSlotLeftHand &&
		pInventoryCharacter->Inventory()->GetItemFromSlot( InventorySlot ) == NULL )
	{
		InventorySlot = KSlotLeftArm;
	}

	m_pDraggingItem = pInventoryCharacter->Inventory()->RemoveItemFromSlot( InventorySlot, ActualSlot );
	m_DragItemSlot = ActualSlot;


	// play a 'pick up' sound
	if( m_pDraggingItem != NULL )
	{
		// remember who owned this!
		m_pItemOwner = pInventoryCharacter;

		// if we shift-click an item, we try to immediately buy or sell it
		if( BuySellOpen() && m_KeyHandler.KeyHeld( VK_SHIFT ) )
		{
			int32 Slot( -1 );
			CCharacter* pTargetCharacter = NULL;
			if( m_pItemOwner == m_pPlayerCharacter )
			{
				pTargetCharacter = m_pPlayerCharacter->Target();
				Slot = m_pPlayerCharacter->Target()->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
				// don't have to have a valid slot to sell
				if( Slot == -1 &&
					m_pPlayerCharacter->Target()->MerchantType() != KMerchantStash )
				{
					Slot = KQuickSlots;
				}
				if( Slot != -1 )
				{
					Slot += KBuySellButton;
				}
			}
			else if( m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) )
			{
				pTargetCharacter = m_pPlayerCharacter->Pet( 0 )->Target();
				Slot = m_pPlayerCharacter->Pet( 0 )->Target()->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
				// don't have to have a valid slot to sell
				if( Slot == -1 &&
					m_pPlayerCharacter->Pet( 0 )->Target()->MerchantType() != KMerchantStash )
				{
					Slot = KQuickSlots;
				}
				if( Slot != -1 )
				{
					Slot += KBuySellButton;
				}
			}
			else
			{
				if( InventoryOpen() )
				{
					pTargetCharacter = m_pPlayerCharacter;
					if( m_pDraggingItem->Type() == KItemPotion )
					{
						Slot = m_pPlayerCharacter->Inventory()->FindFreeQuickSlot();
					}
					// have to have a valid slot to buy though!
					if( Slot == -1 )
					{
						Slot = m_pPlayerCharacter->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
					}
					if( Slot != -1 )
					{
						Slot += KInventoryButton;
					}
				}
				else
				{
					pTargetCharacter = m_pPlayerCharacter->Pet( 0 );
					if( m_pDraggingItem->Type() == KItemPotion )
					{
						Slot = m_pPlayerCharacter->Inventory()->FindFreeQuickSlot();
						if( Slot != -1 )
						{
							pTargetCharacter = m_pPlayerCharacter;
							Slot += KInventoryButton;
						}
					}
					// have to have a valid slot to buy though!
					if( Slot == -1 )
					{
						Slot = m_pPlayerCharacter->Pet( 0 )->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
						if( Slot != -1 )
						{
							Slot += KPetInventoryButton;
						}
					}
				}
			}
			if( Slot != -1 )
			{
				PlaceItem( pLevel, pTargetCharacter, Slot );
			}
			else
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
				ReturnItem( pLevel, pInventoryCharacter );
				return;
			}
		}
		// if we shift-click an item, we try to immediately buy or sell it
		else if( PetInventoryOpen() && InventoryOpen() && m_KeyHandler.KeyHeld( VK_SHIFT ) )
		{
			int32 Slot( -1 );
			CCharacter* pTargetCharacter = NULL;
			if( m_pItemOwner == m_pPlayerCharacter )
			{
				pTargetCharacter = m_pPlayerCharacter->Pet( 0 );
				Slot = m_pPlayerCharacter->Pet( 0 )->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
				if( Slot != -1 )
				{
					Slot += KPetInventoryButton;
				}
			}
			else if( m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) )
			{
				pTargetCharacter = m_pPlayerCharacter;
				Slot = m_pPlayerCharacter->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
				if( Slot != -1 )
				{
					Slot += KInventoryButton;
				}
			}
			if( Slot != -1 )
			{
				PlaceItem( pLevel, pTargetCharacter, Slot );
			}
			else
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
				ReturnItem( pLevel, pInventoryCharacter );
				return;
			}
		}
		else if( m_pEnchantMenu->Open() && m_KeyHandler.KeyHeld( VK_SHIFT ) )
		{

			if( m_pEnchantMenu->Item() == NULL )
			{
				m_pDraggingItem->SetPosition( pLevel, m_pPlayerCharacter->Position() );
				m_pDraggingItem->PlayRemoveSound();
				m_pDraggingItem->PlayTakeSound();
				m_pEnchantMenu->SetItem( m_pItemOwner, m_pDraggingItem );
				m_pDraggingItem = NULL;
			}
			else
			{
				FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
				ReturnItem( pLevel, pInventoryCharacter );
				return;
			}
		}
		else
		{
			// keep it at the player's position, for full volume sound
			m_pDraggingItem->SetPosition( pLevel, m_pPlayerCharacter->Position() );
			m_pDraggingItem->PlayTakeSound();
		}
	}


} // CGameUI::TakeItemFromSlot( )

void CGameUI::ReturnItem( CLevel& pLevel,					// level to drop to
						 CCharacter* pInventoryCharacter )	// character whose inventory this is from
{
	// play a 'remove' sound
	if( m_pDraggingItem != NULL )
	{

		// keep it at the player's position, for full volume sound
		if( pInventoryCharacter != NULL )
		{
			m_pDraggingItem->SetPosition( pLevel, pInventoryCharacter->Position() );
		}
		m_pDraggingItem->PlayRemoveSound();
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		if( m_pItemOwner->Inventory()->AddItem( m_pDraggingItem, m_DragItemSlot, &pLevel ) )
		{
			m_pDraggingItem = NULL;
			if( m_DragItemSlot < KEquipmentSlots )
			{
				m_pItemOwner->Inventory()->EquipItem( pLevel, m_DragItemSlot );
			}
			m_pItemOwner->Inventory()->VerifyEquipment( pLevel );
		}
		else
		{
			if( m_pItemOwner != m_pPlayerCharacter->Target() ||
				( m_pItemOwner != NULL && m_pItemOwner->MerchantType() == KMerchantStash ) )
			{

				DropItem( pLevel, pInventoryCharacter );
			}
			else
			{
				DELETE_SAFELY( m_pDraggingItem );
				m_pDraggingItem = NULL;
			}
		}
	}
} // CGameUI::ReturnItem()

void CGameUI::PlaceItem( CLevel& pLevel,					// level to drop to
						 CCharacter* pInventoryCharacter,	// character whose inventory this is for
						 int32 SlotIndex )					// slot to place item into
{
			// check to see what buttons we are over
	if( SlotIndex == -1 )
	{
		SlotIndex = m_pInventoryMenu->UpdateButtonOver();
		if( SlotIndex == -1 )
		{
			SlotIndex = m_pBottomMenu->UpdateButtonOver();
			// if we dropped an item on the pet's inventory button, just put it in there, if possible
			if( SlotIndex == KButtonPetInventory &&
				m_pPlayerCharacter->Pet( 0 )->AvailableForKeyCommand() )
			{
				SlotIndex = m_pPlayerCharacter->Pet( 0 )->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(), m_pDraggingItem->SlotsTall() );
				SlotIndex += KPetInventoryButton;
				pInventoryCharacter = m_pPlayerCharacter->Pet( 0 );
			}
		}
		if( SlotIndex == -1 )
		{
			SlotIndex = m_pPetInventoryMenu->UpdateButtonOver();
		}
		if( SlotIndex == -1 )
		{
			SlotIndex = m_pPetSellInventoryMenu->UpdateButtonOver();
		}
		if( SlotIndex == -1 )
		{
			SlotIndex = m_pBuySellMenu->UpdateButtonOver();
		}
		if( SlotIndex == -1 )
		{
			SlotIndex = m_pSpellMenu->UpdateButtonOver();
		}
	}

	if( SlotIndex >= KAttackSpellButton )
	{
		EMagicSphere Sphere = KMagicAttack;
		if( m_pDraggingItem->Type() != KItemSpell )
		{
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
			FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
			return;
		}
		if( SlotIndex == KActiveSpellButton )
		{
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpImpossible );
			FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
			return;
		}
		else if( SlotIndex >= KCharmSpellButton )
		{
			Sphere = KMagicCharm;
			SlotIndex -= KCharmSpellButton;
		}
		else if( SlotIndex >= KDefenseSpellButton )
		{
			Sphere = KMagicDefense;
			SlotIndex -= KDefenseSpellButton;
		}
		else
		{
			SlotIndex -= KAttackSpellButton;
		}
		PerformItemUse( pLevel,
						m_pDraggingItem,
						m_pItemOwner,
						m_pPlayerCharacter,
						m_pPlayerCharacter );
	}
	else if( SlotIndex >= KInventoryButton )
	{
		uint32 ActualSlot( 0 );
		bool Buying( kFalse );
		bool Selling( kFalse );
		bool IsMerchantInventory( kFalse );
		bool IsPetInventory( kFalse );
		if( SlotIndex >= KBuySellButton )
		{
			SlotIndex -= KBuySellButton;
			IsMerchantInventory = kTrue;
			if( m_pItemOwner == m_pPlayerCharacter ||
				m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) )
			{
				Selling = kTrue;
			}

		}
		else if( SlotIndex >= KPetInventoryButton )
		{
			SlotIndex -= KPetInventoryButton;
			IsPetInventory = kTrue;
			if( PetSellInventoryOpen() )
			{
				if( m_pItemOwner == m_pPlayerCharacter->Pet( 0 )->Target() &&
					m_pPlayerCharacter->Pet( 0 )->Target()->IsMerchant() )
				{
					Buying = kTrue;
				}
			}
		}
		else
		{
			SlotIndex -= KInventoryButton;

			if( PetSellInventoryOpen() )
			{
				if( m_pItemOwner == m_pPlayerCharacter->Pet( 0 )->Target() &&
					m_pPlayerCharacter->Pet( 0 )->Target()->IsMerchant() )
				{
					Buying = kTrue;
				}
			}
			else
			{
				if( m_pItemOwner == m_pPlayerCharacter->Target() &&
					m_pPlayerCharacter->Target()->IsMerchant() )
				{
					Buying = kTrue;
				}
			}
		}

		// stashes are effectively merchants, but we don't buy and sell from them
		if( m_pItemOwner->MerchantType() == KMerchantStash )
		{
			IsMerchantInventory = kFalse;
			Selling = kFalse;
			Buying = kFalse;
		}
		if( ( m_pItemOwner == m_pPlayerCharacter ||
			  m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) ) &&
			  m_pItemOwner->Target() != NULL &&
			  m_pItemOwner->Target()->MerchantType() == KMerchantStash )
		{
			IsMerchantInventory = kFalse;
			Selling = kFalse;
			Buying = kFalse;
		}

		uint32 PurchaseValue( 0 );
		if( m_pItemOwner != NULL )
		{
			if( m_pItemOwner->MerchantType() == KMerchantGambler &&
				!m_pDraggingItem->Identified() )
			{
				PurchaseValue = m_pDraggingItem->GambleValue();
			}
			else
			{
				PurchaseValue = m_pDraggingItem->PurchaseValue();
			}
		}

		if( !Buying ||
			m_pPlayerCharacter->Inventory()->Gold() >=
			PurchaseValue )
		{
			// play a 'remove' sound
			if( m_pDraggingItem != NULL )
			{
				// keep it at the player's position, for full volume sound
				m_pDraggingItem->SetPosition( pLevel, pInventoryCharacter->Position() );
				m_pDraggingItem->PlayRemoveSound();
			}
				
			// left arm/hand slot doubles for shields and weapons - but they are two DIFFERENT
			// attachments
			if( SlotIndex == KSlotLeftArm &&
				m_pDraggingItem->Category() == KCategoryWeapon )
			{
				SlotIndex = KSlotLeftHand;
			}
			
			bool InstantGemInstall = kFalse;
			bool InstantScrollToBook = kFalse;
			CItem* pTargetSlotItem = pInventoryCharacter->Inventory()->GetItemFromSlot( SlotIndex, 
																						m_pDraggingItem->SlotsWide(),
																						m_pDraggingItem->SlotsTall(),
																						ActualSlot );

			if( m_pDraggingItem->Type() == KItemGem &&
				SlotIndex == KSlotLeftArm &&
 				pTargetSlotItem == NULL )
			{
				SlotIndex = KSlotLeftHand;
				pTargetSlotItem = pInventoryCharacter->Inventory()->GetItemFromSlot( SlotIndex, 
																					m_pDraggingItem->SlotsWide(),
																					m_pDraggingItem->SlotsTall(),
																					ActualSlot );
			}
			else if( m_pDraggingItem->Type() == KItemGem &&
					 SlotIndex == KSlotLeftHand &&
 					 pTargetSlotItem == NULL )
			{
				SlotIndex = KSlotLeftArm;
				pTargetSlotItem = pInventoryCharacter->Inventory()->GetItemFromSlot( SlotIndex, 
																					m_pDraggingItem->SlotsWide(),
																					m_pDraggingItem->SlotsTall(),
																					ActualSlot );
			}

			if( m_pDraggingItem->Type() == KItemScroll &&
				pTargetSlotItem != NULL &&
				pTargetSlotItem->Type() == KItemBook )
			{
				if( pTargetSlotItem->Uses() < 20 )
				{
					if( m_pDraggingItem->Name() == LoadLocalizedString(1137) && // TDC 2008-04-22 Craig Miles (enables placing scroll in book)
						pTargetSlotItem->Name() == LoadLocalizedString(1166) )
					{
						InstantScrollToBook = kTrue;
					}
					if( m_pDraggingItem->Name() == LoadLocalizedString(1140) && // TDC 2008-04-22 Craig Miles (enables placing scroll in book)
						pTargetSlotItem->Name() == LoadLocalizedString(1165) )
					{
						InstantScrollToBook = kTrue;
					}
				}
			}

			if( m_pDraggingItem->Type() == KItemGem &&
				pTargetSlotItem != NULL &&
				pTargetSlotItem->OpenSockets() > 0 )
			{
				InstantGemInstall = kTrue;
			}
			if( ( pInventoryCharacter->Inventory()->FitsSlot( SlotIndex, m_pDraggingItem ) ||
				  InstantGemInstall ||
				  InstantScrollToBook ) ||
				Selling )
			{
				if( Buying )
				{
					FSOUND_PlaySound( FSOUND_FREE, m_pPurchaseSound );
					if( m_pItemOwner->MerchantType() == KMerchantGambler &&
						!m_pDraggingItem->Identified() )
					{
						m_pPlayerCharacter->IncrementStat( KJournalStatTimesGambled );

						m_pPlayerCharacter->TakeGold( m_pDraggingItem->GambleValue() );
						m_pDraggingItem->SetIdentified( kTrue );
					}
					else
					{
						m_pPlayerCharacter->TakeGold( m_pDraggingItem->PurchaseValue() );
					}
				}
				else if( Selling )
				{
					FSOUND_PlaySound( FSOUND_FREE, m_pPurchaseSound );
					m_pPlayerCharacter->GiveGold( m_pDraggingItem->SellValue() );
				}

				CItem* CurrentItem = NULL;

				if( !Selling )
				{
					CurrentItem = pInventoryCharacter->Inventory()->GetItemFromSlot( SlotIndex, 
																					 m_pDraggingItem->SlotsWide(),
																					 m_pDraggingItem->SlotsTall(),
																					 ActualSlot );

					// merge scrolls into books, if required
					if( InstantScrollToBook )	// merge scrolls into book
					{
						// play a 'pick up' sound
						if( m_pDraggingItem != NULL )
						{
							// keep it at the player's position, for full volume sound
							m_pDraggingItem->SetPosition( pLevel, pInventoryCharacter->Position() );
							m_pDraggingItem->PlayTakeSound();
						}

						CurrentItem->SetUses( CurrentItem->Uses() + m_pDraggingItem->Uses() );
						DELETE_SAFELY( m_pDraggingItem )
						m_pDraggingItem = NULL;
						return;
					}

					// apply a socketed item, if possible
					if( CurrentItem != NULL &&
						m_pDraggingItem->Type() == KItemGem &&
						CurrentItem->OpenSockets() > 0 &&
						!( IsMerchantInventory ) )
					{
						if( CurrentItem->Effects( KActivationPassive ) == 0 )
						{
							CurrentItem->SetIdentified( kTrue );
						}
						// add to the item
						CurrentItem->AddItem( m_pDraggingItem );

						for( uint32 i = 0; i < m_pDraggingItem->Effects( KActivationPassive ); i++ )
						{
							CEffect* pEffect = m_pDraggingItem->Effect( KActivationPassive, i );
							// elemental damage is added slightly differently from other effects
							if( pEffect->Type() == KEffectAddDamageType )
							{
								EDamageType DamageType( KDamageFire );
								std::string EffectName = StringUpper( pEffect->Name() );
								for( uint32 a = 0; a < KDamageTypes; a++ )
								{
									if( EffectName == KDamageNames[a] )
									{
										DamageType = (EDamageType)a;
									}
								}
								// non-weapon items get resistances instead of damage bonuses
								if( CurrentItem->Category() != KCategoryWeapon )
								{
									CEffect* pNewEffect = new CEffect( pEffect );
									pNewEffect->SetType( KDamageResistance[DamageType] );
									pNewEffect->SetValue( pNewEffect->Value() * 4 );	// resistances get multiplied by 4x
									CurrentItem->AddNewEffect( pNewEffect );

								}
								else
								{
									if( pEffect->Value() != 0 )
									{
										CurrentItem->AddDamageBonus( DamageType, (int32)pEffect->Value() );
									}
								}
							}
							else
							{
								CurrentItem->AddEffect( pEffect );
							}
						}
						pInventoryCharacter->Inventory()->CalculateEffectValues();
						m_pDraggingItem->Use( pLevel, pInventoryCharacter, pInventoryCharacter );
						m_pDraggingItem = NULL;
						return;
					}
					else
					{
						pInventoryCharacter->Inventory()->RemoveItemFromSlot( SlotIndex, 
																			m_pDraggingItem->SlotsWide(),
																			m_pDraggingItem->SlotsTall(),
																			ActualSlot );
					}

				}

				// if selling, and the merchant doesn't have room, we just have to delete the item -
				if( Selling )
				{
					int32 SellSlot = pInventoryCharacter->Inventory()->FindFreeSlot( m_pDraggingItem->SlotsWide(),
																					 m_pDraggingItem->SlotsTall() );
					if( SellSlot == -1 )
					{
						DELETE_SAFELY( m_pDraggingItem );
					}
					else
					{
						pInventoryCharacter->Inventory()->AddItem( m_pDraggingItem, SellSlot, &pLevel );
					}
				}
				else
				{
					pInventoryCharacter->Inventory()->AddItem( m_pDraggingItem, SlotIndex, &pLevel );
				}
				// make sure the item we just picked up knows its owner
				m_pItemOwner = pInventoryCharacter;
				m_pDraggingItem = CurrentItem;
				if( CurrentItem != NULL )
				{
					m_DragItemSlot = ActualSlot;
				}
				// equip it if we just put it in an equipment slot
				if( SlotIndex < KEquipmentSlots )
				{
					pInventoryCharacter->Inventory()->EquipItem( pLevel, SlotIndex );
				}
				// play a 'pick up' sound
				if( m_pDraggingItem != NULL )
				{
					// keep it at the player's position, for full volume sound
					m_pDraggingItem->SetPosition( pLevel, pInventoryCharacter->Position() );
					m_pDraggingItem->PlayTakeSound();
				}
			}
			else	// failed to move - put it back where it came from
			{
				if( pInventoryCharacter->Inventory()->ItemsInSlot( m_DragItemSlot, 
																m_pDraggingItem->SlotsWide(),
																m_pDraggingItem->SlotsTall() ) == 0 )
				{
					ReturnItem( pLevel, pInventoryCharacter );
					FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
					m_pItemOwner->Inventory()->AddItem( m_pDraggingItem, m_DragItemSlot, &pLevel );
					m_pDraggingItem = NULL;
					m_pItemOwner->Inventory()->VerifyEquipment( pLevel );
				}
			}
		}
		else
		{
			if( Buying )
			{
				m_pPlayerCharacter->PlayHelpSample( KSoundHelpLowGold );
			}

			FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		}
	}
	else if( SlotIndex != -1 )
	{
		ReturnItem( pLevel,
					pInventoryCharacter );

/*		// play a 'remove' sound
		if( m_pDraggingItem != NULL )
		{
			// keep it at the player's position, for full volume sound
			m_pDraggingItem->SetPosition( pLevel, pInventoryCharacter->Position() );
			m_pDraggingItem->PlayRemoveSound();
		}

		bool Buying( kFalse );
		// see if we're trying to buy this item
		if( m_pBuySellMenu->Open() &&
			m_pItemOwner == m_pPlayerCharacter->Target() )
		{
			Buying = kTrue;
		}
				
		if( !Buying ||
			m_pPlayerCharacter->Inventory()->Gold() >=
			m_pDraggingItem->PurchaseValue() )
		{
			if( Buying )
			{
				FSOUND_PlaySound( FSOUND_FREE, m_pPurchaseSound );
				m_pPlayerCharacter->TakeGold( m_pDraggingItem->PurchaseValue() );
			}
			if( pInventoryCharacter->Inventory()->ItemsInSlot( m_DragItemSlot, 
															m_pDraggingItem->SlotsWide(),
															m_pDraggingItem->SlotsTall() ) == 0 )
			{
				pInventoryCharacter->Inventory()->AddItem( m_pDraggingItem, m_DragItemSlot );
				m_pDraggingItem = NULL;
			}
		}
		else
		{
			FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
		}*/
	}
	else
	{
		FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
	}
} // CGameUI::PlaceItem()



void CGameUI::FindTextEvents( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
							  CGameClient& pClient )				// game client
{
	CLLNode<CEventText>* pEvent = pClient.Level()->TextEvents()->GetFirst();
	CLLNode<CEventText>* pNext;
	while ( pEvent != NULL )
	{
		pNext = pEvent->Next();
		if( /*( pEvent->Data()->Parent() == m_pPlayerCharacter ||
			  pEvent->Data()->Target() == m_pPlayerCharacter ) &&*/
			pEvent->Data()->InFrustum( pClient.Camera()->Frustum() ) )
		{
			m_pEventTextList->Queue( pEvent->Data() );
			pEvent->Data()->CreateText( pD3DDevice,
										m_pFontMetric,
										m_pFontMaterial );
			pClient.Level()->TextEvents()->RemoveNoDelete( pEvent );
		}
		pEvent = pNext;
	}
} // CGameUI::FindTextEvents()

void CGameUI::UpdateTextEvents( float32 TimeElapsed )	// time elapsed in seconds
{
	CLLNode<CEventText>*	pEventText	= m_pEventTextList->GetFirst();
	CLLNode<CEventText>*	pNext;
	while ( pEventText != NULL )
	{
		pNext = pEventText->Next();

		pEventText->Data()->Update( TimeElapsed );
		if( pEventText->Data()->Opacity() <= 0 )
		{
			m_pEventTextList->Remove( pEventText );
		}
		pEventText = pNext;
	}
} // CGameUI::UpdateTextEvents()

void CGameUI::RenderTextEvents( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
							   const D3DXMATRIX& ViewportMatrix,	// viewport matrix ( for bringing 3d objects to screen space
							   const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	bool CanFade( kFalse );

	if( m_pSettings.GetSettings( KSetAlphaFade ) )
	{
		CanFade = kTrue;

		// turn on fading
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
	}
	pD3DDevice->SetTexture( 0, m_pFontMaterial->GetTexture() );
	bool IsSecondaryFont( kFalse );

	D3DXMATRIX FinalMatrix;
	D3DXMatrixMultiply( &FinalMatrix, &ViewportMatrix, &ProjectionMatrix );
	D3DXVECTOR3 ScreenPosition;

	// render all characters
	CLLNode<CEventText>*	pEvent	= m_pEventTextList->GetFirst();
	while ( pEvent != NULL )
	{
		if( CanFade )
		{
			uint32 Fade = (uint32)floor( pEvent->Data()->Opacity() + .5f );
			pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 
										D3DCOLOR_RGBA(0, 0, 0, Fade));			
		}
			
		if(  pEvent->Data()->SecondaryFont() != IsSecondaryFont )
		{
			IsSecondaryFont = pEvent->Data()->SecondaryFont();

			if( IsSecondaryFont )
			{
				pD3DDevice->SetTexture( 0, m_pFontMaterial2->GetTexture() );
			}
			else
			{
				pD3DDevice->SetTexture( 0, m_pFontMaterial->GetTexture() );
			}
		}
		D3DXVec3TransformCoord( &ScreenPosition, 
								&pEvent->Data()->Position(),
								&FinalMatrix );

		ScreenPosition /= ScreenPosition.z;
		ScreenPosition.x *= 512;
		ScreenPosition.y *= 384;
		ScreenPosition.x += 512;
		ScreenPosition.y -= 384;
		ScreenPosition.y *= -1.0f;

		if( m_ViewType == KViewLeft )
		{
			ScreenPosition.x *= .5f;
		}
		else if( m_ViewType == KViewRight )
		{
			ScreenPosition.x *= .5f;
			ScreenPosition.x += 512;
		}
		pEvent->Data()->Text()->SetPosition( ScreenPosition.x, ScreenPosition.y );
		pEvent->Data()->Text()->RenderNoTexture( pD3DDevice );

		pEvent = pEvent->Next();
	}

	if( CanFade )
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	}

} // CGameUI::RenderTextEvents()

// render any 3d elements
void CGameUI::Render3D( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						CGameClient& pClient,					// game client
						const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	m_pFateMenu->Render3D( pD3DDevice,
						   pClient );
} // CGameUI::Render3D()

// render any 3d elements
void CGameUI::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					  CGameClient& pClient,					// game client
					  const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
					  const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	FindTextEvents( pD3DDevice,
					pClient );

	if( !FateOpen() )
	{
		RenderTextEvents( pD3DDevice, ViewportMatrix, ProjectionMatrix );
	}

	for( uint32 i = 0; i < 10; i++ )
	{
		RemoveButton( m_pPlayerEffects[i] );
		RemoveButton( m_pTargetEffects[i] );
	}

	int32 ActiveEffect( 0 );
	int32 ActiveTargetEffect( 0 );
	if( !BothCovered() )
	{
		for( uint32 j = 0; j < KActivationTypes; j++ )
		{
			if( m_pTargetCharacter != NULL &&
				!m_pTargetCharacter->DisplayedAsItem() )
			{
				for( i = 0; i < m_pTargetCharacter->Effects( (EEffectActivation)j ); i++ )
				{
					if( m_pTargetCharacter->Effect( (EEffectActivation)j, i )->ParentSpell() != NULL &&
						ActiveTargetEffect < 9 )
					{
						AddButton( m_pTargetEffects[ActiveTargetEffect] );
						if( LeftCovered() )
						{
							m_pTargetEffects[ActiveTargetEffect]->SetPosition( 568 + (float32)ActiveTargetEffect * 28, 16 );
						}
						else if( RightCovered() )
						{
							m_pTargetEffects[ActiveTargetEffect]->SetPosition( 56 + (float32)ActiveTargetEffect * 28, 16 );
						}
						else
						{
							m_pTargetEffects[ActiveTargetEffect]->SetPosition( 312 + (float32)ActiveTargetEffect * 28, 16 );
						}
						
						m_pTargetEffects[ActiveTargetEffect]->SetMaterial( m_pTargetCharacter->Effect( (EEffectActivation)j, i )->ParentSpell()->IconMaterial() );
						std::string Description = "\b" + m_pTargetCharacter->Effect( (EEffectActivation)j, i )->ParentSpell()->Name() + "\b";
						float32 Seconds = m_pTargetCharacter->Effect( (EEffectActivation)j, i )->Duration() - m_pTargetCharacter->Effect( (EEffectActivation)j, i )->TimeActive();
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
						Description = Description + "\n" + IntegerToString( Hours ) + ":" + MinuteString +
										":" + SecondString + " remaining";
						m_pTargetEffects[ActiveTargetEffect]->SetToolTip( Description );
						ActiveTargetEffect++;
					}
				}
			}

			for( i = 0; i < m_pPlayerCharacter->Effects( (EEffectActivation)j ); i++ )
			{
				if( m_pPlayerCharacter->Effect( (EEffectActivation)j, i )->ParentSpell() != NULL &&
					ActiveEffect < 9 )
				{
					AddButton( m_pPlayerEffects[ActiveEffect] );
					if( LeftCovered() )
					{
						m_pPlayerEffects[ActiveEffect]->SetPosition( 532, 180 + (float32)ActiveEffect * 28 );
					}
					else
					{
						m_pPlayerEffects[ActiveEffect]->SetPosition( 20, 180 + (float32)ActiveEffect * 28 );
					}
					
					m_pPlayerEffects[ActiveEffect]->SetMaterial( m_pPlayerCharacter->Effect( (EEffectActivation)j, i )->ParentSpell()->IconMaterial() );
					std::string Description = "\b" + m_pPlayerCharacter->Effect( (EEffectActivation)j, i )->ParentSpell()->Name() + "\b";
					float32 Seconds = m_pPlayerCharacter->Effect( (EEffectActivation)j, i )->Duration() - m_pPlayerCharacter->Effect( (EEffectActivation)j, i )->TimeActive();
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
					Description = Description + "\n" + IntegerToString( Hours ) + ":" + MinuteString +
									":" + SecondString + " remaining";
					m_pPlayerEffects[ActiveEffect]->SetToolTip( Description );
					ActiveEffect++;
				}
			}
		}
	}

	RenderSprites( pD3DDevice );
	RenderButtons( pD3DDevice );
	RenderText( pD3DDevice );


	D3DXMATRIX FinalMatrix;
	D3DXMatrixMultiply( &FinalMatrix, &ViewportMatrix, &ProjectionMatrix );

	D3DXMATRIX Transpose;
	D3DXMatrixTranspose( &Transpose, &ViewportMatrix );
	D3DXVECTOR3 CameraUp( Transpose._21, Transpose._22, Transpose._23 );

	if( FateOpen() )
	{
		m_pDungeonLevel->SetText( pD3DDevice, "" );
	}
	else if( pClient.Level()->LevelDepth() == 0 )
	{
		m_pDungeonLevel->SetText( pD3DDevice, LoadLocalizedString(820) );
	}
	else
	{
		m_pDungeonLevel->SetText( pD3DDevice, LoadLocalizedString1Variable(821, IntegerToString( pClient.Level()->LevelDepth() ) ));
	}

	if( m_pPlayerCharacter->Pets() > 1 )
	{
		m_pSummonCount->SetText( pD3DDevice, LoadLocalizedString2Variables(822, IntegerToString( m_pPlayerCharacter->Pets() - 1 ), IntegerToString( KMaximumSummonedCreatures - 1 ) ));
	}
	else
	{
		m_pSummonCount->SetText( pD3DDevice, "" );
	}

	if( LeftCovered() )
	{
		m_pSummonCount->SetPosition( 522, 8 );
	}
	else
	{
		m_pSummonCount->SetPosition( 10, 8 );
	}

	// if displaying all items on the ground -

	if( m_DisplayAllItems && 
		m_pDraggingItem == NULL )
	{
		CLLNode<CItem>*	pItem = pClient.Level()->OnscreenItems()->GetFirst();

		while ( pItem != NULL )
		{
			if( !( pItem->Data() == m_pTargetItem &&
				  m_pMouseOverItem == NULL && 
				  m_pMouseOverSpell == NULL && 
				  m_pDraggingItem == NULL ) ) 
			{
				float32 VerticalOffset = D3DXVec3Length( &pItem->Data()->GetMaxBounds() );
				if( VerticalOffset > 6 )
				{
					VerticalOffset = 6;
				}
				D3DXVECTOR3 ScreenPosition;
				D3DXVec3TransformCoord( &ScreenPosition, 
										&( pItem->Data()->Position() + 
										CameraUp * VerticalOffset ),
										&FinalMatrix );

				ScreenPosition /= ScreenPosition.z;
				ScreenPosition.x *= 512;
				ScreenPosition.y *= 384;
				ScreenPosition.x += 512;
				ScreenPosition.y -= 384;
				ScreenPosition.y *= -1.0f;

				if( m_ViewType == KViewLeft )
				{
					ScreenPosition.x *= .5f;
				}
				else if( m_ViewType == KViewRight )
				{
					ScreenPosition.x *= .5f;
					ScreenPosition.x += 512;
				}

				if( !pItem->Data()->HasDisplayText() )
				{
					pItem->Data()->CreateNameForDisplay( pD3DDevice, m_pFontMetric, m_pFontMaterial );
				}

				// first resize our description shadow to fit
				float32 Width = pItem->Data()->DisplayText()->MaxBounds().x -
								pItem->Data()->DisplayText()->MinBounds().x;
				float32 Height = pItem->Data()->DisplayText()->MaxBounds().y -
								pItem->Data()->DisplayText()->MinBounds().y;

				Width *= .5f;
				Height *= .5f;
				Width += 6;
				Height += 6;


				if( ScreenPosition.x - Width < 0 )
				{
					ScreenPosition.x = Width;
				}
				if( ScreenPosition.y < 0 )
				{
					ScreenPosition.y = 0;
				}
				if( ScreenPosition.x + Width > 1024 )
				{
					ScreenPosition.x = 1024 - Width;
				}
				if( ScreenPosition.y + Height * 2.0f > 768 )
				{
					ScreenPosition.y = 768 - Height * 2.0f;
				}

				pItem->Data()->DisplayText()->SetPosition( ScreenPosition.x, ScreenPosition.y );

				pItem->Data()->TextBackdrop()->SetPosition( ScreenPosition.x, ScreenPosition.y + ( pItem->Data()->DisplayText()->MaxBounds().y -
															pItem->Data()->DisplayText()->MinBounds().y ) * .5f );

				pD3DDevice->SetTexture( 0, m_pDescriptionShadowMaterial->GetTexture() );

				pItem->Data()->TextBackdrop()->RenderNoTexture( pD3DDevice );

				pItem->Data()->DisplayText()->Render( pD3DDevice );
			}
			pItem = pItem->Next();
		}
	}

	// render description of mouseover items
	if( m_pTargetItem != NULL && 
		m_pMouseOverItem == NULL && 
		m_pMouseOverSpell == NULL && 
		m_pDraggingItem == NULL )
	{

		float32 VerticalOffset = D3DXVec3Length( &m_pTargetItem->GetMaxBounds() );
		if( VerticalOffset > 6 )
		{
			VerticalOffset = 6;
		}
		D3DXVECTOR3 ScreenPosition;
		D3DXVec3TransformCoord( &ScreenPosition, 
								&( m_pTargetItem->Position() + 
								   CameraUp * VerticalOffset ),
								&FinalMatrix );

		ScreenPosition /= ScreenPosition.z;
		ScreenPosition.x *= 512;
		ScreenPosition.y *= 384;
		ScreenPosition.x += 512;
		ScreenPosition.y -= 384;
		ScreenPosition.y *= -1.0f;

		if( m_ViewType == KViewLeft )
		{
			ScreenPosition.x *= .5f;
		}
		else if( m_ViewType == KViewRight )
		{
			ScreenPosition.x *= .5f;
			ScreenPosition.x += 512;
		}
		if( !m_pTargetItem->HasDisplayText() )
		{
			m_pTargetItem->CreateNameForDisplay( pD3DDevice, m_pFontMetric, m_pFontMaterial );
		}

		// first resize our description shadow to fit
		float32 Width = m_pTargetItem->DisplayText()->MaxBounds().x -
						m_pTargetItem->DisplayText()->MinBounds().x;
		float32 Height = m_pTargetItem->DisplayText()->MaxBounds().y -
						 m_pTargetItem->DisplayText()->MinBounds().y;

		Width *= .5f;
		Height *= .5f;
		Width += 6;
		Height += 6;

		if( ScreenPosition.x - Width < 0 )
		{
			ScreenPosition.x = Width;
		}
		if( ScreenPosition.y < 0 )
		{
			ScreenPosition.y = 0;
		}
		if( ScreenPosition.x + Width > 1024 )
		{
			ScreenPosition.x = 1024 - Width;
		}
		if( ScreenPosition.y + Height * 2.0f > 768 )
		{
			ScreenPosition.y = 768 - Height * 2.0f;
		}

		m_pTargetItem->DisplayText()->SetPosition( ScreenPosition.x, ScreenPosition.y );


		m_pTargetItem->TextBackdrop()->SetPosition( ScreenPosition.x, ScreenPosition.y + ( m_pTargetItem->DisplayText()->MaxBounds().y -
																						   m_pTargetItem->DisplayText()->MinBounds().y ) * .5f );

		pD3DDevice->SetTexture( 0, m_pDescriptionHighlightMaterial->GetTexture() );
		m_pTargetItem->TextBackdrop()->RenderNoTexture( pD3DDevice );

		m_pTargetItem->DisplayText()->Render( pD3DDevice );
	}

	if( !BothCovered() )
	{
		CLLNode<CCharacter>*	pCharacter = pClient.Level()->OnscreenCharacters()->GetFirst();

		while ( pCharacter != NULL )
		{
			if( pCharacter->Data() == m_pTargetCharacter &&
				pCharacter->Data()->DisplayedAsItem() )
			{
				float32 VerticalOffset = D3DXVec3Length( &m_pTargetCharacter->GetMaxBounds() );
				if( VerticalOffset > 6 )
				{
					VerticalOffset = 6;
				}
				D3DXVECTOR3 ScreenPosition;
				D3DXVec3TransformCoord( &ScreenPosition, 
										&( m_pTargetCharacter->Position() + 
										   CameraUp * VerticalOffset ),
										&FinalMatrix );

				ScreenPosition /= ScreenPosition.z;
				ScreenPosition.x *= 512;
				ScreenPosition.y *= 384;
				ScreenPosition.x += 512;
				ScreenPosition.y -= 384;
				ScreenPosition.y *= -1.0f;

				if( m_ViewType == KViewLeft )
				{
					ScreenPosition.x *= .5f;
				}
				else if( m_ViewType == KViewRight )
				{
					ScreenPosition.x *= .5f;
					ScreenPosition.x += 512;
				}
				if( !m_pTargetCharacter->HasDisplayText() )
				{
					m_pTargetCharacter->CreateNameForDisplay( pD3DDevice, m_pFontMetric, m_pFontMaterial );
				}

				// first resize our description shadow to fit
				float32 Width = m_pTargetCharacter->DisplayText()->MaxBounds().x -
								m_pTargetCharacter->DisplayText()->MinBounds().x;
				float32 Height = m_pTargetCharacter->DisplayText()->MaxBounds().y -
								 m_pTargetCharacter->DisplayText()->MinBounds().y;

				Width *= .5f;
				Height *= .5f;
				Width += 6;
				Height += 6;

				if( ScreenPosition.x - Width < 0 )
				{
					ScreenPosition.x = Width;
				}
				if( ScreenPosition.y < 0 )
				{
					ScreenPosition.y = 0;
				}
				if( ScreenPosition.x + Width > 1024 )
				{
					ScreenPosition.x = 1024 - Width;
				}
				if( ScreenPosition.y + Height * 2.0f > 768 )
				{
					ScreenPosition.y = 768 - Height * 2.0f;
				}

				m_pTargetCharacter->DisplayText()->SetPosition( ScreenPosition.x, ScreenPosition.y );


				m_pTargetCharacter->TextBackdrop()->SetPosition( ScreenPosition.x, ScreenPosition.y + ( m_pTargetCharacter->DisplayText()->MaxBounds().y -
																 m_pTargetCharacter->DisplayText()->MinBounds().y ) * .5f );

				pD3DDevice->SetTexture( 0, m_pDescriptionHighlightMaterial->GetTexture() );
				m_pTargetCharacter->TextBackdrop()->RenderNoTexture( pD3DDevice );

				m_pTargetCharacter->DisplayText()->Render( pD3DDevice );
			}
			for( uint32 i = 0; i < m_pPlayerCharacter->Quests(); i++ )
			{
				if( m_pPlayerCharacter->Quest( i )->Completed() &&
					m_pPlayerCharacter->Quest( i )->GiverName() == pCharacter->Data()->Name() )
				{
					float32 VerticalOffset = 5;
					D3DXVECTOR3 ScreenPosition;
					D3DXVec3TransformCoord( &ScreenPosition, 
											&( pCharacter->Data()->Position() + 
											CameraUp * VerticalOffset ),
											&FinalMatrix );

					ScreenPosition /= ScreenPosition.z;
					ScreenPosition.x *= 512;
					ScreenPosition.y *= 384;
					ScreenPosition.x += 512;
					ScreenPosition.y -= 384;
					ScreenPosition.y *= -1.0f;

					if( m_ViewType == KViewLeft )
					{
						ScreenPosition.x *= .5f;
					}
					else if( m_ViewType == KViewRight )
					{
						ScreenPosition.x *= .5f;
						ScreenPosition.x += 512;
					}


					m_pExclamation2->SetPosition( ScreenPosition.x, ScreenPosition.y );

					m_pExclamation2->Render( pD3DDevice );
				}
			}
			if( ( pCharacter->Data() == m_pPlayerCharacter &&
				pCharacter->Data()->Fishing() &&
				pCharacter->Data()->FishBiting() ) )
			{
				float32 VerticalOffset = 5;
				D3DXVECTOR3 ScreenPosition;
				D3DXVec3TransformCoord( &ScreenPosition, 
										&( pCharacter->Data()->Position() + 
										CameraUp * VerticalOffset ),
										&FinalMatrix );

				ScreenPosition /= ScreenPosition.z;
				ScreenPosition.x *= 512;
				ScreenPosition.y *= 384;
				ScreenPosition.x += 512;
				ScreenPosition.y -= 384;
				ScreenPosition.y *= -1.0f;

				if( m_ViewType == KViewLeft )
				{
					ScreenPosition.x *= .5f;
				}
				else if( m_ViewType == KViewRight )
				{
					ScreenPosition.x *= .5f;
					ScreenPosition.x += 512;
				}


				m_pExclamation2->SetPosition( ScreenPosition.x, ScreenPosition.y );

				m_pExclamation2->Render( pD3DDevice );
			}
			else if( ( pCharacter->Data()->IsQuestGiver() &&
				pCharacter->Data()->Quests() > 0 &&
				!m_pPlayerCharacter->QuestsFull() ) )
			{
				float32 VerticalOffset = 5;
				D3DXVECTOR3 ScreenPosition;
				D3DXVec3TransformCoord( &ScreenPosition, 
										&( pCharacter->Data()->Position() + 
										CameraUp * VerticalOffset ),
										&FinalMatrix );

				ScreenPosition /= ScreenPosition.z;
				ScreenPosition.x *= 512;
				ScreenPosition.y *= 384;
				ScreenPosition.x += 512;
				ScreenPosition.y -= 384;
				ScreenPosition.y *= -1.0f;

				if( m_ViewType == KViewLeft )
				{
					ScreenPosition.x *= .5f;
				}
				else if( m_ViewType == KViewRight )
				{
					ScreenPosition.x *= .5f;
					ScreenPosition.x += 512;
				}


				m_pExclamation->SetPosition( ScreenPosition.x, ScreenPosition.y );

				m_pExclamation->Render( pD3DDevice );
			}

			pCharacter = pCharacter->Next();
		}
	}
	
	if( m_pTargetCharacter != NULL &&
		!m_pTargetCharacter->DisplayedAsItem() &&
		!BothCovered() )
	{
		m_pTargetHealthBarBack->Render( pD3DDevice );
		
		float32 Percentage = m_pTargetCharacter->HPPercentage();
		if( Percentage < 0 )
		{
			Percentage = 0;
		}

		switch( m_ViewType )
		{
		case KViewFull :
			m_pTargetEliteIcon->SetPosition( 322, 50 );
			m_pTargetHealthBarFront->SetPosition( 312, 48 );
			m_pTargetHealthBarBack->SetPosition( 312, 48 );
			m_pTargetName->SetPosition( 512, 48 );
			m_pTargetLevel->SetPosition( 512, 25 );
			m_pTargetResistances->SetPosition( 512, 80 );
			break;
		case KViewLeft :
			m_pTargetEliteIcon->SetPosition( 66, 50 );
			m_pTargetHealthBarFront->SetPosition( 56, 48 );
			m_pTargetHealthBarBack->SetPosition( 56, 48 );
			m_pTargetName->SetPosition( 256, 48 );
			m_pTargetLevel->SetPosition( 256, 25 );
			m_pTargetResistances->SetPosition( 256, 80 );
			break;
		case KViewRight :
			m_pTargetEliteIcon->SetPosition( 578, 50 );
			m_pTargetHealthBarFront->SetPosition( 568, 48 );
			m_pTargetHealthBarBack->SetPosition( 568, 48 );
			m_pTargetName->SetPosition( 768, 48 );
			m_pTargetLevel->SetPosition( 768, 25 );
			m_pTargetResistances->SetPosition( 768, 80 );
			break;
		}

		m_pTargetHealthBarFront->SetSize( 400 * Percentage, 32 );
		m_pTargetHealthBarFront->Render( pD3DDevice );
		if( m_pTargetCharacter->Poisoned() )
		{
			m_pTargetHealthBarFront->SetUVs( 0, .6f, 1, .7f );
		}
		else
		{
			m_pTargetHealthBarFront->SetUVs( 0, .1f, 1, .2f );
		}

		if( m_pTargetCharacter->IsUnique() )
		{
			m_pTargetName->SetBold( pD3DDevice, kTrue );
		}
		else
		{
			m_pTargetName->SetBold( pD3DDevice, kFalse );
		}
		m_pTargetName->SetText( pD3DDevice, m_pTargetCharacter->Name() );

		m_pTargetLevel->SetText( pD3DDevice, LoadLocalizedString1Variable(823, IntegerToString( m_pTargetCharacter->Level() ) ));

		std::string ResistanceText( "" );
		for( uint32 r = 0; r < KDamageTypes; r++ )
		{
			if( r != KDamageUndead &&
				m_pTargetCharacter->DamageResistance( (EDamageType)r ) > 0 )
			{
				ResistanceText = ResistanceText + 
								 IntegerToString( m_pTargetCharacter->DamageResistance( (EDamageType)r ) ) + "% " +
								 KDamageResistanceDescription[r] + "\n";
			}
			else if( m_pTargetCharacter->DamageResistance( (EDamageType)r ) < 0 )
			{
				ResistanceText = ResistanceText + "\b" +
								 IntegerToString( abs( m_pTargetCharacter->DamageResistance( (EDamageType)r ) ) ) + "% " +
								 KDamageWeaknessDescription[r] + "\b\n";
			}
			else if( r == KDamageUndead &&
					m_pTargetCharacter->DamageResistance( (EDamageType)r ) == 0 )
			{
				ResistanceText = ResistanceText + "\b" +
								 KDamageWeaknessDescription[r] + "\b\n";
			}
		}

		float32 Seconds( 0 );
		if( m_pTargetCharacter->TransformationDuration( ) > 0 )
		{
			Seconds = m_pTargetCharacter->TransformationDuration();
		}
		if( m_pTargetCharacter->LifeDuration( ) > 0 )
		{
			Seconds = m_pTargetCharacter->LifeDuration();
		}
		if( Seconds > 0 )
		{
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
			ResistanceText = ResistanceText + "\b" + IntegerToString( Hours ) + ":" + MinuteString +
							   ":" + SecondString + "\b";
		}
		m_pTargetResistances->SetText( pD3DDevice, ResistanceText );

		m_pTargetName->Render( pD3DDevice );
		m_pTargetLevel->Render( pD3DDevice );
		m_pTargetResistances->Render( pD3DDevice );

		if( m_pTargetCharacter->Rank() > KRankNormal )
		{
			m_pTargetEliteIcon->Render( pD3DDevice );
		}
	}

	if( m_MapVisible &&
		!BothCovered() &&
		!EnchantOpen() &&
		!m_pHelpMenu->Open() &&
		!( m_pBottomMenu->SpellsRolledOut() && ( RightCovered() || LeftCovered() ) ) )
	{
		if( LeftCovered() )
		{
			m_pMiniMapSprite->SetPosition( (float32)KMiniMapTopLeftX + 512,
										   (float32)KMiniMapTopLeftY );
		}
		else
		{
			m_pMiniMapSprite->SetPosition( (float32)KMiniMapTopLeftX,
										   (float32)KMiniMapTopLeftY );
		}
		m_pMiniMapSprite->Render( pD3DDevice );
		RenderMiniMap( pD3DDevice,
					   pClient );
	}

	m_pBottomMenu->Render( pD3DDevice,
						   *this,
						   ViewportMatrix,
						   ProjectionMatrix );

	bool ShowSockets( kFalse );
	if( m_pDraggingItem != NULL &&
		m_pDraggingItem->Type() == KItemGem )
	{
		ShowSockets = kTrue;
	}

	m_pInventoryMenu->Render( pD3DDevice,
							  ViewportMatrix,
							  ProjectionMatrix,
							  m_pMouseOverItem,
							  ShowSockets );

	m_pPetInventoryMenu->Render( pD3DDevice,
								 ViewportMatrix,
								 ProjectionMatrix,
								 m_pMouseOverItem,
								 ShowSockets );

	m_pPetSellInventoryMenu->Render( pD3DDevice,
									 ViewportMatrix,
									 ProjectionMatrix,
									 m_pMouseOverItem,
									 ShowSockets );

	m_pStatsMenu->Render( pD3DDevice,
						  ViewportMatrix,
						  ProjectionMatrix );

	m_pSpellMenu->Render( pD3DDevice,
						  ViewportMatrix,
						  ProjectionMatrix );

	m_pSkillMenu->Render( pD3DDevice,
						  ViewportMatrix,
						  ProjectionMatrix );

	m_pQuestMenu->Render( pD3DDevice,
						  *this,
						  ViewportMatrix,
						  ProjectionMatrix );

	m_pJournalMenu->Render( pD3DDevice,
							  *this,
							  ViewportMatrix,
							  ProjectionMatrix );

	m_pPetStatsMenu->Render( pD3DDevice,
							 ViewportMatrix,
							 ProjectionMatrix );

	m_pBuySellMenu->Render( pD3DDevice,
							ViewportMatrix,
							ProjectionMatrix,
							m_pMouseOverItem,
							ShowSockets );

	m_pEnchantMenu->Render( pD3DDevice,
						    ViewportMatrix,
						    ProjectionMatrix );

	m_pConfirmMenu->Render( pD3DDevice );

	m_pTipMenu->Render( pD3DDevice,
					    ViewportMatrix,
					    ProjectionMatrix );

	m_pMessageMenu->Render( pD3DDevice,
						    ViewportMatrix,
						    ProjectionMatrix );

	m_pHelpMenu->Render( pD3DDevice,
					     ViewportMatrix,
					     ProjectionMatrix );

	m_pFateMenu->Render( pD3DDevice,
					     ViewportMatrix,
					     ProjectionMatrix );

	m_pGameOptionsMenu->Render( pD3DDevice,
							   ViewportMatrix,
							   ProjectionMatrix );

	RenderTooltips( pD3DDevice,
					pClient );


	if( m_pDraggingItem != NULL &&
		m_pDraggingItem->HasIcon() )
	{
			m_pDraggingItem->IconSprite()->Render( pD3DDevice,
												   m_pMousePointer->GetScreenX() - 24,
												   m_pMousePointer->GetScreenY() - 24 ); 
		if( !m_pSettings.GetSettings( KSetCustomCursor ) )
		{
			if( m_CursorIsVisible )
			{
				m_CursorIsVisible = kFalse;
				SetCursor( NULL );
				//pD3DDevice->SetCursorProperties( 0, 0, m_pBlankMouseSurface );
			}
		}

	}
	else
	{
		if( m_pSettings.GetSettings( KSetCustomCursor ) )
		{
			if( Targeting() )
			{
				pD3DDevice->SetTexture( 0, m_pPointerSelectMaterial->GetTexture() );
				m_pMousePointer->RenderNoTexture( pD3DDevice );
			}
			else
			{
				m_pMousePointer->Render( pD3DDevice );
			}
		}
		else
		{
			if( Targeting() ||
				!m_CursorIsVisible )
			{
				if( !m_CursorIsTarget )
				{
					SetCursor( LoadCursor( NULL, IDC_CROSS ) );
					//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurfaceSelect );
					m_CursorIsTarget = kTrue;
					m_CursorIsVisible = kTrue;
				}
			}
			else
			{
				if( m_CursorIsTarget||
					!m_CursorIsVisible )
				{
					SetCursor( LoadCursor( NULL, IDC_ARROW ) );
					//pD3DDevice->SetCursorProperties( 0, 0, m_pMouseSurface );
					m_CursorIsTarget = kFalse;
					m_CursorIsVisible = kTrue;
				}
			}

		}
	}

	if( m_ConsoleOpen )
	{
		m_pConsoleText->SetText( pD3DDevice, "?" + m_ConsoleText );
		m_pConsoleText->Render( pD3DDevice );
	}
} // CGameUI::Render()

void CGameUI::RenderTooltips( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device to use
							  CGameClient& pClient )			// game client
{
	// if not mousing over an item, see about tooltips
	if( m_pMouseOverItem == NULL && 
		m_pMouseOverSpell == NULL && 
		m_pDraggingItem == NULL &&
		!Targeting() )
	{
		std::string ToolTip( "" );
		int32 ButtonOver( -1 );
		ButtonOver = m_pGameOptionsMenu->UpdateButtonOver( ToolTip );
		if( ButtonOver == -1 )
		{
			ButtonOver = m_pConfirmMenu->UpdateButtonOver( ToolTip );
		}
		if( ButtonOver == -1 )
		{
			ButtonOver = m_pEnchantMenu->UpdateButtonOver( ToolTip );
		}
		if( ButtonOver == -1 )
		{
			ButtonOver = m_pHelpMenu->UpdateButtonOver( ToolTip );
		}
		if( ButtonOver == -1 )
		{
			ButtonOver = m_pFateMenu->UpdateButtonOver( ToolTip );
		}
		if( ButtonOver == -1 )
		{
			ButtonOver = m_pTipMenu->UpdateButtonOver( ToolTip );
		}
		if( ButtonOver == -1 )
		{
			ButtonOver = m_pMessageMenu->UpdateButtonOver( ToolTip );
		}
		if( !Paused() )
		{
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pBottomMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pInventoryMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pPetInventoryMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pPetSellInventoryMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pStatsMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pSpellMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pSkillMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pQuestMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pJournalMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pPetStatsMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = m_pBuySellMenu->UpdateButtonOver( ToolTip );
			}
			if( ButtonOver == -1 )
			{
				ButtonOver = UpdateButtonOver( ToolTip );
			}
		}
		if( ToolTip.length() > 0 )
		{
			m_pToolTipText->SetText( pD3DDevice, ToolTip );
			m_pToolTipText->SetBold( pD3DDevice, kFalse );

			// first resize our description shadow to fit
			float32 Width = m_pToolTipText->MaxBounds().x -
							m_pToolTipText->MinBounds().x;
			float32 Height = m_pToolTipText->MaxBounds().y -
							 m_pToolTipText->MinBounds().y;

			Width *= .5f;
			Height *= .5f;
			Width += 6;
			Height += 6;

			float32 X = m_pMousePointer->GetScreenX();
			float32 Y = m_pMousePointer->GetScreenY() + 48;


			m_pDescriptionShadow->SetSize( Width * 2, Height * 2 );

			if( X - Width < 0 )
			{
				X = Width;
			}
			if( Y < 0 )
			{
				Y = 0;
			}
			if( X + Width > 1024 )
			{
				X = 1024 - Width;
			}
			if( Y + Height * 2.0f > 768 )
			{
				Y = 768 - Height * 2.0f;
			}
			if( Y < m_pMousePointer->GetScreenY() + 48 )
			{
				Y = m_pMousePointer->GetScreenY() - Height * 2.0f;
			}
			if( Y < 0 )
			{
				Y = 0;
			}

			m_pToolTipText->SetPosition( X, Y );

			m_pDescriptionShadow->SetPosition( X, Y + ( m_pToolTipText->MaxBounds().y -
														m_pToolTipText->MinBounds().y ) * .5f );

			m_pDescriptionShadow->Render( pD3DDevice );

			m_pToolTipText->Render( pD3DDevice );
		}
	}
	else
	{
		// all the highlights we need to render
		std::vector< EHighlightColor > Highlights;
		float32 CharacterHeight = m_pFontMetric->CharacterHeight();
		CItem* pItem = m_pMouseOverItem;
		if( m_pDraggingItem != NULL )
		{
			pItem = m_pDraggingItem;
		}

		std::string Description;

		if( Targeting() )	// display helper text when targeting items or spells
		{
			if( m_pSpellInUse != NULL )
			{
				if( m_pMouseOverItem != NULL &&
					m_pSpellInUse->CanCastOnItem( m_pMouseOverItem) )
				{
					if( !m_pSpellInUse->SufficientLevelToCastOnItem( m_pPlayerCharacter,
																	 m_pMouseOverItem ) )
					{
						Description = LoadLocalizedString3Variables(824, KMagicSphereShortName[m_pSpellInUse->Sphere()], m_pSpellInUse->Name(), m_pMouseOverItem->BaseModifiedName() );
						Highlights.push_back( KHighlightRed );
						Highlights.push_back( KHighlightColors );
					}
					else
					{
						Description = LoadLocalizedString2Variables(825, m_pSpellInUse->Name(), m_pMouseOverItem->BaseModifiedName() );
						Highlights.push_back( KHighlightBlue );
						Highlights.push_back( KHighlightColors );
					}
				}
				else
				{
					Description = LoadLocalizedString1Variable(826, m_pSpellInUse->Name() );
					Highlights.push_back( KHighlightColors );
					Highlights.push_back( KHighlightColors );
				}
			}
			else if( m_pItemInUse != NULL )
			{
				if( m_pMouseOverItem != NULL &&
					m_pItemInUse->CanUseOnItem( m_pMouseOverItem ) )
				{
					Description = LoadLocalizedString2Variables(827, m_pItemInUse->UseDescription(), m_pMouseOverItem->BaseModifiedName() );
					Highlights.push_back( KHighlightBlue );
					Highlights.push_back( KHighlightColors );
				}
				else
				{
					Description = LoadLocalizedString1Variable(828, m_pItemInUse->UseDescription() );
					Highlights.push_back( KHighlightColors );
					Highlights.push_back( KHighlightColors );
				}
			}
		}
		else if( m_pMouseOverSpell != NULL )
		{
			int32 ButtonOver = m_pSpellMenu->UpdateButtonOver();
			if( m_pMouseOverSpell == m_pPlayerCharacter->ActiveSpell() &&
				ButtonOver == KActiveSpellButton )
			{
				if( m_pMouseOverSpell->Target() == KTargetCharacter ||
					m_pMouseOverSpell->Target() == KTargetArea ||
					m_pMouseOverSpell->Target() == KTargetProjectile )
				{
					Description = LoadLocalizedString(829);
				}
				else
				{
					Description = LoadLocalizedString(830);
				}
				Highlights.push_back( KHighlightColors );
			}
			else
			{
				if( m_pMouseOverSpell->Target() == KTargetCharacter ||
					m_pMouseOverSpell->Target() == KTargetArea ||
					m_pMouseOverSpell->Target() == KTargetProjectile )
				{
					Description = LoadLocalizedString(831);
					Highlights.push_back( KHighlightColors );
					Highlights.push_back( KHighlightColors );
				}
				else
				{
					Description = LoadLocalizedString(832);
					Highlights.push_back( KHighlightColors );
					Highlights.push_back( KHighlightColors );
				}
			}
			Description = Description + m_pMouseOverSpell->Name();
			Highlights.push_back( KHighlightLightPurple );
			
			if( m_pMouseOverSpell->Sphere() == KMagicAttack )
			{
				Description = Description + LoadLocalizedString(833);
				Highlights.push_back( KHighlightRed );
			}
			else if( m_pMouseOverSpell->Sphere() == KMagicDefense )
			{
				Description = Description + LoadLocalizedString(834);
				Highlights.push_back( KHighlightBlue );
			}
			else
			{
				Description = Description + LoadLocalizedString(835);
				Highlights.push_back( KHighlightGreen );
			}

			int32 SkillBonus( 0 );
			switch( m_pMouseOverSpell->Sphere() )
			{
			case KMagicAttack :
				SkillBonus = m_pPlayerCharacter->SkillPoints( KSkillAttackMagic );
				break;
			case KMagicDefense :
				SkillBonus = m_pPlayerCharacter->SkillPoints( KSkillDefenseMagic );
				break;
			case KMagicCharm :
				SkillBonus = m_pPlayerCharacter->SkillPoints( KSkillCharmMagic );
				break;
			}

			Description = Description + LoadLocalizedString(836) + KTargetDescription[m_pMouseOverSpell->Target()];
			Highlights.push_back( KHighlightColors );

			Description = Description + LoadLocalizedString1Variable(837, IntegerToString( (int32)ceil( m_pMouseOverSpell->CastTime() ) ) );
			Highlights.push_back( KHighlightColors );

			Description = Description + LoadLocalizedString(838) + IntegerToString( m_pMouseOverSpell->ManaCost() );
			Highlights.push_back( KHighlightBlue );

			int32 Duration( 0 );
			for( uint32 i = 0; i < m_pMouseOverSpell->Effects(); i++ )
			{
				CEffect* pEffect = m_pMouseOverSpell->Effect( i );
				int32 EffectDuration = pEffect->Duration() + pEffect->DurationBonus() * SkillBonus;
				EffectDuration += (int32)( (float32)EffectDuration * pEffect->DurationBonusPercent() / 100.0f * SkillBonus );

				if( EffectDuration > Duration &&
					pEffect->Type() != KEffectKnockbackEffect )
				{
					Duration = EffectDuration;
				}
			}
			if( Duration == 0 )
			{
				Description = Description + LoadLocalizedString(839);
			}
			else
			{
				Description = Description + LoadLocalizedString1Variable(840, IntegerToString( Duration ) );
			}
			Highlights.push_back( KHighlightLightPurple );

			for( i = 0; i < m_pMouseOverSpell->Effects(); i++ )
			{
				CEffect* pEffect = m_pMouseOverSpell->Effect( i );
				int32 EffectDuration = pEffect->Duration() + pEffect->DurationBonus() * SkillBonus;
				EffectDuration += (int32)( (float32)EffectDuration * pEffect->DurationBonusPercent() / 100.0f * SkillBonus );
				if( EffectDuration < 0 )
				{
					EffectDuration = 0;
				}

				if( pEffect->Value() != 0 || pEffect->Value2() != 0 )
				{
					Highlights.push_back( KHighlightLightPurple );

					// if this effect has values with range
					if( KEffectHasValue[ pEffect->Type() ] )
					{

						int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
						int32 Value2 = (int32)( pEffect->Value2() + pEffect->Value2Bonus() * (float32)SkillBonus );
						Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
						Value2 += Value2 * (int32)( pEffect->Value2BonusPercent() / 100.0f * (float32)SkillBonus );

						// boost HP damages based upon Magic stat
						if( pEffect->Type() == KEffectHP &&
							pEffect->Value() < 0 &&
							m_pMouseOverSpell->TargetAlignment() == KEvil )
						{
							float32 NewValue = (float32)Value;
							NewValue = ( NewValue * ( m_pPlayerCharacter->Magic() + 100 ) / 100.0f );


							NewValue -= (int32)ceil( NewValue * (float32)m_pPlayerCharacter->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							NewValue -= (int32)m_pPlayerCharacter->EffectValue( KEffectDamageBonus );
							Value = (int32)NewValue;

							NewValue = (float32)Value2;
							NewValue = ( NewValue * ( m_pPlayerCharacter->Magic() + 100 ) / 100.0f );


							NewValue -= (int32)ceil( NewValue * (float32)m_pPlayerCharacter->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							NewValue -= (int32)m_pPlayerCharacter->EffectValue( KEffectDamageBonus );
							Value2 = (int32)NewValue;
						}


						if( Value > 0 )
						{
							Description = Description + "\n+";
						}
						else
						{
							Description = Description + "\n";
						}
						if( pEffect->ChanceOfSuccess() != 100 )
						{
							int32 Chance = (int32)( pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonus() * (float32)SkillBonus );
							Chance += Chance * (int32)( pEffect->ChanceOfSuccessBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString1Variable(841, IntegerToString( Chance ));
						}
						if( pEffect->RangedValue() )
						{
							bool Positive( kTrue );
							if( sgn( (float32)Value ) != sgn( (float32)KEffectMaximum[pEffect->Type()] ) )
							{
								Positive = kFalse;
							}
							Value = abs( Value );
							Value2 = abs( Value2 );
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Value = (int32)( (float32)Value / 60 );
							}
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Value2 = (int32)( (float32)Value2 / 60 );
							}
							// swap them for appropriate ordering
							if( Value > Value2 )
							{
								int32 Temp = Value;
								Value = Value2;
								Value2 = Temp;
							}
							if( Positive )
							{
								Description = Description + IntegerToString( Value ) + "-" + IntegerToString( Value2 ) + " " + KEffectSpellPositiveDescriptions[pEffect->Type()];
							}
							else
							{
								Description = Description + IntegerToString( Value ) + "-" + IntegerToString( Value2 ) + " " + KEffectSpellNegativeDescriptions[pEffect->Type()];
							}

							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Description = Description + LoadLocalizedString1Variable(842, "");
							}
						}
						else
						{
							bool Positive( kTrue );
							if( sgn( (float32)Value ) != sgn( (float32)KEffectMaximum[pEffect->Type()] ) )
							{
								Positive = kFalse;
							}
							Value = abs( Value );
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Value = (int32)( (float32)Value / 60 );
							}
							if( Positive )
							{
								Description = Description + IntegerToString( Value ) + " " + KEffectSpellPositiveDescriptions[pEffect->Type()];
							}
							else
							{
								Description = Description + IntegerToString( Value ) + " " + KEffectSpellNegativeDescriptions[pEffect->Type()];
							}

							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Description = Description + LoadLocalizedString1Variable(842, "");
							}
						}
					}
					else	// otherwise we just use its description
					{
						Description = Description + "\n";
						if( pEffect->ChanceOfSuccess() != 100 )
						{
							int32 Chance = (int32)( pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonus() * (float32)SkillBonus );
							Chance += Chance * (int32)( pEffect->ChanceOfSuccessBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString1Variable(843, IntegerToString( Chance ) );
						}


						Description = Description +  KEffectSpellPositiveDescriptions[pEffect->Type()];

						if( pEffect->Type() == KEffectFlee )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(844, "", IntegerToString( Value ));
						}
						if( pEffect->Type() == KEffectDiscover )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(845, "", IntegerToString( Value ));
						}
						if( pEffect->Type() == KEffectOpenPortal )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(845, "", IntegerToString( Value ));
						}
						if( pEffect->Type() == KEffectIdentify )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(846, "",IntegerToString( Value ));
						}
						if( pEffect->Type() == KEffectTurnAlignment )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(847, "", IntegerToString( Value ));
						}
						if( pEffect->Type() == KEffectSummon )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							int32 Value2 = (int32)( pEffect->Value2() + pEffect->Value2Bonus() * (float32)SkillBonus );
							Value2 += Value2 * (int32)( pEffect->Value2BonusPercent() / 100.0f * (float32)SkillBonus );
							int32 Value3 = (int32)( pEffect->Value3() + pEffect->Value3Bonus() * (float32)SkillBonus );
							Value3 += Value3 * (int32)( pEffect->Value3BonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString3Variables(848, "", IntegerToString( Value2 ), IntegerToString( Value3 ));
						}

					}
				}
			}


			for( i = 0; i < m_pMouseOverSpell->PayloadEffects(); i++ )
			{
				CEffect* pEffect = m_pMouseOverSpell->PayloadEffect( i );
				int32 EffectDuration = pEffect->Duration() + pEffect->DurationBonus() * SkillBonus;
				EffectDuration += (int32)( (float32)EffectDuration * pEffect->DurationBonusPercent() / 100.0f * SkillBonus );
				if( EffectDuration < 0 )
				{
					EffectDuration = 0;
				}

				if( pEffect->Value() != 0 || pEffect->Value2() != 0 )
				{
					Highlights.push_back( KHighlightLightPurple );

					// if this effect has values with range
					if( KEffectHasValue[ pEffect->Type() ] )
					{

						int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
						int32 Value2 = (int32)( pEffect->Value2() + pEffect->Value2Bonus() * (float32)SkillBonus );
						Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
						Value2 += Value2 * (int32)( pEffect->Value2BonusPercent() / 100.0f * (float32)SkillBonus );

						// boost HP damages based upon Magic stat
						if( pEffect->Type() == KEffectHP &&
							pEffect->Value() < 0 &&
							m_pMouseOverSpell->TargetAlignment() == KEvil )
						{
							float32 NewValue = (float32)Value;
							NewValue = ( NewValue * ( m_pPlayerCharacter->Magic() + 100 ) / 100.0f );


							NewValue -= (int32)ceil( NewValue * (float32)m_pPlayerCharacter->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							NewValue -= (int32)m_pPlayerCharacter->EffectValue( KEffectDamageBonus );
							Value = (int32)NewValue;

							NewValue = (float32)Value2;
							NewValue = ( NewValue * ( m_pPlayerCharacter->Magic() + 100 ) / 100.0f );


							NewValue -= (int32)ceil( NewValue * (float32)m_pPlayerCharacter->EffectValue( KEffectPercentDamageBonus ) / -100.0f );
							NewValue -= (int32)m_pPlayerCharacter->EffectValue( KEffectDamageBonus );
							Value2 = (int32)NewValue;
						}

						if( Value > 0 )
						{
							Description = Description + "\n+";
						}
						else
						{
							Description = Description + "\n";
						}
						if( pEffect->ChanceOfSuccess() != 100 )
						{
							int32 Chance = (int32)( pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonus() * (float32)SkillBonus );
							Chance += Chance * (int32)( pEffect->ChanceOfSuccessBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString1Variable(841, IntegerToString( Chance ) );
						}
						if( pEffect->RangedValue() )
						{
							Value = abs( Value );
							Value2 = abs( Value2 );
							// swap them for appropriate ordering
							if( Value > Value2 )
							{
								int32 Temp = Value;
								Value = Value2;
								Value2 = Temp;
							}
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Value = (int32)( (float32)Value / 60 );
							}
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Value2 = (int32)( (float32)Value2 / 60 );
							}
							if( m_pMouseOverSpell->Sphere() == KMagicAttack )
							{
								Description = Description + IntegerToString( Value ) + "-" + IntegerToString( Value2 ) + " " + KEffectSpellNegativeDescriptions[pEffect->Type()];
							}
							else
							{
								Description = Description + IntegerToString( Value ) + "-" + IntegerToString( Value2 ) + " " + KEffectSpellPositiveDescriptions[pEffect->Type()];
							}
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Description = Description + LoadLocalizedString1Variable(842, "");
							}

						}
						else
						{
							Value = abs( Value );
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Value = (int32)( (float32)Value / 60 );
							}
							if( m_pMouseOverSpell->Sphere() == KMagicAttack )
							{
								Description = Description + IntegerToString( Value ) + " " + KEffectSpellNegativeDescriptions[pEffect->Type()];
							}
							else
							{
								Description = Description + IntegerToString( Value ) + " " + KEffectSpellPositiveDescriptions[pEffect->Type()];
							}
							if( EffectDuration != 0 &&
								KEffectIsRecharge[pEffect->Type()] )
							{
								Description = Description + LoadLocalizedString1Variable(842, "");
							}
						}
					}
					else	// otherwise we just use its description
					{
						Description = Description + "\n";
						if( pEffect->ChanceOfSuccess() != 100 )
						{
							int32 Chance = (int32)( pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonus() * (float32)SkillBonus );
							Chance += Chance * (int32)( pEffect->ChanceOfSuccessBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString1Variable(843, IntegerToString( Chance ) );
						}


						Description = Description + KEffectSpellPositiveDescriptions[pEffect->Type()];

						if( pEffect->Type() == KEffectDiscover )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(845, "", IntegerToString( Value ) );
						}
						if( pEffect->Type() == KEffectFlee )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(844, "", IntegerToString( Value ));
						}
						if( pEffect->Type() == KEffectOpenPortal )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(845, "", IntegerToString( Value ) );
						}
						if( pEffect->Type() == KEffectIdentify )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(846, "", IntegerToString( Value ) );
						}
						if( pEffect->Type() == KEffectTurnAlignment )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString2Variables(847, "", IntegerToString( Value ) );
						}
						if( pEffect->Type() == KEffectSummon )
						{
							int32 Value = (int32)( pEffect->Value() + pEffect->ValueBonus() * (float32)SkillBonus );
							Value += Value * (int32)( pEffect->ValueBonusPercent() / 100.0f * (float32)SkillBonus );
							int32 Value2 = (int32)( pEffect->Value2() + pEffect->Value2Bonus() * (float32)SkillBonus );
							Value2 += Value2 * (int32)( pEffect->Value2BonusPercent() / 100.0f * (float32)SkillBonus );
							int32 Value3 = (int32)( pEffect->Value3() + pEffect->Value3Bonus() * (float32)SkillBonus );
							Value3 += Value3 * (int32)( pEffect->Value3BonusPercent() / 100.0f * (float32)SkillBonus );
							Description = Description + LoadLocalizedString3Variables(848, "", IntegerToString( Value2 ), IntegerToString( Value3 ) );
						}
					}
				}
			}

			Description = Description + "\n";
			Highlights.push_back( KHighlightColors );
			
			for( i = 0; i < m_pMouseOverSpell->Requirements(); i++ )
			{
				if( m_pMouseOverSpell->RequirementStat( i ) == KStatLevel )
				{
					Description = Description + LoadLocalizedString1Variable(849,
								  KStatisticDescription[m_pMouseOverSpell->RequirementStat( i )] + " " + 
								  IntegerToString( m_pMouseOverSpell->RequirementValue( i ) ));
				}
				else
				{
					Description = Description + LoadLocalizedString1Variable(849,
								  IntegerToString( m_pMouseOverSpell->RequirementValue( i ) ) + " " +
								  KStatisticDescription[m_pMouseOverSpell->RequirementStat( i )]);
				}
				if( m_pPlayerCharacter->RequirementMet( m_pMouseOverSpell->RequirementStat( i ),
														m_pMouseOverSpell->RequirementValue( i ) ) )
				{
					Highlights.push_back( KHighlightGreen );
				}
				else
				{
					Highlights.push_back( KHighlightRed );
				}
			}
			Highlights.push_back( KHighlightColors );


			Description = Description + "\n\n"+ m_pMouseOverSpell->UseDescription();
		}
		else if( m_pMouseOverItem != NULL || 
				 m_pDraggingItem != NULL )
		{
			// determine if we need to show prices
			bool Selling( kFalse );
			bool Buying( kFalse );
			if( m_pBuySellMenu->Open() )
			{
				if( m_pItemOwner == m_pPlayerCharacter ||
					m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) )
				{
					if( m_pItemOwner->Target()->MerchantType() != KMerchantStash )
					{
						Selling = kTrue;
					}
				}
				else
				{
					if( m_pItemOwner->MerchantType() != KMerchantStash )
					{
						Buying = kTrue;
					}
				}
			}

			bool Gambling( kFalse );
			
			if( Buying )
			{
				if( m_pItemOwner->MerchantType() == KMerchantGambler &&
					!pItem->Identified() )
				{
					Gambling = kTrue;
					Description = LoadLocalizedString(850) + IntegerToString( pItem->GambleValue() ) + "\n";
					if( pItem->GambleValue() <= m_pPlayerCharacter->Inventory()->Gold() )
					{
						Highlights.push_back( KHighlightYellow );
					}
					else
					{
						Highlights.push_back( KHighlightRed );
					}
				}
				else
				{
					Description = LoadLocalizedString(851) + IntegerToString( pItem->PurchaseValue() ) + "\n";
					if( pItem->PurchaseValue() <= m_pPlayerCharacter->Inventory()->Gold() )
					{
						Highlights.push_back( KHighlightYellow );
					}
					else
					{
						Highlights.push_back( KHighlightRed );
					}
				}
			}
			else if( Selling )
			{
				Description = LoadLocalizedString(852) + IntegerToString( pItem->SellValue() ) + "\n";
				Highlights.push_back( KHighlightYellow );
			}

			std::string DisplayedName( "" );
			if( ( pItem->Effects( KActivationPassive ) > 0 ||
				  pItem->DamageBonuses() > 0 )
				  && !pItem->Identified() )
			{
				DisplayedName = pItem->BaseModifiedName();
				Description = Description + DisplayedName;
			}
			else
			{
				DisplayedName = pItem->Name();
				Description = Description + DisplayedName;
			}
			// magic items get purple highlight
			if( ( pItem->Effects( KActivationPassive ) > 0 ||
				  pItem->DamageBonuses() > 0 ) &&
				!Gambling )
			{
				if( pItem->IsArtifact() )
				{
					Highlights.push_back( KHighlightYellow );
				}
				else if( pItem->IsUnique() )
				{
					Highlights.push_back( KHighlightTeal );
				}
				else 
				{
					Highlights.push_back( KHighlightLightPurple );
				}

				int32 ExtraLines( 0 );
				int32 Line( -1 );
				while( ( Line = DisplayedName.find( "\n", Line + 1 ) ) != -1 )
				{
					ExtraLines++;
				}
				for( int32 l = 0; l < ExtraLines; l++ )
				{
					if( pItem->IsArtifact() )
					{
						Highlights.push_back( KHighlightYellow );
					}
					else if( pItem->IsUnique() )
					{
						Highlights.push_back( KHighlightTeal );
					}
					else 
					{
						Highlights.push_back( KHighlightLightPurple );
					}
				}
			}
			else	// others get nothing
			{
				Highlights.push_back( KHighlightColors );
			}
			if( pItem->Socketed() &&
				!Gambling  )
			{
				if( pItem->OpenSockets() > 1 )
				{
					Description = Description + LoadLocalizedString1Variable(853, IntegerToString( pItem->OpenSockets() ) );
					Highlights.push_back( KHighlightGray );
				}
				else if( pItem->OpenSockets() == 1 )
				{
					Description = Description + LoadLocalizedString(854);
					Highlights.push_back( KHighlightGray );
				}
			}
			if( pItem->Category() == KCategoryWeapon )
			{
				if( KItemTwoHanded[pItem->Type()] )
				{
					Description = Description + LoadLocalizedString(855);
					Highlights.push_back( KHighlightColors );
				}

				Description = Description + "\n" + KDamageTypeDescription[KItemDamageType[pItem->Type()]];
				Highlights.push_back( KHighlightColors );

				if( pItem->Identified() )
				{
					for( uint32 i = 0; i < pItem->DamageBonuses(); i++ )
					{
						Description = Description + "\n+" + IntegerToString( pItem->DamageBonusValue( i ) ) + " " + 
									  KDamageTypeDescription[pItem->DamageBonusType( i )];
						Highlights.push_back( KHighlightLightPurple );
					}
				}

				Description = Description + LoadLocalizedString(856) +
							  IntegerToString( pItem->MinimumDamage() ) + " - " + IntegerToString( pItem->MaximumDamage() ) + 
							  "\n" + KItemTypeDescription[pItem->Type()] + " - " + KAttackSpeedName[pItem->Speed()];

				Highlights.push_back( KHighlightColors );
				Highlights.push_back( KHighlightColors );
			}
			else if( pItem->Category() == KCategoryArmor )
			{
				Description = Description + LoadLocalizedString(857) +
							  IntegerToString( pItem->ArmorBonus() ) + 
							  "\n" + KItemTypeDescription[pItem->Type()];
				Highlights.push_back( KHighlightColors );
				Highlights.push_back( KHighlightColors );

				if( pItem->Identified() )
				{
					for( uint32 i = 0; i < pItem->DamageBonuses(); i++ )
					{
						Description = Description + "\n+" + IntegerToString( pItem->DamageBonusValue( i ) ) + " " + 
									  KDamageTypeDescription[pItem->DamageBonusType( i )];
						Highlights.push_back( KHighlightLightPurple );
					}
				}
			}
			else if( pItem->Category() == KCategoryJewelry )
			{
				Description = Description + "\n" + KItemTypeDescription[pItem->Type()];
				Highlights.push_back( KHighlightColors );

				if( pItem->Identified() )
				{
					for( uint32 i = 0; i < pItem->DamageBonuses(); i++ )
					{
						Description = Description + "\n+" + IntegerToString( pItem->DamageBonusValue( i ) ) + " " + 
									  KDamageTypeDescription[pItem->DamageBonusType( i )];
						Highlights.push_back( KHighlightLightPurple );
					}
				}
			}
			uint32 Effects = pItem->Effects( KActivationPassive );
			if( Effects > 0 ||
				pItem->DamageBonuses() > 0 )
			{
				if( pItem->Type() == KItemGem )
				{
					Description = Description + LoadLocalizedString(858);
					Highlights.push_back( KHighlightGray );
					for( uint32 i = 0; i < pItem->Effects( KActivationPassive ); i++ )
					{
						CEffect* pEffect = pItem->Effect( KActivationPassive, i );
						// elemental damage is added slightly differently from other effects
						if( pEffect->Type() == KEffectAddDamageType )
						{
							EDamageType DamageType( KDamageFire );
							std::string EffectName = StringUpper( pEffect->Name() );
							for( uint32 a = 0; a < KDamageTypes; a++ )
							{
								if( EffectName == KDamageNames[a] )
								{
									DamageType = (EDamageType)a;
								}
							}
							Description = Description + LoadLocalizedString2Variables(859, IntegerToString( (int32)pEffect->Value() * 4 ), KDamageResistanceDescription[DamageType] );
							Description = Description + LoadLocalizedString2Variables(860, IntegerToString( (int32)pEffect->Value() ), KDamageTypeDescription[DamageType] );
							Highlights.push_back( KHighlightDarkPurple );
							Highlights.push_back( KHighlightDarkPurple );
						}
						else
						{
							bool Positive( kTrue );
							int32 Value = (int32)pEffect->Value();
							if( sgn( (float32)Value ) != sgn( (float32)KEffectMaximum[pEffect->Type()] ) )
							{
								Positive = kFalse;
							}
							Value = abs( Value );
							Description = Description + LoadLocalizedString2Variables(861, "", "");
							if( Positive )
							{
								if( KEffectBonusType[pEffect->Type()] == KBonusValue )
								{
									Description = Description + "+" + IntegerToString( Value ) +
										" " + KEffectItemPositiveDescriptions[pEffect->Type()];
								}
								else
								{
									Description = Description + IntegerToString( Value ) +
										KEffectItemPositiveDescriptions[pEffect->Type()];
								}
							}
							else
							{
								if( KEffectBonusType[pEffect->Type()] == KBonusValue )
								{
									Description = Description + "-" + IntegerToString( Value ) +
										" " + KEffectItemNegativeDescriptions[pEffect->Type()];
								}
								else
								{
									Description = Description + IntegerToString( Value ) +
										KEffectItemNegativeDescriptions[pEffect->Type()];
								}
							}
							Description = Description + LoadLocalizedString1Variable(862, "");

							if( Positive )
							{
								Highlights.push_back( KHighlightDarkPurple );
							}
							else
							{
								Highlights.push_back( KHighlightDarkRed );
							}
						}
					}
				}
				else
				{
					if( pItem->Identified() )
					{
						if( Effects > 0 )
						{
							Description = Description + "\n" + pItem->EffectText( KActivationPassive );
							for( uint32 i = 0; i < Effects; i++ )
							{
								if( sgn( pItem->Effect( KActivationPassive, i )->Value() ) != 
									sgn( (float32)KEffectMaximum[pItem->Effect( KActivationPassive, i )->Type()] ) )
								{
									Highlights.push_back( KHighlightDarkRed );
								}
								else
								{
									Highlights.push_back( KHighlightDarkPurple );
								}
							}
						}
					}
					else if( !Gambling )
					{
						Description = Description + LoadLocalizedString1Variable(863, IntegerToString( pItem->BaseLevel() ));
						Highlights.push_back( KHighlightBlue );
						Highlights.push_back( KHighlightBlue );
					}
				}
			}
			for( uint32 i = 0; i < pItem->Requirements(); i++ )
			{
				int32 RequirementValue = pItem->RequirementValue( i );
				if( pItem->Category() == KCategoryWeapon ||
					pItem->Category() == KCategoryArmor ||
					pItem->Category() == KCategoryJewelry )
				{
					float32 RequirementReduction = m_pPlayerCharacter->EffectValue( KEffectPercentItemRequirements );
					if( RequirementReduction > 75 )
					{
						RequirementReduction = 75;
					}
					if( pItem->RequirementStat( i ) != KStatRenown )
					{
						RequirementValue =(int32)( (float32)RequirementValue - (float32)RequirementValue * RequirementReduction / 100.0f );
					}
				}

				if( pItem->RequirementStat( i ) == KStatRenown )
				{
					Description = Description + LoadLocalizedString1Variable(864,
								  KFameName[RequirementValue]);
				}
				else if( pItem->RequirementStat( i ) == KStatLevel )
				{
					Description = Description + LoadLocalizedString2Variables(865,
								  KStatisticDescription[pItem->RequirementStat( i )],
								  IntegerToString( RequirementValue ));
				}
				else
				{
					Description = Description + LoadLocalizedString2Variables(866,
								  KStatisticDescription[pItem->RequirementStat( i )],
								  IntegerToString( RequirementValue ));
				}
				if( m_pPlayerCharacter->RequirementMet( pItem->RequirementStat( i ),
														pItem->RequirementValue( i ),
														pItem->Type() != KItemSpell ) )
				{
					Highlights.push_back( KHighlightGreen );
				}
				else
				{
					Highlights.push_back( KHighlightRed );
				}
			}

			if( pItem->Useable() )
			{
				if( pItem->Type() == KItemSpell )
				{
					bool CanWrite( kTrue );
					std::string SpellDescription = "";
					std::string WriteDescription = LoadLocalizedString(867);

					// can't write it if we don't own it
					if( !( m_pItemOwner == m_pPlayerCharacter ||
							( m_pPlayerCharacter->Pets() > 0 &&
							m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) ) ) )
					{
						CanWrite = kFalse;
						WriteDescription = "";
					}

					for( uint32 i = 0; i < pItem->Effects( KActivationUsage ); i++ )
					{
						CEffect* pEffect = pItem->Effect( KActivationUsage, i );
						// elemental damage is added slightly differently from other effects
						if( pEffect->Type() == KEffectWriteSpell )
						{
							CSpellDescription* pSpell = pClient.SpellManifest()->SpellDescription( pEffect->Name() );
							if( pSpell != NULL )
							{

								SpellDescription = "\n\n" + pSpell->UseDescription();
								if( m_pPlayerCharacter->HasSpell( pSpell->Name() ) )
								{
									CanWrite = kFalse;
									WriteDescription = LoadLocalizedString(868);
								}
								else if( m_pPlayerCharacter->FreeSpellSlot( pSpell->Sphere() ) == -1 )
								{
									CanWrite = kFalse;
									WriteDescription = LoadLocalizedString2Variables(869, KMagicSphereShortName[pSpell->Sphere()], KMagicSphereShortName[pSpell->Sphere()]);
									Highlights.push_back( KHighlightRed );
								}
							}
						}
					}
					if( WriteDescription.length() > 0 )
					{
						if( CanWrite )
						{
							Highlights.push_back( KHighlightLightPurple );
						}
						else
						{
							Highlights.push_back( KHighlightRed );
						}
					}

					Description = Description + WriteDescription;
					Description = Description + SpellDescription;
				}

			}
			if( pItem->Useable() )
			{
				if( pItem->Type() == KItemSpell )
				{
				}
				else
				{
					bool IsTownPortal( kFalse );
					for( uint32 e = 0; e < pItem->Effects( KActivationUsage ); e++ )
					{
						if( pItem->Effect( KActivationUsage, e )->Type() == KEffectOpenPortal )
						{
							IsTownPortal = kTrue;
						}
					}
					if( IsTownPortal && pClient.Level()->LevelDepth() == 0 )
					{
						if( pItem->Uses() > 0 )
						{
							Description = Description + LoadLocalizedString(870) + IntegerToString( pItem->Uses() );
							Highlights.push_back( KHighlightDarkPurple );
						}
						if( m_pItemOwner == m_pPlayerCharacter ||
						  ( m_pPlayerCharacter->Pets() > 0 &&
							m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) ) )
						{
							Description = Description + LoadLocalizedString(871);
							Highlights.push_back( KHighlightRed );
						}
					}
					else
					{
						if( pItem->Uses() > 0 )
						{
							Description = Description + LoadLocalizedString(870) + IntegerToString( pItem->Uses() );
							Highlights.push_back( KHighlightDarkPurple );
						}
						if( pItem->Type() == KItemPetFood /*&&
							!( m_pItemOwner->IsMerchant() &&
							   m_pItemOwner->MerchantType() == KMerchantFish )*/ )
						{
							if( m_pItemOwner == m_pPlayerCharacter ||
							    ( m_pPlayerCharacter->Pets() > 0 &&
							 	  m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) ) )
							{
								Description = Description + LoadLocalizedString(872);
								Highlights.push_back( KHighlightBlue );
							}

							for( uint32 i = 0; i < pItem->Effects( KActivationUsage ); i++ )
							{
								CEffect* pEffect = pItem->Effect( KActivationUsage, i );
								// elemental damage is added slightly differently from other effects
								if( pEffect->Type() == KEffectRevertTransform )
								{
									Description = Description + LoadLocalizedString(873);
									Highlights.push_back( KHighlightDarkPurple );
								}
								if( pEffect->Type() == KEffectTransform )
								{
									CCharacterDescription * pCharacterDescription = pClient.CharacterManifest()->CharacterDescription( pEffect->Name() );
									if( pCharacterDescription != NULL )
									{
										Description = Description + LoadLocalizedString1Variable(874, pCharacterDescription->Name());
										if( pEffect->Value() == -1 )
										{
											Description = LoadLocalizedString1Variable(875, Description);
										}
										else
										{
											Description = LoadLocalizedString2Variables(876, Description, IntegerToString( (int32)pEffect->Value() ) );
										}
										Highlights.push_back( KHighlightDarkPurple );
									}

									CCharacterDescription* pOriginalDescription = pClient.CharacterManifest()->CharacterDescription( m_pPlayerCharacter->Pet( 0 )->TemplateName() );

									if( pOriginalDescription != NULL &&
										pCharacterDescription != NULL )
									{
										int32 StrengthDelta = pCharacterDescription->Strength() - pOriginalDescription->Strength();
										int32 DexterityDelta = pCharacterDescription->Dexterity() - pOriginalDescription->Dexterity();
										int32 VitalityDelta = pCharacterDescription->Vitality() - pOriginalDescription->Vitality();
										int32 MagicDelta = pCharacterDescription->Magic() - pOriginalDescription->Magic();
										int32 ArmorDelta = pCharacterDescription->NaturalArmor() - pOriginalDescription->NaturalArmor();
										if( StrengthDelta >=0 )
										{
											Highlights.push_back( KHighlightGreen );
											Description = Description + LoadLocalizedString1Variable(877, IntegerToString( StrengthDelta ) );
										}
										else
										{
											Highlights.push_back( KHighlightRed );
											Description = Description + LoadLocalizedString1Variable(878, IntegerToString( StrengthDelta ) );
										}
										if( DexterityDelta >=0 )
										{
											Highlights.push_back( KHighlightGreen );
											Description = Description + LoadLocalizedString1Variable(879, IntegerToString( DexterityDelta ) );
										}
										else
										{
											Highlights.push_back( KHighlightRed );
											Description = Description + LoadLocalizedString1Variable(880, IntegerToString( DexterityDelta ) );
										}
										if( VitalityDelta >=0 )
										{
											Highlights.push_back( KHighlightGreen );
											Description = Description + LoadLocalizedString1Variable(881, IntegerToString( VitalityDelta ) );
										}
										else
										{
											Description = Description + LoadLocalizedString1Variable(882, IntegerToString( VitalityDelta ) );
											Highlights.push_back( KHighlightRed );
										}
										if( MagicDelta >=0 )
										{
											Description = Description + LoadLocalizedString1Variable(883, IntegerToString( MagicDelta ) );
											Highlights.push_back( KHighlightGreen );
										}
										else
										{
											Description = Description + LoadLocalizedString1Variable(884, IntegerToString( MagicDelta ) );
											Highlights.push_back( KHighlightRed );
										}
										if( ArmorDelta >=0 )
										{
											Description = Description + LoadLocalizedString1Variable(885, IntegerToString( ArmorDelta ) );
											Highlights.push_back( KHighlightGreen );
										}
										else
										{
											Description = Description + LoadLocalizedString1Variable(886, IntegerToString( ArmorDelta ) );
											Highlights.push_back( KHighlightRed );
										}
									}
								}
							}
				
						}
						else
						{
							if( m_pItemOwner == m_pPlayerCharacter ||
							    ( m_pPlayerCharacter->Pets() > 0 &&
								  m_pItemOwner == m_pPlayerCharacter->Pet( 0 ) ) )
							{
								Description = Description + LoadLocalizedString(887);
								Highlights.push_back( KHighlightBlue );
							}
						}
					}
				}
			}

		}

		m_pInventoryItemDescription->SetText( pD3DDevice, Description );

		// first resize our description shadow to fit
		float32 Width = m_pInventoryItemDescription->MaxBounds().x -
						m_pInventoryItemDescription->MinBounds().x;
		float32 Height = m_pInventoryItemDescription->MaxBounds().y -
						 m_pInventoryItemDescription->MinBounds().y;

		Width *= .5f;
		Height *= .5f;
		Width += 6;
		Height += 6;

		float32 X = m_pMousePointer->GetScreenX();
		float32 Y = m_pMousePointer->GetScreenY() + 48;


		m_pDescriptionShadow->SetSize( Width * 2, Height * 2 );

		if( X - Width < 0 )
		{
			X = Width;
		}
		if( Y < 0 )
		{
			Y = 0;
		}
		if( X + Width > 1024 )
		{
			X = 1024 - Width;
		}
		if( Y + Height * 2.0f > 768 )
		{
			Y = 768 - Height * 2.0f;
		}
		if( Y < m_pMousePointer->GetScreenY() + 48 )
		{
			Y = m_pMousePointer->GetScreenY() - Height * 2.0f;
		}
		if( Y < 0 )
		{
			Y = 0;
		}

		m_pInventoryItemDescription->SetPosition( X, Y );

		m_pDescriptionShadow->SetPosition( X, Y + ( m_pInventoryItemDescription->MaxBounds().y -
													m_pInventoryItemDescription->MinBounds().y ) * .5f );

		m_pDescriptionShadow->Render( pD3DDevice );

		// now we need to render the highlights...
		for( uint32 i = 0; i < Highlights.size(); i++ )
		{
			if( Highlights[i] != KHighlightColors )
			{
				m_pHighlight[Highlights[i]]->SetSize( Width * 2, CharacterHeight );
				m_pHighlight[Highlights[i]]->SetPosition( X, Y + i * CharacterHeight + CharacterHeight * .5f + 6 );
				m_pHighlight[Highlights[i]]->Render( pD3DDevice );
			}
		}

		m_pInventoryItemDescription->Render( pD3DDevice );

	}

} // CGameUI::RenderTooltips()

void CGameUI::RenderMiniMap( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
							 CGameClient& pClient )				// game client
{

	float32 AspectDifferenceX = ( float32 )m_pSettings.GetSettings( KSetScreenWidth ) / 1024.0f;
	float32 AspectDifferenceY = ( float32 )m_pSettings.GetSettings( KSetScreenHeight ) / 768.0f;
    D3DVIEWPORT8 Viewport;
    Viewport.X = (uint32)floor( (float32)( KMiniMapTopLeftX + 4 ) * AspectDifferenceX );
    Viewport.Y = (uint32)floor( (float32)( KMiniMapTopLeftY + 4 ) * AspectDifferenceY );
    Viewport.Width = (uint32)floor( (float32)( KMiniMapWidth - 8 ) * AspectDifferenceX );
    Viewport.Height = (uint32)floor( (float32)( KMiniMapWidth - 8 ) * AspectDifferenceY );
    Viewport.MinZ = 0.0f;
    Viewport.MaxZ = 1.0f;

	if( LeftCovered() )
	{
		Viewport.X += m_pSettings.GetSettings( KSetScreenWidth ) / 2;
	}

	pD3DDevice->SetViewport( &Viewport );


	D3DXMATRIX ViewMatrix;
	D3DXVECTOR3 LookTarget = pClient.Camera()->LookTarget();
	D3DXVECTOR3 Position( 0, KMiniMapDolly[m_MiniMapDolly], 0 );
	Position += LookTarget;
	D3DXVECTOR3 Up( -1, 0, 1 );
	D3DXVec3Normalize( &Up, &Up );
	D3DXMatrixLookAtLH( &ViewMatrix,
						&Position,
						&LookTarget,
						&Up );


	float32	Aspect	= ( float32 )m_pSettings.GetSettings( KSetScreenWidth ) /
					  ( float32 )m_pSettings.GetSettings( KSetScreenHeight );

	float32 FOV		= ( float32 )( m_pSettings.GetSettings( KSetFOV ) * KDeg2Rad );

	float32 Near	= (float32)m_pSettings.GetSettings( KSetNearClip );
	float32 Far		= 1000;
	D3DXMATRIX Projection;
	D3DXMatrixPerspectiveFovLH( &Projection,
								FOV,
								Aspect,
								Near,
								Far );

	// set  our projection to the camera's projection matrix
	pD3DDevice->SetTransform( D3DTS_PROJECTION, &Projection );

	m_MiniMapFrustum.Update( ViewMatrix,
							 Projection );

	pClient.Level()->Sort( &m_MiniMapFrustum );
	pClient.Level()->RenderMiniMap( ViewMatrix, m_pPlayerCharacter );

	// reset the viewport to standard width
    Viewport.Width = m_pSettings.GetSettings( KSetScreenWidth );
    Viewport.Height = m_pSettings.GetSettings( KSetScreenHeight );
	Viewport.X = 0;
	Viewport.Y = 0;
	pD3DDevice->SetViewport( &Viewport );

	D3DXMATRIX Identity;
	D3DXMATRIX Orthographic;
	// go back to an ortho projection for the rest of the ui
	D3DXMatrixIdentity( &Identity );
	D3DXMatrixOrthoLH ( &Orthographic, 
						1024,
						768,
						1.0f,
						10.0f );

	pD3DDevice->SetTransform( D3DTS_PROJECTION, &Orthographic );
	pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );
	pD3DDevice->SetTransform( D3DTS_VIEW, &Identity );
} // CGameUI::RenderMiniMap()

void CGameUI::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CGameUI::RenderText()

void CGameUI::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CSprite>* pNode = m_pSpriteList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CGameUI::RenderSprites()

void CGameUI::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CGameUI::RenderButtons()

// mouse handler should be updated for this to work properly
void CGameUI::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CGameUI::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CGameUI::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CGameUI::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CGameUI::UpdateButtonOver( void )
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
} // CGameUI::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CGameUI::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
{
	if( BothCovered() )
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
			TooltipText = pNode->Data()->ToolTip();
			return ReturnCode;
		}
		pNode = pNode->Next();
	}

	return -1;
} // CGameUI::UpdateButtonOver()

CButton* CGameUI::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CGameUI::AddButton()

void CGameUI::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CGameUI::RemoveButton()

void CGameUI::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CGameUI::DestroyButtons()

CText* CGameUI::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CGameUI::AddText()

void CGameUI::RemoveText( CText* pText )	// Text to remove from the list
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
} // CGameUI::RemoveText()

void CGameUI::DestroyText( void )
{
	m_pTextList->Destroy();
} // CGameUI::DestroyText()

CSprite* CGameUI::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CGameUI::AddSprite()

CSprite* CGameUI::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CGameUI::AddSpriteFront()

void CGameUI::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CGameUI::RemoveSprite()

void CGameUI::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CGameUI::DestroySprite()
