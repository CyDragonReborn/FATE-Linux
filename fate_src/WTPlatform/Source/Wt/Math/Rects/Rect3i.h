/*
	Rect3i.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector3i.h"
#include "WT/Math/Rects/Rect2i.h"

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union


namespace WT
{
namespace Math
{


//
//	CRect3i :
//		Data class encapsulating a 3D integer box.
//

class CRect3i
{
public:

	union
	{
		struct{ int32 left, top, front, right, bottom, back; };
		struct{ int32 minx, miny, minz, maxx, maxy, maxz; };
		struct{ CVector3i minv, maxv; };
	};


	__forceinline CRect3i() : minv(), maxv() {}
	__forceinline CRect3i(int32 Left, int32 Top, int32 Front, int32 Right, int32 Bottom, int32 Back)
	{
		left= Left; top = Top; front = Front; right = Right; bottom = Bottom; back = Back;
	}

	__forceinline CRect3i(const CRect2i& rect)
	{
		left= rect.left; top = rect.top; front = 0; right = rect.right; bottom = rect.bottom; back = 1;
	}

	__forceinline int32 Width() const { return right - left; }
	__forceinline int32 Height() const { return bottom - top; }
	__forceinline int32 Depth() const { return back - front; }
	__forceinline int32 Volume() const { return Width()*Height()*Depth(); }

	__forceinline bool EqualSize( int32 OtherWidth, int32 OtherHeight, int32 OtherDepth ) const
	{
		return ( (Width() == OtherWidth) && (Height() == OtherHeight) && (Depth() == OtherDepth) );
	}

	__forceinline bool EqualSize( const CRect3i& other ) const
	{
		return ( (Width() == other.Width()) && (Height() == other.Height()) && (Depth() == other.Depth()) );
	}

	__forceinline void Normalize( )
	{
		right = Width(); bottom = Height(); back = Depth();
		left = top = front = 0;
	}
};


} // namespace Math
} // namespace WT

#pragma warning( pop )
