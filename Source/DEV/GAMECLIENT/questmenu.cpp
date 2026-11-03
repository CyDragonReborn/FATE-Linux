/*
	questmenu.cpp

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

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../IO/keyhandler.h"

#include "../CHARACTER/character.h"
#include "../ITEM/item.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "../GAMECLIENT/quest.h"

#include "gameui.h"
#include "questmenu.h"
#include "bottommenu.h"

CQuestMenu::CQuestMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						CRefManager& pRefManager,					// the reference manager, for tracking media we create
						CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
						CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																	m_pGameStateManager( pGameStateManager ),
																	m_pRefManager( pRefManager ),
																	m_pSettings( pSettings ),
																	m_Open( kFalse ),
																	m_RequestConfirmation( kFalse ),
																	m_SelectedQuest( 0 ),
																	m_pButtonList	( new CLList<CButton>() ),
																	m_pTextList		( new CLList<CText>() ),
																	m_pSpriteList	( new CLList<CSprite>() ),
																	m_pFontMaterial( NULL ),
																	m_pFontMetric( NULL ),
																	m_pPlayerCharacter( NULL ),
																	m_pQuestMenuTop( NULL ),
																	m_pQuestMenuCenter( NULL ),
																	m_pQuestMenuBottom( NULL ),
																	m_pUIButtons( NULL ),
																	m_pPopupButtons( NULL ),
																	m_pQuestDescriptionText( NULL ),
																	m_pGoldRewardText( NULL ),
																	m_pExperienceRewardText( NULL ),
																	m_pFameRewardText( NULL ),
																	m_pItemRewardText( NULL ),
																	m_pClickSound( NULL )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CQuestMenu::CQuestMenu()

CQuestMenu::~CQuestMenu( void )
{
	for( uint32 i = 0; i < KMaximumActiveQuests; i++ )
	{
		DELETE_SAFELY( m_pQuestText[i] );
		RemoveButton( m_pQuestButton[i] );
		DELETE_SAFELY( m_pQuestButton[i] );
		RemoveButton( m_pQuestCancelButton[i] );
		DELETE_SAFELY( m_pQuestCancelButton[i] );
	}
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );
	m_pRefManager.RemoveMaterial( m_pPopupButtons );

	m_pRefManager.RemoveMaterial( m_pQuestMenuTop );
	m_pRefManager.RemoveMaterial( m_pQuestMenuBottom );
	m_pRefManager.RemoveMaterial( m_pQuestMenuCenter );

} // CQuestMenu::~CQuestMenu()

void CQuestMenu::FlushAll( void )
{
	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CQuestMenu::FlushAll()

void CQuestMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pPopupButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													  "MENUS/buttons.png",
													  "MENUS/buttonsa.png" );

	m_pQuestMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/questtop.png" );

	m_pQuestMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/questbottom.png"  );

	m_pQuestMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/questcenter.png" );

	CButton* pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									518,
									582,
									51,
									51,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 1,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 51,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 52,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 102,
									KButtonTopLeft,
									KBottomButtonQuests ) );
	pButton->SetToolTip( LoadLocalizedString(170) );


	AddSprite( new CSprite( pD3DDevice,
							m_pQuestMenuTop,
							512,
							0,
							512,
							256,
							0,
							0,
							1,
							.9985f,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pQuestMenuCenter,
							512,
							256,
							512,
							256,
							0,
							.0015f,
							1,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pQuestMenuBottom,
							512,
							512,
							512,
							128,
							0,
							.01f,
							1,
							1,
							KSpriteTopLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(963),
						770,
						100,
						.8f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(964),
						772,
						419,
						.8f,
						kFalse,
						KJustifyCenter ) );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(965),
						645,
						451,
						.6f,
						kFalse,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(917),
						645,
						474,
						.6f,
						kFalse,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(966),
						645,
						497,
						.6f,
						kFalse,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(967),
						645,
						521,
						.6f,
						kFalse,
						KJustifyLeft ) );

	m_pQuestDescriptionText = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"",
											625,
											260,
											.6f, // TDC 2008-02-27 Craig Miles - Reduced font size
											kFalse,
											KJustifyLeft ) );
	m_pQuestDescriptionText->EnableWrapping( pD3DDevice,
											 320 );
	m_pGoldRewardText = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"0",
											905,
											451,
											.6f,
											kFalse,
											KJustifyRight ) );

	m_pExperienceRewardText = AddText( new CText( pD3DDevice,
													m_pFontMaterial,
													m_pFontMetric,
													"0",
													905,
													474,
													.6f,
													kFalse,
													KJustifyRight ) );

	m_pFameRewardText = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"0",
											905,
											497,
											.6f,
											kFalse,
											KJustifyRight ) );

	m_pItemRewardText = AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"",
											905,
											521,
											.6f,
											kFalse,
											KJustifyRight ) );

	for( uint32 i = 0; i < KMaximumActiveQuests; i++ )
	{
		m_pQuestButton[i] = NULL;
		m_pQuestCancelButton[i] = NULL;
		m_pQuestText[i] =  new CText( pD3DDevice,
									  m_pFontMaterial,
									  m_pFontMetric,
									  LoadLocalizedString(968),
									  635,
									  168 + (float32)i * 25,
									  .6f,
									  kFalse,
									  KJustifyLeft );
	}

} // CQuestMenu::CreateMenus()


void CQuestMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CQuestMenu::KeyEvent()

void CQuestMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CQuestMenu::KeyEvent()

bool CQuestMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}

	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( MousePosition.x >= 512 )
	{
		return kTrue;
	}

	return kFalse;
} // CQuestMenu::MouseOver()

void CQuestMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CQuestMenu::ProcessInput()

void CQuestMenu::ProcessConfirmation( int32 ButtonPressed )	// confirmation button pressed
{
	if( ButtonPressed != -1 )
	{
		if( ButtonPressed == KButtonCancelQuestYes )
		{
			m_pPlayerCharacter->RemoveQuest( m_SelectedQuest );
		}
		FlushAll();
	}
} // CQuestMenu::ProcessConfirmation()

void CQuestMenu::Update( HWND hWnd,						// hwnd of the active window
						 float32 TimeElapsed )			// time elapsed, in seconds
{
	if( !Open() )
	{
		return;
	}

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );

	// left mouse button first
	int32 ButtonPressed = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
											m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );

	if( ButtonPressed >= KQuestCancelButton )
	{
		uint32 QuestIndex = ButtonPressed - KQuestCancelButton;
		m_SelectedQuest = QuestIndex;
		m_RequestConfirmation = kTrue;
	}
	else if( ButtonPressed >= KQuestButton )
	{
		uint32 QuestIndex = ButtonPressed - KQuestButton;
		m_SelectedQuest = QuestIndex;
	}

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CQuestMenu::Update()

void CQuestMenu::SetOpen( bool Open )		// state
{
	m_Open = Open;
} // CQuestMenu::SetOpen()

// render any 3d elements
void CQuestMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 CGameUI& pGameUI,						// parent game ui
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						 const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( !Open() )
	{
		return;
	}

	if( m_RequestConfirmation )
	{
		m_RequestConfirmation = kFalse;
		pGameUI.OpenConfirmation( pD3DDevice,
								  LoadLocalizedString1Variable(969, m_pPlayerCharacter->Quest( m_SelectedQuest )->Name() ),
								  LoadLocalizedString(171),
								  KButtonCancelQuestYes,
								  LoadLocalizedString(172),
								  KButtonCancelQuestNo );
	}

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	RenderSprites( pD3DDevice );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	RenderButtons( pD3DDevice );

	if( m_SelectedQuest >= m_pPlayerCharacter->Quests() )
	{
		m_pQuestDescriptionText->SetText( pD3DDevice, "" );
		m_pGoldRewardText->SetText( pD3DDevice, "" );
		m_pExperienceRewardText->SetText( pD3DDevice, "" );
		m_pFameRewardText->SetText( pD3DDevice, "" );
		m_pItemRewardText->SetText( pD3DDevice, "" );
	}
	for( uint32 i = 0; i < m_pPlayerCharacter->Quests(); i++ )
	{
		if( m_pQuestCancelButton[i] == NULL )
		{
			m_pQuestCancelButton[i] = AddButton( new CButton( pD3DDevice,
																m_pPopupButtons,
																NULL,
																m_pClickSound,
																885,
																176 + (float32)i * 25,
																74,
																24,
																108 * m_pPopupButtons->PixelToU(),
																145 * m_pPopupButtons->PixelToV(),
																256 * m_pPopupButtons->PixelToU(),
																193 * m_pPopupButtons->PixelToV(),
																108 * m_pPopupButtons->PixelToU(),
																97 * m_pPopupButtons->PixelToV(),
																256 * m_pPopupButtons->PixelToU(),
																145 * m_pPopupButtons->PixelToV(),
																KButtonCenter,
																KQuestCancelButton + i ) );
			m_pQuestCancelButton[i]->SetToolTip( LoadLocalizedString(970) );
			m_pQuestCancelButton[i]->AddText( pD3DDevice,
											LoadLocalizedString(971),
											m_pFontMaterial,
											m_pFontMetric,
											0,
											-9.0f,
											.6f,
											kTrue,
											KJustifyCenter );
			m_pQuestCancelButton[i]->AddOverText( pD3DDevice,
												LoadLocalizedString(971),
												m_pFontMaterial,
												m_pFontMetric,
												0,
												-9.0f,
												.6f,
												kFalse,
												KJustifyCenter );

		}

		if( m_pQuestButton[i] == NULL )
		{
			m_pQuestButton[i] = AddButton( new CButton( pD3DDevice,
														m_pPopupButtons,
														NULL,
														m_pClickSound,
														635,
														165 + (float32)i * 25,
														205,
														24,
														33 * m_pPopupButtons->PixelToU(),
														102 * m_pPopupButtons->PixelToV(),
														46 * m_pPopupButtons->PixelToU(),
														119 * m_pPopupButtons->PixelToV(),
														33 * m_pPopupButtons->PixelToU(),
														102 * m_pPopupButtons->PixelToV(),
														46 * m_pPopupButtons->PixelToU(),
														119 * m_pPopupButtons->PixelToV(),
														KButtonTopLeft,
														KQuestButton + i ) );
			m_pQuestButton[i]->SetToolTip( LoadLocalizedString(1132) );

		}
		else if( m_SelectedQuest == i )
		{
			m_pQuestButton[i]->SetUVs( 33 * m_pPopupButtons->PixelToU(),
									   102 * m_pPopupButtons->PixelToV(),
									   46 * m_pPopupButtons->PixelToU(),
									   119 * m_pPopupButtons->PixelToV() );

			m_pQuestButton[i]->SetOverUVs( 33 * m_pPopupButtons->PixelToU(),
										   102 * m_pPopupButtons->PixelToV(),
										   46 * m_pPopupButtons->PixelToU(),
										   119 * m_pPopupButtons->PixelToV() );
		}
		else
		{
			m_pQuestButton[i]->SetUVs( 7 * m_pPopupButtons->PixelToU(),
									   102 * m_pPopupButtons->PixelToV(),
									   19 * m_pPopupButtons->PixelToU(),
									   119 * m_pPopupButtons->PixelToV() );
			m_pQuestButton[i]->SetOverUVs( 33 * m_pPopupButtons->PixelToU(),
										   102 * m_pPopupButtons->PixelToV(),
										   46 * m_pPopupButtons->PixelToU(),
										   119 * m_pPopupButtons->PixelToV() );
		}

		if( m_pPlayerCharacter->Quest( i )->Completed() )
		{
			m_pQuestText[i]->SetText( pD3DDevice,
									LoadLocalizedString1Variable(972, m_pPlayerCharacter->Quest( i )->Name() ) );
			m_pQuestText[i]->SetBold( pD3DDevice, kTrue );
		}
		else
		{
			m_pQuestText[i]->SetText( pD3DDevice,
									m_pPlayerCharacter->Quest( i )->Name() );
			m_pQuestText[i]->SetBold( pD3DDevice, kFalse );
		}
		m_pQuestText[i]->Render( pD3DDevice );


		if( m_SelectedQuest == i )
		{
			if( m_pPlayerCharacter->Quest( i )->Completed() )
			{
				m_pQuestDescriptionText->SetText( pD3DDevice,
					"\b" + m_pPlayerCharacter->Quest( i )->MiniDescription() + LoadLocalizedString1Variable(973, m_pPlayerCharacter->Quest( i )->GiverName() )  );
			}
			else
			{
				std::string Description = m_pPlayerCharacter->Quest( i )->MiniDescription();
				if( m_pPlayerCharacter->Quest( i )->Type() == KQuestKillMonsters )
				{
					/*
					Description = Description + "\n(\b" + 
											 IntegerToString( m_pPlayerCharacter->Quest( i )->MonstersRemaining() ) + "\b)\b " +
											 LoadLocalizedString1Variable(974, m_pPlayerCharacter->Quest( i )->QuestMonsterName() );
					*/
					Description = Description + LoadLocalizedString2Variables(974, IntegerToString( m_pPlayerCharacter->Quest( i )->MonstersRemaining() ), m_pPlayerCharacter->Quest( i )->QuestMonsterName() );
				}
				if( m_pPlayerCharacter->Quest( i )->Type() == KQuestKillArmy || 
					m_pPlayerCharacter->Quest( i )->Type() == KQuestArmyItem )
				{
					/*
					Description = Description + "\n(\b" + 
											 IntegerToString( m_pPlayerCharacter->Quest( i )->MinionsRemaining() ) + "\b)\b " +
											 LoadLocalizedString1Variable(974, m_pPlayerCharacter->Quest( i )->QuestMinionMonsterName() );
					*/

					Description = Description + LoadLocalizedString2Variables(974, IntegerToString( m_pPlayerCharacter->Quest( i )->MinionsRemaining() ), m_pPlayerCharacter->Quest( i )->QuestMinionMonsterName() );
				}
				m_pQuestDescriptionText->SetText( pD3DDevice,
												  Description );
			}
			m_pGoldRewardText->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Quest( i )->GoldReward() ) );
			m_pExperienceRewardText->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Quest( i )->ExperienceReward() ) );
			m_pFameRewardText->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Quest( i )->FameReward() ) );
			std::string RewardName = m_pPlayerCharacter->Quest( i )->ItemRewardName();
			if( RewardName.length() > 20 )
			{
				RewardName = RewardName.substr( 0,20 ) + "...";
			}
			while( RewardName.find( "\n" ) != -1 )
			{
				int32 Index = RewardName.find( "\n" );
				RewardName.replace( Index, 1, " " );
			}
			m_pItemRewardText->SetText( pD3DDevice, RewardName );
		}

	}
	for( i = m_pPlayerCharacter->Quests(); i < KMaximumActiveQuests ; i++ )
	{
		if( m_pQuestButton[i] != NULL )
		{
			RemoveButton( m_pQuestButton[i] );
			DELETE_SAFELY( m_pQuestButton[i] );
			m_pQuestButton[i] = NULL;
		}
		if( m_pQuestCancelButton[i] != NULL )
		{
			RemoveButton( m_pQuestCancelButton[i] );
			DELETE_SAFELY( m_pQuestCancelButton[i] );
			m_pQuestCancelButton[i] = NULL;
		}
	}

	RenderText( pD3DDevice );		

} // CQuestMenu::Render()


void CQuestMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CQuestMenu::RenderText()

void CQuestMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CQuestMenu::RenderSprites()

void CQuestMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CQuestMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CQuestMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
{
	if( !Open() )
	{
		return;
	}
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
} // CQuestMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CQuestMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
{
	if( !ButtonPressed || !Open() )
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
} // CQuestMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CQuestMenu::UpdateButtonOver( void )
{
	if( !Open() )
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
			return ReturnCode;
		}
		pNode = pNode->Next();
	}

	return -1;
} // CQuestMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CQuestMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
{
	if( !Open() )
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
} // CQuestMenu::UpdateButtonOver()

CButton* CQuestMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CQuestMenu::AddButton()

void CQuestMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CQuestMenu::RemoveButton()

void CQuestMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CQuestMenu::DestroyButtons()

CText* CQuestMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CQuestMenu::AddText()

void CQuestMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CQuestMenu::RemoveText()

void CQuestMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CQuestMenu::DestroyText()

CSprite* CQuestMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CQuestMenu::AddSprite()

CSprite* CQuestMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CQuestMenu::AddSpriteFront()

void CQuestMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CQuestMenu::RemoveSprite()

void CQuestMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CQuestMenu::DestroySprite()


