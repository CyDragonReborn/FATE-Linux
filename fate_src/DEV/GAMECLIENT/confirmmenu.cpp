/*
	confirmmenu.cpp

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
#include "confirmmenu.h"

CConfirmMenu::CConfirmMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									CRefManager& pRefManager,					// the reference manager, for tracking media we create
									CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
									CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																			m_pGameStateManager( pGameStateManager ),
																			m_pRefManager( pRefManager ),
																			m_pSettings( pSettings ),
																			m_Open( kFalse ),
																			m_LargeMenu( kFalse ),
																			m_pButtonList	( new CLList<CButton>() ),
																			m_pTextList		( new CLList<CText>() ),
																			m_pSpriteList	( new CLList<CSprite>() ),
																			m_pFontMaterial( NULL ),
																			m_pFontMetric( NULL ),
																			m_pPopupSprite( NULL ),
																			m_pPlayerCharacter( NULL ),
																			m_pPopupMenu( NULL ),
																			m_pPopupMenuLarge( NULL ),
																			m_pUIButtons( NULL ),
																			m_pMessageText( NULL ),
																			m_pYesButton( NULL ),
																			m_pNoButton( NULL ),
																			m_pClickSound( NULL ),
																			m_YesCode( 0 ),
																			m_NoCode( 0 ),
																			m_Message( "" )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CConfirmMenu::CConfirmMenu()

CConfirmMenu::~CConfirmMenu( void )
{
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	m_pRefManager.RemoveMaterial( m_pPopupMenu );
	m_pRefManager.RemoveMaterial( m_pPopupMenuLarge );

} // CConfirmMenu::~CConfirmMenu()

void CConfirmMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CConfirmMenu::FlushAll()

void CConfirmMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/buttons.png",
												   "MENUS/buttonsa.png" );

	m_pPopupMenu = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/popup.png" );

	m_pPopupMenuLarge = m_pRefManager.AddMaterial( pD3DDevice,
												  "MENUS/popup3.png" );

	CButton* pButton;

// save and exit button

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									424,
									460,
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
									KButtonYes ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(171),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						  LoadLocalizedString(171),
						  m_pFontMaterial,
						  m_pFontMetric,
						  0,
						  -15,
						  1.0f,
						  kFalse,
						  KJustifyCenter );

	m_pYesButton = pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									600,
									460,
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
									KButtonNo ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(171),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(172),
						m_pFontMaterial,
						m_pFontMetric,
						0,
						-15,
						1.0f,
						kFalse,
						KJustifyCenter );

	m_pNoButton = pButton;

	m_pPopupSprite = AddSprite( new CSprite( pD3DDevice,
											m_pPopupMenu,
											512,
											384,
											512,
											512,
											0,
											0,
											1,
											1,
											KSpriteCenter ) );


	m_pMessageText = AddText( new CText( pD3DDevice,
										 m_pFontMaterial,
										 m_pFontMetric,
										 LoadLocalizedString(173),
										 512,
										 300,
										 1,
										 kFalse,
										 KJustifyCenter ) );
	m_pMessageText->EnableWrapping( pD3DDevice,
									490 );

} // CConfirmMenu::CreateMenus()

void CConfirmMenu::Open( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 const std::string& Message,	// message to be displayed
						 const std::string& YesMessage,	// yes button text
						 int32 YesCode,					// code for yes return
						 const std::string& NoMessage,	// no button text
						 int32 NoCode,					// code for no return
						 float32 Scale )				// text scale
{
	m_Open = kTrue;

	m_pMessageText->SetText( pD3DDevice,
							 Message );
	m_pMessageText->SetScale( pD3DDevice,
							  Scale );

	float32 TextHeight = m_pMessageText->MaxBounds().y - m_pMessageText->MinBounds().y;


	if( TextHeight > 80 )
	{
		m_pYesButton->SetPosition( 424, 588 );
		m_pNoButton->SetPosition( 600, 588 );
		m_pPopupSprite->SetSize( 512, 536 ); // TDC 2008.02.27 Craig Miles - Size increased for nonenglish programmatically assembled names.
		m_pPopupSprite->SetMaterial( m_pPopupMenuLarge );
		m_LargeMenu = kTrue;
		m_pMessageText->SetPosition( 512,
									372 - TextHeight * .5f ); // TDC 2008.02.27 Craig Miles - Text moved up 1 line
	}
	else
	{
		m_pYesButton->SetPosition( 424, 460 );
		m_pNoButton->SetPosition( 600, 460 );
		m_pPopupSprite->SetSize( 512, 256 );
		m_pPopupSprite->SetMaterial( m_pPopupMenu );
		m_LargeMenu = kFalse;
		m_pMessageText->SetPosition( 512,
									 300 );
	}

	m_pYesButton->SetText( pD3DDevice,
						   YesMessage );
	m_pYesButton->SetOverText( pD3DDevice,
							   YesMessage );
	m_pYesButton->SetButtonCode( YesCode );

	m_pNoButton->SetText( pD3DDevice,
						  NoMessage );
	m_pNoButton->SetOverText( pD3DDevice,
							  NoMessage );
	m_pNoButton->SetButtonCode( NoCode );

	FlushAll();
} // CConfirmMenu::Open()

void CConfirmMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CConfirmMenu::KeyEvent()

void CConfirmMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CConfirmMenu::KeyEvent()

bool CConfirmMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}

	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	if( m_LargeMenu )
	{
		if( MousePosition.x >= 256 &&
			MousePosition.x <= 768 &&
			MousePosition.y >= 128 &&
			MousePosition.y <= 640 )
		{
			return kTrue;
		}
	}
	else
	{
		if( MousePosition.x >= 256 &&
			MousePosition.x <= 768 &&
			MousePosition.y >= 256 &&
			MousePosition.y <= 512 )
		{
			return kTrue;
		}
	}

	return kFalse;
} // CConfirmMenu::MouseOver()

void CConfirmMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CConfirmMenu::ProcessInput()

void CConfirmMenu::Update( HWND hWnd,						// hwnd of the active window
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
} // CConfirmMenu::Update()

// render any 3d elements
void CConfirmMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
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

} // CConfirmMenu::Render()


void CConfirmMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CConfirmMenu::RenderText()

void CConfirmMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CConfirmMenu::RenderSprites()

void CConfirmMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CConfirmMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CConfirmMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CConfirmMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CConfirmMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CConfirmMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CConfirmMenu::UpdateButtonOver( void )
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
} // CConfirmMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CConfirmMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CConfirmMenu::UpdateButtonOver()

CButton* CConfirmMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CConfirmMenu::AddButton()

void CConfirmMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CConfirmMenu::RemoveButton()

void CConfirmMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CConfirmMenu::DestroyButtons()

CText* CConfirmMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CConfirmMenu::AddText()

void CConfirmMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CConfirmMenu::RemoveText()

void CConfirmMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CConfirmMenu::DestroyText()

CSprite* CConfirmMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CConfirmMenu::AddSprite()

CSprite* CConfirmMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CConfirmMenu::AddSpriteFront()

void CConfirmMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CConfirmMenu::RemoveSprite()

void CConfirmMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CConfirmMenu::DestroySprite()


