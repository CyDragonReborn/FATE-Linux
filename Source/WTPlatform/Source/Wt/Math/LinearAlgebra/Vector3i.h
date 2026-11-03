//
//	Vector3i.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include <math.h>

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union

namespace WT
{
namespace Math
{


//
//	Vector3i :
//		Data class encapsulating a 3D integer
//		vector and related functionality.
//
//	This is NOT NOT NOT NOT NOT FixedPoint, but 
//	straight Integer math
//

class CVector3i
{
public:

	// union'ed data members
	union
	{
		struct{ int32		x,y,z; };
		struct{ int32		e[3]; };
	};


	// C'tors
	/// Default c'tor does NOT initialize ANY data (perf)
	__forceinline CVector3i() {};
	__forceinline CVector3i(int32 xx, int32 yy, int32 zz) : x(xx), y(yy), z(zz) {};
	__forceinline CVector3i(const CVector3i& v) : x(v.x), y(v.y), z(v.z) {};
	__forceinline CVector3i& __fastcall operator =  (const CVector3i& v)
		{ x = v.x; y = v.y; z = v.z; return *this; }

	// Comparison operators
	__forceinline bool     __fastcall   operator == (const CVector3i& v) const;
	__forceinline bool     __fastcall   operator != (const CVector3i& v) const;

	// Vector -> Vector operations (cross/dot products do not have operators)
	__forceinline CVector3i& __fastcall	operator +=	(const CVector3i& v);
	__forceinline CVector3i  __fastcall	operator +	(const CVector3i& v) const;
	__forceinline CVector3i& __fastcall	operator -=	(const CVector3i& v);
	__forceinline CVector3i  __fastcall	operator -	(const CVector3i& v) const;

	// Vector -> Scalar operations
	__forceinline CVector3i& __fastcall	operator +=	(const int32 v);
	__forceinline CVector3i  __fastcall	operator +	(const int32 v) const;
	__forceinline CVector3i& __fastcall	operator -=	(const int32 v);
	__forceinline CVector3i  __fastcall	operator -	(const int32 v) const;
	__forceinline CVector3i& __fastcall	operator *=	(const int32 v);
	__forceinline CVector3i  __fastcall	operator *	(const int32 v) const;
	__forceinline CVector3i& __fastcall	operator /=	(const int32 v);
	__forceinline CVector3i  __fastcall	operator /	(const int32 v) const;
	__forceinline CVector3i& __fastcall	operator *=	(const float32 v);
	__forceinline CVector3i  __fastcall	operator *	(const float32 v) const;
	__forceinline CVector3i& __fastcall	operator /=	(const float32 v);
	__forceinline CVector3i  __fastcall	operator /	(const float32 v) const;

	// Vector unary operations
	/// Negate (this * (-1))
	__forceinline CVector3i	__fastcall	operator -	() const;
	
	/// Array style accessors into the vector (take care, no bounds checking)
	__forceinline int32		__fastcall	operator [] (const int32 x) const;
	__forceinline int32&	__fastcall	operator [] (const int32 x);
	
	/// Method style accessors
	__forceinline void	__fastcall	Set(int32 X, int32 Y, int32 Z);
	__forceinline void	__fastcall	Get(int32 *X, int32 *Y, int32 *Z) const;
	__forceinline void	__fastcall	Clear();

	/// Vector products
	__forceinline int32	__fastcall	DotProduct(const CVector3i& V2) const;
	__forceinline int32	__fastcall  CrossProduct(const CVector3i& V1, const CVector3i& V2);

	/// Length ops
	__forceinline int32	__fastcall	LengthSquared() const;
	__forceinline int32	__fastcall	Length() const;

	/// This = Result
	__forceinline int32	__fastcall	Normalize();

	/// Negate	
	__forceinline void	__fastcall	Inverse();

	__forceinline int32	__fastcall	DistanceBetweenSquared(const CVector3i& V2) const;
	__forceinline int32	__fastcall	DistanceBetween(const CVector3i& V2) const;
	
	/// Misc. operations
	__forceinline bool	__fastcall 	IsNormalized();
	__forceinline bool	__fastcall	Compare(const CVector3i& V2,int32 tolarance);
};

__forceinline CVector3i operator*( int32 s, CVector3i const& v );


#include "Vector3i.inl"


} // namespace Math
} // namespace WT

#pragma warning( pop )
