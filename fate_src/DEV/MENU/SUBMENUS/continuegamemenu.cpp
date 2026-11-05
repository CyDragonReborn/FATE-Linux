/*
	continuegamemenu.cpp

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

#include "../../GAMECLIENT/tips.h"

#include "continuegamemenu.h"
#include "../../character/character.h"
#include "../../character/player.h"

#include "../../GAMECLIENT/confirmmenu.h"
#include "../../UTILITIES/vistafileutilities.h"
#include "../../UTILITIES/loadlocalizedstring.h"

CContinueGameMenu::CContinueGameMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
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
														m_pFontMaterial( NULL ),
														m_pConfirmMenu( NULL ),
														m_pContinueButton( NULL ),
														m_SelectedProfile( 0 ),
														m_ScrollWindowIndex( 0 ),
														m_UpdatePlayer( kFalse ),
														m_OpenDeletion( kFalse ),
														m_pClickSound ( NULL )
{
	m_pGameSave = new CGameSave();
} // CContinueGameMenu::CContinueGameMenu()

CContinueGameMenu::~CContinueGameMenu( void )
{
	RemoveButton( m_pContinueButton );
	DELETE_SAFELY( m_pContinueButton );
	DELETE_SAFELY( m_pConfirmMenu );

	ClearScrollWindow();
	DeleteProfiles();

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );

} // CContinueGameMenu::~CContinueGameMenu()

void CContinueGameMenu::Activate( void )
{
	CMenu::Activate();
	m_UpdatePlayer = kTrue;
	ClearScrollWindow();

	ReloadProfiles();

	UpdateElements();
} // CContinueGameMenu::Activate()

void CContinueGameMenu::ReloadProfiles( void )
{
	DeleteProfiles();
	// find all the .dat files in the profile folder
	m_ProfileFiles.clear();
	m_ProfileIndices.clear();

	GetProfilesFromFolder( CVistaFileUtilities::GetSharedDataPath() + "/SAVE/" + CGame::GetLocaleString() + "/",
						   m_ProfileFiles,
						   m_ProfileIndices ); //Modified by TDC 2008-02-19. Craig Miles.

	//try to sync cloud profiles
	m_pGameSave->SyncProfiles(CVistaFileUtilities::GetSharedDataPath() 
		+ "/SAVE/" + CGame::GetLocaleString() + "/", CGame::GetLocaleString(), m_ProfileFiles);
	
	DeleteProfiles();
	m_ProfileFiles.clear();
	m_ProfileIndices.clear();
	//call again after cloud save sync
	GetProfilesFromFolder( CVistaFileUtilities::GetSharedDataPath() + "/SAVE/" + CGame::GetLocaleString() + "/",
						   m_ProfileFiles,
						   m_ProfileIndices ); //Modified by TDC 2008-02-19. Craig Miles.
	for( uint32 i = 0; i < m_ProfileFiles.size(); i++ )
	{
		FILE *pFile;
		pFile = fopen ( m_ProfileFiles[i].c_str(), "rb" );

		if( pFile == NULL )
		{
			return;
		}

		bool MapVisible;
		EMiniMapDolly MapDolly;
		fread( &MapVisible, sizeof( bool ), 1, pFile );
		fread( &MapDolly, sizeof( EMiniMapDolly ), 1, pFile );

		bool Running;
		bool ShowingItems;
		fread( &Running, sizeof( bool ), 1, pFile );
		fread( &ShowingItems, sizeof( bool ), 1, pFile );

		for( uint32 j = 0; j < KMaxHotkeys; j++ )
		{
			std::string Hotkey = "";
			uint16 Size( 0 );
			fread( &Size, sizeof( uint16 ), 1, pFile );
			if( Size > 0 )
			{
				Hotkey = ReadString( pFile, Size );
			}
		}

		for( int32 j = 0; j < KContextTips; j++ )
		{
			bool TipSeen;
			fread( &TipSeen, sizeof( bool ), 1, pFile );
		}

		// save the player's description
		CCharacterSaveInstance* pInstance = new CCharacterSaveInstance();
		CCharacterSaveInstance* pPetInstance = NULL;
		pInstance->Load( pFile );
		m_pProfileDescriptions.push_back( pInstance );

		uint32 Pets( 0 );

		fread( &Pets, sizeof( uint32 ), 1, pFile );

		// save the pet's description
		if( Pets > 0 )
		{
			pPetInstance = new CCharacterSaveInstance();
			pPetInstance->Load( pFile );
		}
		m_pPetProfileDescriptions.push_back( pPetInstance );
		
		fclose( pFile );
	}
	if( m_SelectedProfile >= (int32)m_ProfileFiles.size() &&
		m_ProfileFiles.size() > 0 )
	{
		m_SelectedProfile = (int32)m_ProfileFiles.size() - 1;
	}

} // CContinueGameMenu::ReloadProfiles()

void CContinueGameMenu::ClearScrollWindow( void )
{
	for( uint32 i = 0; i < m_pScrollWindowElements.size(); i++ )
	{
		RemoveButton( m_pScrollWindowElements[i] );
		DELETE_SAFELY( m_pScrollWindowElements[i] );
	}
	for( int32 i = 0; i < m_pScrollWindowText.size(); i++ )
	{
		RemoveText( m_pScrollWindowText[i] );
		DELETE_SAFELY( m_pScrollWindowText[i] );
	}
} // CContinueGameMenu::ClearScrollWindow()

void CContinueGameMenu::DeleteProfiles( void )
{
	for( uint32 i = 0; i < m_pProfileDescriptions.size(); i++ )
	{
		DELETE_SAFELY( m_pProfileDescriptions[i] );
	}
	m_pProfileDescriptions.clear();
	for( int32 i = 0; i < m_pPetProfileDescriptions.size(); i++ )
	{
		DELETE_SAFELY( m_pPetProfileDescriptions[i] );
	}
	m_pPetProfileDescriptions.clear();
} // CContinueGameMenu::DeleteProfiles()

void CContinueGameMenu::UpdateScrollWindow( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	ClearScrollWindow();
	int32 Profiles = m_ProfileFiles.size();
	float32 YPosition( 330 );
	int32 LastIndex = m_ScrollWindowIndex + KProfileSelectionSlots;
	if( LastIndex > Profiles )
	{
		LastIndex = Profiles;
	}
	for( int32 i = m_ScrollWindowIndex; i < LastIndex; i++ )
	{
		CButton* pButton;
		// selected profile is pre-highlighted
		if( i == m_SelectedProfile )
		{
			pButton = AddButton( new CButton( pD3DDevice,
											m_pButtonMaterial,
										    NULL,
										    m_pClickSound,
											350,
											YPosition,
											300,
											80,
											31 * m_pButtonMaterial->PixelToU(),
											102 * m_pButtonMaterial->PixelToV(),
											49 * m_pButtonMaterial->PixelToU(),
											121 * m_pButtonMaterial->PixelToV(),
											31 * m_pButtonMaterial->PixelToU(),
											102 * m_pButtonMaterial->PixelToV(),
											49 * m_pButtonMaterial->PixelToU(),
											121 * m_pButtonMaterial->PixelToV(),
											KButtonTopLeft,
											100 + i ) );
		}
		else
		{
			pButton = AddButton( new CButton( pD3DDevice,
											m_pButtonMaterial,
										    NULL,
										    m_pClickSound,
											350,
											YPosition,
											300,
											80,
											6 * m_pButtonMaterial->PixelToU(),
											102 * m_pButtonMaterial->PixelToV(),
											23 * m_pButtonMaterial->PixelToU(),
											121 * m_pButtonMaterial->PixelToV(),
											31 * m_pButtonMaterial->PixelToU(),
											102 * m_pButtonMaterial->PixelToV(),
											49 * m_pButtonMaterial->PixelToU(),
											121 * m_pButtonMaterial->PixelToV(),
											KButtonTopLeft,
											100 + i ) );
		}
		std::string Text;
		if( m_pProfileDescriptions[i]->m_Lineage > 0 )
		{
			Text = LoadLocalizedString3Variables(989, m_pProfileDescriptions[i]->m_Name, KFameName[m_pProfileDescriptions[i]->m_FameRank], RomanNumeral( m_pProfileDescriptions[i]->m_Lineage ) );
		}
		else
		{
			Text = LoadLocalizedString2Variables(990, m_pProfileDescriptions[i]->m_Name, KFameName[m_pProfileDescriptions[i]->m_FameRank] ) + "\n";
		}

		if( m_pProfileDescriptions[i]->m_Lineage == -1 )
		{
			Text = Text + LoadLocalizedString(1023);
		}
		else
		{
			Text = Text + LoadLocalizedString1Variable(821, IntegerToString( m_pProfileDescriptions[i]->m_Level ) ) + "\n";
		}
		float32 Seconds = m_pProfileDescriptions[i]->m_ExistenceTime;
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
		Text = Text + LoadLocalizedString3Variables(914, IntegerToString( Hours ), IntegerToString( Minutes ),
					 IntegerToString( (int32)Seconds ) );

		Text = Text + LoadLocalizedString1Variable(1024, KDifficultyName[ m_pProfileDescriptions[i]->m_Difficulty ] );

		CText* pText = AddText( new CText( pD3DDevice,
										   m_pFontMaterial,
										   m_pFontMetric,
										   Text,
										   355,
										   YPosition,
										   .75f,
										   kFalse,
										   KJustifyLeft ) );
		m_pScrollWindowText.push_back( pText );

		float32 Width = pText->MaxBounds().x -
						pText->MinBounds().x;
		if( Width > 290 )
		{
			float32 ScaleMultiplier = 290.0f / Width;
			pText->SetScale( pD3DDevice, .75f * ScaleMultiplier );

		}

		// keep it in our list of elements so that we can remove it later.
		m_pScrollWindowElements.push_back( pButton );
		YPosition += 90;
	}

	CButton* pButton;
	if( m_ScrollWindowIndex > 0 )
	{
		pButton = AddButton( new CButton( pD3DDevice,
										m_pButtonMaterial,
										NULL,
										m_pClickSound,
										478,
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
								   		KButtonCenter,
										KContinueMenuScrollUp ) );
		// keep it in our list of elements so that we can remove it later.
		m_pScrollWindowElements.push_back( pButton );
	}
	if( Profiles - m_ScrollWindowIndex > KProfileSelectionSlots )
	{
		pButton = AddButton( new CButton( pD3DDevice,
										m_pButtonMaterial,
										NULL,
										m_pClickSound,
										478,
										720,
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
								   		KButtonCenter,
										KContinueMenuScrollDown ) );
		// keep it in our list of elements so that we can remove it later.
		m_pScrollWindowElements.push_back( pButton );
	}

} // CContinueGameMenu::UpdateScrollWindow()


// perform creation of any elements that this menu needs
void CContinueGameMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
	CButton* pButton = AddButton( new CButton( pD3DDevice,
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
											   KContinueMenuSelect ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1025),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1025),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

	m_pContinueButton = pButton;

// continue game button
	pButton = AddButton( new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  45,
									  460,
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
									  KContinueMenuDelete ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1026),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1026),
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
									  45,
									  520,
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
									  KContinueMenuBack ) );
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

	m_pConfirmMenu = new CConfirmMenu( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pGameStateManager );

} // CContinueGameMenu::Create()

void CContinueGameMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
						  WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );
	m_pConfirmMenu->KeyEvent( WMMessage,
							  KeyCode );
} // CContinueGameMenu::KeyEvent()


void CContinueGameMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode( -1 );

	if( !m_pConfirmMenu->Open() )
	{
		ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );
	}

	if( ReturnCode == -1 )
	{
		ReturnCode = m_pConfirmMenu->UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
														   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
	}

	switch( ReturnCode )
	{
	case KContinueMenuScrollDown :
		m_ScrollWindowIndex++;
		if( m_ScrollWindowIndex > (int32)m_ProfileFiles.size() - (int32)KProfileSelectionSlots )
		{
			m_ScrollWindowIndex = (int32)m_ProfileFiles.size() - (int32)KProfileSelectionSlots;
		}
		UpdateElements();
		break;
	case KContinueMenuScrollUp :
		m_ScrollWindowIndex--;
		if( m_ScrollWindowIndex < 0 )
		{
			m_ScrollWindowIndex = 0;
		}
		UpdateElements();
		break;
	// if pressed, back to main menu
	case KContinueMenuBack :
		m_pGameStateManager.SetTargetMenu( KMainMenu );
		break;
	// if pressed, continue the selected game
	case KContinueMenuSelect :
		if( m_SelectedProfile < (int32)m_ProfileFiles.size() &&
			m_pProfileDescriptions[m_SelectedProfile]->m_Lineage != -1 )
		{
			m_pMenuManager.GameClient().SetSaveGamePath( m_ProfileFiles[m_SelectedProfile] );
			m_pMenuManager.GameClient().SetCurrentSlotIndex( m_ProfileIndices[m_SelectedProfile] );
			m_pGameStateManager.SetGameState( KGameStateLoadGame );
		}
		break;
	// if pressed, we go to options menu
	case KContinueMenuDelete :
		if( m_SelectedProfile < (int32)m_ProfileFiles.size() &&
			m_ProfileFiles.size() != 0 )
		{
			m_OpenDeletion = kTrue;
		}
		break;
	case KContinueMenuDeleteConfirm :
		m_pConfirmMenu->SetOpen( kFalse );
		m_UpdatePlayer = kTrue;
		m_pGameSave->DeleteProfile(m_ProfileIndices[m_SelectedProfile], CGame::GetLocaleString());
		DeleteFile( m_ProfileFiles[m_SelectedProfile].c_str() );
		Activate();
		// if nothing is left, head back to the main menu
		if( m_ProfileFiles.size() == 0 )
		{
			m_pGameStateManager.SetTargetMenu( KMainMenu );
		}

		break;
	case KContinueMenuDeleteCancel :
		m_pConfirmMenu->SetOpen( kFalse );
		break;
	}

	if( ReturnCode >= 100 )
	{
		int32 Index = ReturnCode - 100;
		m_SelectedProfile = Index;
		UpdateElements();
		m_UpdatePlayer = kTrue;

	}


} // CContinueGameMenu::ProcessInput()

void CContinueGameMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{
	ProcessInput( hWnd );

	m_pConfirmMenu->Update( hWnd,
						    TimeElapsed );

	if( !m_pConfirmMenu->Open() )
	{
		UpdateButtons( TimeElapsed );
	}
	

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CContinueGameMenu::Update()

void CContinueGameMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::Render( pD3DDevice );


} // CContinueGameMenu::Render()

// render any UI elements
void CContinueGameMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( m_OpenDeletion )
	{
		m_OpenDeletion = kFalse;
		m_pConfirmMenu->Open( pD3DDevice,
							  LoadLocalizedString1Variable(1027, m_pProfileDescriptions[m_SelectedProfile]->m_Name ),
							  LoadLocalizedString(1028),
							  KContinueMenuDeleteConfirm,
							  LoadLocalizedString(1029),
							  KContinueMenuDeleteCancel,
							  1 );
	}
	// update any text that needs to be updated
	if( m_UpdateElements )
	{
		m_UpdateElements = kFalse;
		UpdateScrollWindow( pD3DDevice );
	}

	if( m_UpdatePlayer )
	{
		if( m_SelectedProfile < (int32)m_ProfileFiles.size() &&
			m_pProfileDescriptions[m_SelectedProfile]->m_Lineage == -1 )
		{
			RemoveButton( m_pContinueButton );
		}
		else
		{
			RemoveButton( m_pContinueButton );
			AddButton( m_pContinueButton );
		}

		UpdatePlayer( pD3DDevice );
	}

	CMenu::RenderUI( pD3DDevice );

	m_pConfirmMenu->Render( pD3DDevice );

} // CContinueGameMenu::RenderUI()

void CContinueGameMenu::UpdatePlayer( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
		m_UpdatePlayer = kFalse;
		if( m_SelectedProfile < (int32)m_ProfileFiles.size() )
		{
			m_pMenuManager.CreatePlayer( pD3DDevice,
										m_pProfileDescriptions[m_SelectedProfile],
										m_pPetProfileDescriptions[m_SelectedProfile] );
		}
		else
		{
			m_pMenuManager.CreatePlayer( pD3DDevice,
										 NULL,
										 NULL );
		}

} // CContinueGameMenu::UpdatePlayer()

void CContinueGameMenu::GetProfilesFromFolder( std::string Path,						// path to the music folder to look in
											   std::vector< std::string >& Paths,	// list of paths to add to
											   std::vector< int32 >& Indices )	// list of indices to add to
{

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
					Paths.push_back( Path + Name );
					int32 Index = atoi( Name.substr( 0, Name.length() - 4 ).c_str() );
					Indices.push_back( Index );

				}
			}
		}

		Done = !FindNextFile( Find, &FindFileData );
	}
	FindClose( Find );
} // CContinueGameMenu::GetProfilesFromFolder()
