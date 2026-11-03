//
// Text.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Text.h"
#include "WT/Graphics/UI/FontUtil.h"

using namespace WT::Math;


namespace WT
{
namespace Graphics
{
namespace UI
{

WT_DEFINE_CLASS( CText )

CText::CText()
{
	InitMembers();
}

CText::CText( IDirect3DDevice9* pDevice, CFont* pFont )
{
	InitMembers();

	m_pDevice = pDevice;
	m_pFont = pFont;
}

CText::CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position )
{
	InitMembers();

	m_pDevice = pDevice;
	m_pFont = pFont;
	m_Color = Color;
	m_Position = Position;
	m_AutoSize = true;
}

CText::CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position, ETextAlignment::Alignment Alignment )
{
	InitMembers();

	m_pDevice = pDevice;
	m_pFont = pFont;
	m_Color = Color;
	m_AutoSize = true;
	m_Alignment = Alignment;
}

CText::CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position, const CVector2f& Size )
{
	InitMembers();

	m_pDevice = pDevice;
	m_pFont = pFont;
	m_Color = Color;
	m_Position = Position;
	m_Size = Size;
}

CText::CText( IDirect3DDevice9* pDevice, CFont* pFont, uint32 Color, const CVector2f& Position, const CVector2f& Size, ETextAlignment::Alignment Alignment )
{
	InitMembers();

	m_pDevice = pDevice;
	m_pFont = pFont;
	m_Color = Color;
	m_Position = Position;
	m_Size = Size;
	m_Alignment = Alignment;
}

CText::CText( const CText& Other )
{
	m_Scale = Other.m_Scale;
	m_Size = Other.m_Size;
	m_Position = Other.m_Position;
	m_Offset = Other.m_Offset;
	m_RotationZ = Other.m_RotationZ;
	m_pDevice = Other.m_pDevice;
	m_pFont = Other.m_pFont;
	m_AutoSize = Other.m_AutoSize;
	m_Color = Other.m_Color;
	m_Alignment = Other.m_Alignment;
	m_Direction = Other.m_Direction;

	m_pVB = NULL;
	m_pIB = NULL;
	m_pMaterials = NULL;
	m_CharCount = 0;
	m_TextSize = CVector2f( 0.0f, 0.0f );
}

CText::~CText()	
{
	delete[] m_pMaterials;

	SafeRelease( m_pVB );
	SafeRelease( m_pIB );
}

void CText::InitMembers()
{
	m_Scale = CVector2f( 1.0f, 1.0f );
	m_Size = CVector2f( 100.0f, 100.0f );
	m_Position = CVector2f( 320.0f, 240.0f );
	m_Offset = CVector2f( 0.0f, 0.0f );
	m_RotationZ = 0.0f;
	m_pDevice = NULL;
	m_pVB = NULL;
	m_pIB = NULL;
	m_pFont = NULL;
	m_pMaterials = NULL;
	m_Alignment = ETextAlignment::TopLeft;
	m_Direction = ETextDirection::LeftToRight;
	m_AutoSize = false;
	m_Color = 0xffffffff;
	m_CharCount = 0;
	m_TextSize = CVector2f( 0.0f, 0.0f );
}


void CText::Init( IDirect3DDevice9* pDevice, CFont* pFont )
{
	m_pDevice = pDevice;
	m_pFont = pFont;

	SafeRelease( m_pVB );
	SafeRelease( m_pIB );
}

CVector2f CText::GetSize() const
{
	return m_Size;
}

CVector2f CText::GetPosition() const
{
	return m_Position;
}

CVector2f CText::GetRotationCenter() const
{
	return m_Offset;
}

float32 CText::GetRotationZ() const
{
	return m_RotationZ;
}

void CText::SetSize( CVector2f Size )
{
	if ( !m_AutoSize && m_Size != Size )
	{
		m_Size = Size;
		LayoutText();
	}
}

void CText::SetSize( float32 Width, float32 Height )
{
	if ( !m_AutoSize && ( m_Size.x != Width || m_Size.y != Height ))
	{
		m_Size.x = Width;
		m_Size.y = Height;
		LayoutText();
	}
}

void CText::SetPosition( CVector2f Position )
{
	m_Position = Position;
}

void CText::SetPosition( float32 X, float32 Y )
{
	m_Position.x = X;
	m_Position.y = Y;
}

void CText::SetRotationCenter( CVector2f RotationCenter )
{
	m_Offset = RotationCenter;
}

void CText::SetRotationCenter( float32 RotationCenterX, float32 RotationCenterY )
{
	m_Offset.x = RotationCenterX;
	m_Offset.y = RotationCenterY;
}

void CText::SetRotationZ( float32 RotationZ )
{
	m_RotationZ = RotationZ;
}

void CText::SetText( const wstring& Value )
{
	if ( m_Text == Value )
		return;
	m_Text = Value;
	LayoutText();
}

void CText::Update( const wstring& Text, CFont* pFont, const CVector2f& Size, ETextAlignment::Alignment Alignment )
{
	if ( Text != m_Text || pFont != m_pFont || Size != m_Size || Alignment != m_Alignment )
	{
		m_Text = Text;
		m_Size = Size;
		m_pFont = pFont;
		m_Alignment = Alignment;
		LayoutText();
	}
}

// Set the text drawing direction
void CText::SetDirection( ETextDirection::Direction Direction )
{
	if ( m_Direction == Direction )
		return;
	m_Direction = Direction;
	LayoutText();
}

void CText::SetFont( CFont* pFont )
{
	if ( m_pFont == pFont )
		return;
	m_pFont = pFont;
	LayoutText();
}

void CText::SetAutoSize( bool AutoSize )
{
	if ( m_AutoSize == AutoSize )
		return;
	m_AutoSize = AutoSize;
	if ( m_AutoSize )
	{
		m_Size = m_TextSize;
	}
}

void CText::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return;
	}

	if ( m_pFont == NULL )
	{
		return;
	}

	if ( m_CharCount == 0 )
		return;

	if ( pDevice != m_pDevice )
	{
		assert(0);
		return;
	}

	if ( m_pVB == NULL )
	{
		if ( !CreateVB() )
			return;
	}

	// Save matrices
	CMatrix4f MatView( false ), MatProj( false );
	pDevice->GetTransform( D3DTS_VIEW, MatView.GetD3DMatrixPtr() );
	pDevice->GetTransform( D3DTS_PROJECTION, MatProj.GetD3DMatrixPtr() );

	SetMatrices();

	// Activate alpha blending
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | 0 );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	pDevice->SetIndices ( m_pIB );
	pDevice->SetFVF( Vertex::KFVF );
	pDevice->SetStreamSource( 0, m_pVB, 0, sizeof( Vertex ) );

	//loop over each material (different font sheet), and draw the corresponding indices
	for ( int i = 0; i < static_cast<int>(m_pFont->GetTextures().size()); i++ )
	{
		if ( m_pMaterials[i].CharCount == 0 )
			continue;

		LPDIRECT3DBASETEXTURE9 pBaseTexture = m_pFont->GetTextures()[i];
		pDevice->SetTexture( 0, pBaseTexture );

		uint16 StartOffset = m_pMaterials[i].IndexOffset;
		uint16 VertexCount = m_pMaterials[i].CharCount * 4;
		uint16 PrimCount = m_pMaterials[i].CharCount * 2;
		HRESULT hr = pDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, VertexCount, StartOffset, PrimCount );
		if ( !SUCCEEDED(hr) )
			break;
	}

	//Restore matrices
	pDevice->SetTransform( D3DTS_VIEW, MatView.GetD3DMatrixPtr() );
	pDevice->SetTransform( D3DTS_PROJECTION, MatProj.GetD3DMatrixPtr() );
}

void CText::LayoutText()
{
	uint32 LetterCount = static_cast<uint32>( m_Text.length() );

	m_Lines.clear();
	SafeRelease( m_pVB );
	SafeRelease( m_pIB );
	SafeDeleteArray( m_pMaterials );

	if ( m_pFont == NULL || m_pFont->GetTextures().empty() )
	{
		return;
	}

	uint16 SheetsCount = static_cast<uint16>( m_pFont->GetTextures().size() );
	m_pMaterials = new TextMaterial[SheetsCount];

	uint32 i=0;
	TextLine Line;
	m_TextSize.Set ( 0 , 0 );

	const uint32 LetterHeight = m_pFont->GetHeight();

	m_CharCount = 0;
	GlyphAttributes GlyphAttribs;

	while ( i <= LetterCount )
	{
		if ( i == LetterCount || m_Text[i] == L'\n' )
		{
			//first see if we can fit this line into a non-autosize text
			if ( !m_AutoSize )
			{
				if ( m_TextSize.y + LetterHeight > m_Size.y )
					break;
			}
			//find out if this line is the longest, so we could calculate the entire text width
			if ( m_TextSize.x < Line.Width )
				m_TextSize.x = static_cast<float32>(Line.Width);
			m_TextSize.y += LetterHeight;
			m_CharCount += static_cast<uint32>( Line.Text.size() );
			m_Lines.push_back ( Line );
			Line.Text.clear();
			Line.Width = 0;
		}
		else
		{
			WideLetter c = m_Text[i];

			m_pFont->GetGlyphAttributes ( c , GlyphAttribs );
			//check to see if we can fit this character
			uint32 NewLineWidth = Line.Width + GlyphAttribs.Advance + GlyphAttribs.Offset;
			if ( (!m_AutoSize && NewLineWidth <= m_Size.x) || m_AutoSize )
			{
				Line.Width = NewLineWidth;
				if ( GlyphAttribs.FontSheetIndex < 0 )
					GlyphAttribs.FontSheetIndex = 0;
				m_pMaterials[GlyphAttribs.FontSheetIndex].CharCount++;
				Line.Text += c;
			}
		}
		i++;
	}
	if ( m_AutoSize )
		m_Size = m_TextSize;

	//calculate index offset based on char counts in the material collections
	//each segment in the IB corresponds to a single material, therefore, all indices will follow
	//each other and are spaced based on the number of chars in each material * 6 (2 tris per char).
	for ( i=1; i<SheetsCount; i++ )
	{
		// Recall that m_pMaterials[0].IndexOffset is intialized to zero.
		m_pMaterials[i].IndexOffset = m_pMaterials[i-1].IndexOffset + m_pMaterials[i-1].CharCount * 6;
	}
}

/// Applies the current text color to the glyph vertices
bool CText::ApplyVertexColor()
{
	if( m_pVB == NULL )
	{
		return true;	// Color set before VB created, which is OK
	}

	Vertex* pVertex;
	HRESULT hr = m_pVB->Lock( 0, 4 * m_CharCount * sizeof(Vertex), reinterpret_cast<void**>(&pVertex), 0 );
	if ( !SUCCEEDED(hr) )
	{
		return false;
	}

	uint32 VertexPos = 0;
	uint32 const TextColor = m_Color;

	for( uint32 l=0; l < static_cast<uint32>(m_Lines.size()); ++l )
	{
		uint32 const LetterCount = static_cast<uint32>( m_Lines[l].Text.size() );
		
		for( uint32 i = 0; i < LetterCount; ++i )
		{
			pVertex[VertexPos+0].diffuse = TextColor;
			pVertex[VertexPos+1].diffuse = TextColor;
			pVertex[VertexPos+2].diffuse = TextColor;
			pVertex[VertexPos+3].diffuse = TextColor;

			VertexPos += 4;
		}
	}
	
	m_pVB->Unlock();

	return true;
}

bool CText::CreateVB()
{
	const uint32 TextColor = m_Color;

	uint32 i=0;
	TextLine Line;

	const uint32 LetterHeight = m_pFont->GetHeight();

	//Create a new vertex buffer

	HRESULT hr = m_pDevice->CreateVertexBuffer( 4 * m_CharCount * sizeof(Vertex), D3DUSAGE_WRITEONLY, 
												Vertex::KFVF, D3DPOOL_MANAGED, &m_pVB, NULL );
	if ( !SUCCEEDED(hr) )
	{
		m_pVB = NULL;
		return false;
	}

	Vertex* pVertex;
	hr = m_pVB->Lock( 0, 4 * m_CharCount * sizeof(Vertex), reinterpret_cast<void**>(&pVertex), 0 );
	if ( !SUCCEEDED(hr) )
	{
		SafeRelease(m_pVB);
		return false;
	}

	//Create a new index buffer. we'll use a single big index buffer that will hold all indices too
	//all materials. each segment in it correspond to a single material (a font sheet texture)

	hr = m_pDevice->CreateIndexBuffer( 6 * m_CharCount * sizeof(uint16), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, 
									   D3DPOOL_MANAGED, &m_pIB, NULL );
	if ( !SUCCEEDED(hr) )
	{
		m_pVB->Unlock();
		SafeRelease(m_pVB);
		m_pIB = NULL;
		return false;
	}

	uint16* pIndex = NULL;
	hr = m_pIB->Lock( 0, 6 * m_CharCount * sizeof(uint16), reinterpret_cast<void**>(&pIndex), 0 );
	if ( !SUCCEEDED(hr) )
	{
		m_pVB->Unlock();
		SafeRelease(m_pVB);
		SafeRelease(m_pIB);
		return false;
	}

	uint32 VertexPos = 0;
	float32 VertMargin = 0;

	GlyphAttributes GlyphAttribs;
	if ( m_Alignment == ETextAlignment::TopLeft || m_Alignment == ETextAlignment::TopCenter || m_Alignment == ETextAlignment::TopRight )
		VertMargin = 0;
	else if ( m_Alignment == ETextAlignment::MiddleLeft || m_Alignment == ETextAlignment::MiddleCenter || m_Alignment == ETextAlignment::MiddleRight )
		VertMargin = floor(( m_Size.y - m_TextSize.y ) / 2);
	else
		VertMargin = m_Size.y - m_TextSize.y;

	for ( uint32 l=0; l < m_Lines.size(); l++ )
	{
		TextLine& Line = m_Lines[l];
		wstring Text = Line.Text;
		uint32 LetterCount = static_cast<uint32>( Text.size() );

		if ( m_Direction == ETextDirection::RightToLeft )
			CFontUtil::ReorderString ( Text );

		float32 HorizMargin = 0;

		if ( m_Alignment == ETextAlignment::TopLeft || m_Alignment == ETextAlignment::MiddleLeft || m_Alignment == ETextAlignment::BottomLeft )
			HorizMargin = 0;
		else if ( m_Alignment == ETextAlignment::TopCenter || m_Alignment == ETextAlignment::MiddleCenter || m_Alignment == ETextAlignment::BottomCenter )
			HorizMargin = floor(( m_Size.x - Line.Width ) / 2);
		else
			HorizMargin = m_Size.x - Line.Width;

		for ( i=0; i < LetterCount; i++ )
		{
			WideLetter Letter = Text[i];
			m_pFont->GetGlyphAttributes( Letter , GlyphAttribs );
			//check if the character exists in the font, if not, just stick it into sheet 0.
			if ( GlyphAttribs.FontSheetIndex < 0 )
				GlyphAttribs.FontSheetIndex = 0;
			CRect2f Pos( HorizMargin + GlyphAttribs.Offset, VertMargin, HorizMargin + GlyphAttribs.Offset + GlyphAttribs.Width, VertMargin + LetterHeight );

			pVertex[VertexPos].SetPos( Pos.left, Pos.top, 0 );
			pVertex[VertexPos].SetTex( GlyphAttribs.Bounds.left, GlyphAttribs.Bounds.top );
			pVertex[VertexPos].diffuse = TextColor;

			pVertex[VertexPos+1].SetPos( Pos.right, Pos.top, 0 );
			pVertex[VertexPos+1].SetTex( GlyphAttribs.Bounds.right, GlyphAttribs.Bounds.top );
			pVertex[VertexPos+1].diffuse = TextColor;

			pVertex[VertexPos+2].SetPos( Pos.right, Pos.bottom, 0 );
			pVertex[VertexPos+2].SetTex( GlyphAttribs.Bounds.right, GlyphAttribs.Bounds.bottom );
			pVertex[VertexPos+2].diffuse = TextColor;

			pVertex[VertexPos+3].SetPos( Pos.left, Pos.bottom, 0 );
			pVertex[VertexPos+3].SetTex( GlyphAttribs.Bounds.left, GlyphAttribs.Bounds.bottom );
			pVertex[VertexPos+3].diffuse = TextColor;

			uint32 IndexPos = m_pMaterials[GlyphAttribs.FontSheetIndex].CurrentOffset + m_pMaterials[GlyphAttribs.FontSheetIndex].IndexOffset;

			pIndex[IndexPos  ] = static_cast<uint16>(VertexPos);
			pIndex[IndexPos+1] = static_cast<uint16>(VertexPos+1);
			pIndex[IndexPos+2] = static_cast<uint16>(VertexPos+2);
			pIndex[IndexPos+3] = static_cast<uint16>(VertexPos);
			pIndex[IndexPos+4] = static_cast<uint16>(VertexPos+2);
			pIndex[IndexPos+5] = static_cast<uint16>(VertexPos+3);

			HorizMargin += GlyphAttribs.Advance + GlyphAttribs.Offset;

			VertexPos += 4;
			m_pMaterials[GlyphAttribs.FontSheetIndex].CurrentOffset += 6;
		}
		VertMargin += LetterHeight;
	}
	m_pVB->Unlock();
	m_pIB->Unlock();

	return true;
}

void CText::SetMatrices()
{
	D3DVIEWPORT9 Viewport;
	m_pDevice->GetViewport( &Viewport );

	CMatrix4f WorldMat( false );
	WorldMat.SetScale( 2.0f / Viewport.Width , -2.0f / Viewport.Height , 1.0f );
	WorldMat.PreTranslate( -0.5f + m_Position.x - Viewport.Width * 0.5f , -0.5f + m_Position.y - Viewport.Height * 0.5f , 0 );
	WorldMat.PreScale( m_Scale.x , m_Scale.y , 1.0f );
	if (m_RotationZ != 0.0)
		WorldMat.PreRotateZ( m_RotationZ );
	WorldMat.PreTranslate( -m_Offset.x, -m_Offset.y, 0 );

	m_pDevice->SetTransform( D3DTS_WORLD, WorldMat.GetD3DMatrixPtr() );
	m_pDevice->SetTransform( D3DTS_VIEW, CMatrix4f::gm_Identity.GetD3DMatrixPtr() );
	m_pDevice->SetTransform( D3DTS_PROJECTION, CMatrix4f::gm_Identity.GetD3DMatrixPtr() );
}


} // namespace UI
} // namespace Graphics
} // namespace WT
