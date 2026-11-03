/*
	menu.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/

#include "menu.h"

CMenu::CMenu( CRefManager& pRefManager,				// the reference manager, for tracking media we create
			  CSettings& pSettings,					// the settings manager, for keeping track of card/machine capabilities
		      CGameStateManager& pGameStateManager,	// game manager
			  CMenuManager& pMenuManager ) :		// menu manager
												m_pRefManager( pRefManager ),
												m_pSettings( pSettings ),
												m_pGameStateManager( pGameStateManager ),
												m_pMenuManager( pMenuManager ),
												m_pButtonList( NULL ),
												m_pSpriteList( NULL ),
												m_pDropdownList( NULL ),
												m_pTextList( NULL ),
												m_UpdateElements( kFalse )
{
	ZeroMemory ( &m_KeyHandler, sizeof ( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler, sizeof ( m_MouseHandler ));

	m_pButtonList = new CLList<CButton>();
	m_pTextList = new CLList<CText>();
	m_pSpriteList = new CLList<CSprite>();
	m_pDropdownList = new CLList<CDropdown>();
} // CMenu::CMenu()

CMenu::~CMenu( void )
{
	
	DELETE_SAFELY( m_pDropdownList );
	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );
} // CMenu::~CMenu()

void CMenu::Activate( void )
{
	// flush our keyboard & mouse handlers
	m_KeyHandler.FlushAll();
	m_MouseHandler.FlushAll();
} // CMenu::Activate()

void CMenu::Deactivate( void )
{
} // CMenu::Deactivate()

// perform creation of any elements that this menu needs
void CMenu::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
} // CMenu::Create()

void CMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					   WPARAM KeyCode )	// the wparam which will be the keycode
{
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CMenu::KeyEvent()

void CMenu::MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONDOWN, etc.
					    WPARAM Param )		// the wparam which will be the button code
{
	m_MouseHandler.MouseEvent( WMMessage, Param );
} // CMenu::KeyEvent()

void CMenu::ProcessInput( HWND hWnd )							// hwnd of the active window
{
	m_MouseHandler.Update( hWnd );

	int32 ReturnCode = UpdateButtonPress( m_MouseHandler.ButtonPressed( KButtonLeft ) );

} // CMenu::ProcessInput()

void CMenu::Update( HWND hWnd,								// hwnd of the active window
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
} // CMenu::Update()

void CMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{

} // CMenu::Render()

// render any UI elements
void CMenu::RenderUI( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	RenderSprites( pD3DDevice );
	RenderButtons( pD3DDevice );
	RenderText( pD3DDevice );
	RenderDropdowns( pD3DDevice );
} // CMenu::RenderUI()

void CMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CText>* pNode = m_pTextList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CMenu::RenderText()

void CMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CSprite>* pNode = m_pSpriteList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CMenu::RenderSprites()

void CMenu::RenderDropdowns( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CDropdown>* pNode = m_pDropdownList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CMenu::RenderSprites()

void CMenu::CloseDropdowns( void )
{
	CLLNode<CDropdown>* pNode = m_pDropdownList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Close();
		pNode = pNode->Next();
	}
} // CMenu::CloseDropdowns()

void CMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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

} // CMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CMenu::UpdateButtonPress( bool ButtonPressed,		// left mouse button pressed?
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
} // CMenu::UpdateButtonPress()

CButton* CMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CMenu::AddButton()

void CMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CMenu::RemoveButton()

void CMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CMenu::DestroyButtons()

CText* CMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CMenu::AddText()

void CMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CMenu::RemoveText()

void CMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CMenu::DestroyText()

CSprite* CMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CMenu::AddSprite()

CSprite* CMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CMenu::AddSpriteFront()

void CMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CMenu::RemoveSprite()

void CMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CMenu::DestroySprite()


CDropdown* CMenu::AddDropdown( CDropdown* pDropdown )	// dropdown to add to the list
{
	m_pDropdownList->Queue( pDropdown );

	return pDropdown;
} // CMenu::AddDropdown()

CDropdown* CMenu::AddDropdownFront( CDropdown* pDropdown )	// dropdown to add to the list
{
	m_pDropdownList->QueueEnd( pDropdown );

	return pDropdown;
} // CMenu::AddDropdownFront()

void CMenu::RemoveDropdown( CDropdown* pDropdown )	// dropdown to remove from the list
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
} // CMenu::RemoveDropdown()

void CMenu::DestroyDropdowns( void )
{
	m_pDropdownList->Destroy();
} // CMenu::DestroyDropdown()

