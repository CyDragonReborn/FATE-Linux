/*
	levelmesh.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <assert.h>
#include <D3D8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/capabilities.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/cullingbounds.h"
#include "../UTILITIES/quadtree.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../UI/sprite.h"

#include "../MODEL/facelist.h"

#include "../GAMEOBJECTS/lightmanager.h"

#include "levelquadtree.h"
#include "level.h"

#include "template.h"
#include "modeltemplate.h"
#include "levelmeshpatch.h"
#include "levelmesh.h"

CLevelMesh::CLevelMesh( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					    CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
						CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
						CTemplate& pTemplate,			// level template
						uint32 CellsWide,				// cells wide ( must be divisible by 16 )
						uint32 CellsHigh,				// cells high ( must be divisible by 16 )
						float32 CellWidth ) :			// cell width, in worldspace units
												m_pRefManager( pRefManager ),
												m_pSettings( pSettings ),
												m_pTemplate( pTemplate ),
												m_CellsWide( CellsWide ),
												m_CellsHigh( CellsHigh ),
												m_CellWidth( CellWidth ),
												m_pQuadTree( NULL ),
												m_pShadowMaterial( NULL ),
												m_pMiniMapMaterial( NULL ),
												m_pWaterMaterial( NULL ),
												m_UpdateVisibility( kFalse ),
												m_WaterOffset( 0 ),
												m_LastTimeElapsed( 0 ),
												m_PatchesRendered( 0 )
{
	ZeroMemory ( &m_DungeonD3DMaterial, sizeof ( m_DungeonD3DMaterial ));
	ZeroMemory ( &m_ProjectorBias, sizeof ( m_ProjectorBias ));

	assert( CellsWide % KPatchTiles == 0 );
	assert( CellsHigh % KPatchTiles == 0 );

	Create( pD3DDevice );

	if( m_pTemplate.HasWater() )
	{
		m_pWaterMaterial = m_pRefManager.AddMaterial( pD3DDevice, 
													  m_pTemplate.WaterPath(),
													  m_pTemplate.WaterPathAlpha(),
													  m_pTemplate.WaterPathAVI(),
													  "",
													  "" );
		m_pWaterMaterial->SetAnimationShifting( 0, -.05f, -.075f );
		m_pWaterMaterial->SetAnimationShifting( 1, .025f, .035f );

	}

//	m_pShadowMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice, "IMAGES//fog.png", "IMAGES//foga.png" );
	if( m_pSettings.GetSettings( KSetAnimatedFog ) )
	{
		m_pShadowMaterial = m_pRefManager.AddMaterial( pD3DDevice, "WATER/dark.avi" );
	}
	else
	{
		m_pShadowMaterial = m_pRefManager.AddMaterial( pD3DDevice, "IMAGES/black.png" );
	}

	m_pMiniMapMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice, 
														 "IMAGES/minimap.png",
														 "IMAGES/minimapa.png" );


	m_DungeonD3DMaterial.Diffuse.r	= 1.0f;					
	m_DungeonD3DMaterial.Diffuse.g	= 1.0f;
	m_DungeonD3DMaterial.Diffuse.b	= 1.0f;
	m_DungeonD3DMaterial.Diffuse.a	= 1.0f;

	m_DungeonD3DMaterial.Specular.r	= 0.0F;
	m_DungeonD3DMaterial.Specular.g	= 0.0F;
	m_DungeonD3DMaterial.Specular.b	= 0.0F;

	m_DungeonD3DMaterial.Power		= 0.0f;

	m_DungeonD3DMaterial.Ambient.r	= 1.0f;						
	m_DungeonD3DMaterial.Ambient.g	= 1.0f;
	m_DungeonD3DMaterial.Ambient.b	= 1.0f;
	m_DungeonD3DMaterial.Ambient.a	= 1.0f;

	m_DungeonD3DMaterial.Emissive.r = 0.0f;
	m_DungeonD3DMaterial.Emissive.g = 0.0f;
	m_DungeonD3DMaterial.Emissive.b = 0.0f;
	m_DungeonD3DMaterial.Emissive.a = 0.0f;

	// set up a projector bias for use in the default reflection projections
	float32	Offset	= .5f + ( .5f / 256 );

	m_ProjectorBias._11 = 0.5f; 
	m_ProjectorBias._12 = 0.0f; 
	m_ProjectorBias._13 = 0.0f; 
	m_ProjectorBias._14 = 0;

	m_ProjectorBias._21 = 0.0f; 
	m_ProjectorBias._22 = -0.5f; 
	m_ProjectorBias._23 = 0.0f; 
	m_ProjectorBias._24 = 0;

	m_ProjectorBias._31 = 0.0f; 
	m_ProjectorBias._32 = 0.0f; 
	m_ProjectorBias._33 = 1.0f; 
	m_ProjectorBias._34 = 0;

	m_ProjectorBias._41 = Offset; 
	m_ProjectorBias._42 = Offset; 
	m_ProjectorBias._43 = 0.0f; 
	m_ProjectorBias._44 = 1.0f;

} // CLevelMesh::CLevelMesh()

CLevelMesh::~CLevelMesh( void )
{

	m_pRefManager.RemoveMaterial( m_pShadowMaterial );
	m_pRefManager.RemoveMaterial( m_pMiniMapMaterial );
	m_pRefManager.RemoveMaterial( m_pWaterMaterial );

	for( uint32 i = 0; i < m_pLevelPatches.size(); i++ )
	{
		for( uint32 j = 0; j < m_pLevelPatches[i].size(); j++ )
		{
			DELETE_SAFELY( m_pLevelPatches[i][j] );
		}
		m_pLevelPatches[i].clear();
	}
	m_pLevelPatches.clear();

	m_pSortedPatches.clear();
	
	DELETE_SAFELY( m_pQuadTree );

} // CLevelMesh::~CLevelMesh()

void CLevelMesh::Update( float32 TimeElapsed )	// time elapsed in seconds
{
	m_LastTimeElapsed = TimeElapsed;
	m_WaterOffset += TimeElapsed * 2.0f;
	while( m_WaterOffset > (float32)KPI * 2.0f )
	{
		m_WaterOffset -= (float32)KPI * 2.0f;
	}
} // CLevelMesh::Update()

void CLevelMesh::Create( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	D3DXVECTOR3 LevelMinBounds( -(float32)KPatchTiles * m_CellWidth,
								0,
								( m_CellsHigh + 1 ) * (float32)KPatchTiles * -m_CellWidth );
	D3DXVECTOR3 LevelMaxBounds( ( m_CellsWide + 1 ) * (float32)KPatchTiles * m_CellWidth,
								100,
								(float32)KPatchTiles * m_CellWidth );

	m_pQuadTree = new CLevelQuadtree( LevelMinBounds,
									  LevelMaxBounds,
									  m_CellWidth );


	uint32 PatchesWide = m_CellsWide / KPatchTiles;
	uint32 PatchesHigh = m_CellsHigh / KPatchTiles;

	m_pLevelPatches.resize( PatchesWide );

	for( uint32 i = 0; i < PatchesWide; i++ )
	{
		for( uint32 j = 0; j < PatchesHigh; j++ )
		{
			D3DXVECTOR3 Position( (float32)i * (float32)KPatchTiles * m_CellWidth, 
								  0, 
								  (float32)j * (float32)KPatchTiles * -m_CellWidth );


			CLevelMeshPatch* pPatch = new CLevelMeshPatch( pD3DDevice,
														   m_pRefManager,
														   m_pSettings,
														   Position,
														   i * KPatchTiles,
														   j * KPatchTiles,
														   m_pTemplate.GroundLayers(),
														   m_pTemplate.TileTypes(),
														   m_pTemplate.TileWidth(),
														   m_pTemplate.TileHeight() );

			m_pLevelPatches[i].push_back( pPatch );
		}
	}
} // CLevelMesh::Create()

void CLevelMesh::Finalize( CLevel& pLevel,					// parent level
						   LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	uint32 PatchesWide = m_CellsWide / KPatchTiles;
	uint32 PatchesHigh = m_CellsHigh / KPatchTiles;

	for( uint32 i = 0; i < PatchesWide; i++ )
	{
		for( uint32 j = 0; j < PatchesHigh; j++ )
		{
			m_pLevelPatches[i][j]->Create( pLevel, pD3DDevice );
			m_pQuadTree->AddData( m_pLevelPatches[i][j],
								  m_pLevelPatches[i][j]->MinBounds(),
								  m_pLevelPatches[i][j]->MaxBounds() );
		}
	}

} // CLevelMesh::Finalize()

void CLevelMesh::AddWall( CLevel& pLevel,				// parent level
						  uint32 Type,					// type of wall
						  const D3DXVECTOR3& Position,	// position to add at
						  CModelTemplate* pTemplate,	// template to add
						  CModelTemplate* pCollisionTemplate )	// template to add
{

	int32 XPatch = (int32)floor( Position.x / ( (float32)KPatchTiles * m_CellWidth ) );
	int32 YPatch = (int32)floor( -Position.z / ( (float32)KPatchTiles * m_CellWidth ) );

	if( XPatch >= 0 && YPatch >= 0 )
	{
		m_pLevelPatches[XPatch][YPatch]->AddWall( pLevel,
												  Type,
												  Position,	
												  pTemplate,
												  pCollisionTemplate );	
	}
} // CLevelMesh::AddWall()

void CLevelMesh::SupplyCollision( CFaceList& pFaceList )	// face list to add collision data to
{
	uint32 PatchesWide = m_CellsWide / KPatchTiles;
	uint32 PatchesHigh = m_CellsHigh / KPatchTiles;

	for( uint32 i = 0; i < PatchesWide; i++ )
	{
		for( uint32 j = 0; j < PatchesHigh; j++ )
		{
			m_pLevelPatches[i][j]->SupplyCollision( pFaceList );
		}
	}

} // CLevelMesh::SupplyCollision()

// sort for rendering
void CLevelMesh::Sort( CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	if( pFrustum != NULL )
	{
		m_pSortedPatches.resize( 0 );
		m_pQuadTree->Sort( *pFrustum,
						   m_pSortedPatches );
	}
} // CLevelMesh::Sort()

void CLevelMesh::UpdateLevelWideVisibility( CLevel& pLevel,						// parent level
											LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	uint32 PatchesWide = m_CellsWide / KPatchTiles;
	uint32 PatchesHigh = m_CellsHigh / KPatchTiles;
		// render visibility shadows (fog of war)
	for( uint32 i = 0; i < PatchesWide; i++ )
	{
		for( uint32 j = 0; j < PatchesHigh; j++ )
		{
			m_pLevelPatches[i][j]->UpdateVisibility( pLevel,
													 pD3DDevice,
													 1000 );
			m_pLevelPatches[i][j]->UpdateMiniMap( pLevel,
											   pD3DDevice );
		}
	}

} // CLevelMesh::UpdateLevelWideVisibility()


void CLevelMesh::Render( CLevel& pLevel,						// parent level
						 LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						 CLightManager* pLightManager,			// light manager - NULL for no light manager
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						 CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	
//	pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

	D3DXMATRIX	Identity;
	D3DXMatrixIdentity( &Identity );
	pLightManager->UpdateLights( pD3DDevice, Identity );

	Sort( pFrustum );

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	m_PatchesRendered = m_pSortedPatches.size();

	pD3DDevice->SetMaterial( &m_DungeonD3DMaterial );
//	if( m_UpdateVisibility )
	{
	//	m_UpdateVisibility = kFalse;
		// render visibility shadows (fog of war)
		for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
		{
			m_pSortedPatches[i]->UpdateVisibility( pLevel,
												   pD3DDevice,
												   m_LastTimeElapsed );
			m_pSortedPatches[i]->UpdateMiniMap( pLevel,
												pD3DDevice );
		}

	}

	if( m_pTemplate.HasWater() )
	{
		for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
		{
			m_pSortedPatches[i]->UpdateWater( pD3DDevice,
											m_WaterOffset );
		}
	}
	//pD3DDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );

	// the floor doesn't z-Write at all

	for( uint32 layer = 0; layer < m_pTemplate.GroundLayers(); layer++ )
	{

		if( layer == 0 )	// base layers are not alpha blended
		{
			pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		}
		else	// non-base layers are alpha blended
		{
			pD3DDevice->SetRenderState( D3DRS_ZBIAS, 5 );
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		}
		// set the texture for this layer
		CMaterial* pMaterialReference = m_pTemplate.GroundMaterial( layer );
		if( pMaterialReference != NULL )
		{
			pD3DDevice->SetTexture( 0, pMaterialReference->GetTexture() );
		}

		for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
		{
			m_pSortedPatches[i]->RenderFloor( layer,
											  pD3DDevice,
											  pLightManager,
											  ViewportMatrix );
		}

		if( layer == 0 )	// render the ceiling
		{
			// set the texture for this layer
			pMaterialReference = m_pTemplate.GroundMaterial( m_pTemplate.CeilingTerrain() );
			if( pMaterialReference != NULL )
			{
				pD3DDevice->SetTexture( 0, pMaterialReference->GetTexture() );
			}

			for( int32 i = 0; i < m_pSortedPatches.size(); i++ )
			{
				m_pSortedPatches[i]->RenderCeiling( pD3DDevice,
												    pLightManager,
												    ViewportMatrix );
			}
		}

		// now we render any walls that correspond to this ground layer, for blending
		for( uint32 j = 0; j < m_pTemplate.TileTypes(); j++ )
		{
			pD3DDevice->SetRenderState( D3DRS_ZBIAS, 4 );
			if( m_pTemplate.TileGroundLayer( j ) == layer )
			{
				// alpha blending back off
				pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
				pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
				pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
				pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
				// z-write/read back on
				pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
				pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );


				pD3DDevice->SetTexture( 0, m_pTemplate.WallMaterial( j )->GetTexture() );//pMaterialReference2->GetTexture() );
				for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
				{
					m_pSortedPatches[i]->RenderWalls( j,
													  pD3DDevice,
													  pLightManager,
													  ViewportMatrix );
				}


			}
			pD3DDevice->SetRenderState( D3DRS_ZBIAS, 5 );
		}
	}



	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	// alpha blending back off
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );

//	pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
//	pD3DDevice->SetTextureStageState( 0,
//									  D3DTSS_MIPFILTER,
//									  m_pSettings.GetSettings( KSetMipFilter ) );	

} // CLevelMesh::Render()

void CLevelMesh::RenderWater( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							  CLightManager* pLightManager,			// light manager - NULL for no light manager
							  const D3DXMATRIX& ViewportMatrix )	// viewport matrix
							  
{
	if( !m_pTemplate.HasWater() )
	{
		return;
	}
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );
	D3DXMATRIX	Identity;
	D3DXMatrixIdentity( &Identity );
	pLightManager->UpdateLights( pD3DDevice, Identity );

	pD3DDevice->SetMaterial( &m_DungeonD3DMaterial );

	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// turn zwrite/read back on
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

	// render water elements
	pD3DDevice->SetTexture( 0, m_pWaterMaterial->GetTexture() );
	pD3DDevice->SetTexture( 0, m_pWaterMaterial->GetIlluminationTexture() );

	m_pWaterMaterial->SetAnimationLayerStates( pD3DDevice, 0 );
	for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderWater( pD3DDevice,
										  pLightManager,
										  ViewportMatrix );
	}

	m_pWaterMaterial->UnSetAnimationLayerStates( pD3DDevice, 0 );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );


	if( m_pSettings.Capabilities().IsCapable( KCanDoModulate2X ) )
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	}
	else
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	}

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_COUNT2 );

	D3DXMATRIX	matShift;
	D3DXMatrixIdentity( &matShift );

	// set the matrix using the shifting component of the material
	matShift._31 = m_pWaterMaterial->GetU( 1 );
	matShift._32 = m_pWaterMaterial->GetV( 1 );

	pD3DDevice->SetTransform( D3DTS_TEXTURE0, &matShift );


	// render water elements
	pD3DDevice->SetTexture( 0, m_pWaterMaterial->GetIlluminationTexture() );
	for( int32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderWater( pD3DDevice,
										  pLightManager,
										  ViewportMatrix );
	}

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_DISABLE );


	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	// alpha blending back off
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

} // CLevelMesh::RenderWater()

// assumes that the level has already been sorted
void CLevelMesh::RenderVisibility( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								   const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );

	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	if( m_pShadowMaterial->IsValid() )
	{
		pD3DDevice->SetTexture( 0, m_pShadowMaterial->GetTexture() );
	}

	// render visibility shadows (fog of war)
	for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderVisibility( pD3DDevice,
											   ViewportMatrix );
	}

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

	// alpha blending back off
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	// turn zwrite/read back on
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );


} // CLevelMesh::RenderVisibility()

// assumes that the level has already been sorted
void CLevelMesh::RenderVisibilityMask( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									   const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{
	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kTrue );
		pD3DDevice->SetRenderState( D3DRS_ALPHAREF, ( DWORD ) 50 );
		pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
	}	

	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	pD3DDevice->SetTexture( 0, m_pShadowMaterial->GetTexture() );
	
	// render visibility shadows (fog of war)
	for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderVisibility( pD3DDevice,
											   ViewportMatrix );
	}

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

	// alpha blending back off
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	if ( m_pSettings.GetSettings( KSetAlphaTest ) )
	{
		pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, kFalse );
	}	
	// turn zwrite/read back on
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

} // CLevelMesh::RenderVisibilityMask()

// assumes that the level has already been sorted
void CLevelMesh::RenderMiniMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								CLevel& pLevel )						// level to check visibility from
{
	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXMODEL );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	pD3DDevice->SetTexture( 0, m_pMiniMapMaterial->GetTexture() );
	
	// render MiniMap shadows (fog of war)
	for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderMiniMap( pD3DDevice,
											ViewportMatrix );
	}

	// we leave the state alone - this is only rendered in the UI,
	// so they're doing alpha already.

} // CLevelMesh::RenderMiniMap()

// render projected shadow maps for the mesh
void CLevelMesh::RenderShadowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								   const D3DXMATRIX& LightMatrix,			// matrix for the shadow caster
								   CMaterial* pShadowMaterial,				// projected shadow material
								   CMaterial* pLightMaterial,				// projected shadow material
								   bool Orthographic )						// false by default
{

	if( pShadowMaterial->IsDynamicTextureValid() )
	{
		pD3DDevice->SetTexture( 0, pShadowMaterial->GetDynamicTexture() );
	}
	else
	{
		pD3DDevice->SetTexture( 0, pShadowMaterial->GetTexture() );
	}

	D3DXMATRIX ProjectionMatrix;
	D3DXMATRIX InverseView;

	float32 FOV		= ( float32 )( 90 * KDeg2Rad );

	if( !m_pSettings.Capabilities().IsCapable( KCanDoShadowTextureProjection ) )
	{
		Orthographic = kTrue;
	}

	if( Orthographic )
	{
		if( m_pSettings.GetSettings( KSetShadowDetail ) == 0 )
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								125,
								125,
								.01f,
								2000 );	// far clip
		}
		else
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								50,
								50,
								.01f,
								2000 );	// far clip
		}
	}
	else
	{
		if( m_pSettings.GetSettings( KSetShadowDetail ) == 0 )
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										( float32 )( 120 * KDeg2Rad ), 
										1.0f, 
										.01f, 
										2000 );
		}
		else
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										FOV, 
										1.0f, 
										.01f, 
										2000 );
		}

	}

	D3DXMatrixInverse( &InverseView, NULL, &ViewportMatrix );

	InverseView = InverseView * LightMatrix * ProjectionMatrix * m_ProjectorBias;
		
	pD3DDevice->SetTransform( D3DTS_TEXTURE0, &InverseView );

	// now we need to set up layer 0 to do the sort of projection we want when it comes time

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSU,
									  D3DTADDRESS_CLAMP );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSV,
									  D3DTADDRESS_CLAMP );
		
	if( Orthographic )
	{
		pD3DDevice->SetTextureStageState( 0,
										D3DTSS_TEXTURETRANSFORMFLAGS,
										D3DTTFF_COUNT3 );
	}
	else
	{
		pD3DDevice->SetTextureStageState( 0,
										D3DTSS_TEXTURETRANSFORMFLAGS,
										D3DTTFF_COUNT3 |
										D3DTTFF_PROJECTED );
	}
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXCOORDINDEX,
									  D3DTSS_TCI_CAMERASPACEPOSITION |
									  1 );



	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );

	for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderFloor( 0,
										  pD3DDevice,
										  NULL,
										  ViewportMatrix );
	}


	if( pLightMaterial->IsDynamicTextureValid() )
	{
		pD3DDevice->SetTexture( 0, pLightMaterial->GetDynamicTexture() );
	}
	else
	{
		pD3DDevice->SetTexture( 0, pLightMaterial->GetTexture() );
	}

	for( int32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderCeiling( pD3DDevice,
											NULL,
											ViewportMatrix );
	}


	// now we render any walls that correspond to this ground layer, for blending
	pD3DDevice->SetRenderState( D3DRS_ZBIAS, 5 );

	for( uint32 j = 0; j < m_pTemplate.TileTypes(); j++ )
	{
		for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
		{
			m_pSortedPatches[i]->RenderWalls( j,
											  pD3DDevice,
											  NULL,
											  ViewportMatrix );
		}
	}


	pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_TEXTURETRANSFORMFLAGS,
									  D3DTTFF_DISABLE );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSU,
									  D3DTADDRESS_WRAP );
	pD3DDevice->SetTextureStageState( 0,
									  D3DTSS_ADDRESSV,
									  D3DTADDRESS_WRAP );


} // CLevelMesh::RenderShadowPass()


// render a glow pass to the z-buffer
void CLevelMesh::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								 const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

	for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
	{
		m_pSortedPatches[i]->RenderFloor( 0,
										  pD3DDevice,
										  NULL,
										  ViewportMatrix );
	}

	for( uint32 j = 0; j < m_pTemplate.TileTypes(); j++ )
	{
		for( uint32 i = 0; i < m_pSortedPatches.size(); i++ )
		{
			m_pSortedPatches[i]->RenderWalls( j,
											  pD3DDevice,
											  NULL,
											  ViewportMatrix );
		}
	}

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

} // CLevelMesh::RenderGlowPass()
