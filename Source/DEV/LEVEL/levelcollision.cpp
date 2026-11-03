/*
	levelcollision.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/quadtree.h"


#include "../MODEL/facelist.h"

#include "level.h"
#include "maze.h"
#include "levelcollision.h"

CLevelCollision::CLevelCollision( void ) :
											m_CollisionQuadTree( NULL ),
											m_pFaceList( NULL ),
											m_MinBounds ( 0, 0, 0 ),
											m_MaxBounds ( 0, 0, 0 ),
											m_TileWidth ( 0 ),
											m_TileHeight ( 0 )
{
} // CLevelCollision::CLevelCollision()

CLevelCollision::~CLevelCollision( void )
{
	DELETE_SAFELY( m_pFaceList );
	DELETE_SAFELY( m_CollisionQuadTree );
} // CLevelCollision::~CLevelCollision()

void CLevelCollision::Create( CLevel& pLevel,		// level containing peripheral data
							  CMaze& pMaze,			// maze to retrieve height information from
							  float32 TileWidth,	// width of tiles
							  float32 TileHeight )	// height of tiles
{
	m_TileWidth = TileWidth;
	m_TileHeight = TileHeight;
	m_pFaceList = new CFaceList();

	CalculateGroundCollision( pLevel, pMaze );


} // CLevelCollision::Create()

void CLevelCollision::CreateForScene( void )
{
	m_pFaceList = new CFaceList();

} // CLevelCollision::Create()

void CLevelCollision::AddFaceList( CFaceList* pFaceList,				// face list to add
								   const D3DXMATRIX& Transformation,	// transformation of the face list
								   int32 MaterialID )					// override material ID ( -1 uses existing data )
{
	m_pFaceList->AddFaceList( pFaceList,
							  Transformation,
							  MaterialID );
} // CLevelCollision::AddFaceList()

void CLevelCollision::Finalize( void )
{
	m_pFaceList->CalculateFaceBounds();					
	m_pFaceList->CalculateNormals();

	DELETE_SAFELY( m_CollisionQuadTree );

	m_CollisionQuadTree = new CQuadtree< uint32 >( m_MinBounds,
												   m_MaxBounds,
												   1 );

	// sort face data into a quadtree
	for( uint32 i = 0; i < m_pFaceList->Faces() ; i++ )
	{
		m_CollisionQuadTree->AddData( i, 
									  m_pFaceList->GetMinFaceBounds( i ),
									  m_pFaceList->GetMaxFaceBounds( i ) );
	}

} // CLevelCollision::Finalize()

void CLevelCollision::CalculateGroundCollision( CLevel& pLevel,		// level containing peripheral data
												CMaze& pMaze )		// maze to retrieve height information from
{
	uint32 FaceIndex( 0 );
	for( uint32 y = 0; y < pMaze.Height(); y++ )
	{
		for( uint32 x = 0; x < pMaze.Width(); x++ )
		{
			D3DXVECTOR3 Position( (float32)x * m_TileWidth, 
								  0,//m_TileHeight * pMaze.CellHeight( x, y ), 
								  (float32)y * -m_TileWidth );

			EWallType WallType = KWTypeOpen;
			float32 TopLeftHeight = m_TileHeight * (float32)pMaze.CellHeight( x, y );

			WallType = pMaze.WallTypeNW( x, y );
			if( WallType != KWTypeOpen &&
				WallType != KWTypeNWSolid )
			{
				TopLeftHeight -= m_TileHeight;
			}

			float32 TopRightHeight = m_TileHeight * (float32)pMaze.CellHeight( x, y );

			WallType = pMaze.WallTypeNE( x, y );
			if( WallType != KWTypeOpen &&
				WallType != KWTypeNESolid )
			{
				TopRightHeight -= m_TileHeight;
			}

			float32 BottomLeftHeight = m_TileHeight * (float32)pMaze.CellHeight( x, y );

			WallType = pMaze.WallTypeSW( x, y );
			if( WallType != KWTypeOpen &&
				WallType != KWTypeSWSolid )
			{
				BottomLeftHeight -= m_TileHeight;
			}

			float32 BottomRightHeight = m_TileHeight * (float32)pMaze.CellHeight( x, y );

			WallType = pMaze.WallTypeSE( x, y );
			if( WallType != KWTypeOpen &&
				WallType != KWTypeSESolid )
			{
				BottomRightHeight -= m_TileHeight;
			}
			uint32 MaterialTopLeft = KFloorMaterialID;
			uint32 MaterialTopRight = KFloorMaterialID;
			uint32 MaterialBottomLeft = KFloorMaterialID;
			uint32 MaterialBottomRight = KFloorMaterialID;


			if( BottomLeftHeight < 0 )
			{
				MaterialBottomLeft = KImpassableMaterialID;
				BottomLeftHeight = -1.0f;
			}
			if( BottomRightHeight < 0 )
			{
				MaterialBottomRight = KImpassableMaterialID;
				BottomRightHeight = -1.0f;
			}
			if( TopLeftHeight < 0 )
			{
				MaterialTopLeft = KImpassableMaterialID;
				TopLeftHeight = -1.0f;
			}
			if( TopRightHeight < 0 )
			{
				MaterialTopRight = KImpassableMaterialID;
				TopRightHeight = -1.0f;
			}


			D3DXVECTOR3 Vertex;

			// top bounds

			Vertex = Position;
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			if( x == 0 && y == 0 )
			{
				m_MinBounds = Vertex;
				m_MaxBounds = Vertex;
			}
			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, 0 );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, 0 );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth, 0, 0 );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );
// center

			Vertex = Position + D3DXVECTOR3( 0, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + TopRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

// center2
			Vertex = Position + D3DXVECTOR3( 0, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth, 0, m_TileWidth * -.5f );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

// bottom
			Vertex = Position + D3DXVECTOR3( 0, 0, -m_TileWidth );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, -m_TileWidth );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomLeftHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, -m_TileWidth );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );

			Vertex = Position + D3DXVECTOR3( m_TileWidth, 0, -m_TileWidth );
			Vertex.y += pLevel.FloorHeight( Vertex ) + BottomRightHeight;
			Vertex.x += pLevel.FloorOffsetX( Vertex );
			Vertex.z += pLevel.FloorOffsetY( Vertex );
			m_pFaceList->AddVertex( Vertex );

			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );


			if( TopLeftHeight != 0 )
			{
				MaterialTopLeft = KImpassableMaterialID;
			}
			if( TopRightHeight != 0 )
			{
				MaterialTopRight = KImpassableMaterialID;
			}
			if( BottomLeftHeight != 0 )
			{
				MaterialBottomLeft = KImpassableMaterialID;
			}
			if( BottomRightHeight != 0 )
			{
				MaterialBottomRight = KImpassableMaterialID;
			}

			m_pFaceList->AddFace( FaceIndex + 4, 
								  FaceIndex + 5, 
								  FaceIndex,
								  MaterialTopLeft,
								  -1 );

			m_pFaceList->AddFace( FaceIndex + 5, 
								  FaceIndex + 1, 
								  FaceIndex,
								  MaterialTopLeft,
								  -1  );

			m_pFaceList->AddFace( FaceIndex + 6, 
								  FaceIndex + 3, 
								  FaceIndex + 2,
								  MaterialTopRight,
								  -1  );

			m_pFaceList->AddFace( FaceIndex + 6, 
								  FaceIndex + 7, 
								  FaceIndex + 3,
								  MaterialTopRight,
								  -1  );

			m_pFaceList->AddFace( FaceIndex + 12, 
								  FaceIndex + 9, 
								  FaceIndex + 8,
								  MaterialBottomLeft,
								  -1  );

			m_pFaceList->AddFace( FaceIndex + 13, 
								  FaceIndex + 9, 
								  FaceIndex + 12,
								  MaterialBottomLeft,
								  -1  );

			m_pFaceList->AddFace( FaceIndex + 14, 
								  FaceIndex + 15, 
								  FaceIndex + 10,
								  MaterialBottomRight,
								  -1  );

			m_pFaceList->AddFace( FaceIndex + 15, 
								  FaceIndex + 11, 
								  FaceIndex + 10,
								  MaterialBottomRight,
								  -1  );

			FaceIndex+=16;

		}
	}

/*	for( y = 0; y < pMaze.Height(); y++ )
	{
		for( uint32 x = 0; x < pMaze.Width(); x++ )
		{
		}
	}*/

} // CLevelCollision::CalculateGroundCollision()


// presorts faces for collision later.
// this is excellent to use if you are going to perform many collisions in a set area
// (for instance, colliding with four points on the ground to place a shadow )
// call SortForCollision, and then either PreSortedRayCollision or PreSortedSphereCollision
// to collide with this data.
void CLevelCollision::SortForCollision( const D3DXVECTOR3& MinBounds,	// min bounds of area to sort
							  const D3DXVECTOR3& MaxBounds )	// max bounds of area to sort
{
	m_SortedFaces.resize( 0 );

	m_CollisionQuadTree->Sort( MinBounds,
							   MaxBounds,
							   m_SortedFaces);

} // CLevelCollision::SortForCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevelCollision::PreSortedRayCollision( const D3DXVECTOR3& Start,		// start of collision ray
											 const D3DXVECTOR3& End,		// end of collision ray
											 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
											 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
											 uint32& FaceMaterial,			// material id of the impacted face
											 D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );


	ExpandBounds( MinBounds, MaxBounds, End );

	m_pFaceList->RayCollision( m_SortedFaces,
							   Start,
							   End,
							   MinBounds,
							   MaxBounds,
							   ImpactPoint,
							   ImpactNormal,
							   FaceMaterial,
							   AverageColor,
							   BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CLevelCollision::PreSortedRayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevelCollision::PreSortedSphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
												const D3DXVECTOR3& End,		// end of collision ray
												float32 Radius,				// radius of sphere to check along collision ray
												D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
												D3DXVECTOR3& ImpactPoint,	// impact point to be filled
												D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
												uint32& FaceMaterial,		// material id of the impacted face
												D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );

	ExpandBounds( MinBounds, MaxBounds, End );
 

	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;

	m_pFaceList->SphereCollision( m_SortedFaces,
								  Start,
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

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CLevelCollision::PreSortedSphereCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevelCollision::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
									const D3DXVECTOR3& End,		// end of collision ray
									D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									uint32& FaceMaterial,		// material id of the impacted face
								    D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );


	ExpandBounds( MinBounds, MaxBounds, End );

	m_LocalSortedFaces.resize( 0 );

	m_CollisionQuadTree->Sort( MinBounds,
							   MaxBounds,
							   m_LocalSortedFaces);

	m_pFaceList->RayCollision( m_LocalSortedFaces,
							   Start,
							   End,
							   MinBounds,
							   MaxBounds,
							   ImpactPoint,
							   ImpactNormal,
							   FaceMaterial,
							   AverageColor,
							   BestDistance );

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CLevelCollision::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevelCollision::SphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
									   const D3DXVECTOR3& End,		// end of collision ray
									   float32 Radius,				// radius of sphere to check along collision ray
									   D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									   uint32& FaceMaterial,		// material id of the impacted face
									   D3DXVECTOR3& AverageColor )	// averaged colors of the face's verts
{
	float32		BestDistance	( KCollisionInvalid );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );

	ExpandBounds( MinBounds, MaxBounds, End );
 

	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;

	m_LocalSortedFaces.resize( 0 );

	m_CollisionQuadTree->Sort( MinBounds,
							   MaxBounds,
							   m_LocalSortedFaces );


	m_pFaceList->SphereCollision( m_LocalSortedFaces,
								  Start,
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

	if ( BestDistance != KCollisionInvalid )
	{
		return kTrue;
	}

	return kFalse;
} // CLevelCollision::SphereCollision()
