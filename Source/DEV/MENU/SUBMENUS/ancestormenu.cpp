/*
	ancestormenu.cpp

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
#include "../../UTILITIES/loadlocalizedstring.h"

#include "../../CHARACTER/gamesave.h"

#include "ancestormenu.h"

#include "../../UTILITIES/StatsAndAchievements.h"


CAncestorMenu::CAncestorMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
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
														m_pButtonMaterial2( NULL ),
														m_pUIButtonMaterial( NULL ),
														m_pFontMaterial( NULL ),
														m_Name( "" ),
														m_pNameGenerator( NULL ),
														m_pNameText( NULL ),
														m_BlinkTimer( 0 ),
														m_UpdatePlayer( kFalse ),
														m_Gender( KMale ),
														m_HeadIndex( 0 ),
														m_HairIndex( 0 ),
														m_MaleHairs ( 13 ),
														m_MaleHeads ( 8 ),
														m_FemaleHairs ( 19 ),
														m_FemaleHeads ( 8 ),
														m_pClickSound ( NULL )
{

	m_pNameGenerator = new CNameGenerator( "PLAYERNAMES" );
	m_pGameSave = new CGameSave();

} // CAncestorMenu::CAncestorMenu()

CAncestorMenu::~CAncestorMenu( void )
{
	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial2 );
	m_pRefManager.RemoveMaterial( m_pUIButtonMaterial );

	DELETE_SAFELY( m_pNameGenerator );

} // CAncestorMenu::~CAncestorMenu()

void CAncestorMenu::Activate( void )
{
	CMenu::Activate();
	m_Name = "";
	m_UpdatePlayer = kTrue;
} // CAncestorMenu::Activate()

// perform creation of any elements that this menu needs
void CAncestorMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
	m_pButtonMaterial2 = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons2.png",
													    "MENUS/buttons2a.png" );
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
									  KAncestorMenuBack ) );
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
									  KAncestorMenuAccept ) );
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

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1014),
						512,
						600,
						1.0f,
						kTrue,
						KJustifyCenter ) );

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pUIButtonMaterial,
									  NULL,
									  m_pClickSound,
									  472,
									  540,
									  49,
									  49,
									  155 * m_pUIButtonMaterial->PixelToU(),
									  103 * m_pUIButtonMaterial->PixelToV(),
									  203 * m_pUIButtonMaterial->PixelToU(),
									  151 * m_pUIButtonMaterial->PixelToV(),
									  155 * m_pUIButtonMaterial->PixelToU(),
									  155 * m_pUIButtonMaterial->PixelToV(),
									  203 * m_pUIButtonMaterial->PixelToU(),
									  203 * m_pUIButtonMaterial->PixelToV(),
									  KButtonCenter,
									  KAncestorMenuMale ) );
	pButton->SetToolTip( LoadLocalizedString(1015) );
	m_pMaleButton = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									  m_pUIButtonMaterial,
									  NULL,
									  m_pClickSound,
									  552,
									  540,
									  49,
									  49,
									  206 * m_pUIButtonMaterial->PixelToU(),
									  103 * m_pUIButtonMaterial->PixelToV(),
									  254 * m_pUIButtonMaterial->PixelToU(),
									  151 * m_pUIButtonMaterial->PixelToV(),
									  206 * m_pUIButtonMaterial->PixelToU(),
									  155 * m_pUIButtonMaterial->PixelToV(),
									  254 * m_pUIButtonMaterial->PixelToU(),
									  203 * m_pUIButtonMaterial->PixelToV(),
									  KButtonCenter,
									  KAncestorMenuFemale ) );
	pButton->SetToolTip( LoadLocalizedString(1015) );
	m_pFemaleButton = pButton;

	m_pNameText = AddText( new CText( pD3DDevice,
									  m_pFontMaterial,
									  m_pFontMetric,
									  m_Name,
									  512,
									  640,
									  1.0f,
									  kFalse,
									  KJustifyCenter ) );

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
									  KAncestorMenuRandomName ) );
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


// randomize features
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  700,
									  125,
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
									  KAncestorMenuRandomFeatures ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1017),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1017),
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
						LoadLocalizedString(1018),
						828,
						170,
						1.0f,
						kTrue,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1018),
						828,
						170,
						1.0f,
						kTrue,
						KJustifyCenter ) );

	pButton = AddButton( new CButton( pD3DDevice,
										m_pButtonMaterial2,
										NULL,
										m_pClickSound,
										776,
										200,
										52,
										52,
										1 * m_pButtonMaterial->PixelToU(),
										151 * m_pButtonMaterial->PixelToV(),
										53 * m_pButtonMaterial->PixelToU(),
										202 * m_pButtonMaterial->PixelToV(),
										1 * m_pButtonMaterial->PixelToU(),
										203 * m_pButtonMaterial->PixelToV(),
										53 * m_pButtonMaterial->PixelToU(),
										254 * m_pButtonMaterial->PixelToV(),
								   		KButtonTopLeft,
										KAncestorMenuPrevHair ) );


	pButton = AddButton( new CButton( pD3DDevice,
										m_pButtonMaterial2,
										NULL,
										m_pClickSound,
										828,
										200,
										52,
										52,
										55 * m_pButtonMaterial->PixelToU(),
										151 * m_pButtonMaterial->PixelToV(),
										106 * m_pButtonMaterial->PixelToU(),
										202 * m_pButtonMaterial->PixelToV(),
										55 * m_pButtonMaterial->PixelToU(),
										203 * m_pButtonMaterial->PixelToV(),
										106 * m_pButtonMaterial->PixelToU(),
										254 * m_pButtonMaterial->PixelToV(),
								   		KButtonTopLeft,
										KAncestorMenuNextHair ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1019),
						828,
						250,
						1.0f,
						kTrue,
						KJustifyCenter ) );

	pButton = AddButton( new CButton( pD3DDevice,
										m_pButtonMaterial2,
										NULL,
										m_pClickSound,
										776,
										280,
										52,
										52,
										1 * m_pButtonMaterial->PixelToU(),
										151 * m_pButtonMaterial->PixelToV(),
										53 * m_pButtonMaterial->PixelToU(),
										202 * m_pButtonMaterial->PixelToV(),
										1 * m_pButtonMaterial->PixelToU(),
										203 * m_pButtonMaterial->PixelToV(),
										53 * m_pButtonMaterial->PixelToU(),
										254 * m_pButtonMaterial->PixelToV(),
								   		KButtonTopLeft,
										KAncestorMenuPrevHead ) );


	pButton = AddButton( new CButton( pD3DDevice,
										m_pButtonMaterial2,
										NULL,
										m_pClickSound,
										828,
										280,
										52,
										52,
										55 * m_pButtonMaterial->PixelToU(),
										151 * m_pButtonMaterial->PixelToV(),
										106 * m_pButtonMaterial->PixelToU(),
										202 * m_pButtonMaterial->PixelToV(),
										55 * m_pButtonMaterial->PixelToU(),
										203 * m_pButtonMaterial->PixelToV(),
										106 * m_pButtonMaterial->PixelToU(),
										254 * m_pButtonMaterial->PixelToV(),
								   		KButtonTopLeft,
										KAncestorMenuNextHead ) );
} // CAncestorMenu::Create()

void CAncestorMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
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

} // CAncestorMenu::KeyEvent()


void CAncestorMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we go to character creation
	case KAncestorMenuBack :
		m_pGameStateManager.SetTargetMenu( KAncestorDifficultyMenu );
		break;
	// if pressed, pick a random name
	case KAncestorMenuRandomName :
		srand( GetTickCount() );
		m_Name = m_pNameGenerator->GeneratePhonicName();
		break;
	// if pressed, pick some random features
	case KAncestorMenuRandomFeatures :
		if( m_Gender == KMale )
		{
			m_HairIndex = RandomIntegerBetween( 0, m_MaleHairs - 1 );
			m_HeadIndex = RandomIntegerBetween( 0, m_MaleHeads - 1 );
		}
		else 
		{
			m_HairIndex = RandomIntegerBetween( 0, m_FemaleHairs - 1 );
			m_HeadIndex = RandomIntegerBetween( 0, m_FemaleHeads - 1 );
		}
		m_UpdatePlayer = kTrue;
		VerifySelection();
		break;
	// if pressed, we request an exit
	case KAncestorMenuAccept :
		Begin();
		break;
	case KAncestorMenuNextHead :
		m_HeadIndex++;
		m_UpdatePlayer = kTrue;
		VerifySelection();
		break;
	case KAncestorMenuPrevHead :
		m_HeadIndex--;
		m_UpdatePlayer = kTrue;
		VerifySelection();
		break;
	case KAncestorMenuNextHair :
		m_HairIndex++;
		m_UpdatePlayer = kTrue;
		VerifySelection();
		break;
	case KAncestorMenuPrevHair :
		m_HairIndex--;
		m_UpdatePlayer = kTrue;
		VerifySelection();
		break;
	case KAncestorMenuMale :
		if( m_Gender != KMale )
		{
			m_Gender = KMale;
			m_UpdatePlayer = kTrue;
			VerifySelection();
		}
		break;
	case KAncestorMenuFemale :
		if( m_Gender != KFemale )
		{
			m_Gender = KFemale;
			m_UpdatePlayer = kTrue;
			VerifySelection();
		}

		break;
	}

} // CAncestorMenu::ProcessInput()

void CAncestorMenu::Begin( void )
{
	if( m_Name.length() == 0 )
	{
		return;
	}
	m_pMenuManager.GameClient().SetNewCharacterLineage( m_pMenuManager.GameClient().RetirementLineage() + 1 );
	m_pMenuManager.GameClient().SetNewCharacterName( m_Name );
	m_pMenuManager.GameClient().SetNewCharacterGender( m_Gender );
	m_pMenuManager.GameClient().SetNewCharacterHeadIndex( m_HeadIndex );
	m_pMenuManager.GameClient().SetNewCharacterHairIndex( m_HairIndex );
	std::string NewPath = SelectEmptySave( CVistaFileUtilities::GetSharedDataPath() + "/SAVE/" + CGame::GetLocaleString() + "/" ); //Modified by TDC 2008-02-19. Craig Miles.
	m_pMenuManager.GameClient().SetSaveGamePath( NewPath );
	m_pMenuManager.GameClient().SetCurrentSlotIndex( m_pGameSave->NextAvailableSlot(CGame::GetLocaleString()));

	CStatsAndAchievements::GetInstance()->CreateDescendentCharacter();

	m_pGameStateManager.SetTargetMenu( KAncestorPetMenu );
} // CAncestorMenu::Begin()

void CAncestorMenu::VerifySelection( void )
{
	if( m_Gender == KMale )
	{
		if( m_HairIndex >= m_MaleHairs )
		{
			m_HairIndex = 0;
		}
		if( m_HeadIndex >= m_MaleHeads )
		{
			m_HeadIndex = 0;
		}
		if( m_HairIndex < 0 )
		{
			m_HairIndex = m_MaleHairs - 1;
		}
		if( m_HeadIndex < 0 )
		{
			m_HeadIndex = m_MaleHeads - 1;
		}
	}
	else
	{
		if( m_HairIndex >= m_FemaleHairs )
		{
			m_HairIndex = 0;
		}
		if( m_HeadIndex >= m_FemaleHeads )
		{
			m_HeadIndex = 0;
		}
		if( m_HairIndex < 0 )
		{
			m_HairIndex = m_FemaleHairs - 1;
		}
		if( m_HeadIndex < 0 )
		{
			m_HeadIndex = m_FemaleHeads - 1;
		}
	}
} // CAncestorMenu::VerifySelection()

void CAncestorMenu::Update( HWND hWnd,								// hwnd of the active window
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

} // CAncestorMenu::Update()

void CAncestorMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

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
	if( m_UpdatePlayer )
	{
		m_UpdatePlayer = kFalse;
		m_pMenuManager.CreatePlayer( pD3DDevice,
									 NULL,
									 NULL,
									 m_Gender,
									 m_HeadIndex,
									 m_HairIndex );
	}

	CMenu::Render( pD3DDevice );


} // CAncestorMenu::Render()

// render any UI elements
void CAncestorMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	std::string BlinkCharacter( "|" );
	if( m_BlinkTimer < .5 )
	{
		BlinkCharacter = "  ";
	}

	m_pNameText->SetText( pD3DDevice,
						  m_Name + BlinkCharacter );

	CMenu::RenderUI( pD3DDevice );

} // CAncestorMenu::RenderUI()

std::string CAncestorMenu::SelectEmptySave( std::string Path )						// path to the music folder to look in
{

	std::vector< int32 >	Indices;
	WIN32_FIND_DATA FindFileData;
	HANDLE Find;

	Find = FindFirstFile( ( Path + "/*.*" ).c_str(), &FindFileData );

	BOOL Done( kFalse );
       
	while ( Find != INVALID_HANDLE_VALUE && !Done )
    {
		std::string Name = FindFileData.cFileName;
		
		if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			if( Name.length() > 4 )
			{
				std::string Extension = StringUpper( Name.substr( Name.length() - 3, 3 ) );
				if( Extension == KSaveExtension )
				{
					int32 Index = atoi( Name.substr( 0, Name.length() - 4 ).c_str() );
					Indices.push_back( Index );

				}
			}
		}

		Done = !FindNextFile( Find, &FindFileData );
	}
	FindClose( Find );

	int32 FinalIndex( 0 );
	Done = kFalse ;
	while( !Done )
	{
		Done = kTrue;
		for( uint32 i = 0; i < Indices.size(); i++ )
		{
			if( Indices[i] == FinalIndex )
			{
				FinalIndex++;
				Done = kFalse;
			}
		}
	}
	return Path + "\\" + IntegerToString( FinalIndex ) + "." + KSaveExtension;
} // CAncestorMenu::SelectEmptySave()
