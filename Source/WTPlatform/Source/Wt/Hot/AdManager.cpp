//
// AdManager.cpp
//
// Copyright (C) 2006 WildTangent, Inc.
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "private/LMReader.h"
#include "WT/Application/RuntimeAttribute.h"
#include "WT/System/Platform.h"
#include "WT/System/Logging/Log.h"
#include "WT/System/IO/File.h"
#include "WT/HOT/AdManager.h"
#include "WT/Graphics/HTMLRenderer.h"
#include "WT/Math/Math.h"
#include "WT/GameTrack/GameTrack.h"
#include "WT/System/Net/NetInfo.h"
#include "WT/HOT/Private/AdImpl.h"
#include "WT/HOT/Private/AdManagerImpl.h"
#include "WT/Graphics/Flash8.tlh"


#ifdef _DEBUG
#pragma comment( lib, "LMReaderGameDevd.lib" )
#pragma comment( lib, "SVSGameDevd.lib" )
#else
#pragma comment( lib, "LMReaderGameDev.lib" )
#pragma comment( lib, "SVSGameDev.lib" )
#endif

using namespace WT::System;
using namespace WT::Graphics;
using namespace WT::Math;
using namespace WT::System::IO;
using namespace WT::Tracking;
using namespace WT::System::Net;
using namespace WT::Application;

namespace WT
{
namespace HOT
{

CAdManager::CAdManager( IDirect3DDevice* pDevice, HWND hParentWindow )
{
	CRuntimeAttribute RuntimeAttribute;
	std::string const ProductCodeName = RuntimeAttribute.GetProductCodeName();
	std::string const DPCodeName = RuntimeAttribute.GetDPCodeName();
	bool const UseTracking = RuntimeAttribute.GetUseTracking();

	m_pImpl = new CAdManagerImpl( this, pDevice, hParentWindow, ProductCodeName, DPCodeName, UseTracking );
}

CAdManager::CAdManager( IDirect3DDevice* pDevice, HWND hParentWindow, const std::string& ProductCodeName, const std::string& DPCodeName )
{
	if (ProductCodeName.empty())
	{
		throw std::invalid_argument( "ProductCodeName" );
	}

	if (DPCodeName.empty())
	{
		throw std::invalid_argument( "DPCodeName" );
	}

	m_pImpl = new CAdManagerImpl( this, pDevice, hParentWindow, ProductCodeName, DPCodeName, true );
}

CAdManagerImpl::CAdManagerImpl( CAdManager* pAdManager, IDirect3DDevice* pDevice, HWND hParentWindow,
	std::string const& ProductCodeName, std::string const& DPCodeName, bool UseTracking )
{
	m_pAdManager = pAdManager;

	if (UseTracking)
	{
		m_pGameTrack = new CGameTrack();
		m_pGameTrack->Init();
		m_pGameTrack->StartUpload( 100, true, false, 100, 60000, INFINITE );
	}
	
	m_pDevice = pDevice;
	if ( pDevice != NULL)
	{
		pDevice->AddRef();
	}

	m_hParentWindow = hParentWindow;
	m_ProductCodeName = ProductCodeName;
	m_DPCodeName = DPCodeName;
	m_MaxRenderers = 5;
	m_RenderersUsedCount = 0;
	m_TimeSinceOptimization = 0.0f;

#ifdef WT_HOT_FULLSCREEN_ADS
#ifndef USE_DIRECTX_7
	m_pFullScreenSprite = NULL;
	m_pFullScreenMaterial = NULL;
	CreateFullScreenAdResources();
#endif
#endif

	m_OptimizationInterval = 1.0f;
	m_FlashVersion = GetFlashVersion();
	m_RenderThreadPriority = THREAD_PRIORITY_NORMAL;
	m_TotalTime = 0.0;
}

CAdManagerImpl::~CAdManagerImpl()
{
	for ( uint32 i=0; i < m_Ads.size(); i++ )
	{
		delete m_Ads[i];
	}
	m_Ads.clear();

	for ( uint32 i=0; i < m_Renderers.size(); i++ )
	{
		delete m_Renderers[i];
	}
	m_Renderers.clear();

#ifdef WT_HOT_FULLSCREEN_ADS
#ifndef USE_DIRECTX_7
	SafeRelease ( m_pFullScreenMaterial );
	SafeDelete ( m_pFullScreenSprite );
#endif
#endif

	m_hParentWindow = NULL;
	SafeRelease( m_pDevice );

	//Stop uploading
	SafeDelete( m_pGameTrack );
}

CAdManager::~CAdManager()
{
	delete m_pImpl;
}

void CAdManager::SetParentWindow( HWND hParentWindow )
{
	m_pImpl->m_hParentWindow = hParentWindow;
}

HWND CAdManager::GetParentWindow() const
{
	return m_pImpl->m_hParentWindow;
}

void CAdManager::SetProductCodeName( std::string const& ProductCodeName )
{
	if (ProductCodeName.empty())
	{
		throw std::invalid_argument( "ProductCodeName" );
	}
	m_pImpl->m_ProductCodeName = ProductCodeName;
}

std::string const& CAdManager::GetProductCodeName() const
{
	return m_pImpl->m_ProductCodeName;
}

void CAdManager::SetDPCodeName( std::string const& DPCodeName )
{
	if (DPCodeName.empty())
	{
		throw std::invalid_argument( "DPCodeName" );
	}
	m_pImpl->m_DPCodeName = DPCodeName;
}

std::string const& CAdManager::GetDPCodeName() const
{
	return m_pImpl->m_DPCodeName;
}

/*static*/ bool CAdManager::IsOnline()
{
	return CNetInfo::IsOnline();
}

CAd* CAdManager::AddAd(
						 LPDIRECT3DSURFACE pSurface ,
						 const std::string& AdID,
						 uint32 Width, uint32 Height,
						 std::string const& ImageName,
						 RECT* DestRect ,
	                     const std::string& CustomParams, 
						 bool VisibleImmediately )
{
	//Initialize the surface
	if (ImageName.empty())
	{
		throw std::invalid_argument( "ImageName" );
	}

	if (!CFile::Exists( ImageName ))
	{
		throw std::runtime_error( std::string( "File '" ) + ImageName + "' not found'" );
	}

	D3DSURFACE_DESC DestDesc;
#ifdef USE_DIRECTX_7
	HRESULT Result = pSurface->Lock( NULL, &DestDesc, 0, NULL );
#else
	D3DLOCKED_RECT LockedDestRect;
	HRESULT Result = pSurface->LockRect( &LockedDestRect, NULL, 0 );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "LockRect failure" );
	}

	pSurface->GetDesc( &DestDesc );
#endif
	
#ifdef USE_DIRECTX_7
	LPDIRECTDRAWSURFACE pSourceSurface = NULL;
	
	DWORD Flags = D3DX_TEXTURE_NOMIPMAP;
	DWORD SourceWidth = DestDesc.dwWidth;
	DWORD SourceHeight = DestDesc.dwHeight;

	D3DX_SURFACEFORMAT PixelFormat = D3DX_SF_UNKNOWN;
	if (DestDesc.ddpfPixelFormat.dwRBitMask == 0x00ff0000u &&
		DestDesc.ddpfPixelFormat.dwGBitMask == 0x0000ff00u &&
		DestDesc.ddpfPixelFormat.dwBBitMask == 0x000000ffu)
	{
		if (DestDesc.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000u)
		{
			PixelFormat = D3DX_SF_A8R8G8B8;
		} else if (DestDesc.ddpfPixelFormat.dwRGBBitCount == 24) {
			PixelFormat = D3DX_SF_R8G8B8;
		} else if (DestDesc.ddpfPixelFormat.dwRGBBitCount == 32) {
			PixelFormat = D3DX_SF_X8R8G8B8;
		}
	}

	if (PixelFormat == D3DX_SF_UNKNOWN)
	{
		pSurface->Unlock( NULL );
		throw std::runtime_error( "Sorry, only D3DX_SF_A8R8G8B8, X8R8G8B8, and R8G8B8 destination surface supported" );
	}
		
	DWORD NumMipMaps = 0;

	Result = D3DXCreateTextureFromFile( m_pImpl->m_pDevice, 
		&Flags, &SourceWidth, &SourceHeight, &PixelFormat,
		0, &pSurface, &NumMipMaps, const_cast<char*>( ImageName.c_str() ), D3DX_FT_LINEAR );
#else
	LPDIRECT3DTEXTURE pSourceTexture = NULL;
	Result = D3DXCreateTextureFromFileEx( m_pImpl->m_pDevice, ImageName.c_str(), 
		DestDesc.Width, DestDesc.Height, 1, 0, DestDesc.Format,
		D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0,  NULL, NULL, &pSourceTexture );
#endif

	if (FAILED( Result ))
	{
#ifdef USE_DIRECTX_7
		pSurface->Unlock( NULL );
#else
		pSurface->UnlockRect();
#endif
		throw std::runtime_error( std::string( "D3DXCreateTextureFromFile failure for '" ) + ImageName + "'" );
	}

#ifndef USE_DIRECTX_7
	LPDIRECT3DSURFACE pSourceSurface = NULL;
	Result = pSourceTexture->GetSurfaceLevel( 0, &pSourceSurface );
	if (FAILED( Result ))
	{
		pSourceTexture->Release();
		pSurface->UnlockRect();
		throw std::runtime_error( "GetSurfaceLevel failure" );
	}
#endif

#ifdef USE_DIRECTX_7
	DDSURFACEDESC SourceDesc;
	Result = pSourceSurface->Lock( NULL, &SourceDesc, DDLOCK_READONLY, NULL );
#else
	D3DSURFACE_DESC SourceDesc;
	D3DLOCKED_RECT LockedSourceRect;
	Result = pSourceSurface->LockRect( &LockedSourceRect, NULL, D3DLOCK_READONLY );
	if (FAILED( Result ))
	{
		pSourceSurface->Release();
#ifdef USE_DIRECTX_7
		pSurface->Unlock( NULL );
#else
		pSourceTexture->Release();
		pSurface->UnlockRect();
#endif
		throw std::runtime_error( "LockRect failure" );
	}

	pSourceSurface->GetDesc( &SourceDesc );
#endif

#ifdef USE_DIRECTX_7
	if (DestDesc.dwWidth  != SourceDesc.dwWidth ||
		DestDesc.dwHeight != SourceDesc.dwHeight ||
		memcmp( &DestDesc.ddpfPixelFormat, &SourceDesc.ddpfPixelFormat, sizeof DestDesc.ddpfPixelFormat ))
#else
	if (DestDesc.Width  != SourceDesc.Width ||
		DestDesc.Height != SourceDesc.Height ||
		DestDesc.Format != SourceDesc.Format)
#endif
	{
#ifdef USE_DIRECTX_7
		pSourceSurface->Unlock( NULL );
#else
		pSourceSurface->UnlockRect();
#endif
		pSourceSurface->Release();		
#ifdef USE_DIRECTX_7
		pSurface->Unlock( NULL );
#else
		pSourceTexture->Release();
		pSurface->UnlockRect();
#endif
		throw std::runtime_error( "Unexpected D3DXCreateTextureFromFileEx behavior" );
	}

#ifdef USE_DIRECTX_7
	memcpy( DestDesc.lpSurface, SourceDesc.lpSurface, 
		DestDesc.dwSize );
#else

	int32 VertBlockCount = DestDesc.Height;
	switch (DestDesc.Format)
	{
		case D3DFMT_DXT1:
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			VertBlockCount = (VertBlockCount + 3) >> 2;
	}

	memcpy( LockedDestRect.pBits, LockedSourceRect.pBits, 
		LockedDestRect.Pitch * VertBlockCount );
#endif

#ifdef USE_DIRECTX_7
	pSourceSurface->Unlock( NULL );
#else
	pSourceSurface->UnlockRect();
#endif
	pSourceSurface->Release();		
#ifdef USE_DIRECTX_7
	pSurface->Unlock( NULL );
#else
	pSourceTexture->Release();
	pSurface->UnlockRect();
#endif

	CAd* pAd = new CAd( this );
	pAd->SetSurface( pSurface );
	pAd->SetID( AdID );
	pAd->SetVisible( VisibleImmediately );
	pAd->SetDestRect( DestRect );
	pAd->SetCustomParams( CustomParams );
	pAd->SetSize( Width, Height );
	if ( m_pImpl->m_RenderersUsedCount < m_pImpl->m_MaxRenderers )
	{
		m_pImpl->AssignRenderer( pAd );
	}
	m_pImpl->m_Ads.push_back( pAd );
	return pAd;
}

#ifdef WT_HOT_FULLSCREEN_ADS
#ifndef USE_DIRECTX_7
CAd* CAdManager::AddFullScreenAd ( const std::string& AdID, const std::string& CustomParams,
	                               bool VisibleImmediately, float32 ShowTime ,
								   bool FadeEffect, float32 FadeTime )
{
	CAd* pAd = new CAd( this );
	pAd->SetFullScreen( true );
	pAd->SetCustomParams( CustomParams );
	pAd->SetVisible( VisibleImmediately );
	pAd->SetID( AdID );
	CAdFullScreenInfo Info;
	Info.FadeEffect = FadeEffect;
	Info.FadeTime = FadeTime;
	Info.ShowTime = ShowTime;
	Info.ShowingTime = 0;
	pAd->SetFullScreenInfo( &Info );
	CHTMLRenderer* pRenderer = new CHTMLRenderer();
	pRenderer->Create ( m_pImpl->m_hParentWindow , 64 , 64 , pAd->GetCurrentURL() , false );
	pAd->m_pImpl->SetRenderer( pRenderer );

	LPDIRECT3DSURFACE pSurface = NULL;
	if ( m_pImpl->m_pFullScreenMaterial )
	{
		m_pImpl->m_pFullScreenMaterial->GetSurfaceLevel( 0 , &pSurface );
	}
	pAd->SetSurface( pSurface );
	pSurface->Release();

	m_pImpl->m_Renderers.push_back ( pRenderer );
	m_pImpl->m_Ads.push_back ( pAd );
	return pAd;
}
#endif
#endif

void CAdManagerImpl::RemoveRenderer( CHTMLRenderer* pRenderer )
{
	vector<CHTMLRenderer*>::iterator it = 
		find( m_Renderers.begin(), m_Renderers.end(), pRenderer );

	if (it != m_Renderers.end())
	{
		m_Renderers.erase( it );
		delete pRenderer;
	} else {
		assert( 0 );
	}
}

void CAdManager::RemoveAd ( CAd* pAd )
{
	vector<CAd*>::iterator Ad = find ( m_pImpl->m_Ads.begin() , m_pImpl->m_Ads.end() , pAd );
	if ( Ad != m_pImpl->m_Ads.end())
	{
#ifdef WT_HOT_FULLSCREEN_ADS
		if ( !pAd->IsFullScreen())
		{
			m_pImpl->m_RenderersUsedCount--;
		}
#endif
		if ( pAd->m_pImpl->GetRenderer())
		{
			m_pImpl->RemoveRenderer( pAd->m_pImpl->GetRenderer() );
		}
		delete pAd;
		m_pImpl->m_Ads.erase ( Ad );
	}
}

void CAdManager::RemoveAllAds ()
{
	//Expedite the destruction process by making non-blocking calls to each renderer
	for (uint32 i = 0; i < m_pImpl->m_Ads.size(); i++)
	{
		if (m_pImpl->m_Ads[i]->m_pImpl->GetRenderer() != NULL)
		{
			m_pImpl->m_Ads[i]->m_pImpl->GetRenderer()->Close();
		}
	}

	for (uint32 i = 0; i < m_pImpl->m_Ads.size(); i++)
	{
		if (m_pImpl->m_Ads[i]->m_pImpl->GetRenderer() != NULL)
		{
			m_pImpl->RemoveRenderer( m_pImpl->m_Ads[i]->m_pImpl->GetRenderer() );
		}

		delete m_pImpl->m_Ads[i];
	}
	m_pImpl->m_Ads.clear();
	m_pImpl->m_RenderersUsedCount = 0;
}

CAd* CAdManager::GetAd ( uint32 AdIndex ) const
{
	if ( AdIndex < 0 || AdIndex >= m_pImpl->m_Ads.size() )
	{
		return NULL;
	} else {
		return m_pImpl->m_Ads[AdIndex];
	}
}

uint32 CAdManager::GetAdCount() const
{
	return static_cast<uint32>( m_pImpl->m_Ads.size() );
}

bool CAdManagerImpl::FreeRenderer()
{
	assert( m_RenderersUsedCount > 0 );

	CAd* pOldestAd = NULL;
	float32 OldestTime = 0.0f;

	for (uint32 i = 0; i < m_Ads.size(); ++i)
	{
		CAd* pAd = m_Ads[i];
	
#ifdef WT_HOT_FULLSCREEN_ADS
		//skip over full screen ads
		if (pAd->IsFullScreen())
		{
			continue;
		}
#endif

		bool const AdNeedsUpdate = pAd->IsVisible() && (!pAd->m_pImpl->m_Static || !pAd->m_pImpl->m_FinalRenderCount);

		if (pAd->m_pImpl->m_pRenderer != NULL)
		{
			if (!AdNeedsUpdate)
			{
				pOldestAd = pAd;
				break;
			} else if (pAd->GetTimeSinceFirstRender() >= OldestTime) {
				pOldestAd = pAd;
				OldestTime = pAd->GetTimeSinceFirstRender();
			}
		}
	}

	if (pOldestAd == NULL)
	{
		return false;
	}

	CHTMLRenderer* pRenderer = pOldestAd->m_pImpl->GetRenderer();
	pOldestAd->m_pImpl->SetRenderer( NULL );
	RemoveRenderer( pRenderer );
	m_RenderersUsedCount--;
	return true;
}

void CAdManagerImpl::AssignRenderer( CAd* pAd )
{
	assert( m_RenderersUsedCount < m_MaxRenderers );
	assert( pAd->m_pImpl->GetRenderer() == NULL );

	CHTMLRenderer* pRenderer = new CHTMLRenderer();
	pRenderer->Create( m_hParentWindow, pAd->GetWidth(), pAd->GetHeight(), pAd->GetCurrentURL(), true );
	pAd->m_pImpl->m_RenderCount = 0;
	pAd->m_pImpl->SetRenderer( pRenderer );
	m_Renderers.push_back( pRenderer );
	m_RenderersUsedCount++;
}

void CAdManager::SetMaxRenderers ( uint32 Value )
{
	m_pImpl->m_MaxRenderers = Value;

	//Remove one render at a time based on the oldest rendering ad, until we have
	//satisfied the max renderer value.
	while (m_pImpl->m_RenderersUsedCount > Value)
	{
		m_pImpl->FreeRenderer();
	}
}

uint32 CAdManager::GetMaxRenderers() const
{
	return m_pImpl->m_MaxRenderers;
}

#ifdef WT_HOT_FULLSCREEN_ADS
#ifndef USE_DIRECTX_7
void CAdManagerImpl::HandleFullScreenAdFrameMove ( CAd* pAd, CHTMLRenderer* pRenderer , float32 ElapsedTime )
{
	if ( !pAd->IsVisible() )
	{
		if ( pRenderer && pRenderer->IsPageError() && pAd->GetOfflineURL() != "" )
			pRenderer->Navigate ( pAd->GetOfflineURL() );
		if ( pAd->IsComplete() )
			pAd->SetVisible(true);
		else
			return;
	}
	CAdFullScreenInfo Info;
	if ( !pAd->GetFullScreenInfo(&Info) )
	{
		m_pAdManager->RemoveAd ( pAd );
		m_pFullScreenSprite->SetColor ( 0x00ffffff );
		return;
	}
	//advance the clock on the ad
	Info.ShowingTime += ElapsedTime;
	if ( !Info.FadeEffect)
	{
		if ( Info.ShowingTime <= Info.ShowTime )
			//handle showing
			m_pFullScreenSprite->SetColor ( 0xffffffff );
		else
		{
			//ad finished showing, auto remove it.
			m_pAdManager->RemoveAd ( pAd );
			m_pFullScreenSprite->SetColor ( 0x00ffffff );
			return;
		}
	}
	else
	{
		if (  Info.ShowingTime < Info.FadeTime )
		{
			//handle fade in
			D3DCOLOR Color = 0xffffffff;
			if ( Info.FadeTime > 0 )
			{
				float32 FadeValue = Info.ShowingTime * 255.0f / Info.FadeTime;
				Color = D3DCOLOR_ARGB ( Float32ToInt32(FadeValue),0xff,0xff,0xff );
			}
			m_pFullScreenSprite->SetColor ( Color );
		}
		else if (Info.ShowingTime >= Info.FadeTime &&
				 Info.ShowingTime < Info.ShowTime - Info.FadeTime)
		{
			m_pFullScreenSprite->SetColor ( 0xffffffff );
		}
		else if ( Info.ShowingTime > Info.ShowTime - Info.FadeTime &&
				  Info.ShowingTime <= Info.ShowTime )
		{
			//handle fade out
			D3DCOLOR Color = 0xffffffff;
			if ( Info.FadeTime > 0 )
			{
				float32 FadeValue = (Info.ShowTime - Info.ShowingTime) * 255.0f / Info.FadeTime;
				Color = D3DCOLOR_ARGB ( Float32ToInt32(FadeValue),0xff,0xff,0xff );
			}
			m_pFullScreenSprite->SetColor ( Color );
		}
		else
		{
			//ad finished showing, auto remove it.
			m_pAdManager->RemoveAd ( pAd );
			m_pFullScreenSprite->SetColor ( 0x00ffffff );
			return;
		}
		pAd->SetFullScreenInfo(&Info);
	}

    LPDIRECT3DSURFACE pBackBuffer = NULL;
#ifdef USE_DIRECTX_8
	if ( FAILED ( m_pDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer )))
	{
		return;
	}
#else
	if ( FAILED ( m_pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer )))
	{
		return;
	}
#endif

	D3DSURFACE_DESC SurfaceDesc;
	pBackBuffer->GetDesc ( &SurfaceDesc );
	if ( SurfaceDesc.Width != pRenderer->GetWidth() || SurfaceDesc.Height != pRenderer->GetHeight())
	{
		pRenderer->ResizeClient( SurfaceDesc.Width , SurfaceDesc.Height );
		m_pFullScreenSprite->SetSize( (float32) SurfaceDesc.Width , (float32) SurfaceDesc.Height);
		pAd->SetSurface ( NULL );
		SafeRelease ( m_pFullScreenMaterial );
#ifdef USE_DIRECTX_8
		m_pDevice->CreateTexture ( SurfaceDesc.Width, SurfaceDesc.Height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_pFullScreenMaterial);
#else
		m_pDevice->CreateTexture ( SurfaceDesc.Width, SurfaceDesc.Height, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_pFullScreenMaterial, NULL );
#endif
		if ( m_pFullScreenMaterial )
		{
		    LPDIRECT3DSURFACE pSurface = NULL;
			m_pFullScreenSprite->SetTexture ( m_pFullScreenMaterial );
			m_pFullScreenMaterial->GetSurfaceLevel ( 0 , &pSurface );
			pAd->SetSurface ( pSurface );
			pSurface->Release();
		}
	}
	SafeRelease( pBackBuffer );
}

void CAdManagerImpl::CreateFullScreenAdResources ()
{
	m_pFullScreenSprite = new CFullScreenSprite();
	m_pFullScreenSprite->SetPosition(0,0);
	m_pFullScreenSprite->SetColor ( 0x00ffffff );
#ifdef USE_DIRECTX_8
	if ( FAILED(m_pDevice->CreateTexture ( 64, 64, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_pFullScreenMaterial)))
#else
	if ( FAILED(m_pDevice->CreateTexture ( 64, 64, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_pFullScreenMaterial, NULL )))
#endif
		return;
	m_pFullScreenSprite->SetTexture(m_pFullScreenMaterial );
}
#endif
#endif

void CAdManager::OnFrameMove( float ElapsedTime , D3DXVECTOR3 const* CameraPosition )
{
	m_pImpl->m_TotalTime += ElapsedTime;

	for (uint32 i = 0; i < m_pImpl->m_Ads.size(); i++)
	{
		CAd* pAd = m_pImpl->m_Ads[i];
		if (pAd->IsVisible())
		{
			pAd->m_pImpl->m_TotalTimeShown += ElapsedTime;
		}

		CHTMLRenderer* pRenderer = pAd->m_pImpl->GetRenderer();
		if (pRenderer == NULL)
		{
			continue;
		}

#ifdef WT_HOT_FULLSCREEN_ADS
#ifndef USE_DIRECTX_7
		if ( pAd->IsFullScreen())
		{
			m_pImpl->HandleFullScreenAdFrameMove ( pAd, pRenderer, ElapsedTime );
			continue;
		}
#endif
#endif

		pAd->m_pImpl->OnFrameMove( m_pImpl->m_TotalTime, CameraPosition );
	}

	Sleep( 0 );
}

void DrawTextOnSurface( LPDIRECT3DSURFACE pSurface, std::string const& Text )
{
#ifndef USE_DIRECTX_8
	if (Text.empty())
	{
		return;
	}

	HDC hDC = NULL;
	RECT r;

	D3DSURFACE_DESC SurfaceDesc;
#ifdef USE_DIRECTX_7
	if (FAILED(pSurface->GetSurfaceDesc ( &SurfaceDesc )))
	{
		return;
	}
	SetRect( &r, 0, (SurfaceDesc.dwHeight - 60) / 2, SurfaceDesc.dwWidth, (SurfaceDesc.dwHeight - 24) / 2 + 60);
#else
	if (FAILED(pSurface->GetDesc ( &SurfaceDesc )))
	{
		return;
	}
	SetRect( &r, 0, (SurfaceDesc.Height - 60) / 2, SurfaceDesc.Width, (SurfaceDesc.Height - 24) / 2 + 60);
#endif

	if (FAILED( pSurface->GetDC( &hDC ) ))
	{
		return;
	}

	SetTextColor( hDC, RGB( 0xff, 0x00, 0x00 ) );
	SetBkMode( hDC, TRANSPARENT );
	DrawText( hDC, Text.c_str(), static_cast<int>( Text.size() ), &r, DT_CENTER | DT_WORDBREAK );
	pSurface->ReleaseDC( hDC );
#endif
}

static inline void AddWaterMark ( LPDIRECT3DSURFACE pSurface )
{
#ifdef WTPLATFORM_TRIAL
	// Simply "WildTangent\nHOT SDK\nTrial" enctyped with XOR 0x97
	static const uint8 TrialText[] = { 0xc0,0xfe,0xfb,0xf3,0xc3,0xf6,0xf9,0xf0,0xf2,0xf9,0xe3,0x9a,0xdf,0xd8,0xc3,0xb7,0xc4,0xd3,0xdc,0x9a,0xc3,0xe5,0xfe,0xf6,0xfb};
	static char DecryptedTrialText[sizeof TrialText + 1] = "";
	if ( DecryptedTrialText[0] == 0 )
	{
		for (int i = 0; i < sizeof TrialText; ++i)
		{
			DecryptedTrialText[i] = TrialText[i] ^ 0x97;
		}
		DecryptedTrialText[i] = 0;
	}

	DrawTextOnSurface( pSurface, DecryptedTrialText );
#endif
}

std::string CAdManagerImpl::GetFlashVersion()
{
	ShockwaveFlashObjects::IShockwaveFlash* pFlash = NULL;
	CoCreateInstance( __uuidof ( ShockwaveFlashObjects::ShockwaveFlash), NULL, CLSCTX_INPROC_SERVER,
	                  __uuidof ( ShockwaveFlashObjects::IShockwaveFlash ), (void**) &pFlash);
	if (pFlash )
	{
		long v = pFlash->FlashVersion();
		SafeRelease( pFlash );

		char buf[1024];
		sprintf( buf, "%x", v );
		return buf;
	}
	else
	{
		return "None";
	}
}

void CAdManager::SetOptimizationInterval( float32 Interval )
{
	if (m_pImpl->m_OptimizationInterval < 0.0f)
	{
		throw std::out_of_range( "Interval" );
	}

	m_pImpl->m_OptimizationInterval = Interval;
}

float32 CAdManager::GetOptimizationInterval( ) const
{
	return m_pImpl->m_OptimizationInterval;
}

void CAdManagerImpl::OptimizeRenderers()
{
	bool Done = false;
	while (!Done)
	{
		//Find the ad that most needs updating, and the ad that least needs updating.
		bool FreeARenderer = false;
		CAd* pOldestAd = NULL;
		float32 OldestTime = 0.0f;
		for (uint32 i = 0; i < m_Ads.size(); ++i)
		{
			CAd* pAd = m_Ads[i];

#ifdef WT_HOT_FULLSCREEN_ADS
			//skip over full screen ads and invisible ads
			if (pAd->IsFullScreen())
 			{
				continue;
			}
#endif

			bool const AdNeedsUpdate = pAd->IsVisible() && (!pAd->m_pImpl->m_Static || !pAd->m_pImpl->m_FinalRenderCount);

			//If an ad has a renderer and doesn't need it, then we should free up the renderer.
			if (pAd->m_pImpl->m_pRenderer && !AdNeedsUpdate)
			{
				FreeARenderer = true;
			}

			if (!pAd->m_pImpl->m_pRenderer && AdNeedsUpdate && pAd->GetTimeSinceLastRender() >= OldestTime) 
			{
				pOldestAd = pAd;
				OldestTime = pAd->GetTimeSinceLastRender();
			}
		}

		Done = true;

		//If an ad no longer needs updating, or if we are at the limit of the number of renderers
		//and an important ad needs updating, then free up a renderer.
		if (FreeARenderer || (pOldestAd != NULL && m_RenderersUsedCount >= m_MaxRenderers))
		{
			if (FreeRenderer())
			{
				Done = false;
			}
		}

		//If an important ad needs updating, and if the limit of the number of renderers is not exceeded,
		//then assign a renderer to the ad.
		if (pOldestAd != NULL && m_RenderersUsedCount < m_MaxRenderers)
		{
			AssignRenderer( pOldestAd );
			Done = false;
		}
	}
}

void CAdManager::OnFrameRender( float ElapsedTime , D3DXVECTOR3 const* CameraPosition )
{
	m_pImpl->m_TimeSinceOptimization += ElapsedTime;
	//Try to optimize only at the given intervals
	if ( m_pImpl->m_TimeSinceOptimization >= m_pImpl->m_OptimizationInterval )
	{
		m_pImpl->OptimizeRenderers();
		m_pImpl->m_TimeSinceOptimization = 0.0f;
	}

	for (uint32 i = 0; i < m_pImpl->m_Ads.size(); ++i)
	{
		CAd* Ad = m_pImpl->m_Ads[i];
		Ad->m_pImpl->UpdateMetrics ( m_pImpl->m_pDevice , ElapsedTime );
		if ( Ad->m_pImpl->GetRenderer() && Ad->GetSurface() && Ad->IsVisible() )
		{
			if (Ad->m_pImpl->GetRenderer()->IsUpdated() && 
				/*Should the ad be rendered?*/ Ad->m_pImpl->OnFrameRender( CameraPosition ))
			{
				LPDIRECT3DSURFACE pSurface = Ad->GetSurface();

				RECT DestRect;
				if (!Ad->GetDestRect( &DestRect ))
				{
					//If no explicit destination rectangle is specified, then
					//use the whole surface.
					DestRect.left = 0;
					DestRect.top = 0;

					D3DSURFACE_DESC Desc;

#ifdef USE_DIRECTX_7
					if (FAILED( pSurface->GetSurfaceDesc( &Desc ) ))
					{
						throw std::runtime_error( "IDirectDrawSurface7::GetSurfaceDesc failure" );
					}
					DestRect.right = Desc.dwWidth;
					DestRect.bottom = Desc.dwHeight;
#else
					if (FAILED( pSurface->GetDesc( &Desc ) ))
					{
#ifdef USE_DIRECTX_8
						throw std::runtime_error( "IDirect3DSurface8::GetDesc failure" );
#else
						throw std::runtime_error( "IDirect3DSurface9::GetDesc failure" );
#endif
					}
					DestRect.right = Desc.Width;
					DestRect.bottom = Desc.Height;
#endif
				}
				
				Ad->m_pImpl->GetRenderer()->RenderToSurface( pSurface, &DestRect );

				AddWaterMark( Ad->GetSurface() );

#ifndef NDEBUG
				//Write additional text to the surface as specified by the ad.
				//This is typically used to display error codes.
				DrawTextOnSurface( Ad->GetSurface(), Ad->m_pImpl->m_DisplayedText );
#endif

				Ad->m_pImpl->IncRenderCount();

				//If this is the first time the ad has been rendered with this renderer,
				//and the ad is in its final state, then record the time.
				if (Ad->GetRenderCount() == 1 && Ad->m_pImpl->m_FinalRenderCount)
				{
					Ad->SetTimeSinceFirstRender( 0.0f );
				}
				Ad->SetTimeSinceLastRender( 0.0f );

				assert( !Ad->IsError() );
				if (Ad->IsComplete()) //Why would this fail?
				{
					if (Ad->m_pImpl->m_State == CAdImpl::KStateTryOnline)
					{
						Ad->m_pImpl->SetState( CAdImpl::KStateOnline );
					} else if (Ad->m_pImpl->m_State == CAdImpl::KStateTryOffline) {
						Ad->m_pImpl->SetState( CAdImpl::KStateOffline );
					}
				}
			}

#ifdef WT_HOT_FULLSCREEN_ADS
#ifndef USE_DIRECTX_7
			if ( Ad->IsFullScreen() )
			{
				DWORD LightingRS,AlphaBlendRS;
				m_pImpl->m_pDevice->GetRenderState ( D3DRS_LIGHTING , &LightingRS );
				m_pImpl->m_pDevice->GetRenderState ( D3DRS_ALPHABLENDENABLE , &AlphaBlendRS );

				m_pImpl->m_pDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE , TRUE );
				m_pImpl->m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
				m_pImpl->m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
				m_pImpl->m_pDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

				m_pImpl->m_pFullScreenSprite->Render ( m_pImpl->m_pDevice );

				m_pImpl->m_pDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE , AlphaBlendRS );
				m_pImpl->m_pDevice->SetRenderState( D3DRS_LIGHTING, LightingRS );
			}
#endif
#endif
		}
	}
}

void CAdManager::TrackUsage( CAd* pAd )
{
	if (m_pImpl->m_pGameTrack == NULL)
	{
		//Ad tracking not enabled.
		return;
	}

	if (pAd != NULL)
	{
		pAd->m_pImpl->TrackAdUsage( *m_pImpl->m_pGameTrack );
	}
	else
	{
		for ( uint32 i=0; i < m_pImpl->m_Ads.size(); i++)
		{
			m_pImpl->m_Ads[i]->m_pImpl->TrackAdUsage( *m_pImpl->m_pGameTrack );
		}
	}
}

void CAdManager::SetRenderThreadPriority( int Priority )
{
	if (m_pImpl->m_RenderThreadPriority == Priority)
	{
		return;
	}
	m_pImpl->m_RenderThreadPriority = Priority;
	for (uint32 i = 0; i < m_pImpl->m_Renderers.size(); ++i)
	{
		m_pImpl->m_Renderers[i]->SetRenderThreadPriority( Priority );
	}
}

int CAdManager::GetRenderThreadPriority() const
{
	return m_pImpl->m_RenderThreadPriority;
}

//--------------------------------------------
//Checks if ads should be displayed
//Returns true if ads should be displayed, false otherwise
//--------------------------------------------
/*static*/ bool WTCall CAdManager::ShouldAdsBeDisplayed()
{
	WT::Application::CRuntimeAttribute RuntimeAttribute;
	std::string LocationID = RuntimeAttribute.GetString( "WTDRMLicenseGUID" );

	LocationID = "{" + LocationID + "}";
	LMSetDefaultLocation(LocationID.c_str());

	return LMGetDisplayAds() != 0;
}


} //namespace HOT
} //namespace WT
