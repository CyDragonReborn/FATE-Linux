/*
	skinnedsubmesh.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

	Change History :
	9/25/2004	-	Fixed some issues with cacheing previous animation states. Discovered that to use
					the same VB for multiple animations, it is not always properly marked as 'dirty' if the
					polygons are not processed on the card, so you have to 'touch' the vb by locking and unlocking
					it to make sure that a dirty update happens. There may be a better way to do this, but for now...

	1/24/2005	-	Fixed a bug where verts were not being properly duplicated in some cases during IB optimization.

*/
#include <D3D8.h>
#include <d3dx8.h>


#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include "skeleton.h"
#include "skinnedmeshsegment.h"
#include "skinnedsubmesh.h"
#include "activeskinnedanimation.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../MODEL/shadowvolume.h"

#include "weightedvertex.h"


CSkinnedSubMesh::CSkinnedSubMesh( void ) :
											m_pVB( NULL ),
											m_pIB( NULL ),
											m_pShadowVolume( NULL ),
											m_pSkeleton( NULL ),
											m_pCurrentPose( NULL ),
											m_ShadowsEnabled( kFalse ),
											m_TotalFaces( 0 ),
											m_TotalVertices( 0 ),
											m_CurrentIndex( 0 ),
											m_Name( "" )
{
	ZeroMemory ( &m_MinBounds, sizeof( m_MinBounds ));
	ZeroMemory ( &m_MaxBounds, sizeof( m_MaxBounds ));
} // CSkinnedSubMesh::CSkinnedSubMesh()

CSkinnedSubMesh::~CSkinnedSubMesh( void )
{
	DELETE_SAFELY( m_pCurrentPose );
	m_pSkeleton = NULL;

	for( uint32 i = 0; i < m_pWeightedVertices.size(); i++ )
	{
		DELETE_SAFELY( m_pWeightedVertices[i] );
	}
	m_pWeightedVertices.clear();

	for( int32 i = 0; i < m_pSkinnedMeshSegments.size(); i++ )
	{
		DELETE_SAFELY( m_pSkinnedMeshSegments[i] );
	}
	m_pSkinnedMeshSegments.clear();

	DELETE_SAFELY( m_pShadowVolume );

	RELEASE_SAFELY( m_pVB );
	RELEASE_SAFELY( m_pIB );

} // CSkinnedSubMesh::~CSkinnedSubMesh()


void CSkinnedSubMesh::InvalidateDeviceObjects( void )
{
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->InvalidateDeviceObjects();
	}

} // CSkinnedSubMesh::InvalidateDeviceObjects()

void CSkinnedSubMesh::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->RestoreDeviceObjects( pD3DDevice );
	}
} // CSkinnedSubMesh::InvalidateDeviceObjects()


void CSkinnedSubMesh::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if ( !m_ShadowsEnabled )
	{
		m_ShadowsEnabled = kTrue;
		m_pShadowVolume = new CShadowVolume( pD3DDevice );
	}
} // CSkinnedSubMesh::EnableShadows()


// update the VB to reflect the current state of the verties
void CSkinnedSubMesh::CreateVB( LPDIRECT3DDEVICE8 pD3DDevice,						// direct3d device
								uint32 Faces,										// faces in the model
								std::vector <CWeightedVertex*>& WeightedVertices,	// vector of vertices
								std::vector <D3DXVECTOR3>& UVs )					// vector of uvs
{
	m_TotalVertices	= WeightedVertices.size();

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( m_TotalVertices*sizeof( PLAINVERTEXMODEL ),
												  D3DUSAGE_WRITEONLY,
												  D3DFVF_PLAINVERTEXMODEL,
												  D3DPOOL_MANAGED,
												  &m_pVB );

	if ( FAILED( hr ) )
	{
		return;
	}	


	PLAINVERTEXMODEL*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	D3DXVECTOR3 Vertex;
	for( uint32 k = 0; k < m_TotalVertices; k++ )
	{
		m_pWeightedVertices.push_back( WeightedVertices[k] );

		D3DXVec3TransformCoord( &Vertex, 
								&WeightedVertices[k]->Offset( 0 ),
								&m_pSkeleton->Bone( WeightedVertices[k]->BoneIndex( 0 ) )->WorldspaceTransformation() );
		D3DXVec3TransformNormal( &pVertices->n, 
								&WeightedVertices[k]->NormalOffset( 0 ),
								&m_pSkeleton->Bone( WeightedVertices[k]->BoneIndex( 0 ) )->WorldspaceTransformation() );
		if( k == 0 )
		{
			m_MinBounds = Vertex;
			m_MaxBounds = Vertex;
		}
		else
		{
			ExpandBounds( m_MinBounds, m_MaxBounds, Vertex );
		}
		pVertices->v = Vertex;

		pVertices->tu = UVs[k].x;
		pVertices->tv = UVs[k].y;
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

} // CSkinnedSubMesh::CreateVB()


// update the VB to reflect the current state of the vertices
void CSkinnedSubMesh::UpdateVB( CPose* pBlendedPose )	// final blended pose
{
	// calculate the correct offsets into the vertex list

	if( m_pCurrentPose == NULL )
	{
		m_pCurrentPose = new CPose( pBlendedPose->Bones() );
	}

	if( !pBlendedPose->IsEqual( m_pCurrentPose ) )
	{
		PLAINVERTEXMODEL*	pVertices;
		if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
		{
			return;
		}

		D3DXVECTOR3 FinalVertex( 0, 0, 0 );
		D3DXVECTOR3 FinalNormal( 0, 0, 0 );
		D3DXVECTOR3 TweenedVertex( 0, 0, 0 );
		D3DXVECTOR3 TweenedNormal( 0, 0, 0 );
		// for each frame, lerp between the current vert position and the next by the specified
		// interpolation factor
		for ( uint32 k = 0; k < m_TotalVertices; k++ )
		{		
			FinalVertex.x = 0;
			FinalVertex.y = 0;
			FinalVertex.z = 0;

			FinalNormal.x = 0;
			FinalNormal.y = 0;
			FinalNormal.z = 0;

			for( uint32 l = 0; l < m_pWeightedVertices[k]->Influences(); l++ )
			{
				D3DXVec3TransformCoord( &TweenedVertex, 
										&m_pWeightedVertices[k]->Offset( l ),
										&pBlendedPose->Transformation( m_pSkeleton->AnimationMap( m_pWeightedVertices[k]->BoneIndex( l ) ) ) );
				D3DXVec3TransformNormal( &TweenedNormal, 
										&m_pWeightedVertices[k]->NormalOffset( l ),
										&pBlendedPose->Transformation( m_pSkeleton->AnimationMap( m_pWeightedVertices[k]->BoneIndex( l ) ) ) );
				FinalVertex += TweenedVertex * m_pWeightedVertices[k]->Weight( l );
				FinalNormal += TweenedNormal * m_pWeightedVertices[k]->Weight( l );
			}
			pVertices[k].v = FinalVertex;		
			// todo: do we really need this?
			//D3DXVec3Normalize( &FinalNormal, &FinalNormal );
			pVertices[k].n = FinalNormal;	
		}
		m_pVB->Unlock();
		m_pCurrentPose->CopyFrom( pBlendedPose );
	}
	else
	{
		// WE NEED TO FLUSH THE CACHE, I GUESS - THERE MUST BE A BETTER WAY TO DO THIS!
		PLAINVERTEXMODEL*	pVertices;
		if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
		{
			return;
		}

		m_pVB->Unlock();
	}


} // CSkinnedSubMesh::UpdateVB()
// resize the index buffer to make sure it matches the number of indices
void CSkinnedSubMesh::CleanIB( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
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
	pIB->Lock( 0, m_CurrentIndex * sizeof( WORD ), ( BYTE * * ) &pNewIndices, 0 );

	m_pIB->Lock( 0, m_CurrentIndex * sizeof( WORD ), ( BYTE * * ) &pOriginalIndices, 0 );

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
	// we'll also have to re-update our IBs in the submodelsegments to reference
	// the correct ranges, based upon the new sorting done.
	// because we need to do this, we have to fill out the attribute buffer so
	// that it can arrange things according to material, and so that we can extract
	// the data from it to give it to the IBs again.
	LPD3DXMESH	MeshToOptimize;

	hr = D3DXCreateMeshFVF( m_CurrentIndex / 3,
						 m_TotalVertices,
						 D3DXMESH_SYSTEMMEM,
						 D3DFVF_PLAINVERTEXMODEL,
						 pD3DDevice,
						 &MeshToOptimize );


	if ( FAILED( hr ) )
	{
		return;
	}	

	// lock our optimization mesh's vb and our original vb
	PLAINVERTEXMODEL*	pOptimizeVertices;
	PLAINVERTEXMODEL*	pVertices;
	hr = MeshToOptimize->LockVertexBuffer( 0, ( BYTE ** ) &pOptimizeVertices );
	if ( FAILED( hr ) )
	{
		return;
	}	
	hr = m_pVB->Lock( 0, m_TotalVertices * sizeof( PLAINVERTEXMODEL ), ( BYTE ** ) &pVertices, NULL );
	if ( FAILED( hr ) )
	{
		return;
	}	

	// copy the original vb into the optimization mesh
	memcpy( pOptimizeVertices, pVertices, m_TotalVertices * sizeof( PLAINVERTEXMODEL ) );
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


	// now we need to set up the attribute buffer according to the indices of our submodelsegments
	// so that we can set up the appropriate index ranges after we're done optimizing

	DWORD *pAttributes;

	MeshToOptimize->LockAttributeBuffer( 0, &pAttributes );

	uint32 CurrentFace( 0 );
	for( uint32 i = 0; i < m_pSkinnedMeshSegments.size(); i++ )
	{
		// for the range of indices, set the attribute buffer to this segment's index
		for( uint32 j = 0; 
			j < m_pSkinnedMeshSegments[i]->TotalFaces(); 
			j++ )
		{
			pAttributes[CurrentFace++] = i;
		}
	}

	MeshToOptimize->UnlockAttributeBuffer();

	// generate an adjacency table (required)
	DWORD *pAdjacency = new DWORD[m_CurrentIndex];
    MeshToOptimize->GenerateAdjacency(0.1f, pAdjacency);
	// map of where our vertices were remapped to (so that we can adjust all our animation info - ulp!)
	LPD3DXBUFFER	VertexMap;

	// sort the ibs and vbs to consolidate the state changes and cache verts in the VB
	hr = MeshToOptimize->OptimizeInplace( D3DXMESHOPT_COMPACT | 
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
	hr	= pD3DDevice->CreateVertexBuffer( MeshToOptimize->GetNumVertices()*sizeof( PLAINVERTEXMODEL ),
										  D3DUSAGE_WRITEONLY,
										  D3DFVF_PLAINVERTEXMODEL,
										  D3DPOOL_MANAGED,
										  &m_pVB );

	hr = MeshToOptimize->LockVertexBuffer( NULL, ( BYTE ** ) &pOptimizeVertices );
	m_pVB->Lock( 0, 
				 MeshToOptimize->GetNumVertices()*sizeof( PLAINVERTEXMODEL ), 
				 ( BYTE * * ) &pVertices, 
				 NULL );

	// copy the original vb into the optimization mesh
	memcpy( pVertices, 
			pOptimizeVertices, 
			MeshToOptimize->GetNumVertices() * sizeof( PLAINVERTEXMODEL ) );
	// unlock the buffers
	hr = MeshToOptimize->UnlockVertexBuffer();
	m_pVB->Unlock();

	// the IB could be bigger now too. Possible. So we need to recreate it as well.
	RELEASE_SAFELY( m_pIB );
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

	D3DXATTRIBUTERANGE* pAttributeTable = new D3DXATTRIBUTERANGE[m_pSkinnedMeshSegments.size()];	
	DWORD				AttributeCount( 0 );
	MeshToOptimize->GetAttributeTable( pAttributeTable, &AttributeCount );

	for( int32 i = 0; i < m_pSkinnedMeshSegments.size(); i++ )
	{
		m_pSkinnedMeshSegments[pAttributeTable[i].AttribId]->SetIBIndex( pAttributeTable[i].FaceStart * 3 );
		m_pSkinnedMeshSegments[pAttributeTable[i].AttribId]->SetTotalFaces( pAttributeTable[i].FaceCount );
		m_pSkinnedMeshSegments[pAttributeTable[i].AttribId]->SetFirstVertIndex( pAttributeTable[i].VertexStart );
		m_pSkinnedMeshSegments[pAttributeTable[i].AttribId]->SetLastVertIndex( pAttributeTable[i].VertexStart + pAttributeTable[i].VertexCount - 1 );
	}

	MeshToOptimize->UnlockAttributeBuffer();

	uint32 NewVertexCount = MeshToOptimize->GetNumVertices();

	std::vector < CWeightedVertex* > Vertices( NewVertexCount ) ;	
	std::vector < bool > Touched( m_TotalVertices );

	// keep track of which vertices we kept
	for( uint32 m = 0; m < m_TotalVertices; m++ )
	{
		Touched[m] = kFalse;
	}

	DWORD* VertexIndex = ( DWORD* )VertexMap->GetBufferPointer();
	for( uint32 m = 0; m < NewVertexCount ; m++ )
	{
		uint32 index = VertexIndex[m];
		// if we've already got an instance of this vert, we need to make a new one
		if( Touched[index] )
		{
			Vertices[m] = new CWeightedVertex( m_pWeightedVertices[index] );
		}
		else
		{
			Vertices[m] = m_pWeightedVertices[index];
		}
		Touched[index] = kTrue;
	}
	// delete any un-kept vertices
	for( uint32 m = 0; m < m_TotalVertices; m++ )
	{
		if( !Touched[m] )
		{
			DELETE_SAFELY( m_pWeightedVertices[m] );
		}
	}

	m_pWeightedVertices.clear();

	m_pWeightedVertices = Vertices;

	m_TotalVertices = NewVertexCount;
	DELETE_SAFELY( pAdjacency );
	DELETE_SAFELY( pAttributeTable );
	RELEASE_SAFELY( MeshToOptimize );
	RELEASE_SAFELY( VertexMap );
	
} // CSkinnedSubMesh::CleanIB()

// add a new mesh segment - it will contain an index buffer to the given material ID
uint32 CSkinnedSubMesh::AddAnimatedMeshSegment( CRefManager& pRefManager,					// reference to refmanager
												 CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
												 LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
												 std::vector <D3DXVECTOR4>& Triangles,		// vector of triangle indices
												 uint32 MaterialID,							// material id
												 int32 MaterialType,						// material type
												 bool DoubleSided )							// double sided material
{
	CSkinnedMeshSegment*	Segment	= new CSkinnedMeshSegment( pRefManager, pSettings );

	m_pSkinnedMeshSegments.push_back( Segment );

	Segment->GenerateMesh( m_pIB,
						   m_CurrentIndex,
						   Triangles,
						   MaterialID,
						   MaterialType,
						   DoubleSided );

	m_TotalFaces +=	Segment->TotalFaces();

	return ( m_pSkinnedMeshSegments.size() - 1 );
} // CSkinnedSubMesh::AddModelSegment()

void CSkinnedSubMesh::SetMaterial( CMaterial* pMaterial,			// material to apply
									uint32 SubSegment )				// subsegment to apply to
{
	if ( SubSegment < m_pSkinnedMeshSegments.size() )
	{
		m_pSkinnedMeshSegments[SubSegment]->SetMaterial( pMaterial );
	}
} // CSkinnedSubMesh::SetMaterial()

void CSkinnedSubMesh::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device

{
	for( uint32 i = 0; i < m_pSkinnedMeshSegments.size(); i++ )
	{
		m_pSkinnedMeshSegments[i]->RenderNoTexture( pD3DDevice );
	}
} // CSkinnedSubMesh::RenderNoTexture()

// use the vb of the submodel to build the shadow (slowest)
void CSkinnedSubMesh::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							   const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
							   const D3DXVECTOR3& Light )			// direction of light shadow is being cast from
{
	// the shadow volume must constantly be rebuild to take into account light direction
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->BuildFromMesh( m_pVB,
										m_pIB,
										m_TotalFaces,
										Light );

		RenderShadows( pD3DDevice, CameraSpaceMatrix );
	}
	else
	{
		return;
	}
} // CSkinnedSubMesh::RenderShadows()

void CSkinnedSubMesh::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									  const D3DXMATRIX& CameraSpaceMatrix )	// camera space matrix
{

	// Disable z-buffer writes (note: z-testing still occurs), and enable the
	// stencil-buffer
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_STENCILENABLE, kTrue );

	// Dont bother with interpolating color
	pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

	// Set up stencil compare fuction, reference value, and masks.
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is kTrue.
	// Note: since we set up the stencil-test to always pass, the STENCILFAIL
	// renderstate is really not needed.

	pD3DDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	pD3DDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
	pD3DDevice->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );

	// If ztest passes, inc/decrement stencil buffer value
	pD3DDevice->SetRenderState( D3DRS_STENCILREF, 0x1 );
	pD3DDevice->SetRenderState( D3DRS_STENCILMASK, 0xffffffff );
	pD3DDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
	pD3DDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT );

	// Make sure that no pixels get drawn to the frame buffer
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	// Draw front-side of shadow volume in stencil/z only
	pD3DDevice->SetTransform( D3DTS_VIEW, &CameraSpaceMatrix );
	m_pShadowVolume->Render( pD3DDevice );


	//pD3DDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_LESSEQUAL );
	// Now reverse cull order so back sides of shadow volume are written.
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	// Decrement stencil buffer value
	pD3DDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
	m_pShadowVolume->Render( pD3DDevice );

	// Restore render states
	pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_STENCILENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
} // CSkinnedSubMesh::RenderShadow()

void CSkinnedSubMesh::SetVisible( uint32 SubSegment,			// subsegment to apply to
								   bool State )				// state to set
{
	assert( SubSegment < m_pSkinnedMeshSegments.size() );

	m_pSkinnedMeshSegments[SubSegment]->SetVisible( State );
} // CSkinnedSubMesh::SetMaterial()
