//
// FullScreenSprite.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/Math/Math.h"
#include "WT/Math/LinearAlgebra/Vector2f.h"

namespace WT
{
using namespace Math;

namespace HOT
{

class CFullScreenSprite
{
public:

	CFullScreenSprite();
	CFullScreenSprite(
#ifdef USE_DIRECTX_8
			IDirect3DDevice8* pDevice,
#else
			IDirect3DDevice9* pDevice,
#endif
			float32 X , float32 Y,				// position of the sprite from top left corner (in screen pixel space)
	        float32 Width , float32 Height,		// size of the sprite in pixels
			float32 ZRotation = 0,								// Z rotation of the sprite
			const CVector2f& TopLeftUV = CVector2f(0.0f,0.0f),	// top left UV coordinates
			const CVector2f& BottomRightUV = CVector2f(1.0f,1.0f),// bottom right UV coordinates
			const CVector2f& RotationCenter = CVector2f(0,0)		// rotation center of the sprite
			);

	CFullScreenSprite( const CFullScreenSprite& Other );
	virtual ~CFullScreenSprite();

	/// Set the size of the sprite in pixels.
	void SetSize( const CVector2f& Size );

	/// Set the size of the sprite in pixels.
	void SetSize( float32 Width, float32 Height );

	/// Get the size of the sprite in pixels.
	const CVector2f& GetSize() const;

	/// Set the position of the sprite in pixels.
	void SetPosition( const CVector2f& Position );

	/// Set the position of the sprite in pixels.
	void SetPosition( float32 X, float32 Y );

	/// Get the position of the sprite in pixels.
	const CVector2f& GetPosition() const;

	/// Set the offset of the sprite in pixels.
	void SetRotationCenter( const CVector2f& RotationCenter );

	/// Set the offset of the sprite in pixels.
	void SetRotationCenter( float32 RotationCenterX, float32 RotationCenterY );

	/// Get the offset of the sprite in pixels.
	const CVector2f& GetRotationCenter() const;

	/// Set the rotation about the screen Z-axis in radians. Note that the sprite rotates about
	/// its center, not about its origin. Positive angles yield rotations that appear
	/// counterclockwise to the user.
	void SetZRotation( float32 ZRotation );

	/// Get the rotation about the screen Z-axis in radians. Note that the sprite rotates
	/// about its center, not about its origin. Positive angles yield rotations that appear
	/// counterclockwise to the user.
	float32 GetZRotation() const;

	/// Get the texture coordinates used for the upper left corner (of an unrotated sprite).
	const CVector2f& GetTopLeftUV() const;

	/// Get the texture coordinates used for the bottom right corner (of an unrotated sprite).
	const CVector2f& GetBottomRightUV() const;

#ifdef USE_DIRECTX_8
	void SetTexture( IDirect3DTexture8* pTexture );
	IDirect3DTexture8* GetTexture() const;
#else
	void SetTexture( IDirect3DTexture9* pTexture );
	IDirect3DTexture9* GetTexture() const;
#endif

	/// Set the texture coordinates used for the upper left corner (of an unrotated sprite).
	void SetTopLeftUV( const CVector2f& TopLeftUV );

	/// Set the texture coordinates used for the upper left corner (of an unrotated sprite).
	void SetTopLeftUV( float32 TopLeftU, float32 TopLeftV );

	/// Set the texture coordinates used for the bottom right corner (of an unrotated sprite).
	void SetBottomRightUV( const CVector2f& BottomRightUV );

	/// Set the texture coordinates used for the bottom right corner (of an unrotated sprite).
	void SetBottomRightUV( float32 BottomRightU, float32 BottomRightV );

	void SetColor( D3DCOLOR Color );

	D3DCOLOR GetColor() const;

	void SetColors( const D3DCOLOR TopLeft, const D3DCOLOR TopRight, const D3DCOLOR BottomLeft, const D3DCOLOR BottomRight );

	void GetColors( D3DCOLOR& TopLeft, D3DCOLOR& TopRight, D3DCOLOR& BottomLeft, D3DCOLOR& BottomRight ) const;
	
	void SetTopLeftColor( D3DCOLOR Color );

	D3DCOLOR GetTopLeftColor() const;

	void SetTopRightColor( D3DCOLOR Color );

	D3DCOLOR GetTopRightColor() const;

	void SetBottomLeftColor( D3DCOLOR Color );

	D3DCOLOR GetBottomLeftColor() const;

	void SetBottomRightColor( D3DCOLOR Color );

	D3DCOLOR GetBottomRightColor() const;

	/// Render the sprite using the specified camera.

#ifdef USE_DIRECTX_8
	virtual void Render( LPDIRECT3DDEVICE8 pDevice );
#else
	virtual void Render( LPDIRECT3DDEVICE9 pDevice );
#endif

protected:

	static int32 const KNumVertices = 4;

#ifdef USE_DIRECTX_8
	HRESULT InitVB( IDirect3DDevice8* pDevice );
	HRESULT SetMatrices( IDirect3DDevice8* pDevice );
#else
	HRESULT InitVB( IDirect3DDevice9* pDevice );
	HRESULT SetMatrices( IDirect3DDevice9* pDevice );
#endif

	CVector2f					m_Size; //Size in vertical screen units
	CVector2f					m_Position; //Position in screen coordinates
	CVector2f					m_RotationCenter; //Offset of sprite
	float32						m_ZRotation; //Rotation about screen Z-axis
	CVector2f					m_TopLeftUV; //Starting texture coordinates
	CVector2f					m_BottomRightUV; //Ending texture coordinates
	D3DCOLOR					m_Color; //Color
	D3DCOLOR					m_Colors[KNumVertices]; //TopLeft, TopRight, BottomLeft, BottomRight

	struct SPRITEVERTEX
	{
		CVector3f position; // The position
		D3DCOLOR color;    // The color
		float32 tu, tv;    // The texture coordinates
	};

	static const DWORD D3DFVF_SPRITEVERTEX = (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);

#ifdef USE_DIRECTX_8
	IDirect3DVertexBuffer8* 	m_pVB;
	IDirect3DTexture8* m_pTexture;
#else
	IDirect3DVertexBuffer9* 	m_pVB;
	IDirect3DTexture9* m_pTexture;
#endif

	CVector2f m_CachedOffset;

	//Cached 2x2 matrix of parameters that help compute the overall upper left 2x2 matrix of the
	//model-to-world transform. These depend only on the Size and ZRotation of the sprite.
	float32 m_Cached2x2[2][2];

	void UpdateCachedOffset()
	{
		//An additional -0.5 is added to ensure accurate texturing - this is necessary
		//as pixels are centered at (0.5,0.5).
		m_CachedOffset.x = m_Position.x + m_RotationCenter.x - 0.5f + 0.5f * (m_Size.x - m_Cached2x2[0][0] - m_Cached2x2[1][0]);
		m_CachedOffset.y = m_Position.y + m_RotationCenter.y - 0.5f + 0.5f * (m_Size.y + m_Cached2x2[0][1] + m_Cached2x2[1][1]);
	}

	void UpdateCached()
	{
		if (m_ZRotation == 0.0f)
		{
			m_Cached2x2[0][0] = m_Size.x;
			m_Cached2x2[0][1] = 0.0f;
			m_Cached2x2[1][0] = 0.0f;
			m_Cached2x2[1][1] = -m_Size.y;
		}
		else
		{
			float32 c = cosf( m_ZRotation );
			float32 s = sinf( m_ZRotation );
			m_Cached2x2[0][0] = c * m_Size.x;
			m_Cached2x2[0][1] = s * m_Size.x;
			m_Cached2x2[1][0] = s * m_Size.y;
			m_Cached2x2[1][1] = -c * m_Size.y;
		}

		UpdateCachedOffset();
	}

	HRESULT SetColorsInternal();
};

}
}
