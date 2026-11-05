/*
	shadowvolume.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History

	Travis Baldree
	4/14/2004	-	removed DrawPrimitiveUP calls and switched to a VB system for
					faster buffer access
*/

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>


#include "facelist.h"
#include "shadowvolume.h"

CShadowVolume::CShadowVolume( LPDIRECT3DDEVICE8 pD3DDevice ) :
							  m_Valid( kFalse ),
							  m_pVB( NULL ),
							  m_CurrentVertex( 0 ),
							  m_Vertices( 0 )
{
	RestoreDeviceObjects( pD3DDevice );
} // CShadowVolume::CShadowVolume()

CShadowVolume::~CShadowVolume( void )
{
	InvalidateDeviceObjects();
} // CShadowVolume::~CShadowVolume()

void CShadowVolume::InvalidateDeviceObjects( void )
{
	RELEASE_SAFELY( m_pVB );
	m_Valid = kFalse;
} // CShadowVolume::InvalidateDeviceObjects()

void CShadowVolume::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if( m_Valid )
	{
		return;
	}

	m_Valid = kTrue;

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( KMaxShadowVertices*sizeof( D3DXVECTOR3 ),
												  D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
												  D3DFVF_PLAINVERTEXMODEL,
												  D3DPOOL_DEFAULT,
												  &m_pVB );
	if( FAILED( hr ) )
	{
		m_Valid = kFalse;
	}
} // CShadowVolume::InvalidateDeviceObjects()

// Render the shadow volume

HRESULT CShadowVolume::Render( LPDIRECT3DDEVICE8 pD3DDevice )			// the direct3d device
{
	if( !m_Valid )
	{
		return S_OK;
	}
	pD3DDevice->SetVertexShader( D3DFVF_XYZ );
	pD3DDevice->SetStreamSource( 0,
								 m_pVB,
								 sizeof( D3DXVECTOR3 ) );

	return pD3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST,
									  m_CurrentVertex,
									  m_Vertices / 3 );
} // CShadowVolume::Render()

// Adds an edge to a list of silohuette edges of a shadow volume.

VOID CShadowVolume::AddEdge( WORD* pEdges,		// list of edges to be inserted into
							 DWORD& NumEdges,	// number of edges in the list
							 WORD Vertex0,		// first vertex of edge
							 WORD Vertex1 )		// second vertex of edge
{
	// Remove interior edges (which appear in the list twice)
	for ( DWORD i = 0; i < NumEdges; i ++ )
	{
		if ( ( pEdges[2 * i + 0] == Vertex0 && pEdges[2 * i + 1] == Vertex1 ) ||
			 ( pEdges[2 * i + 0] == Vertex1 && pEdges[2 * i + 1] == Vertex0 ) )
		{
			if ( NumEdges > 1 )
			{
				pEdges[2 * i + 0] = pEdges[2 * ( NumEdges - 1 ) + 0];
				pEdges[2 * i + 1] = pEdges[2 * ( NumEdges - 1 ) + 1];
			}
			NumEdges --;
			return;
		}
	}

	// insert the new edge
	pEdges[2 * NumEdges + 0] = Vertex0;
	pEdges[2 * NumEdges + 1] = Vertex1;
	// increment our edge count
	NumEdges++;
} // CShadowVolume::AddEdge()


//	Takes a mesh as input, and uses it to build a shadowvolume. The
//  technique used considers each triangle of the mesh, and adds it's
//  edges to a temporary list. The edge list is maintained, such that
//  only silohuette edges are kept. Finally, the silohuette edges are
//  extruded to make the shadow volume vertex list.

HRESULT CShadowVolume::BuildFromMesh( LPDIRECT3DVERTEXBUFFER8 pVB,	// vertex buffer of mesh to be constructed from
									  LPDIRECT3DINDEXBUFFER8 pIB,	// index buffer of mesh to be constructed from
									  uint32 Faces,					// number of faces in index buffer
									  D3DXVECTOR3 LightDirection )	// Direction of light to be cast from
{
	if( !m_Valid )
	{
		return E_FAIL;
	}


	uint32				NumFaces	( Faces );
	uint32				NumEdges	( 0 );
	PLAINVERTEXMODEL*	pVertices;
	WORD*				pIndices;



	// Allocate a temporary edge list
	WORD*	pEdges	= new WORD[NumFaces*6];

	if ( pEdges == NULL )
	{
		m_Valid = kFalse;
		return E_OUTOFMEMORY;
	}

	// Lock the geometry buffers
	if ( FAILED( pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, D3DLOCK_NOOVERWRITE  ) ) )
	{
		DELETE_ARRAY_SAFELY( pEdges );
		return S_OK;
	}

	pIB->Lock( 0, Faces * 3 * sizeof( WORD ), ( BYTE * * ) &pIndices, D3DLOCK_NOOVERWRITE  );

	// For each face
	for ( uint32 i = 0; i < NumFaces; i ++ )
	{
		WORD		Vertex0	= pIndices[3 * i + 0];
		WORD		Vertex1	= pIndices[3 * i + 1];
		WORD		Vertex2	= pIndices[3 * i + 2];

		D3DXVECTOR3	v0		= pVertices[Vertex0].v;
		D3DXVECTOR3	v1		= pVertices[Vertex1].v;
		D3DXVECTOR3	v2		= pVertices[Vertex2].v;

		// Transform vertices or transform light?
		D3DXVECTOR3	vCross1	( v2 - v1 );
		D3DXVECTOR3	vCross2	( v1 - v0 );
		D3DXVECTOR3	vNormal;
		D3DXVec3Cross( &vNormal, &vCross1, &vCross2 );

		if ( D3DXVec3Dot( &vNormal, &LightDirection ) >= 0.0f )
		{
			AddEdge( pEdges, NumEdges, Vertex0, Vertex1 );
			AddEdge( pEdges, NumEdges, Vertex1, Vertex2 );
			AddEdge( pEdges, NumEdges, Vertex2, Vertex0 );
		}
	}
	// Unlock the index buffer
	pIB->Unlock();
	
	if( NumEdges * 6 >= KMaxShadowVertices )
	{
		DELETE_ARRAY_SAFELY( pEdges );
		m_Valid = kFalse;
		pVB->Unlock();
		return E_FAIL;
	}
	D3DXVECTOR3*	pShadowVertices;
	// if it won't fit in our dynamic buffer, we want to discard and start at 0
	if( m_CurrentVertex + m_Vertices + NumEdges * 6 >= KMaxShadowVertices )
	{
		m_CurrentVertex = 0;

		if ( FAILED( m_pVB->Lock( m_CurrentVertex * sizeof( D3DXVECTOR3 ), 
								  NumEdges * 6 * sizeof( D3DXVECTOR3 ), 
								  ( BYTE * * ) &pShadowVertices, 
								  D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD ) ) )
		{
			DELETE_ARRAY_SAFELY( pEdges );
			m_Valid = kFalse;
			return E_FAIL;
		}
	}
	else	// otherwise, we just want to append
	{
		m_CurrentVertex += m_Vertices;
		if ( FAILED( m_pVB->Lock( m_CurrentVertex * sizeof( D3DXVECTOR3 ), 
								  NumEdges * 6 * sizeof( D3DXVECTOR3 ), 
								  ( BYTE * * ) &pShadowVertices, 
								  D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE ) ) )
		{
			DELETE_ARRAY_SAFELY( pEdges );
			m_Valid = kFalse;
			return E_FAIL;
		}
	}

	m_Vertices = 0;


	for( int32 i = 0; i < NumEdges; i ++ )
	{
		D3DXVECTOR3	v1	= pVertices[pEdges[2 * i + 0]].v;
		D3DXVECTOR3	v2	= pVertices[pEdges[2 * i + 1]].v;
		D3DXVECTOR3	v3	= v1 - LightDirection;
		D3DXVECTOR3	v4	= v2 - LightDirection;

		// Add a quad (two triangles) to the vertex list
		pShadowVertices[m_Vertices ++] = v1;
		pShadowVertices[m_Vertices ++] = v2;
		pShadowVertices[m_Vertices ++] = v3;

		pShadowVertices[m_Vertices ++] = v2;
		pShadowVertices[m_Vertices ++] = v4;
		pShadowVertices[m_Vertices ++] = v3;
	}

	// Delete the temporary edge list
	DELETE_ARRAY_SAFELY( pEdges );

	// Unlock the geometry buffers
	pVB->Unlock();
	m_pVB->Unlock();

	return S_OK;
} // CShadowVolume::BuildFromMesh()


//	Takes a facelist as input, and uses it to build a shadowvolume. The
//  technique used considers each triangle of the mesh, and adds it's
//  edges to a temporary list. The edge list is maintained, such that
//  only silohuette edges are kept. Finally, the silohuette edges are
//  extruded to make the shadow volume vertex list.

HRESULT CShadowVolume::BuildFromFaceList( const CFaceList& pFaceList,	// facelist containing the mesh to be cast
										  D3DXVECTOR3 LightDirection )	// Direction of light to be cast from
{
	if( !m_Valid )
	{
		return E_FAIL;
	}


	const std::vector <D3DXVECTOR3>&	pVertices = pFaceList.GetVertices();
	const std::vector <D3DXVECTOR3>&	pNormals = pFaceList.GetNormals();
	const std::vector <uint32>&			pIndicesA = pFaceList.GetIndicesA();
	const std::vector <uint32>&			pIndicesB = pFaceList.GetIndicesB();
	const std::vector <uint32>&			pIndicesC = pFaceList.GetIndicesC();

	uint32				NumFaces	( pIndicesA.size() );
	uint32				NumEdges	( 0 );

	// Allocate a temporary edge list
	WORD*	pEdges	= new WORD[NumFaces*6];

	if ( pEdges == NULL )
	{
		return E_OUTOFMEMORY;
	}


	// For each face
	for ( uint32 i = 0; i < NumFaces; i ++ )
	{
		WORD		Vertex0	= (WORD)pIndicesA[i];
		WORD		Vertex1	= (WORD)pIndicesB[i];
		WORD		Vertex2	= (WORD)pIndicesC[i];



		if ( D3DXVec3Dot( &pNormals[i], &LightDirection ) >= 0.0f )
		{
			AddEdge( pEdges, NumEdges, Vertex0, Vertex1 );
			AddEdge( pEdges, NumEdges, Vertex1, Vertex2 );
			AddEdge( pEdges, NumEdges, Vertex2, Vertex0 );
		}
	}
	if( NumEdges * 6 >= KMaxShadowVertices )
	{
		m_Valid = kFalse;
		DELETE_ARRAY_SAFELY( pEdges );
		return E_FAIL;
	}

	D3DXVECTOR3*	pShadowVertices;
	// if it won't fit in our dynamic buffer, we want to discard and start at 0
	if( m_CurrentVertex + m_Vertices + NumEdges * 6 >= KMaxShadowVertices )
	{
		m_CurrentVertex = 0;

		if ( FAILED( m_pVB->Lock( m_CurrentVertex * sizeof( D3DXVECTOR3 ), 
								  NumEdges * 6 * sizeof( D3DXVECTOR3 ), 
								  ( BYTE * * ) &pShadowVertices, 
								  D3DLOCK_NOSYSLOCK | D3DLOCK_DISCARD ) ) )
		{
			DELETE_ARRAY_SAFELY( pEdges );
			return E_FAIL;
		}
	}
	else	// otherwise, we just want to append
	{
		m_CurrentVertex += m_Vertices;
		if ( FAILED( m_pVB->Lock( m_CurrentVertex * sizeof( D3DXVECTOR3 ), 
								  NumEdges * 6 * sizeof( D3DXVECTOR3 ), 
								  ( BYTE * * ) &pShadowVertices, 
								  D3DLOCK_NOSYSLOCK | D3DLOCK_NOOVERWRITE ) ) )
		{
			DELETE_ARRAY_SAFELY( pEdges );
			return E_FAIL;
		}
	}

	m_Vertices = 0;

	for( int32 i = 0; i < NumEdges; i ++ )
	{
		D3DXVECTOR3	v1	= pVertices[pEdges[2 * i + 0]];
		D3DXVECTOR3	v2	= pVertices[pEdges[2 * i + 1]];
		D3DXVECTOR3	v3	= v1 - LightDirection;
		D3DXVECTOR3	v4	= v2 - LightDirection;

		// Add a quad (two triangles) to the vertex list
		pShadowVertices[m_Vertices ++] = v1;
		pShadowVertices[m_Vertices ++] = v2;
		pShadowVertices[m_Vertices ++] = v3;

		pShadowVertices[m_Vertices ++] = v2;
		pShadowVertices[m_Vertices ++] = v4;
		pShadowVertices[m_Vertices ++] = v3;
	}
	// Delete the temporary edge list
	DELETE_ARRAY_SAFELY( pEdges );

	// Unlock the geometry buffers
	m_pVB->Unlock();

	return S_OK;
} // CShadowVolume::BuildFromFaceList()
