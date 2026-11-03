/*
	skinnedgeometry.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#ifndef _SKINNEDGEOMETRY_H
#define _SKINNEDGEOMETRY_H


#include <d3d8.h>
#include <d3dx8.h>
#include "../MEDIA/RefManager.h"
#include "../MEDIA/materialconstants.h"
#include "../ANIMATEDMESH/tag.h"
#include "activeskinnedanimation.h"
#include "skeleton.h"

#include <string>

class CSkinnedMesh;
class CSkinnedSubMesh;
class CSkinnedMeshSegment;

// animated geometry objects maintain all the underlying frame, material, and geometry
// data for an animated mesh. Animated meshes maintain the individual state of instances
// of animated geometry.  For instance, you might have three of the same type of character loaded.
// They all use the same SkinnedGeometry, but they all may have a different state of current animation
// - thus they all have their own SkinnedMesh
class CSkinnedGeometry
{


public:

			CSkinnedGeometry( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
							   CSettings& pSettings );		// pointer to the settings manager, for keeping track of card/machine capabilities

			~CSkinnedGeometry();

	bool	Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
				  std::string	FileName,			// path to the mesh file
				  std::string TexturePath );		// path to the textures (if "" is supplied, will use the mesh path)

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
					const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
					const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
					CPose* pBlendedPose,					// final blended pose
					const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
					bool UpdateVB = kTrue );							// update the vertex buffer?

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
					const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
					const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
					CPose* pBlendedPose,					// final blended pose
					LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
					const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
					bool UpdateVB = kTrue );							// update the vertex buffer?

	void	RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
							const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations, // animations to blend
							CPose* pBlendedPose,					// final blended pose
							LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
							int32 UserDataValue,					// userdata value that each segment must have set to be rendered
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
							bool UpdateVB = kTrue );							// update the vertex buffer?

	void	RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations,	 // animations to blend
							CPose* pBlendedPose,						// final blended pose
							bool UpdateVB = kTrue );						// update the vertex buffer?

	void	RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							   const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations,	 // animations to blend
							   CPose* pBlendedPose,							// final blended pose
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
							   bool UpdateVB = kTrue );						// update the vertex buffer?

	void	RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							 const std::deque < CActiveSkinnedAnimation* >& ActiveAnimations,	 // animations to blend
							 CPose* pBlendedPose,					// final blended pose
							 bool UpdateVB = kTrue );				// update the vertex buffer?


	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from

	void					InvalidateDeviceObjects( void );
	void					RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	float32		Scale( void )					{	return m_Scale;					};

	CSkeleton*	Skeleton( void )				{	return m_pSkeleton;				};

	/// check for the existence of the named material on this mesh
	bool		HasMaterial( const std::string& Name );	// name of material to check existence of

	uint32		GetMaterials( void )			{	return m_MaterialNames.size();	};

	CMaterial*	GetMaterial( uint32 Index );		// index of material to retrieve
	
	CMaterial*	GetMaterialByIndex( uint32 Index );	// index of material to retrieve

	int32		GetMaterialIDByIndex( uint32 Index );

	void		SetMaterial( uint32 Index,				// index of material to overwrite
							 CMaterial* pMaterial );	// material to replace it with

	void		SetMaterialByIndex( uint32 Index,			// index of material to overwrite
									CMaterial* pMaterial );	// material to replace it with

	std::string	GetMaterialName( uint32 Index )	{	return m_MaterialNames[Index];	};


	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	const std::string&	GetTagUserData( uint32 Index );				// index of tag name to retrieve

	uint32				GetTagCount( void )			{	return m_TagNames.size();	};

	int32				TagIndex( const std::string& Name );	// name to search for in the tag list


	const D3DXVECTOR3&		GetMinBounds( void )	{	return m_LocalMinBounds;	};
	const D3DXVECTOR3&		GetMaxBounds( void )	{	return m_LocalMaxBounds;	};

	bool					HasReflection( void );

	bool	CastsShadows( void )				{	return m_CastsShadows;		};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void	EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device
	void	DisableShadows( void )				{	m_CastsShadows = kFalse;	};

	void	SetUserData( uint32 Id,			// id of material to set user data on
						 int32 Value );		// value to set for user data

	/// set a user data value on a specific material name
	void	SetUserData( std::string Name,		// name of material to set user data on
						 int32 Value );			// value to set for user data


private:

	float32											m_Scale;

	bool											m_CastsShadows;

	D3DXVECTOR3										m_LocalMaxBounds;
	D3DXVECTOR3										m_LocalMinBounds;

	std::vector <int32>								m_UserData;

	CRefManager&									m_pRefManager;
	CSettings&										m_pSettings;

	CSkeleton*										m_pSkeleton;

	std::vector <std::string>						m_MaterialNames;
	
	CSkinnedSubMesh*								m_pSubModel;	
	std::vector < std::string >						m_TagNames;
	std::vector < std::string >						m_TagUserData;
	// this 2-level vector has the following structure
	// --material type (standard, alpha, etc.
	//     --actual material ( by texture )
	std::vector <std::vector <CSkinnedMeshSegment*> > 	m_pMaterialSegments;	


	uint32			m_TotalMaterials;


	void	RenderSegmentSetUserData( LPDIRECT3DDEVICE8 pD3DDevice,								// direct3d device
									  std::vector <CSkinnedMeshSegment*> pMaterialSegments,	// vector of segments to render
									  int32 UserDataValue );									// userdata id to render

	void	RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,								// direct3d device
										  std::vector <CSkinnedMeshSegment*> pMaterialSegments,	// vector of segments to render
										  int32 UserDataValue,										// userdata id to render
										  EMaterialType MaterialType );								// material type we will render

};	// CSkinnedGeometry class


#endif
