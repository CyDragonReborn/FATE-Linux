// CubeSample.cpp : Defines the entry point for the application.
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

#include "WT\Application\Application.h"
#include "WT\Math\Math.h"
#include "WT\Image\Image.h"
#include "WT\Graphics\Camera.h"
#include "WT\Graphics\Model.h"
#include "WT\System\Logging\Log.h"

using namespace WT;
using namespace WT::Application;
using namespace WT::Graphics;
using namespace WT::Math;

class CCubeSample : public CApplication
{
public:

	virtual HRESULT InitDeviceObjects()
	{
		//Initialize render states

		m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_ARGB( 0xff, 0x0, 0x0, 0x0 ) );
		m_pD3DDevice->SetRenderState( D3DRS_COLORVERTEX, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

		//Choose material lighting property sources
		m_pD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
		m_pD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
		m_pD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );

		//enable texture filtering
		m_pD3DDevice->SetSamplerState ( 0 , D3DSAMP_MAGFILTER , D3DTEXF_LINEAR );
		m_pD3DDevice->SetSamplerState ( 0 , D3DSAMP_MINFILTER , D3DTEXF_LINEAR );

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
		m_pD3DDevice->SetLight( 0, &Light );
		m_pD3DDevice->LightEnable( 0, TRUE );

		InitGeometry();

		return S_OK;
	}

	virtual HRESULT DeleteDeviceObjects()
	{
		m_hCamera = NULL;
		m_hModel = NULL;

		return S_OK;
	}

	virtual HRESULT FrameMove()
	{
		//Animate the model's transform
		uint32 iTime  = static_cast<uint32>( m_Timer.GetTotalTime() % 5000 );
		float32 fAngle = iTime * KTwoPi / 5000.0f;
		m_hModel->SetAxisRotation( CVector3f( 0.8f, 0.0f, 0.6f ), fAngle );

		return S_OK;
	}

	virtual HRESULT Render()
	{
		// Setup the view and projection matrices
		m_hCamera->PrepareRender(m_pD3DDevice);

		// Clear the backbuffer
		m_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, sBackgroundColor, 1.0f, 0 );

		// Begin the scene
		m_pD3DDevice->BeginScene();

		CRenderContext Context( m_hCamera );
		m_hModel->Render( m_pD3DDevice, Context );

		// End the scene
		m_pD3DDevice->EndScene();

		return S_OK;
	}

	CCubeSample()
	{
		LOG_SET_WORDPAD( true, "CubeSample.log" );

		m_RequiresVerticalSync = false;

		// Turn on framework framerate display
		SetDisplayFramerate( true );
		m_DisplayExtraStats = true;

		// For UI testing
		SetCursorVisibility( true );

		m_Timer.Start();
	}

private:
	static D3DCOLOR const sBackgroundColor = D3DCOLOR_ARGB( 0xff, 0x40, 0x60, 0x80 );

	HModel		m_hModel;
	HCamera		m_hCamera;

	void CreateCube();

	void InitGeometry()
	{
		//Create a camera
		m_hCamera = new CCamera();
		m_hCamera->SetRenderTarget( m_pD3DDevice );
		m_hCamera->SetFOV( KPi * 0.25f );
		m_hCamera->SetNearClip( 1.0f );
		m_hCamera->SetFarClip( 5000.0f );
		m_hCamera->SetRect( CRect2i( 0, 0, m_WindowWidth, m_WindowHeight ) );
		m_hCamera->SetLookAt( CVector3f( 0.0f, 0.0f, -5.0f ), CVector3f::gm_Zero, CVector3f::gm_YAxis );

		CreateCube();
	}

	System::CTimer m_Timer;
};

void CCubeSample::CreateCube()
{
	HMesh hMesh( CMesh::CreateBox( m_pD3DDevice, 1.0f, 1.0f, 1.0f, 1, 1, 1 ) );
	
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
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	hPrevInstance;
	return WinMainImpl<CCubeSample>( hInstance, lpCmdLine, nCmdShow, "CubeSample" );
}
