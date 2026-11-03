// Shadows.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <d3dx9.h>

#include "WT\Application\Application2.h"
#include "WT\Math\Math.h"
#include "WT\Image\Image.h"
#include "WT\Graphics\Graphics.h"
#include "WT\Graphics\Animation\Motion.h"
#include "WT\System\Logging\Log.h"

using namespace WT;
using namespace WT::Math;
using namespace WT::Application2;
using namespace WT::Graphics;
using namespace WT::Graphics::Animation;

class CShadows : public CApplication
{
public:

	virtual HRESULT OnDeviceCreated( IDirect3DDevice9* /*pDevice*/, const D3DSURFACE_DESC* /*pBackBufferSurfaceDesc*/ )
	{
		InitGeometry();

		return S_OK;
	}

	virtual HRESULT OnDeviceReset( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* /*pBackBufferSurfaceDesc*/ )
	{
		m_hCamera->RestoreDeviceObjects ( pDevice );
		m_hScene->RestoreDeviceObjects ( pDevice );

		//Initialize render states

		m_D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

		m_D3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		m_D3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
		m_D3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
		m_D3DDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

		//Choose material lighting property sources
		//m_D3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
		//m_D3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
		m_D3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );

		//enable texture filtering
		m_D3DDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
		m_D3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );

		return S_OK;
	}

	virtual void OnDeviceLost()
	{
		m_hCamera->InvalidateDeviceObjects();
		m_hScene->InvalidateDeviceObjects();
	}

	virtual void OnDeviceDestroyed()
	{
		m_hScene = NULL;
		
		for (int32 i = 0; i < KShadowCount; ++i)
		{
			m_hShadowMaps[i] = NULL;
		}

		m_hCamera = NULL;

		CApplication::OnDeviceDestroyed();
	}

	virtual void OnFrameMove ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
	{
		CApplication::OnFrameMove( pDevice, TotalTime, ElapsedTime );

		float32 fAngle = static_cast<float32>( fmod( 
			Graphics::gSimulationTimeSeconds * KTwoPi / 10.0f, static_cast<float64>( KTwoPi ) ) );

		m_hCamera->SetLookAt( 
			CVector3f( 20.0f + 20.0f * cosf( fAngle ), 100.0f + 30.0f * cosf( fAngle ), -80.0f ), 
			CVector3f( 20.0f, 0.0f, 0.0f ), CVector3f::gm_YAxis );

		CVector3f LightDir( false );

		if (m_hShadowMaps[0] != NULL)
		{
			LightDir.x = 0.1f;
			LightDir.z = 0.8f * cosf( fAngle );
			LightDir.y = -sqrtf( 1.0f - LightDir.x * LightDir.x - LightDir.z * LightDir.z );
			m_hShadowMaps[0]->SetLightDir( LightDir );
			m_hShadowMaps[0]->SetLightDiffuse( CVector4f( 0.15f, 0.10f, 0.05f, 1.0f ) );
		}

		if (m_hShadowMaps[1] != NULL)
		{
			LightDir.x = -0.5f;
			LightDir.z = 0.8f * sinf( fAngle );
			LightDir.y = -sqrtf( 1.0f - LightDir.x * LightDir.x - LightDir.z * LightDir.z );
			m_hShadowMaps[1]->SetLightDir( LightDir );
			m_hShadowMaps[1]->SetLightDiffuse( CVector4f( 0.05f, 0.10f, 0.15f, 1.0f ) );
		}

		m_hScene->Update( ElapsedTime );
	}

	void RenderToTexture( CRenderToTexture const& RenderToTexture )
	{
		HRenderContext hContext = RenderToTexture.CreateRenderContext();
		RenderScene( *hContext );
	}

	void RenderScene( CRenderContext& Context )
	{
		Context.SetRenderCategory( ERenderCategory::PreOpaque );
		m_hScene->Render( m_D3DDevice, Context );

		Context.SetRenderCategory( ERenderCategory::Opaque );
		m_hScene->Render( m_D3DDevice, Context );

		Context.SetRenderCategory( ERenderCategory::PostOpaque );
		Context.SetTranslucent( true );
		m_hScene->Render( m_D3DDevice, Context );
	}

	virtual void OnFrameRender ( IDirect3DDevice9* /*pDevice*/, double /*TotalTime*/, float /*ElapsedTime*/ )
	{
		m_D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		m_D3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

		m_D3DDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_ARGB( 0x0, 0xa0, 0xa0, 0xa0 ) );

		// Render the reflection textures
		m_hScene->RenderToTextures( *m_hCamera );

		// Setup the view and projection matrices
		m_hCamera->PrepareRender( m_D3DDevice );

		// Clear the backbuffer
		m_D3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, sBackgroundColor, 1.0f, 0 );

		// Begin the scene
		m_D3DDevice->BeginScene();

		//First render ordinary pass with ambient only lighting
		{
			CRenderContext Context( m_hCamera ); 
			RenderScene( Context );
		}

		//Now render shadow passes with diffuse lighting contributions
		for (int32 i = 0; i < KShadowCount; ++i)
		{
			if (m_hShadowMaps[i] != NULL)
			{
				CRenderContextShadowMapDirectionalScene Context( *m_hShadowMaps[i], *m_hCamera );
				Context.BeginShadows();
				RenderScene( Context );
				Context.EndShadows();
			}
		}

		if (m_hShadowMaps[0] != NULL)
		{
			m_hShadowMaps[0]->RenderOverlay( CRenderContext() );
		}

		// End the scene
		m_D3DDevice->EndScene();
	}

	CShadows()
	{
		LaunchWordPadLog( "Shadows" );

		WT::Using<CStandardMaterial>();
		WT::Using<CCompositeMaterial>();
		WT::Using<CDxMaterial>();
		WT::Using<CActor>();

		SetShowStats( EShowStats::Full );

		SetOverrideForceVsync(0);
	}

private:
	static D3DCOLOR const sBackgroundColor = D3DCOLOR_ARGB( 0xff, 0x40, 0x60, 0x80 );

	HCamera						m_hCamera;
	HScene						m_hScene;

	static int32 const KShadowCount = 2;
	HShadowMapDirectional		m_hShadowMaps[KShadowCount];

	void CreateScene();

	void InitGeometry()
	{
		//Create a camera
		m_hCamera = new CCamera();
		m_hCamera->SetRenderTarget( m_D3DDevice );
		m_hCamera->SetFOV( KPi * 0.25f );
		m_hCamera->SetNearClip( 1.0f );
		m_hCamera->SetFarClip( 500.0f );
		m_hCamera->SetRect( CRect2i( 0, 0, GetWindowClientRect().right,GetWindowClientRect().bottom ) );

		CreateScene();

		for (int32 i = 0; i < KShadowCount; ++i)
		{
			try
			{
				m_hShadowMaps[i] = new CShadowMapDirectional( m_D3DDevice, 512 );
			} catch (std::runtime_error const&) {
				m_hShadowMaps[i] = NULL;				
			}

			if (m_hShadowMaps[i] != NULL)
			{
				m_hShadowMaps[i]->SetZBias( 1.0f );
				m_hScene->AddShadowMap( m_hShadowMaps[i] );
			}
		}
	}
};

void CShadows::CreateScene()
{
	m_hScene = Deserialize<CScene>( "Scene.WTSCENE", "textures\\", m_D3DDevice );
	assert( m_hScene != NULL );
	m_hScene->OnRenderToTexture() += CRenderToTexture::UpdateCallback::Create( *this, &CShadows::RenderToTexture );
	m_hScene->SetBackgroundColor( sBackgroundColor );
}

int APIENTRY _tWinMain(HINSTANCE /*hInstance*/,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR    /*lpCmdLine*/,
                     int       /*nCmdShow*/)
{
	return WinMainImpl<CShadows>( "Shadows" );
}
