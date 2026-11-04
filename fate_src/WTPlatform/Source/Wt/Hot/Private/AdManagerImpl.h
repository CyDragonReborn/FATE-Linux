// ----------------------------------------------------------------------------
//
// AdManagerImpl.h
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
#include <string>
#include "WT/HOT/Ad.h"
#include "WT/System/Callback.h"
#include "WT/Graphics/HTMLRenderer.h"

#ifdef USE_DIRECTX_7
#include "d3dx.h"
#elif defined USE_DIRECTX_8
#ifdef WT_HOT_FULLSCREEN_ADS
#include "WT/HOT/Private/FullScreenSprite.h"
#endif
#include "d3dx8.h"
#else
#ifdef WT_HOT_FULLSCREEN_ADS
#include "WT/HOT/Private/FullScreenSprite.h"
#endif
#include "d3dx9.h"
#endif

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

namespace WT
{
namespace HOT
{

class CAdManagerImpl
{
public:

	CAdManagerImpl( CAdManager* pAdManager, IDirect3DDevice* pDevice, HWND hParentWindow,
		std::string const& ProductCodeName, std::string const& DPCodeName,
		bool UseTracking );
	~CAdManagerImpl();

	void HandleAdUpdate( CAd* pAd , Graphics::CHTMLRenderer* pRenderer , D3DXVECTOR3 const* CameraPosition );
	bool HandleAdRender( CAd* pAd , Graphics::CHTMLRenderer* pRenderer , D3DXVECTOR3 const* CameraPosition );
	void OptimizeRenderers();
	void RemoveRenderer( Graphics::CHTMLRenderer* pRenderer );

#ifdef WT_HOT_FULLSCREEN_ADS
	void InitFullScreenSpriteVB();
	void RenderFullScreenSprite();
#endif

	static std::string GetFlashVersion();

	bool FreeRenderer();
	void AssignRenderer( CAd* pAd );

	CAdManager* m_pAdManager;

	IDirect3DDevice* m_pDevice;

#if defined(WT_HOT_FULLSCREEN_ADS) && !defined(USE_DIRECTX_7)
	void HandleFullScreenAdFrameMove ( CAd* pAd, Graphics::CHTMLRenderer* pRenderer , float32 ElapsedTime );
	void CreateFullScreenAdResources ( );
	IDirect3DTexture*  m_pFullScreenMaterial;
	CFullScreenSprite* m_pFullScreenSprite;
#endif

	HWND		m_hParentWindow;
	
	std::vector<Graphics::CHTMLRenderer*> m_Renderers;
	uint32		m_MaxRenderers;
	uint32		m_RenderersUsedCount;

	std::vector<CAd*> m_Ads;
	std::string m_ProductCodeName;
	std::string m_DPCodeName;
	float64		m_TotalTime;
	float32		m_TimeSinceOptimization;
	float32		m_OptimizationInterval;
	std::string m_FlashVersion;
	int			m_RenderThreadPriority;

	Tracking::CGameTrack* m_pGameTrack;

	friend CAdImpl;
};

} //namespace HOT
} //namespace WT
