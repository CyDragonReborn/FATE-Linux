/*
	collisionscene.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _COLLISIONSCENE_H
#define _COLLISIONSCENE_H

#include "../MEDIA/materialconstants.h"
#include "../UTILITIES/octree.h"
#include "../UTILITIES/quadtree.h"
#include "../OBJECTS/datapoint.h"
#include "../OBJECTS/datalight.h"
#include "../OBJECTS/path.h"

// forward declarations
class	CFaceList;



/// basic model - comprised of submodels, which contain vertex buffers, and submodelsegments,
/// which have only one material, and contain the index buffers that reference the parent
/// submodel's vertex buffers. Clear? No? Oh well...
class CCollisionScene
{
public:

			CCollisionScene( void );	
			~CCollisionScene( void );

	void	Load( std::string FileName,					// path to the mesh file
				  ESortType SortType = KSortQuadtree );	// type of sorting to use (quadtree by default)
	
	void	SortForCollision( const D3DXVECTOR3& MinBounds,		// min bounds of area to sort
							  const D3DXVECTOR3& MaxBounds );	// max bounds of area to sort

	bool	RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
						  const D3DXVECTOR3& End,		// end of collision ray
						  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
						  D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

	bool	SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							 const D3DXVECTOR3& End,		// end of collision ray
							 float32 Radius,				// radius of sphere to check along collision ray
							 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							 D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

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

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	CFaceList*	FaceList( void )						{	return m_pFaceList;		};

	uint32		GetPathCount( void )					{	return m_Paths.size();	};
	uint32		GetPointCount( void )					{	return m_Points.size();	};
	uint32		GetLightCount( void )					{	return m_Lights.size();	};

	CPath&		GetPath( uint32 Index );	// index of path to retrieve
	CDataPoint&	GetPoint( uint32 Index );	// index of point to retrieve
	CDataLight&	GetLight( uint32 Index );	// index of light to retrieve

	const D3DXVECTOR3&		GetMinBounds( void )	{	return m_LocalMinBounds;	};
	const D3DXVECTOR3&		GetMaxBounds( void )	{	return m_LocalMaxBounds;	};

	DWORD		GetFogColor( void )					{	return m_FogColor;		};
	DWORD		GetAmbientColor( void )				{	return m_AmbientColor;	};

	float32		GetAmbientR( void )				{	return m_AmbientR;	};
	float32		GetAmbientG( void )				{	return m_AmbientG;	};
	float32		GetAmbientB( void )				{	return m_AmbientB;	};

	uint32		Materials( void )					{	return m_MaterialNames.size();	};

	const std::string& MaterialName( uint32 ID )	{	return m_MaterialNames[ID];		};

private:
	std::vector< CDataPoint* >						m_Points;		// list of points loaded from the file
	std::vector< CPath* >							m_Paths;		// list of paths(splines) loaded from the file
	std::vector< CDataLight* >						m_Lights;		// list of lights loaded from the file

	COctree< uint32 >*								m_pOctree;		// octree for use in collision sorting
	CQuadtree< uint32 >*							m_pQuadtree;	// quadtree for use in collision sorting

	D3DXVECTOR3										m_LocalMaxBounds;	// min bounds of the entire mesh
	D3DXVECTOR3										m_LocalMinBounds;	// max bounds of the entire mesh
	
	uint32											m_TotalMaterials;	// total materials referenced by the mes
	uint32											m_TotalFaces;
	uint32											m_TotalSubMeshes;	

	DWORD											m_FogColor;			// fog color loaded from file
	DWORD											m_AmbientColor;		// ambient scene color loaded from file

	float32											m_AmbientR;
	float32											m_AmbientG;
	float32											m_AmbientB;

	std::vector< std::string >						m_MaterialNames;	// list of material names

	CFaceList*										m_pFaceList;		// facelist object containing all faces and verts

	std::vector < uint32 >							m_SortedFaces;		// a list of indices to all faces that 
																		// are currently sorted for collision
};

#endif