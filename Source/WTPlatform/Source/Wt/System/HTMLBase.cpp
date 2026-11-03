/***************************************************************************************
 *
 * HTMLImpl.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          05/17/2006
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "HTMLBase.h"
#include "HTMLImpl.h"

namespace WT
{
namespace System
{

CHTMLBase::CHTMLBase()
	: m_pBrowser( NULL )
	, m_pImpl( NULL )
{
	m_pDispatch = new CHTMLDispatch( this );
	m_pDispatch->AddRef();
}

CHTMLBase::~CHTMLBase()
{
	m_Window.SetExternalUIHandler( NULL );
	assert( m_pDispatch != NULL );
	m_pDispatch->Release();

	if (m_pBrowser != NULL)
	{
		if (m_pImpl != NULL)
		{	
			m_pImpl->DispEventUnadvise( m_pBrowser, &DIID_DWebBrowserEvents2 );
		}
		m_pBrowser->Release();

	}

	if (m_Window.IsWindow())
	{
		m_Window.DestroyWindow();
	}

	if (m_pImpl != NULL)
	{
		m_pImpl->Release();
	}
}

void CHTMLBase::Init( HWND hParentWindow, Math::CRect2i const& Rect,
	std::string const& URL, DWORD dwStyle, DWORD dwExStyle )
{
	//
	// Create the dispatch implementation
	//

	CHTMLImpl::CreateInstance( reinterpret_cast<IUnknown**>( &m_pImpl ) );
	if (m_pImpl == NULL)
	{
		throw std::runtime_error( "CHTMLImpl::CreateInstance failure" );
	}

	//
	// Create the ActiveX control hosting window
	//
	// This purpose of this trickery - navigating to about:blank, stopping, and renavigating - 
	// is to ensure we get reliable page complete callbacks. We can't set the callback until
	// after the window is created. Unfortunately, we still aren't getting reliable page
	// complete callbacks, making them essentially useless.
	m_Window.Create( 
		hParentWindow, const_cast<LPRECT>( Rect.GetRectPtr() ), "about:blank", dwStyle, dwExStyle );
#ifndef NDEBUG
	m_Window.SetWindowText( "CHTMLBaseWindow" ); //Useful for tools like Spy++
#endif

	if (m_Window.m_hWnd == NULL)
	{
		m_pImpl->Release();
		throw std::runtime_error( "CAxWindow::Create failure" );
	}
	assert( m_Window.IsWindow() );

	//Set the handler. This needs to occur early before the browser is created and starts navigating.
	HRESULT Result = m_Window.SetExternalUIHandler( m_pDispatch );
	if (FAILED( Result ))
	{
		m_Window.DestroyWindow();
		m_pImpl->Release();
		throw std::runtime_error( "CAxWindow::SetExternalUIHandler failure" );
	}

	//
	//Create the browser
	//
	Result = m_Window.QueryControl( &m_pBrowser );
	if (FAILED( Result ))
	{
		m_Window.DestroyWindow();
		m_pImpl->Release();
		throw std::runtime_error( "CAxWindow::QueryControl failure" );
	}
	assert( m_pBrowser != NULL );
	m_pBrowser->Stop();

	Result = m_pBrowser->put_RegisterAsDropTarget( VARIANT_FALSE );
	if (FAILED( Result ))
	{
		m_Window.DestroyWindow();
		m_pImpl->Release();
		m_pBrowser->Release();
		throw std::runtime_error( "IWebBrowser2::put_RegisterAsDropTarget failure" );
	}

#if 0
	CComPtr<IOleObject> spOleObj;
	m_pBrowser->QueryInterface( IID_IOleObject, reinterpret_cast<void**>( &spOleObj ) );
	if (spOleObj)
	{
		HRESULT Result = spOleObj->SetClientSite( m_pImpl );
		Result;
		assert( Result == S_OK );
	}
#endif

	Result = m_pImpl->DispEventAdvise( m_pBrowser, &DIID_DWebBrowserEvents2 );
	if (FAILED( Result ))
	{
		m_Window.DestroyWindow();
		m_pImpl->Release();
		m_pBrowser->Release();
		throw std::runtime_error( "DispEventAdvise failure" );
	}

	m_pImpl->SetReady( true );
	Navigate( URL );
}

/*virtual*/ HWND CHTMLBase::GetWindow()
{
	return m_Window.m_hWnd;
}

CAxWindow& CHTMLBase::GetAxWindow()
{
	return m_Window;
}

CAxWindow const& CHTMLBase::GetAxWindow() const
{
	return m_Window;
}

IWebBrowser2* CHTMLBase::GetBrowser()
{
	return m_pBrowser;
}

IWebBrowser2 const* CHTMLBase::GetBrowser() const
{
	return m_pBrowser;
}

/*virtual*/ void CHTMLBase::ResizeClient( uint32 Width, uint32 Height )
{
	if (m_Window.IsWindow())
	{
		if (!m_Window.ResizeClient( Width, Height ))
		{
			throw std::runtime_error( "CAxWindow::ResizeClient failure" );
		}
	}
}

Math::CRect2i CHTMLBase::GetRect() const
{
	if (m_Window.IsWindow())
	{
		RECT Rect;
		if (!m_Window.GetWindowRect( &Rect ))
		{
			throw std::runtime_error( "CAxWindow::GetWindowRect failure" );
		}
		return Math::CRect2i( Rect );
	} else {
		return Math::CRect2i();
	}
}

Math::CRect2i CHTMLBase::GetClientRect() const
{
	if (m_Window.IsWindow())
	{
		RECT Rect;
		if (!m_Window.GetClientRect( &Rect ))
		{
			throw std::runtime_error( "CAxWindow::GetClientRect failure" );
		}
		return Math::CRect2i( Rect );
	} else {
		return Math::CRect2i();
	}
}

std::string CHTMLBase::GetText() const
{
	if (!m_Window.IsWindow())
	{
		return std::string();
	}

	CComBSTR Text;
	if (const_cast<CAxWindow&>( m_Window ).GetWindowText( &Text ) != TRUE)
	{
		throw std::runtime_error( "CAxWindow::GetWindowText failure" );
	}
	return CW2A( Text ).m_szBuffer;
}

void CHTMLBase::SetText( char const Text[] )
{
	if (m_Window.IsWindow())
	{
		if (!m_Window.SetWindowText( Text ))
		{
			throw std::runtime_error( "CAxWindow::SetWindowText failure" );
		}
	}
}

void CHTMLBase::SetFocus()
{
	if (m_Window.IsWindow())
	{
		m_Window.SetFocus();
	}
}

LRESULT CHTMLBase::SendMessage( UINT message, WPARAM wParam, LPARAM lParam )
{
	if (m_Window.IsWindow())
	{
		return m_Window.SendMessage( message, wParam, lParam );
	}

	return NULL;
}

DWORD CHTMLBase::GetStyle() const
{
	if (m_Window.IsWindow())
	{
		return m_Window.GetStyle();
	} else {
		return 0;
	}
}

void CHTMLBase::ModifyStyle( DWORD dwRemove, DWORD dwAdd, UINT nFlags )
{
	if (!m_Window.IsWindow())
	{
		return;
	}

	if (!m_Window.ModifyStyle( dwRemove, dwAdd, nFlags ))
	{
		throw std::runtime_error( "CAxWindow::ModifyStyle failure" );
	}
}

void CHTMLBase::ModifyStyleEx( DWORD dwRemove, DWORD dwAdd, UINT nFlags )
{
	if (!m_Window.IsWindow())
	{
		return;
	}

	if (!m_Window.ModifyStyleEx( dwRemove, dwAdd, nFlags ))
	{
		throw std::runtime_error( "CAxWindow::ModifyStyleEx failure" );
	}
}

void CHTMLBase::Close()
{
//	m_Window.DestroyWindow();
}

void CHTMLBase::GetValue( std::wstring const& Key, std::wstring& Value ) const
{
	std::map<wstring, wstring>::const_iterator it = m_Dictionary.find( Key );
	if (it == m_Dictionary.end())
	{
		Value.clear();
	} else {
		Value = it->second;
	}
}

void CHTMLBase::SetValue( std::wstring const& Key, std::wstring const& Value )
{
	if (Value.empty())
	{
		std::map<wstring, wstring>::iterator it = m_Dictionary.find( Key );
		if (it != m_Dictionary.end())
		{
			m_Dictionary.erase( it );
		}
	} else {
		m_Dictionary[Key] = Value;
	}
}

void CHTMLBase::SetEvent( std::vector<VARIANTARG> const& Arguments )
{
	std::vector<VARIANTARG> Arguments_( Arguments );

	//Put the arguments into COM order
	std::reverse( Arguments_.begin(), Arguments_.end() );

	DISPPARAMS Params;
	Params.cArgs = static_cast<UINT>( Arguments_.size() );
	Params.rgvarg = &Arguments_[0];
	Params.cNamedArgs = 0;
	Params.rgdispidNamedArgs = NULL;

	m_Event( this, Params );
}

bool CHTMLBase::IsBusy() const
{
	if (m_pBrowser == NULL)
	{
		return false;
	}

	HRESULT Result;
	VARIANT_BOOL Bool;

	{
		//Rarely, get_Busy crashes in MSHTML.DLL. This could be due to lack of mulithreaded
		//safety = it was observed that two calls to get_Busy were in progress at the time of one 
		//crash. Hopefully, serializing calls to this will prevent the crash.
		CAutoClassLock<CCriticalSection> Lock( const_cast<CCriticalSection*>( &m_GetBusyCS ) );
		 Result = m_pBrowser->get_Busy( &Bool );
	}

	if (FAILED( Result ))
	{
		throw std::runtime_error( "IWebBrowser2::get_Busy failure" );
	}

	return Bool != VARIANT_FALSE;
}

/*virtual*/ void CHTMLBase::Refresh()
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	m_pBrowser->Refresh(); //FYI, this never generates page complete events.

	//CHTMLBase::Navigate( GetURL() ); //This (sometimes?) generates page complete events.

	/*
	//More paranoid method - necessary?
	std::string const URL = GetURL();
	m_pImpl->SetReady( false );
	CHTMLBase::Navigate( "about:blank" );
	m_pBrowser->Stop();
	m_pImpl->SetReady( true );
	CHTMLBase::Navigate( URL );
	*/
}

void CHTMLBase::Stop()
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	m_pBrowser->Stop();
	m_pImpl->SetPageComplete( true );
	m_pImpl->ClearPageStatus();
}

/*virtual*/ void CHTMLBase::Navigate( std::string const& URL )
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	m_pImpl->SetPageComplete( false );
	m_pImpl->ClearPageStatus();
	CComVariant NoArg;
	CComVariant URLVariant( URL.c_str() );
	m_pBrowser->Navigate2( &URLVariant, &NoArg, &NoArg, &NoArg, &NoArg );
}

void CHTMLBase::GoBack()
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	m_pImpl->SetPageComplete( false );
	m_pImpl->ClearPageStatus();
	m_pBrowser->GoBack();
}

void CHTMLBase::GoForward()
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	m_pImpl->SetPageComplete( false );
	m_pImpl->ClearPageStatus();
	m_pBrowser->GoForward();
}

void CHTMLBase::GoHome()
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	m_pImpl->SetPageComplete( false );
	m_pImpl->ClearPageStatus();
	m_pBrowser->GoHome();
}

/*virtual*/ std::string CHTMLBase::GetURL() const
{
	if (m_pBrowser == NULL)
	{
		return std::string();
	}

	CComBSTR URL;
	HRESULT Result = m_pBrowser->get_LocationURL( &URL );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IWebBrowser2::get_LocationURL failure" );
	}
	return CW2A( URL ).m_szBuffer;
}

bool CHTMLBase::IsPageComplete() const
{
	//TODO is this function necessary?
	//IsBusy() may be more reliable.

	if (m_pImpl == NULL)
	{
		return false;
	}

	return m_pImpl->GetPageComplete();
}

void CHTMLBase::SetPageComplete( bool PageComplete )
{
	if (m_pImpl == NULL)
	{
		return;
	}

	m_pImpl->SetPageComplete( PageComplete );
}

DWORD CHTMLBase::GetPageStatus() const
{
	if (m_pImpl == NULL)
	{
		return HTTP_STATUS_CONTINUE;
	}

	return m_pImpl->GetPageStatus();
}

bool CHTMLBase::IsPageError() const
{
	if (m_pImpl == NULL)
	{
		return false;
	}

	return m_pImpl->IsPageError();
}

void CHTMLBase::SetPageStatus( DWORD PageStatus )
{
	if (m_pImpl == NULL)
	{
		return;
	}

	m_pImpl->SetPageStatus( PageStatus );
}

void CHTMLBase::ClearPageStatus()
{
	if (m_pImpl == NULL)
	{
		return;
	}

	m_pImpl->ClearPageStatus();
}

std::string CHTMLBase::GetPageStatusString() const
{
	if (m_pImpl == NULL)
	{
		return CHTMLImpl::GetPageStatusString( static_cast<DWORD>( E_UNEXPECTED ) );
	}

	return m_pImpl->GetPageStatusString();
}

std::string CHTMLBase::GetPageStatusName() const
{
	if (m_pImpl == NULL)
	{
		return "(unknown)";
	}

	return m_pImpl->GetPageStatusName();
}

std::string CHTMLBase::GetPageStatusDescription() const
{
	if (m_pImpl == NULL)
	{
		return "(unknown)";
	}

	return m_pImpl->GetPageStatusDescription();
}

/*static*/ std::string WTCall CHTMLBase::GetPageStatusString( DWORD PageStatus )
{
	return CHTMLImpl::GetPageStatusString( PageStatus );
}

/*static*/ std::string WTCall CHTMLBase::GetPageStatusName( DWORD PageStatus )
{
	return CHTMLImpl::GetPageStatusName( PageStatus );
}

/*static*/ std::string WTCall CHTMLBase::GetPageStatusDescription( DWORD PageStatus )
{
	return CHTMLImpl::GetPageStatusDescription( PageStatus );
}

bool CHTMLBase::IsOffline() const
{
	if (m_pBrowser == NULL)
	{
		return true;
	}

	VARIANT_BOOL Bool;
	HRESULT Result = m_pBrowser->get_Offline( &Bool );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IWebBrowser2::get_Offline failure" );
	}
	return Bool != VARIANT_FALSE;
}

void CHTMLBase::SetOffline( bool Offline )
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	HRESULT Result = m_pBrowser->put_Offline( Offline ? VARIANT_TRUE : VARIANT_FALSE );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IWebBrowser2::put_Offline failure" );
	}
}

void CHTMLBase::ExecScript( char const Code[], char const Language[] )
{
	if (m_pBrowser == NULL)
	{
		return;
	}

	CComPtr<IDispatch> pDoc;
	HRESULT Result = m_pBrowser->get_Document( &pDoc );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IWebBrowser2::get_Document failure" );
	}

	CComPtr<IHTMLDocument2> pDoc2;
	pDoc2 = pDoc;
	if (pDoc2 == NULL)
	{
		throw std::runtime_error( "IWebBrowser2::get_Document result not an IHTMLDocument2" );
	}

	CComPtr<IHTMLWindow2> pWindow2;
	Result = pDoc2->get_parentWindow( &pWindow2 );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IHTMLDocument2::get_parentWindow failure" );
	}

	CComPtr<IHTMLWindow2> pWindowTop;
	Result = pWindow2->get_top( &pWindowTop );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IHTMLWindow2::get_top failure" );
	}

	CComBSTR CodeBSTR( Code );
	CComBSTR LangBSTR( Language );

	VARIANT Ignored;
	Result = pWindow2->execScript( CodeBSTR, LangBSTR, &Ignored );
	if (FAILED( Result ))
	{
		throw std::runtime_error( "IHTMLWindow2::execScript failure" );
	}
}

}
}
