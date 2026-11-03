/*
	statsmenu.cpp

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
#include "statsmenu.h"
#include "bottommenu.h"

#include "demorules.h" //marsh

CStatsMenu::CStatsMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
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
																		m_pPlayerCharacter( NULL ),
																		m_pAddStrengthButton( NULL ),
																		m_pAddDexterityButton( NULL ),
																		m_pAddVitalityButton( NULL ),
																		m_pAddMagicButton( NULL ),
																		m_UpgradeButtonsVisible( kFalse ),
																		m_MessagePending( kFalse ),
																		m_pFontMaterial( NULL ),
																		m_pFontMetric( NULL ),
																		m_pStatsMenuTop( NULL ),
																		m_pStatsMenuCenter( NULL ),
																		m_pStatsMenuBottom( NULL ),
																		m_pUIButtons( NULL ),
																		m_pFontHighlightMaterial( NULL ),
																		m_pName( NULL ),
																		m_pLevel( NULL ),
																		m_pExperience( NULL ),
																		m_pNextExperience( NULL ),
																		m_pRenown( NULL ),
																		m_pFame( NULL ),
																		m_pNextFame( NULL ),
																		m_pStrength( NULL ),
																		m_pDexterity( NULL ),
																		m_pVitality( NULL ),
																		m_pMagic( NULL ),
																		m_pStatPoints( NULL ),
																		m_pDamage( NULL ),
																		m_pToHit( NULL ),
																		m_pDefense( NULL ),
																		m_pLife( NULL ),
																		m_pStamina( NULL ),
																		m_pMana( NULL ),
																		m_pStrengthButton( NULL ),
																		m_pDexterityButton( NULL ),
																		m_pVitalityButton( NULL ),
																		m_pMagicButton( NULL ),
																		m_pDamageButton( NULL ),
																		m_pAttackButton( NULL ),
																		m_pDefenseButton( NULL ),
																		m_pStaminaButton( NULL ),
																		m_pLifeButton( NULL ),
																		m_pManaButton( NULL ),
																		m_pGivePointSound( NULL ),
																		m_pClickSound( NULL ),
																		m_LastDisplayedName( "" )
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

} // CStatsMenu::CStatsMenu()

CStatsMenu::~CStatsMenu( void )
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

	m_pRefManager.RemoveMaterial( m_pStatsMenuTop );
	m_pRefManager.RemoveMaterial( m_pStatsMenuBottom );
	m_pRefManager.RemoveMaterial( m_pStatsMenuCenter );

} // CStatsMenu::~CStatsMenu()

//gets the text from the Demo Constants - marsh
std::string CStatsMenu::PendingMessageText( void )
{
	m_MessagePending = kFalse;
	return LoadLocalizedString1Variable(975, IntegerToString( KDemoPlayerLevel ) );
}

void CStatsMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CStatsMenu::FlushAll()

void CStatsMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	m_pGivePointSound = m_pRefManager.AddSample( "SOUNDS/UI/givepoint.wav", kFalse );

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pStatsMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/statstop.png" );

	m_pStatsMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/statsbottom.png"  );

	m_pStatsMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/statscenter.png" );

	CButton* pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									453,
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
									KBottomButtonStats ) );
	pButton->SetToolTip( LoadLocalizedString(170) );


	AddSprite( new CSprite( pD3DDevice,
							m_pStatsMenuTop,
							0,
							0,
							512,
							256,
							0,
							0,
							1,
							.9985f,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pStatsMenuCenter,
							0,
							256,
							512,
							256,
							0,
							.0015f,
							1,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pStatsMenuBottom,
							0,
							512,
							512,
							128,
							0,
							.015f,
							1,
							1,
							KSpriteTopLeft ) );

	m_pName = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"",
							255,
							60,
							.8f,
							kFalse,
							KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(916),
						97,
						122,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pLevel = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"1",
									97,
									134,
									.9f,
									kFalse,
									KJustifyCenter ) );


	m_pExperience = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"0",
									232,
									134,
									.9f,
									kFalse,
									KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(917),
						232,
						122,
						.7f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(918),
						389,
						122,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pNextExperience = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"0",
									389,
									134,
									.9f,
									kFalse,
									KJustifyCenter ) );

	// fame rankings

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(987),
						97,
						188,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pRenown = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"1",
									97,
									200,
									.9f,
									kFalse,
									KJustifyCenter ) );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(966),
						232,
						188,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pFame = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"0",
									232,
									200,
									.9f,
									kFalse,
									KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(918),
						389,
						188,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pNextFame = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"0",
									389,
									200,
									.9f,
									kFalse,
									KJustifyCenter ) );


	// beginning of stats

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(291),
						82,
						258,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pStrength = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"0",
							183,
							257,
							.9f,
							kFalse,
							KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(919),
							337,
							258,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pDamage = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0-0",
								429,
								257,
								.9f,
								kFalse,
								KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(292),
						82,
						338,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pDexterity = AddText( new CText( pD3DDevice,
										m_pFontMaterial,
										m_pFontMetric,
										"0",
										183,
										337,
										.9f,
										kFalse,
										KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(988),
							337,
							339,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pToHit = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								429,
								337,
								.9f,
								kFalse,
								KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(921),
							337,
							373,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pDefense = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								429,
								372,
								.9f,
								kFalse,
								KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(293),
						82,
						418,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pVitality = AddText( new CText( pD3DDevice,
										m_pFontMaterial,
										m_pFontMetric,
										"0",
										183,
										417,
										.9f,
										kFalse,
										KJustifyCenter ) );
		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(922),
							337,
							418,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pStamina = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								429,
								417,
								.9f,
								kFalse,
								KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(923),
							337,
							453,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pLife = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								429,
								452,
								.9f,
								kFalse,
								KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(924),
						82,
						498,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pMagic = AddText( new CText( pD3DDevice,
								   m_pFontMaterial,
								   m_pFontMetric,
								   "0",
								   183,
								   497,
								   .9f,
								   kFalse,
								   KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(925),
							337,
							498,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pMana = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								429,
								497,
								.9f,
								kFalse,
								KJustifyCenter ) );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(976),
						208,
						555,
						.6f,
						kFalse,
						KJustifyCenter ) );

	m_pStatPoints = AddText( new CText( pD3DDevice,
								   m_pFontMaterial,
								   m_pFontMetric,
								   "0",
								   310,
								   552,
								   .9f,
								   kFalse,
								   KJustifyCenter ) );


		// add context tooltips
		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													39,
													255,
													100,
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
		pButton->SetToolTip( LoadLocalizedString(926) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													39,
													334,
													100,
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
		pButton->SetToolTip( LoadLocalizedString(927) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													39,
													415,
													100,
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
		pButton->SetToolTip( LoadLocalizedString(928) );


		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													39,
													495,
													100,
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
		pButton->SetToolTip( LoadLocalizedString(929) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													291,
													255,
													92,
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
		pButton->SetToolTip( LoadLocalizedString(930) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													291,
													334,
													92,
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
		pButton->SetToolTip( LoadLocalizedString(931) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													291,
													370,
													92,
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
		pButton->SetToolTip( LoadLocalizedString(932) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													291,
													415,
													92,
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
		pButton->SetToolTip( LoadLocalizedString(933) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													291,
													452,
													92,
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
		pButton->SetToolTip( LoadLocalizedString(934) );

		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													291,
													493,
													92,
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
		pButton->SetToolTip( LoadLocalizedString(935) );


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
									   KSpriteTopLeft );
	}


	m_pStrengthButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												140,
												254,
												89,
												32,
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
	m_pStrengthButton->SetToolTip( "0" );

	m_pDexterityButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												140,
												334,
												89,
												32,
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
	m_pDexterityButton->SetToolTip( "0" );

	m_pVitalityButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												140,
												415,
												89,
												32,
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
	m_pVitalityButton->SetToolTip( "0" );

	m_pMagicButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												140,
												494,
												89,
												32,
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
	m_pMagicButton->SetToolTip( "0" );


	m_pDamageButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												387,
												254,
												86,
												32,
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
	m_pDamageButton->SetToolTip( "0" );


	m_pAttackButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												387,
												334,
												86,
												32,
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
	m_pAttackButton->SetToolTip( "0" );

	m_pDefenseButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												387,
												370,
												86,
												32,
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
	m_pDefenseButton->SetToolTip( "0" );

	m_pStaminaButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												387,
												415,
												86,
												32,
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
	m_pStaminaButton->SetToolTip( "0" );

	m_pLifeButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												387,
												452,
												86,
												32,
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
	m_pLifeButton->SetToolTip( "0" );

	m_pManaButton = AddButton( new CButton( pD3DDevice,
												NULL,
												NULL,
												NULL,
												387,
												494,
												86,
												32,
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
	m_pManaButton->SetToolTip( "0" );


} // CStatsMenu::CreateMenus()


void CStatsMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CStatsMenu::KeyEvent()

void CStatsMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CStatsMenu::KeyEvent()

bool CStatsMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}

	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( MousePosition.x <= 512 )
	{
		return kTrue;
	}
	return kFalse;
} // CStats::MouseOver()

void CStatsMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CStatsMenu::ProcessInput()

void CStatsMenu::Update( HWND hWnd,						// hwnd of the active window
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


	if( IsGameUnlocked() || ( !IsGameUnlocked() && m_pPlayerCharacter->Level() <= KDemoPlayerLevel ) ) //marsh added in Demo mode
	{
		switch( ButtonPressed )
		{
		case KStatButtonAddStrength :
			m_pPlayerCharacter->SpendStrengthPoint();
			FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
			break;
		case KStatButtonAddDexterity :
			m_pPlayerCharacter->SpendDexterityPoint();
			FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
			break;
		case KStatButtonAddVitality :
			m_pPlayerCharacter->SpendVitalityPoint();
			FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
			break;
		case KStatButtonAddMagic :
			m_pPlayerCharacter->SpendMagicPoint();
			FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
			break;
		}
	}
	else
	{
		switch( ButtonPressed )
		{
		case KStatButtonAddStrength :
		case KStatButtonAddDexterity :
		case KStatButtonAddVitality :
		case KStatButtonAddMagic :
			m_MessagePending = kTrue;
			break;
		}
	}

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CStatsMenu::Update()

// render any 3d elements
void CStatsMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
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
		// if the player has stat points to distribute, show buttons
		// that they can click to upgrade them
		if( m_pPlayerCharacter->UnusedStatPoints() > 0 )
		{
			ShowUpgradeButtons( pD3DDevice );
		}
		else
		{
			RemoveUpgradeButtons();
		}

		if( m_pPlayerCharacter->Lineage() > 0 )
		{
			m_pName->SetText( pD3DDevice, LoadLocalizedString3Variables(989, m_pPlayerCharacter->Name(), KFameName[ m_pPlayerCharacter->FameRank() ], RomanNumeral( m_pPlayerCharacter->Lineage() ) ));
		}
		else
		{
			m_pName->SetText( pD3DDevice, LoadLocalizedString2Variables(990, m_pPlayerCharacter->Name(), KFameName[ m_pPlayerCharacter->FameRank() ] ));
		}
		if( m_pName->Text() != m_LastDisplayedName )
		{
			m_LastDisplayedName = m_pName->Text();
			float32 Width = m_pName->MaxBounds().x -
							m_pName->MinBounds().x;
			if( Width > 315 )
			{
				float32 ScaleMultiplier = 315.0f / Width;
				m_pName->SetScale( pD3DDevice, .8f * ScaleMultiplier );

			}
			else 
			{
				m_pName->SetScale( pD3DDevice, .8f );
			}
		}
		m_pStrength->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Strength() ) );
		m_pDexterity->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Dexterity() ) );
		m_pVitality->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Vitality() ) );
		m_pMagic->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Magic() ) );

		m_pExperience->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Experience() ) );

		m_pNextExperience->SetText( pD3DDevice, IntegerToString( ExperienceGate( m_pPlayerCharacter->Level() ) ) );

		m_pLevel->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Level() ) );

		
		m_pRenown->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->FameRank() ) );

		m_pFame->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->Fame() ) );

		m_pNextFame->SetText( pD3DDevice, IntegerToString( KFameGate[m_pPlayerCharacter->FameRank()]) );

		int32 MinDamage = m_pPlayerCharacter->MinimumDamage() + m_pPlayerCharacter->CharacterDamage() + m_pPlayerCharacter->ElementalDamage();
		int32 MaxDamage = m_pPlayerCharacter->MaximumDamage() + m_pPlayerCharacter->CharacterDamage() + m_pPlayerCharacter->ElementalDamage();
		std::string DamageRange = IntegerToString( MinDamage ) + "-" +
								  IntegerToString( MaxDamage );
		m_pDamage->SetText( pD3DDevice, DamageRange );

		m_pToHit->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->ToHit() ) );

		m_pDefense->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->AC() ) );

		m_pLife->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->MaxHP() ) );

		m_pStamina->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->MaxStamina() ) );

		m_pMana->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->MaxMana() ) );

		m_pStatPoints->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->UnusedStatPoints() ) );
	}

	if( m_pPlayerCharacter->Strength() > m_pPlayerCharacter->BaseStrength() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 140, 254 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pStrengthButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseStrength() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->Strength() - m_pPlayerCharacter->BaseStrength() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->Strength() < m_pPlayerCharacter->BaseStrength() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 140, 254 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pStrengthButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseStrength() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseStrength() - m_pPlayerCharacter->Strength() ) + 
									"\b" );
	}
	else
	{
		m_pStrengthButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseStrength() ) );
	}

	if( m_pPlayerCharacter->Dexterity() > m_pPlayerCharacter->BaseDexterity() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 140, 333 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pDexterityButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseDexterity() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->Dexterity() - m_pPlayerCharacter->BaseDexterity() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->Dexterity() < m_pPlayerCharacter->BaseDexterity() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 140, 333 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pDexterityButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseDexterity() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseDexterity() - m_pPlayerCharacter->Dexterity() ) + 
									"\b" );

	}
	else
	{
		m_pDexterityButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseDexterity() ) );
	}

	if( m_pPlayerCharacter->Vitality() > m_pPlayerCharacter->BaseVitality() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 140, 415 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pVitalityButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseVitality() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->Vitality() - m_pPlayerCharacter->BaseVitality() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->Vitality() < m_pPlayerCharacter->BaseVitality() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 140, 415 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pVitalityButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseVitality() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseVitality() - m_pPlayerCharacter->Vitality() ) + 
									"\b" );
	}
	else
	{
		m_pVitalityButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseVitality() ) );
	}

	if( m_pPlayerCharacter->Magic() > m_pPlayerCharacter->BaseMagic() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 140, 493 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pMagicButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMagic() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->Magic() - m_pPlayerCharacter->BaseMagic() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->Magic() < m_pPlayerCharacter->BaseMagic() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 140, 493 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pMagicButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMagic() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseMagic() - m_pPlayerCharacter->Magic() ) + 
									"\b" );
	}
	else
	{
		m_pMagicButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMagic() ) );
	}


	int32 MinDamage = m_pPlayerCharacter->MinimumDamage() + m_pPlayerCharacter->CharacterDamage();
	int32 MaxDamage = m_pPlayerCharacter->MaximumDamage() + m_pPlayerCharacter->CharacterDamage();

	int32 BaseMinDamage = m_pPlayerCharacter->BaseMinimumDamage() + m_pPlayerCharacter->BaseCharacterDamage();
	int32 BaseMaxDamage = m_pPlayerCharacter->BaseMaximumDamage() + m_pPlayerCharacter->BaseCharacterDamage();

	std::string ElementalText = "";
	for( uint32 i = 0; i < KDamageTypes; i++ )
	{
		int32 Damage = m_pPlayerCharacter->ElementalDamage( (EDamageType)i );
		if( Damage != 0 )
		{
			ElementalText = ElementalText + "\n" + IntegerToString( Damage ) + " \b" + KDamageTypeDescription[i] + "\b";
		}
	}

	if( MaxDamage > BaseMaxDamage )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 387, 254 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pDamageButton->SetToolTip( IntegerToString( BaseMinDamage ) +
									"-" + IntegerToString( BaseMaxDamage ) +
									" + \b" + 
									IntegerToString( MaxDamage - BaseMaxDamage ) + 
									"\b" + ElementalText );
	}
	else if( MaxDamage < BaseMaxDamage )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 387, 254 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pDamageButton->SetToolTip( IntegerToString( BaseMinDamage ) +
									"-" + IntegerToString( BaseMaxDamage ) +
									" - \b" + 
									IntegerToString( BaseMaxDamage - MaxDamage ) + 
									"\b" + ElementalText );
	}
	else
	{
		if( ElementalText.length() > 0 )
		{
			m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
			m_pHighlight[KHighlightGreen]->SetPosition( 387, 254 );
			m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		}
		m_pDamageButton->SetToolTip( IntegerToString( BaseMinDamage ) +
									"-" + IntegerToString( BaseMaxDamage ) + ElementalText );
	}

	if( m_pPlayerCharacter->ToHit() > m_pPlayerCharacter->BaseToHit() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 387, 334 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pAttackButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseToHit() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->ToHit() - m_pPlayerCharacter->BaseToHit() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->ToHit() < m_pPlayerCharacter->BaseToHit() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 387, 334 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pAttackButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseToHit() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseToHit() - m_pPlayerCharacter->ToHit() ) + 
									"\b" );
	}
	else
	{
		m_pAttackButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseToHit() ) );
	}


	if( m_pPlayerCharacter->AC() > m_pPlayerCharacter->BaseAC() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 387, 370 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pDefenseButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseAC() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->AC() - m_pPlayerCharacter->BaseAC() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->AC() < m_pPlayerCharacter->BaseAC() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 387, 370 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pDefenseButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseAC() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseAC() - m_pPlayerCharacter->AC() ) + 
									"\b" );
	}
	else
	{
		m_pDefenseButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseAC() ) );
	}

	if( m_pPlayerCharacter->MaxStamina() > m_pPlayerCharacter->BaseMaxStamina() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 387, 415 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pStaminaButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxStamina() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->MaxStamina() - m_pPlayerCharacter->BaseMaxStamina() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->MaxStamina() < m_pPlayerCharacter->BaseMaxStamina() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 387, 415 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pStaminaButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxStamina() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseMaxStamina() - m_pPlayerCharacter->MaxStamina() ) + 
									"\b" );
	}
	else
	{
		m_pStaminaButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxStamina() ) );
	}

	if( m_pPlayerCharacter->MaxHP() > m_pPlayerCharacter->BaseMaxHP() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 387, 452 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pLifeButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxHP() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->MaxHP() - m_pPlayerCharacter->BaseMaxHP() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->MaxHP() < m_pPlayerCharacter->BaseMaxHP() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 387, 452 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pLifeButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxHP() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseMaxHP() - m_pPlayerCharacter->MaxHP() ) + 
									"\b" );
	}
	else
	{
		m_pLifeButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxHP() ) );
	}

	if( m_pPlayerCharacter->MaxMana() > m_pPlayerCharacter->BaseMaxMana() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 387, 493 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pManaButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxMana() ) + 
									" + \b" + 
									IntegerToString( m_pPlayerCharacter->MaxMana() - m_pPlayerCharacter->BaseMaxMana() ) + 
									"\b" );
	}
	else if( m_pPlayerCharacter->MaxMana() < m_pPlayerCharacter->BaseMaxMana() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 387, 493 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pManaButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxMana() ) + 
									" - \b" + 
									IntegerToString( m_pPlayerCharacter->BaseMaxMana() - m_pPlayerCharacter->MaxMana() ) + 
									"\b" );
	}
	else
	{
		m_pManaButton->SetToolTip( IntegerToString( m_pPlayerCharacter->BaseMaxMana() ) );
	}

	RenderText( pD3DDevice );		

} // CStatsMenu::Render()


void CStatsMenu::ShowUpgradeButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_UpgradeButtonsVisible )
	{

		m_pAddStrengthButton = AddButton( new CButton( pD3DDevice,
														m_pUIButtons,
														234,
														244,
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
														KStatButtonAddStrength ) );

		m_pAddDexterityButton = AddButton( new CButton( pD3DDevice,
														m_pUIButtons,
														234,
														324,
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
														KStatButtonAddDexterity ) );

		m_pAddVitalityButton = AddButton( new CButton( pD3DDevice,
														m_pUIButtons,
														234,
														404,
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
														KStatButtonAddVitality ) );

		m_pAddMagicButton = AddButton( new CButton( pD3DDevice,
														m_pUIButtons,
														234,
														485,
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
														KStatButtonAddMagic ) );

		m_UpgradeButtonsVisible = kTrue;
	}
} // CStatsMenu::ShowUpgradeButtons()

void CStatsMenu::RemoveUpgradeButtons( void )
{
	if( m_UpgradeButtonsVisible )
	{
		RemoveButton( m_pAddStrengthButton );
		DELETE_SAFELY( m_pAddStrengthButton );
		
		RemoveButton( m_pAddDexterityButton );
		DELETE_SAFELY( m_pAddDexterityButton );
		
		RemoveButton( m_pAddVitalityButton );
		DELETE_SAFELY( m_pAddVitalityButton );
		
		RemoveButton( m_pAddMagicButton );
		DELETE_SAFELY( m_pAddMagicButton );
		m_UpgradeButtonsVisible = kFalse;
	}
} // CStatsMenu::RemoveUpgradeButtons()

void CStatsMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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

} // CStatsMenu::RenderText()

void CStatsMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CStatsMenu::RenderSprites()

void CStatsMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CStatsMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CStatsMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CStatsMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CStatsMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CStatsMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CStatsMenu::UpdateButtonOver( void )
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
} // CStatsMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CStatsMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CStatsMenu::UpdateButtonOver()

CButton* CStatsMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CStatsMenu::AddButton()

void CStatsMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CStatsMenu::RemoveButton()

void CStatsMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CStatsMenu::DestroyButtons()

CText* CStatsMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CStatsMenu::AddText()

void CStatsMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CStatsMenu::RemoveText()

void CStatsMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CStatsMenu::DestroyText()

CSprite* CStatsMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CStatsMenu::AddSprite()

CSprite* CStatsMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CStatsMenu::AddSpriteFront()

void CStatsMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CStatsMenu::RemoveSprite()

void CStatsMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CStatsMenu::DestroySprite()

