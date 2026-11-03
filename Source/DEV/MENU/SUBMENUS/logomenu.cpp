/*
	logomenu.cpp

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
#include "../../GAMEOBJECTS/screenfade.h"

#include "logomenu.h"

CLogoMenu::CLogoMenu( CRefManager& pRefManager,	// the reference manager, for tracking media we create
					  CSettings& pSettings,		// the settings manager, for keeping track of card/machine capabilities
					  CGameStateManager& pGameStateManager,	// game manager
					  CMenuManager& pMenuManager ) :		// menu manager
														CMenu( pRefManager,
																pSettings,
																pGameStateManager,
																pMenuManager ),
														m_pScreenFade( NULL ),
														m_pLogoMaterial( NULL ),
														m_pWTLogoMaterial( NULL ),
														m_HoldTime ( 2 ), 
														m_FadeValue ( 255 ),
														m_FadeDirection ( -1 )
{
} // CLogoMenu::CLogoMenu()

CLogoMenu::~CLogoMenu( void )
{
	DELETE_SAFELY( m_pScreenFade );
	m_pRefManager.RemoveMaterial( m_pLogoMaterial );
	m_pRefManager.RemoveMaterial( m_pWTLogoMaterial );

} // CLogoMenu::~CLogoMenu()

void CLogoMenu::Activate( void )
{
	CMenu::Activate();
	m_HoldTime = 2;
	m_FadeValue = 255;
	m_FadeDirection = -1;

} // CLogoMenu::Activate()

void CLogoMenu::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pScreenFade->RestoreDeviceObjects( pD3DDevice, m_pSettings );
	m_pScreenFade->SetColor( 0, 0, 0, 1 );
} // CIntroMenu::RestoreDeviceObjects()

// perform creation of any elements that this menu needs
void CLogoMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	m_pScreenFade = new CScreenFade( pD3DDevice,
									 m_pSettings );
	m_pScreenFade->SetColor( 0, 0, 0, 1 );

	m_pLogoMaterial = m_pRefManager.AddMaterial( pD3DDevice,  "MENUS/intrologo.png" );
	m_pWTLogoMaterial = m_pRefManager.AddMaterial( pD3DDevice,  "MENUS/wtlogo.png" );

	// game logo
	AddSprite( new CSprite( pD3DDevice,
							m_pLogoMaterial,
							512,
							384,
							512,
							256,
							0,
							0,
							1,
							1,
							KSpriteCenter ) );

	// game logo
	AddSprite( new CSprite( pD3DDevice,
							m_pWTLogoMaterial,
							16,
							700,
							128,
							64,
							0,
							0,
							1,
							1,
							KSpriteTopLeft ) );

} // CLogoMenu::Create()

void CLogoMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
						  WPARAM KeyCode )	// the wparam which will be the keycode
{
	CMenu::KeyEvent( WMMessage, KeyCode );
} // CLogoMenu::KeyEvent()


void CLogoMenu::ProcessInput( HWND hWnd )								// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	if( m_MouseHandler.ButtonPressed( KButtonLeft ) )
	{
		m_FadeDirection = 1;
	}

} // CLogoMenu::ProcessInput()

void CLogoMenu::Update( HWND hWnd,								// hwnd of the active window
						float32 TimeElapsed )					// time elapsed, in seconds
{
	ProcessInput( hWnd );

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();

	if( m_FadeDirection != 0 )
	{
		m_FadeValue += m_FadeDirection * TimeElapsed * 200;
		if( m_FadeValue > 255 )
		{
			m_FadeValue = 255;
			m_FadeDirection = 0;
			m_pGameStateManager.SetTargetMenu( KMainMenu );
		}
		else if( m_FadeValue < 0 )
		{
			m_FadeValue = 0;
			m_FadeDirection = 0;
		}
	}
	else
	{
		m_HoldTime -= TimeElapsed;
		if( m_HoldTime <= 0 )
		{
			m_FadeDirection = 1;
		}
	}
} // CLogoMenu::Update()


void CLogoMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

	CMenu::Render( pD3DDevice );


} // CLogoMenu::Render()

// render any UI elements
void CLogoMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
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

} // CLogoMenu::RenderUI()
