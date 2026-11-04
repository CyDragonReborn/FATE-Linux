/*
	game.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

#include <windows.h>
#include <string>
#include <time.h>

using namespace std;

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/capabilities.h"
#include "../UTILITIES/llist.h"
#include "../UTILITIES/vistafileutilities.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../DIRECT3D/Dx8App.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/soundmanager.h"
#include "../MEDIA/texture.h"

#include "../UI/fontmetric.h"
#include "../UI/text.h"

#include "../GAMECLIENT/gameclient.h"
#include "../GAME/game.h"
#include "../CHARACTER/monster.h"

#include "../TRACKING/GameTracking.h"

#include "../MENU/menumanager.h"
#include "../MENU/loadingmenu.h"

#include "gamestatemanager.h"
#include "game.h"


// temporary includes for maze testing
#include "../LEVEL/maze.h"
#include "../LEVEL/template.h"

#include "../OBJECTS/curve.h"

#include "../GAMECLIENT/demorules.h"

//[BLG] IF (10-18-2013): Added for Steam Integration
#include "steam_api.h"
#include "../UTILITIES/StatsAndAchievements.h"

CGame::CGame( std::string Params ) :	// command line parameters 
			m_pSettings( NULL ),
			m_pCapabilities( NULL ),
			m_pRefManager( NULL ),
			m_pSoundManager( NULL ),
			m_pFontMetric( NULL ),
			m_pFPSText( NULL ),
			m_pGameClient( NULL ),
			m_pMenuManager( NULL ),
			m_pGameStateManager( NULL ),
			m_pLoadingMenu( NULL ),
			m_ShouldClose( kFalse ),
			m_Initialized( kFalse ),
			m_PostInitialized( kFalse ),
			m_FirstFrameRendered( kFalse ),
			m_FirstFrameInitialized( kFalse ),
			m_VersionNumber( 1.39f ),
			m_bSteamInitialized( kFalse ),
			m_CallbackGameOverlayActivated( this, &CGame::OnGameOverlayActivated )
{
	SetWindowTitle( "FATE" );
	std::string LocalPathToExe = GetLocalPath();

	// set working directory to exe path
	::SetCurrentDirectory( LocalPathToExe.c_str() );
	
	// create our capabilities object
	m_pCapabilities = new CCapabilities();

	CVistaFileUtilities::CreateTempFile(CVistaFileUtilities::GetSharedDataPath());
	CVistaFileUtilities::CreateTempFile(CVistaFileUtilities::GetSharedDataPath(KFileLocation_SharedCache));

	// create a settings object
	m_pSettings = new CSettings( m_pCapabilities, LocalPathToExe, LocalPathToExe );
	m_pSettings->Load( "config.dat" );

	if( !m_pSettings->GetSettings( KSetCustomCursor ) )
	{
		SetCursorVisibility( kTrue );
	}
	else
	{
		// set startup settings for our application
		SetCursorVisibility( kFalse );
	}

	// if this is an updated game version, we kill old precompressed textures,
	// in case any texture updates were introduced in this version
	if( m_pSettings->GetSettings( KSetVersion ) != m_VersionNumber )
	{
		ClearOldDDS( CVistaFileUtilities::GetSharedDataPath( KFileLocation_SharedCache ) );
	}



	SetStartupResolution( m_pSettings->GetSettings( KSetScreenWidth ),
						  m_pSettings->GetSettings( KSetScreenHeight ) );

	if( IsGameUnlocked() )
	{
		SetStartupFullscreen( m_pSettings->GetSettings( KSetFullscreen ) == 1 );
	}
	else
	{
		SetStartupFullscreen( kFalse );
	}

	SetRequiresVSYNC( m_pSettings->GetSettings( KSetVSYNC ) == 0 );

	// create a sound manager
	m_pSoundManager = new CSoundManager( *m_pSettings,
										  LocalPathToExe );

	// create our reference manager
	m_pRefManager = new CRefManager( *m_pSettings, LocalPathToExe );

	bool Online( kFalse );
	std::string BaseURL( "" );

// online versions need to pull the site URL from the parameters list
#ifdef ONLINEVERSION
	Online = kTrue;
	std::queue< std::string > Parameters;
	TokenizeString( &Parameters, Params, ",", '#' );

	if( Parameters.size() > 0 )
	{
		BaseURL = Parameters.front();
		Parameters.pop();
	}
#endif

	m_pGameStateManager = new CGameStateManager( Params,
												 BaseURL );
} // CGame::CGame()

CGame::~CGame( void )
{
} // CGame::~CGame()


//[BLG] IF Initialize Steam Before D3D Objects
bool CGame::InitializeSteam()
{
	if(!m_bSteamInitialized)
	{
		if ( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
		{
			// if Steam is not running or the game wasn't started through Steam, SteamAPI_RestartAppIfNecessary starts the 
			// local Steam client and also launches this game again.

			// Once you get a public Steam AppID assigned for this game, you need to replace k_uAppIdInvalid with it and
			// removed steam_appid.txt from the game depot.

			return kFalse;
		}

		// Initialize SteamAPI, if this fails we bail out since we depend on Steam for lots of stuff.
		// You don't necessarily have to though if you write your code to check whether all the Steam
		// interfaces are NULL before using them and provide alternate paths when they are unavailable.
		//
		// This will also load the in-game steam overlay dll into your process.  That dll is normally
		// injected by steam when it launches games, but by calling this you cause it to always load,
		// even when not launched via steam.
	
		m_bSteamInitialized = SteamAPI_Init();
	}

	return m_bSteamInitialized;
}

void CGame::ShutDownSteam()
{
	// Shutdown the SteamAPI
	SteamAPI_Shutdown();
}

void CGame::SetFullscreenAfterPal()
{	
	if( m_Windowed )
	{
		if( m_pSettings->GetSettings( KSetFullscreen ) )
		{
			ToggleFullscreen();
			if( m_pMenuManager != NULL )
			{
				m_pMenuManager->ResetMouse( m_pD3DDevice );
			}
		}
	}
}

void CGame::DropFromFullScreenPal() //marsh
{
	if( !m_Windowed &&
		!IsGameUnlocked() )
	{
		ToggleFullscreen();	
	}
}

// overrides a method in the CDX8App class - this is a chance to set up any 
// data that is not d3d device related (standard application data )
HRESULT CGame::OneTimeSceneInit( void )
{
	
	// start the game timer
	Game_Timer( KTimerStart );


	return S_OK;
} // CGame::OneTimeSceneInit( )


// overrides a method in the CDX8App class
// to initialize any 3d scene objects
HRESULT CGame::InitDeviceObjects( void )
{

	// we can now detect our d3d capabilities, as our device has been created
	m_pCapabilities->DetectD3DCapabilities( m_pD3D, m_pD3DDevice, &m_D3Dpp );
	// now that we know what our capabilities are, verify that our
	// settings are possible, and adjust if they aren't
	m_pSettings->VerifySettings();

	// initialize font texture and metric, and an FPS message
	CMaterial*	pFontMaterial	= m_pRefManager->AddMaterialAlpha( m_pD3DDevice, 
															       "FONT/font.png",
															       "FONT/fonta.png" );

	m_pFontMetric = new CFontMetric( GetLocalPath() + "FONT/font.ini" );

	m_pFPSText = new CText( m_pD3DDevice,
							pFontMaterial,
							m_pFontMetric,
							"FPS : 0",
							0,
							0,
							1.0f,
							kFalse,
							KJustifyLeft );

	m_pGameStateManager->SetGameState( KGameStateMenu);

	m_pLoadingMenu = new CLoadingMenu( *m_pRefManager, *m_pSettings );
	m_pLoadingMenu->Create( m_pD3DDevice );

	// we start out the OPPOSITE of our vsync need, to ensure that the device
	// gets reset first thing. The reason for this is that some machines
	// seem to need this reset to do independent UVS properly ( notably my GEFORCE at home )
	// probably a driver bug, but this takes care of it
	if( m_pSettings->GetSettings( KSetVSYNC ) )
	{
		SetRequiresVSYNC( m_pSettings->GetSettings( KSetVSYNC ) == 0 );
	}

	if( !IsFullscreenResolutionAvailable( 800, 600 ) )
	{
		m_pSettings->SetSettings( KSet800x600Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1024, 768 ) )
	{
		m_pSettings->SetSettings( KSet1024x768Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1152, 864 ) )
	{
		m_pSettings->SetSettings( KSet1152x864Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1280, 1024 ) )
	{
		m_pSettings->SetSettings( KSet1280x1024Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1600, 1200 ) )
	{
		m_pSettings->SetSettings( KSet1600x1200Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1280, 768 ) )
	{
		m_pSettings->SetSettings( KSet1280x768Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1920, 1200 ) )
	{
		m_pSettings->SetSettings( KSet1920x1200Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1920, 1080 ) )
	{
		m_pSettings->SetSettings( KSet1920x1080Available, kFalse );
	}
	if( !IsFullscreenResolutionAvailable( 1366, 768 ) )
	{
		m_pSettings->SetSettings( KSet1366x768Available, kFalse );
	}

	return S_OK;
} // CGame::InitDeviceObjects( )

void CGame::Initialize( void )
{
	if( !FAILED( m_pD3DDevice->BeginScene() ) )
	{
		// clear the back buffer
		m_pD3DDevice->Clear( 0,
							NULL,
							D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,
							D3DCOLOR_RGBA( 0, 0, 0, 255 ),
							1.0f,
							0 );
	}

	m_pD3DDevice->EndScene();
	m_pD3DDevice->Present( NULL, NULL, NULL, NULL );

	m_pGameClient = new CGameClient( m_pD3DDevice,
									 *m_pRefManager,
									 *m_pSettings,
									 *m_pGameStateManager,
									 *m_pLoadingMenu,
									 *m_pSoundManager );

	m_pMenuManager = new CMenuManager( m_pD3DDevice,
									   *m_pRefManager,
									   *m_pSettings,
									   *m_pGameClient,
									   *m_pGameStateManager,
									   *m_pSoundManager );

	//[BLG] IF Create the Achievements Manager
	CStatsAndAchievements* m_pStatsAndAchievements = new CStatsAndAchievements();
	m_pStatsAndAchievements->Initialize();

	m_Initialized = kTrue;
}

void CGame::PostInitialize( void )
{
	// if this is an updated game version, we kill old precompressed textures,
	// in case any texture updates were introduced in this version
	if( m_pSettings->GetSettings( KSetVersion ) != m_VersionNumber )
	{
		D3DDISPLAYMODE pMode;
		m_pD3DDevice->GetDisplayMode( &pMode );
		// if we can really do DDS conversion...
		if( !FAILED( m_pD3D->CheckDeviceFormat( D3DADAPTER_DEFAULT,
												D3DDEVTYPE_HAL,
												pMode.Format,
												0,
												D3DRTYPE_TEXTURE,
												D3DFMT_DXT1 ) ) )
		{
			m_pLoadingMenu->SetMessageText( LoadLocalizedString(143) );
			ConvertToDDS( GetLocalPath() );
			m_pLoadingMenu->SetMessageText( "" );
		}
	}

	m_pLoadingMenu->SetDisplayTips( kTrue );

	m_pSettings->SetSettings( KSetVersion, m_VersionNumber );
	m_pSettings->Save();

	// flip the loading menu so we have something to see during initialization
	m_pLoadingMenu->SetProgressPercentage( 10 );
	RenderLoadingMenu( kTrue );

	m_pGameClient->FinalizeCreation( m_pD3DDevice );

	// flip the loading menu so we have something to see during initialization
	m_pLoadingMenu->SetProgressPercentage( 40 );
	RenderLoadingMenu( kTrue );

	m_pMenuManager->FinalizeCreation( m_pD3DDevice );

	// flip the loading menu so we have something to see during initialization
	m_pLoadingMenu->SetProgressPercentage( 80 );
	RenderLoadingMenu( kTrue );


	m_PostInitialized = kTrue;

} // CGame::PostInitialize()

// some operations only work after the first frame render
void CGame::FirstFrame( void )
{
		// set the initial gamma
	D3DGAMMARAMP Ramp;
	uint32 Gamma = m_pSettings->GetSettings( KSetGammaRamp );
	Gamma = 256 - Gamma;
	Gamma += 75;

	FillGammaRamp( Ramp, Gamma );

    m_pD3DDevice->SetGammaRamp( D3DSGR_NO_CALIBRATION, &Ramp );
} // CGame::FirstFrame()

// screen lost focus 
HRESULT CGame::UnFocus( void )
{

	if( m_pGameStateManager != NULL &&
		m_pGameStateManager->GameState() == KGameStateInGame &&
		m_pGameClient != NULL )
	{
		if( !m_pGameClient->Paused() )
		{
			m_pGameClient->Pause();
		}
	}

	if( m_pSoundManager != NULL )
	{
		m_pSoundManager->Mute();
		m_pSoundManager->UpdateVolume();
	}
	CDX8App::UnFocus();
	return S_OK;
} // CGame::UnFocus()

HRESULT CGame::Focus( void )
{
	if( m_pGameStateManager != NULL )
	{
		if( m_pSoundManager != NULL )
		{
			m_pSoundManager->UnMute();
			m_pSoundManager->UpdateVolume();
		}
	}
	if( m_Initialized )
	{
		// reset gamma
		D3DGAMMARAMP Ramp;
		uint32 Gamma = m_pSettings->GetSettings( KSetGammaRamp );
		Gamma = 256 - Gamma;
		Gamma += 75;

		FillGammaRamp( Ramp, Gamma );

		m_pD3DDevice->SetGammaRamp( D3DSGR_NO_CALIBRATION, &Ramp );
	}

	CDX8App::Focus();

	return S_OK;
}

// overrides a method in the CDX8App class
// to perform any game logic before the render
HRESULT CGame::FrameMove( void )
{
	// do a first-frame initialization, now that the device is set
	if( !m_Initialized )
	{
		// flip the loading menu so we have something to see during initialization
		//RenderLoadingMenu( kTrue );

		Initialize();
	}

	// if we're not the focused app, give other apps room to breathe
	if ( !Focused() && Windowed() )
	{
		Sleep( 100 );
	}

	// if someone clicks offscreen just before turning to fullscreen,
	// it ends up minimizing the window - we can fix that by restoring
	// it, and this keeps strange flickery desktop elements from peeking 
	// through.
	if ( (m_pGameStateManager->GameState() == KGameStateLoadMenu) || !m_PostInitialized || !m_Initialized )
	{
		if( Fullscreen() )
		{
			if ( !Focused() || ::GetActiveWindow() != m_hWnd || ::GetFocus() != m_hWnd || ::GetForegroundWindow() != m_hWnd )
			{
				ShowWindow( m_hWnd, SW_SHOWNORMAL );
				SetForegroundWindow( m_hWnd );
				SetActiveWindow ( m_hWnd );
				SetFocus( m_hWnd );
			}
		}
	}

	// find the number of seconds that have passed
	float32 TimeElapsed = (float32)Game_Timer( KTimerGetElapsedTime );

	// keep sound volumes up to date
	m_pSoundManager->Update();

	// update any animated materials
	m_pRefManager->UpdateMaterials( TimeElapsed );

	m_pGameStateManager->Update( TimeElapsed );

	// [BLG] IF Steam Process
	SteamAPI_RunCallbacks();

	//CStatsAndAchievements::GetInstance()->Update();

	if( !m_PostInitialized &&
		m_Initialized &&
		m_pGameStateManager->GameState() == KGameStateLoadMenu &&
		m_pGameStateManager->GetTargetMenu() == KMainMenu )
	{
		// flip the loading menu so we have something to see during initialization
		RenderLoadingMenu( kTrue );

		PostInitialize();
		if( m_pSettings->GetSettings( KSetFullscreen ) == 1 &&
			!Fullscreen() )
		{
			ToggleFullscreen();
			if( m_pMenuManager != NULL )
			{
				m_pMenuManager->ResetMouse( m_pD3DDevice );
			}
		}

		int32 i = 0;
	}
	if( m_FirstFrameInitialized )
	{

		switch( m_pGameStateManager->GameState() )
		{
			case KGameStateLoadMenu :
				m_pMenuManager->SetActiveMenu( m_pGameStateManager->GetTargetMenu() );

				m_pMenuManager->Update( m_pD3DDevice,
										m_hWnd,
										TimeElapsed );
				m_pGameStateManager->SetGameState( KGameStateMenu );
			break;
			case KGameStateQuitToMenu :
				m_pLoadingMenu->SetProgressPercentage( 10 );
				RenderLoadingMenu( kTrue );
				m_pMenuManager->Reload( m_pD3DDevice );
				m_pMenuManager->SetActiveMenu( m_pGameStateManager->GetTargetMenu() );
				m_pLoadingMenu->SetProgressPercentage( 30 );
				RenderLoadingMenu( kTrue );
				m_pGameClient->DestroyLevel();
				m_pLoadingMenu->SetProgressPercentage( 50 );
				RenderLoadingMenu( kTrue );
				// re-set up any objects in the background screen level
				m_pMenuManager->Activate();
				m_pMenuManager->FindObjects();
				m_pLoadingMenu->SetProgressPercentage( 80 );
				RenderLoadingMenu( kTrue );
				m_pGameStateManager->SetGameState( KGameStateMenu );
				m_pRefManager->RemoveUnused();
				m_pMenuManager->ReloadPlayer();
			break;
			case KGameStateLoadGame :
				m_pMenuManager->Unload();
				m_pGameClient->FlushAll();
				m_pGameClient->GenerateLevel( m_pD3DDevice,
											0,
											kTrue,
											kFalse,
											kFalse );
				m_pGameStateManager->SetGameState( KGameStateInGame );

			break;
			// update in-menu state
			case KGameStateMenu :
				m_pGameClient->UpdateBlurRate( FPS() );
				m_pMenuManager->Update( m_pD3DDevice,
										m_hWnd,
										TimeElapsed );
			break;
			// update in-game state
			case KGameStateInGame :
				m_pGameClient->UpdateBlurRate( FPS() );
				m_pGameClient->Update( m_pD3DDevice,
									m_hWnd,
									TimeElapsed );
				break;
			// close the application
			case KGameStateQuit :
				m_ShouldClose = kTrue;
			break;
		}
	}

	if( m_pSettings->GetSettings( KSetShowFPS ) > 0 )
	{
		m_pFPSText->SetText( m_pD3DDevice, FPSText() + "\n" + m_pGameClient->DebugText() );
	}

	// make sure we're using the current vsync setting
	SetRequiresVSYNC( m_pSettings->GetSettings( KSetVSYNC ) == 1 );

	return S_OK;
} // CGame::FrameMove( )

// overrides a method in the CDX8App class
// to render the scene
HRESULT CGame::Render( void )
{
	HRESULT hr;

	if( !m_Initialized )
	{
		return S_OK;
	}


	if( !m_FirstFrameRendered )
	{
		m_FirstFrameRendered = kTrue;
	}
	else
	{
		if( !m_FirstFrameInitialized )
		{
			FirstFrame();
			m_FirstFrameInitialized = kTrue;
		}
	}

	// if settings say we want to do FSAA, and we support it, turn it on
	// we need to do this BEFORE beginscene, because some cards don't support
	// changing between a begin and end
	if( CanSwitchFSAA() )
	{
		if ( m_pSettings->GetSettings( KSetFSAA ) &&
			( ( Windowed() && WindowedFSAA() ) ||
			( Fullscreen() && FullscreenFSAA() ) ) )
		{
			m_pD3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, kTrue );
		}
		else // otherwise make sure to disable it
		{
			m_pD3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, kFalse );
		}
	}

	PreRender();

	// try to begin the render of the primary scene
	if ( hr = FAILED( m_pD3DDevice->BeginScene() ) )
	{
		return hr;
	}

    D3DVIEWPORT8 Viewport;
    Viewport.X = 0;
    Viewport.Y = 0;
    Viewport.Width = m_pSettings->GetSettings( KSetScreenWidth );
    Viewport.Height = m_pSettings->GetSettings( KSetScreenHeight );
    Viewport.MinZ = 0.0f;
    Viewport.MaxZ = 1.0f;

	m_pD3DDevice->SetViewport( &Viewport );


	// clear the back buffer
	m_pD3DDevice->Clear( 0,
						 NULL,
						 D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,
						 D3DCOLOR_RGBA( 0, 0, 0, 255 ),
						 1.0f,
						 0 );



	switch( m_pGameStateManager->GameState() )
	{
		case KGameStateLoadMenu :
		case KGameStateMenu :
			if( m_pMenuManager != NULL )
			{
				m_pMenuManager->Render( m_pD3DDevice );
			}
			break;
		case KGameStateInGame :
			if( m_pGameClient != NULL )
			{
				m_pGameClient->Render( m_pD3DDevice );
			}
		break;
	}



	// render all UI elements
	if( m_FirstFrameInitialized )
	{
		RenderUIElements();
	}


	

	// End rendering
	m_pD3DDevice->EndScene();

	if( m_pSettings->IsResolutionChangeRequested() )
	{
		ChangeResolution( m_pSettings->GetSettings( KSetScreenWidth ),
						  m_pSettings->GetSettings( KSetScreenHeight ) );
		if( m_pMenuManager != NULL )
		{
			m_pMenuManager->ResetMouse( m_pD3DDevice );
		}
	}

	// for devices that CANNOT enable FSAA on the fly, we have to
	// tear down and recreate the device to perform the switch. 
	if ( !CanSwitchFSAA() )
	{

		if ( m_pSettings->GetSettings( KSetFSAA ) &&
			 ( ( Windowed() && SupportsWindowedFSAA() && !AllowFSAA() ) ||
			   ( Fullscreen() && SupportsFullscreenFSAA() && !AllowFSAA() ) ) )
		{
			SetAllowFSAA( kTrue );
			ChangeResolution( m_pSettings->GetSettings( KSetScreenWidth ),
							m_pSettings->GetSettings( KSetScreenHeight ) );
			if( m_pMenuManager != NULL )
			{
				m_pMenuManager->ResetMouse( m_pD3DDevice );
			}
		}
		else if ( !m_pSettings->GetSettings( KSetFSAA ) &&
				  ( ( Windowed() && WindowedFSAA() ) ||
					( Fullscreen() && FullscreenFSAA() ) ) )
		{
			SetAllowFSAA( kFalse );
			ChangeResolution( m_pSettings->GetSettings( KSetScreenWidth ),
							m_pSettings->GetSettings( KSetScreenHeight ) );
			if( m_pMenuManager != NULL )
			{
				m_pMenuManager->ResetMouse( m_pD3DDevice );
			}
		}
	}

	return S_OK;
} // CGame::Render( )


// preform rendering operations that need to happen before the render proper
// (rendering to texture for reflection maps, for instance )
void CGame::PreRender( void )
{

	switch( m_pGameStateManager->GameState() )
	{
		case KGameStateMenu :
			if( m_pMenuManager != NULL )
			{
				m_pMenuManager->PreRender( m_pD3DDevice );
			}
		break;
		case KGameStateInGame :
			if( m_pGameClient != NULL )
			{
				m_pGameClient->PreRender( m_pD3DDevice );
			}
		break;
	}

} // CGame::PreRender()

void CGame::RenderUIElements( void )
{

	// set up an othographic projection for rendering UI elements - we don't need depth!
	D3DXMATRIX Identity;
	D3DXMATRIX Orthographic;
	D3DXMatrixIdentity( &Identity );
	// the values you use for width and height for the orthographic matrix
	// correspond to the scale of pixel coordinates used in UI elements.
	// for instance, positioning something at 1024x768 with an orthographic matrix
	// using 1024x768 will place it in the bottom-right of the screen - 
	// if you use an 800x600 resolution for your ortho matrix, something at 1024x768
	// would be OFFSCREEN. It's all relative, so choose this based upon the size
	// of your source artwork
	D3DXMatrixOrthoLH ( &Orthographic, 
						1024,
						768,
						1.0f,
						10.0f );

	m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &Orthographic );
	m_pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );
	m_pD3DDevice->SetTransform( D3DTS_VIEW, &Identity );


	// turn off lighting
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	// we want to alpha blend our UI elements
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	// no zwrite or ztest while rendering text
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

//////////////////////////////////////////////////////////////////////////////
// Render any UI elements
//////////////////////////////////////////////////////////////////////////////

	// now render UI elements
	switch( m_pGameStateManager->GameState() )
	{
		case KGameStateLoadMenu :
		case KGameStateMenu :
			m_pMenuManager->RenderUI( m_pD3DDevice );
		break;
		case KGameStateInGame :
			m_pGameClient->RenderUI( m_pD3DDevice );
		break;
	}

	if( m_pSettings->GetSettings( KSetShowFPS ) > 0 )
	{
		m_pFPSText->Render( m_pD3DDevice );
	}

//////////////////////////////////////////////////////////////////////////////
// End render any UI elements
//////////////////////////////////////////////////////////////////////////////

	// turn back on the zwrite/ztest
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

	// turn alpha blending back on
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	// turn on lighting
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

} // CGame::RenderUIElements()

void CGame::RenderLoadingMenu( bool Flip )	// force a flip?
{

//////////////////////////////////////////////////////////////////////////////
// Render any UI elements
//////////////////////////////////////////////////////////////////////////////

	m_pLoadingMenu->RenderUI( m_pD3DDevice, Flip );
//////////////////////////////////////////////////////////////////////////////
// End render any UI elements
//////////////////////////////////////////////////////////////////////////////

} // CGame::RenderLoadingMenu()

// overrides a method in the CDX8App class
// invalidate any device objects when required
HRESULT CGame::InvalidateDeviceObjects( void )
{

	if( m_pRefManager != NULL )
	{
		m_pRefManager->InvalidateDeviceObjects();
	}


	return S_OK;
} // CGame::InvalidateDeviceObjects( )

// overrides a method in the CDX8App class
// restore any device objects when required
// we also need to 'clean up' many of the texture and render states
// so we know what stare we're in. You might not have to do all this,
// but better safe than sorry-----
HRESULT CGame::RestoreDeviceObjects( void )
{
	// make sure our settings are still ok
	m_pSettings->VerifySettings();

	// Set up the world identity matrix
	D3DXMATRIX	Identity;
	D3DXMatrixIdentity( &Identity );
	m_pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );

	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetTextureStageState( 0,
										D3DTSS_MIPFILTER,
										m_pSettings->GetSettings( KSetMipFilter ) );

	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetTextureStageState( 1,
										D3DTSS_MIPFILTER,
										m_pSettings->GetSettings( KSetMipFilter ) );

	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	m_pD3DDevice->SetTextureStageState( 2,
										D3DTSS_MIPFILTER,
										m_pSettings->GetSettings( KSetMipFilter ) );

	// set up the standard textures blends and states
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0 );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

	m_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );

	// enable specular lighting if we can do it
	if ( m_pSettings->GetSettings( KSetSpecular ) )
	{
		m_pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE,
									  D3DMCS_MATERIAL );
	}


	// set culling for polys to counterclockwise
	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );


	if ( m_pRefManager != NULL )
	{
		m_pRefManager->RestoreDeviceObjects( m_pD3DDevice );
	}
	
	if ( m_pGameClient != NULL )
	{
		m_pGameClient->RestoreDeviceObjects( m_pD3DDevice );
	}
	
	if ( m_pMenuManager != NULL )
	{
		m_pMenuManager->RestoreDeviceObjects( m_pD3DDevice );
	}


	return S_OK;
} // CGame::RestoreDeviceObjects( )


// overrides a method in the CDX8App class
// delete all device objects when cleaning up at shutdown
// make sure to delete ALL d3d stuff you made, or you'll have dangling references
HRESULT CGame::DeleteDeviceObjects( void )
{
	OutputDebugString( "Delete Device Objects\n" );

	// save our settings
	if( m_pSettings != NULL )
	{
		m_pSettings->Save();
	}

	DELETE_SAFELY( m_pLoadingMenu );

	DELETE_SAFELY( m_pMenuManager );

	DELETE_SAFELY( m_pGameClient );

	OutputDebugString( "Delete FPS Text\n" );

	DELETE_SAFELY( m_pFPSText );

	OutputDebugString( "Delete Ref Manager\n" );

	DELETE_SAFELY( m_pRefManager );

	OutputDebugString( "Done Deleting Ref Manager\n" );
	return S_OK;
} // CGame::DeleteDeviceObjects( )

// overrides a method in the CDX8App class
// remove any application specific data objects
HRESULT CGame::FinalCleanup( void )
{
	DELETE_SAFELY( m_pFontMetric );

	DELETE_SAFELY( m_pSoundManager );
	DELETE_SAFELY( m_pSettings );
	DELETE_SAFELY( m_pCapabilities );

	DELETE_SAFELY( m_pGameStateManager );

	ShutDownSteam();

	return S_OK;
} // CGame::FinalCleanup( )

// process event messages
LRESULT CGame::MsgProc( HWND hWnd, 
					    UINT uMsg, 
					    WPARAM wParam, 
					    LPARAM lParam )
{

	if ( m_hWnd != NULL && ApplicationActive() )
	{

		if( uMsg == WM_SETCURSOR )
		{
			if( m_pGameClient != NULL )
			{
				m_pGameClient->ResetMouse();
			}
		}

		if( uMsg == WM_KEYDOWN || uMsg == WM_KEYUP ||
			uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP )
		{

			if( wParam == VK_SNAPSHOT )
			{
				TakeScreenshot();
			}
			switch( m_pGameStateManager->GameState() )
			{
				case KGameStateMenu :
					if( m_pMenuManager != NULL )
					{
						m_pMenuManager->KeyEvent( uMsg, wParam );
					}
				break;
				case KGameStateInGame :
					if( m_pGameClient != NULL )
					{
						m_pGameClient->KeyEvent( uMsg, wParam );
					}
				break;
			}
		}

		if( WM_LBUTTONDOWN == uMsg || WM_RBUTTONDOWN == uMsg || WM_MBUTTONDOWN == uMsg ||
			WM_LBUTTONDBLCLK == uMsg || WM_RBUTTONDBLCLK == uMsg || WM_MBUTTONDBLCLK == uMsg ||
			WM_LBUTTONUP == uMsg || WM_RBUTTONUP == uMsg || WM_MBUTTONUP == uMsg ||
			WM_MOUSEWHEEL == uMsg )
		{
			switch( m_pGameStateManager->GameState() )
			{
				case KGameStateMenu :
					if( m_pMenuManager != NULL )
					{
						m_pMenuManager->MouseEvent( uMsg, wParam );
					}
				break;
				case KGameStateInGame :
					if( m_pGameClient != NULL )
					{
						m_pGameClient->MouseEvent( uMsg, wParam );
					}
				break;
			}
		}
	}
	if( uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP )
	{
		return 0;
	}

	if( uMsg == WM_CLOSE )
	{
		if( !IsGameUnlocked() )
		{
			DropFromFullScreenPal(); //marsh
		}
	}
	// also process the base-class MsgProc
	return CDX8App::MsgProc( hWnd, uMsg, wParam, lParam );
} // CGame::MsgProc()

void CGame::ClearOldDDS( std::string Path )		// path to the folder to look in
{
	WIN32_FIND_DATA FindFileData;
	HANDLE Find;

	Find = FindFirstFile( ( Path + "/*.*" ).c_str(), &FindFileData );

	BOOL Done( kFalse );
       
	while ( Find != INVALID_HANDLE_VALUE && !Done )
    {
		std::string Name = StringUpper( FindFileData.cFileName );
		
		if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			if( Name.length() > 4 )
			{
				std::string Extension = StringUpper( Name.substr( Name.length() - 3, 3 ) );
				if( Extension == "DDS" )
				{
					bool Delete( kTrue );
					if( Name.length() >= 10 )
					{
						std::string EndFile = StringUpper( Name.substr( Name.length() - 10, 10 ) );
						if( EndFile == "ENVMAP.DDS" )
						{
							Delete = kFalse;
						}
					}
					if( Delete )
					{
						DeleteFile( ( Path + "/" + Name ).c_str() );
					}
				}
			}
		}
		else
		{
			bool Recurse = true;
			if ( Name == "." )	Recurse = false;
			if ( Name == ".." ) Recurse = false;

			if( Recurse )
			{
				ClearOldDDS( Path + "/" + Name );
			}
		}
		Done = !FindNextFile( Find, &FindFileData );
	}

	FindClose( Find );

} // CGame::ClearOldDDS()

// we go through and convert all textures to DDS files if required
void CGame::ConvertToDDS( std::string Path )		// path to the folder to look in
{
	uint32 Depth( 0 );
	uint32 FilesChecked( 0 );

	// first, find all MP3 files in this folder.
	std::vector< std::string > TextureList;
        
	GetTexturesFromFolder( Path,
						   TextureList,
						   Depth,
						   FilesChecked );

	for( uint32 i = 0; i < TextureList.size(); i++ )
	{
		CTexture* pTexture = new CTexture( kTrue );
		pTexture->Create( m_pD3DDevice, TextureList[i] );
		DELETE_SAFELY( pTexture );

		// flip the loading menu so we have something to see during initialization
		float32 Percentage = (float32)i / (float32)TextureList.size() * 100.0f;
		m_pLoadingMenu->SetProgressPercentage( Percentage );
		RenderLoadingMenu( kTrue );

	}
} // CGame::ConvertToDDS()

void CGame::GetTexturesFromFolder( std::string Path,						// path to the folder to look in
									    std::vector< std::string >& Paths,	// list of paths to add to
										uint32& Depth,						// subfolder depth
										uint32& FilesChecked )				// number of files checked
{
	Depth++;

	WIN32_FIND_DATA FindFileData;
	HANDLE Find;

	Find = FindFirstFile( ( Path + "/*.*" ).c_str(), &FindFileData );

	BOOL Done( kFalse );
       
	while ( Find != INVALID_HANDLE_VALUE && !Done )
    {
		std::string Name = FindFileData.cFileName;
		FilesChecked++;

		
		if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			if( Name.length() > 4 )
			{
				std::string Extension = StringUpper( Name.substr( Name.length() - 3, 3 ) );
				if( Extension == "PNG" || Extension == "JPG" )
				{
					Paths.push_back( Path + "/" + Name );
				}
			}
		}
		else
		{
			if( Name != "." && Name != ".." )
			{
				if( StringUpper( Name ) != "WIRE" )
				{
					GetTexturesFromFolder( Path + "/" + Name,
											Paths,
											Depth,
											FilesChecked );
				}
			}
		}
		Done = !FindNextFile( Find, &FindFileData );
	}
	Depth --;

	FindClose( Find );
} // CGame::GetTexturesFromFolder()

void CGame::TakeScreenshot( void )
{
	if( !m_Initialized )
	{
		return;
	}
	CreateLocalDirectory( CVistaFileUtilities::GetSharedDataPath() + "SCREENSHOTS\\" );
	std::string NewPath = SelectEmptyScreenshot( CVistaFileUtilities::GetSharedDataPath() + "SCREENSHOTS\\" ); 

	// Obtain adapter number and device window
	D3DDEVICE_CREATION_PARAMETERS dcp;
	m_pD3DDevice->GetCreationParameters( &dcp );

	// Obtain width and height for the front buffer surface
	IDirect3D8 *pD3D = NULL;
	m_pD3DDevice->GetDirect3D( &pD3D );
	D3DDISPLAYMODE dm;
	ZeroMemory ( &dm, sizeof ( dm ));
	pD3D->GetAdapterDisplayMode( dcp.AdapterOrdinal, &dm );

	// Create the front buffer surface
	IDirect3DSurface8 *pSurface;
	HRESULT hr = m_pD3DDevice->CreateImageSurface( dm.Width, 
												   dm.Height, 
												   D3DFMT_A8R8G8B8, 
												   &pSurface );
	if ( FAILED( hr ) )
	{
		return;
	}

	// Get a copy of the front buffer
	m_pD3DDevice->GetFrontBuffer( pSurface );

	// Obtain the rectangle into which rendering is drawn
	// It's the client rectangle of the focus window in screen coordinates
	RECT rc;
	GetClientRect( dcp.hFocusWindow, &rc );
	ClientToScreen( dcp.hFocusWindow, LPPOINT( &rc.left ) );
	ClientToScreen( dcp.hFocusWindow, LPPOINT( &rc.right ) );

	// Use D3DX to save the front buffer
	D3DXSaveSurfaceToFile( NewPath.c_str(), D3DXIFF_BMP, pSurface, 0, &rc );

	RELEASE_SAFELY( pSurface );

} // CGame::TakeScreenshot()

std::string CGame::SelectEmptyScreenshot( std::string Path )						// path to the music folder to look in
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
				if( Extension == "BMP" )
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
	return Path + "\\" + IntegerToString( FinalIndex ) + ".bmp";
} // CGame::SelectEmptyScreenshot()


CUserLocaleInfo CGame::theUserLocaleInfo;
std::wstring CGame::wlocaleString=L"";
std::string CGame::localeString = CGame::DetermineLocaleString();
HMODULE CGame::localizedDLL = CGame::DetermineLocaleDLL();


std::string CGame::DetermineLocaleString()
{
	CGame::wlocaleString=CGame::theUserLocaleInfo.GetLocaleName();
#ifdef _DEBUG
  int result=MessageBox(NULL,"Press \"Yes\" for en-US, en-UK, or fr-FR\nPress \"No\" for it-IT, de-DE, or es-ES\nPress \"Cancel\" for default locale detection.","Pick Localization...",MB_YESNOCANCEL);
  if (result==IDYES) {
    result=MessageBox(NULL,"Press \"Yes\" for en-US\nPress \"No\" for en-UK\nPress \"Cancel\" for fr-FR.","Pick Localization...",MB_YESNOCANCEL);
	if (result==IDYES){
      CGame::wlocaleString=L"en-US";
	}else if(result==IDNO){
      CGame::wlocaleString=L"en-UK";
	}else{
      CGame::wlocaleString=L"fr-FR";
	}
  }else if(result==IDNO){
    result=MessageBox(NULL,"Press \"Yes\" for it-IT\nPress \"No\" for de-DE\nPress \"Cancel\" for es-ES.","Pick Localization...",MB_YESNOCANCEL);
	if (result==IDYES){
      CGame::wlocaleString=L"it-IT";
	}else if(result==IDNO){
      CGame::wlocaleString=L"de-DE";
	}else{
      CGame::wlocaleString=L"es-ES";
	}
  }else{
	  //use default detection
  }

#endif
  //ws=L"it-IT";  //force to locale
  std::string s(CGame::wlocaleString.begin(),CGame::wlocaleString.end()); //convert from wstring
  s.assign(CGame::wlocaleString.begin(),CGame::wlocaleString.end());      //convert from wstring

#ifdef _DEBUG
  OutputDebugString("*********************\n*   GetLocaleName is: \"");
  OutputDebugString(s.c_str());
  OutputDebugString("\"\n*********************\n");
#endif

  if (s=="en-UK") return "en-UK";

  if(s.substr(0,2)=="fr") return "fr";
  else if (s.substr(0,2)=="it") return "it";
  else if (s.substr(0,2)=="de") return "de";
  else if (s.substr(0,2)=="es") return "es";
  else return "en-US";
}
HMODULE CGame::DetermineLocaleDLL()
{
  //std::wstring ws=CGame::theUserLocaleInfo.GetLocaleName();
  ////ws=L"it-IT";  //force to locale
  std::string s(CGame::wlocaleString.begin(),CGame::wlocaleString.end()); //convert from wstring
  s.assign(CGame::wlocaleString.begin(),CGame::wlocaleString.end());      //convert from wstring
  std::string strResourceLibToLoad;
  if (s=="en-UK"){ 
	  strResourceLibToLoad="en-UK";
  }else{
	  if (s.substr(0,2)=="fr") strResourceLibToLoad="fr-FR";
	  else if (s.substr(0,2)=="it") strResourceLibToLoad="it-IT";
	  else if (s.substr(0,2)=="de") strResourceLibToLoad="de-DE";
	  else if (s.substr(0,2)=="es") strResourceLibToLoad="es-ES";
	  else strResourceLibToLoad="en-US";
  }
    std::string tempstr = "./LANG/"+strResourceLibToLoad+".dll";

	return LoadLibrary(tempstr.c_str());
}

HMODULE	CGame::GetLangDLL()
{
	return localizedDLL;
}

std::string CGame::GetLocaleString()
{
	return localeString;
}

//Steam Overlay
void CGame::OnGameOverlayActivated( GameOverlayActivated_t *pCallback )
{
	if( m_pGameStateManager != NULL &&
		m_pGameStateManager->GameState() == KGameStateInGame &&
		m_pGameClient != NULL )
	{
		if( !m_pGameClient->Paused() )
		{
			m_pGameClient->Pause();
		}
	}
}