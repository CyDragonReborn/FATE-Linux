//
//	Intersections.h
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"

namespace WT
{
namespace Math
{
namespace Intersections
{

namespace NIntersectLineAndPoly
{
	uint32 const KPolyTriangle	= 0 << 0;
	uint32 const KPolyRectangle = 1 << 0;
	uint32 const KPolyMask      = KPolyTriangle | KPolyRectangle;

	uint32 const KDoubleSided	= 1 << 1;

	//Use for a line with parameter T not restricted
	uint32 const KLine          = 0 << 2;

	//Use for a ray or line segment with parameter T restricted by T >= 0
	uint32 const KLineBound0	= 1 << 2;

	//Use for a ray or line segment with parameter T restricted by T <= 1
	uint32 const KLineBound1	= 2 << 2;

	//Use for a ray
	uint32 const KLineRay		= KLineBound0;

	//Use for a line segment
	uint32 const KLineSeg		= KLineBound0 | KLineBound1;

	uint32 const KLineMask		= KLineBound0 | KLineBound1;
};

// Compute the intersection of a line, ray, or line segment and a triangle or rectangle.
// Vert0, Vert1, and Vert2 are three vertices with UV coordinates (0,0), (1,0), and (0,1).
// LinePoint is the line origin, LineDirection is the normalized direction of the line.
// Return true if an intersection happens, and set *pT to the distance on the line 
// (point is LinePoint + *pT * LineDirection) and U,V to the coordinates on the polygon.
template<uint32 Flags> 
	inline bool WTCall IntersectLineAndPoly(
		const CVector3f& LinePoint, const CVector3f& LineDirection,
		const CVector3f& Vert0, const CVector3f& Vert1, const CVector3f& Vert2,
		float32& U, float32& V, float32* pT = NULL );

// Compute the intersection of a line and a sphere. LineDirection must be normalized.
// If Results is not NULL, then up to two values of T (in ascending order) will be added to it;
// the ith intersection occurs at position LinePoint + LineDirection * T. The result of 
// the function is false if the intersection set is empty; otherwise the result is true.
inline bool WTCall IntersectLineAndSphere( 
	const CVector3f& LinePoint, const CVector3f& LineDirection, 
	const CVector3f& Center, float32 RadiusSquared, std::vector<float32>* Results = NULL );

#include "Intersections.inl"

} // namespace Intersections
} // namespace Math
} // namespace WT
