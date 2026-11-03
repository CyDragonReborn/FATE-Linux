/*
	button.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	7/12/2004
	
*/
#include <D3D8.h>
#include <d3dx8.h>

#include <string>
#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/math3d.h"
#include "../MEDIA/material.h"

#include "fmod.h"

#include "text.h"
#include "button.h"

CButton::CButton( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
				  CMaterial* pMaterial,				// material of the sprite texture we want to use
				  float32 X,						// x location onscreen (pixel coords from top left)
				  float32 Y,						// y location onscreen (pixel coords from top left)
				  float32 Width,					// width of the sprite, in pixels
				  float32 Height,					// height of the sprite, in pixels
				  float32 TopLeftU,					// u texture coord at top left 
				  float32 TopLeftV,					// v texture coord at top left 
				  float32 BottomRightU,				// u texture coord at bottom right 
				  float32 BottomRightV,				// v texture coord at bottom right
				  float32 OverTopLeftU,				// u texture coord at top left of rollover
				  float32 OverTopLeftV,				// v texture coord at top left of rollover
				  float32 OverBottomRightU,			// u texture coord at bottom right of rollover
				  float32 OverBottomRightV,			// v texture coord at bottom right of rollover
				  EButtonCenter CenterPoint,		// centerpoint of the sprite
				  int32 ButtonCode,					// code that is returned on click ( default -1 )
				  uint32 ScreenWidth,				// screen width used in orthographic projection
				  uint32 ScreenHeight ) :			// screen height used in orthographic projection
				  m_pVB( NULL ),
				  m_pButtonText( NULL ),
				  m_pButtonOverText( NULL ),
				  m_pMaterial( pMaterial ),
				  m_TopLeftU( TopLeftU ),
				  m_TopLeftV( TopLeftV ),
				  m_BottomRightU( BottomRightU ),
				  m_BottomRightV( BottomRightV ),
				  m_OverTopLeftU( OverTopLeftU ),
				  m_OverTopLeftV( OverTopLeftV ),
				  m_OverBottomRightU( OverBottomRightU ),
				  m_OverBottomRightV( OverBottomRightV ),
				  m_Width( Width ),
				  m_Height( Height ),
				  m_X( X ),
				  m_Y( Y ),
				  m_ScreenX( X ),
				  m_ScreenY( Y ),
				  m_ScreenWidth( ScreenWidth ),
				  m_ScreenHeight( ScreenHeight ),
				  m_CenterPoint( CenterPoint ),
				  m_ButtonState( KButtonOff ),
				  m_SpriteButtonState( KButtonOff ),
				  m_ButtonCode( ButtonCode ),
				  m_ToolTipText( "" ),
				  m_pOverSound( NULL ),
				  m_pClickSound( NULL ),
				  m_AllowClickOnEdge( kFalse ),
				  m_TextX ( 0.0f ),
				  m_TextY ( 0.0f ),
				  m_OverTextX ( 0.0f ),
				  m_OverTextY ( 0.0f ),
				  m_ScreenTopLeftX ( 0.0f ),
				  m_ScreenTopLeftY ( 0.0f ),
				  m_ScreenBottomRightX ( 0.0f ),
				  m_ScreenBottomRightY ( 0.0f )
{
	CreateVB( pD3DDevice,
			  m_Width,
			  m_Height,
			  TopLeftU,
			  TopLeftV,
			  BottomRightU,
			  BottomRightV );

	SetPosition( X, Y, ScreenWidth, ScreenHeight );

} // CButton::CButton()

CButton::CButton( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
				  CMaterial* pMaterial,				// material of the sprite texture we want to use
				  FSOUND_SAMPLE* pOverSound,		// over sound
				  FSOUND_SAMPLE* pClickSound,		// click sound
				  float32 X,						// x location onscreen (pixel coords from top left)
				  float32 Y,						// y location onscreen (pixel coords from top left)
				  float32 Width,					// width of the sprite, in pixels
				  float32 Height,					// height of the sprite, in pixels
				  float32 TopLeftU,					// u texture coord at top left 
				  float32 TopLeftV,					// v texture coord at top left 
				  float32 BottomRightU,				// u texture coord at bottom right 
				  float32 BottomRightV,				// v texture coord at bottom right
				  float32 OverTopLeftU,				// u texture coord at top left of rollover
				  float32 OverTopLeftV,				// v texture coord at top left of rollover
				  float32 OverBottomRightU,			// u texture coord at bottom right of rollover
				  float32 OverBottomRightV,			// v texture coord at bottom right of rollover
				  EButtonCenter CenterPoint,		// centerpoint of the sprite
				  int32 ButtonCode,					// code that is returned on click ( default -1 )
				  uint32 ScreenWidth,				// screen width used in orthographic projection
				  uint32 ScreenHeight ) :			// screen height used in orthographic projection
				  m_pVB( NULL ),
				  m_pButtonText( NULL ),
				  m_pButtonOverText( NULL ),
				  m_pMaterial( pMaterial ),
				  m_TopLeftU( TopLeftU ),
				  m_TopLeftV( TopLeftV ),
				  m_BottomRightU( BottomRightU ),
				  m_BottomRightV( BottomRightV ),
				  m_OverTopLeftU( OverTopLeftU ),
				  m_OverTopLeftV( OverTopLeftV ),
				  m_OverBottomRightU( OverBottomRightU ),
				  m_OverBottomRightV( OverBottomRightV ),
				  m_Width( Width ),
				  m_Height( Height ),
				  m_X( X ),
				  m_Y( Y ),
				  m_ScreenX( X ),
				  m_ScreenY( Y ),
				  m_ScreenWidth( ScreenWidth ),
				  m_ScreenHeight( ScreenHeight ),
				  m_CenterPoint( CenterPoint ),
				  m_ButtonState( KButtonOff ),
				  m_SpriteButtonState( KButtonOff ),
				  m_ButtonCode( ButtonCode ),
				  m_ToolTipText( "" ),
				  m_pOverSound( pOverSound ),
				  m_pClickSound( pClickSound )
{
	CreateVB( pD3DDevice,
			  m_Width,
			  m_Height,
			  TopLeftU,
			  TopLeftV,
			  BottomRightU,
			  BottomRightV );

	SetPosition( X, Y, ScreenWidth, ScreenHeight );

} // CButton::CButton()

CButton::~CButton( void )
{
	m_pMaterial = NULL;

	DELETE_SAFELY( m_pButtonText );
	DELETE_SAFELY( m_pButtonOverText );

	RELEASE_SAFELY( m_pVB );
}

void CButton::Update( const POINT& MousePosition )	// current screen mouse position 
{
	if( OverButton( MousePosition ) )
	{
		if( m_ButtonState != KButtonOver &&
			m_pOverSound != NULL )
		{
			FSOUND_PlaySound( FSOUND_FREE, m_pOverSound );
		}
		m_ButtonState = KButtonOver;
	}
	else
	{
		m_ButtonState = KButtonOff;
	}
	UpdateUVs();
} // CButton::Update()

void CButton::SetText( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
					   const std::string& Text )			// button text to add 
{
	if( m_pButtonText == NULL )
	{
		return;
	}

	m_pButtonText->SetText( pD3DDevice, Text );
} // CButton::SetText()

void CButton::SetOverText( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
						   const std::string& Text )			// button text to add 
{
	if( m_pButtonOverText == NULL )
	{
		return;
	}

	m_pButtonOverText->SetText( pD3DDevice, Text );
} // CButton::SetOverText()

void CButton::AddText( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
					   const std::string& Text,			// button text to add 
					   CMaterial* pMaterial,			// material to use when rendering text
					   CFontMetric* pFontMetric,		// font metric text is based upon
					   float32 X,						// x offset from button centerpoint
					   float32 Y,						// y offset from button centerpoint
					   float32 Scale,					// scale of text
					   bool Bold,						// is text bolded?
					   EJustification Justification,	// text justification
					   uint32 ScreenWidth,				// screen width of ui elements (default 1024)
					   uint32 ScreenHeight ) 			// screen height of ui elements (default 768)

{
	DELETE_SAFELY( m_pButtonText );
	m_TextX = X;
	m_TextY = Y;
	m_pButtonText = new CText( pD3DDevice, 
							   pMaterial,
							   pFontMetric,
							   Text,
							   0,
							   0,
							   Scale,
							   Bold,
							   Justification,
							   ScreenWidth,
							   ScreenHeight );
	UpdateTextPosition( ScreenWidth, ScreenHeight );
								
} // CButton::AddText()

void CButton::AddOverText( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
						   const std::string& Text,			// button text to add 
						   CMaterial* pMaterial,			// material to use when rendering text
						   CFontMetric* pFontMetric,		// font metric text is based upon
						   float32 X,						// x offset from button centerpoint
						   float32 Y,						// y offset from button centerpoint
						   float32 Scale,					// scale of text
						   bool Bold,						// is text bolded?
						   EJustification Justification,	// text justification
						   uint32 ScreenWidth,				// screen width of ui elements (default 1024)
						   uint32 ScreenHeight ) 			// screen height of ui elements (default 768)

{
	DELETE_SAFELY( m_pButtonOverText );
	m_OverTextX = X;
	m_OverTextY = Y;
	m_pButtonOverText = new CText( pD3DDevice, 
								   pMaterial,
								   pFontMetric,
								   Text,
								   0,
								   0,
								   Scale,
								   Bold,
								   Justification,
								   ScreenWidth,
								   ScreenHeight );
	UpdateTextPosition( ScreenWidth, ScreenHeight );
								
} // CButton::AddOverText()

void CButton::ClearText( void )
{
	DELETE_SAFELY( m_pButtonText );
	DELETE_SAFELY( m_pButtonOverText );
} // CButton::ClearText()

void CButton::UpdateTextPosition( uint32 ScreenWidth,		// screen width of ui elements (default 1024)
								  uint32 ScreenHeight )		// screen height of ui elements (default 768)
{
	if( m_pButtonText != NULL )
	{
		m_pButtonText->SetPosition( m_ScreenX + m_TextX,
									m_ScreenY + m_TextY,
									ScreenWidth,
									ScreenHeight );
	}

	if( m_pButtonOverText != NULL )
	{
		m_pButtonOverText->SetPosition( m_ScreenX + m_OverTextX,
										m_ScreenY + m_OverTextY,
										ScreenWidth,
										ScreenHeight );
	}
} // CButton::UpdateTextPosition()

int32 CButton::IsOver( const POINT& Position )	// position to check
{
	if( OverButton( Position ) )
	{
		return m_ButtonCode;
	}
	return -1;
} // CButton::IsOver()


void CButton::Press( void )
{
	FSOUND_PlaySound( FSOUND_FREE, m_pClickSound );
} // CButton::Press()

bool CButton::OverButton( const POINT& Position )	// position to check
{
	if( m_AllowClickOnEdge )
	{
		if( Position.x >= m_ScreenTopLeftX &&
			Position.x <= m_ScreenBottomRightX &&
			Position.y >= m_ScreenTopLeftY &&
			Position.y <= m_ScreenBottomRightY )
		{
			return kTrue;
		}
	}
	else
	{
		if( Position.x > m_ScreenTopLeftX &&
			Position.x < m_ScreenBottomRightX &&
			Position.y > m_ScreenTopLeftY &&
			Position.y < m_ScreenBottomRightY )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CButton::OverButton()

void CButton::UpdateScreenBounds( void )
{
	switch( m_CenterPoint )
	{
	case KButtonTopLeft :
		m_ScreenTopLeftX = m_ScreenX;
		m_ScreenTopLeftY = m_ScreenY;
		m_ScreenBottomRightX = m_ScreenX + m_Width;
		m_ScreenBottomRightY = m_ScreenY + m_Height;
		break;
	case KButtonCenter :
		m_ScreenTopLeftX = m_ScreenX - m_Width * .5f;
		m_ScreenTopLeftY = m_ScreenY - m_Height * .5f;
		m_ScreenBottomRightX = m_ScreenX + m_Width * .5f;
		m_ScreenBottomRightY = m_ScreenY + m_Height * .5f;
		break;
	}
} // CButton::UpdateScreenBounds()

bool CButton::CreateVB( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					    float32 Width,						// width of the sprite, in pixels
					    float32 Height,						// height of the sprite, in pixels
					    float32 TopLeftU,					// u texture coord at top left 
					    float32 TopLeftV,					// v texture coord at top left 
					    float32 BottomRightU,				// u texture coord at bottom right 
					    float32 BottomRightV )				// v texture coord at bottom right
{
	HRESULT hr;

	// set up the vert buffer for all sprites
    hr = pD3DDevice->CreateVertexBuffer( 4*sizeof(PLAINVERTEXSCENE),
										 D3DUSAGE_WRITEONLY, 
										 D3DFVF_PLAINVERTEXSCENE,
										 D3DPOOL_MANAGED, 
										 &m_pVB );
	
	if( FAILED(hr) )
	{
		return kFalse;
	}

	float32 XOffset = 0;
	float32 YOffset = -Height;

	if( m_CenterPoint == KButtonCenter )
	{
		XOffset = Width * -.5f;
		YOffset = Height * -.5f;
	}


	PLAINVERTEXSCENE* pVertices = NULL ;
	if( FAILED( m_pVB->Lock( 0, 0, (BYTE**)&pVertices, NULL ) ) )
	{
		return kFalse;
	}

	pVertices[0].v.x	= XOffset;
	pVertices[0].v.y	= Height + YOffset;
	pVertices[0].v.z	= 0;
	pVertices[0].tu		= TopLeftU;
	pVertices[0].tv		= TopLeftV;
	pVertices[0].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );


	pVertices[1].v.x	= Width + XOffset;
	pVertices[1].v.y	= Height + YOffset;
	pVertices[1].v.z	= 0;
	pVertices[1].tu     = BottomRightU;
	pVertices[1].tv     = TopLeftV;
	pVertices[1].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	pVertices[2].v.x	= Width + XOffset;
	pVertices[2].v.y	= YOffset;
	pVertices[2].v.z	= 0;
	pVertices[2].tu     = BottomRightU;
	pVertices[2].tv     = BottomRightV;
	pVertices[2].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	pVertices[3].v.x	= XOffset;
	pVertices[3].v.y	= YOffset;
	pVertices[3].v.z	= 0;
	pVertices[3].tu     = TopLeftU;
	pVertices[3].tv     = BottomRightV;
	pVertices[3].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );


	m_pVB->Unlock();	


	return kTrue;
} // CButton::CreateVB()

void CButton::SetSize(  float32 Width,						// width of the sprite, in pixels
						float32 Height )					// height of the sprite, in pixels
{
	m_Width = Width;
	m_Height = Height;

	float32 XOffset = 0;
	float32 YOffset = -Height;

	if( m_CenterPoint == KButtonCenter )
	{
		XOffset = Width * -.5f;
		YOffset = Height * -.5f;
	}


	PLAINVERTEXSCENE* pVertices;
	if( FAILED( m_pVB->Lock( 0, 0, (BYTE**)&pVertices, NULL ) ) )
	{
		return;
	}

	pVertices[0].v.x	= XOffset;
	pVertices[0].v.y	= Height + YOffset;

	pVertices[1].v.x	= Width + XOffset;
	pVertices[1].v.y	= Height + YOffset;

	pVertices[2].v.x	= Width + XOffset;
	pVertices[2].v.y	= YOffset;

	pVertices[3].v.x	= XOffset;
	pVertices[3].v.y	= YOffset;

	m_pVB->Unlock();	

	UpdateScreenBounds();

} // CButton::SetSize()

void CButton::SetUVs( float32 TopLeftU,					// u texture coord at top left 
					  float32 TopLeftV,					// v texture coord at top left 
					  float32 BottomRightU,				// u texture coord at bottom right 
					  float32 BottomRightV )			// v texture coord at bottom right
{
	if( m_TopLeftU != TopLeftU ||
		m_TopLeftV != TopLeftV ||
		m_BottomRightU != BottomRightU ||
		m_BottomRightV != BottomRightV )
	{
		m_TopLeftU = TopLeftU;
		m_TopLeftV = TopLeftV;
		m_BottomRightU = BottomRightU;
		m_BottomRightV = BottomRightV;
		if( m_SpriteButtonState == KButtonOff )
		{
			m_SpriteButtonState = KButtonRequiresUpdate;
			UpdateUVs();
		}
	}
} // CButton::SetUVs()

void CButton::SetOverUVs( float32 OverTopLeftU,					// u texture coord at top left 
						  float32 OverTopLeftV,					// v texture coord at top left 
						  float32 OverBottomRightU,				// u texture coord at bottom right 
						  float32 OverBottomRightV )			// v texture coord at bottom right
{
	if( m_OverTopLeftU != OverTopLeftU ||
		m_OverTopLeftV != OverTopLeftV ||
		m_OverBottomRightU != OverBottomRightU ||
		m_OverBottomRightV != OverBottomRightV )
	{
		m_OverTopLeftU = OverTopLeftU;
		m_OverTopLeftV = OverTopLeftV;
		m_OverBottomRightU = OverBottomRightU;
		m_OverBottomRightV = OverBottomRightV;
		if( m_SpriteButtonState == KButtonOver )
		{
			m_SpriteButtonState = KButtonRequiresUpdate;
			UpdateUVs();
		}
	}
} // CButton::SetUVs()

// make sure that the button's UVs match the button's actual state
void CButton::UpdateUVs( void )
{
	if( m_SpriteButtonState != m_ButtonState )
	{
		m_SpriteButtonState = m_ButtonState;

		PLAINVERTEXSCENE* pVertices;
		if( FAILED( m_pVB->Lock( 0, 0, (BYTE**)&pVertices, NULL ) ) )
		{
			return;
		}

		switch( m_ButtonState )
		{
			case KButtonOff :
				pVertices[0].tu		= m_TopLeftU;
				pVertices[0].tv		= m_TopLeftV;

				pVertices[1].tu     = m_BottomRightU;
				pVertices[1].tv     = m_TopLeftV;

				pVertices[2].tu     = m_BottomRightU;
				pVertices[2].tv     = m_BottomRightV;

				pVertices[3].tu     = m_TopLeftU;
				pVertices[3].tv     = m_BottomRightV;
			break;
			case KButtonOver :
				pVertices[0].tu		= m_OverTopLeftU;
				pVertices[0].tv		= m_OverTopLeftV;

				pVertices[1].tu     = m_OverBottomRightU;
				pVertices[1].tv     = m_OverTopLeftV;

				pVertices[2].tu     = m_OverBottomRightU;
				pVertices[2].tv     = m_OverBottomRightV;

				pVertices[3].tu     = m_OverTopLeftU;
				pVertices[3].tv     = m_OverBottomRightV;
			break;
		}

		m_pVB->Unlock();	
	}
} // CButton::UpdateUVs()

void CButton::SetPosition( float32 X,					// x location onscreen (pixel coords from top left)
						   float32 Y )					// y location onscreen (pixel coords from top left)
{
	SetPosition( X, Y, m_ScreenWidth, m_ScreenHeight );
} // CButton::SetPosition()

void CButton::SetPosition( float32 X,					// x location onscreen (pixel coords from top left)
						   float32 Y,					// y location onscreen (pixel coords from top left)
						   uint32 ScreenWidth,			// screen width used in orthographic projection
						   uint32 ScreenHeight )		// screen height used in orthographic projection
{
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;

	m_X = X;
	m_Y = Y;
	m_ScreenX = X;
	m_ScreenY = Y;
	// convert from screen space to camera space
	m_X -= (float32)ScreenWidth * .5f;
	m_Y -= (float32)ScreenHeight * .5f;
	m_Y *= -1.0f;

	UpdateScreenBounds();
	UpdateTextPosition();
} // CButton::SetPosition()

// render with a specific transfomration
void CButton::Render( LPDIRECT3DDEVICE8 pD3DDevice, 		// our direct3d device
					  const D3DXMATRIX& Transformation )	// transformation for the sprite
{

	if( m_pMaterial != NULL )
	{
		pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

		if ( m_pMaterial->IsValid() )
		{
			pD3DDevice->SetTexture( 0, m_pMaterial->GetTexture() );
		}	

		pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

		pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );

		pD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,  0, 2 );
	}

	if( m_pButtonText != NULL )
	{
		if( m_SpriteButtonState == KButtonOver &&
			m_pButtonOverText != NULL )
		{
			m_pButtonOverText->Render( pD3DDevice );
		}
		else
		{
			m_pButtonText->Render( pD3DDevice );
		}
	}
} // CButton::Render()

// render with a specific transfomration, and no texture, useful for batching
void CButton::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice, 		// our direct3d device
							   const D3DXMATRIX& Transformation )	// transformation for the sprite
{

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

	pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );

	pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,  0, 2 );
} // CButton::RenderNoTexture()

// render with the default transformation
void CButton::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	D3DXMATRIX	Transformation;
	D3DXMatrixTranslation( &Transformation, m_X, m_Y, 1.0f);
	
	Render( pD3DDevice, Transformation );

} // CButton::Render()

// render without textures, if you want to batch these up yourself
// with the default transformation
void CButton::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	D3DXMATRIX	Transformation;
	D3DXMatrixTranslation( &Transformation, m_X, m_Y, 1.0f);

	RenderNoTexture( pD3DDevice, Transformation );
} // CButton::RenderNoTexture()
