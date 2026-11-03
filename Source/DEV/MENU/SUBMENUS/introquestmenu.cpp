/*
	introquestmenu.cpp

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
#include "../../UTILITIES/vistafileutilities.h"

#include "introquestmenu.h"

#include "../../GAMECLIENT/quest.h"
#include "../../GAMECLIENT/questdescriptions.h"
#include "../../GAMEOBJECTS/screenfade.h"
#include "../../MEDIA/soundmanager.h"
#include "../../NAMES/namegenerator.h"

CIntroQuestMenu::CIntroQuestMenu( CRefManager& pRefManager,				// the reference manager, for tracking media we create
					    CSettings& pSettings,					// the settings manager, for keeping track of card/machine capabilities
					    CGameStateManager& pGameStateManager,	// game manager
					    CSoundManager& pSoundManager,			// sound manager
					    CMenuManager& pMenuManager ) :			// menu manager
														CMenu( pRefManager,
																pSettings,
																pGameStateManager,
																pMenuManager ),
														m_pIntroMaterial( NULL ),
														m_pSoundManager( pSoundManager ),
														m_pFontMetric( NULL ),
														m_pButtonMaterial( NULL ),
														m_pFontMaterial( NULL ),
														m_pScreenFade( NULL ),
														m_pQuest( NULL ),
														m_SpriteYPosition( 344 ),
														m_FadeValue ( 0.0f ),
														m_FadeDirection ( 0.0f ),
														m_pClickSound( NULL ),
														m_pIntroMaterial2( NULL ),
														m_pGradientMaterial( NULL ),
														m_pIntro1( NULL ),
														m_pIntro2( NULL ),
														m_pContinueButton( NULL ),
														m_pText( NULL )
{

} // CIntroQuestMenu::CIntroQuestMenu()

CIntroQuestMenu::~CIntroQuestMenu( void )
{
	DELETE_SAFELY( m_pQuest );
	RemoveButton( m_pContinueButton );
	DELETE_SAFELY( m_pContinueButton );
	DELETE_SAFELY( m_pScreenFade );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pIntroMaterial );
	m_pRefManager.RemoveMaterial( m_pIntroMaterial2 );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );
	m_pRefManager.RemoveMaterial( m_pGradientMaterial );


} // CIntroQuestMenu::~CIntroQuestMenu()

void CIntroQuestMenu::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pScreenFade->RestoreDeviceObjects( pD3DDevice, m_pSettings );
} // CIntroQuestMenu::RestoreDeviceObjects()

void CIntroQuestMenu::Activate( void )
{
	m_SpriteYPosition = 344;
	m_FadeValue = 255;
	m_FadeDirection = -1;

	m_pSoundManager.StopMusic();


	// create a new quest

	DELETE_SAFELY( m_pQuest );
	int32 Level = RandomIntegerBetween( 40, 50 );
	m_pQuest = new CQuest( *m_pMenuManager.GameClient().NameGenerator(),
							 *m_pMenuManager.GameClient().QuestDescriptions(),
							 *m_pMenuManager.GameClient().CharacterManifest(),
							 *m_pMenuManager.GameClient().ItemManifest(),
							 Level,
							 KQuestMasterQuest,
							 NULL,
							 NULL,
							 m_pMenuManager.GameClient().Difficulty(),
							 "Bremen" );

	m_pMenuManager.GameClient().SetNewCharacterQuest( *m_pQuest );
	CMenu::Activate();
} // CIntroQuestMenu::Activate()



void CIntroQuestMenu::Deactivate( void )
{

	CMenu::Deactivate();
} // CIntroQuestMenu::Activate()

// perform creation of any elements that this menu needs
void CIntroQuestMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	m_pScreenFade = new CScreenFade( pD3DDevice,
									 m_pSettings );
	m_pScreenFade->SetColor( 0, 0, 0, 1 );

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );


	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );

	m_pIntroMaterial = m_pRefManager.AddMaterial( pD3DDevice,
												  "MENUS/intro2.png" );
	m_pIntroMaterial2 = m_pRefManager.AddMaterial( pD3DDevice,
												  "MENUS/intro5.png" );
	m_pButtonMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons.png",
													    "MENUS/buttonsa.png" );

	m_pGradientMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/black.png",
													    "MENUS/gradient.png",
														kFalse,
														kFalse );
	AddSprite( new CSprite( pD3DDevice,
							m_pGradientMaterial,
							0,
							768 - 128,
							1024,
							129,
							0,
							0.1f,
							1,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pGradientMaterial,
							0,
							-1,
							1024,
							48,
							0,
							1,
							1,
							0.1f,
							KSpriteTopLeft ) );

	// intro page
	m_pIntro1 = AddSprite( new CSprite( pD3DDevice,
										m_pIntroMaterial,
										768,
										m_SpriteYPosition,
										512,
										512,
										0,
										0,
										1,
										1,
										KSpriteCenter ) );

	m_pIntro2 = AddSprite( new CSprite( pD3DDevice,
										m_pIntroMaterial2,
										768,
										m_SpriteYPosition - 680,
										512,
										512,
										0,
										0,
										1,
										1,
										KSpriteCenter ) );

	m_pText = AddText( new CText( pD3DDevice,
										m_pFontMaterial,
										m_pFontMetric,
										LoadLocalizedString(1039),
										20,
										200,
										1,
										kFalse,
										KJustifyLeft ) );
	
	m_pText->EnableWrapping( pD3DDevice, 510 );

	
// new game button
	CButton* pButton;

	pButton = new CButton( pD3DDevice,
									  m_pButtonMaterial,
									  NULL,
									  m_pClickSound,
									  724,
									  700,
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
									  KIntroQuestMenuAccept );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1038),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1038),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );


	m_pContinueButton = pButton;

} // CIntroQuestMenu::Create()

void CIntroQuestMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );


} // CIntroQuestMenu::KeyEvent()


void CIntroQuestMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{

	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we request an exit
	case KIntroQuestMenuAccept :
		RemoveButton( m_pContinueButton );
		m_FadeDirection = 1;
		break;
	}

} // CIntroQuestMenu::ProcessInput()

void CIntroQuestMenu::Begin( void )
{
	Deactivate();
	m_pGameStateManager.SetGameState( KGameStateLoadGame );
} // CIntroQuestMenu::Begin()

void CIntroQuestMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{

	m_SpriteYPosition += TimeElapsed * 20;

	if( m_SpriteYPosition > 344 + 680 )
	{
		m_SpriteYPosition = 344 + 680;
	}
	m_pIntro1->SetPosition( 768, m_SpriteYPosition );
	m_pIntro2->SetPosition( 768, m_SpriteYPosition - 680 );

	if( m_FadeDirection != 0 )
	{
		m_FadeValue += m_FadeDirection * TimeElapsed * 100;
		if( m_FadeValue > 255 )
		{
			m_FadeValue = 255;
			m_FadeDirection = 0;
			Begin();
		}
		else if( m_FadeValue < 0 )
		{
			AddButton( m_pContinueButton );
			m_FadeValue = 0;
			m_FadeDirection = 0;
		}
	}
	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );
	

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CIntroQuestMenu::Update()

void CIntroQuestMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::Render( pD3DDevice );


} // CIntroQuestMenu::Render()

// render any UI elements
void CIntroQuestMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	m_pText->SetText( pD3DDevice, m_pQuest->Description() );

	RenderText( pD3DDevice );
	RenderSprites( pD3DDevice );
	RenderButtons( pD3DDevice );
	RenderDropdowns( pD3DDevice );

	m_pScreenFade->SetColor( 0, 0, 0, m_FadeValue / 255.0f );

	m_pScreenFade->Render( pD3DDevice );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// no zwrite or ztest while rendering text
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

} // CIntroQuestMenu::RenderUI()
