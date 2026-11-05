/*
	skillmenu.cpp

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

#include "gameui.h"
#include "skillmenu.h"
#include "bottommenu.h"

#include "demorules.h" //marsh

CSkillMenu::CSkillMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						CRefManager& pRefManager,					// the reference manager, for tracking media we create
						CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
						CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																		m_pGameStateManager( pGameStateManager ),
																		m_pRefManager( pRefManager ),
																		m_pSettings( pSettings ),
																		m_pButtonList	( new CLList<CButton>() ),
																		m_pTextList		( new CLList<CText>() ),
																		m_pSpriteList	( new CLList<CSprite>() ),
																		m_Open( kFalse ),
																		m_UpgradeButtonsVisible( kFalse ),
																		m_MessagePending( kFalse ),
																		m_pFontMaterial( NULL ),
																		m_pFontMetric( NULL ),
																		m_pPlayerCharacter( NULL ),
																		m_pSkillMenuTop( NULL ),
																		m_pSkillMenuCenter( NULL ),
																		m_pSkillMenuBottom( NULL ),
																		m_pUIButtons( NULL ),
																		m_pSkillPoints( NULL ),
																		m_pGivePointSound( NULL ),
																		m_pClickSound( NULL ),
																		m_pFontHighlightMaterial ( NULL )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));


	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	// initialize font texture and metric
	m_pFontHighlightMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
																  "FONT/highlights.png",
																  "FONT/highlightsa.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CSkillMenu::CSkillMenu()

CSkillMenu::~CSkillMenu( void )
{
	m_pRefManager.RemoveMaterial( m_pFontHighlightMaterial );

	for( uint32 i = 0; i < KHighlightColors; i++ )
	{
		DELETE_SAFELY( m_pHighlight[i] );
	}

	RemoveUpgradeButtons();
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveSample( m_pGivePointSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	m_pRefManager.RemoveMaterial( m_pSkillMenuTop );
	m_pRefManager.RemoveMaterial( m_pSkillMenuBottom );
	m_pRefManager.RemoveMaterial( m_pSkillMenuCenter );

} // CSkillMenu::~CSkillMenu()


//gets the text from the Demo Constants - marsh
std::string CSkillMenu::PendingMessageText( void )
{
	m_MessagePending = kFalse;
	return LoadLocalizedString1Variable(975, IntegerToString( KDemoPlayerLevel ));
}

void CSkillMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CSkillMenu::FlushAll()

void CSkillMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	m_pGivePointSound = m_pRefManager.AddSample( "SOUNDS/UI/givepoint.wav", kFalse );

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pSkillMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/skilltop.png" );

	m_pSkillMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/skillbottom.png"  );

	m_pSkillMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/skillcenter.png" );

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
									KBottomButtonSkills ) );
	pButton->SetToolTip( LoadLocalizedString(170) );


	AddSprite( new CSprite( pD3DDevice,
							m_pSkillMenuTop,
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
							m_pSkillMenuCenter,
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
							m_pSkillMenuBottom,
							512,
							512,
							512,
							128,
							0,
							.01f,
							1,
							1,
							KSpriteTopLeft ) );

	for( uint32 i = 0; i < KSkills; i++ )
	{
		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							KSkillDescription[i],
							751,
							83 + (float32)i * 32.0f,
							.8f,
							kFalse,
							KJustifyLeft ) );

		CText* pText = 	AddText( new CText( pD3DDevice,
											m_pFontMaterial,
											m_pFontMetric,
											"0",
											698,
											84 + (float32)i * 32.0f,
											.8f,
											kFalse,
											KJustifyCenter ) );
		m_pSkillPointText.push_back( pText );

		CButton* pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													743,
													80 + (float32)i * 32.0f,
													200,
													31,
													0,
													0,
													1,
													1,
													0,
													0,
													1,
													1,
													KButtonTopLeft,
													-900 ) );
		pButton->SetToolTip( KSkillExplanation[i] );
		m_pSkillButtonTooltip[i] = pButton;

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													651,
													80 + (float32)i * 32.0f,
													89,
													31,
													0,
													0,
													1,
													1,
													0,
													0,
													1,
													1,
													KButtonTopLeft,
													-900 ) );
		m_pSkillButton.push_back( pButton );

	}



	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(976),
						730,
						575,
						.6f,
						kFalse,
						KJustifyCenter ) );

	m_pSkillPoints = AddText( new CText( pD3DDevice,
								   m_pFontMaterial,
								   m_pFontMetric,
								   "0",
								   832,
								   572,
								   .9f,
								   kFalse,
								   KJustifyCenter ) );

	float32 StepSize = 1.0f / (float32)KHighlightColors;
	for( int32 i = 0; i < KHighlightColors; i++ )
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
									   KSpriteTopLeft );
	}

} // CSkillMenu::CreateMenus()


void CSkillMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CSkillMenu::KeyEvent()

void CSkillMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CSkillMenu::KeyEvent()

bool CSkillMenu::MouseOver( void )
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
} // CSkillMenu::MouseOver()

void CSkillMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CSkillMenu::ProcessInput()

void CSkillMenu::Update( HWND hWnd,						// hwnd of the active window
					  float32 TimeElapsed )				// time elapsed, in seconds
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

	if( IsGameUnlocked() || ( !IsGameUnlocked() && m_pPlayerCharacter->Level() <= KDemoPlayerLevel ) ) //marsh
	{
		if( ButtonPressed >= KSkillButtonAdd )
		{
			ESkill Skill = (ESkill)( ButtonPressed - KSkillButtonAdd );
			m_pPlayerCharacter->SpendSkillPoint( Skill );
			FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
		}
	}
	else 
	{

		if( ButtonPressed >= KSkillButtonAdd )
		{
			m_MessagePending = kTrue;
		}
	}


	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CSkillMenu::Update()

// render any 3d elements
void CSkillMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						 const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( !Open() )
	{
		return;
	}

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	RenderSprites( pD3DDevice );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	RenderButtons( pD3DDevice );


	if( m_pPlayerCharacter != NULL )
	{
		// if the player has Skill points to distribute, show buttons
		// that they can click to upgrade them
		if( m_pPlayerCharacter->UnusedSkillPoints() > 0 )
		{
			ShowUpgradeButtons( pD3DDevice );
		}
		else
		{
			RemoveUpgradeButtons();
		}

		for( uint32 i = 0; i < KSkills; i++ )
		{

			if( i <= KSkillBow )
			{
				m_pSkillButtonTooltip[i]->SetToolTip( KSkillExplanation[i] + "\n\b" + LoadLocalizedString(977));
			}
			else if( i == KSkillCriticalStrike )
			{
				float32 StrikeSkill = 50.0f / ( (float32)m_pPlayerCharacter->SkillPoints( KSkillCriticalStrike ) * .03f + 1 );
				StrikeSkill = 50 - StrikeSkill;
				if( StrikeSkill > 50 )
				{
					StrikeSkill = 50;
				}

				float32 StrikeSkill2 = 50.0f / ( ( (float32)m_pPlayerCharacter->SkillPoints( KSkillCriticalStrike ) + 1 ) * .03f + 1 );
				StrikeSkill2 = 50 - StrikeSkill2;
				if( StrikeSkill2 > 50 )
				{
					StrikeSkill2 = 50;
				}
				m_pSkillButtonTooltip[i]->SetToolTip( KSkillExplanation[i] + "\n\b" + LoadLocalizedString2Variables(978, FloatToString( StrikeSkill ), FloatToString( StrikeSkill2 ) ) );
			}
			else if( i == KSkillShield )
			{
				float32 BlockSkill = 50.0f / ( (float32)m_pPlayerCharacter->SkillPoints( KSkillShield ) * .03f + 1 );
				BlockSkill = 50 - BlockSkill;
				if( BlockSkill > 50 )
				{
					BlockSkill = 50;
				}


				float32 BlockSkill2 = 50.0f / ( ( (float32)m_pPlayerCharacter->SkillPoints( KSkillShield ) + 1 ) * .03f + 1 );
				BlockSkill2 = 50 - BlockSkill2;
				if( BlockSkill2 > 50 )
				{
					BlockSkill2 = 50;
				}
				m_pSkillButtonTooltip[i]->SetToolTip( KSkillExplanation[i] + "\n\b" + LoadLocalizedString2Variables( 979, FloatToString( BlockSkill ), FloatToString( BlockSkill2 )));
			}
			else if( i == KSkillSpellcasting )
			{
				float32 SpellSkill = 50.0f / ( (float32)m_pPlayerCharacter->SkillPoints( KSkillSpellcasting ) * .03f + 1 );
				SpellSkill = 50 - SpellSkill;
				if( SpellSkill > 50 )
				{
					SpellSkill = 50;
				}
				SpellSkill /= 50.0f;
				SpellSkill *= 80.0f;

				float32 SpellSkill2 = 50.0f / ( ( (float32)m_pPlayerCharacter->SkillPoints( KSkillSpellcasting ) + 1 ) * .03f + 1 );
				SpellSkill2 = 50 - SpellSkill2;
				if( SpellSkill2 > 50 )
				{
					SpellSkill2 = 50;
				}
				SpellSkill2 /= 50.0f;
				SpellSkill2 *= 80.0f;

				m_pSkillButtonTooltip[i]->SetToolTip( KSkillExplanation[i] + "\n\b" + LoadLocalizedString2Variables(980, FloatToString( SpellSkill ), FloatToString( SpellSkill2 )));
			}
			else if( i == KSkillDualWield )
			{
				float32 PrimarySkillReduction = 50.0f / ( (float32)m_pPlayerCharacter->SkillPoints( KSkillDualWield ) * .05f + 1 );
				PrimarySkillReduction = 50 - PrimarySkillReduction;
				if( PrimarySkillReduction > 50 )
				{
					PrimarySkillReduction = 50;
				}
				PrimarySkillReduction /= 50.0f;
				PrimarySkillReduction *= .25f;
				PrimarySkillReduction = .75f + PrimarySkillReduction;
				PrimarySkillReduction *= 100.0f;


				float32 SecondarySkillReduction = 50.0f / ( (float32)m_pPlayerCharacter->SkillPoints( KSkillDualWield ) * .05f + 1 );
				SecondarySkillReduction = 50 - SecondarySkillReduction;
				if( SecondarySkillReduction > 50 )
				{
					SecondarySkillReduction = 50;
				}
				SecondarySkillReduction /= 50.0f;
				SecondarySkillReduction *= .5f;
				SecondarySkillReduction = .5f + SecondarySkillReduction;
				SecondarySkillReduction *= 100.0f;

				float32 PrimarySkillReduction2 = 50.0f / ( ( (float32)m_pPlayerCharacter->SkillPoints( KSkillDualWield ) + 1 ) * .05f + 1 );
				PrimarySkillReduction2 = 50 - PrimarySkillReduction2;
				if( PrimarySkillReduction2 > 50 )
				{
					PrimarySkillReduction2 = 50;
				}
				PrimarySkillReduction2 /= 50.0f;
				PrimarySkillReduction2 *= .25f;
				PrimarySkillReduction2 = .75f + PrimarySkillReduction2;
				PrimarySkillReduction2 *= 100.0f;
	

				float32 SecondarySkillReduction2 = 50.0f / ( ( (float32)m_pPlayerCharacter->SkillPoints( KSkillDualWield ) + 1 ) * .05f + 1 );
				SecondarySkillReduction2 = 50 - SecondarySkillReduction2;
				if( SecondarySkillReduction2 > 50 )
				{
					SecondarySkillReduction2 = 50;
				}
				SecondarySkillReduction2 /= 50.0f;
				SecondarySkillReduction2 *= .5f;
				SecondarySkillReduction2 = .5f + SecondarySkillReduction2;
				SecondarySkillReduction2 *= 100.0f;

				m_pSkillButtonTooltip[i]->SetToolTip( KSkillExplanation[i] + "\n\b" + LoadLocalizedString4Variables( 981, FloatToString( PrimarySkillReduction ), FloatToString( PrimarySkillReduction2 ), FloatToString( SecondarySkillReduction ), FloatToString( SecondarySkillReduction2 )));
			}

			m_pSkillPointText[i]->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->SkillPoints( (ESkill)i ) ) );

			if( m_pPlayerCharacter->SkillPoints( (ESkill)i ) > m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) )
			{
				m_pHighlight[KHighlightGreen]->SetSize( 88, 27 );
				m_pHighlight[KHighlightGreen]->SetPosition( 652, 82 + (float32)i * 32.0f );
				m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
				m_pSkillButton[i]->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) ) + 
											" + \b" + 
											IntegerToString( m_pPlayerCharacter->SkillPoints( (ESkill)i ) - m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) ) + 
											"\b" );




			}
			else if( m_pPlayerCharacter->SkillPoints( (ESkill)i ) < m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) )
			{
				m_pHighlight[KHighlightRed]->SetSize( 88, 27 );
				m_pHighlight[KHighlightRed]->SetPosition( 652, 82 + (float32)i * 32.0f );
				m_pHighlight[KHighlightRed]->Render( pD3DDevice );
				m_pSkillButton[i]->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) ) + 
											" - \b" + 
											IntegerToString( m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) - m_pPlayerCharacter->SkillPoints( (ESkill)i ) ) + 
											"\b" );
			}
			else
			{
				m_pSkillButton[i]->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseSkillPoints( (ESkill)i ) ) );
			}
		}

		m_pSkillPoints->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->UnusedSkillPoints() ) );
	}

	RenderText( pD3DDevice );		

} // CSkillMenu::Render()


void CSkillMenu::ShowUpgradeButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_UpgradeButtonsVisible )
	{

		for( uint32 i = 0; i < KSkills; i++ )
		{
			CButton* pButton = AddButton( new CButton( pD3DDevice,
												  	   m_pUIButtons,
													   620,
													   84 + (float32)i * 32.0f,
													   26,
													   25,
													   m_pUIButtons->PixelToU() * 185,
													   m_pUIButtons->PixelToV() * 2,
													   m_pUIButtons->PixelToU() * 210,
													   m_pUIButtons->PixelToV() * 26,
													   m_pUIButtons->PixelToU() * 185,
													   m_pUIButtons->PixelToV() * 27,
													   m_pUIButtons->PixelToU() * 210,
													   m_pUIButtons->PixelToV() * 51,
													   KButtonTopLeft,
													   KSkillButtonAdd + i ) );

			if(!CGame::GetLocaleString().compare("de"))
			{
				pButton->SetToolTip(KSkillDescription[i] + " " + LoadLocalizedString(1163) );
			}
			else
			{
				pButton->SetToolTip( LoadLocalizedString(1163) + " " + KSkillDescription[i] );
			}
			m_pUpgradeButtons.push_back( pButton );
		}


		m_UpgradeButtonsVisible = kTrue;
	}
} // CSkillMenu::ShowUpgradeButtons()

void CSkillMenu::RemoveUpgradeButtons( void )
{
	if( m_UpgradeButtonsVisible )
	{
		for( uint32 i = 0; i < m_pUpgradeButtons.size(); i++ )
		{
			RemoveButton( m_pUpgradeButtons[i] );
			DELETE_SAFELY( m_pUpgradeButtons[i] );
		}
		m_pUpgradeButtons.clear();
		m_UpgradeButtonsVisible = kFalse;
	}
} // CSkillMenu::RemoveUpgradeButtons()


void CSkillMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CSkillMenu::RenderText()

void CSkillMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CSkillMenu::RenderSprites()

void CSkillMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CSkillMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CSkillMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CSkillMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CSkillMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CSkillMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CSkillMenu::UpdateButtonOver( void )
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
} // CSkillMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CSkillMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CSkillMenu::UpdateButtonOver()

CButton* CSkillMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CSkillMenu::AddButton()

void CSkillMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CSkillMenu::RemoveButton()

void CSkillMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CSkillMenu::DestroyButtons()

CText* CSkillMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CSkillMenu::AddText()

void CSkillMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CSkillMenu::RemoveText()

void CSkillMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CSkillMenu::DestroyText()

CSprite* CSkillMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CSkillMenu::AddSprite()

CSprite* CSkillMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CSkillMenu::AddSpriteFront()

void CSkillMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CSkillMenu::RemoveSprite()

void CSkillMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CSkillMenu::DestroySprite()


