#include "WT\Application\Application2.h"
#include "WT\Math\Math.h"
#include "WT\Graphics\Camera.h"
#include "WT\Graphics\Model.h"
#include "WT\HOT\AdManager.h"
#include "WT\Graphics\UI\Text.h"
#include "WT\System\IO\FilePath.h"
#include "WT\Math\Collision\Intersections.h"

using namespace WT;
using namespace WT::Application2;
using namespace WT::Graphics;
using namespace WT::Math;
using namespace WT::HOT;
using namespace WT::System::IO;


static const float32 KDefaultCameraRotX		= 0.12f;
static const CVector3f KDefaultCameraEye	= CVector3f( 0, 5.0f, -5.0f );
static const CVector3f KDefaultEulerAngles	= CVector3f( KDefaultCameraRotX, 0.0f, 0.0f );

class CMyApplication : public CApplication
{
public:
	const static uint32 KBillboardCount = 5;

	CMyApplication()
	{
		LaunchWordPadLog( "HOTSample1" );

		SetOverrideForceVsync(0);
		m_Eye = KDefaultCameraEye;
		m_Rotation = KDefaultEulerAngles;
		m_MovingBackward = m_MovingForward = m_MovingLeft = m_MovingRight = false;
		m_Speed = 30.0f;
		SetNotifyOnMouseMove ( true );
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

		m_pAdManager = new CAdManager( GetD3DDevice(), GetHWND(), "Game1", "wildgames" );
		CreateBillboards();

		m_HelpText = new UI::CText(pDevice,m_pFrameStatsFont);
		m_HelpText->SetPosition ( 0,40);
		m_HelpText->SetAutoSize(true);
		m_HelpText->SetText ( L"A,S,D,W to move the the camera\nLeft mouse button to rotate camera\nT to report ads usage to the server");

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
	void OnFrameRender( IDirect3DDevice9* pDevice, double /*TotalTime*/, float ElapsedTime )
	{
		// Clear the render target and the zbuffer 
		pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0);

		// Render the scene
		if( SUCCEEDED( pDevice->BeginScene() ) )
		{
			// Setup the view and projection matrices
			m_hCamera->PrepareRender(pDevice);

			CRenderContext Context( m_hCamera );
/*			RECT r;
			SetRect ( &r,0,0,729,90);
			pRenderer->RenderElementToTexture ( gpTexture,"x02",&r);*/
			for (uint32 i = 0; i < m_hBillboards.size(); i++)
			{
				m_hBillboards[i]->Render( pDevice, Context );
			}

			// Call the ad manager to render ads into all surfaces where ads are placed
			m_pAdManager->OnFrameRender( ElapsedTime, m_hCamera->GetPositionComponent().GetD3DXVector3Ptr() );

			m_HelpText->Render ( pDevice , Context );
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

		SafeDelete ( m_pAdManager );

		m_hBillboards.clear();

		SafeDelete( m_HelpText );

		CApplication::OnDeviceDestroyed();
	}

	//--------------------------------------------------------------------------------------
	// Handle updates to the scene
	//--------------------------------------------------------------------------------------
	void OnFrameMove( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
	{
		CApplication::OnFrameMove( pDevice, TotalTime, ElapsedTime );

		CMatrix4f CameraTransform;
		CameraTransform.SetEulerAngles( CVector3f( m_Rotation.x, m_Rotation.y, 0.0f ) );
		CVector3f ForwardDir = CameraTransform.RowZ;
		ForwardDir.Normalize();
		CVector3f SideDir = CVector3f( CameraTransform.RowX.x, 0.0f, CameraTransform.RowX.z );
		SideDir.Normalize();

		float32 fDistance = m_Speed * ElapsedTime;
		if ( m_MovingForward )
			m_Eye += ForwardDir * fDistance;
		if ( m_MovingBackward )
			m_Eye -= ForwardDir * fDistance;
		if ( m_MovingLeft )
			m_Eye -= SideDir * fDistance;
		if ( m_MovingRight )
			m_Eye += SideDir * fDistance;

		CameraTransform.PostTranslate( m_Eye );

		m_hCamera->SetTransform(CameraTransform);

		m_pAdManager->OnFrameMove( ElapsedTime, m_hCamera->GetPositionComponent().GetD3DXVector3Ptr() );
	}

	void OnKeyboardEvent ( UINT Char, bool KeyDown, bool /*AltDown*/ )
	{
		if ( KeyDown )
		{
			switch ( Char )
			{
			case 'A':
				m_MovingLeft = true;
				m_MovingRight = false;
				m_MovingForward = false;
				m_MovingBackward = false;
				break;
			case 'D':
				m_MovingLeft = false;
				m_MovingRight = true;
				m_MovingForward = false;
				m_MovingBackward = false;
				break;
			case 'W':
				m_MovingLeft = false;
				m_MovingRight = false;
				m_MovingForward = true;
				m_MovingBackward = false;
				break;
			case 'S':
				m_MovingLeft = false;
				m_MovingRight = false;
				m_MovingForward = false;
				m_MovingBackward = true;
				break;
			case 'T':
				m_pAdManager->TrackUsage();
				break;
			case 'R':
				m_pAdManager->RemoveAd ( m_pAdManager->GetAd(0));
			}
		}
		else
		{
			m_MovingLeft = false;
			m_MovingRight = false;
			m_MovingForward = false;
			m_MovingBackward = false;
		}
	}

	void OnMouseEvent ( bool LeftButtonDown, bool /*RightButtonDown*/, bool /*MiddleButtonDown*/, bool /*SideButton1Down*/,
	                    bool /*SideButton2Down*/, int /*MouseWheelDelta*/, int XPos, int YPos )
	{
		if ( LeftButtonDown )
		{
			const float32 FOV = 3.0f * m_hCamera->GetFOV( );
			
			int dx = XPos - m_MousePos.x;
			int dy = YPos - m_MousePos.y;

			if ( dx==0 && dy==0 )
				return;

			float32 fx = (float32) dx/(float32) GetWindowWidth();
			float32 fy = (float32) dy/(float32) GetWindowHeight();

			if( fabsf(fx) != 0.0f )
			{
				m_Rotation.y += FOV * fx;
				
				while( m_Rotation.y < -KTwoPi )
					m_Rotation.y += KTwoPi;
				while( m_Rotation.y > +KTwoPi )
					m_Rotation.y -= KTwoPi;
				
			}
			if( fabsf(fy) != 0.0f )
			{				
				m_Rotation.x += FOV * fy;

				static const float32 HalfPi = KPi * 0.5f;
				if( m_Rotation.x < -HalfPi )
					m_Rotation.x = -HalfPi;
				if( m_Rotation.x > +HalfPi )
					m_Rotation.x = +HalfPi;				
			}
		}

		m_MousePos.x = XPos;
		m_MousePos.y = YPos;
	}

private:

	void CreateBillboards()
	{
		for (uint32 i = 0; i < KBillboardCount; ++i)
		{
			HModel hBillboard = new CModel( CMesh::CreateRect( m_D3DDevice, 5.0f, -5.0f, 1, 1 ) );
			float32 Alpha = Math::KTwoPi / KBillboardCount * i;
			hBillboard->PostTranslate( 0, 0, 10 );
			hBillboard->PostRotateY( Alpha );
			
			IDirect3DTexture9* pBillboardTexture = NULL;
			m_D3DDevice->CreateTexture( 220, 262, 1, NULL, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pBillboardTexture, NULL );

			if (pBillboardTexture != NULL)
			{
				HMaterialTextured hBillboardMaterial = new CMaterialTextured ( pBillboardTexture );
				pBillboardTexture->Release();
				hBillboard->GetMaterials()->push_back ( hBillboardMaterial.get() );
				hBillboard->GetMesh()->GetParts()[0]->SetMaterialID (0);
				LPDIRECT3DSURFACE9 pSurface = NULL;
				pBillboardTexture->GetSurfaceLevel ( 0 , &pSurface );
				char buf[100];
				sprintf( buf, "ad%d", (i % 5) + 1 );

				CAd* pAd = m_pAdManager->AddAd( pSurface, buf, 220, 262, "LocalHTML\\InitialImage.png" );

				//Set the effective 3D info for the ad so that visibility metrics can be computed
				CAd3DInfo Info;
				Info.Width = 5;
				Info.Height = -5;
				Info.Position = *hBillboard->GetTransform().RowT.GetD3DXVector3Ptr();
				Info.Normal = *hBillboard->GetTransform().RowZ.GetD3DXVector3Ptr();
				pAd->Set3DInfo( &Info );

				pSurface->Release();
				m_hBillboards.push_back( hBillboard );
			}
		}
	}

	HCamera						m_hCamera;

	vector<HModel>				m_hBillboards;

	CAdManager*					m_pAdManager;

	CVector3f					m_Rotation;
	float32						m_Speed;
	CVector3f					m_Eye;
	CVector2i					m_MousePos;

	UI::CText*					m_HelpText;

	bool						m_MovingForward;
	bool						m_MovingBackward;
	bool						m_MovingRight;
	bool						m_MovingLeft;
};

int APIENTRY WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/ )
{
	return WinMainImpl<CMyApplication>( "HOT Sample 1" );
}
