//
// Text.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include <vector>
#include "d3d9.h"
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/Math/Math.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/UI/Font.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace WT::Math;


// Enums
namespace ETextAlignment
{
	enum Alignment
	{
		TopLeft,		// Text is aligned to the left (default) and the top
		TopCenter,		// Text is top centered
		TopRight,		// Text is aligned to the right at the top
		MiddleLeft,		// Text is aligned to the left and at the middle vertically
		MiddleCenter,	// Text is centered in all directions
		MiddleRight,	// Text is aligned to the right and at the middle vertically
		BottomLeft,		// Text is aligned to the left and at the bottom
		BottomCenter,	// Text is aligned in the center at the bottom
		BottomRight		// Text is aligned to the bottom right
	};
}

namespace ETextDirection
{
	enum Direction
	{
		LeftToRight,	// Text is drawn from left to right (default)
		RightToLeft		// Text is drawn right to left (for RTL languages like Hebrew and Arabic)
	};
}

// Structs
struct TextMaterial
{
	TextMaterial() : IndexOffset(0) , CharCount(0) , CurrentOffset(0) {}
	uint16 IndexOffset;		//the offset in the index buffer where this material starts
	uint16 CharCount;		//how many characters are using this material
	uint32 CurrentOffset;	//for filling in the buffer only (will start at 0, and go up to CharCount-1)
};

struct TextLine
{
	TextLine () : Width(0) {}
	wstring Text;
	uint32 Width;
};


class CText : public System::CObject, public CRenderableImpl
{
public:
	WT_BEGIN_CLASS(CText, System::CObject);
		WT_INHERITS_INTERFACE( IRenderable );
	WT_END_CLASS();

	CText();
	CText( IDirect3DDevice9* pDevice, CFont* pFont );
	CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position );
	CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position, ETextAlignment::Alignment Alignment );
	CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position, const CVector2f& Size );
	CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position, const CVector2f& Size, ETextAlignment::Alignment Alignment );
	CText( const CText& Other );
	virtual ~CText();


	// Initialize the Text object
	virtual void Init( IDirect3DDevice9* pDevice, CFont* pFont );

	// Get the size of the text in pixels.
	virtual CVector2f GetSize() const;

	// Get the position of the text in pixels.
	virtual CVector2f GetPosition() const;

	// Get the offset of the center of the text in pixels.
	virtual CVector2f GetRotationCenter() const;

	// Get the rotation about the screen Z-axis in radians. Note that the text rotates
	// about its center, not about its origin. Positive angles yield rotations that appear
	// counterclockwise to the user.
	virtual float32 GetRotationZ() const;

	// Set the size of the text in pixels.
	virtual void SetSize( CVector2f Size );

	// Set the height of the text in pixels.
	virtual void SetSize( float32 Width, float32 Height );

	// Set the position of the text in pixels.
	virtual void SetPosition( CVector2f Position );

	// Set the position of the text in pixels.
	virtual void SetPosition( float32 X, float32 Y );

	// Set the offset of the center of the text in pixels.
	virtual void SetRotationCenter( CVector2f RotationCenter );

	// Set the offset of the center of the text in pixels.
	virtual void SetRotationCenter( float32 RotationCenterX, float32 RotationCenterY );

	// Set the rotation about the screen Z-axis in radians. Note that the text rotates about
	// its center, not about its origin. Positive angles yield rotations that appear
	// counterclockwise to the user.
	virtual void SetRotationZ( float32 RotationZ );

	// Render the text using the specified context
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	// Set the text value this object represets
	void SetText( const wstring& Value );

	// Get the text value for this object
	wstring GetText() const
	{
		return m_Text;
	}

	// Set the font this text will be rendered in
	virtual void SetFont( CFont* pFont );

	// Get the font this text is rendered in
	virtual CFont* GetFont () const
	{
		return m_pFont;
	}

	// Sets the vertex color for the text object
	// The default text color is white (0xffffffff)
	virtual void SetColor( uint32 Color )
	{
		// Only apply the color if the new value differs from the current one
		if( m_Color != Color )
		{
			m_Color = Color;
			ApplyVertexColor();
		}
	}

	// Retrieves the current vertex color for the text object
	virtual uint32 GetColor( ) const
	{
		return m_Color;
	}

	// Get the alignment of the text
	virtual ETextAlignment::Alignment GetAlignment() const
	{
		return m_Alignment;
	}

	// Set the alignment of the text
	virtual void SetAlignment( ETextAlignment::Alignment Alignment )
	{
		if ( m_Alignment != Alignment )
		{
			m_Alignment = Alignment;
			LayoutText();
		}
	}

	// Set the scale of the text
	virtual void SetScale( float32 ScaleX, float32 ScaleY )
	{
		m_Scale.x = ScaleX;
		m_Scale.y = ScaleY;
	}

	// Set the scale of the text
	virtual void SetScale( CVector2f Scale )
	{
		m_Scale = Scale;
	}

	// Get the scale of the text
	virtual CVector2f GetScale() const
	{
		return m_Scale;
	}

	// Get the text drawing direction
	virtual ETextDirection::Direction GetDirection() const
	{
		return m_Direction;
	}

	// Set the text drawing direction
	virtual void SetDirection( ETextDirection::Direction Direction );

	// Is this text auto-sizeing to fit its contents
	virtual bool GetAutoSize() const
	{
		return m_AutoSize;
	}

	// Set whether this text is auto-sizing to fit its contents.
	virtual void SetAutoSize( bool AutoSize );

	virtual void Update( const wstring& Text, CFont* pFont, const CVector2f& Size, ETextAlignment::Alignment Alignment );


private: // methods

	bool CreateVB();
	void SetMatrices();
	void LayoutText();
	// Applies the current text color to the glyph vertices
	bool ApplyVertexColor();
	void InitMembers();


private: // data

	CVector2f					m_Size;			//Size in vertical screen units
	CVector2f					m_Position;		//Position in screen coordinates
	CVector2f					m_Offset;		//Offset to center of text
	float32						m_RotationZ;	//Rotation about screen Z-axis
	CVector2f					m_Scale;		//scale factor to be applied to the text.
	uint32						m_Color;

	// Vertex buffer cached based on device pointer
	LPDIRECT3DDEVICE9			m_pDevice;
	LPDIRECT3DVERTEXBUFFER9		m_pVB;
	LPDIRECT3DINDEXBUFFER9		m_pIB;
	wstring						m_Text;
	CFont*						m_pFont;
	uint32						m_CharCount;	//real letter count (without line breaks)
	TextMaterial*				m_pMaterials;	//materials used
	ETextAlignment::Alignment	m_Alignment;
	ETextDirection::Direction	m_Direction;	//direction of the text (LTR, RTL, etc.)
	std::vector<TextLine>		m_Lines;
	CVector2f					m_TextSize;
	bool						m_AutoSize : 1;

	// Typedefs
	struct Vertex
	{
		float32		x, y, z;		// Position
		uint32		diffuse;		// Diffuse color
		float32		u, v;			// Texture coordinates

		void SetPos(float32 fx, float32 fy, float32 fz) { x=fx; y=fy; z=fz; }
		void SetTex(float32 fu, float32 fv) { u=fu; v=fv; }

		static const uint32 KFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	};
};


} // UI
} // Graphics
} // WT
