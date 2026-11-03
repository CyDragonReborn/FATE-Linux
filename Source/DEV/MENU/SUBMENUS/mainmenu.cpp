/*
	mainmenu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/


#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include <windows.h>

#include "../menu.h"
#include "../../UI/fontmetric.h"
#include "../../UTILITIES/spawn.h"
#include "../../UTILITIES/vistafileutilities.h"
#include "../../UTILITIES/loadlocalizedstring.h"
#include "../../GAME/game.h"

#include "mainmenu.h"

#include "../../GAMECLIENT/demorules.h" //marsh
//#include "WT/Application/RuntimeAttribute.h"

CMainMenu::CMainMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
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
													m_pContinueButton( NULL ),
													m_pFontMaterial( NULL ),
													m_pDPLogoMaterial( NULL ),
													m_pClickSound ( NULL ),
													m_pVersionText( NULL )

{
	m_pGameSave = new CGameSave();
} // CMainMenu::CMainMenu()

CMainMenu::~CMainMenu( void )
{
	RemoveButton ( m_pContinueButton );
	DELETE_SAFELY( m_pContinueButton );
	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pDPLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );

} // CMainMenu::~CMainMenu()

void CMainMenu::Activate( void )
{
	CMenu::Activate();

	RemoveButton( m_pContinueButton );
	WIN32_FIND_DATA FindFileData;
	HANDLE Find;

	int32 Files( 0 );

	//std::string Path = GetLocalPath() + "/SAVE/";
	std::string Path = CVistaFileUtilities::GetSharedDataPath() + "/SAVE/" + CGame::GetLocaleString() + "/"; //Modified by TDC 2008-02-19. Craig Miles.
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
					Files++;
				}
			}
		}

		Done = !FindNextFile( Find, &FindFileData );
	}
	
	int32 FilesCloud = m_pGameSave->HasGameSaves(CGame::GetLocaleString()); //for steam cloud save
	if(FilesCloud >= 0)
	{
		Files = FilesCloud;
	}

	if( Files > 0 )
	{
		AddButton( m_pContinueButton );
	}

} // CMainMenu::Activate()

// perform creation of any elements that this menu needs
void CMainMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMaterial2 = m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );

	std::string Path = AssembleAbsolutePath( GetLocalPath(), "dplogo_menu.jpg" );			
	// low quality version doesn't have the secondary music files
	if( FileExists( Path ) )
	{
		m_pDPLogoMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															"dplogo_menu.jpg",
															"dplogo_menu_a.jpg",
															kFalse,
															kFalse );
		// game logo
		AddSprite( new CSprite( pD3DDevice,
								m_pDPLogoMaterial,
								1014 - (float32)m_pDPLogoMaterial->Width(),
								758 - (float32)m_pDPLogoMaterial->Height(),
								(float32)m_pDPLogoMaterial->Width(),
								(float32)m_pDPLogoMaterial->Height(),
								0,
								0,
								1,
								1,
								KSpriteTopLeft ) );
	}

	m_pLogoMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													  "MENUS/logo.png",
													  "MENUS/logoa.png" );


	m_pButtonMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons.png",
													    "MENUS/buttonsa.png" );
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

	if( !IsGameUnlocked() )		//marsh - check for demo mode
	{
		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							LoadLocalizedString(1040),
							512,
							710,
							1.0f,
							kFalse,
							KJustifyCenter ) );
	}

	/*char Trademark = (char)159;
	std::string Registered = "Another Great Game By \bWildTangent Ÿ\b";

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						Registered,
						172,
						735,
						.6f,
						kTrue,
						KJustifyCenter ) );*/

	m_pVersionText = AddText( new CText( pD3DDevice,
										m_pFontMaterial,
										m_pFontMaterial2,
										m_pFontMetric,
										LoadLocalizedString1Variable(1041, FloatToString( m_pSettings.GetSettings( KSetVersion ) ) ),
										16,
										735,
										.75f,
										kFalse,
										KJustifyLeft ) );
	
// new game button
	CButton* pButton = AddButton( new CButton( pD3DDevice,
											   m_pButtonMaterial,
											   NULL,
											   m_pClickSound,
											   45,
											   360,
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
											   KMainMenuNewGame ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1042),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1042),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

// continue game button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  45,
									  300,
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
									  KMainMenuContinueGame ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1043),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1043),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

	m_pContinueButton = pButton;

// options button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  45,
									  420,
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
									  KMainMenuOptions ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1044),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1044),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

// credits button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  45,
									  480,
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
									  KMainMenuCredits ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1045),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1045),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

// help button disabled on steam build
	/*pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  45,
									  540,
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
									  KMainMenuHelp ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1046),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1046),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );*/

// quit button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  45,
									  540,
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
									  KMainMenuQuit ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(180),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(180),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );


// website button
	pButton = AddButton( new CButton( pD3DDevice,
									m_pButtonMaterial,
									NULL,
									m_pClickSound,
									45,
									600,
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
									KMainMenuWebsite ) );
	pButton->AddText( pD3DDevice,
					LoadLocalizedString(1047),
					m_pFontMaterial,
					m_pFontMetric,
					128,
					10,
					1.0f,
					kTrue,
					KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1047),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

	Activate();
} // CMainMenu::Create()

void CMainMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
						  WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );
} // CMainMenu::KeyEvent()


void CMainMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we go to character creation
	case KMainMenuNewGame :
		m_pGameStateManager.SetTargetMenu( KDifficultyMenu );
		break;
	// if pressed, we continue a previous game
	case KMainMenuContinueGame :
		m_pGameStateManager.SetTargetMenu( KContinueGameMenu );
		break;
	// if pressed, we go to options menu
	case KMainMenuOptions :
		m_pGameStateManager.SetTargetMenu( KOptionsMenu );
		break;
	// if pressed, we go to credits
	case KMainMenuCredits :
		m_pGameStateManager.SetTargetMenu( KCreditsMenu );
		break;
	// if pressed, we launch help pdf
	case KMainMenuHelp :
		{
		ShowWindow( hWnd, SW_MINIMIZE );

		m_KeyHandler.Flush();
		m_MouseHandler.Flush();

		std::string helpStr = GetLocalPath() + "/HELP/" + CGame::GetLocaleString() + "/help.exe"; // Modified by TDC. Craig Miles 2008-02-19.
		SpawnApp( (char *)(helpStr.c_str()), "", KSpawnAsync ); 
		}
		break;
	// if pressed, we request an exit
	case KMainMenuQuit :
		m_pGameStateManager.SetGameState( KGameStateQuit );
		break;

	// if pressed, we request an exit
	case KMainMenuWebsite :
		ShowWindow( hWnd, SW_MINIMIZE );

		m_KeyHandler.Flush();
		m_MouseHandler.Flush();
		
		SpawnApp( "iexplore.exe", "http://www.playfate.com", KSpawnAsync );
		break;

	}

} // CMainMenu::ProcessInput()

void CMainMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{
	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );


	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CMainMenu::Update()

void CMainMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::Render( pD3DDevice );


} // CMainMenu::Render()

// render any UI elements
void CMainMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	m_pVersionText->SetText( pD3DDevice,	
							 "Version " + FloatToString( m_pSettings.GetSettings( KSetVersion ) ) + "b" );
	CMenu::RenderUI( pD3DDevice );

} // CMainMenu::RenderUI()
