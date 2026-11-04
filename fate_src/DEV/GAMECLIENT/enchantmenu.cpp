/*
	enchantmenu.cpp

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

#include "../LEVEL/level.h"

#include "gameui.h"
#include "enchantmenu.h"

CEnchantMenu::CEnchantMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					CRefManager& pRefManager,					// the reference manager, for tracking media we create
					CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
					CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																m_pGameStateManager( pGameStateManager ),
																m_pRefManager( pRefManager ),
																m_pSettings( pSettings ),
																m_Open( kFalse ),
																m_pOwner( NULL ),
																m_pItem( NULL ),
																m_pButtonList	( new CLList<CButton>() ),
																m_pTextList		( new CLList<CText>() ),
																m_pSpriteList	( new CLList<CSprite>() ),
																m_pFontMaterial( NULL ),
																m_pFontMetric( NULL ),
																m_pPlayerCharacter( NULL ),
																m_pPopupMenu( NULL ),
																m_pUIButtons( NULL ),
																m_pMessageText( NULL ),
																m_pYesButton( NULL ),
																m_pNoButton( NULL ),
																m_pClickSound( NULL ),
																m_Message( "" )
{
	ZeroMemory ( &m_WorldPosition,	sizeof ( m_WorldPosition ));
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));


	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CEnchantMenu::CEnchantMenu()

CEnchantMenu::~CEnchantMenu( void )
{
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	m_pRefManager.RemoveMaterial( m_pPopupMenu );

} // CEnchantMenu::~CEnchantMenu()

void CEnchantMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CEnchantMenu::FlushAll()

void CEnchantMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/buttons.png",
												   "MENUS/buttonsa.png" );

	m_pPopupMenu = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/popup2.png" );

	CButton* pButton;

// save and exit button

// save and exit button

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									256,
									532,
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
									256,
									582,
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

	AddSprite( new CSprite( pD3DDevice,
							m_pPopupMenu,
							256,
							384,
							256,
							512,
							0,
							0,
							1,
							1,
							KSpriteCenter ) );



	m_pMessageText = AddText( new CText( pD3DDevice,
										 m_pFontMaterial,
										 m_pFontMetric,
										 "This is a tip", // debug text? assuming it's non-translatable
										 256,
										 174,
										 .8f,
										 kFalse,
										 KJustifyCenter ) );

	m_pMessageText->EnableWrapping( pD3DDevice,
									210 );


	pButton = AddButton( new CButton( pD3DDevice,
												NULL,
												208,
												318,
												96,
												192,
												0,
												0,
												1,
												1,
												0,
												0,
												1,
												1,
												KButtonTopLeft,
												KEnchantItemButton ) );

	pButton->SetAllowClickOnEdge( kTrue );

} // CEnchantMenu::CreateMenus()

void CEnchantMenu::SetText( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
						    const std::string& Message )		// message to be displayed
{
	m_pMessageText->SetText( pD3DDevice,
							 Message );
} // CEnchantMenu::SetText()

void CEnchantMenu::Open( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 const D3DXVECTOR3& Position,	// position of enchanting item
						 const std::string& Message,	// message to be displayed
						 const std::string& YesMessage,	// yes button text
						 int32 YesCode,					// code for yes return
						 const std::string& NoMessage,	// no button text
						 int32 NoCode,					// code for no return
						 float32 Scale )				// text scale
{
	m_WorldPosition = Position;
	m_Open = kTrue;

	m_pMessageText->SetText( pD3DDevice,
							 Message );
	m_pMessageText->SetScale( pD3DDevice,
							  Scale );
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
} // CEnchantMenu::Open()

void CEnchantMenu::SetOpen( CLevel& pLevel,	// level to drop items to
							bool Open )		// open state
{
	if( !Open && m_pItem != NULL )
	{
		pLevel.AddItem( m_pItem, m_WorldPosition );
		m_pItem->Drop( pLevel );
		m_pItem = NULL;
	}
	m_Open = Open;
} // CEnchantMenu::SetOpen()

void CEnchantMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CEnchantMenu::KeyEvent()

void CEnchantMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CEnchantMenu::KeyEvent()

bool CEnchantMenu::MouseOver( void )
{
	if( !Open() )
	{
		return kFalse;
	}

	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	if( MousePosition.x <= 512 )
	{
		return kTrue;
	}
	return kFalse;
} // CEnchantMenu::MouseOver()

void CEnchantMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CEnchantMenu::ProcessInput()

void CEnchantMenu::Update( HWND hWnd,						// hwnd of the active window
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
} // CEnchantMenu::Update()

// render any 3d elements
void CEnchantMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
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

	RenderInventoryIcons( pD3DDevice );

} // CEnchantMenu::Render()

// render any 3d elements
void CEnchantMenu::RenderInventoryIcons( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	if( m_pItem != NULL )
	{
		float32 X = 256;
		float32 Y = 414;
		X -= m_pItem->IconSprite()->Width() * .5f;
		Y -= m_pItem->IconSprite()->Height() * .5f;
			
		
		m_pItem->IconSprite()->Render( pD3DDevice,
									   X,
									   Y );	   
	}

} // CEnchantMenu::RenderInventoryIcons()


void CEnchantMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CEnchantMenu::RenderText()

void CEnchantMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CEnchantMenu::RenderSprites()

void CEnchantMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CEnchantMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CEnchantMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CEnchantMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CEnchantMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CEnchantMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CEnchantMenu::UpdateButtonOver( void )
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
} // CEnchantMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CEnchantMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CEnchantMenu::UpdateButtonOver()

CButton* CEnchantMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CEnchantMenu::AddButton()

void CEnchantMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CEnchantMenu::RemoveButton()

void CEnchantMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CEnchantMenu::DestroyButtons()

CText* CEnchantMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CEnchantMenu::AddText()

void CEnchantMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CEnchantMenu::RemoveText()

void CEnchantMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CEnchantMenu::DestroyText()

CSprite* CEnchantMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CEnchantMenu::AddSprite()

CSprite* CEnchantMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CEnchantMenu::AddSpriteFront()

void CEnchantMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CEnchantMenu::RemoveSprite()

void CEnchantMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CEnchantMenu::DestroySprite()


