/*
	shadowvolume.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SHADOWVOLUME_H
#define _SHADOWVOLUME_H

#include "facelist.h"
const uint32	KMaxShadowVertices	=	32000;

/// The shadow volume, given a vertex and index buffer, maintains a vertex list
/// which renders to the stencil buffer.  The shadow volume must be updated each time
/// the casting mesh or light direction changes, by calling BuildFromMesh
class CShadowVolume
{

public:
					CShadowVolume( LPDIRECT3DDEVICE8 pD3DDevice );

					~CShadowVolume( void );


	HRESULT			BuildFromMesh( LPDIRECT3DVERTEXBUFFER8 pVB,	// vertex buffer of mesh to be constructed from
								   PDIRECT3DINDEXBUFFER8 pIB,	// index buffer of mesh to be constructed from
								   uint32 Faces,				// number of faces in index buffer
								   D3DXVECTOR3 LightDirection );// Direction of light to be cast from

	HRESULT			BuildFromFaceList( const CFaceList& pFaceList,	// facelist containing the mesh to be cast
									   D3DXVECTOR3 LightDirection );// Direction of light to be cast from

	HRESULT			Render( LPDIRECT3DDEVICE8 pD3DDevice );		// the direct3d device


	void				InvalidateDeviceObjects( void );
	void				RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

private:
	LPDIRECT3DVERTEXBUFFER8			m_pVB;		// dynamic buffer containing currently 
												// extruded verts of the shadow volume

	bool			m_Valid;					// shadow has been provided with a vertex buffer of facelist for extrusion

	uint32			m_Vertices;					// number of vertices to render from the buffer
	uint32			m_CurrentVertex;			// current index within the dynamic buffer

	void			AddEdge( WORD* pEdges,		// list of edges to be inserted into
							 DWORD& NumEdges,	// number of edges in the list
							 WORD Vertex0,		// first vertex of edge
							 WORD Vertex1 );	// second vertex of edge
};


#endif