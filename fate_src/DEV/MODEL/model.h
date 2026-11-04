/*
	model.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _MODEL_H
#define _MODEL_H

#include "d3dx8_shim.h"



#include "../MEDIA/materialconstants.h"

#include "../OBJECTS/datapoint.h"
#include "../OBJECTS/datalight.h"
#include "../OBJECTS/path.h"

// forward declarations
class	CMaterial;
class	CRefManager;
class	CFaceList;
class	CSettings;
class	CSubModel;
class	CShadowVolume;
class	CSubModelSegment;


/// basic model - comprised of submodels, which contain vertex buffers, and submodelsegments,
/// which have only one material, and contain the index buffers that reference the parent
/// submodel's vertex buffers. Clear? No? Oh well...
class CModel
{
public:

			CModel( CRefManager& pRefManager,		// pointer to the reference manager, for tracking submaterials
				    CSettings& pSettings );			// pointer to the settings manager, for keeping track of card/machine capabilities
			~CModel( void );

	void	Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
				  std::string FileName,			// path to the mesh file
				  std::string TexturePath,		// path to the textures (if "" is supplied, will use the mesh path)
				  bool LocalSpace,				// ignore worldspace positioning in the file?
				  bool LoadTextures,			// should the mesh load and apply textures
				  bool FlipNormals = kFalse );	// flip the normals at load time?

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
					const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ) );	// direction of light for use in normal mapping

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
					const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
					LPDIRECT3DCUBETEXTURE8 pCubeMap,			// cubemap to use - NULL if none
					const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ) );	// direction of light for use in normal mapping

	void	RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						    const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
						    LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
						    int32 UserDataValue,					// userdata value that each segment must have set to be rendered
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ) );	// direction of light for use in normal mapping

	void	RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							const D3DXMATRIX& ViewportMatrix );			// viewport matrix

	void	RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							   const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ) );	// direction of light for use in normal mapping

	void	RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							 const D3DXMATRIX& CameraSpaceMatrix );		// camera space matrix

	void	RenderUserDataNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									 int32 UserDataValue );					// userdata value that each segment must have set to be rendered

	/// render shadows
	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from

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


	void	Morph( CModel* target,		// target mesh to morph to
				   D3DXVECTOR3 vPoint,	// starting point of the morph
				   float32 radius,		// radius of morph influence
				   float32 percent );	// percentage to morph

	void				InvalidateDeviceObjects( void );
	void				RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	CFaceList*	FaceList( void )				{	return m_pCollisionList;		};

	/// check for the existence of the named material on this mesh
	bool		HasMaterial( const std::string& Name );	// name of material to check existence of

	uint32		GetMaterials( void )			{	return m_MaterialNames.size();	};

	CMaterial*	GetMaterial( uint32 MaterialIDx );	// material id of material to retrieve

	CMaterial*	GetMaterialByIndex( uint32 Index );	// index of material to retrieve

	int32		GetMaterialIDByIndex( uint32 Index );

	std::string	GetMaterialName( uint32 Index )	{	return m_MaterialNames[Index];	};

	bool		CastsShadows( void )			{	return m_CastsShadows;		};

	bool		IsTextured( void )				{	return m_IsTextured;		};
	bool		HasAlpha( void )				{ 	return m_HasAlpha;			};
	bool		HasReflection( void );

	uint32		GetPathCount( void )			{	return m_Paths.size();		};
	uint32		GetPointCount( void )			{	return m_Points.size();		};
	uint32		GetLightCount( void )			{	return m_Lights.size();		};

	CPath&		GetPath( uint32 Index );	// index of path to retrieve
	CDataPoint&	GetPoint( uint32 Index );	// index of point to retrieve
	CDataLight&	GetLight( uint32 Index );	// index of light to retrieve

	const D3DXVECTOR3&		GetMinBounds( void )	{	return m_LocalMinBounds;	};
	const D3DXVECTOR3&		GetMaxBounds( void )	{	return m_LocalMaxBounds;	};

	bool	GetBoundsOfMaterial( const std::string& Name,	// name of the material to check for
								 D3DXVECTOR3* Min,			// min bounds to fill
								 D3DXVECTOR3* Max );		// max bounds to fill

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void		SetMaterial( uint32 MaterialID,			// material ID to overwrite
							 CMaterial* pMaterial );	// material to replace it with

	void		SetMaterialByIndex( uint32 Index,			// index of material to overwrite
									CMaterial* pMaterial );	// material to replace it with

	/// enable stencil shadowcasting
	void	EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device
	void	DisableShadows( void )				{	m_CastsShadows = kFalse;	};

	void	ResetMorph( void );

	/// enable morphing on this mesh
	void	SetMorphable( void );

	/// set a user data value on a specific material id
	void	SetUserData( uint32 Id,				// id of material to set user data on
						 int32 Value );			// value to set for user data

	/// set a user data value on a specific material name
	void	SetUserData( std::string Name,		// name of material to set user data on
						 int32 Value );			// value to set for user data

private:

	std::vector< CDataPoint* >						m_Points;
	std::vector< CPath* >							m_Paths;
	std::vector< CDataLight* >						m_Lights;

	CRefManager&									m_pRefManager;
	CSettings&										m_pSettings;

	D3DXVECTOR3										m_LocalMaxBounds;
	D3DXVECTOR3										m_LocalMinBounds;

	D3DXVECTOR3										m_MaxBounds;
	D3DXVECTOR3										m_MinBounds;
	D3DXVECTOR3										m_Bounds[8];

	uint32											m_TotalMaterials;
	uint32											m_TotalFaces;
	uint32											m_TotalSubMeshes;

	bool											m_HasAlpha;
	bool											m_IsTextured;
	bool											m_HasReflection;
	bool											m_CastsShadows;

	std::vector <std::string>						m_MaterialNames;
	std::vector <int32>								m_UserData;

	CSubModel*										m_pSubModel;
	CFaceList*										m_pCollisionList;	

	CShadowVolume*			m_pShadowVolume;

	// this 2-level vector has the following structure
	// --material type (standard, alpha, etc.
	//     --actual material ( by texture )
	std::vector <std::vector <CSubModelSegment*> >	m_pMaterialSegments;	

	void											RenderSegmentSetUserData( LPDIRECT3DDEVICE8 pD3DDevice,								// direct3d device
																			  std::vector <CSubModelSegment*> pMaterialSegments,		// vector of segments to render
																			  int32 UserDataValue );									// userdata id to render

	void											RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,										// direct3d device
																				  std::vector <CSubModelSegment*> pMaterialSegments,	// vector of segments to render
																				  int32 UserDataValue,
																				  EMaterialType MaterialType );										// material type we will render

};
#endif
