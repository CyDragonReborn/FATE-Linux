/*
	fatemenu.cpp

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

#include "../LEVEL/levelhistory.h"
#include "../CHARACTER/character.h"
#include "../ITEM/item.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "gameclient.h"

#include "../LEVEL/level.h"

#include "../GAMEOBJECTS/camera.h"

#include "../GAMEOBJECTS/genericmodel.h"
#include "../GAMEOBJECTS/lightmanager.h"

#include "../PARTICLES/particlemanager.h"
#include "../GLOWS/glowmanager.h"
#include "../GLOWS/glowsystem.h"

#include "gameui.h"
#include "fatemenu.h"
#include "../GAMEOBJECTS/screenfade.h"

#include "demorules.h" //marsh

CFateMenu::CFateMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					  CRefManager& pRefManager,					// the reference manager, for tracking media we create
					  CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
				  	  CParticleManager& pParticleManager,		// particle manager
					  CGlowManager& pGlowManager,				// glow manager
					  CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																m_pGameStateManager( pGameStateManager ),
																m_pRefManager( pRefManager ),
																m_pSettings( pSettings ),
																m_pButtonList	( new CLList<CButton>() ),
																m_pTextList		( new CLList<CText>() ),
																m_pSpriteList	( new CLList<CSprite>() ),
																m_pGlowManager( pGlowManager ),
																m_pParticleManager( pParticleManager ),
																m_pCamera( NULL ),
																m_pFateModel( NULL ),
																m_pScreenFade( NULL ),
																m_FadeValue( 255 ),
																m_FadeDirection( 0 ),
																m_RequestingQuit( kFalse ),
																m_Open( kFalse ),
																m_pFontMaterial( NULL ),
																m_pFontMetric( NULL ),
																m_pPlayerCharacter( NULL ),
																m_pUIButtons( NULL ),
																m_pChoice1Text( NULL ),
																m_pChoice2Text( NULL ),
																m_pChoice3Text( NULL ),
																m_pChoice1Button( NULL ),
																m_pChoice2Button( NULL ),
																m_pChoice3Button( NULL ),
																m_pClickSound( NULL ),
																m_pFateSound( NULL ),
																m_Message( "" ),
																m_FameLoss1( 0 ),
																m_ExperienceLoss1( 0 ),
																m_GoldLoss1( 0 ),
																m_FameLoss2( 0 ),
																m_ExperienceLoss2( 0 ),
																m_GoldLoss2( 0 ),
																m_FameLoss3( 0 ),
																m_ExperienceLoss3( 0 ),
																m_GoldLoss3( 0 ),
																m_FateSoundChannel( 0 )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));
	ZeroMemory ( &m_DisplayMatrix,  sizeof( m_DisplayMatrix ));

	m_pScreenFade = new CScreenFade( pD3DDevice,
									 pSettings );
	m_pScreenFade->SetColor( 0, 0, 0, 1 );

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	// create a camera
	m_pCamera = new CCamera( m_pSettings );

	m_pCamera->SetPosition( D3DXVECTOR3( 0, -83, 0 ) );
	m_pCamera->SetLookTarget( D3DXVECTOR3( 0, -95, -25 ) );
	m_pCamera->UpdateFixed( 0 );

	D3DXMatrixTranslation( &m_DisplayMatrix,
						   0,
						   -100,
						   -25 );


	CreateMenus( pD3DDevice );

	m_pFateModel = new CGenericModel( pD3DDevice,
									  m_pRefManager,
									  m_pSettings,
									  "MONSTERS/FATE/fate.sms",
									  "" );

	m_pFateModel->PlayAnimation( "IDLE", kTrue );

} // CFateMenu::CFateMenu()

CFateMenu::~CFateMenu( void )
{
	DELETE_SAFELY( m_pScreenFade );

	DELETE_SAFELY( m_pFateModel );
	DELETE_SAFELY( m_pCamera );

	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );
	m_pRefManager.RemoveSample( m_pFateSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	ClearParticles();


} // CFateMenu::~CFateMenu()

void CFateMenu::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pScreenFade->RestoreDeviceObjects( pD3DDevice, m_pSettings );
} // CFateMenu::RestoreDeviceObjects()

void CFateMenu::ClearParticles( void )
{
	for( uint32 i = 0; i < m_pGlowList.size(); i++ )
	{
		m_pGlowList[i]->m_RequestingRemoval = kTrue;
	}
	m_pGlowList.clear();
	for( i = 0; i < m_pModelGlowList.size(); i++ )
	{
		m_pModelGlowList[i]->m_RequestingRemoval = kTrue;
	}
	m_pModelGlowList.clear();

	m_GlowIndex.clear();
	m_GlowPosition.clear();
	m_ParticlePosition.clear();
	m_ParticleType.clear();
	m_TagParticleIndex.clear();
	m_TagParticleType.clear();
} // CFateMenu::ClearParticles()

void CFateMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CFateMenu::FlushAll()

void CFateMenu::FindParticles( void )
{
	for( uint32 i = 0; i < m_pFateModel->GetPointCount(); i++ )
	{
		CDataPoint& Point = m_pFateModel->GetPoint( i );
		std::string Name = StringUpper( Point.Name() );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( Point.UserData() );
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
			int32 GlowIndex = m_pGlowManager.GetGlowType( Point.UserData() );
			D3DXVECTOR3 Position = Point.Position();
			if( GlowIndex != -1 )
			{
				m_GlowPosition.push_back( Position );
				m_pModelGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, Point.Position() ) );					
			}
		}
	}

	for( i = 0; i < m_pFateModel->GetTagCount(); i++ )
	{
		std::string Name = StringUpper( m_pFateModel->GetTagName( i ) );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( m_pFateModel->GetTagUserData( i )  );
			if( ParticleIndex != -1 )
			{
				m_TagParticleIndex.push_back( i );
				m_TagParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGlowManager.GetGlowType( m_pFateModel->GetTagUserData( i ) );
			if( GlowIndex != -1 )
			{
				m_pGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, D3DXVECTOR3( 0, -10000, 0 ) ) );					
				m_GlowIndex.push_back( i );
			}
		}
	}
} // CFateMenu::FindParticles()

void CFateMenu::EmitParticles( void )
{

	for( uint32 i = 0; i < m_ParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position = m_ParticlePosition[i];
		D3DXVec3TransformCoord( &Position,  &m_ParticlePosition[i], &m_DisplayMatrix );
		D3DXMATRIX Transform = m_ParticleOrientation[i];
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pParticleManager.AddParticle( m_ParticleType[i], Transform );
	}

	for( i = 0; i < m_TagParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXMATRIX Transform = m_pFateModel->GetTransformedTag( m_TagParticleIndex[i] );
		Transform *= m_DisplayMatrix;
		m_pParticleManager.AddParticle( m_TagParticleType[i], Transform );
	}

	for( i = 0; i < m_pGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position = m_pFateModel->GetTagPosition( m_GlowIndex[i] );
		D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
		m_pGlowList[i]->m_Position = Position;
	}
	for( i = 0; i < m_pModelGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position = m_GlowPosition[i];
		D3DXVec3TransformCoord( &Position,  &m_GlowPosition[i], &m_DisplayMatrix );
		D3DXMATRIX Transform = m_ParticleOrientation[i];
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pModelGlowList[i]->m_Position = Position;
	}

} // CFateMenu::EmitParticles()

void CFateMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pFateSound = m_pRefManager.AddSample( "SOUNDS/AMBIENT/fate.wav", kTrue );

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/buttons.png",
												   "MENUS/buttonsa.png" );

	CButton* pButton;

// choice buttons

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									136,
									512,
									148,
									48,
									108 * m_pUIButtons->PixelToU(),
									145 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									193 * m_pUIButtons->PixelToV(),
									108 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    145 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonFate1 ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(174),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						  LoadLocalizedString(174),
						  m_pFontMaterial,
						  m_pFontMetric,
						  0,
						  -15,
						  1.0f,
						  kFalse,
						  KJustifyCenter );

	m_pChoice1Button = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									136,
									597,
									148,
									48,
									108 * m_pUIButtons->PixelToU(),
									145 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									193 * m_pUIButtons->PixelToV(),
									108 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    145 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonFate2 ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(174),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						  LoadLocalizedString(174),
						  m_pFontMaterial,
						  m_pFontMetric,
						  0,
						  -15,
						  1.0f,
						  kFalse,
						  KJustifyCenter );

	m_pChoice2Button = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									136,
									682,
									148,
									48,
									108 * m_pUIButtons->PixelToU(),
									145 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									193 * m_pUIButtons->PixelToV(),
									108 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    145 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonFate3 ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(174),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						  LoadLocalizedString(174),
						  m_pFontMaterial,
						  m_pFontMetric,
						  0,
						  -15,
						  1.0f,
						  kFalse,
						  KJustifyCenter );

	m_pChoice3Button = pButton;


	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(175),
						512,
						64,
						1,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(176),
						512,
						100,
						1,
						kFalse,
						KJustifyCenter ) );

	m_pChoice1Text = AddText( new CText( pD3DDevice,
										 m_pFontMaterial,
										 m_pFontMetric,
										 LoadLocalizedString(177),
										 255,
										 487,
										 0.9f,  // TDC 2008-03-05 Craig Miles
										 kFalse,
										 KJustifyLeft ) );
	m_pChoice1Text->EnableWrapping( pD3DDevice, 700 );

	m_pChoice2Text = AddText( new CText( pD3DDevice,
										 m_pFontMaterial,
										 m_pFontMetric,
										 LoadLocalizedString(178),
										 255,
										 572,
										 0.9f,  // TDC 2008-03-05 Craig Miles
										 kFalse,
										 KJustifyLeft ) );

	m_pChoice2Text->EnableWrapping( pD3DDevice, 700 );

	m_pChoice3Text = AddText( new CText( pD3DDevice,
										 m_pFontMaterial,
										 m_pFontMetric,
										 LoadLocalizedString(179),
										 255,
										 657,
										 0.9f,  // TDC 2008-03-05 Craig Miles
										 kFalse,
										 KJustifyLeft ) );
	m_pChoice3Text->EnableWrapping( pD3DDevice, 700 );

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									940,
									730,
									148,
									48,
									108 * m_pUIButtons->PixelToU(),
									145 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									193 * m_pUIButtons->PixelToV(),
									108 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    145 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonQuitNoSave ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(180),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						  LoadLocalizedString(180),
						  m_pFontMaterial,
						  m_pFontMetric,
						  0,
						  -15,
						  1.0f,
						  kFalse,
						  KJustifyCenter );

} // CFateMenu::CreateMenus()

void CFateMenu::SetOpen( bool Open )	// open state
{
	if( !Open )
	{
		if( m_FateSoundChannel != -1 )
		{
			FSOUND_StopSound( m_FateSoundChannel );
		}
		ClearParticles();
	}
	m_Open = Open;
} // CFateMenu::SetOpen()

void CFateMenu::Open( LPDIRECT3DDEVICE8 pD3DDevice,				// d3d device
					  CLevel& pLevel )							// active level
{
	m_RequestingQuit = kFalse;
	m_FadeValue = 255;
	m_FadeDirection = -1;
	m_FateSoundChannel = FSOUND_PlaySound( FSOUND_FREE, m_pFateSound );

	FindParticles();
	m_Open = kTrue;

	m_FameLoss2 = 0;
	m_ExperienceLoss2 = 0;
	m_GoldLoss2 = m_pPlayerCharacter->Inventory()->Gold() / 10;

	m_ExperienceLoss1 = ( ExperienceGate( m_pPlayerCharacter->Level() ) - ExperienceGate( m_pPlayerCharacter->Level() - 1 ) ) / 10;
	m_FameLoss1 = ( KFameGate[m_pPlayerCharacter->FameRank()] - KFameGate[m_pPlayerCharacter->FameRank() - 1] ) / 10;
	m_GoldLoss1 = 0;

	if( m_ExperienceLoss1 > m_pPlayerCharacter->Experience() )
	{
		m_ExperienceLoss1 = m_pPlayerCharacter->Experience();
	}

	if( m_FameLoss1 > m_pPlayerCharacter->Fame() )
	{
		m_FameLoss1 = m_pPlayerCharacter->Fame();
	}

	m_FameLoss3 = 0;
	m_ExperienceLoss3 = 0;
	m_GoldLoss3 = 0;

	m_pChoice1Text->SetText( pD3DDevice, LoadLocalizedString2Variables(181, IntegerToString( m_ExperienceLoss1 ), IntegerToString( m_FameLoss1 ) ) );
	m_pChoice2Text->SetText( pD3DDevice, LoadLocalizedString1Variable(182, IntegerToString( m_GoldLoss2 )) );
	if( pLevel.LevelDepth() > 3 )
	{
		m_pChoice3Text->SetText( pD3DDevice, LoadLocalizedString(183) );
	}
	else
	{
		m_pChoice3Text->SetText( pD3DDevice, LoadLocalizedString(184) );
	}
	FlushAll();
} // CFateMenu::Open()

void CFateMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
						  WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CFateMenu::KeyEvent()

void CFateMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CFateMenu::KeyEvent()

bool CFateMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}
	return kTrue;
} // CFateMenu::MouseOver()

void CFateMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CFateMenu::ProcessInput()

void CFateMenu::Update( HWND hWnd,							// hwnd of the active window
					    CLevel& pLevel,						// active level
					    float32 TimeElapsed )				// time elapsed, in seconds
{
	if( !Open() )
	{
		return;
	}


	int32 Volume = m_pSettings.GetSettings( KSetSoundFXVolume ) - 75;
	if( Volume < 0 )
	{
		Volume = 0;
	}
	FSOUND_SetSFXMasterVolume( Volume );

	// so that the help audio doesn't mute it
	if( m_FateSoundChannel != -1 )
	{
		FSOUND_SetVolumeAbsolute( m_FateSoundChannel, m_pSettings.GetSettings( KSetSoundFXVolume ) );
	}
	m_pPlayerCharacter->UpdateHelp( 0 );

	if( m_FadeDirection != 0 )
	{
		m_FadeValue += m_FadeDirection * TimeElapsed * 100;
		if( m_FadeValue > 255 )
		{
			m_FadeValue = 255;
			m_FadeDirection = 0;
			SetOpen( kFalse );
			if( m_RequestingQuit )
			{
				m_pGameStateManager.SetTargetMenu( KMainMenu );
			}
		}
		else if( m_FadeValue < 0 )
		{
			m_pPlayerCharacter->PlayHelpSample( KSoundHelpChooseFate, kTrue );
			m_FadeValue = 0;
			m_FadeDirection = 0;
		}
	}

	m_pFateModel->UpdateAnimation( TimeElapsed );

	EmitParticles();

	ProcessInput( hWnd );


	UpdateButtons( TimeElapsed );

	// left mouse button first
	int32 ButtonPressed = -1;
	if( m_FadeDirection <= 0 )
	{
		ButtonPressed = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
									  	   m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );
	}

	int32 RandomDirection = RandomIntegerBetween( 1, 2 );
	if( RandomIntegerBetween( 0, 100 ) < 50 &&
		pLevel.LevelDepth() > 2 )
	{
		RandomDirection *= -1;
	}

	if( !IsGameUnlocked() && //marsh - just to see if we are in demo mode - if so make sure we go up if we are on the last level in the dungeon
		pLevel.LevelDepth() >= KDemoDungeonLevel ) //DEMO MODE
	{
		RandomDirection = -1;
	}

	bool AddSFX( kFalse );
	switch( ButtonPressed )
	{
	case KButtonFate1 :
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpFateChosen, kTrue );
		m_pPlayerCharacter->SetAIState( KAIInactive );
		m_pPlayerCharacter->ClearEffects();
		m_pPlayerCharacter->ModifyHP( (float32)m_pPlayerCharacter->MaxHP() );
		m_pPlayerCharacter->Pet( 0 )->ModifyHP( (float32)m_pPlayerCharacter->Pet( 0 )->MaxHP() );
		m_pPlayerCharacter->ModifyMana( (float32)m_pPlayerCharacter->MaxMana() );
		m_pPlayerCharacter->ModifyStamina( (float32)m_pPlayerCharacter->MaxStamina() );
		m_pPlayerCharacter->AwardExperience( pLevel, NULL, -m_ExperienceLoss1 );
		m_pPlayerCharacter->AwardFame( pLevel, NULL, -m_FameLoss1 );
		m_pPlayerCharacter->TakeGold( m_GoldLoss1 );
		m_FadeDirection = 1;
		AddSFX = kTrue;
		break;
	case KButtonFate2 :		
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpFateChosen, kTrue );
		m_pPlayerCharacter->SetAIState( KAIInactive );
		m_pPlayerCharacter->ClearEffects();
		m_pPlayerCharacter->ModifyHP( (float32)m_pPlayerCharacter->MaxHP() );
		m_pPlayerCharacter->Pet( 0 )->ModifyHP( (float32)m_pPlayerCharacter->Pet( 0 )->MaxHP() );
		m_pPlayerCharacter->ModifyMana( (float32)m_pPlayerCharacter->MaxMana() );
		m_pPlayerCharacter->ModifyStamina( (float32)m_pPlayerCharacter->MaxStamina() );
		m_pPlayerCharacter->AwardExperience( pLevel, NULL, -m_ExperienceLoss2 );
		m_pPlayerCharacter->AwardFame( pLevel, NULL, -m_FameLoss2 );
		m_pPlayerCharacter->TakeGold( m_GoldLoss2 );
		pLevel.RequestLevelChange( RandomDirection, kTrue );
		m_FadeDirection = 1;
		break;
	case KButtonFate3 :
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpFateChosen, kTrue );
		m_pPlayerCharacter->SetAIState( KAIInactive );
		m_pPlayerCharacter->ClearEffects();
		m_pPlayerCharacter->ModifyHP( (float32)m_pPlayerCharacter->MaxHP() );
		m_pPlayerCharacter->Pet( 0 )->ModifyHP( (float32)m_pPlayerCharacter->Pet( 0 )->MaxHP() );
		m_pPlayerCharacter->ModifyMana( (float32)m_pPlayerCharacter->MaxMana() );
		m_pPlayerCharacter->ModifyStamina( (float32)m_pPlayerCharacter->MaxStamina() );		
		m_pPlayerCharacter->AwardExperience( pLevel, NULL, -m_ExperienceLoss3 );
		m_pPlayerCharacter->AwardFame( pLevel, NULL, -m_FameLoss3 );
		m_pPlayerCharacter->TakeGold( m_GoldLoss3 );
		m_pPlayerCharacter->Inventory()->DropGold( pLevel );
		pLevel.LevelHistory()->SetLastPopulationTime( m_pPlayerCharacter->ExistenceTime() );
		if( m_pPlayerCharacter->GetLevelHistory( pLevel.LevelDepth() ) != NULL )
		{
			m_pPlayerCharacter->GetLevelHistory( pLevel.LevelDepth() )->SetLastPopulationTime( m_pPlayerCharacter->ExistenceTime() );
		}
		pLevel.RequestLevelChange( -3, kTrue );
		m_FadeDirection = 1;
		break;
	case KButtonQuitNoSave :
		m_pPlayerCharacter->PlayHelpSample( KSoundHelpFateChosen, kTrue );
		m_FadeDirection = 1;
		m_RequestingQuit = kTrue;
		break;
	}

	if( AddSFX )
	{
		CSFXDescription* pSFX = new CSFXDescription( KSFXFollowTarget,
												     "SPELLS\\EFFECTS\\HEAL\\heal.ams",
												     -1,
												     kFalse,
													 0,
													 0,
													 kFalse,
													 kFalse );
												   
		pLevel.AddSFX( *pSFX,
					   m_pPlayerCharacter,
					   m_pPlayerCharacter,
					   m_pPlayerCharacter->Position(),
					   m_pPlayerCharacter->Position() );
		DELETE_SAFELY( pSFX );
	}

	// update all particle systems
	m_pParticleManager.Update( TimeElapsed );

	// update all glow systems
	m_pGlowManager.Update( TimeElapsed );

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CFateMenu::Update()

// render any 3d elements
void CFateMenu::Render3D( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
 						  CGameClient& pClient )				// game client

{
	if( !Open() )
	{
		return;
	}

	pD3DDevice->SetTransform( D3DTS_PROJECTION, &m_pCamera->ProjectionMatrix() );

	pClient.LightManager()->UpdateLights( pD3DDevice, m_DisplayMatrix );

	m_pFateModel->Render( pD3DDevice,
						  m_DisplayMatrix * m_pCamera->ViewMatrix() );

	// lighting back off
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	m_pParticleManager.Render( pD3DDevice,
								m_pCamera->ViewMatrix(),
								&m_pCamera->Frustum() );

	m_pGlowManager.Render( pD3DDevice,
							m_pCamera->ViewMatrix(),
							&m_pCamera->Frustum() );

} // CFateMenu::Render3D()

// render any 3d elements
void CFateMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						 const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( !Open() )
	{
		return;
	}

	m_pScreenFade->SetColor( 0, 0, 0, m_FadeValue / 255.0f );

	m_pScreenFade->Render( pD3DDevice );

	if( m_FadeDirection > 0 )
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		// no zwrite or ztest while rendering text
		pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
		pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
		return;
	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// no zwrite or ztest while rendering text
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	RenderSprites( pD3DDevice );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	RenderButtons( pD3DDevice );


	RenderText( pD3DDevice );		

} // CFateMenu::Render()


void CFateMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CFateMenu::RenderText()

void CFateMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CFateMenu::RenderSprites()

void CFateMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CFateMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CFateMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CFateMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CFateMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CFateMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CFateMenu::UpdateButtonOver( void )
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
} // CFateMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CFateMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CFateMenu::UpdateButtonOver()

CButton* CFateMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CFateMenu::AddButton()

void CFateMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CFateMenu::RemoveButton()

void CFateMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CFateMenu::DestroyButtons()

CText* CFateMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CFateMenu::AddText()

void CFateMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CFateMenu::RemoveText()

void CFateMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CFateMenu::DestroyText()

CSprite* CFateMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CFateMenu::AddSprite()

CSprite* CFateMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CFateMenu::AddSpriteFront()

void CFateMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CFateMenu::RemoveSprite()

void CFateMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CFateMenu::DestroySprite()


