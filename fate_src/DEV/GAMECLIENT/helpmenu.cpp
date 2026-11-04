/*
	helpmenu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/10/2004

*/


#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include <windows.h>
#include <tchar.h>

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

#include "../GAMEOBJECTS/screenfade.h"

#include "gameui.h"
#include "helpmenu.h"

CHelpMenu::CHelpMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									CRefManager& pRefManager,					// the reference manager, for tracking media we create
									CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
									CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																				m_pGameStateManager( pGameStateManager ),
																				m_pRefManager( pRefManager ),
																				m_pSettings( pSettings ),
																				m_Open( kFalse ),
																				m_pScreenFade( NULL ),
																				m_pButtonList	( new CLList<CButton>() ),
																				m_pTextList		( new CLList<CText>() ),
																				m_pSpriteList	( new CLList<CSprite>() ),
																				m_pFontMaterial( NULL ),
																				m_pFontMetric( NULL ),
																				m_pUIButtons( NULL ),
																				m_pClickSound( NULL )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));

	m_pScreenFade = new CScreenFade( pD3DDevice,
									 pSettings );

	m_pScreenFade->SetColor( 0, 0, 0, .5f );

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CHelpMenu::CHelpMenu()

CHelpMenu::~CHelpMenu( void )
{
	DELETE_SAFELY( m_pScreenFade );
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

} // CHelpMenu::~CHelpMenu()


void CHelpMenu::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	m_pScreenFade->RestoreDeviceObjects( pD3DDevice, m_pSettings );
} // CHelpMenu::RestoreDeviceObjects()


void CHelpMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CHelpMenu::FlushAll()

void CHelpMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/buttons.png",
												   "MENUS/buttonsa.png" );

	CButton* pButton;

// save and exit button

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									512,
									75,
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
									KButtonCloseHelp ) );


	pButton->AddText( pD3DDevice,
					  LoadLocalizedString(888),
					  m_pFontMaterial,
					  m_pFontMetric,
					  0,
					  -15,
					  1.0f,
					  kTrue,
					  KJustifyCenter );
	pButton->AddOverText( pD3DDevice,
						LoadLocalizedString(888),
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
					    LoadLocalizedString(889),
						512,
						10,
						1,
						kTrue,
						KJustifyCenter ) );

	// another lengthy string which is long enough to warrant a specially-sized array
	std::string lengthyHelpText;
	TCHAR helpTextCharArray[2000];
	LoadString(CGame::GetLangDLL(), 890, helpTextCharArray, 2000);
	lengthyHelpText = helpTextCharArray;

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    lengthyHelpText,
						218,
						102,
						1,
						kFalse,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(891),
						97,
						398,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(892),
						102,
						461,
						1,
						kTrue,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(893),
						222,
						575,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(894),
						684,
						575,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(895),
						872,
						575,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(896),
						350,
						487,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(897),
						538,
						487,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(898),
						380,
						440,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(899),
						518,
						440,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(900),
						564,
						610,
						1,
						kTrue,
						KJustifyLeft ) );

	AddText( new CText( pD3DDevice,
					    m_pFontMaterial,
					    m_pFontMetric,
					    LoadLocalizedString(901),
						264,
						610,
						1,
						kTrue,
						KJustifyLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							112,
							158,
							5,
							234,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							42,
							112,
							5,
							347,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							237,
							605,
							5,
							142,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							768,
							605,
							5,
							142,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							927,
							605,
							5,
							49,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							600,
							640,
							5,
							92,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							420,
							640,
							5,
							50,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							462,
							515,
							5,
							185,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							552,
							515,
							5,
							166,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	// toglle bars
	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							492,
							468,
							5,
							190,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pUIButtons,
							532,
							468,
							5,
							190,
							56 * m_pUIButtons->PixelToU(),
							105 * m_pUIButtons->PixelToV(),
							76 * m_pUIButtons->PixelToU(),
							119 * m_pUIButtons->PixelToV(),
							KSpriteTopLeft ) );

} // CHelpMenu::CreateMenus()



void CHelpMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CHelpMenu::KeyEvent()

void CHelpMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CHelpMenu::KeyEvent()

bool CHelpMenu::MouseOver( void )
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
} // CHelpMenu::MouseOver()

void CHelpMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CHelpMenu::ProcessInput()

void CHelpMenu::Update( HWND hWnd,						// hwnd of the active window
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
} // CHelpMenu::Update()

// render any 3d elements
void CHelpMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
						 const D3DXMATRIX& ProjectionMatrix )	// projection matrix  ( for bringing 3d objects to screen space
{
	if( !Open() )
	{
		return;
	}

	m_pScreenFade->Render( pD3DDevice );

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

} // CHelpMenu::Render()


void CHelpMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CHelpMenu::RenderText()

void CHelpMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CHelpMenu::RenderSprites()

void CHelpMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CHelpMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CHelpMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CHelpMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CHelpMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CHelpMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CHelpMenu::UpdateButtonOver( void )
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
} // CHelpMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CHelpMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CHelpMenu::UpdateButtonOver()

CButton* CHelpMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CHelpMenu::AddButton()

void CHelpMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CHelpMenu::RemoveButton()

void CHelpMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CHelpMenu::DestroyButtons()

CText* CHelpMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CHelpMenu::AddText()

void CHelpMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CHelpMenu::RemoveText()

void CHelpMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CHelpMenu::DestroyText()

CSprite* CHelpMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CHelpMenu::AddSprite()

CSprite* CHelpMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CHelpMenu::AddSpriteFront()

void CHelpMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CHelpMenu::RemoveSprite()

void CHelpMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CHelpMenu::DestroySprite()


