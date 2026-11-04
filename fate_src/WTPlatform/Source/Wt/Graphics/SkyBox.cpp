/*
	SkyBox.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#include "WT/stdafx.h"
#include "WT/Graphics/SkyBox.h"
#include "WT/Math/Math.h"

namespace WT
{
using namespace Math;

namespace Graphics
{

WT_DEFINE_CLASS(CSkyBox)

struct SkyboxVertex
{
	CVector3f position; // The position
	CVector3f uvw;    // The texture coordinates
};

static const DWORD SKYBOX_FVF = (D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE3(0));


SkyboxVertex Cube[] =
{
	//front face
	{ CVector3f(-1,-1,-1),CVector3f(-1,-1,-1)},
	{ CVector3f(-1,+1,-1),CVector3f(-1,+1,-1)},
	{ CVector3f(+1,+1,-1),CVector3f(+1,+1,-1)},
	{ CVector3f(+1,-1,-1),CVector3f(+1,-1,-1)},
	//back face
	{ CVector3f(-1,-1,+1),CVector3f(-1,-1,+1)},
	{ CVector3f(+1,-1,+1),CVector3f(+1,-1,+1)},
	{ CVector3f(+1,+1,+1),CVector3f(+1,+1,+1)},
	{ CVector3f(-1,+1,+1),CVector3f(-1,+1,+1)},
	//top face
	{ CVector3f(-1,+1,-1),CVector3f(-1,+1,-1)},
	{ CVector3f(-1,+1,+1),CVector3f(-1,+1,+1)},
	{ CVector3f(+1,+1,+1),CVector3f(+1,+1,+1)},
	{ CVector3f(+1,+1,-1),CVector3f(+1,+1,-1)},
	//bottom face
	{ CVector3f(-1,-1,-1),CVector3f(-1,-1,-1)},
	{ CVector3f(-1,-1,+1),CVector3f(-1,-1,+1)},
	{ CVector3f(+1,-1,+1),CVector3f(+1,-1,+1)},
	{ CVector3f(+1,-1,-1),CVector3f(+1,-1,-1)},
	//left face
	{ CVector3f(-1,-1,-1),CVector3f(-1,-1,-1)},
	{ CVector3f(-1,+1,-1),CVector3f(-1,+1,-1)},
	{ CVector3f(-1,+1,+1),CVector3f(-1,+1,+1)},
	{ CVector3f(-1,-1,+1),CVector3f(-1,-1,+1)},
	//right face
	{ CVector3f(+1,-1,-1),CVector3f(+1,-1,-1)},
	{ CVector3f(+1,-1,+1),CVector3f(+1,-1,+1)},
	{ CVector3f(+1,+1,+1),CVector3f(+1,+1,+1)},
	{ CVector3f(+1,+1,-1),CVector3f(+1,+1,-1)},

};

WORD CubeFaces[]=
{
	//front face
	0,2,1,0,3,2,
	//back face
	4,7,6,4,6,5,
	//top face
	8,10,9,8,11,10,
	//bottom face
	12,13,14,12,14,15,
	//left face
	16,17,18,16,18,19,
	//right face
	20,21,22,20,22,23
};



CSkyBox::CSkyBox() :
	m_pVB( NULL ),
	m_pIB( NULL ),
	m_pMaterial( NULL )
{
}

CSkyBox::CSkyBox( IDirect3DDevice9* pDevice , CMaterial* pMaterial	) :
	m_pVB( NULL ),
	m_pMaterial( pMaterial )
{
	InitVB( pDevice , "" );
}

CSkyBox::CSkyBox( IDirect3DDevice9* pDevice, const std::string& CubeTextureName ) :
	m_pVB( NULL ),
	m_pMaterial( NULL )
{
	InitVB( pDevice , CubeTextureName );
}

CSkyBox::~CSkyBox()	
{
	SafeRelease( m_pTexture );
	SafeRelease( m_pVB );
	SafeRelease( m_pIB );
}

void CSkyBox::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return;
	}

	if ( m_pVB == NULL )
	{
		HRESULT hr = InitVB( pDevice ,"");
		if ( FAILED(hr) )
		{
			return;
		}
	}

	// Disable the z buffer
	pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	// Disable lighting
	pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	CMatrix4f ObjectTransform;
	pDevice->SetTransform(D3DTS_WORLD,ObjectTransform);
	CMatrix4f ViewTransform(false),SavedViewTransform(false);
	pDevice->GetTransform ( D3DTS_VIEW , (D3DMATRIX *) &ViewTransform );
	SavedViewTransform = ViewTransform;
//	CMatrix4f ViewTransform = m_Camera.GetWorldToView();
	ViewTransform._41 = 0;
	ViewTransform._42 = 0;
	ViewTransform._43 = 1;
	pDevice->SetTransform(D3DTS_VIEW,ViewTransform);
	pDevice->SetStreamSource ( 0,m_pVB,0,sizeof(SkyboxVertex));
	pDevice->SetIndices( m_pIB);
	pDevice->SetFVF(SKYBOX_FVF);
	pDevice->SetTexture(0,m_pTexture);
	pDevice->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST , 0 , 0 , 24 , 0 , 12 );

	pDevice->SetTransform( D3DTS_VIEW, SavedViewTransform );

	// reenable the z buffer
	pDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
	pDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

	// Enable lighting
	pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

}

HRESULT CSkyBox::InitVB( IDirect3DDevice9* pDevice , const std::string& CubeTextureName )
{
	assert( m_pVB == NULL );

	m_pTexture = NULL;
	if ( CubeTextureName != "" )
		if ( D3DXCreateCubeTextureFromFile ( pDevice , CubeTextureName.c_str() , &m_pTexture ) != D3D_OK )
			return E_FAIL;

	pDevice->CreateVertexBuffer ( 24*sizeof(SkyboxVertex) , 0, SKYBOX_FVF, D3DPOOL_MANAGED, &m_pVB, NULL );
	pDevice->CreateIndexBuffer  ( 36*sizeof(WORD) , 0, D3DFMT_INDEX16 , D3DPOOL_MANAGED, &m_pIB, NULL );

	SkyboxVertex* v = NULL;
	m_pVB->Lock ( 0,0,(void **)&v,0);
	memcpy ( v , Cube , sizeof ( Cube ));
	m_pVB->Unlock();

	WORD *f = NULL;
	m_pIB->Lock ( 0,0,(void**) &f,0);
	memcpy ( f , CubeFaces , sizeof ( CubeFaces ));
	m_pIB->Unlock();

	return S_OK;
}

}
}