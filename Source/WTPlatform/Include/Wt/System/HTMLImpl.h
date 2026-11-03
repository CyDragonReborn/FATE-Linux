/***************************************************************************************
 *
 * HTMLImpl.h
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
#include <map>
#include <wininet.h>
#include <exdispid.h>
#include "WT/System/Callback.h"
#include "WT/System/HTMLBase.h"

namespace WT
{
namespace System
{

class CHTMLDispatch : public IDocHostUIHandlerDispatch
{
public:
	enum
	{
		KDispatchIDClose,
		KDispatchIDGetValue,
		KDispatchIDSetValue,
		KDispatchIDSetEvent,
		KMethodCount
	};

	//
	// Exposed dispatch methods
	//

	void Close();
	void GetValue( std::wstring const& Key, std::wstring& Value ) const;
	void SetValue( std::wstring const& Key, std::wstring const& Value );
	void SetEvent( DISPPARAMS const& Params );

	CHTMLDispatch( CHTMLBase* pBase );
	~CHTMLDispatch();

	//
	// IUnknown interface
	//

	STDMETHODIMP QueryInterface( REFIID riid, void** ppvObject );
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	//
	// IDispatch interface
	//
	STDMETHODIMP GetTypeInfoCount( UINT* pctinfo );
	STDMETHODIMP GetTypeInfo( UINT iTInfo, LCID /*lcid*/, ITypeInfo** ppTInfo );
	STDMETHODIMP GetIDsOfNames( REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID /*lcid*/, DISPID* rgDispId );
	STDMETHODIMP Invoke(
		DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
		EXCEPINFO* pExcepInfo, UINT* puArgErr );

	//
	// IDocHostUIHandlerDispatch interface 
	//

	virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( 
		/* [in] */ DWORD dwID,
		/* [in] */ DWORD x,
		/* [in] */ DWORD y,
		/* [in] */ IUnknown *pcmdtReserved,
		/* [in] */ IDispatch *pdispReserved,
		/* [retval][out] */ HRESULT *dwRetVal);

	virtual HRESULT STDMETHODCALLTYPE GetHostInfo( 
		/* [out][in] */ DWORD *pdwFlags,
		/* [out][in] */ DWORD *pdwDoubleClick);

	virtual HRESULT STDMETHODCALLTYPE ShowUI( 
		/* [in] */ DWORD dwID,
		/* [in] */ IUnknown *pActiveObject,
		/* [in] */ IUnknown *pCommandTarget,
		/* [in] */ IUnknown *pFrame,
		/* [in] */ IUnknown *pDoc,
		/* [retval][out] */ HRESULT *dwRetVal);

	virtual HRESULT STDMETHODCALLTYPE HideUI();
	virtual HRESULT STDMETHODCALLTYPE UpdateUI();
	virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
		/* [in] */ VARIANT_BOOL fEnable);

	virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
		/* [in] */ VARIANT_BOOL fActivate);

	virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
		/* [in] */ VARIANT_BOOL fActivate);

	virtual HRESULT STDMETHODCALLTYPE ResizeBorder( 
		/* [in] */ long left,
		/* [in] */ long top,
		/* [in] */ long right,
		/* [in] */ long bottom,
		/* [in] */ IUnknown *pUIWindow,
		/* [in] */ VARIANT_BOOL fFrameWindow);

	virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
		/* [in] */ DWORD_PTR hWnd,
		/* [in] */ DWORD nMessage,
		/* [in] */ DWORD_PTR wParam,
		/* [in] */ DWORD_PTR lParam,
		/* [in] */ BSTR bstrGuidCmdGroup,
		/* [in] */ DWORD nCmdID,
		/* [retval][out] */ HRESULT *dwRetVal);

	virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
		/* [out] */ BSTR *pbstrKey,
		/* [in] */ DWORD dw);

	virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
		/* [in] */ IUnknown *pDropTarget,
		/* [out] */ IUnknown **ppDropTarget);

	virtual HRESULT STDMETHODCALLTYPE GetExternal( 
		/* [out] */ IDispatch **ppDispatch);

	virtual HRESULT STDMETHODCALLTYPE TranslateUrl( 
		/* [in] */ DWORD dwTranslate,
		/* [in] */ BSTR bstrURLIn,
		/* [out] */ BSTR *pbstrURLOut);

	virtual HRESULT STDMETHODCALLTYPE FilterDataObject( 
		/* [in] */ IUnknown *pDO,
		/* [out] */ IUnknown **ppDORet);

private:

	//Automation structure helper methods
	void AddMethodParam( int32 MethodIndex, std::string const& Name, VARTYPE VarType );

	void ClearAutomationStructures();

private:
	//IUnknown-related members
	__declspec( align( 4 ) ) int32 m_RefCount;

	//IDispatch-related members
	METHODDATA m_Methods[KMethodCount];
	INTERFACEDATA m_Interface;

	ITypeInfo* m_ptinfo;

	CHTMLBase* m_pBase;
};

class CHTMLOleClientSite : public IOleClientSite
{
public:
	virtual HRESULT STDMETHODCALLTYPE SaveObject()
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetMoniker( DWORD dwAssign , DWORD dwWhichMoniker , IMoniker **ppmk )
	{
		*ppmk = NULL;
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetContainer( IOleContainer **ppContainer )
	{
		*ppContainer = NULL;
		return E_NOINTERFACE;
	}

	virtual HRESULT STDMETHODCALLTYPE ShowObject()
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnShowWindow( BOOL fShow )
	{
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout()
	{
		return E_NOTIMPL;
	}
};

// {9B97FD21-1D3F-4a8a-988A-AF4D1A3EAE85}
DEFINE_GUID(CLSID_CHTMLImpl, 
0x9B97FD21, 0x1D3F, 0x4a8a, 0x98, 0x8A, 0xAF, 0x4D, 0x1A, 0x3E, 0xAE, 0x85);

//This is an internal class for handling the events we get from the browser

class ATL_NO_VTABLE CHTMLImpl
	: public CComObjectRootEx<CComSingleThreadModel>
	, public CComCoClass<CHTMLImpl, &CLSID_CHTMLImpl>
	, public IDispEventImpl<0,CHTMLImpl,&DIID_DWebBrowserEvents2,&LIBID_SHDocVw,1,0>
//	, public CHTMLOleClientSite
{
public:
	CHTMLImpl();

	//Set whether events are handled
	void SetReady( bool Ready );

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_SINK_MAP(CHTMLImpl)
		SINK_ENTRY_EX( 0, DIID_DWebBrowserEvents2, DISPID_NAVIGATEERROR, OnNavigateError )
		SINK_ENTRY_EX( 0, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE, OnDocumentComplete )
	END_SINK_MAP()

	BEGIN_COM_MAP(CHTMLImpl)
		COM_INTERFACE_ENTRY_IID( IID_IUnknown, CHTMLImpl )
		COM_INTERFACE_ENTRY_IID( DIID_DWebBrowserEvents2, CHTMLImpl )
	END_COM_MAP()

	STDMETHOD(OnNavigateError)( IDispatch* pDisp, VARIANT* URL, VARIANT* TargetFrameName, VARIANT* StatusCode, VARIANT_BOOL* Cancel );
	STDMETHOD(OnDocumentComplete)( IDispatch* pDisp, VARIANT* URL );

	DWORD GetPageStatus() const {return m_PageStatus;}
	bool IsPageError() const {return m_PageStatus >= HTTP_STATUS_BAD_REQUEST;}
	void SetPageStatus( DWORD PageStatus ) {m_PageStatus = PageStatus;}
	void ClearPageStatus() {m_PageStatus = HTTP_STATUS_CONTINUE;}

	bool GetPageComplete() const {return m_PageComplete;}
	void SetPageComplete( bool PageComplete ) {m_PageComplete = PageComplete;}

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

private:
	DWORD m_PageStatus;
	bool m_Ready  : 1;
	bool m_PageComplete : 1;
};

}
}
				