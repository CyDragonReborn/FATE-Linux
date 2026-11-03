/*
	newpetmenu.cpp

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
#include "../../UTILITIES/vistafileutilities.h"

#include "newpetmenu.h"

#include "../../UTILITIES/StatsAndAchievements.h"


CNewPetMenu::CNewPetMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
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
														m_pUIButtonMaterial2( NULL ),
														m_pFontMaterial( NULL ),
														m_Name( "" ),
														m_pNameGenerator( NULL ),
														m_pNameText( NULL ),
														m_IsDog( kTrue ),
														m_BlinkTimer( 0 ),
														m_UpdatePlayer ( kFalse ),
														m_pClickSound ( NULL ),
														m_pCatButton ( NULL ),
														m_pDogButton ( NULL )
{
	m_pNameGenerator = new CNameGenerator( "PLAYERNAMES" );

} // CNewPetMenu::CNewPetMenu()

CNewPetMenu::~CNewPetMenu( void )
{
	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );
	m_pRefManager.RemoveMaterial( m_pUIButtonMaterial2 );

	DELETE_SAFELY( m_pNameGenerator );

} // CNewPetMenu::~CNewPetMenu()

void CNewPetMenu::Activate( void )
{
	CMenu::Activate();
	m_Name = "";
} // CNewPetMenu::Activate()

// perform creation of any elements that this menu needs
void CNewPetMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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

	m_pUIButtonMaterial2 = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons2.png",
													    "MENUS/buttons2a.png" );
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
									  KNewPetMenuAccept ) );
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

// quit button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  700,
									  640,
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
									  KNewPetMenuBack ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1013),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1013),
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
						LoadLocalizedString(1020),
						512,
						600,
						1.0f,
						kTrue,
						KJustifyCenter ) );

	m_pNameText = AddText( new CText( pD3DDevice,
									  m_pFontMaterial,
									  m_pFontMetric,
									  m_Name,
									  512,
									  640,
									  1.0f,
									  kFalse,
									  KJustifyCenter ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pUIButtonMaterial2,
									  NULL,
									  m_pClickSound,
									  472,
									  540,
									  49,
									  49,
									  155 * m_pUIButtonMaterial2->PixelToU(),
									  103 * m_pUIButtonMaterial2->PixelToV(),
									  203 * m_pUIButtonMaterial2->PixelToU(),
									  151 * m_pUIButtonMaterial2->PixelToV(),
									  155 * m_pUIButtonMaterial2->PixelToU(),
									  155 * m_pUIButtonMaterial2->PixelToV(),
									  203 * m_pUIButtonMaterial2->PixelToU(),
									  203 * m_pUIButtonMaterial2->PixelToV(),
									  KButtonCenter,
									  KNewPetMenuDog ) );
	pButton->SetToolTip( LoadLocalizedString(1021) );
	m_pDogButton = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pUIButtonMaterial2,
									  NULL,
									  m_pClickSound,
									  552,
									  540,
									  49,
									  49,
									  206 * m_pUIButtonMaterial2->PixelToU(),
									  103 * m_pUIButtonMaterial2->PixelToV(),
									  254 * m_pUIButtonMaterial2->PixelToU(),
									  151 * m_pUIButtonMaterial2->PixelToV(),
									  206 * m_pUIButtonMaterial2->PixelToU(),
									  155 * m_pUIButtonMaterial2->PixelToV(),
									  254 * m_pUIButtonMaterial2->PixelToU(),
									  203 * m_pUIButtonMaterial2->PixelToV(),
									  KButtonCenter,
									  KNewPetMenuCat ) );
	pButton->SetToolTip( LoadLocalizedString(1022) );
	m_pCatButton = pButton;


// random name
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  512,
									  720,
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
									  KNewPetMenuRandomName ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1016),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1016),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );

} // CNewPetMenu::Create()

void CNewPetMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );

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
			  ( Code >= 97 && Code <= 122 ) ||
			  Code == '_' ||
			  Code == '-' ||
			  Code == '\'' ||
			  Code == '`' ||
			  Code == 32 ) && 
			  ( m_Name.length() < 16 ) )
		{

			m_Name = m_Name + Code;
		}
		// if backspace, delete a character
		else if( KeyCode == 8 && m_Name.length() > 0 )
		{
			m_Name.resize( m_Name.length() - 1 );
		}
		// pressing enter will save it
		else if( KeyCode == 13 && m_Name.length() > 0 )
		{
			Begin();
		}
	}

} // CNewPetMenu::KeyEvent()


void CNewPetMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we go to character creation
	case KNewPetMenuBack :
		m_pGameStateManager.SetTargetMenu( KNewGameMenu );
		break;
	// if pressed, pick a random name
	case KNewPetMenuRandomName :
		srand( GetTickCount() );
		m_Name = m_pNameGenerator->GeneratePhonicName();
		break;
	// if pressed, we request an exit
	case KNewPetMenuAccept :
		Begin();
		break;
	case KNewPetMenuDog :
		if( !m_IsDog )
		{
			m_IsDog = kTrue;
			m_UpdatePlayer = kTrue;
		}
		break;
	case KNewPetMenuCat :
		if( m_IsDog )
		{
			m_IsDog = kFalse;
			m_UpdatePlayer = kTrue;
		}
		break;
	}

} // CNewPetMenu::ProcessInput()

void CNewPetMenu::Begin( void )
{
	if( m_Name.length() == 0 )
	{
		return;
	}
	m_pMenuManager.GameClient().SetNewPetIsDog( m_IsDog );
	m_pMenuManager.GameClient().SetNewCharacterPetName( m_Name );

	CStatsAndAchievements::GetInstance()->SetPlayer(NULL);

	m_pGameStateManager.SetTargetMenu( KIntroMenu );
} // CNewPetMenu::Begin()

void CNewPetMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{
	m_BlinkTimer += TimeElapsed;
	while( m_BlinkTimer > 1 )
	{
		m_BlinkTimer -= 1.0f;
	}

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );
	

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CNewPetMenu::Update()

void CNewPetMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( m_IsDog )
	{
		m_pDogButton->SetUVs( 155 * m_pUIButtonMaterial2->PixelToU(),
							   155 * m_pUIButtonMaterial2->PixelToV(),
							   203 * m_pUIButtonMaterial2->PixelToU(),
							   203 * m_pUIButtonMaterial2->PixelToV() );
		m_pDogButton->SetOverUVs( 155 * m_pUIButtonMaterial2->PixelToU(),
								   155 * m_pUIButtonMaterial2->PixelToV(),
								   203 * m_pUIButtonMaterial2->PixelToU(),
								   203 * m_pUIButtonMaterial2->PixelToV() );

		m_pCatButton->SetUVs( 206 * m_pUIButtonMaterial2->PixelToU(),
								 103 * m_pUIButtonMaterial2->PixelToV(),
								 254 * m_pUIButtonMaterial2->PixelToU(),
								 151 * m_pUIButtonMaterial2->PixelToV() );
		m_pCatButton->SetOverUVs( 206 * m_pUIButtonMaterial2->PixelToU(),
									 155 * m_pUIButtonMaterial2->PixelToV(),
									 254 * m_pUIButtonMaterial2->PixelToU(),
									 203 * m_pUIButtonMaterial2->PixelToV() );
	}
	else
	{
		m_pDogButton->SetUVs( 155 * m_pUIButtonMaterial2->PixelToU(),
							   103 * m_pUIButtonMaterial2->PixelToV(),
							   203 * m_pUIButtonMaterial2->PixelToU(),
							   151 * m_pUIButtonMaterial2->PixelToV() );
		m_pDogButton->SetOverUVs( 155 * m_pUIButtonMaterial2->PixelToU(),
								   155 * m_pUIButtonMaterial2->PixelToV(),
								   203 * m_pUIButtonMaterial2->PixelToU(),
								   203 * m_pUIButtonMaterial2->PixelToV() );

		m_pCatButton->SetUVs( 206 * m_pUIButtonMaterial2->PixelToU(),
									 155 * m_pUIButtonMaterial2->PixelToV(),
									 254 * m_pUIButtonMaterial2->PixelToU(),
									 203 * m_pUIButtonMaterial2->PixelToV() );
		m_pCatButton->SetOverUVs( 206 * m_pUIButtonMaterial2->PixelToU(),
									 155 * m_pUIButtonMaterial2->PixelToV(),
									 254 * m_pUIButtonMaterial2->PixelToU(),
									 203 * m_pUIButtonMaterial2->PixelToV() );

	}

	if( m_UpdatePlayer )
	{
		m_UpdatePlayer = kFalse;
		m_pMenuManager.CreatePet( pD3DDevice,
								  NULL,
								  m_IsDog );
	}

	CMenu::Render( pD3DDevice );


} // CNewPetMenu::Render()

// render any UI elements
void CNewPetMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	std::string BlinkCharacter( "|" );
	if( m_BlinkTimer < .5 )
	{
		BlinkCharacter = "  ";
	}

	m_pNameText->SetText( pD3DDevice,
						  m_Name + BlinkCharacter );

	CMenu::RenderUI( pD3DDevice );

} // CNewPetMenu::RenderUI()
