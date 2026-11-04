/*
	submodel.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SUBMODEL_H
#define _SUBMODEL_H

#include "d3dx8_shim.h"

#include <assert.h>

class	CSubModelSegment;
class	CFaceList;
class	CRefManager;
class	CMaterial;
class	CShadowVolume;
class	CSettings;

/// submodels are sub-meshes of the parent mesh, and are
/// basically a collection of submodel segments.
/// the submodel contains the vertex and index buffers, and the
/// segments reference specific subsets of these buffers

class CSubModel
{
public:

				CSubModel();
				~CSubModel();

	/// reset morphing
	void		ResetMorph( void );

	/// enable morphing
	void		SetMorphable( void );

	void		Morph( CSubModel* MorphTarget,		// target mesh to morph to
					   D3DXVECTOR3 MorphPointVEC3D,	// point of 'impact' for the mortph
					   float32 Radius,				// radius of impact for the morph
					   float32 Percent );			// percent to morph from start to target 

	void		CreateBuffers(  LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								uint32 Faces,								// faces in the mesh
								std::vector <uint32>& VertexReferences,	// vector of vertex references
								std::vector <uint32>& UVReferences,		// vector of uv references
								std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
								std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
								std::vector <uint32>& NormalReferences,		// indices of normals
								std::vector <D3DXVECTOR3>& Normals,		// vector of normals
								std::vector <D3DXVECTOR3>& VertexColors );// vector of vertex colors


	uint32		AddSubModelSegment( CRefManager& pRefManager,				// reference to refmanager
									CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
								    LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								    std::vector <uint32>& VertexReferences,	// vector of vertex references
								    std::vector <D3DXVECTOR4>& Triangles,	// vector of triangle indices
								    uint32 MaterialID,						// material id
								    int32 MaterialType,						// material type
								    bool MaterialDoubleSided );				// is double sided?

	void		SetMaterial( CMaterial* pMaterial,				// material to apply
							 uint32 SubSegment );				// subsegment to apply to

	void	EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device


	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const CFaceList& pFaceList,			// face list to build shadow volume from
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from


	void SetMaxBounds( const D3DXVECTOR3& Bounds )	{	m_MaxBounds = Bounds;	};
	void SetMinBounds( const D3DXVECTOR3& Bounds )	{	m_MinBounds = Bounds;	};

	const D3DXVECTOR3& MaxBounds( void )		{	return m_MaxBounds;		};

	const D3DXVECTOR3& MinBounds( void )		{	return m_MinBounds;		};

	// vertices prior to optimization
	uint32 OriginalVertices( void )				{	return m_Vertices.size();	};

	uint32 TotalVertices( void )				{	return m_TotalVertices;	};
	uint32 TotalFaces( void )					{	return m_TotalFaces;	};
	
	uint32 SubModelSegments( void )				{	return m_pSubModelSegments.size();	};

	CSubModelSegment& GetSubModelSegment( uint32 Index )
	{
		assert ( Index <= m_pSubModelSegments.size() );
		return *m_pSubModelSegments[Index];
	};

	void		SetVisible( uint32 SubSegment,			// subsegment to apply to
						    bool State );				// state to set

	LPDIRECT3DVERTEXBUFFER8 GetVB( void )			{	return m_pVB;				};
	LPDIRECT3DINDEXBUFFER8	GetIB( void )			{	return m_pIB;				};

	void			InvalidateDeviceObjects( void );
	void			RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void			CleanIB( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

private:

	LPDIRECT3DVERTEXBUFFER8			m_pVB;
	LPDIRECT3DINDEXBUFFER8			m_pIB;			// index buffer for this sub-mesh

	CShadowVolume*					m_pShadowVolume;

	D3DXVECTOR3						m_MaxBounds;
	D3DXVECTOR3						m_MinBounds;

	bool							m_Morphable;
	bool							m_ShadowsEnabled;	// casts shadows?

	std::vector <uint32>			m_VertexMap;	
	std::vector <D3DXVECTOR3>		m_ExpandedVertices;	// vertices expanded out for inconsistencies in normal,
														// uv, etc. we keep them separate from the
														// original sets of vertices, which can
														// be shared regardless of normal, for use in morphing
	std::vector <D3DXVECTOR3>		m_Vertices;	
	std::vector <D3DXVECTOR3>		m_OriginalVertices;	


	std::vector <CSubModelSegment*>	m_pSubModelSegments;

	uint32							m_CurrentIndex;		// current index in the index buffer
	uint32							m_TotalFaces;
	uint32							m_TotalVertices;

	void							UpdateVB( void );

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix );		// camera space matrix

};

#endif 