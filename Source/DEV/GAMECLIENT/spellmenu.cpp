/*
	spellmenu.cpp

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
#include "../ITEM/spelltemplate.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "gameui.h"
#include "spellmenu.h"
#include "bottommenu.h"

CSpellMenu::CSpellMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							  CRefManager& pRefManager,					// the reference manager, for tracking media we create
							  CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
							  CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																		m_pGameStateManager( pGameStateManager ),
																		m_pRefManager( pRefManager ),
																		m_pSettings( pSettings ),
																		m_pButtonList	( new CLList<CButton>() ),
																		m_pTextList		( new CLList<CText>() ),
																		m_pSpriteList	( new CLList<CSprite>() ),
																		m_Open( kFalse ),
																		m_pIconSprite( NULL ),
																		m_pPlayerCharacter( NULL ),
																		m_pFontMaterial( NULL ),
																		m_pFontMetric( NULL ),
																		m_pSpellMenuTop( NULL ),
																		m_pSpellMenuCenter( NULL ),
																		m_pSpellMenuBottom( NULL ),
																		m_pCrossMaterial( NULL ),
																		m_pUIButtons( NULL ),
																		m_pAttackSkill( NULL ),
																		m_pDefenseSkill( NULL ),
																		m_pCharmSkill( NULL ),
																		m_pClickSound( NULL )
{
	ZeroMemory ( &m_KeyHandler,		sizeof( m_KeyHandler ));
	ZeroMemory ( &m_MouseHandler,	sizeof( m_MouseHandler ));


	m_pIconSprite = new CSprite( pD3DDevice,
								 NULL,
								 0,
								 0,
								 48,
								 48,
								 0,
								 0,
								 1,
								 1,
								 KSpriteTopLeft );

	// initialize font texture and metric
	m_pFontMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
													  "FONT/font.png",
													  "FONT/fonta.png" );

	m_pFontMetric = m_pRefManager.AddFontMetric( "FONT/font.ini" );

	CreateMenus( pD3DDevice );

} // CSpellMenu::CSpellMenu()

CSpellMenu::~CSpellMenu( void )
{
	DELETE_SAFELY( m_pIconSprite );

	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	m_pRefManager.RemoveSample( m_pClickSound );


	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	m_pRefManager.RemoveMaterial( m_pCrossMaterial );

	m_pRefManager.RemoveMaterial( m_pSpellMenuTop );
	m_pRefManager.RemoveMaterial( m_pSpellMenuBottom );
	m_pRefManager.RemoveMaterial( m_pSpellMenuCenter );

} // CSpellMenu::~CSpellMenu()

void CSpellMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CSpellMenu::FlushAll()

void CSpellMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pSpellMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/spelltop.png" );

	m_pSpellMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/spellbottom.png"  );

	m_pSpellMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/spellcenter.png" );

	m_pCrossMaterial	= m_pRefManager.AddMaterialAlpha( pD3DDevice, 
														  "MENUS/cross.png",
														  "MENUS/crossa.png" );

	CButton* pButton;

	pButton = AddButton( new CButton( pD3DDevice,
									m_pUIButtons,
									NULL,
									m_pClickSound,
									453,
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
									KBottomButtonMagic ) );
	pButton->SetToolTip( "Close" );


	AddSprite( new CSprite( pD3DDevice,
							m_pSpellMenuTop,
							0,
							0,
							512,
							256,
							0,
							0,
							1,
							.9985f,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pSpellMenuCenter,
							0,
							256,
							512,
							256,
							0,
							.0015f,
							1,
							1,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pSpellMenuBottom,
							0,
							512,
							512,
							128,
							0,
							0.01f,
							1,
							1,
							KSpriteTopLeft ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(982),
						253,
						100,
						.8f,
						kFalse,
						KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(983),
						160,
						220,
						.8f,
						kTrue,
						KJustifyCenter ) );

	m_pAttackSkill = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"0",
							381,
							218,
							.9f,
							kFalse,
							KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(984),
						160,
						320,
						.8f,
						kTrue,
						KJustifyCenter ) );

	m_pDefenseSkill = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"0",
							381,
							318,
							.9f,
							kFalse,
							KJustifyCenter ) );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(985),
						160,
						420,
						.8f,
						kTrue,
						KJustifyCenter ) );

	m_pCharmSkill = AddText( new CText( pD3DDevice,
							m_pFontMaterial,
							m_pFontMetric,
							"0",
							381,
							418,
							.9f,
							kFalse,
							KJustifyCenter ) );

	// add invisible buttons for the item slots
	for( uint32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			CButton* pButton = AddButton( new CButton( pD3DDevice,
														NULL,
														87 + (float32)j * 58,
														257 + (float32)i * 100,
														48,
														48,
														0,
														0,
														1,
														1,
														0,
														0,
														1,
														1,
														KButtonTopLeft,
														KAttackSpellButton +
														i * 1000 + j ) );
			pButton->SetAllowClickOnEdge( kTrue );

		}
	}

	pButton = AddButton( new CButton( pD3DDevice,
										NULL,
										230,
										143,
										48,
										48,
										0,
										0,
										1,
										1,
										0,
										0,
										1,
										1,
										KButtonTopLeft,
										KActiveSpellButton ) );
	pButton->SetAllowClickOnEdge( kTrue );

	AddText( new CText( pD3DDevice,
						m_pFontMaterial,
						m_pFontMetric,
						LoadLocalizedString(986),
						253,
						510,
						.8f,
						kFalse,
						KJustifyCenter ) );

} // CSpellMenu::CreateMenus()


void CSpellMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CSpellMenu::KeyEvent()

void CSpellMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CSpellMenu::KeyEvent()

bool CSpellMenu::MouseOver( void )
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
} // CSpell::MouseOver()

void CSpellMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CSpellMenu::ProcessInput()

void CSpellMenu::Update( HWND hWnd,						// hwnd of the active window
					  float32 TimeElapsed )				// time elapsed, in seconds
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
} // CSpellMenu::Update()

// render any 3d elements
void CSpellMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
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


	if( m_pPlayerCharacter != NULL )
	{
		m_pAttackSkill->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->SkillPoints( KSkillAttackMagic ) ) );
		m_pDefenseSkill->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->SkillPoints( KSkillDefenseMagic ) ) );
		m_pCharmSkill->SetText( pD3DDevice, IntegerToString( m_pPlayerCharacter->SkillPoints( KSkillCharmMagic ) ) );
	}

	RenderSpellIcons( pD3DDevice );
	RenderText( pD3DDevice );		

} // CSpellMenu::Render()

void CSpellMenu::RenderSpellIcons( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	for( uint32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( m_pPlayerCharacter->SpellInSlot( (EMagicSphere)i, j ) )
			{
				
				pD3DDevice->SetTexture( 0, 
										m_pPlayerCharacter->Spell( (EMagicSphere)i, j )->IconMaterial()->GetTexture() );
				m_pIconSprite->RenderNoTexture( pD3DDevice,
												87 + (float32)j * 58,
												257 + (float32)i * 100 );
				
				if( !m_pPlayerCharacter->Spell( (EMagicSphere)i, j )->RequirementsMet( *m_pPlayerCharacter ) )
				{
					pD3DDevice->SetTexture( 0, 
											m_pCrossMaterial->GetTexture() );
					m_pIconSprite->RenderNoTexture( pD3DDevice,
													87 + (float32)j * 58,
													257 + (float32)i * 100 );
				}
			}
		}
	}

	if( m_pPlayerCharacter->ActiveSpell() != NULL )
	{
				
		pD3DDevice->SetTexture( 0, 
								m_pPlayerCharacter->ActiveSpell()->IconMaterial()->GetTexture() );
		m_pIconSprite->RenderNoTexture( pD3DDevice,
										230,
										143 );		   
	}

} // CSpellMenu::RenderSpellIcons()



void CSpellMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CSpellMenu::RenderText()

void CSpellMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CSpellMenu::RenderSprites()

void CSpellMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CSpellMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CSpellMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CSpellMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CSpellMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CSpellMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CSpellMenu::UpdateButtonOver( void )
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
} // CSpellMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CSpellMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CSpellMenu::UpdateButtonOver()

CButton* CSpellMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CSpellMenu::AddButton()

void CSpellMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CSpellMenu::RemoveButton()

void CSpellMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CSpellMenu::DestroyButtons()

CText* CSpellMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CSpellMenu::AddText()

void CSpellMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CSpellMenu::RemoveText()

void CSpellMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CSpellMenu::DestroyText()

CSprite* CSpellMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CSpellMenu::AddSprite()

CSprite* CSpellMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CSpellMenu::AddSpriteFront()

void CSpellMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CSpellMenu::RemoveSprite()

void CSpellMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CSpellMenu::DestroySprite()


