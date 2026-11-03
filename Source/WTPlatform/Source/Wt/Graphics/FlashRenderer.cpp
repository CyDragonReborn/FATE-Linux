//
//	FlashRenderer.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "FlashRenderer.h"
#include <imagehlp.h>

#pragma comment( lib, "imagehlp" )

namespace WT
{
namespace Graphics
{


struct CFlashEvent
{
	CFlashEvent ( std::string _Command , std::string _Args )
	{
		Command = _Command;
		Args = _Args;
	}

	std::string Command;
	std::string Args;
};

// {6EE8BCB7-210E-4c8f-949D-864A8C0AD3D7}
DEFINE_GUID(CLSID_FlashEventsObject, 
0x6ee8bcb7, 0x210e, 0x4c8f, 0x94, 0x9d, 0x86, 0x4a, 0x8c, 0xa, 0xd3, 0xd7);

//This is an internal class for handling the events we get from flash

class ATL_NO_VTABLE CFlashEvents : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFlashEvents, &CLSID_FlashEventsObject>,
	public IDispEventImpl<0,CFlashEvents,&__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents),&__uuidof(ShockwaveFlashObjects::__ShockwaveFlashObjects),1,0>
{
public:

	CFlashEvents()
	{
		InitializeCriticalSection ( &m_QueueCriticalSection );
	}

	~CFlashEvents()
	{
		DeleteCriticalSection ( &m_QueueCriticalSection );
	}

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_SINK_MAP(CFlashEvents)
		SINK_ENTRY_EX(0,__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents) , 0x96, OnFSCommand)
//		SINK_ENTRY_EX(0,__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents) , 0x7a6, OnProgress)
//		SINK_ENTRY_EX(0,__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents) , 0xc5, OnFlashCall)
//		SINK_ENTRY_EX(0,__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents) , DISPID_READYSTATECHANGE, OnReadyStateChange)
	END_SINK_MAP()

	BEGIN_COM_MAP(CFlashEvents)
		COM_INTERFACE_ENTRY_IID(IID_IUnknown, CFlashEvents)
		COM_INTERFACE_ENTRY_IID(__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents), CFlashEvents)
	END_COM_MAP()

	STDMETHOD(OnFSCommand) ( BSTR Command, BSTR Args )
	{
		EnterCriticalSection ( &m_QueueCriticalSection );
		m_EventsQueue.push ( CFlashEvent ( CW2A(Command).m_szBuffer ,CW2A(Args).m_szBuffer ));
		LeaveCriticalSection ( &m_QueueCriticalSection );
		return S_OK;
	}

	bool GetNextEvent ( std::string& Command , std::string& Args )
	{
		EnterCriticalSection ( &m_QueueCriticalSection );
		if ( m_EventsQueue.empty())
		{
			LeaveCriticalSection ( &m_QueueCriticalSection );
			return false;
		}
		Command = m_EventsQueue.front().Command;
		Args =  m_EventsQueue.front().Args;
		m_EventsQueue.pop();
		LeaveCriticalSection ( &m_QueueCriticalSection );
		return true;
	}

/*    STDMETHOD(OnProgress) ( long percentDone )
	{
		return S_OK;
	}
	STDMETHOD(OnReadyStateChange) ( long newState )
	{
		return S_OK;
	}
	STDMETHOD(OnFlashCall)( BSTR request)
	{
		return S_OK;
	}
	*/
private:
	std::queue<CFlashEvent> m_EventsQueue;
	CRITICAL_SECTION m_QueueCriticalSection;
};

static const BYTE cPushOpCode = 0x68;
static PVOID sm_pvMaxAppAddr = NULL;

static BOOL ReplaceMethodInOneModule( PCSTR pszCalleeModName, PCSTR pszFuncName , PROC pfnNew, HMODULE hmodCaller ) 
{
	PROC pfnCurrent = GetProcAddress( ::GetModuleHandleA(pszCalleeModName) , pszFuncName );
	if ( pfnCurrent == NULL )
		return FALSE;

	BOOL bResult = FALSE;
	__try
	{
		ULONG ulSize;
		// Get the address of the module's import section
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = 
			(PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
			hmodCaller, 
			TRUE, 
			IMAGE_DIRECTORY_ENTRY_IMPORT, 
			&ulSize
			);
		// Does this module has import section ?
		if (pImportDesc == NULL)
			__leave;  
		// Loop through all descriptors and
		// find the import descriptor containing references to callee's functions
		while (pImportDesc->Name)
		{
			PSTR pszModName = (PSTR)((PBYTE) hmodCaller + pImportDesc->Name);
			if (stricmp(pszModName, pszCalleeModName) == 0) 
				break;   // Found
			pImportDesc++;
		} // while
		// Does this module import any functions from this callee ?
		if (pImportDesc->Name == 0)
			__leave;  
		// Get caller's IAT 
		PIMAGE_THUNK_DATA pThunk = 
			(PIMAGE_THUNK_DATA)( (PBYTE) hmodCaller + pImportDesc->FirstThunk );
		// Replace current function address with new one
		while (pThunk->u1.Function)
		{
			// Get the address of the function address
			PROC* ppfn = (PROC*) &pThunk->u1.Function;
			// Is this the function we're looking for?
			BOOL bFound = (*ppfn == pfnCurrent);
			// Is this Windows 9x
			if (!bFound && (*ppfn > sm_pvMaxAppAddr)) 
			{
				PBYTE pbInFunc = (PBYTE) *ppfn;
				// Is this a wrapper (debug thunk) represented by PUSH instruction?
				if (pbInFunc[0] == cPushOpCode) 
				{
					ppfn = (PROC*) &pbInFunc[1];
					// Is this the function we're looking for?
					bFound = (*ppfn == pfnCurrent);
				} // if
			} // if

			if (bFound) 
			{
				MEMORY_BASIC_INFORMATION mbi;
				::VirtualQuery(ppfn, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
				// In order to provide writable access to this part of the 
				// memory we need to change the memory protection
				if (FALSE == ::VirtualProtect(
					mbi.BaseAddress,
					mbi.RegionSize,
					PAGE_READWRITE,
					&mbi.Protect)
					)
					__leave;
				// Hook the function.
                *ppfn = *pfnNew;
				bResult = TRUE;
				// Restore the protection back
                DWORD dwOldProtect;
				::VirtualProtect(
					mbi.BaseAddress,
					mbi.RegionSize,
					mbi.Protect,
					&dwOldProtect
					);
				break;
			} // if
			pThunk++;
		} // while
	}
	__finally
	{
		// do nothing
	}
	// This function is not in the caller's import section
	return bResult;
}

static HWND g_hCurrentFlashWindow = NULL;

static BOOL WINAPI MyGetCursorPos( LPPOINT lpPoint )
{
	RECT r;
	GetWindowRect( g_hCurrentFlashWindow ,&r);
	lpPoint->x = r.left;
	lpPoint->y = r.top;
	return TRUE;
}

static HWND WINAPI MyWindowFromPoint( POINT Point )
{
	return g_hCurrentFlashWindow;
}

static HCURSOR WINAPI MySetCursor( HCURSOR hCursor )
{
	return NULL;
}

static HWND WINAPI MySetCapture( HWND hWnd )
{
	return NULL;
}

CFlashRenderer::CFlashRenderer() : m_Flash ( NULL ) , m_ViewObject ( NULL ) , m_Width(0) , m_Height(0)
{
	m_Async = false;
	m_hRenderThread = NULL;
	m_hDC = NULL;
	m_hOldBitmap = NULL;
	m_hBitmap = NULL;
	m_RenderThreadID = 0;
	m_hParentWindow = NULL;
	m_BitmapBits = NULL;
	m_Updating = true;
	m_Events = NULL;
	m_RenderInterval = 0;
	m_TimeSinceLastRender = 0;
	m_Transparent = false;
	m_ColorKey = 0xff000000u;
}

CFlashRenderer::~CFlashRenderer()	
{
	Destroy();
}

void CFlashRenderer::CreateBackgroundBitmap()
{
	HDC hDesktopDC = GetDC( GetDesktopWindow() );

	m_hDC = CreateCompatibleDC ( hDesktopDC );	

	memset ( &m_BitmapInfo , 0 , sizeof(BITMAPINFO) );
	m_BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.bmiHeader.biPlanes = 1;
	m_BitmapInfo.bmiHeader.biBitCount = 32;
	m_BitmapInfo.bmiHeader.biCompression = BI_RGB;
	m_BitmapInfo.bmiHeader.biWidth = m_Width;
	m_BitmapInfo.bmiHeader.biHeight = -(LONG) m_Height;
	
	m_hBitmap = CreateDIBSection ( m_hDC, &m_BitmapInfo, DIB_RGB_COLORS, &m_BitmapBits , NULL , 0 );
	m_hOldBitmap = (HBITMAP) SelectObject ( m_hDC, m_hBitmap );
	ReleaseDC ( GetDesktopWindow() , hDesktopDC );
}

DWORD WINAPI CFlashRenderer::RenderThreadRoutine ( LPVOID lpParameter )
{
	CFlashRenderer* This = (CFlashRenderer *) lpParameter;

	This->CreateResources();

	while (1)
	{
		MSG Msg;
		if ( !PeekMessage(&Msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			This->UpdateBackgroundImage();
		}
		if (GetMessage(&Msg, NULL, NULL, NULL))
		{
			TranslateMessage( &Msg );
			DispatchMessage( &Msg );
		}
		else
			break;
	}
	This->FreeResources();
	return 0;
}

void CFlashRenderer::UpdateBackgroundImage()
{
	if ( !m_Updating )
		return;
	//update the time that passed since the last render
	m_TimeSinceLastRender += m_RenderTimer.GetElapsedTime();

	//check if we need to update the background image based on the render interval
	if ( m_TimeSinceLastRender >= m_RenderInterval )
	{
		RenderBackgroundImage();
		m_TimeSinceLastRender = 0;
	}
}

void CFlashRenderer::Create ( HWND hParentWindow , int Width , int Height , const std::string& Movie , bool Async )
{
	m_Width = Width;
	m_Height = Height;
	m_hParentWindow = hParentWindow;
	m_Movie = Movie;
	m_Async = Async;

	CreateBackgroundBitmap();


	if ( Async )
	{
		m_hRenderThread = CreateThread ( NULL , NULL , RenderThreadRoutine , (LPVOID) this , NULL , &m_RenderThreadID );
	}
	else
	{
		CreateResources();
	}
}

void CFlashRenderer::CreateResources()
{
	RECT rc = {0,0,m_Width,m_Height};
	m_Window.Create ( m_hParentWindow , &rc , "ShockwaveFlash.ShockwaveFlash" , WS_CHILD );
	m_Window.QueryControl( &m_ViewObject );
	m_Window.QueryControl( &m_Flash );

	//replace all the API we need replacing in Flash

	ReplaceMethodInOneModule ( "user32.dll" , "GetCursorPos" , (PROC) MyGetCursorPos , GetModuleHandle("Flash8.ocx"));
	ReplaceMethodInOneModule ( "user32.dll" , "WindowFromPoint" , (PROC) MyWindowFromPoint , GetModuleHandle("Flash8.ocx"));
	ReplaceMethodInOneModule ( "user32.dll" , "SetCursor" ,(PROC) MySetCursor , GetModuleHandle("Flash8.ocx"));
	ReplaceMethodInOneModule ( "user32.dll" , "SetCapture" ,(PROC) MySetCapture , GetModuleHandle("Flash8.ocx"));

	CFlashEvents::CreateInstance ( (IUnknown **) &m_Events );
	if ( m_Events )
		m_Events->DispEventAdvise ( m_Flash , &__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents) );
	if ( m_Movie != "" )
		SetMovie ( m_Movie );
	m_RenderTimer.Start();
}

void CFlashRenderer::FreeResources()
{
	if ( m_ViewObject )
	{
		m_ViewObject->Release();
		m_ViewObject = NULL;
	}
	if ( m_Events )
	{
		m_Events->DispEventUnadvise ( m_Flash , &__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents) );
		m_Events->Release();
		m_Events = NULL;
	}
	if ( m_Flash )
	{
		m_Flash->Release();
		m_Flash = NULL;
	}
}

void CFlashRenderer::FreeBackgroundBitmap()
{
	if ( m_hOldBitmap )
	{
		SelectObject ( m_hDC , m_hOldBitmap );
		DeleteDC( m_hDC );
		m_hDC = NULL;
		m_hOldBitmap = NULL;
	}
	if ( m_hBitmap )
	{
		DeleteObject ( m_hBitmap );
		m_hBitmap = NULL;
	}
}

void CFlashRenderer::Destroy()
{
	if ( m_Async )
	{
		if ( m_hRenderThread )
		{
			PostThreadMessage ( m_RenderThreadID , WM_QUIT , 0 , 0 );
			WaitForSingleObject ( m_hRenderThread , INFINITE );
			CloseHandle ( m_hRenderThread );
			m_hRenderThread = NULL;
			m_RenderThreadID = 0;
		}
	}
	else
	{
		FreeResources();
	}
	if ( m_Window.IsWindow())
		m_Window.DestroyWindow();
}

void CFlashRenderer::PauseUpdating()
{
	m_Updating = false;
}

void CFlashRenderer::ResumeUpdating()
{
	m_Updating = true;
}

void CFlashRenderer::SetSize ( uint32 Width , uint32 Height )
{
	if ( m_Width == Width && m_Height == Height )
		return;
	if ( m_Window.IsWindow() )
		m_Window.ResizeClient ( Width , Height );
	m_Width = Width;
	m_Height = Height;
	FreeBackgroundBitmap();
	CreateBackgroundBitmap();
}

void CFlashRenderer::SetWidth ( uint32 Width )
{
	SetSize ( Width , m_Height );
}

void CFlashRenderer::SetHeight ( uint32 Height )
{
	SetSize ( m_Width , Height );
}

uint32 CFlashRenderer::GetWidth () const
{
	return m_Width;
}

uint32 CFlashRenderer::GetHeight () const
{
	return m_Height;
}

void CFlashRenderer::RenderBackgroundImage ()
{
	RECTL r;
	r.left = 0;
	r.top = 0;
	r.right = m_Width;
	r.bottom = m_Height;
	m_ViewObject->Draw ( DVASPECT_CONTENT , -1 , NULL , NULL , NULL , m_hDC , &r , NULL , NULL , NULL );
}

void CFlashRenderer::RenderToDC ( HDC hDC , RECT* DestRect , RECT* SourceRect )
{
	//first check to see if we have a valid IViewObject interface, if not, no need to proceed
	if ( !m_ViewObject )
		return;

	if ( !m_Async )
		RenderBackgroundImage();
	
	int DestX , DestY , DestWidth , DestHeight;
	int SourceX , SourceY , SourceWidth , SourceHeight;
	if ( DestRect == NULL )
	{
		DestX = DestY = 0;
		DestWidth = m_Width;
		DestHeight = m_Height;
	}
	else
	{
		DestX = DestRect->left;
		DestY = DestRect->top;
		DestWidth = DestRect->right - DestRect->left;
		DestHeight = DestRect->bottom - DestRect->top;
	}
	if ( SourceRect == NULL )
	{
		SourceX = SourceY = 0;
		SourceWidth = m_Width;
		SourceHeight = m_Height;
	}
	else
	{
		SourceX = SourceRect->left;
		SourceY = m_Height - SourceRect->bottom;
		SourceWidth = SourceRect->right - SourceRect->left;
		SourceHeight = SourceRect->bottom - SourceRect->top;
	}

	StretchDIBits ( hDC,
	                DestX, DestY, DestWidth, DestHeight,
					SourceX, SourceY, SourceWidth, SourceHeight,
					m_BitmapBits, &m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY );
}

void CFlashRenderer::RenderToSurface ( LPDIRECT3DSURFACE9 pSurface , RECT* DestRect , RECT* SourceRect )
{
	//first check to see if we have a valid IViewObject interface, if not, no need to proceed
	if ( !m_ViewObject )
		return;
	if ( m_Transparent )
	{
		if ( !m_Async )
			RenderBackgroundImage();

		D3DLOCKED_RECT LockedRect;
		if ( FAILED(pSurface->LockRect ( &LockedRect , NULL , 0 )))
			return;

		ULONG* DestBits = (ULONG *) LockedRect.pBits;
		ULONG* SourceBits = (ULONG *) m_BitmapBits;

		//calulate pitch in 4 bytes (format is argb)
		int Pitch = LockedRect.Pitch >> 2;
		uint32 x;
		ULONG ColorValue;

		//loop over all pixels and alpha all the color keyed ones

		for ( uint32 y = 0; y < m_Height; y++ )
		{
			for ( x = 0; x < m_Width; x++ )
			{
				ColorValue = SourceBits[x];
				if ( ColorValue == m_ColorKey )
					DestBits[x] = 0;
				else
					DestBits[x] = ColorValue;
			}
			SourceBits += Pitch;
			DestBits += Pitch;
		}

		pSurface->UnlockRect();
	}
	else
	{
		HDC hDC = NULL;
		if ( pSurface->GetDC( &hDC ) != S_OK )
		{
			pSurface->Release();
			return;
		}
		RenderToDC( hDC , DestRect , SourceRect );
		pSurface->ReleaseDC( hDC );
	}
}

void CFlashRenderer::RenderToTexture ( LPDIRECT3DTEXTURE9 pTexture , RECT* DestRect , RECT* SourceRect )
{
	//first check to see if we have a valid IViewObject interface, if not, no need to proceed
	if ( !m_ViewObject )
		return;

	IDirect3DSurface9* pSurface = NULL;
	if ( pTexture->GetSurfaceLevel ( 0 , &pSurface ) != S_OK )
		return;
	RenderToSurface ( pSurface , DestRect , SourceRect );
	pSurface->Release();
}

void CFlashRenderer::SetMovie ( const std::string& Movie )
{
	if ( !m_Flash )
		return;
	m_Movie = Movie;
	m_Flash->put_Movie(CComBSTR(Movie.c_str()));
}

ShockwaveFlashObjects::IShockwaveFlash* CFlashRenderer::GetFlash()
{
	return m_Flash;
}

std::string CFlashRenderer::GetMovie()
{
	return m_Movie;
}

HWND CFlashRenderer::GetWindow()
{
	if ( m_Window.IsWindow() )
		return m_Window.GetWindow(GW_CHILD);
	else
		return NULL;
}

void CFlashRenderer::HandleMouseMove ( int X , int Y , WPARAM wParam )
{
	g_hCurrentFlashWindow = GetWindow();
	if ( g_hCurrentFlashWindow )
		PostMessage ( g_hCurrentFlashWindow , WM_MOUSEMOVE , wParam , MAKELPARAM(X,Y));
}

void CFlashRenderer::HandleMouseDown ( int X , int Y , WPARAM wParam )
{
	g_hCurrentFlashWindow = GetWindow();
	if ( g_hCurrentFlashWindow )
		PostMessage ( g_hCurrentFlashWindow , WM_LBUTTONDOWN , wParam , MAKELPARAM(X,Y));
}

void CFlashRenderer::HandleMouseUp ( int X , int Y , WPARAM wParam )
{
	g_hCurrentFlashWindow = GetWindow();
	if ( g_hCurrentFlashWindow )
		PostMessage ( g_hCurrentFlashWindow , WM_LBUTTONUP , wParam , MAKELPARAM(X,Y));
}

void CFlashRenderer::Play ()
{
	if ( m_Flash )
		m_Flash->Play();
}

void CFlashRenderer::Stop ()
{
	if ( m_Flash )
		m_Flash->Stop();
}

void CFlashRenderer::Back ()
{
	if ( m_Flash )
		m_Flash->Back();
}

void CFlashRenderer::Forward ()
{
	if ( m_Flash )
		m_Flash->Forward();
}

void CFlashRenderer::Rewind ()
{
	if ( m_Flash )
		m_Flash->Rewind();
}

void CFlashRenderer::StopPlay ()
{
	if ( m_Flash )
		m_Flash->StopPlay();
}

void CFlashRenderer::GotoFrame ( long FrameNum )
{
	if ( m_Flash )
		m_Flash->GotoFrame ( FrameNum );
}

long CFlashRenderer::GetCurrentFrame ()
{
	if ( m_Flash )
		return m_Flash->CurrentFrame();
	else
		return -1;
}

bool CFlashRenderer::IsPlaying ()
{
	if ( m_Flash )
		return m_Flash->IsPlaying() == VARIANT_TRUE;
	else
		return false;
}

long CFlashRenderer::GetPercentLoaded ()
{
	if ( m_Flash )
		return m_Flash->PercentLoaded();
	else
		return 0;
}

bool CFlashRenderer::IsFrameLoaded ( long FrameNum )
{
	if ( m_Flash )
		return m_Flash->FrameLoaded(FrameNum)== VARIANT_TRUE;
	else
		return false;
}

long CFlashRenderer::GetFlashVersion ()
{
	if ( m_Flash )
		return m_Flash->FlashVersion();
	else
		return -1;
}

void CFlashRenderer::SetQuality( int Quality )
{
	if ( m_Flash )
		m_Flash->PutQuality ( Quality );
}

int CFlashRenderer::GetQuality()
{
	if ( m_Flash )
		return m_Flash->GetQuality();
	else
		return -1;
}

void CFlashRenderer::SetRenderInterval ( int64 RenderInterval )
{
	m_RenderInterval = RenderInterval;
}

int64 CFlashRenderer::GetRenderInterval()
{
	return m_RenderInterval;
}

bool CFlashRenderer::GetNextEvent ( std::string& Command , std::string& Args )
{
	if ( m_Events )
		return m_Events->GetNextEvent ( Command , Args );
	else
		return false;
}

void CFlashRenderer::SetTransparent ( bool Transparent )
{
	m_Transparent = Transparent;
}

bool CFlashRenderer::GetTransparent () const
{
	return m_Transparent;
}

void CFlashRenderer::SetColorKey ( ULONG ColorKey )
{
	m_ColorKey = ColorKey;
}

ULONG CFlashRenderer::GetColorKey () const
{
	return m_ColorKey;
}



}
}