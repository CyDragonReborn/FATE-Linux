/*
	menumanager.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <assert.h>
#include <time.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

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

#include "../OBJECTS/datalight.h"

#include "../GAME/gamestatemanager.h"

#include "../GAMEOBJECTS/blurlayer.h"
#include "../GAMEOBJECTS/camera.h"
#include "../GAMEOBJECTS/lightmanager.h"

#include "../ITEM/itemtemplate.h"
#include "../ITEM/spelltemplate.h"
#include "../ITEM/item.h"

#include "../CHARACTER/character.h"
#include "../CHARACTER/player.h"
#include "../CHARACTER/monster.h"

#include "../SCENE/scene.h"

#include "menumanager.h"
#include "menu.h"
#include "SUBMENUS/mainmenu.h"
#include "SUBMENUS/newgamemenu.h"
#include "SUBMENUS/newpetmenu.h"
#include "SUBMENUS/ancestormenu.h"
#include "SUBMENUS/ancestorpetmenu.h"
#include "SUBMENUS/continuegamemenu.h"
#include "SUBMENUS/optionsmenu.h"
#include "SUBMENUS/intromenu.h"
#include "SUBMENUS/introquestmenu.h"
#include "SUBMENUS/logomenu.h"
#include "SUBMENUS/difficultymenu.h"
#include "SUBMENUS/ancestordifficultymenu.h"
#include "SUBMENUS/creditsmenu.h"
#include "SUBMENUS/retiremenu.h"


CMenuManager::CMenuManager( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						    CRefManager& pRefManager,					// pointer to the reference manager, for tracking media
						    CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
							CGameClient& pGameClient,					// game client
						    CGameStateManager& pGameStateManager,		// game manager
						    CSoundManager& pSoundManager ) :
							m_pRefManager( pRefManager ),
							m_pSettings( pSettings ),
							m_pGameClient( pGameClient ),
							m_pSoundManager( pSoundManager ),
							m_pGameStateManager( pGameStateManager ),
							m_pMousePointer( NULL ),
							m_pCamera( NULL ),
							m_pPlayer( NULL ),
							m_pBackgroundScene( NULL ),
							m_pMouseSurface( NULL ),
							m_pPet( NULL ),
							m_pPointerMaterial( NULL ),
							m_pPointerSelectMaterial( NULL ),
							m_pFontMaterial( NULL ),
							m_pFontHighlightMaterial( NULL ),
							m_pFontMetric( NULL ),
							m_pBlack( NULL ),
							m_IsDog( kTrue ),
							m_BlurTime( 0 ),
							m_CameraPath( -1 ),
							m_CameraPathPosition( 0 ),
							m_ActiveMenu( KLogoMenu ),
							m_ReloadPlayer( kTrue ),
							m_Finalized( kFalse ),
							m_pButtonList	( new CLList<CButton>() ),
							m_pTextList		( new CLList<CText>() ),
							m_pSpriteList	( new CLList<CSprite>() )
{
	ZeroMemory ( &m_KeyHandler,				sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,			sizeof( m_MouseHandler ));
	ZeroMemory ( &m_ShadowFrustum,			sizeof( m_ShadowFrustum ));
	ZeroMemory ( &m_ShadowViewMatrix,		sizeof( m_ShadowViewMatrix ));
	ZeroMemory ( &m_PreviewPosition,		sizeof( m_PreviewPosition ));
	ZeroMemory ( &m_PreviewOrientation,		sizeof( m_PreviewOrientation ));
	ZeroMemory ( &m_PetPreviewPosition,		sizeof( m_PetPreviewPosition ));
	ZeroMemory ( &m_PetPreviewOrientation,	sizeof( m_PetPreviewOrientation ));

	for( uint32 i = 0; i < KMenus; i++ )
	{
		m_pMenus[i] = NULL;
	}

	m_pPointerSelectMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															   "MENUS/pointerselect.png",
															   "MENUS/pointerselectalpha.png" );

	if( !m_pSettings.GetSettings( KSetCustomCursor ) )
	{
		m_pPointerMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															"MENUS/pointersmall.png",
															"MENUS/pointersmallalpha.png" );
		// make this a system texture now, for the windows cursor
		m_pPointerMaterial->ConvertToFormatSystem( pD3DDevice, 
												   D3DFMT_A8R8G8B8 );

		// grab the surface
		m_pPointerMaterial->GetTexture()->GetSurfaceLevel( 0, &m_pMouseSurface );
	}
	else
	{
		m_pPointerMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															"MENUS/pointer.png",
															"MENUS/pointeralpha.png" );
	}

	m_pMousePointer = new CSprite( pD3DDevice,
								   m_pPointerMaterial,
								   0,
								   0,
								   48,
								   48,
								   0,
								   0,
								   1,
								   1,
								   KSpriteTopLeft );


	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	// initialize font texture and metric
	m_pFontHighlightMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
																  "FONT/highlights.png",
																  "FONT/highlightsa.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );
	m_pFontMetric->SetKerning( -4 );
	m_pFontMetric->Pad( 4 );

	// create a camera
	m_pCamera = new CCamera( m_pSettings );

	m_pBlack = m_pRefManager.AddMaterial( pD3DDevice, "IMAGES//black.png" );

	CreateMenus( pD3DDevice );			   

} // CMenuManager::CMenuManager()

void CMenuManager::FinalizeCreation( LPDIRECT3DDEVICE8 pD3DDevice )				// direct3d device
{
	Reload( pD3DDevice );

	CreatePlayer( pD3DDevice, NULL, NULL );

	FindObjects();

	m_Finalized = kTrue;
} // CMenuManager::FinalizeCreation()

void CMenuManager::CreatePlayer( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								 CCharacterSaveInstance* pInstance,	// instance to copy over
								 CCharacterSaveInstance* pPetInstance,	// instance to copy over
								 EGender Gender,
								 int32 HeadIndex,
								 int32 HairIndex )
{
	DELETE_SAFELY( m_pPlayer );
	m_pPlayer = new CPlayer( pD3DDevice,
							 m_pRefManager,
							 m_pSettings,
							 *m_pGameClient.SpellManifest(),
							 *m_pGameClient.ParticleManager(),
							 *m_pGameClient.GlowManager(),
							 *m_pGameClient.PositionalAudioManager(),
							 Gender,
							 HeadIndex,
							 HairIndex );

	if( pInstance != NULL )
	{
		m_pPlayer->ApplyInstance( pD3DDevice,
								  NULL,
								  *m_pGameClient.CharacterManifest(),
								  *pInstance,
								  *GameClient().ItemManifest() );
	}

	m_pPlayer->SetPosition( m_PreviewPosition );
	m_pPlayer->SetOrientation( m_PreviewOrientation );



	CreatePet( pD3DDevice, pPetInstance, m_IsDog );

} // CMenuManager::CreatePlayer()

void CMenuManager::CreatePet( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							  CCharacterSaveInstance* pPetInstance,	// instance to copy over
							  bool IsDog )
{
	m_IsDog = IsDog;

	DELETE_SAFELY( m_pPet);
	CCharacterDescription* pDescription;

	if( pPetInstance != NULL )
	{
		pDescription = m_pGameClient.CharacterManifest()->CharacterDescription( pPetInstance->m_TemplateName );

		if( pDescription == NULL )
		{
			pDescription = m_pGameClient.CharacterManifest()->CharacterDescription( pPetInstance->m_OriginalTemplateName );
		}
	}
	else
	{
		if( IsDog )
		{
			pDescription = m_pGameClient.CharacterManifest()->CharacterDescription( "Terrier" );
		}
		else
		{
			pDescription = m_pGameClient.CharacterManifest()->CharacterDescription( LoadLocalizedString(1141) );
		}
	}

	m_pPet = new CMonster( pD3DDevice,
						   m_pRefManager,
						   m_pSettings,
						   *m_pGameClient.SpellManifest(),
						   *m_pGameClient.ParticleManager(),
						   *m_pGameClient.GlowManager(),
						   *m_pGameClient.PositionalAudioManager(),
						   *pDescription );

	if( pPetInstance != NULL )
	{
		m_pPet->ApplyInstance( pD3DDevice,
							   NULL,
							   *m_pGameClient.CharacterManifest(),
							   *pPetInstance,
							   *GameClient().ItemManifest() );
	}

	m_pPet->SetCollisionRadius( 1.2f );
	m_pPet->SetPosition( m_PetPreviewPosition );
	m_pPet->SetOrientation( m_PetPreviewOrientation );
} // CMenuManager::CreatePet()

CMenuManager::~CMenuManager( void )
{
	RELEASE_SAFELY( m_pMouseSurface );

	m_pRefManager.RemoveScene( m_pBackgroundScene );

	m_pRefManager.RemoveMaterial( m_pBlack );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	for( uint32 i = 0; i < KMenus; i++ )
	{
		DELETE_SAFELY( m_pMenus[i] );
	}

	DELETE_SAFELY( m_pCamera );

	DELETE_SAFELY( m_pPlayer );
	DELETE_SAFELY( m_pPet );

	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	DELETE_SAFELY( m_pMousePointer );
} // CMenuManager::~CMenuManager()

void CMenuManager::FindObjects( void )
{
	if( m_pBackgroundScene == NULL )
	{
		return;
	}
	m_pSoundManager.StopMusic();
	m_pSoundManager.Update();
	m_pSoundManager.StartMusic( "MUSIC\\title.ogg", kTrue );


	m_pGameClient.PositionalAudioManager()->Clear();
	m_pGameClient.ParticleManager()->Clear();
	m_pGameClient.GlowManager()->Clear();

	m_pGameClient.LightManager()->SetSceneAmbient( .5f, .5f, .5f );
	m_pGameClient.LightManager()->SetSceneSpecular( .5f, .5f, .5f );

	for( uint32 i = 0; i < m_pBackgroundScene->GetPathCount(); i++ )
	{
		CPath& Path = m_pBackgroundScene->GetPath( i );
		std::string Name = StringUpper( Path.Name() );
		if( Name == "CAMERAPATH" )
		{
			m_CameraPath = i;
		}
	}

	for( i = 0; i < m_pBackgroundScene->GetPointCount(); i++ )
	{
		CDataPoint& Point = m_pBackgroundScene->GetPoint( i );
		std::string Name = StringUpper( Point.Name() );
		std::string UserData = StringUpper( Point.UserData() );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pGameClient.ParticleManager()->GetParticleType( Point.UserData() );
			if( ParticleIndex != -1 )
			{
				D3DXVECTOR3 Position = Point.Position();
				m_ParticlePosition.push_back( Position );
				m_ParticleOrientation.push_back( Point.Orientation() );
				m_ParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGameClient.GlowManager()->GetGlowType( Point.UserData() );
			if( GlowIndex != -1 )
			{
				D3DXVECTOR3 Position = Point.Position();
				m_pGameClient.GlowManager()->AddGlow( GlowIndex, Position );
			}
		}
		else if( Name == "CAMERA" )
		{
			m_pCamera->SetPosition( m_pBackgroundScene->GetPoint( i ).Position() );
		}
		else if( Name == "TARGET" )
		{
			m_pCamera->SetLookTarget( m_pBackgroundScene->GetPoint( i ).Position() );
		}
		else if( Name == "PLAYER" )
		{
			m_PreviewPosition = m_pBackgroundScene->GetPoint( i ).Position();
			m_PreviewOrientation = m_pBackgroundScene->GetPoint( i ).Orientation();
			m_pPlayer->SetPosition( m_pBackgroundScene->GetPoint( i ).Position() );
			m_pPlayer->SetOrientation( m_pBackgroundScene->GetPoint( i ).Orientation() );
		}
		else if( Name == "PET" )
		{
			m_PetPreviewPosition = m_pBackgroundScene->GetPoint( i ).Position();
			m_PetPreviewOrientation = m_pBackgroundScene->GetPoint( i ).Orientation();
			m_pPet->SetPosition( m_pBackgroundScene->GetPoint( i ).Position() );
			m_pPet->SetOrientation( m_pBackgroundScene->GetPoint( i ).Orientation() );
		}
		else if( Name.length() >= 5 &&
				 Name.substr( 0, 5 ) == "SOUND" )
		{
			queue<std::string>	Tokens;
			std::string Path( "" );
			float32 Radius( 100 );
			uint32 BaseVolume( 255 );
			TokenizeString( &Tokens, UserData, "\n=\t\r", '#' );	

			// parse all user elements of the positional audio
			while( Tokens.size() > 0 )
			{
				std::string Key = Tokens.front();
				Tokens.pop();
				if( Key == "PATH" )	// path to sample
				{
					if( Tokens.size() > 0 )
					{
						Path = Tokens.front();
					}
				}
				else if( Key == "RADIUS" )	// max radius at which sound can be heard
				{
					if( Tokens.size() > 0 )
					{
						Radius = (float32) atof( Tokens.front().c_str() );
					}
				}
				else if( Key == "VOLUME" )	// base volume (0-255) of sound sample
				{
					if( Tokens.size() > 0 )
					{
						BaseVolume = atoi( Tokens.front().c_str() );
					}
				}
				if( Tokens.size() > 0 )
				{
					Tokens.pop();
				}
			}
			m_pGameClient.PositionalAudioManager()->AddAudio( Point.Position(),
															  Path,
															  BaseVolume,
															  Radius );
		}
	}

} // CMenuManager::FindParticles()

void CMenuManager::EmitParticles( void )
{
	for( uint32 i = 0; i < m_ParticleType.size(); i++ )
	{
		D3DXMATRIX Transform = m_ParticleOrientation[i];
		Transform._41 = m_ParticlePosition[i].x;
		Transform._42 = m_ParticlePosition[i].y;
		Transform._43 = m_ParticlePosition[i].z;
		m_pGameClient.ParticleManager()->AddParticle( m_ParticleType[i], Transform );
	}

} // CMenuManager::EmitParticles()

void CMenuManager::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	for( uint32 i = 0; i < KMenus; i++ )
	{
		if( m_pMenus[i] != NULL )
		{
			m_pMenus[i]->RestoreDeviceObjects( pD3DDevice );
		}
	}

	ResetMouse( pD3DDevice );
} // CMenuManager::RestoreDeviceObjects()

void CMenuManager::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device

{
	m_pMenus[KMainMenu] = new CMainMenu( m_pRefManager,
										 m_pSettings,
									     m_pGameStateManager,
										 *this );

	m_pMenus[KMainMenu]->Create( pD3DDevice );

	m_pMenus[KOptionsMenu] = new COptionsMenu( m_pRefManager,
											   m_pSettings,
											   m_pGameStateManager,
											   *this );

	m_pMenus[KOptionsMenu]->Create( pD3DDevice );

	m_pMenus[KNewGameMenu] = new CNewGameMenu( m_pRefManager,
											   m_pSettings,
											   m_pGameStateManager,
											   *this );

	m_pMenus[KNewGameMenu]->Create( pD3DDevice );

	m_pMenus[KNewPetMenu] = new CNewPetMenu( m_pRefManager,
											 m_pSettings,
											 m_pGameStateManager,
											 *this );

	m_pMenus[KNewPetMenu]->Create( pD3DDevice );

	m_pMenus[KAncestorMenu] = new CAncestorMenu( m_pRefManager,
											   m_pSettings,
											   m_pGameStateManager,
											   *this );

	m_pMenus[KAncestorMenu]->Create( pD3DDevice );

	m_pMenus[KAncestorPetMenu] = new CAncestorPetMenu( m_pRefManager,
											 m_pSettings,
											 m_pGameStateManager,
											 *this );

	m_pMenus[KAncestorPetMenu]->Create( pD3DDevice );

	m_pMenus[KContinueGameMenu] = new CContinueGameMenu( m_pRefManager,
														 m_pSettings,
														 m_pGameStateManager,
														 *this );

	m_pMenus[KContinueGameMenu]->Create( pD3DDevice );

	m_pMenus[KIntroMenu] = new CIntroMenu( m_pRefManager,
										   m_pSettings,
									  	   m_pGameStateManager,
										   m_pSoundManager,
										   *this );


	m_pMenus[KIntroMenu]->Create( pD3DDevice );

	m_pMenus[KLogoMenu] = new CLogoMenu( m_pRefManager,
										 m_pSettings,
									  	 m_pGameStateManager,
										 *this );
	m_pMenus[KLogoMenu]->Create( pD3DDevice );


	m_pMenus[KIntroQuestMenu] = new CIntroQuestMenu( m_pRefManager,
													m_pSettings,
									  				m_pGameStateManager,
													m_pSoundManager,
													*this );

	m_pMenus[KIntroQuestMenu]->Create( pD3DDevice );

	m_pMenus[KDifficultyMenu] = new CDifficultyMenu( m_pRefManager,
													m_pSettings,
									  				m_pGameStateManager,
													*this );

	m_pMenus[KDifficultyMenu]->Create( pD3DDevice );

	m_pMenus[KAncestorDifficultyMenu] = new CAncestorDifficultyMenu( m_pRefManager,
													m_pSettings,
									  				m_pGameStateManager,
													*this );

	m_pMenus[KAncestorDifficultyMenu]->Create( pD3DDevice );

	m_pMenus[KCreditsMenu] = new CCreditsMenu( m_pRefManager,
													m_pSettings,
									  				m_pGameStateManager,
													*this );

	m_pMenus[KCreditsMenu]->Create( pD3DDevice );

	m_pMenus[KRetireMenu] = new CRetireMenu( m_pRefManager,
											 m_pSettings,
									  		 m_pGameStateManager,
											 m_pSoundManager,
											 m_pGameClient,
											 *this );

	m_pMenus[KRetireMenu]->Create( pD3DDevice );

} // CMenuManager::CreateMenus()

void CMenuManager::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	m_pMenus[m_ActiveMenu]->KeyEvent( WMMessage, KeyCode );
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CMenuManager::KeyEvent()

void CMenuManager::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
							  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	m_pMenus[m_ActiveMenu]->MouseEvent( WMMessage, WMParam );

	m_MouseHandler.MouseEvent( WMMessage, WMParam );

} // CMenuManager::KeyEvent()

void CMenuManager::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );
	// esc requests a shutdown

	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	m_pMousePointer->SetPosition( (float32)MousePosition.x,
								  (float32)MousePosition.y,
								  1024,
								  768 );


} // CMenuManager::ProcessInput()

void CMenuManager::Update( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						  HWND hWnd,							// hwnd of the active window
						  float32 TimeElapsed )					// time elapsed, in seconds
{
	
	ProcessInput( hWnd );


	if( !m_MouseHandler.ButtonHeld( KButtonLeft ) )
	{
	}

	if( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
		m_MouseHandler.ButtonDoubleClick( KButtonLeft ) )
	{
	}
	else if( m_MouseHandler.ButtonHeld( KButtonLeft ) )
	{
	}

	// cap the elapsed time in case it's really large
	if( TimeElapsed > .3 )
	{
		TimeElapsed = .3f;
	}

	if( m_pPlayer != NULL )
	{
		m_pPlayer->UpdateAnimation( TimeElapsed );
	}
	if( m_pPet != NULL )
	{
		m_pPet->UpdateAnimation( TimeElapsed );
	}

	assert( m_pMenus[m_ActiveMenu] != NULL );
	m_pMenus[m_ActiveMenu]->Update( hWnd,
									TimeElapsed );


	if( m_CameraPath != -1 )
	{
		m_CameraPathPosition += TimeElapsed * .25f;
		m_pCamera->SetPosition( m_pBackgroundScene->GetPath( m_CameraPath ).GetSplinePositionAtDistance( m_CameraPathPosition ) );
	}

	// we want to update the camera every frame, whether we move it or not,
	// because something about the projection matrix might have changed
	m_pCamera->UpdateFixed( TimeElapsed );

	if( m_Finalized )
	{
		// update all particle systems
		m_pGameClient.ParticleManager()->Update( TimeElapsed );

		// update all glow systems
		m_pGameClient.GlowManager()->Update( TimeElapsed );

		// update all positional audio samples
		m_pGameClient.PositionalAudioManager()->Update( m_pCamera->Position(),
														m_pCamera->Orientation(),
														TimeElapsed );
	}

	m_BlurTime += TimeElapsed;

	m_MouseHandler.Flush();
	m_KeyHandler.Flush();

} // CMenuManager::Update()


void CMenuManager::PreRender( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if( m_pMenus[m_ActiveMenu] != NULL &&
		!m_pMenus[m_ActiveMenu]->Has3DBackdrop() )
	{
		return;
	}

	D3DXVECTOR3 Target = m_PreviewPosition;
	D3DXVECTOR3 Start = m_PreviewPosition;
	D3DXVECTOR3 Up( 0, 0, 1 );
	Start.y += 35;

	D3DXMatrixLookAtLH( &m_ShadowViewMatrix, &Start, &Target, &Up );

	
	RenderShadows( pD3DDevice, m_ShadowViewMatrix );

	D3DXMatrixLookAtLH( &m_ShadowViewMatrix, &Start, &Target, &Up );

	RenderBlurLayer( pD3DDevice );

} // CMenuManager::PreRender()

void CMenuManager::Render( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if( m_pMenus[m_ActiveMenu] != NULL &&
		!m_pMenus[m_ActiveMenu]->Has3DBackdrop() )
	{
		return;
	}

	if( !m_Finalized )
	{
		return;
	}

	if( m_ReloadPlayer )
	{
		( (CContinueGameMenu*)m_pMenus[KContinueGameMenu] )->ReloadProfiles();
		( (CContinueGameMenu*)m_pMenus[KContinueGameMenu] )->UpdatePlayer( pD3DDevice );
		m_ReloadPlayer = kFalse;
	}

	// set  our projection to the camera's projection matrix
	pD3DDevice->SetTransform( D3DTS_PROJECTION, &m_pCamera->ProjectionMatrix() );

	// turn on lighting for objects
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );


	m_pBackgroundScene->Sort( m_pCamera->Frustum() );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	if( m_pBackgroundScene != NULL )
	{
		m_pBackgroundScene->RenderSorted( pD3DDevice,
										m_pCamera->ViewMatrix() );

		if( m_pGameClient.RenderedShadow() != NULL &&
			m_pSettings.GetSettings( KSetProjectedShadows ) &&
			m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
		{
				pD3DDevice->SetRenderState( D3DRS_ZBIAS, 16 );
				pD3DDevice->SetTexture( 0, m_pGameClient.RenderedShadow()->GetDynamicTexture() );
				m_pBackgroundScene->RenderShadowPass( pD3DDevice, 
													m_pCamera->ViewMatrix(), 
													m_ShadowViewMatrix,
													NULL, 
													kTrue );
				pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );
		}
	}

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

	if( m_pPlayer != NULL )
	{
		m_pPlayer->EmitParticles();
		m_pPlayer->Render( pD3DDevice,
						   m_pGameClient.LightManager(),
						   m_pCamera->ViewMatrix(),
						   &m_pCamera->Frustum() );
	}
	if( m_pPet != NULL )
	{
		// in case the pet is in town in the savegame, we still want to make
		// sure it gets rendered
		m_pPet->SetAIState( KAIInactive );
		m_pPet->EmitParticles();
		m_pPet->Render( pD3DDevice,
			 		    m_pGameClient.LightManager(),
						m_pCamera->ViewMatrix(),
						&m_pCamera->Frustum() );
	}

	// lighting back off
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	m_pGameClient.ParticleManager()->Render( pD3DDevice,
											 m_pCamera->ViewMatrix(),
											 &m_pCamera->Frustum() );

	m_pGameClient.GlowManager()->Render( pD3DDevice,
										 m_pCamera->ViewMatrix(),
										 &m_pCamera->Frustum() );

	// now render the blur layer on top of everything else
	if( m_pSettings.GetSettings( KSetMotionBlur ) )
	{
		m_pGameClient.BlurLayer()->RenderBlur( pD3DDevice );
	}

	assert( m_pMenus[m_ActiveMenu] != NULL );
	m_pMenus[m_ActiveMenu]->Render( pD3DDevice );

} // CMenuManager::Render()

void CMenuManager::RenderBlurLayer( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( m_pMenus[m_ActiveMenu] != NULL &&
		!m_pMenus[m_ActiveMenu]->Has3DBackdrop() )
	{
		return;
	}

	if( !m_pSettings.GetSettings( KSetMotionBlur ) )
	{
		return;
	}

	if( !m_pGameClient.BlurLayer()->IsValid() )
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

	// render our lights to the blur layer
	m_pGameClient.BlurLayer()->BeginRender( pD3DDevice );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );

	if( m_Finalized )
	{
		if( m_pBackgroundScene != NULL )
		{
			m_pBackgroundScene->RenderGlowPass( pD3DDevice, m_pCamera->ViewMatrix()  );
		}

		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );


		m_pGameClient.ParticleManager()->RenderAdditive( pD3DDevice,
														m_pCamera->ViewMatrix(),
														&m_pCamera->Frustum() );

		m_pGameClient.GlowManager()->RenderAdditive( pD3DDevice,
													m_pCamera->ViewMatrix(),
													&m_pCamera->Frustum() );



		m_pGameClient.BlurLayer()->EndRender( pD3DDevice );
	}
} // CMenuManager::RenderBlurLayer()

void CMenuManager::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								 const D3DXMATRIX& ViewportMatrix )			// viewport matrix
{
	if( !m_Finalized )
	{
		return;
	}
	if( m_pMenus[m_ActiveMenu] != NULL &&
		!m_pMenus[m_ActiveMenu]->Has3DBackdrop() )
	{
		return;
	}

	if( !m_pSettings.GetSettings( KSetProjectedShadows ) )
	{
		return;
	}
	if( m_pSettings.GetSettings( KSetShadowDetail ) < 1 )
	{
		return;
	}
	// no shadow texture, no rendering
	if( m_pGameClient.RenderedShadow() == NULL )
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

	D3DXMatrixOrthoLH ( &ProjectionMatrix, 
						50,
						50,
						.01f,
						2000 );	// far clip

	// start rendering to our s texture
	m_pGameClient.RenderedShadow()->BeginRender();

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

	if( m_pPlayer != NULL )
	{
		m_pPlayer->RenderShadow( pD3DDevice, 
								 ViewportMatrix,
								 &m_pCamera->Frustum() );
	}

	if( m_pPet != NULL )
	{
		m_pPet->RenderShadow( pD3DDevice, 
							  ViewportMatrix,
							  &m_pCamera->Frustum() );
	}

//	RenderCharacterShadows( ViewportMatrix,
//							&m_ShadowFrustum );

	pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );

	m_ShadowFrustum.Update( ViewportMatrix,
							 ProjectionMatrix );

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

	m_pGameClient.RenderedShadow()->EndRender();

} // CMenuManager::RenderShadows()

void CMenuManager::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	assert( m_pMenus[m_ActiveMenu] != NULL );
	m_pMenus[m_ActiveMenu]->RenderUI( pD3DDevice );

	if( m_pSettings.GetSettings( KSetCustomCursor ) )
	{
		m_pMousePointer->Render( pD3DDevice );
	}

} // CMenuManager::RenderUI()

void CMenuManager::Reload( LPDIRECT3DDEVICE8 pD3DDevice )				// direct3d device
{
	if( m_pBackgroundScene == NULL )
	{
		m_pBackgroundScene = m_pRefManager.AddScene( pD3DDevice,
													"TOWN//splash.scn",
													"",
													kFalse,
													kFalse,
													KSortQuadtree );
	}

	ResetMouse( pD3DDevice );
} // CMenuManager::Reload()

void CMenuManager::ResetMouse( LPDIRECT3DDEVICE8 pD3DDevice )				// direct3d device
{
	if( !m_pSettings.GetSettings( KSetCustomCursor ) )
	{
		SetCursor( LoadCursor( NULL, IDC_ARROW ) );
	}
} // CMenuManager::ResetMouse()


void CMenuManager::Unload( void )
{
	m_pRefManager.RemoveScene( m_pBackgroundScene );
	m_pBackgroundScene = NULL;
} // CMenuManager::Unload()

//  perform any menu activation functions for the active menu
void CMenuManager::Activate( void )
{
	assert( m_pMenus[m_ActiveMenu] != NULL );
	m_pMenus[m_ActiveMenu]->Activate();

	// flush our keyboard & mouse handlers
	m_KeyHandler.FlushAll();
	m_MouseHandler.FlushAll();

} // CMenuManager::Activate( void )

void CMenuManager::Deactivate( void )
{
	assert( m_pMenus[m_ActiveMenu] != NULL );
	m_pMenus[m_ActiveMenu]->Deactivate();

	m_pRefManager.RemoveUnused();

} // CMenuManager::Deactivate( void )

void CMenuManager::SetActiveMenu( EMenu ActiveMenu )	// menu to activate
{
	Deactivate();
	m_ActiveMenu = ActiveMenu;
	Activate();
} // CMenuManager::SetActiveMenu()
