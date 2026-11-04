//
//	Vector2f.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector2i.h"
#include "WT/Math/Utils/MathUtil.h"
#ifdef USE_DIRECTX_7
#include "d3dxmath.h"
#elif defined USE_DIRECTX_8
#include "d3dx8math.h"
#else
#include "d3dx9math.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union

namespace WT
{
namespace Math
{


//
//	CVector2f :
//		Data class encapsulating a 2D floating point vector.
//

class CVector2f
{
public:

	// union'ed data members
	union
	{
		struct{ float32	x,y; };
		struct{ float32	e[2]; };
	};

	static const CVector2f gm_Zero;  // (0, 0)
	static const CVector2f gm_XAxis; // (1, 0)
	static const CVector2f gm_YAxis; // (0, 1)
	static const CVector2f gm_NegXAxis; // (-1, 0)
	static const CVector2f gm_NegYAxis; // (0, -1)
	
	// C'tors
	__forceinline CVector2f() : x(0), y(0) {};

	// Explicit Initialize to nothing c'tor
	__forceinline explicit CVector2f(bool) {}

	__forceinline explicit CVector2f(float32 s) : x(s), y(s) {}

	__forceinline CVector2f(float32 xx, float32 yy) : x(xx), y(yy) {};

	// Vector2i conversions
	__forceinline explicit	CVector2f				( const CVector2i& src ) : x((float32)src.x), y((float32)src.y) {};
	__forceinline			CVector2f&	operator =	( const CVector2i& src ) { x = (float32)src.x; y = (float32)src.y; return *this; }
	__forceinline operator	CVector2i				( )						{ return CVector2i((int32)(x+0.5f), (int32)(y+0.5f)); }
	
	// Comparison operators
	__forceinline bool     __fastcall	operator == (const CVector2f& v) const;
	__forceinline bool     __fastcall	operator != (const CVector2f& v) const;

	// Vector -> Vector operations (cross/dot products do not have operators)
	__forceinline CVector2f& __fastcall	operator +=	(const CVector2f& v);
	__forceinline CVector2f  __fastcall	operator +	(const CVector2f& v) const;
	__forceinline CVector2f& __fastcall	operator -=	(const CVector2f& v);
	__forceinline CVector2f  __fastcall	operator -	(const CVector2f& v) const;

	// Vector -> Scalar operations
	__forceinline CVector2f& __fastcall	operator +=	(const float32 v);
	__forceinline CVector2f  __fastcall	operator +	(const float32 v) const;
	__forceinline CVector2f& __fastcall	operator -=	(const float32 v);
	__forceinline CVector2f  __fastcall	operator -	(const float32 v) const;
	__forceinline CVector2f& __fastcall	operator *=	(const float32 v);
	__forceinline CVector2f  __fastcall	operator *	(const float32 v) const;
	__forceinline CVector2f& __fastcall	operator /=	(const float32 v);
	__forceinline CVector2f  __fastcall	operator /	(const float32 v) const;

	// Negate (this * (-1))
	__forceinline CVector2f  __fastcall	operator -	() const;

	// Array style accessors into the vector (take care, no bounds checking)
	__forceinline float32	__fastcall	operator [] (const int32 x) const;
	__forceinline float32&	__fastcall	operator [] (const int32 x);
	
	// Method style accessors
	__forceinline void		__fastcall	Set(float32 X, float32 Y);
	__forceinline void		__fastcall	Get(float32 *X, float32 *Y) const;

	// Vector Products
	__forceinline float32	__fastcall	DotProduct(const CVector2f& V2) const;
	__forceinline float32	__fastcall  CrossProduct(const CVector2f& V2);

	// Length ops
	__forceinline float32	__fastcall	LengthSquared() const;
	__forceinline float32	__fastcall	Length() const;
	__forceinline float32	__fastcall	DistanceBetweenSquared(const CVector2f& V2) const;
	__forceinline float32	__fastcall	DistanceBetween(const CVector2f& V2) const;
	
	// Unary ops
	__forceinline float32	__fastcall	Normalize();
	__forceinline void		__fastcall	Inverse();
	__forceinline void		__fastcall	Clear();
	
	// Misc. ops
	__forceinline bool		__fastcall 	IsNormalized();
	__forceinline bool		__fastcall	Compare(const CVector2f& V2,float32 tolerance);
	__forceinline void		__fastcall	Rand();
	__forceinline void		__fastcall	SetToHalf();

	// D3DX support
	__forceinline explicit CVector2f(const D3DXVECTOR2& D3DXSource);
	__forceinline CVector2f& __fastcall	operator=(const D3DXVECTOR2& rhs);
	__forceinline D3DXVECTOR2 * __fastcall GetD3DXVector2Ptr();
};

__forceinline CVector2f operator*( float32 s, CVector2f const& v );

#include "Vector2f.inl"


} // namespace Math
} // namespace WT

#pragma warning( pop )
