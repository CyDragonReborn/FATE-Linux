#include "WT\Application\Application2.h"
#include "WT\Math\Math.h"
#include "WT\Graphics\Camera.h"
#include "WT\Graphics\Model.h"

using namespace WT;
using namespace WT::Application2;
using namespace WT::Graphics;
using namespace WT::Math;


class CMyApplication : public CApplication
{
public:
	CMyApplication()
	{
		LaunchWordPadLog( "CubeSample2" );
		SetOverrideForceVsync(0);
	}

	HRESULT OnDeviceCreated( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* /*pBackBufferSurfaceDesc*/ )
	{
		//Create a camera
		m_hCamera = new CCamera();

		m_hCamera->SetFOV( KPi * 0.25f );
		m_hCamera->SetNearClip( 1.0f );
		m_hCamera->SetFarClip( 5000.0f );
		m_hCamera->SetRect( CRect2i( 0, 0, GetWindowClientRect().right,GetWindowClientRect().bottom ) );
		m_hCamera->SetLookAt( CVector3f( 0.0f, 0.0f, -5.0f ), CVector3f::gm_Zero, CVector3f::gm_YAxis );

		//Create a mesh
		HMesh hMesh = CMesh::CreateBox( pDevice, 1.0f, 1.0f, 1.0f, 1, 1, 1 );

		//Choose some vertex colors
		hMesh->SetHasDiffuse( true );
		for (int32 i = 0; i < hMesh->GetVertexCount(); ++i)
		{
			CVector3f const Pos = hMesh->GetVertexPosition( i );
			float32 const r = Pos.x + 0.5f;
			float32 const g = Pos.y + 0.5f;
			float32 const b = Pos.z + 0.5f;
			hMesh->SetVertexDiffuse( i, D3DCOLOR_COLORVALUE( r, g, b, 1.0f ) );
		}
		
		m_hModel = new CModel( hMesh );

		return S_OK;
	}

	HRESULT OnDeviceReset( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* /*pBackBufferSurfaceDesc*/ )
	{
		m_hCamera->RestoreDeviceObjects ( pDevice );
		pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

		pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		pDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_ARGB( 0xff, 0x0, 0x0, 0x0 ) );
		pDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
		pDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
		pDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

		//Choose material lighting property sources
		pDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
		pDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
		pDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );

		//enable texture filtering
		pDevice->SetSamplerState ( 0 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
		pDevice->SetSamplerState ( 0 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );

		//Create and enable a light
		D3DLIGHT9 Light;
		memset( &Light, 0, sizeof Light );
		Light.Type = D3DLIGHT_DIRECTIONAL;
		Light.Diffuse.r = 0.4f;
		Light.Diffuse.g = 0.3f;
		Light.Diffuse.b = 0.2f;
		Light.Diffuse.a = 1.0f;
		Light.Specular.r = 0.2f;
		Light.Specular.g = 0.1f;
		Light.Specular.b = 0.0f;
		Light.Specular.a = 1.0f;
		Light.Ambient.r = 0.6f;
		Light.Ambient.g = 0.7f;
		Light.Ambient.b = 0.8f;
		Light.Ambient.a = 1.0f;
		Light.Direction.x = -0.5f;
		Light.Direction.y = -0.7f;
		Light.Direction.z = +0.5f;
		Light.Falloff = 1.0f;
		pDevice->SetLight( 0, &Light );
		pDevice->LightEnable( 0, TRUE );

		return S_OK;
	}
	//--------------------------------------------------------------------------------------
	// Render the scene 
	//--------------------------------------------------------------------------------------
	void OnFrameRender ( IDirect3DDevice9* pDevice, double /*TotalTime*/, float /*ElapsedTime*/ )
	{
		// Setup the view and projection matrices
		m_hCamera->PrepareRender(pDevice);

		// Clear the render target and the zbuffer 
		pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0);

		// Render the scene
		if( SUCCEEDED( pDevice->BeginScene() ) )
		{
			CRenderContext Context( m_hCamera );
			m_hModel->Render( pDevice, Context );

			pDevice->EndScene();
		}
	}

	//--------------------------------------------------------------------------------------
	// Release resources created in the OnResetDevice callback here 
	//--------------------------------------------------------------------------------------
	void OnDeviceLost()
	{
		m_hCamera->InvalidateDeviceObjects();
	}

	//--------------------------------------------------------------------------------------
	// Release resources created in the OnCreateDevice callback here
	//--------------------------------------------------------------------------------------
	void OnDeviceDestroyed()
	{
		m_hCamera = NULL;
		m_hModel = NULL;

		CApplication::OnDeviceDestroyed();
	}

	//--------------------------------------------------------------------------------------
	// Handle updates to the scene
	//--------------------------------------------------------------------------------------
	void OnFrameMove ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
	{
		CApplication::OnFrameMove( pDevice, TotalTime, ElapsedTime );

		//Animate the model's transform
		uint32 iTime  = (uint32) (TotalTime*1000) % 5000;
		float32 Angle = iTime * KTwoPi / 5000.0f;
		m_hModel->SetAxisRotation( CVector3f( 0.8f, 0.0f, 0.6f ), Angle );
	}

private:
	HModel	m_hModel;
	HCamera m_hCamera;
};

int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/ )
{
	return WinMainImpl<CMyApplication>( "CubeSample2" );
}
