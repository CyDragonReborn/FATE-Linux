//
//	Geometry.h
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

class CConvexPolytope3D : public System::CObject
{
public:
	WT_CLASS( CConvexPolytope3D, System::CObject )

	//Compute the convex hull of the specified points.
	void ComputeConvexHull( std::vector<CVector3f> const& Points );

	class CVertex;
	class Edge;
	class Face;


	class CVertex
	{
	public:
		CVertex() 
			: m_Position( false )
		{
		}

		explicit CVertex( CVector3f const& Position ) 
			: m_Position( Position )
		{
		}

		CVertex( CVertex const& Vertex )
			: m_Position( Vertex.m_Position )
		{
		}

		CVertex& operator=( CVertex const& Vertex )
		{
			m_Position = Vertex.m_Position;
			return *this;
		}

		CVector3f const& GetPosition() const
		{
			return m_Position;
		}

		void SetPosition( CVector3f const& Position )
		{
			m_Position = Position;	
		}

	private:
		CVector3f m_Position;
	};

	class Edge
	{
	public:
		Edge( int32 Vertex0, int32 Vertex1 )
		{
			assert( Vertex0 != Vertex1 );
			m_Vertices[0] = Vertex0;
			m_Vertices[1] = Vertex1;
		}

		int32 GetVertex( int32 i ) const
		{
			assert( i >= 0 );
			assert( i <  2 );
			return m_Vertices[i];
		}

		void SetVertex( int32 i, int32 v )
		{
			assert( i >= 0 );
			assert( i <  2 );
			m_Vertices[i] = v;
		}

		void AddToFace( int32 f );
		void RemoveFromFace( int32 f );

		void AddToFaces();
		void RemoveFromFaces();

		//Verts and Faces, when considered seperately, are unordered; however,
		//the two arrays are correlated.
		//Faces[0] uses the order Verts[0], Verts[1] for its edge; while
		//Faces[1] uses the order Verts[1], Verts[0] for its edge.
		Face* Faces[2];

	private:
		int32 m_Vertices[2];
	};

	class Face
	{
	public:
		Face()
			: Visible( false )
			, Dirty( true )
			, m_TriPos0( false )
			, m_TriPos1( false )
			, m_TriPos2( false )
		{
		}

		std::vector<Edge*> Edges; //Unordered list of edges

		CVector3f m_TriPos0;
		CVector3f m_TriPos1;
		CVector3f m_TriPos2;

		bool Visible : 1; //Visibility flag; this is used during processing.
		mutable bool Dirty : 1;

		CVector3f ComputeNormal( std::vector<CVertex> const& P ) const;

		float64 ComputeOrientation( CVector3f const& Point, std::vector<CVertex> const& P ) const;

		bool IsVisible( CVector3f const& Point, std::vector<CVertex> const& P ) const
		{
			return ComputeOrientation( Point, P ) > 0.0;
		}

		bool IsCoplanar( CVector3f const& Point, std::vector<CVertex> const& P ) const
		{
			return !ComputeOrientation( Point, P );
		}

		//These functions are identical to those above, except that they require a CPrecisionOverride object 
		//to be in effect before they can be safely called.
		float64 ComputeOrientation_Precision( CVector3f const& Point, std::vector<CVertex> const& P ) const;
		bool IsVisible_Precision( CVector3f const& Point, std::vector<CVertex> const& P ) const
		{
			return ComputeOrientation_Precision( Point, P ) > 0.0;
		}

		bool IsCoplanar_Precision( CVector3f const& Point, std::vector<CVertex> const& P ) const
		{
			return !ComputeOrientation_Precision( Point, P );
		}

		//Update m_TriPos and set Dirty to false, if necessary. Requires a CPrecisionOverride 
		//object to be in effect.
		void Update_Precision( std::vector<CVertex> const& P ) const
		{
			if (Dirty)
			{
				Update_Precision_Aux( P );
			}
		}

	private:
		//Dirty must be true. Update TriPos and set Dirty to false. Requires a CPrecisionOverride 
		//object to be in effect.
		void Update_Precision_Aux( std::vector<CVertex> const& P ) const;
	};

	CConvexPolytope3D();

	~CConvexPolytope3D()
	{
		Clear();
	}

	void Clear();

	//Get a list of unique vertices in the polytope.
	void GetVertices( std::vector<CVector3f>& Verts ) const
	{
		int32 const VertexCount = static_cast<int32>( m_Verts.size() );
		for (int32 i = 0; i < VertexCount; ++i)
		{
			Verts.push_back( m_Verts[i].GetPosition() );
		}
	}

	int32 GetFaceCount() const {return static_cast<int32>( m_Faces.size() );}

	CPlane GetFacePlane( int32 FaceIndex ) const;

	void ExtendToEnclose( CVector3f const& Point );
	void ExtendToEnclose_Precision( CVector3f const& Point );

	//Extends a polygon to enclose the specified point, which should be
	//coplanar. Due to numerical imprecision, a skew polygon will likely
	//result from this, which could cause problems later if a polyhedron
	//is produced with operations like ExtendToEnclose().
	void ExtendPolygonToEnclose( CVector3f const& Point );
	void ExtendPolygonToEnclose_Precision( CVector3f const& Point );

	void MergeCoplanarFaces_Precision();
	void MergeCollinearEdges();
	void RemoveUnusedVertices();

	//Intersect with a halfspace, consisting of points inside the plane
	//(non-positive dot product with the normal).
	void IntersectHalfspace( CPlane const& Plane );

	void Intersect( CConvexPolytope3D const& Poly );

	float32 ComputeArea() const;

private:
	std::vector<CVertex> m_Verts;
	std::vector<Edge*> m_Edges;
	std::vector<Face*> m_Faces;

	void GetPerpPoint_Precision( CVector3f& PerpPoint, float32& OrientationSign ) const;
};

//Compute the orientation of the tetrahedron formed by points V1, V2, V3, and V4. The result is roughly six
//times the signed volume of the tetrahedron, and the sign of the result is exactly correct. In cases where 
//numerical error makes the sign difficult to determine, this function resorts to (much slower) exact arithmetic.
float64 WTCall ComputeOrientation( CVector3f const& V1, CVector3f const& V2, CVector3f const& V3, CVector3f const& V4 );

//Get the unique points from the input array Points and store the result in Result.
void WTCall GetUniquePoints( std::vector<CVector3f> const& Points, std::vector<CVector3f>& Result );

} // namespace Math
} // namespace WT
