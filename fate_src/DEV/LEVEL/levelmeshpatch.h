/*
	LevelMeshPatch.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVELMESHPATCH_H
#define _LEVELMESHPATCH_H

#include "d3dx8_shim.h"

#include "../UTILITIES/vertexdefinitions.h"
#include "level.h"
// how many tiles wide/high per patch?
const uint32 KPatchTiles( 4 );
const uint32 KPatchVisibilityTiles( KPatchTiles * KVisibilityCellsPerTile );

class CRefManager;
class CSettings;
class CFrustum;
class CCullingBounds;
class CModelTemplate;
class CFaceList;
class CLevel;

// a LevelMeshPatch contains a model, which is its visual representation,
// as well as data about contents and passability.

class CLevelMeshPatch
{


public:

	CLevelMeshPatch( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
					 CRefManager& pRefManager,			// pointer to the reference manager, for tracking media
					 CSettings& pSettings,				// pointer to the settings manager, for keeping track of card/machine capabilities
					 const D3DXVECTOR3& Position,		// top-left corner position
					 int32 TopLeftCellX,				// top left cell index
					 int32 TopLeftCellY,				// top left cell index
					 uint32 GroundLayers,				// number of layers possible for ground textures
					 uint32 WallLayers,					// number of layers possible for walls
					 float32 TileWidth,					// width of individual tile, in world units
					 float32 TileHeight  );				// height (tallness) of individual tile
	~CLevelMeshPatch( void );
	
	void		RenderFloor( uint32 Layer,							// layer to render
							 LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 CLightManager* pLightManager,			// light manager - null for no light manager
							 const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderCeiling( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							   CLightManager* pLightManager,			// light manager - null for no light manager
							   const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderWalls( uint32 Layer,							// layer to render
							 LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 CLightManager* pLightManager,			// light manager - null for no light manager
							 const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderWater( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 CLightManager* pLightManager,			// light manager - null for no light manager
							 const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderVisibility( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderMiniMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	D3DXVECTOR3*	GetCullingBounds( void )		{	return m_CullingBounds;	};

	const D3DXVECTOR3& MaxBounds( void )			{	return m_MaxBounds;		};
	const D3DXVECTOR3& MinBounds( void )			{	return m_MinBounds;		};

	bool			HasFloorTiles( uint32 Layer )			{	return m_FloorIndices[Layer] > 0;		};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////


	void		AddWall( CLevel& pLevel,				// parent level
						 uint32 Layer,					// wall layer
						 const D3DXVECTOR3& Position,	// position to add at
						 CModelTemplate* pTemplate,		// template to add
						 CModelTemplate* pCollisionTemplate = NULL );	// template to add

	void		Create( CLevel& pLevel,						// parent level
						LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void		SupplyCollision( CFaceList& pFaceList );	// face list to add collision data to

	void		UpdateVisibility( CLevel& pLevel,					// parent level
								  LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								  float32 TimeElapsed );			// time elapsed in seconds

	void		UpdateMiniMap( CLevel& pLevel,					// parent level
							   LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		UpdateWater( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							 float32 SinPosition );			// position of sinewave

	void		CalculateBounds( void )
	{
		m_CullingBounds[0] = D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MinBounds.z );
		m_CullingBounds[1] = D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MinBounds.z );
		m_CullingBounds[2] = D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MinBounds.z );
		m_CullingBounds[3] = D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MinBounds.z );
		m_CullingBounds[4] = D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MaxBounds.z );
		m_CullingBounds[5] = D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MaxBounds.z );
		m_CullingBounds[6] = D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MaxBounds.z );
		m_CullingBounds[7] = D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MaxBounds.z );

	} // CalculateBounds()


private:

	void		SetVisibilityTile( LITPLAINVERTEXSCENE*	pVertices,	// vertex list
								   int32 X,							// x tile
								   int32 Y,							// y tile
								   ETerrainType Type );				// tile type

	void		SetMiniMapTile( PLAINVERTEXMODEL*	pVertices,	// vertex list
							    int32 X,						// x tile
							    int32 Y,						// y tile
							    ETerrainType Type );			// tile type

	void		AddTile( CLevel& pLevel,							// parent level
					     uint32 Layer,							// map layer
						 int32 CellX,								// maze cell
					     int32 CellY,								// maze cell
						 std::vector< LITPLAINVERTEXSCENE >& Vertices,	// vertices to add to
					     std::vector< uint32 >& FaceIndices,		// face indices to add to
						 ETerrainType Type,							// tile type
						 const D3DXVECTOR3& TopLeftPosition );		// position of top-left corner 

	void		AddCeilingTile( CLevel& pLevel,							// parent level
								int32 CellX,								// maze cell
								int32 CellY,								// maze cell
								std::vector< LITPLAINVERTEXSCENE >& Vertices,	// vertices to add to
								std::vector< uint32 >& FaceIndices,		// face indices to add to
								ETerrainType Type,							// tile type
								const D3DXVECTOR3& TopLeftPosition );		// position of top-left corner 
	
	void		AddWaterTile( CLevel& pLevel,							// parent level
								std::vector< LITPLAINVERTEXSCENE >& Vertices,	// vertices to add to
								std::vector< uint32 >& FaceIndices,		// face indices to add to
								const D3DXVECTOR3& TopLeftPosition );		// position of top-left corner 

	void		CreateWater( CLevel& pLevel,					// parent level
							 LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		CreateFloor( CLevel& pLevel,					// parent level
							 LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		CreateCeilingLayer( CLevel& pLevel,					// parent level
									LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		CreateFloorLayer( uint32 Layer,						// layer to create
								  CLevel& pLevel,					// parent level
								  LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device


	void		CreateVisibility( CLevel& pLevel,					// parent level
								  LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		CreateMiniMap( CLevel& pLevel,					// parent level
							   LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		CreateWalls( CLevel& pLevel,					// parent level
							 LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		CreateWallLayer( uint32 WallType,
								 LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	D3DXVECTOR3			m_Position;

	float32				m_TileWidth;

	float32				m_TileHeight;

	uint32				m_GroundLayers;
	uint32				m_WallLayers;

	std::vector< uint32 >	m_FloorVertices;
	std::vector< uint32	>	m_FloorIndices;

	uint32					m_WallVertices;
	std::vector< uint32 >	m_WallIndices;


	uint32					m_VisibilityTilesWide;
	uint32					m_VisibilityTilesHigh;

	uint32					m_VisibilityVertices;
	uint32					m_VisibilityIndices;

	uint32					m_MiniMapVertices;
	uint32					m_MiniMapIndices;

	uint32					m_WaterVertices;
	uint32					m_WaterIndices;

	uint32					m_CeilingVertices;
	uint32					m_CeilingIndices;

	int32					m_TopLeftCellX;
	int32					m_TopLeftCellY;

	LPDIRECT3DINDEXBUFFER8		m_pVisibilityIB;
	LPDIRECT3DVERTEXBUFFER8		m_pVisibilityVB;

	LPDIRECT3DINDEXBUFFER8		m_pWaterIB;
	LPDIRECT3DVERTEXBUFFER8		m_pWaterVB;

	std::vector< LPDIRECT3DINDEXBUFFER8 >	m_pFloorIB;
	std::vector< LPDIRECT3DVERTEXBUFFER8 >	m_pFloorVB;


	std::vector< LPDIRECT3DINDEXBUFFER8	>	m_pWallIB;
	LPDIRECT3DVERTEXBUFFER8					m_pWallVB;

	LPDIRECT3DINDEXBUFFER8		m_pMiniMapIB;
	LPDIRECT3DVERTEXBUFFER8		m_pMiniMapVB;

	LPDIRECT3DINDEXBUFFER8		m_pCeilingIB;
	LPDIRECT3DVERTEXBUFFER8		m_pCeilingVB;

	std::vector <D3DXVECTOR3>						m_Vertices;
	std::vector <uint32>							m_VertexReferences;
	std::vector <D3DXVECTOR4>						m_Triangles;
	std::vector <uint32>							m_UVReferences;
	std::vector <D3DXVECTOR3>						m_UVs;
	std::vector <D3DXVECTOR3>						m_Normals;
	std::vector <uint32>							m_NormalReferences;

	std::vector <D3DXVECTOR3>						m_CollisionVertices;
	std::vector <uint32>							m_CollisionVertexReferences;
	std::vector <D3DXVECTOR4>						m_CollisionTriangles;


	bool						m_VisibilityEmpty;	// has no partially transparent
	bool						m_VisibilityFull;	// has ONLY opaque pieces

	std::vector <D3DXVECTOR3>						m_WaterPoints;

	D3DXVECTOR3				m_MaxBounds;	// maximum local Bounds of the mesh
	D3DXVECTOR3				m_MinBounds;	// minimum local Bounds of the mesh
	D3DXVECTOR3				m_CullingBounds[8];

	int32					m_VisibilityLevel[KPatchVisibilityTiles+2][KPatchVisibilityTiles+2];
	float32					m_FloatVisibilityLevel[KPatchVisibilityTiles+2][KPatchVisibilityTiles+2];
	ETerrainType			m_MiniMapCornerType[KPatchTiles+4][KPatchTiles+4];

};

#endif