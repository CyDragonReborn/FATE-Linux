/*
	creditsmenu.cpp

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

#include "creditsmenu.h"


CCreditsMenu::CCreditsMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
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
														m_pUIButtonMaterial( NULL ),
														m_pFontMaterial( NULL ),
														m_pClickSound ( NULL )
								
{

} // CCreditsMenu::CCreditsMenu()

CCreditsMenu::~CCreditsMenu( void )
{
	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pButtonMaterial );
	m_pRefManager.RemoveMaterial( m_pUIButtonMaterial );


} // CCreditsMenu::~CCreditsMenu()

void CCreditsMenu::Activate( void )
{
	CMenu::Activate();
} // CCreditsMenu::Activate()

// perform creation of any elements that this menu needs
void CCreditsMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
									  512,
									  730,
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
									  KCreditsMenuBack ) );
	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(915),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(915),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1030),
						712,
						50,
						1.0f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1031),
						712,
						140,
						1.0f,
						kFalse,
						KJustifyCenter ) );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1032),
						712,
						210,
						1.0f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1033),
						512,
						280,
						1.0f,
						kFalse,
						KJustifyCenter ) );


	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1034),
						512,
						400,
						1.0f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1035),
						512,
						470,
						1.0f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(1036),
						512,
						590,
						1.0f,
						kFalse,
						KJustifyCenter ) );

} // CCreditsMenu::Create()

void CCreditsMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );


} // CCreditsMenu::KeyEvent()


void CCreditsMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

	switch( ReturnCode )
	{

	// if pressed, we go to character creation
	case KCreditsMenuBack :
		m_pGameStateManager.SetTargetMenu( KMainMenu );
		break;
	}

} // CCreditsMenu::ProcessInput()

void CCreditsMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );
	

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

} // CCreditsMenu::Update()

void CCreditsMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{


	CMenu::Render( pD3DDevice );


} // CCreditsMenu::Render()

// render any UI elements
void CCreditsMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::RenderUI( pD3DDevice );

} // CCreditsMenu::RenderUI()

