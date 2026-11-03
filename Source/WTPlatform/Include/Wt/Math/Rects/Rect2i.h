/*
	Rect2i.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector2i.h"
#include "windows.h" //Needed by d3d9types.h
#ifdef USE_DIRECTX_7
#include "d3dtypes.h"
#elif defined USE_DIRECTX_8
#include "d3d8types.h"
#else
#include "d3d9types.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union


namespace WT
{
namespace Math
{


//
//	CRect2i :
//		Data class encapsulating a 2D integer rect.
//

class CRect2i
{
public:

	// union'ed data members
	union
	{
		struct{ int32 left, top, right, bottom; };
		struct{ int32 minx, miny, maxx, maxy; };
		struct{ int32 ulx, uly, lrx, lry; };
		struct{ CVector2i mmin, mmax; };
	};

	// C'tors
	__forceinline CRect2i() : mmin(), mmax() {}
	__forceinline CRect2i(int32 Left, int32 Top, int32 Right, int32 Bottom) 
    {
		left = Left;
        top = Top; 
        right = Right; 
        bottom = Bottom;
    }

	__forceinline int32 Width() const  { return right - left; }
	__forceinline int32 Height() const { return bottom - top; }

	// D3D support
	__forceinline explicit CRect2i(const D3DRECT& D3DRect)
	{
		left = D3DRect.x1;
		top = D3DRect.y1;
		right = D3DRect.x2;
		bottom = D3DRect.y2;
	}
	__forceinline CRect2i& __fastcall operator=(const D3DRECT& rhs)
	{
		left = rhs.x1;
		top = rhs.y1;
		right = rhs.x2;
		bottom = rhs.y2;

		return *this;
	}
	__forceinline D3DRECT * __fastcall GetD3DRectPtr()
	{
		return reinterpret_cast<D3DRECT *>(this);
	}

	// Win32 support
	__forceinline explicit CRect2i(const RECT& Rect)
	{
		left = Rect.left;
		top = Rect.top;
		right = Rect.right;
		bottom = Rect.bottom;
	}
	__forceinline CRect2i& __fastcall operator=(const RECT& rhs)
	{
		left = rhs.left;
		top = rhs.top;
		right = rhs.right;
		bottom = rhs.bottom;

		return *this;
	}

	__forceinline LPRECT __fastcall GetRectPtr() {return reinterpret_cast<LPRECT>( this );}
	__forceinline LPCRECT __fastcall GetRectPtr() const {return reinterpret_cast<LPCRECT>( this );}
};


} // namespace Math
} // namespace WT

#pragma warning( pop )
