/*
	animatedsubmesh.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _ANIMATEDSUBMESH_H
#define _ANIMATEDSUBMESH_H

#include <assert.h>

class	CAnimatedMeshSegment;
class	CRefManager;
class	CMaterial;
class	CActiveAnimation;
class	CShadowVolume;
class	CSettings;

/// sub-meshes of the parent animated mesh, are
/// basically a collection of submesh segments.
/// the submesh contains the vertex buffer and index buffer, and the
/// segments reference the vert and index buffer 
class CAnimatedSubMesh
{
public:

				CAnimatedSubMesh();
				~CAnimatedSubMesh();

	void		CreateVB( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  uint32 VerticesPerFrame,					// how many vertices there are in the base model
						  uint32 Faces,								// faces in the mesh
						  std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
						  std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
						  std::vector <D3DXVECTOR3>& Normals );		// vector of normals



	uint32		AddAnimatedMeshSegment( CRefManager& pRefManager,				// reference to refmanager
										CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
										LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
										std::vector <D3DXVECTOR4>& Triangles,	// vector of triangle indices
										uint32 MaterialID,						// material id
										int32 MaterialType, 					// material type
										bool DoubleSided );						// double sided material

	void		SetMaterial( CMaterial* pMaterial,				// material to apply
							 uint32 SubSegment );				// subsegment to apply to

	void	EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix );	// camera space matrix

	void SetMaxBounds( const D3DXVECTOR3& Bounds )	{	m_MaxBounds = Bounds;	};
	void SetMinBounds( const D3DXVECTOR3& Bounds )	{	m_MinBounds = Bounds;	};

	const D3DXVECTOR3& MaxBounds( void )		{	return m_MaxBounds;		};

	const D3DXVECTOR3& MinBounds( void )		{	return m_MinBounds;		};

	uint32 TotalVertices( void )				{	return m_TotalVertices;	};
	uint32 TotalFaces( void )					{	return m_TotalFaces;	};

	uint32							AnimatedMeshSegments( void )	{	return m_pAnimatedMeshSegments.size();		};

	CAnimatedMeshSegment&			GetAnimatedMeshSegment( uint32 Index )
	{
		assert( Index <= m_pAnimatedMeshSegments.size() );
		return *m_pAnimatedMeshSegments[Index];
	};

	LPDIRECT3DVERTEXBUFFER8 GetVB( void )			{	return m_pVB;				};
	LPDIRECT3DINDEXBUFFER8	GetIB( void )			{	return m_pIB;				};

	void			InvalidateDeviceObjects( void );
	void			RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void							UpdateVB( uint32 CurrentVertexOffset,
											  uint32 NextVertexOffset,
											  float32 Interpolation );

	void							UpdateVB( const std::deque < CActiveAnimation* >& ActiveAnimations ); // animations to blend

	void							RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	std::string						Name( void )							{ return m_Name; };
	void							SetName( const std::string& Name )		{ m_Name = Name; };

	void							CleanIB( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	void							SetVisible( uint32 SubSegment,			// subsegment to apply to
												bool State );				// state to set
private:
	uint32							m_TotalFaces;	// total faces in this mesh
	uint32							m_TotalVertices;	// total vertices in this mesh
	uint32							m_CurrentIndex;

	uint32							m_CurrentFrame;
	uint32							m_NextFrame;
	float32							m_Interpolation;

	std::string						m_Name;
	uint32							m_FrameVertices;

	LPDIRECT3DVERTEXBUFFER8			m_pVB;
	LPDIRECT3DINDEXBUFFER8			m_pIB;			// index buffer for this sub-mesh

	CShadowVolume*					m_pShadowVolume;

	D3DXVECTOR3						m_MaxBounds;
	D3DXVECTOR3						m_MinBounds;

	bool							m_ShadowsEnabled;	// casts shadows?

	std::vector <D3DXVECTOR3>		m_pVertices;	
	std::vector <D3DXVECTOR3>		m_pNormals;	

	std::vector <CAnimatedMeshSegment*>	m_pAnimatedMeshSegments;


};

#endif 