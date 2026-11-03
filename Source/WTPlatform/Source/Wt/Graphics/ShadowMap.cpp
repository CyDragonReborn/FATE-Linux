/***************************************************************************************
 *
 * ShadowMap.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/24/2006: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Graphics/ShadowMap.h"
#include "WT/Graphics/Level.h"

//#define LOAD_SHADERS
//#define USE_CONVEX_HULLS
//#define USE_RECEIVER_EXTRUSION

/*
TODO:
Better convex hull representing relevant combination of shadow caster / receiver / eye frustum volumes
PCF / blurring techniques
Convert pixel shaders from 2_0 to 1_4 (could be difficult and buggy, low payoff)
*/

using namespace WT::System;
using namespace WT::System::Serialization;
using namespace WT::Math;
using namespace WT::Graphics;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CShadowMap )
WT_DEFINE_CLASS( CShadowMapDirectional )

CShadowMap::SharedDataMap CShadowMap::gm_SharedDataMap;
CShadowMapDirectional::SharedDataMap CShadowMapDirectional::gm_SharedDataMap;

#ifdef LOAD_SHADERS

//-----------------------------------------------------------------------------
// Name: LoadVertexShader()
// Desc: Creates and loads vertex shader from file
//-----------------------------------------------------------------------------
void LoadVertexShader(LPDIRECT3DDEVICE9 pd3dDevice, TCHAR* fileName, LPDIRECT3DVERTEXSHADER9* pShader)
{
	LPD3DXBUFFER pShaderBuf = NULL;
    LPD3DXBUFFER pErrorBuf = NULL;

	HRESULT hr = D3DXAssembleShaderFromFile(fileName, NULL, NULL, 0, &pShaderBuf, &pErrorBuf);
	if (!SUCCEEDED( hr ) || pShaderBuf == NULL)
	{
		throw std::runtime_error( "D3DXAssembleShaderFromFile failure" );
	}
		
	hr = pd3dDevice->CreateVertexShader( (DWORD*)pShaderBuf->GetBufferPointer(), pShader );
	if (!SUCCEEDED( hr ))
	{
		throw std::runtime_error( "CreateVertexShader failure" );
	}

	SafeRelease( pShaderBuf );
	SafeRelease( pErrorBuf );
}

//-----------------------------------------------------------------------------
// Name: LoadPixelShader()
// Desc: Creates and loads pixel shader from file
//-----------------------------------------------------------------------------
void LoadPixelShader(LPDIRECT3DDEVICE9 pd3dDevice, TCHAR* fileName, LPDIRECT3DPIXELSHADER9* pShader)
{
	LPD3DXBUFFER pShaderBuf = NULL;
    LPD3DXBUFFER pErrorBuf = NULL;

	HRESULT hr = D3DXAssembleShaderFromFile(fileName, NULL, NULL, 0, &pShaderBuf, &pErrorBuf);
	if (!SUCCEEDED( hr ) || pShaderBuf == NULL)
	{
		throw std::runtime_error( "D3DXAssembleShaderFromFile failure" );
	}
		
	hr = pd3dDevice->CreatePixelShader( (DWORD*)pShaderBuf->GetBufferPointer(), pShader );
	if (!SUCCEEDED( hr ))
	{
		throw std::runtime_error( "CreatePixelShader failure" );
	}

	SafeRelease( pShaderBuf );
	SafeRelease( pErrorBuf );
}

#endif

CShadowMap::SharedData::SharedData( LPDIRECT3DDEVICE9 pDevice ) 
	: m_RefCount( 0 )
	, m_pDevice( pDevice )
{
	D3DCAPS9 Caps;
	HRESULT Result = m_pDevice->GetDeviceCaps( &Caps );

	if (!SUCCEEDED( Result ))
	{
		throw std::runtime_error( "GetDeviceCaps() failure" );
	}

	if ((Caps.VertexShaderVersion & 0xffffu) < 0x0101)
	{
		throw std::runtime_error( "vertex shader 1.1 required" );
	}

#ifdef LOAD_SHADERS
	LoadPixelShader( pDevice, "shaders\\showmap.psh", &m_pShowMapPS );
#else
	static DWORD const ShowMapPS[] =
	{
		0xFFFF0200, 0x0200001F, 0x80000000, 0xB0030000,
		0x0200001F, 0x90000000, 0xA00F0800, 0x03000042,
		0x800F0000, 0xB0E40000, 0xA0E40800, 0x02000001,
		0x800F0000, 0x80000000, 0x02000001, 0x800F0800,
		0x80E40000, 0x0000FFFF,
	};
	pDevice->CreatePixelShader( ShowMapPS, &m_pShowMapPS );
#endif
}

CShadowMap::SharedData::~SharedData() 
{
	assert( !m_RefCount );

	SafeRelease( m_pShowMapPS );
}

CShadowMap::CShadowMap( LPDIRECT3DDEVICE9 pDevice, int32 Width, int32 Height ) 
	: CRenderToTexture( pDevice, Width, Height, 0xffffffffu, D3DFMT_R32F, D3DUSAGE_RENDERTARGET, D3DFMT_D16 )
	, m_UseExplicitHull( false )
{
	SharedDataMap::iterator it = gm_SharedDataMap.find( pDevice );
	if (it == gm_SharedDataMap.end())
	{
		it = gm_SharedDataMap.insert( std::make_pair( pDevice, new SharedData( pDevice ) ) ).first;
	}
	assert( it != gm_SharedDataMap.end() );
	m_pSharedData = it->second;
	++m_pSharedData->m_RefCount;
}

/*virtual*/ CShadowMap::~CShadowMap()
{
	assert( m_pSharedData->m_RefCount > 0 );
	if (!--m_pSharedData->m_RefCount)
	{
		LPDIRECT3DDEVICE9 pDevice = m_pSharedData->m_pDevice;
		delete m_pSharedData;
		gm_SharedDataMap.erase( pDevice );
	}
}

class CRenderToTextureMaterial : public CMaterial
{
public:
	CRenderToTextureMaterial( CRenderToTexture const& RenderToTexture ) 
		: m_RenderToTexture( RenderToTexture )
	{
	}

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
	{
		if (PassIndex < 0 || PassIndex >= 1)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
		}

		m_RenderToTexture.SetTextureToDevice( Context, 0 );
	}

private:
	CRenderToTexture const& m_RenderToTexture;

	CRenderToTextureMaterial& operator=( CRenderToTextureMaterial const& ); //Not implemented
};

void CShadowMap::RenderOverlay( CRenderContext const& Context )
{
	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();

	if (m_hOverlay == NULL)
	{
		m_hOverlay = CMesh::CreateRect( pD3DDevice, 0.5f, 0.6f, 1, 1 );
		m_hOverlay->Scale( -1.0f, 1.0f, -1.0f );
		m_hOverlay->Translate( -0.75f, 0.70f, 0.0f );
	}

	pD3DDevice->SetPixelShader( m_pSharedData->m_pShowMapPS );

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	pD3DDevice->SetTransform( D3DTS_WORLD,		CMatrix4f::gm_Identity.GetD3DMatrixPtr() );
	pD3DDevice->SetTransform( D3DTS_VIEW,		CMatrix4f::gm_Identity.GetD3DMatrixPtr() );
	pD3DDevice->SetTransform( D3DTS_PROJECTION,	CMatrix4f::gm_Identity.GetD3DMatrixPtr() );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	{
		HMaterial hMaterial = new CRenderToTextureMaterial( *this );
		std::vector<HMaterial> Materials( 1 );
		Materials[0] = hMaterial;
		m_hOverlay->Render( pD3DDevice, Context, &Materials );
		CMaterial::ResetCachedMaterial( Context, pD3DDevice );
	}

	Context.SetTexture( pD3DDevice, 0, NULL );
	pD3DDevice->SetPixelShader( NULL );

	pD3DDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
}

/*virtual*/ void CShadowMap::Update( CCamera const& RealCamera, CScene const& Scene )
{
	if (!GetEnabled())
	{
		return;
	}

	UpdateInternal( RealCamera, Scene );
}

void CShadowMap::UpdateInternal( CCamera const& RealCamera, CScene const& Scene )
{
	RealCamera;
	Scene;

	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, FALSE );
	pD3DDevice->SetTexture( 0, NULL );

	_BaseClass::UpdateInternal();

	pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
	pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
}

/*virtual*/ void CShadowMap::BeginShadows( CCamera const& RealCamera ) const
{
	RealCamera;

	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	//Small depth bias to avoid Z-fighting due to numerical precision problems
	float32 const DepthBias = -5.0e-5f;
	pD3DDevice->SetRenderState( D3DRS_DEPTHBIAS, *reinterpret_cast<DWORD const*>( &DepthBias ) );

	SetTextureToDevice( CRenderContext( NULL ), KTextureStage );
	pD3DDevice->SetSamplerState( KTextureStage, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	pD3DDevice->SetSamplerState( KTextureStage, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

	pD3DDevice->SetSamplerState( KTextureStage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
	pD3DDevice->SetSamplerState( KTextureStage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
}

/*virtual*/ void CShadowMap::EndShadows() const
{
	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();

	pD3DDevice->SetSamplerState( KTextureStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	pD3DDevice->SetSamplerState( KTextureStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	pD3DDevice->SetTexture( KTextureStage, NULL );
	pD3DDevice->SetVertexShader( NULL );
	pD3DDevice->SetPixelShader( NULL );

	float32 const DepthBias = 0.0f;
	pD3DDevice->SetRenderState( D3DRS_DEPTHBIAS, *reinterpret_cast<DWORD const*>( &DepthBias ) );

	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}

CShadowMapDirectional::SharedData::SharedData( LPDIRECT3DDEVICE9 pDevice )
	: m_RefCount( 0 )
	, m_pDevice( pDevice )
{

	D3DCAPS9 Caps;
	HRESULT Result = m_pDevice->GetDeviceCaps( &Caps );

	if (!SUCCEEDED( Result ))
	{
		throw std::runtime_error( "GetDeviceCaps() failure" );
	}

	if ((Caps.PixelShaderVersion & 0xffffu) < 0x0200)
	{
		throw std::runtime_error( "pixel shader 2.0 required" );
	}

#ifdef LOAD_SHADERS
	LoadVertexShader( pDevice, "shaders\\shadowmap.vsh", &m_pShadowMapVS );
	LoadPixelShader( pDevice, "shaders\\shadowmap.psh", &m_pShadowMapPS );
	LoadVertexShader( pDevice, "shaders\\shadowscene.vsh", &m_pSceneVS );
	LoadPixelShader( pDevice, "shaders\\shadowscene.psh", &m_pScenePS );
	LoadVertexShader( pDevice, "shaders\\shadowscenetex.vsh", &m_pSceneTexVS );
	LoadPixelShader( pDevice, "shaders\\shadowscenetex.psh", &m_pSceneTexPS );
#else
	// Create shadow map shaders
	static DWORD const ShadowMapVS[] =
	{
		0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000,
		0x00000014, 0x800F0000, 0x90E40000, 0xA0E40000,
		0x00000001, 0xC00F0000, 0x80E40000, 0x00000006,
		0x800F0001, 0x80FF0000, 0x00000005, 0xE0010000,
		0x80E40001, 0x80AA0000, 0x0000FFFF,
	};
	pDevice->CreateVertexShader( ShadowMapVS, &m_pShadowMapVS );

	static DWORD const ShadowMapPS[] =
	{
		0xFFFF0200, 0x0200001F, 0x80000000, 0xB0010000,
		0x02000001, 0x800F0000, 0xB0000000, 0x02000001,
		0x800F0800, 0x80E40000, 0x0000FFFF,
	};
	pDevice->CreatePixelShader( ShadowMapPS, &m_pShadowMapPS );

	// Create scene shaders
	static DWORD const ShadowSceneVS[] =
	{
		0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000,
		0x0000001F, 0x80000003, 0x900F0001, 0x0000001F,
		0x8000000A, 0x900F0002, 0x00000014, 0xC00F0000,
		0x90E40000, 0xA0E40000, 0x00000008, 0x800F0000,
		0x90E40001, 0xA0E40004, 0x00000005, 0x800F0000,
		0x80E40000, 0xA0E40005, 0x00000005, 0xD00F0000,
		0x80E40000, 0x90E40002, 0x00000014, 0x800F0001,
		0x90E40000, 0xA0E40008, 0x00000006, 0x800F0002,
		0x80FF0001, 0x00000005, 0xE00F0000, 0x80E40002,
		0x80E40001, 0x00000014, 0x800F0001, 0x90E40000,
		0xA0E4000C, 0x00000006, 0x800F0002, 0x80FF0001,
		0x00000005, 0xE0010001, 0x80E40002, 0x80AA0001,
		0x0000FFFF,
	};
	pDevice->CreateVertexShader( ShadowSceneVS, &m_pSceneVS );

	static DWORD const ShadowScenePS[] =
	{
		0xFFFF0200, 0x0200001F, 0x80000000, 0x900F0000,   
		0x0200001F, 0x80000000, 0xB0030000, 0x0200001F,   
		0x80000000, 0xB0010001, 0x0200001F, 0x90000000,   
		0xA00F0807, 0x03000042, 0x800F0001, 0xB0E40000,
		0xA0E40807, 0x03000002, 0x800F0001, 0x80000001,
		0xB1000001, 0x01000041, 0x800F0001, 0x02000001,
		0x800F0800, 0x90E40000, 0x0000FFFF,
	};
	pDevice->CreatePixelShader( ShadowScenePS, &m_pScenePS );

	static DWORD const ShadowSceneTexVS[] =
	{
		0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000,
		0x0000001F, 0x80000003, 0x900F0001, 0x0000001F,
		0x8000000A, 0x900F0002, 0x0000001F, 0x80000005,
		0x900F0003, 0x00000014, 0xC00F0000, 0x90E40000,
		0xA0E40000, 0x00000008, 0x800F0000, 0x90E40001,
		0xA0E40004, 0x00000005, 0x800F0000, 0x80E40000,
		0xA0E40005, 0x00000005, 0xD00F0000, 0x80E40000,
		0x90E40002, 0x00000001, 0xE00F0000, 0x90E40003,
		0x00000014, 0x800F0001, 0x90E40000, 0xA0E40008,
		0x00000006, 0x800F0002, 0x80FF0001, 0x00000005,
		0xE00F0001, 0x80E40002, 0x80E40001, 0x00000014,
		0x800F0001, 0x90E40000, 0xA0E4000C, 0x00000006,
		0x800F0002, 0x80FF0001, 0x00000005, 0xE0010002,
		0x80E40002, 0x80AA0001, 0x0000FFFF,
	};
	pDevice->CreateVertexShader( ShadowSceneTexVS, &m_pSceneTexVS );

	static DWORD const ShadowSceneTexPS[] =
	{
		0xFFFF0200, 0x0200001F, 0x80000000, 0x900F0000, 
		0x0200001F, 0x80000000, 0xB0030000, 0x0200001F, 
		0x80000000, 0xB0030001, 0x0200001F, 0x80000000, 
		0xB0010002, 0x0200001F, 0x90000000, 0xA00F0800, 
		0x0200001F, 0x90000000, 0xA00F0807, 0x03000042, 
		0x800F0000, 0xB0E40000, 0xA0E40800, 0x03000005, 
		0x802F0000, 0x80E40000, 0x90E40000, 0x03000042, 
		0x800F0001, 0xB0E40001, 0xA0E40807, 0x03000002, 
		0x800F0001, 0x80000001, 0xB1000002, 0x01000041, 
		0x800F0001, 0x02000001, 0x800F0800, 0x80E40000, 
		0x0000FFFF, 
	};
	pDevice->CreatePixelShader( ShadowSceneTexPS, &m_pSceneTexPS );
#endif
}

CShadowMapDirectional::SharedData::~SharedData()
{
	assert( !m_RefCount );

	SafeRelease( m_pSceneTexPS );
	SafeRelease( m_pSceneTexVS );
	SafeRelease( m_pScenePS );
	SafeRelease( m_pSceneVS );
	SafeRelease( m_pShadowMapPS );
	SafeRelease( m_pShadowMapVS );
}

CVector3f const& CShadowMapDirectional::gm_DefaultLightDir( CVector3f::gm_NegYAxis );
float32 const CShadowMapDirectional::gm_DefaultZBias = 5.0f;

CShadowMapDirectional::CShadowMapDirectional( LPDIRECT3DDEVICE9 pDevice, int32 Width, int32 Height )
	: CShadowMap( pDevice, Width, Height )
	, m_LightDir( gm_DefaultLightDir )
	, m_ZBias( gm_DefaultZBias )
	, m_LightDiffuse( 1.0f, 1.0f, 1.0f, 1.0f )
{
	SharedDataMap::iterator it = gm_SharedDataMap.find( pDevice );
	if (it == gm_SharedDataMap.end())
	{
		it = gm_SharedDataMap.insert( std::make_pair( pDevice, new SharedData( pDevice ) ) ).first;
	}
	assert( it != gm_SharedDataMap.end() );
	m_pSharedData = it->second;
	++m_pSharedData->m_RefCount;

	GetCamera().SetOrthographic( true );
	GetCamera().SetNearClip( 0.0f );
}

/*virtual*/ CShadowMapDirectional::~CShadowMapDirectional()
{
	assert( m_pSharedData->m_RefCount > 0 );
	if (!--m_pSharedData->m_RefCount)
	{
		LPDIRECT3DDEVICE9 pDevice = m_pSharedData->m_pDevice;
		delete m_pSharedData;
		gm_SharedDataMap.erase( pDevice );
	}
}

void GetModelTransformedBox( std::vector<CVector3f>& TransformedBox, CModel const& Model )
{
	CAABox const& Box = Model.GetBoundingBox();

	if (Box.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < EBoxVert::Count; ++i)
	{
		CVector3f Point(
			i & (1 << 0) ? Box.Max.x : Box.Min.x,
			i & (1 << 1) ? Box.Max.y : Box.Min.y,
			i & (1 << 2) ? Box.Max.z : Box.Min.z );

		//Transform bounding box vertex to world coordinates
		CVector3f TransformedPoint( false );
		Model.GetTransform().TransformNoOverlap( TransformedPoint, Point );

		TransformedBox.push_back( TransformedPoint );
	}
}

/*virtual*/ void CShadowMapDirectional::Update( CCamera const& RealCamera, CScene const& Scene )
{
	if (!GetEnabled())
	{
		return;
	}

	UpdateInternal( RealCamera, Scene );
}

void CShadowMapDirectional::UpdateInternal( CCamera const& RealCamera, CScene const& Scene )
{
	RealCamera;

	std::vector<CVector3f> Hull;
	std::vector<CVector3f> *pHull = &Hull;

	std::vector<CVector3f> FrustumHull;
	for (int32 i = 0; i < EBoxVert::Count; ++i)
	{
		FrustumHull.push_back( RealCamera.GetFrustum().m_Verts[i] );
	} 

	if (GetUseExplicitHull())
	{
		pHull = &GetExplicitHull();
	} else {
		/*
		Get the convex hull of the bounding volume of interest.
		
		TODO intersect the shadow receivers with the frustum
		to form set R. Then, extrude R in the opposite direction of 
		the light to form R'. Intersect the shadow casters with R'
		to form set S. The volume of interest is the union of R
		and S.

		As an approximation, we'll use the convex hull of the receivers
		and the convex hull of the casters, instead of dealing with
		concave geometry (sets of polyhedra).
		*/

		std::vector<CVector3f> ReceiverPoints;
		std::vector<CVector3f> CasterPoints;

		//Look for shadow receivers
		if (Scene.GetReceivesShadows())
		{
			int32 ModelCount = Scene.GetObjectCount<CModel>();
			for (int32 j = 0; j < ModelCount; ++j)
			{
				CModel& Model = *Scene.GetObject<CModel>( j );
				if (Model.GetReceivesShadows())
				{
					GetModelTransformedBox( ReceiverPoints, Model );
				}
			}

			int32 LevelCount = Scene.GetObjectCount<CLevel>();
			for (int32 j = 0; j < LevelCount; ++j)
			{
				CLevel& Level = *Scene.GetObject<CLevel>( j );
				
				if (!Level.GetReceivesShadows())
				{
					continue;
				}

				for (int32 i = 0; i < Level.GetModelCount(); ++i)
				{
					CModel& Model( Level.GetModel( i ) );
					if (Model.GetReceivesShadows())
					{
						GetModelTransformedBox( ReceiverPoints, Model );
					}
				}
			}
		}

#ifdef USE_CONVEX_HULLS
		//Intersect R with the frustum
		CConvexPolytope3D FrustumHedron;
		FrustumHedron.ComputeConvexHull( FrustumHull );

		CConvexPolytope3D ReceiverHedron;
		ReceiverHedron.ComputeConvexHull( ReceiverPoints );
		ReceiverHedron.Intersect( FrustumHedron );
#endif

		//Look for shadow casters
		if (Scene.GetCastsShadows())
		{
			int32 ModelCount = Scene.GetObjectCount<CModel>();
			for (int32 j = 0; j < ModelCount; ++j)
			{
				CModel& Model = *Scene.GetObject<CModel>( j );
				if (Model.GetCastsShadows())
				{
					GetModelTransformedBox( CasterPoints, Model );
				}
			}

			int32 LevelCount = Scene.GetObjectCount<CLevel>();
			for (int32 j = 0; j < LevelCount; ++j)
			{
				CLevel& Level = *Scene.GetObject<CLevel>( j );
				
				if (!Level.GetCastsShadows())
				{
					continue;
				}

				for (int32 i = 0; i < Level.GetModelCount(); ++i)
				{
					CModel& Model( Level.GetModel( i ) );
					if (Model.GetCastsShadows())
					{
						GetModelTransformedBox( CasterPoints, Model );
					}
				}
			}
		}

#ifdef USE_CONVEX_HULLS
		//Compute S
		CConvexPolytope3D CasterHedron;
		CasterHedron.ComputeConvexHull( CasterPoints );

#ifdef USE_RECEIVER_EXTRUSION
		//Intersect S with R' - this means intersect S only with face planes of
		//R that face away from the light.
		{
			int32 const FaceCount = ReceiverHedron.GetFaceCount();
			for (int32 i = 0; i < FaceCount; ++i)
			{
				CPlane Plane( ReceiverHedron.GetFacePlane( i ) );

				if (Plane.Normal.DotProduct( m_LightDir ) >= 0.0f)
				{
					CasterHedron.IntersectHalfspace( Plane );
				}
			}			
		}
#endif

		//Unite the result with R
		{
			std::vector<CVector3f> ReceiverVertices;
			ReceiverHedron.GetVertices( ReceiverVertices );
			int32 const RVertexCount = static_cast<int32>( ReceiverVertices.size() );
			for (int32 i = 0; i < RVertexCount; ++i)
			{
				CasterHedron.ExtendToEnclose( ReceiverVertices[i] ); 
			}

			assert( pHull->empty() );
			CasterHedron.GetVertices( *pHull );
		}
#else
		pHull->insert( pHull->end(), ReceiverPoints.begin(), ReceiverPoints.end() );
		pHull->insert( pHull->end(), CasterPoints.begin(), CasterPoints.end() );
#endif
	}

	if (pHull->empty())
	{
		//Default to using the frustum
		//TODO set a bool that is used to disable this shadow
		pHull = &Hull;
		Hull = FrustumHull;
	}

//	UpdateCameraUniform( *pHull );
	UpdateCameraLiSPSM( *pHull, RealCamera );

	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();

	pD3DDevice->SetVertexShader( m_pSharedData->m_pShadowMapVS );
	pD3DDevice->SetPixelShader( m_pSharedData->m_pShadowMapPS );

	SetShadowWorldTransform( pD3DDevice, CMatrix4f::gm_Identity );

	_BaseClass::UpdateInternal( RealCamera, Scene );

	pD3DDevice->SetPixelShader( NULL );
	pD3DDevice->SetVertexShader( NULL );
}

/*virtual*/ void CShadowMapDirectional::BeginShadows( CCamera const& RealCamera ) const
{
	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();
	
	_BaseClass::BeginShadows( RealCamera );

	SetSceneTextured( false );
	SetSceneWorldTransform( pD3DDevice, RealCamera, CMatrix4f::gm_Identity );

	pD3DDevice->SetVertexShaderConstantF( 5, m_LightDiffuse.e, 1 );
}

void CShadowMapDirectional::SetShadowWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CMatrix4f const& ModelToWorld ) const
{
	CMatrix4f WorldToShadowScreen( ModelToWorld * GetCamera().GetWorldToScreen() );
	pD3DDevice->SetVertexShaderConstantF( 0, WorldToShadowScreen.GetTranspose4x4().e, 4 );
}

void CShadowMapDirectional::SetSceneWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CCamera const& RealCamera, CMatrix4f const& ModelToWorld ) const
{
	CMatrix4f WorldToScreen( RealCamera.GetWorldToScreen() );
	CMatrix4f WorldToShadowScreen( GetCamera().GetWorldToScreen() );

	CMatrix4f ModelToScreen( ModelToWorld * WorldToScreen );
	CMatrix4f ModelToShadowScreen( ModelToWorld * WorldToShadowScreen );
	CMatrix4f ModelToShadowTexture = ModelToShadowScreen * GetScreenToTexture();

   	CMatrix4f ModelToShadowScreenBiased( false );
//	ModelToShadowScreenBiased = ModelToShadowScreen;
//	ModelToShadowScreenBiased.RowT.z -= m_ZBias;
	//For shadow maps in post-projective space, use a worldspace bias
	ModelToShadowScreenBiased = ModelToWorld;
	ModelToShadowScreenBiased.PostTranslate( m_LightDir * -m_ZBias );
	ModelToShadowScreenBiased *= WorldToShadowScreen;

	CMatrix4f WorldToModel( false );
	ModelToWorld.GetInverse4x4( &WorldToModel );
	CVector3f ModelLightDir( false );
	WorldToModel.RotateNoOverlap( ModelLightDir, m_LightDir );
	ModelLightDir.Normalize();

	pD3DDevice->SetVertexShaderConstantF( 0, ModelToScreen.GetTranspose4x4().e, 4 );
	pD3DDevice->SetVertexShaderConstantF( 4, CVector4f( -ModelLightDir.x, -ModelLightDir.y, -ModelLightDir.z, 0.0f ).e, 1 );
	pD3DDevice->SetVertexShaderConstantF( 8, ModelToShadowTexture.GetTranspose4x4().e, 4 );
	pD3DDevice->SetVertexShaderConstantF( 12, ModelToShadowScreenBiased.GetTranspose4x4().e, 4 );
}

void CShadowMapDirectional::SetSceneTextured( bool Textured ) const
{
	LPDIRECT3DDEVICE9 pD3DDevice = GetDevice();
	if (Textured)
	{
		pD3DDevice->SetPixelShader( m_pSharedData->m_pSceneTexPS );
		pD3DDevice->SetVertexShader( m_pSharedData->m_pSceneTexVS );
	} else {
		pD3DDevice->SetPixelShader( m_pSharedData->m_pScenePS );
		pD3DDevice->SetVertexShader( m_pSharedData->m_pSceneVS );
	}
}

void CShadowMapDirectional::UpdateCameraUniform( std::vector<CVector3f> const& Hull )
{
	int32 const HullSize = static_cast<int32>( Hull.size() );

	CAABox Box;
	float32 MinZ = +std::numeric_limits<float32>::max();
	float32 MaxZ = -std::numeric_limits<float32>::max();
	for (int32 i = 0; i < HullSize; ++i)
	{
		Box.ExtendToEnclose( Hull[i] );
		float32 const Z = m_LightDir.DotProduct( Hull[i] );
		MinZ = std::min( MinZ, Z );
		MaxZ = std::max( MaxZ, Z );
	}
	MaxZ = std::max( MaxZ, MinZ + 0.1f );

	CVector3f const Centroid( Box.ComputeCenter() );

	//Choose position of shadow camera
	CVector4f ShadowCameraPosition( Centroid + (MinZ - m_LightDir.DotProduct( Centroid )) * m_LightDir);
	ShadowCameraPosition.w = 1.0f;

	CVector4f ShadowCameraTarget( Centroid + (MaxZ - MinZ) * m_LightDir );
	ShadowCameraTarget.w = 1.0f;
	
#if 0 //Include an additonal projection, e.g. the camera's world-to-screen matrix
	Projection.Transform4d( ShadowCameraPosition, ShadowCameraPosition );
	if (ShadowCameraPosition.w != 0.0f)
	{
		float32 iw = 1.0f / ShadowCameraPosition.w;
		ShadowCameraPosition.CVector3f::operator*=( iw );
	}

	Projection.Transform4d( ShadowCameraTarget, ShadowCameraTarget );
	if (ShadowCameraTarget.w != 0.0f)
	{
		float32 iw = 1.0f / ShadowCameraTarget.w;
		ShadowCameraTarget.CVector3f::operator*=( iw );
	}
#endif

	//Choose shadow camera transform
	CMatrix4f ShadowCameraTransform( false );
	ShadowCameraTransform.SetLookAt( ShadowCameraPosition, ShadowCameraTarget, CVector3f::gm_YAxis );
#if 0 //Include an additonal projection, e.g. the camera's world-to-screen matrix
	CMatrix4f InverseProjection( false );
	Projection.GetInverse4x4( &InverseProjection );
	ShadowCameraTransform *= InverseProjection;
#endif

	FocusCamera( Hull, ShadowCameraTransform );
}

void CShadowMapDirectional::FocusCamera( std::vector<CVector3f> const& Hull, CMatrix4f& ShadowCameraTransform )
{
	//Set the camera transform and choose optimal FOV angles and far clip for the camera.
	//We will improve the provided camera transform by performing roll, shear, and translation in
	//the view X and Y coordinates. This requires that we find the minimum-area parallelogram
	//enclosing the 2D viewspace hull.

	int32 const HullSize = static_cast<int32>( Hull.size() );

	//Find the 2D points in view space corresponding to the hull
	std::vector<CVector2f> ViewPoints( HullSize );
	{
		CMatrix4f ShadowWorldToView( false );
		ShadowCameraTransform.GetInverse4x4( &ShadowWorldToView );

		for (int32 i = 0; i < HullSize; ++i)
		{
			CVector4f WorldPoint( Hull[i].x, Hull[i].y, Hull[i].z, 1.0f );

			CVector4f ViewPoint( false );
			ShadowWorldToView.Transform4dNoOverlap( ViewPoint, WorldPoint );

			assert( ViewPoint.w );
			float32 iW = 1.0f / ViewPoint.w;
			ViewPoints[i].x = ViewPoint.x * iW;
			ViewPoints[i].y = ViewPoint.y * iW;
		}
	}

	//Find the polygonal hull of the 2D points in view space
	CConvexPolytope2D ViewPoly;
	ViewPoly.ComputeConvexHull( ViewPoints );

	std::vector<CVector2f> const& Vertices = ViewPoly.GetVertices();
	int32 VertexCount = static_cast<int32>( Vertices.size() );

	//Find the edge normals and altitudes of the polygon for each edge
	std::vector<CVector2f> EdgeNormals( VertexCount );
	std::vector<float32> Altitudes( VertexCount );
	for (int32 i = 0; i < VertexCount; ++i)
	{
		CVector2f const& Vertex1 = Vertices[i];
		CVector2f const& Vertex2 = Vertices[(i + 1) % VertexCount];

		EdgeNormals[i].x = Vertex2.y - Vertex1.y;
		EdgeNormals[i].y = Vertex1.x - Vertex2.x;
		EdgeNormals[i].Normalize();
	
		Altitudes[i] = std::numeric_limits<float32>::max();
		for (int32 j = 0; j < VertexCount; ++j)
		{
			Altitudes[i] = std::min( Altitudes[i], EdgeNormals[i].DotProduct( Vertices[j] ) );
		}
		Altitudes[i] = EdgeNormals[i].DotProduct( Vertex1 ) - Altitudes[i];
		assert( Altitudes[i] >= 0.0f );
		Altitudes[i] = std::max( Altitudes[i], 1.0e-6f );
	}

	//Find the minimum enclosing parallelogram
	float32 MinArea = std::numeric_limits<float32>::max();
	int32 BestEdge1 = -1;
	int32 BestEdge2 = -1;
	for (int32 i = 0; i < VertexCount; ++i)
	{
		for (int32 j = i + 1; j < VertexCount; ++j)
		{
			float32 SinAngle = EdgeNormals[i].x * EdgeNormals[j].y - EdgeNormals[i].y * EdgeNormals[j].x;
			if (!SinAngle)
			{
				continue; //Parallel edges
			}

			float32 Area = Altitudes[i] * Altitudes[j] / fabsf( SinAngle );
			if (MinArea > Area)
			{
				MinArea = Area;
				BestEdge1 = i;
				BestEdge2 = j;
			}
		}
	}

	CMatrix4f Matrix;
	if (BestEdge1 != -1)
	{
		//We found the minimum enclosing parallelogram. Now, set the camera transform so that
		//the 2D hull can occupy as much of screen space as possible.
		
		CVector2f const EdgeVec1 = Vertices[(BestEdge1 + 1) % VertexCount] - Vertices[BestEdge1];
		CVector2f const EdgeVec2 = Vertices[(BestEdge2 + 1) % VertexCount] - Vertices[BestEdge2];

		CVector2f EdgeDir1 = EdgeVec1;
		EdgeDir1.Normalize();

		CVector2f EdgeDir2 = EdgeVec2;
		EdgeDir2.Normalize();

		//Find the intersection of the two lines containing the edges
		CVector2f Intersection( false );
		Intersection = Vertices[BestEdge1] + EdgeVec1 * 
			(EdgeNormals[BestEdge2].DotProduct( Vertices[BestEdge2] - Vertices[BestEdge1] ) /
			EdgeVec1.DotProduct( EdgeNormals[BestEdge2] ) );

		float32 Sign1 = EdgeDir1.DotProduct( Vertices[BestEdge1] + EdgeVec1 * 0.5f - Intersection );
		Sign1 = Sign1 <= 0.0f ? -1.0f : 1.0f;

		float32 Sign2 = EdgeDir2.DotProduct( Vertices[BestEdge2] + EdgeVec2 * 0.5f - Intersection );
		Sign2 = Sign2 <= 0.0f ? -1.0f : 1.0f;
		
		float32 const HalfCosecant = 0.5f / fabsf( EdgeNormals[BestEdge1].x * EdgeNormals[BestEdge2].y - 
			EdgeNormals[BestEdge1].y * EdgeNormals[BestEdge2].x );
		Matrix._11 = EdgeDir1.x * HalfCosecant * Altitudes[BestEdge2];
		Matrix._12 = EdgeDir1.y * HalfCosecant * Altitudes[BestEdge2];
		Matrix._21 = EdgeDir2.x * HalfCosecant * Altitudes[BestEdge1];
		Matrix._22 = EdgeDir2.y * HalfCosecant * Altitudes[BestEdge1];
		Matrix._41 = Intersection.x + Sign1 * Matrix._11 + Sign2 * Matrix._21;
		Matrix._42 = Intersection.y + Sign1 * Matrix._12 + Sign2 * Matrix._22;

#ifndef NDEBUG
		CMatrix4f M( false );
		Matrix.GetInverse4x4( &M );
		for (int32 i = 0; i < VertexCount; ++i)
		{
			CVector3f T( false );
			M.Transform( T, CVector3f( Vertices[i].x, Vertices[i].y, 0.0f ) );
			assert( T.x >= -1.01f );
			assert( T.x <=  1.01f );
			assert( T.y >= -1.01f );
			assert( T.y <=  1.01f );
		}
#endif

		ShadowCameraTransform = Matrix * ShadowCameraTransform;
	}

	m_Camera.SetTransform( ShadowCameraTransform );

	//Choose far clip and FOV angles for shadow camera
	{
		float32 MaxX = 1.0e-6f;
		float32 MaxY = 1.0e-6f;
		float32 MaxZ = 1.0e-6f;
		for (int32 i = 0; i < static_cast<int32>( Hull.size() ); ++i)
		{
			CVector3f const ViewPoint = m_Camera.TransformWorldToView( Hull[i] );

			MaxX = std::max( MaxX, fabsf( ViewPoint.x ) );
			MaxY = std::max( MaxY, fabsf( ViewPoint.y ) );
			MaxZ = std::max( MaxZ, ViewPoint.z );
		}

		float32 const iZ = 1.0f / MaxZ;
		m_Camera.SetFOV( 2.0f * atanf( MaxX * iZ ) );
		m_Camera.SetVerticalFOV( 2.0f * atanf( MaxY * iZ ) );
		m_Camera.SetFarClip( MaxZ );
	}
}


void CShadowMapDirectional::UpdateCameraLiSPSM( std::vector<CVector3f> const& Hull, CCamera const& RealCamera )
{
	//Light space perspective shadow maps method

    //Gamma is the angle between the light and the eye (real camera) direction.
	float32 const CosGamma = m_LightDir.DotProduct( RealCamera.GetTransform().RowZ );
	
	//Degenerate to uniform shadow maps if the light is nearly the same or opposite of the eye direction.
	if (fabsf( CosGamma ) >= 0.99f)
	{
		return UpdateCameraUniform( Hull );
	}
	
  	//Compute the "light space" matrix.
	CMatrix4f LightToView( false );
	{
		CMatrix4f ViewToLight( false );
		ViewToLight.RowY = -m_LightDir;
		ViewToLight.RowX = ViewToLight.RowY.CrossProduct( RealCamera.GetTransform().RowZ );
		ViewToLight.RowX.Normalize();
		ViewToLight.RowZ = ViewToLight.RowX.CrossProduct( ViewToLight.RowY );
		ViewToLight.RowT = CVector3f::gm_Zero;
		ViewToLight.RowX.w = 0.0f;
		ViewToLight.RowY.w = 0.0f;
		ViewToLight.RowZ.w = 0.0f;
		ViewToLight.RowT.w = 1.0f;
		LightToView = ViewToLight.GetTranspose4x4();
	}

	//Transform the hull points to light space
	CAABox Box;
	int32 const HullSize = static_cast<int32>( Hull.size() );
	std::vector<CVector3f> TransformedHull( HullSize );
	for (int32 i = 0; i < HullSize; ++i)
	{
		LightToView.RotateNoOverlap( TransformedHull[i], Hull[i] );
		Box.ExtendToEnclose( TransformedHull[i] );
	}
	Box.Max.z = std::max( Box.Max.z, Box.Min.z + 0.1f );
	
	//Compute the optimal near-plane distance for the perspective
	//transformation.
	float32 const SinGamma = sqrtf( 1.0f - CosGamma * CosGamma );
	float32 const ZNear = RealCamera.GetNearClip();
	float32 const ZFar = RealCamera.GetFarClip();
    float32 Nopt = (ZNear + sqrtf( ZNear * ZFar )) / SinGamma;

	//The formula recommended in the paper leads to large perspective aliasing near the eye.
	//Use a larger near clip value for the perspective point, resulting in something
	//closer to uniform shadow mapping.
	Nopt += 3.0f * (Box.Max.z - Box.Min.z);

	//Choose the best position for the light-space coordinate system.
	CVector3f LightSpaceOrigin( false );
	//The paper recommends using the transformed viewpoint for the perspective point x coordinate, 
	//but that doesn't make sense.
	LightSpaceOrigin.x = (Box.Min.x + Box.Max.x) * 0.5f;
	LightSpaceOrigin.y = (Box.Min.y + Box.Max.y) * 0.5f;
	LightSpaceOrigin.z = Box.Min.z - Nopt;
	
	//Include the translation in the light space matrix
	LightToView.RowT = -LightSpaceOrigin;

	//Compute the parameters of the perspective transformation
	float32 MaxX = 1.0e-3f;
	float32 MaxY = 1.0e-3f;
	float32 const MaxZ = Box.Max.z - LightSpaceOrigin.z;

	for (int32 i = 0; i < HullSize; ++i)
	{
		CVector3f Tmp = TransformedHull[i] - LightSpaceOrigin;
		float32 const iZ = 1.0f / Tmp.z;

		MaxX = std::max( MaxX, fabsf( Tmp.x * iZ ) );
		MaxY = std::max( MaxY, fabsf( Tmp.y * iZ ) );
	}

	//Compute the perspective transformation
	CMatrix4f Persp( false );
	Persp.MakePerspProjectionXY( MaxX, MaxY, Nopt, MaxZ );
	
	//Rotate 90 degrees about the x-axis to let the shadow map look down 
	//the y-axis after the perspective transformation is applied. This 
	//matrix also scales the resulting z values from range [-1,+1] to [0,1] 
	//and y values from [0,1] to [-1,+1].
	CMatrix4f Rotate(
		1.0f,  0.0f,  0.0f, 0.0f,
		0.0f,  0.0f, -0.5f, 0.0f,
		0.0f,  2.0f,  0.0f, 0.0f,
		0.0f, -1.0f,  0.5f, 1.0f
	);
	
	CMatrix4f ShadowCameraTransform( false );
	(LightToView * Persp * Rotate).GetInverse4x4( &ShadowCameraTransform );

#if 0
	m_Camera.SetTransform( ShadowCameraTransform );

	//Look at the entire projected space cube
	m_Camera.SetFOV( KPi * 0.5f );
	m_Camera.SetVerticalFOV( -1.0f );
	m_Camera.SetFarClip( 1.0f );
#else
	FocusCamera( Hull, ShadowCameraTransform );
#endif
}

typedef System::SmartPointer<class CRenderContextShadowMapDirectionalShadow> HRenderContextShadowMapDirectionalShadow;
class CRenderContextShadowMapDirectionalShadow : public CRenderContext
{
public:
	WT_CLASS_NO_CREATOR( CRenderContextShadowMapDirectionalShadow, CRenderContext );

	CRenderContextShadowMapDirectionalShadow( CShadowMapDirectional const& ShadowMap,
		ERenderCategory::Type RenderCategory = ERenderCategory::Opaque, bool Translucent = false )
		: CRenderContext( &ShadowMap.GetCamera(), RenderCategory, Translucent )
		, m_ShadowMap( ShadowMap )
	{
	}

	virtual bool ShouldRender( IRenderable const& Renderable ) const
	{
		return Renderable.GetCastsShadows();
	}

	virtual void SetWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CMatrix4f const& Transform ) const
	{
		assert( pD3DDevice != NULL );
//		pD3DDevice->SetTransform( D3DTS_WORLD, Transform.GetD3DMatrixPtr() );
		m_ShadowMap.SetShadowWorldTransform( pD3DDevice, Transform );
	}

private:
	CShadowMapDirectional const& m_ShadowMap;
};

/*virtual*/ HRenderContext CShadowMapDirectional::CreateRenderContext() const
{
	return new CRenderContextShadowMapDirectionalShadow( *this );
}

/*virtual*/ void CRenderContextShadowMapDirectionalScene::SetWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CMatrix4f const& Transform ) const
{
	assert( pD3DDevice != NULL );
//	pD3DDevice->SetTransform( D3DTS_WORLD, Transform.GetD3DMatrixPtr() );
	m_ShadowMap.SetSceneWorldTransform( pD3DDevice, *GetCamera(), Transform );
}

/*virtual*/ void CRenderContextShadowMapDirectionalScene::SetTexture( LPDIRECT3DDEVICE9 pDevice, int32 Stage, LPDIRECT3DBASETEXTURE9 pTexture ) const
{
	_BaseClass::SetTexture( pDevice, Stage, pTexture );
	if (Stage == 0)
	{
		m_ShadowMap.SetSceneTextured( pTexture != NULL );
	}
}

} //namespace Graphics

} //namespace WT
