//
//	Vector2i.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union

#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{


//
//	CVector2i :
//		Data class encapsulating a 2D integer vector.
//

class CVector2i
{
public:

	// union'ed data members
	union
	{
		struct{ int32	x,y; };
		struct{ int32	e[2]; };
	};

	// C'tors
	// Default c'tor initialize to 0
	__forceinline CVector2i() : x(0), y(0) {};

	// Explicit Initialize to nothing c'tor
	__forceinline explicit CVector2i(bool) {}

	__forceinline explicit CVector2i(int32 s) : x(s), y(s) {}

	__forceinline CVector2i(int32 xx, int32 yy) : x(xx), y(yy) {};

	// Comparison operators
	__forceinline bool __fastcall		operator == (const CVector2i& v) const;
	__forceinline bool __fastcall		operator != (const CVector2i& v) const;

	// Vector -> Vector operations (cross/dot products do not have operators)
	__forceinline CVector2i& __fastcall	operator +=	(const CVector2i& v);
	__forceinline CVector2i  __fastcall	operator +	(const CVector2i& v) const;
	__forceinline CVector2i& __fastcall	operator -=	(const CVector2i& v);
	__forceinline CVector2i  __fastcall	operator -	(const CVector2i& v) const;

	// Vector -> Scalar operations
	__forceinline CVector2i& __fastcall	operator +=	(const int32 v);
	__forceinline CVector2i  __fastcall	operator +	(const int32 v) const;
	__forceinline CVector2i& __fastcall	operator -=	(const int32 v);
	__forceinline CVector2i  __fastcall	operator -	(const int32 v) const;
	__forceinline CVector2i& __fastcall	operator *=	(const int32 v);
	__forceinline CVector2i  __fastcall	operator *	(const int32 v) const;
	__forceinline CVector2i& __fastcall	operator /=	(const int32 v);
	__forceinline CVector2i  __fastcall	operator /	(const int32 v) const;
	__forceinline CVector2i& __fastcall	operator *=	(const float32 v);
	__forceinline CVector2i  __fastcall	operator *	(const float32 v) const;
	__forceinline CVector2i& __fastcall	operator /=	(const float32 v);
	__forceinline CVector2i  __fastcall	operator /	(const float32 v) const;

	/// Negate (this * (-1))
	__forceinline CVector2i	__fastcall	operator -	() const;

	/// Array style accessors into the vector (take care, no bounds checking)
	__forceinline int32		__fastcall	operator [] (const int32 x) const;
	__forceinline int32&	__fastcall	operator [] (const int32 x);
	
	/// Method style accessors
	__forceinline void	__fastcall	Set(int32 X, int32 Y);
	__forceinline void	__fastcall	Get(int32 *X, int32 *Y) const;

	/// Vector Products
	__forceinline int32	__fastcall	DotProduct(const CVector2i& V2) const;
	__forceinline int32	__fastcall  CrossProduct(const CVector2i& V2);

	/// Length ops
	__forceinline int32	__fastcall	LengthSquared() const;
	__forceinline int32	__fastcall	Length() const;
	__forceinline int32	__fastcall	DistanceBetweenSquared(const CVector2i& V2) const;
	__forceinline int32	__fastcall	DistanceBetween(const CVector2i& V2) const;

	/// Unary ops
	__forceinline int32	__fastcall	Normalize();
	__forceinline void	__fastcall	Inverse();
	__forceinline void	__fastcall	Clear();
	
	/// Misc. ops
	__forceinline bool	__fastcall 	IsNormalized();
	__forceinline bool	__fastcall	Compare(const CVector2i& V2,int32 tolerance);
};

__forceinline CVector2i operator*( int32 s, CVector2i const& v );

} // namespace Math
} // namespace WT

#include "Vector2i.inl"

#pragma warning( pop )
