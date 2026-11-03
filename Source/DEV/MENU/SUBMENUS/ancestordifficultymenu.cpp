/*
	ancestordifficultymenu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/


#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include "../menu.h"
#include "../../UI/fontmetric.h"
#include "../../UTILITIES/spawn.h"
#include "../../NAMES/namegenerator.h"
#include "../../UTILITIES/loadlocalizedstring.h"

#include "ancestordifficultymenu.h"


CAncestorDifficultyMenu::CAncestorDifficultyMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
					  CSettings& pSettings,		// the settings manager, for keeping track of card/machine capabilities
					  CGameStateManager& pGameStateManager,	// game manager
					  CMenuManager& pMenuManager ) :		// menu manager
														CMenu( pRefManager,
																pSettings,
																pGameStateManager,
																pMenuManager ),
														m_pLogoMaterial( NULL ),
														m_pFontMetric( NULL ),
														m_pButtonMaterial( NULL ),
														m_pUIButtonMaterial( NULL ),
														m_pFontMaterial( NULL ),
														m_Difficulty( KDifficultyAdventurer ),
														m_pClickSound( NULL ),
														m_pPageButton( NULL ),
														m_pAdventurerButton( NULL ),
														m_pHeroButton( NULL ),
														m_pLegendButton( NULL )						
{

} // CAncestorDifficultyMenu::CAncestorDifficultyMenu()

CAncestorDifficultyMenu::~CAncestorDifficultyMenu( void )
{
	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );
	m_pRefManager.RemoveMaterial( m_pUIButtonMaterial );


} // CAncestorDifficultyMenu::~CAncestorDifficultyMenu()

void CAncestorDifficultyMenu::Activate( void )
{
	CMenu::Activate();
} // CAncestorDifficultyMenu::Activate()

// perform creation of any elements that this menu needs
void CAncestorDifficultyMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );

	m_pLogoMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													  "MENUS/logo.png",
													  "MENUS/logoa.png" );
	m_pButtonMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons.png",
													    "MENUS/buttonsa.png" );
	m_pUIButtonMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/uibuttons.png",
													    "MENUS/uibuttonsa.png" );
	// game logo
	AddSprite( new CSprite( pD3DDevice,
							m_pLogoMaterial,
							16,
							20,
							512,
							256,
							0,
							0,
							1,
							1,
							KSpriteTopLeft ) );

	
// new game button
	CButton* pButton;

// options button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  700,
									  580,
									  256,
									  48,
									  0 * m_pButtonMaterial->PixelToU(),
									  49 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  97 * m_pButtonMaterial->PixelToV(),
									  0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV(),
									  KButtonTopLeft,
									  KAncestorDifficultyMenuAccept ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(208),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(208),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1004),
						512,
						290,
						1.0f,
						kFalse,
						KJustifyCenter ) );


// difficulty
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  512,
									  350,
									  256,
									  48,
									  0 * m_pButtonMaterial->PixelToU(),
									  49 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  97 * m_pButtonMaterial->PixelToV(),
									  0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV(),
									  KButtonCenter,
									  KAncestorDifficultyMenuPage ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1005),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1005),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );
	pButton->SetToolTip( LoadLocalizedString(1006) );
	m_pPageButton = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  512,
									  400,
									  256,
									  48,
									  0 * m_pButtonMaterial->PixelToU(),
									  49 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  97 * m_pButtonMaterial->PixelToV(),
									  0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV(),
									  KButtonCenter,
									  KAncestorDifficultyMenuAdventurer ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1007),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1007),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );
	pButton->SetToolTip( LoadLocalizedString(1008) );
	m_pAdventurerButton = pButton;


	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  512,
									  450,
									  256,
									  48,
									  0 * m_pButtonMaterial->PixelToU(),
									  49 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  97 * m_pButtonMaterial->PixelToV(),
									  0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV(),
									  KButtonCenter,
									  KAncestorDifficultyMenuHero ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1009),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1009),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );
	pButton->SetToolTip( LoadLocalizedString(1010) );
	m_pHeroButton = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  512,
									  500,
									  256,
									  48,
									  0 * m_pButtonMaterial->PixelToU(),
									  49 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  97 * m_pButtonMaterial->PixelToV(),
									  0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV(),
									  KButtonCenter,
									  KAncestorDifficultyMenuLegend ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1011),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1011),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );
	pButton->SetToolTip( LoadLocalizedString(1012) );
	m_pLegendButton = pButton;

} // CAncestorDifficultyMenu::Create()

void CAncestorDifficultyMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );


} // CAncestorDifficultyMenu::KeyEvent()


void CAncestorDifficultyMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we go to character creation
	case KAncestorDifficultyMenuBack :
		m_pGameStateManager.SetTargetMenu( KMainMenu );
		break;
	// if pressed, select difficulty
	case KAncestorDifficultyMenuPage :
		m_Difficulty = KDifficultyPage;
		break;
	case KAncestorDifficultyMenuAdventurer :
		m_Difficulty = KDifficultyAdventurer;
		break;
	case KAncestorDifficultyMenuHero :
		m_Difficulty = KDifficultyHero;
		break;
	case KAncestorDifficultyMenuLegend :
		m_Difficulty = KDifficultyLegend;
		break;
	// if pressed, we request an exit
	case KAncestorDifficultyMenuAccept :
		Begin();
		break;
	}

} // CAncestorDifficultyMenu::ProcessInput()

void CAncestorDifficultyMenu::Begin( void )
{
	m_pMenuManager.GameClient().SetDifficulty( m_Difficulty );
	m_pGameStateManager.SetTargetMenu( KAncestorMenu );
} // CAncestorDifficultyMenu::Begin()


void CAncestorDifficultyMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );
	

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CAncestorDifficultyMenu::Update()

void CAncestorDifficultyMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	m_pPageButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
						   49 * m_pButtonMaterial->PixelToV(),
						   256 * m_pButtonMaterial->PixelToU(),
						   97 * m_pButtonMaterial->PixelToV() );
	m_pAdventurerButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
						   49 * m_pButtonMaterial->PixelToV(),
						   256 * m_pButtonMaterial->PixelToU(),
						   97 * m_pButtonMaterial->PixelToV() );
	m_pHeroButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
						   49 * m_pButtonMaterial->PixelToV(),
						   256 * m_pButtonMaterial->PixelToU(),
						   97 * m_pButtonMaterial->PixelToV() );
	m_pLegendButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
						   49 * m_pButtonMaterial->PixelToV(),
						   256 * m_pButtonMaterial->PixelToU(),
						   97 * m_pButtonMaterial->PixelToV() );


	m_pPageButton->SetOverUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
	m_pAdventurerButton->SetOverUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
	m_pHeroButton->SetOverUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
	m_pLegendButton->SetOverUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );

	switch( m_Difficulty )
	{
	case KDifficultyPage :
	m_pPageButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
		break;
	case KDifficultyAdventurer :
		m_pAdventurerButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
		break;
	case KDifficultyHero :
		m_pHeroButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
		break;
	case KDifficultyLegend :
		m_pLegendButton->SetUVs( 0 * m_pButtonMaterial->PixelToU(),
									  0 * m_pButtonMaterial->PixelToV(),
									  256 * m_pButtonMaterial->PixelToU(),
									  48 * m_pButtonMaterial->PixelToV() );
		break;
	}
/*
	if( m_Gender == KMale )
	{
		m_pMaleButton->SetUVs( 155 * m_pUIButtonMaterial->PixelToU(),
							   155 * m_pUIButtonMaterial->PixelToV(),
							   203 * m_pUIButtonMaterial->PixelToU(),
							   203 * m_pUIButtonMaterial->PixelToV() );
		m_pMaleButton->SetOverUVs( 155 * m_pUIButtonMaterial->PixelToU(),
								   155 * m_pUIButtonMaterial->PixelToV(),
								   203 * m_pUIButtonMaterial->PixelToU(),
								   203 * m_pUIButtonMaterial->PixelToV() );

		m_pFemaleButton->SetUVs( 206 * m_pUIButtonMaterial->PixelToU(),
								 103 * m_pUIButtonMaterial->PixelToV(),
								 254 * m_pUIButtonMaterial->PixelToU(),
								 151 * m_pUIButtonMaterial->PixelToV() );
		m_pFemaleButton->SetOverUVs( 206 * m_pUIButtonMaterial->PixelToU(),
									 155 * m_pUIButtonMaterial->PixelToV(),
									 254 * m_pUIButtonMaterial->PixelToU(),
									 203 * m_pUIButtonMaterial->PixelToV() );
	}
	else
	{
		m_pMaleButton->SetUVs( 155 * m_pUIButtonMaterial->PixelToU(),
							   103 * m_pUIButtonMaterial->PixelToV(),
							   203 * m_pUIButtonMaterial->PixelToU(),
							   151 * m_pUIButtonMaterial->PixelToV() );
		m_pMaleButton->SetOverUVs( 155 * m_pUIButtonMaterial->PixelToU(),
								   155 * m_pUIButtonMaterial->PixelToV(),
								   203 * m_pUIButtonMaterial->PixelToU(),
								   203 * m_pUIButtonMaterial->PixelToV() );

		m_pFemaleButton->SetUVs( 206 * m_pUIButtonMaterial->PixelToU(),
									 155 * m_pUIButtonMaterial->PixelToV(),
									 254 * m_pUIButtonMaterial->PixelToU(),
									 203 * m_pUIButtonMaterial->PixelToV() );
		m_pFemaleButton->SetOverUVs( 206 * m_pUIButtonMaterial->PixelToU(),
									 155 * m_pUIButtonMaterial->PixelToV(),
									 254 * m_pUIButtonMaterial->PixelToU(),
									 203 * m_pUIButtonMaterial->PixelToV() );

	}
*/

	CMenu::Render( pD3DDevice );


} // CAncestorDifficultyMenu::Render()

// render any UI elements
void CAncestorDifficultyMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::RenderUI( pD3DDevice );

} // CAncestorDifficultyMenu::RenderUI()

