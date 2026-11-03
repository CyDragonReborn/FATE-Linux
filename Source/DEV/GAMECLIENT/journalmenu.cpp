/*
	journalmenu.cpp

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

#include "../GAMECLIENT/quest.h"

#include "gameui.h"
#include "journalmenu.h"
#include "bottommenu.h"

CJournalMenu::CJournalMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
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
																	m_pJournalMenuTop( NULL ),
																	m_pJournalMenuCenter( NULL ),
																	m_pJournalMenuBottom( NULL ),
																	m_pUIButtons( NULL ),
																	m_pPopupButtons( NULL ),
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

} // CJournalMenu::CJournalMenu()

CJournalMenu::~CJournalMenu( void )
{
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );
	m_pRefManager.RemoveMaterial( m_pPopupButtons );

	m_pRefManager.RemoveMaterial( m_pJournalMenuTop );
	m_pRefManager.RemoveMaterial( m_pJournalMenuBottom );
	m_pRefManager.RemoveMaterial( m_pJournalMenuCenter );

} // CJournalMenu::~CJournalMenu()

void CJournalMenu::FlushAll( void )
{
	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CJournalMenu::FlushAll()

void CJournalMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pPopupButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
													  "MENUS/buttons.png",
													  "MENUS/buttonsa.png" );

	m_pJournalMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/journaltop.png" );

	m_pJournalMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/journalbottom.png"  );

	m_pJournalMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/journalcenter.png" );

	CButton* pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									518,
									582,
									51,
									51,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 1,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 51,
									m_pUIButtons->PixelToU() * 1,
									m_pUIButtons->PixelToV() * 52,
									m_pUIButtons->PixelToU() * 51,
									m_pUIButtons->PixelToV() * 102,
									KButtonTopLeft,
									KBottomButtonJournal ) );
	pButton->SetToolTip( LoadLocalizedString(170) );


	AddSprite( new CSprite( pD3DDevice,
							m_pJournalMenuTop,
							512,
							0,
							512,
							256,
							0,
							0,
							1,
							.9985f,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pJournalMenuCenter,
							512,
							256,
							512,
							256,
							0,
							.0015f,
							1,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pJournalMenuBottom,
							512,
							512,
							512,
							128,
							0,
							.01f,
							1,
							1,
							KSpriteTopLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(913),
						770,
						100,
						.8f,
						kFalse,
						KJustifyCenter ) );
	
	int32 vPosition = -1;
	for( uint32 i = 0; i < KJournalStatistics; i++ )
	{
		if((EJournalStatistic)i == KJournalStatLargeChestsOpened)
		{
			continue;
		}
		else
		{
			vPosition++;
		}

		AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							KJournalStatName[i],
							645,
							175 + (float32)vPosition * 23,
							.6f,
							kTrue,
							KJustifyLeft) );

		m_pJournalText[i] = AddText( new CText( pD3DDevice,
												m_pFontMaterial,
												m_pFontMetric,
												"0",
												905,
												175 + (float32)vPosition * 23,
												.7f,
												kFalse,
												KJustifyRight ) );
		
	}


} // CJournalMenu::CreateMenus()


void CJournalMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CJournalMenu::KeyEvent()

void CJournalMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CJournalMenu::KeyEvent()

bool CJournalMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}

	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( MousePosition.x >= 512 )
	{
		return kTrue;
	}

	return kFalse;
} // CJournalMenu::MouseOver()

void CJournalMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CJournalMenu::ProcessInput()


void CJournalMenu::Update( HWND hWnd,						// hwnd of the active window
						 float32 TimeElapsed )			// time elapsed, in seconds
{
	if( !Open() )
	{
		return;
	}

	ProcessInput( hWnd );

	UpdateButtons( TimeElapsed );

	// left mouse button first
	int32 ButtonPressed = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) ||
											m_MouseHandler.ButtonDoubleClick( KButtonLeft ) );

	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CJournalMenu::Update()

void CJournalMenu::SetOpen( bool Open )		// state
{
	m_Open = Open;
} // CJournalMenu::SetOpen()

// render any 3d elements
void CJournalMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 CGameUI& pGameUI,						// parent game ui
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						 const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( !Open() )
	{
		return;
	}

	float32 Seconds = m_pPlayerCharacter->ExistenceTime();
	uint32 Minutes = 0;
	uint32 Hours = 0;
	while( Seconds >= 60 )
	{
		Seconds -= 60;
		Minutes++;
	}
	while( Minutes >= 60 )
	{
		Minutes -= 60;
		Hours++;
	}
	std::string Text = LoadLocalizedString3Variables(914, IntegerToString( Hours ), 
				IntegerToString( Minutes ), IntegerToString( (int32)Seconds ) );
	m_pJournalText[0]->SetText( pD3DDevice, Text );


	for( uint32 i = 1; i < KJournalStatistics; i++ )
	{
		if((EJournalStatistic)i == KJournalStatLargeChestsOpened) continue;
		m_pJournalText[i]->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->JournalStat( (EJournalStatistic)i ) ) );
	}

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	RenderSprites( pD3DDevice );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	RenderButtons( pD3DDevice );


	RenderText( pD3DDevice );		

} // CJournalMenu::Render()


void CJournalMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CJournalMenu::RenderText()

void CJournalMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CJournalMenu::RenderSprites()

void CJournalMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CJournalMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CJournalMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CJournalMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CJournalMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CJournalMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CJournalMenu::UpdateButtonOver( void )
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
} // CJournalMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CJournalMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CJournalMenu::UpdateButtonOver()

CButton* CJournalMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CJournalMenu::AddButton()

void CJournalMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CJournalMenu::RemoveButton()

void CJournalMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CJournalMenu::DestroyButtons()

CText* CJournalMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CJournalMenu::AddText()

void CJournalMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CJournalMenu::RemoveText()

void CJournalMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CJournalMenu::DestroyText()

CSprite* CJournalMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CJournalMenu::AddSprite()

CSprite* CJournalMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CJournalMenu::AddSpriteFront()

void CJournalMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CJournalMenu::RemoveSprite()

void CJournalMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CJournalMenu::DestroySprite()


