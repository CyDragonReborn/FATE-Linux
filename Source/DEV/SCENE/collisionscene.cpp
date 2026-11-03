/*
	scene.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History :

	Travis Baldree
	4/14/2004	-	changed return type for lights/points/paths from pointers to refs
	
	Travis Baldree
	4/16/2004	-	Added support for point orientations

	Travis Baldree
	5/27/2004	-	Nasty bug! erroneous faces were being added at load time! 

*/

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include "../UTILITIES/octree.h"
#include "../UTILITIES/quadtree.h"

#include "collisionscene.h"

#include "../MODEL/facelist.h"





CCollisionScene::CCollisionScene( void ) :
											m_TotalSubMeshes( 0 ),
											m_TotalFaces( 0 ),
											m_TotalMaterials( 0 ),
											m_pOctree( NULL ),
											m_pQuadtree( NULL ),
											m_pFaceList( NULL ),
											m_FogColor ( 0 ),
											m_AmbientColor ( 0 ),
											m_AmbientR ( 0.0f ),
											m_AmbientG ( 0.0f ),
											m_AmbientB ( 0.0f ),
											m_LocalMinBounds ( 0, 0, 0 ),
											m_LocalMaxBounds ( 0, 0, 0 )
{
} // CCollisionScene::CCollisionScene()


CCollisionScene::~CCollisionScene()
{
	DELETE_SAFELY( m_pOctree );
	DELETE_SAFELY( m_pQuadtree );
	
	DELETE_SAFELY( m_pFaceList );

	for( uint32 i =0; i < m_Lights.size(); i++ )
	{
		DELETE_SAFELY( m_Lights[i] );
	}
	m_Lights.clear();

	for( i =0; i < m_Paths.size(); i++ )
	{
		DELETE_SAFELY( m_Paths[i] );
	}
	m_Paths.clear();

	for( i =0; i < m_Points.size(); i++ )
	{
		DELETE_SAFELY( m_Points[i] );
	}
	m_Points.clear();


} // CCollisionScene::~CCollisionScene()

void CCollisionScene::Load( std::string FileName,			// path to the mesh file
						    ESortType SortType )			// type of sorting to use (quadtree by default)
						   
{

	m_TotalFaces = 0;

	DELETE_SAFELY( m_pFaceList );


	FILE*	File	= NULL;

	File = fopen( FileName.c_str(), "rb" );

	if ( File == NULL )
	{
		return;
	}


	bool	BoundsSet		( kFalse );
	int32	MeshVertices	( 0 );



	std::vector <uint32>	MaterialCounts			( KMaterialTypes );

	for( uint32 i = 0; i < MaterialCounts.size(); i++ )
	{
		MaterialCounts[i] = 0;
	}

	float32 FogR = ReadFloat( File );
	float32 FogG = ReadFloat( File );
	float32 FogB = ReadFloat( File );

	m_FogColor = D3DCOLOR_ARGB( 0,
							    (int32) ( FogR * 255 ),
							    (int32) ( FogG * 255 ),
							    (int32) ( FogB * 255 ) );

	m_AmbientR = ReadFloat( File );
	m_AmbientG = ReadFloat( File );
	m_AmbientB = ReadFloat( File );

	m_AmbientColor = D3DCOLOR_ARGB( 0,
									(int32) ( m_AmbientR * 255 ),
								    (int32) ( m_AmbientG * 255 ),
								    (int32) ( m_AmbientB * 255 ) );

	// first block of the model file - basic info about the mesh
	float32	SceneScale			( ReadFloat( File ) );				// scale of the entire model
	uint32	Objects				( ReadInt( File ) );				// how many sub-objects are in the model
	uint32	Lights				( ReadInt( File ) );				// how many lights are in the model
	uint32	Points				( ReadInt( File ) );				// how many glows are in the model
	uint32	Paths				( ReadInt( File ) );				// how many lights are in the model
	uint32	TotalMaterials		( ReadInt( File ) );				// how many individual materials are applied

	m_TotalMaterials = TotalMaterials;

	uint32	m_Materials			( m_TotalMaterials );

	m_MaterialNames.resize( m_TotalMaterials );

	// load the material ids into their indices
	for( i = 0; i < TotalMaterials; i++ )
	{
		m_MaterialNames[i]	= ReadString( File );

	}

	if ( m_TotalMaterials <= 0 )
	{
		m_TotalMaterials = 1;
	}

	// we keep track of the smallest objects, 
	// so that we know how far down to go in our octree/quadtree
	float32 SmallestFace = 1000;


	m_pFaceList = new CFaceList();

	uint32 VerticesLoaded( 0 );	// need to keep track of how many verts parsed
								// so that we can keep triangle refs in sync

	// now parse the individual sub-models within the file
	for ( uint32 j = 0; j < Objects; j ++ )
	{

		// parse the pivot location of the mesh
		// making sure to scale it
		float32	PivotX	= ReadFloat( File ) * SceneScale;
		float32	PivotY	= ReadFloat( File ) * SceneScale;
		float32	PivotZ	= ReadFloat( File ) * SceneScale;


		int32		a, b, c;
		float32		x, y, z;
		D3DXVECTOR3	MinBounds		( -1, -1, -1 );
		D3DXVECTOR3	MaxBounds		( -1, -1, -1 );		

		// parse the total vertex count
		uint32		TotalVertices	= ReadInt( File );

		MeshVertices += TotalVertices;

		// load the vertex indices
		std::vector <uint32>	VertexReferences	( TotalVertices );

		for ( uint32 k = 0; k < TotalVertices; k++ )
		{
			VertexReferences[k] = ReadInt( File );
		}

		uint32						ActualVertices	= ReadInt( File );

		std::vector <D3DXVECTOR3>	Vertices		( ActualVertices );

		// parse the actual vertex values 
		for ( k = 0; k < ActualVertices; k++ )
		{
			x = PivotX + ReadFloat( File ) * SceneScale;
			y = PivotY + ReadFloat( File ) * SceneScale;
			z = PivotZ + ReadFloat( File ) * SceneScale;

			Vertices[k] = D3DXVECTOR3( x, y, z );

			if ( MinBounds.x == -1 && MaxBounds.z == -1 )
			{
				MinBounds = Vertices[k];
				MaxBounds = Vertices[k];
			}
			else
			{
				ExpandBounds( MinBounds, MaxBounds, Vertices[k] );
			}
		}

		for ( k = 0; k < TotalVertices; k++ )
		{
			m_pFaceList->AddVertex( Vertices[VertexReferences[k]] );
		}

		// parse vertex colors
		uint32						ActualVertexColors	= ReadInt( File );

		for ( k = 0; k < TotalVertices; k++ )
		{
			if ( k >= ActualVertexColors )
			{
				m_pFaceList->AddColor( D3DXVECTOR3( 1, 1, 1 ) );
			}
			else
			{
				x = ReadShort( File );
				y = ReadShort( File );
				z = ReadShort( File );

				if ( x > 255 )
				{
					x = 255;
				}
				if ( x < 0 )
				{
					x = 0;
				}
				if ( y > 255 )
				{
					y = 255;
				}
				if ( y < 0 )
				{
					y = 0;
				}
				if ( z > 255 )
				{
					z = 255;
				}
				if ( z < 0 )
				{
					z = 0;
				}

				m_pFaceList->AddColor( D3DXVECTOR3( x / 255.0f, 
													y / 255.0f, 
													z / 255.0f ) );
			}
		}

		// parse the number of triangles in the whole mesh
		uint32	TotalTriangles		= ReadInt( File );
		// and the number of submodelsegments in this object
		uint32	SubSceneSegments	= ReadInt( File );


		if ( !BoundsSet )
		{
			BoundsSet = kTrue;
			m_LocalMinBounds = MinBounds;
			m_LocalMaxBounds = MaxBounds;
		}

		// constantly expand the bounding box to encompass these new subobjects
		ExpandBounds( m_LocalMinBounds, m_LocalMaxBounds, MinBounds );
		ExpandBounds( m_LocalMinBounds, m_LocalMaxBounds, MaxBounds );


		// parse each sub-mesh
		for ( uint32 m = 0; m < SubSceneSegments; m++ )
		{
			// parse the material ID of this submesh (each submesh only has 1)
			uint32	MaterialID	= ReadShort( File );
			// make sure its valid
			if ( MaterialID >= m_TotalMaterials )
			{
				MaterialID = m_TotalMaterials - 1;
			}

			uint32	ActualTriangles	= ReadInt( File );

			m_TotalFaces += ActualTriangles;

			// parse the actual triangle indices for each corner vert
			for ( uint32 k = 0 ; k < ActualTriangles; k++ )
			{
				a = ReadInt( File );
				b = ReadInt( File );
				c = ReadInt( File );

				m_pFaceList->AddFace( c + VerticesLoaded, 
									  b + VerticesLoaded, 
									  a + VerticesLoaded,
									  MaterialID );


				float32 AxisDiagonal = D3DXVec3Length( &D3DXVECTOR3( m_pFaceList->GetVertex( a + VerticesLoaded ) -
																	 m_pFaceList->GetVertex( b + VerticesLoaded ) ) );
				
				if( AxisDiagonal < SmallestFace )
				{
					SmallestFace = AxisDiagonal;
				}
			
			}
			m_TotalSubMeshes++;

		}

		VerticesLoaded += TotalVertices;
	}

	// read all light data in the file - a type, position, quaternion (which we'll convert to a ray )
	// and a color
	for( i = 0; i < Lights; i ++ )
	{
		std::string Name = ReadString( File );

		ELightType Type = (ELightType)ReadShort(File);

		float32 X = ReadFloat( File ) * SceneScale;
		float32 Y = ReadFloat( File ) * SceneScale;
		float32 Z = ReadFloat( File ) * SceneScale;
		
			
		D3DXMATRIX Orientation;
		D3DXMatrixIdentity( &Orientation );

		Orientation._11 = ReadFloat( File );
		Orientation._12 = ReadFloat( File );
		Orientation._13 = ReadFloat( File );

		Orientation._21 = ReadFloat( File );
		Orientation._22 = ReadFloat( File );
		Orientation._23 = ReadFloat( File );

		Orientation._31 = ReadFloat( File );
		Orientation._32 = ReadFloat( File );
		Orientation._33 = ReadFloat( File );
	
		float32 R = ReadFloat( File );
		float32 G = ReadFloat( File );
		float32 B = ReadFloat( File );

		float32 Range = ReadFloat( File );

		m_Lights.push_back( new CDataLight( Name, 
											Type,
											Range,
											D3DXVECTOR3( X, Y, Z ),
											D3DXVECTOR3( Orientation._31, Orientation._32, Orientation._33 ),
											D3DXVECTOR3( R, G, B ) ) );

		m_Lights.back()->D3DLight().Range = Range;

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Lights.back()->SetUserData( ReadString( File ) );
		}

	}

	// read all the points in the file - each has a position and location
	for( i = 0; i < Points; i ++ )
	{
		std::string Name = ReadString( File );

		float32	X = ReadFloat( File ) * SceneScale;
		float32	Y = ReadFloat( File ) * SceneScale;
		float32	Z = ReadFloat( File ) * SceneScale;

		D3DXMATRIX Orientation;
		D3DXMatrixIdentity( &Orientation );

		Orientation._11 = ReadFloat( File );
		Orientation._12 = ReadFloat( File );
		Orientation._13 = ReadFloat( File );

		Orientation._21 = ReadFloat( File );
		Orientation._22 = ReadFloat( File );
		Orientation._23 = ReadFloat( File );

		Orientation._31 = ReadFloat( File );
		Orientation._32 = ReadFloat( File );
		Orientation._33 = ReadFloat( File );

		m_Points.push_back( new CDataPoint( Name, D3DXVECTOR3( X, Y, Z ), Orientation ) );

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Points.back()->SetUserData( ReadString( File ) );
		}

	}


	// read all the paths in the file - each has a position and location
	for( i = 0; i < Paths; i ++ )
	{
		std::string Name = ReadString( File );

		float32	X = ReadFloat( File ) * SceneScale;
		float32	Y = ReadFloat( File ) * SceneScale;
		float32	Z = ReadFloat( File ) * SceneScale;

		uint32 SubPaths = ReadShort( File );

		for( uint32 k = 0; k < SubPaths; k++ )
		{
			uint32 Points	= ReadShort( File );
			bool Closed		= ReadBool( File);

			if( k == 0 )
			{
				m_Paths.push_back( new CPath( Name, Closed, D3DXVECTOR3( X, Y, Z ) ) );
			}

			for( uint32 l = 0; l < Points; l++ )
			{
				float32 PX = ReadFloat( File ) * SceneScale;
				float32 PY = ReadFloat( File ) * SceneScale;
				float32 PZ = ReadFloat( File ) * SceneScale;

				if( k == 0 )
				{
					m_Paths.back()->AddPoint( D3DXVECTOR3( PX, PY, PZ ) );
				}
			}
			if( Closed )
			{
				m_Paths.back()->ClosePath();
			}
		}

		uint32 HasUserData = ReadShort( File );

		if( HasUserData > 0 )
		{
			m_Paths.back()->SetUserData( ReadString( File ) );
		}

	}


	fclose( File );

	// do all setup calculations for this collision mesh
	m_pFaceList->CalculateFaceBounds();					
	m_pFaceList->CalculateNormals();


	float32 BoundDiagonal = D3DXVec3Length( &( m_LocalMinBounds - m_LocalMaxBounds ) );
	BoundDiagonal /= 1000.0f;
	if( SmallestFace < BoundDiagonal )
	{
		SmallestFace = BoundDiagonal;
	}

	// if we requested an octree
	if( SortType == KSortOctree )
	{
		m_pOctree = new COctree< uint32 >( m_LocalMinBounds,
										   m_LocalMaxBounds,
										   SmallestFace);

		// sort face data into an octree
		for( i = 0; i < m_pFaceList->Faces() ; i++ )
		{
			m_pOctree->AddData( i, 
								m_pFaceList->GetMinFaceBounds( i ),
								m_pFaceList->GetMaxFaceBounds( i ) );
		}
	}
	else
	{
		m_pQuadtree = new CQuadtree< uint32 >( m_LocalMinBounds,
											   m_LocalMaxBounds,
											   SmallestFace);

		// sort face data into a quadtree
		for( i = 0; i < m_pFaceList->Faces() ; i++ )
		{
			m_pQuadtree->AddData( i, 
								  m_pFaceList->GetMinFaceBounds( i ),
								  m_pFaceList->GetMaxFaceBounds( i ) );
		}
	}

}	// CCollisionScene::Load()


// presorts faces for collision later.
// this is excellent to use if you are going to perform many collisions in a set area
// (for instance, colliding with four points on the ground to place a shadow )
// call SortForCollision, and then either PreSortedRayCollision or PreSortedSphereCollision
// to collide with this data.
void CCollisionScene::SortForCollision( const D3DXVECTOR3& MinBounds,	// min bounds of area to sort
										const D3DXVECTOR3& MaxBounds )	// max bounds of area to sort
{
	m_SortedFaces.resize( 0 );

	if( m_pOctree )
	{
		m_pOctree->Sort( MinBounds,
						 MaxBounds,
						 m_SortedFaces);
	}
	else
	{
		m_pQuadtree->Sort( MinBounds,
						   MaxBounds,
						   m_SortedFaces);
	}

} // CCollisionScene::SortForCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCollisionScene::PreSortedRayCollision( const D3DXVECTOR3& Start,		// start of collision ray
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
} // CCollisionScene::PreSortedRayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCollisionScene::PreSortedSphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
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
} // CCollisionScene::PreSortedSphereCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCollisionScene::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
									const D3DXVECTOR3& End,		// end of collision ray
									D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	D3DXVECTOR3 Color;
	uint32 Material;
	return RayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color );
} // CCollisionScene::RayCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCollisionScene::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
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

	std::vector < uint32 >	pSortedFaces;
	pSortedFaces.reserve( 500 );

	if( m_pOctree )
	{
		m_pOctree->Sort( MinBounds,
						 MaxBounds,
						 pSortedFaces);
	}
	else
	{
		m_pQuadtree->Sort( MinBounds,
						   MaxBounds,
						   pSortedFaces);
	}

	m_pFaceList->RayCollision( pSortedFaces,
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
} // CCollisionScene::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCollisionScene::SphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
									   const D3DXVECTOR3& End,		// end of collision ray
									   float32 Radius,				// radius of sphere to check along collision ray
									   D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									   D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	D3DXVECTOR3 Color;
	uint32 Material;
	return SphereCollision( Start, 
							End, 
							Radius, 
							FinalPosition, 
							ImpactPoint, 
							ImpactNormal, 
							Material, 
							Color );

} // CCollisionScene::SphereCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCollisionScene::SphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
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

	std::vector < uint32 >	pSortedFaces;
	pSortedFaces.reserve( 500 );

	if( m_pOctree )
	{
		m_pOctree->Sort( MinBounds,
						 MaxBounds,
						 pSortedFaces);
	}
	else
	{
		m_pQuadtree->Sort( MinBounds,
						   MaxBounds,
						   pSortedFaces);
	}


	m_pFaceList->SphereCollision( pSortedFaces,
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
} // CCollisionScene::SphereCollision()

CPath&	CCollisionScene::GetPath( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Paths.size() );

	return *m_Paths[Index];
} // CCollisionScene::GetPath()

CDataPoint&	CCollisionScene::GetPoint( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Points.size() );

	return *m_Points[Index];

} // CCollisionScene::GetPoint()

CDataLight&	CCollisionScene::GetLight( uint32 Index )		// Index of point to retrieve
{
	assert( Index < m_Lights.size() );

	return *m_Lights[Index];

} // CCollisionScene::GetLight()

