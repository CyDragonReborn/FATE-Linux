/*
	levelmeshpatch.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/cullingbounds.h"

#include "../UTILITIES/vertexdefinitions.h"

#include "../MODEL/facelist.h"
#include "../MEDIA/refmanager.h"

#include "template.h"
#include "level.h"
#include "maze.h"

#include "../GAMEOBJECTS/lightmanager.h"

#include "modeltemplate.h"

#include "levelmeshpatch.h"

CLevelMeshPatch::CLevelMeshPatch( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								  CRefManager& pRefManager,			// pointer to the reference manager, for tracking media
								  CSettings& pSettings,				// pointer to the settings manager, for keeping track of card/machine capabilities
								  const D3DXVECTOR3& Position,  	// top-left corner position
								  int32 TopLeftCellX,				// top left cell index
								  int32 TopLeftCellY,				// top left cell index
								  uint32 GroundLayers,				// number of layers possible for ground textures
								  uint32 WallLayers,				// number of layers possible for walls
								  float32 TileWidth, 				// width of individual tile, in world units
								  float32 TileHeight ) :			// height (tallness) of an individual tile
															m_pRefManager( pRefManager ),
															m_pSettings( pSettings ),
															m_Position( Position ),
															m_GroundLayers( GroundLayers ),
															m_WallLayers( WallLayers ),
															m_TileWidth( TileWidth ),
															m_TileHeight( TileHeight ),
															m_TopLeftCellX( TopLeftCellX ),
															m_TopLeftCellY( TopLeftCellY ),
															m_VisibilityEmpty( kFalse ),
															m_VisibilityFull( kFalse ),
															m_VisibilityVertices( 0 ),
															m_VisibilityIndices( 0 ),
															m_MiniMapVertices( 0 ),
															m_MiniMapIndices( 0 ),
															m_WallVertices( 0 ),
															m_WaterIndices( 0 ),
															m_WaterVertices( 0 ),
															m_CeilingIndices( 0 ),
															m_CeilingVertices( 0 ),
															m_MinBounds( Position ),
															m_MaxBounds( Position ),
															m_pWallVB( NULL ),
															m_pWaterVB( NULL ),
															m_pWaterIB( NULL ),
															m_pCeilingVB( NULL ),
															m_pCeilingIB( NULL ),
															m_pMiniMapVB( NULL ),
															m_pMiniMapIB( NULL ),
															m_pVisibilityVB( NULL ),
															m_pVisibilityIB( NULL ),
															m_VisibilityTilesWide ( 0 ),
															m_VisibilityTilesHigh ( 0 )
{

	m_FloorVertices.resize( m_GroundLayers );
	m_FloorIndices.resize( m_GroundLayers );

	m_pFloorVB.resize( m_GroundLayers );
	m_pFloorIB.resize( m_GroundLayers );

	for( uint32 i = 0; i < m_GroundLayers; i++ )
	{
		m_pFloorVB[i] = NULL;
		m_pFloorIB[i] = NULL;
		m_FloorVertices[i] = 0;
		m_FloorIndices[i] = 0;
	}
	m_CeilingVertices = 0;
	m_CeilingIndices = 0;

	for( uint32 X = 0; X <= KPatchVisibilityTiles; X ++ )
	{
		for( uint32 Y = 0; Y <= KPatchVisibilityTiles; Y ++ )
		{
			m_VisibilityLevel[X][Y] = 255;
			m_FloatVisibilityLevel[X][Y] = 255;
		}
	}

	for( X = 0; X < KPatchTiles+4; X ++ )
	{
		for( uint32 Y = 0; Y < KPatchTiles+4; Y ++ )
		{
			m_MiniMapCornerType[X][Y] = KTTypeOpen;
		}
	}

	m_WallIndices.resize( m_WallLayers );

	m_pWallIB.resize( m_WallLayers );

	for( i = 0; i < m_WallLayers; i++ )
	{
		m_pWallIB[i] = NULL;
		m_WallIndices[i] = 0;
	}

} // CLevelMeshPatch::CLevelMeshPatch()

CLevelMeshPatch::~CLevelMeshPatch( void )
{
	for( uint32 i = 0; i < m_GroundLayers; i++ )
	{
		RELEASE_SAFELY( m_pFloorVB[i] );
		RELEASE_SAFELY( m_pFloorIB[i] );
	}
	RELEASE_SAFELY( m_pWallVB );
	for( i = 0; i < m_WallLayers; i++ )
	{
		RELEASE_SAFELY( m_pWallIB[i] );
	}
	RELEASE_SAFELY( m_pVisibilityVB );
	RELEASE_SAFELY( m_pVisibilityIB );

	RELEASE_SAFELY( m_pCeilingVB );
	RELEASE_SAFELY( m_pCeilingIB );

	RELEASE_SAFELY( m_pWaterVB );
	RELEASE_SAFELY( m_pWaterIB );

	RELEASE_SAFELY( m_pMiniMapVB );
	RELEASE_SAFELY( m_pMiniMapIB );
} // CLevelMeshPatch::~CLevelMeshPatch()

void CLevelMeshPatch::AddWall( CLevel& pLevel,				// parent level
	   						   uint32 Layer,				// wall layer
							   const D3DXVECTOR3& Position,	// position to add at
							   CModelTemplate* pTemplate,	// template to add
							   CModelTemplate* pCollisionTemplate )	// collision template
{
	if( pCollisionTemplate != NULL )
	{
		const std::vector< D3DXVECTOR3 >& CollisionVertices = pCollisionTemplate->Vertices();
		const std::vector< uint32 >& CollisionVertexReferences = pCollisionTemplate->VertexReferences();
		const std::vector< D3DXVECTOR4 >& CollisionTriangles = pCollisionTemplate->Triangles();

		uint32 VertexCount = m_CollisionVertices.size();
		uint32 VertexIndexCount = m_CollisionVertexReferences.size();
		D3DXVECTOR3 FinalVertex;
		int32 PrecisionTrim;
		for( uint32 i = 0; i < CollisionVertices.size(); i++ )
		{
			FinalVertex = Position + CollisionVertices[i];
			// trim some precision from the verts to make sure they weld together nicely
			PrecisionTrim = (int32)floor( FinalVertex.x * 100.0f + .5f );
			FinalVertex.x = (float32)PrecisionTrim / 100.0f;
			PrecisionTrim = (int32)floor( FinalVertex.z * 100.0f + .5f );
			FinalVertex.z = (float32)PrecisionTrim / 100.0f;
			PrecisionTrim = (int32)floor( FinalVertex.y * 100.0f + .5f );
			FinalVertex.y = (float32)PrecisionTrim / 100.0f;

			if( fabs( FinalVertex.y ) < .1f )
			{
				FinalVertex.y += pLevel.FloorHeight( FinalVertex );
			}
			else
			{
				FinalVertex.y += pLevel.FloorHeight( FinalVertex );
			}

			FinalVertex.x += pLevel.FloorOffsetX( FinalVertex );
			FinalVertex.z += pLevel.FloorOffsetY( FinalVertex );

			m_CollisionVertices.push_back( FinalVertex );
		}
		for( i = 0; i <  CollisionVertexReferences.size(); i++ )
		{
			m_CollisionVertexReferences.push_back( CollisionVertexReferences[i] + VertexCount );
		}
		for( i = 0; i < CollisionTriangles.size(); i++ )
		{
			D3DXVECTOR4 Triangle = CollisionTriangles[i];

			Triangle.x += VertexIndexCount;
			Triangle.y += VertexIndexCount;
			Triangle.z += VertexIndexCount;
			Triangle.w = (float32)Layer;
			m_CollisionTriangles.push_back( Triangle );

		}

	}
	const std::vector< D3DXVECTOR3 >& Vertices = pTemplate->Vertices();
	const std::vector< uint32 >& VertexReferences = pTemplate->VertexReferences();
	const std::vector< D3DXVECTOR3 >& UVs = pTemplate->UVs();
	const std::vector< uint32 >& UVReferences = pTemplate->UVReferences();
	const std::vector< D3DXVECTOR3 >& Normals = pTemplate->Normals();
	const std::vector< uint32 >& NormalReferences = pTemplate->NormalReferences();
	const std::vector< D3DXVECTOR4 >& Triangles = pTemplate->Triangles();

	uint32 VertexCount = m_Vertices.size();
	uint32 VertexIndexCount = m_VertexReferences.size();
	uint32 UVCount = m_UVs.size();
	uint32 NormalCount = m_Normals.size();
	D3DXVECTOR3 FinalVertex;
	int32 PrecisionTrim;
	for( uint32 i = 0; i < Vertices.size(); i++ )
	{
		FinalVertex = Position + Vertices[i];
		// trim some precision from the verts to make sure they weld together nicely
		PrecisionTrim = (int32)floor( FinalVertex.x * 100.0f + .5f );
		FinalVertex.x = (float32)PrecisionTrim / 100.0f;
		PrecisionTrim = (int32)floor( FinalVertex.z * 100.0f + .5f );
		FinalVertex.z = (float32)PrecisionTrim / 100.0f;
		PrecisionTrim = (int32)floor( FinalVertex.y * 100.0f + .5f );
		FinalVertex.y = (float32)PrecisionTrim / 100.0f;

		if( fabs( FinalVertex.y ) < .1f )
		{
			FinalVertex.y += pLevel.FloorHeight( FinalVertex );
		}
		else
		{
			FinalVertex.y += pLevel.FloorHeight( FinalVertex );
		}

		FinalVertex.x += pLevel.FloorOffsetX( FinalVertex );
		FinalVertex.z += pLevel.FloorOffsetY( FinalVertex );


		ExpandBounds( m_MinBounds, m_MaxBounds, FinalVertex );

		m_Vertices.push_back( FinalVertex );
	}
	for( i = 0; i <  VertexReferences.size(); i++ )
	{
		m_VertexReferences.push_back( VertexReferences[i] + VertexCount );
	}
	for( i = 0; i <  UVs.size(); i++ )
	{
		m_UVs.push_back( UVs[i] );
	}
	for( i = 0; i <  UVReferences.size(); i++ )
	{
		m_UVReferences.push_back( UVReferences[i] + UVCount );
	}
	for( i = 0; i <  Normals.size(); i++ )
	{
		m_Normals.push_back( Normals[i] );
	}
	for( i = 0; i <  NormalReferences.size(); i++ )
	{
		m_NormalReferences.push_back( NormalReferences[i] + NormalCount );
	}
	for( i = 0; i < Triangles.size(); i++ )
	{
		D3DXVECTOR4 Triangle = Triangles[i];

		Triangle.x += VertexIndexCount;
		Triangle.y += VertexIndexCount;
		Triangle.z += VertexIndexCount;
		Triangle.w = (float32)Layer;
		m_Triangles.push_back( Triangle );

	}
} // CLevelMeshPatch::AddWall()

void CLevelMeshPatch::SupplyCollision( CFaceList& pFaceList )	// face list to add collision data to
{
	if( m_CollisionVertices.size() > 0 )
	{
		uint32 CurrentVertices = pFaceList.Vertices();
		for( uint32 i = 0; i < m_CollisionVertices.size(); i++ )
		{
			pFaceList.AddVertex( m_CollisionVertices[i] );
		}

		for( i = 0; i < m_CollisionTriangles.size(); i++ )
		{
			pFaceList.AddFace( m_CollisionVertexReferences[( int32 )m_CollisionTriangles[i].z] + CurrentVertices,
							m_CollisionVertexReferences[( int32 )m_CollisionTriangles[i].y] + CurrentVertices,
							m_CollisionVertexReferences[( int32 )m_CollisionTriangles[i].x] + CurrentVertices,
							( int32 )m_CollisionTriangles[i].w,
							-1 );
		}
	}
	else
	{
		uint32 CurrentVertices = pFaceList.Vertices();
		for( uint32 i = 0; i < m_Vertices.size(); i++ )
		{
			pFaceList.AddVertex( m_Vertices[i] );
		}

		for( i = 0; i < m_Triangles.size(); i++ )
		{
			pFaceList.AddFace( m_VertexReferences[( int32 )m_Triangles[i].z] + CurrentVertices,
							m_VertexReferences[( int32 )m_Triangles[i].y] + CurrentVertices,
							m_VertexReferences[( int32 )m_Triangles[i].x] + CurrentVertices,
							( int32 )m_Triangles[i].w,
							-1 );
		}
	}
} // CLevelMeshPatch::SupplyCollision()

void CLevelMeshPatch::Create( CLevel& pLevel,					// parent level
							  LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{

	CreateVisibility( pLevel, pD3DDevice );

	CreateMiniMap( pLevel, pD3DDevice );

	CreateFloor( pLevel,	
				 pD3DDevice );

	CreateWalls( pLevel,
				 pD3DDevice );

	CreateWater( pLevel,
				 pD3DDevice );

	// clear out internal template storage
	m_Vertices.clear();
	m_VertexReferences.clear();
	m_Triangles.clear();
	m_UVReferences.clear();
	m_UVs.clear();
	m_Normals.clear();
	m_NormalReferences.clear();
	m_CollisionVertices.clear();
	m_CollisionVertexReferences.clear();
	m_CollisionTriangles.clear();

	CalculateBounds();
} // CLevelMeshPatch::Create()

void CLevelMeshPatch::CreateVisibility( CLevel& pLevel,					// parent level
										LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	float32 YPosition = pLevel.TileHeight();
	m_VisibilityTilesWide = KPatchVisibilityTiles;
	m_VisibilityTilesHigh = KPatchVisibilityTiles;
	if( m_TopLeftCellX * KVisibilityCellsPerTile + KPatchVisibilityTiles >= pLevel.VisibilityWidth() )
	{
		m_VisibilityTilesWide++;
	}
	if( m_TopLeftCellY * KVisibilityCellsPerTile + KPatchVisibilityTiles >= pLevel.VisibilityHeight() )
	{
		m_VisibilityTilesHigh++;
	}

	D3DXVECTOR3 Position( m_Position );

	m_VisibilityVertices = ( m_VisibilityTilesWide + 1 ) * ( m_VisibilityTilesHigh + 1 );
	// create the vertex buffer
	HRESULT hr = pD3DDevice->CreateVertexBuffer( m_VisibilityVertices * sizeof( LITPLAINVERTEXSCENE ),
												 D3DUSAGE_WRITEONLY,
												 D3DFVF_LITPLAINVERTEXSCENE,
												 D3DPOOL_MANAGED,
												 &m_pVisibilityVB );

	if ( FAILED( hr ) )
	{
		return;
	}


	// Fill vertex buffer
	LITPLAINVERTEXSCENE*	pVertices;
	hr = m_pVisibilityVB->Lock( 0, 
							    0, 
							    ( BYTE * * ) &pVertices, 
							    NULL );
	if ( FAILED( hr ) )
	{
		return;
	}

	D3DXVECTOR3 StartingPosition( m_Position );

	float32 TileWidth = m_TileWidth * KPatchTiles / KPatchVisibilityTiles;

	int32	TopLeftX( (int32)m_TopLeftCellX * (int32)KVisibilityCellsPerTile );
	int32	TopLeftY( (int32)m_TopLeftCellY * (int32)KVisibilityCellsPerTile );
	
	for( uint32 y = 0; y < m_VisibilityTilesHigh + 1; y++ )
	{
		for( uint32 x = 0; x < m_VisibilityTilesWide + 1; x++ )
		{
			pVertices->v = StartingPosition + D3DXVECTOR3( (float32)x * TileWidth + pLevel.FloorOffsetX( m_Position.x + (float32)x * TileWidth, m_Position.z + (float32)y * -TileWidth ),
															YPosition + pLevel.FloorHeight( m_Position.x + (float32)x * TileWidth, m_Position.z + (float32)y * -TileWidth ),
															(float32)y * -TileWidth  + pLevel.FloorOffsetY( m_Position.x + (float32)x * TileWidth, m_Position.z + (float32)y * -TileWidth ) );
			pVertices->tu = x * .15f + y *.15f;
			pVertices->tv = y * .15f - x * .15f;
			if( x + TopLeftX == 0 ||
				y + TopLeftY == 0 ||
				x + TopLeftX >= pLevel.VisibilityWidth() ||
				y + TopLeftY >= pLevel.VisibilityHeight() )
			{
				pVertices->color = D3DCOLOR_RGBA( 0, 0, 0, 255 );
			}
			else
			{
				pVertices->color = D3DCOLOR_RGBA( 24, 24, 34, 255 );
			}
			pVertices++;		

		}
	}

	m_pVisibilityVB->Unlock();



	m_VisibilityIndices = m_VisibilityTilesWide * m_VisibilityTilesHigh * 6;

	// Create the index buffer
	WORD*	pIndices;
	hr = pD3DDevice->CreateIndexBuffer( m_VisibilityIndices * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pVisibilityIB );
	if ( FAILED( hr ) )
	{
		return;
	}

	// Fill the index buffer
	m_pVisibilityIB->Lock( 0, 
						  m_VisibilityIndices * sizeof( WORD ), 
						  ( BYTE * * ) &pIndices, 
						  0 );

	if ( FAILED( hr ) )
	{
		return;
	}
	uint32 FaceIndex( 0 );
	for( y = 0; y < m_VisibilityTilesHigh; y++ )
	{
		for( uint32 x = 0; x < m_VisibilityTilesWide; x++ )
		{
			uint32 Index = ( y * ( m_VisibilityTilesWide + 1 ) ) + ( x );
			*pIndices++ = ( WORD ) ( Index  );
			*pIndices++ = ( WORD ) ( Index + 1 );
			*pIndices++ = ( WORD ) ( Index + m_VisibilityTilesWide + 2 );
			*pIndices++ = ( WORD ) ( Index );
			*pIndices++ = ( WORD ) ( Index + m_VisibilityTilesWide + 2 );
			*pIndices++ = ( WORD ) ( Index + m_VisibilityTilesWide + 1 );
		}
	}
	m_pVisibilityIB->Unlock();
} // CLevelMeshPatch::CreateVisibility()

void CLevelMeshPatch::UpdateVisibility( CLevel& pLevel,				// parent level
										LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
										float32 TimeElapsed )		// time elapsed in seconds
{

	bool MustUpdate( kFalse );
	uint32 VertexIndex( 0 );
	bool   Visible( kTrue );
	int32	TopLeftX( (int32)m_TopLeftCellX * (int32)KVisibilityCellsPerTile );
	int32	TopLeftY( (int32)m_TopLeftCellY * (int32)KVisibilityCellsPerTile );
	for( int32 X = 0; X <= (int32)m_VisibilityTilesWide; X ++ )
	{
		for( int32 Y = 0; Y <= (int32)m_VisibilityTilesHigh; Y ++ )
		{
	
			ETerrainType Type = KTTypeEmpty;
			int32 Level = uint32( pLevel.VisibilityLevel( X + TopLeftX, Y + TopLeftY ) * 255 );
			if( Level > 255 )
			{
				Level = 255;
			}
			if( Level < 0 )
			{
				Level = 0;
			}
			if( m_FloatVisibilityLevel[X][Y] > Level )
			{
				float32 Delta = m_FloatVisibilityLevel[X][Y] - (float32)Level;
				Delta *= pow( .01f, TimeElapsed );
				m_FloatVisibilityLevel[X][Y] = Level + Delta;
				if( m_FloatVisibilityLevel[X][Y] < Level )
				{
					m_FloatVisibilityLevel[X][Y] = (float32)Level;
				}
			}
			else if( m_FloatVisibilityLevel[X][Y] < Level )
			{
				float32 Delta = (float32)Level - m_FloatVisibilityLevel[X][Y];
				Delta *= pow( .01f, TimeElapsed );
				m_FloatVisibilityLevel[X][Y] = Level - Delta;
				if( m_FloatVisibilityLevel[X][Y] > Level )
				{
					m_FloatVisibilityLevel[X][Y] = (float32)Level;
				}
			}

			if( (int32)m_FloatVisibilityLevel[X][Y] != m_VisibilityLevel[X][Y] )
			{
				m_VisibilityLevel[X][Y] = (int32)m_FloatVisibilityLevel[X][Y];
				MustUpdate = kTrue;
			}
		}
	}


	if( MustUpdate )
	{
		m_VisibilityEmpty = kTrue;
		m_VisibilityFull = kTrue;
		// Fill vertex buffer
		LITPLAINVERTEXSCENE*	pVertices;
		HRESULT hr = m_pVisibilityVB->Lock( 0, 
			 							0, 
										( BYTE * * ) &pVertices, 
										NULL );
		if ( FAILED( hr ) )
		{
			return;
		}

		for( X = 0; X <= (int32)m_VisibilityTilesWide; X ++ )
		{
			for( int32 Y = 0; Y <= (int32)m_VisibilityTilesHigh; Y ++ )
			{
				if( m_VisibilityLevel[X][Y] != 0 )
				{
					m_VisibilityEmpty = kFalse;
				}
				if( m_VisibilityLevel[X][Y] != 255 )
				{
					m_VisibilityFull = kFalse;
				}
				uint32 Index = ( Y * ( m_VisibilityTilesWide + 1 ) ) + ( X );

				if( X + TopLeftX == 0 ||
					Y + TopLeftY == 0 ||
					X + TopLeftX >= (int32)pLevel.VisibilityWidth() ||
					Y + TopLeftY >= (int32)pLevel.VisibilityHeight() )
				{
					pVertices[Index].color = D3DCOLOR_RGBA( 0, 0, 0, 255 );
				}
				else
				{
					pVertices[Index].color = D3DCOLOR_RGBA( 24, 
															24, 
															34, 
															m_VisibilityLevel[X][Y] );
				}
			}
		}
		m_pVisibilityVB->Unlock();
	}

} // CLevelMeshPatch::UpdateVisibility()

void CLevelMeshPatch::SetVisibilityTile( LITPLAINVERTEXSCENE*	pVertices,	// vertex list
										 int32 X,						// x tile
										 int32 Y,						// y tile
										 ETerrainType Type )			// tile type
{
	if( X < 0 || Y < 0 || 
		X >= (int32)m_VisibilityTilesWide || 
		Y >= (int32)m_VisibilityTilesHigh )
	{
		return;
	}

	float32 V( (float32)floor( (float32)Type / 4.0f ) );
	float32 U( ( (float32)Type - V * 4.0f ) );
	V *= .25f;
	U *= .25f;

	float32 TexelSize( 1.0f / 512.0f );

	if( Type == KTTypeOpen ||
		Type == KTTypeEmpty )
	{
		TexelSize = ( 1.0f / 64.0f );
	}

	uint32 Index = ( Y * m_VisibilityTilesWide * 4 ) + ( X * 4 );

	pVertices[Index].tu = U + TexelSize;
	pVertices[Index].tv = V + TexelSize;

	pVertices[Index + 1].tu = U + .25f - TexelSize;
	pVertices[Index + 1].tv = V + TexelSize;

	pVertices[Index + 2].tu = U + TexelSize;
	pVertices[Index + 2].tv = V + .25f - TexelSize;

	pVertices[Index + 3].tu = U + .25f - TexelSize;
	pVertices[Index + 3].tv = V + .25f - TexelSize;
} // CLevelMeshPatch::SetVisibilityTile()

void CLevelMeshPatch::CreateMiniMap( CLevel& pLevel,					// parent level
									 LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	D3DXVECTOR3 Position( m_Position );

	m_MiniMapVertices = KPatchTiles * KPatchTiles * 4;
	// create the vertex buffer
	HRESULT hr = pD3DDevice->CreateVertexBuffer( m_MiniMapVertices * sizeof( PLAINVERTEXMODEL ),
												 D3DUSAGE_WRITEONLY,
												 D3DFVF_PLAINVERTEXMODEL,
												 D3DPOOL_MANAGED,
												 &m_pMiniMapVB );

	if ( FAILED( hr ) )
	{
		return;
	}

	float32 TexelSize( 1.0f / 64.0f );

	// Fill vertex buffer
	PLAINVERTEXMODEL*	pVertices;
	hr = m_pMiniMapVB->Lock( 0, 
							   0, 
							   ( BYTE * * ) &pVertices, 
							   NULL );
	if ( FAILED( hr ) )
	{
		return;
	}

	D3DXVECTOR3 StartingPosition( m_Position );

	float32 TileWidth = m_TileWidth;

	for( uint32 y = 0; y < KPatchTiles; y++ )
	{
		for( uint32 x = 0; x < KPatchTiles; x++ )
		{
			pVertices->v = StartingPosition + D3DXVECTOR3( (float32)x * TileWidth - TileWidth * .5f,
															0,
															(float32)y * -TileWidth + TileWidth * .5f );
			pVertices->tu = TexelSize;
			pVertices->tv = TexelSize;
			pVertices++;		

			pVertices->v = StartingPosition + D3DXVECTOR3( (float32)x * TileWidth + TileWidth - TileWidth * .5f,
										0,
										(float32)y * -TileWidth + TileWidth * .5f);
			pVertices->tu = .25f - TexelSize;
			pVertices->tv = TexelSize;
			pVertices++;		

			pVertices->v = StartingPosition + D3DXVECTOR3( (float32)x * TileWidth - TileWidth * .5f,
										0,
										(float32)y * -TileWidth - TileWidth + TileWidth * .5f );
			pVertices->tu = TexelSize;
			pVertices->tv = .25f - TexelSize;
			pVertices++;		

			pVertices->v = StartingPosition + D3DXVECTOR3( (float32)x * TileWidth + TileWidth - TileWidth * .5f,
										0,
										(float32)y * -TileWidth - TileWidth + TileWidth * .5f );
			pVertices->tu = .25f - TexelSize;
			pVertices->tv = .25f - TexelSize;
			pVertices++;		
		}
	}

	m_pMiniMapVB->Unlock();


	m_MiniMapIndices = KPatchTiles * KPatchTiles * 6;

	// Create the index buffer
	WORD*	pIndices;
	hr = pD3DDevice->CreateIndexBuffer( m_MiniMapIndices * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pMiniMapIB );
	if ( FAILED( hr ) )
	{
		return;
	}

	// Fill the index buffer
	m_pMiniMapIB->Lock( 0, 
						  m_MiniMapIndices * sizeof( WORD ), 
						  ( BYTE * * ) &pIndices, 
						  0 );

	if ( FAILED( hr ) )
	{
		return;
	}
	uint32 FaceIndex( 0 );
	for( y = 0; y < KPatchTiles; y++ )
	{
		for( uint32 x = 0; x < KPatchTiles; x++ )
		{
			*pIndices++ = ( WORD ) ( FaceIndex  );
			*pIndices++ = ( WORD ) ( FaceIndex + 1 );
			*pIndices++ = ( WORD ) ( FaceIndex + 2 );
			*pIndices++ = ( WORD ) ( FaceIndex + 1 );
			*pIndices++ = ( WORD ) ( FaceIndex + 3 );
			*pIndices++ = ( WORD ) ( FaceIndex + 2 );

			FaceIndex+=4;
		}
	}
	m_pMiniMapIB->Unlock();
} // CLevelMeshPatch::CreateMiniMap()

void CLevelMeshPatch::UpdateMiniMap( CLevel& pLevel,				// parent level
									 LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{

	bool MustUpdate( kFalse );
	bool Touched[KPatchTiles+4][KPatchTiles+4];
	ETerrainType StoredType[KPatchTiles+4][KPatchTiles+4];
	for( int32 X = 0; X < (int32)KPatchTiles + 3; X++ )
	{
		for( int32 Y = 0; Y < (int32)KPatchTiles + 3; Y ++ )
		{
			Touched[X][Y] = kFalse;
		}
	}

	uint32 VertexIndex( 0 );
	bool   Visible( kTrue );
	int32	TopLeftX( (int32)m_TopLeftCellX );
	int32	TopLeftY( (int32)m_TopLeftCellY );
	for( X = -1; X < (int32)KPatchTiles; X ++ )
	{
		for( int32 Y = -1; Y < (int32)KPatchTiles; Y ++ )
		{
	
			ETerrainType Type = KTTypeEmpty;


			Visible = pLevel.Maze()->CellHeight( X + TopLeftX, Y + TopLeftY ) == 0;

			if( Visible )
			{
				Type = KTTypeEmpty;
			}

			bool Discovered = kTrue;
			for( int32 i = -1; i < (int32)KVisibilityCellsPerTile - 1; i++ )
			{
				for( int32 j = -1; j < (int32)KVisibilityCellsPerTile - 1; j++ )
				{
					if( pLevel.Visible( ( X + TopLeftX + i ) * (int32)KVisibilityCellsPerTile,
										( Y + TopLeftY + j ) * (int32)KVisibilityCellsPerTile ) )
					{
						Discovered = kFalse;
					}
				}
			}
			if( Discovered )
			{
				Type = KTTypeOpen;
				Visible = kTrue;
			}

			if( !Touched[X+1][Y+1] )
			{
				if( !Visible )
				{
					Type = pLevel.MiniMapTypeNW( X + TopLeftX, Y + TopLeftY );
				}
				
				StoredType[X+1][Y+1] = Type;
				if( Type != KTTypeEmpty )
				{
					Touched[X+1][Y+1] = kTrue;
				}
			}

			if( !Touched[X+2][Y+1] )
			{
				if( !Visible )
				{
					Type = pLevel.MiniMapTypeNE( X + TopLeftX, Y + TopLeftY );
				}

				StoredType[X+2][Y+1] = Type;
				if( Type != KTTypeEmpty )
				{
					Touched[X+2][Y+1] = kTrue;
				}
			}
			if( !Touched[X+1][Y+2] )
			{
				if( !Visible )
				{
					Type = pLevel.MiniMapTypeSW( X + TopLeftX, Y + TopLeftY );
				}

				StoredType[X+1][Y+2] = Type;
				if( Type != KTTypeEmpty )
				{
					Touched[X+1][Y+2] = kTrue;
				}
			}
			if( !Touched[X+2][Y+2] )
			{
				if( !Visible )
				{
					Type = pLevel.MiniMapTypeSE( X + TopLeftX, Y + TopLeftY );
				}

				StoredType[X+2][Y+2] = Type;
				if( Type != KTTypeEmpty )
				{
					Touched[X+2][Y+2] = kTrue;
				}
			}
		}
	}

	for( X = 0; X <= KPatchTiles; X ++ )
	{
		for( uint32 Y = 0; Y <= KPatchTiles; Y ++ )
		{
			if( StoredType[X][Y] != m_MiniMapCornerType[X][Y] )
			{
				MustUpdate = kTrue;
			}
		}
	}

	if( MustUpdate )
	{
		// Fill vertex buffer
		PLAINVERTEXMODEL*	pVertices;
		HRESULT hr = m_pMiniMapVB->Lock( 0, 
			 							0, 
										( BYTE * * ) &pVertices, 
										NULL );
		if ( FAILED( hr ) )
		{
			return;
		}

		for( X = -1; X < (int32)KPatchTiles; X ++ )
		{
			for( int32 Y = -1; Y < (int32)KPatchTiles; Y ++ )
			{
				if( StoredType[X+1][Y+1] != m_MiniMapCornerType[X+1][Y+1] )
				{
					SetMiniMapTile( pVertices, X, Y, StoredType[X+1][Y+1] );
					m_MiniMapCornerType[X+1][Y+1] = StoredType[X+1][Y+1];
				}
			}
		}
		m_pMiniMapVB->Unlock();
	}

} // CLevelMeshPatch::UpdateMiniMap()

void CLevelMeshPatch::SetMiniMapTile( PLAINVERTEXMODEL*	pVertices,	// vertex list
									  int32 X,						// x tile
									  int32 Y,						// y tile
									  ETerrainType Type )			// tile type
{
	if( X < 0 || Y < 0 || X >= KPatchTiles || Y >= KPatchTiles )
	{
		return;
	}

	float32 V( (float32)floor( (float32)Type / 4.0f ) );
	float32 U( ( (float32)Type - V * 4.0f ) );
	V *= .25f;
	U *= .25f;

	float32 TexelSize( 1.0f / 64.0f );

	uint32 Index = ( Y * KPatchTiles * 4 ) + ( X * 4 );

	pVertices[Index].tu = U + TexelSize;
	pVertices[Index].tv = V + TexelSize;

	pVertices[Index + 1].tu = U + .25f - TexelSize;
	pVertices[Index + 1].tv = V + TexelSize;

	pVertices[Index + 2].tu = U + TexelSize;
	pVertices[Index + 2].tv = V + .25f - TexelSize;

	pVertices[Index + 3].tu = U + .25f - TexelSize;
	pVertices[Index + 3].tv = V + .25f - TexelSize;
} // CLevelMeshPatch::SetMiniMapTile()

void CLevelMeshPatch::CreateFloor( CLevel& pLevel,					// parent level
								   LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	for( uint32 i = 0; i < m_GroundLayers; i++ )
	{
		CreateFloorLayer( i, pLevel, pD3DDevice );
	}
	CreateCeilingLayer( pLevel, pD3DDevice );

} // CLevelMeshPatch::CreateBaseFloor()

void CLevelMeshPatch::CreateFloorLayer( uint32 Layer,					// layer to create
										CLevel& pLevel,					// parent level
										LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	CMaze* pMaze = pLevel.Maze();

	D3DXVECTOR3 Position( m_Position );
	D3DXVECTOR3 FinalPosition( Position );
	std::vector< uint32 >			FaceIndices;
	std::vector< LITPLAINVERTEXSCENE >	Vertices;

	bool Touched[KPatchTiles+1][KPatchTiles+1];

	for( int32 X = 0; X < (int32)KPatchTiles + 1; X++ )
	{
		for( int32 Y = 0; Y < (int32)KPatchTiles + 1; Y ++ )
		{
			Touched[X][Y] = kFalse;

		}
	}
	uint32 VertexIndex( 0 );
	int32 Width = KPatchTiles;
	int32 Height = KPatchTiles;
	// layer 0 doesn't have to do overlaps
	if( Layer == 0 &&
		!( m_TopLeftCellX + Width >= (int32)pLevel.Maze()->Width()) )
	{
		Width --;
	}
	if( Layer == 0 &&
		!( m_TopLeftCellY + Height >= (int32)pLevel.Maze()->Height() - 1 ) )
	{
		Height --;
	}

	for( X = 0; X < Width; X ++ )
	{
		for( int32 Y = 0; Y < Height; Y ++ )
		{

			if( Layer == 0 ||
				pMaze->TerrainType( X + m_TopLeftCellX, Y + m_TopLeftCellY ) == Layer )
			{
	
				ETerrainType Type = KTTypeOpen;

				if( !Touched[X][Y] )
				{
					if( Layer > 0 )
					{
						Type = pMaze->TerrainTypeNW( X + m_TopLeftCellX, Y + m_TopLeftCellY );
					}

					if( Type != KTTypeEmpty )
					{
						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( -m_TileWidth, 0, m_TileWidth );
						AddTile( pLevel, Layer, X + m_TopLeftCellX - 1, Y + m_TopLeftCellY - 1, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X][Y] = kTrue;
					}
				}

				if( !Touched[X+1][Y] )
				{
					if( Layer > 0 )
					{
						Type = pMaze->TerrainTypeNE( X + m_TopLeftCellX, Y + m_TopLeftCellY );
					}
					if( Type != KTTypeEmpty )
					{
	
						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( 0, 0, m_TileWidth );
						AddTile( pLevel, Layer, X + m_TopLeftCellX, Y + m_TopLeftCellY - 1, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X+1][Y] = kTrue;
					}
				}
				if( !Touched[X][Y+1] )
				{
					if( Layer > 0 )
					{
						Type = pMaze->TerrainTypeSW( X + m_TopLeftCellX, Y + m_TopLeftCellY );
					}

					if( Type != KTTypeEmpty )
					{
	
						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( -m_TileWidth, 0, 0 );
						AddTile( pLevel, Layer, X + m_TopLeftCellX - 1, Y + m_TopLeftCellY, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X][Y+1] = kTrue;
					}
				}
				if( !Touched[X+1][Y+1] )
				{
					if( Layer > 0 )
					{
						Type = pMaze->TerrainTypeSE( X + m_TopLeftCellX, Y + m_TopLeftCellY );
					}
					if( Type != KTTypeEmpty )
					{

						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( 0, 0, 0 );
						AddTile( pLevel, Layer, X + m_TopLeftCellX, Y + m_TopLeftCellY, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X+1][Y+1] = kTrue;
					}
				}
			}
		}
	}

	if( FaceIndices.size() == 0 )
	{
		return;
	}

	m_FloorVertices[Layer] = Vertices.size();
	if( Vertices.size() == 0 )
	{
		return;
	}
	// create the vertex buffer
	HRESULT hr = pD3DDevice->CreateVertexBuffer( m_FloorVertices[Layer] * sizeof( LITPLAINVERTEXSCENE ),
												 D3DUSAGE_WRITEONLY,
												 D3DFVF_LITPLAINVERTEXSCENE,
												 D3DPOOL_MANAGED,
												 &m_pFloorVB[Layer] );

	if ( FAILED( hr ) )
	{
		m_FloorIndices[Layer] = 0;
		m_FloorVertices[Layer] = 0;
		RELEASE_SAFELY( m_pFloorVB[Layer] );
		m_pFloorVB[Layer] = NULL;
		return;
	}


	// Fill vertex buffer
	LITPLAINVERTEXSCENE*	pVertices;
	hr = m_pFloorVB[Layer]->Lock( 0, 
							   0, 
							   ( BYTE * * ) &pVertices, 
							   NULL );
	if ( FAILED( hr ) )
	{
		m_FloorIndices[Layer] = 0;
		m_FloorVertices[Layer] = 0;
		RELEASE_SAFELY( m_pFloorVB[Layer] );
		m_pFloorVB[Layer] = NULL;
		return;
	}
	// plug in our vert list
	for( uint32 i = 0; i < m_FloorVertices[Layer]; i++ )
	{
		*pVertices = Vertices[i];
		if( Vertices[i].v.y > pLevel.TileHeight() ||
			Vertices[i].v.y <  pLevel.Template()->DarkStart() )
		{
			float32 Level = 255;
			if( Vertices[i].v.y < 0 )
			{
				Level = (float32)fabs( Vertices[i].v.y );
				Level -= pLevel.Template()->DarkStart();
				Level /= ( pLevel.Template()->DarkEnd() - pLevel.Template()->DarkStart() );
				if( Level > 1 )
				{
					Level = 1;
				}
				Level *= 255;
				Level = 255 - Level;
			}
			else
			{
				Level = (float32)fabs( Vertices[i].v.y );
				if( Level > pLevel.TileHeight() + 4 )
				{
					Level = 64;
				}
				else
				{
					Level = 255;
				}
			}
			pVertices->color = D3DCOLOR_RGBA( (int32)Level, (int32)Level, (int32)Level, 255 );
		}

		if( Layer == 0 )
		{
			ExpandBounds( m_MinBounds, m_MaxBounds, Vertices[i].v );
		}
		pVertices++;		
	}
	m_pFloorVB[Layer]->Unlock();


	m_FloorIndices[Layer] = FaceIndices.size();

	// Create the index buffer
	WORD*	pIndices;
	hr = pD3DDevice->CreateIndexBuffer( m_FloorIndices[Layer] * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pFloorIB[Layer] );
	if ( FAILED( hr ) )
	{
		m_FloorIndices[Layer] = 0;
		m_FloorVertices[Layer] = 0;
		RELEASE_SAFELY( m_pFloorIB[Layer] );
		m_pFloorIB[Layer] = NULL;
		return;
	}

	// Fill the index buffer
	m_pFloorIB[Layer]->Lock( 0, 
						  m_FloorIndices[Layer] * sizeof( WORD ), 
						  ( BYTE * * ) &pIndices, 
						  0 );

	if ( FAILED( hr ) )
	{
		m_FloorIndices[Layer] = 0;
		m_FloorVertices[Layer] = 0;
		RELEASE_SAFELY( m_pFloorIB[Layer] );
		m_pFloorIB[Layer] = NULL;
		return;
	}
	// plug in our indices
	for( i = 0; i < m_FloorIndices[Layer]; i++ )
	{
		*pIndices++ = ( WORD ) ( FaceIndices[i] );
	}


	m_pFloorIB[Layer]->Unlock();
} // CLevelMeshPatch::CreateFloorLayer()

void CLevelMeshPatch::AddTile( CLevel& pLevel,							// parent level
							   uint32 Layer,							// map layer
							   int32 CellX,								// maze cell
							   int32 CellY,								// maze cell
							   std::vector< LITPLAINVERTEXSCENE >& Vertices,	// vertices to add to
							   std::vector< uint32 >& FaceIndices,		// face indices to add to
							   ETerrainType Type,						// tile type
							   const D3DXVECTOR3& Position )			// position of top-left corner 
{
	if( CellX < 0 ||
		CellY < 0 )
	{
		return;
	}
	D3DCOLOR NW = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR N = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR NE = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR W = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR CENTER = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR E = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR SW = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR S = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR SE = D3DCOLOR_RGBA( 255, 255, 255, 255 );

	bool TopLeftWall( kFalse );
	bool TopRightWall( kFalse );
	bool BottomLeftWall( kFalse );
	bool BottomRightWall( kFalse );
	EWallType WallType = KWTypeOpen;
	float32 TopLeftHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX, CellY );

	WallType = pLevel.Maze()->WallTypeNW( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeNWSolid )
	{
		TopLeftHeight -= m_TileHeight;
		TopLeftWall = kTrue;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
	}

	float32 TopRightHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX , CellY );

	WallType = pLevel.Maze()->WallTypeNE( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeNESolid )
	{
		TopRightWall = kTrue;
		TopRightHeight -= m_TileHeight;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
	}

	float32 BottomLeftHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX, CellY );

	WallType = pLevel.Maze()->WallTypeSW( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeSWSolid )
	{
		BottomLeftWall = kTrue;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
		BottomLeftHeight -= m_TileHeight;
	}

	float32 BottomRightHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX, CellY );

	WallType = pLevel.Maze()->WallTypeSE( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeSESolid )
	{
		BottomRightWall = kTrue;
		BottomRightHeight -= m_TileHeight;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
	}


	LITPLAINVERTEXSCENE Vertex;
	uint32 StartingIndex = Vertices.size();

	float32 V( (float32)floor( (float32)Type / 4.0f ) );
	float32 U( ( (float32)Type - V * 4.0f ) );
	V *= .25f;
	U *= .25f;
	float32 TexelSize( 1.0f / 1024.0f );

	

	Vertex.v = Position;
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + TexelSize;
	Vertex.color = NW;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U +  .125f;
	Vertex.tv = V + TexelSize;
	Vertex.color = N;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U +  .125f;
	Vertex.tv = V + TexelSize;
	Vertex.color = N;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + TexelSize;
	Vertex.color = NE;
	Vertices.push_back( Vertex );

	// center
	Vertex.v = Position + D3DXVECTOR3( 0, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = W;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( TopLeftWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = W;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( TopRightWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = E;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = E;
	Vertices.push_back( Vertex );

// center 2

	Vertex.v = Position + D3DXVECTOR3( 0, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = W;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( BottomLeftWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = W;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( BottomRightWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = E;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = E;
	Vertices.push_back( Vertex );


// bottom
	Vertex.v = Position + D3DXVECTOR3( 0, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = SW;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = S;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = S;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = SE;
	Vertices.push_back( Vertex );

	if( TopLeftHeight <= 0 && 
		( TopLeftHeight == 0 ||
		  Layer == 0 ||
		  Layer == pLevel.Template()->TileGroundLayer( pLevel.Maze()->WallType( CellX, CellY ) ) ) )
	{
		FaceIndices.push_back( StartingIndex );
		FaceIndices.push_back( StartingIndex + 5 );
		FaceIndices.push_back( StartingIndex + 4 );

		FaceIndices.push_back( StartingIndex );
		FaceIndices.push_back( StartingIndex + 1 );
		FaceIndices.push_back( StartingIndex + 5 );
	}

	if( TopRightHeight <= 0 &&
		( TopRightHeight == 0 ||
		  Layer == 0 ||
		  Layer == pLevel.Template()->TileGroundLayer( pLevel.Maze()->WallType( CellX, CellY ) ) ) )
	{
		FaceIndices.push_back( StartingIndex + 2 );
		FaceIndices.push_back( StartingIndex + 3 );
		FaceIndices.push_back( StartingIndex + 6 );

		FaceIndices.push_back( StartingIndex + 3 );
		FaceIndices.push_back( StartingIndex + 7 );
		FaceIndices.push_back( StartingIndex + 6 );
	}

	if( BottomLeftHeight <= 0 &&
		( BottomLeftHeight == 0 ||
		  Layer == 0 ||
		  Layer == pLevel.Template()->TileGroundLayer( pLevel.Maze()->WallType( CellX, CellY ) )  ))
	{
		FaceIndices.push_back( StartingIndex + 8 );
		FaceIndices.push_back( StartingIndex + 9 );
		FaceIndices.push_back( StartingIndex + 12 );

		FaceIndices.push_back( StartingIndex + 12 );
		FaceIndices.push_back( StartingIndex + 9 );
		FaceIndices.push_back( StartingIndex + 13 );
	}

	if( BottomRightHeight <= 0 &&
		( BottomRightHeight == 0 ||
		  Layer == 0 ||
		  Layer == pLevel.Template()->TileGroundLayer( pLevel.Maze()->WallType( CellX, CellY ) ) ) )
	{
		FaceIndices.push_back( StartingIndex + 10 );
		FaceIndices.push_back( StartingIndex + 15 );
		FaceIndices.push_back( StartingIndex + 14 );

		FaceIndices.push_back( StartingIndex + 10 );
		FaceIndices.push_back( StartingIndex + 11 );
		FaceIndices.push_back( StartingIndex + 15 );
	}

} // CLevelMeshPatch::AddTile()


void CLevelMeshPatch::CreateCeilingLayer( CLevel& pLevel,					// parent level
										  LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	CMaze* pMaze = pLevel.Maze();

	D3DXVECTOR3 Position( m_Position );
	D3DXVECTOR3 FinalPosition( Position );
	std::vector< uint32 >			FaceIndices;
	std::vector< LITPLAINVERTEXSCENE >	Vertices;

	bool Touched[KPatchTiles+1][KPatchTiles+1];

	for( int32 X = 0; X < (int32)KPatchTiles + 1; X++ )
	{
		for( int32 Y = 0; Y < (int32)KPatchTiles + 1; Y ++ )
		{
			Touched[X][Y] = kFalse;

		}
	}
	uint32 VertexIndex( 0 );
	int32 Width = KPatchTiles;
	int32 Height = KPatchTiles;
	// layer 0 doesn't have to do overlaps
	if( !( m_TopLeftCellX + Width >= (int32)pLevel.Maze()->Width()) )
	{
		Width --;
	}
	if( !( m_TopLeftCellY + Height >= (int32)pLevel.Maze()->Height() - 1 ) )
	{
		Height --;
	}

	for( X = 0; X < Width; X ++ )
	{
		for(  int32 Y = 0; Y < Height; Y ++ )
		{

			{
	
				ETerrainType Type = KTTypeOpen;

				if( !Touched[X][Y] )
				{

					if( Type != KTTypeEmpty )
					{
						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( -m_TileWidth, 0, m_TileWidth );
						AddCeilingTile( pLevel, X + m_TopLeftCellX - 1, Y + m_TopLeftCellY - 1, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X][Y] = kTrue;
					}
				}

				if( !Touched[X+1][Y] )
				{
					if( Type != KTTypeEmpty )
					{
	
						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( 0, 0, m_TileWidth );
						AddCeilingTile( pLevel, X + m_TopLeftCellX, Y + m_TopLeftCellY - 1, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X+1][Y] = kTrue;
					}
				}
				if( !Touched[X][Y+1] )
				{

					if( Type != KTTypeEmpty )
					{
	
						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( -m_TileWidth, 0, 0 );
						AddCeilingTile( pLevel, X + m_TopLeftCellX - 1, Y + m_TopLeftCellY, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X][Y+1] = kTrue;
					}
				}
				if( !Touched[X+1][Y+1] )
				{
					if( Type != KTTypeEmpty )
					{

						Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, 0, (float32)Y * -m_TileWidth );
						FinalPosition = Position + D3DXVECTOR3( 0, 0, 0 );
						AddCeilingTile( pLevel, X + m_TopLeftCellX, Y + m_TopLeftCellY, Vertices, FaceIndices, Type, FinalPosition );
						Touched[X+1][Y+1] = kTrue;
					}
				}
			}
		}
	}

	if( FaceIndices.size() == 0 )
	{
		return;
	}

	m_CeilingVertices = Vertices.size();
	if( Vertices.size() == 0 )
	{
		return;
	}
	// create the vertex buffer
	HRESULT hr = pD3DDevice->CreateVertexBuffer( m_CeilingVertices * sizeof( LITPLAINVERTEXSCENE ),
												 D3DUSAGE_WRITEONLY,
												 D3DFVF_LITPLAINVERTEXSCENE,
												 D3DPOOL_MANAGED,
												 &m_pCeilingVB );

	if ( FAILED( hr ) )
	{
		return;
	}


	// Fill vertex buffer
	LITPLAINVERTEXSCENE*	pVertices;
	hr = m_pCeilingVB->Lock( 0, 
							 0, 
							 ( BYTE * * ) &pVertices, 
							 NULL );
	if ( FAILED( hr ) )
	{
		return;
	}
	// plug in our vert list
	for( uint32 i = 0; i < m_CeilingVertices; i++ )
	{
		*pVertices = Vertices[i];
		if( Vertices[i].v.y > pLevel.TileHeight() ||
			Vertices[i].v.y <  pLevel.TileHeight() * -.5f )
		{
			float32 Level = 255;
			if( Vertices[i].v.y < 0 )
			{
				Level = (float32)fabs( Vertices[i].v.y );
				Level -= pLevel.Template()->DarkStart();
				Level /= ( pLevel.Template()->DarkEnd() - pLevel.Template()->DarkStart() );
				if( Level > 1 )
				{
					Level = 1;
				}
				Level *= 255;
				Level = 255 - Level;
			}
			else
			{
				Level = (float32)fabs( Vertices[i].v.y );
				if( Level > pLevel.TileHeight() + 4 )
				{
					Level = 64;
				}
				else
				{
					Level = 255;
				}
			}

			pVertices->color = D3DCOLOR_RGBA( (int32)Level, (int32)Level, (int32)Level, 255 );
		}

		ExpandBounds( m_MinBounds, m_MaxBounds, Vertices[i].v );
		pVertices++;		
	}
	m_pCeilingVB->Unlock();


	m_CeilingIndices = FaceIndices.size();

	// Create the index buffer
	WORD*	pIndices;
	hr = pD3DDevice->CreateIndexBuffer( m_CeilingIndices * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pCeilingIB );
	if ( FAILED( hr ) )
	{
		return;
	}

	// Fill the index buffer
	m_pCeilingIB->Lock( 0, 
						m_CeilingIndices * sizeof( WORD ), 
						( BYTE * * ) &pIndices, 
						0 );

	if ( FAILED( hr ) )
	{
		return;
	}
	// plug in our indices
	for( i = 0; i < m_CeilingIndices; i++ )
	{
		*pIndices++ = ( WORD ) ( FaceIndices[i] );
	}


	m_pCeilingIB->Unlock();
} // CLevelMeshPatch::CreateFloorLayer()

void CLevelMeshPatch::AddCeilingTile( CLevel& pLevel,							// parent level
									  int32 CellX,								// maze cell
									  int32 CellY,								// maze cell
									  std::vector< LITPLAINVERTEXSCENE >& Vertices,	// vertices to add to
									  std::vector< uint32 >& FaceIndices,		// face indices to add to
									  ETerrainType Type,						// tile type
									  const D3DXVECTOR3& Position )			// position of top-left corner 
{
	if( CellX < 0 ||
		CellY < 0 )
	{
		return;
	}
	D3DCOLOR NW = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR N = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR NE = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR W = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR CENTER = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR E = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR SW = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR S = D3DCOLOR_RGBA( 255, 255, 255, 255 );
	D3DCOLOR SE = D3DCOLOR_RGBA( 255, 255, 255, 255 );

	bool TopLeftWall( kFalse );
	bool TopRightWall( kFalse );
	bool BottomLeftWall( kFalse );
	bool BottomRightWall( kFalse );
	EWallType WallType = KWTypeOpen;
	float32 TopLeftHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX, CellY );

	WallType = pLevel.Maze()->WallTypeNW( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeNWSolid )
	{
		TopLeftHeight -= m_TileHeight;
		TopLeftWall = kTrue;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
	}

	float32 TopRightHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX , CellY );

	WallType = pLevel.Maze()->WallTypeNE( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeNESolid )
	{
		TopRightWall = kTrue;
		TopRightHeight -= m_TileHeight;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
	}

	float32 BottomLeftHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX, CellY );

	WallType = pLevel.Maze()->WallTypeSW( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeSWSolid )
	{
		BottomLeftWall = kTrue;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
		BottomLeftHeight -= m_TileHeight;
	}

	float32 BottomRightHeight = m_TileHeight * (float32)pLevel.Maze()->CellHeight( CellX, CellY );

	WallType = pLevel.Maze()->WallTypeSE( CellX, CellY );
	if( WallType != KWTypeOpen &&
		WallType != KWTypeSESolid )
	{
		BottomRightWall = kTrue;
		BottomRightHeight -= m_TileHeight;
		CENTER = D3DCOLOR_RGBA( 32, 32, 32, 255 );
	}


	LITPLAINVERTEXSCENE Vertex;
	uint32 StartingIndex = Vertices.size();

	float32 V( (float32)floor( (float32)Type / 4.0f ) );
	float32 U( ( (float32)Type - V * 4.0f ) );
	V *= .25f;
	U *= .25f;
	float32 TexelSize( 1.0f / 1024.0f );

	

	Vertex.v = Position;
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + TexelSize;
	Vertex.color = NW;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U +  .125f;
	Vertex.tv = V + TexelSize;
	Vertex.color = N;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U +  .125f;
	Vertex.tv = V + TexelSize;
	Vertex.color = N;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + TexelSize;
	Vertex.color = NE;
	Vertices.push_back( Vertex );

	// center
	Vertex.v = Position + D3DXVECTOR3( 0, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = W;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( TopLeftWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = W;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( TopRightWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = E;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + TopRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = E;
	Vertices.push_back( Vertex );

// center 2

	Vertex.v = Position + D3DXVECTOR3( 0, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = W;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( BottomLeftWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = W;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .125f;
	if( BottomRightWall )
	{
		Vertex.color = CENTER;
	}
	else
	{
		Vertex.color = E;
	}
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, m_TileWidth * -.5f );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + .125f;
	Vertex.color = E;
	Vertices.push_back( Vertex );


// bottom
	Vertex.v = Position + D3DXVECTOR3( 0, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + TexelSize;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = SW;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomLeftHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = S;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth * .5f, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .125f;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = S;
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v ) + BottomRightHeight;
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = U + .25f - TexelSize;
	Vertex.tv = V + .25f - TexelSize;
	Vertex.color = SE;
	Vertices.push_back( Vertex );

	if( TopLeftHeight > 0 )
	{
		FaceIndices.push_back( StartingIndex );
		FaceIndices.push_back( StartingIndex + 5 );
		FaceIndices.push_back( StartingIndex + 4 );

		FaceIndices.push_back( StartingIndex );
		FaceIndices.push_back( StartingIndex + 1 );
		FaceIndices.push_back( StartingIndex + 5 );
	}

	if( TopRightHeight > 0 )
	{
		FaceIndices.push_back( StartingIndex + 2 );
		FaceIndices.push_back( StartingIndex + 3 );
		FaceIndices.push_back( StartingIndex + 6 );

		FaceIndices.push_back( StartingIndex + 3 );
		FaceIndices.push_back( StartingIndex + 7 );
		FaceIndices.push_back( StartingIndex + 6 );
	}

	if( BottomLeftHeight > 0 )
	{
		FaceIndices.push_back( StartingIndex + 8 );
		FaceIndices.push_back( StartingIndex + 9 );
		FaceIndices.push_back( StartingIndex + 12 );

		FaceIndices.push_back( StartingIndex + 12 );
		FaceIndices.push_back( StartingIndex + 9 );
		FaceIndices.push_back( StartingIndex + 13 );
	}

	if( BottomRightHeight > 0 )
	{
		FaceIndices.push_back( StartingIndex + 10 );
		FaceIndices.push_back( StartingIndex + 15 );
		FaceIndices.push_back( StartingIndex + 14 );

		FaceIndices.push_back( StartingIndex + 10 );
		FaceIndices.push_back( StartingIndex + 11 );
		FaceIndices.push_back( StartingIndex + 15 );
	}

} // CLevelMeshPatch::AddCeilingTile()



void CLevelMeshPatch::AddWaterTile( CLevel& pLevel,								// parent level
									std::vector< LITPLAINVERTEXSCENE >& Vertices,	// vertices to add to
									std::vector< uint32 >& FaceIndices,			// face indices to add to
									const D3DXVECTOR3& Position )				// position of top-left corner 
{
	if( Position.x <= 0 || Position.z >= 0 )
	{
		return;
	}
	LITPLAINVERTEXSCENE Vertex;
	uint32 StartingIndex = Vertices.size();


	Vertex.v = Position;
	Vertex.v.y += pLevel.FloorHeight( Vertex.v );
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = 0;
	Vertex.tv = 0;
	Vertex.color = pLevel.Template()->WaterColor();
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, 0 );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v );
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = 1;
	Vertex.tv = 0;
	Vertex.color = pLevel.Template()->WaterColor();
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( 0, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v );
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = 0;
	Vertex.tv = 1;
	Vertex.color = pLevel.Template()->WaterColor();
	Vertices.push_back( Vertex );

	Vertex.v = Position + D3DXVECTOR3( m_TileWidth, 0, -m_TileWidth );
	Vertex.v.y += pLevel.FloorHeight( Vertex.v );
	Vertex.v.x += pLevel.FloorOffsetX( Vertex.v );
	Vertex.v.z += pLevel.FloorOffsetY( Vertex.v );
	Vertex.n = pLevel.FloorNormal( Vertex.v );
	Vertex.tu = 1;
	Vertex.tv = 1;
	Vertex.color = pLevel.Template()->WaterColor();
	Vertices.push_back( Vertex );

	FaceIndices.push_back( StartingIndex );
	FaceIndices.push_back( StartingIndex + 1 );
	FaceIndices.push_back( StartingIndex + 2 );

	FaceIndices.push_back( StartingIndex + 1 );
	FaceIndices.push_back( StartingIndex + 3 );
	FaceIndices.push_back( StartingIndex + 2 );
} // CLevelMeshPatch::AddWaterTile()

void CLevelMeshPatch::CreateWater( CLevel& pLevel,					// parent level
								   LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	CMaze* pMaze = pLevel.Maze();

	D3DXVECTOR3 Position( m_Position );
	D3DXVECTOR3 FinalPosition( Position );
	std::vector< uint32 >			FaceIndices;
	std::vector< LITPLAINVERTEXSCENE >	Vertices;

	uint32 VertexIndex( 0 );
	for( uint32 X = 0; X < KPatchTiles; X ++ )
	{
		for( uint32 Y = 0; Y < KPatchTiles; Y ++ )
		{

			if( pMaze->HasWater( X + m_TopLeftCellX, Y + m_TopLeftCellY ) )
			{
				float32 HeightOffset = pLevel.Template()->WaterDepth();
				Position = m_Position + D3DXVECTOR3( (float32)X * m_TileWidth, HeightOffset, (float32)Y * -m_TileWidth );

				AddWaterTile( pLevel, Vertices, FaceIndices, Position );
			}
		}
	}
	if( Vertices.size() == 0 )
	{
		return;
	}

	if( FaceIndices.size() == 0 )
	{
		return;
	}

	m_WaterVertices = Vertices.size();
	// create the vertex buffer
	HRESULT hr = pD3DDevice->CreateVertexBuffer( m_WaterVertices * sizeof( LITPLAINVERTEXSCENE ),
												 D3DUSAGE_WRITEONLY,
												 D3DFVF_LITPLAINVERTEXSCENE,
												 D3DPOOL_MANAGED,
												 &m_pWaterVB );

	if ( FAILED( hr ) )
	{
		return;
	}


	// Fill vertex buffer
	LITPLAINVERTEXSCENE*	pVertices;
	hr = m_pWaterVB->Lock( 0, 
						   0, 
						   ( BYTE * * ) &pVertices, 
						   NULL );
	if ( FAILED( hr ) )
	{
		return;
	}
	// plug in our vert list
	for( uint32 i = 0; i < m_WaterVertices; i++ )
	{
		*pVertices = Vertices[i];
		m_WaterPoints.push_back( Vertices[i].v );
		pVertices++;		
	}
	m_pWaterVB->Unlock();


	m_WaterIndices = FaceIndices.size();

	// Create the index buffer
	WORD*	pIndices;
	hr = pD3DDevice->CreateIndexBuffer( m_WaterIndices * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pWaterIB );
	if ( FAILED( hr ) )
	{
		return;
	}

	// Fill the index buffer
	m_pWaterIB->Lock( 0, 
					  m_WaterIndices * sizeof( WORD ), 
					  ( BYTE * * ) &pIndices, 
					  0 );

	if ( FAILED( hr ) )
	{
		return;
	}
	// plug in our indices
	for( i = 0; i < m_WaterIndices; i++ )
	{
		*pIndices++ = ( WORD ) ( FaceIndices[i] );
	}


	m_pWaterIB->Unlock();
} // CLevelMeshPatch::CreateWater()

void CLevelMeshPatch::UpdateWater( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
								   float32 SinPosition )			// position of sinewave
{
	if( m_WaterVertices == 0 )
	{
		return;
	}
	// Fill vertex buffer
	LITPLAINVERTEXSCENE*	pVertices;
	HRESULT	hr = m_pWaterVB->Lock( 0, 
								   0, 
								   ( BYTE * * ) &pVertices, 
									NULL );
	if ( FAILED( hr ) )
	{
		return;
	}
	D3DXVECTOR3 FinalVertex;
	// udpate water vertex positions
	for( uint32 i = 0; i < m_WaterVertices; i++ )
	{
		FinalVertex = m_WaterPoints[i];
		FinalVertex.y += sin( SinPosition + m_WaterPoints[i].x * 1.5f );
		FinalVertex.y += cos( SinPosition + m_WaterPoints[i].z * 1.5f );
		pVertices->v = FinalVertex;

		float32 DeltaX = sin( SinPosition + m_WaterPoints[i].x * 1.5f + 1.5F ) -
						 sin( SinPosition + m_WaterPoints[i].x * 1.5f - 1.5F );
		float32 DeltaZ = cos( SinPosition + m_WaterPoints[i].z * 1.5f + 1.5F ) -
						 cos( SinPosition + m_WaterPoints[i].z * 1.5f - 1.5F );

		FinalVertex = D3DXVECTOR3( -DeltaX, 4.0f, DeltaZ );
		D3DXVec3Normalize( &FinalVertex, &FinalVertex );
		pVertices->n = FinalVertex;

		pVertices++;		
	}
	m_pWaterVB->Unlock();

} // CLevelMeshPatch::UpdateWater()

void CLevelMeshPatch::CreateWalls( CLevel& pLevel,					// parent level
								   LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if( m_Vertices.size() == 0 )
	{
		return;
	}

	std::vector <LITPLAINVERTEXSCENE> VBVertices ( m_Vertices.size() );


	for ( uint32 k = 0; k < m_VertexReferences.size(); k++ )
	{
		// if this vert hasn't been filled into the list yet, fill it out
		if( m_Vertices[m_VertexReferences[k]] != VBVertices[m_VertexReferences[k]].v )
		{
			VBVertices[m_VertexReferences[k]].v = m_Vertices[m_VertexReferences[k]];
			VBVertices[m_VertexReferences[k]].n = m_Normals[m_NormalReferences[k]];
			VBVertices[m_VertexReferences[k]].tu = m_UVs[m_UVReferences[k]].x;
			VBVertices[m_VertexReferences[k]].tv = m_UVs[m_UVReferences[k]].y;	
		}
		else
		{
			// if this vertex doesn't match its index, we have to make a new vert, and give it a new index
			if( !( VBVertices[m_VertexReferences[k]].v == m_Vertices[m_VertexReferences[k]] &&
				   VBVertices[m_VertexReferences[k]].n == m_Normals[m_NormalReferences[k]] &&
				   VBVertices[m_VertexReferences[k]].tu == m_UVs[m_UVReferences[k]].x &&
				   VBVertices[m_VertexReferences[k]].tv == m_UVs[m_UVReferences[k]].y ) )
			{
				LITPLAINVERTEXSCENE NewVertex;
				NewVertex.v = m_Vertices[m_VertexReferences[k]];
				NewVertex.n = m_Normals[m_NormalReferences[k]];
				NewVertex.tu = m_UVs[m_UVReferences[k]].x;
				NewVertex.tv = m_UVs[m_UVReferences[k]].y;

				// assign a new vertex reference
				m_VertexReferences[k] = VBVertices.size();
				// and put the new vert in the list
				VBVertices.push_back( NewVertex );
			}
		}
	}


	m_WallVertices	= VBVertices.size();

	// recalculate upward facing normals to make them follow the ground,
	// and take into account any 'rippling' that we did
	for( k = 0; k < m_WallVertices; k++ )
	{
		if( VBVertices[k].n.y > .8f )
		{
			VBVertices[k].n = pLevel.FloorNormal( VBVertices[k].v );
		}

		if( VBVertices[k].v.y > pLevel.TileHeight() ||
			VBVertices[k].v.y <  0 )
		{
			float32 Level = 255;
			if( VBVertices[k].v.y < 0 )
			{
				Level = (float32)fabs( VBVertices[k].v.y );
				Level -= pLevel.Template()->DarkStart();
				Level /= ( pLevel.Template()->DarkEnd() - pLevel.Template()->DarkStart() );
				if( Level > 1 )
				{
					Level = 1;
				}
				if( Level < 0 )
				{
					Level = 0;
				}
				Level *= 255;
				Level = 255 - Level;
			}
			else
			{
				Level = (float32)fabs( VBVertices[k].v.y );
				if( Level > pLevel.TileHeight() + 4 )
				{
					Level = 64;
				}
				else
				{
					Level = 255;
				}
			}

			VBVertices[k].color = D3DCOLOR_RGBA( (int32)Level, (int32)Level, (int32)Level, 255 );
		}
		else
		{
			VBVertices[k].color = D3DCOLOR_RGBA( 255, 255, 255, 255 );
		}
	}

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( m_WallVertices * sizeof( LITPLAINVERTEXSCENE ),
												  D3DUSAGE_WRITEONLY,
												  D3DFVF_LITPLAINVERTEXSCENE,
												  D3DPOOL_MANAGED,
												  &m_pWallVB );


	LITPLAINVERTEXSCENE*	pVertices;
	if ( FAILED( m_pWallVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	for ( k = 0; k < m_WallVertices; k++ )
	{
		*pVertices = VBVertices[k];
		pVertices++;
	}

	m_pWallVB->Unlock();

	for( uint32 i = 0; i < m_WallLayers; i++ )
	{
		CreateWallLayer( i, pD3DDevice );
	}
} // CLevelMeshPatch::CreateWalls()

void CLevelMeshPatch::CreateWallLayer( uint32 Layer,					// layer to generate
									  LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{

	uint32 Triangles( 0 );
	for( uint32 i = 0; i < m_Triangles.size(); i++ )
	{
		if( m_Triangles[i].w == Layer )
		{
			Triangles++;
		}
	}

	if( Triangles == 0 )
	{
		return;
	}

	m_WallIndices[Layer] = Triangles * 3;

	// create an IB that is as large as we might possibly go ( no vert refs, and all double-sided )
	HRESULT	hr = pD3DDevice->CreateIndexBuffer( m_WallIndices[Layer] * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pWallIB[Layer] );

	// Create the index buffer
	WORD*	pIndices;

	hr = m_pWallIB[Layer]->Lock( 0, m_WallIndices[Layer] * sizeof( WORD ), ( BYTE * * ) &pIndices, 0 );

	if ( FAILED( hr ) )
	{
		m_pWallIB[Layer]->Unlock();
		return;
	}

	// Fill in indices
	for ( uint32 k = 0; k < m_Triangles.size(); k++ )
	{
		if( m_Triangles[k].w == Layer )
		{
			*pIndices++ = ( WORD )
						m_VertexReferences[( int32 )m_Triangles[k].x];
			*pIndices++ = ( WORD )
						m_VertexReferences[( int32 )m_Triangles[k].y];
			*pIndices++ = ( WORD )
						m_VertexReferences[( int32 )m_Triangles[k].z];
		}
	}

	m_pWallIB[Layer]->Unlock();


} // CLevelMeshPatch::CreateWallLayer()

void CLevelMeshPatch::RenderFloor( uint32 Layer,						// layer to render
								   LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								   CLightManager* pLightManager,		// light manager - NULL for no light manager
								   const D3DXMATRIX& ViewportMatrix )	// viewport matrix
{
	assert( Layer < m_GroundLayers );

	if( m_FloorIndices[Layer] > 0 )
	{
		pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );

		pD3DDevice->SetVertexShader( D3DFVF_LITPLAINVERTEXSCENE );
		pD3DDevice->SetStreamSource( 0, m_pFloorVB[Layer], sizeof( LITPLAINVERTEXSCENE ) );
		pD3DDevice->SetIndices( m_pFloorIB[Layer], 0L );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										m_FloorVertices[Layer],
										0,
										m_FloorIndices[Layer] / 3 );
	}
} // CLevelMeshPatch::RenderFloor()

void CLevelMeshPatch::RenderCeiling( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								     CLightManager* pLightManager,		// light manager - NULL for no light manager
								     const D3DXMATRIX& ViewportMatrix )	// viewport matrix
{

	if( m_CeilingIndices > 0 )
	{
		pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );

		pD3DDevice->SetVertexShader( D3DFVF_LITPLAINVERTEXSCENE );
		pD3DDevice->SetStreamSource( 0, m_pCeilingVB, sizeof( LITPLAINVERTEXSCENE ) );
		pD3DDevice->SetIndices( m_pCeilingIB, 0L );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										m_CeilingVertices,
										0,
										m_CeilingIndices / 3 );
	}
} // CLevelMeshPatch::RenderCeiling()

void CLevelMeshPatch::RenderWalls( uint32 Layer,						// layer to render
								   LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								   CLightManager* pLightManager,		// light manager - NULL for no light manager
								   const D3DXMATRIX& ViewportMatrix )	// viewport matrix
{

	if( m_WallIndices[Layer] > 0 )
	{
		pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );
		pD3DDevice->SetVertexShader( D3DFVF_LITPLAINVERTEXSCENE );
		pD3DDevice->SetStreamSource( 0, m_pWallVB, sizeof( LITPLAINVERTEXSCENE ) );
		pD3DDevice->SetIndices( m_pWallIB[Layer], 0L );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										m_WallVertices,
										0,
										m_WallIndices[Layer] / 3 );
	}
} // CLevelMeshPatch::RenderWalls()

void CLevelMeshPatch::RenderVisibility( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								    const D3DXMATRIX& ViewportMatrix )	// viewport matrix
{

	if( m_VisibilityIndices > 0 && !m_VisibilityEmpty )
	{
		// Visibility is totally opaque - no need to do expensive alpha blending
		if( m_VisibilityFull )
		{
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
		}
		pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );

		pD3DDevice->SetVertexShader( D3DFVF_LITPLAINVERTEXSCENE );
		pD3DDevice->SetStreamSource( 0, m_pVisibilityVB, sizeof( LITPLAINVERTEXSCENE ) );
		pD3DDevice->SetIndices( m_pVisibilityIB, 0L );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										m_VisibilityVertices,
										0,
										m_VisibilityIndices / 3 );
		if( m_VisibilityFull )
		{
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
		}
	}
} // CLevelMeshPatch::RenderVisibility()

void CLevelMeshPatch::RenderMiniMap( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								     const D3DXMATRIX& ViewportMatrix )	// viewport matrix
{

	if( m_MiniMapIndices > 0 )
	{
		// MiniMap is totally opaque - no need to do expensive alpha blending
		pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );

		pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );
		pD3DDevice->SetStreamSource( 0, m_pMiniMapVB, sizeof( PLAINVERTEXMODEL ) );
		pD3DDevice->SetIndices( m_pMiniMapIB, 0L );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										m_MiniMapVertices,
										0,
										m_MiniMapIndices / 3 );
	}
} // CLevelMeshPatch::RenderMiniMap()

void CLevelMeshPatch::RenderWater( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								   CLightManager* pLightManager,		// light manager - NULL for no light manager
								   const D3DXMATRIX& ViewportMatrix )	// viewport matrix
{

	if( m_WaterIndices > 0 )
	{
		pD3DDevice->SetTransform( D3DTS_VIEW, &ViewportMatrix );

		pD3DDevice->SetVertexShader( D3DFVF_LITPLAINVERTEXSCENE );
		pD3DDevice->SetStreamSource( 0, m_pWaterVB, sizeof( LITPLAINVERTEXSCENE ) );
		pD3DDevice->SetIndices( m_pWaterIB, 0L );
		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										0,
										m_WaterVertices,
										0,
										m_WaterIndices / 3 );
	}
} // CLevelMeshPatch::RenderWater()
