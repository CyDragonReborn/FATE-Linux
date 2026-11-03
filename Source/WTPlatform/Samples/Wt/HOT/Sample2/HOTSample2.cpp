#include <atlbase.h>
#include <atlwin.h>
#include "WT\Application\Application2.h"
#include "WT\Math\Math.h"
#include "WT\Graphics\Camera.h"
#include "WT\Graphics\Model.h"
#include "WT\Graphics\UI\Text.h"
#include "WT\System\IO\FilePath.h"
#include "WT\Graphics\HTMLRenderer.h"
#include "WT\Math\Collision\Intersections.h"

using namespace WT;
using namespace WT::Application2;
using namespace WT::Graphics;
using namespace WT::Math;
using namespace WT::System::IO;


static const float32 KDefaultCameraRotX		= 0.12f;
static const CVector3f KDefaultCameraEye	= CVector3f( 0, 5.0f, -5.0f );
static const CVector3f KDefaultEulerAngles	= CVector3f( KDefaultCameraRotX, 0.0f, 0.0f );
static const int32 KUIWidth = 400;
static const int32 KUIHeight = 400;


class CMyApplication : public CApplication
{
public:
	CMyApplication()
		: m_pRenderer( NULL )
	{
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

		CreateBillboard();

		m_pRenderer = new CHTMLRenderer ();
		m_pRenderer->Create ( GetHWND() , KUIWidth,KUIHeight,"http://www.google.com/" /*CFilePath::GetFullPath( "HTMLPage1.htm")*/ );
//		m_pRenderer->Create ( GetHWND() , KUIWidth,KUIHeight,"http://www.dpreview.com/");
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
	void OnFrameRender ( IDirect3DDevice9* pDevice, double /*TotalTime*/, float /*ElapsedTime*/ )
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
			m_pRenderer->RenderElementToTexture ( gpTexture,"x02",&r);*/
			m_pRenderer->RenderToTexture ( (LPDIRECT3DTEXTURE9) m_hBillboardMaterial->GetTexture());
			m_hBillboard->Render ( pDevice , Context );
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
		SafeDelete( m_pRenderer );
		m_hCamera = NULL;
		m_hBillboard = NULL;
		m_hBillboardMaterial = NULL;
		SafeDelete( m_HelpText );

		CApplication::OnDeviceDestroyed();
	}

	//--------------------------------------------------------------------------------------
	// Handle updates to the scene
	//--------------------------------------------------------------------------------------
	void OnFrameMove ( IDirect3DDevice9* pDevice, double TotalTime, float ElapsedTime )
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
			case 'R':
				m_pRenderer->ReplaceAPIs();
				break;
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

	LRESULT OnMessage ( HWND /*hWnd*/, UINT uMsg, WPARAM wParam, LPARAM lParam , bool& /*NoFurtherProcessing*/ )
	{
		if ( !m_hBillboard || uMsg < WM_MOUSEFIRST || uMsg > WM_MOUSELAST )
			return 0;

		CVector3f v0 = m_hBillboard->GetMesh()->GetVertexPosition(0);
		CVector3f v1 = m_hBillboard->GetMesh()->GetVertexPosition(1);
		CVector3f v2 = m_hBillboard->GetMesh()->GetVertexPosition(2);
		CVector3f v3 = m_hBillboard->GetMesh()->GetVertexPosition(3);

		//no need to transform model to world for the billboard as its transform is identity.

		float32 u,v;
		CVector3f Intersection;
		if ( !m_hCamera->PickRectangle ( GET_X_LPARAM ( lParam ) , GET_Y_LPARAM ( lParam ) , v0,v2,v1,u,v,Intersection))
			return 0;
		int32 x = (int32) (u*KUIWidth);
		int32 y = (int32) (v*KUIHeight);
		switch ( uMsg )
		{
		case WM_MOUSEMOVE:
			m_pRenderer->HandleMouseMove ( x , y , wParam );
			break;
		case WM_LBUTTONDOWN:
			m_pRenderer->HandleMouseDown ( x , y , wParam );
			break;
		case WM_LBUTTONUP:
			m_pRenderer->HandleMouseUp ( x , y , wParam );
			break;
		}
		return 0;
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

	void CreateBillboard()
	{
		m_hBillboard = new CModel( CMesh::CreateRect ( m_D3DDevice , 5 , -5 , 1 , 1 ));
		IDirect3DTexture9* pBillboardTexture = NULL;
		m_D3DDevice->CreateTexture ( KUIWidth, KUIHeight, 1, NULL, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &pBillboardTexture, NULL );
		if ( pBillboardTexture )
		{
			m_hBillboardMaterial = new CMaterialTextured ( pBillboardTexture );
			pBillboardTexture->Release();
			m_hBillboard->GetMaterials()->push_back ( m_hBillboardMaterial.get() );
			m_hBillboard->GetMesh()->GetParts()[0]->SetMaterialID (0);
		}
	}

	HCamera						m_hCamera;
	HModel						m_hBillboard;
	HMaterialTextured			m_hBillboardMaterial;

	CHTMLRenderer				*m_pRenderer;

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
	return WinMainImpl<CMyApplication>( "HOT Sample 2" );
}
