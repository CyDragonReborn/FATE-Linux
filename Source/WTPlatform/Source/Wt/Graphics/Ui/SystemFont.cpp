//
// SystemFont.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/SystemFont.h"

using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{
namespace UI
{

WT_DEFINE_CLASS( CSystemFont )


CSystemFont::CSystemFont( )
	: m_pDevice( NULL )
	, m_pCurrentTexture( NULL )
	, m_hFont( NULL )
	, m_hDC( NULL )
	, m_Initialized( false )
{	
}

CSystemFont::CSystemFont( const CSystemFont& Other )
	: m_pDevice( NULL )
	, m_pCurrentTexture( NULL )
	, m_hFont( NULL )
	, m_hDC( NULL )
	, m_Initialized( false )
{
	// Clone all properties
	Initialize ( Other.m_Face, Other.m_FontHeight, Other.m_CharacterSet, 
		Other.m_TextureWidth, Other.m_TextureHeight, Other.m_Weight, 
		Other.m_Italic, Other.m_Underline, Other.m_Strikeout,
		Other.m_BorderEffect, Other.m_ShadowEffect );
}

CSystemFont::~CSystemFont( )
{
	if ( m_hFont != NULL )
	{
		::DeleteObject( m_hFont );
	}

	SafeDelete( m_pCurrentTexture );
}


//
// Public methods
//

// Adds all characters within the provided range to the font
bool CSystemFont::AddChars ( WideLetter StartCharacter, WideLetter EndCharacter )
{
	if( m_pDevice == NULL )
	{
		return false;
	}

	std::vector<WideLetter> CharacterStream;
	if( BuildCharacterStream( StartCharacter, EndCharacter, CharacterStream ) )
	{
		return AddChars ( m_pDevice, CharacterStream );		
	}

	return false;
}

// Adds all unique characters within the provided string to the font
bool CSystemFont::AddChars ( const wstring& Chars )
{
	if( m_pDevice == NULL )
	{
		return false;
	}

	std::vector<WideLetter> CharacterStream;
	if( BuildCharacterStream( Chars, CharacterStream ) )
	{
		return AddChars ( m_pDevice, CharacterStream );		
	}

	return false;
}

// Adds all unique characters within the provided stream to the font
bool CSystemFont::AddChars ( CStream* pGlyphStream )
{
	if( m_pDevice == NULL )
	{
		return false;
	}

	std::vector<WideLetter> CharacterStream;
	if( BuildCharacterStream( pGlyphStream, CharacterStream ) )
	{
		return AddChars ( m_pDevice, CharacterStream );
	}

	return false;
}


//
// Private methods
//

// Fills the sent vector stream with the given range of characters, including the NULL character
bool CSystemFont::BuildCharacterStream( WideLetter Start, WideLetter End, std::vector<WideLetter>& CharacterStream ) const
{
	int32 GlyphCount= (End - Start) + 1;
	if( GlyphCount < 0 ) GlyphCount = 0;

    CharacterStream.reserve( GlyphCount );

	// Ensure the character stream contains the NULL ('\0') character
	if ( Start > L'\0' && !GlyphAttributesExist( L'\0' ) )
	{
		CharacterStream.push_back( L'\0' );
	}

	for ( WideLetter c = Start; c <= End; ++c )
	{
		if ( !GlyphAttributesExist( c ) )
		{
			CharacterStream.push_back( c );
		}
	}

	return true;
}

// Fills the sent vector stream with all unique characters found in the given string, including the NULL character
bool CSystemFont::BuildCharacterStream( const wstring& Chars, std::vector<WideLetter>& CharacterStream ) const
{
    CharacterStream.reserve( Chars.length( ) );

	bool ContainsNull = GlyphAttributesExist( L'\0' );	// To ensure the character stream contains the NULL ('\0') character

	for( uint32 i = 0; i < Chars.length( ); ++i )
	{
		const WideLetter c = Chars.at( i );

		if( c == L'\0' )
		{
			ContainsNull = true;
		}

		if( !GlyphAttributesExist( c ) )
		{
			CharacterStream.push_back( c );
		}
	}

	// Ensure the character stream contains the NULL ('\0') character
	if( !ContainsNull )
	{
		CharacterStream.push_back( L'\0' );
	}

	return true;
}

// Fills the sent vector stream with all unique characters found in the given stream, including the NULL character
bool CSystemFont::BuildCharacterStream( CStream* pGlyphStream, std::vector<WideLetter>& CharacterStream ) const
{
	if( pGlyphStream == NULL )
	{
		return false;
	}

	const uint32 BytesAvailable = static_cast<uint32>( pGlyphStream->GetLength() );
	const uint32 SizeOfWideLetter = static_cast<uint32>( sizeof(WideLetter) );

	// Reject the stream if it does not contain an even number of wide characters
	if( (BytesAvailable % SizeOfWideLetter) != 0 )
	{
		return false;
	}

	const uint32 GlyphCount= BytesAvailable / SizeOfWideLetter;
	
	CharacterStream.reserve( GlyphCount );

	bool ContainsNull = GlyphAttributesExist( L'\0' );	// To ensure the character stream contains the NULL ('\0') character

    for( uint32 i = 0; i < GlyphCount; ++i )
	{
		WideLetter c;

		uint32 BytesRead = pGlyphStream->Read( reinterpret_cast<uint8*>(&c), SizeOfWideLetter );
		if( BytesRead != SizeOfWideLetter )
		{
			return false;
		}

		if( c == L'\0' )
		{
			ContainsNull = true;
		}

		// TODO : Review - Look at using std::set to exclude duplicate chars in CharacterStream
		if( !GlyphAttributesExist( c ) &&						// we don't already have this character
			!StreamContainsCharacter( CharacterStream, c ) )	// character is not redundant in the current set
		{
			CharacterStream.push_back( c );
		}
	}

	// Ensure the character stream contains the NULL ('\0') character
	if( !ContainsNull )
	{
		CharacterStream.push_back( L'\0' );
	}

	return true;
}

// Determines whether the given vector stream contains the given character
/*static*/
bool CSystemFont::StreamContainsCharacter( const std::vector<WideLetter>& CharacterStream, WideLetter c )
{
	std::vector<WideLetter>::const_iterator it = CharacterStream.begin( );
	while( it != CharacterStream.end( ) )
	{
		if( *it == c )
		{
			return true;
		}

		++it;
	}

	return false;
}

// Finalizes updates to the current working texture and adds the next one to the fontsheet collection
bool CSystemFont::AddNextTexture( LPDIRECT3DDEVICE9 pDevice )
{
	if( pDevice == NULL )
	{
		return false;
	}

	// Unlock the previous texture (if this isn't the first one) and apply its alpha channel
	if( m_pCurrentTexture )
	{
		// Release DC, add alpha channel, add font texture to collection.
		FinishFontTexture( m_pCurrentTexture );

		// Invalidate the current working texture
		SafeDelete( m_pCurrentTexture );
	}

	// Create the next texture and add it to the sheet collection
	m_pCurrentTexture = CFontTexture::Create( pDevice, m_TextureWidth, m_TextureHeight );
	if ( !m_pCurrentTexture )
	{
		return false;
	}

	// Establish the current working texture and DC
	m_hDC = m_pCurrentTexture->GetDC();
	if ( m_hDC == NULL )
	{
		SafeDelete( m_pCurrentTexture );
		return false;
	}
	
	// Fill the texture with solid blue
	HBRUSH hBrush = ::CreateSolidBrush( RGB( 0, 0, 255 ) );
	if( hBrush != NULL )
	{
		RECT const TexRect = { 0, 0, m_TextureWidth, m_TextureHeight }; 
		::FillRect( m_hDC, &TexRect, hBrush );
		::DeleteObject( hBrush );
	}

	// Begin writing at (1,1)
	m_LastWritePos.x = 1;
	m_LastWritePos.y = 1;

	return true;
}

// Renders all glyphs starting at the provided iterator to the given device context
bool CSystemFont::RenderGlyphs ( HDC hDC, 
								std::vector<WideLetter>::const_iterator& it, 
								std::vector<WideLetter>::const_iterator& end )
{
	bool Result = true;

	// Setup the DC for the font
	const int	   OldMode		= ::SetMapMode( hDC, MM_TEXT );	
	const COLORREF OldTextColor	= ::SetTextColor( hDC, RGB(255,255,255) );
	const COLORREF OldBackColor	= ::SetBkColor( hDC, RGB(0,0,255) );
	const int	   OldBkMode	= ::SetBkMode( hDC, TRANSPARENT );
	const UINT	   OldTextAlign	= ::SetTextAlign( hDC, TA_LEFT|TA_TOP|TA_UPDATECP );
		
	// Texture sheet is added after all writing is done to it, so the current texture collection size
	// is the current texture index.
	uint32 const FontSheetIndex = static_cast<uint32>( m_Sheets.size() );

	HRGN hRgn = ::CreateRectRgn( 0, 0, m_TextureWidth, m_TextureHeight );
	::SelectClipRgn ( hDC, hRgn );
    
    // Get the effective font height
    WCHAR str[2] = L"X";
    SIZE  size;
	::GetTextExtentPoint32W( hDC, str, 1, &size );

	m_FontHeight = size.cy;

    // Loop through all printable characters and output them to the bitmap, keeping track 
    // of the corresponding texture coords for each character
    LONG& x     = m_LastWritePos.x;
    LONG& y     = m_LastWritePos.y;
    int   sx;
    int   sy;

	// For each character, draw text on the DC and advance the current position
	while( it != end )
	{
		WideLetter c = *it;

		// Set an unprintable character
		if( c == L'\0' )
		{
			c = 0xffff;
		}

		str[0] = c;

		if( 0 == ::GetTextExtentPoint32W( hDC, str, 1, &size ) )
		{
			Result = false;
			goto LCleanup;
		}

		// Get char width a different way
		int charwidth;
		::GetCharWidth32( hDC, str[0], str[0], &charwidth );

		// Get the ABC widths for the letter
		ABC abc;
		if( FALSE == ::GetCharABCWidthsW( hDC, str[0], str[0], &abc ) )
		{
			abc.abcA = 0;
			abc.abcB = size.cx;
			abc.abcC = 0;
		}

		int w = abc.abcB;
		int h = size.cy;

		// Determine padding for outline and shadow effects
		int left_padding   = ( m_BorderEffect ? 1 : 0 );
		int right_padding  = ( m_BorderEffect ? ( m_ShadowEffect ? 2 : 1 ) : ( m_ShadowEffect ? 2 : 0 ) );
		int top_padding    = ( m_BorderEffect ? 1 : 0 );
		int bottom_padding = ( m_BorderEffect ? ( m_ShadowEffect ? 2 : 1 ) : ( m_ShadowEffect ? 2 : 0 ) );
	    
		// Hack to adjust for Kanji
		if( str[0] > 0x1000 )
		{
			w = h;
		}

		// Advance to the next line, if necessary
		if( x + w + left_padding + right_padding + 1 >= (int)m_TextureWidth )
		{
			x  = 1;
			y += h + top_padding + bottom_padding + 1;

			// Check to see if there's room to write the character here
			if( static_cast<uint32>(y + size.cy) > m_TextureHeight )
			{
				Result = true;
				goto LCleanup;
			}
		}

		sx = x;
		sy = y;

		// Adjust ccordinates to account for the leading edge
		if( abc.abcA >= 0 )
			x += abc.abcA;
		else
			sx -= abc.abcA;

		// Hack to adjust for Kanji
		if( str[0] > 0x1000 )
		{
			sx += abc.abcA;
		}

		// Add padding to the width and height
		w += left_padding + right_padding;
		h += top_padding + bottom_padding;
		abc.abcA -= left_padding;
		abc.abcB += left_padding + right_padding;
		abc.abcC -= right_padding;

		if( m_BorderEffect )
		{			
			::SetTextColor( hDC, RGB(0,0,0) );
			::MoveToEx( hDC, sx+0, sy+0, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+1, sy+0, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+2, sy+0, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+0, sy+1, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+2, sy+1, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+0, sy+2, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+1, sy+2, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			::MoveToEx( hDC, sx+2, sy+2, NULL );	::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );

			if( m_ShadowEffect )
			{
				::MoveToEx( hDC, sx+3, sy+3, NULL ); 
				::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			}

			// Output the letter
			::SetTextColor( hDC, RGB(255,255,255) );
			::MoveToEx( hDC, sx+1, sy+1, NULL  );	::ExtTextOutW( hDC, sx, sy, ETO_OPAQUE, NULL, str, 1, NULL );
		}
		else
		{
			if( m_ShadowEffect )
			{
				::SetTextColor( hDC, RGB(0,0,0) );
				::MoveToEx( hDC, sx+2, sy+2, NULL );	
				::ExtTextOutW( hDC, 0, 0, ETO_OPAQUE, NULL, str, 1, NULL );
			}

			// Output the letter
			::SetTextColor( hDC, RGB(255,255,255) );
			::MoveToEx( hDC, sx, sy, NULL );		
			::ExtTextOutW( hDC, sx, sy, ETO_OPAQUE, NULL, str, 1, NULL );
		}

		// Hack for extended characters (like Kanji) that don't seem to report
		// correct ABC widths. In this case, use the width calculated from
		// drawing the glyph.
		if( str[0] > 0x1000 )
		{
			POINT pos;
			::GetCurrentPositionEx( hDC, &pos );
			abc.abcB = pos.x - sx;

			if( abc.abcC < 0 )
				abc.abcB -= abc.abcC;

			w = abc.abcB;
		}

		GlyphAttributes GlyphAttribs;
		GlyphAttribs.Bounds.left		= ((float32)(x+0)) / m_TextureWidth;
		GlyphAttribs.Bounds.top			= ((float32)(y+0)) / m_TextureHeight;
		GlyphAttribs.Bounds.right		= ((float32)(x+w)) / m_TextureWidth;	
		GlyphAttribs.Bounds.bottom		= ((float32)(y+h)) / m_TextureHeight;
		GlyphAttribs.Offset				= (int16)(abc.abcA);
		GlyphAttribs.Width				= (int16)(abc.abcB);
		GlyphAttribs.Advance			= (int16)(abc.abcB + abc.abcC);
		GlyphAttribs.FontSheetIndex		= FontSheetIndex;

		// Check if we need to reset the unprintable character
		if( c == 0xffff )
		{
			c = 0;
		}

		SetGlyphAttributes( c, GlyphAttribs );

		// Advance the cursor to the next position
		x += w + 1;

		// Advance to next character
		++it;
    }

LCleanup:

	::SelectClipRgn( hDC, NULL );
	::DeleteObject( hRgn );
	::SetMapMode( hDC, OldMode );	
	::SetTextColor( hDC, OldTextColor );
	::SetBkColor( hDC, OldBackColor );	
	::SetBkMode( hDC, OldBkMode );
	::SetTextAlign( hDC, OldTextAlign );
	
	return Result;
}

// Applies an alpha channel to the provided device texture
/*static*/
bool WTCall CSystemFont::ApplyAlphaChannel( CFontTexture* pTexture )
{
	if( pTexture == NULL )
	{
		return false;
	}

	// If image is successfully locked then it will be unlocked when the TexLock object goes out of scope.
	HImageLock hTexLock = pTexture->GetImageLock();
	if (hTexLock == NULL)
	{
		return false;
	}

	CImage::Desc TexDesc;
	if ( !pTexture->GetDesc( TexDesc ) )
	{
		return false;
	}
    
	uint8* pBits = hTexLock->GetDataPtr();
	uint32 Pitch = static_cast<uint32>( TexDesc.RowPitch );
	for( uint32 i = 0; i < static_cast<uint32>(TexDesc.Height); ++i )
	{
		for( uint32 j = 0; j < static_cast<uint32>(TexDesc.Width); ++j )
		{
			uint32& Color = reinterpret_cast<uint32*>( pBits )[j];

			uint32 Result = 0x00000000u;

			//We wrote a grayscale color with alpha onto a blue background. The resultant 
			//green channel and blue channel can now be used to determine the luminance and 
			//alpha that was written into this pixel.

			uint32 G = Color >> 8;
			uint32 const InverseB = ~Color;
			uint32 const Alpha = (G + InverseB) & 0xffu;
			if (Alpha)
			{
				G &= 0xffu;
				int32 const Lum = Float32ToInt32( 255.0f * G / Alpha ); // Extract lum from above
				Result = (Alpha << 24L) + (Lum << 16) + (Lum << 8) + Lum;
			}

			Color = Result;
		}

		// Advance to the next row
		pBits += Pitch;
	}
	
	return true;
}

// Internal AddChars method; adds all characters in the provided stream to the font
// All public AddChars methods are funnelled to this method
bool CSystemFont::AddChars ( LPDIRECT3DDEVICE9 pDevice, const std::vector<WideLetter>& CharacterStream )
{
	assert( m_pDevice != NULL );
	if( m_pDevice == NULL || m_hFont == NULL || CharacterStream.size( ) == 0 )
	{
		// Invalid state
		return false;
	}

	if ( !m_pCurrentTexture )
	{
		// Create a valid texture/image for the collection and get the DC to it.
		if ( !AddNextTexture( pDevice ) )
		{
			return false;
		}
	}

	if ( m_pCurrentTexture == NULL || m_hDC == NULL )
	{
		// Failures within AddNextTexture should have been caught, but just to be safe
		return false;
	}

	std::vector<WideLetter>::const_iterator it = CharacterStream.begin( );
	std::vector<WideLetter>::const_iterator itEnd = CharacterStream.end( );
	do
	{
		// Render as many characters as we can to the current DC
		HGDIOBJ hOldObj	= ::SelectObject( m_hDC, m_hFont );
		bool Result = RenderGlyphs ( m_hDC, it, itEnd );
		::SelectObject( m_hDC, hOldObj );

		if( Result != true )
		{
			return Result;
		}

		// If there are any remaining characters in the stream, add the next texture 
		// to the collection and update from there
		if( it != CharacterStream.end( ) )
		{
			if( !AddNextTexture( pDevice ) )
			{
				return false;
			}
		}

	}	while( it != CharacterStream.end( ) );
	
	return true;
}

// Release Font texture image DC handle, apply alpha to image bits, and add device texture to font sheet
// collection.
bool CSystemFont::FinishFontTexture( CFontTexture * pFontTexture )
{
	pFontTexture->ReleaseDC();
	m_hDC = NULL;
	bool Result = ApplyAlphaChannel( pFontTexture );

	// Get actual device texture and add it to the font texture collection
	LPDIRECT3DBASETEXTURE9 pDeviceTexture = pFontTexture->GetTexture();
	if ( pDeviceTexture )
	{
		pDeviceTexture->AddRef();
		m_Sheets.push_back( pDeviceTexture );
	}
	else
	{
		Result = false;
	}

	return Result;
}


} // namespace UI
} // namespace Graphics
} // WT
