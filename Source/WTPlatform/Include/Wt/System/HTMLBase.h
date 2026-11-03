/***************************************************************************************
 *
 * HTMLBase.h
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

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include <string>
#include <map>
#include <windows.h>
#include <wininet.h> //Provides HTTP status codes
#include <atlbase.h>
#include <atlwin.h>
#include "WT/System/Callback.h"
#include "WT/Math/Rects/Rect2i.h"

//Forward declarations
struct IWebBrowser2;

namespace ATL
{
	class CWindow;
	template <class TBase> class CAxWindowT;
	typedef CAxWindowT<CWindow> CAxWindow;
}

namespace WT
{
namespace System
{

//
//CHTMLBase is a window hosting an HTLM document. CHTMLBase supports the ability to create 
//a window with different styles, to manipulate window settings, to browse forward and 
//backward through HTML documents, to tab through HTML elements, to click on hyperlinks, and 
//to execute JScript. CHTMLBase also exposes various window.external methods.
//
//Dragging-and-dropping shortcuts into the window is initially disabled.
//
class CHTMLBase
{
public:
	CHTMLBase();
	virtual ~CHTMLBase() = 0;

	//
	//
	// Navigation
	//
	//

	//Navigate the HTML to a given URL
	virtual void Navigate( std::string const& URL );

	//Test whether there is pending navigation
	bool IsBusy() const;

	//Refresh the page
	virtual void Refresh();

	//Stop pending navigation and any dynamic elements
	virtual void Stop();

	//Navigate to the previous URL in the history list
	void GoBack();

	//Navigate to the next URL in the history list
	void GoForward();

	//Navigate to the home page
	void GoHome();

	//Return the current URL displayed in the browser
	virtual std::string GetURL() const;

	//Returns true when the navigation to a new URL is complete
	bool IsPageComplete() const;
	void SetPageComplete( bool PageComplete );

	//Returns a NavigationError event status code. Returns HTTP_STATUS_BAD_REQUEST or greater
	//on error.
	DWORD GetPageStatus() const;

	bool IsPageError() const;

	void SetPageStatus( DWORD PageStatus );

	//Sets the status code to HTTP_STATUS_CONTINUE.
	void ClearPageStatus();

	/// Get the status code in a human-readable form
	std::string GetPageStatusString() const;

	/// Get a symbolic name corresponding to the page status
	std::string GetPageStatusName() const;

	/// Get a description for the page status
	std::string GetPageStatusDescription() const;

	/// Convert the specified status code into a human-readable form
	static std::string WTCall GetPageStatusString( DWORD PageStatus );

	/// Get a symbolic name corresponding to the specified page status
	static std::string WTCall GetPageStatusName( DWORD PageStatus );

	/// Get a description for the specified page status
	static std::string WTCall GetPageStatusDescription( DWORD PageStatus );

	//
	//
	// Browser settings
	//
	//

	//Execute script code. The default language is JScript.
	void ExecScript( char const Code[], char const Language[] = NULL );

	//Test whether the browser is offline.
	bool IsOffline() const;

	//Set whether the browser is offline.
	void SetOffline( bool Offline );

	//
	//
	// Window settings
	//
	//

	//Get the window rectangle
	Math::CRect2i GetRect() const;

	//Get the window client area rectangle
	Math::CRect2i GetClientRect() const;

	//Resize the window client area
	virtual void ResizeClient( uint32 Width, uint32 Height );

	//Get the window text.
	std::string GetText() const;
	
	//Set the window text.
	void SetText( char const Text[] );

	//Get the window style.
	DWORD GetStyle() const;

	//Modify window style.
	void ModifyStyle( DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0 );

	//Modify window extended style.
	void ModifyStyleEx( DWORD dwRemove, DWORD dwAdd, UINT nFlags = 0 );

	//
	//
	// Window manipulation
	//
	//

	//Make the window the focus.
	void SetFocus();

	LRESULT SendMessage( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 );

	//
	//
	// window.external methods
	//
	//

	//Close the window; if appropriate.
	virtual void Close();

	//Look up a value in a dictionary. This is exposed to script as a dispatch method
	//BSTR GetValue( BSTR Key ).
	void GetValue( std::wstring const& Key, std::wstring& Value ) const;

	//Set a value in a dictionary; use L"" to erase the entry. This is exposed to script as
	//void SetValue( BSTR Key, BSTR Value ).
	void SetValue( std::wstring const& Key, std::wstring const& Value );

	//Set an event. This is used to trigger user-defined callbacks.
	//Any number of variant arguments may be supplied to the C++ method or exposed 
	//dispatch method. The user-defined callbacks will receive a DISPPARAMS structure
	//containing all of the arguments (arranged in reverse order).
	//
	//Here are some examples of attaching user-defined callbacks:
	//
	//void MyOnEvent( CHTMLBase* pBase, DISPPARAMS const& Params );
	//pHTMLBase->OnEvent() += CHTMLBase::EventCallback::Create( MyOnEvent );
	//
	//class CMyClass {void OnEvent( CHTMLBase* pBase, DISPPARAMS const& Params );}
	//CMyClass::CMyClass() {m_pHTMLBase->OnEvent() += CHTMLBase::EventCallback::Create( *this, &CMyClass::OnEvent );}
	void SetEvent( std::vector<VARIANTARG> const& Arguments );
	
	typedef System::CCallback2<CHTMLBase*, DISPPARAMS const&> EventCallback;
	EventCallback::Event& OnEvent() {return m_Event;}

	//
	// Access to internals
	//

	//Get the window handle
	virtual HWND GetWindow();

	//Get the ATL window object hosting the ActiveX control
	CAxWindow& GetAxWindow();
	CAxWindow const& GetAxWindow() const;

	//Get the associated web browser interface
	IWebBrowser2* GetBrowser();
	IWebBrowser2 const* GetBrowser() const;

protected:
	static void WTCall InitATLModule();

	void Init( HWND hParentWindow, Math::CRect2i const& Rect, std::string const& URL = "about:blank",
		DWORD dwStyle = 0, DWORD dwExStyle = 0 );

	CAxWindow					m_Window;
	IWebBrowser2*				m_pBrowser;
	class CHTMLDispatch*		m_pDispatch;
	class CHTMLImpl*			m_pImpl;
	std::map<wstring, wstring>	m_Dictionary;
	EventCallback::Event		m_Event;
	CCriticalSection			m_GetBusyCS;
};

}
}
