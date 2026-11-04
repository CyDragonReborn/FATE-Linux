//
// Font.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/Callback.h"
#include "WT/Graphics/UI/GlyphAttributes.h"
#include <vector>
#include <map>
#include "d3d9.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

// Typedefs
class CFont;
typedef std::vector<CFont *>					FontCollection;

typedef wchar_t									WideLetter;
typedef std::vector<LPDIRECT3DBASETEXTURE9>		TextureCollection;
typedef std::map<WideLetter, GlyphAttributes>	GlyphMap;
typedef std::map<uint32, int16>					KerningMap;

// Defines an event that is raised while measuring glyph attributes 
// Implementations should return true if the passed GlyphAttributes structure was modified; false otherwise
typedef System::CCallback2r<WideLetter, GlyphAttributes&, bool>	GlyphAttributesCallback;
typedef System::CEvent2r<WideLetter, GlyphAttributes&, bool>	GlyphAttributesEvent;

#define KERNING_KEY(a, b)	(a & 0xffff) | ((b & 0xffff) << 16)


class CFont : public System::CObject
{
public:

	WT_CLASS(CFont, System::CObject)

	CFont::CFont() : m_FontHeight( 0 )
	{
		ClearGlyphAttributesTable();
	}

	virtual CFont::~CFont()
	{
		// Release all textures in the texture collection
		TextureCollection::iterator it = m_Sheets.begin();
		while (it != m_Sheets.end())
		{
			SafeRelease(*it);
			++it;
		}
	}


	// Retrieves the texture collection for this font
	virtual TextureCollection& GetTextures ( )
	{
		return m_Sheets;
	}

	// Sets the glyph attributes associated with the given character
	virtual void SetGlyphAttributes ( WideLetter Letter, const GlyphAttributes& GlyphAttribs )
	{
		// Set within the appropriate container for the given letter, depending on whether the high byte is set or not
		if( Letter <= 0xff )
		{
			::memcpy( &m_GlyphAttributes[Letter], &GlyphAttribs, sizeof(GlyphAttributes) );
		}
		else
		{
			// Insert the pair into the extended attribute map
			m_GlyphAttributesEx.insert( std::pair<WideLetter, GlyphAttributes>( Letter, GlyphAttribs ) );
		}
	}

	// Retrieves the glyph attributes associated with the given character
	// This method raises the OnMeasureGlyphAttributes event before returning the final structure.
	// This method returns true if either the glyph information was found internally, or a 
	// registered callback modified the GlyphAttributes structure.
	// If glyph attributes for the given character could not be found, attributes representing a default
	// 'unprintable' character will be copied into the given glyph attribute structure (if possible) before the
	// OnMeasureGlyphAttributes callback is raised
	virtual bool GetGlyphAttributes ( WideLetter Letter, GlyphAttributes& GlyphAttribs ) const
	{
		bool Found = false;

		// Retrieve from the appropriate container for the given letter, depending on whether the high byte is set or not
		if( Letter <= 0xff )
		{
			::memcpy( &GlyphAttribs, &m_GlyphAttributes[Letter], sizeof(GlyphAttributes) );
			Found = GlyphAttribs.FontSheetIndex >= 0 ? true : false;
		}
		else
		{
			// Retrieve from the extended attribute map
			GlyphMap::const_iterator it = m_GlyphAttributesEx.find( Letter );
			if( it != m_GlyphAttributesEx.end() )
			{
				::memcpy( &GlyphAttribs, &it->second, sizeof(GlyphAttributes) );
				Found = true;
			}
		}	

		// If the glyph entry was not found, copy the data from the default 'null' glyph from the 
		// flat attribute array
		if( !Found )
		{
			::memcpy( &GlyphAttribs, &m_GlyphAttributes[0], sizeof(GlyphAttributes) );
			// Update the Found result, as the null character might not exist
			Found = GlyphAttribs.FontSheetIndex >= 0 ? true : false;
		}

		// Raise the attributes event so the attributes structure can be potentially modified by the user
		if( m_GlyphAttributesEvent.GetCount() > 0 )
		{
			Found = Found || m_GlyphAttributesEvent( Letter, GlyphAttribs );
		}

		return Found;
	}

	// Clears all glyph attributes mappings from the font
	// If the ClearKerningTable parameter is set to kTrue, then all kerning information is cleared as well
	virtual void ClearGlyphAttributes( bool ClearKerningTable )
	{
		ClearGlyphAttributesTable();
		m_GlyphAttributesEx.clear();

		if( ClearKerningTable )
		{
			m_KerningMap.clear();
		}
	}

	// Retrieves the glyph attributes event associated with the font. This event is raised during calls to GetGlyphAttributes.
	// This allows developers to intercept and potentially modify glyph attributes during text layout
	virtual GlyphAttributesEvent& OnMeasureGlyphAttributes()
	{
		return m_GlyphAttributesEvent;
	}

	// Retrieves the logical height of the font, in pixels
	virtual int32 GetHeight() const
	{
		return m_FontHeight;
	}

	// Sets the kerning amount, in pixels, between the two given characters
	virtual void SetCharacterKerning ( WideLetter First, WideLetter Second, int16 Kerning )
	{
		m_KerningMap.insert( std::pair<uint32, int16>( KERNING_KEY( First, Second ), Kerning ) );
	}

	// Retrieves the kerning amount, in pixels, between the two given characters
	virtual int16 GetCharacterKerning ( WideLetter First, WideLetter Second ) const
	{
		KerningMap::const_iterator it = m_KerningMap.find( KERNING_KEY( First, Second ) );
		if( it != m_KerningMap.end( ) )
		{
			return it->second;
		}

		return 0;
	}

protected:	// methods

	// Clears the flat extended ASCII glyph table with default attribute information
	void ClearGlyphAttributesTable()
	{
		const uint32 AttributeCount = sizeof(m_GlyphAttributes) / sizeof(GlyphAttributes);

		const GlyphAttributes DefaultGlyphAttributes;

		GlyphAttributes* pAttributes = &m_GlyphAttributes[0];
		for( uint32 i = 0; i < AttributeCount; ++i )
		{
			::memcpy( pAttributes, &DefaultGlyphAttributes, sizeof(GlyphAttributes) );
			++pAttributes;
		}
	}

	// Determines whether glyph attributes exist for the given character
	inline bool GlyphAttributesExist( WideLetter Letter ) const
	{
		if( Letter <= 0xff )
		{
			// Look up glyph from extended ASCII table
			return m_GlyphAttributes[Letter].FontSheetIndex >= 0 ? true : false;
		}

		// Retrieve from the extended attribute map
		return m_GlyphAttributesEx.find( Letter ) != m_GlyphAttributesEx.end() ? true : false;			
	}

protected:	// data

	int32							m_FontHeight;

	TextureCollection				m_Sheets;					// font sheet collection

	// Glyph attributes are partitioned into two structures; one for range [0x0000, 0x00ff], and one for range [0x0100, 0xffff]
	GlyphAttributes					m_GlyphAttributes[256];		// extended ASCII table
	GlyphMap						m_GlyphAttributesEx;		// attributes beyond 0x00ff
												
	mutable GlyphAttributesEvent	m_GlyphAttributesEvent;		// glyph attributes event raised during GetGlyphAttributes
	
	KerningMap						m_KerningMap;				// maintains kerning relationships between letters
};


//
// Enums
//

namespace EFontWeight
{
	// Describes various font weight types
	enum Weight
	{
		DontCare	= 0,
		Thin		= 1,
		ExtraLight	= 2,
		Light		= 3,
		Normal		= 4,
		Medium		= 5,
		SemiBold	= 6,
		Bold		= 7,
		ExtraBold	= 8,
		Heavy		= 9,
	};
}

// Specifies character sets for various locale
namespace ECharacterSet
{
	enum Set
	{
		Ansi		= 0,
		Default		= 1,
		Symbol		= 2,
		Mac			= 77,
		Baltic		= 186,
		ChineseBig5 = 136,
		EastEurope	= 238,
		GB2312		= 134,
		Greek		= 161,
		Hangul		= 129,		
		Russian		= 204,
		Shiftjis	= 128,		
		Turkish		= 162,
		Vietnamese	= 163,
		Johab		= 130,
		Arabic		= 178,
		Hebrew		= 177,
		Thai		= 222,
		OEM			= 255,
	};
}


} // namespace UI
} // namespace Graphics
} // namespace WT
