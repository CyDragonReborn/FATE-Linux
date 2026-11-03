//
//	Vector3f.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector3i.h"
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


#include <math.h>

namespace WT
{
namespace Math
{


//Forward declarations
class CMatrix4f;

//
//	CVector3f :
//		Data class encapsulating a 3D floating point
//		vector and related functionality
//

class CVector3f
{
public:

	// union'ed data members
	union
	{
		struct{ float32	x,y,z; };
		struct{ float32 e[3]; };
	};

	static CVector3f const gm_Zero;  // (0, 0, 0)
	static CVector3f const gm_XAxis; // (1, 0, 0)
	static CVector3f const gm_YAxis; // (0, 1, 0)
	static CVector3f const gm_ZAxis; // (0, 0, 1)
	static CVector3f const gm_NegXAxis; // (-1, 0, 0)
	static CVector3f const gm_NegYAxis; // (0, -1, 0)
	static CVector3f const gm_NegZAxis; // (0, 0, -1)
	static CVector3f const gm_UnitScale; // (1, 1, 1)

	// C'tors

	// Default c'tor initialize to 0 
	__forceinline CVector3f() : x(0), y(0), z(0) {};

	// Explicit Initialize to nothing c'tor
	__forceinline explicit CVector3f(bool) {}

	__forceinline explicit CVector3f(float32 s) : x(s), y(s), z(s) {}

	__forceinline CVector3f(const CVector3f& v) 
		: x(v.x), y(v.y), z(v.z) {};
	__forceinline CVector3f(float32 xx, float32 yy, float32 zz) : x(xx), y(yy), z(zz) {};

	__forceinline CVector3f& __fastcall operator =  (const CVector3f& v)
		{ x = v.x; y = v.y; z = v.z; return *this; }

	// Vector3i conversions
	__forceinline explicit	CVector3f( const CVector3i& src ) : x((float32)src.x), y((float32)src.y), z((float32)src.z) {};
	__forceinline operator	CVector3i() {return CVector3i( Float32ToInt32( x ), Float32ToInt32( y ), Float32ToInt32( z ) ); }
	__forceinline			CVector3f&	operator = ( const CVector3i& src ) { x = (float32)src.x; y = (float32)src.y; z = (float32)src.z; return *this; }

	// Comparison operators
	__forceinline bool __fastcall		operator == (const CVector3f& v) const;
	__forceinline bool __fastcall		operator != (const CVector3f& v) const;

	// Vector -> Vector operations (cross/dot products do not have operators)
	__forceinline CVector3f&	__fastcall	operator +=	(const CVector3f& v);
	__forceinline CVector3f		__fastcall	operator +	(const CVector3f& v) const;
	__forceinline CVector3f&	__fastcall	operator -=	(const CVector3f& v);
	__forceinline CVector3f		__fastcall	operator -	(const CVector3f& v) const;

	// Vector -> Scalar operations
	// (be carefull with 3d->4d operations, these are 3D, but in the case 
	// of a CVector4f they are 4D ops
	__forceinline CVector3f&	__fastcall	operator +=	(const float32 v);
	__forceinline CVector3f		__fastcall	operator +	(const float32 v) const;
	__forceinline CVector3f&	__fastcall	operator -=	(const float32 v);
	__forceinline CVector3f		__fastcall	operator -	(const float32 v) const;
	__forceinline CVector3f&	__fastcall	operator *=	(const float32 v);
	__forceinline CVector3f		__fastcall	operator *	(const float32 v) const;
	__forceinline CVector3f&	__fastcall	operator /=	(const float32 v);
	__forceinline CVector3f		__fastcall	operator /	(const float32 v) const;

	// Vector unary operations

	// Negate (this * (-1))
	__forceinline CVector3f	__fastcall	operator -	() const;
	
	// Array style accessors into the vector (take care, no bounds checking)
	__forceinline float32 const& __fastcall	operator [] (const int32 x) const;
	__forceinline float32&		 __fastcall	operator [] (const int32 x);
	
	// Method style accessors
	__forceinline	void	__fastcall	Set(float32 X, float32 Y, float32 Z);
	__forceinline	void	__fastcall	Get(float32 *X, float32 *Y, float32 *Z) const;
	__forceinline	void	__fastcall	Clear();

	// Vector products
	__forceinline	float32	__fastcall	DotProduct(const CVector3f& V2) const;

	__forceinline		 CVector3f&	__fastcall	CrossProduct(const CVector3f& V1, const CVector3f& V2);
	__forceinline		 CVector3f	__fastcall  CrossProduct(const CVector3f& V2) const { return CVector3f( false ).CrossProduct(*this, V2); }

	// Vector/matrix operators
	// The vector is treated as a 4-vector with a fourth component of 1. The 
	// 4th component of the result is discarded.
	__forceinline CVector3f __fastcall  operator *( const CMatrix4f& M ) const;
	__forceinline CVector3f __fastcall  operator /( const CMatrix4f& M ) const;

	// Length ops
	__forceinline float32	__fastcall	LengthSquared() const;
	__forceinline float32	__fastcall	Length() const;
	
	// This = Result
	__forceinline float32 __fastcall Normalize();

	// Negate
	__forceinline void __fastcall Inverse();
	
	__forceinline float32 __fastcall DistanceBetweenSquared(const CVector3f& V2) const;
	__forceinline float32 __fastcall DistanceBetween(const CVector3f& V2) const;

	// This * (End - Start)
	__forceinline float32 __fastcall RayDotProduct(const CVector3f& RayStart,const CVector3f& RayEnd); 

	// 3D only scales (this is to separate the functionality from the 4D operations which CVector4f adds)
	__forceinline void	__fastcall	Scale3d( float32 Scale );
	__forceinline void	__fastcall	Subtract3d(const CVector3f& V1, const CVector3f& V2);
	__forceinline void	__fastcall	Add3d(const CVector3f& V1, const CVector3f& V2 );
	__forceinline void	__fastcall	AddScaled3d(const CVector3f& V1, const CVector3f& V2, float32 V2Scale);


	// Misc. operations

	// this = result
	float32	__fastcall TriangleNormal( const CVector3f& V1, const CVector3f& V2, const CVector3f& V3 );

	bool	__fastcall IsNormalized( ) const;
	bool	__fastcall Compare( const CVector3f& V2, float32 tolerance ) const;
	float32	__fastcall TriangleArea( const CVector3f& V2, const CVector3f& V3 ) const;

	//Compute and return a perpendicular, non-zero vector
	CVector3f __fastcall Perp() const;

	//Compute and return a perpendicular, unit vector
	CVector3f __fastcall UnitPerp() const;

	__forceinline void __fastcall Rand();

	// D3D support
	__forceinline explicit CVector3f(const D3DVECTOR& D3DSource);
	__forceinline CVector3f& __fastcall operator=(const D3DVECTOR& rhs);
	__forceinline D3DVECTOR * __fastcall GetD3DVectorPtr();
	__forceinline D3DVECTOR const* __fastcall GetD3DVectorPtr() const;
	__forceinline __fastcall operator D3DVECTOR*();
	__forceinline __fastcall operator const D3DVECTOR*() const;

	// D3DX support
	__forceinline explicit CVector3f(const D3DXVECTOR3& D3DXSource);
	__forceinline CVector3f& __fastcall operator=(const D3DXVECTOR3& rhs);
	__forceinline D3DXVECTOR3* __fastcall GetD3DXVector3Ptr();
	__forceinline D3DXVECTOR3 const * __fastcall GetD3DXVector3Ptr() const;
};

__forceinline CVector3f operator*( float32 s, CVector3f const& v );

#include "Vector3f.inl"


} // namespace Math
} // namespace WT

#pragma warning( pop )
