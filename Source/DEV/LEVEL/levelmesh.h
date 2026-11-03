/*
	levelmesh.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVELMESH_H
#define _LEVELMESH_H

#include "../UTILITIES/quadtree.h"


class CRefManager;
class CSettings;
class CFrustum;
class CCullingBounds;
class CLevelMeshPatch;
class CMaterial;
class CModelTemplate;
class CFaceList;
class CLevel;
class CTemplate;
class CLevelQuadtree;
class CLightManager;
class CSprite;

// a LevelMesh contains a model, which is its visual representation,
// as well as data about contents and passability.

class CLevelMesh
{


public:

	CLevelMesh( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
				CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
				CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
				CTemplate& pTemplate,			// level template
				uint32 CellsWide,				// cells wide ( must be divisible by 16 )
				uint32 CellsHigh,				// cells high ( must be divisible by 16 )
				float32 CellWidth );			// cell width, in worldspace units

	~CLevelMesh( void );
	
	void		Update( float32 TimeElapsed );	// time elapsed in seconds

	void		Sort( CFrustum* pFrustum );			// frustum to cull with - NULL for none

	void		Render( CLevel& pLevel,				// parent level
						LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						CLightManager* pLightManager,			// light manager - null for no light manager
					    const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								const D3DXMATRIX& ViewportMatrix );		// viewport matrix

	void		RenderWater( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 CLightManager* pLightManager,			// light manager - NULL for no light manager
							 const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderShadowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								  const D3DXMATRIX& LightMatrix,		// matrix for the shadow caster
								  CMaterial* pShadowMaterial,			// projected shadow material
								  CMaterial* pLightMaterial,			// projected shadow material
								  bool Orthographic = kFalse );			// false by default

	void		RenderVisibility( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderVisibilityMask( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
								  const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		RenderMiniMap( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   CLevel& pLevel );						// level to check visibility from

	void		Finalize( CLevel& pLevel,						// parent level
						  LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device
	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void		UpdateVisibility( void )		{	m_UpdateVisibility = kTrue;	};

	uint32		PatchesRendered( void )			{	return m_PatchesRendered;	};

	float32		WaterOffset( void )				{	return m_WaterOffset;		};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void		UpdateLevelWideVisibility( CLevel& pLevel,						// parent level
											LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void		AddWall( CLevel& pLevel,				// parent level
						 uint32 Type,					// type of wall
						 const D3DXVECTOR3& Position,	// position to add at
						 CModelTemplate* pTemplate,		// template to add
						 CModelTemplate* pCollisionTemplate );	// template to add

	void		SupplyCollision( CFaceList& pFaceList );	// face list to add collision data to

private:

	void		Create( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CTemplate&			m_pTemplate;

	uint32				m_CellsWide;
	uint32				m_CellsHigh;

	float32				m_CellWidth;

	float32				m_LastTimeElapsed;

	float32				m_WaterOffset;	// sinusoidal offset for water rippling

	CLevelQuadtree*		m_pQuadTree;

	std::vector< std::vector< CLevelMeshPatch* > >		m_pLevelPatches;

	std::vector< CLevelMeshPatch* >						m_pSortedPatches;

	CMaterial*			m_pShadowMaterial;
	CMaterial*			m_pMiniMapMaterial;
	CMaterial*			m_pWaterMaterial;

	bool				m_UpdateVisibility;

	D3DMATERIAL8		m_DungeonD3DMaterial;

	D3DXMATRIX			m_ProjectorBias;	// projection bias for projected texture coordinates

	uint32				m_PatchesRendered;


};

#endif