/*
	animatedgeometry.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	4/14/2004	-	big changes to vert/index buffer arrangement! consolidated them
					for the entire model, and subsegments now only have an index
					to their position in the parent buffer. Minimizes vert buffer changes

*/

#ifndef _ANIMATEDGEOMETRY_H
#define _ANIMATEDGEOMETRY_H


#include <d3d8.h>
#include <d3dx8.h>
#include "../MEDIA/RefManager.h"
#include "../MEDIA/materialconstants.h"
#include "tag.h"
#include "activeanimation.h"
#include <string>

class CAnimatedMesh;
class CAnimatedSubMesh;
class CAnimatedMeshSegment;


// animated geometry objects maintain all the underlying frame, material, and geometry
// data for an animated mesh. Animated meshes maintain the individual state of instances
// of animated geometry.  For instance, you might have three of the same type of character loaded.
// They all use the same AnimatedGeometry, but they all may have a different state of current animation
// - thus they all have their own AnimatedMesh
class CAnimatedGeometry
{


public:

			CAnimatedGeometry( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
							   CSettings& pSettings );		// pointer to the settings manager, for keeping track of card/machine capabilities

			~CAnimatedGeometry();

	bool	Load( LPDIRECT3DDEVICE8 pD3DDevice,	// the direct3d device - needed to create vert and index buffers
				  std::string	FileName,			// path to the mesh file
				  std::string TexturePath );		// path to the textures (if "" is supplied, will use the mesh path)

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
					const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
					const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
					const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),		// direction of light for use in normal mapping
					bool UpdateVB = kTrue );								// update the vertex buffer?

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
					const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
					const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
					LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
					const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
					bool UpdateVB = kTrue );					// update the vertex buffer?

	void	RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
							const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
							LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
							int32 UserDataValue,					// userdata value that each segment must have set to be rendered
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
							bool UpdateVB = kTrue );					// update the vertex buffer?

	void	RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							const D3DXMATRIX& ViewportMatrix,			// viewport matrix
							const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
							bool UpdateVB = kTrue );					// update the vertex buffer?

	void	RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								const D3DXMATRIX& ViewportMatrix,			// viewport matrix
								const std::deque < CActiveAnimation* >& ActiveAnimations, // animations to blend
								const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
								bool UpdateVB = kTrue );					// update the vertex buffer?

	void	RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							 const std::deque < CActiveAnimation* >& ActiveAnimations , // animations to blend
							 bool UpdateVB = kTrue );					// update the vertex buffer?

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from

	void					InvalidateDeviceObjects( void );
	void					RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	/// check for the existence of the named material on this mesh
	bool		HasMaterial( const std::string& Name );	// name of material to check existence of

	uint32		GetMaterials( void )			{	return m_MaterialNames.size();	};

	CMaterial*	GetMaterial( uint32 MaterialID );	// index of material to retrieve
	
	CMaterial*	GetMaterialByIndex( uint32 Index );	// index of material to retrieve

	int32		GetMaterialIDByIndex( uint32 Index );

	void		SetMaterial( uint32 MaterialID,			// id of material to overwrite
							 CMaterial* pMaterial );	// material to replace it with

	void		SetMaterialByIndex( uint32 Index,			// index of material to overwrite
									CMaterial* pMaterial );	// material to replace it with

	std::string	GetMaterialName( uint32 Index )	{	return m_MaterialNames[Index];	};

	D3DXMATRIX	GetTransformedTag( uint32 Index,				// index of tag to get transformed info for
								   uint32 CurrentFrame,			// current frame
								   uint32 NextFrame,			// next frame
								   float32 Interpolation );		// interpolation between the frames

	D3DXMATRIX	GetTransformedTag( uint32 Index,				// index of tag to get transformed info for
								   const std::deque < CActiveAnimation* >& ActiveAnimations ); // animations to blend

	D3DXVECTOR3	GetTagPosition( uint32 Index,				// index of tag to get transformed info for
								uint32 CurrentFrame,		// current frame
								uint32 NextFrame,			// next frame
								float32 Interpolation );	// interpolation between the frames

	D3DXVECTOR3	GetTagPosition( uint32 Index,				// index of tag to get transformed info for
								const std::deque < CActiveAnimation* >& ActiveAnimations ); // animations to blend

	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	const std::string&	GetTagUserData( uint32 Index );				// index of tag name to retrieve

	float32	CompressionFactor( void )			{	return m_CompressionFactor;	};

	uint32	GetTagCount( void )					{	return m_TagList.size();	};

	uint32	AnimationFrames( void )				{	return m_AnimationFrames;	};

	const D3DXVECTOR3&		GetMinBounds( void )	{	return m_LocalMinBounds;	};
	const D3DXVECTOR3&		GetMaxBounds( void )	{	return m_LocalMaxBounds;	};

	bool					HasReflection( void );


	bool	CastsShadows( void )				{	return m_CastsShadows;		};

	bool	HasFrameMap( void )					{	return m_FrameMap.size() > 0;	};

	int32	NearestMappedFrame( int32 UnmappedIndex );	// unmapped frame to find frame mapping for

	uint32	MappedFrame( uint32 Index )			{	return m_FrameMap[Index];	};

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

	bool											m_CastsShadows;

	D3DXVECTOR3										m_LocalMaxBounds;
	D3DXVECTOR3										m_LocalMinBounds;

	std::vector <int32>								m_UserData;

	CRefManager&									m_pRefManager;
	CSettings&										m_pSettings;

	std::vector <std::string>						m_MaterialNames;
	
	CAnimatedSubMesh*								m_pSubModel;	
	std::vector < CTag* >							m_TagList;
	// this 2-level vector has the following structure
	// --material type (standard, alpha, etc.
	//     --actual material ( by texture )
	std::vector <std::vector <CAnimatedMeshSegment*> > 	m_pMaterialSegments;	


	float32			m_CompressionFactor;

	uint32			m_TotalMaterials;

	uint32			m_AnimationFrames;

	std::vector< int32 >	m_FrameMap;	// keeps track of the original frame indices of compressed frames

	void	RenderSegmentSetUserData( LPDIRECT3DDEVICE8 pD3DDevice,								// direct3d device
									  std::vector <CAnimatedMeshSegment*> pMaterialSegments,	// vector of segments to render
									  int32 UserDataValue );									// userdata id to render

	void	RenderSegmentSetIllumination( LPDIRECT3DDEVICE8 pD3DDevice,								// direct3d device
										  std::vector <CAnimatedMeshSegment*> pMaterialSegments,	// vector of segments to render
										  int32 UserDataValue,										// userdata id to render
										  EMaterialType MaterialType );								// material type we will render

};	// CAnimatedGeometry class


#endif
