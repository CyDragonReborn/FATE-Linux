//
// TextureFont.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/TextureFont.h"
#include "WT/System/IO/File.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace System::IO;

WT_DEFINE_CLASS( CTextureFont )

CTextureFont::CTextureFont() {}
CTextureFont::CTextureFont( const CTextureFont& Other ) {}	// TODO : Do we need a copy constructor for a TextureFont object?
CTextureFont::~CTextureFont() {}


// Adds a new texture sheet from the files referenced by the given texture and glyph database paths
bool CTextureFont::AddSheet ( LPDIRECT3DBASETEXTURE9 pTexture, const string& DatabasePath )
{
	HStream hFileStream = CFile::Open(DatabasePath, FileMode::Open,
		FileAccess::Read, FileShare::Read );
	if (hFileStream == NULL)
	{
		return false;
	}

	return AddSheet( pTexture, hFileStream );
}

bool CTextureFont::AddSheet ( LPDIRECT3DBASETEXTURE9 pTexture, CStream* pDatabaseStream )
{
	if( pTexture == NULL || pDatabaseStream == NULL )
	{
		return false;
	}

	pTexture->AddRef();
	m_Sheets.push_back( pTexture );
	uint32 const FontSheetIndex = static_cast<uint32>( m_Sheets.size() - 1 );

	try
	{
		uint32 Version;
		int32 BytesRead = pDatabaseStream->Read( reinterpret_cast<uint8*>(&Version), sizeof(uint32) );
		if( BytesRead != static_cast<int32>(sizeof(uint32)) )
		{
			goto LFail;
		}
		pDatabaseStream->Seek( -BytesRead, SeekOrigin::Current );

		switch( Version )
		{
		default:
			goto LFail;
			break;

		case 0x00000005:
			if ( !LoadFontMakerFormat5( pDatabaseStream, FontSheetIndex ) )
			{
				goto LFail;
			}
			break;
		}
	}
	catch(...)
	{
		goto LFail;
	}

	return true;

LFail:

	m_Sheets.pop_back();
	SafeRelease(pTexture);
	return false;
}


bool CTextureFont::LoadFontMakerFormat5( CStream* pDatabaseStream, uint32 FontSheetIndex )
{
	if ( !pDatabaseStream )
		return false;

	struct FontMakerGlyphAttr
	{
		Math::CRect2f Bounds;
		int16 Offset;
		int16 Width;
		int16 Advance;
	};

	uint32 Version;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&Version), sizeof(uint32) ) )
	{
		return false;
	}
	if( Version != 0x00000005 )
	{
		return false;
	}

	float32 fFontHeight;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&fFontHeight), sizeof(fFontHeight) ) )
	{
		return false;
	}
	m_FontHeight = Float32ToInt32( fFontHeight );

	float32 TopPadding = 0.0f;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&TopPadding), sizeof(TopPadding) ) )
	{
		return false;
	}
	float32 BottomPadding = 0.0f;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&BottomPadding), sizeof(BottomPadding) ) )
	{
		return false;
	}
	float32 YAdvance = 0.0f;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&YAdvance), sizeof(YAdvance) ) )
	{
		return false;
	}

	uint16 IndexCount = 0;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&IndexCount), sizeof(IndexCount) ) )
	{
		return false;
	}

	std::map<uint16, uint16> CharMapping;

	WideLetter LowChar  = 0xffff;
	WideLetter HighChar = 0x0000;
	for( uint16 i = 0; i <= IndexCount; ++i )
	{
		uint16 Index;
		if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&Index), sizeof(Index) ) )
		{
			return false;
		}
		if( Index != 0 )
		{
			CharMapping[Index] = i;
			if( i < LowChar ) LowChar = i;
			if( i > HighChar ) HighChar = i;
		}
	}

	uint32 GlyphCount;
	if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&GlyphCount), sizeof(GlyphCount) ) )
	{
		return false;
	}

	for( uint32 c = 0; c < GlyphCount; ++c )
	{
		FontMakerGlyphAttr fmga;
		if ( 0 == pDatabaseStream->Read( reinterpret_cast<uint8*>(&fmga), sizeof(fmga) ) )
		{
			return false;
		}

		GlyphAttributes GlyphAttributes;
		
		GlyphAttributes.Bounds			= fmga.Bounds;
		GlyphAttributes.Offset			= fmga.Offset;
		GlyphAttributes.Width			= fmga.Width;
		GlyphAttributes.Advance			= fmga.Advance;

		GlyphAttributes.TopPadding		= TopPadding;
		GlyphAttributes.BottomPadding	= BottomPadding;
		GlyphAttributes.YAdvance		= YAdvance;

		GlyphAttributes.FontSheetIndex = FontSheetIndex;

		SetGlyphAttributes( CharMapping[static_cast<uint16>(c)], GlyphAttributes );
	}

	return true;
}


} // namespace UI
} // namespace Graphics
} // WT
