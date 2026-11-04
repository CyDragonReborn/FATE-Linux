// ----------------------------------------------------------------------------
//
// AdImpl.h
//
// Copyright (C) 2006 WildTangent, Inc. 
// All Rights Reserved
//
// ----------------------------------------------------------------------------

#pragma once

// ----------------------------------------------------------------------------
// Include Files
// ----------------------------------------------------------------------------

#include "WT/System/Platform.h"
#include "WT/HOT/Ad.h"
#include "WT/GameTrack/GameTrack.h"
#include "WT/Graphics/HTMLRenderer.h"
#include <string>

//#define WT_HOT_FULLSCREEN_ADS

#ifdef USE_DIRECTX_7
#include "d3dx.h"
#elif defined USE_DIRECTX_8
#include "d3dx8.h"
#else
#include "d3dx9.h"
#endif

namespace WT
{
namespace HOT
{

class CAdManager;

class CAdImpl
{
public:
	CAdImpl( CAd& Ad, CAdManager* pManager );
	~CAdImpl();

	void SetRenderer ( Graphics::CHTMLRenderer* Renderer );
	Graphics::CHTMLRenderer* GetRenderer();

#ifdef USE_DIRECTX_7
	void UpdateMetrics( IDirect3DDevice7* pDevice, float32 ElapsedTime );
#elif defined USE_DIRECTX_8
	void UpdateMetrics( IDirect3DDevice8* pDevice, float32 ElapsedTime );
#else
	void UpdateMetrics( IDirect3DDevice9* pDevice, float32 ElapsedTime );
#endif

	void TrackAdUsage( Tracking::CGameTrack& GameTrack );
	void CalculateUpdateIntervalBasedOnCamera( D3DXVECTOR3 const* CameraPosition );
	void CalculateRenderIntervalBasedOnCamera( D3DXVECTOR3 const* CameraPosition );
	void OnFrameMove( float64 TotalTime, D3DXVECTOR3 const* CameraPosition  );
	bool OnFrameRender ( D3DXVECTOR3 const* CameraPosition  );
	void IncRenderCount();

	CAd*						m_pAd;
	CAdManager*					m_pManager;
	std::string					m_ID;
	uint32						m_Width;
	uint32						m_Height;
	std::string					m_CustomParams;
	std::string					m_OfflineURL;
#ifdef USE_DIRECTX_7
	LPDIRECTDRAWSURFACE7		m_pSurface;
#elif defined USE_DIRECTX_8
	LPDIRECT3DSURFACE8			m_pSurface;
#else
	LPDIRECT3DSURFACE9			m_pSurface;
#endif
	float32						m_UpdateInterval;
	float32						m_RenderInterval;
	EAdPolicy::AdPolicy			m_UpdatePolicy;
	EAdPolicy::AdPolicy			m_RenderPolicy;
	Graphics::CHTMLRenderer*	m_pRenderer;
	ULONG						m_ColorKey;
	float64						m_TimeOfFirstRender;
	float64						m_TimeOfLastRender;
	RECT*						m_DestRect;
	CAd3DInfo*					m_3DInfo;
	float64						m_TotalTimeShown;
	float64						m_ActualTimeViewed;
	int32						m_ViewCount;
	float32						m_Metrics[EAdMetric::Last];
	uint32						m_RenderCount; //Number of times rendered
	uint32						m_FinalRenderCount; //Number of times rendered, in the final ad state (online/offline)

	//A callback used to intercept window.external.SetEvent() calls
	System::CCallback2<System::CHTMLBase*,DISPPARAMS const&>* m_EventCallback;

	CAdUserData* m_pUserData;

	enum EState
	{
		KStateTryOnline, //Try online URL
		KStateOnline, //Using online URL
		KStateTryOffline, //Try offline URL
		KStateOffline, //Using offline URL
		KStateCount
	};

	EState m_State;
	void SetState( EState State );

	bool						m_Visible	  : 1;
	bool						m_Transparent : 1;
	bool						m_Static	  : 1;
#ifdef WT_HOT_FULLSCREEN_ADS
	bool						m_FullScreen;
	CAdFullScreenInfo*			m_FullScreenInfo;
#endif

#ifndef NDEBUG
	//Text to display on the ad; typically used to report errors
	std::string					m_DisplayedText;

	//Add a line of text to the debug text displayed on the ad surface
	void AddDisplayedTextLine( std::string const& Line );
#endif

protected:
	//React to a window.external.SetEvent() call. Params contains all of the arguments, in reverse order.
	void OnEvent( System::CHTMLBase* pBase, DISPPARAMS const& Params );

	//Set an ad info name/value pair. Typically this occurs due to a window.external.SetEvent() call.
	void SetAdInfo( std::wstring const& Name, std::wstring const& Value );
};

} //namespace HOT
} //namespace WT
