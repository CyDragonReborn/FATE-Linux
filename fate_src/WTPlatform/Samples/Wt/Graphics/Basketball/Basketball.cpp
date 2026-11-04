// Basketball.cpp : Defines the entry point for the application.
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

class CBasketball : public CApplication
{
public:

	HRESULT OnDeviceCreated( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* /*pBackBufferSurfaceDesc*/ )
	{
		//Create a camera
		m_hCamera = new CCamera();
		m_hCamera->SetRenderTarget( pDevice );
		m_hCamera->SetFOV( KPi * 0.25f );
		m_hCamera->SetNearClip( 1.0f );
		m_hCamera->SetFarClip( 5000.0f );
		m_hCamera->SetRect( CRect2i( 0, 0, GetWindowClientRect().right,GetWindowClientRect().bottom ) );

		CreateCourt();
		CreatePlayer();

		return S_OK;
	}

	HRESULT OnDeviceReset( IDirect3DDevice9* pDevice, const D3DSURFACE_DESC* /*pBackBufferSurfaceDesc*/ )
	{
		m_hCamera->RestoreDeviceObjects( pDevice );
		m_hScene->RestoreDeviceObjects( pDevice );

		//Initialize render states

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

	void OnDeviceLost()
	{
		m_hScene->InvalidateDeviceObjects();
		m_hCamera->InvalidateDeviceObjects();
	}

	void OnDeviceDestroyed()
	{
		m_hScene = NULL;
		m_hPlayer = NULL;
		m_hCamera = NULL;

		CApplication::OnDeviceDestroyed();
	}

	void OnFrameMove ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
	{
		CApplication::OnFrameMove( pDevice, TotalTime, ElapsedTime );

		//Animate the model's transform
		uint32 iTime  = static_cast<uint32>( TotalTime * 1000 ) % 10000;
		float32 fAngle = iTime * KTwoPi / 10000.0f;

		m_hCamera->SetYRotation( KPi );
		m_hCamera->PostTranslate( 0.0f, 5.0f, 25.0f );
		m_hCamera->PostRotateY( fAngle );

		m_hScene->Update( ElapsedTime );
	}

	void RenderToTexture( CRenderToTexture const& RenderToTexture )
	{
		HRenderContext hRenderContext = RenderToTexture.CreateRenderContext();
		RenderScene( *hRenderContext );
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


	void OnFrameRender ( IDirect3DDevice9* pDevice, double /*TotalTime*/, float /*ElapsedTime*/ )
	{
		// Render the reflection textures
		m_hScene->RenderToTextures( *m_hCamera );

		// Setup the view and projection matrices
		m_hCamera->PrepareRender( pDevice );

		// Clear the backbuffer
		pDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, sBackgroundColor, 1.0f, 0 );

		// Begin the scene
		pDevice->BeginScene();

		CRenderContext RenderContext( m_hCamera );
		RenderScene( RenderContext );

		// End the scene
		pDevice->EndScene();
	}

	CBasketball()
	{
		LaunchWordPadLog( "Basketball" );

		WT::Using<CStandardMaterial>();
		WT::Using<CCompositeMaterial>();
		WT::Using<CDxMaterial>();
		WT::Using<CActor>();

		SetShowStats( EShowStats::Full );

		SetOverrideForceVsync( 0 );
	}

private:
	static D3DCOLOR const sBackgroundColor = D3DCOLOR_ARGB( 0xff, 0x40, 0x60, 0x80 );

	HCamera		m_hCamera;
	HScene		m_hScene;
	HActor		m_hPlayer;

	void CreateCourt();
	void CreatePlayer();
};

void CBasketball::CreateCourt()
{
	m_hScene = Deserialize<CScene>( "3D Assets\\court\\court.WTSCENE", "textures\\", m_D3DDevice );
	assert( m_hScene != NULL );
	m_hScene->OnRenderToTexture() += CRenderToTexture::UpdateCallback::Create( *this, &CBasketball::RenderToTexture );
}

void CBasketball::CreatePlayer()
{
	m_hPlayer = Deserialize<CActor>( "player models\\Player22.WTACTOR", "textures\\", m_D3DDevice );
	m_hScene->AddObject( *m_hPlayer );

	m_hPlayer->SetScale( 0.1f, 0.1f, 0.1f );

	//Load motion and assign it to the player

	HMotion hMotion = Deserialize<CMotion>( "player models\\Player221.WTMOTION", m_D3DDevice );
	assert( hMotion != NULL );
	m_hPlayer->AddMotion( hMotion );

	//Set initial pose.
	CActiveMotion( 0 ).Apply( *m_hPlayer );

	//Insert animation into queue
	HActiveMotion hActiveMotion = new CActiveMotion( 0 );
	hActiveMotion->SetLooping( true );
	m_hPlayer->GetActiveMotionQueue().Push( hActiveMotion.get() );
}

int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/ )
{
	return WinMainImpl<CBasketball>( "Basketball" );
}
