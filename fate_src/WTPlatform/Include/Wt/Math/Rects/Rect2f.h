/*
/	Rect2f.h
/
/	Copyright (C) 2005 WildTangent, Inc. 
/	All Rights Reserved
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector2f.h"

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union


namespace WT
{
namespace Math
{


//
//	CRect2f :
//		Data class encapsulating a 2D float rect.
//

class CRect2f
{
public:

	// union'ed data members
	union
	{
		struct{ float32 left, top, right, bottom; };
		struct{ float32 minx, miny, maxx, maxy; };
		struct{ float32 ulx, uly, lrx, lry; };
		struct{ CVector2f mmin, mmax; };
	};

	// C'tors
	// Default c'tor does NOT initialize ANY data (perf)
	__forceinline CRect2f() : mmin(), mmax() {}
	__forceinline CRect2f(float32 Left, float32 Top, float32 Right, float32 Bottom)
    {
		left = Left;
        top = Top;
        right = Right;
        bottom = Bottom;
	}

	__forceinline float32 Width() const  { return right - left; }
	__forceinline float32 Height() const { return bottom - top; }

};


} // namespace Math
} // namespace WT

#pragma warning( pop )
