//
// HTMLRenderer.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// The CHTMLRenderer class provides functionality for rendering HTML using IE to a DirectX surface
//

#pragma once

#include <atlbase.h>
#include <atlwin.h>
#include <ExDispid.h>

#ifdef USE_DIRECTX_7
#include <d3d.h>
#include <ddraw.h>
#include <d3dx.h>
#elif defined USE_DIRECTX_8
#include "d3d8.h"
#else
#include "d3d9.h"
#endif

#include <vector>
#include "WT/System/Timer2.h"
#include "WT/System/HTMLBase.h"

namespace WT
{

namespace Graphics
{

/// Render HTML into a surface using Internet Explorer

class CHTMLRenderer : public System::CHTMLBase
{
public:

	CHTMLRenderer();
	virtual ~CHTMLRenderer();

	///Construct the renderer, this will also create the embedded Internet Explorer control window
	///which is required for rendering - without calling this the rendering will not work
	void Create( HWND hParentWindow , int Width , int Height , const std::string& URL = "about:blank" ,
	             bool Async = false , const std::vector<std::string>& Tags = std::vector<std::string>() );

	//Issue a message to close the internal thread. This can be used to expedite the destruction 
	//of multiple CHTMLRenderer's without the blocking effects of triggering the full destructor 
	//for each CHTMLRenderer in succession. It's recommend to trigger destruction (e.g. by 
	//calling delete) shortly after calling Close().
	void Close();

	///Release any allocated resources with this renderer
	void Destroy();

#ifdef USE_DIRECTX_7

	///Render the currently showing HTML to the provided DirectX surface, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToSurface ( LPDIRECTDRAWSURFACE7 pSurface , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the currently showing HTML to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
//	void RenderToTexture ( LPDIRECT3DTEXTURE pTexture , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the specified HTML element to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface
//	void RenderElementToTexture ( LPDIRECT3DTEXTURE pTexture , const std::string& ElementName , RECT* DestRect = NULL );

#elif defined USE_DIRECTX_8

	///Render the currently showing HTML to the provided DirectX surface, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToSurface ( LPDIRECT3DSURFACE8 pSurface , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the currently showing HTML to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToTexture ( LPDIRECT3DTEXTURE8 pTexture , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the specified HTML element to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface
	void RenderElementToTexture ( LPDIRECT3DTEXTURE8 pTexture , const std::string& ElementName , RECT* DestRect = NULL );

#else

	///Render the currently showing HTML to the provided DirectX surface, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToSurface ( LPDIRECT3DSURFACE9 pSurface , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the currently showing HTML to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToTexture ( LPDIRECT3DTEXTURE9 pTexture , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the specified HTML element to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface
	void RenderElementToTexture ( LPDIRECT3DTEXTURE9 pTexture , const std::string& ElementName , RECT* DestRect = NULL );

#endif

	///Render the currently showing HTML to the provided device context, pass NULL for the destination rect
	///to draw into the entire DC
	void RenderToDC ( HDC hDC , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the specified HTML element to the provided device context, pass NULL for the destination rect
	///to draw into the entire DC
	void RenderElementToDC ( HDC hDC , const std::string& ElementName , RECT* DestRect = NULL );

	///For an async rendering of the HTML, pause on going background rendering
	void PauseUpdating();

	///For an async rendering of the HTML, resume on going background rendering
	void ResumeUpdating();

	///Set the HTML page size (in pixels)
	virtual void ResizeClient( uint32 Width , uint32 Height );

	///Set the HTML page width
	void SetWidth ( uint32 Width );

	///Set the HTML page height
	void SetHeight ( uint32 Height );

	///Get the HTML page width
	uint32 GetWidth () const;

	///Get the HTML page height
	uint32 GetHeight () const;

	///Navigate the HTML to a given URL
	virtual void Navigate ( const std::string& URL );

	//Stop pending navigation and any dynamic elements
	virtual void Stop();

	///Refresh the page
	virtual void Refresh();

	///Return the current URL displayed in the browser
	virtual std::string GetURL() const;

	///Set the background render update interval (in milliseconds)
	void SetRenderInterval ( float32 RenderInterval );

	///Get the background render update interval (in milliseconds)
	float32 GetRenderInterval() const;

	///Turn on transparency processing (using the color key) on the rendering.
	///in order for that to work, the rendering has to be done into an ARGB surface
	///and alpha blending has to be turned on as well. This will not work with rendering
	///directly into a DC, as it doesn't have an alpha channel
	void SetTransparent ( bool Transparent );

	///Return true if transparency is turned on
	bool GetTransparent () const;

	///Set the color key for the transparency processing. Use the form 0xffrrggbb.
	void SetColorKey ( ULONG ColorKey );

	///Get the color key used for the transparency processing
	ULONG GetColorKey () const;

	///Returns the actual IE window used for rendering the contents
	virtual HWND GetWindow();

	///process mouse move messages (relative to the window itself)
	void HandleMouseMove ( int X , int Y , WPARAM wParam );

	///process mouse down messages (relative to the window itself)
	void HandleMouseDown ( int X , int Y , WPARAM wParam );

	///process mouse Up messages (relative to the window itself)
	void HandleMouseUp ( int X , int Y , WPARAM wParam );

	void ReplaceAPIs();

	// Set the background render thread priority (needs to operate in async mode
	// for that.
	// see the SetThreadPriority windows call for values.
	void SetRenderThreadPriority( int ThreadPriority );

	// Get the background render thread priority (needs to operate in async mode
	// for that. see the SetThreadPriority windows call for values.
	int GetRenderThreadPriority() const;

	// Set whether the internal image has been updated from the web page.
	void SetUpdated( bool Updated );

	// Set whether the internal image has been updated from the web page. If true, then rendering
	// to a surface is appropriate.
	bool IsUpdated() const;

	//The default value, in milliseconds, for the update period.
	static int32 const KDefaultUpdatePeriodMS = 20;

	// Set the update period in milliseconds. The internal image will be updated no more
	// frequently than specified by this period. Larger values decrease CPU consumption.
	void SetUpdatePeriodMS( int32 PeriodMS );

	// Get the update period in milliseconds. The internal image is updated no more
	// frequently than specified by this period.
	int32 GetUpdatePeriodMS() const;

private:
	bool GetRectOfElement ( const std::string& ElementName , RECT& ElementRect );
	void CreateBackgroundBitmap();
	void FreeBackgroundBitmap();
	void RenderBackgroundImage();
	void CreateResources();
	void FreeResources();
	void UpdateBackgroundImage();

	static DWORD WINAPI RenderThreadRoutine ( LPVOID lpParameter );
	DWORD RenderThreadRoutine();

	HDC m_hDC;
	void* m_BitmapBits;
	BITMAPINFO m_BitmapInfo;
	HBITMAP m_hBitmap, m_hOldBitmap;

	uint32 m_Width;
	uint32 m_Height;
	IViewObject* m_ViewObject;
	std::string m_URL;
	std::vector<std::string> m_Tags;
	std::vector<RECT> m_TagRects;
	HWND m_hParentWindow;
	DWORD m_RenderThreadID;
	HANDLE m_hRenderThread;
	WT::System::CTimer2 m_RenderTimer;
	float32 m_RenderInterval;
	float64 m_TimeSinceLastRender;
	bool m_Transparent;
	ULONG m_ColorKey;
	HANDLE m_hRenderMutex;
	int32 m_UpdatePeriodMS;

	HANDLE									m_hBrowserEvent;
	__declspec( align( 4 ) ) LONG volatile	m_BrowserCommandRefresh;
	__declspec( align( 4 ) ) LONG volatile	m_BrowserCommandNavigate;
	__declspec( align( 4 ) ) LONG volatile	m_BrowserCommandStop;

	bool m_Async;
	bool m_Updating;
	bool m_Updated; //false initially, true once the internal image has been updated from the web page.
	bool m_Stopped;
};


}
}