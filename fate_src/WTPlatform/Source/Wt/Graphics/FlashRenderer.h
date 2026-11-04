//
// FlashRenderer.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// The CFlashRenderer class provides functionality for rendering HTML using IE to a DirectX surface
//

#pragma once

#include "d3d9.h"
#include <atlbase.h>
#include <atlwin.h>
#include <string>
#include <queue>
#include "WT/System/Timer.h"

#include "Flash8.tlh"

namespace WT
{

namespace Graphics
{

struct CFlashEvent;
class CFlashEvents;

/// Render Flash content into a surface using the Flash player

class CFlashRenderer
{
public:

	CFlashRenderer();
	virtual ~CFlashRenderer();

	///Construct the renderer, this will also create the embedded Internet Explorer control window
	///which is required for rendering - without calling this the rendering will not work
	void Create (  HWND hParentWindow , int Width , int Height , const std::string& Movie = "" ,
	               bool Async = false );

	///Release any allocated resources with this renderer
	void Destroy();

	///Render the currently showing flash movie to the provided DirectX texture, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToTexture ( LPDIRECT3DTEXTURE9 pTexture , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the currently showing flash movie to the provided DirectX surface, pass NULL for the destination rect
	///to draw into the entire surface, Pass NULL for the source rect to render the entire source page
	void RenderToSurface ( LPDIRECT3DSURFACE9 pSurface , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///Render the currently showing HTML to the provided device context, pass NULL for the destination rect
	///to draw into the entire DC
	void RenderToDC ( HDC hDC , RECT* DestRect = NULL , RECT* SourceRect = NULL );

	///For an async rendering of flash, pause on going background rendering
	void PauseUpdating();

	///For an async rendering of flash, resume on going background rendering
	void ResumeUpdating();

	///Set the flash page size (in pixels)
	void SetSize ( uint32 Width , uint32 Height );

	///Set the flash page widht
	void SetWidth ( uint32 Width );

	///Set the flash page height
	void SetHeight ( uint32 Height );

	///Get the flash page width
	uint32 GetWidth () const;

	///Get the flash page height
	uint32 GetHeight () const;

	///Retrieve the flash interface associated with this renderer
	ShockwaveFlashObjects::IShockwaveFlash* GetFlash();

	///Set flash to play a given movie
	void SetMovie ( const std::string& Movie );

	///Return the currnet URL displayed in the browser
	std::string GetMovie();

	///Returns the actual flash window used for rendering the contents
	HWND GetWindow();

	///process mouse move messages (relative to the window itself)
	void HandleMouseMove ( int X , int Y , WPARAM wParam );

	///process mouse down messages (relative to the window itself)
	void HandleMouseDown ( int X , int Y , WPARAM wParam );

	///process mouse Up messages (relative to the window itself)
	void HandleMouseUp ( int X , int Y , WPARAM wParam );

	///Play the movie
	void Play ( );
	
	///Stop playing the movie
	void Stop ( );

	///Move to the start of the movie
	void Back ( );

	///Move forward on the movie
	void Forward ( );

	///Rewind the movie
	void Rewind ( );

	///Stop playing the movie
	void StopPlay ( );

	///Goto a specific frame in the movie
	void GotoFrame ( long FrameNum );

	///Return the current frame playing
	long GetCurrentFrame ();

	///Return true if the movie is currently playing
	bool IsPlaying ();

	///Get the percent of the part of the movie that is loaded
	long GetPercentLoaded ();

	///Ask is loaded on a specific frame
	bool IsFrameLoaded ( long FrameNum );

	///Get the flash version currently installed
	long GetFlashVersion ();

	///Set movie playing quality
	void SetQuality( int Quality);

	///Get Movie playing quality
	int GetQuality();

	///Set the background render update interval (in milliseconds)
	void SetRenderInterval ( int64 RenderInterval );

	///Get the background render update interval (in milliseconds)
	int64 GetRenderInterval();

	///Returns true if an events from flash happened, return the command and the args
	///use getURL("FSCommand:<Command>","<Args>") to trigger events from flash, these
	///will queue up in the flash render.
	bool GetNextEvent ( std::string& Command , std::string& Args );

	///Turn on trasparency processing (using the color key) on the rendering.
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

private:

	void CreateBackgroundBitmap();
	void FreeBackgroundBitmap();
	void RenderBackgroundImage();
	void CreateResources();
	void FreeResources();
	void UpdateBackgroundImage();
	static DWORD WINAPI RenderThreadRoutine ( LPVOID lpParameter );
	HDC m_hDC;
	void* m_BitmapBits;
	BITMAPINFO m_BitmapInfo;
	HBITMAP m_hBitmap , m_hOldBitmap;
	uint32 m_Width;
	uint32 m_Height;
	CAxWindow m_Window;
	IViewObject* m_ViewObject;
	ShockwaveFlashObjects::IShockwaveFlash* m_Flash;
	std::string m_Movie;
	HWND m_hParentWindow;
	bool m_Async;
	bool m_Updating;
	DWORD m_RenderThreadID;
	HANDLE m_hRenderThread;
	CFlashEvents* m_Events;
	WT::System::CTimer m_RenderTimer;
	int64 m_RenderInterval;
	int64 m_TimeSinceLastRender;
	bool m_Transparent;
	ULONG m_ColorKey;

};


}
}