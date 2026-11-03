//
// GlyphAttributes.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
#pragma once

#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

/// Describies attributes for a single displayable font glpyh
struct GlyphAttributes
{
	GlyphAttributes( )
		: Bounds( )
		, Offset( 0 )
		, Width( 0 )
		, Advance( 0 )
		, TopPadding( 0.0f )
		, BottomPadding( 0.0f )
		, YAdvance( 0.0f )
		, FontSheetIndex( -1 )
	{
	}

	Math::CRect2f	Bounds;				// texture coordinates for the glyph image
	float32			TopPadding;			// top padding
	float32			BottomPadding;		// bottom padding
	float32			YAdvance;			// y-advance
	int32			FontSheetIndex;		// index of owning font sheet; -1 if unowned
    int16			Offset;				// the glyph's start pixel offset
	int16			Width;				// pixel width of the glyph
	int16			Advance;			// number of pixels to advance after the glyph
};


} // namespace UI
} // namespace Graphics
} // namespace WT
