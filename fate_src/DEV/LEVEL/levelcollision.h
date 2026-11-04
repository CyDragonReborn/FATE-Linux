/*
	levelcollision.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVELCOLLISION_H
#define _LEVELCOLLISION_H

#include "d3dx8_shim.h"

#include "../UTILITIES/quadtree.h"


class CMaze;
class CFaceList;
class CLevel;

// a levelcollision contains the face collision data for an entire
// stitched-together level

class CLevelCollision
{
public:

			CLevelCollision( void );	

			~CLevelCollision( void );
	

	void	CreateForScene( void );

	void	Create( CLevel& pLevel,		// level containing peripheral data
					CMaze& pMaze,			// maze to retrieve height information from
					float32 TileWidth,		// width of tiles
					float32 TileHeight );	// height of tiles

	void	Finalize( void );

	void	AddFaceList( CFaceList* pFaceList,				// face list to add
						 const D3DXMATRIX& Transformation,	// transformation of the face list
						 int32 MaterialID = -1 );			// override material ID ( -1 uses existing data )


	void	SortForCollision( const D3DXVECTOR3& MinBounds,		// min bounds of area to sort
							  const D3DXVECTOR3& MaxBounds );	// max bounds of area to sort


	bool	RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
						  const D3DXVECTOR3& End,		// end of collision ray
						  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
						  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
						  uint32& FaceMaterial,			// material id of the impacted face
						  D3DXVECTOR3& AverageColor );	// averaged colors of the face's verts

	bool	SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							 const D3DXVECTOR3& End,		// end of collision ray
							 float32 Radius,				// radius of sphere to check along collision ray
							 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
							 uint32& FaceMaterial,			// material id of the impacted face
							 D3DXVECTOR3& AverageColor );	// averaged colors of the face's verts

	bool	PreSortedRayCollision( const D3DXVECTOR3& Start,	// start of collision ray
								   const D3DXVECTOR3& End,		// end of collision ray
								   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
								   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								   uint32& FaceMaterial,		// material id of the impacted face
								   D3DXVECTOR3& AverageColor );	// averaged colors of the face's verts

	bool	PreSortedSphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
									  const D3DXVECTOR3& End,		// end of collision ray
									  float32 Radius,				// radius of sphere to check along collision ray
									  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
									  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									  uint32& FaceMaterial,			// material id of the impacted face
									  D3DXVECTOR3& AverageColor );	// averaged colors of the face's verts

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	CFaceList*	FaceList( void )		{	return m_pFaceList;		};

private:

	void	CalculateGroundCollision( CLevel& pLevel,		// level containing peripheral data
									  CMaze& pMaze );	// maze to retrieve height information from


	CFaceList*			m_pFaceList;

	CQuadtree< uint32 >*		m_CollisionQuadTree;

	std::vector < uint32 >		m_SortedFaces;			// a list of indices to all faces that 
	std::vector < uint32 >		m_LocalSortedFaces;		// a list of indices to all faces that 

	D3DXVECTOR3			m_MinBounds;
	D3DXVECTOR3			m_MaxBounds;


	float32				m_TileWidth;
	float32				m_TileHeight;

};

#endif