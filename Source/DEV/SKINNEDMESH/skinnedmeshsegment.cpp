/*
	SkinnedMeshSegment.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004
*/


#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/settings.h"

#include "SkinnedSubMesh.h"
#include "SkinnedMeshSegment.h"

#include "../MEDIA/refmanager.h"

#include "../MEDIA/material.h"


CSkinnedMeshSegment::CSkinnedMeshSegment( CRefManager& pRefManager,	// reference to refmanager
			 								CSettings& pSettings ):		// pointer to the settings manager, for keeping track of card/machine capabilities
																	m_pRefManager( pRefManager ),
																	m_pSettings( pSettings ),
																	m_Visible( kTrue ),
																	m_pMaterial( NULL ),
																	m_TotalFaces( 0 ),
																	m_FirstIndex( 0 ),
																	m_LastIndex( 0 ),
																	m_IBIndex( 0 ),
																	m_MaterialID( 0 ),
																	m_MaterialType( 0 )
{
} // CSkinnedMeshSegment::CSkinnedMeshSegment()


CSkinnedMeshSegment::~CSkinnedMeshSegment( void )
{

	// remove our material reference
	if ( m_pMaterial != NULL )
	{
		m_pRefManager.RemoveMaterial( m_pMaterial );
	}


} // CSkinnedMeshSegment::~CSkinnedMeshSegment()


void CSkinnedMeshSegment::RemoveMaterial( void )
{
	if ( m_pMaterial != NULL )
	{
		m_pRefManager.RemoveMaterial( m_pMaterial ); 
		m_pMaterial = NULL;
	}
} // CSkinnedMeshSegment::~RemoveMaterial()


void CSkinnedMeshSegment::SetMaterial( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
								    std::string TexName,			// path to texture
								    bool Colorkey )					// is texture colorkeyed?
{
	m_pMaterial = m_pRefManager.AddMaterial( pD3DDevice,
												  TexName,
												  Colorkey );
} // CSkinnedMeshSegment::SetMaterial()

void CSkinnedMeshSegment::SetMaterial( CMaterial* pMaterial )			// material to add
{
	if ( pMaterial != NULL )
	{
		m_pMaterial = m_pRefManager.AddMaterial( pMaterial );
	}
} // CSkinnedMeshSegment::SetMaterial()

// given lists of Vertices and Triangles, generate the Vertex and index buffers of this sub-model
// but only uses the faces of the specified material ID
void CSkinnedMeshSegment::GenerateMesh( LPDIRECT3DINDEXBUFFER8 pIB,			// buffer to add indices to
										 uint32& CurrentIndex,					// index within the buffer
										 std::vector <D3DXVECTOR4>& Triangles,	// list of Triangles indices for the mesh
										 uint32 MaterialID,						// global MaterialID id for this submesh
										 int32 MaterialType,					// type of MaterialID
										 bool DoubleSided )						// double sided texture?
{
	HRESULT			hr;

	m_MaterialID = MaterialID;
	m_MaterialType = MaterialType;

	m_IBIndex = CurrentIndex;

	uint32	Faces	= 0;
	m_FirstIndex = Triangles.size() * 3;
	m_LastIndex = 0;

	// count how many faces of the currect material id are in this triangle list
	for ( uint32 j = 0; j < Triangles.size(); j++ )
	{
		if ( ( uint32 ) ( Triangles[j].w ) == MaterialID )
		{
			Faces++;

			// we need to find out the first indexed vert and last indexed vert,
			// so that we can grab only the parts of the vert buffer we need when rendering
			if ( ( uint32 ) Triangles[j].x < m_FirstIndex )
			{
				m_FirstIndex = ( uint32 ) Triangles[j].x;
			}
			if ( ( uint32 ) Triangles[j].y < m_FirstIndex )
			{
				m_FirstIndex = ( uint32 ) Triangles[j].y;
			}
			if ( ( uint32 ) Triangles[j].z < m_FirstIndex )
			{
				m_FirstIndex = ( uint32 ) Triangles[j].z;
			}

			if ( ( uint32 ) Triangles[j].x > m_LastIndex )
			{
				m_LastIndex = ( uint32 ) Triangles[j].x;
			}
			if ( ( uint32 ) Triangles[j].y > m_LastIndex )
			{
				m_LastIndex = ( uint32 ) Triangles[j].y;
			}
			if ( ( uint32 ) Triangles[j].z > m_LastIndex )
			{
				m_LastIndex = ( uint32 ) Triangles[j].z;
			}

		}
	}

	if( DoubleSided )
	{
		Faces *= 2;
	}


	// Create the index buffer
	WORD*	pIndices;

	// Fill the index buffer
	//first index, we need a clean slate

	if( CurrentIndex == 0 )
	{
		hr = pIB->Lock( CurrentIndex * sizeof( WORD ), Faces * 3 * sizeof( WORD ), ( BYTE * * ) &pIndices, 0 );
	}
	else	// otherwise we are appending
	{
		hr = pIB->Lock( CurrentIndex * sizeof( WORD ), Faces * 3 * sizeof( WORD ), ( BYTE * * ) &pIndices, 0 );
	}

	if ( FAILED( hr ) )
	{
		pIB->Unlock();
		return;
	}

	// increment our place in the IB
	CurrentIndex += Faces * 3;

	// Fill in indices

	for ( uint32 k = 0; k < Triangles.size(); k++ )
	{
		// if this face is of the correct material ID
		if ( ( int32 ) ( Triangles[k].w ) == MaterialID )
		{
			*pIndices++ = ( WORD ) Triangles[k].x;
			*pIndices++ = ( WORD ) Triangles[k].y;
			*pIndices++ = ( WORD ) Triangles[k].z;

			if( DoubleSided )
			{
				*pIndices++ = ( WORD ) Triangles[k].z;
				*pIndices++ = ( WORD ) Triangles[k].y;
				*pIndices++ = ( WORD ) Triangles[k].x;

			}

		}
	}
	m_TotalFaces = Faces;


	pIB->Unlock();
} // CSkinnedMeshSegment::GenerateMesh()

// turn off any texture transformations that may have been active for this mesh
void CSkinnedMeshSegment::UnSet( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{

	if ( m_pMaterial != NULL && m_pMaterial->IsValid() )
	{
		// turn off texture transformation if we've been rendering a shifting texture
		if ( m_pMaterial->IsAnimated( 0 ) )
		{
			bool AllowNormalMapping = m_pSettings.GetSettings( KSetNormalMapping ) > 0;
			uint32 LayerToSet = 0;
			// for normal mapping, the base layer has the normal map, not the texture
			if ( AllowNormalMapping && m_pMaterial->HasNormalMap() )
			{
				LayerToSet = 1;
			}
			pD3DDevice->SetTextureStageState( LayerToSet,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_DISABLE );
		}

		if( m_pMaterial->AdditiveReflection() )
		{
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		}
		else if( m_pMaterial->TransparentReflection() )
		{
			if( !m_pMaterial->AllowZWrite() )
			{
				pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
			}
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

		}
	}
} // CSkinnedMeshSegment::UnSet()

// renders a subsegment, and sets textures 
void CSkinnedMeshSegment::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	bool AllowNormalMapping = m_pSettings.GetSettings( KSetNormalMapping ) > 0;

	// only set the material if one exists, and it has loaded properly
	if ( m_pMaterial != NULL && m_pMaterial->IsValid() )
	{
		// set diffuse/ambient/specular material
		pD3DDevice->SetMaterial( &m_pMaterial->D3DMaterial() );

		// if there is a normal map set us up for normal-mapped rendering
		if ( AllowNormalMapping && m_pMaterial->HasNormalMap() )
		{
			pD3DDevice->SetTexture( 0, m_pMaterial->GetNormalTexture() );
			pD3DDevice->SetTexture( 2, m_pMaterial->GetTexture() );
		} // if there is a base texture,set it
		else if ( m_pMaterial->HasBaseTexture() )
		{
			pD3DDevice->SetTexture( 0, m_pMaterial->GetTexture() );
		}
		// if no base texture exists, then we just use the glow texture
		else if ( m_pMaterial->HasIllumination() )
		{
			pD3DDevice->SetTexture( 0, m_pMaterial->GetIlluminationTexture() );
		}

		if( m_pMaterial->HasCubeMap() )
		{
			pD3DDevice->SetTexture( 1, m_pMaterial->GetCubeMap()->GetTexture() );
		}
		else if( m_pMaterial->HasSphereMap() )
		{
			pD3DDevice->SetTexture( 1, m_pMaterial->GetSphereMapTexture() );
		}

		if( m_pMaterial->AdditiveReflection() )
		{
			pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

		}
		else if( m_pMaterial->TransparentReflection() )
		{
			if( !m_pMaterial->AllowZWrite() )
			{
				pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
			}
			pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
			pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		}

		// if this is a shifting material, set the texture transformation
		if ( m_pMaterial->IsAnimated( 0 ) )
		{
			uint32 LayerToSet = 0;
			// for normal mapping, the base layer has the normal map, not the texture
			if ( AllowNormalMapping && m_pMaterial->HasNormalMap() )
			{
				LayerToSet = 1;
			}
			pD3DDevice->SetTextureStageState( LayerToSet,
											  D3DTSS_TEXTURETRANSFORMFLAGS,
											  D3DTTFF_COUNT2 );

			D3DXMATRIX	matShift;
			D3DXMatrixIdentity( &matShift );

			// set the matrix using the shifting component of the material
			matShift._31 = m_pMaterial->GetU( 0 );
			matShift._32 = m_pMaterial->GetV( 0 );

			pD3DDevice->SetTransform( D3DTS_TEXTURE0, &matShift );
		}
	}

	// render the base layer
	pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
									  m_FirstIndex,
									  m_LastIndex-m_FirstIndex + 1,
									  m_IBIndex,
									  m_TotalFaces );

	// we're doing a second pass for specular on normal maps
	// couldn't cram it all into a single pass, so here we go.
	if ( AllowNormalMapping && m_pMaterial->HasNormalMap() && 
		 m_pSettings.GetSettings( KSetSpecular ) &&
		 m_pSettings.GetSettings( KSetSpecularMapping ) )
	{
		pD3DDevice->SetTexture( 0, m_pMaterial->GetNormalTexture() );


		// set blending to additive
		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
		pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );


		// if we have an alpha texture, we are going to do specular mapping as well 
		if( m_pMaterial->HasAlphaTexture() )
		{
			pD3DDevice->SetTexture( 1, m_pMaterial->GetAlphaTexture() );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );

			pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 2, D3DTSS_COLOROP, m_pSettings.GetSettings( KSetNormalMapModulate ) );
		}
		else // otherwise the specular goes straight in
		{
			pD3DDevice->SetTextureStageState( 2,
											  D3DTSS_COLOROP,
											  D3DTOP_DISABLE );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR );
			pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, m_pSettings.GetSettings( KSetNormalMapModulate ) );
		}

		pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
										  m_FirstIndex,
										  m_LastIndex-m_FirstIndex + 1,
										  m_IBIndex,
										  m_TotalFaces );

		pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );

		pD3DDevice->SetTextureStageState( 2,
										  D3DTSS_COLOROP,
										  D3DTOP_MODULATE );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, m_pSettings.GetSettings( KSetNormalMapAdditive ) );

	} 

} // CSkinnedMeshSegment::Render()


// renders a subsegment while ignoring textures - use this for batching meshes of the same type
void CSkinnedMeshSegment::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if ( m_pMaterial != NULL && m_pMaterial->IsValid() )
	{
		// set diffuse/ambient/specular material
		pD3DDevice->SetMaterial( &m_pMaterial->D3DMaterial() );
	}

	pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
									  m_FirstIndex,
									  m_LastIndex-m_FirstIndex + 1,
									  m_IBIndex,
									  m_TotalFaces );
} // CSkinnedMeshSegment::RenderNoTexture()
