/*
	submodel.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History

	Travis Baldree
	4/14/2004	-	changed return type for lights/points/paths from pointers to refs
					big changes to vert/index buffer arrangement! consolidated them
					for the entire model, and subsegments now only have an index
					to their position in the parent buffer. Minimizes vert buffer changes

	Travis Baldree
	4/20/2004		- now takes advantage of OptimizeInPlace to generate a mesh
					  that has been compacted, takes advantage of vertex cacheing,
					  and is grouped by material in the IB

	7/1/2004		- morphing was broken due to the optimization of meshes, which was reordering the verts.
					  This has now been fixed with a new vertex map - an added benefit is that the
					  set of verts which must be morphed is greatly reduced now. The plugin to export
					  MDL files was also modified to preserver ordering of verts at a slight cost in
					  export time - this is an option however, so it only need be used for morphing meshes.

*/
#include <D3D8.h>
#include <d3dx8.h>


#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include "shadowvolume.h"

#include "submodelsegment.h"
#include "submodel.h"

#include "../MEDIA/refmanager.h"

#include "../MEDIA/material.h"


CSubModel::CSubModel( void ):
		m_pVB( NULL ),
		m_pIB( NULL ),
		m_pShadowVolume( NULL ),
		m_Morphable( kFalse ),
		m_ShadowsEnabled( kFalse ),
		m_TotalFaces( 0 ),
		m_TotalVertices( 0 ),
		m_CurrentIndex( 0 )
{
	ZeroMemory( &m_MaxBounds, sizeof ( m_MaxBounds ));
	ZeroMemory( &m_MinBounds, sizeof ( m_MinBounds ));

} // CSubModel::CSubModel()

CSubModel::~CSubModel( void )
{
	InvalidateDeviceObjects();

	for( uint32 i = 0; i < m_pSubModelSegments.size(); i++ )
	{
		DELETE_SAFELY( m_pSubModelSegments[i] );
	}
	m_pSubModelSegments.clear();

	DELETE_SAFELY( m_pShadowVolume );

	RELEASE_SAFELY( m_pVB );
	RELEASE_SAFELY( m_pIB );

} // CSubModel::~CSubModel()

void CSubModel::InvalidateDeviceObjects( void )
{
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->InvalidateDeviceObjects();
	}

} // CSubModel::InvalidateDeviceObjects()

void CSubModel::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->RestoreDeviceObjects( pD3DDevice );
	}
} // CSubModel::InvalidateDeviceObjects()

void CSubModel::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if ( !m_ShadowsEnabled )
	{
		m_ShadowsEnabled = kTrue;
		m_pShadowVolume = new CShadowVolume( pD3DDevice );
	}
} // CSubModel::EnableShadows()

void CSubModel::SetMorphable( void )
{
	m_Morphable = kTrue;

	// we store the original vertex positions when we enable morphing,
	// so that we have a 'starting point', and can also return to an unmorphed state
	// later

	m_OriginalVertices.resize( m_Vertices.size() );

	std::copy( m_Vertices.begin(), m_Vertices.end(), m_OriginalVertices.begin() );
} // CSubModel::SetMorphable()

void CSubModel::Morph( CSubModel* MorphTarget,		// target mesh to morph to
					   D3DXVECTOR3 MorphPointVEC3D,	// point of 'impact' for the morph
					   float32 Radius,				// radius of impact for the morph
					   float32 Percent )			// percent to morph from start to target
{
	if ( !m_Morphable )
	{
		return;
	}

	if( MorphTarget->OriginalVertices() != m_Vertices.size() )
	{
		return;
	}

	D3DXVECTOR3	TempVEC3D;

	// calculate the cube bounds of the morph 'sphere'

	float32		minx			= MorphPointVEC3D.x - Radius;
	float32		miny			= MorphPointVEC3D.y - Radius;
	float32		minz			= MorphPointVEC3D.z - Radius;
	float32		maxx			= MorphPointVEC3D.x + Radius;
	float32		maxy			= MorphPointVEC3D.y + Radius;
	float32		maxz			= MorphPointVEC3D.z + Radius;

	float32		LengthToTarget, FullLengthToTarget, Multiplier;

	// calculate the squared radius for quicker length calculations later
	float32		RadiusSquared	= Radius* Radius;


	for ( uint32 i = 0; i < m_Vertices.size() ; i++ )
	{
		// make sure the vert is within the rough bounds
		if ( MorphTarget->m_Vertices[i].x >= minx &&
			 MorphTarget->m_Vertices[i].y >= miny &&
			 MorphTarget->m_Vertices[i].z >= minz &&
			 MorphTarget->m_Vertices[i].x <= maxx &&
			 MorphTarget->m_Vertices[i].y <= maxy &&
			 MorphTarget->m_Vertices[i].z <= maxz )
		{
			// distance from this vert to the origin of the morph
			TempVEC3D = m_OriginalVertices[i] - MorphPointVEC3D;
			// squared length
			LengthToTarget = D3DXVec3LengthSq( &TempVEC3D );

			// if it's within the radius
			if ( LengthToTarget < RadiusSquared )
			{
				// calculate a percentage of influence based on distance from the center
				Multiplier = LengthToTarget / RadiusSquared;
				if ( Multiplier > 1 )
				{
					Multiplier = 1;
				}
				Multiplier = 1 - Multiplier;


				// calculate the total length from the original unmorphed vert to the morph target
				TempVEC3D = MorphTarget->m_Vertices[i] -
							m_OriginalVertices[i];
				FullLengthToTarget = D3DXVec3LengthSq( &TempVEC3D );

				if( FullLengthToTarget != 0 )
				{
					// calculate the total length from the currently morphed vert to the morph target
					TempVEC3D = MorphTarget->m_Vertices[i] - m_Vertices[i];
					LengthToTarget = D3DXVec3LengthSq( &TempVEC3D );

					// figure out how far along we already are, and adjust the amount we need to morph
					// appropriately
					LengthToTarget /= FullLengthToTarget;
					LengthToTarget -= Percent * Multiplier;
					if ( LengthToTarget < 0 )
					{
						LengthToTarget = 0;
					}

					// perform the actual lerp to the new morphed vert position
					D3DXVec3Lerp( &m_Vertices[i],
								&MorphTarget->m_Vertices[i],
								&m_OriginalVertices[i],
								LengthToTarget );
				}
			}
		}
	}

	UpdateVB();
} // CSubModel::Morph()

// reset the mesh to its original state
void CSubModel::ResetMorph( void )
{
	if ( !m_Morphable )
	{
		return;
	}

	for ( uint32 i = 0; i < m_Vertices.size(); i++ )
	{
		m_Vertices[i] = m_OriginalVertices[i];
	}

	UpdateVB();
} // CSubModel::ResetMorph()


// update the VB to reflect the current state of the verties
void CSubModel::CreateBuffers(  LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								uint32 Faces,								// faces in the mesh
								std::vector <uint32>& VertexReferences,	// vector of vertex references
								std::vector <uint32>& UVReferences,		// vector of uv references
								std::vector <D3DXVECTOR3>& Vertices,		// vector of vertices
								std::vector <D3DXVECTOR3>& UVs,			// vector of uvs
								std::vector <uint32>& NormalReferences,		// indices of normals
								std::vector <D3DXVECTOR3>& Normals,		// vector of normals
								std::vector <D3DXVECTOR3>& VertexColors )	// vector of vertex colors
{
	std::vector <PLAINVERTEXMODEL> VBVertices ( Vertices.size() );
	m_VertexMap.clear();
	m_VertexMap.resize( Vertices.size() );

	m_Vertices.clear();
	m_Vertices = Vertices;

	for ( uint32 k = 0; k < VertexReferences.size(); k++ )
	{
		// if this vert hasn't been filled into the list yet, fill it out
		if( Vertices[VertexReferences[k]] != VBVertices[VertexReferences[k]].v )
		{
			VBVertices[VertexReferences[k]].v = Vertices[VertexReferences[k]];
			VBVertices[VertexReferences[k]].n = Normals[NormalReferences[k]];
			VBVertices[VertexReferences[k]].tu = UVs[UVReferences[k]].x;
			VBVertices[VertexReferences[k]].tv = UVs[UVReferences[k]].y;	
			// keep track of our vertex map back to the original set
			m_VertexMap[VertexReferences[k]] = VertexReferences[k];
		}
		else
		{
			// if this vertex doesn't match its index, we have to make a new vert, and give it a new index
			if( !( VBVertices[VertexReferences[k]].n == Normals[NormalReferences[k]] &&
				   VBVertices[VertexReferences[k]].tu == UVs[UVReferences[k]].x &&
				   VBVertices[VertexReferences[k]].tv == UVs[UVReferences[k]].y ) )
			{
				PLAINVERTEXMODEL NewVertex;
				NewVertex.v = Vertices[VertexReferences[k]];
				NewVertex.n = Normals[NormalReferences[k]];
				NewVertex.tu = UVs[UVReferences[k]].x;
				NewVertex.tv = UVs[UVReferences[k]].y;
				// keep track of our vertex map back to the original set
				m_VertexMap.push_back( VertexReferences[k] );

				// assign a new vertex reference
				VertexReferences[k] = VBVertices.size();
				// and put the new vert in the list
				VBVertices.push_back( NewVertex );
			}
		}
	}

	m_TotalVertices	= VBVertices.size();


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

	m_ExpandedVertices.resize( m_TotalVertices );

	for ( k = 0; k < m_TotalVertices; k++ )
	{
		*pVertices = VBVertices[k];
		m_ExpandedVertices[k] = VBVertices[k].v;

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


} // CSubModel::CreateBuffers()

// resize the index buffer to make sure it matches the number of indices
void CSubModel::CleanIB( LPDIRECT3DDEVICE8 pD3DDevice )	// d3d device
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
	hr = m_pVB->Lock( 0, 0, ( BYTE ** ) &pVertices, NULL );
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

	MeshToOptimize->LockAttributeBuffer( D3DLOCK_DISCARD, &pAttributes );

	uint32 CurrentFace( 0 );
	for( uint32 i = 0; i < m_pSubModelSegments.size(); i++ )
	{
		// for the range of indices, set the attribute buffer to this segment's index
		for( uint32 j = 0; 
			j < m_pSubModelSegments[i]->TotalFaces(); 
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



	hr	= pD3DDevice->CreateVertexBuffer( MeshToOptimize->GetNumVertices()*sizeof( PLAINVERTEXMODEL ),
										  D3DUSAGE_WRITEONLY,
										  D3DFVF_PLAINVERTEXMODEL,
										  D3DPOOL_MANAGED,
										  &m_pVB );

	MeshToOptimize->LockVertexBuffer( NULL, ( BYTE ** ) &pOptimizeVertices );

	// store our new vertex total
	m_TotalVertices = MeshToOptimize->GetNumVertices();

	m_pVB->Lock( 0, 
				 MeshToOptimize->GetNumVertices()*sizeof( PLAINVERTEXMODEL ), 
				 ( BYTE * * ) &pVertices, 
				 0 );

	// copy the original vb into the optimization mesh
	memcpy( pVertices, 
			pOptimizeVertices, 
			MeshToOptimize->GetNumVertices() * sizeof( PLAINVERTEXMODEL ) );
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

	D3DXATTRIBUTERANGE* pAttributeTable = new D3DXATTRIBUTERANGE[m_pSubModelSegments.size()];	
	DWORD				AttributeCount( 0 );
	MeshToOptimize->GetAttributeTable( pAttributeTable, &AttributeCount );

	for( i = 0; i < m_pSubModelSegments.size(); i++ )
	{
		m_pSubModelSegments[pAttributeTable[i].AttribId]->SetIBIndex( pAttributeTable[i].FaceStart * 3 );
		m_pSubModelSegments[pAttributeTable[i].AttribId]->SetTotalFaces( pAttributeTable[i].FaceCount );
		m_pSubModelSegments[pAttributeTable[i].AttribId]->SetFirstVertIndex( pAttributeTable[i].VertexStart );
		m_pSubModelSegments[pAttributeTable[i].AttribId]->SetLastVertIndex( pAttributeTable[i].VertexStart + pAttributeTable[i].VertexCount - 1 );
	}

	MeshToOptimize->UnlockAttributeBuffer();

	// now we have to update our base vertex list for use in morphing -
	// since we've optimized the mesh, the vertex count and arrangement
	// are different, and we need to apply those changes back to our raw vertices
	// via a vertex map
	uint32 NewVertexCount = MeshToOptimize->GetNumVertices();

	std::vector<uint32> NewVertexMap( NewVertexCount );

	DWORD* VertexIndex = ( DWORD* )VertexMap->GetBufferPointer();
	for( uint32 m = 0; m < NewVertexCount ; m++ )
	{
		NewVertexMap[m] = m_VertexMap[VertexIndex[m]];
	}
	m_VertexMap.clear();
	m_VertexMap = NewVertexMap;

	DELETE_SAFELY( pAdjacency );
	DELETE_SAFELY( pAttributeTable );
	RELEASE_SAFELY( MeshToOptimize );
	RELEASE_SAFELY( VertexMap );
} // CSubModel::CleanIB()

// update the VB to reflect the current state of the verties
void CSubModel::UpdateVB( void )
{
	PLAINVERTEXMODEL*	pVertices;
	if ( FAILED( m_pVB->Lock( 0, 0, ( BYTE * * ) &pVertices, NULL ) ) )
	{
		return;
	}

	for ( uint32 k = 0; k < m_TotalVertices; k++ )
	{
		pVertices->v = m_Vertices[m_VertexMap[k]];	
		pVertices++;
	}

	m_pVB->Unlock();
} // CSubModel::UpdateVB()


uint32 CSubModel::AddSubModelSegment( CRefManager& pRefManager,					// reference to refmanager
									  CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
									  LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									  std::vector <uint32>& VertexReferences,	// vector of vertex references
									  std::vector <D3DXVECTOR4>& Triangles,		// vector of triangle indices
									  uint32 MaterialID,						// material id
									  int32 MaterialType,						// material type
									  bool MaterialDoubleSided )					// is double sided?
{
	CSubModelSegment*	Segment	= new CSubModelSegment( pRefManager, pSettings );

	m_pSubModelSegments.push_back( Segment );

	Segment->GenerateMesh( m_pIB,
						   m_CurrentIndex,
						   m_ExpandedVertices,
						   Triangles,
						   VertexReferences,
						   MaterialID,
						   MaterialType,
						   MaterialDoubleSided );

	m_TotalFaces +=	Segment->TotalFaces();

	return ( m_pSubModelSegments.size() - 1 );
} // CSubModel::AddModelSegment()

void CSubModel::SetMaterial( CMaterial* pMaterial,			// material to apply
							 uint32 SubSegment )			// subsegment to apply to
{
	if ( SubSegment < m_pSubModelSegments.size() )
	{
		m_pSubModelSegments[SubSegment]->SetMaterial( pMaterial );
	}
} // CSubModel::SetMaterial()

// use a supplied face list to build the shadow volume (fastest)
void CSubModel::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							   const CFaceList& pFaceList,			// face list to build volume from
							   const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
							   const D3DXVECTOR3& Light )			// direction of light shadow is being cast from
{
	// the shadow volume must constantly be rebuild to take into account light direction
	if ( m_ShadowsEnabled )
	{
		m_pShadowVolume->BuildFromFaceList( pFaceList,
											Light );

		RenderShadows( pD3DDevice, CameraSpaceMatrix );
	}
	else
	{
		return;
	}

} // CSubModel::RenderShadows()

// use the vb of the submodel to build the shadow (slowest)
void CSubModel::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
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
} // CSubModel::RenderShadows()

void CSubModel::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
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
} // CSubModel::RenderShadow()

void CSubModel::SetVisible( uint32 SubSegment,			// subsegment to apply to
						    bool State )				// state to set
{
	assert( SubSegment < m_pSubModelSegments.size() );

	m_pSubModelSegments[SubSegment]->SetVisible( State );
} // CSubModel::SetMaterial()
