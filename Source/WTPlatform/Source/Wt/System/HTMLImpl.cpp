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
#include "HTMLImpl.h"
#include "WT/System/Logging/Log.h"
#include <sstream>

namespace WT
{
namespace System
{

BSTR AllocString( wchar_t const s[] )
{
	assert( s != NULL );

	BSTR Result = SysAllocString( s );
	if (Result == NULL)
	{
		throw std::runtime_error( "SysAllocString failure" );
	}

	return Result;
}

void CHTMLDispatch::Close()
{
	assert( m_pBase != NULL );
	m_pBase->Close();
}

void CHTMLDispatch::GetValue( std::wstring const& Key, std::wstring& Value ) const
{
	assert( m_pBase != NULL );
	m_pBase->GetValue( Key, Value );
}

void CHTMLDispatch::SetValue( std::wstring const& Key, std::wstring const& Value )
{
	assert( m_pBase != NULL );
	m_pBase->SetValue( Key, Value );
}

void CHTMLDispatch::SetEvent( DISPPARAMS const& Params )
{
	m_pBase->OnEvent()( m_pBase, Params );
}

CHTMLDispatch::CHTMLDispatch( CHTMLBase* pBase )
	: m_pBase( pBase )
	, m_RefCount( 0 )
{
	assert( m_pBase != NULL );

	for (int32 i = 0; i < KMethodCount; ++i)
	{
		m_Methods[i].dispid = static_cast<DISPID>( i );
		m_Methods[i].cc = CC_STDCALL;
		m_Methods[i].wFlags = DISPATCH_METHOD;
		m_Methods[i].cArgs = 0;
		m_Methods[i].ppdata = NULL;
		m_Methods[i].vtReturn = VT_VOID;
	}

	try
	{
		m_Methods[KDispatchIDClose].szName = AllocString( L"Close" );

		m_Methods[KDispatchIDGetValue].szName = AllocString( L"GetValue" );
		m_Methods[KDispatchIDGetValue].vtReturn = VT_BSTR;
		AddMethodParam( KDispatchIDGetValue, "Key", VT_BSTR );

		m_Methods[KDispatchIDSetValue].szName = AllocString( L"SetValue" );
		AddMethodParam( KDispatchIDSetValue, "Key", VT_BSTR );
		AddMethodParam( KDispatchIDSetValue, "Value", VT_BSTR );

		m_Methods[KDispatchIDSetEvent].szName = AllocString( L"SetEvent" );
		AddMethodParam( KDispatchIDSetEvent, "Event", VT_BSTR );

		m_Interface.pmethdata = m_Methods;
		m_Interface.cMembers = KMethodCount;

		HRESULT Result = CreateDispTypeInfo( &m_Interface, LOCALE_SYSTEM_DEFAULT, &m_ptinfo );
		if (FAILED( Result ))
		{
			throw std::runtime_error( "CreateDispTypeInfo failure" );
		}
	} catch (std::runtime_error const&) {
		ClearAutomationStructures();
		throw;
	}
}

CHTMLDispatch::~CHTMLDispatch()
{
	assert( m_RefCount == 0 );

	assert( m_ptinfo != NULL );
	m_ptinfo->Release();

	ClearAutomationStructures();
}

STDMETHODIMP CHTMLDispatch::QueryInterface( REFIID riid, void** ppvObject )
{
	if (IsEqualIID( riid, IID_IUnknown ))
	{
		if (ppvObject != NULL)
		{
			AddRef();
			*ppvObject = static_cast<IUnknown*>( this );
		}
		return S_OK;
	}

	if (IsEqualIID( riid, IID_IDispatch ))
	{
		if (ppvObject != NULL)
		{
			AddRef();
			*ppvObject = static_cast<IDispatch*>( this );
		}
		return S_OK;
	}

	if (IsEqualIID( riid, IID_IDocHostUIHandlerDispatch ))
	{
		if (ppvObject != NULL)
		{
			AddRef();
			*ppvObject = static_cast<IDocHostUIHandlerDispatch*>( this );
		}
		return S_OK;
	}

	if (ppvObject != NULL)
	{
		*ppvObject = NULL;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CHTMLDispatch::AddRef()
{
	return static_cast<ULONG>( InterlockedIncrement( reinterpret_cast<LPLONG>( &m_RefCount ) ) );
}

ULONG STDMETHODCALLTYPE CHTMLDispatch::Release()
{
	ULONG Result = static_cast<ULONG>( InterlockedDecrement( reinterpret_cast<LPLONG>( &m_RefCount ) ) );
	assert( Result >= 0 );
	if (!Result)
	{
		delete this;
	}
	return Result;
}

STDMETHODIMP CHTMLDispatch::GetTypeInfoCount( UINT* pctinfo )
{
	*pctinfo = 1;
	return S_OK;
}

STDMETHODIMP CHTMLDispatch::GetTypeInfo( UINT iTInfo, LCID /*lcid*/, ITypeInfo** ppTInfo )
{
	if (iTInfo)
	{
		return DISP_E_BADINDEX;
	}

	if (ppTInfo != NULL)
	{
		m_ptinfo->AddRef();
		*ppTInfo = m_ptinfo;
	}

	return S_OK;
}

STDMETHODIMP CHTMLDispatch::GetIDsOfNames( REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID /*lcid*/, DISPID* rgDispId )
{
	if (!IsEqualIID( riid, IID_NULL ))
	{
		return DISP_E_UNKNOWNINTERFACE;
	}

	return DispGetIDsOfNames( m_ptinfo, rgszNames, cNames, rgDispId );
}

STDMETHODIMP CHTMLDispatch::Invoke(
	DISPID dispIdMember, REFIID riid, LCID /*lcid*/, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
	EXCEPINFO* pExcepInfo, UINT* puArgErr )
{
	if (!IsEqualIID( riid, IID_NULL ))
	{
		return DISP_E_UNKNOWNINTERFACE;
	}

	if (!(wFlags & DISPATCH_METHOD))
	{
		//Only methods supported
		return DISP_E_MEMBERNOTFOUND;
	}

	if (dispIdMember < 0 || dispIdMember >= KMethodCount)
	{
		return DISP_E_MEMBERNOTFOUND;
	}

	if (pDispParams->cNamedArgs)
	{
		//No support for named arguments
		return DISP_E_NONAMEDARGS;
	}

	if (pDispParams->cArgs)
	{
		if (pDispParams->rgvarg == NULL)
		{
			return DISP_E_PARAMNOTOPTIONAL;
		}

		if (dispIdMember != KDispatchIDSetEvent)
		{
			METHODDATA const& Method = m_Methods[dispIdMember];

			//Check the number and types of arguments
			if (pDispParams->cArgs != Method.cArgs)
			{
				return DISP_E_BADPARAMCOUNT;
			}

			//Check types of arguments
			//TODO take coercion into account.
			for (uint32 i = 0; i < pDispParams->cArgs; ++i)
			{
				if (pDispParams->rgvarg[i].vt != Method.ppdata[Method.cArgs - 1 - i].vt)
				{
					if (puArgErr != NULL)
					{
						*puArgErr = i;
					}

					return DISP_E_TYPEMISMATCH;
				}
			}
		}
	}

	switch (dispIdMember)
	{
		case KDispatchIDClose:
			Close();
			break;

		case KDispatchIDGetValue:
			{
				std::wstring Value;
				GetValue( pDispParams->rgvarg[0].bstrVal, Value );

				if (pVarResult != NULL)
				{
					pVarResult->vt = VT_BSTR;
					pVarResult->bstrVal = SysAllocString( Value.c_str() );
				}
			}
			break;

		case KDispatchIDSetValue:
			{
				std::wstring const Key   = pDispParams->rgvarg[1].bstrVal;
				std::wstring const Value = pDispParams->rgvarg[0].bstrVal;
				SetValue( Key, Value );
			}
			break;

		case KDispatchIDSetEvent:
			{
				SetEvent( *pDispParams );
			}
			break;

		default:
			assert( 0 );
	}

	return S_OK;
}

//Automation structure helper methods
void CHTMLDispatch::AddMethodParam( int32 MethodIndex, std::string const& Name, VARTYPE VarType )
{
	assert( MethodIndex >= 0 );
	assert( MethodIndex < KMethodCount );

	int32 const ArgCount = m_Methods[MethodIndex].cArgs + 1;
	PARAMDATA* pNew = static_cast<PARAMDATA*>( realloc(
		m_Methods[MethodIndex].ppdata, ArgCount * sizeof( PARAMDATA ) ) );

	if (pNew == NULL)
	{
		throw std::runtime_error( "realloc failure" );
	}

	m_Methods[MethodIndex].cArgs = ArgCount;
	m_Methods[MethodIndex].ppdata = pNew;

	PARAMDATA& Param = m_Methods[MethodIndex].ppdata[ArgCount - 1];
	Param.szName = AllocString( AnsiToWide( Name ).c_str() );
	Param.vt = VarType;
}

void CHTMLDispatch::ClearAutomationStructures()
{
	for (int32 i = 0; i < KMethodCount; ++i)
	{
		SysFreeString( m_Methods[i].szName );

		int32 ParamCount = static_cast<int32>( m_Methods[i].cArgs );
		for (int32 j = 0; j < ParamCount; ++j)
		{
			SysFreeString( m_Methods[i].ppdata[j].szName );
		}

		free( m_Methods[i].ppdata );
	}
}

// This method is used to control the showing of context menus.
// 0x02 is the text context menu for input fields.
// The others are other context menu IDs. See the documentation for ShowContextMenu
// for the other ID names.
// We return S_FALSE for the text context menu ID so that it will display the
// context menu in input fields.
// For all of the others, we return S_OK, which tells the framework that we're
// taking care of putting up the menu. We don't want any other menus right now,
// so we just return S_OK so nothing happens.

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::ShowContextMenu( 
	/* [in] */ DWORD dwID,
	/* [in] */ DWORD x,
	/* [in] */ DWORD y,
	/* [in] */ IUnknown *pcmdtReserved,
	/* [in] */ IDispatch *pdispReserved,
	/* [retval][out] */ HRESULT *dwRetVal)
{
	x;
	y;
	pcmdtReserved;
	pdispReserved;
	if (dwRetVal != NULL)
	{
		switch (dwID)
		{
		case 0x02:	// Text context menu
			*dwRetVal = S_FALSE;

		case 0x04:
		case 0x08:
		case 0x20:
		case 0x30:
		default:
			*dwRetVal = S_OK;
		}
	}

	return S_OK;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::GetHostInfo( 
	/* [out][in] */ DWORD *pdwFlags,
	/* [out][in] */ DWORD *pdwDoubleClick)
{
	DWORD dwFlags = /*DOCHOSTUIFLAG_DIALOG |*/ //Disable selection of static text
		DOCHOSTUIFLAG_NO3DOUTERBORDER | 
		DOCHOSTUIFLAG_SCROLL_NO | //Disable scroll bars
		DOCHOSTUIFLAG_THEME | 
		DOCHOSTUIFLAG_DISABLE_HELP_MENU;

	*pdwFlags = dwFlags;
	*pdwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
	return S_OK;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::ShowUI( 
	/* [in] */ DWORD dwID,
	/* [in] */ IUnknown *pActiveObject,
	/* [in] */ IUnknown *pCommandTarget,
	/* [in] */ IUnknown *pFrame,
	/* [in] */ IUnknown *pDoc,
	/* [retval][out] */ HRESULT *dwRetVal)
{
	dwID;
	pActiveObject;
	pCommandTarget;
	pFrame;
	pDoc;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::HideUI()
{
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::UpdateUI()
{
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::EnableModeless( 
	/* [in] */ VARIANT_BOOL fEnable)
{
	fEnable;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::OnDocWindowActivate( 
	/* [in] */ VARIANT_BOOL fActivate)
{
	fActivate;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::OnFrameWindowActivate( 
	/* [in] */ VARIANT_BOOL fActivate)
{
	fActivate;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::ResizeBorder( 
	/* [in] */ long left,
	/* [in] */ long top,
	/* [in] */ long right,
	/* [in] */ long bottom,
	/* [in] */ IUnknown *pUIWindow,
	/* [in] */ VARIANT_BOOL fFrameWindow)
{
	left;
	top;
	right;
	bottom;
	pUIWindow;
	fFrameWindow;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::TranslateAccelerator( 
	/* [in] */ DWORD_PTR hWnd,
	/* [in] */ DWORD nMessage,
	/* [in] */ DWORD_PTR wParam,
	/* [in] */ DWORD_PTR lParam,
	/* [in] */ BSTR bstrGuidCmdGroup,
	/* [in] */ DWORD nCmdID,
	/* [retval][out] */ HRESULT *dwRetVal)
{
	hWnd;
	nMessage;
	wParam;
	lParam;
	bstrGuidCmdGroup;
	nCmdID;
	dwRetVal;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::GetOptionKeyPath( 
	/* [out] */ BSTR *pbstrKey,
	/* [in] */ DWORD dw)
{
	pbstrKey;
	dw;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::GetDropTarget( 
	/* [in] */ IUnknown *pDropTarget,
	/* [out] */ IUnknown **ppDropTarget)
{
	pDropTarget;
	*ppDropTarget = NULL;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::GetExternal( 
	/* [out] */ IDispatch **ppDispatch)
{
	AddRef();
	*ppDispatch = this;
	return S_OK;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::TranslateUrl( 
	/* [in] */ DWORD dwTranslate,
	/* [in] */ BSTR bstrURLIn,
	/* [out] */ BSTR *pbstrURLOut)
{
	dwTranslate;
	bstrURLIn;
	*pbstrURLOut = NULL;
	return E_NOTIMPL;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLDispatch::FilterDataObject( 
	/* [in] */ IUnknown *pDO,
	/* [out] */ IUnknown **ppDORet)
{
	pDO;
	*ppDORet = NULL;
	return E_NOTIMPL;
}

CHTMLImpl::CHTMLImpl()
	: m_PageStatus( HTTP_STATUS_CONTINUE )
	, m_Ready( false )
	, m_PageComplete( false )
{
}

void CHTMLImpl::SetReady( bool Ready )
{
	m_Ready = Ready;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLImpl::OnNavigateError( IDispatch* /*pDisp*/, VARIANT* URL, VARIANT* /*TargetFrameName*/, VARIANT* StatusCode, VARIANT_BOOL* Cancel )
{
	if (!m_Ready)
	{
		return S_OK;
	}

	//Get the error code. This can be difficult to interpret. It may be that 
	//m_PageStatus < HTTP_STATUS_BAD_REQUEST - OnNavigateError may be called with a non-error 
	//code. This seems to happen at random even though the web page renders properly. Also, 
	//according to the documentation for CHTMLView::OnNavigateError, the error code might 
	//not be available. 
	m_PageStatus = StatusCode->lVal;

	URL;
#ifdef WTLOG_ENABLED
	LOG_PURPLE( "CHTMLImpl at 0x%x: navigation error %s = %d (\"%s\"); URL = '%S'.", this, 
		GetPageStatusName().c_str(), m_PageStatus, GetPageStatusDescription().c_str(),
		(URL != NULL && URL->vt == VT_BSTR) ? URL->bstrVal : L"(unknown)" );
#endif

	assert( Cancel != NULL );
	*Cancel = VARIANT_TRUE;

	return S_OK;
}

/*virtual*/ HRESULT STDMETHODCALLTYPE CHTMLImpl::OnDocumentComplete( IDispatch* /*pDisp*/, VARIANT* URL )
{
	if (!m_Ready)
	{
		return S_OK;
	}

	URL;

#if 0 //Our callback is not called reliably and is basically worthless.
#ifdef WTLOG_ENABLED
	LOG_GREEN( "CHTMLImpl at 0x%x: document complete; URL = '%S'.", this,
		(URL != NULL && URL->vt == VT_BSTR) ? URL->bstrVal : L"unknown" );
#endif
#endif

	m_PageComplete = true;
	return S_OK;
}

std::string CHTMLImpl::GetPageStatusString() const
{
	return GetPageStatusString( m_PageStatus );
}

std::string CHTMLImpl::GetPageStatusName() const
{
	return GetPageStatusName( m_PageStatus );
}

std::string CHTMLImpl::GetPageStatusDescription() const
{
	return GetPageStatusDescription( m_PageStatus );
}

/*static*/ std::string WTCall CHTMLImpl::GetPageStatusString( DWORD PageStatus )
{
	std::ostringstream o;
	if (PageStatus & 0x80000000)
	{
		o << "0x" << std::hex;
	}
	o << PageStatus;
	return o.str();
}

#ifndef HTMLIMPL_DEFSTATUSNAME
#define HTMLIMPL_DEFSTATUSNAME(E) case E: return #E;
#endif

/*static*/ std::string WTCall CHTMLImpl::GetPageStatusName( DWORD PageStatus )
{
	switch (PageStatus)
	{
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_CONTINUE				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_SWITCH_PROTOCOLS		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_OK					)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_CREATED				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_ACCEPTED				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_PARTIAL				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_NO_CONTENT			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_RESET_CONTENT		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_PARTIAL_CONTENT		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_AMBIGUOUS			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_MOVED				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_REDIRECT				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_REDIRECT_METHOD		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_NOT_MODIFIED			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_USE_PROXY			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_REDIRECT_KEEP_VERB	)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_BAD_REQUEST			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_DENIED				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_PAYMENT_REQ			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_FORBIDDEN			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_NOT_FOUND			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_BAD_METHOD			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_NONE_ACCEPTABLE		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_PROXY_AUTH_REQ		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_REQUEST_TIMEOUT		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_CONFLICT				)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_GONE					)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_LENGTH_REQUIRED		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_PRECOND_FAILED		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_REQUEST_TOO_LARGE	)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_URI_TOO_LONG			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_UNSUPPORTED_MEDIA	)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_RETRY_WITH			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_SERVER_ERROR			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_NOT_SUPPORTED		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_BAD_GATEWAY			)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_SERVICE_UNAVAIL		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_GATEWAY_TIMEOUT		)
		HTMLIMPL_DEFSTATUSNAME(HTTP_STATUS_VERSION_NOT_SUP		)
		HTMLIMPL_DEFSTATUSNAME(INET_E_INVALID_URL				)
		HTMLIMPL_DEFSTATUSNAME(INET_E_NO_SESSION				)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CANNOT_CONNECT			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_RESOURCE_NOT_FOUND		)
		HTMLIMPL_DEFSTATUSNAME(INET_E_OBJECT_NOT_FOUND			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_DATA_NOT_AVAILABLE		)
		HTMLIMPL_DEFSTATUSNAME(INET_E_DOWNLOAD_FAILURE			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_AUTHENTICATION_REQUIRED	)
		HTMLIMPL_DEFSTATUSNAME(INET_E_NO_VALID_MEDIA			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CONNECTION_TIMEOUT		)
		HTMLIMPL_DEFSTATUSNAME(INET_E_INVALID_REQUEST			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_UNKNOWN_PROTOCOL			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_SECURITY_PROBLEM			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CANNOT_LOAD_DATA			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CANNOT_INSTANTIATE_OBJECT	)
		HTMLIMPL_DEFSTATUSNAME(INET_E_REDIRECT_FAILED			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_REDIRECT_TO_DIR			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CANNOT_LOCK_REQUEST		)
		HTMLIMPL_DEFSTATUSNAME(INET_E_USE_EXTEND_BINDING		)
		HTMLIMPL_DEFSTATUSNAME(INET_E_TERMINATED_BIND			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CODE_DOWNLOAD_DECLINED	)
		HTMLIMPL_DEFSTATUSNAME(INET_E_RESULT_DISPATCHED			)
		HTMLIMPL_DEFSTATUSNAME(INET_E_CANNOT_REPLACE_SFP_FILE	)
	}

	return "(unknown)";
}

#undef HTMLIMPL_DEFSTATUSNAME

/*static*/ std::string WTCall CHTMLImpl::GetPageStatusDescription( DWORD PageStatus )
{
	switch (PageStatus)
	{
		case HTTP_STATUS_CONTINUE				: return "OK to continue with request";
		case HTTP_STATUS_SWITCH_PROTOCOLS		: return "server has switched protocols in upgrade header";
		case HTTP_STATUS_OK						: return "request completed";
		case HTTP_STATUS_CREATED				: return "object created, reason = new URI";
		case HTTP_STATUS_ACCEPTED				: return "async completion (TBS)";
		case HTTP_STATUS_PARTIAL				: return "partial completion";
		case HTTP_STATUS_NO_CONTENT				: return "no info to return";
		case HTTP_STATUS_RESET_CONTENT			: return "request completed, but clear form";
		case HTTP_STATUS_PARTIAL_CONTENT		: return "partial GET furfilled";
		case HTTP_STATUS_AMBIGUOUS			    : return "server couldn't decide what to return";
		case HTTP_STATUS_MOVED					: return "object permanently moved";
		case HTTP_STATUS_REDIRECT			    : return "object temporarily moved";
		case HTTP_STATUS_REDIRECT_METHOD		: return "redirection w/ new access method";
		case HTTP_STATUS_NOT_MODIFIED		    : return "if-modified-since was not modified";
		case HTTP_STATUS_USE_PROXY				: return "redirection to proxy, location header specifies proxy to use";
		case HTTP_STATUS_REDIRECT_KEEP_VERB		: return "HTTP/1.1: keep same verb";
		case HTTP_STATUS_BAD_REQUEST			: return "Invalid syntax.";
		case HTTP_STATUS_DENIED					: return "Access denied.";
		case HTTP_STATUS_PAYMENT_REQ			: return "Payment required.";
		case HTTP_STATUS_FORBIDDEN				: return "Request forbidden.";
		case HTTP_STATUS_NOT_FOUND				: return "Object not found.";
		case HTTP_STATUS_BAD_METHOD				: return "Method is not allowed.";
		case HTTP_STATUS_NONE_ACCEPTABLE		: return "No response acceptable to client found.";
		case HTTP_STATUS_PROXY_AUTH_REQ			: return "Proxy authentication required.";
		case HTTP_STATUS_REQUEST_TIMEOUT		: return "Server timed out waiting for request.";
		case HTTP_STATUS_CONFLICT				: return "User should resubmit with more info.";
		case HTTP_STATUS_GONE					: return "Resource is no longer available.";
		case HTTP_STATUS_LENGTH_REQUIRED		: return "Server refused to accept request without a length.";
		case HTTP_STATUS_PRECOND_FAILED			: return "Precondition given in request failed.";
		case HTTP_STATUS_REQUEST_TOO_LARGE		: return "Request entity was too large.";
		case HTTP_STATUS_URI_TOO_LONG			: return "Request Uniform Resource Identifier (URI) too long.";
		case HTTP_STATUS_UNSUPPORTED_MEDIA		: return "Unsupported media type.";
		case HTTP_STATUS_RETRY_WITH				: return "Retry after doing the appropriate action.";
		case HTTP_STATUS_SERVER_ERROR			: return "Internal server error.";
		case HTTP_STATUS_NOT_SUPPORTED			: return "Server does not support the functionality required to fulfill the request.";
		case HTTP_STATUS_BAD_GATEWAY			: return "Error response received from gateway.";
		case HTTP_STATUS_SERVICE_UNAVAIL		: return "Temporarily overloaded.";
		case HTTP_STATUS_GATEWAY_TIMEOUT		: return "Timed out waiting for gateway.";
		case HTTP_STATUS_VERSION_NOT_SUP		: return "HTTP version not supported.";
		case INET_E_INVALID_URL					: return "URL string is not valid.";
		case INET_E_NO_SESSION					: return "No session found.";
		case INET_E_CANNOT_CONNECT				: return "Unable to connect to server.";
		case INET_E_RESOURCE_NOT_FOUND			: return "Requested resource is not found.";
		case INET_E_OBJECT_NOT_FOUND			: return "Requested object is not found.";
		case INET_E_DATA_NOT_AVAILABLE			: return "Requested data is not available.";
		case INET_E_DOWNLOAD_FAILURE			: return "Failure occurred during download.";
		case INET_E_AUTHENTICATION_REQUIRED		: return "Requested navigation requires authentication.";
		case INET_E_NO_VALID_MEDIA				: return "Required media not available or valid.";
		case INET_E_CONNECTION_TIMEOUT			: return "Connection timed out.";
		case INET_E_INVALID_REQUEST				: return "Request is invalid.";
		case INET_E_UNKNOWN_PROTOCOL			: return "Protocol is not recognized.";
		case INET_E_SECURITY_PROBLEM			: return "Navigation request has encountered a security issue.";
		case INET_E_CANNOT_LOAD_DATA			: return "Unable to load data from the server.";
		case INET_E_CANNOT_INSTANTIATE_OBJECT	: return "Unable to create an instance of the object.";
		case INET_E_REDIRECT_FAILED				: return "Attempt to redirect the navigation failed.";
		case INET_E_REDIRECT_TO_DIR				: return "Navigation redirected to a directory.";
		case INET_E_CANNOT_LOCK_REQUEST			: return "Unable to lock request with the server.";
		case INET_E_USE_EXTEND_BINDING			: return "Reissue request with extended binding.";
		case INET_E_TERMINATED_BIND				: return "Binding is terminated.";
		case INET_E_CODE_DOWNLOAD_DECLINED		: return "Permission to download is declined.";
		case INET_E_RESULT_DISPATCHED			: return "Result is dispatched.";
		case INET_E_CANNOT_REPLACE_SFP_FILE		: return "Cannot replace a protected System File Protection (SFP) file.";
	}

	return "(unknown)";
}

}
}
