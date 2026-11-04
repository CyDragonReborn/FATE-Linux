//
//	Geometry.cpp
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include <set>
#include "WT/Math/Utils/Geometry2D.h"
#include "WT/Math/Utils/MathUtil.h"
#include "private/predicates.h"

namespace WT
{
namespace Math
{
	
WT_DEFINE_CLASS( CConvexPolytope2D )

float64 ComputeOrientation_Precision( CVector2f const& V1, CVector2f const& V2, CVector2f const& V3 )
{
	assert( CheckPrecision() );
	float64 pa[2] = {V1.x, V1.y,};
	float64 pb[2] = {V2.x, V2.y,};
	float64 pc[2] = {V3.x, V3.y,};
	return orient2d( pa, pb, pc );
}

float64 WTCall ComputeOrientation( CVector2f const& V1, CVector2f const& V2, CVector2f const& V3 )
{
	CPrecisionOverride PrecisionOverride;
	return ComputeOrientation_Precision( V1, V2, V3 );
}

CConvexPolytope2D::CConvexPolytope2D()
{
	ExactInit();
}

void CConvexPolytope2D::IntersectHalfspace( CVector2f const& LineNormal, float32 LineDist )
{
	int32 const VertCount = static_cast<int32>( m_Verts.size() );

	if (!VertCount)
	{
		return;
	}

	if (VertCount == 1)
	{
		if (LineNormal.DotProduct( m_Verts[0] ) + LineDist > 0.0f)
		{
			m_Verts.clear();
		}
		return;
	}

	std::vector<CVector2f> Result;
	Result.reserve( VertCount + 1 );

	for (int32 i = 0; i < VertCount; ++i)
	{
		CVector2f const& CurVert = m_Verts[i];
		CVector2f const& NextVert = m_Verts[(i + 1) % VertCount];

		float32 CurDist = LineNormal.DotProduct( CurVert ) + LineDist;
		float32 NextDist = LineNormal.DotProduct( NextVert ) + LineDist;
		
		if (CurDist <= 0.0f)
		{
			Result.push_back( CurVert );
		}

		if (CurDist * NextDist < 0.0f)
		{
			//Add a point where the line splits the polytope
			if (VertCount != 2 || i != 1)
			{
				//If this is a line segment, don't add the split the second time
				CVector2f const Split = CurVert + (NextVert - CurVert) * (CurDist / (CurDist - NextDist));
				Result.push_back( Split );
			}
		}
	}

	m_Verts = Result;
}

void CConvexPolytope2D::Intersect( CConvexPolytope2D const& Poly )
{
	int32 const PolyVertexCount = static_cast<int32>( Poly.m_Verts.size() );

	//TODO optimize these cases
	switch (PolyVertexCount)
	{
	case 0:
		Clear();
		return;

	case 1:
		IntersectHalfspace( CVector2f::gm_NegXAxis	, +Poly.m_Verts[0].x );
		IntersectHalfspace( CVector2f::gm_XAxis		, -Poly.m_Verts[0].x );
		IntersectHalfspace( CVector2f::gm_NegYAxis	, +Poly.m_Verts[0].y );
		IntersectHalfspace( CVector2f::gm_YAxis		, -Poly.m_Verts[0].y );
		return;

	case 2:
		{
			CVector2f EdgeDir( Poly.m_Verts[1] - Poly.m_Verts[0] );
			EdgeDir.Normalize();
			IntersectHalfspace(  EdgeDir, -EdgeDir.DotProduct( Poly.m_Verts[1] ) );
			IntersectHalfspace( -EdgeDir, +EdgeDir.DotProduct( Poly.m_Verts[0] ) );
			CVector2f EdgeNormal( EdgeDir.y, -EdgeDir.x );
			IntersectHalfspace(  EdgeNormal, -EdgeNormal.DotProduct( Poly.m_Verts[0] ) );
			IntersectHalfspace( -EdgeNormal, +EdgeNormal.DotProduct( Poly.m_Verts[0] ) );
			return;
		}
	}

	for (int32 i = 0; i < PolyVertexCount; ++i)
	{
		CVector2f const& Vertex1 = Poly.m_Verts[i];
		CVector2f const& Vertex2 = Poly.m_Verts[(i + 1) % PolyVertexCount];

		CVector2f EdgeNormal( false );
		EdgeNormal.x = Vertex2.y - Vertex1.y;
		EdgeNormal.y = Vertex1.x - Vertex2.x;
		EdgeNormal.Normalize();

		IntersectHalfspace( EdgeNormal, -EdgeNormal.DotProduct( Vertex1 ) );
	}
}

float32 CConvexPolytope2D::ComputeArea() const
{
	int32 const VertexCount = static_cast<int32>( m_Verts.size() );
	if (VertexCount < 3)
	{
		return 0.0f;
	}

	CVector2f Centroid;
	for (int32 i = 0; i < VertexCount; ++i)
	{
		Centroid += m_Verts[i];
	}
	Centroid /= static_cast<float32>( VertexCount );

	float32 Area = 0.0f;
	for (int32 i = 0; i < VertexCount; ++i)
	{
		CVector2f const& Vertex = m_Verts[i];
		CVector2f const& Next = m_Verts[( i + 1) % VertexCount];

		float32 const a = Vertex.DistanceBetween( Next );
		float32 const b = Next.DistanceBetween( Centroid );
		float32 const c = Centroid.DistanceBetween( Vertex );
		float32 const s = (a + b + c) * 0.5f;
		assert( a >= 0.0f );
		assert( b >= 0.0f );
		assert( c >= 0.0f );
		assert( s >= 0.0f );

		if (s >= a && s >= b && s >= c) //Watch out for numerical error
		{
			//Compute triangle area.
			float32 const Radicand = s * (s - a) * (s - b) * (s - c);
			assert( Radicand >= 0.0f );
			float32 const TriangleArea = sqrtf( Radicand );

			Area += TriangleArea;
		}
	}

	return Area;
}

bool IsVector2fLesser( CVector2f const& V1, CVector2f const& V2 )
{
	if (V1.x < V2.x) return true;
	if (V1.x > V2.x) return false;
	return V1.y < V2.y;
}

void WTCall GetUniquePoints( std::vector<CVector2f> const& Points, std::vector<CVector2f>& Result )
{
	Result = Points;
	std::sort( Result.begin(), Result.end(), IsVector2fLesser );
	Result.erase( std::unique( Result.begin(), Result.end() ), Result.end() );
}

void CConvexPolytope2D::ComputeConvexHull( std::vector<CVector2f> const& Points )
{
	//TODO use heuristic to eliminate points
  
	CPrecisionOverride PrecisionOverride;

	int32 const PointCount = static_cast<int32>( Points.size() );
	for (int32 i = 0; i < PointCount; ++i)
	{
		ExtendToEnclose_Precision( Points[i] );
	}
}

void CConvexPolytope2D::ExtendToEnclose( CVector2f const& Point )
{
	CPrecisionOverride PrecisionOverride;
	ExtendToEnclose_Precision( Point );
}

void CConvexPolytope2D::ExtendToEnclose_Precision( CVector2f const& Point )
{
	int32 VertexCount = static_cast<int32>( m_Verts.size() );

	if (VertexCount == 0)
	{
		m_Verts.push_back( Point );
		return;
	}
	
	if (VertexCount == 1)
	{
		if (m_Verts[0] != Point)
		{
			m_Verts.push_back( Point );
		}
		return;
	}

	if (VertexCount == 2)
	{
		float32 Orientation = static_cast<float32>( ComputeOrientation_Precision( m_Verts[0], m_Verts[1], Point ) );

		if (Orientation > 0.0)
		{
			m_Verts.push_back( Point );
		} else if (Orientation < 0.0) {
			m_Verts.insert( m_Verts.begin() + 1, Point );
		} else {
			//Three collinear points
			float32 const Dot = (Point - m_Verts[0]).DotProduct( m_Verts[1] - m_Verts[0] );
			if (Dot < 0.0f)
			{
				m_Verts[0] = Point;
			} else if (Dot > (m_Verts[1] - m_Verts[0]).LengthSquared()) {
				m_Verts[1] = Point;
			}
		}
		return;
	}

	int32 InsertionPoint = -1;

	bool bPrevEdgeVisible = ComputeOrientation_Precision( *m_Verts.rbegin(), m_Verts[0], Point ) < 0.0;
	for (int32 i = 0; i < VertexCount;)
	{
		bool const bNextEdgeVisible = ComputeOrientation_Precision( m_Verts[i], m_Verts[(i + 1) % VertexCount],
			Point ) < 0.0;

		if (bPrevEdgeVisible) 
		{
			//This is an acceptable location to insert the new point at.
			InsertionPoint = i;

			if (bNextEdgeVisible)
			{
				//More than one edge visible; remove this point.
				m_Verts.erase( m_Verts.begin() + i );
				--VertexCount;

				if (i >= VertexCount)
				{
					break;
				}

				bPrevEdgeVisible = ComputeOrientation_Precision( 
					m_Verts[(i + VertexCount - 1) % VertexCount], m_Verts[i], Point ) < 0.0;
				continue;
			}
		}

		bPrevEdgeVisible = bNextEdgeVisible;

		++i;

		//TODO after edges cease to be visible, we can stop checking. However, we
		//would need to worry about the case where the visible edges wrap around the range 
		//of indices [0, Verts.size() - 1].
	}

	if (InsertionPoint != -1)
	{
		m_Verts.insert( m_Verts.begin() + InsertionPoint, Point );
		++VertexCount;

		//Check for collinear edges
		int32 const PrevPrev	= (InsertionPoint + VertexCount - 2) % VertexCount;
		int32 const Prev		= (InsertionPoint + VertexCount - 1) % VertexCount;
		if (!ComputeOrientation_Precision( m_Verts[PrevPrev], m_Verts[Prev], m_Verts[InsertionPoint] ))
		{
			if (InsertionPoint >= Prev)
			{
				--InsertionPoint;
			}
			m_Verts.erase( m_Verts.begin() + Prev );
			--VertexCount;
		}

		int32 const Next		= (InsertionPoint + 1) % VertexCount;
		int32 const NextNext	= (InsertionPoint + 2) % VertexCount;
		if (!ComputeOrientation_Precision( m_Verts[InsertionPoint], m_Verts[Next], m_Verts[NextNext] ))
		{
			m_Verts.erase( m_Verts.begin() + Next );
			--VertexCount;
		}
	}

#ifndef NDEBUG
	assert( m_Verts.size() >= 3 );
	for (int32 i = 0; i < VertexCount; ++i)
	{
		assert( ComputeOrientation_Precision( m_Verts[i], m_Verts[(i + 1) % VertexCount],
			m_Verts[(i + 2) % VertexCount] ) > 0.0f );
	}
#endif
}

} // namespace Math
} // namespace WT
