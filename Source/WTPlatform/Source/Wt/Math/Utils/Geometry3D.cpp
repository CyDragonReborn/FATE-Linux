//
//	Geometry.cpp
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Utils/Geometry3D.h"
#include "WT/Math/Utils/MathUtil.h"
#include "private/predicates.h"

namespace WT
{
namespace Math
{
	
WT_DEFINE_CLASS( CConvexPolytope3D )

float64 ComputeOrientation_Precision( CVector3f const& V1, CVector3f const& V2, CVector3f const& V3, CVector3f const& V4 )
{
	assert( CheckPrecision() );

	float64 pa[3] = {V1.x, V1.y, V1.z,};
	float64 pb[3] = {V2.x, V2.y, V2.z,};
	float64 pc[3] = {V3.x, V3.y, V3.z,};
	float64 pd[3] = {V4.x, V4.y, V4.z,};

	//Use a different order; orient3d() gives us the opposite of the sign we want
	return orient3d( pd, pb, pc, pa );
}

float64 WTCall ComputeOrientation( CVector3f const& V1, CVector3f const& V2, CVector3f const& V3, CVector3f const& V4 )
{
	CPrecisionOverride PrecisionOverride;
	return ComputeOrientation_Precision( V1, V2, V3, V4 );
}

bool IsCollinear_Precision( CVector3f const& V1, CVector3f const& V2, CVector3f const& V3 )
{
	assert( CheckPrecision() );

	float64 pa[3] = {V1.x, V1.y, V1.z,};
	float64 pb[3] = {V2.x, V2.y, V2.z,};
	float64 pc[3] = {V3.x, V3.y, V3.z,};
	float64 pd[3] = {V3.x, V3.y, V3.z,};
	
	pd[0] += 1.0;
	if (orient3d( pd, pb, pc, pa )) return false;
	pd[1] += 1.0;
	if (orient3d( pd, pb, pc, pa )) return false;
	pd[2] += 1.0;
	if (orient3d( pd, pb, pc, pa )) return false;

	return true;
}

bool IsVector3fLesser( CVector3f const& V1, CVector3f const& V2 )
{
	if (V1.x < V2.x) return true;
	if (V1.x > V2.x) return false;
	if (V1.y < V2.y) return true;
	if (V1.y > V2.y) return false;
	return V1.z < V2.z;
}

void WTCall GetUniquePoints( std::vector<CVector3f> const& Points, std::vector<CVector3f>& Result )
{
	Result = Points;
	std::sort( Result.begin(), Result.end(), IsVector3fLesser );
	Result.erase( std::unique( Result.begin(), Result.end() ), Result.end() );
}

CConvexPolytope3D::CConvexPolytope3D()
{
	ExactInit();
}

void CConvexPolytope3D::Intersect( CConvexPolytope3D const& Poly )
{
	int32 const FaceCount = static_cast<int32>( Poly.m_Faces.size() );
	for (int32 i = 0; i < FaceCount; ++i)
	{
		CPlane const Plane = Poly.GetFacePlane( i );
		if (Plane.Normal != CVector3f::gm_Zero)
		{
			IntersectHalfspace( Plane );
		}
	}
}

CPlane CConvexPolytope3D::GetFacePlane( int32 FaceIndex ) const
{
	if (FaceIndex < 0 || FaceIndex >= static_cast<int32>( m_Faces.size() ))
	{
		throw std::invalid_argument( "FaceIndex" );
	}

	Face const *const pFace = m_Faces[FaceIndex];

	assert( !pFace->Edges.empty() );
	Edge const *const pEdge = *pFace->Edges.begin();
	assert( pEdge != NULL );

	CVector3f const Normal = pFace->ComputeNormal( m_Verts );
	return CPlane( Normal, m_Verts[ pEdge->GetVertex( 0 )].GetPosition() );
}

/*virtual*/ void CConvexPolytope3D::IntersectHalfspace( CPlane const& Plane )
{
	if (m_Faces.empty())
	{
		if (m_Verts.empty())
		{
			return;
		}

		if (m_Verts.size() == 1)
		{
			if (!Plane.IsInside( m_Verts[0].GetPosition() ))
			{
				m_Verts.clear();
			}
			return;
		}

		std::vector<CVertex> Result;

		int32 VertexCount = static_cast<int32>( m_Verts.size() );
		for (int32 i = 0; i < VertexCount; ++i)
		{
			CVector3f const& CurVert = m_Verts[i].GetPosition();
			CVector3f const& NextVert = m_Verts[(i + 1) % VertexCount].GetPosition();

			float32 CurDist = Plane.PointDistance( CurVert );
			float32 NextDist = Plane.PointDistance( NextVert );
			
			if (CurDist <= 0.0f)
			{
				Result.push_back( CVertex( CurVert ) );
			}

			if (CurDist * NextDist < 0.0f)
			{
				//Add a point where the plane splits the polygon
				if (VertexCount != 2 || i != 1)
				{
					//If this is a line segment, don't add the split the second time
					CVector3f const Split = CurVert + (NextVert - CurVert) * (CurDist / (CurDist - NextDist));
					Result.push_back( CVertex( Split ) );
				}
			}
		}

		m_Verts = Result;
		return;
	}	

	int32 const InitialVertexCount = static_cast<int32>( m_Verts.size() );

	int32 InsideCount = 0;

	//Find the post-intersection vertices
	std::vector<CVector3f> NewVerts;
	for (int32 i = 0; i < InitialVertexCount; ++i)
	{
		CVector3f const& Vertex = m_Verts[i].GetPosition();
		bool const Inside = Plane.PointDistance( Vertex ) <= 0.0f;
		if (Inside)
		{
			NewVerts.push_back( Vertex );
			++InsideCount;
		}
	}

	if (InsideCount == InitialVertexCount)
	{
		return;
	}

	if (InsideCount == 0)
	{
		Clear();
		return;
	}

	std::vector<Edge*>::const_iterator const EdgesEnd = m_Edges.end();
	for (std::vector<Edge*>::const_iterator EdgeIt = m_Edges.begin(); EdgeIt != EdgesEnd; ++EdgeIt)
	{
		Edge const* pE = *EdgeIt;

		float32 const Dist1 = Plane.PointDistance( m_Verts[pE->GetVertex( 0 )].GetPosition() );
		float32 const Dist2 = Plane.PointDistance( m_Verts[pE->GetVertex( 1 )].GetPosition() );

		if (Dist1 * Dist2 >= 0.0f)
		{
			continue;
		}

		NewVerts.push_back( m_Verts[pE->GetVertex( 0 )].GetPosition() + 
			(m_Verts[pE->GetVertex( 1 )].GetPosition() - m_Verts[pE->GetVertex( 0 )].GetPosition()) * 
			(Dist1 / (Dist1 - Dist2)) );
	}

	ComputeConvexHull( NewVerts );
}

void CConvexPolytope3D::Edge::AddToFace( int32 f )
{
	Face* const pFace = Faces[f];
	assert( pFace != NULL );
	pFace->Edges.push_back( this );
}

void CConvexPolytope3D::Edge::RemoveFromFace( int32 f )
{
	Face* const pFace = Faces[f];
	assert( pFace != NULL );

	std::vector<Edge*>::iterator it = std::find( pFace->Edges.begin(), 
		pFace->Edges.end(), this );
	assert( it != pFace->Edges.end() );
	pFace->Edges.erase( it );
//	pFace->Dirty = true;
}

void CConvexPolytope3D::Edge::AddToFaces()
{
	assert( Faces[0] != Faces[1] );
	AddToFace( 0 );
	AddToFace( 1 );
}

void CConvexPolytope3D::Edge::RemoveFromFaces()
{
	RemoveFromFace( 0 );
	RemoveFromFace( 1 );
}

void CConvexPolytope3D::Face::Update_Precision_Aux( std::vector<CVertex> const& P ) const
{
	assert( CheckPrecision() );
	assert( Dirty );
	assert( static_cast<int32>( Edges.size() ) >= 3 );

	Edge const* pE = Edges[0];

	int32 f = pE->Faces[0] == this ? 0 : 1;
	assert( pE->Faces[f] == this );
	assert( pE->Faces[f ^ 1] != this );

	int32 const Index1 = pE->GetVertex( f );
	int32 const Index2 = pE->GetVertex( f ^ 1 );
	int32 Index3;
	assert( Index1 != Index2 );	

	CVertex const *const pVerts = &P[0];

	int32 i = 1;
	for (;; ++i)
	{
		assert( i < static_cast<int32>( Edges.size() ) ); //break should have occured before this can fail

		pE = Edges[i];

		int32 f = pE->Faces[0] == this ? 0 : 1;
		assert( pE->Faces[f] == this );
		assert( pE->Faces[f ^ 1] != this );

		Index3 = pE->GetVertex( f );
		
		if (Index3 == Index2)
		{
			continue;
		}

		assert( Index3 != Index1 );

		//Check if three indices are collinear verts
		if (!IsCollinear_Precision( pVerts[Index1].GetPosition(), 
			pVerts[Index2].GetPosition(), pVerts[Index3].GetPosition() ))
		{
			break;
		}
	}

	const_cast<CVector3f&>( m_TriPos0 ) = pVerts[Index1].GetPosition();
	const_cast<CVector3f&>( m_TriPos1 ) = pVerts[Index2].GetPosition();
	const_cast<CVector3f&>( m_TriPos2 ) = pVerts[Index3].GetPosition();
	Dirty = false;
}

float64 CConvexPolytope3D::Face::ComputeOrientation_Precision( CVector3f const& Point, std::vector<CVertex> const& P ) const
{
	assert( CheckPrecision() );
	Update_Precision( P );

	return Math::ComputeOrientation_Precision( m_TriPos0, m_TriPos1, m_TriPos2, Point );
}

float64 CConvexPolytope3D::Face::ComputeOrientation( CVector3f const& Point, std::vector<CVertex> const& P ) const
{
	CPrecisionOverride PrecisionOverride;
	return ComputeOrientation_Precision( Point, P );
}

//This computes the normal of a face. It shouldn't be used for visibility tests because it uses
//inexact arithmetic; use IsVisible() instead. This function works by checking the triangles that
//make up the face (similar to representing the face as a tri-strip). The contributions from each
//triangle to the normal are summed.
CVector3f CConvexPolytope3D::Face::ComputeNormal( std::vector<CVertex> const& P ) const
{
	std::map<int32, int32> Next;
	std::map<int32, int32> Prev;

	int32 const EdgeCount = static_cast<int32>( Edges.size() );
	assert( EdgeCount >= 3 );
	for (int32 i = 0; i < EdgeCount; ++i)
	{
		Edge const* pE = Edges[i];

		int32 f = pE->Faces[0] == this ? 0 : 1;
		assert( pE->Faces[f] == this );
		assert( pE->Faces[f ^ 1] != this );

		Next[pE->GetVertex( f     )] = pE->GetVertex( f ^ 1 );
		Prev[pE->GetVertex( f ^ 1 )] = pE->GetVertex( f     );
	}

	float64 Nx = 0.0;
	float64 Ny = 0.0;
	float64 Nz = 0.0;
	int32 Index1 = Edges[0]->GetVertex( 0 );
	int32 Index2 = Next[Index1];
	int32 Index3 = Prev[Index1];
	for (int32 i = 2; i < EdgeCount; ++i)
	{
		assert( Index1 != Index2 );
		assert( Index2 != Index3 );
		assert( Index3 != Index1 );

		CVector3f const& A = P[Index1].GetPosition();
		CVector3f const& B = P[Index2].GetPosition();
		CVector3f const& C = P[Index3].GetPosition();

		float64 Bx,By,Bz,Cx,Cy,Cz;
		Bx = static_cast<float64>( B.x ) - static_cast<float64>( A.x );
		By = static_cast<float64>( B.y ) - static_cast<float64>( A.y );
		Bz = static_cast<float64>( B.z ) - static_cast<float64>( A.z );
		Cx = static_cast<float64>( C.x ) - static_cast<float64>( A.x );
		Cy = static_cast<float64>( C.y ) - static_cast<float64>( A.y );
		Cz = static_cast<float64>( C.z ) - static_cast<float64>( A.z );

		float64 dNx = By * Cz - Bz * Cy;
		float64 dNy = Bz * Cx - Bx * Cz;
		float64 dNz = Bx * Cy - By * Cx;

		int32 NewIndex;
		if (i & 1)
		{
			Nx -= dNx;
			Ny -= dNy;
			Nz -= dNz;
			NewIndex = Prev[Index2];
		} else {
			Nx += dNx;
			Ny += dNy;
			Nz += dNz;
			NewIndex = Next[Index2];
		}

		Index1 = Index2;
		Index2 = Index3;
		Index3 = NewIndex;
	}

	//Normalize the result
	float64 const Ax = fabs( Nx );
	float64 const Ay = fabs( Ny );
	float64 const Az = fabs( Nz );
	float64 Mag;

	if (Ax > Ay && Ax > Az )
	{
		float64 const y = Ny / Nx;
		float64 const z = Nz / Nx;
		Mag = Ax * sqrt( 1.0 + y * y + z * z );
	} else if (Ay > Az) {
		float64 const z = Nz / Ny;
		float64 const x = Nx / Ny;
		Mag = Ay * sqrt( 1.0 + z * z + x * x );
	} else if (Az) {
		float64 const x = Nx / Nz;
		float64 const y = Ny / Nz;
		Mag = Az * sqrt( 1.0 + x * x + y * y );
	} else {
		Mag = 1.0;
	}
	Mag = 1.0 / Mag;

	return CVector3f( 
		static_cast<float32>( Nx * Mag ),
		static_cast<float32>( Ny * Mag ),
		static_cast<float32>( Nz * Mag ) );
}

void CConvexPolytope3D::Clear()
{
	m_Verts.clear();

	std::vector<Face*>::const_iterator const FacesEnd = m_Faces.end();
	for (std::vector<Face*>::const_iterator FaceIt = m_Faces.begin(); FaceIt != FacesEnd; ++FaceIt)
	{
		Face const* pFace = *FaceIt;
		delete pFace;
	}
	m_Faces.clear();

	std::vector<Edge*>::const_iterator const EdgesEnd = m_Edges.end();
	for (std::vector<Edge*>::const_iterator EdgeIt = m_Edges.begin(); EdgeIt != EdgesEnd; ++EdgeIt)
	{
		Edge const* pE = *EdgeIt;
		delete pE;
	}
	m_Edges.clear();
}

void CConvexPolytope3D::ExtendToEnclose( CVector3f const& Point )
{
	CPrecisionOverride PrecisionOverride;
	ExtendToEnclose_Precision( Point );
}

void CConvexPolytope3D::GetPerpPoint_Precision( CVector3f& PerpPoint, float32& OrientationSign ) const
{
	assert( CheckPrecision() );

	float64 Orientation = 0.0;
	for (;;)
	{
		CVertex const* const pVerts = &m_Verts[0];

		PerpPoint = CVector3f::gm_Zero;
		Orientation = ComputeOrientation_Precision( pVerts[0].GetPosition(), pVerts[1].GetPosition(), 
			pVerts[2].GetPosition(), PerpPoint) ;
		if (Orientation) break;

		PerpPoint = CVector3f::gm_XAxis;
		Orientation = ComputeOrientation_Precision( pVerts[0].GetPosition(), pVerts[1].GetPosition(), 
			pVerts[2].GetPosition(), PerpPoint) ;
		if (Orientation) break;

		PerpPoint = CVector3f::gm_YAxis;
		Orientation = ComputeOrientation_Precision( pVerts[0].GetPosition(), pVerts[1].GetPosition(), 
			pVerts[2].GetPosition(), PerpPoint) ;
		if (Orientation) break;

		PerpPoint = CVector3f::gm_ZAxis;
		Orientation = ComputeOrientation_Precision( pVerts[0].GetPosition(), pVerts[1].GetPosition(), 
			pVerts[2].GetPosition(), PerpPoint) ;
		assert( Orientation );
		break;
	}

	if (Orientation > 0.0)
	{
		OrientationSign = 1.0f;
	} else {
		OrientationSign = -1.0f;
	}
}

void CConvexPolytope3D::ExtendPolygonToEnclose( CVector3f const& Point )
{
	CPrecisionOverride PrecisionOverride;
	ExtendPolygonToEnclose_Precision( Point );
}

void CConvexPolytope3D::ExtendPolygonToEnclose_Precision( CVector3f const& Point )
{
	assert( CheckPrecision() );

	if (!m_Faces.empty())
	{
		throw std::runtime_error( "CConvexPolytope3D::ExtendPolygonToEnclose called on polyhedron" );
	}

	int32 const VertCount = static_cast<int32>( m_Verts.size() );

	if (VertCount <= 2)
	{
		if (VertCount == 0)
		{
			m_Verts.push_back( CVertex( Point ) );
			return;
		}
		
		if (VertCount == 1)
		{
			if (m_Verts[0].GetPosition() != Point)
			{
				m_Verts.push_back( CVertex( Point ) );
			}
			return;
		}

		assert( VertCount == 2 );

		if (IsCollinear_Precision( m_Verts[0].GetPosition(), m_Verts[1].GetPosition(), Point ))
		{
			//Three collinear points
			float32 const Dot = (Point - m_Verts[0].GetPosition()).DotProduct( m_Verts[1].GetPosition() - m_Verts[0].GetPosition() );
			if (Dot < 0.0f)
			{
				m_Verts[0].SetPosition( Point );
			} else if (Dot > (m_Verts[1].GetPosition() - m_Verts[0].GetPosition()).LengthSquared()) {
				m_Verts[1].SetPosition( Point );
			}
		} else {
			m_Verts.push_back( CVertex( Point ) );
		}

		return;
	}

	CVector3f PerpPoint( false ); //A point outside of the plane
	float32 OrientationSign;
	GetPerpPoint_Precision( PerpPoint, OrientationSign );
	
	bool bPrevEdgeVisible = OrientationSign * 
		ComputeOrientation_Precision( m_Verts[m_Verts.size() - 1].GetPosition(), m_Verts[0].GetPosition(), 
		Point, PerpPoint ) < 0.0;

	int InsertionPoint = -1;
	for (int32 j = 0; j < static_cast<int32>( m_Verts.size() );)
	{
		bool const bNextEdgeVisible = OrientationSign * 
			ComputeOrientation_Precision( m_Verts[j].GetPosition(), m_Verts[(j + 1) % m_Verts.size()].GetPosition(),
			Point, PerpPoint ) < 0.0;

		if (bPrevEdgeVisible) 
		{
			//This is an acceptable location to insert the new point at, whether there is only
			//one edge visible or more than one.
			InsertionPoint = j;

			if (bNextEdgeVisible)
			{
				//More than one edge visible; remove this point.
				assert( m_Verts.size() >= 3 ); //This shouldn't yield a degenerate polygon
				m_Verts.erase( m_Verts.begin() + j );

				if (j >= static_cast<int32>( m_Verts.size() ))
				{
					break;
				}

				bPrevEdgeVisible = OrientationSign * 
					ComputeOrientation_Precision( m_Verts[(j + m_Verts.size() - 1) % m_Verts.size()].GetPosition(), 
					m_Verts[j].GetPosition(), Point, PerpPoint ) < 0.0;
				continue;
			}
		}

		bPrevEdgeVisible = bNextEdgeVisible;

		++j;

		//TODO after edges cease to be visible, we can stop checking. However, we
		//would need to worry about the case where the visible edges wrap around the range 
		//of indices [0, Verts.size() - 1].
	}

	if (InsertionPoint != -1)
	{
		m_Verts.insert( m_Verts.begin() + InsertionPoint, CVertex( Point ) );

		int32 VertexCount = static_cast<int32>( m_Verts.size() );

		//Check for collinear edges
		int32 const PrevPrev	= (InsertionPoint + VertexCount - 2) % VertexCount;
		int32 const Prev		= (InsertionPoint + VertexCount - 1) % VertexCount;
		if (IsCollinear_Precision( m_Verts[PrevPrev].GetPosition(), m_Verts[Prev].GetPosition(), 
			m_Verts[InsertionPoint].GetPosition() ))
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
		if (IsCollinear_Precision( m_Verts[InsertionPoint].GetPosition(), 
			m_Verts[Next].GetPosition(), m_Verts[NextNext].GetPosition() ))
		{
			m_Verts.erase( m_Verts.begin() + Next );
			--VertexCount;
		}
	}

#ifndef NDEBUG
	{
		for (int32 i = 0; i < static_cast<int32>( m_Verts.size() ); ++i)
		{
			float64 CurOrientation = ComputeOrientation_Precision( m_Verts[i].GetPosition(), 
				m_Verts[(i + 1) % m_Verts.size()].GetPosition(), 
				m_Verts[(i + 2) % m_Verts.size()].GetPosition(),
				PerpPoint );
			assert( OrientationSign * CurOrientation > 0.0 );
		}
	}
#endif
}

void CConvexPolytope3D::ExtendToEnclose_Precision( CVector3f const& Point )
{
	assert( CheckPrecision() );

	if (m_Faces.empty())
	{
		if (m_Verts.size() <= 2)
		{
			ExtendPolygonToEnclose_Precision( Point );
			return;
		}

		float32 Orientation = static_cast<float32>( 
			ComputeOrientation_Precision( m_Verts[0].GetPosition(), m_Verts[1].GetPosition(), 
			m_Verts[2].GetPosition(), Point ) );

		if (!Orientation)
		{
			ExtendPolygonToEnclose_Precision( Point );
		} else {
			int32 f = (Orientation > 0.0f);

			m_Verts.push_back( CVertex( Point ) );

			int32 VertexCount = static_cast<int32>( m_Verts.size() );
			int32 const BaseVertexCount = VertexCount - 1;
			int32 const ApexIndex = BaseVertexCount;

			m_Faces.resize( VertexCount );
			for (int32 i = 0; i < VertexCount; ++i)
			{
				m_Faces[i] = new Face();
			}
			Face *const pBaseFace = m_Faces[BaseVertexCount];

			//Add two edges for each pyramid base edge
			for (int32 i = 0; i < BaseVertexCount; ++i)
			{
				Edge* pE1 = new Edge( (i + 1) % BaseVertexCount, i );
				m_Edges.push_back( pE1 );
				pE1->Faces[f    ] = m_Faces[i];
				pE1->Faces[1 ^ f] = pBaseFace;
				pE1->AddToFaces();
				
				Edge* pE2 = new Edge( i, ApexIndex );
				m_Edges.push_back( pE2 );
				pE2->Faces[f    ] = m_Faces[i];
				pE2->Faces[1 ^ f] = m_Faces[(i + BaseVertexCount - 1) % BaseVertexCount];
				pE2->AddToFaces();
			}
		}

		return;
	}

	bool bAnyVisible = false;

	int32 FaceCount = static_cast<int32>( m_Faces.size() );
	{
		Face** const pFaces = &m_Faces[0];
		for (int32 FaceIndex = 0; FaceIndex < FaceCount; ++FaceIndex)
		{
			Face* const pFace = pFaces[FaceIndex];

			assert( !pFace->Visible );

			if (pFace->IsVisible_Precision( Point, m_Verts ))
			{
				bAnyVisible = true;

				pFace->Visible = true;
			}
		}
	}

	if (!bAnyVisible)
	{
		return;
	}

	int32 NewVertex = static_cast<int32>( m_Verts.size() );
	m_Verts.push_back( CVertex( Point ) );

	//Check each edge for its faces' visibility
	int32 EdgeCount = static_cast<int32>( m_Edges.size() );
	for (int32 EdgeIndex = 0; EdgeIndex < EdgeCount; ++EdgeIndex)
	{
		Edge* pE = m_Edges[EdgeIndex];

		//Invisible edge?
		if (!pE->Faces[0]->Visible &&
			!pE->Faces[1]->Visible)
		{
			//Ignore this edge.
			continue;
		}

		//Interior edge?
		if (pE->Faces[0]->Visible &&
			pE->Faces[1]->Visible)
		{
			//Delete the edge.
			delete pE;
			m_Edges.erase( m_Edges.begin() + EdgeIndex );
			--EdgeCount;
			--EdgeIndex; //Decrement this; it will be re-incremented after the continue statement
			continue;
		}

		//Sillhouette edge
		//Add two edges and a triangle with the new point as the apex.

		int32 const f = pE->Faces[0]->Visible ? 0 : 1;
		assert( pE->Faces[f]->Visible );
		assert( !pE->Faces[f ^ 1]->Visible );

		Face* pNewFace = new Face();
		pNewFace->Edges.reserve( 3 );
		m_Faces.push_back( pNewFace );

		pE->Faces[f] = pNewFace;
		pE->AddToFace( f );

		Edge* pEdge1 = NULL;
		Edge* pEdge2 = NULL;
		
		//Look through the recently created edges to avoid creating duplicate
		//edges - TODO improve this algorithm
		int32 const NewEdgeCount = static_cast<int32>( m_Edges.size() );
		for (int32 Edge2Index = EdgeCount; Edge2Index < NewEdgeCount; ++Edge2Index)
		{
			Edge* pE2 = m_Edges[Edge2Index];

			//An assertion failure here usually indicates that the visible sillhouette of the polygon
			//(i.e. the collection of edges surrounded by visible and invisible faces) does not consist
			//of a sequence of edges forming a single polygon, as it should, but something more complex 
			//(such as two triangles joined at one vertex). This is an error and can occur if the
			//polyhedron is inconsistent (e.g. with faces facing the wrong way).
			if (pE2->GetVertex( 0 ) == pE->GetVertex( f ))
			{
				assert( pEdge2 == NULL );
				assert( pE2->GetVertex( 1 ) == NewVertex );
				assert( pE2->Faces[0] != NULL );
				assert( pE2->Faces[1] == NULL );
				pEdge2 = pE2;
			} else if (pE2->GetVertex( 1 ) == pE->GetVertex( f ^ 1 )) {
				assert( pEdge1 == NULL );
				assert( pE2->GetVertex( 0 ) == NewVertex );
				assert( pE2->Faces[0] != NULL );
				assert( pE2->Faces[1] == NULL );
				pEdge1 = pE2;
			}
		}

		if (pEdge1 == NULL)
		{
			pEdge1 = new Edge( pE->GetVertex( f ^ 1 ), NewVertex );

			pEdge1->Faces[0] = pNewFace;
			pEdge1->Faces[1] = NULL;
			m_Edges.push_back( pEdge1 );
		} else {
			assert( pEdge1->Faces[1] == NULL );
			assert( pEdge1->Faces[0] != pNewFace );
			pEdge1->Faces[1] = pNewFace;
			pEdge1->AddToFaces();
		}

		if (pEdge2 == NULL)
		{
			pEdge2 = new Edge( NewVertex, pE->GetVertex( f ) );

			pEdge2->Faces[0] = pNewFace;
			pEdge2->Faces[1] = NULL;
			m_Edges.push_back( pEdge2 );
		} else {
			assert( pEdge2->Faces[1] == NULL );
			assert( pEdge2->Faces[0] != pNewFace );
			pEdge2->Faces[1] = pNewFace;
			pEdge2->AddToFaces();
		}
	}

	int32 const OrigFaceCount = FaceCount;

	//Delete visible faces
	Face** pFaces = &m_Faces[0];
	for (int32 FaceIndex = 0; FaceIndex < FaceCount;)
	{
		Face const* pFace = pFaces[FaceIndex];

		if (pFace->Visible)
		{
			delete pFace;
			pFaces[FaceIndex] = pFaces[--FaceCount];
		} else {
			++FaceIndex;
		}
	}
	m_Faces.erase( m_Faces.begin() + FaceCount, m_Faces.begin() + OrigFaceCount );
}

void CConvexPolytope3D::MergeCoplanarFaces_Precision()
{
	assert( CheckPrecision() );

	int32 FaceCount = static_cast<int32>( m_Faces.size() );
	Face** pFaces = &m_Faces[0];

	//Update every face
	for (int32 i = 0; i < FaceCount; ++i)
	{
		pFaces[i]->Update_Precision( m_Verts );
	}

	//Merge coplanar faces
	int32 EdgeCount = static_cast<int32>( m_Edges.size() );
	for (int32 i = 0; i < EdgeCount; ++i)
	{
		Edge* pEdge = m_Edges[i];

		Face* pFace = pEdge->Faces[0];
		Face* pAdjFace = pEdge->Faces[1];
		assert( pAdjFace != pFace );

		//Check vertices of the face and see if there is zero distance with respect to the adjacent face
		CVector3f const& Vertex1 = pAdjFace->m_TriPos0;
		CVector3f const& Vertex2 = pAdjFace->m_TriPos1;
		CVector3f const& Vertex3 = pAdjFace->m_TriPos2;

		if (Math::ComputeOrientation_Precision( Vertex1, Vertex2, Vertex3, pFace->m_TriPos0 )) continue;
		if (Math::ComputeOrientation_Precision( Vertex1, Vertex2, Vertex3, pFace->m_TriPos1 )) continue;
		if (Math::ComputeOrientation_Precision( Vertex1, Vertex2, Vertex3, pFace->m_TriPos2 )) continue;

		//Coplanar - assign each edge to the other poly
		int32 const FaceEdgeCount = static_cast<int32>( pFace->Edges.size() );
		for (int32 k = 0; k < FaceEdgeCount; ++k)
		{
			//Remove first edge from face until no edges are left
			Edge* pE = pFace->Edges[0];

			int32 f = pE->Faces[0] == pFace ? 0 : 1;
			assert( pE->Faces[f] == pFace );
			assert( pE->Faces[f ^ 1] != pFace );

			if (pE->Faces[f ^ 1] == pAdjFace)
			{
				pE->RemoveFromFaces();
				delete pE;
				std::vector<Edge*>::iterator it = std::find( m_Edges.begin(), m_Edges.end(), pE );
				m_Edges.erase( it );
				--EdgeCount;

				if (i >= 0 && m_Edges.begin() + i >= it)
				{
					--i;
				}
			} else {
				pE->RemoveFromFace( f );
				pE->Faces[f] = pAdjFace;
				pE->AddToFace( f );
			}
		}
	}

	for (int32 i = 0; i < FaceCount;)
	{
		Face* pFace = pFaces[i];

		if (pFace->Edges.empty())
		{
			delete pFace;
			pFaces[i] = pFaces[--FaceCount];
		} else {
			++i;
		}
	}
	m_Faces.resize( FaceCount );
}

void CConvexPolytope3D::MergeCollinearEdges()
{
	//Merge collinear edges
	int32 VertexCount = static_cast<int32>( m_Verts.size() );
	std::vector< std::vector<Edge*> > Edges( VertexCount );

	int32 EdgeCount = static_cast<int32>( m_Edges.size() );
	for (int32 i = 0; i < EdgeCount; ++i)
	{
		Edge* pE = m_Edges[i];

		std::vector<Edge*>* pEdges;
		
		pEdges = &Edges[pE->GetVertex( 0 )];
		if (pEdges->size() <= 2)
		{
			pEdges->push_back( pE );
		}

		pEdges = &Edges[pE->GetVertex( 1 )];
		if (pEdges->size() <= 2)
		{
			pEdges->push_back( pE );
		}
	}

	for (int32 i = 0; i < VertexCount; ++i)
	{
		if (Edges[i].size() != 2)
		{
			continue;
		}

		int32 x = Edges[i][0]->GetVertex( 0 ) == i ? 0 : 1;
		int32 y = Edges[i][1]->GetVertex( 0 ) == i ? 0 : 1;

		assert( Edges[i][0]->Faces[x ^ 1] == Edges[i][1]->Faces[y    ] );
		assert( Edges[i][0]->Faces[x    ] == Edges[i][1]->Faces[y ^ 1] );

		int32 NewVertex = Edges[i][1]->GetVertex( y ^ 1 );
		Edges[i][0]->SetVertex( x, NewVertex );

		if (Edges[NewVertex].size() == 2) //If adjacent edge is relevant
		{
			std::vector<Edge*>::iterator it = std::find( Edges[NewVertex].begin(), Edges[NewVertex].end(), Edges[i][1] );
			assert( it != Edges[NewVertex].end() );
			*it = Edges[i][0];
		}

		Edges[i][1]->RemoveFromFaces();
		delete Edges[i][1];
		m_Edges.erase( std::find( m_Edges.begin(), m_Edges.end(), Edges[i][1] ) );
	}
}

void CConvexPolytope3D::RemoveUnusedVertices()
{
	if (m_Faces.empty())
	{
		return;
	}

	//Remove unused vertices
	int32 const OldVertexCount = static_cast<int32>( m_Verts.size() );

	std::vector<bool> Used( OldVertexCount );
	std::fill( Used.begin(), Used.end(), false );

	int32 const EdgeCount = static_cast<int32>( m_Edges.size() );
	for (int32 i = 0; i < EdgeCount; ++i)
	{
		Edge* pE = m_Edges[i];
		Used[pE->GetVertex( 0 )] = true;
		Used[pE->GetVertex( 1 )] = true;
	}

	int32 UsedVertCount = static_cast<int32>( std::count( Used.begin(), Used.end(), true ) );

	std::vector<CVertex> NewVerts( UsedVertCount );
	std::vector<int32> DestIndex( OldVertexCount );
	int32 Dest = 0;
	for (int32 i = 0; i < OldVertexCount; ++i)
	{
		if (Used[i])
		{
			NewVerts[Dest] = m_Verts[i];
			DestIndex[i] = Dest;
			++Dest;
		} else {
			DestIndex[i] = -1;
		}
	}
		
	//Renumber the edges' vertex indices to point to the compacted vertex list
	for (int32 i = 0; i < EdgeCount; ++i)
	{
		Edge* pE = m_Edges[i];
		pE->SetVertex( 0, DestIndex[pE->GetVertex( 0 )] );
		pE->SetVertex( 1, DestIndex[pE->GetVertex( 1 )] );
	}
	
	m_Verts = NewVerts;
}

float32 CConvexPolytope3D::ComputeArea() const
{
	if (!m_Faces.empty())
	{
		throw std::runtime_error( "CConvexPolytope3D::ComputeArea() not implemented for polyhedra" );
	}

	int32 const VertexCount = static_cast<int32>( m_Verts.size() );
	if (VertexCount < 3)
	{
		return 0.0f;
	}

	CVector3f Centroid;
	for (int32 i = 0; i < VertexCount; ++i)
	{
		Centroid += m_Verts[i].GetPosition();
	}
	Centroid /= static_cast<float32>( VertexCount );

	float32 Area = 0.0f;
	for (int32 i = 0; i < VertexCount; ++i)
	{
		CVector3f const& Vertex = m_Verts[i].GetPosition();
		CVector3f const& Next = m_Verts[( i + 1) % VertexCount].GetPosition();

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

void CConvexPolytope3D::ComputeConvexHull( std::vector<CVector3f> const& Points )
{
	//TODO use heuristic to eliminate points

	Clear();

	CPrecisionOverride PrecisionOverride;

	int32 const PointCount = static_cast<int32>( Points.size() );
	for (int32 i = 0; i < PointCount; ++i)
	{
		ExtendToEnclose_Precision( Points[i] );
	}

	MergeCoplanarFaces_Precision();
	MergeCollinearEdges();
	RemoveUnusedVertices();
}

} // namespace Math
} // namespace WT
