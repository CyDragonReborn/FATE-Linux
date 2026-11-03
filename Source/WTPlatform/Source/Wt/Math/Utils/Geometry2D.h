//
//	Geometry2D.h
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include <vector>
#include "WT/Math/LinearAlgebra/Vector2f.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Plane.h"

namespace WT
{
namespace Math
{

//A CConvexPolytope2D may represent an empty set, an order 0 polytope (point),
//an order 1 polytope (line segment), or an order 2 polytope (polygon). Polygons
//always have positive orientation with a normal vector of (0,0,1).
class CConvexPolytope2D : public System::CObject
{
public:
	WT_CLASS( CConvexPolytope2D, System::CObject )

	CConvexPolytope2D();

	CConvexPolytope2D( CConvexPolytope2D const& Poly )
		: m_Verts( Poly.m_Verts )
	{
	}

	CConvexPolytope2D& operator=( CConvexPolytope2D const& Poly )
	{
		m_Verts = Poly.m_Verts;
		return *this;
	}

	void ComputeConvexHull( std::vector<CVector2f> const& Points );

	//Get a list of unique vertices in the polytope.
	std::vector<CVector2f> const& GetVertices() const
	{
		return m_Verts;
	}

	//Intersect with a halfspace, consisting of all points V satisfying
	//(LineNormal.DotProduct( V ) + LineDist <= 0.0f).
	void IntersectHalfspace( CVector2f const& LineNormal, float32 LineDist );

	void Intersect( CConvexPolytope2D const& Poly );

	void Clear() {m_Verts.clear();}

	float32 ComputeArea() const;

	//Extend the polytope to include the specified point.
	void ExtendToEnclose( CVector2f const& Point );

	//Extend the polytope to include the specified point.
	void ExtendToEnclose_Precision( CVector2f const& Point );

private:
	std::vector<CVector2f> m_Verts;
};

//Compute the orientation of the triangle formed by points V1, V2, and V3. The result is roughly twice
//the signed area of the triangle, and the sign of the result is exactly correct. In cases where numerical 
//error makes the sign difficult to determine, this function resorts to (much slower) exact arithmetic.
float64 WTCall ComputeOrientation( CVector2f const& V1, CVector2f const& V2, CVector2f const& V3 );

//Get the unique points from the input array Points and store the result in Result.
void WTCall GetUniquePoints( std::vector<CVector2f> const& Points, std::vector<CVector2f>& Result );

} // namespace Math
} // namespace WT
