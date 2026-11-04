/*
	petstatsmenu.cpp

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
#include "petstatsmenu.h"
#include "bottommenu.h"

CPetStatsMenu::CPetStatsMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							  CRefManager& pRefManager,					// the reference manager, for tracking media we create
							  CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
							  CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																		m_pGameStateManager( pGameStateManager ),
																		m_pRefManager( pRefManager ),
																		m_pSettings( pSettings ),
																		m_Open( kFalse ),
																		m_pPetCharacter( NULL ),
																		m_pAddStrengthButton( NULL ),
																		m_pAddDexterityButton( NULL ),
																		m_pAddVitalityButton( NULL ),
																		m_pAddMagicButton( NULL ),
																		m_UpgradeButtonsVisible( kFalse ),
																		m_pButtonList	( new CLList<CButton>() ),
																		m_pTextList		( new CLList<CText>() ),
																		m_pSpriteList	( new CLList<CSprite>() ),
																		m_pFontMaterial( NULL ),
																		m_pFontMetric( NULL ),
																		m_pPetStatsMenuTop( NULL ),
																		m_pPetStatsMenuCenter( NULL ),
																		m_pPetStatsMenuBottom( NULL ),
																		m_pUIButtons( NULL ),
																		m_pFontHighlightMaterial( NULL ),
																		m_pName( NULL ),
																		m_pLevel( NULL ),
																		m_pExperience( NULL ),
																		m_pNextExperience( NULL ),
																		m_pStrength( NULL ),
																		m_pDexterity( NULL ),
																		m_pVitality( NULL ),
																		m_pMagic( NULL ),
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

} // CPetStatsMenu::CPetStatsMenu()

CPetStatsMenu::~CPetStatsMenu( void )
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

	m_pRefManager.RemoveMaterial( m_pPetStatsMenuTop );
	m_pRefManager.RemoveMaterial( m_pPetStatsMenuBottom );
	m_pRefManager.RemoveMaterial( m_pPetStatsMenuCenter );

} // CPetStatsMenu::~CPetStatsMenu()

void CPetStatsMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CPetStatsMenu::FlushAll()

void CPetStatsMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	m_pGivePointSound = m_pRefManager.AddSample( "SOUNDS/UI/givepoint.wav", kFalse );

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pPetStatsMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/statstop.png" );

	m_pPetStatsMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/petstatsbottom.png"  );

	m_pPetStatsMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/statscenter.png" );

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
									KButtonPetStats ) );
	pButton->SetToolTip( LoadLocalizedString(170) );


	AddSprite( new CSprite( pD3DDevice,
							m_pPetStatsMenuTop,
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
							m_pPetStatsMenuCenter,
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
							m_pPetStatsMenuBottom,
							512,
							512,
							512,
							128,
							0,
							.01f,
							1,
							.99f,
							KSpriteTopLeft ) );

	m_pName = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"",
							767,
							60,
							1.0f,
							kFalse,
							KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(916),
						609,
						122,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pLevel = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"1",
									609,
									134,
									.9f,
									kFalse,
									KJustifyCenter ) );


	m_pExperience = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"0",
									744,
									134,
									.9f,
									kFalse,
									KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(917),
						744,
						122,
						.7f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(918),
						901,
						122,
						.7f,
						kFalse,
						KJustifyCenter ) );

	m_pNextExperience = AddText( new CText( pD3DDevice,
									m_pFontMaterial,
									m_pFontMetric,
									"0",
									901,
									134,
									.9f,
									kFalse,
									KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(291),
						594,
						258,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pStrength = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"0",
							695,
							257,
							.9f,
							kFalse,
							KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(919),
							849,
							258,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pDamage = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0-0",
								941,
								257,
								.9f,
								kFalse,
								KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(292),
						594,
						338,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pDexterity = AddText( new CText( pD3DDevice,
										m_pFontMaterial,
										m_pFontMetric,
										"0",
										695,
										337,
										.9f,
										kFalse,
										KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(920),
							849,
							339,
							.6f,
							kFalse,
							KJustifyCenter ) );

		m_pToHit = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								941,
								337,
								.9f,
								kFalse,
								KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(921),
							849,
							373,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pDefense = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								941,
								372,
								.9f,
								kFalse,
								KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(293),
						594,
						418,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pVitality = AddText( new CText( pD3DDevice,
										m_pFontMaterial,
										m_pFontMetric,
										"0",
										695,
										417,
										.9f,
										kFalse,
										KJustifyCenter ) );
		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(922),
							849,
							418,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pStamina = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								941,
								417,
								.9f,
								kFalse,
								KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(923),
							849,
							453,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pLife = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								941,
								452,
								.9f,
								kFalse,
								KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(924),
						594,
						498,
						.8f,
						kFalse,
						KJustifyCenter ) );

	m_pMagic = AddText( new CText( pD3DDevice,
								   m_pFontMaterial,
								   m_pFontMetric,
								   "0",
								   695,
								   497,
								   .9f,
								   kFalse,
								   KJustifyCenter ) );

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(925),
							849,
							498,
							.8f,
							kFalse,
							KJustifyCenter ) );

		m_pMana = AddText( new CText( pD3DDevice,
								m_pFontMaterial,
								m_pFontMetric,
								"0",
								941,
								497,
								.9f,
								kFalse,
								KJustifyCenter ) );


		// add context tooltips
		pButton = AddButton( new CButton( pD3DDevice,
													NULL,
													NULL,
													NULL,
													551,
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
													551,
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
													551,
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
													551,
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
													803,
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
													803,
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
													803,
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
													803,
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
													803,
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
													803,
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
												652,
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
												652,
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
												652,
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
												652,
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
												899,
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
												899,
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
												899,
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
												899,
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
												899,
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
												899,
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

} // CPetStatsMenu::CreateMenus()


void CPetStatsMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CPetStatsMenu::KeyEvent()

void CPetStatsMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CPetStatsMenu::KeyEvent()

bool CPetStatsMenu::MouseOver( void )
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
} // CPetStats::MouseOver()

void CPetStatsMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CPetStatsMenu::ProcessInput()

void CPetStatsMenu::Update( HWND hWnd,						// hwnd of the active window
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

	switch( ButtonPressed )
	{
	case KStatButtonAddStrength :
		m_pPetCharacter->SpendStrengthPoint();
		FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
		break;
	case KStatButtonAddDexterity :
		m_pPetCharacter->SpendDexterityPoint();
		FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
		break;
	case KStatButtonAddVitality :
		m_pPetCharacter->SpendVitalityPoint();
		FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
		break;
	case KStatButtonAddMagic :
		m_pPetCharacter->SpendMagicPoint();
		FSOUND_PlaySound( FSOUND_FREE, m_pGivePointSound );
		break;
	}

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CPetStatsMenu::Update()

// render any 3d elements
void CPetStatsMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
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


	if( m_pPetCharacter != NULL )
	{
		// if the player has stat points to distribute, show buttons
		// that they can click to upgrade them
		if( m_pPetCharacter->UnusedStatPoints() > 0 )
		{
			ShowUpgradeButtons( pD3DDevice );
		}
		else
		{
			RemoveUpgradeButtons();
		}
		m_pName->SetText( pD3DDevice, m_pPetCharacter->Name() );

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

		m_pStrength->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->Strength() ) );
		m_pDexterity->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->Dexterity() ) );
		m_pVitality->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->Vitality() ) );
		m_pMagic->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->Magic() ) );

		m_pExperience->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->Experience() ) );

		m_pNextExperience->SetText( pD3DDevice, IntegerToString( ExperienceGate( m_pPetCharacter->Level() ) ) );

		m_pLevel->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->Level() ) );

		int32 MinDamage = m_pPetCharacter->MinimumDamage() + m_pPetCharacter->CharacterDamage();
		int32 MaxDamage = m_pPetCharacter->MaximumDamage() + m_pPetCharacter->CharacterDamage();
		std::string DamageRange = IntegerToString( MinDamage ) + "-" +
								  IntegerToString( MaxDamage );
		m_pDamage->SetText( pD3DDevice, DamageRange );

		m_pToHit->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->ToHit() ) );

		m_pDefense->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->AC() ) );

		m_pLife->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->MaxHP() ) );

		m_pStamina->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->MaxStamina() ) );

		m_pMana->SetText( pD3DDevice, IntegerToString( m_pPetCharacter->MaxMana() ) );

	}

	if( m_pPetCharacter->Strength() > m_pPetCharacter->BaseStrength() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 652, 254 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pStrengthButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseStrength() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->Strength() - m_pPetCharacter->BaseStrength() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->Strength() < m_pPetCharacter->BaseStrength() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 652, 254 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pStrengthButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseStrength() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseStrength() - m_pPetCharacter->Strength() ) + 
									"\b" );
	}
	else
	{
		m_pStrengthButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseStrength() ) );
	}

	if( m_pPetCharacter->Dexterity() > m_pPetCharacter->BaseDexterity() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 652, 333 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pDexterityButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseDexterity() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->Dexterity() - m_pPetCharacter->BaseDexterity() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->Dexterity() < m_pPetCharacter->BaseDexterity() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 652, 333 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pDexterityButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseDexterity() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseDexterity() - m_pPetCharacter->Dexterity() ) + 
									"\b" );

	}
	else
	{
		m_pDexterityButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseDexterity() ) );
	}

	if( m_pPetCharacter->Vitality() > m_pPetCharacter->BaseVitality() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 652, 415 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pVitalityButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseVitality() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->Vitality() - m_pPetCharacter->BaseVitality() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->Vitality() < m_pPetCharacter->BaseVitality() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 652, 415 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pVitalityButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseVitality() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseVitality() - m_pPetCharacter->Vitality() ) + 
									"\b" );
	}
	else
	{
		m_pVitalityButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseVitality() ) );
	}

	if( m_pPetCharacter->Magic() > m_pPetCharacter->BaseMagic() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 88, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 652, 493 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pMagicButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMagic() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->Magic() - m_pPetCharacter->BaseMagic() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->Magic() < m_pPetCharacter->BaseMagic() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 88, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 652, 493 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pMagicButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMagic() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseMagic() - m_pPetCharacter->Magic() ) + 
									"\b" );
	}
	else
	{
		m_pMagicButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMagic() ) );
	}


	int32 MinDamage = m_pPetCharacter->MinimumDamage() + m_pPetCharacter->CharacterDamage();
	int32 MaxDamage = m_pPetCharacter->MaximumDamage() + m_pPetCharacter->CharacterDamage();

	int32 BaseMinDamage = m_pPetCharacter->BaseMinimumDamage() + m_pPetCharacter->BaseCharacterDamage();
	int32 BaseMaxDamage = m_pPetCharacter->BaseMaximumDamage() + m_pPetCharacter->BaseCharacterDamage();

	if( MaxDamage > BaseMaxDamage )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 899, 254 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pDamageButton->SetToolTip( IntegerToString( BaseMinDamage ) +
									"-" + IntegerToString( BaseMaxDamage ) +
									" + \b" + 
									IntegerToString( MaxDamage - BaseMaxDamage ) + 
									"\b" );
	}
	else if( MaxDamage < BaseMaxDamage )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 899, 254 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pDamageButton->SetToolTip( IntegerToString( BaseMinDamage ) +
									"-" + IntegerToString( BaseMaxDamage ) +
									" - \b" + 
									IntegerToString( BaseMaxDamage - MaxDamage ) + 
									"\b" );
	}
	else
	{
		m_pDamageButton->SetToolTip( IntegerToString( BaseMinDamage ) +
									"-" + IntegerToString( BaseMaxDamage ) );
	}

	if( m_pPetCharacter->ToHit() > m_pPetCharacter->BaseToHit() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 899, 334 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pAttackButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseToHit() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->ToHit() - m_pPetCharacter->BaseToHit() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->ToHit() < m_pPetCharacter->BaseToHit() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 899, 334 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pAttackButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseToHit() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseToHit() - m_pPetCharacter->ToHit() ) + 
									"\b" );
	}
	else
	{
		m_pAttackButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseToHit() ) );
	}


	if( m_pPetCharacter->AC() > m_pPetCharacter->BaseAC() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 899, 370 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pDefenseButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseAC() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->AC() - m_pPetCharacter->BaseAC() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->AC() < m_pPetCharacter->BaseAC() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 899, 370 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pDefenseButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseAC() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseAC() - m_pPetCharacter->AC() ) + 
									"\b" );
	}
	else
	{
		m_pDefenseButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseAC() ) );
	}

	if( m_pPetCharacter->MaxStamina() > m_pPetCharacter->BaseMaxStamina() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 899, 415 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pStaminaButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxStamina() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->MaxStamina() - m_pPetCharacter->BaseMaxStamina() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->MaxStamina() < m_pPetCharacter->BaseMaxStamina() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 899, 415 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pStaminaButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxStamina() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseMaxStamina() - m_pPetCharacter->MaxStamina() ) + 
									"\b" );
	}
	else
	{
		m_pStaminaButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxStamina() ) );
	}

	if( m_pPetCharacter->MaxHP() > m_pPetCharacter->BaseMaxHP() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 899, 452 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pLifeButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxHP() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->MaxHP() - m_pPetCharacter->BaseMaxHP() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->MaxHP() < m_pPetCharacter->BaseMaxHP() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 899, 452 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pLifeButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxHP() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseMaxHP() - m_pPetCharacter->MaxHP() ) + 
									"\b" );
	}
	else
	{
		m_pLifeButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxHP() ) );
	}

	if( m_pPetCharacter->MaxMana() > m_pPetCharacter->BaseMaxMana() )
	{
		m_pHighlight[KHighlightGreen]->SetSize( 86, 33 );
		m_pHighlight[KHighlightGreen]->SetPosition( 899, 493 );
		m_pHighlight[KHighlightGreen]->Render( pD3DDevice );
		m_pManaButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxMana() ) + 
									" + \b" + 
									IntegerToString( m_pPetCharacter->MaxMana() - m_pPetCharacter->BaseMaxMana() ) + 
									"\b" );
	}
	else if( m_pPetCharacter->MaxMana() < m_pPetCharacter->BaseMaxMana() )
	{
		m_pHighlight[KHighlightRed]->SetSize( 86, 33 );
		m_pHighlight[KHighlightRed]->SetPosition( 899, 493 );
		m_pHighlight[KHighlightRed]->Render( pD3DDevice );
		m_pManaButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxMana() ) + 
									" - \b" + 
									IntegerToString( m_pPetCharacter->BaseMaxMana() - m_pPetCharacter->MaxMana() ) + 
									"\b" );
	}
	else
	{
		m_pManaButton->SetToolTip( IntegerToString( m_pPetCharacter->BaseMaxMana() ) );
	}

	RenderText( pD3DDevice );		

} // CPetStatsMenu::Render()


void CPetStatsMenu::ShowUpgradeButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_UpgradeButtonsVisible )
	{

		m_pAddStrengthButton = AddButton( new CButton( pD3DDevice,
														m_pUIButtons,
														241,
														190,
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
														241,
														270,
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
														241,
														350,
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
														241,
														430,
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
} // CPetStatsMenu::ShowUpgradeButtons()

void CPetStatsMenu::RemoveUpgradeButtons( void )
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
} // CPetStatsMenu::RemoveUpgradeButtons()


void CPetStatsMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CPetStatsMenu::RenderText()

void CPetStatsMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CPetStatsMenu::RenderSprites()

void CPetStatsMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CPetStatsMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CPetStatsMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CPetStatsMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CPetStatsMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CPetStatsMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CPetStatsMenu::UpdateButtonOver( void )
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
} // CPetStatsMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CPetStatsMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CPetStatsMenu::UpdateButtonOver()

CButton* CPetStatsMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CPetStatsMenu::AddButton()

void CPetStatsMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CPetStatsMenu::RemoveButton()

void CPetStatsMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CPetStatsMenu::DestroyButtons()

CText* CPetStatsMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CPetStatsMenu::AddText()

void CPetStatsMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CPetStatsMenu::RemoveText()

void CPetStatsMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CPetStatsMenu::DestroyText()

CSprite* CPetStatsMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CPetStatsMenu::AddSprite()

CSprite* CPetStatsMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CPetStatsMenu::AddSpriteFront()

void CPetStatsMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CPetStatsMenu::RemoveSprite()

void CPetStatsMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CPetStatsMenu::DestroySprite()


