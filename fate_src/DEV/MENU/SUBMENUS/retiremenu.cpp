/*
	retiremenu.cpp

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

#include "retiremenu.h"

#include "../../GAMEOBJECTS/screenfade.h"
#include "../../MEDIA/soundmanager.h"
#include "../../NAMES/namegenerator.h"
#include "../../GAMECLIENT/gameclient.h"

CRetireMenu::CRetireMenu( CRefManager& pRefManager,				// the reference manager, for tracking media we create
					    CSettings& pSettings,					// the settings manager, for keeping track of card/machine capabilities
					    CGameStateManager& pGameStateManager,	// game manager
					    CSoundManager& pSoundManager,			// sound manager
		  			    CGameClient& pGameClient,				// game client
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
														m_pGameClient( pGameClient ),
														m_UpdateText( kFalse ),
														m_pClickSound( NULL ),
														m_FadeValue ( 0.0f ),
														m_FadeDirection ( 0.0f ),
														m_pContinueButton( NULL ),
														m_pRetireButton( NULL ),
														m_pText( NULL ),
														m_Continue ( kFalse )
{
} // CRetireMenu::CRetireMenu()

CRetireMenu::~CRetireMenu( void )
{
	RemoveButton( m_pContinueButton );
	RemoveButton( m_pRetireButton );
	DELETE_SAFELY( m_pContinueButton );
	DELETE_SAFELY( m_pRetireButton );
	DELETE_SAFELY( m_pScreenFade );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pIntroMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );


} // CRetireMenu::~CRetireMenu()

void CRetireMenu::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pScreenFade->RestoreDeviceObjects( pD3DDevice, m_pSettings );
} // CRetireMenu::RestoreDeviceObjects()

void CRetireMenu::Activate( void )
{
	m_UpdateText = kTrue;

	m_FadeValue = 255;
	m_FadeDirection = -1;

	m_pSoundManager.StopMusic();

	CMenu::Activate();
} // CRetireMenu::Activate()



void CRetireMenu::Deactivate( void )
{

	CMenu::Deactivate();
} // CRetireMenu::Activate()

// perform creation of any elements that this menu needs
void CRetireMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
												  "MENUS/retire.png" );
	m_pButtonMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													    "MENUS/buttons.png",
													    "MENUS/buttonsa.png" );
	// intro page
	AddSprite( new CSprite( pD3DDevice,
							m_pIntroMaterial,
							768,
							344,
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
										LoadLocalizedString(1078),
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
									  650,
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
									  KRetireMenuContinue );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1079),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1079),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );

	m_pContinueButton = pButton;


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
									  KRetireMenuAccept );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(1080),
					  m_pFontMaterial,
					  m_pFontMetric,
					  128,
					  10,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(1080),
						m_pFontMaterial,
						m_pFontMetric,
						128,
						10,
						1.0f,
						kFalse,
						KJustifyCenter );
	m_pRetireButton = pButton;

} // CRetireMenu::Create()

void CRetireMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );


} // CRetireMenu::KeyEvent()


void CRetireMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{

	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we request an exit
	case KRetireMenuAccept :
		RemoveButton( m_pContinueButton );
		RemoveButton( m_pRetireButton );
		m_FadeDirection = 1;
		m_Continue = kFalse;
		break;
	// if pressed, we continue
	case KRetireMenuContinue :
		RemoveButton( m_pContinueButton );
		RemoveButton( m_pRetireButton );
		m_FadeDirection = 1;
		m_Continue = kTrue;
		break;
	}

} // CRetireMenu::ProcessInput()

void CRetireMenu::Begin( void )
{
	if( m_Continue )
	{
		m_pGameStateManager.SetTargetMenu( KAncestorDifficultyMenu );
	}
	else
	{
		m_pGameStateManager.SetTargetMenu( KMainMenu );
	}
	Deactivate();
} // CRetireMenu::Begin()

void CRetireMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{

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
			AddButton( m_pRetireButton );
			m_FadeValue = 0;
			m_FadeDirection = 0;
		}
	}
	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );
	

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CRetireMenu::Update()

void CRetireMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::Render( pD3DDevice );


} // CRetireMenu::Render()

// render any UI elements
void CRetireMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	if( m_UpdateText )
	{
		m_pText->SetText( pD3DDevice, LoadLocalizedString(1081) + m_pGameClient.RetirementText() );
		m_UpdateText = kFalse;
	}

	CMenu::RenderUI( pD3DDevice );

	m_pScreenFade->SetColor( 0, 0, 0, m_FadeValue / 255.0f );

	m_pScreenFade->Render( pD3DDevice );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// no zwrite or ztest while rendering text
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

} // CRetireMenu::RenderUI()
