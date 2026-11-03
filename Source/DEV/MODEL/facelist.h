/*
	facelist.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _FACELIST_H
#define _FACELIST_H

#include "../UTILITIES/math3d.h"

/// The FaceList is used primarily for performing collisions on a set of faces,
/// and holds a vertex list as well as face indices and normals used for this operation
/// FaceList assumes a non-dynamic set of data. You must know the vertex and face
/// counts when creating it, and it is assumed that they will not change after
/// creation time.
/// When creating a FaceList, use the AddVertex and AddFace methods to assemble your
/// face list, then call CalculateNormals() and CalculateFaceBounds() to prepare it
/// for collisions

class CFaceList
{
public:




			CFaceList( void );
			~CFaceList( void );

	/// calculate the min and max bounds of all faces
	/// within the mesh, as well as the mesh's min and max
	void	CalculateFaceBounds( void ); 

	void	CalculateNormals( void );
	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	uint32						Faces( void )	{	return m_pFaceA.size();			}; 
	uint32						Vertices( void ){	return m_pVertices.size();		}; 

	/// retrieves a vertex from the list
	const D3DXVECTOR3&			GetVertex( uint32 Index );	// Index of vertex to retrieve

	/// retrieves a normal from the list
	const D3DXVECTOR3&			GetNormal( uint32 Index );	// Index of face normal to retrieve

	uint32						GetVertexIndex( uint32 FaceIndex,		// index of face
												uint32 VertexIndex );	// index of vertex within face (0-2)

	uint32						GetVertexIndexA( uint32 FaceIndex );		// index of face
	uint32						GetVertexIndexB( uint32 FaceIndex );		// index of face
	uint32						GetVertexIndexC( uint32 FaceIndex );		// index of face

	uint32						GetMaterial( uint32 FaceIndex );		// index of face

	const std::vector <D3DXVECTOR3>&	GetVertices( void )const		{ return m_pVertices;	};
	const std::vector <D3DXVECTOR3>&	GetColors( void )const			{ return m_pColors;	};
	const std::vector <D3DXVECTOR3>&	GetNormals( void )const			{ return m_pNormals;	};
	const std::vector <uint32>&			GetIndicesA( void )const		{ return m_pFaceA;	};
	const std::vector <uint32>&			GetIndicesB( void )const		{ return m_pFaceB;	};
	const std::vector <uint32>&			GetIndicesC( void )const		{ return m_pFaceC;	};


	const D3DXVECTOR3&			GetMinFaceBounds( uint32 FaceIndex );			// index of face's bounds to retrieve 
	const D3DXVECTOR3&			GetMaxFaceBounds( uint32 FaceIndex );			// index of face's bounds to retrieve 

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	/// add a vertex to the list
	void						AddVertex( const D3DXVECTOR3& Vertex,	// Vertex to add
										   int32 Index = -1 );	// Index to insert at - no argument or -1 simply pushes it onto the end of the list

	/// add a normal to the list
	void						AddNormal( const D3DXVECTOR3& Normal,	// Normal to add
										   int32 Index = -1 );	// Index to insert at - no argument or -1 simply pushes it onto the end of the list

	/// add a vertex cp;pr to the list
	void						AddColor( const D3DXVECTOR3& Color,	// Color to add RGB range of (0-1)
										  int32 Index = -1 );	// Index to insert at - no argument or -1 simply pushes it onto the end of the list

	/// add a Face to the list
	void						AddFace( int32 A,			// First index
										 int32 B,			// Second index 
										 int32 C,			// Third index 
										 int32 Index = -1 );// Index to insert at - no argument or -1 simply pushes it onto the end of the list

	/// add a Face to the list
	void						AddFace( int32 A,			// First index
										 int32 B,			// Second index 
										 int32 C,			// Third index 
										 uint32 Material,	// Material applied to the face
										 int32 Index = -1 );// Index to insert at - no argument or -1 simply pushes it onto the end of the list

	void						AddFaceList( CFaceList* pFaceList,				// face list to add
											 const D3DXMATRIX& Transformation,	// transformation of the face list
											 int32 MaterialID = -1 );			// override material ID ( -1 uses existing data )

	void						Optimize( void );

	// the following two methods are not required - but prevent memory thrashing if you
	// already know what will be going into these structures.

	bool						RayCollision( const D3DXVECTOR3& Start,	// start of ray
											  const D3DXVECTOR3& End,	// endpoint of ray
											  D3DXVECTOR3& ImpactPoint,	// impact point to be filled
											  D3DXVECTOR3& ImpactNormal,// impact normal to be filled
											  float32& BestDistance );	// distance from start point

	bool						RayCollision( const D3DXVECTOR3& Start,	// start of ray
											  const D3DXVECTOR3& End,	// endpoint of ray
											  D3DXVECTOR3& ImpactPoint,	// impact point to be filled
											  D3DXVECTOR3& ImpactNormal,// impact normal to be filled
			 								  uint32& FaceMaterial,			// material of the impacted face
											  float32& BestDistance );	// distance from start point

	bool						RayCollision( const D3DXVECTOR3& Start,		// start of ray 
											  const D3DXVECTOR3& End,		// endpoint of ray 
											  const D3DXVECTOR3& MinBounds,// min bounds of collision ray
											  const D3DXVECTOR3& MaxBounds,// max bounds of collision ray
											  D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
											  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
											  float32& BestDistance );		// distance from start point

	bool						RayCollision( const D3DXVECTOR3& Start,		// start of ray 
											  const D3DXVECTOR3& End,		// endpoint of ray 
											  const D3DXVECTOR3& MinBounds,// min bounds of collision ray
											  const D3DXVECTOR3& MaxBounds,// max bounds of collision ray
											  D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
											  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
			 								  uint32& FaceMaterial,			// material of the impacted face
											  D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
											  float32& BestDistance );		// distance from start point

	bool						RayCollision( const std::vector< uint32 >& pFaceList,	// list of face indices to collide with
											  const D3DXVECTOR3& Start,		// start of ray 
											  const D3DXVECTOR3& End,		// endpoint of ray 
											  const D3DXVECTOR3& MinBounds,// min bounds of collision ray
											  const D3DXVECTOR3& MaxBounds,// max bounds of collision ray
											  D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
											  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
			 								  uint32& FaceMaterial,			// material of the impacted face
											  D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
											  float32& BestDistance );		// distance from start point



	bool						SphereCollision( const D3DXVECTOR3& Start,	// start of ray 
												 const D3DXVECTOR3& End,	// endpoint of ray 
												 float32 Radius,			// radius of sphere
												 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
												 D3DXVECTOR3& ImpactPoint,	// impact point to be filled 
												 D3DXVECTOR3& ImpactNormal,	// impact normal to be filled 
												 float32& BestDistance );		// distance from start point

	bool						SphereCollision( const D3DXVECTOR3& Start,	// start of ray 
												 const D3DXVECTOR3& End,	// endpoint of ray 
												 float32 Radius,			// radius of sphere
												 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
												 D3DXVECTOR3& ImpactPoint,	// impact point to be filled 
												 D3DXVECTOR3& ImpactNormal,	// impact normal to be filled 
				 								 uint32& FaceMaterial,			// material of the impacted face
												 float32& BestDistance );		// distance from start point

	bool						SphereCollision( const D3DXVECTOR3& Start,		// start of ray 
												 const D3DXVECTOR3& End,		// endpoint of ray 
												 const D3DXVECTOR3& MinBounds,	// min bounds of collision ray 
												 const D3DXVECTOR3& MaxBounds,	// max bounds of collision ray
												 float32 Radius,				// radius of sphere
												 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
												 D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
												 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled 
												 float32& BestDistance );			// distance from start point

	bool						SphereCollision( const D3DXVECTOR3& Start,		// start of ray 
												 const D3DXVECTOR3& End,		// endpoint of ray 
												 const D3DXVECTOR3& MinBounds,	// min bounds of collision ray 
												 const D3DXVECTOR3& MaxBounds,	// max bounds of collision ray
												 float32 Radius,				// radius of sphere
												 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
												 D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
												 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled 
				 								 uint32& FaceMaterial,			// material of the impacted face
												 D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
												 float32& BestDistance );			// distance from start point

	bool						SphereCollision( const std::vector< uint32 >& pFaceList,	// list of face indices to collide with
												 const D3DXVECTOR3& Start,		// start of ray 
												 const D3DXVECTOR3& End,		// endpoint of ray 
												 const D3DXVECTOR3& MinBounds,	// min bounds of collision ray 
												 const D3DXVECTOR3& MaxBounds,	// max bounds of collision ray
												 float32 Radius,				// radius of sphere
												 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
												 D3DXVECTOR3& ImpactPoint,		// impact point to be filled 
												 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled 
				 								 uint32& FaceMaterial,			// material of the impacted face
												 D3DXVECTOR3& AverageColor,		// averaged color of the vertices of the impacted face
												 float32& BestDistance );			// distance from start point


private:

	D3DXVECTOR3					m_MaxMeshBoundsVEC3D;		/// the local max bounds of the entire mesh
	D3DXVECTOR3					m_MinMeshBoundsVEC3D;		/// the local min bounds of the entire mesh



	std::vector <D3DXVECTOR3>	m_pMaxFaceBoundsVEC3D;		/// the local max bounds of the each face
	std::vector <D3DXVECTOR3>	m_pMinFaceBoundsVEC3D;		/// the local min bounds of the each face

	std::vector <D3DXVECTOR3>	m_pVertices;
	std::vector <D3DXVECTOR3>	m_pColors;					/// RGB with range of 0-1
	std::vector <D3DXVECTOR3>	m_pNormals;
	std::vector <uint32>		m_pFaceA;
	std::vector <uint32>		m_pFaceB;
	std::vector <uint32>		m_pFaceC;
	std::vector	<uint32>		m_pFaceMaterial;

	/// axis of the normal - KNormalMaxX, KNormalMaxY, KNormalMaxZ
	/// for use in optimized bounds checking

	/// Determines whether a point is in the bounds of a face by index
	bool						PointIn( const D3DXVECTOR3& Point,	// point to compare
										 int32 FaceIndex );			// face to check									   // index of face to compare against
};


#endif