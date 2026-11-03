/*
	frustum.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History

	Travis Baldree
	2/26/2004	-	Forward vector was being improperly extracted from view matrix during
					update - fixed.

	6/23/2004	-	DistanceFromNearClip was broken - was not taking into account planar offset

*/



#ifndef _FRUSTUM_H
#define _FRUSTUM_H


#include <D3D8.h>
#include <d3dx8.h>

#include "math3d.h"
#include "cullingbounds.h"

const enum	EFrustumIntersect
{
	KFrustumOutside,
	KFrustumInside,
	KFrustumIntersect
};

// the frustum is used in culling, primarily.
// it must be continually updated whenever the camera is transformed
class CFrustum
{
public:
	CFrustum( void ) :
						m_Forward( 0, 0, 1 ),
						m_AABBMinBounds( 0, 0, 0 ),
						m_AABBMaxBounds( 0, 0, 0 )
	{
	} // CFrustum()

	~CFrustum( void )
	{
	} // ~CFrustum()

	void Update( const D3DXMATRIX& ViewMatrix,			// view matrix
				 const D3DXMATRIX& ProjectionMatrix )	// projection matrix
	{

		m_FrustumBounds[0] = D3DXVECTOR3( -1.0f, -1.0f, 0.0f ); // xyz
		m_FrustumBounds[1] = D3DXVECTOR3( 1.0f, -1.0f, 0.0f ); // Xyz
		m_FrustumBounds[2] = D3DXVECTOR3( -1.0f, 1.0f, 0.0f ); // xYz
		m_FrustumBounds[3] = D3DXVECTOR3( 1.0f, 1.0f, 0.0f ); // XYz
		m_FrustumBounds[4] = D3DXVECTOR3( -1.0f, -1.0f, 1.0f ); // xyZ
		m_FrustumBounds[5] = D3DXVECTOR3( 1.0f, -1.0f, 1.0f ); // XyZ
		m_FrustumBounds[6] = D3DXVECTOR3( -1.0f, 1.0f, 1.0f ); // xYZ
		m_FrustumBounds[7] = D3DXVECTOR3( 1.0f, 1.0f, 1.0f ); // XYZ

		D3DXMATRIX FinalMatrix;
		D3DXMATRIX Orientation;
		D3DXMatrixMultiply( &FinalMatrix, &ViewMatrix, &ProjectionMatrix );
		D3DXMatrixInverse( &FinalMatrix, NULL, &FinalMatrix );

		D3DXMatrixTranspose( &Orientation, &ViewMatrix );
		m_Forward = D3DXVECTOR3( Orientation._31,
								 Orientation._32,
								 Orientation._33 );

		for ( uint32 i = 0; i < 8; i++ )
		{
			D3DXVec3TransformCoord( &m_FrustumBounds[i], 
									&m_FrustumBounds[i], 
									&FinalMatrix );
		}

		// we also maintain an Axial-aligned bounding box for very quick and rough frustum culls
		m_AABBMinBounds = m_FrustumBounds[0];
		m_AABBMaxBounds = m_FrustumBounds[0];

		for ( i = 1; i < 8; i++ )
		{
			ExpandBounds( m_AABBMinBounds, m_AABBMaxBounds, m_FrustumBounds[i] );

		}

		// now we generate planes from these bounding coordinates
		D3DXPlaneFromPoints( &m_FrustumPlane[0],
							 &m_FrustumBounds[0],
							 &m_FrustumBounds[1],
							 &m_FrustumBounds[2] ); // Near
		D3DXPlaneFromPoints( &m_FrustumPlane[1],
							 &m_FrustumBounds[6],
							 &m_FrustumBounds[7],
							 &m_FrustumBounds[5] ); // Far
		D3DXPlaneFromPoints( &m_FrustumPlane[2],
							 &m_FrustumBounds[2],
							 &m_FrustumBounds[6],
							 &m_FrustumBounds[4] ); // Left
		D3DXPlaneFromPoints( &m_FrustumPlane[3],
							 &m_FrustumBounds[7],
							 &m_FrustumBounds[3],
							 &m_FrustumBounds[5] ); // Right
		D3DXPlaneFromPoints( &m_FrustumPlane[4],
							 &m_FrustumBounds[2],
							 &m_FrustumBounds[3],
							 &m_FrustumBounds[6] ); // Top
		D3DXPlaneFromPoints( &m_FrustumPlane[5],
							 &m_FrustumBounds[1],
							 &m_FrustumBounds[0],
							 &m_FrustumBounds[4] ); // Bottom

	} // Update()

	void UpdateCW( const D3DXMATRIX& ViewMatrix,			// view matrix
				 const D3DXMATRIX& ProjectionMatrix )	// projection matrix
	{

		m_FrustumBounds[0] = D3DXVECTOR3( -1.0f, 1.0f, 0.0f ); // xyz
		m_FrustumBounds[1] = D3DXVECTOR3( 1.0f, 1.0f, 0.0f ); // Xyz
		m_FrustumBounds[2] = D3DXVECTOR3( -1.0f, -1.0f, 0.0f ); // xYz
		m_FrustumBounds[3] = D3DXVECTOR3( 1.0f, -1.0f, 0.0f ); // XYz
		m_FrustumBounds[4] = D3DXVECTOR3( -1.0f, 1.0f, 1.0f ); // xyZ
		m_FrustumBounds[5] = D3DXVECTOR3( 1.0f, 1.0f, 1.0f ); // XyZ
		m_FrustumBounds[6] = D3DXVECTOR3( -1.0f, -1.0f, 1.0f ); // xYZ
		m_FrustumBounds[7] = D3DXVECTOR3( 1.0f, -1.0f, 1.0f ); // XYZ

		D3DXMATRIX FinalMatrix;
		D3DXMATRIX Orientation;
		D3DXMatrixMultiply( &FinalMatrix, &ViewMatrix, &ProjectionMatrix );
		D3DXMatrixInverse( &FinalMatrix, NULL, &FinalMatrix );

		D3DXMatrixTranspose( &Orientation, &ViewMatrix );
		m_Forward = D3DXVECTOR3( Orientation._31,
								 Orientation._32,
								 Orientation._33 );

		for ( uint32 i = 0; i < 8; i++ )
		{
			D3DXVec3TransformCoord( &m_FrustumBounds[i], 
									&m_FrustumBounds[i], 
									&FinalMatrix );
		}

		// we also maintain an Axial-aligned bounding box for very quick and rough frustum culls
		m_AABBMinBounds = m_FrustumBounds[0];
		m_AABBMaxBounds = m_FrustumBounds[0];

		for ( i = 1; i < 8; i++ )
		{
			ExpandBounds( m_AABBMinBounds, m_AABBMaxBounds, m_FrustumBounds[i] );

		}

		// now we generate planes from these bounding coordinates
		D3DXPlaneFromPoints( &m_FrustumPlane[0],
							 &m_FrustumBounds[0],
							 &m_FrustumBounds[1],
							 &m_FrustumBounds[2] ); // Near
		D3DXPlaneFromPoints( &m_FrustumPlane[1],
							 &m_FrustumBounds[6],
							 &m_FrustumBounds[7],
							 &m_FrustumBounds[5] ); // Far
		D3DXPlaneFromPoints( &m_FrustumPlane[2],
							 &m_FrustumBounds[2],
							 &m_FrustumBounds[6],
							 &m_FrustumBounds[4] ); // Left
		D3DXPlaneFromPoints( &m_FrustumPlane[3],
							 &m_FrustumBounds[7],
							 &m_FrustumBounds[3],
							 &m_FrustumBounds[5] ); // Right
		D3DXPlaneFromPoints( &m_FrustumPlane[4],
							 &m_FrustumBounds[2],
							 &m_FrustumBounds[3],
							 &m_FrustumBounds[6] ); // Top
		D3DXPlaneFromPoints( &m_FrustumPlane[5],
							 &m_FrustumBounds[1],
							 &m_FrustumBounds[0],
							 &m_FrustumBounds[4] ); // Bottom

	} // Update()

	EFrustumIntersect	Intersects( CCullingBounds& CullingBounds )	// culling bounds of the object to intersect
	{
		return Intersects( CullingBounds.GetBounds() );
	} // Intersects()

	// return the distance of a point in worldspace to the nearclip plane
	float32				DistanceFromNearClip( const D3DXVECTOR3& Point )	// point to find distance from plane
	{
		return m_FrustumPlane[0].a * Point.x +
			   m_FrustumPlane[0].b * Point.y +
			   m_FrustumPlane[0].c * Point.z +
			   m_FrustumPlane[0].d;

	} // DistanceFromNearClip()

	EFrustumIntersect	Intersects( const D3DXVECTOR3* Bounds )	// culling bounds of the object to intersect						
	{
		uint32		TotalInside = 0;

		// Check boundary vertices against all 6 frustum planes, 
		// and store result (1 if outside) in a bitfield

		for ( uint8 Plane = 0; Plane < 6; Plane++ )
		{
			int32 InsideCount = 8;
			int32 PointsIn = 1;

			for ( uint8 Point = 0; Point < 8; Point++ )
			{
				if ( m_FrustumPlane[Plane].a * Bounds[Point].x +
					m_FrustumPlane[Plane].b * Bounds[Point].y +
					m_FrustumPlane[Plane].c * Bounds[Point].z +
					m_FrustumPlane[Plane].d < 0 )
				{
						PointsIn = 0;
						--InsideCount;
				}
			}

			// if all points were on the outside of this plane, we're out
			if( InsideCount == 0 )
			{
				return KFrustumOutside;
			}
			TotalInside += PointsIn;
		}

		// we're inside the frustum totally
		if( TotalInside == 6 )
		{
			return KFrustumInside;
		}
		
		// otherwise we're intersecting
		return KFrustumIntersect;

	} // Intersects()

	EFrustumIntersect	IntersectsAABB( const D3DXVECTOR3& MinBounds,	// min AABB bounds of the object to intersect
										const D3DXVECTOR3& MaxBounds)	// max AABB bounds of the object to intersect
						
	{

		// see if the AABBs intersect
		if( BoundsIntersect( m_AABBMinBounds,
							 m_AABBMaxBounds,
							 MinBounds,
							 MaxBounds ) )
		{
			// see if they contain one another
			if( BoundsContains( m_AABBMinBounds,
								m_AABBMaxBounds,
								MinBounds,
								MaxBounds ) )
			{
				return KFrustumInside;
			}

			return KFrustumIntersect;
		}
		return KFrustumOutside;
	} // IntersectsAABB()	

	EFrustumIntersect	IntersectsAABBXY( const D3DXVECTOR3& MinBounds,	// min AABB bounds of the object to intersect
										  const D3DXVECTOR3& MaxBounds)	// max AABB bounds of the object to intersect
						
	{

		// see if the AABBs intersect
		if( BoundsIntersectXY( m_AABBMinBounds,
							   m_AABBMaxBounds,
							   MinBounds,
							   MaxBounds ) )
		{
			// see if they contain one another
			if( BoundsContainsXY( m_AABBMinBounds,
								  m_AABBMaxBounds,
								  MinBounds,
								  MaxBounds ) )
			{
				return KFrustumInside;
			}

			return KFrustumIntersect;
		}
		return KFrustumOutside;
	} // IntersectsAABBXY()

	EFrustumIntersect	SphereIntersects( const D3DXVECTOR3& Position,
										  float32 Radius )	// radius object to intersect						
	{
		uint32		TotalInside = 0;

		// Check boundary vertices against all 6 frustum planes, 
		// and store result (1 if outside) in a bitfield

		for ( uint8 Plane = 0; Plane < 6; Plane++ )
		{

			if ( m_FrustumPlane[Plane].a * Position.x +
				 m_FrustumPlane[Plane].b * Position.y +
				 m_FrustumPlane[Plane].c * Position.z +
				 m_FrustumPlane[Plane].d < -Radius )
			{
				return KFrustumOutside;
			}
			TotalInside ++;
		}

		// we're inside the frustum totally
		if( TotalInside == 6 )
		{
			return KFrustumInside;
		}
		
		// otherwise we're intersecting
		return KFrustumIntersect;

	} // SphereIntersects()

	EFrustumIntersect	SphereIntersectsAABB( const D3DXVECTOR3& Position,
											  float32 Radius )	// radius object to intersect
						
	{

		// see if the AABBs intersect
		if( BoundsIntersect( m_AABBMinBounds,
							 m_AABBMaxBounds,
							 Position,
							 Radius ) )
		{
			// see if they contain one another
			if( BoundsContains( m_AABBMinBounds,
								m_AABBMaxBounds,
								Position,
								Radius ) )
			{
				return KFrustumInside;
			}

			return KFrustumIntersect;
		}
		return KFrustumOutside;
	} // SphereIntersectsAABB()	

	const D3DXVECTOR3&	Forward( void )			{	return m_Forward;	};

private:

	D3DXVECTOR3	m_FrustumBounds[8];	// local bounds of the view frustum
	D3DXPLANE	m_FrustumPlane[6];	// planes of the view frustum

	D3DXVECTOR3	m_AABBMinBounds;
	D3DXVECTOR3	m_AABBMaxBounds;

	D3DXVECTOR3	m_Forward;			// forward direction of the view frustum


};


#endif