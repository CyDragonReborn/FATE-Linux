/*
	dropdown.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/8/2004

*/

#include "dropdown.h"

CDropdown::CDropdown( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					  CMaterial* pSpriteMaterial,			// material for backdrop
					  CMaterial* pFontMaterial,				// material for sprite
					  CFontMetric* pFontMetric,				// font metric for items
					  float32 X,							// x position
					  float32 Y,							// y position
					  float32 Width,						// width of dropdown
					  float32 TopHeight,					// height of top item
					  float32 BottomHeight,					// height of bottom item
					  float32 ItemHeight,					// height of individual items
					  float32 TextScale,					// scale of text within
					  float32 TextOffsetX,					// text offset per button
					  float32 TextOffsetY,					// text offset per button
					  float32 TopU1,						// top u
					  float32 TopV1,						// top v
					  float32 TopU2,						// top u2
					  float32 TopV2,						// top v2
					  float32 MidU1,						// mid u
					  float32 MidV1,						// mid v
					  float32 MidU2,						// mid u2
					  float32 MidV2,						// mid v2
					  float32 BotU1,						// bot u
					  float32 BotV1,						// bot v
					  float32 BotU2,						// bot u2
					  float32 BotV2 ) :						// bot v2					  
										m_pButtonList( NULL ),
										m_pTopSprite( NULL ),
										m_pMidSprite( NULL ),
										m_pBotSprite( NULL ),
										m_UpdateElements( kFalse ),
										m_Open( kFalse ),
										m_pFontMaterial( pFontMaterial ),
										m_pSpriteMaterial( pSpriteMaterial ),
										m_pFontMetric( pFontMetric ),
										m_TextScale( TextScale ),
										m_X( X ),
										m_Y( Y ),
										m_TextOffsetX( TextOffsetX ),
										m_TextOffsetY( TextOffsetY ),
										m_Width( Width ),
										m_TopHeight( TopHeight ),
										m_BottomHeight( BottomHeight ),
										m_ItemHeight( ItemHeight ),
										m_TopU1( TopU1 ),
										m_TopV1( TopV1 ),
										m_TopU2( TopU2 ),
										m_TopV2( TopV2 ),
										m_MidU1( MidU1 ),
										m_MidV1( MidV1 ),
										m_MidU2( MidU2 ),
										m_MidV2( MidV2 ),
										m_BotU1( BotU1 ),
										m_BotV1( BotV1 ),
										m_BotU2( BotU2 ),
										m_BotV2( BotV2 )
{
	m_pButtonList = new CLList<CButton>();
	Create( pD3DDevice );
} // CDropdown::CDropdown()

CDropdown::~CDropdown( void )
{
	DELETE_SAFELY( m_pTopSprite );
	DELETE_SAFELY( m_pMidSprite );
	DELETE_SAFELY( m_pBotSprite );

	DELETE_SAFELY( m_pButtonList );
} // CDropdown::~CDropdown()

// perform creation of any elements that this menu needs
void CDropdown::Create( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	m_pTopSprite = new CSprite( pD3DDevice,
								m_pSpriteMaterial,
								m_X,
								m_Y,
								m_Width,
								m_TopHeight,
								m_TopU1,
								m_TopV1,
								m_TopU2,
								m_TopV2,
								KSpriteTopLeft );

	m_pMidSprite = new CSprite( pD3DDevice,
								m_pSpriteMaterial,
								m_X,
								m_Y + m_TopHeight,
								m_Width,
								1,
								m_MidU1,
								m_MidV1,
								m_MidU2,
								m_MidV2,
								KSpriteTopLeft );

	m_pBotSprite = new CSprite( pD3DDevice,
								m_pSpriteMaterial,
								m_X,
								m_Y + m_TopHeight + 1,
								m_Width,
								m_BottomHeight,
								m_BotU1,
								m_BotV1,
								m_BotU2,
								m_BotV2,
								KSpriteTopLeft );
} // CDropdown::Create()

bool CDropdown::IsOver( const POINT& MousePosition )	// current screen mouse position 
{
	if( !m_Open )
	{
		return kFalse;
	}
	if( MousePosition.x >= m_X && MousePosition.x <= m_X + m_Width &&
		MousePosition.y >= m_Y && MousePosition.y <= ( m_Y + 
													   m_TopHeight +
													   m_BottomHeight +
													   m_ItemHeight * m_EntryText.size() ) )
	{
		return kTrue;
	}
	return kFalse;
} // CDropdown::IsOver();

void CDropdown::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	if( !m_Open )
	{
		return;
	}
	pD3DDevice->SetTexture( 0, m_pSpriteMaterial->GetTexture() );
	m_pTopSprite->RenderNoTexture( pD3DDevice );
	m_pMidSprite->RenderNoTexture( pD3DDevice );
	m_pBotSprite->RenderNoTexture( pD3DDevice );

	RenderButtons( pD3DDevice );

	if( m_UpdateElements )
	{
		m_pMidSprite->SetSize( m_Width, m_EntryText.size() * m_ItemHeight );
		m_pBotSprite->SetPosition( m_X, m_Y + m_TopHeight + m_EntryText.size() * m_ItemHeight );
		m_UpdateElements = kFalse;
		DestroyButtons();
		for( uint32 i = 0; i < m_EntryText.size(); i++ )
		{
			CButton* pButton = AddButton( new CButton( pD3DDevice,
														NULL,
														NULL,
														NULL,
														m_X,
														m_Y + m_TopHeight + i * m_ItemHeight,
														m_Width,
														m_ItemHeight - 1,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														0,
														KButtonTopLeft,
														m_EntryCode[i] ) );

			pButton->AddText( pD3DDevice,
							  m_EntryText[i],
							  m_pFontMaterial,
							  m_pFontMetric,
							  m_TextOffsetX,
							  m_TextOffsetY,
							  m_TextScale,
							  kFalse,
							  KJustifyLeft );
			pButton->AddOverText( pD3DDevice,
								  m_EntryText[i],
								  m_pFontMaterial,
								  m_pFontMetric,
								  m_TextOffsetX,
								  m_TextOffsetY,
								  m_TextScale,
								  kTrue,
								  KJustifyLeft );

		}
	}

} // CDropdown::Render()

void CDropdown::RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice )		// d3d device
{
	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Render( pD3DDevice );
		pNode = pNode->Next();
	}
} // CDropdown::RenderButtons()

// mouse handler should be updated for this to work properly
void CDropdown::Update( const POINT& MousePosition )	// current screen mouse position 
{
	if( !m_Open )
	{
		return;
	}

	CLLNode<CButton>* pNode = m_pButtonList->GetFirst();
	while( pNode != NULL )
	{
		pNode->Data()->Update( MousePosition );
		pNode = pNode->Next();
	}
} // CDropdown::Update()

// mouse handler should be updated for this to work properly
int32 CDropdown::UpdateButtonPress( const POINT& MousePosition,		// current screen mouse position 
									bool ButtonPressed )			// left mouse button pressed?
{
	if( !m_Open )
	{
		return -1;
	}

	if( !ButtonPressed )
	{
		return -1;
	}

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
} // CDropdown::UpdateButtonPress()

CButton* CDropdown::AddButton( CButton* pButton )	// button to add to the list
{
	m_pButtonList->Queue( pButton );

	return pButton;
} // CDropdown::AddButton()

void CDropdown::RemoveButton( CButton* pButton )	// button to remove from the list
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
} // CDropdown::RemoveButton()

void CDropdown::DestroyButtons( void )
{
	m_pButtonList->Destroy();
} // CDropdown::DestroyButtons()
