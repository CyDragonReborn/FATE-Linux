//
// Ad.cpp
//
// Copyright (C) 2005 WildTangent, Inc.
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/System/Platform.h"
#include "WT/System/Logging/Log.h"
#include "WT/System/IO/File.h"
#include "WT/System/IO/FilePath.h"
#include "WT/Math/Math.h"
#include "WT/HOT/Ad.h"
#include "WT/HOT/AdManager.h"

#include "WT/HOT/Private/AdImpl.h"
#include "WT/HOT/Private/AdManagerImpl.h"

#ifdef USE_DIRECTX_7
#define IDirect3DDevice IDirect3DDevice7
#define LPDIRECT3DSURFACE LPDIRECTDRAWSURFACE7
#define D3DSURFACE_DESC DDSURFACEDESC2
#define D3DVIEWPORT D3DVIEWPORT7
#elif defined USE_DIRECTX_8
#define IDirect3DDevice IDirect3DDevice8
#define LPDIRECT3DSURFACE LPDIRECT3DSURFACE8
#define D3DVIEWPORT D3DVIEWPORT8
#define LPDIRECT3DTEXTURE LPDIRECT3DTEXTURE8
#else
#define IDirect3DDevice IDirect3DDevice9
#define LPDIRECT3DSURFACE LPDIRECT3DSURFACE9
#define D3DVIEWPORT D3DVIEWPORT9
#define LPDIRECT3DTEXTURE LPDIRECT3DTEXTURE9
#endif

using namespace WT::System;
using namespace WT::Graphics;
using namespace WT::System::IO;
using namespace WT::Tracking;
using namespace WT::Math;

namespace WT
{
namespace HOT
{

static const float32 KViewThreshold = 0.8f;

static std::string GetUrlFromIds( bool Online , const std::string& ProductCodeName, 
								  const std::string& DPCodeName, const std::string& AdID, 
								  const string& Params, const string& FlashVersion )
{
#ifndef WTPLATFORM_TRIAL
	if (Online)
	{
		//encrypted data for "http://hotwire.wildgames.com/ad.aspx", xored with 0x97
		static const uint8 EncryptedOnlineServer[] = {0xff,0xe3,0xe3,0xe7,0xad,0xb8,0xb8,0xff,0xf8,0xe3,0xe0,0xfe,0xe5,0xf2,0xb9,0xe0,0xfe,0xfb,0xf3,0xf0,0xf6,0xfa,0xf2,0xe4,0xb9,0xf4,0xf8,0xfa,0xb8,0xf6,0xf3,0xb9,0xf6,0xe4,0xe7,0xef};
		static char DecryptedOnlineServer[sizeof EncryptedOnlineServer + 1] = "";
		if (DecryptedOnlineServer[0] == 0)
		{
			int i = 0;
			for (; i < sizeof EncryptedOnlineServer; ++i)
			{
				DecryptedOnlineServer[i] = EncryptedOnlineServer[i] ^ 0x97;
			}
			DecryptedOnlineServer[i] = 0;
		}

		std::string OnlineURL = DecryptedOnlineServer;
		OnlineURL += "?GID=" + ProductCodeName;
		OnlineURL += "&DPID=" + DPCodeName;
		OnlineURL += "&AID=" + AdID;
		
		if (!Params.empty())
		{
			OnlineURL += "&" + Params;
		}

		if (!FlashVersion.empty())
		{
			OnlineURL += "&FLV=" + FlashVersion;
		}

		return OnlineURL;
	}
#endif

	const std::string LocalServer = CFilePath::GetFullPath( "." );
	std::string LocalURL = CFilePath::Combine( LocalServer, "LocalHTML" );
	LocalURL = CFilePath::Combine( LocalURL, AdID + ".html" );
	return LocalURL;
}

CAd::CAd( CAdManager* pManager )
{
	m_pImpl = new CAdImpl( *this, pManager );
}

CAd::~CAd()
{
	delete m_pImpl;
}

void CAd::SetTransparent( bool Transparent )
{
	m_pImpl->m_Transparent = Transparent;
	if (m_pImpl->m_pRenderer != NULL)
	{
		m_pImpl->m_pRenderer->SetTransparent( Transparent );
	}
}

bool CAd::GetTransparent() const
{
	return m_pImpl->m_Transparent;
}

void CAd::SetColorKey( ULONG ColorKey )
{
	m_pImpl->m_ColorKey = ColorKey;
	if (m_pImpl->m_pRenderer != NULL)
	{
		m_pImpl->m_pRenderer->SetColorKey( ColorKey );
	}
}

ULONG CAd::GetColorKey() const
{
	return m_pImpl->m_ColorKey;
}

void CAd::SetStatic( bool Static )
{
	m_pImpl->m_Static = Static;
}

bool CAd::GetStatic() const
{
	return m_pImpl->m_Static;
}

void CAd::SetSurface( LPDIRECT3DSURFACE pSurface )
{
	if (pSurface)
	{
		pSurface->AddRef();
	}
	SafeRelease( m_pImpl->m_pSurface );
	m_pImpl->m_pSurface = pSurface;
}

LPDIRECT3DSURFACE CAd::GetSurface() const
{
	return m_pImpl->m_pSurface;
}

void CAd::SetID( const std::string& ID )
{
	if (m_pImpl->m_ID == ID)
	{
		Refresh();
		return;
	}

	m_pImpl->m_ViewCount = 0;
	m_pImpl->m_ID = ID;

	m_pImpl->m_OfflineURL = GetUrlFromIds( false, 
		m_pImpl->m_pManager->GetProductCodeName(), m_pImpl->m_pManager->GetDPCodeName(), 
		m_pImpl->m_ID, m_pImpl->m_CustomParams, m_pImpl->m_pManager->m_pImpl->m_FlashVersion );

	if (!CFile::Exists( m_pImpl->m_OfflineURL ))
	{
		throw std::runtime_error( std::string( "File '" ) + m_pImpl->m_OfflineURL + "' not found'" );
	}

	m_pImpl->SetState( CAdImpl::KStateTryOnline );
}

std::string const& CAd::GetID() const
{
	return m_pImpl->m_ID;
}

void CAd::SetSize( uint32 Width , uint32 Height )
{
	if (m_pImpl->m_Width == Width && m_pImpl->m_Height == Height)
	{
		return;
	}
	m_pImpl->m_Width = Width;
	m_pImpl->m_Height = Height;
	if (m_pImpl->m_pRenderer != NULL)
	{
		m_pImpl->m_pRenderer->ResizeClient( Width , Height );
	}
}

void CAd::SetWidth( uint32 Width )
{
	SetSize( Width , m_pImpl->m_Height );
}

void CAd::SetHeight( uint32 Height )
{
	SetSize( m_pImpl->m_Width , Height );
}

uint32 CAd::GetWidth() const
{
	return m_pImpl->m_Width;
}

uint32 CAd::GetHeight() const
{
	return m_pImpl->m_Height;
}

void CAd::SetCustomParams( const std::string& CustomParams )
{
	m_pImpl->m_CustomParams = CustomParams;
}

std::string const& CAd::GetCustomParams() const
{
	return m_pImpl->m_CustomParams;
}

void CAd::Refresh()
{
	if (m_pImpl->m_pRenderer != NULL)
	{
		m_pImpl->m_pRenderer->Refresh();
		m_pImpl->m_ViewCount++;
	}
}

std::string CAd::GetCurrentURL() const
{
	switch (m_pImpl->m_State)
	{
	case CAdImpl::KStateTryOnline:
	case CAdImpl::KStateOnline:
		return GetOnlineURL();

	case CAdImpl::KStateTryOffline:
	case CAdImpl::KStateOffline:
		return GetOfflineURL();

	default:
		assert( 0 );
	}

	return "about:blank";
}

std::string CAd::GetOnlineURL() const
{
	return GetUrlFromIds( true, 
		m_pImpl->m_pManager->GetProductCodeName(), m_pImpl->m_pManager->GetDPCodeName(),
		m_pImpl->m_ID, m_pImpl->m_CustomParams, m_pImpl->m_pManager->m_pImpl->m_FlashVersion );
}

std::string CAd::GetOfflineURL() const
{
	return m_pImpl->m_OfflineURL;
}

void CAd::SetDestRect( const RECT* DestRect )
{
	if (DestRect != NULL)
	{
		if (!m_pImpl->m_DestRect)
		{
			m_pImpl->m_DestRect = new RECT();
		}
		*m_pImpl->m_DestRect = *DestRect;
	}
	else
	{
		SafeDelete( m_pImpl->m_DestRect );
	}
}

bool CAd::GetDestRect( RECT* const DestRect ) const
{
	if (!DestRect || !m_pImpl->m_DestRect)
	{
		return false;
	}
	*DestRect = *m_pImpl->m_DestRect;
	return true;
}

void CAd::Set3DInfo( const CAd3DInfo* Info )
{
	if (Info)
	{
		if (!m_pImpl->m_3DInfo)
		{
			m_pImpl->m_3DInfo = new CAd3DInfo;
		}
		*m_pImpl->m_3DInfo = *Info;
	}
	else
	{
		SafeDelete( m_pImpl->m_3DInfo );
	}
}

bool CAd::Get3DInfo( CAd3DInfo* const Info ) const
{
	if (!Info || !m_pImpl->m_3DInfo)
	{
		return false;
	}
	*Info = *m_pImpl->m_3DInfo;
	return true;
}

#ifdef WT_HOT_FULLSCREEN_ADS
void CAd::SetFullScreenInfo( const CAdFullScreenInfo* Info )
{
	if (Info)
	{
		if (!m_pImpl->m_FullScreenInfo)
		{
			m_pImpl->m_FullScreenInfo = new CAdFullScreenInfo;
		}
		*m_pImpl->m_FullScreenInfo = *Info;
	}
	else
	{
		SafeDelete( m_pImpl->m_FullScreenInfo );
	}
}

bool CAd::GetFullScreenInfo( CAdFullScreenInfo* const Info ) const
{
	if (!Info || !m_pImpl->m_FullScreenInfo || !m_pImpl->m_FullScreen)
	{
		return false;
	}
	*Info = *m_pImpl->m_FullScreenInfo;
	return true;
}

void CAd::SetFullScreen( bool IsFullScreen )
{
	m_pImpl->m_FullScreen = IsFullScreen;
}

bool CAd::IsFullScreen() const
{
	return m_pImpl->m_FullScreen;
}
#endif

void CAd::SetUpdatePolicy( EAdPolicy::AdPolicy Policy )
{
	m_pImpl->m_UpdatePolicy = Policy;
}

EAdPolicy::AdPolicy CAd::GetUpdatePolicy() const
{
	return m_pImpl->m_UpdatePolicy;
}

void CAd::SetRenderPolicy( EAdPolicy::AdPolicy Policy )
{
	m_pImpl->m_RenderPolicy = Policy;
}

EAdPolicy::AdPolicy CAd::GetRenderPolicy() const
{
	return m_pImpl->m_RenderPolicy;
}

CAdImpl::CAdImpl( CAd& Ad, CAdManager* pManager )
{
	m_pAd = &Ad;
	m_pManager = pManager;
	m_OfflineURL = "about:blank";
	m_pSurface = NULL;
	m_pRenderer = NULL;
	m_UpdatePolicy = EAdPolicy::AsFastAsPossible;
	m_RenderPolicy = EAdPolicy::AsFastAsPossible;
	m_UpdateInterval = 0.0f;
	m_RenderInterval = 0.0f;
	m_ColorKey = 0xff000000u;
	m_TimeOfFirstRender = 0.0;
	m_TimeOfLastRender = -1000000.0;
	m_DestRect = NULL;
	m_3DInfo = NULL;
	m_Height = 64;
	m_Width = 64;
	m_TotalTimeShown = 0;
	m_ActualTimeViewed = 0;
	m_ViewCount = 0;
	m_RenderCount = 0;
	m_FinalRenderCount = 0;
	m_EventCallback = NULL;
	m_pUserData = NULL;
	m_State = KStateTryOnline;
	m_Visible		 = true;
	m_Transparent	 = false;
	m_Static		 = false;
#ifdef WT_HOT_FULLSCREEN_ADS
	m_FullScreen	 = false;
	m_FullScreenInfo = NULL;
#endif

	std::fill( m_Metrics, m_Metrics + EAdMetric::Last, 0.0f );
}

CAdImpl::~CAdImpl()
{
	//m_EventCallback is owned by the CHTMLBase class, not by us.
	SafeRelease( m_pSurface );
	SafeDelete( m_DestRect );
	SafeDelete( m_3DInfo );
#ifdef WT_HOT_FULLSCREEN_ADS
	SafeDelete( m_FullScreenInfo );
#endif

	//TODO delete or check if m_pRenderer is NULL?
}

void CAdImpl::SetRenderer( CHTMLRenderer* Renderer )
{
	if (Renderer == m_pRenderer)
	{
		return;
	}

	if (m_pRenderer != NULL)
	{
		m_pRenderer->Stop();
		assert( m_EventCallback != NULL );
		m_pRenderer->OnEvent().Remove( m_EventCallback );
		m_EventCallback = NULL;
	}
	assert( m_EventCallback == NULL );

	//copy whatever settings are required from the old renderer
	if (m_pRenderer != NULL && Renderer != NULL)
	{
		Renderer->SetRenderInterval( m_pRenderer->GetRenderInterval() );
		Renderer->SetRenderThreadPriority( m_pRenderer->GetRenderThreadPriority() );
	}
	m_pRenderer = Renderer;
	m_pAd->SetTransparent( m_Transparent );
	m_pAd->SetColorKey( m_ColorKey );

	if (m_pRenderer != NULL)
	{
		//Increment the view count to account for the new renderer, which was initialized
		//with our URL. TODO find a better place for this.
		++m_ViewCount;

		assert( m_EventCallback == NULL );
		m_EventCallback = CHTMLBase::EventCallback::Create( *this, &CAdImpl::OnEvent );
		m_pRenderer->OnEvent() += m_EventCallback;
		assert( m_EventCallback != NULL );
	} else {
		assert( m_EventCallback == NULL );
	}
}

CHTMLRenderer* CAdImpl::GetRenderer()
{
	return m_pRenderer;
}

//React to a window.external.SetEvent() call. Params contains all of the arguments, in reverse order.
void CAdImpl::OnEvent( System::CHTMLBase* pBase, DISPPARAMS const& Params )
{
	//The following code expects this function to be called with the string
	//"SetAdInfo" and then an array of name/value pairs. If these criteria
	//are not met, then the window.external.SetEvent() call is ignored.

	if (Params.cArgs < 1)
	{
		return;
	}
	
	VARIANTARG const& FirstArg = Params.rgvarg[Params.cArgs - 1];
	if (FirstArg.vt == VT_BSTR && !wcscmp( FirstArg.bstrVal, L"SetAdInfo" ))
	{
		if (Params.cArgs != 2)
		{
			return;
		}

		VARIANTARG const& ArrayArg = Params.rgvarg[0];
		if (ArrayArg.vt != VT_DISPATCH || ArrayArg.pdispVal == NULL)
		{
			return;
		}

		VARIANT Var;
        VariantInit( &Var );

		DISPPARAMS DispParams = {NULL, NULL, 0, 0};
		HRESULT Result = ArrayArg.pdispVal->Invoke( DISPID_VALUE, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
			DISPATCH_PROPERTYGET | DISPATCH_METHOD, &DispParams, &Var, NULL, NULL );
		
		if (FAILED( Result ) || Var.vt != VT_BSTR || Var.bstrVal == NULL)
        {
			VariantClear( &Var );
            return;
		}

		//The string value will be comma-delimited name / value entries. Hopefully the data does 
		//not contain any commas, because they are passed as-is!

		wchar_t* p = Var.bstrVal;
		for (;;)
		{
			wchar_t* p2 = wcschr( p + 1, L',' );
			if (p2 == NULL)
			{
				//Name without corresponding value
				break;
			}

			std::wstring Name( p, p2 - p );
			++p2;

			std::wstring Value;

			wchar_t* p3 = wcschr( p2, L',' );
			if (p3)
			{
				Value = std::wstring( p2, p3 - p2 );
			} else {
				Value = std::wstring( p2 );
			}

			SetAdInfo( Name, Value );

			if (p3 == NULL)
			{
				break;
			}
			p = p3 + 1;
		}

        VariantClear( &Var );

	} else {
		//SetEvent call not relevant to this class
	}
}

void CAdImpl::SetAdInfo( std::wstring const& Name, std::wstring const& Value )
{
	if (Name == L"UID")
	{
		//TODO
	} else if (Name == L"ServerDate") {
		//TODO interpret values such as "Tue May 30 18:36:42 2006"
	}
}

void CAd::SetUpdateInterval( float32 Interval )
{
	m_pImpl->m_UpdateInterval = Interval;
}

float32 CAd::GetUpdateInterval() const
{
	return m_pImpl->m_UpdateInterval;
}

void CAd::SetRenderInterval( float32 Interval )
{
	m_pImpl->m_RenderInterval = Interval;
}

float32 CAd::GetRenderInterval() const
{
	return m_pImpl->m_RenderInterval;
}

void CAd::SetVisible( bool Visible )
{
	m_pImpl->m_Visible = Visible;
}

bool CAd::IsVisible() const
{
	return m_pImpl->m_Visible;
}

void CAd::SetTimeSinceFirstRender( float32 TimeSinceFirstRender )
{
	m_pImpl->m_TimeOfFirstRender = m_pImpl->m_pManager->m_pImpl->m_TotalTime + TimeSinceFirstRender;
}

float32 CAd::GetTimeSinceFirstRender() const
{
	return m_pImpl->m_RenderCount == 0 ? -1.0f : 
		static_cast<float32>( m_pImpl->m_pManager->m_pImpl->m_TotalTime - m_pImpl->m_TimeOfFirstRender );
}

void CAd::SetTimeSinceLastRender( float32 TimeSinceLastRender )
{
	m_pImpl->m_TimeOfLastRender = m_pImpl->m_pManager->m_pImpl->m_TotalTime + TimeSinceLastRender;
}

float32 CAd::GetTimeSinceLastRender() const
{
	return static_cast<float32>( m_pImpl->m_pManager->m_pImpl->m_TotalTime - m_pImpl->m_TimeOfLastRender );
}

uint32 CAd::GetViewCount() const
{
	return m_pImpl->m_ViewCount;
}

void CAd::ResetViewCount()
{
	m_pImpl->m_ViewCount = 0;
}

float64 CAd::GetTotalTimeShown() const
{
	return m_pImpl->m_TotalTimeShown;
}

void CAd::ResetTotalTimeShown()
{
	m_pImpl->m_TotalTimeShown = 0.0f;
}

float64 CAd::GetActualTimeViewed() const
{
	return m_pImpl->m_ActualTimeViewed;
}

void CAd::ResetActualTimeViewed()
{
	m_pImpl->m_ActualTimeViewed = 0.0f;
}

void CAdImpl::SetState( EState State )
{
	assert( State >= 0 );
	assert( State < KStateCount );

	if (m_State == State)
	{
		return;
	}

	m_State = State;

	//If there is a renderer, and we need it to use a different URL, then 
	//navigate to the new URL.
	if (m_pRenderer != NULL && m_pRenderer->GetURL() != m_pAd->GetCurrentURL())
	{
		m_ViewCount = 1;
		m_pRenderer->Navigate( m_pAd->GetCurrentURL() );
	}
}

void CAdImpl::IncRenderCount()
{
	m_RenderCount++;

	//Increment the final render count only if we are in a final state
	switch (m_State)
	{
		case CAdImpl::KStateOnline:
		case CAdImpl::KStateOffline:
			m_FinalRenderCount++;
	}
}

void CAdImpl::UpdateMetrics( IDirect3DDevice* pDevice , float32 ElapsedTime )
{
	std::fill( m_Metrics, m_Metrics + EAdMetric::Last, 0.0f );

	if (!m_Visible)
	{
		return;
	}

#ifdef WT_HOT_FULLSCREEN_ADS
	if (m_FullScreen)
	{
		std::fill( m_Metrics, m_Metrics + EAdMetric::Last, 1.0f );
		m_ActualTimeViewed += ElapsedTime;
		return;
	}
#endif

	if (m_3DInfo == NULL)
	{
		return;
	}

	//Get the camera information by examining the device
	CMatrix4f ViewMatrix( false );
	CMatrix4f ProjMatrix( false );
#ifdef USE_DIRECTX_7
	pDevice->GetTransform( D3DTRANSFORMSTATE_VIEW, ViewMatrix.GetD3DMatrixPtr() );
	pDevice->GetTransform( D3DTRANSFORMSTATE_PROJECTION, ProjMatrix.GetD3DMatrixPtr() );
#else
	pDevice->GetTransform( D3DTS_VIEW, ViewMatrix.GetD3DMatrixPtr() );
	pDevice->GetTransform( D3DTS_PROJECTION, ProjMatrix.GetD3DMatrixPtr() );
#endif

	D3DVIEWPORT Viewport;
	pDevice->GetViewport( &Viewport );
#ifdef USE_DIRECTX_7
	float32 ViewportWidth = static_cast<float32>( Viewport.dwWidth );
	float32 ViewportHeight = static_cast<float32>( Viewport.dwHeight );
#else
	float32 ViewportWidth = static_cast<float32>( Viewport.Width );
	float32 ViewportHeight = static_cast<float32>( Viewport.Height );
#endif
	
	//Get the ad-to-world matrix
	CMatrix4f AdToWorld( false );
	{
		//First start by computing the ad-to-world matrix
		AdToWorld.RowZ.x = m_3DInfo->Normal.x;
		AdToWorld.RowZ.y = m_3DInfo->Normal.y;
		AdToWorld.RowZ.z = m_3DInfo->Normal.z;
		if ((m_3DInfo->Width < 0.0f) ^ (m_3DInfo->Height < 0.0f))
		{
			AdToWorld.RowZ.Inverse();
		}
		AdToWorld.RowZ.Normalize();
		AdToWorld.RowZ.w = 0.0f;
		
		AdToWorld.RowX.CrossProduct( CVector3f::gm_YAxis, AdToWorld.RowZ );
		AdToWorld.RowX.Normalize();
		AdToWorld.RowX.w = 0.0f;

		AdToWorld.RowY.CrossProduct( AdToWorld.RowZ, AdToWorld.RowX );
		AdToWorld.RowY.w = 0.0f;

		AdToWorld.RowT.x = m_3DInfo->Position.x;
		AdToWorld.RowT.y = m_3DInfo->Position.y;
		AdToWorld.RowT.z = m_3DInfo->Position.z;
		AdToWorld.RowT.w = 1.0f;
	}

	//Now compute the ad-to-view matrix
	CMatrix4f AdToView( AdToWorld * ViewMatrix );

	//Check if the ad is facing the viewer (RowZ.z)
	float32 const KBestAlignment = 1.0f; //Best possible alignment
	float32 const KWorstAlignment = 0.4f; //Worst tolerable alignment
	m_Metrics[EAdMetric::Alignment] = (-AdToView.RowZ.z - KWorstAlignment) / (KBestAlignment - KWorstAlignment);
	if (m_Metrics[EAdMetric::Alignment] < 0.0f)
	{
		m_Metrics[EAdMetric::Alignment] = 0.0f;
	}

	//Check if the ad is rolled with respect to the viewer
	float32 RollAngle = atan2f( AdToView.RowY.x, AdToView.RowY.y );
	float32 const KBestRoll = 0.0f; //Best possible roll
	float32 const KWorstRoll = KPi * 0.16f; //Worst tolerable roll
	m_Metrics[EAdMetric::Roll] = (fabsf( RollAngle ) - KWorstRoll) / (KBestRoll - KWorstRoll);
	if (m_Metrics[EAdMetric::Roll] < 0.0f)
	{
		m_Metrics[EAdMetric::Roll] = 0.0f;
	}
	
	//Check for clipping of the ad by the view frustum.
	//First, form a polygon representing the add.
	CConvexPolytope3D ViewPoly;
	CVector3f AdPoint( false );
	AdPoint.z = 0.0f;
	for (int32 i = 0; i < 4; ++i)
	{
		AdPoint.x = (i == 1 || i == 2 ? 0.5f : -0.5f) * m_3DInfo->Width; 
		AdPoint.y = (i == 2 || i == 3 ? 0.5f : -0.5f) * m_3DInfo->Height; 

		CVector3f ViewPoint( false );
		AdToView.TransformNoOverlap( ViewPoint, AdPoint );

		ViewPoly.ExtendPolygonToEnclose( ViewPoint );
	}

	float32 const InitialViewArea = ViewPoly.ComputeArea();
	assert( InitialViewArea >= 0.0f );

	//Now clip the polygon by each of the six planes of the view frustum.
	{
		CPlane FrustumViewPlanes[6];

		//Negative x
		FrustumViewPlanes[0].Normal.x = -ProjMatrix.RowX.x - ProjMatrix.RowX.w;
		FrustumViewPlanes[0].Normal.y = -ProjMatrix.RowY.x - ProjMatrix.RowY.w;
		FrustumViewPlanes[0].Normal.z = -ProjMatrix.RowZ.x - ProjMatrix.RowZ.w;
		FrustumViewPlanes[0].Distance = -ProjMatrix.RowT.x - ProjMatrix.RowT.w;
		//Positive x
		FrustumViewPlanes[1].Normal.x =  ProjMatrix.RowX.x - ProjMatrix.RowX.w;
		FrustumViewPlanes[1].Normal.y =  ProjMatrix.RowY.x - ProjMatrix.RowY.w;
		FrustumViewPlanes[1].Normal.z =  ProjMatrix.RowZ.x - ProjMatrix.RowZ.w;
		FrustumViewPlanes[1].Distance =  ProjMatrix.RowT.x - ProjMatrix.RowT.w;
		//Negative y
		FrustumViewPlanes[2].Normal.x =  ProjMatrix.RowX.y - ProjMatrix.RowX.w;
		FrustumViewPlanes[2].Normal.y =  ProjMatrix.RowY.y - ProjMatrix.RowY.w;
		FrustumViewPlanes[2].Normal.z =  ProjMatrix.RowZ.y - ProjMatrix.RowZ.w;
		FrustumViewPlanes[2].Distance =  ProjMatrix.RowT.y - ProjMatrix.RowT.w;
		//Positive y
		FrustumViewPlanes[3].Normal.x = -ProjMatrix.RowX.y - ProjMatrix.RowX.w;
		FrustumViewPlanes[3].Normal.y = -ProjMatrix.RowY.y - ProjMatrix.RowY.w;
		FrustumViewPlanes[3].Normal.z = -ProjMatrix.RowZ.y - ProjMatrix.RowZ.w;
		FrustumViewPlanes[3].Distance = -ProjMatrix.RowT.y - ProjMatrix.RowT.w;
		//Negative z
		FrustumViewPlanes[4].Normal.x = -ProjMatrix.RowX.z;
		FrustumViewPlanes[4].Normal.y = -ProjMatrix.RowY.z;
		FrustumViewPlanes[4].Normal.z = -ProjMatrix.RowZ.z;
		FrustumViewPlanes[4].Distance = -ProjMatrix.RowT.z;
		//Positive z
		FrustumViewPlanes[5].Normal.x =  ProjMatrix.RowX.z - ProjMatrix.RowX.w;
		FrustumViewPlanes[5].Normal.y =  ProjMatrix.RowY.z - ProjMatrix.RowY.w;
		FrustumViewPlanes[5].Normal.z =  ProjMatrix.RowZ.z - ProjMatrix.RowZ.w;
		FrustumViewPlanes[5].Distance =  ProjMatrix.RowT.z - ProjMatrix.RowT.w;

		for (int32 i = 0; i < 6; ++i)
		{
			FrustumViewPlanes[i].Normalize();
			ViewPoly.IntersectHalfspace( FrustumViewPlanes[i] );
		}
	}

	//Compare the clipped ad area to the initial ad area and decrease ranking accordingly
	float32 const FinalViewArea = ViewPoly.ComputeArea();
	assert( FinalViewArea >= 0.0f );
	assert( FinalViewArea <= InitialViewArea * 1.1f /*Allow for numerical error*/);
	if (InitialViewArea)
	{
		float32 Ratio = FinalViewArea / InitialViewArea;
		float32 const KBestRatio = 1.0f;
		float32 const KWorstRatio = 0.6f;
		m_Metrics[EAdMetric::ViewArea] = (Ratio - KWorstRatio) / (KBestRatio - KWorstRatio);

		m_Metrics[EAdMetric::ViewArea] = std::max<float32>( m_Metrics[EAdMetric::ViewArea], 0.0f );
		m_Metrics[EAdMetric::ViewArea] = std::min<float32>( m_Metrics[EAdMetric::ViewArea], 1.0f );
	}

	//Transform the clipped polygon to pixel space and compute the visible area in squared pixels
	CMatrix4f ViewToPixel( ProjMatrix );
	ViewToPixel.PostTranslate( CVector3f( 1.0f, 1.0f, 0.0f ) );
	ViewToPixel.PostScale( CVector3f( ViewportWidth * 0.5f, ViewportHeight * 0.5f, 1.0f ) );

	CConvexPolytope2D ScreenPoly;

	std::vector<CVector3f> PolyVertices;
	ViewPoly.GetVertices( PolyVertices );
	int32 const PolyVertexCount = static_cast<int32>( PolyVertices.size() );
	for (int32 i = 0; i < PolyVertexCount; ++i)
	{
		CVector4f Temp( false );
		ViewToPixel.Transform4dNoOverlap( Temp, CVector4f( PolyVertices[i].x, PolyVertices[i].y, PolyVertices[i].z, 1.0f ) );

		assert( Temp.w > 0.0f );
		float32 const iW = 1.0f / Temp.w;

		Temp.x *= iW;
		Temp.y *= iW;

		ScreenPoly.ExtendToEnclose( CVector2f( Temp.x, Temp.y ) );
	}

	//Compute the area in squared pixels occupied by the ad
	float32 const PixelArea = ScreenPoly.ComputeArea();
	assert( PixelArea >= 0.0f );

	//Compute the area in squared texels occupied by the ad
	float32 TexelArea;

	if (m_DestRect)
	{
		TexelArea = static_cast<float32>(m_DestRect->right - m_DestRect->left) * 
			(m_DestRect->bottom - m_DestRect->top);
	}
	else
	{
		D3DSURFACE_DESC Desc;
#ifdef USE_DIRECTX_7
		m_pSurface->GetSurfaceDesc( &Desc );
		TexelArea = static_cast<float32>( Desc.dwWidth ) * Desc.dwHeight;
#else
		m_pSurface->GetDesc( &Desc );
		TexelArea = static_cast<float32>( Desc.Width ) * Desc.Height;
#endif
	}

	//Account for the visible size of the ad. If the ad occupies at least as many pixels
	//as there are texels in the ad, then it is legible. Otherwise, decrease the ranking 
	//based on the logarithm of the ratio.
	if (TexelArea)
	{
		float32 LogSize = logf( PixelArea / TexelArea );
		float32 const KBestLogSize = 0.0f; //Satisfactory size
		float32 const KWorstLogSize = -1.5f; //Worst tolerable size (in powers of e)

		if (LogSize >= KBestLogSize)
		{
			m_Metrics[EAdMetric::ScreenSpace] = 1.0f;
		} else {
			m_Metrics[EAdMetric::ScreenSpace] = (LogSize - KWorstLogSize) / (KBestLogSize - KWorstLogSize);
			if (m_Metrics[EAdMetric::ScreenSpace] < 0)
			{
				m_Metrics[EAdMetric::ScreenSpace] = 0.0f;
			}
		}
	}

	m_Metrics[EAdMetric::View] = m_Metrics[EAdMetric::Alignment] * m_Metrics[EAdMetric::Roll] *
	                             m_Metrics[EAdMetric::ViewArea] * m_Metrics[EAdMetric::ScreenSpace];

	if (m_Metrics[EAdMetric::View] >= KViewThreshold)
	{
		m_ActualTimeViewed += ElapsedTime;
	}
}

float32 CAd::GetMetric( EAdMetric::AdMetric Metric ) const
{
	if (Metric < 0 || Metric >= EAdMetric::Last)
	{
		return 0;
	}
	else
	{
		return m_pImpl->m_Metrics[Metric];
	}
}

void CAdImpl::TrackAdUsage( CGameTrack& GameTrack)
{
	char buf[1024];
	sprintf ( buf,"aid=%s,vct=%d,tts=%.02f,vsl=%d,alm=%.02f,rlm=%0.2f,vam=%0.2f,ssm=%0.2f,atv=%0.1f",
		      m_ID.c_str(), m_ViewCount , m_TotalTimeShown,
			  m_Visible ? 1 : 0,
		      m_Metrics[EAdMetric::Alignment] , m_Metrics[EAdMetric::Roll] , m_Metrics[EAdMetric::ViewArea],
		      m_Metrics[EAdMetric::ScreenSpace], m_ActualTimeViewed );
	GameTrack.Log( GAMETRACK_LOGGERID_WEBDDRIVER , m_pManager->GetProductCodeName() , buf ,"" );
}

void CAdImpl::CalculateUpdateIntervalBasedOnCamera( D3DXVECTOR3 const* CameraPosition )
{
	if (!CameraPosition || !m_3DInfo)
	{
		return;
	}
	D3DXVECTOR3 DistanceVec;
	D3DXVec3Subtract( &DistanceVec , CameraPosition , &m_3DInfo->Position );
	float32 Distance = D3DXVec3Length( &DistanceVec );
	//use (a+bx)^2 where a=0 and b=0.01 to give a very shallow projection like curve for the render interval
	//calculation
	float32 RenderInterval = Distance * Distance * 0.0001f;
	if (m_pRenderer != NULL)
	{
		m_pRenderer->SetRenderInterval( RenderInterval );
	}
}

#ifndef NDEBUG
void CAdImpl::AddDisplayedTextLine( std::string const& Line )
{
	if (!m_DisplayedText.empty())
	{
		m_DisplayedText += "\r\n";
	}
	m_DisplayedText += Line;
}
#endif

void CAdImpl::OnFrameMove( float64 TotalTime, D3DXVECTOR3 const* CameraPosition  )
{
	if (m_pRenderer != NULL)
	{
		if (m_pRenderer->IsPageError())
		{
			if (m_State == KStateTryOnline) 
			{
#ifndef NDEBUG
				AddDisplayedTextLine( "Online Error: " + m_pRenderer->GetPageStatusString() );
#endif
				//Use the offline URL.
				SetState( KStateTryOffline );
			} else if (m_State == KStateTryOffline) {
				throw std::runtime_error( "Error " + m_pRenderer->GetPageStatusName() + "in local URL '" + m_OfflineURL + "'" );
			}
		}
	}

	switch (m_UpdatePolicy)
	{
	case EAdPolicy::AsFastAsPossible:
		m_pRenderer->SetRenderInterval( 0.0f );
		m_pRenderer->ResumeUpdating();
		break;
	case EAdPolicy::BasedOnCameraLocation:
		m_pRenderer->ResumeUpdating();
		CalculateUpdateIntervalBasedOnCamera( CameraPosition );
		break;
	case EAdPolicy::Interval:
		m_pRenderer->SetRenderInterval( m_UpdateInterval );
		m_pRenderer->ResumeUpdating();
		break;
	case EAdPolicy::Never:
		m_pRenderer->PauseUpdating();
		break;
	case EAdPolicy::Custom:
		if (!m_pManager->OnUpdateAd.IsEmpty())
		{
			m_pRenderer->SetRenderInterval( m_pManager->OnUpdateAd(m_pAd) );
			m_pRenderer->ResumeUpdating();
		}
		break;
	}
}

void CAdImpl::CalculateRenderIntervalBasedOnCamera( D3DXVECTOR3 const* CameraPosition )
{
	if (!CameraPosition || !m_3DInfo)
	{
		return;
	}

	D3DXVECTOR3 DistanceVec;
	D3DXVec3Subtract( &DistanceVec , CameraPosition , &m_3DInfo->Position );
	float32 Distance = D3DXVec3Length( &DistanceVec );
	//use (a+bx)^2 where a=0 and b=0.01 to give a very shallow projection like curve for the render interval
	//calculation
	m_RenderInterval = Distance * Distance * 0.0001f;
}

bool CAdImpl::OnFrameRender( D3DXVECTOR3 const* CameraPosition  )
{
	assert( m_pRenderer != NULL );
	if (m_pRenderer->IsPageError() || !m_pAd->IsComplete())
	{
		return false;
	}

	switch ( m_RenderPolicy )
	{
	case EAdPolicy::AsFastAsPossible:
		break;

	case EAdPolicy::BasedOnCameraLocation:
		CalculateRenderIntervalBasedOnCamera( CameraPosition );
		return m_pAd->GetTimeSinceLastRender() >= m_RenderInterval;

	case EAdPolicy::Interval:
		return m_pAd->GetTimeSinceLastRender() >= m_RenderInterval;

	case EAdPolicy::Never:
		return false;

	case EAdPolicy::Custom:
		if (!m_pManager->OnRenderAd.IsEmpty())
		{
			return m_pManager->OnRenderAd( m_pAd );
		}
	}

	return true;
}

uint32 CAd::GetRenderCount() const
{
	return m_pImpl->m_RenderCount;
}

void CAd::ResetRenderCount()
{
	m_pImpl->m_RenderCount = 0;
	m_pImpl->m_FinalRenderCount = 0;
}

bool CAd::IsComplete() const
{
	if (m_pImpl->m_pRenderer != NULL)
	{
		return !m_pImpl->m_pRenderer->IsBusy();
	}
	else
	{
		return false;
	}
}

bool CAd::IsError() const
{
	if (m_pImpl->m_pRenderer != NULL)
	{
		return m_pImpl->m_pRenderer->IsPageError();
	}
	else
	{
		return false;
	}
}

void CAd::SetUserData( CAdUserData* pData )
{
	if (m_pImpl->m_pUserData != pData)
	{
		delete m_pImpl->m_pUserData;
		m_pImpl->m_pUserData = pData;
	}
}

CAdUserData* CAd::GetUserData()
{
	return m_pImpl->m_pUserData;
}

CAdUserData* CAd::GetUserData() const
{
	return m_pImpl->m_pUserData;
}

} //namespace HOT
} //namespace WT
