/*
	facelist.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History:

	Travis Baldree
	8/31/2004	- point in poly test improved - was falling through some gaps before
				  as discovered by Pete. Fixed now with no noticeable performance loss -
				  added bonus that normal classifications are no longer stored.
*/

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/math3D.h"

#include <assert.h>
#include <d3dx8.h>

#include "facelist.h"


CFaceList::CFaceList():
		m_MaxMeshBoundsVEC3D( 0, 0, 0 ),
		m_MinMeshBoundsVEC3D( 0, 0, 0 )
{
} // CFaceList::CFaceList()

CFaceList::~CFaceList()
{
} // CFaceList::~CFaceList()

const D3DXVECTOR3& CFaceList::GetVertex( uint32 Index )			// index of vertex to retrieve
{
	// make sure this is a valid index 
	assert( Index < m_pVertices.size() );

	return m_pVertices[Index];
} // CFaceList::GetVertex()

const D3DXVECTOR3& CFaceList::GetNormal( uint32 FaceIndex )			// index of face normal to retrieve
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pNormals.size() );

	return m_pNormals[FaceIndex];

} // CFaceList::GetNormal()

const D3DXVECTOR3& CFaceList::GetMinFaceBounds( uint32 FaceIndex )			// index of face's bounds to retrieve 
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pMinFaceBoundsVEC3D.size() );

	return m_pMinFaceBoundsVEC3D[FaceIndex];

} // CFaceList::GetMinFaceBounds()

const D3DXVECTOR3& CFaceList::GetMaxFaceBounds( uint32 FaceIndex )			// index of face's bounds to retrieve 
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pMaxFaceBoundsVEC3D.size() );

	return m_pMaxFaceBoundsVEC3D[FaceIndex];

} // CFaceList::GetMaxFaceBounds()

uint32 CFaceList::GetVertexIndex( uint32 FaceIndex,		// index of face
								  uint32 VertexIndex )	// index of vertex within face (0-2)
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pFaceA.size() );
	assert( VertexIndex < 3 );

	if( VertexIndex == 0 )
	{
		return m_pFaceA[FaceIndex];
	}
	else if( VertexIndex == 1 )
	{
		return m_pFaceB[FaceIndex];
	}
	else 
	{
		return m_pFaceC[FaceIndex];
	}
} // CFaceList::GetVertexIndex()

uint32 CFaceList::GetVertexIndexA( uint32 FaceIndex )		// index of face
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pFaceA.size() );

	return m_pFaceA[FaceIndex];
} // CFaceList::GetVertexIndexA()

uint32 CFaceList::GetVertexIndexB( uint32 FaceIndex )		// index of face
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pFaceA.size() );

	return m_pFaceB[FaceIndex];
} // CFaceList::GetVertexIndexB()

uint32 CFaceList::GetVertexIndexC( uint32 FaceIndex )		// index of face
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pFaceA.size() );

	return m_pFaceC[FaceIndex];
} // CFaceList::GetVertexIndexC()

uint32 CFaceList::GetMaterial( uint32 FaceIndex )		// index of face
{
	// make sure this is a valid index 
	assert( FaceIndex < m_pFaceMaterial.size() );

	return m_pFaceMaterial[FaceIndex];
} // CFaceList::GetMaterial()

/// Add a vertex - passing -1 or no second argument automatically adds at the end of the list
void CFaceList::AddVertex( const D3DXVECTOR3& Vertex,	// vertex to add
						   int32 Index )		// index to add at (-1 or no param for end of list)
{
	if ( Index == -1 )
	{
		m_pVertices.push_back( Vertex );
	}
	else
	{
		if ( Index < (int32)m_pVertices.size() )
		{
			m_pVertices[Index] = Vertex;
		}
	}
} // CFaceList::AddVertex()

/// Add a vertex color - passing -1 or no second argument automatically adds at the end of the list
void CFaceList::AddColor( const D3DXVECTOR3& Color,	// color to add
						  int32 Index )		// index to add at (-1 or no param for end of list)
{
	if ( Index == -1 )
	{
		m_pColors.push_back( Color );
	}
	else
	{
		if ( Index < (int32)m_pColors.size() )
		{
			m_pColors[Index] = Color;
		}
	}
} // CFaceList::AddColor()

/// Add a normal - passing -1 or no second argument automatically adds at the end of the list
void CFaceList::AddNormal( const D3DXVECTOR3& Normal,	// normal to add
						   int32 Index )		// index to add at (-1 or no param for end of list)
{
	if ( Index == -1 )
	{
		m_pNormals.push_back( Normal );
	}
	else
	{
		if ( Index < (int32)m_pNormals.size() )
		{
			m_pNormals[Index] = Normal;
		}
	}
} // CFaceList::AddHormal()

/// Add a face definition - passing -1 or no final argument automatically adds at the end of the list
void CFaceList::AddFace( int32 A,			// vert index 1
						 int32 B,			// vert index 2
						 int32 C,			// vert index 3
						 int32 Index )		// index to add at (-1 or no param for end of list)
{
	AddFace( A, B, C, 1, Index );
} // CFaceList::AddFace()

/// Add a face definition - passing -1 or no final argument automatically adds at the end of the list
void CFaceList::AddFace( int32 A,			// vert index 1
						 int32 B,			// vert index 2
						 int32 C,			// vert index 3
						 uint32 Material,	// material id for the face
						 int32 Index )		// index to add at (-1 or no param for end of list)
{
	if ( Index == -1 )
	{
		m_pFaceMaterial.push_back( Material );
		m_pFaceA.push_back( A );
		m_pFaceB.push_back( B );
		m_pFaceC.push_back( C );
	}
	else
	{
		if ( Index < (int32)m_pFaceA.size() )
		{
			m_pFaceA[Index] = A;
			m_pFaceB[Index] = B;
			m_pFaceC[Index] = C;
		}
		if ( Index < (int32)m_pFaceMaterial.size() )
		{
			m_pFaceMaterial[Index] = Material;
		}
	}
} // CFaceList::AddFace()

void CFaceList::AddFaceList( CFaceList* pFaceList,				// face list to add
							 const D3DXMATRIX& Transformation,	// transformation of the face list
							 int32 MaterialID )					// override material ID ( -1 uses existing data )
{
	uint32 CurrentVertices = Vertices();
	D3DXVECTOR3 TransformedVertex;
	for( uint32 i = 0; i < pFaceList->Vertices(); i++ )
	{
		D3DXVec3TransformCoord( &TransformedVertex, &pFaceList->GetVertex( i ), &Transformation );
		AddVertex( TransformedVertex );
	}

	for( int32 i = 0; i < pFaceList->GetColors().size(); i++ )
	{
		AddColor( pFaceList->GetColors()[ i ] );
	}

	for( int32 i = 0; i < pFaceList->Faces(); i++ )
	{
		if( MaterialID == -1 )
		{
			AddFace( pFaceList->GetVertexIndexA( i ) + CurrentVertices,
					 pFaceList->GetVertexIndexB( i ) + CurrentVertices,
					 pFaceList->GetVertexIndexC( i ) + CurrentVertices,
					 pFaceList->GetMaterial( i ),
					 -1 );
		}
		else
		{
			AddFace( pFaceList->GetVertexIndexA( i ) + CurrentVertices,
					 pFaceList->GetVertexIndexB( i ) + CurrentVertices,
					 pFaceList->GetVertexIndexC( i ) + CurrentVertices,
					 (uint32) MaterialID,
					 -1 );
		}
	}

} // CFaceList::AddFaceList()

// weld redundant vertices, and update the indices accordingly
void CFaceList::Optimize( void )
{
	for( uint32 i = 0; i < m_pVertices.size(); i++ )
	{
		for( uint32 j = i+1; j < m_pVertices.size(); j++ )
		{
			// if these two verts are the same
			if( m_pVertices[i] == m_pVertices[j] )
			{
				// update all face indices
				for( uint32 k = 0; k < m_pFaceA.size(); k++ )
				{
					if( m_pFaceA[k] == j )
					{
						m_pFaceA[k] = i;
					}
					if( m_pFaceB[k] == j )
					{
						m_pFaceB[k] = i;
					}
					if( m_pFaceC[k] == j )
					{
						m_pFaceC[k] = i;
					}
				}
				uint32 LostIndex = m_pVertices.size() - 1;

				// get rid of the useless vert
				m_pVertices[j] = m_pVertices[m_pVertices.size() - 1];
				m_pVertices.pop_back();
				// update all face indices
				for( int32 k = 0; k < m_pFaceA.size(); k++ )
				{
					if( m_pFaceA[k] == LostIndex )
					{
						m_pFaceA[k] = j;
					}
					if( m_pFaceB[k] == LostIndex )
					{
						m_pFaceB[k] = j;
					}
					if( m_pFaceC[k] == LostIndex )
					{
						m_pFaceC[k] = j;
					}
				}
			}
		}
	}
} // CFaceList::Optimize()

/// Calculates the local bounds of the entire mesh, as well as the individual bounds
/// of each face to optimize collision checks later
void CFaceList::CalculateFaceBounds()
{
	if( m_pVertices.size() == 0 )
	{
		return;
	}
	// set the min and max bounds of the mesh to the first vertex
	m_MinMeshBoundsVEC3D = m_pVertices[0];
	m_MaxMeshBoundsVEC3D = m_pVertices[0];


	// calculate the bounding Bounds of the entire mesh
	for ( uint32 i = 0; i < m_pVertices.size(); i++ )
	{
		ExpandBounds( m_MinMeshBoundsVEC3D,
					  m_MaxMeshBoundsVEC3D,
					  m_pVertices[i] );
	}

	m_pMinFaceBoundsVEC3D.clear();
	m_pMaxFaceBoundsVEC3D.clear();

	for( int32 i = 0; i < m_pFaceA.size(); i++ )
	{
		// set the min and max Bounds for this face to it's first vertex
		m_pMinFaceBoundsVEC3D.push_back( m_pVertices[m_pFaceA[i]] );
		m_pMaxFaceBoundsVEC3D.push_back( m_pVertices[m_pFaceA[i]] );

		// check the second vert in the face and adjust the bounds accordingly
		ExpandBounds( m_pMinFaceBoundsVEC3D[i],
					  m_pMaxFaceBoundsVEC3D[i],
					  m_pVertices[m_pFaceB[i]] );
		// check the third vert in the face and adjust the bounds accordingly
		ExpandBounds( m_pMinFaceBoundsVEC3D[i],
					  m_pMaxFaceBoundsVEC3D[i],
					  m_pVertices[m_pFaceC[i]] );
	}

} // CFaceList::CalculateFaceBounds()

/// Calculates the normals and face classifications for the mesh
void CFaceList::CalculateNormals()
{
	D3DXVECTOR3	TempVEC3D1	( 0, 0, 0 );
	D3DXVECTOR3	TempVEC3D2	( 0, 0, 0 );
	D3DXVECTOR3	Normal		( 0, 0, 0 );


	m_pNormals.clear();

	for ( uint32 i = 0; i < m_pFaceA.size(); i++ )
	{
		TempVEC3D1 = m_pVertices[m_pFaceC[i]];
		TempVEC3D2 = m_pVertices[m_pFaceB[i]];
		TempVEC3D1 -= m_pVertices[m_pFaceB[i]];
		TempVEC3D2 -= m_pVertices[m_pFaceA[i]];

		D3DXVec3Cross( &Normal, &TempVEC3D1, &TempVEC3D2 );
		D3DXVec3Normalize( &Normal, &Normal );
		m_pNormals.push_back( Normal );


		float32	LargestComponent	= fabs( Normal.x );
	}
} // CFaceList::CalculateNormals()

/// Calculate a collision along a ray with the mesh
/// using supplied Bounds of the ray
bool CFaceList::RayCollision( const D3DXVECTOR3& Start,			// start of ray 
							  const D3DXVECTOR3& End,			// endpoint of ray
							  const D3DXVECTOR3& MinBounds,		// min bounds of collision ray
							  const D3DXVECTOR3& MaxBounds,		// max bounds of collision ray
							  D3DXVECTOR3& ImpactPoint,			// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
							  float32& BestDistance )				// distance from start location
{
	uint32 FaceMaterial;
	D3DXVECTOR3 AverageColor;
	return RayCollision( Start,
						 End,
						 MinBounds,
						 MaxBounds,
						 ImpactPoint,
						 ImpactNormal,
						 FaceMaterial,
						 AverageColor,
						 BestDistance );

} // CFaceList::RayCollision()

/// Calculate a collision along a ray with the mesh
/// using supplied Bounds of the ray
bool CFaceList::RayCollision( const D3DXVECTOR3& Start,			// start of ray 
							  const D3DXVECTOR3& End,			// endpoint of ray
							  const D3DXVECTOR3& MinBounds,		// min bounds of collision ray
							  const D3DXVECTOR3& MaxBounds,		// max bounds of collision ray
							  D3DXVECTOR3& ImpactPoint,			// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
							  uint32& FaceMaterial,				// material of the impacted face
							  D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
							  float32& BestDistance )				// distance from start location
{
	// occasionally we get meshes that are empty - skip these
	if ( m_pFaceA.size() > 0 )
	{
		// ensure that the bounds of the ray intersect the bounds of the entire mesh
		if ( BoundsIntersect( m_MinMeshBoundsVEC3D,
							  m_MaxMeshBoundsVEC3D,
							  MinBounds,
							  MaxBounds ) )
		{
			for ( uint32 FaceIndex( 0 );
				  FaceIndex < m_pFaceA.size();
				  FaceIndex++ )
			{
				// for each face, make sure that the ray-sphere bounds intersect the bounds of the face
				// before doing the heavy lifting
				if ( BoundsIntersect( m_pMinFaceBoundsVEC3D[FaceIndex],
									  m_pMaxFaceBoundsVEC3D[FaceIndex],
									  MinBounds,
									  MaxBounds ) )
				{
					// determine whether the ray passes through the plane, by classifying the start and
					// end of the ray
					int8	StartClassification	= ClassifyPoint( Start,
																 m_pVertices[m_pFaceA[FaceIndex]],
																 m_pNormals[FaceIndex] );

					int8	EndClassification	= ClassifyPoint( End,
																 m_pVertices[m_pFaceA[FaceIndex]],
																 m_pNormals[FaceIndex] );

					// if it intersects, and the start point is on the front side of the plane,
					if ( ( StartClassification != EndClassification ) &&
						 ( StartClassification != KPlaneBack ) )
					{
						D3DXVECTOR3	TemporaryVEC3D	( 0, 0, 0 );
						D3DXVECTOR3	TemporaryVEC3D2	( End ); 
						// Find the intersection point
						if ( GetLinePlaneIntersection( Start,
													   End,
													   m_pVertices[m_pFaceA[FaceIndex]],
													   m_pNormals[FaceIndex],
													   TemporaryVEC3D ) )
						{
							// determine if the point lies within the bounds of this face
							if ( PointIn( TemporaryVEC3D, FaceIndex ) )
							{
								TemporaryVEC3D2 = TemporaryVEC3D - Start;
								float32	Length	= D3DXVec3Length( &TemporaryVEC3D2 );
								// and if it is closer than previous collisions, we keep it
								if ( Length < BestDistance )
								{
									BestDistance = Length;
									ImpactPoint = TemporaryVEC3D;
									ImpactNormal = m_pNormals[FaceIndex];
									FaceMaterial = m_pFaceMaterial[FaceIndex];
									if( m_pColors.size() > m_pFaceA[FaceIndex] )
									{
										// average the colors of the corners of the face
										AverageColor = ( m_pColors[m_pFaceA[FaceIndex]] +
													     m_pColors[m_pFaceB[FaceIndex]] +
													     m_pColors[m_pFaceC[FaceIndex]] ) / 3.0f;
									
									}
									else
									{
										AverageColor = D3DXVECTOR3( 1, 1, 1 );
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CFaceList::RayCollision()

/// Calculate a collision along a ray with the mesh
/// using supplied Bounds of the ray, and given a list of faces
bool CFaceList::RayCollision( const std::vector< uint32 >& pFaceList,	// list of face indices to collide with
							  const D3DXVECTOR3& Start,			// start of ray 
							  const D3DXVECTOR3& End,			// endpoint of ray
							  const D3DXVECTOR3& MinBounds,		// min bounds of collision ray
							  const D3DXVECTOR3& MaxBounds,		// max bounds of collision ray
							  D3DXVECTOR3& ImpactPoint,			// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
							  uint32& FaceMaterial,				// material of the impacted face
							  D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
							  float32& BestDistance )				// distance from start location
{
	// occasionally we get meshes that are empty - skip these
	if ( pFaceList.size() > 0 )
	{
		// ensure that the bounds of the ray intersect the bounds of the entire mesh
		if ( BoundsIntersect( m_MinMeshBoundsVEC3D,
							  m_MaxMeshBoundsVEC3D,
							  MinBounds,
							  MaxBounds ) )
		{
			for ( uint32 FaceIndex( 0 );
				  FaceIndex <  pFaceList.size();
				  FaceIndex++ )
			{
				// for each face, make sure that the ray-sphere bounds intersect the bounds of the face
				// before doing the heavy lifting
				if ( BoundsIntersect( m_pMinFaceBoundsVEC3D[pFaceList[FaceIndex]],
									  m_pMaxFaceBoundsVEC3D[pFaceList[FaceIndex]],
									  MinBounds,
									  MaxBounds ) )
				{
					// determine whether the ray passes through the plane, by classifying the start and
					// end of the ray
					int8	StartClassification	= ClassifyPoint( Start,
																 m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
																 m_pNormals[pFaceList[FaceIndex]] );

					int8	EndClassification	= ClassifyPoint( End,
																 m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
																 m_pNormals[pFaceList[FaceIndex]] );

					// if it intersects, and the start point is on the front side of the plane,
					if ( ( StartClassification != EndClassification ) &&
						 ( StartClassification != KPlaneBack ) )
					{
						D3DXVECTOR3	TemporaryVEC3D	( 0, 0, 0 );
						D3DXVECTOR3	TemporaryVEC3D2	( End ); 
						// Find the intersection point
						if ( GetLinePlaneIntersection( Start,
													   End,
													   m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
													   m_pNormals[pFaceList[FaceIndex]],
													   TemporaryVEC3D ) )
						{
							// determine if the point lies within the bounds of this face
							if ( PointIn( TemporaryVEC3D, pFaceList[FaceIndex] ) )
							{
								TemporaryVEC3D2 = TemporaryVEC3D - Start;
								float32	Length	= D3DXVec3Length( &TemporaryVEC3D2 );
								// and if it is closer than previous collisions, we keep it
								if ( Length < BestDistance )
								{
									BestDistance = Length;
									ImpactPoint = TemporaryVEC3D;
									ImpactNormal = m_pNormals[pFaceList[FaceIndex]];
									FaceMaterial = m_pFaceMaterial[pFaceList[FaceIndex]];
									if( m_pColors.size() > m_pFaceA[pFaceList[FaceIndex]] )
									{
										// average the colors of the corners of the face
										AverageColor = ( m_pColors[m_pFaceA[pFaceList[FaceIndex]]] +
													     m_pColors[m_pFaceB[pFaceList[FaceIndex]]] +
													     m_pColors[m_pFaceC[pFaceList[FaceIndex]]] ) / 3.0f;
									
									}
									else
									{
										AverageColor = D3DXVECTOR3( 1, 1, 1 );
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CFaceList::RayCollision()

/// Calculate a collision along a ray with the mesh
/// after first calculating the Bounds of the ray we are colliding along.
bool CFaceList::RayCollision( const D3DXVECTOR3& Start,		// start of ray
							  const D3DXVECTOR3& End,		// endpoint of ray
							  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
							  float32& BestDistance )			// distance from stating location
{
	uint32 FaceMaterial;
	return RayCollision( Start,
						 End,
						 ImpactPoint,
						 ImpactNormal,
						 FaceMaterial,
						 BestDistance );
} // CFaceList::RayCollision()

/// Calculate a collision along a ray with the mesh
/// after first calculating the Bounds of the ray we are colliding along.
bool CFaceList::RayCollision( const D3DXVECTOR3& Start,		// start of ray
							  const D3DXVECTOR3& End,		// endpoint of ray
							  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
							  uint32& FaceMaterial,			// material of the impacted face
							  float32& BestDistance )			// distance from stating location
{
	D3DXVECTOR3 MinBounds( Start );
	D3DXVECTOR3	MaxBounds	( Start );
	D3DXVECTOR3 AverageColor;
	ExpandBounds( MinBounds, MaxBounds, End );


	return RayCollision( Start,
						 End,
						 MinBounds,
						 MaxBounds,
						 ImpactPoint,
						 ImpactNormal,
						 FaceMaterial,
						 AverageColor,
						 BestDistance );
} // CFaceList::RayCollision()

/// Calculate a spherical collision along a ray with the mesh
/// using supplied Bounds of the ray-sphere
bool CFaceList::SphereCollision( const D3DXVECTOR3& Start,		// start of ray
								 const D3DXVECTOR3& End,		// end point of ray
								 const D3DXVECTOR3& MinBounds,	// min bounds of collision ray
								 const D3DXVECTOR3& MaxBounds,	// max bounds of collision ray
								 float32 Radius,				// radius of sphere
								 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
								 D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
								 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
								 float32& BestDistance )			// distance from start location
{
	uint32 FaceMaterial;
	D3DXVECTOR3 AverageColor;
	return SphereCollision( Start,
							End,
							MinBounds,
							MaxBounds,
							Radius,
							FinalPosition,
							ImpactPoint,
							ImpactNormal,
							FaceMaterial,
							AverageColor,
							BestDistance );
} // CFaceList::SphereCollision()

/// Calculate a spherical collision along a ray with the mesh
/// using supplied Bounds of the ray-sphere
bool CFaceList::SphereCollision( const D3DXVECTOR3& Start,		// start of ray
								 const D3DXVECTOR3& End,		// end point of ray
								 const D3DXVECTOR3& MinBounds,	// min bounds of collision ray
								 const D3DXVECTOR3& MaxBounds,	// max bounds of collision ray
								 float32 Radius,				// radius of sphere
								 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
								 D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
								 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
								 uint32& FaceMaterial,			// material of the impacted face
								 D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
								 float32& BestDistance )			// distance from start location
{
	float32 EdgeDot( 1 );
	// occasionally we get meshes that are empty - skip these
	if ( m_pFaceA.size() > 0 )
	{
		// ensure that the bounds of the ray-sphere intersect the bounds of the entire mesh
		if ( BoundsIntersect( m_MinMeshBoundsVEC3D,
							  m_MaxMeshBoundsVEC3D,
							  MinBounds,
							  MaxBounds ) )
		{
			for ( uint32 FaceIndex( 0 );
				  FaceIndex < m_pFaceA.size();
				  FaceIndex++ )
			{
				// for each face, make sure that the ray-sphere bounds intersect the bounds of the face
				// before doing the heavy lifting
				if ( BoundsIntersect( m_pMinFaceBoundsVEC3D[FaceIndex],
									  m_pMaxFaceBoundsVEC3D[FaceIndex],
									  MinBounds,
									  MaxBounds ) )
				{
					// determine whether the ray passes through the plane, by classifying the start and
					// end of the ray
					int8	StartClassification	= ClassifyPoint( Start,
																 m_pVertices[m_pFaceA[FaceIndex]],
																 m_pNormals[FaceIndex] );

					int8	EndClassification	= ClassifyPointForSphere( End,
																		  m_pVertices[m_pFaceA[FaceIndex]],
																		  m_pNormals[FaceIndex],
																		  Radius );



					// if it intersects 
					if ( StartClassification != EndClassification 
						 && EndClassification != KPlaneFront )
					{
						D3DXVECTOR3	TemporaryVEC3D	( 0, 0, 0 );
						D3DXVECTOR3	TemporaryVEC3D2	( End ); 

						TemporaryVEC3D2 -= m_pNormals[FaceIndex] * Radius;
						// Find the intersection point
						if ( GetSpherePlaneIntersection( Start,
														 TemporaryVEC3D2,
														 m_pVertices[m_pFaceA[FaceIndex]],
														 m_pNormals[FaceIndex],
														 TemporaryVEC3D ) )
						{
							// determine if the point lies within the bounds of this face
							if ( PointIn( TemporaryVEC3D, FaceIndex ) )
							{
								TemporaryVEC3D2 = TemporaryVEC3D - Start;
								float32	Length	= D3DXVec3Length( &TemporaryVEC3D2 );
								// and if it is closer than previous collisions, we keep it
								if ( Length < BestDistance )
								{
									BestDistance = Length;
									ImpactPoint = TemporaryVEC3D;
									FinalPosition = ImpactPoint + m_pNormals[FaceIndex] * Radius;
									ImpactNormal = m_pNormals[FaceIndex];
									FaceMaterial = m_pFaceMaterial[FaceIndex];

									if( m_pColors.size() > m_pFaceA[FaceIndex] )
									{
										// average the colors of the corners of the face
										AverageColor = ( m_pColors[m_pFaceA[FaceIndex]] +
													     m_pColors[m_pFaceB[FaceIndex]] +
													     m_pColors[m_pFaceC[FaceIndex]] ) / 3.0f;
									
									}
									else
									{
										AverageColor = D3DXVECTOR3( 1, 1, 1 );
									}

								}
							}
							// otherwise we check for sphere edge collision
							else 
							{
								ClosestPointOnTriangle( m_pVertices[m_pFaceA[FaceIndex]],
														m_pVertices[m_pFaceB[FaceIndex]],
														m_pVertices[m_pFaceC[FaceIndex]],
														Start,
														TemporaryVEC3D );

								TemporaryVEC3D2 = End - TemporaryVEC3D;
								float32	Length	= D3DXVec3Length( &TemporaryVEC3D2 );
								if ( Length <= Radius )
								{
									TemporaryVEC3D2 = TemporaryVEC3D - Start;
									Length	= D3DXVec3Length( &TemporaryVEC3D2 );
									if( Length <= BestDistance )
									{
										TemporaryVEC3D2 = End - TemporaryVEC3D;
										D3DXVec3Normalize( &TemporaryVEC3D2,
														   &TemporaryVEC3D2 );
										float32 Dot =  D3DXVec3Dot( &TemporaryVEC3D2,
																	&m_pNormals[FaceIndex] );
										if( Dot < EdgeDot )
										{
											int8	EndEdgeClassification	= ClassifyPoint( End,
																			  m_pVertices[m_pFaceA[FaceIndex]],
																			  m_pNormals[FaceIndex] );

											if( EndEdgeClassification == KPlaneFront )
											{
												EdgeDot = Dot;
												BestDistance = Length;

												ImpactNormal = TemporaryVEC3D2;


												ImpactPoint = TemporaryVEC3D;

												FinalPosition = ImpactPoint + ImpactNormal * Radius;
												FaceMaterial = m_pFaceMaterial[FaceIndex];


												if( m_pColors.size() > m_pFaceA[FaceIndex] )
												{
													// average the colors of the corners of the face
													AverageColor = ( m_pColors[m_pFaceA[FaceIndex]] +
																	m_pColors[m_pFaceB[FaceIndex]] +
																	m_pColors[m_pFaceC[FaceIndex]] ) / 3.0f ;
												
												}
												else
												{
													AverageColor = D3DXVECTOR3( 1, 1, 1 );
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CFaceList::SphereCollision()

/// Calculate a spherical collision along a ray with the mesh
/// using supplied Bounds of the ray-sphere and given a list of faces
bool CFaceList::SphereCollision( const std::vector< uint32 >& pFaceList,	// list of face indices to collide with
								 const D3DXVECTOR3& Start,		// start of ray
								 const D3DXVECTOR3& End,		// end point of ray
								 const D3DXVECTOR3& MinBounds,	// min bounds of collision ray
								 const D3DXVECTOR3& MaxBounds,	// max bounds of collision ray
								 float32 Radius,				// radius of sphere
								 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
								 D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
								 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
								 uint32& FaceMaterial,			// material of the impacted face
								 D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
								 float32& BestDistance )			// distance from start location
{
	float32 EdgeDot( 1 );

	// occasionally we get meshes that are empty - skip these
	if (pFaceList.size() > 0 )
	{
		// ensure that the bounds of the ray-sphere intersect the bounds of the entire mesh
		if ( BoundsIntersect( m_MinMeshBoundsVEC3D,
							  m_MaxMeshBoundsVEC3D,
							  MinBounds,
							  MaxBounds ) )
		{
			for ( uint32 FaceIndex( 0 );
				  FaceIndex < pFaceList.size();
				  FaceIndex++ )
			{
				// for each face, make sure that the ray-sphere bounds intersect the bounds of the face
				// before doing the heavy lifting
				if ( BoundsIntersect( m_pMinFaceBoundsVEC3D[pFaceList[FaceIndex]],
									  m_pMaxFaceBoundsVEC3D[pFaceList[FaceIndex]],
									  MinBounds,
									  MaxBounds ) )
				{
					// determine whether the ray passes through the plane, by classifying the start and
					// end of the ray
					int8	StartClassification	= ClassifyPoint( Start,
																 m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
																 m_pNormals[pFaceList[FaceIndex]] );

					int8	EndClassification	= ClassifyPointForSphere( End,
																		  m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
																		  m_pNormals[pFaceList[FaceIndex]],
																		  Radius );



					// if it intersects
					if ( StartClassification != EndClassification
						 && EndClassification != KPlaneFront )
					{
						D3DXVECTOR3	TemporaryVEC3D	( 0, 0, 0 );
						D3DXVECTOR3	TemporaryVEC3D2	( End ); 

						TemporaryVEC3D2 -= m_pNormals[pFaceList[FaceIndex]] * Radius;
						// Find the intersection point
						if ( GetSpherePlaneIntersection( Start,
														 TemporaryVEC3D2,
														 m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
														 m_pNormals[pFaceList[FaceIndex]],
														 TemporaryVEC3D ) )
						{
							// determine if the point lies within the bounds of this face
							if ( PointIn( TemporaryVEC3D, pFaceList[FaceIndex] ) )
							{
								TemporaryVEC3D2 = TemporaryVEC3D - Start;
								float32	Length	= D3DXVec3Length( &TemporaryVEC3D2 );
								// and if it is closer than previous collisions, we keep it
								if ( Length <= BestDistance )
								{
									BestDistance = Length;
									ImpactPoint = TemporaryVEC3D;
									ImpactNormal = m_pNormals[pFaceList[FaceIndex]];
									FinalPosition = ImpactPoint + ImpactNormal * Radius;
									FaceMaterial = m_pFaceMaterial[pFaceList[FaceIndex]];

									if( m_pColors.size() > m_pFaceA[pFaceList[FaceIndex]] )
									{
										// average the colors of the corners of the face
										AverageColor = ( m_pColors[m_pFaceA[pFaceList[FaceIndex]]] +
													     m_pColors[m_pFaceB[pFaceList[FaceIndex]]] +
													     m_pColors[m_pFaceC[pFaceList[FaceIndex]]] ) / 3.0f;
									
									}
									else
									{
										AverageColor = D3DXVECTOR3( 1, 1, 1 );
									}

								}
							}
							// otherwise we check for sphere edge collision
							else 
							{
								ClosestPointOnTriangle( m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
														m_pVertices[m_pFaceB[pFaceList[FaceIndex]]],
														m_pVertices[m_pFaceC[pFaceList[FaceIndex]]],
														Start,
														TemporaryVEC3D );

								TemporaryVEC3D2 = End - TemporaryVEC3D;
								float32	Length	= D3DXVec3Length( &TemporaryVEC3D2 );
								if ( Length <= Radius )
								{

									TemporaryVEC3D2 = TemporaryVEC3D - Start;
									Length	= D3DXVec3Length( &TemporaryVEC3D2 );
									if( Length <= BestDistance )
									{
										TemporaryVEC3D2 = End - TemporaryVEC3D;
										D3DXVec3Normalize( &TemporaryVEC3D2,
														   &TemporaryVEC3D2 );
										float32 Dot =  D3DXVec3Dot( &TemporaryVEC3D2,
																	&m_pNormals[pFaceList[FaceIndex]] );
										if( Dot < EdgeDot )
										{
											int8	EndEdgeClassification	= ClassifyPoint( End,
																			  m_pVertices[m_pFaceA[pFaceList[FaceIndex]]],
																			  m_pNormals[pFaceList[FaceIndex]] );

											if( EndEdgeClassification == KPlaneFront )
											{
												EdgeDot = Dot;
												BestDistance = Length;

												ImpactNormal = TemporaryVEC3D2;


												ImpactPoint = TemporaryVEC3D;

												FinalPosition = ImpactPoint + ImpactNormal * Radius;
												FaceMaterial = m_pFaceMaterial[pFaceList[FaceIndex]];

												if( m_pColors.size() > m_pFaceA[pFaceList[FaceIndex]] )
												{
													// average the colors of the corners of the face
													AverageColor = ( m_pColors[m_pFaceA[pFaceList[FaceIndex]]] +
																	m_pColors[m_pFaceB[pFaceList[FaceIndex]]] +
																	m_pColors[m_pFaceC[pFaceList[FaceIndex]]] ) / 3.0f ;
												}
												else
												{
													AverageColor = D3DXVECTOR3( 1, 1, 1 );
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CFaceList::SphereCollision()

/// Calculate a spherical collision along a ray with the mesh
/// after first calculating the Bounds of the sphere-ray we are colliding along.
bool CFaceList::SphereCollision( const D3DXVECTOR3& Start,		// starting point
								 const D3DXVECTOR3& End,		// ending point
								 float32 Radius,				// radius of sphere
								 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
								 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
								 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
								 float32& BestDistance )			// distance from start point
{
	uint32 FaceMaterial;
	return SphereCollision( Start,
							End,
							Radius,
							FinalPosition,
							ImpactPoint,
							ImpactNormal,
							FaceMaterial,
							BestDistance );
} // CFaceList::SphereCollision(

/// Calculate a spherical collision along a ray with the mesh
/// after first calculating the Bounds of the sphere-ray we are colliding along.
bool CFaceList::SphereCollision( const D3DXVECTOR3& Start,		// starting point
								 const D3DXVECTOR3& End,		// ending point
								 float32 Radius,				// radius of sphere
								 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
								 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
								 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
								 uint32& FaceMaterial,			// material of the impacted face
								 float32& BestDistance )			// distance from start point
{
	D3DXVECTOR3 MinBounds( Start );
	D3DXVECTOR3	MaxBounds( Start );
	D3DXVECTOR3 AverageColor;

	ExpandBounds( MinBounds, MaxBounds, End );

	// expand the bounds of min and max by the radius of the sphere
	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;

	return SphereCollision( Start,
							End,
							MinBounds,
							MaxBounds,
							Radius,
							FinalPosition,
							ImpactPoint,
							ImpactNormal,
							FaceMaterial,
							AverageColor,
							BestDistance );
} // CFaceList::SphereCollision()

/// determine whether the given point lies within the bounds of a face,
/// given that the point lies upon the face's plane.
bool CFaceList::PointIn( const D3DXVECTOR3& Point,// point to check
						 int32 FaceIndex )		  // index of face to check against
{

	D3DXVECTOR3 Edge1 = m_pVertices[m_pFaceB[FaceIndex]] - m_pVertices[m_pFaceA[FaceIndex]];
	D3DXVECTOR3 Edge2 = m_pVertices[m_pFaceC[FaceIndex]] - m_pVertices[m_pFaceA[FaceIndex]];

	D3DXVECTOR3 DeterminantVec;
	D3DXVec3Cross( &DeterminantVec, &m_pNormals[FaceIndex], &Edge2 );

	float32 Determinant = D3DXVec3Dot( &Edge1, &DeterminantVec );

	D3DXVECTOR3 Ray = Point - m_pVertices[m_pFaceA[FaceIndex]];

	float32 u = D3DXVec3Dot( &Ray, &DeterminantVec );

	if( u < 0 || u > Determinant )
	{
		return kFalse;
	}

	D3DXVECTOR3 QVector;
	D3DXVec3Cross( &QVector, &Ray, &Edge1 );

	float32 v = D3DXVec3Dot( &m_pNormals[FaceIndex], &QVector );
	if( v < 0 || u + v > Determinant )
	{
		return kFalse;
	}

	return kTrue;
} // CFaceList::PointIn()

