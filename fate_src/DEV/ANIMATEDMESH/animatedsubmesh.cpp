/*
	animatedsubmesh.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	4/14/2004	-	big changes to vert/index buffer arrangement! consolidated them
					for the entire model, and subsegments now only have an index
					to their position in the parent buffer. Minimizes vert buffer changes

*/
#include <D3D8.h>
#include <d3dx8.h>


#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include "animatedmeshsegment.h"
#include "animatedsubmesh.h"
#include "activeanimation.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../MODEL/shadowvolume.h"


CAnimatedSubMesh::CAnimatedSubMesh( void ) :
												m_pVB( NULL ),
												m_pIB( NULL ),
												m_pShadowVolume( NULL ),
												m_ShadowsEnabled( kFalse ),
												m_TotalFaces( 0 ),
												m_TotalVertices( 0 ),
												m_FrameVertices( 0 ),
												m_CurrentFrame( 0 ),
												m_NextFrame( 0 ),
												m_Interpolation( 0 ),
												m_CurrentIndex( 0 ),
												m_Name( "" ),
												m_MaxBounds( 0, 0, 0 ),
												m_MinBounds( 0, 0, 0 )
{
} // CAnimatedSubMesh::CAnimatedSubMesh()

CAnimatedSubMesh::~CAnimatedSubMesh( void )
{

	for( uint32 i = 0; i < m_pAnimatedMeshSegments.size(); i++ )
	{
		DELETE_SAFELY( m_pAnimatedMeshSegments[i] );
	}
	m_pAnimatedMeshSegments.clear();

	DELETE_SAFELY( m_pShadowVolume );

	RELEASE_SAFELY( m_pVB );
	RELEASE_SAFELY( m_pIB );

} // CAnimatedSubMesh::~CAnimatedSubMesh()


void CAnimatedSubMesh::InvalidateDeviceObjects( void )
{
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->InvalidateDeviceObjects();
	}

} // CAnimatedSubMesh::InvalidateDeviceObjects()

void CAnimatedSubMesh::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->RestoreDeviceObjects( pD3DDevice );
	}
} // CAnimatedSubMesh::InvalidateDeviceObjects()


void CAnimatedSubMesh::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if ( !m_ShadowsEnabled )
	{
		m_ShadowsEnabled = kTrue;
		m_pShadowVolume = new CShadowVolume( pD3DDevice );
	}
} // CAnimatedSubMesh::EnableShadows()


// update the VB to reflect the current state of the verties
void CAnimatedSubMesh::CreateVB( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
						  uint32 VerticesPerFrame,					// how many vertices the base model has
						  uint32 Faces,								// faces in the model
						  std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
						  std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
						  std::vector <D3DXVECTOR3>& Normals )		// vector of normals
{
	m_TotalVertices	= Vertices.size();

	m_FrameVertices = VerticesPerFrame;


	if( m_TotalVertices == 0 )
	{
		return;
	}

	HRESULT	hr	= pD3DDevice->CreateVertexBuffer( VerticesPerFrame*sizeof( PLAINVERTEXMODEL ),
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

	m_pVertices.resize( m_TotalVertices );
	m_pNormals.resize( m_TotalVertices );

	for( uint32 k = 0; k < m_TotalVertices; k++ )
	{
		m_pVertices[k] = Vertices[k];
		m_pNormals[k] = Normals[k];
	}

	for( int32 k = 0; k < VerticesPerFrame; k++ )
	{
		pVertices->v = Vertices[k];
		pVertices->n = Normals[k];

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

} // CAnimatedSubMesh::CreateVB()


// update the VB to reflect the current state of the verties
void CAnimatedSubMesh::UpdateVB( uint32 CurrentFrame,		// current frame
								 uint32 NextFrame,			// frame to interpolate to
								 float32 Interpolation )	// interpolation factor
{

	PLAINVERTEXMODEL*	pVertices;
	// only update the VB if something has changed!
	if( CurrentFrame == m_CurrentFrame &&
		NextFrame == m_NextFrame &&
		m_Interpolation == Interpolation )
	{
		if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
		{
			return;
		}

		m_pVB->Unlock();
		return;
	}

	m_CurrentFrame = CurrentFrame;
	m_NextFrame = NextFrame;
	m_Interpolation = Interpolation;


	// calculate the correct offsets into the vertex list
	CurrentFrame *= m_FrameVertices;
	NextFrame *= m_FrameVertices;

	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}


	// for each frame, lerp between the current vert position and the next by the specified
	// interpolation factor
	for ( uint32 k = 0; k < m_FrameVertices; k++ )
	{		
		D3DXVec3Lerp( &pVertices[k].v, 
					  &m_pVertices[CurrentFrame + k], 
					  &m_pVertices[NextFrame + k], 
					  Interpolation );

		D3DXVec3Lerp( &pVertices[k].n, 
					  &m_pNormals[CurrentFrame + k], 
					  &m_pNormals[NextFrame + k], 
					  Interpolation );
	}

	m_pVB->Unlock();
} // CAnimatedSubMesh::UpdateVB()

// update the VB to reflect the current state of the verties
void CAnimatedSubMesh::UpdateVB( const std::deque < CActiveAnimation* >& ActiveAnimations ) // animations to blend
{
	if( ActiveAnimations.size() == 0 )
	{
		return;
	}
	if( ActiveAnimations.size() == 1)
	{
		UpdateVB( ActiveAnimations[0]->CurrentFrame(),
				  ActiveAnimations[0]->NextFrame(),
				  ActiveAnimations[0]->Interpolation() );

		return;
	}

	uint32 Animations = ActiveAnimations.size();
	uint32 StartAnimation = 0;

	for( uint32 k = 0; k < Animations; k++ )
	{
		if( !ActiveAnimations[k]->QueuedToBlend() ) 
		{
			StartAnimation = k;
			break;
		}
	}

	if( StartAnimation == Animations-1 )
	{
		UpdateVB( ActiveAnimations[StartAnimation]->CurrentFrame(),
				  ActiveAnimations[StartAnimation]->NextFrame(),
				  ActiveAnimations[StartAnimation]->Interpolation() );

		return;
	}

	// make sure to set to an impossible frame, so that any non-blended motions
	// do not think that they have their VB set cached
	m_CurrentFrame = -1;

	uint32*	CurrentFrameOffset = new uint32[Animations];
	uint32*	NextFrameOffset = new uint32[Animations];
	float32* FrameBlend = new float32[Animations];
	float32* FrameInterpolation = new float32[Animations];
	int frame = ActiveAnimations[StartAnimation]->CurrentFrame();
	int frame2= ActiveAnimations[StartAnimation]->NextFrame();
	// we want to calculate our frame blending outside of the vertex update,
	CurrentFrameOffset[StartAnimation] = ActiveAnimations[StartAnimation]->CurrentFrame() * m_FrameVertices;
	NextFrameOffset[StartAnimation] = ActiveAnimations[StartAnimation]->NextFrame() * m_FrameVertices;
	FrameBlend[StartAnimation] = ActiveAnimations[StartAnimation]->BlendPercentage();
	FrameInterpolation[StartAnimation] = ActiveAnimations[StartAnimation]->Interpolation();
	float32 CurrentBlend = ActiveAnimations[StartAnimation]->BlendPercentage();
	for( uint32 step = StartAnimation+1; step < Animations; step++ )
	{
			CurrentFrameOffset[step] = ActiveAnimations[step]->CurrentFrame() * m_FrameVertices;
			NextFrameOffset[step] = ActiveAnimations[step]->NextFrame() * m_FrameVertices;
			FrameBlend[step] = CurrentBlend;
			FrameInterpolation[step] = ActiveAnimations[step]->Interpolation();
			CurrentBlend *= ActiveAnimations[step]->BlendPercentage();
	}

	D3DXVECTOR3 FirstVertex;
	D3DXVECTOR3 FirstNormal;
	D3DXVECTOR3 FinalVertex;
	D3DXVECTOR3 FinalNormal;


	PLAINVERTEXMODEL*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}


	// for each frame, lerp between the current vert position and the next by the specified
	// interpolation factor
	for( int32 k = 0; k < m_FrameVertices; k++ )
	{		

				
		D3DXVec3Lerp( &FirstVertex, 
					  &m_pVertices[CurrentFrameOffset[StartAnimation] + k], 
					  &m_pVertices[NextFrameOffset[StartAnimation] + k], 
					  FrameInterpolation[StartAnimation] );

		D3DXVec3Lerp( &FirstNormal, 
					  &m_pNormals[CurrentFrameOffset[StartAnimation] + k], 
					  &m_pNormals[NextFrameOffset[StartAnimation] + k], 
					  FrameInterpolation[StartAnimation] );

		for( uint32 step = StartAnimation+1; step < Animations; step++ )
		{

			D3DXVec3Lerp( &FinalVertex, 
						  &m_pVertices[CurrentFrameOffset[step] + k], 
						  &m_pVertices[NextFrameOffset[step] + k], 
						  FrameInterpolation[step] );
			
			D3DXVec3Lerp( &FinalNormal, 
						  &m_pNormals[CurrentFrameOffset[step] + k], 
						  &m_pNormals[NextFrameOffset[step] + k], 
						  FrameInterpolation[step] );

			D3DXVec3Lerp( &FirstVertex, 
						  &FirstVertex, 
						  &FinalVertex, 
						  FrameBlend[step] );

			D3DXVec3Lerp( &FirstNormal, 
						  &FirstNormal, 
						  &FinalNormal, 
						  FrameBlend[step] );
		}
		pVertices[k].v = FirstVertex;
		pVertices[k].n = FirstNormal;
	
	}

	m_pVB->Unlock();

	DELETE_SAFELY( CurrentFrameOffset );
	DELETE_SAFELY( NextFrameOffset );
	DELETE_SAFELY( FrameBlend );
	DELETE_SAFELY( FrameInterpolation );


} // CAnimatedSubMesh::UpdateVB()

// resize the index buffer to make sure it matches the number of indices
void CAnimatedSubMesh::CleanIB( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
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
						 m_FrameVertices,
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
	hr = m_pVB->Lock( 0, m_FrameVertices * sizeof( PLAINVERTEXMODEL ), ( BYTE ** ) &pVertices, NULL );
	if ( FAILED( hr ) )
	{
		return;
	}	

	// copy the original vb into the optimization mesh
	memcpy( pOptimizeVertices, pVertices, m_FrameVertices * sizeof( PLAINVERTEXMODEL ) );
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
	for( uint32 i = 0; i < m_pAnimatedMeshSegments.size(); i++ )
	{
		// for the range of indices, set the attribute buffer to this segment's index
		for( uint32 j = 0; 
			j < m_pAnimatedMeshSegments[i]->TotalFaces(); 
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

	D3DXATTRIBUTERANGE* pAttributeTable = new D3DXATTRIBUTERANGE[m_pAnimatedMeshSegments.size()];	
	DWORD				AttributeCount( 0 );
	MeshToOptimize->GetAttributeTable( pAttributeTable, &AttributeCount );

	for( int32 i = 0; i < m_pAnimatedMeshSegments.size(); i++ )
	{
		m_pAnimatedMeshSegments[pAttributeTable[i].AttribId]->SetIBIndex( pAttributeTable[i].FaceStart * 3 );
		m_pAnimatedMeshSegments[pAttributeTable[i].AttribId]->SetTotalFaces( pAttributeTable[i].FaceCount );
		m_pAnimatedMeshSegments[pAttributeTable[i].AttribId]->SetFirstVertIndex( pAttributeTable[i].VertexStart );
		m_pAnimatedMeshSegments[pAttributeTable[i].AttribId]->SetLastVertIndex( pAttributeTable[i].VertexStart + pAttributeTable[i].VertexCount - 1 );
	}

	MeshToOptimize->UnlockAttributeBuffer();


	// how many frames in the anim?
	uint32 Frames = m_TotalVertices / m_FrameVertices;
	uint32 NewVertexCount = MeshToOptimize->GetNumVertices();


	std::vector <D3DXVECTOR3>		Vertices( NewVertexCount * Frames ) ;	
	std::vector <D3DXVECTOR3>		Normals( NewVertexCount * Frames );	

	DWORD* VertexIndex = ( DWORD* )VertexMap->GetBufferPointer();
	for( uint32 m = 0; m < NewVertexCount ; m++ )
	{
		uint32 index = VertexIndex[m];
		for( uint32 f = 0; f < Frames; f++ )
		{
			Vertices[f*NewVertexCount+m] = m_pVertices[f*m_FrameVertices+index];
			Normals[f*NewVertexCount+m] = m_pNormals[f*m_FrameVertices+index];
		}
	}
	m_pVertices.clear();
	m_pNormals.clear();

	m_pVertices = Vertices;
	m_pNormals = Normals;

	m_FrameVertices = NewVertexCount;
	DELETE_SAFELY( pAdjacency );
	DELETE_SAFELY( pAttributeTable );
	RELEASE_SAFELY( MeshToOptimize );
	RELEASE_SAFELY( VertexMap );
} // CAnimatedSubMesh::CleanIB()

// add a new mesh segment - it will contain an index buffer to the given material ID
uint32 CAnimatedSubMesh::AddAnimatedMeshSegment( CRefManager& pRefManager,					// reference to refmanager
												 CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
												 LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
												 std::vector <D3DXVECTOR4>& Triangles,		// vector of triangle indices
												 uint32 MaterialID,							// material id
												 int32 MaterialType,						// material type
												 bool DoubleSided )							// double sided material
{
	CAnimatedMeshSegment*	Segment	= new CAnimatedMeshSegment( pRefManager, pSettings );

	m_pAnimatedMeshSegments.push_back( Segment );

	Segment->GenerateMesh( m_pIB,
						   m_CurrentIndex,
						   Triangles,
						   MaterialID,
						   MaterialType,
						   DoubleSided );

	m_TotalFaces +=	Segment->TotalFaces();

	return ( m_pAnimatedMeshSegments.size() - 1 );
} // CAnimatedSubMesh::AddModelSegment()

void CAnimatedSubMesh::SetMaterial( CMaterial* pMaterial,			// material to apply
									uint32 SubSegment )				// subsegment to apply to
{
	if ( SubSegment < m_pAnimatedMeshSegments.size() )
	{
		m_pAnimatedMeshSegments[SubSegment]->SetMaterial( pMaterial );
	}
} // CAnimatedSubMesh::SetMaterial()

void CAnimatedSubMesh::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device

{
	for( uint32 i = 0; i < m_pAnimatedMeshSegments.size(); i++ )
	{
		m_pAnimatedMeshSegments[i]->RenderNoTexture( pD3DDevice );
	}
} // CAnimatedSubMesh::RenderNoTexture()

// use the vb of the submodel to build the shadow (slowest)
void CAnimatedSubMesh::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
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
} // CAnimatedSubMesh::RenderShadows()

void CAnimatedSubMesh::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
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
} // CAnimatedSubMesh::RenderShadow()

void CAnimatedSubMesh::SetVisible( uint32 SubSegment,			// subsegment to apply to
								   bool State )				// state to set
{
	assert( SubSegment < m_pAnimatedMeshSegments.size() );

	m_pAnimatedMeshSegments[SubSegment]->SetVisible( State );
} // CAnimatedSubMesh::SetMaterial()
