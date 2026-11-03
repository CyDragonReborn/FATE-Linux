/*
	HttpWebRequest.cpp

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005

	TODO: Uses unsafe string functions strcpy strcat etc.
	TODO: Uses malloc instead of new
 */
#include "WT/stdafx.h"

#include <crtdbg.h>
#ifdef _DEBUG
#ifndef WTLOG_ENABLED
#define WTLOG_ENABLED
#endif
#endif
// #define DO_LOG_RAII

#include <windows.h>

#include <iostream>
#include <sstream>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "WT/System/Timer.h"
#include "WT/System/IO/IO.h"
#include "WT/System/Net/CookieContainer.h"
#include "WT/System/Net/HttpWebRequest.h"
#include "WT/System/Net/WebRequestEnum.h"
#include "WT/System/Logging/Logging.h"

#pragma comment( lib, "wininet" )

namespace WT
{
namespace System
{
namespace Net
{
using namespace WT;
using namespace WT::System::IO;

// Forward declaration
static void Logf (const int8* const FormatString, ...);
static int32 H2int32(const HANDLE h);

//
// Comment on all of the CRAII* class:
//    These classes all "do" the same thing: they exist so that
//    the RAII (http://www.hackcraft.net/raii/
//    or http://en.wikipedia.org/wiki/Resource_Acquisition_Is_Initialization)
//    can be done with objects that need to be "fclosed", CloseHandled, etc.
// 
//    In all cases, if you have something and you need to "foo" it when the
//    method ends, you should use the CRAIIfooMe class.
// 
//

class CRAIIfcloseMe
{
public:
	CRAIIfcloseMe (FILE*f=0):m_f(f)
	{
	}
	~CRAIIfcloseMe ()
	{
		End();
	}
	void Init(FILE* f)
	{
		End();
		m_f = f;
	}
	void End ()
	{
		if (m_f)
		{
			fclose (m_f);
		}
		m_f = 0;
	}

private:
	FILE* m_f;
};


class CRAIIfreeMe
{
public:
	CRAIIfreeMe(void* Ptr):m_Ptr(Ptr) { }
	~CRAIIfreeMe() 
	{ 
#ifdef DO_LOG_RAII
		Logf ("CRAIIfreeMe: free ptr %p\n", m_Ptr);
#endif
		free (m_Ptr); 
		m_Ptr=0; 
	} // free handles nulls ok
private:
	void* m_Ptr;
};

class CRAIIInternetCloseHandleMeZero
{
public:
	CRAIIInternetCloseHandleMeZero(HANDLE* Handle):m_Handle(Handle) { }
	~CRAIIInternetCloseHandleMeZero() 
	{ 
		if (!m_Handle)
		{
#ifdef DO_LOG_RAII
			Logf ("CRAIIInternetCloseHandleMe: closing NULL; do nothing\n");
#endif
			return;
		}
#ifdef DO_LOG_RAII
		Logf ("CRAIIInternetCloseHandleMe: closing %d\n", H2int32 (*m_Handle));
#endif
		if (*m_Handle) 
		{
			InternetCloseHandle (*m_Handle); 
		}
		*m_Handle = 0; 
		m_Handle = 0;
	}
private:
	HANDLE* m_Handle;
};

static bool FileExists (const string& Filename)
{
	struct _stat sd;
	const int Status = _stat (Filename.c_str(), &sd);
	bool Retval = (Status == 0);
	return Retval;
}


//
// Logging for this file:
//



#ifdef WTLOG_ENABLED
static void Logf (const int8* const FormatString, ...) 
{
	int8 Buff[1024];
	va_list args;
	va_start(args, FormatString);

	_vsnprintf( Buff, sizeof(Buff), FormatString, args);

	LOG (Buff);
}
#else
static void Logf (const int8* const FormatString, ...) 
{
}
#endif


// Used with the DateStamp functions
const char* const KLastModified = "Last-Modified: ";
const char* const KContentLength = "Content-Length: ";
const char* const KFileDataVersion = "File-Data-Version: ";
const char* const KUrl = "Url: ";


// A trivial little class that's the glue for StartDownloadFromMainThread
// to work
class CHttpWebRequest_ThreadHelper
{
public:
	static DWORD __stdcall StartDownloadFromThreadHelper(void* ThisVoid)
	{
		CHttpWebRequest* This = (CHttpWebRequest*)ThisVoid;
		This->StartDownloadFromWorkerThread ();
		return 0;
	}
};

// Trivial function to convert a handle into an int.
// It's here so that when we do a 64-bit port I can 
// find all cases of converting handles to ints.
static int32 H2int32(const HANDLE h)
{
#pragma warning (push)
#pragma warning (disable:4311)
	return (int32)h;
#pragma warning (pop)
}

// Looks through a standard HTTP return buffer looking for data.
// The data should be HTTP_QUERY_RAW_HEADERS (without a CRLF)
//
static const int8* const QueryBuffer (const int8* const Buffer, const int8* const LookFor, int8* TempBuffer=0, const int32 TempBufferLen=0)
{
	if (!Buffer || !LookFor)
	{
		return 0;
	}

	// Special checks
	if (strcmp (LookFor, "Status-Code")==0)
	{
		// Status code is in the first line
		// HTTP/1.1 200 OK
		const int8* const pSpace1 = strchr (Buffer, ' ');
		const int8* const pSpace2 = pSpace1 ? strchr (pSpace1+1, ' ') : 0;
		const int32 len = (int32)(pSpace2 - pSpace1);
		if (pSpace1 && pSpace2 && len < TempBufferLen-2 && TempBuffer)
		{
			strncpy (TempBuffer, pSpace1+1, len);
			TempBuffer[len-1] = '\0';
			return TempBuffer;
		}
		else
		{
			Logf ("Error: QueryBuffer: unable to find the status code in %s\n", Buffer);
			return 0;
		}

	}
	else if (strcmp (LookFor, "Status-Text")==0)
	{
		// Status code is in the first line
		// HTTP/1.1 200 OK
		const int8* const pSpace1 = strchr (Buffer, ' ');
		const int8* const pSpace2 = pSpace1 ? strchr (pSpace1+1, ' ') : 0;
		if (pSpace1 && pSpace2)
		{
			return pSpace2+1;
		}
		else
		{
			Logf ("Error: QueryBuffer: unable to find the status text in %s\n", Buffer);
			return 0;
		}

	}

	//
	// Normal header values
	//
	const size_t Len = strlen (LookFor);
	for (const char*p = Buffer; p&&*p; p+=strlen(p)+1)
	{
		// p is, e.g., "LookFor: <value>"
		if ((strncmp (p, LookFor, Len) == 0) && p[Len]==':')
		{
			const int8* const Retval = p+Len+1;
			return Retval;
		}
	}
	return 0;
}

static int32 QueryBufferInt (const int8* const Buffer, const int8* const LookFor, const int32 DefaultValue)
{
	const int32 KStrLen = 64;
	int8 Str [KStrLen];
	Str[0] = '\0';
	const char* const pValue = QueryBuffer (Buffer, LookFor, Str, KStrLen);
	const int32 Retval = (pValue && pValue[0]) ? atoi (pValue) : DefaultValue;
	return Retval;
}

static int64 QueryBufferInt64 (const int8* const Buffer, const int8* const LookFor, const int64 DefaultValue)
{
	const int32 KStrLen = 64; //63bytes@ 3.3bits/char=209 bits -- more than enough for anything.
			// 64 bits=19.2 chars
	int8 Str [KStrLen];
	Str[0] = '\0';
	const char* const pValue = QueryBuffer (Buffer, LookFor, Str, KStrLen);
	const int64 Retval = (pValue && pValue[0]) ? _atoi64 (pValue) : DefaultValue;
	return Retval;
}

// non-const version of QueryBuffer
static int8* QueryBufferNC (int8* Buffer, const int8* const LookFor, int8* TempBuffer=0, const int32 TempBufferLen=0)
{
	int8* Retval = const_cast<int8 *>(QueryBuffer (Buffer, LookFor, TempBuffer, TempBufferLen));
	return Retval;
}

#define MEMCHECK() assert (_CrtCheckMemory())
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
#define GUARDCHECK() assert (m_StartString[0] == '*')
#else
#define GUARDCHECK() assert (TRUE)
#endif
#define CHECK() MEMCHECK(); GUARDCHECK()

WT_DEFINE_CLASS(CHttpWebRequest)

#pragma warning(disable:4355) // 'this' : used in base member initializer list
// This is turn off because the Response variable needs a pointer back to the parent object.
CHttpWebRequest::CHttpWebRequest()
	:m_Response(this)
	,m_AbortOnDelete(true)
	,m_StartedDownload(false)
	,m_hWorkerThread(0)
	,m_Timeout(-1)
	,m_UserData(0)
	,m_ReadTimeout(KConstants::KAbortIndividualReadTimeout)
	,mtw_ContentLength(0)
	,mtw_ContentLengthSave(0)
	,mtw_HaveContentLength(false)
	,mtw_HaveResponse(false)
	,mtw_hInternet(0)
	,mtw_hSite(0)
	,mtw_hRequest(0)
	,mtw_IsDone(false)
	,mtw_NBytesDownloaded(0)
	,mtw_pReadTimer(0)
	,mtw_StatusAbort(false)
	,mtw_StatusCode(WT::System::Net::HttpStatusCode::NotSet)
	,mtw_StatusError(false)
	,mtw_StatusPause(false)
	,mtw_pThreadTimer(0)
	,mtr_AbortRequested(false)
	,mtr_AllowAutoRedirect(true)
	,mtr_AsyncResult(this)
	,mtr_DefaultMaximumResponseHeadersLength(64)
	,mtr_DLFilename("")
	,mtr_HeaderAccept("")				// default is blank
	,mtr_HeaderConnection("")
	,mtr_HeaderContentType("")
	,mtr_HeaderExpect("")
	,mtr_HeaderIfModifiedSince(CDateTime::Now().ToString("r"))
	,mtr_HeaderIfModifiedSinceSet(false)
	,mtr_HeaderMediaType("")
	,mtr_HeaderProtocolVersion("HTTP/1.1")
	,mtr_HeaderRangeBegin(-1)
	,mtr_HeaderRangeEnd(-1)
	,mtr_HeaderRangeSpecifier("")
	,mtr_HeaderReferer("")
	,mtr_HeaderTransferEncoding("")
	,mtr_HeaderUserAgent("Mozilla/4.0 (compatible; MSIE 5.0; CDA 6.0)")
	,mtr_MaximumResponseHeadersLength(-1)
	,mtr_Method("GET")
	,mtr_PauseRequested(false)
	,mtr_pAsyncEvent(new CAsyncEvent)

	,mmt_pCookies(0)
	,mmt_pAsyncObj(0)
	,mmt_pNotifyOnDelete(0)
	,mmt_Status(WT::System::Net::DownloadStatus::KNotStarted)

	,mtw_DidSaveFileData(false)
	,mtw_DidCheckFileDataExists(false)
	,mtw_FileDataExists(false)
	,mtw_HaveDateStampSave(false)
	,mtw_HaveOldContentLengthSave(false)

{
	mtw_pStream = new CMemoryStream();
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	strcpy (m_StartString, "*PDS:HttpWebRequest");
	strcpy (m_EndString, "END:HttpENDRequEND");
	CHECK();

	DWORD OldProtect=0;
	const BOOL Status = VirtualProtect (&m_GuardPage [sizeof (m_GuardPage)/2], 1, PAGE_NOACCESS, &OldProtect);
	assert (Status);
#endif
}

CHttpWebRequest::CHttpWebRequest(const CUri& Uri)
	:m_Response(this)
	,m_AbortOnDelete(true)
	,m_StartedDownload(false)
	,m_hWorkerThread(0)
	,m_Timeout(-1)
	,m_UserData(0)
	,m_ReadTimeout(KConstants::KAbortIndividualReadTimeout)
	,mtw_ContentLength(0)
	,mtw_ContentLengthSave(0)
	,mtw_HaveContentLength(false)
	,mtw_HaveResponse(false)
	,mtw_hInternet(0)
	,mtw_hSite(0)
	,mtw_hRequest(0)
	,mtw_IsDone(false)
	,mtw_NBytesDownloaded(0)
	,mtw_pReadTimer(0)
	,mtw_StatusAbort(false)
	,mtw_StatusCode(WT::System::Net::HttpStatusCode::NotSet)
	,mtw_StatusError(false)
	,mtw_StatusPause(false)
	,mtw_pThreadTimer(0)
	,mtr_AbortRequested(false)
	,mtr_AllowAutoRedirect(true)
	,mtr_AsyncResult(this)
	,mtr_DefaultMaximumResponseHeadersLength(64)
	,mtr_HeaderAccept("")				// default is blank
	,mtr_HeaderConnection("")
	,mtr_HeaderContentType("")
	,mtr_HeaderExpect("")
	,mtr_HeaderIfModifiedSince(CDateTime::Now().ToString("r"))
	,mtr_HeaderIfModifiedSinceSet(false)
	,mtr_HeaderMediaType("")
	,mtr_HeaderProtocolVersion("HTTP/1.1")
	,mtr_HeaderRangeBegin(-1)
	,mtr_HeaderRangeEnd(-1)
	,mtr_HeaderRangeSpecifier("")
	,mtr_HeaderReferer("")
	,mtr_HeaderTransferEncoding("")
	,mtr_HeaderUserAgent("Mozilla/4.0 (compatible; MSIE 5.0; CDA 6.0)")
	,mtr_MaximumResponseHeadersLength(-1)
	,mtr_Method("GET")
	,mtr_PauseRequested(false)
	,mtr_pAsyncEvent(new CAsyncEvent)

	,mmt_pCookies(0)
	,mmt_pAsyncObj(0)
	,mmt_pNotifyOnDelete(0)
	,mmt_Status(WT::System::Net::DownloadStatus::KNotStarted)

	,mtw_DidSaveFileData(false)
	,mtw_DidCheckFileDataExists(false)
	,mtw_FileDataExists(false)
	,mtw_HaveDateStampSave(false)
	,mtw_HaveOldContentLengthSave(false)

	// Uri is initialized here but not in the default constructor.
	,mtr_Uri(Uri)
{
	mtw_pStream = new CMemoryStream();
	const string Scheme = mtr_Uri.GetScheme ();
	assert (Scheme == "http" || Scheme == "https");
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	strcpy (m_StartString, "*PDS:HttpWebRequest");
	strcpy (m_EndString, "END:HttpENDRequEND");
	CHECK();

	DWORD OldProtect=0;
	const BOOL Status = VirtualProtect (&m_GuardPage [sizeof (m_GuardPage)/2], 1, PAGE_NOACCESS, &OldProtect);
	assert (Status);
#endif
}

CHttpWebRequest::~CHttpWebRequest()
{
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	CHECK();
	strcpy (m_StartString, "*~~S:HttpWebReque~~");
	strcpy (m_EndString, "~~D:HttpENDRequE~~");
#endif
	if (m_AbortOnDelete)
	{
		Abort();
	}
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	strcpy (m_StartString, "*ABR:HttpWebRequ~~~");
	strcpy (m_EndString, "*ABR:HttpENDRequ~~~");
#endif

	delete mtw_pStream;
	mtw_pStream = 0;

	delete mtw_pReadTimer;
	mtw_pReadTimer = 0;

	delete mtw_pThreadTimer;
	mtw_pThreadTimer = 0;

	m_UserData = 0;

	if (mmt_pNotifyOnDelete)
	{
		//
		// If we're in a callback (which is when this flag is set),
		// they above all, don't delete the callback object!
		//
		*mmt_pNotifyOnDelete = true;
	}
	else
	{
		delete mtr_pAsyncEvent;
	}
	mtr_pAsyncEvent = 0;

#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	strcpy (m_StartString, "*ABR:~~~pWebRequ~~~");
	strcpy (m_EndString, "*ABR:~~~pENDRequ~~~");

	DWORD OldProtect=0;
	const BOOL Status = VirtualProtect (&m_GuardPage [sizeof (m_GuardPage)/2], 1, PAGE_READWRITE, &OldProtect);
	assert (Status);
#endif
}

void CHttpWebRequest::Abort() 
{ 
	if (!m_hWorkerThread)
	{
		Logf ("HttpWebRequest::Abort.  No worker thread, so exit right away\n");
		mtw_StatusAbort = true;
		return;
	}

	CHECK();
	Logf ("HttpWebRequest::Abort.  mtw_StatusAbort %d WaitTimeout %d\n", mtw_StatusAbort, KConstants::KAbortWaitTimeout);

	mmt_Status = DownloadStatus::KDoneFail;
	mtr_AbortRequested = true; 
	// Wait for how long?
	const int32 KSleepTime (10);
	int32 TimeLeft = KConstants::KAbortWaitTimeout;
	while (!mtw_StatusAbort && TimeLeft > 0)
	{
		Sleep (KSleepTime);
		TimeLeft -= KSleepTime;
	}
	Logf ("HttpWebRequest::Abort.  Slept %d mtw_StatusAbort %d TimeLeft %d IsDone %d\n", (KConstants::KAbortWaitTimeout - TimeLeft), mtw_StatusAbort, TimeLeft, GetIsDone());

	// Didn't really abort; kill it dead
	if (!mtw_StatusAbort)
	{
		// Order is important; KillWorkerThread will try to clean up the thread
		// by hand; this in turn checks the download size (which will be 
		// ignored if we're already in an 'abort' mode)
		mtw_StatusAbort = true;
		CHECK();
		KillWorkerThread();
		CHECK();
	}

	assert (GetIsDone());
}


bool CHttpWebRequest::AbortIfTimeout()
{
	bool Retval = false;
	if (m_Timeout > 0)
	{
		const int64 ThreadTime = GetThreadTime();
		if (ThreadTime > m_Timeout)
		{
			Logf ("HttpWebRequest::AbortIfTimeout: abort because THREAD TIME %d > Timeout %d\n", (int)ThreadTime, m_Timeout);
			Abort();
			Sleep (1);
			assert (GetIsDone());
			Retval = true;
		}
	}

	if (!Retval && m_ReadTimeout > 0)
	{
		const int64 ReadTime = GetReadTime();
		if (ReadTime > m_ReadTimeout)
		{
			Logf ("HttpWebRequest::AbortIfTimeout: abort because READ TIME %d > Timeout %d\n", (int)ReadTime, m_ReadTimeout);
			Abort ();
			Sleep (1);
			assert (GetIsDone());
			Retval = true;
		}
	}

	return Retval;
}

void CHttpWebRequest::AddCallback (CAsyncCallback* Callback)
{
	(*mtr_pAsyncEvent) += Callback;
}

void CHttpWebRequest::AddRange(const int64 Range)
{
	if (Range < 0)
	{
		// from Range to end
		mtr_HeaderRangeBegin = -Range;
		mtr_HeaderRangeEnd = -1;
	}
	else
	{
		// from begin to Range
		mtr_HeaderRangeBegin = 0;
		mtr_HeaderRangeEnd = Range;
	}
	mtr_HeaderRangeSpecifier = "bytes";
}


void CHttpWebRequest::AddRange(const int64 RangeBegin, const int64 RangeEnd)
{
	mtr_HeaderRangeBegin = RangeBegin;
	mtr_HeaderRangeEnd = RangeEnd;
	mtr_HeaderRangeSpecifier = "bytes";
}


void CHttpWebRequest::AddRange(const string& RangeSpecifier, const int64 Range)
{
	if (Range < 0)
	{
		// from Range to end
		mtr_HeaderRangeBegin = Range;
		mtr_HeaderRangeEnd = -1;
	}
	else
	{
		// from begin to Range
		mtr_HeaderRangeBegin = 0;
		mtr_HeaderRangeEnd = Range;
	}
	mtr_HeaderRangeSpecifier = RangeSpecifier;
}


void CHttpWebRequest::AddRange(const string& RangeSpecifier, const int64 RangeBegin, const int64 RangeEnd)
{
	mtr_HeaderRangeBegin = RangeBegin;
	mtr_HeaderRangeEnd = RangeEnd;
	mtr_HeaderRangeSpecifier = "bytes";
}

void CHttpWebRequest::Begin()
{
	if (!m_StartedDownload)
	{
		StartDownloadFromMainThread();
		m_StartedDownload = true;
	}
}


const IAsyncResult* CHttpWebRequest::BeginDownloadToFile (CAsyncCallback* Callback, void* pObj, const string& DLFilename)
{
	mtr_DLFilename = DLFilename;
	const IAsyncResult* Retval = BeginGetResponse(Callback, pObj);
	return Retval;
}

const IAsyncResult*  CHttpWebRequest::BeginGetResponse (CAsyncCallback* Callback, void* pObj)
{
	CHECK();
	if (Callback)
	{
		(*mtr_pAsyncEvent) += Callback;
	}
	CHECK();
	if (pObj)
	{
		mmt_pAsyncObj = pObj;
	}
	Begin();
	if (!m_StartedDownload)
	{
		StartDownloadFromMainThread();
		m_StartedDownload = true;
	}
	CHECK();
	return& mtr_AsyncResult;
}


bool CHttpWebRequest::CallEventCallbacks()
{
	//
	// Save the date stamp / file data (if we haven't already)
	//
	const bool ShouldSaveFD = GetShouldSaveFileData ();
	if (ShouldSaveFD)
	{
		
		const CHttpWebResponse& Resp = GetOriginalHttpResponse ();
		const string UrlStr = GetRequestUri ().ToString ();
		const string LMStr = Resp.GetHeaderValue ("Last-Modified", "");
		const string CLStr = Resp.GetHeaderValue ("Content-Length", "0");
		const int CL = atoi (CLStr.c_str());
		if (LMStr != "" && !mtw_DidSaveFileData)
		{
			SaveFileData (LMStr.c_str(), CL, UrlStr.c_str());
			mtw_DidSaveFileData = true;
		}
	}

	//
	// If we're done then copy the file over
	//
	if (GetPercentDone () == 100.0f)	// 100% done doesn't mean that the download worked 
											// -- only that it's not going to ever be more done
	{
		const HttpStatusCode::Enum HttpStatus = GetStatusCode ();
		const bool HttpStatusBad = GetStatusIsBad();
		if(!HttpStatusBad && AmDoingCache())
		{
			switch (HttpStatus)
			{
			case HttpStatusCode::OK:
				ReplaceFile ();
				break;
			case HttpStatusCode::NotModified:
				DeleteCacheFile ();
				break;

			case HttpStatusCode::PartialContent:
				// Partial?  In the sense that (overwhelming chances are)
				// we _asked_ for only part of the file -- for example,
				// to finish a download.
				ReplaceFile ();
				break;
			}
		} // end if StatusBad is false

		// Update the download status
		mmt_Status = (!HttpStatusBad && HttpStatus >= 200 && HttpStatus <= 299) ? DownloadStatus::KDoneOk : DownloadStatus::KDoneFail;
	}

	bool NotifyOnDelete=false;
	mmt_pNotifyOnDelete = &NotifyOnDelete;
	CAsyncEvent* pAsyncEventOld = mtr_pAsyncEvent;

	CHECK();
	(*mtr_pAsyncEvent) ( &mtr_AsyncResult ); 
	CHECK();
	if (NotifyOnDelete)
	{
		// The object went away -- get rid of the Async object too!
		//this = 0; // I simply don't have a this pointer any more -- it's gone, gone, gone!
		delete pAsyncEventOld;
	}
	else
	{
		mmt_pNotifyOnDelete = 0;
	}
	CHECK();
	return NotifyOnDelete;
}

int64 CHttpWebRequest::CacheFileSize ()
{
	struct __stat64 sd;
	const int Status = _stat64 (mtr_DLFilename.c_str(), &sd);
	const int64 Retval = Status==0 ? sd.st_size : 0;
	return Retval;		
}

void CHttpWebRequest::DeleteCacheFile()
{
	if (AmDoingCache())
	{
		unlink (mtr_DLFilename.c_str());
	}
}


void CHttpWebRequest::DeleteFileDataFile()
{
	if(AmDoingCache())
	{
		unlink (mtr_FileDataFilename.c_str());
	}
}


bool CHttpWebRequest::DownloadIsOk() const
{
	const float Pct = GetPercentDone();
	const int32 Code = GetStatusCode ();
	const bool Retval = Pct>=100.0f && !GetStatusIsBad() && Code>=200 && Code<=299;
	return Retval;
}

void CHttpWebRequest::EndDownloadWork ()
{
	CHECK();
	mtw_pReadTimer->Pause();
	CHECK();
	mtw_pThreadTimer->Pause();
	CHECK();

	// Does the finializing of the variables if needed
	if (mtw_hRequest)
	{
		InternetCloseHandle (mtw_hRequest);
		mtw_hRequest = 0;
	}
	if (mtw_hSite)
	{
		InternetCloseHandle (mtw_hSite);
		mtw_hSite = 0;
	}
	if (mtw_hInternet)
	{
		InternetCloseHandle (mtw_hInternet);
		mtw_hInternet = 0;
	}
	CHECK();
	//
	// Done
	//
	if (mtw_HaveContentLength)
	{
		assert (mtw_StatusAbort || mtw_StatusError || mtw_StatusPause || mtw_ContentLength == mtw_NBytesDownloaded);
	}
	else
	{
		CHECK();
		mtw_ContentLength = mtw_NBytesDownloaded;
		if (!mtw_ContentLengthSave)
		{
			mtw_ContentLengthSave = mtw_ContentLength;
		}
	}

	mtw_IsDone = true;
	m_hWorkerThread = 0;

	//
	// Shut Down
	//
	CHECK();
	Logf ("HttpWebRequest::DL: done.  ContentLength %I64d; call callbacks?\n", mtw_ContentLength);

	if (!mtw_StatusPause)
	{
		CallEventCallbacks();
	}
	CHECK();
}


CWebResponse& CHttpWebRequest::EndGetResponse(const IAsyncResult*)
{
	CHECK();
	return m_Response;
}


CHttpWebResponse& CHttpWebRequest::EndGetHttpResponse(const IAsyncResult*)
{
	CHECK();
	return m_Response;
}


const string& CHttpWebRequest::GetAccept() const 
{ 
	CHECK();
	return mtr_HeaderAccept; 
}


bool CHttpWebRequest::GetAllowAutoRedirect() const 
{ 
	CHECK();
	return mtr_AllowAutoRedirect; 
}


void* CHttpWebRequest::GetAsyncObj() 
{ 
		return mmt_pAsyncObj; 
}

int64 CHttpWebRequest::GetBytesDownloaded () const
{
	CHECK();
	return mtw_NBytesDownloaded;
}


int64 CHttpWebRequest::GetBytesDownloadedTotal() const
{
	CHECK();
	const int64 Offset = (mtr_HeaderRangeBegin > 0 && mtr_HeaderRangeEnd < 0) ? mtr_HeaderRangeBegin : 0;
	const int64 Retval = mtw_NBytesDownloaded + Offset;
	return Retval;
}


const string& CHttpWebRequest::GetConnection() const 
{ 
	CHECK();
	return mtr_HeaderConnection; 
}


int64 CHttpWebRequest::GetContentLength() const 
{ 
	CHECK();
	if (mtw_HaveContentLength) 
	{
		return mtw_ContentLength; 
	}
	return -1; 
}


int64 CHttpWebRequest::GetContentLengthSave() const
{
	CHECK();
	return mtw_ContentLengthSave;
}


const string& CHttpWebRequest::GetContentType() const 
{ 
	CHECK();
	return mtr_HeaderContentType; 
}


WT::System::Net::CCookieContainer* CHttpWebRequest::GetCookieContainer() 
{ 
	CHECK();
	return mmt_pCookies; 
}

int32 CHttpWebRequest::GetDefaultMaximumResponseHeadersLength() const 
{ 
	CHECK();
	return mtr_DefaultMaximumResponseHeadersLength; 
}


const string& CHttpWebRequest::GetExpect() const 
{ 
	CHECK();
	return mtr_HeaderExpect; 
}


bool CHttpWebRequest::GetHaveResponse() const
{
	CHECK();
	return mtw_HaveResponse; 
}


vector<string>& CHttpWebRequest::GetHeaders() 
{ 
	CHECK();
	return m_Headers; 
}


CHttpWebResponse& CHttpWebRequest::GetHttpResponse()
{
	CHECK();
	GetResponse();
	CHECK();
	return m_Response;
}


const string& CHttpWebRequest::GetHttpsCertificate() const 
{ 
	CHECK();
	return mtw_HttpsCert; 
}

const string& CHttpWebRequest::GetHttpsSubject() const 
{ 
	CHECK();
	return mtw_HttpsSubject; 
}

const string& CHttpWebRequest::GetHttpsIssuer() const 
{ 
	CHECK();
	return mtw_HttpsIssuer; 
}

const string& CHttpWebRequest::GetIfModifiedSince() const 
{ 
	CHECK();
	return mtr_HeaderIfModifiedSince; 
}


const bool CHttpWebRequest::GetIsDeletable() const
{
	CHECK();
	if (GetPercentDone () != 100.0f)
	{
		return false;
	}
	if (m_hWorkerThread)
	{
		return false;
	}
	return true;
}


bool CHttpWebRequest::GetIsDone() const 
{ 
	return mtw_IsDone; 
}


int32 CHttpWebRequest::GetMaximumResponseHeadersLength() const
{
	CHECK();
	const int32 Retval = (mtr_MaximumResponseHeadersLength == -1) ? 
		GetDefaultMaximumResponseHeadersLength()
		: mtr_MaximumResponseHeadersLength;
	return Retval;
}

const string& CHttpWebRequest::GetMediaType() const 
{ 
	CHECK();
	return mtr_HeaderMediaType; 
}


const string& CHttpWebRequest::GetMethod() const 
{ 
	CHECK();
	return mtr_Method; 
}

CHttpWebResponse& CHttpWebRequest::GetOriginalHttpResponse ()
{
	CHECK();
	return m_Response;
}


const IAsyncResult* CHttpWebRequest::GetOriginalAsyncResult ()
{
	CHECK();
	return& mtr_AsyncResult;
}


// Conceptually returns 100 * Value/DivideBy but with the 
// following extra properties:
//  - the return value is _always_ between 1 and 99
static float32 Percent (const int64 Value, const int64 DivideBy)
{
	CHECK();
	float32 Retval = 1.0f;
	if (DivideBy >= 1)
	{
		Retval = (float32)(((float64)Value / (float64)DivideBy) * 98 + 1);
		if (Retval < 1.0f) 
		{
			Retval = 1.0f;
		}
		else if (Retval > 99.0f)
		{
			Retval = 99.0f;
		}
	}
	return Retval;
}


float32 CHttpWebRequest::GetPercentDone() const
{
	CHECK();
	const bool IsDone = GetIsDone();
	// Offset is used when we're downloading the rest of a file
	const int64 Offset = (mtr_HeaderRangeBegin > 0 && mtr_HeaderRangeEnd < 0) ? mtr_HeaderRangeBegin : 0;
	const int64 TotalLen = (GetContentLength() > 0) ? GetContentLength()+Offset : GetContentLengthSave();
	const int64 CurrLen = mtw_NBytesDownloaded; //GetResponseStream()->GetLength();
	const HttpStatusCode::Enum StatusCode = GetStatusCode();

	float32 Retval = Percent (Offset, GetContentLengthSave()); // the caller-specified return value
	if (IsDone)
	{
		Retval = 100.0f;
		Logf ("GetPercentDone: IsDone so retval %6.2f\n", Retval);
	}
	else if (StatusCode < 0)
	{
		if (mtw_ContentLengthSave<=0 || Offset<=0)
		{
			Logf ("GetPercentDone: No status code + no caller-specified (offset %I64d save %I64d) so retval %6.2f\n", Offset, mtw_ContentLengthSave, Retval);
			Retval = 0.0f;
		}
		else
		{
			Logf ("GetPercentDone: No status code but caller-specified (offset %I64d save %I64d) so retval %6.2f\n", Offset, mtw_ContentLengthSave, Retval);
		}
	}
	else
	{
		// convert to 1..99 value
		if (TotalLen > 0)
		{ 
			Retval = Percent (CurrLen+Offset, TotalLen);
			Logf ("GetPercentDone: (normal) TotalLen %I64d  Offset %I64d Save %I64d CurrLen %I64d Retval %6.2f\n",
				TotalLen, Offset, mtw_ContentLengthSave, CurrLen, Retval);
		}
		else
		{
			Retval = 22.0; // Just make something up
			Logf ("GetPercentDone: normal+no len; returning %6.2f\n", Retval);
		}
	}
	Logf ("GetPercentDone: final return %6.2f\n", Retval);
	CHECK();
	return Retval;
}


const string& CHttpWebRequest::GetProtocolVersion() const 
{ 
	CHECK();
	return mtr_HeaderProtocolVersion; 
}


const string& CHttpWebRequest::GetReferer() const 
{ 
	CHECK();
	return mtr_HeaderReferer; 
}


WT::System::IO::CStream& CHttpWebRequest::GetRequestStream() 
{ 
	CHECK();
	return mtr_RequestStream; 
}


int64 CHttpWebRequest::GetReadTime() const
{
	CHECK();
	if (!mtw_pReadTimer) 
	{
		return 0;
	}
	return mtw_pReadTimer->GetTotalTime();
}

const CUri& CHttpWebRequest::GetRequestUri () const
{
	CHECK();
	return mtr_Uri;
}

CWebResponse& CHttpWebRequest::GetResponse()
{
	CHECK();
	if (!m_StartedDownload)
	{
		StartDownloadFromMainThread();
		m_StartedDownload = true;
	}

	// Convert async download to sync by checking GetIsDone() and sleeping.
	// Note that the maximum sleep time here is one second.
	CHECK();
	while (!GetIsDone())
	{

		const int64 ThreadTime = GetThreadTime();
		const int32 KDefaultTimeToSleep = 1000;
		const int32 TimeToSleep = (m_Timeout < 0 || ThreadTime < (m_Timeout-KDefaultTimeToSleep)) ? KDefaultTimeToSleep : (int)(m_Timeout-ThreadTime);
		if (TimeToSleep > 0) // this will happen when the remaining time dips to zero
		{
			CHECK();
			Sleep (TimeToSleep);
			CHECK();
		}
		AbortIfTimeout();
		CHECK();
	}

	CHECK();
	return m_Response;
}


WT::System::IO::CStream* CHttpWebRequest::GetResponseStream() const 
{ 
	CHECK();
	return mtw_pStream; 
}

bool CHttpWebRequest::GetShouldSaveFileData () 
{
	if (mtw_DidSaveFileData)
	{
		return false;
	}
	// If we're not caching we shouldn't be saving.
	if (!AmDoingCache())
	{
		return false;
	}
	if (!mtw_DidCheckFileDataExists)
	{
		mtw_FileDataExists = FileExists (mtr_FileDataFilename);
		mtw_DidCheckFileDataExists = false;
	}
	if (mtw_FileDataExists)
	{
		return false;
	}
	return true;
}

DownloadStatus::Enum CHttpWebRequest::GetStatus() const 
{ 
	CHECK();
	return mmt_Status; 
}

HttpStatusCode::Enum CHttpWebRequest::GetStatusCode() const 
{ 
	CHECK();
	return mtw_StatusCode; 
}


bool CHttpWebRequest::GetStatusIsBad () const
{
	const bool Retval = mtw_StatusError;
	return Retval;
}

int32 CHttpWebRequest::GetTimeout() const 
{ 
	CHECK();
	return m_Timeout; 
}


const string& CHttpWebRequest::GetTransferEncoding() const 
{ 
	CHECK();
	return mtr_HeaderTransferEncoding; 
}


const string& CHttpWebRequest::GetUserAgent() const 
{ 
	CHECK();
	return mtr_HeaderUserAgent; 
}


void* CHttpWebRequest::GetUserData()
{
	CHECK();
	return m_UserData;
}


void CHttpWebRequest::Pause(const bool Wait) 
{ 
	CHECK();
	Logf ("HttpWebRequest::Pause. \n");
	if (mmt_Status == DownloadStatus::KDownloading)
	{
		mmt_Status = DownloadStatus::KPaused;
	}
	mtr_PauseRequested = true;
	if (Wait)
	{
		CHECK();
		while (m_hWorkerThread)
		{
			Sleep (100);
		}
	}
}

void CHttpWebRequest::ReadFileData()
{
	// Always initialize to false
	mtw_HaveDateStampSave = false;
	mtw_HaveOldContentLengthSave = false;

	const bool FinalFileExists = FileExists (mtr_FinalFilename);
	const bool CacheFileExists = FileExists (mtr_DLFilename);
	if (!FinalFileExists && !CacheFileExists)
	{
		return;
	}

	FILE* f = fopen (mtr_FileDataFilename.c_str(), "r");
	if (f)
	{
		const int KLineSize = 1024; // large enough for an URL
		char Buffer [KLineSize];
		int Version = 0;

		// Read in the date stamp
		while (fgets (Buffer, KLineSize, f))
		{
			Buffer [KLineSize-1] = '\0';

			// Trim the cr/lf at the end of the line
			int Len = (int)strlen (Buffer);
			for (int i=Len-1; i>=0; i--)
			{
				if (Buffer[i]=='\n' || Buffer[i]=='\r')
				{
					Buffer[i]='\0'; // trim the cr
				}
				else
				{
					break;
				}
			}

			// Handle LastModified
			if (strncmp (Buffer, KFileDataVersion, strlen (KFileDataVersion)) == 0)
			{
				Version=atoi (&Buffer [strlen (KFileDataVersion)]);
			}

			// Handle LastModified
			else if (strncmp (Buffer, KLastModified, strlen (KLastModified)) == 0)
			{
				mtw_DateStampSave = &Buffer [strlen (KLastModified)];
				mtw_HaveDateStampSave = true;
			}

			// Handle ContentLength
			else if (strncmp (Buffer, KContentLength, strlen (KContentLength)) == 0)
			{
				mtw_OldContentLengthSave = _atoi64 (&Buffer [strlen (KContentLength)]);
				mtw_HaveOldContentLengthSave = true;
			}

			// Handle Url
			else if (strncmp (Buffer, KUrl, strlen (KUrl)) == 0)
			{
				mtw_UrlSave = &Buffer [strlen (KUrl)];
			}
		}

		// Should I do anything with the version?
		fclose (f);
	}
}

// Replaced the original _Filename with the 
// _Cachefilename.  Closes the cache file first.
void CHttpWebRequest::ReplaceFile()
{
	if(AmDoingCache())
	{
		unlink (mtr_FinalFilename.c_str());
		rename (mtr_DLFilename.c_str(), mtr_FinalFilename.c_str());
	}
}

void CHttpWebRequest::Resume() 
{ 
	CHECK();
	Logf ("HttpWebRequest::Resume. This is lots of work...\n");
	mmt_Status = DownloadStatus::KDownloading;
	
	//
	// Wait for the download to stop
	//
	while (m_hWorkerThread)
	{
		Sleep (100);
	}
	CHECK();

	m_StartedDownload = false;
	mtr_PauseRequested = false; 
	mtw_HaveContentLength = false;
	mtw_HaveResponse=false; 
	mtw_IsDone=false; 
	mtw_NBytesDownloaded=0;
	mtw_StatusAbort=false;
	mtw_StatusError=false;
	mtw_StatusPause=false;
	mtr_AbortRequested=false;
	mtr_PauseRequested=false;

	CHECK();
	StartDownloadFromMainThread();
	CHECK();

	m_StartedDownload = true;
}

void CHttpWebRequest::SaveFileData (const char* const DateStamp, const int ContentLength, const char* const Url)
{
	WT::System::IO::CDirectory::CreateDirectoryToFile (mtr_FileDataFilename.c_str());
	FILE* f = fopen (mtr_FileDataFilename.c_str(), "wb");
	if (f)
	{
		fprintf (f, "%s%d\n", KFileDataVersion, 1);
		fprintf (f, "%s%s\n", KLastModified, DateStamp);
		fprintf (f, "%s%d\n", KContentLength, ContentLength);
		fprintf (f, "%s%s\n", KUrl, Url);
		fclose (f);
	}
}



void CHttpWebRequest::SetAccept(const string& Value) 
{ 
	CHECK();
	mtr_HeaderAccept = Value; 
}


void CHttpWebRequest::SetAllowAutoRedirect (const bool Value) 
{ 
	CHECK();
	mtr_AllowAutoRedirect = Value; 
}

void CHttpWebRequest::SetAsyncObj (void* pObj)
{
	mmt_pAsyncObj = pObj;
}


void CHttpWebRequest::SetConnection(const string& Value) 
{ 
	CHECK();
	mtr_HeaderConnection = Value; 
}


void CHttpWebRequest::SetContentLength(const int64 Value) 
{ 
	assert (false); 
	mtw_ContentLength = Value; 
}


void CHttpWebRequest::SetContentLengthSave (const int64 Value)
{
	CHECK();
	mtw_ContentLengthSave = Value;
}


void CHttpWebRequest::SetContentType(const string& Value) 
{ 
	CHECK();
	mtr_HeaderContentType = Value; 
}


int64 CHttpWebRequest::GetThreadTime() const
{
	CHECK();
	if (!mtw_pThreadTimer) 
	{
		return 0;
	}
	return mtw_pThreadTimer->GetTotalTime();
}


void CHttpWebRequest::KillWorkerThread()
{
	// TODO: technically should have a lock here
	CHECK();
	if (!m_hWorkerThread)
	{
		Logf ("HttpWebRequest::KillHelperThread: no thread, so do nothing\n");
		return;
	}

	CHECK();
	TerminateThread (m_hWorkerThread, 0);
	CHECK();
	m_hWorkerThread = 0;
	EndDownloadWork(); // Since the thread didn't do the cleanup, do it for the thread.
	CHECK();


}


void CHttpWebRequest::SetCookieContainer (CCookieContainer* Value)
{ 
	CHECK();
	if (mmt_pCookies)
	{
		delete mmt_pCookies;
		mmt_pCookies = 0;
	}

	mmt_pCookies=Value; 
	mmt_pCookies->WTInit (mtr_Uri, 0);
	CCookieCollection* Cookies = mmt_pCookies->GetCookies (mtr_Uri);
	m_Response.SetCookieCollection (Cookies); 
	CHECK();
} 


void CHttpWebRequest::SetDefaultMaximumResponseHeadersLength(const int32 Value) 
{ 
	CHECK();
	mtr_DefaultMaximumResponseHeadersLength = Value; 
}

void CHttpWebRequest::SetDoCache()
{
	if (mtr_FinalFilename == "")
	{
		mtr_FinalFilename = mtr_DLFilename;
		mtr_DLFilename = mtr_FinalFilename + "_cache";
		mtr_FileDataFilename = mtr_FinalFilename + "_filedata";
	}
}
	
void CHttpWebRequest::SetDownloadFilename (const string& Filename)
{
	mtr_DLFilename = Filename;
}

void CHttpWebRequest::SetExpect(const string& Value) 
{ 
	CHECK();
	mtr_HeaderExpect = Value; 
}


void CHttpWebRequest::SetIfModifiedSince(const CDateTime& Value) 
{ 
	CHECK();
	mtr_HeaderIfModifiedSince = Value.ToString("r"); 
	mtr_HeaderIfModifiedSinceSet = true;
}


void CHttpWebRequest::SetIfModifiedSince(const string& Value) 
{ 
	CHECK();
	mtr_HeaderIfModifiedSince = Value; 
	mtr_HeaderIfModifiedSinceSet = true;
}


void CHttpWebRequest::SetMaximumResponseHeadersLength(const int32 Value) 
{ 
	CHECK();
	mtr_MaximumResponseHeadersLength = Value; 
}


void CHttpWebRequest::SetMediaType(const string& Value) 
{ 
	CHECK();
	mtr_HeaderMediaType = Value; 
}


void CHttpWebRequest::SetMethod(const string& Value) 
{ 
	CHECK();
	mtr_Method = Value; 
}


void CHttpWebRequest::SetProtocolVersion(const string& Value) 
{ 
	CHECK();
	mtr_HeaderProtocolVersion = Value; 
}


void CHttpWebRequest::SetReferer(const string& Value) 
{ 
	CHECK();
	mtr_HeaderReferer = Value; 
}


void CHttpWebRequest::SetTimeout(const int32 Value) 
{ 
	CHECK();
	m_Timeout = Value; 
}


void CHttpWebRequest::SetTransferEncoding(const string& Value) 
{ 
	CHECK();
	mtr_HeaderTransferEncoding = Value; 
}


void CHttpWebRequest::SetUserAgent(const string& Value) 
{ 
	CHECK();
	mtr_HeaderUserAgent = Value; 
}


void CHttpWebRequest::SetUserData (void* UserData)
{
	CHECK();
	m_UserData = UserData;
}


void CHttpWebRequest::StartDownloadFromMainThread ()
{
	// It's already started.
	CHECK();
	if (m_hWorkerThread)
	{
		Logf ("HttpWebRequest::StartDownloadFromMainThread: already started so do nothing\n");
		return;
	}
	mmt_Status = DownloadStatus::KDownloading;
	DWORD hWorkerThread=0;
	m_hWorkerThread = CreateThread (
		NULL, // Thread security attributes
		KConstants::KThreadStackSize, 
        CHttpWebRequest_ThreadHelper::StartDownloadFromThreadHelper,
		(void*)this,
		0, // start right away
		&hWorkerThread
		);
	Logf ("HttpWebRequest::StartDownloadFromMainThread: created thread %d %X\n", H2int32 (m_hWorkerThread), H2int32 (m_hWorkerThread));
	CHECK();
	assert (m_hWorkerThread);
}

bool g_Verbose = true;
static int32 strleni (const int8* const Str)
{
	const int32 Retval = Str ? (int)strlen (Str) : 0;
	return Retval;
}
static void AddBuffer (const int8* const LogStr, HINTERNET hRequest, const int8* const HeaderName, const string& HeaderValue)
{
	CHECK();
	if (HeaderValue == "")
	{
		Logf ("%s AddHeaders: No value for %s (normal)\n", LogStr, HeaderName);
		return;
	}
	const int32 NameLen = strleni (HeaderName);
	const int32 ValueLen = strleni (HeaderValue.c_str());
	const int32 Len = NameLen + ValueLen + 2 + 1; //2=CRLF 1=NUL
	int8* Buffer = (char*)malloc (Len);
	if (!Buffer)
	{
		assert (false);
		Logf ("%s HttpAddRequestHeaders: unable to alloc a buffer %d bytes\n", Len);
		return;
	}
	CRAIIfreeMe fmb (Buffer); // RAII
	strcpy (Buffer, HeaderName);
	strcat (Buffer, HeaderValue.c_str());
	strcat (Buffer, "\r\n");
	const BOOL Status = HttpAddRequestHeaders (hRequest, Buffer, (DWORD)-1, HTTP_ADDREQ_FLAG_ADD);
	if (!Status)
	{
		const DWORD LastError = GetLastError();
		Logf ("%s HttpAddRequestHeaders (%d, %s, -1, HTTP_ADDREQ_FLAG_ADD) GetLastError()-->%d\n", LogStr, hRequest, Buffer, LastError);
	}
	CHECK();
}

void CHttpWebRequest::StartDownloadFromWorkerThread()
{
	CHECK();
	Logf ("HttpWebRequest::DL: started\n");

	//
	// DO THE ACTUAL WORK HERE
	//
	StartDownloadFromWorkerThread_h();
	//
	// DONE WITH THE ACTUAL WORK
	//

	CHECK();
	EndDownloadWork();
	CHECK();

}
void CHttpWebRequest::StartDownloadFromWorkerThread_h()
{
	CHECK();
	Logf ("StartDownloadFromWorkerThread_h: URI %s\n", mtr_Uri.ToString().c_str());
	bool DeletedThis = CallEventCallbacks (); // Event: started
	if (DeletedThis)
	{
		return;
	}

	//
	// Handle the cache file (if any)
	// Technically wrong because it's the wrong thread?
	//
	if (AmDoingCache())
	{
		ReadFileData ();
		bool AmGettingRestOfFile = false;
		const int64 CurrCacheSize = CacheFileSize ();
		if (mtw_UrlSave != mtr_Uri.ToString())
		{
			// Downloading from a different url -- delete the cache!
			// delete all our old data!  It's all wrong!
			DeleteFileDataFile();
			ReadFileData();
			DeleteCacheFile();
		}
		else if (CurrCacheSize < 100)
		{
			DeleteCacheFile();
		}
		else if (mtw_HaveOldContentLengthSave && CurrCacheSize >= mtw_OldContentLengthSave)
		{
			// Something horrible happened; we've got a cache file
			// that is the length of the file we wanted -- this is 
			// bad because it was supposed to automatically
			// switch over.
			DeleteFileDataFile();
			ReadFileData();
			DeleteCacheFile();
		}
		else if (CurrCacheSize > 0)
		{
			AddRange (-CurrCacheSize);
			if (mtw_HaveOldContentLengthSave && mtw_OldContentLengthSave)
			{
				SetContentLengthSave (mtw_OldContentLengthSave);
			}
			AmGettingRestOfFile = true;
		}
		const bool FinalFileExists = FileExists (mtr_FinalFilename);
		if (!AmGettingRestOfFile && FinalFileExists && mtw_HaveDateStampSave)
		{
			// Only add the if-modified-since if we're getting an entire file.
			// If we're getting a partial file, don't!
			// Also: only do this if the final file exists.
			SetIfModifiedSince (mtw_DateStampSave);
		}
	}


	const long KMaxBufferSize = GetMaximumResponseHeadersLength() * 1024;

	//
	// Deal with the Accept string.  Luckily InternetOpen can handle the
	// entire-string-in-one-shot madness that is the .NET "Accept" property.
	//
	int8* AcceptStrAlloc = strdup (mtr_HeaderAccept.c_str());
	CRAIIfreeMe fmAccept (AcceptStrAlloc);
	const int8* const AcceptStr = (strcmp (AcceptStrAlloc, "") == 0) ? "*/*" : AcceptStrAlloc;
	const int8* const Accept[] = { AcceptStr, 0, 0 };
	LPCSTR* lplpszAccept = (LPCSTR*)Accept;


#define CHECK_ABORT(_STR)\
	do if (mtr_AbortRequested)\
	{\
		Logf ("HttpWebRequestst::DL: abort requested at %s\n", _STR);\
		CHECK();\
		mtw_StatusAbort = true;\
		return;\
	} while (mtr_AbortRequested)

#define CHECK_PAUSE(_STR)\
	do if (mtr_PauseRequested)\
	{\
		Logf ("HttpWebRequestst::DL: pause requested at %s\n", _STR);\
		CHECK();\
		mtw_StatusPause = true;\
		return;\
	} while (mtr_PauseRequested)

#define ON_ERROR(_STR)\
	do\
	{\
		CHECK();\
		Logf ("HttpWebRequest::DL: error at %s\n", _STR);\
		mtw_StatusError = true;\
		return;\
	} while (!mtw_StatusError)

	//
	// Initialize
	//


	if (!mtw_pReadTimer)
	{
		mtw_pReadTimer = new CTimer (ETimerResolution::Medium);
	}
	if (!mtw_pThreadTimer)
	{
		mtw_pThreadTimer = new CTimer (ETimerResolution::Medium);
	}
	mtw_pThreadTimer->Start();

	mtw_hInternet = ::InternetOpen( mtr_HeaderUserAgent.c_str(),	// User-Agent string
									INTERNET_OPEN_TYPE_PRECONFIG,	// let system resolve proxy
									"",							// proxy
									"",							// proxy bypass
									0L );							// flags
	if (!mtw_hInternet)
	{
		ON_ERROR ("InternetOpen");
	}
	CRAIIInternetCloseHandleMeZero ICHI (&mtw_hInternet);
	Logf ("HttpWebRequest::DL: hInternet %d from InternetOpen\n", H2int32 (mtw_hInternet));
	DeletedThis = CallEventCallbacks (); // Event: opened internet
	if (DeletedThis)
	{
		return;
	}
	const string Site = mtr_Uri.GetHost();
	CHECK();


	CHECK_ABORT ("InternetOpen");

	const WORD Port = (WORD)mtr_Uri.GetPort();
	mtw_hSite = ::InternetConnect (mtw_hInternet, 
		Site.c_str(), // e.g. www.wildtangent.com
		Port,  // e.g. 80
		"",  // username
		0,  // password
		INTERNET_SERVICE_HTTP, 
		0, // flags
		NULL); // context (used for callbacks)
	if (!mtw_hSite)
	{
		ON_ERROR ("InternetConnect");
	}
	Logf ("HTtpWebRequest::DL: hSite %d from InternetConnect\n", H2int32 (mtw_hSite));
	Logf ("HttpWebRequest::DL: connect to %s:%d\n", Site.c_str(), Port);
	DeletedThis = CallEventCallbacks (); // Event: connected to site
	if (DeletedThis)
	{
		return;
	}
	CRAIIInternetCloseHandleMeZero ICHmeS (&mtw_hSite);
	CHECK_ABORT ("InternetConnect");
	CHECK();

	int8* const UrlPath = strdup (mtr_Uri.GetPathAndQuery().c_str());
	CRAIIfreeMe fmeUP (UrlPath); // RAII
	int8* const HV = strdup (mtr_HeaderProtocolVersion.c_str());
	CRAIIfreeMe fmeHV (HV); // RAII

	DWORD Flags = INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD;
	if (!mtr_AllowAutoRedirect)
	{
		Flags |= INTERNET_FLAG_NO_AUTO_REDIRECT;
	}
	if (mtr_Uri.GetScheme () == "https")
	{
		Flags |= INTERNET_FLAG_SECURE;
	}
	mtw_hRequest = HttpOpenRequest (mtw_hSite, 
		mtr_Method.c_str(), // either GET or POST
		UrlPath, // e.g. /index.html
		HV, //version e.g. "HTTP/1.1"
		mtr_HeaderReferer.c_str(), 
		lplpszAccept, 
		Flags, 
		0);
	if (!mtw_hRequest)
	{
		ON_ERROR ("HttpOpenRequest");
	}
	CRAIIInternetCloseHandleMeZero ICHmeR (&mtw_hRequest); // RAII
	CHECK_ABORT ("InternetOpenRequest");
	CHECK();

	AddBuffer ("HttpWebRequest::DL: ", mtw_hRequest, "Connection: ", mtr_HeaderConnection);
	AddBuffer ("HttpWebRequest::DL: ", mtw_hRequest, "Content-Type: ", mtr_HeaderContentType);
	AddBuffer ("HttpWebRequest::DL: ", mtw_hRequest, "Expect: ", mtr_HeaderExpect);
	AddBuffer ("HttpWebRequest::DL: ", mtw_hRequest, "Transfer-Encoding: ", mtr_HeaderTransferEncoding);
	if(mtr_HeaderIfModifiedSinceSet)
	{
		AddBuffer ("HttpWebRequest::DL: ", mtw_hRequest, "If-Modified-Since: ", mtr_HeaderIfModifiedSince);
	}
	if (mtr_HeaderRangeBegin >= 0 || mtr_HeaderRangeEnd >= 0)
	{
		const int32 KRangeBufferSize(1000);
		int8* RangeBuffer = (char*)malloc (KRangeBufferSize);
		CRAIIfreeMe fmRB (RangeBuffer); // RAII
		// TODO: Check all mallocs everywhere

		if (mtr_HeaderRangeBegin >= 0 && mtr_HeaderRangeEnd >= 0)
		{
			sprintf (RangeBuffer, "%s=%I64d-%I64d", mtr_HeaderRangeSpecifier.c_str(), mtr_HeaderRangeBegin, mtr_HeaderRangeEnd);
		}
		else if (mtr_HeaderRangeBegin >= 0)
		{
			sprintf (RangeBuffer, "%s=%I64d-", mtr_HeaderRangeSpecifier.c_str(), mtr_HeaderRangeBegin);
		}
		else if (mtr_HeaderRangeEnd >= 0)
		{
			sprintf (RangeBuffer, "%s=-%I64d", mtr_HeaderRangeSpecifier.c_str(), mtr_HeaderRangeEnd);
		}
		else
		{
			assert (false);
			RangeBuffer[0] = '\0';
		}
		AddBuffer ("HttpWebRequest::DL: ", mtw_hRequest, "Range: ", RangeBuffer);

	}

	Logf ("HttpWebRequest::DL: hRequest %d from HttpOpenRequest\n", H2int32 (mtw_hRequest));
	Logf ("HttpWebRequest::DL: urlpath %s\n", UrlPath);
	Logf ("HttpWebRequest::DL: KMaxBufferSize (MaximumResponseHeadersLengthx1024) %d\n", KMaxBufferSize);
	DeletedThis = CallEventCallbacks (); // Event: opened request
	if (DeletedThis)
	{
		return;
	}


	// Buffer for headers etc.
	int8* ClientHeaderBuffer = (char*)malloc (KMaxBufferSize);
	DWORD Len = 0; // will be set each time
	CRAIIfreeMe fmeBuffer (ClientHeaderBuffer);

	//
	// Log the headers going out and coming back
	//
	strcpy (ClientHeaderBuffer, "--blank--");
	Len = KMaxBufferSize;
	const DWORD StatusQIHeadersBefore = HttpQueryInfo( mtw_hRequest,							// handle returned from ::HttpOpenRequest
				HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,			// query all headers, including CRLF characters
				ClientHeaderBuffer,		// pointer to header buffer
				&Len,					// sizeof header buffer
				NULL 
				);	
	Logf ("HttpWebRequest::DL: BEFORE Header client status %d len %d\n%s\n", StatusQIHeadersBefore, Len, ClientHeaderBuffer);

	const int32 PostLen = mtr_RequestStream.GetLength();
	LPVOID PostBuffer = mtr_RequestStream.GetHandle();
	string Headers;
	ComposeHeaders(Headers);
	HttpSendRequest (mtw_hRequest, 
		Headers.size()? Headers.c_str() : NULL, //headers
		(DWORD)Headers.size(), //headers length
		PostBuffer, // options
		PostLen // optional length
		);
	CHECK_ABORT ("HttpSendRequest");
	CHECK();

	int8* ServerHeaderBuffer = (char*)malloc (KMaxBufferSize);
	CRAIIfreeMe fmeHbuffer (ServerHeaderBuffer);

	//
	// Log the headers going out and coming back
	//
	strcpy (ClientHeaderBuffer, "--blank--");
	Len = KMaxBufferSize;
	const DWORD StatusQIHeaders = HttpQueryInfo( mtw_hRequest,							// handle returned from ::HttpOpenRequest
				HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,			// query all headers, including CRLF characters
				ClientHeaderBuffer,		// pointer to header buffer
				&Len,					// sizeof header buffer
				NULL );	
	Logf ("HttpWebRequest::DL: AFTER Header client status %d len %d\n%s\n", StatusQIHeaders, Len, ClientHeaderBuffer);

	// Get the same thing in a NUL-delimited format
	strcpy (ClientHeaderBuffer, "--blank--");
	Len = KMaxBufferSize;
	const DWORD StatusQIRawHeaders = HttpQueryInfo( mtw_hRequest,							// handle returned from ::HttpOpenRequest
				HTTP_QUERY_RAW_HEADERS | HTTP_QUERY_FLAG_REQUEST_HEADERS,			// query all headers, including CRLF characters
				ClientHeaderBuffer,		// pointer to header buffer
				&Len,					// sizeof header buffer
				NULL );	
	Logf ("HttpWebRequest::DL: ditto, raw headers (no crlf) status status %d len %d\n", StatusQIRawHeaders, Len);

	strcpy (ServerHeaderBuffer, "--blank--");
	Len = KMaxBufferSize;
	const DWORD StatusQIServerHeaders = HttpQueryInfo( mtw_hRequest,							// handle returned from ::HttpOpenRequest
				HTTP_QUERY_RAW_HEADERS_CRLF,			// query all headers, including CRLF characters
				ServerHeaderBuffer,		// pointer to header buffer
				&Len,					// sizeof header buffer
				NULL );	
	Logf ("HttpWebRequest::DL: Header server status %d len %d\n%s\n", StatusQIServerHeaders, Len, ServerHeaderBuffer);
	
	// Get the non-CRLF buffers for my own parsing
	ServerHeaderBuffer[0] = '\0';
	Len = KMaxBufferSize;
	const DWORD StatusQIServerHeadersReg = HttpQueryInfo( mtw_hRequest,							// handle returned from ::HttpOpenRequest
				HTTP_QUERY_RAW_HEADERS,			// query all headers, NUL seperated
				ServerHeaderBuffer,						// pointer to header buffer
				&Len,					// sizeof header buffer
				NULL );	
	Logf ("HttpWebRequest::DL: Header(non crlf) status %d len %d\n", StatusQIServerHeadersReg, Len);
	CHECK();

	//Parse the headers and put them in response.
	if (StatusQIServerHeadersReg)
	{
		ParseResponseHeaders(ServerHeaderBuffer, Len);
	}

	//If this is a https connection, get the certificate information.
	if (mtr_Uri.GetScheme () == "https")
	{
		vector<char> buf;
		DWORD dwSize = 0;
		if (!InternetQueryOption(mtw_hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE, NULL, &dwSize) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			buf.resize(dwSize);
			if (InternetQueryOption(mtw_hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE, &buf[0], &dwSize))
			{
				mtw_HttpsCert = &buf[0];
				//printf("The whole cert string: ====\n%s\n", &buf[0]);
			}
		}

		INTERNET_CERTIFICATE_INFO ci;
		dwSize = sizeof(ci);
		if (InternetQueryOption(mtw_hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT, &ci, &dwSize))
		{
			//printf("Got cert============\n%s\nIssuer================\n%s\n", ci.lpszSubjectInfo, ci.lpszIssuerInfo);
			if (ci.lpszSubjectInfo)
			{
				mtw_HttpsSubject = ci.lpszSubjectInfo;
			}
			if (ci.lpszIssuerInfo)
			{
				mtw_HttpsIssuer = ci.lpszIssuerInfo;
			}
		}
	}

	//
	// Get the content length
	//
	const int64 CLen = QueryBufferInt64 (ServerHeaderBuffer, "Content-Length", -1);
	mtw_ContentLength = 0;

	mtw_HaveContentLength = (CLen>=0) ? true : false;
	if (CLen >= 0)
	{
		mtw_ContentLength = CLen;
		if (!mtw_ContentLengthSave)
		{
			mtw_ContentLengthSave = mtw_ContentLength;
		}
	}
	int64 BytesLeft = mtw_HaveContentLength ? mtw_ContentLength : 0;
	Logf ("CHttpWebRequest::DL: ContentLength %I64d \n", mtw_ContentLength);

	mtw_StatusCode = (HttpStatusCode::Enum)QueryBufferInt (ServerHeaderBuffer, "Status-Code", (int)HttpStatusCode::NotSet);
	Logf ("CHttpWebRequest::DL: Status Code %d \n", (int)mtw_StatusCode);

	if ((int)Len == KMaxBufferSize || Len==0)
	{
		mtw_HaveResponse = false;
		return;
	}
	else
	{
		mtw_HaveResponse = true;
		DeletedThis = CallEventCallbacks (); // Event: got status code
		if (DeletedThis)
		{
			return;
		}
	}

	int8* CookieString = QueryBufferNC (ClientHeaderBuffer, "Cookie");
	if (mmt_pCookies && CookieString)
	{
		mmt_pCookies->WTInit (mtr_Uri, CookieString);
	}

	CHECK_ABORT ("read return headers");
	CHECK();

	//
	// Read the data
	//
	int8* Buffer = (char*)malloc (KMaxBufferSize);
	CRAIIfreeMe fmeReadbuffer (Buffer);

	bool DLToFile = mtr_DLFilename != "";
	FILE* f = DLToFile ? fopen (mtr_DLFilename.c_str(), "abS") : 0; // why ab and not a+b?  Because a binary file might include a ^Z and we might be unfortunate enough to have stopped at one.
			// The 'S' means: optimize for Sequential disk access
	CRAIIfcloseMe AutoClose(f);

	Logf ("CHttpWebRequest::DL: about to start InternetReadFile loop on %d\n", H2int32 (mtw_hRequest));
	CMemoryStream* pStream = DynamicCast<CMemoryStream>(mtw_pStream);
	assert (DLToFile || (mtw_ContentLength < 0x7FFFFFFF));
	if (mtw_HaveContentLength && !DLToFile)
	{
		pStream->SetCapacity ((int32)mtw_ContentLength);
	}
	bool KeepGoing = true;

	//
	// There should be only one callback per second, max
	//
	time_t LastCallbackTime = 0;

	while (KeepGoing)
	{
		DWORD NRead = 0;
		const long MaxBytesToRead = mtw_HaveContentLength ? (int32)std::min ((int64)KMaxBufferSize-1, BytesLeft) : KMaxBufferSize-1;
			// long is OK -- each individual read is always less than KMaxBufferSize which is 
			// always a 32-bit value.

		CHECK_PAUSE ("InternetReadFile (Pause Check)");
		CHECK();

		mtw_pReadTimer->Stop(); // reset timer
		mtw_pReadTimer->Start();
		const BOOL Status = InternetReadFile (mtw_hRequest, Buffer, MaxBytesToRead, &NRead);
		mtw_pReadTimer->Pause();
		if (!Status)
		{
			const DWORD LastError = GetLastError();
			Logf ("CHttpWebRequest::DL: InternetReadFile status %d LastError %d %x nread %d\n", Status, LastError, LastError, NRead);
			int8 Buffer [1024];
			DWORD Error;
			DWORD Size = sizeof (Buffer);
			InternetGetLastResponseInfo (&Error, Buffer, &Size);
			Logf ("CHttpWebRequest::DL: full error %d = %s\n", Error, Buffer);
			ON_ERROR ("InternetReadFile");
		}
		if (NRead)
		{
			Buffer [NRead] = '\0'; // MaxBytesToRead is adjusted for this
			if (DLToFile)
			{
				if (f)
				{
					fwrite (Buffer, 1, NRead, f);
				}
				// else -- just drop it on the ground. 
				// TODO: handle error case: what if I can't write the file?
			}
			else
			{
				pStream->WriteCur ( WT::System::IO::KConstants::KWriteCur, (uint8*)Buffer, NRead);
			}
			mtw_NBytesDownloaded += NRead;
		}
		if (mtw_HaveContentLength) 
		{
			BytesLeft -= NRead;
		}
		if (NRead == 0)
		{
			Logf ("CHttpWebRequest::DL: InternetReadFile read zero; exit loop\n", Status, NRead);
			KeepGoing = false;
		}
		else
		{
			const time_t CurrTime = time(NULL);
			const bool ShouldDoCallback = (!mtw_HaveContentLength || BytesLeft > 0) && (LastCallbackTime != CurrTime);
			if (ShouldDoCallback)
			{
				Logf ("CHttpWebRequest::DL: InternetReadFile status %d nread %d\n", Status, NRead);
				LastCallbackTime = CurrTime;
				DeletedThis = CallEventCallbacks (); // Event: got data
				if (DeletedThis)
				{
					return;
				}
				Sleep (0); // Be a little nice to other threads
			}
			// Don't send an event if we're at the end; there will be an
			// event later on for that.

			CHECK_ABORT ("InternetReadFile (before sleep)");
			CHECK();
			if ((int)NRead < MaxBytesToRead)
			{
				// Means that we are getting ahead of the modem, etc.
				Sleep (100); // Sleep a bit to let the modem (etc) catch up.
			}
			CHECK_ABORT ("InternetReadFile (after sleep)");
			CHECK();
		}
	}
	CHECK();

#undef CHECK_ABORT
}

//Compose headers into a single string.
void CHttpWebRequest::ComposeHeaders(string& Ret)
{
	CHECK();
	Ret.clear();
	for(int32 i = 0; i < (int)m_Headers.size(); i++)
	{
		if (Ret.size())
			Ret += "\r\n";
		Ret += m_Headers[i];
	}
}

//Parse the response header string to WebResponse.
void CHttpWebRequest::ParseResponseHeaders(int8* Buffer, int32 BufferLength)
{
	//Header string are separated by \0.
	CHECK();
	m_Response.ClearHeaders();
	int32 Prev = 0;
	for(int32 i = 0; i < BufferLength; i++)
	{
		if (Buffer[i] == 0)
		{
			if (i > Prev)
			{
				m_Response.AddHeader(Buffer + Prev);
			}
			Prev = i+1;
		}
	}
}

void* CHttpWebRequestAsyncResult::GetAsyncState() const
{
	CHECK();
	void* Retval = m_Parent ? m_Parent->GetAsyncObj() : 0;
	return Retval;
}

bool CHttpWebRequestAsyncResult::GetIsCompleted() const
{
	CHECK();
	if (!m_Parent) return true; // bad object; not going to get more done!
	const bool IsCompleted = m_Parent->GetIsDone();
	return IsCompleted;
}


} // end namespace Net
} // end namespace System
} // end namespace WT