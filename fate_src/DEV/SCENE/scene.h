/*
	scene.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SCENE_H
#define _SCENE_H

#include "../MEDIA/materialconstants.h"
#include "sceneoctree.h"
#include "scenequadtree.h"
#include "../UTILITIES/math3d.h"
#include "../OBJECTS/datapoint.h"
#include "../OBJECTS/datalight.h"
#include "../OBJECTS/path.h"

// forward declarations
class	CMaterial;
class	CRefManager;
class	CFaceList;
class	CSettings;
class	CSubScene;
class	CSubSceneSegment;


/// basic model - comprised of submodels, which contain vertex buffers, and submodelsegments,
/// which have only one material, and contain the index buffers that reference the parent
/// submodel's vertex buffers. Clear? No? Oh well...
class CScene
{
public:

			CScene( CRefManager& pRefManager,		// pointer to the reference manager, for tracking submaterials
				    CSettings& pSettings );			// pointer to the settings manager, for keeping track of card/machine capabilities
			~CScene( void );

	void	Load( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device - needed to create vert and index buffers
				  std::string FileName,					// path to the mesh file
				  std::string TexturePath,				// path to the textures (if "" is supplied, will use the mesh path)
				  bool AllowCollision,					// is this mesh collideable?
				  bool AllowRealtimeReflection = kTrue,	// allow automatic real-time reflection? ( mirrors )
				  ESortType SortType = KSortQuadtree );	// type of sorting to use (quadtree is default)

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
					const D3DXMATRIX& ViewportMatrix,				// viewport matrix
					LPDIRECT3DTEXTURE8 pReflectionMap1 = NULL );	// reflection map to use - NULL if none

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					const D3DXMATRIX& ViewportMatrix,			// viewport matrix
					D3DXMATRIX* pReflectionMatrix,				// reflection matrix
					LPDIRECT3DTEXTURE8 pReflectionMap1 );		// reflection map to use - NULL if none

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					const D3DXMATRIX& ViewportMatrix,			// viewport matrix
					D3DXMATRIX* pReflectionMatrix,				// reflection matrix - NULL if none
					LPDIRECT3DTEXTURE8 pReflectionMap1,		// reflection map to use - NULL if none
					std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments ); // vector containing all segments to be rendered

	void	RenderSorted( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
						  const D3DXMATRIX& ViewportMatrix,				// viewport matrix
						  LPDIRECT3DTEXTURE8 pReflectionMap1 = NULL );	// reflection map to use - NULL if none

	void	RenderSorted( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						  D3DXMATRIX* pReflectionMatrix,		// reflection matrix - NULL if none
						  LPDIRECT3DTEXTURE8 pReflectionMap1 );// reflection map to use - NULL if none

	void	RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const D3DXMATRIX& ViewportMatrix );	// viewport transformation matrix

	void	RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
							const D3DXMATRIX& ViewportMatrix,				// viewport matrix
							bool RenderGlows = kTrue );						// render glows, or just zbuffer?

	void	RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments,
							bool RenderGlows = kTrue );						// render glows, or just zbuffer?

	void	RenderGlowPassSorted( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
								  const D3DXMATRIX& ViewportMatrix,				// viewport matrix
								  bool RenderGlows = kTrue );					// render glows, or just zbuffer?

	void	RenderHeadlightPass( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
								 const D3DXMATRIX& ViewportMatrix,				// viewport matrix
							     const D3DXMATRIX& LightMatrix );				// matrix for the headlight direction

	void	RenderHeadlightPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							     const D3DXMATRIX& LightMatrix,					// matrix for the headlight direction
								 std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments ); // vector containing all segments to be rendered

	void	RenderHeadlightPassSorted( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									   const D3DXMATRIX& ViewportMatrix,	// viewport matrix
									   const D3DXMATRIX& LightMatrix );		// matrix for the headlight direction

	void	RenderShadowPass( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
								 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							     const D3DXMATRIX& LightMatrix,				// matrix for the Shadow direction
								 D3DXMATRIX* LightProjectionMatrix = NULL,	// null by default
								 bool Orthographic = kFalse );					// is this orthographic? false by default

	void	RenderShadowPass( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
								 const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							     const D3DXMATRIX& LightMatrix,				// matrix for the Shadow direction
								 std::vector <std::vector <std::vector <CSubSceneSegment*> > >&	pMaterialSegments,  // vector containing all segments to be rendered
								 D3DXMATRIX* LightProjectionMatrix = NULL,	// null by default
								 bool Orthographic = kFalse );				// is this orthographic? false by default

	void	RenderShadowPassSorted( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									const D3DXMATRIX& ViewportMatrix,	// viewport matrix
									const D3DXMATRIX& LightMatrix,		// matrix for the Shadow direction
									D3DXMATRIX* LightProjectionMatrix = NULL,	// null by default
									bool Orthographic = kFalse );				// is this orthographic? false by default


	void	SortForCollision( const D3DXVECTOR3& MinBounds,		// min bounds of area to sort
							  const D3DXVECTOR3& MaxBounds );	// max bounds of area to sort

	void	Sort( CFrustum& pFrustum);					// frustum to cull to

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

	bool	HasReflection( void );

	uint32	ReflectionsVisible( void );

	CFaceList*	FaceList( void )					{	return m_pFaceList;		};

	CMaterial*	GetReflectionMaterial( uint32 Index );	// index in the sorted set

	DWORD		GetFogColor( void )					{	return m_FogColor;		};
	DWORD		GetAmbientColor( void )				{	return m_AmbientColor;	};

	float32		GetAmbientR( void )					{	return m_AmbientR;	};
	float32		GetAmbientG( void )					{	return m_AmbientG;	};
	float32		GetAmbientB( void )					{	return m_AmbientB;	};

	uint32		GetPathCount( void )					{	return m_Paths.size();	};
	uint32		GetPointCount( void )					{	return m_Points.size();	};
	uint32		GetLightCount( void )					{	return m_Lights.size();	};

	CPath&		GetPath( uint32 Index );	// index of path to retrieve
	CDataPoint&	GetPoint( uint32 Index );	// index of point to retrieve
	CDataLight&	GetLight( uint32 Index );	// index of light to retrieve

	const D3DXVECTOR3&		GetMinBounds( void )	{	return m_LocalMinBounds;	};
	const D3DXVECTOR3&		GetMaxBounds( void )	{	return m_LocalMaxBounds;	};

	CMaterial*	GetMaterial( uint32 Index );	// index of material pointer to retrieve

	uint32		Materials( void )					{	return m_MaterialNames.size();	};

	const std::string& MaterialName( uint32 ID )	{	return m_MaterialNames[ID];		};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void		SetRenderAlpha( bool State )		{	m_RenderAlpha = State;			};
	void		SetRenderOpaque( bool State )		{	m_RenderOpaque = State;			};

	void		CalculateTangentNormals( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
										 const D3DXVECTOR3& LightDirection );	// light direction

	void	FindVisibleReflections( CFrustum& pFrustum ); // frustum to use to determine whether
	// the reflections are facing the camera

private:
	std::vector< CDataPoint* >						m_Points;		// list of points loaded from the file
	std::vector< CPath* >							m_Paths;		// list of paths(splines) loaded from the file
	std::vector< CDataLight* >						m_Lights;		// list of lights loaded from the file
	
	std::vector< CMaterial* >						m_pMaterialReferences;	// scene level references to materials

	CSceneOctree*									m_pOctree;		// octree for use in visibility sorting
	CSceneQuadtree*									m_pQuadtree;	// quadtree for use in visibility sorting
	COctree< uint32 >*								m_pCollisionOctree;		// octree for use in collision sorting
	CQuadtree< uint32 >*							m_pCollisionQuadtree;	// quadtree for use in collision sorting

	CRefManager&									m_pRefManager;	// reference to a refmanager for managing materials
	CSettings&										m_pSettings;	// reference to a settings file to tell us what we can do

	D3DXMATRIX										m_ProjectorBias;	// projection bias for projected texture coordinates


	D3DXVECTOR3										m_LocalMaxBounds;	// min bounds of the entire mesh
	D3DXVECTOR3										m_LocalMinBounds;	// max bounds of the entire mesh

	DWORD											m_CurrentVertexShader;	// active vertex shader

	DWORD											m_FogColor;			// fog color loaded from file
	DWORD											m_AmbientColor;		// ambient scene color loaded from file

	float32											m_AmbientR;
	float32											m_AmbientG;
	float32											m_AmbientB;

	std::vector <std::string>						m_MaterialNames;

	uint32											m_TotalMaterials;	// total materials referenced by the mes
	uint32											m_TotalFaces;
	uint32											m_TotalSubMeshes;

	bool											m_HasCollision;		// collision enabled on the scene?
	bool											m_HasAlpha;			// alpha materials exist in the scene?
	bool											m_IsTextured;		// has any textures at all?
	bool											m_HasReflection;	// has reflective textures

	bool											m_RenderOpaque;
	bool											m_RenderAlpha;

	std::vector <CSubScene*>						m_ModelList;		// list of submodels in the scene
	CFaceList*										m_pFaceList;		// facelist object containing all faces and verts

	std::vector < uint32 >							m_SortedFaces;		// a list of indices to all faces that 
																		// are currently sorted for collision

	// this 3-level vector has the following structure
	// --material type (standard, alpha, etc.
	//     --actual material ( by texture )
	//           -- submeshsegments of the above material
	std::vector <std::vector <std::vector <CSubSceneSegment*> > >	m_pMaterialSegments;	

	std::vector <std::vector <std::vector <CSubSceneSegment*> > >	m_pSortedMaterialSegments;	

	std::vector <CMaterial*>										m_pSortedReflectiveMaterials;	

	void		RenderSegmentSet( LPDIRECT3DDEVICE8 pD3DDevice,											// direct3d device
								   std::vector <std::vector <CSubSceneSegment*> > pMaterialSegments );	// vector of segments to render

	void		RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,											// direct3d device
											  std::vector <std::vector <CSubSceneSegment*> > pMaterialSegments,		// vector of segments to render
											  EMaterialType MaterialType );											// type of material to render
};

#endif