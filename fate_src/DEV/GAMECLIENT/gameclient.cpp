/*
	gameclient.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <assert.h>
#include <time.h>
#include <D3D8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/capabilities.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/datafile.h"
#include "../UTILITIES/vistafileutilities.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../UI/sprite.h"

#include "../PARTICLES/particlemanager.h"

#include "../GLOWS/glowmanager.h"

#include "../POSITIONALAUDIO/positionalaudiomanager.h"

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"
#include "../MEDIA/soundmanager.h"

#include "../LEVEL/level.h"
#include "../LEVEL/levelhistory.h"

#include "../EFFECTS/sfx.h"

#include "tips.h"
#include "quest.h"
#include "questdescriptions.h"

#include "../MENU/loadingmenu.h"

#include "../OBJECTS/datalight.h"

#include "../GAME/gamestatemanager.h"

#include "../GAMECLIENT/gameclient.h"
#include "../GAME/game.h"

#include "../GAMEOBJECTS/blurlayer.h"
#include "../GAMEOBJECTS/camera.h"
#include "../GAMEOBJECTS/lightmanager.h"

#include "../ITEM/itemtemplate.h"
#include "../ITEM/spelltemplate.h"
#include "../ITEM/item.h"

#include "../CHARACTER/charactertemplate.h"
#include "../CHARACTER/character.h"
#include "../CHARACTER/player.h"
#include "../CHARACTER/monster.h"

#include "../NAMES/namegenerator.h"

#include "templatemanifest.h"

#include "gameui.h"
#include "gameclient.h"

CGameClient::CGameClient( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  CRefManager& pRefManager,					// pointer to the reference manager, for tracking media
						  CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
						  CGameStateManager& pGameStateManager,		// game manager
						  CLoadingMenu& pLoadingMenu,
						  CSoundManager& pSoundManager ) :
															m_pRefManager( pRefManager ),
															m_pSettings( pSettings ),
															m_pSoundManager( pSoundManager ),
															m_pGameStateManager( pGameStateManager ),
															m_pLoadingMenu( pLoadingMenu ),
															m_pLightManager( NULL ),
															m_pBlurLayer( NULL ),
															m_pGameUI( NULL ),
															m_pCamera( NULL ),
															m_pActiveLevel( NULL ),
															m_pNameGenerator( NULL ),
															m_pPlayer( NULL ),
															m_pTips( NULL ),
															m_pMasterQuest( NULL ),
															m_pQuestDescriptions( NULL ),
															m_pMouseOverCharacter( NULL ),
															m_pMouseOverItem( NULL ),
															m_pRenderedShadow( NULL ),
															m_pRenderedLight( NULL ),
															m_pParticleManager( NULL ),
															m_pGlowManager( NULL ),
															m_pPositionalAudioManager( NULL ),
															m_pCharacterManifest( NULL ),
															m_pTemplateManifest( NULL ),
															m_pItemManifest( NULL ),
															m_pSpellManifest( NULL ),
															m_pHeirloomItem( NULL ),
															m_pBlack( NULL ),
															m_pWhite( NULL ),
															m_pErrorSound( NULL ),
															m_pGoldSound( NULL ),
															m_pRewardSound( NULL ),
															m_pQuestSound( NULL ),
															m_pEnchantSound( NULL ),
															m_pFailSound( NULL ),
															m_NewPetIsDog( kTrue ),
															m_TimeToRePath( 0 ),
															m_BlurTime( 0 ),
															m_HeldWhenInactive( kFalse ),
															m_ClickBroken( kTrue ),
															m_Dolly( KDefaultDolly ),
															m_DungeonSeed( 11050 ),
															m_ItemSeed( 11050 ),
															m_DebugText(""),
															m_NewCharacterName(""),
															m_NewCharacterPetName(""),
															m_NewCharacterGender( KMale ),
															m_NewCharacterHeadIndex(0),
															m_NewCharacterHairIndex(0),
															m_SaveGamePath(""),
															m_EventTime(0.0f),
															m_RenderTime(0.0f),
															m_Difficulty(KDifficultyAdventurer),
															m_RetirementLineage(0),
															m_NewCharacterLineage(0),
															m_RetirementText("")

{
	ZeroMemory ( &m_KeyHandler, sizeof ( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler, sizeof ( m_MouseHandler ));
	ZeroMemory ( &m_ShadowFrustum, sizeof ( m_ShadowFrustum ));
	ZeroMemory ( &m_ShadowViewMatrix, sizeof ( m_ShadowViewMatrix ));

	//DetermineSystemLocale();

	if( m_pSettings.GetSettings( KSetDungeonSeed ) == 0 )
	{
		m_DungeonSeed = time( NULL );
		m_pSettings.SetSettings( KSetDungeonSeed, m_DungeonSeed );
	}
	else
	{
		m_DungeonSeed = m_pSettings.GetSettings( KSetDungeonSeed );
	}

	m_pBlurLayer = new CBlurLayer( pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   "",
								   "",
								   256,
								   .75f );

	m_pGameSave = new CGameSave();

} // CGameClient::CGameClient()

CGameClient::~CGameClient( void )
{
	DELETE_SAFELY( m_pMasterQuest );
	DELETE_SAFELY( m_pTips );
	DELETE_SAFELY( m_pQuestDescriptions );
	DELETE_SAFELY( m_pTemplateManifest );
	DELETE_SAFELY( m_pCharacterManifest );
	DELETE_SAFELY( m_pItemManifest );
	DELETE_SAFELY( m_pSpellManifest );

	m_pRefManager.RemoveMaterial( m_pBlack );
	m_pRefManager.RemoveMaterial( m_pWhite );
	m_pRefManager.RemoveMaterial( m_pRenderedShadow );
	m_pRefManager.RemoveMaterial( m_pRenderedLight );

	m_pRefManager.RemoveSample( m_pErrorSound );
	m_pRefManager.RemoveSample( m_pGoldSound );
	m_pRefManager.RemoveSample( m_pRewardSound );
	m_pRefManager.RemoveSample( m_pQuestSound );
	m_pRefManager.RemoveSample( m_pEnchantSound );
	m_pRefManager.RemoveSample( m_pFailSound );

	DELETE_SAFELY( m_pBlurLayer );

	DELETE_SAFELY( m_pGameUI );

	DELETE_SAFELY( m_pCamera );

	DELETE_SAFELY( m_pNameGenerator );

	DELETE_SAFELY( m_pActiveLevel );

	DELETE_SAFELY( m_pPositionalAudioManager );

	DELETE_SAFELY( m_pParticleManager );

	DELETE_SAFELY( m_pGlowManager );

	DELETE_SAFELY( m_pLightManager );
	OutputDebugString( "Done Deleting Client\n" );

} // CGameClient::~CGameClient()

void CGameClient::FinalizeCreation( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pParticleManager = new CParticleManager( pD3DDevice,
											   m_pRefManager,
											   m_pSettings,
											   GetLocalPath() + "PARTICLES\\particles.dat" );

	m_pGlowManager = new CGlowManager( pD3DDevice,
								 	   m_pRefManager,
									   m_pSettings,
									   GetLocalPath() + "PARTICLES\\glows.dat" );

	m_pPositionalAudioManager = new CPositionalAudioManager( m_pRefManager,
															 m_pSettings );

	m_pNameGenerator = new CNameGenerator( "NAMES" );

	m_pQuestDescriptions = new CQuestDescriptions( "QUESTS\\" + CGame::GetLocaleString() + "\\quests.dat" );

	m_pTemplateManifest = new CTemplateManifest( GetLocalPath() + "TEMPLATES\\manifest.dat" );

	m_pCharacterManifest = new CCharacterTemplate( *m_pParticleManager,
												   GetLocalPath() + "MONSTERS\\" + CGame::GetLocaleString() + "\\monsters.dat" );

	m_pItemManifest = new CItemTemplate( GetLocalPath() + "ITEMS\\" + CGame::GetLocaleString() + "\\items.dat" );

	m_pSpellManifest = new CSpellTemplate( pD3DDevice, 
										   m_pRefManager, 
										   m_pSettings,
										   *m_pParticleManager,
										   *m_pPositionalAudioManager,
										   GetLocalPath() + "SPELLS\\" + CGame::GetLocaleString() + "\\spells.dat" );

	m_pLightManager = new CLightManager( m_pSettings );
	m_pLightManager->SetSceneAmbient( .25f, .25f, .25f );
	m_pLightManager->SetSceneSpecular( .5f, .5f, .5f );


	m_pLightManager->AddLight( new CDataLight( "SUN",
											   KLightDirectional,
											   0,
											   D3DXVECTOR3( 0, 0, 0 ),
											   D3DXVECTOR3( -.2f, -.5f, .4f ),
											   D3DXVECTOR3( 1.5f, 1.5f, 1.5f ) ) );


	// create a camera
	m_pCamera = new CCamera( m_pSettings );

	m_pGameUI = new CGameUI( pD3DDevice,
							 m_pRefManager,
							 m_pSettings,
							 *m_pParticleManager,
							 *m_pGlowManager,
							 m_pGameStateManager );

	m_pBlack = m_pRefManager.AddMaterial( pD3DDevice, "IMAGES//black.png" );
	m_pWhite = m_pRefManager.AddMaterial( pD3DDevice, "IMAGES//white.png" );

	// we're adding an EMPTY material with no texture.
	m_pRenderedShadow = m_pRefManager.AddMaterial( pD3DDevice, 
												   "",
												   "",
												   "",
												   "",
												   "",
												   kTrue );
	m_pRenderedShadow->EnableDynamicTexture( pD3DDevice, 
											 m_pSettings.GetSettings( KSetShadowResolution ) );

	m_pRenderedLight = m_pRefManager.AddMaterial( pD3DDevice, 
												   "",
												   "",
												   "",
												   "",
												   "",
												   kTrue );
	m_pRenderedLight->EnableDynamicTexture( pD3DDevice, 
										   m_pSettings.GetSettings( KSetShadowResolution ) / 2 );


	m_pTips = new CTips( "TIPS\\" + CGame::GetLocaleString() + "\\tips.dat" );

	m_pGoldSound = m_pRefManager.AddSample( "SOUNDS/ITEMS/gold.wav", kFalse );
	m_pErrorSound = m_pRefManager.AddSample( "SOUNDS/UI/error.wav", kFalse );
	m_pRewardSound = m_pRefManager.AddSample( "SOUNDS/UI/reward.wav", kFalse );
	m_pQuestSound = m_pRefManager.AddSample( "SOUNDS/UI/quest.wav", kFalse );
	m_pEnchantSound = m_pRefManager.AddSample( "SOUNDS/ITEMS/anvil.wav", kFalse );
	m_pFailSound = m_pRefManager.AddSample( "SOUNDS/ITEMS/anvilfail.wav", kFalse );

} // CGameClient::FinalizeCreation()

void CGameClient::UpdateShadowResolution( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if( m_pRenderedShadow == NULL )
	{
		return;
	}
	if( m_pRenderedShadow->DynamicTextureWidth() !=
		m_pSettings.GetSettings( KSetShadowResolution ) )
	{
		m_pRefManager.RemoveMaterial( m_pRenderedShadow );
		m_pRefManager.RemoveUnused();

		// we're adding an EMPTY material with no texture.
		m_pRenderedShadow = m_pRefManager.AddMaterial( pD3DDevice, 
													"",
													"",
													"",
													"",
													"",
													kTrue );
		m_pRenderedShadow->EnableDynamicTexture( pD3DDevice, 
												m_pSettings.GetSettings( KSetShadowResolution ) );
	}

	if( m_pRenderedLight == NULL )
	{
		return;
	}
	if( m_pRenderedLight->DynamicTextureWidth() !=
		m_pSettings.GetSettings( KSetShadowResolution ) )
	{
		m_pRefManager.RemoveMaterial( m_pRenderedLight );
		m_pRefManager.RemoveUnused();

		// we're adding an EMPTY material with no texture.
		m_pRenderedLight = m_pRefManager.AddMaterial( pD3DDevice, 
													"",
													"",
													"",
													"",
													"",
													kTrue );
		m_pRenderedLight->EnableDynamicTexture( pD3DDevice, 
												m_pSettings.GetSettings( KSetShadowResolution ) / 2 );
	}

} // CGameClient::UpdateShadowResolution( void )

void CGameClient::SetNewCharacterQuest( CQuest& pQuest )	// new quest
{
	DELETE_SAFELY( m_pMasterQuest );	
	m_pMasterQuest = new CQuest( pQuest );	
} // CGameClient::SetNewCharacterQuest()

void CGameClient::FlushAll( void )
{
	m_pGameUI->FlushAll();

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CGameUI::FlushAll()

void CGameClient::SaveCharacter( void )
{
	if( m_pPlayer == NULL )
	{
		return;
	}
	if( !m_pPlayer->Alive() )
	{
		return;
	}

	CVistaFileUtilities::CreateTempFile(m_SaveGamePath);
	std::string SavePath = m_SaveGamePath;
	FILE *pFile;
	pFile = fopen ( SavePath.c_str(), "wb" );

	if( pFile == NULL )
	{
		return;
	}
	m_pPlayer->RememberDiscovery( *m_pActiveLevel );

	bool MapVisible = m_pGameUI->MapVisible();
	EMiniMapDolly MapDolly = m_pGameUI->MapDolly();
	fwrite( &MapVisible, sizeof( bool ), 1, pFile );
	fwrite( &MapDolly, sizeof( EMiniMapDolly ), 1, pFile );

	bool Running( m_pGameUI->AlwaysRunning() );
	bool ShowingItems( m_pGameUI->AlwaysShowingItems() );
	fwrite( &Running, sizeof( bool ), 1, pFile );
	fwrite( &ShowingItems, sizeof( bool ), 1, pFile );

	for( uint32 j = 0; j < KMaxHotkeys; j++ )
	{
		std::string Hotkey = m_pGameUI->HotkeyAssignment( j );
		uint16 Size = Hotkey.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( Hotkey.c_str(), sizeof( char ) * Size, 1, pFile );
	}

	for( int32 j = 0; j < KContextTips; j++ )
	{
		bool TipSeen = m_pTips->TipSeen( (EContextTip)j );
		fwrite( &TipSeen, sizeof( bool ), 1, pFile );
	}
	// save the player's description
	CCharacterSaveInstance* pInstance = new CCharacterSaveInstance();
	m_pPlayer->FillInstance( *pInstance, *m_pActiveLevel );
	pInstance->m_DungeonSeed = m_DungeonSeed;
	pInstance->Save( pFile );
	DELETE_SAFELY( pInstance );

	uint32 Pets = m_pPlayer->Pets();

	fwrite( &Pets, sizeof( uint32 ), 1, pFile );

	// save the pet's description
	for( uint32 i = 0; i < Pets; i++ )
	{
		pInstance = new CCharacterSaveInstance();
		m_pPlayer->Pet( i )->FillInstance( *pInstance, *m_pActiveLevel );
		pInstance->Save( pFile );
		DELETE_SAFELY( pInstance );
	}

	uint32 Histories = m_pPlayer->LevelHistories();
	fwrite( &Histories, sizeof( uint32 ), 1, pFile );

	for( int32 i = 0; i < Histories; i++ )
	{
		m_pPlayer->GetLevelHistoryByIndex( i )->Save( pFile );
	}

	//m_pPlayer->SetGemsCollected("Ruby1,Ruby2");
	/*m_pPlayer->SetFishCaught("fish1,fish2");*/

	uint16 Size = m_pPlayer->GetGemsCollected().length();
	fwrite( &Size, sizeof( uint16 ), 1, pFile );
	fwrite( m_pPlayer->GetGemsCollected().c_str(), sizeof( char ) * Size, 1, pFile );

	Size = m_pPlayer->GetFishCaught().length();
	fwrite( &Size, sizeof( uint16 ), 1, pFile );
	fwrite( m_pPlayer->GetFishCaught().c_str(), sizeof( char ) * Size, 1, pFile );	

	//Cheat System Control / Achievements
	bool cheatAchievements = m_pPlayer->GetDisabledCheatAchievements();
	fwrite( &cheatAchievements, sizeof( bool ), 1, pFile );

	fclose( pFile );
	
	//steam cloud save
	m_pGameSave->StoreSlotOnCloud(m_pSlotIndex, SavePath, CGame::GetLocaleString());

} // CGameClient::SaveCharacter()

void CGameClient::LoadCharacter( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
								 const std::string& Path,		// path to file
								 std::vector< CCharacter* >& PetList,
								 std::vector< CCharacterSaveInstance* >& PetDescriptions )	// list of pets
{
	m_pGameUI->SetHelpButtonVisible( kFalse );
	m_pTips->ClearState();

	m_pGameSave->GetSlotDataFromCloud(m_pSlotIndex, Path, CGame::GetLocaleString());

	FILE *pFile;
	pFile = fopen ( Path.c_str(), "rb" );

	if( pFile == NULL )
	{
		return;
	}

	m_pPlayer =  new CPlayer( pD3DDevice,
							  m_pRefManager,
							  m_pSettings,
							  *m_pSpellManifest,
							  *m_pParticleManager,
							  *m_pGlowManager,
							  *m_pPositionalAudioManager,
							  KMale,
							  0,
							  0 );

	bool MapVisible = m_pGameUI->MapVisible();
	EMiniMapDolly MapDolly = m_pGameUI->MapDolly();
	fread( &MapVisible, sizeof( bool ), 1, pFile );
	fread( &MapDolly, sizeof( EMiniMapDolly ), 1, pFile );

	m_pGameUI->SetMapVisible( MapVisible );
	m_pGameUI->SetMapDolly( MapDolly );

	bool Running;
	bool ShowingItems;
	fread( &Running, sizeof( bool ), 1, pFile );
	fread( &ShowingItems, sizeof( bool ), 1, pFile );

	m_pGameUI->SetAlwaysRunning( Running );
	m_pGameUI->SetAlwaysShowingItems( ShowingItems );

	for( uint32 j = 0; j < KMaxHotkeys; j++ )
	{
		std::string Hotkey = "";
		uint16 Size( 0 );
		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			Hotkey = ReadString( pFile, Size );
		}
		m_pGameUI->SetHotkeyAssignment( j, Hotkey );
	}

	m_pGameUI->SetHotkeysChanged();

	for( int32 j = 0; j < KContextTips; j++ )
	{
		bool TipSeen;
		fread( &TipSeen, sizeof( bool ), 1, pFile );
		m_pTips->SetTipSeen( (EContextTip)j, TipSeen );
	}

	// save the player's description
	CCharacterSaveInstance* pInstance = new CCharacterSaveInstance;
	pInstance->Load( pFile );
	m_DungeonSeed = pInstance->m_DungeonSeed;
	m_pPlayer->ApplyInstance( pD3DDevice, m_pActiveLevel, *m_pCharacterManifest, *pInstance, *m_pItemManifest );
	DELETE_SAFELY( pInstance );

	uint32 Pets( 0 );

	fread( &Pets, sizeof( uint32 ), 1, pFile );

	// save the pet's description
	for( uint32 i = 0; i < Pets; i++ )
	{
		CCharacterSaveInstance* pPetInstance = new CCharacterSaveInstance();
		pPetInstance->Load( pFile );

		CCharacterDescription* pDescription = m_pCharacterManifest->CharacterDescription( pPetInstance->m_OriginalTemplateName );
		if( pDescription != NULL )
		{
			CCharacter* pNewPet =	new CMonster( pD3DDevice,
												  m_pRefManager,
												  m_pSettings,
												  *m_pSpellManifest,
												  *m_pParticleManager,
												  *m_pGlowManager,
												  *m_pPositionalAudioManager,
												  *pDescription );

			
			PetList.push_back( pNewPet );
			PetDescriptions.push_back( pPetInstance );
		}
		else
		{
			DELETE_SAFELY( pInstance );
		}

	}

	uint32 Histories( 0 );
	fread( &Histories, sizeof( uint32 ), 1, pFile );

	for( int32 i = 0; i < Histories; i++ )
	{
		CLevelHistory* pHistory = new CLevelHistory( 0, 0, 0 );
		pHistory->Load( pFile );
		m_pPlayer->AddDiscoveryHistory( pHistory );
	} 

	uint16 Size( 0 );
	
	fread( &Size, sizeof( uint16 ), 1, pFile );
	std::string m_GemsCollected = ReadString( pFile, Size );
	fread( &Size, sizeof( uint16 ), 1, pFile );
	std::string m_FishCaught = ReadString( pFile, Size );

	m_pPlayer->SetGemsCollected(m_GemsCollected);
	m_pPlayer->SetFishCaught(m_FishCaught);

	//Cheat System Control / Achievements
	bool cheatAchievements = kFalse;
	fread( &cheatAchievements, sizeof( bool ), 1, pFile );
	m_pPlayer->SetDisabledCheatAchievements(cheatAchievements);

	fclose( pFile );

	SetDifficulty( m_pPlayer->Difficulty() );

} // CGameClient::LoadCharacter()

void CGameClient::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if( m_pSettings.GetSettings( KSetMotionBlur ) )
	{
		m_pBlurLayer->RestoreDeviceObjects( pD3DDevice, m_pSettings );
	}

	if( m_pGameUI != NULL )
	{
		m_pGameUI->RestoreDeviceObjects( pD3DDevice );
	}
} // CGameClient::RestoreDeviceObjects()



void CGameClient::ResetMouse( void )
{
	if( m_pGameUI != NULL )
	{
		m_pGameUI->ResetMouse();
	}
} // CGameClient::ResetMouse()


void CGameClient::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{

	m_pGameUI->KeyEvent( WMMessage, KeyCode );
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CGameClient::KeyEvent()

void CGameClient::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
							  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	m_pGameUI->MouseEvent( WMMessage, WMParam );

	m_MouseHandler.MouseEvent( WMMessage, WMParam );

} // CGameClient::KeyEvent()

void CGameClient::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	float32 Delta = (float32)m_MouseHandler.MouseWheelDelta();
	Delta *= -.05f;
	m_Dolly += Delta;
	if( m_Dolly < KMinimumDolly )
	{
		m_Dolly = KMinimumDolly;
	}
	else if( m_Dolly > KMaximumDolly )
	{
		m_Dolly = KMaximumDolly;
	}

	if( m_MouseHandler.ButtonPressed( KButtonMiddle ) )
	{
		m_Dolly = KDefaultDolly;
	}

	if( m_pGameUI->ConsoleOpen() )
	{
		return;
	}
 
	if( m_pPlayer != NULL &&
		m_pPlayer->Alive() )
	{
		if( m_KeyHandler.KeyPressed( 82 ) )
		{
			m_pGameUI->SetAlwaysRunning( !m_pGameUI->AlwaysRunning() );
		}

		if( m_KeyHandler.KeyHeld( VK_CONTROL ) ||
			m_pGameUI->AlwaysRunning() )
		{
			m_pPlayer->SetRunning( kTrue );
		}
		else
		{
			m_pPlayer->SetRunning( kFalse );
		}
		if( m_KeyHandler.KeyHeld( VK_SHIFT ) )
		{
			m_pPlayer->SetHoldPosition( kTrue );
		}
		else		
		{
			m_pPlayer->SetHoldPosition( kFalse );
		}


	}

	if( !m_pGameUI->Paused() &&
		!m_pGameUI->ActionPaused() )
	{
		if( m_KeyHandler.KeyPressed( VK_UP )  ||
			m_KeyHandler.KeyPressed( 87 ) ) // W
		{
			m_Dolly -= 2;
		}
		if( m_KeyHandler.KeyPressed( VK_DOWN ) ||
			m_KeyHandler.KeyPressed( 83 ) ) // S 
		{
			m_Dolly += 2;
		}

		if( ( m_KeyHandler.KeyHeld( VK_LEFT ) ||
			m_KeyHandler.KeyHeld( 65 ) ) && 
			!( m_KeyHandler.KeyHeld( VK_RIGHT ) ||
				m_KeyHandler.KeyHeld( 68 ) ) )
		{
			m_pCamera->SetTargetOrbit( 45 );
		}
		else if( ( m_KeyHandler.KeyHeld( VK_RIGHT ) ||
			m_KeyHandler.KeyHeld( 68 ) ) && 
			!( m_KeyHandler.KeyHeld( VK_LEFT ) ||
				m_KeyHandler.KeyHeld( 65 ) ) )
		{
			m_pCamera->SetTargetOrbit( -45 );
		}
		else
		{
			m_pCamera->SetTargetOrbit( 0 );
		}
	}

} // CGameClient::ProcessInput()

void CGameClient::Update( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						  HWND hWnd,							// hwnd of the active window
						  float32 TimeElapsed )					// time elapsed, in seconds
{

	m_pActiveLevel->ClearEvents();
	m_EventTime = (float32)Game_Timer( KTimerGetAbsoluteTime );

	// check for any pending messages
	if( m_pPlayer->Alive() &&
		!m_pGameUI->Paused() )
	{
		if( m_pActiveLevel->TipPending() )
		{
			m_pGameUI->OpenTip( pD3DDevice, m_pActiveLevel->PendingTipText() );
			m_pActiveLevel->ClearPendingTip();
		}
		else if( m_pActiveLevel->MessagePending() )
		{
			m_pGameUI->OpenMessage( pD3DDevice, 
									m_pActiveLevel->PendingMessageText(),
									LoadLocalizedString(185) );
			m_pActiveLevel->ClearPendingMessage();
		}
		else if( m_pActiveLevel->EventMessagePending() )
		{
			m_pGameUI->OpenConfirmation( pD3DDevice, 
										 m_pActiveLevel->PendingMessageText(),
										 m_pActiveLevel->YesMessageText(),
										 KButtonEventYes,
										 m_pActiveLevel->NoMessageText(),
										 KButtonEventNo );
			m_pActiveLevel->ClearPendingEventMessage();
		}
		if( m_pActiveLevel->EnchantMessagePending() )
		{
			m_pGameUI->CloseAllMenus( *m_pActiveLevel );
			m_pGameUI->ToggleInventoryMenu( *m_pActiveLevel );
			m_pGameUI->OpenEnchant( pD3DDevice, 
									m_pActiveLevel->EnchantPosition(),
									 m_pActiveLevel->PendingMessageText(),
									 m_pActiveLevel->YesMessageText(),
									 KButtonEventYes,
									 m_pActiveLevel->NoMessageText(),
									 KButtonEventNo );
			m_pActiveLevel->ClearPendingEnchantMessage();
		}
	}

	// check player's quests
	if( m_pPlayer->Alive() &&
		!m_pGameUI->Paused() &&
		!m_pGameUI->ActionPaused() )
	{
		UpdateQuests( pD3DDevice );
		if( !m_pSoundManager.MusicPlaying() )
		{
			StartMusic();
		}
	}
	// check player's fish catching
	if( m_pPlayer->Alive() &&
		!m_pGameUI->Paused() &&
		!m_pGameUI->ActionPaused() )
	{
		if( m_pPlayer->AIState() == KAIReceiveFish )
		{
			if( m_pPlayer->CaughtFish() )
			{
				m_pGameUI->FlushAll();
				CItemDescription* pItemDescription = m_pItemManifest->ItemDescription( m_pActiveLevel->RandomFishItemIndex() );
				if( pItemDescription != NULL )
				{

					FSOUND_PlaySound( FSOUND_FREE, m_pRewardSound );
					CItem* pItem = m_pActiveLevel->AddItem( *pItemDescription, m_pPlayer->Position() );
					m_pActiveLevel->RemoveItem( pItem );
					if( RandomIntegerBetween( 0, 100 ) < 20 )
					{
						pItem->BestowPowers( m_pActiveLevel->LevelDepth(), kTrue );
					}
					if( RandomIntegerBetween( 0, 100 ) < 10 )
					{
						pItem->BestowSockets();
					}
					m_pPlayer->GiveItem( *m_pActiveLevel, pItem );

					if( pItem->Type() != KItemPetFood )
					{
						m_pPlayer->IncrementStat( KJournalStatFishCaught );

						m_pGameUI->OpenMessage( pD3DDevice, 
												LoadLocalizedString1Variable(186, pItem->BaseModifiedName() ),
												LoadLocalizedString(185));
					}
					else
					{
						m_pPlayer->IncrementStat( KJournalStatFishCaught );

						m_pGameUI->OpenMessage( pD3DDevice, 
												LoadLocalizedString1Variable(186, pItem->BaseModifiedName() ),
												LoadLocalizedString(185));
					}
				}
				else
				{
					m_pGameUI->FlushAll();
				}
			}
			else
			{
				m_pGameUI->FlushAll();
			}
			m_pPlayer->SetAIState( KAIInactive );
		}
	}

	if( m_pPlayer->AIState() == KAIDead &&
		!m_pGameUI->FateOpen() &&
		!m_pGameUI->Paused() &&
		!m_pGameUI->ActionPaused() &&
		m_pGameStateManager.GameState() != KGameStateQuitToMenu )
	{
		m_pGameUI->OpenMessage( pD3DDevice, 
								LoadLocalizedString(188),
								LoadLocalizedString(185),
								KButtonFate );
	}


	VerifyMouseOvers();

	// find all onscreen items
	m_pActiveLevel->FindItemsOnscreen( m_pCamera->ViewMatrix(),
									   m_pCamera->ProjectionMatrix() );

	// find all onscreen characters
	m_pActiveLevel->FindCharactersOnscreen( m_pCamera->ViewMatrix(),
										    m_pCamera->ProjectionMatrix() );

	bool MouseWasOver = m_pGameUI->MouseOverMenus() || m_pGameUI->Paused() || m_pGameUI->ActionPaused();
	bool DraggingItem = m_pGameUI->DraggingItem();
	m_pGameUI->Update( pD3DDevice,
					   *this,
					   *m_pActiveLevel,
					   hWnd, 
					   m_pCamera->ViewType(), 
					   TimeElapsed );



	// try to automatically pick up any gold around the player
	AutoGetGold();

	// if we just finished manipulating an item,
	// flush the mouse buffer - we don't want to
	// immediately start pathing
	if( DraggingItem != m_pGameUI->DraggingItem() )
	{
		m_MouseHandler.FlushAll();
	}

	if( ( m_pGameUI->MouseOverMenus() ||
		  m_pGameUI->Paused() ||
		  m_pGameUI->ActionPaused() ||
		  MouseWasOver ) &&
		( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
		  m_MouseHandler.ButtonDoubleClick( KButtonLeft ) ) )
	{
		m_MouseHandler.FlushAll();
	}

	ProcessInput( hWnd );

	if( m_TimeToRePath > 0 )
	{
		m_TimeToRePath -= TimeElapsed;
	}

	if( !m_MouseHandler.ButtonHeld( KButtonLeft ) )
	{
		m_ClickBroken = kTrue;
		UpdateMouseOver();
	}

	if( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
		m_MouseHandler.ButtonDoubleClick( KButtonLeft ) )
	{
		UpdateMouseOver();
		m_ClickBroken = kTrue;
		m_TimeToRePath = 0;
		Click();
	}
	else if( m_MouseHandler.ButtonHeld( KButtonLeft ) )
	{
		Hold();
	}
	else
	{
		m_ClickBroken = kTrue;
		m_TimeToRePath = 0;
	}

	if( m_pMouseOverCharacter != NULL )
	{
		m_pMouseOverCharacter->SetMouseOver();
	}

	// cap the elapsed time in case it's really large
	if( TimeElapsed > .3 )
	{
		TimeElapsed = .3f;
	}
	if( m_pGameUI->Paused() ||
		m_pGameUI->ActionPaused() )
	{
		// make sure we update help voices
		m_pPlayer->UpdateHelp( TimeElapsed );
		TimeElapsed = 0;
	}
	if( m_pGameUI->BothCovered() )
	{
		// make sure we update help voices
		if( TimeElapsed != 0 )
		{
			m_pPlayer->UpdateHelp( TimeElapsed );
		}
		TimeElapsed = 0;
		m_pCamera->SetViewType( KViewCovered );
	}
	else if( m_pGameUI->RightCovered() )
	{
		m_pCamera->SetViewType( KViewLeft );
	}
	else if( m_pGameUI->LeftCovered() )
	{
		m_pCamera->SetViewType( KViewRight );
	}
	else
	{	
		m_pCamera->SetViewType( KViewFull );
	}

	// break the timeslice into smaller pieces, if required
	// this is important for solid physics - you can't allow too large of a timestep,
	// or things begin to break down
	uint32	Ticks		= ( uint32 )ceil( TimeElapsed / .01f );
	if ( Ticks > 5 )
	{
		Ticks = 5;
	}
	if( Ticks < 1 )
	{
		Ticks = 1;
	}
	// calculate the sub-interval length for each tick
	float32	TickInterval	= ( TimeElapsed / ( ( float32 ) Ticks ) );

	if( !m_pActiveLevel->RequestingLevelChange() )
	{
		// perform ticks for the objects in the scene
		for ( int32 i = Ticks - 1; i >= 0; i-- )
		{
			// update any moving game objects
			m_pActiveLevel->UpdateCharacters( m_pPlayer->Position(),
												TickInterval );
		}
	}

	m_pActiveLevel->UpdateCharacterAnimation( TimeElapsed );

	float32 LightRadius( (float32)KMaxVisibilityRadius * m_pActiveLevel->VisibilityCellWidth() );

	m_pActiveLevel->CalculateLocalLight( m_pPlayer->Position(),
										 TimeElapsed,
										 LightRadius );

	D3DXVECTOR3 Target = m_pPlayer->Position();
	Target.y = 0;
	D3DXVECTOR3 Offset( .35f, .86f, -.35f );

	m_pCamera->SetLookTarget( Target );
	m_pCamera->SetCameraOffset( Offset );
	m_pCamera->SetDolly( m_Dolly );
	
	// we want to update the camera every frame, whether we move it or not,
	// because something about the projection matrix might have changed
	m_pCamera->Update( TimeElapsed );

	m_pActiveLevel->Update( TimeElapsed );

	m_pActiveLevel->UpdateVisibility( m_pPlayer->Position() );

	// update all particle systems
	m_pParticleManager->Update( TimeElapsed );

	// update all glow systems
	m_pGlowManager->Update( TimeElapsed );

	// update all positional audio samples
	m_pPositionalAudioManager->Update( m_pPlayer->Position(),
									   m_pCamera->Orientation(),
									   TimeElapsed );

	m_BlurTime += TimeElapsed;

	m_MouseHandler.Flush();
	m_KeyHandler.Flush();

	VerifyMouseOvers();


	m_EventTime = (float32)Game_Timer( KTimerGetAbsoluteTime ) - m_EventTime;

} // CGameClient::Update()

void CGameClient::UpdateBlurRate( float32 FPS )	// frames/per/second
{
	// now update the fading percentage of the blur layer based upon time elapsed
	// the longer the timeslice, the more we need to fade the previous blur.
	if( m_pSettings.GetSettings( KSetMotionBlur ) )
	{

		float32 BlurAmount = FPS;
		if( BlurAmount > 75 )
		{
			BlurAmount = 75;
		}
		BlurAmount -= 25;
		BlurAmount /= 100.0f;

		BlurAmount *= 1.33f;
		BlurAmount *= -.75f;
		BlurAmount += 1.0f;

		if( BlurAmount > .35f )
		{
			BlurAmount = .35f;
		}
		if( BlurAmount < .2f )
		{
			BlurAmount = .2f;
		}

		m_pBlurLayer->SetFrameFade( BlurAmount );
	}
} // CGameClient::UpdateBlurRate()

void CGameClient::PreRender( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	D3DXVECTOR3 Target = m_pPlayer->Position();
	D3DXVECTOR3 Start = m_pPlayer->Position();
	D3DXVECTOR3 Up( 0, 0, 1 );
	Start.y += 35;

	D3DXMatrixLookAtLH( &m_ShadowViewMatrix, &Start, &Target, &Up );

	
	RenderShadows( pD3DDevice, m_ShadowViewMatrix, m_pActiveLevel->LevelDepth() == 0 );

	D3DXMatrixLookAtLH( &m_ShadowViewMatrix, &Start, &Target, &Up );

	RenderBlurLayer( pD3DDevice );

} // CGameClient::PreRender()

void CGameClient::Render( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{

	if( m_pCamera->ViewType() == KViewCovered )
	{
		D3DVIEWPORT8 Viewport;
		Viewport.X = 0;
		Viewport.Y = 0;
		Viewport.Width = m_pSettings.GetSettings( KSetScreenWidth );
		Viewport.Height = m_pSettings.GetSettings( KSetScreenHeight );
		Viewport.MinZ = 0.0f;
		Viewport.MaxZ = 1.0f;

		pD3DDevice->SetViewport( &Viewport );

		// set  our projection to the camera's projection matrix
		pD3DDevice->SetTransform( D3DTS_PROJECTION, &m_pCamera->ProjectionMatrix() );

		// turn on lighting for objects
		pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

		m_pGameUI->Render3D( pD3DDevice,
							 *this,
							 m_pCamera->ViewMatrix(),
							 m_pCamera->ProjectionMatrix() );
		return;
	}

	if( m_pActiveLevel->RequestingLevelChange() )
	{
		if( !m_pGameUI->Paused() )
		{
			m_pGameUI->OpenConfirmation( pD3DDevice,
										 m_pActiveLevel->LevelChangeMessage(),
										 LoadLocalizedString(171),
										 KButtonChangeLevels,
										 LoadLocalizedString(172),
										 KButtonNo );
		}

		if( m_pGameUI->ConfirmMenuResult() != -1 ||
			m_pActiveLevel->IsLevelChangeSilent() )
		{
			// currently the only other confirmation is for level change -
			// there should be more ( purchasing, etc. )
			if( m_pGameUI->ConfirmMenuResult() == KButtonChangeLevels ||
				m_pActiveLevel->IsLevelChangeSilent() )
			{
				if( m_pActiveLevel->RequestingTeleport() )
				{
					m_pPlayer->PlayHelpSample( KSoundHelpPortal, kTrue );
					m_pPlayer->UpdateHelp( 1 );
				}

				uint32 NewLevelDepth = m_pActiveLevel->RequestedLevelChange();
				GenerateLevel( pD3DDevice,
							NewLevelDepth,
							m_pActiveLevel->RequestedLevelChange() > m_pActiveLevel->LevelDepth(),
							m_pActiveLevel->RequestingTeleport(),
							m_pActiveLevel->IsLevelChangeSilent() );	// silent level changes are from the fate character
			}
			else
			{
				// set the requested level to the current level - they said no
				m_pActiveLevel->RequestLevelTeleport( m_pActiveLevel->LevelDepth() );
			}
			m_pGameUI->CloseConfirmation();
			// flush all events as we close
			FlushAll();
			return;
		}
	}
	else if( m_pPlayer->AIState() == KAISendPetToTown )
	{
		if( !m_pGameUI->Paused() &&
			!m_pGameUI->ActionPaused() )
		{
			if( m_pActiveLevel->LevelDepth() == 0 )
			{
				m_pGameUI->OpenMessage( pD3DDevice, LoadLocalizedString(1124) );
				m_pPlayer->SetAIState( KAIInactive );
			}
			else
			{
				m_pGameUI->OpenConfirmation( pD3DDevice,
											LoadLocalizedString(189),
											LoadLocalizedString(171),
											KButtonEventYes,
											LoadLocalizedString(172),
											KButtonNo );
			}
		}
		else
		{
			if( m_pGameUI->ConfirmMenuResult() != -1 )
			{
				// event confirmations are passed to the player
				if( m_pGameUI->ConfirmMenuResult() == KButtonEventYes )
				{
					m_pPlayer->Pet( 0 )->ReturnToTown( *m_pActiveLevel );
				}
				m_pGameUI->CloseConfirmation();
				m_pPlayer->SetAIState( KAIInactive );
			}
		}
	}
	else if( m_pPlayer->AIState() == KAIInEvent ||
			 ( m_pPlayer->Pets() > 0 && m_pPlayer->Pet( 0 )->AIState() == KAIInEvent ) )
	{
		// select the player participating in the event ( player or pet )
		CCharacter* pPlayer = m_pPlayer;
		if( m_pPlayer->Pets() > 0 && m_pPlayer->Pet( 0 )->AIState() == KAIInEvent )
		{
			pPlayer = m_pPlayer->Pet( 0 );
		}

		if( m_pGameUI->ConfirmMenuResult() != -1 )
		{
			// event confirmations are passed to the player
			if( m_pGameUI->ConfirmMenuResult() == KButtonEventYes ||
				m_pGameUI->ConfirmMenuResult() == KButtonEventNo )
			{
				pPlayer->CompleteEvent( *m_pNameGenerator,
										  *m_pActiveLevel,
										  m_pGameUI->ConfirmMenuResult(),
										  NULL );
				m_pGameUI->CloseConfirmation();
			}
		}
		if( m_pGameUI->EnchantMenuResult() != -1 )
		{
			// gem/weapon extractions are a special event
			if( m_pGameUI->EnchantMenuResult() == KButtonEventBreakCancel )
			{
				if( m_pGameUI->EnchantMenuItem() != NULL )
				{
					m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
															m_pGameUI->EnchantMenuItem() );
					m_pGameUI->SetEnchantMenuItem( NULL, NULL );
											 
				}
				m_pGameUI->CloseEnchant( *m_pActiveLevel );
				m_pGameUI->CloseAllMenus( *m_pActiveLevel );
				pPlayer->SetAIState( KAIInactive );
				if( m_pPlayer->Pet( 0 )->AvailableForCommand() )
				{
					m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
				}
			}
			else if( m_pGameUI->EnchantMenuResult() == KButtonEventHeirloom )
			{
				if( m_pGameUI->EnchantMenuItem() != NULL )
				{
					pPlayer->SetAIState( KAIInactive );
					if( m_pPlayer->Pet( 0 )->AvailableForCommand() )
					{
						m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
					}
					m_RetirementLineage = m_pPlayer->Lineage();
					m_pPlayer->SetLineage( -1 );
					SaveCharacter();
					DELETE_SAFELY( m_pHeirloomItem );

					if( m_pGameUI->EnchantMenuItem() != NULL )
					{
						m_pHeirloomItem = new CItemSaveInstance();
						m_pGameUI->EnchantMenuItem()->FillInstance( *m_pHeirloomItem, -1, kFalse );
						if( m_pGameUI->EnchantMenuItem()->Category() == KCategoryWeapon ||
							m_pGameUI->EnchantMenuItem()->Category() == KCategoryArmor ||
							m_pGameUI->EnchantMenuItem()->Category() == KCategoryJewelry )
						{
							for( uint32 i = 0; i < KActivationTypes; i++ )
							{
								for( uint32 j = 0; j < m_pHeirloomItem->m_pEffects[i].size(); j++ )
								{
									m_pHeirloomItem->m_pEffects[i][j]->SetValue( m_pHeirloomItem->m_pEffects[i][j]->Value() + ceil( m_pHeirloomItem->m_pEffects[i][j]->Value() * .25f ) );
								}
							}

							for( int32 i = 0; i < m_pHeirloomItem->m_DamageBonusValue.size(); i++ )
							{
								m_pHeirloomItem->m_DamageBonusValue[i] += (int32)ceil( m_pHeirloomItem->m_DamageBonusValue[i] * .25f );
							}
							m_pHeirloomItem->m_Uses++;
						}
						if( ( m_pGameUI->EnchantMenuItem()->Category() == KCategoryWeapon ||
							m_pGameUI->EnchantMenuItem()->Category() == KCategoryArmor ) &&
							m_pGameUI->EnchantMenuItem()->Grade() < KGradeFlawless )
						{
							// improve the grade, if possible
							m_pHeirloomItem->m_Grade = (EGrade)( (int32)m_pHeirloomItem->m_Grade + 1 );
						}
						m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
																m_pGameUI->EnchantMenuItem() );
						m_pGameUI->SetEnchantMenuItem( NULL, NULL );
					}
					m_RetirementText = LoadLocalizedString2Variables(190, m_pPlayer->Name(), IntegerToString( m_pPlayer->Level()));
					m_RetirementText = m_RetirementText + LoadLocalizedString1Variable(191, KFameName[ m_pPlayer->FameRank() ]);
					m_RetirementText = m_RetirementText + LoadLocalizedString(192) + IntegerToString( m_pPlayer->JournalStat( KJournalStatGoldGathered ) );
					m_RetirementText = m_RetirementText + LoadLocalizedString(193) + IntegerToString( m_pPlayer->JournalStat( KJournalStatQuestsCompleted ) );
					m_RetirementText = m_RetirementText + LoadLocalizedString(194) + IntegerToString( m_pPlayer->JournalStat( KJournalStatMonstersKilled ) );
					m_RetirementText = m_RetirementText + LoadLocalizedString(195) + IntegerToString( m_pPlayer->JournalStat( KJournalStatFamousMonstersKilled ) );

					m_pGameStateManager.SetTargetMenu( KRetireMenu );


					m_pGameUI->CloseEnchant( *m_pActiveLevel );
					m_pGameUI->CloseAllMenus( *m_pActiveLevel );
				}
				else
				{
					m_pGameUI->ClearEnchantMenuResult();
				}

			}
			else if( m_pGameUI->EnchantMenuResult() == KButtonEventBreakGems )
			{
				if( m_pGameUI->EnchantMenuItem() != NULL )
				{
					if( m_pGameUI->EnchantMenuItem()->Items() > 0 )
					{
						m_pGameUI->EnchantMenuItem()->DeleteContainedItems();
						if( !m_pGameUI->EnchantMenuItem()->IsUnique() )
						{
							m_pGameUI->EnchantMenuItem()->DetermineName();
						}
						m_pGameUI->OpenMessage( pD3DDevice,
												LoadLocalizedString(196), LoadLocalizedString(185) );
						FSOUND_PlaySound( FSOUND_FREE, m_pQuestSound );
					}
					else
					{
						FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
						m_pGameUI->OpenMessage( pD3DDevice,
												LoadLocalizedString(197), LoadLocalizedString(185) );
					}
					
					m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
															 m_pGameUI->EnchantMenuItem() );
					m_pGameUI->SetEnchantMenuItem( NULL, NULL );
											 
				}
				m_pGameUI->CloseEnchant( *m_pActiveLevel );
				m_pGameUI->CloseAllMenus( *m_pActiveLevel );
				pPlayer->SetAIState( KAIInactive );
				if( m_pPlayer->Pet( 0 )->AvailableForCommand() )
				{
					m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
				}
			}
			else if( m_pGameUI->EnchantMenuResult() == KButtonEventBreakWeapon )
			{
				if( m_pGameUI->EnchantMenuItem() != NULL )
				{

					if( m_pGameUI->EnchantMenuItem()->Items() > 0 )
					{
						std::string ResultText = LoadLocalizedString1Variable(198, m_pGameUI->EnchantMenuItem()->Name() );
						// give all the internal items of the weapon/armor to the character
						while( m_pGameUI->EnchantMenuItem()->Items() > 0 )
						{
							ResultText = ResultText + "\n\b" + m_pGameUI->EnchantMenuItem()->SubItem( 0 )->Name() + "\b";
							m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
																	 m_pGameUI->EnchantMenuItem()->SubItem( 0 ) );
							m_pGameUI->EnchantMenuItem()->RemoveItem( m_pGameUI->EnchantMenuItem()->SubItem( 0 ) );
						}
						CItem* pItem = m_pGameUI->EnchantMenuItem();
						DELETE_SAFELY( pItem );
						m_pGameUI->SetEnchantMenuItem( NULL, NULL );											 
						m_pGameUI->OpenMessage( pD3DDevice,
												ResultText, LoadLocalizedString(185) );
						FSOUND_PlaySound( FSOUND_FREE, m_pQuestSound );
					}
					else
					{
						FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
						m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
																 m_pGameUI->EnchantMenuItem() );
						m_pGameUI->SetEnchantMenuItem( NULL, NULL );
						m_pGameUI->OpenMessage( pD3DDevice,
												LoadLocalizedString(199), LoadLocalizedString(185) );
					}
				}
				m_pGameUI->CloseEnchant( *m_pActiveLevel );
				m_pGameUI->CloseAllMenus( *m_pActiveLevel );
				pPlayer->SetAIState( KAIInactive );
				if( m_pPlayer->Pet( 0 )->AvailableForCommand() )
				{
					m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
				}
			}
			else if( m_pGameUI->EnchantMenuResult() == KButtonEventEnchantItem )
			{
				if( m_pGameUI->EnchantMenuItem() != NULL )
				{
					uint32 Price = m_pGameUI->EnchantMenuItem()->EnchantValue();
					if( m_pPlayer->Inventory()->Gold() >= Price )
					{
						bool Success = kFalse;
						m_pPlayer->TakeGold( Price );
						FSOUND_PlaySound( FSOUND_FREE, m_pGoldSound );
						std::string ResultText;
						int32 ModifiedLevelDepth( m_pPlayer->Level() );
						ModifiedLevelDepth = (int32)ceil( (float32)ModifiedLevelDepth * KDifficultyMultiplier[Difficulty()] );
						ModifiedLevelDepth += (int32)KDifficultyLevelBase[Difficulty()];
						if( ModifiedLevelDepth < 4 )
						{
							ModifiedLevelDepth = 4;
						}

						CItem* pItem = m_pGameUI->EnchantMenuItem();
						if( pItem->Category() != KCategoryWeapon &&
							pItem->Category() != KCategoryArmor &&
							pItem->Category() != KCategoryJewelry )
						{
							ResultText = LoadLocalizedString1Variable(200, pItem->BaseModifiedName() );
							Success = kFalse;
						}
						else if( pItem->Sockets() < (uint32)pItem->SlotsTall() &&
								RandomIntegerBetween( 0, 100 ) < 20 )
						{
							pItem->BestowSockets( kTrue );
							ResultText = LoadLocalizedString1Variable(201, pItem->BaseModifiedName() );
							Success = kTrue;
						}
						else if( RandomIntegerBetween( 0, 100 ) < 50 &&
								!pItem->IsArtifact() )
						{
							pItem->BestowPowers( ModifiedLevelDepth,
												kTrue );
							pItem->SetIdentified( kTrue );
							ResultText = LoadLocalizedString1Variable(202, pItem->BaseModifiedName() );
							Success = kTrue;
							// make SURE it got something.
							while( pItem->Effects( KActivationPassive ) == 0 &&
									pItem->DamageBonuses() == 0 )
							{
								pItem->BestowPowers( ModifiedLevelDepth,
													kTrue );
								pItem->SetIdentified( kTrue );
							}
						}
						else if( RandomIntegerBetween( 0, 100 ) < 25 &&
								!pItem->IsUnique() )
						{
							pItem->BestowUniquePowers( *NameGenerator(),
													   ModifiedLevelDepth,
													   kFalse );
							pItem->SetIdentified( kTrue );
							ResultText = LoadLocalizedString1Variable(203, pItem->BaseModifiedName() );
							Success = kTrue;
						}
						else if( RandomIntegerBetween( 0, 100 ) < 6 &&
								pItem->Effects( KActivationPassive ) > 0 )
						{
							pItem->RemovePowers();
							ResultText = LoadLocalizedString1Variable(204, pItem->BaseModifiedName() );
							Success = kFalse;
						}
						else
						{
							ResultText = LoadLocalizedString1Variable(205, pItem->BaseModifiedName() );
							Success = kFalse;
						}
						if( Success )
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pEnchantSound );

							if( !pItem->IsCursed() )
							{
								m_pPlayer->PlayHelpSample( KSoundHelpFateSmiles );
							}
						}
						else
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pEnchantSound );
						}
						m_pActiveLevel->RequestMessage( ResultText );

						m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
																m_pGameUI->EnchantMenuItem() );
						m_pGameUI->SetEnchantMenuItem( NULL, NULL );

						m_pPlayer->Inventory()->CalculateEffectValues();
						m_pGameUI->CloseEnchant( *m_pActiveLevel );
						m_pGameUI->CloseAllMenus( *m_pActiveLevel );
						pPlayer->SetAIState( KAIInactive );
						if( m_pPlayer->Pet( 0 )->AvailableForCommand() )
						{
							m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
						}
					}
					else
					{
						m_pPlayer->PlayHelpSample( KSoundHelpLowGold );
					}
					m_pGameUI->ClearEnchantMenuResult();

				}

			}
			// event confirmations are passed to the player
			else if( m_pGameUI->EnchantMenuResult() == KButtonEventYes ||
				m_pGameUI->EnchantMenuResult() == KButtonEventNo )
			{
				if( m_pGameUI->EnchantMenuResult() == KButtonEventYes &&
					m_pGameUI->EnchantMenuItem() == NULL )
				{
					m_pGameUI->ClearEnchantMenuResult();
					FSOUND_PlaySound( FSOUND_FREE, m_pErrorSound );
				}
				else
				{
					if( m_pPlayer->Pet( 0 )->AvailableForCommand() )
					{
						m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
					}
					pPlayer->CompleteEvent( *m_pNameGenerator,
											*m_pActiveLevel,
											m_pGameUI->EnchantMenuResult(),
											m_pGameUI->EnchantMenuItem() );
					if( m_pGameUI->EnchantMenuItem() != NULL )
					{
						m_pGameUI->EnchantMenuOwner()->GiveItem( *m_pActiveLevel,
																 m_pGameUI->EnchantMenuItem() );
						m_pGameUI->SetEnchantMenuItem( NULL, NULL );
											 
					}
					m_pGameUI->CloseEnchant( *m_pActiveLevel );
					m_pGameUI->CloseAllMenus( *m_pActiveLevel );
				}
			}
		}
	}
	else if( m_pPlayer->AIState() == KAIReceiveQuest )
	{
		if( m_pGameUI->ConfirmMenuResult() != -1 )
		{
			if( m_pGameUI->ConfirmMenuResult() == KButtonAcceptQuest )
			{
				FSOUND_PlaySound( FSOUND_FREE, m_pQuestSound );
				m_pPlayer->GiveQuest( *m_pPlayer->Target()->Quest( 0 ) );
				m_pPlayer->Target()->ClearQuests();
				m_pGameUI->CloseConfirmation();
				m_pGameUI->OpenQuestMenu();
				m_pPlayer->SetTarget( NULL );
			}
			else
			{
				m_pGameUI->CloseConfirmation();
				m_pPlayer->SetTarget( NULL );
			}
			m_pPlayer->SetAIState( KAIInactive );
			// flush all events as we close
			FlushAll();
		}
	}
	else if( m_pPlayer->AIState() == KAISpeakMerchant )
	{
		if( m_pGameUI->Paused() )
		{
			if( m_pGameUI->ConfirmMenuResult() != -1 )
			{
				if( m_pGameUI->ConfirmMenuResult() == KButtonYes )
				{

					m_pGameUI->CloseConfirmation();
					if( m_pPlayer->Target()->MerchantType() == KMerchantHealer )
					{
						m_pPlayer->PlayHelpSample( KSoundHelpRefreshed );
						m_pActiveLevel->AddEventText( m_pPlayer,
													  m_pPlayer,
													  m_pPlayer->Position() + D3DXVECTOR3( 0, 6, 0 ),
													  LoadLocalizedString(206),
													  .75f,
													  kFalse,
													  .6f );
						m_pPlayer->ModifyHP( (float32)m_pPlayer->MaxHP() );
						m_pPlayer->ModifyStamina( (float32)m_pPlayer->MaxStamina() );
						m_pPlayer->ModifyMana( (float32)m_pPlayer->MaxMana() );
						m_pPlayer->RemoveEffect( "POISON" );

						m_pPlayer->Pet( 0 )->ModifyHP( (float32)m_pPlayer->Pet( 0 )->MaxHP() );
						m_pPlayer->Pet( 0 )->ModifyStamina( (float32)m_pPlayer->Pet( 0 )->MaxStamina() );
						m_pPlayer->Pet( 0 )->ModifyMana( (float32)m_pPlayer->Pet( 0 )->MaxMana() );
						m_pPlayer->Pet( 0 )->RemoveEffect( "POISON" );

						m_pPlayer->SetAIState( KAIInactive );
						m_pGameUI->CloseConfirmation();
						m_pPlayer->SetTarget( NULL );
					}
					else if( m_pPlayer->Target()->MerchantType() == KMerchantMinstrel )
					{
						int32 Price = KFameGate[m_pPlayer->FameRank() + 1] * 5;
						float32 Fame = (float32)m_pPlayer->Fame();
						if( Fame < (float32)KFameGate[m_pPlayer->FameRank() - 1] )
						{
							Fame = (float32)KFameGate[m_pPlayer->FameRank() - 1] ;
						}
						float32 Delta = Fame - (float32)KFameGate[m_pPlayer->FameRank() - 1];

						Delta /= KFameGate[m_pPlayer->FameRank()] - KFameGate[m_pPlayer->FameRank() - 1];
						Price = Price - (int32)( Delta * (float32)Price );
						if( (int32)m_pPlayer->Inventory()->Gold() >= Price )
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pGoldSound );
							m_pPlayer->TakeGold( Price );
							m_pPlayer->AwardFame( *m_pActiveLevel, NULL, KFameGate[m_pPlayer->FameRank()] - m_pPlayer->Fame() );
						}
						else
						{
							m_pPlayer->PlayHelpSample( KSoundHelpLowGold );
						}
						m_pPlayer->SetAIState( KAIInactive );
						m_pGameUI->CloseConfirmation();
						m_pPlayer->SetTarget( NULL );
					}
					else
					{
						m_pPlayer->SetAIState( KAIBuySell );
					}
				}
				else
				{
					m_pPlayer->SetAIState( KAIInactive );
					m_pGameUI->CloseConfirmation();
					m_pPlayer->SetTarget( NULL );
				}
				// flush all events as we close
				FlushAll();
			}
		}
	}
	else if( m_pPlayer->AIState() == KAIRetire )
	{
		if( m_pGameUI->Paused() )
		{
			if( m_pGameUI->ConfirmMenuResult() != -1 )
			{
				if( m_pGameUI->ConfirmMenuResult() == KButtonYes )
				{

					m_pGameUI->CloseConfirmation();
					m_pPlayer->SetAIState( KAICompleteRetire );
				}
				else
				{
					m_pPlayer->SetAIState( KAIInactive );
					m_pGameUI->CloseConfirmation();
					m_pPlayer->SetTarget( NULL );
				}
				// flush all events as we close
				FlushAll();
			}
		}
	}
	else if( m_pPlayer->Pets() > 0 &&
		     m_pPlayer->Pet( 0 )->AIState() == KAISpeakMerchant )	// pet speak to merchant
	{
		if( m_pGameUI->Paused() )
		{
			if( m_pGameUI->ConfirmMenuResult() != -1 )
			{
				if( m_pGameUI->ConfirmMenuResult() == KButtonYes )
				{
					m_pGameUI->CloseConfirmation();
					if( m_pPlayer->Pet( 0 )->Target()->MerchantType() == KMerchantHealer )
					{
						m_pPlayer->PlayHelpSample( KSoundHelpRefreshed );
						m_pActiveLevel->AddEventText( m_pPlayer,
													  m_pPlayer,
													  m_pPlayer->Position() + D3DXVECTOR3( 0, 6, 0 ),
													  LoadLocalizedString(206),
													  .75f,
													  kFalse,
													  .6f );
						m_pPlayer->ModifyHP( (float32)m_pPlayer->MaxHP() );
						m_pPlayer->ModifyStamina( (float32)m_pPlayer->MaxStamina() );
						m_pPlayer->ModifyMana( (float32)m_pPlayer->MaxMana() );
						m_pPlayer->RemoveEffect( "POISON" );

						m_pPlayer->Pet( 0 )->ModifyHP( (float32)m_pPlayer->Pet( 0 )->MaxHP() );
						m_pPlayer->Pet( 0 )->ModifyStamina( (float32)m_pPlayer->Pet( 0 )->MaxStamina() );
						m_pPlayer->Pet( 0 )->ModifyMana( (float32)m_pPlayer->Pet( 0 )->MaxMana() );
						m_pPlayer->Pet( 0 )->RemoveEffect( "POISON" );
						m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
						m_pGameUI->CloseConfirmation();
						m_pPlayer->Pet( 0 )->SetTarget( NULL );
					}
					else if( m_pPlayer->Pet( 0 )->Target()->MerchantType() == KMerchantMinstrel )
					{
						int32 Price = KFameGate[m_pPlayer->FameRank() + 1] * 5;
						float32 Fame = (float32)m_pPlayer->Fame();
						if( Fame < (float32)KFameGate[m_pPlayer->FameRank() - 1] )
						{
							Fame = (float32)KFameGate[m_pPlayer->FameRank() - 1] ;
						}
						float32 Delta = Fame - (float32)KFameGate[m_pPlayer->FameRank() - 1];
						Delta /= KFameGate[m_pPlayer->FameRank()] - KFameGate[m_pPlayer->FameRank() - 1];
						Price = Price - (int32)( Delta * (float32)Price );
						if( (int32)m_pPlayer->Inventory()->Gold() >= Price )
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pGoldSound );
							m_pPlayer->TakeGold( Price );
							m_pPlayer->AwardFame( *m_pActiveLevel, NULL, KFameGate[m_pPlayer->FameRank() + 1] - m_pPlayer->Fame() );
						}
						else
						{
							m_pPlayer->PlayHelpSample( KSoundHelpLowGold );
						}
						m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
						m_pGameUI->CloseConfirmation();
						m_pPlayer->Pet( 0 )->SetTarget( NULL );
					}
					else
					{
						m_pPlayer->Pet( 0 )->SetAIState( KAIBuySell );
					}
				}
				else
				{
					m_pPlayer->Pet( 0 )->SetAIState( KAIInactive );
					m_pGameUI->CloseConfirmation();
					m_pPlayer->Pet( 0 )->SetTarget( NULL );
				}
				// flush all events as we close
				FlushAll();
			}
		}
	}



	m_RenderTime = (float32)Game_Timer( KTimerGetAbsoluteTime );

    D3DVIEWPORT8 Viewport;
    Viewport.X = 0;
    Viewport.Y = 0;
    Viewport.Width = m_pSettings.GetSettings( KSetScreenWidth );
    Viewport.Height = m_pSettings.GetSettings( KSetScreenHeight );
    Viewport.MinZ = 0.0f;
    Viewport.MaxZ = 1.0f;

	if( m_pCamera->ViewType() != KViewFull )
	{
		Viewport.Width /= 2;
		if( m_pCamera->ViewType() == KViewRight )
		{
			Viewport.X = Viewport.Width;
		}
	}
	pD3DDevice->SetViewport( &Viewport );

	// set  our projection to the camera's projection matrix
	pD3DDevice->SetTransform( D3DTS_PROJECTION, &m_pCamera->ProjectionMatrix() );

	// turn on lighting for objects
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );


	m_pActiveLevel->Render( m_pPlayer->Position(),
							m_pLightManager,
							m_pCamera->ViewMatrix(),
							&m_pCamera->Frustum(),
							&m_ShadowViewMatrix,
							m_pRenderedShadow,
							m_pRenderedLight );

	m_pActiveLevel->RenderCharacters( m_pLightManager,
									  m_pCamera->ViewMatrix(),
									  &m_pCamera->Frustum() );
	m_pActiveLevel->RenderDetail( m_pPlayer->Position(),
								  m_pLightManager,
								  m_pCamera->ViewMatrix(),
								  &m_pCamera->Frustum(),
								  &m_ShadowViewMatrix,
								  m_pRenderedShadow,
								  m_pRenderedLight );


	// lighting back off
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	m_pActiveLevel->RenderArrowTrails( m_pCamera->ViewMatrix(),
									   &m_pCamera->Frustum() );

	m_pActiveLevel->RenderSFX( m_pCamera->ViewMatrix(),
							   &m_pCamera->Frustum() );

	m_pParticleManager->Render( pD3DDevice,
								m_pCamera->ViewMatrix(),
								&m_pCamera->Frustum() );

	m_pGlowManager->Render( pD3DDevice,
							m_pCamera->ViewMatrix(),
							&m_pCamera->Frustum() );


	m_pActiveLevel->RenderVisibility( m_pCamera->ViewMatrix() );


	// now render the blur layer on top of everything else
	if( m_pSettings.GetSettings( KSetMotionBlur ) )
	{
		// we render the blur layers differently depending
		// on whether this is a full or half-screen render
		switch( m_pCamera->ViewType() )
		{
		case KViewFull :
			m_pBlurLayer->RenderBlur( pD3DDevice );
			break;
		case KViewLeft :
			m_pBlurLayer->RenderBlurLeft( pD3DDevice );
			break;
		case KViewRight :
			m_pBlurLayer->RenderBlurRight( pD3DDevice );
			break;
		}
	}

	// reset the viewport to standard width
    Viewport.Width = m_pSettings.GetSettings( KSetScreenWidth );
	Viewport.X = 0;

	pD3DDevice->SetViewport( &Viewport );

	m_RenderTime = (float32)Game_Timer( KTimerGetAbsoluteTime ) - m_RenderTime;

	m_DebugText = "Render Time : " + FloatToString( m_RenderTime * 100 ) +
				  "\nEvent Time : " + FloatToString( m_EventTime * 100 ) +
				  "\nPatches : " + 
				  IntegerToString( m_pActiveLevel->PatchesRendered() ) + "\n" +
				  "Props : " + 
				  IntegerToString( m_pActiveLevel->PropsRendered() ) + "\n" +
				  "Item Seed : " + IntegerToString( m_ItemSeed ) +
				  m_pActiveLevel->DebugText();

} // CGameClient::Render()

void CGameClient::RenderBlurLayer( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_pSettings.GetSettings( KSetMotionBlur ) )
	{
		return;
	}

	if( !m_pBlurLayer->IsValid() )
	{
		return;
	}

	if( m_BlurTime < KBlurRate )
	{
		return;
	}
	if( m_pCamera->ViewType() == KViewCovered )
	{
		return;
	}

	while( m_BlurTime > KBlurRate )
	{
		m_BlurTime -= KBlurRate;
	}

	// turn off lighting
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

    D3DVIEWPORT8 Viewport;
    Viewport.X = 0;
    Viewport.Y = 0;
    Viewport.Width = 256;
    Viewport.Height = 256;
    Viewport.MinZ = 0.0f;
    Viewport.MaxZ = 1.0f;

	if( m_pCamera->ViewType() != KViewFull )
	{
		Viewport.Width /= 2;
		if( m_pCamera->ViewType() == KViewRight )
		{
			Viewport.X = 256;
		}
		pD3DDevice->SetViewport( &Viewport );
		// render our lights to the blur layer
		m_pBlurLayer->BeginRenderHalfScreen( pD3DDevice );
	}
	else
	{
		// render our lights to the blur layer
		m_pBlurLayer->BeginRender( pD3DDevice );
	}



	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );

	m_pActiveLevel->RenderGlowPass( m_pCamera->ViewMatrix()  );

	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );


	m_pParticleManager->RenderAdditive( pD3DDevice,
										m_pCamera->ViewMatrix(),
										&m_pCamera->Frustum() );

	m_pGlowManager->RenderAdditive( pD3DDevice,
									m_pCamera->ViewMatrix(),
									&m_pCamera->Frustum() );

	m_pActiveLevel->RenderVisibility( m_pCamera->ViewMatrix() );

	// reset the viewport to standard width
    Viewport.Width = 256;
	Viewport.X = 0;

	pD3DDevice->SetViewport( &Viewport );

	m_pBlurLayer->EndRender( pD3DDevice );

} // CGameClient::RenderBlurLayer()

void CGameClient::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
								 bool Orthographic )						// false by default
{
	if( !m_pSettings.GetSettings( KSetProjectedShadows ) )
	{
		return;
	}
	if( m_pSettings.GetSettings( KSetShadowDetail ) < 1 )
	{
		return;
	}
	// no shadow texture, no rendering
	if( m_pRenderedShadow == NULL )
	{
		return;
	}
	if( m_pCamera->ViewType() == KViewCovered )
	{
		return;
	}

	// projection matrix for the texture
	D3DXMATRIX ProjectionMatrix;

	float32 FOV		= ( float32 )( 90 * KDeg2Rad );

	D3DXMATRIX ViewOrientation;
	D3DXMatrixTranspose( &ViewOrientation, &ViewportMatrix );

	D3DXVECTOR3 ViewForward( ViewOrientation._31, ViewOrientation._32, ViewOrientation._33 );

	// first we render really close-in objects,
	// so we trim the farclip of the projection matrix

	if( Orthographic ||
		!m_pSettings.Capabilities().IsCapable( KCanDoShadowTextureProjection ))
	{

		if( m_pSettings.GetSettings( KSetShadowDetail ) == 0 )
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								125,
								125,
								.01f,
								2000 );	// far clip
		}
		else
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								50,
								50,
								.01f,
								2000 );	// far clip
		}
	}
	else
	{
		if( m_pSettings.GetSettings( KSetShadowDetail ) == 0 )
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										( float32 )( 120 * KDeg2Rad ), 
										1.0f, 
										.01f, 
										2000 );
		}
		else
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										FOV, 
										1.0f, 
										.01f, 
										2000 );
		}
	}

	// start rendering to our s texture
	m_pRenderedShadow->BeginRender();

	pD3DDevice->Clear( 0,
					   NULL,
					   D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,
					   0xffffff,
					   1.0f,
					   0 );
	

	// turn off lighting
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
	// no zwrite or ztest
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

	// set  our projection to the this projection matrix
	pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );


	m_ShadowFrustum.Update( ViewportMatrix,
							 ProjectionMatrix );

	pD3DDevice->SetTexture( 0, m_pBlack->GetTexture() );

	// we only do this if shadow detail is set up high enough
	if( m_pSettings.GetSettings( KSetShadowDetail ) > 2 )
	{
		m_pActiveLevel->RenderPropShadows( ViewportMatrix,
											&m_ShadowFrustum );

		m_pActiveLevel->RenderItemShadows( ViewportMatrix,
										&m_ShadowFrustum );
	}


	if( m_pSettings.GetSettings( KSetShadowDetail ) > 1 )
	{
		m_pActiveLevel->RenderCharacterShadows( ViewportMatrix,
												&m_ShadowFrustum );
	}
	else
	{
		m_pPlayer->RenderShadow( pD3DDevice, ViewportMatrix, &m_ShadowFrustum );
	}

	pD3DDevice->SetTexture( 0, m_pWhite->GetTexture() );

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );

	if( m_pActiveLevel->IsTorchlit() )
	{
		m_pActiveLevel->RenderTorchLight();
	}

	pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );

	m_ShadowFrustum.Update( ViewportMatrix,
							 ProjectionMatrix );

	if( m_pActiveLevel->IsTorchlit() )
	{
		m_pActiveLevel->RenderHardCharacterLights( ViewportMatrix, &m_ShadowFrustum );
	}

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	if( m_pActiveLevel->IsTorchlit() )
	{
		m_pActiveLevel->RenderPropLights( ViewportMatrix, &m_pCamera->Frustum() );
		m_pActiveLevel->RenderItemLights( ViewportMatrix, &m_pCamera->Frustum() );
	}

	if( m_pActiveLevel->IsTorchlit() )
	{
		m_pActiveLevel->RenderCharacterLights( ViewportMatrix, &m_ShadowFrustum );
	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// render any fading of shadows/lights
	m_pActiveLevel->RenderShadowFade();

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

	m_pRenderedShadow->EndRender();

	if( m_pActiveLevel->IsTorchlit() )
	{

		// start rendering to our s texture
		m_pRenderedLight->BeginRender();

		pD3DDevice->Clear( 0,
						   NULL,
						   D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,
						   0xffffff,
						   1.0f,
						   0 );


		// turn off lighting
		pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
		// no zwrite or ztest
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
		pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

		// set  our projection to the this projection matrix
		pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );

		pD3DDevice->SetTexture( 0, m_pWhite->GetTexture() );

		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );


		m_pActiveLevel->RenderTorchLight();

		pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );

		m_ShadowFrustum.Update( ViewportMatrix,
								 ProjectionMatrix );

		m_pActiveLevel->RenderHardCharacterLights( ViewportMatrix, &m_ShadowFrustum );

		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

		m_pActiveLevel->RenderPropLights( ViewportMatrix, &m_pCamera->Frustum() );
		m_pActiveLevel->RenderItemLights( ViewportMatrix, &m_pCamera->Frustum() );

		m_pActiveLevel->RenderCharacterLights( ViewportMatrix, &m_pCamera->Frustum() );

		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		// render any fading of shadows/lights
		m_pActiveLevel->RenderShadowFade();

		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );


		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
		pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

		m_pRenderedLight->EndRender();
	}
	

} // CGameClient::RenderShadows()

void CGameClient::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{

	m_pGameUI->Render( pD3DDevice,
					   *this,
					   m_pCamera->ViewMatrix(),
					   m_pCamera->ProjectionMatrix() );

} // CGameClient::RenderUI()

void CGameClient::AutoGetGold( void )
{
	if( m_pPlayer == NULL ||
		!m_pPlayer->Alive() ||
		m_pActiveLevel->RequestingLevelChange() )
	{
		return;
	}

	CLLNode<CItem>*	pItem	= m_pActiveLevel->OnscreenItems()->GetFirst();
	CLLNode<CItem>*	pNext;
	while ( pItem != NULL )
	{
		pNext = pItem->Next();
		if( pItem->Data()->Category() == KCategoryGold )
		{
			D3DXVECTOR3 Delta( pItem->Data()->Position() - m_pPlayer->Position() );
			Delta.y = 0;
			if( D3DXVec3Length( &Delta ) < KGetGoldProximity )
			{
				m_pPlayer->GetItem( pItem->Data(), *m_pActiveLevel );
			}
		}
		pItem = pNext;
	}

} // CGameClient::AutoGetGold()

void CGameClient::Click( void )
{
	if( !m_pPlayer->Alive() ||
		m_pGameUI->DraggingItem() ||
		m_pGameUI->Paused() ||
		m_pGameUI->ActionPaused() )
	{
		return;
	}

	m_HeldWhenInactive = kFalse;

	if( m_pPlayer->Fishing() )
	{
		return;
	}

	if( m_pMouseOverCharacter != NULL &&
		m_pMouseOverCharacter->Alignment() == KEvil )
	{
		if( !( m_pPlayer->PerformingAttack() &&
			   m_pPlayer->InStrikeRange() ) &&
			   !m_pPlayer->PerformingCast() )
		{
			m_pPlayer->SetTarget( m_pMouseOverCharacter );
			m_pPlayer->SetAIState( KAIHunt );
			m_ClickBroken = kFalse;
		}
	}
	else if( m_pMouseOverItem != NULL &&
			 ( m_pMouseOverItem->Takeable() ||
			   ( m_pMouseOverItem->IsContainer() &&
				 m_pMouseOverItem->CanInteract() ) ) )
	{
		m_ClickBroken = kFalse;
		// holding shift will cause the pet to pick up the item
		if( m_pMouseOverItem->Takeable() && 
			m_KeyHandler.KeyHeld( VK_SHIFT ) &&
			m_pPlayer->HasPets() &&
			m_pPlayer->Pet( 0 )->AvailableForCommand() )
		{
			m_pPlayer->Pet( 0 )->SetTarget( NULL );
			m_pPlayer->Pet( 0 )->SetTargetItem( m_pMouseOverItem );
			m_pPlayer->Pet( 0 )->SetAIState( KAIGetItem );
		}
		else
		{
			m_pPlayer->SetTarget( NULL );
			m_pPlayer->SetTargetItem( m_pMouseOverItem );
			m_pPlayer->SetAIState( KAIGetItem );
		}
	}
	else if( m_pMouseOverCharacter != NULL &&
		m_pMouseOverCharacter->Alignment() == KNeutral &&
		m_pMouseOverCharacter->IsMerchant() )
	{
		m_ClickBroken = kFalse;
		if( m_KeyHandler.KeyHeld( VK_SHIFT ) &&
			m_pPlayer->Pets() > 0 &&
			m_pPlayer->Pet( 0 )->AvailableForCommand() )
		{
			m_pPlayer->Pet( 0 )->SetTarget( m_pMouseOverCharacter );
			m_pPlayer->Pet( 0 )->SetAIState( KAIApproach );
		}
		else
		{
			m_pPlayer->SetTarget( m_pMouseOverCharacter );
			m_pPlayer->SetAIState( KAIApproach );
		}

	}
	else if( m_pMouseOverCharacter != NULL &&
		m_pMouseOverCharacter->Alignment() == KNeutral &&
		m_pMouseOverCharacter->IsQuestGiver() )
	{
		m_ClickBroken = kFalse;
		m_pPlayer->SetTarget( m_pMouseOverCharacter );
		m_pPlayer->SetAIState( KAIApproach );

	}
	else if( m_pMouseOverItem != NULL &&
			 m_pMouseOverItem->Operateable() )
	{
		m_ClickBroken = kFalse;
		if( m_KeyHandler.KeyHeld( VK_SHIFT ) &&
			m_pPlayer->Pets() > 0 &&
			m_pPlayer->Pet( 0 )->AvailableForCommand() &&
			( m_pMouseOverItem->Type() == KItemHealFountain || 
			  m_pMouseOverItem->Type() == KItemStaminaFountain ||
			  m_pMouseOverItem->Type() == KItemManaFountain ||
			  m_pMouseOverItem->Type() == KItemWellnessFountain ) )
		{
			m_pPlayer->Pet( 0 )->SetTarget( NULL );
			m_pPlayer->Pet( 0 )->SetTargetItem( m_pMouseOverItem );
			m_pPlayer->Pet( 0 )->SetAIState( KAIOperateItem );
		}
		else
		{
			m_pPlayer->SetTarget( NULL );
			m_pPlayer->SetTargetItem( m_pMouseOverItem );
			m_pPlayer->SetAIState( KAIOperateItem );
		}
	}
	else 
	{
		m_pMouseOverCharacter = NULL;
		m_ClickBroken = kFalse;
		m_pPlayer->SetTarget( NULL );
		m_pPlayer->SetTargetItem( NULL );
		m_pPlayer->SetAIState( KAIInactive );
		
		WalkToMouse();

		// let us know that we weren't doing anything important when
		// we started holding down the mouse button
		m_HeldWhenInactive = kTrue;

	}
} // CGameClient::Click()

void CGameClient::Hold( void )
{
	if( !m_pPlayer->Alive() )
	{
		return;
	}

	if( !m_pPlayer->PerformingAttack() &&
		!m_pPlayer->PerformingCast() &&
		m_ClickBroken )
	{
		UpdateMouseOver();
		if( m_pMouseOverCharacter != NULL &&
			m_pMouseOverCharacter->Alignment() == KEvil )
		{
			m_pPlayer->SetTarget( m_pMouseOverCharacter );
			m_pPlayer->SetAIState( KAIHunt );
			m_ClickBroken = kFalse;
			return;
		}
	}
	else if( m_pPlayer->PerformingAttack() &&
		m_pPlayer->Target() == NULL &&
		!m_pPlayer->PerformingCast() )
	{
		UpdateMouseOver();
		if( m_pMouseOverCharacter != NULL &&
			m_pMouseOverCharacter->Alignment() == KEvil )
		{
			m_pPlayer->SetTarget( m_pMouseOverCharacter );
			m_pPlayer->SetAIState( KAIHunt );
			return;
		}
		m_pMouseOverCharacter = NULL;
	}
	if( m_pPlayer->PerformingAttack() &&
		m_pPlayer->Target() != NULL &&
		!m_ClickBroken )
	{
		m_pMouseOverCharacter = m_pPlayer->Target();
	}
	if( m_pPlayer->AIState() == KAIInactive ||
		m_pPlayer->AIState() == KAIWander )
	{
		if( m_pPlayer->Target() != NULL &&
			m_pPlayer->Target()->Alignment() == KEvil &&
			!m_ClickBroken )
		{
			m_pPlayer->SetAIState( KAIAttack );
		}
		else if( m_pPlayer->TargetItem() != NULL )
		{
			// do nothing when item is targeted
		}
		else if( m_HeldWhenInactive )
		{
			WalkToMouse();
		}
	}
} // CGameClient::Hold()

void CGameClient::WalkToMouse( void )
{
	D3DXVECTOR3 MoveTarget;
	FindWorldLocation( MoveTarget );

	if( MoveTarget == m_pPlayer->Position() &&
		!m_pPlayer->PerformingCast() )
	{
		m_pPlayer->SetDestination( *m_pActiveLevel, MoveTarget.x, MoveTarget.z );
		m_TimeToRePath = .1f;
	}
	if( m_TimeToRePath <= 0 &&
		!m_pPlayer->PerformingCast() )
	{
		m_pPlayer->SetDestination( *m_pActiveLevel, MoveTarget.x, MoveTarget.z );
		m_TimeToRePath = .1f;
	}
} // CGameClient::WalkToMouse()

void CGameClient::FindWorldLocation( D3DXVECTOR3& Location )	// location to fill
{
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	if( m_pCamera->ViewType() == KViewLeft )
	{
		MousePosition.x *= 2;
		if( MousePosition.x > 1024 )
		{
			Location = m_pPlayer->Position();
			m_pMouseOverCharacter = NULL;
			m_pMouseOverItem = NULL;
			return;
		}
	}
	else if( m_pCamera->ViewType() == KViewRight )
	{
		MousePosition.x *= 2;
		MousePosition.x -= 1024;
		if( MousePosition.x < 0 )
		{
			Location = m_pPlayer->Position();
			m_pMouseOverCharacter = NULL;
			m_pMouseOverItem = NULL;
			return;
		}
	}

	float32 WorldspaceX( (float32)MousePosition.x -512.0f );
	float32 WorldspaceY( ( (float32)MousePosition.y - 384.0f ) * -1.0f );
	WorldspaceX /= 1024.0f;
	WorldspaceY /= 1024.0f;

	D3DXVECTOR3 Normal( 0, 1, 0 );
	D3DXVECTOR3 Ray( WorldspaceX, WorldspaceY, 1.0f );
		
	D3DXVec3TransformNormal( &Ray, &Ray, &m_pCamera->Orientation() );
		
	D3DXVec3Normalize( &Ray, &Ray );
	float32 Distance( DistanceToPlane( m_pCamera->Position(), Ray, Normal, 0 ) );
	Ray *= Distance;
	Ray += m_pCamera->Position();

	Location = Ray;
} // CGameClient::FindWorldLocation()

void CGameClient::UpdateMouseOver( void )
{
	if( !m_pPlayer->Alive() ||
		m_pGameUI->DraggingItem() ||
		m_pGameUI->Paused() ||
		m_pGameUI->ActionPaused() )
	{
		m_pMouseOverCharacter = NULL;
		m_pMouseOverItem = NULL;
		return;
	}
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( m_pCamera->ViewType() == KViewLeft )
	{
		MousePosition.x *= 2;
		if( MousePosition.x > 1024 )
		{
			m_pMouseOverCharacter = NULL;
			m_pMouseOverItem = NULL;
			return;
		}
	}
	else if( m_pCamera->ViewType() == KViewRight )
	{
		MousePosition.x *= 2;
		MousePosition.x -= 1024;
		if( MousePosition.x < 0 )
		{
			m_pMouseOverCharacter = NULL;
			m_pMouseOverItem = NULL;
			return;
		}
	}


	float32 WorldspaceX( (float32)MousePosition.x - 512.0f );
	float32 WorldspaceY( ( (float32)MousePosition.y - 384.0f ) * -1.0f );
	WorldspaceX /= 512.0f;
	WorldspaceY /= 384.0f;

	m_pMouseOverCharacter = m_pActiveLevel->FindCharacterAtScreenCoordinates( m_pCamera->ViewMatrix(),
																			  m_pCamera->ProjectionMatrix(),
																			  WorldspaceX,
																			  WorldspaceY,
																			  m_pPlayer );

	if( m_pMouseOverCharacter == NULL ||
		( m_pMouseOverCharacter->HasMaster() && 
		  m_pMouseOverCharacter->Master() == m_pPlayer ) ||
		m_pMouseOverCharacter->Alignment() == KNeutral )
	{
		m_pMouseOverItem = m_pActiveLevel->FindItemAtScreenCoordinates( m_pCamera->ViewMatrix(),
																	    m_pCamera->ProjectionMatrix(),
																		WorldspaceX,
																		WorldspaceY );
	}
	else
	{
		m_pMouseOverItem = NULL;
	}
} // CGameClient::UpdateMouseOver()

void CGameClient::VerifyMouseOvers( void )
{
	if( m_pGameUI->Paused() ||
		m_pGameUI->ActionPaused() )
	{
		m_pMouseOverItem = NULL;
		m_pMouseOverCharacter = NULL;
	}
	if( m_pPlayer->Fishing() )
	{
		m_pMouseOverItem = NULL;
		m_pMouseOverCharacter = NULL;
	}

	// verify that mouseover items still exist
	if( m_pMouseOverItem != NULL &&
		!m_pActiveLevel->ItemExists( m_pMouseOverItem ) )
	{
		m_pMouseOverItem = NULL;
	}

	if( m_pMouseOverItem != NULL &&
		m_pMouseOverItem->IsContainer() &&
		!m_pMouseOverItem->CanInteract() )
	{
		m_pMouseOverItem = NULL;
	}

	// verify that mouseover characters still exist
	if( m_pMouseOverCharacter != NULL &&
		!m_pActiveLevel->CharacterExists( m_pMouseOverCharacter ) )
	{
		m_pMouseOverCharacter = NULL;
	}

	// and are still alive
	if( m_pMouseOverCharacter != NULL &&
		!m_pMouseOverCharacter->Alive() )
	{
		m_pMouseOverCharacter = NULL;
	}

	// if the player is dead, no mouseover either
	if( !m_pPlayer->Alive() )
	{
		m_pMouseOverItem = NULL;
		m_pMouseOverCharacter = NULL;
	}

	m_pGameUI->SetPlayerCharacter( m_pPlayer );
	m_pGameUI->SetTargetCharacter( m_pMouseOverCharacter );
	m_pGameUI->SetTargetItem( m_pMouseOverItem );

} // CGameClient::VerifyMouseOvers()

void CGameClient::DestroyLevel( void )
{
	DELETE_SAFELY( m_pActiveLevel );
	m_pPlayer = NULL;

	m_pPositionalAudioManager->Clear();
	m_pParticleManager->Clear();
	m_pGlowManager->Clear();
} // CGameClient::DestroyLevel()

void CGameClient::GenerateLevel( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								 int32 LevelDepth,					// dungeon level depth
								 bool Descending,					// have we just gone down?
								 bool IsTeleport,					// is this a teleport?
								 bool IsFateRespawn )				// is a respawn from fate
{
	m_pGameUI->Reset( pD3DDevice );

	CLList<CSFX>*		pTempSFXList;
	pTempSFXList = new CLList< CSFX >();

	// carry over any FX items that apply to the character
	if( m_pActiveLevel != NULL )
	{
		// make sure any dying characters finish doing so
		m_pActiveLevel->UpdateCharacterDeaths();

		CLLNode<CSFX>*	pSFX	= m_pActiveLevel->SFXList()->GetFirst();
		CLLNode<CSFX>*	pNext;
		while ( pSFX != NULL )
		{
			pNext = pSFX->Next();

			bool CarryOver( kFalse );
			// if we find an fx that needs to move to the next level, we yank it out so it isn't deleted
			if( pSFX->Data()->Target() == m_pPlayer )
			{
				CarryOver = kTrue;
			}
			for( uint32 i = 0; i < m_pPlayer->Pets(); i++ )
			{
				if( pSFX->Data()->Target() == m_pPlayer->Pet( i ) )
				{
					CarryOver = kTrue;
				}
			}

			if( pSFX->Data()->Type() == KSFXProjectileLocation ||
				pSFX->Data()->Type() == KSFXProjectileCharacter ||
				pSFX->Data()->Type() == KSFXProjectileLocationErratic ||
				pSFX->Data()->Type() == KSFXProjectileCharacterErratic )
			{
				CarryOver = kFalse;
			}

			// keep this in a temporary list to carry over
			if( CarryOver )
			{
				bool ClearOwner( kTrue );
				if( pSFX->Data()->Owner() == m_pPlayer )
				{
					ClearOwner = kFalse;
				}
				for( uint32 i = 0; i < m_pPlayer->Pets(); i++ )
				{
					if( pSFX->Data()->Owner() == m_pPlayer->Pet( i ) )
					{
						ClearOwner = kFalse;
					}
				}
				if( ClearOwner )
				{
					pSFX->Data()->SetOwner( NULL );
				}
				pSFX->Data()->ClearParticles();
				pTempSFXList->Queue( pSFX->Data() );
				m_pActiveLevel->SFXList()->RemoveNoDelete( pSFX );

			}
			pSFX = pNext;
		}
	}

	if( LevelDepth < 0 )
	{
		LevelDepth = 0;
	}

	if( m_pActiveLevel != NULL &&
		m_pPlayer != NULL )
	{
		// buffer our respawn of the level we were just on by 5 minutes, if required
		if( m_pPlayer->ExistenceTime() > 
			m_pActiveLevel->LevelHistory()->LastPopulationTime() + KMaximumLevelRespawnTime - 300 )
		{
			m_pActiveLevel->LevelHistory()->SetLastPopulationTime( m_pPlayer->ExistenceTime() - KMaximumLevelRespawnTime + 300 );
		}
	}

	m_pLoadingMenu.SetProgressPercentage( 5 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );
	m_pLightManager->SetSceneAmbient( .25f, .25f, .25f );
	m_pLightManager->SetSceneSpecular( .5f, .5f, .5f );

	m_pPositionalAudioManager->Clear();
	bool NewlyLoadedCharacter( kFalse );
	bool NewlyCreatedCharacter( kFalse );
	std::vector< CCharacter* > pPets;
	std::vector< CCharacterSaveInstance* > pPetDescriptions;
	if( m_pPlayer != NULL && m_pActiveLevel != NULL )
	{
		m_pPlayer->RememberDiscovery( *m_pActiveLevel );
		m_pActiveLevel->RemoveCharacter( m_pPlayer );
		m_pPlayer->ClearParticles();
		for( uint32 i = 0; i < m_pPlayer->Pets(); i++ )
		{
			pPets.push_back( m_pPlayer->Pet( i ) );
		}
		m_pPlayer->RemovePets();
		m_pPlayer->ClearState();
		for( int32 i = 0; i < pPets.size(); i++ )
		{
			m_pActiveLevel->RemoveCharacter( pPets[i] );
			if( pPets[i]->AIState() != KAIInTown )
			{
				pPets[i]->ClearState();
			}
			pPets[i]->ClearParticles();
			pPets[i]->SetMaster( NULL );
		}

	}



	DELETE_SAFELY( m_pActiveLevel );

	m_pParticleManager->Clear();
	m_pGlowManager->Clear();

	m_pLoadingMenu.SetProgressPercentage( 10 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

// create or load a player object
	if( m_pPlayer == NULL )
	{
		NewlyLoadedCharacter = kTrue;

		//std::string LoadPath = AssembleAbsolutePath( GetLocalPath(), m_SaveGamePath );
		std::string LoadPath = m_SaveGamePath;
		if( m_SaveGamePath.length() != 0 && FileExists( LoadPath ) )
		{
			LoadCharacter( pD3DDevice, LoadPath, pPets, pPetDescriptions );

			// get the last level the player was on
			LevelDepth = m_pPlayer->LastDungeonLevel();
		}
		else
		{
			m_DungeonSeed = time( NULL );
			m_pPlayer =  new CPlayer( pD3DDevice,
									m_pRefManager,
									m_pSettings,
									*m_pSpellManifest,
									*m_pParticleManager,
									*m_pGlowManager,
									*m_pPositionalAudioManager,
									m_NewCharacterGender,
									m_NewCharacterHeadIndex,
									m_NewCharacterHairIndex );
			m_pPlayer->SetName( m_NewCharacterName );
			m_pPlayer->SetLineage( m_NewCharacterLineage );
			NewlyCreatedCharacter = kTrue;
			m_pPlayer->GiveMasterQuest( *m_pMasterQuest );
			m_pPlayer->SetDifficulty( m_Difficulty );
			m_pTips->ClearState();
		}
		
	}

	m_pPlayer->SetAllowDropInventory( kFalse );

	if( NewlyCreatedCharacter )
	{
		m_pGameUI->SetHelpButtonVisible( kTrue );
	}

	srand( m_DungeonSeed * 500 + LevelDepth );

	uint32 LevelTemplate = RandomIntegerBetween( 0, m_pTemplateManifest->Templates() - 1 );

	// first level is always the first dungeon type
	if( LevelDepth == 1 )
	{
		LevelTemplate = 0;
	}

	m_pLoadingMenu.SetProgressPercentage( 15 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	m_pActiveLevel = new CLevel( pD3DDevice,
								 m_pRefManager,
								 m_pSettings,
								 *m_pQuestDescriptions,
								 *m_pCharacterManifest,
								 *m_pItemManifest,
								 *m_pSpellManifest,
								 *m_pParticleManager,
								 *m_pGlowManager,
								 *m_pPositionalAudioManager,
								 *m_pNameGenerator,
								 *m_pTips,
								 Difficulty(),
								 GetLocalPath() + m_pTemplateManifest->TemplatePath( LevelTemplate ) );

	m_pLoadingMenu.SetProgressPercentage( 20 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	m_pActiveLevel->Create( m_pLoadingMenu,
							m_DungeonSeed,
							LevelDepth );

	m_pLoadingMenu.SetProgressPercentage( 50 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	m_ItemSeed = time( NULL );
	//m_ItemSeed = 1097108044;
	// more randomization for items - not based off the seed
	srand( m_ItemSeed );

	D3DXVECTOR3 StartingPosition;
	
	if( NewlyCreatedCharacter )
	{
		StartingPosition = m_pActiveLevel->StartPosition();
	}
	else if( !NewlyLoadedCharacter )
	{
		if( IsFateRespawn )
		{
			if( LevelDepth == 0 )
			{
				StartingPosition = m_pActiveLevel->PositionNearStairsUp();
			}
			else
			{
				StartingPosition = m_pActiveLevel->RandomOpenPosition();
			}
		}
		else if( IsTeleport )
		{
			if( LevelDepth == 0 )
			{
				StartingPosition = m_pActiveLevel->PositionNearStairsUp();
			}
			else
			{
				StartingPosition = m_pPlayer->PortalPosition();
			}
		}
		else
		{
			if( Descending && LevelDepth != 0 )
			{
				StartingPosition = m_pActiveLevel->PositionNearStairsUp();
				m_pActiveLevel->SetVisibility( StartingPosition, kTrue );
			}
			else
			{
				StartingPosition = m_pActiveLevel->PositionNearStairsDown();
			}
		}
		StartingPosition = m_pActiveLevel->RandomOpenPosition( StartingPosition, 12 );
	}
	else
	{
		StartingPosition = m_pPlayer->Position();
	}

	m_pActiveLevel->AddCharacter( m_pPlayer, StartingPosition );
	m_pPlayer->FindParticles();

	m_pPlayer->SetMaximumTreeDepth( 800 );

	m_pLoadingMenu.SetProgressPercentage( 53 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	if( NewlyCreatedCharacter )
	{
		m_pPlayer->GiveGold( 250 );
		// try and give them some starting equipment
		// previously these keyed off the English item names, now they work with localized versions
		CItemDescription* pItemDescription = m_pItemManifest->ItemDescription( LoadLocalizedString(1138) );
		if( pItemDescription != NULL )
		{
			CItem* pItem =  new CItem( pD3DDevice,
										m_pRefManager,
										m_pSettings,
										*m_pParticleManager,
										*m_pGlowManager,
										*m_pPositionalAudioManager,
										*pItemDescription );
			m_pPlayer->GiveItem( *m_pActiveLevel, pItem );
			m_pPlayer->Inventory()->EquipBest( *m_pActiveLevel );
		}

		// two health potions to help out in the beginning
		pItemDescription = m_pItemManifest->ItemDescription( LoadLocalizedString(1139) );
		if( pItemDescription != NULL )
		{
			for( uint32 i = 0; i < 3; i++ )
			{
				CItem* pItem =  new CItem( pD3DDevice,
										m_pRefManager,
										m_pSettings,
										*m_pParticleManager,
										*m_pGlowManager,
										*m_pPositionalAudioManager,
										*pItemDescription );
				m_pPlayer->GiveItem( *m_pActiveLevel, pItem );
			}
		}
		pItemDescription = m_pItemManifest->ItemDescription( LoadLocalizedString(1140) );
		if( pItemDescription != NULL )
		{
			CItem* pItem =  new CItem( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   *m_pParticleManager,
									   *m_pGlowManager,
									   *m_pPositionalAudioManager,
									   *pItemDescription );
			m_pPlayer->GiveItem( *m_pActiveLevel, pItem );
		}

		pItemDescription = m_pItemManifest->ItemDescription( LoadLocalizedString(1137) );
		if( pItemDescription != NULL )
		{
			CItem* pItem =  new CItem( pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   *m_pParticleManager,
									   *m_pGlowManager,
									   *m_pPositionalAudioManager,
									   *pItemDescription );
			m_pPlayer->GiveItem( *m_pActiveLevel, pItem );
		}

		// this is an ancestor character, and gets some perks
		if( m_NewCharacterLineage != NULL &&
			m_pHeirloomItem != NULL )
		{
			int32 BaseFame = 1 + m_NewCharacterLineage;
			if( BaseFame > 10 )
			{
				BaseFame = 10;
			}
			m_pPlayer->LevelUp();
			m_pPlayer->AwardFame( *m_pActiveLevel, NULL, KFameGate[ BaseFame ] );
			pItemDescription = m_pItemManifest->ItemDescription( m_pHeirloomItem->m_BaseName );
			if( pItemDescription != NULL )
			{
				CItem* pItem =  new CItem( pD3DDevice,
										m_pRefManager,
										m_pSettings,
										*m_pParticleManager,
										*m_pGlowManager,
										*m_pPositionalAudioManager,
										*pItemDescription );

				pItem->ApplyInstance( pD3DDevice,
									  *m_pHeirloomItem,
									  *m_pItemManifest );
				pItem->DetermineName();
				m_pPlayer->GiveItem( *m_pActiveLevel, pItem );
			}
		}
	}

	m_pLoadingMenu.SetProgressPercentage( 55 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	m_pActiveLevel->CreateRandomizationTables( m_pPlayer->Level() );

	bool RemembersLevel = m_pPlayer->RemembersLevel( LevelDepth );
	if( RemembersLevel )
	{
		m_pActiveLevel->LevelHistory()->Copy( *m_pPlayer->GetLevelHistory( LevelDepth ) );
	}

	if( IsTeleport && LevelDepth > 0 )
	{
		m_pPlayer->SetHasOpenPortal( kFalse );
	}

	m_pPlayer->CreatePortals( *m_pActiveLevel );

	m_pLoadingMenu.SetProgressPercentage( 60 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

// add a pet for the player
	if( NewlyCreatedCharacter )
	{
		CCharacterDescription* pDescription;
		if( m_NewPetIsDog )
		{
			pDescription = m_pCharacterManifest->CharacterDescription( "Terrier" ); // doesn't appear to be displayable text, not localizing
		}
		else
		{
			pDescription = m_pCharacterManifest->CharacterDescription( LoadLocalizedString(1141) ); // "Cat" causes a game crash in localized versions
		}
		if( pDescription != NULL )
		{
			CCharacter* pNewPet =	m_pActiveLevel->AddMonster( *pDescription,
																m_pActiveLevel->RandomOpenPosition( m_pPlayer->Position(), 12 ) );
			pNewPet->SetName( m_NewCharacterPetName );

			pNewPet->SetInvincible( kTrue );

			m_pPlayer->AddPet( pNewPet );
			pNewPet->SetMaximumTreeDepth( 800 );
			pNewPet->CalculateMaxHP();
			pNewPet->ModifyHP( 1000 );

		}
	}
	else if( pPets.size() > 0 )
	{
		for( uint32 i = 0; i < pPets.size(); i++ )
		{
			if( pPetDescriptions.size() > 0 )
			{
				pPets[i]->ApplyInstance( pD3DDevice, m_pActiveLevel, *m_pCharacterManifest, *pPetDescriptions[i], *m_pItemManifest );
			}

			m_pActiveLevel->AddCharacter( pPets[i],
										  m_pActiveLevel->RandomOpenPosition( m_pPlayer->Position(), 5 ) );
			pPets[i]->FindParticles();

			if( i == 0 )
			{
				pPets[i]->SetInvincible( kTrue );
				pPets[i]->SetMaximumTreeDepth( 800 );
			}

			m_pPlayer->AddPet( pPets[i] );
		}
	}

	m_pLoadingMenu.SetProgressPercentage( 70 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	for( uint32 i = 0; i < pPetDescriptions.size(); i++ )
	{
		DELETE_SAFELY( pPetDescriptions[i] );
	}
	pPetDescriptions.clear();


	m_pLoadingMenu.SetProgressPercentage( 75 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	if( !RemembersLevel )
	{
		m_pActiveLevel->GenerateRandomItems();

		m_pActiveLevel->GenerateRandomMonsters();

	}
	else
	{
		CLevelHistory* pHistory = m_pPlayer->GetLevelHistory( LevelDepth );
		std::vector< CCharacter* > pCharacterList;
		for( uint32 i = 0; i < pHistory->Characters(); i++ )
		{
			CCharacterSaveInstance* pInstance = pHistory->GetCharacter( i );

			CCharacterDescription* pDescription = m_pCharacterManifest->CharacterDescription( pHistory->GetCharacter( i )->m_OriginalTemplateName );
			if( pDescription != NULL )
			{
				CCharacter* pMonster = m_pActiveLevel->AddMonster( *pDescription,
																	pInstance->m_Position );
				pMonster->ApplyInstance( pD3DDevice,
										 m_pActiveLevel,
										 *m_pCharacterManifest,
										 *pInstance,
										 *m_pItemManifest );

				// re parent pets
				if( pInstance->m_MasterIndex >= 0 )
				{
					pCharacterList[pInstance->m_MasterIndex]->AddPet( pMonster );
				}
				pCharacterList.push_back( pMonster );
			}
			else
			{
				pCharacterList.push_back( NULL );
			}
		}

		for( int32 i = 0; i < pHistory->Items(); i++ )
		{
			CItemSaveInstance* pInstance = pHistory->GetItem( i );
			if( pInstance->m_BaseName == "Gold" )
			{
				m_pActiveLevel->AddGold( pInstance->m_Value,
										 pInstance->m_Position );
			}
			else
			{
				CItemDescription* pDescription = m_pItemManifest->ItemDescription( pHistory->GetItem( i )->m_BaseName );
				if( pDescription != NULL )
				{
					CItem* pItem = m_pActiveLevel->AddItem( *pDescription,
															pInstance->m_Position );
					pItem->ApplyInstance( pD3DDevice,
										*pInstance,
										*m_pItemManifest );
					pItem->DropToGround( *m_pActiveLevel );
				}
			}
		}

	}

	m_pActiveLevel->FindCharacters( m_pPlayer );

	m_pLoadingMenu.SetProgressPercentage( 80 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	// check all player quests

	m_pGameUI->SetPlayerCharacter( m_pPlayer );
	m_pGameUI->SetTargetCharacter( NULL );
	m_pGameUI->SetTargetItem( NULL );
	m_pMouseOverCharacter = NULL;
	m_pMouseOverItem = NULL;


	m_pLoadingMenu.SetProgressPercentage( 85 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	m_pActiveLevel->UpdateLevelWideVisibility();

	m_pLoadingMenu.SetProgressPercentage( 90 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	m_pRefManager.RemoveUnused();

	m_pLoadingMenu.SetProgressPercentage( 95 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	SpawnQuests( RemembersLevel );

	SaveCharacter();

	m_pLoadingMenu.SetProgressPercentage( 100 );
	m_pLoadingMenu.RenderUI( pD3DDevice, kTrue );

	// copy any carried-over effects to the new level
	CLLNode<CSFX>*	pSFX	= pTempSFXList->GetFirst();
	CLLNode<CSFX>*	pNext;
	while ( pSFX != NULL )
	{
		pNext = pSFX->Next();

		m_pActiveLevel->SFXList()->Queue( pSFX->Data() );
		pSFX->Data()->FindParticles();
		pSFX = pNext;
	}
	pTempSFXList->Clear();
	DELETE_SAFELY( pTempSFXList );
	StartMusic();

	if( IsFateRespawn )
	{
		CSFXDescription* pSFX = new CSFXDescription( KSFXFollowTarget,
												     "SPELLS\\EFFECTS\\HEAL\\heal.ams",
												     -1,
												     kFalse,
													 0,
													 0,
													 kFalse,
													 kFalse );
												   
		m_pActiveLevel->AddSFX( *pSFX,
								m_pPlayer,
								m_pPlayer,
								m_pPlayer->Position(),
								m_pPlayer->Position() );
		DELETE_SAFELY( pSFX );
	}

	if( NewlyCreatedCharacter &&
		m_pSettings.GetSettings( KSetShowTips ) &&
		!m_pTips->TipSeen( KTipWelcome ) )
	{
		m_pGameUI->OpenTip( pD3DDevice, m_pTips->ContextTip( KTipWelcome ) );
		m_pTips->SetTipSeen( KTipWelcome, kTrue );
	}
	else if( m_pTips->Tips() > 0 &&
			 m_pSettings.GetSettings( KSetShowTips ) )
	{
		m_pGameUI->OpenTip( pD3DDevice, m_pTips->RandomTip() );
	}

	if( NewlyCreatedCharacter )
	{
		m_pPlayer->PlayHelpSample( KSoundHelpJourneyBegin, kTrue );
	}

} // CGameClient::GenerateLevel()

void CGameClient::StopMusic( void )
{
	m_pSoundManager.StopMusic();
} // CGameClient::StopMusic()

void CGameClient::StartMusic( void )
{
	if( m_pActiveLevel == NULL )
	{
		return;
	}

	if( m_pActiveLevel->LevelDepth() == 0 )
	{
		std::vector< std::string > TownTracks;
		for( uint32 i = 1; i <= 100; i++ )
		{
			std::string Path = AssembleAbsolutePath( GetLocalPath(), "MUSIC\\town" + IntegerToString( i ) + ".ogg" );			
			// low quality version doesn't have the secondary music files
			if( FileExists( Path ) )
			{
				TownTracks.push_back( Path );
			}
			else
			{
				Path = AssembleAbsolutePath( GetLocalPath(), "MUSIC\\town" + IntegerToString( i ) + ".mp3" );			
				// low quality version doesn't have the secondary music files
				if( FileExists( Path ) )
				{
					TownTracks.push_back( Path );
				}
			}
		}

		m_pSoundManager.Update();
		if( TownTracks.size() > 0 )
		{
			uint32 Track = RandomIntegerBetween( 0, TownTracks.size() );
			if( Track >= TownTracks.size() )
			{
				Track = 0;
			}
			m_pSoundManager.StartMusic( TownTracks[Track], kTrue );
		}
	}
	else
	{
		std::vector< std::string > DungeonTracks;
		for( uint32 i = 1; i <= 100; i++ )
		{
			std::string Path = AssembleAbsolutePath( GetLocalPath(), "MUSIC\\dungeon" + IntegerToString( i ) + ".ogg" );			
			// low quality version doesn't have the secondary music files
			if( FileExists( Path ) )
			{
				DungeonTracks.push_back( Path );
			}
			else
			{
				Path = AssembleAbsolutePath( GetLocalPath(), "MUSIC\\dungeon" + IntegerToString( i ) + ".mp3" );			
				// low quality version doesn't have the secondary music files
				if( FileExists( Path ) )
				{
					DungeonTracks.push_back( Path );
				}
			}
		}

		m_pSoundManager.Update();
		if( DungeonTracks.size() > 0 )
		{
			uint32 Track = RandomIntegerBetween( 0, DungeonTracks.size() );
			if( Track >= DungeonTracks.size() )
			{
				Track = 0;
			}
			m_pSoundManager.StartMusic( DungeonTracks[Track], kTrue );
		}
	}

} // CGameClient::StartMusic()

void CGameClient::SpawnQuests( bool RemembersLevel )	// player knows this level already?
{
	bool SpawnedQuest( kFalse );
	for( uint32 i = 0; i < m_pPlayer->Quests(); i++ )
	{
		if( ( !m_pPlayer->Quest( i )->Spawned() ||
			  !RemembersLevel ) &&
			  !m_pPlayer->Quest( i )->Completed() &&
			m_pPlayer->Quest( i )->Level() == m_pActiveLevel->LevelDepth() )
		{
			m_pPlayer->Quest( i )->Spawn( *m_pActiveLevel );
			SpawnedQuest = kTrue;
		}
	}

	if( m_pPlayer->MasterQuest() != NULL )
	{
		if( ( !m_pPlayer->MasterQuest()->Spawned() ||
			!RemembersLevel ) &&
			!m_pPlayer->MasterQuest()->Completed() &&
			m_pPlayer->MasterQuest()->Level() == m_pActiveLevel->LevelDepth() )
		{
			m_pPlayer->MasterQuest()->Spawn( *m_pActiveLevel );
			SpawnedQuest = kTrue;
		}
	}

	// if we spawned a quest on a level we've already been too,
	// make sure to fill in any monsters so that it isn't boring.
	if( SpawnedQuest && 
		m_pPlayer->RemembersLevel( m_pActiveLevel->LevelDepth() ) )
	{
		m_pActiveLevel->GenerateRandomMonsters();
	}

} // CGameClient::SpawnQuests()

void CGameClient::UpdateQuests( LPDIRECT3DDEVICE8 pD3DDevice )				// direct3d device
{
	if( m_pPlayer->AIState() == KAIReceiveQuest )
	{
		if( m_pPlayer->Target()->Quests() == 0 )
		{
			m_pGameUI->OpenMessage( pD3DDevice,
									LoadLocalizedString(207), LoadLocalizedString(185) );
			m_pPlayer->SetAIState( KAIInactive );
			m_pPlayer->SetTarget( NULL );
		}
		else if( !m_pPlayer->QuestsFull() )
		{
			m_pGameUI->OpenConfirmation( pD3DDevice, 
										 m_pPlayer->Target()->Quest( 0 )->Description(),
										 LoadLocalizedString(208),
										 KButtonAcceptQuest,
										 LoadLocalizedString(209),
										 KButtonDeclineQuest );
			return;
		}
		else
		{
			m_pGameUI->OpenMessage( pD3DDevice,
									LoadLocalizedString(210), LoadLocalizedString(185) );
			m_pPlayer->SetAIState( KAIInactive );
			m_pPlayer->SetTarget( NULL );
		}
	}
	for( uint32 i = 0; i < m_pPlayer->Quests(); i++ )
	{
		if( m_pPlayer->Quest( i )->Completed() &&
			!m_pPlayer->Quest( i )->CompletionNotified() )
		{
			m_pPlayer->PlayHelpSample( KSoundHelpQuestComplete, kTrue );
			m_pGameUI->OpenMessage( pD3DDevice, LoadLocalizedString1Variable(211, m_pPlayer->Quest( i )->GiverName() )  );
			m_pPlayer->Quest( i )->SetCompletionNotified( kTrue );
			return;
		}
		if( m_pPlayer->AIState() == KAICollectReward )
		{
			if( m_pPlayer->Target() != NULL )
			{
				if( StringUpper( m_pPlayer->Target()->Name() ) ==
					StringUpper( m_pPlayer->Quest( i )->GiverName() ) )
				{
					bool QuestSuccessful( m_pPlayer->Quest( i )->Completed() );
					if( QuestSuccessful &&
						m_pPlayer->Quest( i )->QuestItem() != NULL )
					{
						QuestSuccessful = kFalse;
						// item based quests check for the items, then remove them
						if( m_pPlayer->Inventory()->HasItems( *m_pPlayer->Quest( i )->QuestItem(),
															  m_pPlayer->Quest( i )->QuestItems() ) )
						{
							for( uint32 j = 0; j < m_pPlayer->Quest( i )->QuestItems(); j++ )
							{
								CItem* pItem = m_pPlayer->Inventory()->FindItem( *m_pPlayer->Quest( i )->QuestItem() );
								if( pItem != NULL )
								{
									QuestSuccessful = kTrue;
									m_pPlayer->Inventory()->RemoveItem( pItem );
									DELETE_SAFELY( pItem );
								}
							}
						}
						if( !QuestSuccessful )
						{
							if( m_pPlayer->Pets() > 0 )
							{
								// item based quests check for the items, then remove them
								if( m_pPlayer->Pet( 0 )->Inventory()->HasItems( *m_pPlayer->Quest( i )->QuestItem(),
																				m_pPlayer->Quest( i )->QuestItems() ) )
								{
									for( uint32 j = 0; j < m_pPlayer->Quest( i )->QuestItems(); j++ )
									{
										CItem* pItem = m_pPlayer->Pet( 0 )->Inventory()->FindItem( *m_pPlayer->Quest( i )->QuestItem() );
										if( pItem != NULL )
										{
											QuestSuccessful = kTrue;
											m_pPlayer->Pet( 0 )->Inventory()->RemoveItem( pItem );
											DELETE_SAFELY( pItem );
										}
									}
								}
							}
						}
					}
					if( QuestSuccessful )
					{
						FSOUND_PlaySound( FSOUND_FREE, m_pRewardSound );
						if( m_pPlayer->Quest( i )->GoldReward() > 0 )
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pGoldSound );
						}

						m_pGameUI->OpenMessage( pD3DDevice, m_pPlayer->Quest( i )->CompletionDescription() );

						m_pPlayer->ReceiveReward( *m_pPlayer->Quest( i ),
												pD3DDevice,
												*m_pActiveLevel,
												*m_pItemManifest );
						m_pPlayer->RemoveQuest( i );
						m_pPlayer->IncrementStat( KJournalStatQuestsCompleted );
					}
					else
					{
						m_pGameUI->OpenMessage( pD3DDevice, m_pPlayer->Quest( i )->IncompleteDescription() );
					}
					m_pPlayer->Target()->SetTarget( NULL );
					m_pPlayer->SetTarget( NULL );
					m_pPlayer->SetAIState( KAIInactive );
					return;
				}
			}
		}
	}

	// check the uber-quest for completion
	if( m_pPlayer->MasterQuest() != NULL )
	{
		if( m_pPlayer->MasterQuest()->Completed() &&
			!m_pPlayer->MasterQuest()->CompletionNotified() )
		{
			m_pPlayer->PlayHelpSample( KSoundHelpJourneyEnd, kTrue );
			m_pGameUI->OpenMessage( pD3DDevice, LoadLocalizedString2Variables(212, m_pPlayer->MasterQuest()->QuestMonsterName(), m_pPlayer->MasterQuest()->GiverName() ) );
			m_pPlayer->MasterQuest()->SetCompletionNotified( kTrue );
			return;
		}
		if( m_pPlayer->AIState() == KAICollectReward )
		{
			if( m_pPlayer->Target() != NULL )
			{
				if( StringUpper( m_pPlayer->Target()->Name() ) ==
					StringUpper( m_pPlayer->MasterQuest()->GiverName() ) )
				{
					bool QuestSuccessful( m_pPlayer->MasterQuest()->Completed() );
					if( QuestSuccessful &&
						m_pPlayer->MasterQuest()->QuestItem() != NULL )
					{
						QuestSuccessful = kFalse;
						// item based quests check for the items, then remove them
						if( m_pPlayer->Inventory()->HasItems( *m_pPlayer->MasterQuest()->QuestItem(),
															  m_pPlayer->MasterQuest()->QuestItems() ) )
						{
							for( uint32 j = 0; j < m_pPlayer->MasterQuest()->QuestItems(); j++ )
							{
								CItem* pItem = m_pPlayer->Inventory()->FindItem( *m_pPlayer->MasterQuest()->QuestItem() );
								if( pItem != NULL )
								{
									QuestSuccessful = kTrue;
									m_pPlayer->Inventory()->RemoveItem( pItem );
									DELETE_SAFELY( pItem );
								}
							}
						}
						if( !QuestSuccessful )
						{
							if( m_pPlayer->Pets() > 0 )
							{
								// item based quests check for the items, then remove them
								if( m_pPlayer->Pet( 0 )->Inventory()->HasItems( *m_pPlayer->MasterQuest()->QuestItem(),
																				m_pPlayer->MasterQuest()->QuestItems() ) )
								{
									for( uint32 j = 0; j < m_pPlayer->MasterQuest()->QuestItems(); j++ )
									{
										CItem* pItem = m_pPlayer->Pet( 0 )->Inventory()->FindItem( *m_pPlayer->MasterQuest()->QuestItem() );
										if( pItem != NULL )
										{
											QuestSuccessful = kTrue;
											m_pPlayer->Pet( 0 )->Inventory()->RemoveItem( pItem );
											DELETE_SAFELY( pItem );
										}
									}
								}
							}
						}
					}
					if( QuestSuccessful )
					{
						m_pPlayer->PlayHelpSample( KSoundHelpFateSmiles, kTrue );
						FSOUND_PlaySound( FSOUND_FREE, m_pRewardSound );
						if( m_pPlayer->MasterQuest()->GoldReward() > 0 )
						{
							FSOUND_PlaySound( FSOUND_FREE, m_pGoldSound );
						}

						m_pGameUI->OpenMessage( pD3DDevice, m_pPlayer->MasterQuest()->CompletionDescription() );

						m_pPlayer->ReceiveReward( *m_pPlayer->MasterQuest(),
												pD3DDevice,
												*m_pActiveLevel,
												*m_pItemManifest );
						m_pPlayer->ClearMasterQuest();
					}
					else
					{
						m_pGameUI->OpenMessage( pD3DDevice, m_pPlayer->MasterQuest()->IncompleteDescription() );
					}
					m_pPlayer->Target()->SetTarget( NULL );
					m_pPlayer->SetTarget( NULL );
					m_pPlayer->SetAIState( KAIInactive );
					return;
				}
			}
		}
	}

	if( m_pPlayer->AIState() == KAICollectReward )
	{
		if( m_pPlayer->Target() != NULL )
		{
			m_pPlayer->Target()->SetTarget( NULL );
			m_pPlayer->SetTarget( NULL );
		}
	}
} // CGameClient::UpdateQuests()