#include "d3dx8.h"
/*
	subscene.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/
#include <D3D8.h>
#include <d3dx8.h>


#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include "subscenesegment.h"
#include "subscene.h"

#include "../MEDIA/refmanager.h"

#include "../MEDIA/material.h"


CSubScene::CSubScene( void ) :
								m_pVB( NULL ),
								m_pIB( NULL ),
								m_VertexSize( sizeof( PLAINVERTEXSCENE ) ),
								m_TotalFaces( 0 ),
								m_TotalVertices( 0 ),
								m_CurrentIndex( 0 ),
								m_ZDistance( 0 ),
								m_VertexShader( 0 ),
								m_MinBounds ( 0, 0, 0 ),
								m_MaxBounds ( 0, 0, 0 )
{
} // CSubScene::CSubScene()

CSubScene::~CSubScene( void )
{

	for( uint32 i = 0; i < m_pSubSceneSegments.size(); i++ )
	{
		DELETE_SAFELY( m_pSubSceneSegments[i] );
	}
	m_pSubSceneSegments.clear();

	RELEASE_SAFELY( m_pVB );
	RELEASE_SAFELY( m_pIB );

} // CSubScene::~CSubScene()


// update the VB to reflect the current state of the verties
void CSubScene::CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   uint32 Faces,								// faces in the mesh
							   std::vector <uint32>& VertexReferences,	// vector of vertex references
							   std::vector <uint32>& UVReferences,		// vector of uv references
							   std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
							   std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
							   std::vector <D3DCOLOR>& VertexColors ) // vector of vertex colors
{
	// keep track of how large this vert type is - we'll need it at render time
	m_VertexSize = sizeof( PLAINVERTEXSCENE );
	m_VertexShader = D3DFVF_PLAINVERTEXSCENE;

	std::vector <PLAINVERTEXSCENE> VBVertices ( Vertices.size() );

	for ( uint32 k = 0; k < VertexReferences.size(); k++ )
	{
		// if this vert hasn't been filled into the list yet, fill it out
		if( Vertices[VertexReferences[k]] != VBVertices[VertexReferences[k]].v )
		{
			VBVertices[VertexReferences[k]].v = Vertices[VertexReferences[k]];
			VBVertices[VertexReferences[k]].color = VertexColors[ k ];
			VBVertices[VertexReferences[k]].tu = UVs[UVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv = UVs[UVReferences[k]].y;	
		}
		else
		{
			// if this vertex doesn't match its index, we have to make a new vert, and give it a new index
			if( !( VBVertices[VertexReferences[k]].color == VertexColors[ k ] &&
				   VBVertices[VertexReferences[k]].tu == UVs[UVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv == UVs[UVReferences[k]].y ) )
			{
				PLAINVERTEXSCENE NewVertex;
				NewVertex.v = Vertices[VertexReferences[k]];
				NewVertex.color = VertexColors[k];
				NewVertex.tu = UVs[UVReferences[k]].x;
				NewVertex.tv = UVs[UVReferences[k]].y;
				// assign a new vertex reference
				VertexReferences[k] = VBVertices.size();
				// and put the new vert in the list
				VBVertices.push_back( NewVertex );
			}
		}
	}

	m_TotalVertices	= VBVertices.size();

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( m_TotalVertices * m_VertexSize,
												  D3DUSAGE_WRITEONLY,
												  m_VertexShader,
												  D3DPOOL_MANAGED,
												  &m_pVB );

	if ( FAILED( hr ) )
	{
		return;
	}	


	PLAINVERTEXSCENE*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	m_pVertices.resize( m_TotalVertices );

	for( int32 k = 0; k < m_TotalVertices; k++ )
	{
		*pVertices = VBVertices[k];
		m_pVertices[k] = VBVertices[k].v;

		pVertices++;
	}
	m_pVB->Unlock();

	// create an IB that is as large as we might possibly go ( no vert refs, and all double-sided )
	hr = pD3DDevice->CreateIndexBuffer( Faces * 6 * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pIB );

	if ( FAILED( hr ) )
	{
		return;
	}	

} // CSubScene::CreateBuffers()

// update the VB to reflect the current state of the verties
// this one takes secondary mapping cooridinates for a shadow map
void CSubScene::CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   uint32 Faces,								// faces in the mesh
							   std::vector <uint32>& VertexReferences,	// vector of vertex references
							   std::vector <uint32>& UVReferences,		// vector of uv references
							   std::vector <uint32>& ShadowUVReferences,	// vector of uv references for shadow mapping layer
							   std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
							   std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
							   std::vector <D3DCOLOR>& VertexColors ) // vector of vertex colors
{
	// keep track of how large this vert type is - we'll need it at render time
	m_VertexSize = sizeof( SHADOWVERTEXSCENE );
	m_VertexShader = D3DFVF_SHADOWVERTEXSCENE;

	std::vector <SHADOWVERTEXSCENE> VBVertices ( Vertices.size() );

	for ( uint32 k = 0; k < VertexReferences.size(); k++ )
	{
		// if this vert hasn't been filled into the list yet, fill it out
		if( Vertices[VertexReferences[k]] != VBVertices[VertexReferences[k]].v )
		{
			VBVertices[VertexReferences[k]].v = Vertices[VertexReferences[k]];
			VBVertices[VertexReferences[k]].color = VertexColors[ k ];
			VBVertices[VertexReferences[k]].tu = UVs[UVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv = UVs[UVReferences[k]].y;			
			VBVertices[VertexReferences[k]].tu2 = UVs[ShadowUVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv2 = UVs[ShadowUVReferences[k]].y;			
		}
		else
		{
			// if this vertex doesn't match its index, we have to make a new vert, and give it a new index
			if( !( VBVertices[VertexReferences[k]].color == VertexColors[ k ] &&
				   VBVertices[VertexReferences[k]].tu == UVs[UVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv == UVs[UVReferences[k]].y &&
				   VBVertices[VertexReferences[k]].tu2 == UVs[ShadowUVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv2 == UVs[ShadowUVReferences[k]].y ) )
			{
				SHADOWVERTEXSCENE NewVertex;
				NewVertex.v = Vertices[VertexReferences[k]];
				NewVertex.color = VertexColors[k];
				NewVertex.tu = UVs[UVReferences[k]].x;
				NewVertex.tv = UVs[UVReferences[k]].y;
				NewVertex.tu2 = UVs[ShadowUVReferences[k]].x;
				NewVertex.tv2 = UVs[ShadowUVReferences[k]].y;
				// assign a new vertex reference
				VertexReferences[k] = VBVertices.size();
				// and put the new vert in the list
				VBVertices.push_back( NewVertex );
			}
		}
	}

	uint32	TotalVertices	= VBVertices.size();

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( TotalVertices * m_VertexSize,
												  D3DUSAGE_WRITEONLY,
												  m_VertexShader,
												  D3DPOOL_MANAGED,
												  &m_pVB );

	if ( FAILED( hr ) )
	{
		return;
	}	


	SHADOWVERTEXSCENE*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	m_pVertices.resize( TotalVertices );

	for( int32 k = 0; k < TotalVertices; k++ )
	{
		*pVertices = VBVertices[k];
		m_pVertices[k] = VBVertices[k].v;

		pVertices++;
	}
	m_pVB->Unlock();

	// create an IB that is as large as we might possibly go ( no vert refs, and all double-sided )
	hr = pD3DDevice->CreateIndexBuffer( Faces * 6 * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pIB );

	if ( FAILED( hr ) )
	{
		return;
	}	

} // CSubScene::CreateBuffers()

void CSubScene::CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   uint32 Faces,								// faces in the mesh
							   std::vector <uint32>& NormalReferences,	// vector of normal references
							   std::vector <D3DXVECTOR3>& Normals,		// vector of normals
							   std::vector <uint32>& VertexReferences,	// vector of vertex references
							   std::vector <uint32>& UVReferences,		// vector of uv references
							   std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
							   std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
							   std::vector <D3DCOLOR>& VertexColors ) // vector of vertex colors
{
	// keep track of how large this vert type is - we'll need it at render time
	m_VertexSize = sizeof( LITPLAINVERTEXSCENE );
	m_VertexShader = D3DFVF_LITPLAINVERTEXSCENE;

	std::vector <LITPLAINVERTEXSCENE> VBVertices ( Vertices.size() );

	for ( uint32 k = 0; k < VertexReferences.size(); k++ )
	{
		// if this vert hasn't been filled into the list yet, fill it out
		if( Vertices[VertexReferences[k]] != VBVertices[VertexReferences[k]].v )
		{
			VBVertices[VertexReferences[k]].v = Vertices[VertexReferences[k]];
			VBVertices[VertexReferences[k]].n = Normals[NormalReferences[k]];
			VBVertices[VertexReferences[k]].color = VertexColors[ k ];
			VBVertices[VertexReferences[k]].tu = UVs[UVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv = UVs[UVReferences[k]].y;			
		}
		else
		{
			// if this vertex doesn't match its index, we have to make a new vert, and give it a new index
			if( !( VBVertices[VertexReferences[k]].color == VertexColors[ k ] &&
				   VBVertices[VertexReferences[k]].n == Normals[NormalReferences[k]] &&
				   VBVertices[VertexReferences[k]].tu == UVs[UVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv == UVs[UVReferences[k]].y ) )
			{
				LITPLAINVERTEXSCENE NewVertex;
				NewVertex.v = Vertices[VertexReferences[k]];
				NewVertex.n = Normals[NormalReferences[k]];
				NewVertex.color = VertexColors[k];
				NewVertex.tu = UVs[UVReferences[k]].x;
				NewVertex.tv = UVs[UVReferences[k]].y;
				// assign a new vertex reference
				VertexReferences[k] = VBVertices.size();
				// and put the new vert in the list
				VBVertices.push_back( NewVertex );
			}
		}
	}

	m_TotalVertices	= VBVertices.size();

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( m_TotalVertices * m_VertexSize,
												  D3DUSAGE_WRITEONLY,
												  m_VertexShader,
												  D3DPOOL_MANAGED,
												  &m_pVB );

	if ( FAILED( hr ) )
	{
		return;
	}	


	LITPLAINVERTEXSCENE*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	m_pVertices.resize( m_TotalVertices );

	for( int32 k = 0; k < m_TotalVertices; k++ )
	{
		*pVertices = VBVertices[k];
		m_pVertices[k] = VBVertices[k].v;

		pVertices++;
	}
	m_pVB->Unlock();

	// create an IB that is as large as we might possibly go ( no vert refs, and all double-sided )
	hr = pD3DDevice->CreateIndexBuffer( Faces * 6 * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pIB );

	if ( FAILED( hr ) )
	{
		return;
	}	

} // CSubScene::CreateBuffers()

// update the VB to reflect the current state of the verties
// this one takes secondary mapping cooridinates for a shadow map
void CSubScene::CreateBuffers( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   uint32 Faces,								// faces in the mesh
							   std::vector <uint32>& NormalReferences,	// vector of normal references
							   std::vector <D3DXVECTOR3>& Normals,		// vector of normals
							   std::vector <uint32>& VertexReferences,	// vector of vertex references
							   std::vector <uint32>& UVReferences,		// vector of uv references
							   std::vector <uint32>& ShadowUVReferences,	// vector of uv references for shadow mapping layer
							   std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
							   std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
							   std::vector <D3DCOLOR>& VertexColors ) // vector of vertex colors
{
	// keep track of how large this vert type is - we'll need it at render time
	m_VertexSize = sizeof( LITSHADOWVERTEXSCENE );
	m_VertexShader = D3DFVF_LITSHADOWVERTEXSCENE;

	std::vector <LITSHADOWVERTEXSCENE> VBVertices ( Vertices.size() );

	for ( uint32 k = 0; k < VertexReferences.size(); k++ )
	{
		// if this vert hasn't been filled into the list yet, fill it out
		if( Vertices[VertexReferences[k]] != VBVertices[VertexReferences[k]].v )
		{
			VBVertices[VertexReferences[k]].v = Vertices[VertexReferences[k]];
			VBVertices[VertexReferences[k]].n = Normals[NormalReferences[k]];
			VBVertices[VertexReferences[k]].color = VertexColors[ k ];
			VBVertices[VertexReferences[k]].tu = UVs[UVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv = UVs[UVReferences[k]].y;			
			VBVertices[VertexReferences[k]].tu2 = UVs[ShadowUVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv2 = UVs[ShadowUVReferences[k]].y;			
		}
		else
		{
			// if this vertex doesn't match its index, we have to make a new vert, and give it a new index
			if( !( VBVertices[VertexReferences[k]].color == VertexColors[ k ] &&
				   VBVertices[VertexReferences[k]].n == Normals[NormalReferences[k]] &&
				   VBVertices[VertexReferences[k]].tu == UVs[UVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv == UVs[UVReferences[k]].y &&
				   VBVertices[VertexReferences[k]].tu2 == UVs[ShadowUVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv2 == UVs[ShadowUVReferences[k]].y ) )
			{
				LITSHADOWVERTEXSCENE NewVertex;
				NewVertex.v = Vertices[VertexReferences[k]];
				NewVertex.n = Normals[NormalReferences[k]];
				NewVertex.color = VertexColors[k];
				NewVertex.tu = UVs[UVReferences[k]].x;
				NewVertex.tv = UVs[UVReferences[k]].y;
				NewVertex.tu2 = UVs[ShadowUVReferences[k]].x;
				NewVertex.tv2 = UVs[ShadowUVReferences[k]].y;
				// assign a new vertex reference
				VertexReferences[k] = VBVertices.size();
				// and put the new vert in the list
				VBVertices.push_back( NewVertex );
			}
		}
	}

	m_TotalVertices	= VBVertices.size();

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( m_TotalVertices * m_VertexSize,
												  D3DUSAGE_WRITEONLY,
												  m_VertexShader,
												  D3DPOOL_MANAGED,
												  &m_pVB );

	if ( FAILED( hr ) )
	{
		return;
	}	


	LITSHADOWVERTEXSCENE*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	m_pVertices.resize( m_TotalVertices );

	for( int32 k = 0; k < m_TotalVertices; k++ )
	{
		*pVertices = VBVertices[k];
		m_pVertices[k] = VBVertices[k].v;

		pVertices++;
	}
	m_pVB->Unlock();

	// create an IB that is as large as we might possibly go ( no vert refs, and all double-sided )
	hr = pD3DDevice->CreateIndexBuffer( Faces * 6 * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pIB );

	if ( FAILED( hr ) )
	{
		return;
	}	

} // CSubScene::CreateBuffers()

// transforms vertex normals into light-space and stores them in the diffuse RGBA-
// removes any current vertex colors. For use in normal mapping
void CSubScene::CalculateTangentNormals( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
										 const D3DXVECTOR3& LightDirection, // light direction
										 uint32 StartingVertex,				// vertex to begin the calculation at
										 uint32 EndingVertex )				// vertex to end the calculation at
{

	HRESULT hr;
	D3DXVECTOR3 Normal;
	uint32 FinalVertex = EndingVertex - StartingVertex + 1;
	if( m_VertexShader == D3DFVF_LITSHADOWVERTEXSCENE )
	{
		LITSHADOWVERTEXSCENE*	pVertices;
		hr = m_pVB->Lock( StartingVertex * m_VertexSize , 
						  FinalVertex * m_VertexSize, 
						  ( BYTE ** ) &pVertices, 
						  NULL );
		if ( FAILED( hr ) )
		{
			return;
		}	
		for( uint32 i = 0; i < FinalVertex; i++ )
		{
			Normal.x = D3DXVec3Dot( &LightDirection, &m_Tangents[i+StartingVertex] );
			Normal.y = D3DXVec3Dot( &LightDirection, &m_Binormals[i+StartingVertex] );
			Normal.z = D3DXVec3Dot( &LightDirection, &m_Normals[i+StartingVertex] );

			pVertices[i].color = VectorToRGB( Normal );
		}
	}
	else
	{
		LITPLAINVERTEXSCENE*	pVertices;
		hr = m_pVB->Lock( StartingVertex * m_VertexSize , 
						  FinalVertex * m_VertexSize, 
						  ( BYTE ** ) &pVertices, 
						  NULL );
		if ( FAILED( hr ) )
		{
			return;
		}	
		for( uint32 i = 0; i < FinalVertex; i++ )
		{
			Normal.x = D3DXVec3Dot( &LightDirection, &m_Tangents[i+StartingVertex] );
			Normal.y = D3DXVec3Dot( &LightDirection, &m_Binormals[i+StartingVertex] );
			Normal.z = D3DXVec3Dot( &LightDirection, &m_Normals[i+StartingVertex] );

			pVertices[i].color = VectorToRGB( Normal );
		}
	}

	m_pVB->Unlock();
} // CSubScene::CalculateTangentNormals()

// resize the index buffer to make sure it matches the number of indices
void CSubScene::CleanIB( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	if( m_pIB == NULL )
	{
		return;
	}
	
	// create a new IB of the appropriate size
	LPDIRECT3DINDEXBUFFER8			pIB;			
	HRESULT hr = pD3DDevice->CreateIndexBuffer( m_CurrentIndex * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&pIB );

	if ( FAILED( hr ) )
	{
		return;
	}	
	WORD*	pOriginalIndices;
	WORD*	pNewIndices;

	// lock the new and old buffer
	pIB->Lock( 0, m_CurrentIndex * sizeof( WORD ), ( BYTE ** ) &pNewIndices, 0 );

	m_pIB->Lock( 0, m_CurrentIndex * sizeof( WORD ), ( BYTE ** ) &pOriginalIndices, 0 );

	// copy the indices we want
	memcpy( pNewIndices, pOriginalIndices, m_CurrentIndex * sizeof( WORD ) );

	// unlock them both
	pIB->Unlock();
	m_pIB->Unlock();

	// release the original buffer, and set it to the new one
	RELEASE_SAFELY( m_pIB );
	m_pIB = pIB;

	// we're going to create a d3dx mesh, copy our vb and ib into it,
	// optimize it for vertex cache, and then copy the data back out.
	// we'll also have to re-update our IBs in the subScenesegments to reference
	// the correct ranges, based upon the new sorting done.
	// because we need to do this, we have to fill out the attribute buffer so
	// that it can arrange things according to material, and so that we can extract
	// the data from it to give it to the IBs again.
	LPD3DXMESH	MeshToOptimize;
	hr = D3DXCreateMeshFVF( m_CurrentIndex / 3,
						 m_TotalVertices,
						 D3DXMESH_SYSTEMMEM,
						 m_VertexShader,
						 pD3DDevice,
						 &MeshToOptimize );

	if ( FAILED( hr ) )
	{
		return;
	}	

	// lock our optimization mesh's vb and our original vb
	BYTE*	pOptimizeVertices;
	BYTE*	pVertices;

	hr = MeshToOptimize->LockVertexBuffer( 0, ( BYTE ** ) &pOptimizeVertices );
	if ( FAILED( hr ) )
	{
		return;
	}	
	hr = m_pVB->Lock( 0, 0, ( BYTE ** ) &pVertices, NULL );
	if ( FAILED( hr ) )
	{
		return;
	}	

	// copy the original vb into the optimization mesh
	memcpy( pOptimizeVertices, pVertices, m_TotalVertices  * m_VertexSize );
	// unlock the buffers
	MeshToOptimize->UnlockVertexBuffer();
	m_pVB->Unlock();

//	now we need to copy over the IBs

	WORD*	pOptimizeIndices;
	WORD*	pIndices;
	MeshToOptimize->LockIndexBuffer( 0, ( BYTE ** ) &pOptimizeIndices );
	m_pIB->Lock( 0, m_CurrentIndex * sizeof( WORD ), ( BYTE ** ) &pIndices, 0 );

	// copy the original ib into the optimization mesh
	memcpy( pOptimizeIndices, pIndices, m_CurrentIndex * sizeof( WORD ) );
	// unlock the buffers
	MeshToOptimize->UnlockIndexBuffer();
	m_pIB->Unlock();


	// now we need to set up the attribute buffer according to the indices of our subScenesegments
	// so that we can set up the appropriate index ranges after we're done optimizing

	DWORD *pAttributes;

	MeshToOptimize->LockAttributeBuffer( 0, &pAttributes );

	uint32 CurrentFace( 0 );
	for( uint32 i = 0; i < m_pSubSceneSegments.size(); i++ )
	{
		// for the range of indices, set the attribute buffer to this segment's index
		for( uint32 j = 0; 
			j < m_pSubSceneSegments[i]->TotalFaces(); 
			j++ )
		{
			pAttributes[CurrentFace++] = i;
		}
	}

	MeshToOptimize->UnlockAttributeBuffer();

	// generate an adjacency table (required)
	DWORD *pAdjacency = new DWORD[m_CurrentIndex];
    MeshToOptimize->GenerateAdjacency(0.0f, pAdjacency);

	// map of where our vertices were remapped to (so that we can adjust all our base vert info)
	LPD3DXBUFFER	VertexMap;

	// sort the ibs and vbs to consolidate the state changes and cache verts in the VB
	MeshToOptimize->OptimizeInplace( D3DXMESHOPT_COMPACT | 
									 D3DXMESHOPT_ATTRSORT | 
									 D3DXMESHOPT_VERTEXCACHE ,
									 pAdjacency,
									 NULL,
									 NULL,
 									 &VertexMap );

	// now we've got to copy everything back!  
	// BUT! strange but true, the new buffers MIGHT be larger to take advantage of
	// cacheing, and to rearrange verts. Odd. True.
	// so we create a new vb that we ensure is the correct size
	RELEASE_SAFELY( m_pVB );
	hr	= pD3DDevice->CreateVertexBuffer( MeshToOptimize->GetNumVertices() * m_VertexSize,
										  D3DUSAGE_WRITEONLY,
										  m_VertexShader,
										  D3DPOOL_MANAGED,
										  &m_pVB );

	MeshToOptimize->LockVertexBuffer( NULL, ( BYTE ** ) &pOptimizeVertices );

	// store our new vertex total
	m_TotalVertices = MeshToOptimize->GetNumVertices();
	
	m_pVB->Lock( 0, 
				 MeshToOptimize->GetNumVertices() * m_VertexSize, 
				 ( BYTE * * ) &pVertices, 
				 0 );

	// copy the original vb into the optimization mesh
	memcpy( pVertices, 
			pOptimizeVertices, 
			MeshToOptimize->GetNumVertices() * m_VertexSize );

	// unlock the buffers
	MeshToOptimize->UnlockVertexBuffer();
	m_pVB->Unlock();

	// the IB could be bigger now too. Possible. So we need to recreate it as well.
	RELEASE_SAFELY( m_pIB );

	// store the new face total
	m_TotalFaces = MeshToOptimize->GetNumFaces();

	hr = pD3DDevice->CreateIndexBuffer( MeshToOptimize->GetNumFaces() * 3 * sizeof( WORD ),
										D3DUSAGE_WRITEONLY,
										D3DFMT_INDEX16,
										D3DPOOL_MANAGED,
										&m_pIB );

	MeshToOptimize->LockIndexBuffer( NULL, ( BYTE ** ) &pOptimizeIndices );
	m_pIB->Lock( 0, 
				 MeshToOptimize->GetNumFaces() * 3 * sizeof( WORD ), 
				 ( BYTE ** ) &pIndices, 
				 0 );

	// copy the original ib into the optimization mesh
	memcpy( pIndices, pOptimizeIndices, MeshToOptimize->GetNumFaces() * 3 * sizeof( WORD ) );
	// unlock the buffers
	MeshToOptimize->UnlockIndexBuffer();
	m_pIB->Unlock();

	// now we have to go through the attribute buffer, and reassign indices

	D3DXATTRIBUTERANGE* pAttributeTable = new D3DXATTRIBUTERANGE[m_pSubSceneSegments.size()];	
	DWORD				AttributeCount( 0 );
	MeshToOptimize->GetAttributeTable( pAttributeTable, &AttributeCount );

	for( int32 i = 0; i < m_pSubSceneSegments.size(); i++ )
	{
		m_pSubSceneSegments[pAttributeTable[i].AttribId]->SetIBIndex( pAttributeTable[i].FaceStart * 3 );
		m_pSubSceneSegments[pAttributeTable[i].AttribId]->SetTotalFaces( pAttributeTable[i].FaceCount );
		m_pSubSceneSegments[pAttributeTable[i].AttribId]->SetFirstVertIndex( pAttributeTable[i].VertexStart );
		m_pSubSceneSegments[pAttributeTable[i].AttribId]->SetLastVertIndex( pAttributeTable[i].VertexStart + pAttributeTable[i].VertexCount - 1 );
	}

	MeshToOptimize->UnlockAttributeBuffer();

	// now we have to update our base vertex list for use in morphing -
	// since we've optimized the mesh, the vertex count and arrangement
	// are different, and we need to apply those changes back to our raw vertices
	uint32 NewVertexCount = MeshToOptimize->GetNumVertices();

	std::vector <D3DXVECTOR3>		Vertices( NewVertexCount ) ;	

	DWORD* VertexIndex = ( DWORD* )VertexMap->GetBufferPointer();
	for( uint32 m = 0; m < NewVertexCount ; m++ )
	{
		uint32 index = VertexIndex[m];
		Vertices[m] = m_pVertices[index];
	}
	m_pVertices.clear();

	m_pVertices = Vertices;

	DELETE_SAFELY( pAdjacency );
	DELETE_SAFELY( pAttributeTable );
	RELEASE_SAFELY( MeshToOptimize );
	RELEASE_SAFELY( VertexMap );

} // CSubScene::CleanIB()

// generate tangent and binormal vectors for use in normal mapping with dotproduct3
void CSubScene::GenerateTangentBinormal( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
{
	HRESULT hr;
	// we're going to create a d3dx mesh, copy our vb and ib into it,
	// calculate tangent values, and then extract them into m_Tangents and calculate m_Binormals
	// we do it this way, because the D3DXComputeTangents method has support for averaging -
	// which is VERY helpful, otherwise you get non-matching tangent vectors
	LPD3DXMESH	TangentMesh;
	LPD3DXMESH	FinalMesh;
	hr = D3DXCreateMeshFVF( m_TotalFaces,
							m_TotalVertices,
							D3DXMESH_SYSTEMMEM,
							m_VertexShader,
							pD3DDevice,
							&TangentMesh );

	if ( FAILED( hr ) )
	{
		return;
	}	

	// lock our optimization mesh's vb and our original vb
	BYTE*	pTangentVertices;
	BYTE*	pOriginalVertices;

	hr = TangentMesh->LockVertexBuffer( D3DLOCK_DISCARD, ( BYTE ** ) &pTangentVertices );
	if ( FAILED( hr ) )
	{
		return;
	}	
	hr = m_pVB->Lock( 0, 0, ( BYTE ** ) &pOriginalVertices, NULL );
	if ( FAILED( hr ) )
	{
		return;
	}	

	// copy the original vb into the optimization mesh
	memcpy( pTangentVertices, pOriginalVertices, m_TotalVertices  * m_VertexSize );
	// unlock the buffers
	TangentMesh->UnlockVertexBuffer();
	m_pVB->Unlock();

	//	now we need to copy over the IBs
	WORD*	pTangentIndices;
	WORD*	pIndices;
	TangentMesh->LockIndexBuffer( 0, ( BYTE ** ) &pTangentIndices );
	m_pIB->Lock( 0, m_TotalFaces * 3 * sizeof( WORD ), ( BYTE ** ) &pIndices, 0 );

	// copy the original ib into the optimization mesh
	memcpy( pTangentIndices, pIndices, m_TotalFaces * 3 * sizeof( WORD ) );
	// unlock the buffers
	TangentMesh->UnlockIndexBuffer();
	m_pIB->Unlock();

	// put it in a format with space for our tangent info
	TangentMesh->CloneMeshFVF( D3DXMESH_SYSTEMMEM, D3DFVF_TANGENTSPACEVERTEX, pD3DDevice, &FinalMesh );

	// clean up this temporary d3dxmesh
	RELEASE_SAFELY( TangentMesh );

	// calculate the tangent vectors into U
	D3DXComputeTangent( FinalMesh, 0, FinalMesh, 1, D3DX_COMP_TANGENT_NONE, kTrue, NULL );

	// allocate the data to be filled out
	m_Binormals.resize( m_TotalVertices );
	m_Tangents.resize( m_TotalVertices );
	m_Normals.resize( m_TotalVertices );

	TANGENTSPACEVERTEX*	pVertices;

	// Lock the geometry buffers
	if( FAILED( FinalMesh->LockVertexBuffer( D3DLOCK_DISCARD, ( BYTE ** ) &pVertices ) ) )
	{
		return;
	}


	// For each vertex, we copy out the tangent from the temporary mesh we created,
	// and then we cross it with the vert normal to get the binormal
	for ( uint32 i = 0; i < m_TotalVertices; i ++ )
	{
		TANGENTSPACEVERTEX* CurrentVertex = &pVertices[ i ];

		m_Tangents[i] = CurrentVertex->u;
		m_Normals[i] = CurrentVertex->n;
			
		// The Binormal vector is the vector perpendicular to the Normal and Tangent (and
		// that follows the AlignedVector direction)
		D3DXVec3Cross( &m_Binormals[i], &CurrentVertex->n, &m_Tangents[i] );

		// normalize the binormal and tangent
		D3DXVec3Normalize( &m_Tangents[i], &m_Tangents[i] );
		D3DXVec3Normalize( &m_Binormals[i], &m_Binormals[i] );

	}
	// Unlock the buffers
	FinalMesh->UnlockVertexBuffer();

	// clean up
	RELEASE_SAFELY( FinalMesh );

} // CSubScene::GenerateTangentBinormal()

uint32 CSubScene::AddSubSceneSegment( CRefManager& pRefManager,					// reference to refmanager
									  CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
									  LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									  std::vector <uint32>& VertexReferences,	// vector of vertex references
									  std::vector <D3DXVECTOR4>& Triangles,		// vector of triangle indices
									  uint32 MaterialID,						// material id
									  int32 MaterialType,						// material type
									  bool MaterialDoubleSided )					// is double sided?
{
	CSubSceneSegment*	Segment	= new CSubSceneSegment( pRefManager, 
														pSettings,
														this );

	m_pSubSceneSegments.push_back( Segment );

	Segment->GenerateMesh( m_pIB,
						   m_CurrentIndex,
						   m_pVertices,
						   Triangles,
						   VertexReferences,
						   MaterialID,
						   MaterialType,
						   MaterialDoubleSided );

	m_TotalFaces +=	Segment->TotalFaces();

	return ( m_pSubSceneSegments.size() - 1 );
} // CSubScene::AddModelSegment()

bool CSubScene::HasNormalMapping( void )
{
	for( uint32 i = 0; i < m_pSubSceneSegments.size(); i++ )
	{
		CMaterial* Material = m_pSubSceneSegments[i]->GetMaterial();
		if( Material != NULL &&
			Material->HasNormalMap() )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CSubScene::HasNormalMapping()

void CSubScene::SetMaterial( CMaterial* pMaterial,			// material to apply
							 uint32 SubSegment )			// subsegment to apply to
{
	assert( SubSegment < m_pSubSceneSegments.size() );

	m_pSubSceneSegments[SubSegment]->SetMaterial( pMaterial );
} // CSubScene::SetMaterial()

void CSubScene::SetShadowMaterial( CMaterial* pMaterial,			// material to apply
								   uint32 SubSegment )			// subsegment to apply to
{
	assert ( SubSegment < m_pSubSceneSegments.size() );

	m_pSubSceneSegments[SubSegment]->SetShadowMaterial( pMaterial );
} // CSubScene::SetShadowMaterial()

void CSubScene::SetVisible( uint32 SubSegment,			// subsegment to apply to
						    bool State )				// state to set
{
	assert( SubSegment < m_pSubSceneSegments.size() );

	m_pSubSceneSegments[SubSegment]->SetVisible( State );
} // CSubScene::SetMaterial()
