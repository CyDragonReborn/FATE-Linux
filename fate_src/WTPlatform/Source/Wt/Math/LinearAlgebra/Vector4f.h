//
//	Vector4f.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include <math.h>
#include "WT/Math/LinearAlgebra/Vector3f.h"
#ifdef USE_DIRECTX_7
#include "d3dxmath.h"
#elif defined USE_DIRECTX_8
#include "d3dx8math.h"
#else
#include "d3dx9math.h"
#endif

#pragma warning (push)
#pragma warning (disable: 4714)		// warning: "__forceinline not inlined"
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union

namespace WT
{
namespace Math
{


//Forward declarations
class CMatrix4f;

// ---------------------------------------------------
//	CVector4f :
//		Data class encapsulating a 4D floating point
//		vector and related functionality, extends
//		CVector3f
//
//	Note: 
//	CVector4f inherits from CVector3f, and
//	brings it's functionality. However some things
//	behave differently, eg: scalar ops on CVector3f
//	are 3D, but on CVector4f are 4D. When using a 
//	CVector4f be aware that some of it's methods modify
//	4th element. This is important to remeber especially
//	when dealing with CMatrix4f's, which uses CVector4f
//	for it's internal storage. 99% of the time, you
//	don't actually want to modify the 4th elements in
//	in a matrix, and so be carefull what operations you
//	use.
//
//	CVector4f is provided mostly for the use of CMatrix4f,
//	and thus it doesn't provided all the complimentary 
//	4d operations for the 3d ones from CVector3f. It can
//	be thought really as a 3d vector with a 4th component.
// ---------------------------------------------------

class CVector4f : public CVector3f
{
public:
	// 4th dimension component
	float32	w;

	// C'tors

	// Default C'tor initializes to 0,0,0,1
	__forceinline CVector4f() : CVector3f(), w(1) {};

	// Explicit Initialize to nothing c'tor
	__forceinline explicit CVector4f(bool) : CVector3f(false) {}

	__forceinline explicit CVector4f(float32 s) : CVector3f(s), w(s) {}

	// 4D C'tors
	__forceinline CVector4f(const CVector4f& v) : CVector3f(v.x, v.y, v.z), w(v.w) {};
	__forceinline CVector4f(float32 xx, float32 yy, float32 zz, float32 ww) 
		: CVector3f(xx, yy, zz), w(ww) {};

	// 3D C'tors Zero 4th element
	__forceinline CVector4f(float32 xx, float32 yy, float32 zz) 
		: CVector3f(xx, yy, zz), w(0) {};
	__forceinline CVector4f(const CVector3f& v) : CVector3f(v), w(0) {};


	// Assignment

	// 3D assigment SLICES the vector. 4th element is NOT touched.
	__forceinline CVector4f& __fastcall	operator =  (const CVector3f& v)
		{ x = v.x; y = v.y; z = v.z; return *this; }

	// 4D assigment. Full copy.
	__forceinline CVector4f& __fastcall	operator =  (const CVector4f& v)
		{ x = v.x; y = v.y; z = v.z; w = v.w; return *this; }

	// Vector->Vector Ops

	// Comparison operators
	__forceinline bool   __fastcall	operator == (const CVector4f& v) const;
	__forceinline bool   __fastcall	operator != (const CVector4f& v) const;

	// 3D Ops do not touch the 4th element
	__forceinline CVector4f& __fastcall	operator +=	(const CVector3f& v);
	__forceinline CVector4f  __fastcall	operator +	(const CVector3f& v) const;
	__forceinline CVector4f& __fastcall	operator -=	(const CVector3f& v);
	__forceinline CVector4f  __fastcall	operator -	(const CVector3f& v) const;

	// 4D Ops do a full 4D op
	__forceinline CVector4f& __fastcall	operator +=	(const CVector4f& v);
	__forceinline CVector4f  __fastcall	operator +	(const CVector4f& v) const;
	__forceinline CVector4f& __fastcall	operator -=	(const CVector4f& v);
	__forceinline CVector4f  __fastcall	operator -	(const CVector4f& v) const;

	// Vector->Scalar Ops

	// These do a FULL 4D op, in contrast to the base CVector3f ones.
	// If you with 3D ops on a CVector4f use the inherited 
	// ::Scale3d ::Add3d etc. methods from CVector3f.
	__forceinline CVector4f& __fastcall	operator +=	(const float32 v);
	__forceinline CVector4f  __fastcall	operator +	(const float32 v) const;
	__forceinline CVector4f& __fastcall	operator -=	(const float32 v);
	__forceinline CVector4f  __fastcall	operator -	(const float32 v) const;
	__forceinline CVector4f& __fastcall	operator *=	(const float32 v);
	__forceinline CVector4f  __fastcall	operator *	(const float32 v) const;
	__forceinline CVector4f& __fastcall	operator /=	(const float32 v);
	__forceinline CVector4f  __fastcall	operator /	(const float32 v) const;

	// Negate (4D)
	__forceinline CVector4f  __fastcall	operator -	() const;
	
	// Accessors
	__forceinline void	__fastcall	Set(float32 X, float32 Y, float32 Z, float32 W);
	__forceinline void	__fastcall	Get(float32 *X, float32 *Y, float32 *Z, float32 *W) const;
	__forceinline void	__fastcall	Set(float32 X, float32 Y, float32 Z);
	__forceinline void	__fastcall	Get(float32 *X, float32 *Y, float32 *Z) const;

	// Vector products
	__forceinline	float32	__fastcall	DotProduct(const CVector4f& V2) const;

	// Vector/matrix operators
	__forceinline CVector4f __fastcall  operator *( const CMatrix4f& M ) const;
	__forceinline CVector4f __fastcall  operator /( const CMatrix4f& M ) const;

	// D3DX support
	__forceinline explicit CVector4f(const D3DXVECTOR4& D3DXSource);
	__forceinline CVector4f& __fastcall operator=(const D3DXVECTOR4& rhs);
	__forceinline D3DXVECTOR4* __fastcall GetD3DXVector4Ptr();
	__forceinline D3DXVECTOR4 const * __fastcall GetD3DXVector4Ptr() const;
};

__forceinline CVector4f operator*( float32 s, CVector4f const& v );

#include "Vector4f.inl"


} // namespace Math
} // namespace WT

#pragma warning (pop)
