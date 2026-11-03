// ParticlesSample.cpp : Defines the entry point for the application.
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
#include "WT\Particles\ParticleSystem.h"
#include "WT\Particles\ParticleSystemCollection.h"

using namespace WT;
using namespace WT::Math;
using namespace WT::Application2;
using namespace WT::Graphics;
using namespace WT::Particles;

static const int32 ID_EMIT_BEHAVIOR			= 100;
static const int32 ID_GRAVITY_BEHAVIOR		= 101;
static const int32 ID_RANDOMMOVE_BEHAVIOR	= 102;
static const int32 ID_KILLPARTICLE_BEHAVIOR	= 103;
static const int32 ID_MOVE_BEHAVIOR			= 104;

class CParticlesSample : public CApplication
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
		// Delete all particle system collection objects
		m_ParticleSystems.GetParticleSystems().clear();
		m_ParticleSystems.ReleaseStates();

		m_hScene = NULL;
		m_hCamera = NULL;

		CApplication::OnDeviceDestroyed();
	}

	virtual void OnFrameMove ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
	{
		CApplication::OnFrameMove( pDevice, TotalTime, ElapsedTime );

		// Animate particles
		m_ParticleSystems.Update( ElapsedTime );

		float32 fAngle = static_cast<float32>( fmod( 
			Graphics::gSimulationTimeSeconds * KTwoPi / 10.0f, static_cast<float64>( KTwoPi ) ) );

		m_hCamera->SetLookAt( 
			CVector3f( 20.0f + 20.0f * cosf( fAngle ), 100.0f + 30.0f * cosf( fAngle ), -80.0f ), 
			CVector3f( 20.0f, 0.0f, 0.0f ) );

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
		m_ParticleSystems.Render( m_D3DDevice, Context );
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

		// End the scene
		m_D3DDevice->EndScene();
	}

	CParticlesSample()
	{
		LaunchWordPadLog( "ParticlesSample" );

		WT::Using<CStandardMaterial>();
		WT::Using<CCompositeMaterial>();
		WT::Using<CDxMaterial>();
		WT::Using<CActor>();

		SetShowStats( EShowStats::Full );

		SetOverrideForceVsync( 0 );
	}

private:
	static D3DCOLOR const sBackgroundColor = D3DCOLOR_ARGB( 0xff, 0x40, 0x60, 0x80 );

	HCamera						m_hCamera;
	HScene						m_hScene;

	CParticleSystemCollection m_ParticleSystems;

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

		InitParticles();
	}

	void InitParticles()
	{
		// Create the particle material.
		HParticleMaterial hLeafMaterial = new CParticleMaterial( m_D3DDevice, "Leaf.png" );

		//We'll use 32x32 frames from the texture for each particle. The texture is actually 64x64, so there
		//are four frames (0 through 3).
		HParticleSystemData hLeaf = CParticleSystemData::Create( m_D3DDevice, hLeafMaterial, 32, 32 );

		hLeaf->SetMaxParticleCount( 1000 );
		hLeaf->SetAdditive( true );

		// Add behaviors

		// Particle emitting behavior
		HEmitParticleBehavior hEmitBehavior = new CEmitParticleBehavior();
		Math::HDisc		 hEmitPosition = new CDisc( CVector3f( 20.0f, 30.0f, 0.0f ), CVector3f::gm_YAxis, 50.0f );
		Math::HPoint3d	 hEmitVelocity = new CPoint3d( CVector3f( 0.0f, -3.0f, 0.0f ) );
		Math::HLine		 hEmitSize = new CLine( CVector3f( 3.0f, 3.0f, 3.0f ), CVector3f( 5.0f, 5.0f, 5.0f ) );
		Math::HPoint3d	 hEmitColor = new CPoint3d( CVector3f( 255.0f, 255.0f, 255.0f ) );
		Math::HLine	     hEmitRotation = new CLine( CVector3f::gm_Zero, CVector3f( 0.0f, 0.0f, KTwoPi ) );

		//Create the emission behavior. The particles will use frames 0 through 3.
		hEmitBehavior->Init( ID_EMIT_BEHAVIOR, hEmitPosition, hEmitVelocity, hEmitSize, hEmitColor, 0xff, 0xff, 
			hEmitRotation, 0, 3, 0, 1 );
		hEmitBehavior->SetParticlesPerSecond( 10.0f );
		hLeaf->GetBehaviors().push_back( hEmitBehavior.get() );

		// Particle gravity behavior
		HGravityParticleBehavior hGravityBehavior = new CGravityParticleBehavior();
		hGravityBehavior->Init( ID_GRAVITY_BEHAVIOR, CVector3f( 0.0f, -2.0f, 0.0f ) );
		hLeaf->GetBehaviors().push_back( hGravityBehavior.get() );

		// Particle random movement behavior
		HRandomMoveParticleBehavior hRandMoveBehavior = new CRandomMoveParticleBehavior();
		Math::HLine hRandMoveShape = new CLine( CVector3f( -1.0f, -1.0f, -1.0f ), CVector3f( 1.0f, 1.0f, 1.0f ) );
		hRandMoveBehavior->Init( ID_RANDOMMOVE_BEHAVIOR, hRandMoveShape );
		hLeaf->GetBehaviors().push_back( hRandMoveBehavior.get() );

		// Particle lifetime behavior
		HKillInsideParticleBehavior hKillInsideBehavior = new CKillInsideParticleBehavior();
		Math::HPlaneShape hPlaneShape = new CPlaneShape( CVector3f::gm_YAxis, CVector3f::gm_Zero );
		hKillInsideBehavior->Init( ID_KILLPARTICLE_BEHAVIOR, hPlaneShape );
		hLeaf->GetBehaviors().push_back( hKillInsideBehavior.get() );

		// Particle movement behavior
		HMoveParticleBehavior hMoveBehavior = new CMoveParticleBehavior();
		hMoveBehavior->SetID( ID_MOVE_BEHAVIOR );
		hLeaf->GetBehaviors().push_back( hMoveBehavior.get() );

		m_ParticleSystems.GetParticleSystems().push_back( hLeaf );
	}
};

void CParticlesSample::CreateScene()
{
	m_hScene = Deserialize<CScene>( "Scene.WTSCENE", "textures\\", m_D3DDevice );
	assert( m_hScene != NULL );
	m_hScene->OnRenderToTexture() += CRenderToTexture::UpdateCallback::Create( *this, &CParticlesSample::RenderToTexture );
	m_hScene->SetBackgroundColor( sBackgroundColor );
}

int APIENTRY _tWinMain(HINSTANCE /*hInstance*/,
                     HINSTANCE /*hPrevInstance*/,
                     LPTSTR    /*lpCmdLine*/,
                     int       /*nCmdShow*/)
{
	return WinMainImpl<CParticlesSample>( "ParticlesSample" );
}
