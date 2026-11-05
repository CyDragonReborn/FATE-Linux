/*
	petinventorymenu.cpp

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
#include "../CHARACTER/inventory.h"
#include "../ITEM/item.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"

#include "../GAMECLIENT/quest.h"

#include "gameui.h"
#include "petinventorymenu.h"
#include "bottommenu.h"

CPetInventoryMenu::CPetInventoryMenu( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							  CRefManager& pRefManager,					// the reference manager, for tracking media we create
							  CSettings& pSettings,						// the settings manager, for keeping track of card/machine capabilities
							  CGameStateManager& pGameStateManager ) :	// to determine whether needed media exists
																		m_pGameStateManager( pGameStateManager ),
																		m_pRefManager( pRefManager ),
																		m_pSettings( pSettings ),
																		m_Open( kFalse ),
																		m_pPetCharacter( NULL ),
																		m_pButtonList	( new CLList<CButton>() ),
																		m_pTextList		( new CLList<CText>() ),
																		m_pSpriteList	( new CLList<CSprite>() ),
																		m_pFontMaterial( NULL ),
																		m_pFontMetric( NULL ),
																		m_pSocketSprite( NULL ),
																		m_pSocketMaterial( NULL ),
																		m_pMiniSprite( NULL ),
																		m_pInventoryMenuTop( NULL ),
																		m_pInventoryMenuCenter( NULL ),
																		m_pInventoryMenuBottom( NULL ),
																		m_pErrorIconMaterial( NULL ),
																		m_pMagicIconMaterial( NULL ),
																		m_pUniqueIconMaterial( NULL ),
																		m_pArtifactIconMaterial( NULL ),
																		m_pQuestItemMaterial( NULL ),
																		m_pGradedItemMaterial( NULL ),
																		m_pEliteItemMaterial( NULL ),
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

} // CPetInventoryMenu::CPetInventoryMenu()

CPetInventoryMenu::~CPetInventoryMenu( void )
{
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		DELETE_SAFELY( m_pEquipmentIcon[i] );
		m_pRefManager.RemoveMaterial( m_pEquipmentMaterial[i] );
	}

	DELETE_SAFELY( m_pSpriteList );
	DELETE_SAFELY( m_pTextList );
	DELETE_SAFELY( m_pButtonList );

	DELETE_SAFELY( m_pSocketSprite );
	DELETE_SAFELY( m_pMiniSprite );

	m_pRefManager.RemoveSample( m_pClickSound );

	m_pRefManager.RemoveFontMetric( m_pFontMetric );
	m_pRefManager.RemoveMaterial( m_pFontMaterial );

	m_pRefManager.RemoveMaterial( m_pUIButtons );

	m_pRefManager.RemoveMaterial( m_pSocketMaterial );

	m_pRefManager.RemoveMaterial( m_pErrorIconMaterial );
	m_pRefManager.RemoveMaterial( m_pMagicIconMaterial );
	m_pRefManager.RemoveMaterial( m_pUniqueIconMaterial );
	m_pRefManager.RemoveMaterial( m_pArtifactIconMaterial );
	m_pRefManager.RemoveMaterial( m_pQuestItemMaterial );
	m_pRefManager.RemoveMaterial( m_pGradedItemMaterial );
	m_pRefManager.RemoveMaterial( m_pEliteItemMaterial );

	m_pRefManager.RemoveMaterial( m_pInventoryMenuTop );
	m_pRefManager.RemoveMaterial( m_pInventoryMenuBottom );
	m_pRefManager.RemoveMaterial( m_pInventoryMenuCenter );

} // CPetInventoryMenu::~CPetInventoryMenu()

void CPetInventoryMenu::FlushAll( void )
{

	m_MouseHandler.FlushAll();
	m_KeyHandler.FlushAll();
} // CPetInventoryMenu::FlushAll()

void CPetInventoryMenu::CreateMenus( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	m_pClickSound = m_pRefManager.AddSample( "SOUNDS/UI/click.wav", kFalse );

	m_pUIButtons = m_pRefManager.AddMaterialAlpha( pD3DDevice,
												   "MENUS/uibuttons.png",
												   "MENUS/uibuttonsa.png" );

	m_pSocketMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														"MENUS/socket.png",
														"MENUS/socketa.png" );

	m_pInventoryMenuTop = m_pRefManager.AddMaterial( pD3DDevice,
													 "MENUS/petinventorytop.png" );

	m_pInventoryMenuBottom = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/petinventorybottom.png"  );

	m_pInventoryMenuCenter = m_pRefManager.AddMaterial( pD3DDevice,
														"MENUS/petinventorycenter.png" );

	m_pErrorIconMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/erroricon.png",
														   "MENUS/erroricona.png" );

	m_pMagicIconMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/magicicon.png",
														   "MENUS/modeicona.png" );

	m_pUniqueIconMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/uniqueicon.png",
														   "MENUS/modeicona.png" );
	m_pArtifactIconMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/artifacticon.png",
														   "MENUS/modeicona.png" );
	m_pQuestItemMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/questitem.png",
														   "MENUS/questitema.png" );

	m_pGradedItemMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/goldstar.png",
														   "MENUS/stara.png" );

	m_pEliteItemMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														   "MENUS/elite.png",
														   "MENUS/elitea.png" );

	AddSprite( new CSprite( pD3DDevice,
							m_pInventoryMenuTop,
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
							m_pInventoryMenuCenter,
							0,
							256,
							512,
							256,
							0,
							.0015f,
							1,
							.9985f,
							KSpriteTopLeft ) );

	AddSprite( new CSprite( pD3DDevice,
							m_pInventoryMenuBottom,
							0,
							512,
							512,
							128,
							0,
							.01f,
							1,
							1,
							KSpriteTopLeft ) );

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
									KButtonPetInventory ) );
	pButton->SetToolTip( LoadLocalizedString(170) );

	// add invisible buttons for the item slots
	for( uint32 i = 0; i < KInventorySlotsWide; i++ )
	{
		for( uint32 j = 0; j < KInventorySlotsHigh; j++ )
		{
			CButton* pButton = AddButton( new CButton( pD3DDevice,
														NULL,
														15 + i * 48.0f,
														378 + j * 48.0f,
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
														KPetInventoryButton + KQuickSlots +
														i + j * KInventorySlotsWide ) );

			pButton->SetAllowClickOnEdge( kTrue );

		}
	}

	// add invisible buttons for equipment slots

	for( int32 i = 0; i < KEquipmentSlots; i++ )
	{
		m_pEquipmentButton[i] = AddButton( new CButton( pD3DDevice,
														NULL,
														KPetEquipmentSlotX[i],
														KPetEquipmentSlotY[i],
														KPetEquipmentSlotWidth[i],
														KPetEquipmentSlotHeight[i],
														0,
														0,
														1,
														1,
														0,
														0,
														1,
														1,
														KButtonTopLeft,
														KPetInventoryButton + i ) );

		// also make the unequipped icon placeholders
		if( KPetEquipmentIconPath[i].length() > 0 )
		{
			m_pEquipmentMaterial[i] = m_pRefManager.AddMaterialAlpha( pD3DDevice,
																	  KPetEquipmentIconPath[i],
																	  KPetEquipmentIconPathAlpha[i] );

			m_pEquipmentIcon[i] =  new CSprite( pD3DDevice,
												m_pEquipmentMaterial[i],
												KPetEquipmentSlotX[i] + KPetEquipmentSlotWidth[i] * .5f - KPetEquipmentIconWidth[i] * .5f,
												KPetEquipmentSlotY[i] + KPetEquipmentSlotHeight[i] * .5f - KPetEquipmentIconHeight[i] * .5f,
												KPetEquipmentIconWidth[i],
												KPetEquipmentIconHeight[i],
												0,
												0,
												1,
												1,
												KSpriteTopLeft );
		}
		else
		{
			m_pEquipmentIcon[i] = NULL;
			m_pEquipmentMaterial[i] = NULL;
		}
	}

	m_pSocketSprite =  new CSprite( pD3DDevice,
									m_pSocketMaterial,
									0,
									0,
									48,
									48,
									0,
									0,
									1,
									1,
									KSpriteCenter );


	m_pMiniSprite =  new CSprite( pD3DDevice,
									m_pSocketMaterial,
									0,
									0,
									32,
									32,
									0,
									0,
									1,
									1,
									KSpriteTopLeft );

} // CPetInventoryMenu::CreateMenus()


void CPetInventoryMenu::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							 WPARAM KeyCode )	// the wparam which will be the keycode
{
	if( !Open() )
	{
		return;
	}
	m_KeyHandler.KeyEvent( WMMessage, KeyCode );
} // CPetInventoryMenu::KeyEvent()

void CPetInventoryMenu::MouseEvent( UINT WMMessage,	// windows message - WM_LBUTTONDOWN, etc.
						  WPARAM WMParam )	// the wparam which contains mousewheel info

{
	if( !Open() )
	{
		return;
	}
	m_MouseHandler.MouseEvent( WMMessage, WMParam );
} // CPetInventoryMenu::KeyEvent()

bool CPetInventoryMenu::MouseOver( void )
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
} // CPetInventory::MouseOver()

void CPetInventoryMenu::ProcessInput( HWND hWnd )	// hwnd of the active window
{
	// update our mouse handler so it has a current position
	m_MouseHandler.Update( hWnd );


} // CPetInventoryMenu::ProcessInput()

void CPetInventoryMenu::Update( HWND hWnd,						// hwnd of the active window
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
} // CPetInventoryMenu::Update()

// render any 3d elements
void CPetInventoryMenu::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
								const D3DXMATRIX& ViewportMatrix,		// viewport matrix ( for bringing 3d objects to screen space
								const D3DXMATRIX& ProjectionMatrix,		// projection matrix  ( for bringing 3d objects to screen space
								CItem* pMouseOverItem,					// mouseover item
								bool ShowSockets )						// show sockets for items?
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
	RenderInventoryIconSockets( pD3DDevice, pMouseOverItem, ShowSockets );

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_MIPFILTER,
									  m_pSettings.GetSettings( KSetMipFilter ) );	
	pD3DDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

} // CPetInventoryMenu::Render()


// render any 3d elements
void CPetInventoryMenu::RenderInventoryIcons( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{

	CLLNode<CItemRef>* Root = m_pPetCharacter->Inventory()->ItemList()->GetFirst();
	while( Root != NULL )
	{
		int32 Index = Root->Data()->m_SlotIndex;

		if( Index >= KEquipmentSlots )
		{
			// we skip rendering of everything in the quickslots
			if( Index >= KQuickSlots )
			{
				Index -= KQuickSlots;

				float32 Row = floor( (float32)Index / (float32)KInventorySlotsWide );
				float32 Column = Index - Row * KInventorySlotsWide;

				if( Root->Data()->m_pItem->HasIcon() )
				{
					// render a purple 'haze' behind the object if it is magical
					if( Root->Data()->m_pItem->Effects( KActivationPassive ) > 0 ||
						Root->Data()->m_pItem->DamageBonuses() > 0 )
					{
						if( Root->Data()->m_pItem->IsArtifact() )
						{
							pD3DDevice->SetTexture( 0, m_pArtifactIconMaterial->GetTexture() );
						}
						else if( Root->Data()->m_pItem->IsUnique() )
						{
							pD3DDevice->SetTexture( 0, m_pUniqueIconMaterial->GetTexture() );
						}
						else
						{
							pD3DDevice->SetTexture( 0, m_pMagicIconMaterial->GetTexture() );
						}
						Root->Data()->m_pItem->IconSprite()->RenderNoTexture( pD3DDevice,
																			  16 + Column * 48,
																			  378 + Row * 48 );		   
					}

					Root->Data()->m_pItem->IconSprite()->Render( pD3DDevice,
																 16 + Column * 48,
																 378 + Row * 48 );	
					
					for( uint32 i = 0; i < m_pPetCharacter->Master()->Quests(); i++ )
					{
						if( m_pPetCharacter->Master()->Quest( i )->ItemMatches( Root->Data()->m_pItem ) )
						{

							pD3DDevice->SetTexture( 0, m_pQuestItemMaterial->GetTexture() );

							m_pMiniSprite->RenderNoTexture( pD3DDevice,
															16 + Column * 48,
															378 + Row * 48 );
						}
					}
					if( Root->Data()->m_pItem->Grade() > KGradeNormal )
					{

						pD3DDevice->SetTexture( 0, m_pGradedItemMaterial->GetTexture() );

						m_pMiniSprite->RenderNoTexture( pD3DDevice,
														16 + Column * 48 + Root->Data()->m_pItem->IconSprite()->Width() - 32,
														378 + Row * 48 );
					}
					if( Root->Data()->m_pItem->Rank() > KRankNormal )
					{

						pD3DDevice->SetTexture( 0, m_pEliteItemMaterial->GetTexture() );

						m_pMiniSprite->RenderNoTexture( pD3DDevice,
														16 + Column * 48 + Root->Data()->m_pItem->IconSprite()->Width() - 32,
														378 + Row * 48 );
					}

				}
			}
		}
		else
		{
			if( Root->Data()->m_pItem->HasIcon() )
			{
				float32 X = KPetEquipmentSlotX[Index] + KPetEquipmentSlotWidth[Index] * .5f;
				float32 Y = KPetEquipmentSlotY[Index] + KPetEquipmentSlotHeight[Index] * .5f;
				X -= Root->Data()->m_pItem->IconSprite()->Width() * .5f;
				Y -= Root->Data()->m_pItem->IconSprite()->Height() * .5f;
			
				// render a purple 'haze' behind the object if it is magical
				if( Root->Data()->m_pItem->Effects( KActivationPassive ) > 0 ||
						Root->Data()->m_pItem->DamageBonuses() > 0 )
				{
					if( Root->Data()->m_pItem->IsArtifact() )
					{
						pD3DDevice->SetTexture( 0, m_pArtifactIconMaterial->GetTexture() );
					}
					else if( Root->Data()->m_pItem->IsUnique() )
					{
						pD3DDevice->SetTexture( 0, m_pUniqueIconMaterial->GetTexture() );
					}
					else
					{
						pD3DDevice->SetTexture( 0, m_pMagicIconMaterial->GetTexture() );
					}
					
					m_pEquipmentButton[Index]->RenderNoTexture( pD3DDevice );
				}
				
				Root->Data()->m_pItem->IconSprite()->Render( pD3DDevice,
															 X,
															 Y );	   
						
				for( uint32 i = 0; i < m_pPetCharacter->Master()->Quests(); i++ )
				{
					if( m_pPetCharacter->Master()->Quest( i )->ItemMatches( Root->Data()->m_pItem ) )
					{

						pD3DDevice->SetTexture( 0, m_pQuestItemMaterial->GetTexture() );

						m_pMiniSprite->RenderNoTexture( pD3DDevice,
														KPetEquipmentSlotX[Index],
														KPetEquipmentSlotY[Index] );
					}
				}				
				if( Root->Data()->m_pItem->Grade() > KGradeNormal )
				{

					pD3DDevice->SetTexture( 0, m_pGradedItemMaterial->GetTexture() );

					m_pMiniSprite->RenderNoTexture( pD3DDevice,
													KPetEquipmentSlotX[Index] + KPetEquipmentSlotWidth[Index] - 32,
													KPetEquipmentSlotY[Index] );
				}
				if( Root->Data()->m_pItem->Rank() > KRankNormal )
				{

					pD3DDevice->SetTexture( 0, m_pEliteItemMaterial->GetTexture() );

					m_pMiniSprite->RenderNoTexture( pD3DDevice,
													KPetEquipmentSlotX[Index] + KPetEquipmentSlotWidth[Index] - 32,
													KPetEquipmentSlotY[Index] );
				}
			}
		}
		Root = Root->Next();
	}

	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pPetCharacter->Inventory()->GetItemFromSlot( i ) == NULL &&
			m_pEquipmentIcon[i] != NULL )
		{
			if( !( i == KSlotLeftArm && 
				   m_pPetCharacter->Inventory()->GetItemFromSlot( KSlotLeftHand ) != NULL ) )
			{			    
				m_pEquipmentIcon[i]->Render( pD3DDevice );
			}
		}
	}

} // CPetInventoryMenu::RenderInventoryIcons()

// render any 3d elements
void CPetInventoryMenu::RenderInventoryIconSockets( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
													CItem* pMouseOverItem,			// mouseover item
													bool ShowSockets )				// show sockets for items?
{

	CLLNode<CItemRef>* Root = m_pPetCharacter->Inventory()->ItemList()->GetFirst();
	while( Root != NULL )
	{
		int32 Index = Root->Data()->m_SlotIndex;

		if( Index >= KEquipmentSlots )
		{
			// we skip rendering of everything in the quickslots
			if( Index >= KQuickSlots )
			{
				Index -= KQuickSlots;

				float32 Row = floor( (float32)Index / (float32)KInventorySlotsWide );
				float32 Column = Index - Row * KInventorySlotsWide;

				float32 YOffset = (float32)Root->Data()->m_pItem->Sockets() * -48;
				YOffset *= .5f;
				YOffset += 24;
				if( Root->Data()->m_pItem->HasIcon() &&
					Root->Data()->m_pItem->Socketed() &&
					( Root->Data()->m_pItem == pMouseOverItem ||
					  ShowSockets ) )
				{
					for( uint32 i = 0; i < Root->Data()->m_pItem->Sockets(); i++ )
					{

						m_pSocketSprite->Render( pD3DDevice,
												 16 + Column * 48 + Root->Data()->m_pItem->IconSprite()->Width() *.5f,
												 378 + Row * 48 + YOffset + Root->Data()->m_pItem->IconSprite()->Height() *.5f );
						if( i < Root->Data()->m_pItem->Items() )
						{
							pD3DDevice->SetTexture( 0, Root->Data()->m_pItem->SubItem( i )->IconMaterial()->GetTexture() );
							m_pSocketSprite->RenderNoTexture( pD3DDevice,
															  16 + Column * 48 + Root->Data()->m_pItem->IconSprite()->Width() *.5f,
															  378 + Row * 48 + YOffset + Root->Data()->m_pItem->IconSprite()->Height() *.5f );
						}

						YOffset += 48;
					}
				}
			}
		}
		else
		{
			if( Root->Data()->m_pItem->HasIcon() &&
				Root->Data()->m_pItem->Socketed() &&
				( Root->Data()->m_pItem == pMouseOverItem ||
				  ShowSockets ) )
			{
				float32 X = KPetEquipmentSlotX[Index] + KPetEquipmentSlotWidth[Index] * .5f;
				float32 Y = KPetEquipmentSlotY[Index] + KPetEquipmentSlotHeight[Index] * .5f;
				X -= Root->Data()->m_pItem->IconSprite()->Width() * .5f;
				Y -= Root->Data()->m_pItem->IconSprite()->Height() * .5f;

				float32 YOffset = (float32)Root->Data()->m_pItem->Sockets() * -48;
				YOffset *= .5f;
				YOffset += 24;
				if( Root->Data()->m_pItem->HasIcon() &&
					Root->Data()->m_pItem->Socketed() )
				{
					for( uint32 i = 0; i < Root->Data()->m_pItem->Sockets(); i++ )
					{
						m_pSocketSprite->Render( pD3DDevice,
												 X + Root->Data()->m_pItem->IconSprite()->Width() *.5f,
												 Y + YOffset + Root->Data()->m_pItem->IconSprite()->Height() *.5f );

						if( i < Root->Data()->m_pItem->Items() )
						{
							pD3DDevice->SetTexture( 0, Root->Data()->m_pItem->SubItem( i )->IconMaterial()->GetTexture() );
							m_pSocketSprite->RenderNoTexture( pD3DDevice,
															  X + Root->Data()->m_pItem->IconSprite()->Width() *.5f,
															  Y + YOffset + Root->Data()->m_pItem->IconSprite()->Height() *.5f );
						}

						YOffset += 48;
					}
				}									   
			}
		}
		Root = Root->Next();
	}

} // CPetInventoryMenu::RenderInventoryIconSockets()


void CPetInventoryMenu::RenderText( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CPetInventoryMenu::RenderText()

void CPetInventoryMenu::RenderSprites( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
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
} // CPetInventoryMenu::RenderSprites()

void CPetInventoryMenu::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}

} // CPetInventoryMenu::RenderButtons()

// mouse handler should be updated for this to work properly
void CPetInventoryMenu::UpdateButtons( float32 TimeElapsed )		// time elapsed in seconds
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
} // CPetInventoryMenu::UpdateButtons()

// mouse handler should be updated for this to work properly
int32 CPetInventoryMenu::UpdateButtonPress( bool ButtonPressed )		// left mouse button pressed?
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
} // CPetInventoryMenu::UpdateButtonPress()

// mouse handler should be updated for this to work properly
int32 CPetInventoryMenu::UpdateButtonOver( void )
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
} // CPetInventoryMenu::UpdateButtonOver()

// mouse handler should be updated for this to work properly
int32 CPetInventoryMenu::UpdateButtonOver( std::string& TooltipText )	// tooltip text to fill out
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
} // CPetInventoryMenu::UpdateButtonOver()

CButton* CPetInventoryMenu::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CPetInventoryMenu::AddButton()

void CPetInventoryMenu::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CPetInventoryMenu::RemoveButton()

void CPetInventoryMenu::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CPetInventoryMenu::DestroyButtons()

CText* CPetInventoryMenu::AddText( CText* pText )	// Text to add to the list
{
	m_pTextList->Queue( pText );

	return pText;
} // CPetInventoryMenu::AddText()

void CPetInventoryMenu::RemoveText( CText* pText )	// Text to remove from the list
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
} // CPetInventoryMenu::RemoveText()

void CPetInventoryMenu::DestroyText( void )
{
	m_pTextList->Destroy();
} // CPetInventoryMenu::DestroyText()

CSprite* CPetInventoryMenu::AddSprite( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->Queue( pSprite );

	return pSprite;
} // CPetInventoryMenu::AddSprite()

CSprite* CPetInventoryMenu::AddSpriteFront( CSprite* pSprite )	// sprite to add to the list
{
	m_pSpriteList->QueueEnd( pSprite );

	return pSprite;
} // CPetInventoryMenu::AddSpriteFront()

void CPetInventoryMenu::RemoveSprite( CSprite* pSprite )	// sprite to remove from the list
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
} // CPetInventoryMenu::RemoveSprite()

void CPetInventoryMenu::DestroySprites( void )
{
	m_pSpriteList->Destroy();
} // CPetInventoryMenu::DestroySprite()

