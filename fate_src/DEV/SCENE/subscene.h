/*
	submodel.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SUBSCENE_H
#define _SUBSCENE_H

#include <assert.h>

class	CSubSceneSegment;
class	CFaceList;
class	CRefManager;
class	CMaterial;

/// submodels are sub-meshes of the parent mesh, and are
/// basically a collection of submodel segments.
/// the submodel contains the vertex buffer, and the
/// segments reference the vert buffer with index buffers

class CSubScene
{
public:

				CSubScene();
				~CSubScene();

	/// reset morphing
	void		ResetMorph( void );

	void		CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  uint32 Faces,								// faces in the mesh
						  std::vector <uint32>& VertexReferences,	// vector of vertex references
						  std::vector <uint32>& UVReferences,		// vector of uv references
						  std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
						  std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
						  std::vector <D3DCOLOR>& VertexColors );   // vector of vertex colors

	void		CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  uint32 Faces,								// faces in the mesh
						  std::vector <uint32>& VertexReferences,	// vector of vertex references
						  std::vector <uint32>& UVReferences,		// vector of uv references
						  std::vector <uint32>& ShadowUVReferences,	// vector of uv references for shadow mapping layer
						  std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
						  std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
						  std::vector <D3DCOLOR>& VertexColors );   // vector of vertex colors

	void		CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  uint32 Faces,								// faces in the mesh
						  std::vector <uint32>& NormalReferences,	// vector of normal references
						  std::vector <D3DXVECTOR3>& Normals,		// vector of normals
						  std::vector <uint32>& VertexReferences,	// vector of vertex references
						  std::vector <uint32>& UVReferences,		// vector of uv references
						  std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
						  std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
						  std::vector <D3DCOLOR>& VertexColors );   // vector of vertex colors

	void		CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						  uint32 Faces,								// faces in the mesh
						  std::vector <uint32>& NormalReferences,	// vector of normal references
						  std::vector <D3DXVECTOR3>& Normals,		// vector of normals
						  std::vector <uint32>& VertexReferences,	// vector of vertex references
						  std::vector <uint32>& UVReferences,		// vector of uv references
						  std::vector <uint32>& ShadowUVReferences,	// vector of uv references for shadow mapping layer
						  std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
						  std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
						  std::vector <D3DCOLOR>& VertexColors );   // vector of vertex colors

	uint32		AddSubSceneSegment( CRefManager& pRefManager,				// reference to refmanager
									CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
								    LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								    std::vector <uint32>& VertexReferences,	// vector of vertex references
								    std::vector <D3DXVECTOR4>& Triangles,	// vector of triangle indices
								    uint32 MaterialID,						// material id
								    int32 MaterialType,						// material type
								    bool MaterialDoubleSided );				// is double sided?

	void		SetMaterial( CMaterial* pMaterial,				// material to apply
							 uint32 SubSegment );				// subsegment to apply to

	void		SetShadowMaterial( CMaterial* pMaterial,				// material to apply
								   uint32 SubSegment );				// subsegment to apply to

	void		SetVisible( uint32 SubSegment,			// subsegment to apply to
						    bool State );				// state to set

	void SetMaxBounds( D3DXVECTOR3 Bounds )	{	m_MaxBounds = Bounds;	};
	void SetMinBounds( D3DXVECTOR3 Bounds )	{	m_MinBounds = Bounds;	};

	const D3DXVECTOR3& MaxBounds( void )	{	return m_MaxBounds;		};
	const D3DXVECTOR3& MinBounds( void )	{	return m_MinBounds;		};

	uint32 TotalVertices( void )			{	return m_TotalVertices;	};
	uint32 TotalFaces( void )				{	return m_TotalFaces;	};

	uint32 SubSceneSegments( void )			{	return m_pSubSceneSegments.size();	};

	CSubSceneSegment& GetSubSceneSegment( uint32 Index )
	{
		assert ( Index <= m_pSubSceneSegments.size() );
		return *m_pSubSceneSegments[Index];
	};

	LPDIRECT3DVERTEXBUFFER8 GetVB( void )			{	return m_pVB;				};
	LPDIRECT3DINDEXBUFFER8	GetIB( void )			{	return m_pIB;				};

	void					CleanIB( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	DWORD				VertexSize( void )				{	return m_VertexSize;	};

	float32				ZDistance( void )				{	return m_ZDistance;		};
	void				SetZDistance( float32 Value )	{	m_ZDistance = Value;	};

	DWORD				VertexShader( void )			{	return m_VertexShader;	};

	const D3DXVECTOR3&	GetVertex( uint32 Index)		{	return m_pVertices[Index];	};

	void			CalculateTangentNormals( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
											 const D3DXVECTOR3& LightDirection, // light direction
											 uint32 StartingVertex,				// vertex to begin the calculation at
											 uint32 EndingVertex );				// vertex to end the calculation at
	void			GenerateTangentBinormal( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device

	bool			HasNormalMapping( void );

private:
	DWORD							m_VertexSize;		// size of vertex type

	DWORD							m_VertexShader;

	uint32							m_TotalVertices;	
	uint32							m_TotalFaces;
	uint32							m_CurrentIndex;		//  current index into the index buffer

	LPDIRECT3DVERTEXBUFFER8			m_pVB;				// vb containing verts for all subscenesegments
	LPDIRECT3DINDEXBUFFER8			m_pIB;				// index buffer for this sub-mesh

	D3DXVECTOR3						m_MaxBounds;		// bounds of the subscene
	D3DXVECTOR3						m_MinBounds;

	std::vector <D3DXVECTOR3>		m_pVertices;
	std::vector <D3DXVECTOR3>		m_Binormals;
	std::vector <D3DXVECTOR3>		m_Tangents;
	std::vector <D3DXVECTOR3>		m_Normals;


	std::vector <CSubSceneSegment*>	m_pSubSceneSegments;		// list of segments with IBs that reference this VB


	float32					m_ZDistance;						// distance from near frustum plane

};

#endif 