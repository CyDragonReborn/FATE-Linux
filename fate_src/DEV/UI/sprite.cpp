/*
	sprite.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
	CHANGE HISTORY

	Travis Baldree
	7/13/2004		-	Added Z-Rotation parameter to sprite
*/
#include <D3D8.h>
#include <d3dx8.h>

#include <string>
#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/math3d.h"
#include "../MEDIA/material.h"
#include "sprite.h"

CSprite::CSprite( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
				  CMaterial* pMaterial,				// material of the sprite texture we want to use
				  float32 X,						// x location onscreen (pixel coords from top left)
				  float32 Y,						// y location onscreen (pixel coords from top left)
				  float32 Width,					// width of the sprite, in pixels
				  float32 Height,					// height of the sprite, in pixels
				  float32 TopLeftU,					// u texture coord at top left 
				  float32 TopLeftV,					// v texture coord at top left 
				  float32 BottomRightU,				// u texture coord at bottom right 
				  float32 BottomRightV,				// v texture coord at bottom right
				  ESpriteCenter CenterPoint,		// centerpoint of the sprite
				  uint32 ScreenWidth,				// screen width used in orthographic projection
				  uint32 ScreenHeight ) :			// screen height used in orthographic projection
											m_pVB( NULL ),
											m_pMaterial( pMaterial ),
											m_X( X ),
											m_Y( Y ),
											m_ScreenX( X ),
											m_ScreenY( Y ),
											m_ScreenWidth( ScreenWidth ),
											m_ScreenHeight( ScreenHeight ),
											m_ZRotation( 0 ),
											m_CenterPoint( CenterPoint ),
											m_Width( 0 ),
											m_Height( 0 )
{
	ZeroMemory (&m_Transformation, sizeof( m_Transformation ));

	CreateVB( pD3DDevice,
			  Width,
			  Height,
			  TopLeftU,
			  TopLeftV,
			  BottomRightU,
			  BottomRightV );

	SetPosition( X, Y, ScreenWidth, ScreenHeight );

} // CSprite::CSprite()


CSprite::~CSprite( void )
{
	m_pMaterial = NULL;
	RELEASE_SAFELY( m_pVB );
}

bool CSprite::CreateVB( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
					    float32 Width,						// width of the sprite, in pixels
					    float32 Height,						// height of the sprite, in pixels
					    float32 TopLeftU,					// u texture coord at top left 
					    float32 TopLeftV,					// v texture coord at top left 
					    float32 BottomRightU,				// u texture coord at bottom right 
					    float32 BottomRightV )				// v texture coord at bottom right
{
	m_Width = Width;
	m_Height = Height;

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

	if( m_CenterPoint == KSpriteCenter )
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
} // CSprite::CreateVB()

void CSprite::SetSize(  float32 Width,						// width of the sprite, in pixels
						float32 Height )					// height of the sprite, in pixels
{
	if( Width == m_Width &&
		Height == m_Height )
	{
		return;
	}
	m_Width = Width;
	m_Height = Height;
	float32 XOffset = 0;
	float32 YOffset = -Height;

	if( m_CenterPoint == KSpriteCenter )
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
} // CSprite::SetSize()

void CSprite::SetUVs( float32 TopLeftU,					// u texture coord at top left 
					  float32 TopLeftV,					// v texture coord at top left 
					  float32 BottomRightU,				// u texture coord at bottom right 
					  float32 BottomRightV )			// v texture coord at bottom right
{
	PLAINVERTEXSCENE* pVertices;
	if( FAILED( m_pVB->Lock( 0, 0, (BYTE**)&pVertices, NULL ) ) )
	{
		return;
	}

	pVertices[0].tu		= TopLeftU;
	pVertices[0].tv		= TopLeftV;

	pVertices[1].tu     = BottomRightU;
	pVertices[1].tv     = TopLeftV;

	pVertices[2].tu     = BottomRightU;
	pVertices[2].tv     = BottomRightV;

	pVertices[3].tu     = TopLeftU;
	pVertices[3].tv     = BottomRightV;


	m_pVB->Unlock();	
} // CSprite::SetUVs()

void CSprite::SetPosition( float32 X,					// x location onscreen (pixel coords from top left)
						   float32 Y )					// y location onscreen (pixel coords from top left)
{
	SetPosition( X, Y, m_ScreenWidth, m_ScreenHeight );
} // CSprite::SetPosition()

void CSprite::SetPosition( float32 X,					// x location onscreen (pixel coords from top left)
						   float32 Y,					// y location onscreen (pixel coords from top left)
						   uint32 ScreenWidth,			// screen width used in orthographic projection
						   uint32 ScreenHeight )		// screen height used in orthographic projection
{
	m_ScreenWidth = ScreenWidth;
	m_ScreenHeight = ScreenHeight;
	m_ScreenX = X;
	m_ScreenY = Y;
	m_X = X;
	m_Y = Y;
	// convert from camera space to screen space
	m_X -= (float32)ScreenWidth * .5f;
	m_Y -= (float32)ScreenHeight * .5f;
	m_Y *= -1.0f;

	D3DXMatrixTranslation( &m_Transformation, m_X, m_Y, 1.0f);
} // CSprite::SetPosition()

void CSprite::Render( LPDIRECT3DDEVICE8 pD3DDevice, 		// our direct3d device
					  float32 X,							// screen x
					  float32 Y,							// screen y position
					  uint32 ScreenWidth,			// screen width used in orthographic projection
					  uint32 ScreenHeight )		// screen height used in orthographic projection
{

	// convert from camera space to screen space
	X -= (float32)ScreenWidth * .5f;
	Y -= (float32)ScreenHeight * .5f;
	Y *= -1.0f;

	D3DXMATRIX	Transformation;
	D3DXMatrixTranslation( &Transformation, X, Y, 1.0f);

	Render( pD3DDevice,
			Transformation );
} // CSprite::Render()

void CSprite::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice, 		// our direct3d device
							   float32 X,							// screen x
							   float32 Y,							// screen y position
							   uint32 ScreenWidth,					// screen width used in orthographic projection
							   uint32 ScreenHeight )				// screen height used in orthographic projection
{

	// convert from camera space to screen space
	X -= (float32)ScreenWidth * .5f;
	Y -= (float32)ScreenHeight * .5f;
	Y *= -1.0f;

	D3DXMATRIX	Transformation;
	D3DXMatrixTranslation( &Transformation, X, Y, 1.0f);

	RenderNoTexture( pD3DDevice,
					 Transformation );
} // CSprite::RenderNoTexture()

// render with a specific transfomration
void CSprite::Render( LPDIRECT3DDEVICE8 pD3DDevice, 		// our direct3d device
					  const D3DXMATRIX& Transformation )	// transformation for the sprite
{

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

	if ( m_pMaterial->IsValid() )
	{
		pD3DDevice->SetTexture( 0, m_pMaterial->GetTexture() );
	}	

	pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

	pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );

	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,  0, 2 );
} // CSprite::Render()

// render with a specific transfomration, and no texture, useful for batching
void CSprite::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice, 		// our direct3d device
							   const D3DXMATRIX& Transformation )	// transformation for the sprite
{

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

	pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );

	pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,  0, 2 );
} // CSprite::RenderNoTexture()

// render with the default transformation
void CSprite::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	D3DXMATRIX	Transformation( m_Transformation );

	if( m_ZRotation != 0 )
	{
		D3DXMATRIX	Rotation;
		D3DXMatrixRotationZ( &Rotation, (float32) KDeg2Rad * m_ZRotation );
		Transformation = Rotation * m_Transformation;
	}
	
	Render( pD3DDevice, Transformation );

} // CSprite::Render()

// render without textures, if you want to batch these up yourself
// with the default transformation
void CSprite::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	D3DXMATRIX	Transformation( m_Transformation );
	//D3DXMatrixTranslation( &Transformation, m_X, m_Y, 1.0f);

	if( m_ZRotation != 0 )
	{
		D3DXMATRIX	Rotation;
		D3DXMatrixRotationZ( &Rotation, (float32) KDeg2Rad * m_ZRotation );
		Transformation = Rotation * m_Transformation;
	}

	RenderNoTexture( pD3DDevice, Transformation );
} // CSprite::RenderNoTexture()
