/*
	FullScreenSprite.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#include "WT/stdafx.h"
#include "WT/System/Exception.h"
#include "WT/HOT/Private/FullScreenSprite.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"

namespace WT
{
	using namespace Math;

namespace HOT
{

static char g_SharedTransformMemory[sizeof( CMatrix4f )];

CFullScreenSprite::CFullScreenSprite() :
	m_Size( 40.0f, 30.0f ),
	m_Position( 320.0, 240.0f ),
	m_RotationCenter( 0.0f, 0.0f ),
	m_ZRotation( 0.0f ),
	m_TopLeftUV( 0.0f, 0.0f ),
	m_BottomRightUV( 1.0f, 1.0f ),
	m_Color( 0xffffffffu ),
	m_pVB( NULL ),
	m_pTexture( NULL )
{
	for( uint32 i = 0; i < 4; ++i )
	{
		m_Colors[i] = m_Color;
	}
	UpdateCached();
}

CFullScreenSprite::CFullScreenSprite(
#ifdef USE_DIRECTX_8
				IDirect3DDevice8* pDevice,
#else
				IDirect3DDevice9* pDevice,
#endif
				float32 X, float32 Y,
				float32 Width, float32 Height,
				float32 ZRotation,
				const CVector2f& TopLeftUV,
				const CVector2f& BottomRightUV,
				const CVector2f& RotationCenter
				) :
	m_Position (X,Y),
	m_Size (Width, Height),
	m_RotationCenter(RotationCenter),
	m_TopLeftUV(TopLeftUV),
	m_BottomRightUV(BottomRightUV),
	m_ZRotation(ZRotation),
	m_pVB( NULL ),
	m_pTexture(NULL)
{
	InitVB( pDevice );

	// Ensure all SetXxx calls are made on the data to ensure the object
	// is correctly updated
	SetPosition( X,Y );
	SetSize( Width, Height);
	SetRotationCenter(RotationCenter);
	SetTopLeftUV(TopLeftUV);
	SetBottomRightUV(BottomRightUV);
	SetZRotation(ZRotation);	
}

CFullScreenSprite::CFullScreenSprite( const CFullScreenSprite& Other ) :
	m_Size( Other.m_Size ),
	m_Position( Other.m_Position ),
	m_RotationCenter( Other.m_RotationCenter ),
	m_ZRotation( Other.m_ZRotation ),
	m_TopLeftUV( Other.m_TopLeftUV ),
	m_BottomRightUV( Other.m_BottomRightUV ),
	m_Color( Other.m_Color ),
	m_pVB( NULL )
{
	for( uint32 i = 0; i < KNumVertices; ++i )
	{
		m_Colors[i] = Other.m_Colors[i];
	}
}

CFullScreenSprite::~CFullScreenSprite()	
{
	SafeRelease ( m_pTexture );
	SafeRelease( m_pVB );
}

const CVector2f& CFullScreenSprite::GetSize() const
{
	return m_Size;
}

const CVector2f& CFullScreenSprite::GetPosition() const
{
	return m_Position;
}

const CVector2f& CFullScreenSprite::GetRotationCenter() const
{
	return m_RotationCenter;
}

float32 CFullScreenSprite::GetZRotation() const
{
	return m_ZRotation;
}

const CVector2f& CFullScreenSprite::GetTopLeftUV() const
{
	return m_TopLeftUV;
}

const CVector2f& CFullScreenSprite::GetBottomRightUV() const
{
	return m_BottomRightUV;
}

#ifdef USE_DIRECTX_8
IDirect3DTexture8* CFullScreenSprite::GetTexture() const
#else
IDirect3DTexture9* CFullScreenSprite::GetTexture() const
#endif
{
	return m_pTexture;
}

D3DCOLOR CFullScreenSprite::GetColor() const
{
	return m_Color;
}

void CFullScreenSprite::SetSize( const CVector2f& Size )
{
	m_Size = Size;
	UpdateCached();
}

void CFullScreenSprite::SetSize( float32 Width, float32 Height )
{
	m_Size.x = Width;
	m_Size.y = Height;
	UpdateCached();
}

void CFullScreenSprite::SetPosition( const CVector2f& Position )
{
	m_Position = Position;
	UpdateCachedOffset();
}

void CFullScreenSprite::SetPosition( float32 X, float32 Y )
{
	m_Position.x = X;
	m_Position.y = Y;
	UpdateCachedOffset();
}

void CFullScreenSprite::SetRotationCenter( const CVector2f& RotationCenter )
{
	m_RotationCenter = RotationCenter;
	UpdateCachedOffset();
}

void CFullScreenSprite::SetRotationCenter( float32 RotationCenterX, float32 RotationCenterY )
{
	m_RotationCenter.x = RotationCenterX;
	m_RotationCenter.y = RotationCenterY;
	UpdateCachedOffset();
}

void CFullScreenSprite::SetZRotation( float32 ZRotation )
{
	m_ZRotation = ZRotation;
	UpdateCached();
}

void CFullScreenSprite::SetTopLeftUV( const CVector2f& TopLeftUV )
{
	SetTopLeftUV( TopLeftUV.x, TopLeftUV.y );
}

void CFullScreenSprite::SetTopLeftUV( float32 TopLeftU, float32 TopLeftV )
{
	m_TopLeftUV.Set ( TopLeftU , TopLeftV );

	//Update vertex buffer
	if (m_pVB != NULL)
	{
		SPRITEVERTEX* pVertex;
#ifdef USE_DIRECTX_8
		HRESULT hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<BYTE**>( &pVertex ), 0 );
#else
		HRESULT hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<void**>( &pVertex ), 0 );
#endif
		if (FAILED(hr))
		{
			throw System::CException( "CFullScreenSprite: Failed to lock vertex buffer" );
		}

		pVertex[0].tu = m_TopLeftUV.x;
		pVertex[0].tv = m_TopLeftUV.y;
		pVertex[1].tv = m_TopLeftUV.y;
		pVertex[2].tu = m_TopLeftUV.x;

		m_pVB->Unlock();
	}
}

void CFullScreenSprite::SetBottomRightUV( const CVector2f& BottomRightUV )
{
	SetBottomRightUV( BottomRightUV.x, BottomRightUV.y );
}

void CFullScreenSprite::SetBottomRightUV( float32 BottomRightU, float32 BottomRightV )
{
	m_BottomRightUV.Set ( BottomRightU , BottomRightV );

	//Update vertex buffer
	if (m_pVB != NULL)
	{
		SPRITEVERTEX* pVertex;
#ifdef USE_DIRECTX_8
		HRESULT hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<BYTE**>( &pVertex ), 0 );
#else
		HRESULT hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<void**>( &pVertex ), 0 );
#endif
		if (FAILED(hr))
		{
			throw System::CException( "CFullScreenSprite: Failed to lock vertex buffer" );
		}

		pVertex[1].tu = m_BottomRightUV.x;
		pVertex[2].tv = m_BottomRightUV.y;
		pVertex[3].tu = m_BottomRightUV.x;
		pVertex[3].tv = m_BottomRightUV.y;

		m_pVB->Unlock();
	}
}
#ifdef USE_DIRECTX_8
void CFullScreenSprite::SetTexture( IDirect3DTexture8* pTexture )
#else
void CFullScreenSprite::SetTexture( IDirect3DTexture9* pTexture )
#endif
{
	if ( pTexture )
		pTexture->AddRef();
	SafeRelease ( m_pTexture );
	m_pTexture = pTexture;
}

void CFullScreenSprite::SetColor( D3DCOLOR Color )
{
	m_Color = Color;
	for( uint32 i = 0; i < KNumVertices; ++i )
	{
		m_Colors[i] = Color;
	}

	SetColorsInternal();
}

void CFullScreenSprite::SetColors( const D3DCOLOR TopLeft, const D3DCOLOR TopRight, const D3DCOLOR BottomLeft, const D3DCOLOR BottomRight )
{
	bool NeedSet = false;
	if( m_Colors[0] != TopLeft )
	{
		m_Colors[0] = TopLeft;
		NeedSet = true;
	}
	if( m_Colors[1] != TopRight )
	{
		m_Colors[1] = TopRight;
		NeedSet = true;
	}
	if( m_Colors[2] != BottomLeft )
	{
		m_Colors[2] = BottomLeft;
		NeedSet = true;
	}
	if( m_Colors[3] != BottomRight )
	{
		m_Colors[3] = BottomRight;
		NeedSet = true;
	}

	if( NeedSet )
	{
		SetColorsInternal();
	}
}

void CFullScreenSprite::GetColors( D3DCOLOR& TopLeft, D3DCOLOR& TopRight, D3DCOLOR& BottomLeft, D3DCOLOR& BottomRight ) const
{
	TopLeft = m_Colors[0];
	TopRight = m_Colors[1];
	BottomLeft = m_Colors[2];
	BottomRight = m_Colors[3];
}

void CFullScreenSprite::SetTopLeftColor( D3DCOLOR Color )
{
	if( Color != m_Colors[0] )
	{
		m_Colors[0] = Color;
		SetColorsInternal();
	}
}

D3DCOLOR CFullScreenSprite::GetTopLeftColor() const
{
	return m_Colors[0];
}

void CFullScreenSprite::SetTopRightColor( D3DCOLOR Color )
{
	if( Color != m_Colors[1] )
	{
		m_Colors[1] = Color;
		SetColorsInternal();
	}
}

D3DCOLOR CFullScreenSprite::GetTopRightColor() const
{
	return m_Colors[1];
}

void CFullScreenSprite::SetBottomLeftColor( D3DCOLOR Color )
{
	if( Color != m_Colors[2] )
	{
		m_Colors[2] = Color;
		SetColorsInternal();
	}
}

D3DCOLOR CFullScreenSprite::GetBottomLeftColor() const
{
	return m_Colors[2];
}

void CFullScreenSprite::SetBottomRightColor( D3DCOLOR Color )
{
	if( Color != m_Colors[3] )
	{
		m_Colors[3] = Color;
		SetColorsInternal();
	}
}

D3DCOLOR CFullScreenSprite::GetBottomRightColor() const
{
	return m_Colors[3];
}
#ifdef USE_DIRECTX_8
void CFullScreenSprite::Render( LPDIRECT3DDEVICE8 pDevice )
#else
void CFullScreenSprite::Render( LPDIRECT3DDEVICE9 pDevice )
#endif
{
	if ( m_pVB == NULL )
	{
		HRESULT hr = InitVB( pDevice );
		if ( FAILED(hr) )
		{
			return;
		}
	}

	SetMatrices( pDevice );

	pDevice->SetTexture ( 0 , m_pTexture );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

#ifdef USE_DIRECTX_8
	pDevice->SetVertexShader( D3DFVF_SPRITEVERTEX );
	pDevice->SetStreamSource( 0, m_pVB, sizeof( SPRITEVERTEX ) );
#else
	pDevice->SetFVF( D3DFVF_SPRITEVERTEX );
	pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( SPRITEVERTEX ) );
#endif

	pDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
}

#ifdef USE_DIRECTX_8
HRESULT CFullScreenSprite::InitVB( IDirect3DDevice8* pDevice )
#else
HRESULT CFullScreenSprite::InitVB( IDirect3DDevice9* pDevice )
#endif
{
	assert( m_pVB == NULL );

	//Create a new vertex buffer
	HRESULT hr = pDevice->CreateVertexBuffer( KNumVertices * sizeof( SPRITEVERTEX ),	// Length
											  0,								// Usage
											  D3DFVF_SPRITEVERTEX,				// Format
											  D3DPOOL_MANAGED,					// Pool
											  &m_pVB
#ifndef USE_DIRECTX_8
											  , NULL
#endif
											  
											  );							// Output VB
	if (FAILED(hr))
	{
		return hr;
	}

	SPRITEVERTEX* pVertex;
#ifdef USE_DIRECTX_8
	hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<BYTE**>( &pVertex ), 0 );
#else
	hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<void**>( &pVertex ), 0 );
#endif
	if (FAILED(hr))
	{
		return hr;
	}

	for (uint32 i = 0; i < KNumVertices; ++i)
	{
		pVertex[i].position.x = static_cast<float32>( i & 1 );
		pVertex[i].position.y = static_cast<float32>( i >> 1 );
		pVertex[i].position.z = 0.0f;
		pVertex[i].color = m_Colors[i];
		pVertex[i].tu = i & 1 ? m_BottomRightUV.x : m_TopLeftUV.x;
		pVertex[i].tv = i & 2 ? m_BottomRightUV.y : m_TopLeftUV.y;
	}

	m_pVB->Unlock();

	return S_OK;
}

#ifdef USE_DIRECTX_8
HRESULT CFullScreenSprite::SetMatrices( IDirect3DDevice8* pDevice )
#else
HRESULT CFullScreenSprite::SetMatrices( IDirect3DDevice9* pDevice )
#endif
{
	assert( m_pVB != NULL );
	assert( pDevice != NULL );

#ifdef USE_DIRECTX_8
	D3DVIEWPORT8 Viewport;
#else
	D3DVIEWPORT9 Viewport;
#endif
	pDevice->GetViewport( &Viewport );
	float32 InvWidth = Viewport.Width ? 2.0f / Viewport.Width : 0.0f;
	float32 InvHeight = Viewport.Height ? 2.0f / Viewport.Height : 0.0f;

	CMatrix4f& Transform = *reinterpret_cast<CMatrix4f*>( g_SharedTransformMemory );

	Transform._11 = m_Cached2x2[0][0] * InvWidth;
	Transform._12 = m_Cached2x2[0][1] * InvHeight;
	Transform._21 = m_Cached2x2[1][0] * InvWidth;
	Transform._22 = m_Cached2x2[1][1] * InvHeight;
	Transform._41 = -1.0f + (m_CachedOffset.x - Viewport.X) * InvWidth;
	Transform._42 = +1.0f - (m_CachedOffset.y - Viewport.Y) * InvHeight;
	Transform._44 = 1.0f;

	pDevice->SetTransform( D3DTS_WORLD, Transform.GetD3DMatrixPtr() );

	pDevice->SetTransform( D3DTS_VIEW, CMatrix4f::gm_Identity.GetD3DMatrixPtr() );
	pDevice->SetTransform( D3DTS_PROJECTION, CMatrix4f::gm_Identity.GetD3DMatrixPtr() );

	return S_OK;
}

HRESULT CFullScreenSprite::SetColorsInternal()
{
	// Update vertex buffer diffuse components
	if (m_pVB != NULL)
	{
		SPRITEVERTEX* pVertex;
#ifdef USE_DIRECTX_8
		HRESULT hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<BYTE**>( &pVertex ), 0 );
#else
		HRESULT hr = m_pVB->Lock( 0, KNumVertices * sizeof( SPRITEVERTEX ), reinterpret_cast<void**>( &pVertex ), 0 );
#endif
		if (FAILED(hr))
		{
			return hr;
		}

		for( uint32 i = 0; i < KNumVertices; ++i )
		{
			pVertex[i].color = m_Colors[i];
		}

		return m_pVB->Unlock();
	}
	return E_FAIL;
}

}
}