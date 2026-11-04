//
//	Plane.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
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
//	CPlane
//		Class encapsulating data and functionality
//		related to the manipulation of a plane
//

// ---------------------------------------------------
//	CPlane class
// ---------------------------------------------------

class CPlane
{
public:
	union
	{
		struct
		{
			CVector3f		Normal;				// Unit normal vector
			float32			Distance;			// Negated dot product of point on plane, and normal
		};
		struct {float32 e[4];};
	};

	// C'tors
	CPlane( bool ) : Normal( false ) {};
	CPlane() : Normal( CVector3f::gm_YAxis ), Distance( 0.0f ) {};

	CPlane( const CVector3f& norm ) { Normal = norm; Distance = 0.0f; }
	
	//	Set from normal, negated dot product of point on plane and normal.
	CPlane( const CVector3f& norm, float32 dist ) { Normal = norm; Distance = dist; }

	// Set from normal, position on plane
	CPlane( const CVector3f& norm, const CVector3f& position ) { Normal = norm; Distance = -norm.DotProduct( position ); }

	// Compute the plane data from 3 points in space
	void __fastcall	SetFromVerts			(const CVector3f& V1, const CVector3f& V2, const CVector3f& V3);

	// Compute various plane/matrix transformations
	void __fastcall	Rotate					(const CMatrix4f& xform, CPlane& Out) const;
	void __fastcall	Transform				(const CMatrix4f& xform, CPlane& Out) const;
	void __fastcall	TransformRenorm			(const CMatrix4f& xform, CPlane& Out) const;

	// ComputePlaneTransform will generate a matrix that will take you INTO plane space
	// plane space is defined as basis X = Undefined, Y = Normal, Z = Undefined
	// with Translation = Normal * -Distance
	// X and Z are not guranteed to be anything specific, however
	// the matrix is Guaranteed to be orthonormal
	void __fastcall	ComputePlaneTransform(CMatrix4f& Out) const;

	// Positive distance means you're inside, negative outside
	__forceinline float32 __fastcall PointDistance(const CVector3f& Point) const
	{
		return Point.DotProduct( Normal ) + Distance;
	}

	// Snap the plane distance to whole Epsilon units
	void __fastcall Snap(float32 Epsilon);

	// Misc.
	bool	__fastcall	IsEqual		(const CPlane& B) const;
	bool	__fastcall	IsOpposite	(const CPlane& B) const;
	bool	__fastcall	Compare		(const CPlane& plane2, float32 NEpsilon, float32 DEpsilon) const;
	float32	__fastcall	Normalize( );
	void	__fastcall	Inverse( );

	__forceinline bool __fastcall IsInside ( const CVector3f& Point ) const;

	// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
	bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;

	// D3DX support
	__forceinline explicit CPlane(const D3DXPLANE& D3DXPlane);
	__forceinline CPlane& __fastcall operator=(const D3DXPLANE& rhs);
	__forceinline D3DXPLANE * __fastcall GetD3DXPlanePtr();
};

#include "Plane.inl"


} // namespace Math
} // namespace WT

#pragma warning( pop )
