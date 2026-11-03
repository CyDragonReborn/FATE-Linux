/*
	loadingmenu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/

#include "loadingmenu.h"
#include "../GAMECLIENT/tips.h"
#include "../GAME/game.h"

CLoadingMenu::CLoadingMenu( CRefManager& pRefManager,				// the reference manager, for tracking media we create
							CSettings& pSettings ) :				// the settings manager, for keeping track of card/machine capabilities
														m_pRefManager( pRefManager ),
														m_pSettings( pSettings ),
														m_pButtonList( NULL ),
														m_pSpriteList( NULL ),
														m_pDropdownList( NULL ),
														m_pDPLogoMaterial( NULL ),
														m_pTextList( NULL ),
														m_UpdateElements( kFalse ),
														m_Text( "" ),
														m_pFontMaterial( NULL ),
														m_ProgressPercentage( 0 ),
														m_DisplayTips( kFalse ),
														m_LastTipTime( -1 ),
														m_pPopupMenu( NULL ),
														m_pBottomMenuCenter( NULL ),
														m_pProgressBar( NULL ),
														m_pMessageText( NULL ),
														m_pFontMetric( NULL ),
														m_pTips( NULL )
{
	ZeroMemory ( &m_KeyHandler, sizeof ( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler, sizeof ( m_MouseHandler ));

	m_pButtonList = new CLList<CButton>();
	m_pTextList = new CLList<CText>();
	m_pSpriteList = new CLList<CSprite>();
	m_pDropdownList = new CLList<CDropdown>();
} // CLoadingMenu::CLoadingMenu()

CLoadingMenu::~CLoadingMenu( void )
{
	DELETE_SAFELY( m_pTips );
	m_pRefManager.RemoveMaterial( m_pPopupMenu );
	
	DELETE_SAFELY( m_pDropdownList );
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );
	m_pRefManager.RemoveMaterial( m_pDPLogoMaterial );

} // CLoadingMenu::~CLoadingMenu()

void CLoadingMenu::Activate( void )
{
	// flush our keyboard & mouse handlers
	m_KeyHandler.FlushAll();
	m_MouseHandler.FlushAll();
} // CLoadingMenu::Activate()

void CLoadingMenu::Deactivate( void )
{
} // CLoadingMenu::Deactivate()

// perform creation of any elements that this menu needs
void CLoadingMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );
	m_pFontMetric->SetCharacterHeight( 24 );
	m_pFontMetric->SetKerning( -4 );
	m_pFontMetric->Pad( 4 );

	m_pBottomMenuCenter = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														"MENUS/bottombarcenter.png",
														"MENUS/bottombarcentera.png" );

	m_pPopupMenu = m_pRefManager.AddMaterial( pD3DDevice,
		"MENUS/" + CGame::GetLocaleString() + "/loading.png" ); //use localized loading screen

	std::string Path = AssembleAbsolutePath( GetLocalPath(), "dplogo_load.jpg" );			
	// low quality version doesn't have the secondary music files
	if( FileExists( Path ) )
	{
		m_pDPLogoMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
															"dplogo_load.jpg",
															"dplogo_load_a.jpg",
															kFalse,
															kFalse );
		// game logo
		AddSprite( new CSprite( pD3DDevice,
								m_pDPLogoMaterial,
								10,
								10,
								(float32)m_pDPLogoMaterial->Width(),
								(float32)m_pDPLogoMaterial->Height(),
								0,
								0,
								1,
								1,
								KSpriteTopLeft ) );
	}
	m_pProgressBar = AddSprite( new CSprite( pD3DDevice,
												 m_pBottomMenuCenter,
												 385,
												 329,
												 254,
												 28,
												 m_pBottomMenuCenter->PixelToU() * 165,
												 m_pBottomMenuCenter->PixelToV() * 105,
												 m_pBottomMenuCenter->PixelToU() * 174,
												 m_pBottomMenuCenter->PixelToV() * 125,
												 KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pPopupMenu,
							512,
							334,
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
										m_Text,
										512,
										600,
										1.0f,
										kFalse,
										KJustifyCenter ) );
	m_pMessageText->EnableWrapping( pD3DDevice, 900 );

	m_pTips = new CTips( "TIPS\\" + CGame::GetLocaleString() + "\\tips.dat" );

} // CLoadingMenu::Create()

void CLoadingMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					   WPARAM KeyCode )	// the wparam which will be the keycode
{
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CLoadingMenu::KeyEvent()

void CLoadingMenu::MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONDOWN, etc.
					    WPARAM Param )		// the wparam which will be the button code
{
	m_MouseHandler.MouseEvent( WMMessage, Param );
} // CLoadingMenu::KeyEvent()

void CLoadingMenu::ProcessInput( HWND hWnd )							// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

} // CLoadingMenu::ProcessInput()

void CLoadingMenu::Update( HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed )					// time elapsed, in seconds
{
	ProcessInput( hWnd );

	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );

	UpdateButtons( TimeElapsed );


	// flush our keyboard handler
	m_KeyHandler.Flush();
	m_MouseHandler.Flush();
} // CLoadingMenu::Update()

void CLoadingMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

} // CLoadingMenu::Render()

// render any UI elements
void CLoadingMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
							 bool Flip )						// flip immediately?
{
	if( m_DisplayTips )
	{
		float32 Time = (float32)Game_Timer( KTimerGetAbsoluteTime );
		if( m_LastTipTime == -1 ||
			m_LastTipTime + 15 < Time )
		{
			m_LastTipTime = Time;
			m_Text = m_pTips->RandomTip();
		}
	}

	// try to begin the render of the primary scene
	if ( Flip &&
		 !FAILED( pD3DDevice->BeginScene() ) )
	{
		// clear the back buffer
		pD3DDevice->Clear( 0,
							NULL,
							D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,
							D3DCOLOR_RGBA( 0, 0, 0, 255 ),
							1.0f,
							0 );
	}

	// set up an othographic projection for rendering UI elements - we don't need depth!
	D3DXMATRIX Identity;
	D3DXMATRIX Orthographic;
	D3DXMatrixIdentity( &Identity );
	// the values you use for width and height for the orthographic matrix
	// correspond to the scale of pixel coordinates used in UI elements.
	// for instance, positioning something at 1024x768 with an orthographic matrix
	// using 1024x768 will place it in the bottom-right of the screen - 
	// if you use an 800x600 resolution for your ortho matrix, something at 1024x768
	// would be OFFSCREEN. It's all relative, so choose this based upon the size
	// of your source artwork
	D3DXMatrixOrthoLH ( &Orthographic, 
						1024,
						768,
						1.0f,
						10.0f );

	pD3DDevice->SetTransform( D3DTS_PROJECTION, &Orthographic );
	pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );
	pD3DDevice->SetTransform( D3DTS_VIEW, &Identity );


	// turn off lighting
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	// we want to alpha blend our UI elements
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	// no zwrite or ztest while rendering text
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );



	float32 Percentage = m_ProgressPercentage / 100.0f;
	if( Percentage > 1 )
	{
		Percentage = 1;
	}
	if( Percentage < 0 )
	{
		Percentage = 0;
	}
	m_pProgressBar->SetSize( 254 * Percentage, 28 );

	m_pMessageText->SetText( pD3DDevice, m_Text );

	RenderSprites( pD3DDevice );
	RenderButtons( pD3DDevice );
	RenderText( pD3DDevice );
	RenderDropdowns( pD3DDevice );


	// turn back on the zwrite/ztest
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

	// turn alpha blending back on
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	// turn on lighting
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

	if( Flip )
	{
		pD3DDevice->EndScene();
		pD3DDevice->Present( NULL, NULL, NULL, NULL );
	}

} // CLoadingMenu::RenderUI()

void CLoadingMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CLoadingMenu::RenderText()

void CLoadingMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CSprite>* pNode = m_pSpriteList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CLoadingMenu::RenderSprites()

void CLoadingMenu::RenderDropdowns( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CDropdown>* pNode = m_pDropdownList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CLoadingMenu::RenderSprites()

void CLoadingMenu::CloseDropdowns( void )
{
	CLLNode<CDropdown>* pNode = m_pDropdownList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Close();
		pNode = pNode->Next();
	}
} // CLoadingMenu::CloseDropdowns()

void CLoadingMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CLoadingMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CLoadingMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
{
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	bool OverDropdown( kFalse );
	bool DropdownOpen( kFalse );

	CLLNode<CDropdown>* pDNode = m_pDropdownList->GetFirst();
	while( pDNode != NULL )
	{
		pDNode->Data()->Update( MousePosition );
		if( pDNode->Data()->IsOver( MousePosition ) )
		{
			OverDropdown = kTrue;
		}
		if( pDNode->Data()->IsOpen() )
		{
			DropdownOpen = kTrue;
		}
		pDNode = pDNode->Next();
	}

	if( DropdownOpen )
	{
		return;
	}
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Update( MousePosition );
		pNode = pNode->Next();
	}

} // CLoadingMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CLoadingMenu::UpdateButtonPress( bool ButtonPressed,		// left mouse button pressed?
							    bool PlaySounds )		//  play sounds?
{
	if( !ButtonPressed )
	{
		return -1;
	}
	// update the mouse cursor sprite to match the actual cursor position
	POINT MousePosition = m_MouseHandler.VirtualMousePosition( (float32)m_pSettings.GetSettings( KSetScreenWidth ),
															   (float32)m_pSettings.GetSettings( KSetScreenHeight ),
															   1024,
															   768 );
	bool OverDropdown( kFalse );
	bool DropdownOpen( kFalse );

	CLLNode<CDropdown>* pDNode = m_pDropdownList->GetFirst();
	while( pDNode != NULL )
	{
		int32 ReturnCode = pDNode->Data()->UpdateButtonPress( MousePosition, ButtonPressed);
		if( ReturnCode != -1 )
		{
			return ReturnCode;
		}
		if( pDNode->Data()->IsOver( MousePosition ) )
		{
			OverDropdown = kTrue;
		}
		if( pDNode->Data()->IsOpen() )
		{
			DropdownOpen = kTrue;
		}
		pDNode = pDNode->Next();
	}

	if( !OverDropdown )
	{
		CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
		while( pNode != NULL )
		{
			int32 ReturnCode = pNode->Data()->IsOver( MousePosition );
			if( ReturnCode != -1 )
			{
				if( DropdownOpen )
				{
					ReturnCode = -1;
					CloseDropdowns();
				}
				if( PlaySounds )
				{
					pNode->Data()->Press();
				}
				return ReturnCode;
			}
			pNode = pNode->Next();
		}

		if( DropdownOpen )
		{
			m_MouseHandler.FlushAll();
			CloseDropdowns();
		}
	}

	return -1;
} // CLoadingMenu::UpdateButtonPress()

CButton* CLoadingMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CLoadingMenu::AddButton()

void CLoadingMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CLoadingMenu::RemoveButton()

void CLoadingMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CLoadingMenu::DestroyButtons()

CText* CLoadingMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CLoadingMenu::AddText()

void CLoadingMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CLoadingMenu::RemoveText()

void CLoadingMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CLoadingMenu::DestroyText()

CSprite* CLoadingMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CLoadingMenu::AddSprite()

CSprite* CLoadingMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CLoadingMenu::AddSpriteFront()

void CLoadingMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CLoadingMenu::RemoveSprite()

void CLoadingMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CLoadingMenu::DestroySprite()


CDropdown* CLoadingMenu::AddDropdown( CDropdown* pDropdown )	// dropdown to add to the list
{
	m_pDropdownList->Queue( pDropdown );

	return pDropdown;
} // CLoadingMenu::AddDropdown()

CDropdown* CLoadingMenu::AddDropdownFront( CDropdown* pDropdown )	// dropdown to add to the list
{
	m_pDropdownList->QueueEnd( pDropdown );

	return pDropdown;
} // CLoadingMenu::AddDropdownFront()

void CLoadingMenu::RemoveDropdown( CDropdown* pDropdown )	// dropdown to remove from the list
{
	CLLNode<CDropdown>* pNode = m_pDropdownList->GetFirst();
	while( pNode != NULL )
	{
		if( pNode->Data() == pDropdown )
		{
			m_pDropdownList->RemoveNoDelete( pNode );
			return;
		}
		pNode = pNode->Next();
	}
} // CLoadingMenu::RemoveDropdown()

void CLoadingMenu::DestroyDropdowns( void )
{
	m_pDropdownList->Destroy();
} // CLoadingMenu::DestroyDropdown()

