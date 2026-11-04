//
//	HTMLRenderer.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#define NOMINMAX
#include "HTMLRenderer.h"
#include <imagehlp.h>
#include "WT/System/Logging/Log.h"

#pragma comment( lib, "imagehlp" )

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

//
//This function implementation belongs in System\HTMLBase.cpp, but placing it there results
//in a strange error (exception 0x8001010D = RPC_E_CANTCALLOUT_ININPUTSYNCCALL) when 
//CAxWindow::Create is called!
//
namespace System
{
const GUID KApplicationLibraryID = {0x2DC1CA2F,0x0FBE,0x4D2D,0xB9,0x4B,0x5B,0x6A,0xA5,0xD5,0x9B,0x46};

class CAtlApplicationModule : public CAtlExeModuleT< CAtlApplicationModule >
{
public :
	DECLARE_LIBID(KApplicationLibraryID)
};

std::auto_ptr<CAtlApplicationModule> gpAtlApplicationModule;

/*static*/ void WTCall CHTMLBase::InitATLModule()
{
	if (ATL::_pAtlModule == NULL)
	{
		gpAtlApplicationModule.reset( new CAtlApplicationModule() );
		assert( ATL::_pAtlModule != NULL );
	}
}

}

namespace Graphics
{

static bool g_ThreadProtection = false;

uint32 gm_RenderCriticalSectionRefCount = 0;
CRITICAL_SECTION gm_RenderCriticalSection;

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

static HWND g_hCurrentHTMLWindow = NULL;

static BOOL WINAPI MyGetCursorPos( LPPOINT lpPoint )
{
	RECT r;
	GetWindowRect( g_hCurrentHTMLWindow ,&r);
	lpPoint->x = r.left;
	lpPoint->y = r.top;
	return TRUE;
}

static HWND WINAPI MyWindowFromPoint( POINT /*Point*/ )
{
	return g_hCurrentHTMLWindow;
}

static HCURSOR WINAPI MySetCursor( HCURSOR /*hCursor*/ )
{
	return NULL;
}

static HWND WINAPI MySetCapture( HWND /*hWnd*/ )
{
	return NULL;
}

CHTMLRenderer::CHTMLRenderer() 
	: m_ViewObject( NULL )
	, m_Width( 0 )
	, m_Height( 0 )
	, m_Updated( false )
	, m_Stopped( false )
	, m_UpdatePeriodMS( KDefaultUpdatePeriodMS )
{
	InitATLModule();

	m_Async = false;
	m_hRenderThread = NULL;
	m_hDC = NULL;
	m_hOldBitmap = NULL;
	m_hBitmap = NULL;
	m_RenderThreadID = 0;
	m_hParentWindow = NULL;
	m_BitmapBits = NULL;
	m_Updating = true;
	m_RenderInterval = 0.0f;
	m_TimeSinceLastRender = 0.0f;
	m_Transparent = false;
	m_ColorKey = 0xff000000u;
	if ( gm_RenderCriticalSectionRefCount == 0 )
	{
		InitializeCriticalSection ( &gm_RenderCriticalSection );
	}
	gm_RenderCriticalSectionRefCount++;

	m_hRenderMutex = NULL;

	m_hBrowserEvent = NULL;
	m_BrowserCommandRefresh		= 0;
	m_BrowserCommandNavigate	= 0;
	m_BrowserCommandStop		= 0;
}

CHTMLRenderer::~CHTMLRenderer()	
{
	Destroy();
	gm_RenderCriticalSectionRefCount--;
	if ( gm_RenderCriticalSectionRefCount == 0 )
	{
		DeleteCriticalSection ( &gm_RenderCriticalSection );
	}
}

void CHTMLRenderer::CreateBackgroundBitmap()
{
	HDC DesktopDC = GetDC( GetDesktopWindow() );

	m_hDC = CreateCompatibleDC ( DesktopDC );	
	SetMapMode( m_hDC, MM_TEXT );

	memset ( &m_BitmapInfo , 0 , sizeof(BITMAPINFO) );
	m_BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_BitmapInfo.bmiHeader.biPlanes = 1;
	m_BitmapInfo.bmiHeader.biBitCount = 24;
	m_BitmapInfo.bmiHeader.biCompression = BI_RGB;
	m_BitmapInfo.bmiHeader.biWidth = m_Width;
	m_BitmapInfo.bmiHeader.biHeight = (LONG)-static_cast<int32>( m_Height );
	
	m_hBitmap = CreateDIBSection ( m_hDC, &m_BitmapInfo, DIB_RGB_COLORS, &m_BitmapBits , NULL , 0 );
	if (m_hBitmap == NULL)
	{
		throw std::runtime_error( "CreateDIBSection failure" );

	}
	assert( m_BitmapBits != NULL );

	m_hOldBitmap = (HBITMAP) SelectObject ( m_hDC, m_hBitmap );
}

/*static*/ DWORD WINAPI CHTMLRenderer::RenderThreadRoutine ( LPVOID lpParameter )
{
	CHTMLRenderer* This = (CHTMLRenderer *) lpParameter;
	return This->RenderThreadRoutine();
}

void CHTMLRenderer::SetUpdatePeriodMS( int32 PeriodMS )
{
	if (PeriodMS < 0)
	{
		throw std::out_of_range( "PeriodMS" );
	}

	m_UpdatePeriodMS = PeriodMS;
}

int32 CHTMLRenderer::GetUpdatePeriodMS() const
{
	return m_UpdatePeriodMS;
}

DWORD CHTMLRenderer::RenderThreadRoutine()
{
	CreateResources();

	DWORD NextRenderTime = ::GetTickCount();

	for (;;)
	{
		assert( m_hBrowserEvent != NULL );

		if (WaitForSingleObject( m_hBrowserEvent, 0 ) == WAIT_OBJECT_0)
		{
			LONG bStop = m_BrowserCommandStop;
			if (bStop)
			{
				m_Stopped = true;
				CHTMLBase::Stop();
				::InterlockedExchange( &m_BrowserCommandStop, 0 );
			}

			if (m_BrowserCommandRefresh)
			{
				if (!bStop)
				{
					CHTMLBase::Refresh();
					m_Stopped = false;
				}
				::InterlockedExchange( &m_BrowserCommandRefresh, 0 );
			}

			if (m_BrowserCommandNavigate)
			{
				if (!bStop)
				{
					::InterlockedExchange( &m_BrowserCommandNavigate, 0 );
					CHTMLBase::Navigate( m_URL );
					m_Stopped = false;
				}
			}
		}

		MSG Msg;
		if (!PeekMessage( &Msg, NULL, NULL, NULL, PM_NOREMOVE 
			| PM_NOYIELD /*We'll do the yielding ourselves*/ ))
		{
			DWORD dwTickCount = GetTickCount();
			if (dwTickCount - NextRenderTime >= 0)
			{
				if (!m_Stopped)
				{
					if (!g_ThreadProtection)
					{
						UpdateBackgroundImage();
					}
					else if (WaitForSingleObject( m_hRenderMutex, 2000 ) == WAIT_OBJECT_0)
					{
						UpdateBackgroundImage();
						ReleaseMutex( m_hRenderMutex );
					}
				}
				NextRenderTime = dwTickCount + m_UpdatePeriodMS;
			}

			//Sleep for a while; allow some messages to accumulate.
			Sleep( 5 );
		} else {

			BOOL Result = GetMessage( &Msg, NULL, NULL, NULL );

			if (!Result)
			{
				break;
			} else if (Result != -1) {
				if (Msg.message == WM_CLOSE)
				{
					PostQuitMessage( 0 );
				} else {
					TranslateMessage( &Msg );
					DispatchMessage( &Msg );
				}
			}

			Sleep( 0 );
		}
	}

	m_Window.DestroyWindow();
	FreeResources();
	return 0;
}

void CHTMLRenderer::UpdateBackgroundImage()
{
	assert( !m_Stopped );

	if (!m_Updating)
	{
		return;
	}

	//update the time that passed since the last render
	m_TimeSinceLastRender += m_RenderTimer.GetElapsedTime();

	//check if we need to update the background image based on the render interval
	if (!IsBusy() && !IsPageError() && m_TimeSinceLastRender >= m_RenderInterval)
	{
		//update all tags locations

		for (unsigned int i = 0; i < m_Tags.size(); ++i)
		{
			GetRectOfElement( m_Tags[i], m_TagRects[i] );
		}

		RenderBackgroundImage();
		m_TimeSinceLastRender = 0.0f;
	}
}

void CHTMLRenderer::Create(  HWND hParentWindow, int Width, int Height, const std::string& URL, bool Async,
	const std::vector<std::string>& Tags )
{
	m_Width = Width;
	m_Height = Height;
	m_hParentWindow = hParentWindow;
	m_URL = URL;
	m_Async = Async;
	m_Tags = Tags;
	for (unsigned int i = 0; i < m_Tags.size(); ++i)
	{
		RECT r;
		SetRect( &r,0,0,0,0 );
		m_TagRects.push_back( r );
	}

	CreateBackgroundBitmap();

	if (Async)
	{
		if (g_ThreadProtection)
		{
			m_hRenderMutex = CreateMutex( NULL, FALSE, NULL );
		}

		m_hBrowserEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		if (m_hBrowserEvent == NULL)
		{
			CloseHandle( m_hRenderMutex );
			throw std::runtime_error( "CreateEvent failure" );
		}

		m_hRenderThread = CreateThread( NULL , NULL , RenderThreadRoutine , (LPVOID) this , NULL , &m_RenderThreadID );
	}
	else
	{
		CreateResources();
	}
}

void CHTMLRenderer::CreateResources()
{
	Init( NULL /*m_hParentWindow */, Math::CRect2i( 0, 0, m_Width, m_Height ), m_URL, /*WS_CHILD*/ WS_POPUP );

	m_Window.QueryControl( &m_ViewObject );

	m_RenderTimer.Start();
//	m_Window.ShowWindow(SW_SHOW );
}

void CHTMLRenderer::ReplaceAPIs()
{
	//replace all the API we need replacing in Flash

	HMODULE hModule;
	hModule = GetModuleHandle( "mshtml.dll" );
	ReplaceMethodInOneModule( "user32.dll", "GetCursorPos", (PROC)MyGetCursorPos, hModule );
	ReplaceMethodInOneModule( "user32.dll", "WindowFromPoint", (PROC)MyWindowFromPoint, hModule );
	hModule = GetModuleHandle("shdocvw.dll" );
	ReplaceMethodInOneModule( "user32.dll", "SetCursor", (PROC)MySetCursor, hModule );
	ReplaceMethodInOneModule( "user32.dll", "SetCapture", (PROC)MySetCapture, hModule );
}

void CHTMLRenderer::FreeResources()
{
	SafeRelease( m_ViewObject );
	FreeBackgroundBitmap();
}

void CHTMLRenderer::FreeBackgroundBitmap()
{
	if ( m_hOldBitmap )
	{
		SelectObject( m_hDC, m_hOldBitmap );
		DeleteDC( m_hDC );
		m_hDC = NULL;
		m_hOldBitmap = NULL;
	}
	if ( m_hBitmap )
	{
		DeleteObject( m_hBitmap );
		m_hBitmap = NULL;
	}
}

void CHTMLRenderer::Close()
{
	if (m_Async)
	{
		if (m_hRenderThread != NULL)
		{
			PostThreadMessage( m_RenderThreadID, WM_CLOSE, 0, 0 );
		}
	}
}

void CHTMLRenderer::Destroy()
{
	if (m_Async)
	{
		if (m_hRenderThread != NULL)
		{
			//We could call Close() here, but it's not really adequate. We need
			//blocking code that ensures delivery of the WM_CLOSE message.
			for (;;)
			{
				BOOL const Result = PostThreadMessage( m_RenderThreadID, WM_CLOSE, 0, 0 );
				if (Result)
				{
					break;
				}

				DWORD const dwError = GetLastError();
				if (dwError == ERROR_INVALID_THREAD_ID)
				{
					//This is unusual. The thread is invalid or it doesn't have 
					//a message queue (it has just begun or is just ending). Post 
					//the message periodically until it's been received or 
					//the thread has finished.
					if (WaitForSingleObject( m_hRenderThread, 5 ) == WAIT_OBJECT_0)
					{
						//The thread has finished.
						break;
					}
				} else {
					//An unknown error has occured.
				}
			}

			WaitForSingleObject( m_hRenderThread, INFINITE );

#ifndef NDEBUG
			{
				FILETIME CreationTime;
				FILETIME ExitTime;
				FILETIME KernelTime;
				FILETIME UserTime;

				if (GetThreadTimes( m_hRenderThread, 
						&CreationTime, &ExitTime, &KernelTime, &UserTime ))
				{
					//Filetimes consist of 100-nanosecond intervals
					static float32 const KIntervalSeconds = 100.0e-9f;

					int64 Age 
						= (ExitTime.dwHighDateTime - CreationTime.dwHighDateTime) * (1i64 << 32)
						+ (ExitTime.dwLowDateTime - CreationTime.dwLowDateTime);

					float32 const AgeSeconds 
						= Age * KIntervalSeconds;

					float32 const KernelTimeSeconds 
						= KernelTime.dwLowDateTime * KIntervalSeconds 
						+ KernelTime.dwHighDateTime * KIntervalSeconds * (1 << 16) * (1 << 16);
					float32 const UserTimeSeconds   
						= UserTime.dwLowDateTime * KIntervalSeconds 
						+ UserTime.dwHighDateTime * KIntervalSeconds * (1 << 16) * (1 << 16);
					float32 const ExecutionTimeSeconds = KernelTimeSeconds + UserTimeSeconds;
					
					LOG_GREEN( 
						"Thread for URL = '%s':\n"
						"  Age = %fs\n"
						"  Execution time = %f%% = %f%% User + %f%% Kernel\n"
						"  %% time spent executing = %f%% = %f%% User + %f%% Kernel\n",
						GetURL().c_str(),
						AgeSeconds,
						ExecutionTimeSeconds,
						UserTimeSeconds,
						KernelTimeSeconds,
						AgeSeconds ? 100.0f * ExecutionTimeSeconds / AgeSeconds : 0.0f,
						AgeSeconds ? 100.0f * UserTimeSeconds / AgeSeconds : 0.0f,
						AgeSeconds ? 100.0f * KernelTimeSeconds / AgeSeconds : 0.0f );
				}
			}
#endif

			CloseHandle( m_hRenderThread );
			m_hRenderThread = NULL;
			m_RenderThreadID = 0;
		}
		if (m_hRenderMutex != NULL)
		{
			CloseHandle( m_hRenderMutex );
			m_hRenderMutex = NULL;
		}

		if (m_hBrowserEvent != NULL)
		{
			CloseHandle( m_hBrowserEvent );
			m_hBrowserEvent = NULL;
		}
	}
	else
	{
		FreeResources();
	}
}

void CHTMLRenderer::PauseUpdating()
{
	m_Updating = false;
}

void CHTMLRenderer::ResumeUpdating()
{
	m_Updating = true;
}

/*virtual*/ void CHTMLRenderer::ResizeClient( uint32 Width, uint32 Height )
{
	//dont resize if dimensions are the same
	if (m_Width == Width && m_Height == Height)
	{
		return;
	}

	CHTMLBase::ResizeClient( Width, Height );
	m_Width = Width;
	m_Height = Height;
	FreeBackgroundBitmap();
	CreateBackgroundBitmap();
}

void CHTMLRenderer::SetWidth( uint32 Width )
{
	ResizeClient( Width, m_Height );
}

void CHTMLRenderer::SetHeight( uint32 Height )
{
	ResizeClient( m_Width, Height );
}

uint32 CHTMLRenderer::GetWidth() const
{
	return m_Width;
}

uint32 CHTMLRenderer::GetHeight() const
{
	return m_Height;
}

void CHTMLRenderer::SetUpdated( bool Updated )
{
	m_Updated = Updated;
}

bool CHTMLRenderer::IsUpdated() const
{
	return m_Updated;
}

void CHTMLRenderer::RenderBackgroundImage()
{
	if (!m_ViewObject)
	{
		return;
	}

	RECTL r;
	r.left = 0;
	r.top = 0;
	r.right = m_Width;
	r.bottom = m_Height;
	EnterCriticalSection( &gm_RenderCriticalSection );
	HRESULT Result = m_ViewObject->Draw( DVASPECT_CONTENT , 0 , NULL , NULL , NULL , m_hDC , &r , NULL , NULL , NULL );
	LeaveCriticalSection( &gm_RenderCriticalSection );

	if (SUCCEEDED( Result ))
	{
		m_Updated = true;
	}
}

void CHTMLRenderer::RenderToDC( HDC hDC , RECT* DestRect , RECT* SourceRect )
{
	//first check to see if we have a valid IViewObject interface, if not, no need to proceed
	if (!m_ViewObject)
	{
		return;
	}

	if (!m_Async)
	{
		RenderBackgroundImage();
	}
	else
	{
		if (g_ThreadProtection)
		{
			if (WaitForSingleObject( m_hRenderMutex, 2000 ) != WAIT_OBJECT_0)
			{
				return;
			}
		}
	}
	
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

	StretchBlt( hDC, DestX, DestY, DestWidth, DestHeight, this->m_hDC, SourceX, SourceY, SourceWidth, SourceHeight, SRCCOPY );
/*	StretchDIBits ( hDC,
	                DestX, DestY, DestWidth, DestHeight,
					SourceX, SourceY, SourceWidth, SourceHeight,
					m_BitmapBits, &m_BitmapInfo, DIB_RGB_COLORS, SRCCOPY );*/

	if (g_ThreadProtection)
	{
		ReleaseMutex( m_hRenderMutex );
	}
}

void CHTMLRenderer::RenderElementToDC( HDC hDC , const std::string& ElementName , RECT* DestRect )
{
	RECT SourceRect;
	if (m_Async)
	{
		for (unsigned int i = 0; i < m_Tags.size(); ++i)
		{
			if ( m_Tags[i] == ElementName )
			{
				SourceRect = m_TagRects[i];
				if (SourceRect.left | SourceRect.top | SourceRect.right | SourceRect.bottom)
				{
					RenderToDC ( hDC , DestRect , &SourceRect );
				}
			}
		}
	}
	else
	{
		if ( GetRectOfElement ( ElementName , SourceRect ))
		{
			RenderToDC ( hDC , DestRect , &SourceRect );
		}
	}
}

void CHTMLRenderer::RenderToSurface( LPDIRECT3DSURFACE pSurface, RECT* DestRect, RECT* SourceRect )
{
	//first check to see if we have a valid IViewObject interface, if not, no need to proceed
	if (!m_ViewObject)
	{
		return;
	}

	//Because the DIB is 24-bits, in order to use transparency, the color key must have alpha 
	//of 0xff. Do not just ignore the color key alpha - we may need to use it in the future.
	bool Transparent = m_Transparent && m_ColorKey >= 0xff000000u;

#if !defined(USE_DIRECTX_7) && !defined(USE_DIRECTX_8)
	//This is enabled for DX9 only because only DX9 surfaces have a GetDC() method.
	//Attempt to render to the DC, relying on GDI's StretchDIBits
	//Note that GetDC can fail for a number of reasons.
	if (!Transparent)
	{
		HDC hDC;
		HRESULT Result = pSurface->GetDC( &hDC );
		if (SUCCEEDED( Result ))
		{
			RenderToDC( hDC, DestRect, SourceRect );
			pSurface->ReleaseDC( hDC );
			return;
		}
	}
#endif

	if (!m_Async)
	{
		RenderBackgroundImage();
	}

	enum EFormat
	{
		KFormatR8G8B8,
		KFormatA8R8G8B8,
		KFormatX8R8G8B8,
		KFormatA8B8G8R8,
		KFormatX8B8G8R8
	} Format;

	D3DSURFACE_DESC Desc;
#ifdef USE_DIRECTX_7
	HRESULT Result = pSurface->GetSurfaceDesc( &Desc );
	if (FAILED( Result ))
	{
		return;
	}

	switch (Desc.ddpfPixelFormat.dwRGBBitCount)
	{
	case 24:
		if (Desc.ddpfPixelFormat.dwRBitMask == 0xff0000 &&
			Desc.ddpfPixelFormat.dwGBitMask == 0xff00 &&
			Desc.ddpfPixelFormat.dwBBitMask == 0xff)
		{
			Format = KFormatR8G8B8;
			break;
		}
		pSurface->Unlock( NULL );
		return;

	case 32:
		if (Desc.ddpfPixelFormat.dwRBitMask == 0xff0000 &&
			Desc.ddpfPixelFormat.dwGBitMask == 0xff00 &&
			Desc.ddpfPixelFormat.dwBBitMask == 0xff)
		{
			if (Desc.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000)
			{
				Format = KFormatA8R8G8B8;
				break;
			} else if (Desc.ddpfPixelFormat.dwRGBAlphaBitMask == 0x00000000) {
				Format = KFormatX8R8G8B8;
				break;
			}
		} else if (Desc.ddpfPixelFormat.dwRBitMask == 0xff &&
		  		   Desc.ddpfPixelFormat.dwGBitMask == 0xff00 &&
				   Desc.ddpfPixelFormat.dwBBitMask == 0xff0000)
		{
			if (Desc.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000)
			{
				Format = KFormatA8B8G8R8;
				break;
			} else if (Desc.ddpfPixelFormat.dwRGBAlphaBitMask == 0x00000000) {
				Format = KFormatX8B8G8R8;
				break;
			}
		}
		//Fall through

	default:
		return;
	}
#else
	HRESULT Result = pSurface->GetDesc( &Desc );
	if (FAILED( Result ))
	{
		return;
	}

	switch (Desc.Format)
	{
		case D3DFMT_R8G8B8	: Format = KFormatR8G8B8; break;
		case D3DFMT_A8R8G8B8: Format = KFormatA8R8G8B8; break;
		case D3DFMT_X8R8G8B8: Format = KFormatX8R8G8B8; break;
#ifndef USE_DIRECTX_8
		case D3DFMT_A8B8G8R8: Format = KFormatA8B8G8R8; break;
		case D3DFMT_X8B8G8R8: Format = KFormatX8B8G8R8; break;
#endif
		default:
			return; //Unsupported format
	}
#endif

	uint32 DestPixelBytes = 4;
	if (Format == KFormatR8G8B8)
	{
		DestPixelBytes = 3;
	}

	int const SourcePixelBytes = m_BitmapInfo.bmiHeader.biBitCount >> 3;
	int const SourcePitchBytes = m_Width * SourcePixelBytes;

	int RectWidth = m_Width;
	int RectHeight = m_Height;
	int SourceRectTop = 0;
	int SourceRectLeft = 0;
	int DestRectTop = 0;
	int DestRectLeft = 0;
#if USE_DIRECTX_7
	int DestRectRight = Desc.dwWidth;
	int DestRectBottom = Desc.dwHeight;
#else
	int DestRectRight = Desc.Width;
	int DestRectBottom = Desc.Height;
#endif
	
	if (SourceRect != NULL)
	{
		RectWidth = SourceRect->right - SourceRect->left;
		RectHeight = SourceRect->bottom - SourceRect->top;
		SourceRectLeft = SourceRect->left;
		SourceRectTop = SourceRect->top;
	}

	if (DestRect != NULL)
	{
		DestRectLeft = DestRect->left;
		DestRectTop = DestRect->top;
		DestRectRight = DestRect->right;
		DestRectBottom = DestRect->bottom;

		if (DestRect->left >= DestRect->right || //Mirroring not implemented
			DestRect->top >= DestRect->bottom ||
			DestRect->left < 0 || DestRect->top < 0 || //Clipping not implemented
			DestRect->right > DestRectRight || DestRect->bottom > DestRectBottom ||
			DestRectRight - DestRectLeft < RectWidth || //Shrinking not implemented
			DestRectBottom - DestRectTop < RectHeight)
		{
			return;
		}
	}

	//Lock the surface memory and render to it directly
#ifdef USE_DIRECTX_7
	if (FAILED( pSurface->Lock( NULL, &Desc, 0, NULL ) ))
	{
		return;
	}
	int DestPitchBytes = Desc.lPitch;
#else
	D3DLOCKED_RECT LockedRect;
	if (FAILED( pSurface->LockRect( &LockedRect, NULL, 0 ) ))
	{
		return;
	}

	int DestPitchBytes = LockedRect.Pitch;
#endif

	uint8* SourceBits = static_cast<uint8*>( m_BitmapBits );
	SourceBits += SourceRectTop * SourcePitchBytes + SourceRectLeft * SourcePixelBytes;

#ifdef USE_DIRECTX_7
	uint8* DestBits = static_cast<uint8*>( Desc.lpSurface );
#else
	uint8* DestBits = static_cast<uint8*>( LockedRect.pBits );
#endif
	DestBits += DestRectTop * DestPitchBytes + DestRectLeft * DestPixelBytes;

	int x,y;
	ULONG ColorValue;

	int32 DestRectWidth = DestRectRight - DestRectLeft;
	int32 DestRectHeight = DestRectBottom - DestRectTop;
	
	if (DestRectWidth == RectWidth && DestRectHeight == RectHeight)
	{
		//Loop over all pixels and alpha all the color keyed ones
		switch (Format)
		{
			case KFormatR8G8B8:
				if (!Transparent)
				{
					for (y = 0; y < RectHeight; ++y, SourceBits += SourcePitchBytes, DestBits += DestPitchBytes)
					{
						memcpy( DestBits, SourceBits, RectWidth * 3 );
					}
				}
				else
				{
					uint32 const ColorKeyRGB = m_ColorKey & 0x00ffffff; //Ignore colorkey alpha component
					for (y = 0; y < RectHeight; ++y, SourceBits += SourcePitchBytes, DestBits += DestPitchBytes)
					{
						uint8* pDest = DestBits;
						uint8 const* pSource = SourceBits;
						for (x = 0; x < RectWidth; ++x, pDest += DestPixelBytes)
						{
							ColorValue = pSource[0] + (pSource[1] << 8) + (pSource[2] << 16);
							pSource += SourcePixelBytes;
							if (ColorValue == ColorKeyRGB)
							{
								pDest[0] = 0;
								pDest[1] = 0;
								pDest[2] = 0;
							} else {
								pDest[0] = pSource[0];
								pDest[1] = pSource[1];
								pDest[2] = pSource[2];
							}
						}
					}
				}
				break;

			case KFormatA8R8G8B8:
			case KFormatX8R8G8B8:
				if (!Transparent)
				{
					for (y = 0; y < RectHeight; ++y, SourceBits += SourcePitchBytes, DestBits += DestPitchBytes)
					{
						uint8* pSource = SourceBits;
						for (x = 0; x < RectWidth; ++x)
						{
							reinterpret_cast<uint32*>( DestBits )[x] = pSource[0] + (pSource[1] << 8) + (pSource[2] << 16) + 0xff000000;
							pSource += SourcePixelBytes;
						}
					}
				}
				else
				{
					uint32 const ColorKeyRGB = m_ColorKey & 0x00ffffff; //Ignore colorkey alpha component
					for (y = 0; y < RectHeight; ++y, SourceBits += SourcePitchBytes, DestBits += DestPitchBytes)
					{
						uint8* pSource = SourceBits;
						for (x = 0; x < RectWidth; ++x)
						{
							ColorValue = pSource[0] + (pSource[1] << 8) + (pSource[2] << 16);
							pSource += SourcePixelBytes;
							reinterpret_cast<uint32*>( DestBits )[x] = ColorValue == ColorKeyRGB ? 0x00000000 : ColorValue + 0xff000000;
						}
					}
				}
				break;

			case KFormatA8B8G8R8:
			case KFormatX8B8G8R8:
				if (!Transparent)
				{
					for (y = 0; y < RectHeight; ++y, SourceBits += SourcePitchBytes, DestBits += DestPitchBytes)
					{
						uint8* pSource = SourceBits;
						for (x = 0; x < RectWidth; ++x)
						{
							reinterpret_cast<uint32*>( DestBits )[x] = pSource[2] + (pSource[1] << 8) + (pSource[0] << 16) + 0xff000000;
							pSource += SourcePixelBytes;
						}
					}
				}
				else
				{
					//Ignore the colorkey alpha component and swap the R and B components
					uint32 const ColorKeyBGR = ((m_ColorKey & 0xff0000) >> 16) + (m_ColorKey & 0xff00) + ((m_ColorKey & 0xff) << 16);

					for (y = 0; y < RectHeight; ++y, SourceBits += SourcePitchBytes, DestBits += DestPitchBytes)
					{
						uint8* pSource = SourceBits;
						for (x = 0; x < RectWidth; ++x)
						{
							ColorValue = pSource[2] + (pSource[1] << 8) + (pSource[0] << 16);
							pSource += SourcePixelBytes;
							reinterpret_cast<uint32*>( DestBits )[x] = ColorValue == ColorKeyBGR ? 0x00000000 : ColorValue + 0xff000000;
						}
					}
				}
				break;
		}
	} else {
		//Loop over all pixels and alpha all the color keyed ones
		switch (Format)
		{
			case KFormatR8G8B8:
				break; //Not implemented

			case KFormatA8R8G8B8:
			case KFormatX8R8G8B8:
				{
					float32 ScaleX = static_cast<float32>( RectWidth - 1 ) / (DestRectWidth - 1);
					float32 ScaleY = static_cast<float32>( RectHeight - 1 ) / (DestRectHeight - 1);

					if (!Transparent)
					{
						for (y = 0; y < DestRectHeight; ++y, DestBits += DestPitchBytes)
						{
							float32 SourceY = ScaleY * y;
							int32 Y0 = Float32ToInt32( floorf( SourceY ) );
							int32 Y1 = Float32ToInt32( ceilf( SourceY ) );
							float32 YFactor1 = SourceY - Y0;
							float32 YFactor0 = 1.0f - YFactor1;
							uint8* SourceRow0 = &SourceBits[SourcePitchBytes * Y0];
							uint8* SourceRow1 = &SourceBits[SourcePitchBytes * Y1];

							for (x = 0; x < DestRectWidth; ++x)
							{
								float32 SourceX = x * ScaleX;

								int32 X0 = Float32ToInt32( /*ceilf( SourceX )*/ SourceX - 0.5f );

								//If SourceX is an integer and the above operation rounded down, we need
								//to increment X0.
								if (X0 <= SourceX - 1.0f)
								{
									++X0;
								}

								//The next value of X1 is usually X0 + 1. If X0 happens to be an exact integer,
								//just use it as this avoids the case where X1 == DestRectWidth.
								int32 X1 = X0;
								if (X0 != SourceX)
								{
									++X1;
								}

								float32 XFactor1 = SourceX - X0;
								float32 XFactor0 = 1.0f - XFactor1;

								float32 const Factor00 = YFactor0 * XFactor0;
								float32 const Factor01 = YFactor0 * XFactor1;
								float32 const Factor10 = YFactor1 * XFactor0;
								float32 const Factor11 = YFactor1 * XFactor1;

								int32 iX0 = X0 + (X0 << 1); //3 * X0
								int32 iX1 = X1 + (X1 << 1); //3 * X1
								
								float32 b = Factor00 * SourceRow0[iX0] + Factor01 * SourceRow0[iX1]
										  + Factor10 * SourceRow1[iX0] + Factor11 * SourceRow1[iX1];
								++iX0; ++iX1;
								float32 g = Factor00 * SourceRow0[iX0] + Factor01 * SourceRow0[iX1]
										  + Factor10 * SourceRow1[iX0] + Factor11 * SourceRow1[iX1];
								++iX0; ++iX1;
								float32 r = Factor00 * SourceRow0[iX0] + Factor01 * SourceRow0[iX1]
										  + Factor10 * SourceRow1[iX0] + Factor11 * SourceRow1[iX1];

								int32 ir = Float32ToInt32( r );
								int32 ig = Float32ToInt32( g );
								int32 ib = Float32ToInt32( b );
								
								reinterpret_cast<uint32*>( DestBits )[x] = (ir << 16) + (ig << 8) + ib + 0xff000000;
							}
						}
					} else {
						uint32 const ColorKeyRGB = m_ColorKey & 0x00ffffff; //Ignore colorkey alpha component

						for (y = 0; y < DestRectHeight; ++y, DestBits += DestPitchBytes)
						{
							float32 SourceY = ScaleY * y;
							int32 Y0 = Float32ToInt32( floorf( SourceY ) );
							int32 Y1 = Float32ToInt32( ceilf( SourceY ) );
							float32 YFactor1 = SourceY - Y0;
							float32 YFactor0 = 1.0f - YFactor1;
							uint8* SourceRow0 = &SourceBits[SourcePitchBytes * Y0];
							uint8* SourceRow1 = &SourceBits[SourcePitchBytes * Y1];

							for (x = 0; x < DestRectWidth; ++x)
							{
								float32 SourceX = x * ScaleX;

								int32 X0 = Float32ToInt32( /*ceilf( SourceX )*/ SourceX - 0.5f );

								//If SourceX is an integer and the above operation rounded down, we need
								//to increment X0.
								if (X0 <= SourceX - 1.0f)
								{
									++X0;
								}

								//The next value of X1 is usually X0 + 1. If X0 happens to be an exact integer,
								//just use it as this avoids the case where X1 == DestRectWidth.
								int32 X1 = X0;
								if (X0 != SourceX)
								{
									++X1;
								}

								float32 XFactor1 = SourceX - X0;
								float32 XFactor0 = 1.0f - XFactor1;

								float32 const Factor00 = YFactor0 * XFactor0;
								float32 const Factor01 = YFactor0 * XFactor1;
								float32 const Factor10 = YFactor1 * XFactor0;
								float32 const Factor11 = YFactor1 * XFactor1;

								int32 iX0 = X0 + (X0 << 1); //3 * X0
								int32 iX1 = X1 + (X1 << 1); //3 * X1

								uint32 Color00 = SourceRow0[iX0] + (SourceRow0[iX0 + 1] << 8) + (SourceRow0[iX0 + 2] << 16);
								uint32 Color01 = SourceRow0[iX1] + (SourceRow0[iX1 + 1] << 8) + (SourceRow0[iX1 + 2] << 16);
								uint32 Color10 = SourceRow1[iX0] + (SourceRow1[iX0 + 1] << 8) + (SourceRow1[iX0 + 2] << 16);
								uint32 Color11 = SourceRow1[iX1] + (SourceRow1[iX1 + 1] << 8) + (SourceRow1[iX1 + 2] << 16);
								if (Color00 == ColorKeyRGB) Color00 = 0x00000000; else Color00 += 0xff000000;
								if (Color01 == ColorKeyRGB) Color01 = 0x00000000; else Color01 += 0xff000000;
								if (Color10 == ColorKeyRGB) Color10 = 0x00000000; else Color10 += 0xff000000;
								if (Color11 == ColorKeyRGB) Color11 = 0x00000000; else Color11 += 0xff000000;

								float32 a = Factor00 * ((Color00 >> 24)       ) + Factor01 * ((Color01 >> 24)       )
										  + Factor10 * ((Color10 >> 24)       ) + Factor11 * ((Color11 >> 24)       );
								float32 r = Factor00 * ((Color00 >> 16) & 0xff) + Factor01 * ((Color01 >> 16) & 0xff)
										  + Factor10 * ((Color10 >> 16) & 0xff) + Factor11 * ((Color11 >> 16) & 0xff);
								float32 g = Factor00 * ((Color00 >>  8) & 0xff) + Factor01 * ((Color01 >>  8) & 0xff)
										  + Factor10 * ((Color10 >>  8) & 0xff) + Factor11 * ((Color11 >>  8) & 0xff);
								float32 b = Factor00 * ((Color00      ) & 0xff) + Factor01 * ((Color01      ) & 0xff)
										  + Factor10 * ((Color10      ) & 0xff) + Factor11 * ((Color11      ) & 0xff);

								int32 ia = Float32ToInt32( a );
								int32 ir = Float32ToInt32( r );
								int32 ig = Float32ToInt32( g );
								int32 ib = Float32ToInt32( b );
								
								reinterpret_cast<uint32*>( DestBits )[x] = (ia << 24) + (ir << 16) + (ig << 8) + ib;
							} //for (x...
						} //for (y...
					} //if (!Transparent)
				}
				break;
		}
	}

#ifdef USE_DIRECTX_7
		pSurface->Unlock ( NULL );
#else
		pSurface->UnlockRect();
#endif
}

#ifndef USE_DIRECTX_7
void CHTMLRenderer::RenderToTexture ( LPDIRECT3DTEXTURE pTexture , RECT* DestRect , RECT* SourceRect )
{
	//first check to see if we have a valid IViewObject interface, if not, no need to proceed
	if ( !m_ViewObject )
		return;

	LPDIRECT3DSURFACE pSurface = NULL;
	if ( pTexture->GetSurfaceLevel ( 0 , &pSurface ) != S_OK )
		return;
	RenderToSurface ( pSurface , DestRect , SourceRect );
	pSurface->Release();
}
#endif

bool CHTMLRenderer::GetRectOfElement ( const std::string& ElementName , RECT& ElementRect )
{
	if ( !m_pBrowser )
	{
		return false;
	}
	CComPtr<IDispatch> Doc;
	m_pBrowser->get_Document ( &Doc );
 	CComPtr<IHTMLDocument2> Doc2;
	Doc2 = Doc;
	if ( !Doc2 )
	{
		return false;
	}

	CComPtr<IHTMLElementCollection> AllCollection;
	Doc2->get_all ( &AllCollection );
	if ( !AllCollection )
		return false;
	CComPtr<IDispatch> DElement;
	AllCollection->item ( CComVariant(ElementName.c_str()) , CComVariant( 0 ) , &DElement );
	CComPtr<IHTMLElement> Element;
	Element = DElement;
	if ( !Element )
		return false;

	long Left,Top,Width,Height;
	Element->get_offsetLeft( &Left );
	Element->get_offsetTop( &Top );
	Element->get_offsetWidth( &Width );
	Element->get_offsetHeight( &Height );
	SetRect ( &ElementRect , Left , Top , Left+Width , Top+Height );
	return true;
}

#ifndef USE_DIRECTX_7

void CHTMLRenderer::RenderElementToTexture ( LPDIRECT3DTEXTURE pTexture , const std::string& ElementName , RECT* DestRect )
{
	RECT SourceRect;
	if ( m_Async )
	{
		for ( unsigned int i=0 ; i<m_Tags.size() ; i++ )
		{
			if ( m_Tags[i] == ElementName )
			{
				SourceRect = m_TagRects[i];
				if ( SourceRect.left || SourceRect.top || SourceRect.right || SourceRect.bottom )
					RenderToTexture ( pTexture , DestRect , &SourceRect );
			}
		}
	}
	else
		if ( GetRectOfElement ( ElementName , SourceRect ))
			RenderToTexture ( pTexture , DestRect , &SourceRect );
}
#endif

/*virtual*/ void CHTMLRenderer::Refresh()
{
	m_Updated = false;

	if ( !m_pBrowser )
	{
		return;
	}

	if ( m_Async )
	{
		::InterlockedExchange( &m_BrowserCommandRefresh, 1 );
		if (m_hBrowserEvent != NULL)
		{
			::SetEvent( m_hBrowserEvent );
		}
	}
	else
	{
		CHTMLBase::Refresh();
		m_Stopped = false;
	}
}

/*virtual*/ void CHTMLRenderer::Navigate( const std::string& URL )
{
	m_Updated = false;

	m_URL = URL;
	if (m_pBrowser == NULL)
	{
		return;
	}

	SetPageComplete( false );
	ClearPageStatus();

	if ( m_Async )
	{
		::InterlockedExchange( &m_BrowserCommandNavigate, 1 );
		if (m_hBrowserEvent != NULL)
		{
			::SetEvent( m_hBrowserEvent );
		}
	}
	else
	{
		CHTMLBase::Navigate( URL );
		m_Stopped = false;
	}
}

/*virtual*/ void CHTMLRenderer::Stop()
{
	m_Stopped = true;
	m_Updated = false;
	SetPageComplete( false );
	ClearPageStatus();

	if (m_Async)
	{
		::InterlockedExchange( &m_BrowserCommandStop, 1 );
		if (m_hBrowserEvent != NULL)
		{
			::SetEvent( m_hBrowserEvent );
		}
	} else {
		CHTMLBase::Stop();
	}
}

/*virtual*/ std::string CHTMLRenderer::GetURL() const
{
	return m_URL;
}

void CHTMLRenderer::SetRenderInterval ( float32 RenderInterval )
{
	m_RenderInterval = RenderInterval;
}

float32 CHTMLRenderer::GetRenderInterval() const
{
	return m_RenderInterval;
}

void CHTMLRenderer::SetTransparent ( bool Transparent )
{
	m_Transparent = Transparent;
}

bool CHTMLRenderer::GetTransparent() const
{
	return m_Transparent;
}

void CHTMLRenderer::SetColorKey ( ULONG ColorKey )
{
	m_ColorKey = ColorKey;
}

ULONG CHTMLRenderer::GetColorKey() const
{
	return m_ColorKey;
}

/*virtual*/ HWND CHTMLRenderer::GetWindow()
{
	if ( m_Window.IsWindow() )
	{
		HWND hWnd = m_Window.GetWindow(GW_CHILD);
		hWnd = ::GetWindow(hWnd,GW_CHILD);
		return ::GetWindow(hWnd,GW_CHILD);
	}
	else
	{
		return NULL;
	}
}

void CHTMLRenderer::HandleMouseMove ( int X , int Y , WPARAM wParam )
{
	g_hCurrentHTMLWindow = GetWindow();
	if ( g_hCurrentHTMLWindow )
	{
		PostMessage ( g_hCurrentHTMLWindow , WM_MOUSEMOVE , wParam , MAKELPARAM(X,Y));
	}
}

void CHTMLRenderer::HandleMouseDown ( int X , int Y , WPARAM wParam )
{
	g_hCurrentHTMLWindow = GetWindow();
	if ( g_hCurrentHTMLWindow )
	{
		PostMessage ( g_hCurrentHTMLWindow , WM_LBUTTONDOWN , wParam , MAKELPARAM(X,Y));
	}
}

void CHTMLRenderer::HandleMouseUp ( int X , int Y , WPARAM wParam )
{
	g_hCurrentHTMLWindow = GetWindow();
	if (g_hCurrentHTMLWindow != NULL)
	{
		PostMessage ( g_hCurrentHTMLWindow , WM_LBUTTONUP , wParam , MAKELPARAM(X,Y));
	}
}

void CHTMLRenderer::SetRenderThreadPriority( int ThreadPriority )
{
	if (m_hRenderThread != NULL)
	{
		SetThreadPriority ( m_hRenderThread , ThreadPriority );
	}
}

int CHTMLRenderer::GetRenderThreadPriority () const
{
	if (m_hRenderThread != NULL)
	{
		return GetThreadPriority ( m_hRenderThread );
	}
	else
	{
		return THREAD_PRIORITY_NORMAL;
	}
}
}
}
