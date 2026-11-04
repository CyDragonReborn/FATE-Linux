/*
	tread.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/24/2004

*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../utilities/macros.h"
#include "../utilities/constants.h"
#include "../utilities/utilities.h"
#include "../utilities/vertexdefinitions.h"
#include "../utilities/cullingbounds.h"
#include "../utilities/frustum.h"

#include "tread.h"


CTread::CTread( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
			    const D3DXVECTOR3& Position,	// ground contact position of wheel
				const D3DXMATRIX& Orientation,	// orientation of wheel
				ETreadType Type,				// type of tread
				float32 Width ) :				// width of the tread
				m_Width( Width ),
				m_Type( Type ),
				m_Position( Position ),
				m_CurrentIndex( 0 ),
				m_CurrentTime( 0 ),
				m_Active( kTrue ),
				m_pVB( NULL ),
				m_pCullingBounds( NULL )
				

{
	m_MinBounds = D3DXVECTOR3( 0, 0, 0 );
	m_MaxBounds = D3DXVECTOR3( 0, 0, 0 );

	m_pCullingBounds = new CCullingBounds( m_MinBounds,
										   m_MaxBounds );

	// calculate a base transformation matrix, assuming identity orientation
	D3DXMatrixTranslation( &m_TransformedMatrix,
						   Position.x,
						   Position.y,
						   Position.z );

	CreateVB( pD3DDevice );


	Update( pD3DDevice,
			Position,
			Orientation,
			KTimePerSegment );
} // CTread::CTread()

CTread::~CTread()
{
	DELETE_SAFELY( m_pCullingBounds );

	RELEASE_SAFELY( m_pVB );
} // CTread::~CTread()

bool CTread::CreateVB( LPDIRECT3DDEVICE8 pD3DDevice )		// our direct3d device
{
	HRESULT hr;

	// set up the vert buffer 
	uint32 Points = KTreadLength + 1;

    hr = pD3DDevice->CreateVertexBuffer( Points * sizeof(PLAINVERTEXSCENE) * 2,
										 D3DUSAGE_WRITEONLY, 
										 D3DFVF_PLAINVERTEXSCENE,
										 D3DPOOL_MANAGED, 
										 &m_pVB );
	
	if( FAILED(hr) )
	{
		return kFalse;
	}
	return kTrue;
} // CTread::CreateVB()

void CTread::Render( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
					 const D3DXMATRIX& ViewportMatrix,	// viewport matrix
					 CFrustum* pFrustum )				// frustum to cull with - NULL for none
{
	if( m_CurrentIndex == 0 )
	{
		return;
	}

	uint32 Triangles = m_CurrentIndex;
	if( Triangles > KTreadLength )
	{
		Triangles = KTreadLength;
	}

	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

	D3DXMATRIX Transformation = m_TransformedMatrix * ViewportMatrix;

	pD3DDevice->SetVertexShader( D3DFVF_PLAINVERTEXSCENE );

	pD3DDevice->SetTransform( D3DTS_VIEW, &Transformation );

	pD3DDevice->SetStreamSource( 0, m_pVB, sizeof( PLAINVERTEXSCENE ) );

	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, Triangles * 2 );

	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,  0, Triangles * 2 );

	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
} // CTread::Render()

void CTread::Update( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
					 const D3DXVECTOR3& Position,	// ground contact position of wheel
					 const D3DXMATRIX& Orientation, // orientation of wheel
					 float32 TimeElapsed )			// time elapsed in seconds
{

	if( !m_Active )
	{
		return;
	}

	m_CurrentTime += TimeElapsed;

	if( m_CurrentTime > KTimePerSegment )
	{
		m_CurrentIndex++;
		m_CurrentTime = 0;
	}

	// we don't update when we've gone beyond the end of the tread
	if( m_CurrentIndex > KTreadLength )
	{
		m_CurrentIndex = KTreadLength;
		m_Active = kFalse;
	}

	PLAINVERTEXSCENE* pVertices = NULL ;
	if( FAILED( m_pVB->Lock( 0, 0, (BYTE**)&pVertices, NULL ) ) )
	{
		return;
	}

	float32 TreadUWidth = 1.0f / (float32)( KTreadTypes - 1 );

	D3DXVECTOR3 Perpendicular = D3DXVECTOR3( Orientation._11, Orientation._12, Orientation._13 );
	D3DXVECTOR3	Up = D3DXVECTOR3( Orientation._21, Orientation._22, Orientation._23 ) * KNormalBias;
	Perpendicular *= m_Width * .5f;

	// keep our mesh bounds up to date
	D3DXVECTOR3 VertexPosition = Position - Perpendicular + Up - m_Position;
	ExpandBounds( m_MinBounds, m_MaxBounds, VertexPosition );

	pVertices[m_CurrentIndex * 2].v			= VertexPosition;
	pVertices[m_CurrentIndex * 2].tu		= TreadUWidth * (float32)( m_Type - 1 );
	pVertices[m_CurrentIndex * 2].tv		= (float32)m_CurrentIndex;
	pVertices[m_CurrentIndex * 2].color		= D3DXCOLOR( 1, 1, 1, 1 );

	// keep our mesh bounds up to date
	VertexPosition = Position + Perpendicular + Up - m_Position;
	ExpandBounds( m_MinBounds, m_MaxBounds, VertexPosition );

	pVertices[m_CurrentIndex * 2 + 1].v			= VertexPosition;
	pVertices[m_CurrentIndex * 2 + 1].tu		= TreadUWidth * (float32)( m_Type - 1 ) + TreadUWidth;
	pVertices[m_CurrentIndex * 2 + 1].tv		= (float32)m_CurrentIndex;
	pVertices[m_CurrentIndex * 2 + 1].color		= D3DXCOLOR( 1, 1, 1, 1 );

	m_pVB->Unlock();	

	// update culling bounds so we don't have to render offscreen
	m_pCullingBounds->SetLocalMinBounds( m_MinBounds );
	m_pCullingBounds->SetLocalMaxBounds( m_MaxBounds );
	m_pCullingBounds->Update( m_TransformedMatrix );

} // CTread::Update()

