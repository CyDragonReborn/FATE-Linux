/*
	subscenesegment.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/


#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/capabilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3D.h"
#include "../UTILITIES/vertexdefinitions.h"

#include "subscene.h"
#include "subscenesegment.h"

#include "../MEDIA/refmanager.h"

#include "../MEDIA/material.h"


CSubSceneSegment::CSubSceneSegment( CRefManager& pRefManager,	// reference to refmanager
			 						CSettings& pSettings,		// pointer to the settings manager, for keeping track of card/machine capabilities
									CSubScene* pParent ) :
															m_pRefManager( pRefManager ),
															m_pSettings( pSettings ),
															m_pParent( pParent ),
															m_pMaterial( NULL ),
															m_pShadowMaterial( NULL ),
															m_Visible( kTrue ),
															m_TotalFaces( 0 ),
															m_FirstIndex( 0 ),
															m_LastIndex( 0 ),
															m_ZDistance( 0 ),
															m_SubIndex ( 0 ),
															m_CenterPoint ( 0, 0, 0 ),
															m_MaxBounds ( 0, 0, 0 ),
															m_MinBounds ( 0, 0, 0 ),
															m_IBIndex ( 0 ),
															m_MaterialID ( 0 ),
															m_MaterialType ( 0 )
{
} // CSubSceneSegment::CSubSceneSegment()


CSubSceneSegment::~CSubSceneSegment( void )
{
	m_pParent = NULL;

	// remove our material reference
	if ( m_pMaterial != NULL )
	{
		m_pRefManager.RemoveMaterial( m_pMaterial );
		m_pMaterial = NULL;
	}

	// remove our material reference
	if ( m_pShadowMaterial != NULL )
	{
		m_pRefManager.RemoveMaterial( m_pShadowMaterial );
		m_pShadowMaterial = NULL;
	}


} // CSubSceneSegment::~CSubSceneSegment()

void CSubSceneSegment::RemoveMaterial( void )
{
	if ( m_pMaterial != NULL )
	{
		m_pRefManager.RemoveMaterial( m_pMaterial ); 
		m_pMaterial = NULL;
	}
	if ( m_pShadowMaterial != NULL )
	{
		m_pRefManager.RemoveMaterial( m_pShadowMaterial ); 
		m_pShadowMaterial = NULL;
	}
} // CSubSceneSegment::~RemoveMaterial()

void CSubSceneSegment::SetMaterial( CMaterial* pMaterial )			// material to add
{
	if ( pMaterial != NULL )
	{
		m_pMaterial = m_pRefManager.AddMaterial( pMaterial );
	}
} // CSubSceneSegment::SetMaterial()

// set the shadow mapping material
void CSubSceneSegment::SetShadowMaterial( CMaterial* pMaterial )			// material to add
{
	if ( pMaterial != NULL )
	{
		m_pShadowMaterial = m_pRefManager.AddMaterial( pMaterial );
	}
} // CSubSceneSegment::SetShadowMaterial()

// given lists of Vertices and Triangles, generate the Vertex and index buffers of this sub-model
// but only uses the faces of the specified material ID
void CSubSceneSegment::GenerateMesh( LPDIRECT3DINDEXBUFFER8 pIB,			// buffer to add indices to
									 uint32& CurrentIndex,					// index within the buffer
									 std::vector <D3DXVECTOR3>& Vertices,	// list of Vertices in the mesh
									 std::vector <D3DXVECTOR4>& Triangles,	// list of Triangles indices for the mesh
									 std::vector <uint32>& VertexIndices,	// reference map of Triangle indices, to actual Vertices
									 uint32 MaterialID,						// global MaterialID id for this submesh
									 int32 MaterialType,					// type of MaterialID
									 bool DoubleSided )						// is the mesh double-sided?
{
	HRESULT			hr;


	m_MaterialID = MaterialID;
	m_MaterialType = MaterialType;

	m_IBIndex = CurrentIndex;

	uint32	Faces( 0 );
	m_FirstIndex = Vertices.size();
	m_LastIndex = 0;
	bool	FoundFirstFace( kFalse );

	// count how many faces of the currect material id are in this triangle list
	for ( uint32 j = 0; j < Triangles.size(); j++ )
	{
		if ( ( uint32 ) ( Triangles[j].w ) == MaterialID )
		{
			Faces++;
			// we need to find out the first indexed vert and last indexed vert,
			// so that we can grab only the parts of the vert buffer we need when rendering
			if ( VertexIndices[( int32 ) Triangles[j].x] < m_FirstIndex )
			{
				m_FirstIndex = VertexIndices[( int32 ) Triangles[j].x];
			}
			if ( VertexIndices[( int32 ) Triangles[j].y] < m_FirstIndex )
			{
				m_FirstIndex = VertexIndices[( int32 ) Triangles[j].y];
			}
			if ( VertexIndices[( int32 ) Triangles[j].z] < m_FirstIndex )
			{
				m_FirstIndex = VertexIndices[( int32 ) Triangles[j].z];
			}

			if ( VertexIndices[( int32 ) Triangles[j].x] > m_LastIndex )
			{
				m_LastIndex = VertexIndices[( int32 ) Triangles[j].x];
			}
			if ( VertexIndices[( int32 ) Triangles[j].y] > m_LastIndex )
			{
				m_LastIndex = VertexIndices[( int32 ) Triangles[j].y];
			}
			if ( VertexIndices[( int32 ) Triangles[j].z] > m_LastIndex )
			{
				m_LastIndex = VertexIndices[( int32 ) Triangles[j].z];
			}
			// start our min/max Bounds for this mesh at the first vertex added
			if ( !FoundFirstFace )
			{
				m_MinBounds = Vertices[VertexIndices[( int32 ) Triangles[j].x]];
				m_MaxBounds = Vertices[VertexIndices[( int32 ) Triangles[j].x]];
				FoundFirstFace = kTrue;
			}

			ExpandBounds( m_MinBounds, 
						  m_MaxBounds, 
						  Vertices[VertexIndices[( int32 ) Triangles[j].x]] );

			ExpandBounds( m_MinBounds, 
						  m_MaxBounds, 
						  Vertices[VertexIndices[( int32 ) Triangles[j].y]] );

			ExpandBounds( m_MinBounds, 
						  m_MaxBounds, 
						  Vertices[VertexIndices[( int32 ) Triangles[j].z]] );
		}
	}


	// Create the index buffer
	WORD*	pIndices;

	// double the size of the index buffer for double-sided meshes
	if ( DoubleSided )
	{
		Faces *= 2;
	}

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
			*pIndices++ = ( WORD )
						  VertexIndices[( int32 )Triangles[k].x];
			*pIndices++ = ( WORD )
						  VertexIndices[( int32 )Triangles[k].y];
			*pIndices++ = ( WORD )
						  VertexIndices[( int32 )Triangles[k].z];
			// for double sided meshes, add another face with the reverse winding
			if ( DoubleSided )
			{
				*pIndices++ = ( WORD )
							  VertexIndices[( int32 )Triangles[k].z];
				*pIndices++ = ( WORD )
							  VertexIndices[( int32 )Triangles[k].y];
				*pIndices++ = ( WORD )
							  VertexIndices[( int32 )Triangles[k].x];
			}
		}
	}
	m_TotalFaces = Faces;


	pIB->Unlock();

	CalculateBounds();
} // CSubSceneSegment::GenerateMesh()

// set the stream and indices for this submesh segment
void CSubSceneSegment::SetStream( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	pD3DDevice->SetStreamSource( 0,
								 m_pParent->GetVB(),
								 m_pParent->VertexSize() );

	pD3DDevice->SetIndices( m_pParent->GetIB(), 0L );

} // CSubSceneSegment::SetStream()

// renders a subsegment, and sets textures 
void CSubSceneSegment::SetShadowMap( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if( m_pSettings.Capabilities().GetCapabilityValue( KValueMaxSimultaneousBlendStages ) < 3 &&
		( m_pMaterial != NULL && 
			( m_pMaterial->HasReflection() || 
			  m_pMaterial->HasSphereMap() ) ) )
	{
		return;
	}

	// only set the material if one exists, and it has loaded properly
	if ( m_pMaterial != NULL && m_pMaterial->IsValid() &&
		 m_pShadowMaterial != NULL && m_pShadowMaterial->IsValid() )
	{
		bool AllowNormalMapping = m_pSettings.GetSettings( KSetNormalMapping ) > 0;
		uint32 ShadowLayer = 1;

		// check these states to see if layer 2 is already occupied
		if ( AllowNormalMapping && m_pMaterial->HasNormalMap() )
		{
			ShadowLayer = 2;
		} 
		if( m_pMaterial->HasSphereMap() )
		{

			ShadowLayer = 2;
		}
		else if( m_pMaterial->HasReflection() && 
				m_pMaterial->IsDynamicTextureValid() )
		{
			ShadowLayer = 2;
		}

		pD3DDevice->SetTextureStageState( ShadowLayer, D3DTSS_TEXCOORDINDEX, 1 );
		pD3DDevice->SetTexture( ShadowLayer, m_pShadowMaterial->GetTexture() );
		pD3DDevice->SetTextureStageState( ShadowLayer, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( ShadowLayer,
											D3DTSS_COLOROP,
											D3DTOP_MODULATE2X );
	}

} // CSubSceneSegment::SetShadowMap()

// turn off any texture transformations that may have been active for this mesh
void CSubSceneSegment::UnSetShadowMap( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if( m_pSettings.Capabilities().GetCapabilityValue( KValueMaxSimultaneousBlendStages ) < 3 &&
		( m_pMaterial != NULL && 
			( m_pMaterial->HasReflection() || 
			  m_pMaterial->HasSphereMap() ) ) )
	{
		return;
	}

	// only set the material if one exists, and it has loaded properly
	if ( m_pMaterial != NULL && m_pMaterial->IsValid() &&
		 m_pShadowMaterial != NULL && m_pShadowMaterial->IsValid() )
	{

		bool AllowNormalMapping = m_pSettings.GetSettings( KSetNormalMapping ) > 0;
		uint32 ShadowLayer = 1;

		if ( AllowNormalMapping && 
			m_pMaterial != NULL && 
			m_pMaterial->HasNormalMap() )
		{
			ShadowLayer = 2;
		} 
		else if ( m_pMaterial != NULL && 
				( m_pMaterial->HasReflection() || 
					m_pMaterial->HasSphereMap() ) )
		{
			ShadowLayer = 2;
		}

		// turn off the shadow map
		if( m_pShadowMaterial != NULL && m_pShadowMaterial->IsValid() )
		{
			pD3DDevice->SetTextureStageState( ShadowLayer, D3DTSS_TEXCOORDINDEX, 0 );
			pD3DDevice->SetTextureStageState( ShadowLayer,
											D3DTSS_COLOROP,
											D3DTOP_DISABLE );
		}
	}

} // CSubSceneSegment::UnSetShadowMap()


// renders a subsegment, and sets textures 
void CSubSceneSegment::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	bool AllowNormalMapping = m_pSettings.GetSettings( KSetNormalMapping ) > 0;


	// only set the material if one exists, and it has loaded properly
	if ( m_pMaterial != NULL && m_pMaterial->IsValid() )
	{
		// if there is a normal map set us up for normal-mapped rendering
		if ( AllowNormalMapping && m_pMaterial->HasNormalMap() )
		{
			pD3DDevice->SetTexture( 0, m_pMaterial->GetNormalTexture() );
			pD3DDevice->SetTexture( 1, m_pMaterial->GetTexture() );
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

		if( m_pMaterial->HasSphereMap() )
		{

			pD3DDevice->SetTexture( 1, m_pMaterial->GetSphereMapTexture() );
		}
		else if( m_pMaterial->HasReflection() && 
				 m_pMaterial->IsDynamicTextureValid() )
		{

			pD3DDevice->SetTexture( 1, m_pMaterial->GetDynamicTexture() );
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

			if( LayerToSet == 0 )
			{
				pD3DDevice->SetTransform( D3DTS_TEXTURE0, &matShift );
			}
			else
			{
				pD3DDevice->SetTransform( D3DTS_TEXTURE1, &matShift );
			}
		}
	}

	// render the base layer
	pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
									  m_FirstIndex,
									  m_LastIndex-m_FirstIndex + 1,
									  m_IBIndex,
									  m_TotalFaces );


} // CSubSceneSegment::Render()


// turn off any texture transformations that may have been active for this mesh
void CSubSceneSegment::UnSet( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	bool AllowNormalMapping = m_pSettings.GetSettings( KSetNormalMapping ) > 0;

	if ( m_pMaterial != NULL && 
			  ( m_pMaterial->HasReflection() || 
			    m_pMaterial->HasSphereMap() ) )
	{
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

	if ( m_pMaterial != NULL && m_pMaterial->IsValid() )
	{
		// turn off texture transformation if we've been rendering a shifting texture
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
											  D3DTTFF_DISABLE );
		}
	}

} // CSubSceneSegment::UnSet()



// renders a subsegment while ignoring textures - use this for batching meshes of the same type
void CSubSceneSegment::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{

	pD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST,
									  m_FirstIndex,
									  m_LastIndex-m_FirstIndex + 1,
									  m_IBIndex,
									  m_TotalFaces );
} // CSubSceneSegment::RenderNoTexture()
