/*
	gameoptionsmenu.cpp

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

#include "../CHARACTER/character.h"
#include "../ITEM/item.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "gameui.h"
#include "gameoptionsmenu.h"

CGameOptionsMenu::CGameOptionsMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									CRefManager& pRefManager,					// the reference manager, for tracking media we create
									CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
									CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																				m_pGameStateManager( pGameStateManager ),
																				m_pRefManager( pRefManager ),
																				m_pSettings( pSettings ),
																				m_Open( kFalse ),
																				m_pButtonList	( new CLList<CButton>() ),
																				m_pTextList		( new CLList<CText>() ),
																				m_pSpriteList	( new CLList<CSprite>() ),
																				m_pFontMaterial( NULL ),
																				m_pFontMetric( NULL ),
																				m_pPlayerCharacter( NULL ),
																				m_pPopupMenu( NULL ),
																				m_pUIButtons( NULL ),
																				m_pClickSound( NULL )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CGameOptionsMenu::CGameOptionsMenu()

CGameOptionsMenu::~CGameOptionsMenu( void )
{
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	m_pRefManager.RemoveMaterial( m_pPopupMenu );

} // CGameOptionsMenu::~CGameOptionsMenu()

void CGameOptionsMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CGameOptionsMenu::FlushAll()

void CGameOptionsMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/buttons.png",
												   "MENUS/buttonsa.png" );

	m_pPopupMenu = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/popup.png" );

	CButton* pButton;

// save and exit button

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									512,
									364,
									256,
									48,
									0 * m_pUIButtons->PixelToU(),
									49 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									0 * m_pUIButtons->PixelToU(),
									0 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    48 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonSaveAndExit ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(213),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(213),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );

// save and continue button
/*
	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									512,
									394,
									256,
									48,
									0 * m_pUIButtons->PixelToU(),
									49 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									0 * m_pUIButtons->PixelToU(),
									0 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    48 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonSaveAndContinue ) );


	pButton->AddText( pD3DDevice,
					  "Resume Playing",
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						"Resume Playing",
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );
*/
	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									512,
									424,
									256,
									48,
									0 * m_pUIButtons->PixelToU(),
									49 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
									97 * m_pUIButtons->PixelToV(),
									0 * m_pUIButtons->PixelToU(),
									0 * m_pUIButtons->PixelToV(),
									256 * m_pUIButtons->PixelToU(),
								    48 * m_pUIButtons->PixelToV(),
									KButtonCenter,
									KButtonResume ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(214),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(214),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );


	AddSprite( new CSprite( pD3DDevice,
							m_pPopupMenu,
							512,
							384,
							512,
							256,
							0,
							0,
							1,
							1,
							KSpriteCenter ) );


} // CGameOptionsMenu::CreateMenus()


void CGameOptionsMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CGameOptionsMenu::KeyEvent()

void CGameOptionsMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CGameOptionsMenu::KeyEvent()

bool CGameOptionsMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}

	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( MousePosition.x >= 256 &&
		MousePosition.x <= 768 &&
		MousePosition.y >= 256 &&
		MousePosition.y <= 512 )
	{
		return kTrue;
	}
	return kFalse;
} // CGameOptionsMenu::MouseOver()

void CGameOptionsMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CGameOptionsMenu::ProcessInput()

void CGameOptionsMenu::Update( HWND hWnd,						// hwnd of the active window
					  float32 TimeElapsed )				// time elapsed, in seconds
{
	if( !Open() )
	{
		return;
	}
	ProcessInput( hWnd );


	UpdateButtons( TimeElapsed );



	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CGameOptionsMenu::Update()

// render any 3d elements
void CGameOptionsMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						 const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( !Open() )
	{
		return;
	}

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	RenderSprites( pD3DDevice );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	RenderButtons( pD3DDevice );


	RenderText( pD3DDevice );		

} // CGameOptionsMenu::Render()

void CGameOptionsMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CGameOptionsMenu::RenderText()

void CGameOptionsMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CSprite>* pNode = m_pSpriteList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CGameOptionsMenu::RenderSprites()

void CGameOptionsMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CGameOptionsMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CGameOptionsMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CGameOptionsMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CGameOptionsMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CGameOptionsMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CGameOptionsMenu::UpdateButtonOver( void )
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
} // CGameOptionsMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CGameOptionsMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CGameOptionsMenu::UpdateButtonOver()

CButton* CGameOptionsMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CGameOptionsMenu::AddButton()

void CGameOptionsMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CGameOptionsMenu::RemoveButton()

void CGameOptionsMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CGameOptionsMenu::DestroyButtons()

CText* CGameOptionsMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CGameOptionsMenu::AddText()

void CGameOptionsMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CGameOptionsMenu::RemoveText()

void CGameOptionsMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CGameOptionsMenu::DestroyText()

CSprite* CGameOptionsMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CGameOptionsMenu::AddSprite()

CSprite* CGameOptionsMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CGameOptionsMenu::AddSpriteFront()

void CGameOptionsMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CGameOptionsMenu::RemoveSprite()

void CGameOptionsMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CGameOptionsMenu::DestroySprite()


