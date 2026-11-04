/*
	HttpWebRequest.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005

	Requires WinInet.lib to work!
 */
#pragma once

//#ifndef _WININET_
//#error Error: HttpWebRequest.h: must #include wininet.h before including this file (wininet requires windows.h or equivilent)
//#endif

#include "WT/System/Platform.h"
#include "WT/System/Callback.h"
#include "WT/System/DateTime.h"
#include "WT/System/IAsyncResult.h"
#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/MemoryStream.h"
#include "WT/System/IO/IOException.h"

// Include all of the System::Net files so that applications
// can have a single include file for the entire Net "space"
#include "WT/System/Uri.h"
#include "WT/System/Net/CookieContainer.h"
#include "WT/System/Net/NetInfo.h"
#include "WT/System/Net/WebRequest.h"
#include "WT/System/Net/WebResponse.h"
#include "WT/System/Net/HttpWebResponse.h"
#include "WT/System/Net/WebRequestEnum.h"
#include "WT/System/Net/Cookie.h"
#include "WT/System/Net/CookieCollection.h"
#include "WT/System/Net/CookieContainer.h"
// When turned on, the guard does two things: it adds 8k of data
// which has some access protection and it adds small strings
// at the start and end of the HttpWebRequest class.  These strings
// can be queried for correctness.
#define zzWT_SYSTEM_NET_HTTPREQUEST_GUARD
#include <wininet.h>

namespace WT
{
namespace System
{
// Forward declarations
namespace IO
{
class CStream;
}
class CTimer;

namespace Net
{
class CWebResponse;
class CHttpWebRequest;
class CHttpWebRequest_ThreadHelper;

namespace KConstants
{
	enum Enum { 
		/// How large the download thread's stack should be.  Note that the callbacks
		/// are in that threads context, so you may need to set this larger.
		KThreadStackSize=4000, 

		/// How long to wait before just killing the helper thread after an Abort has 
		/// been requested.  Essentially, the "Abort" method may take up to this amount
		/// of time to complete.
		KAbortWaitTimeout=2000,

		/// Watchdog timeout for each individual InternetReadFile.  We have this because
		/// a read should only take a few seconds but if you have a computer hooked to
		/// a hub, and the hub looses its upstream cable, it can take half an hour
		/// or more for InternetReadFile to time out.  Only active in Sync mode.
		/// TODO: should there be a way to set this?
		KAbortIndividualReadTimeout=10000,
	};
}

class CHttpWebRequestAsyncResult : public IAsyncResult
{
public:
	CHttpWebRequestAsyncResult(CHttpWebRequest* Parent):m_Parent(Parent)
	{
	}

	virtual ~CHttpWebRequestAsyncResult()
	{
	}
	virtual void* GetAsyncState() const;

	virtual bool GetIsCompleted() const;
private:
	CHttpWebRequest* m_Parent; 
};
/*
The HttpWebRequest class is the foundation on which the rest of the WTPlatformat
downloading depends.  The basic usage scenario is:

1. Create a new HttpWebRequest using the WebRequest::Create static method.
	It creates a WebRequest which you must dynamically cast into an
	HttpWebRequest (in the future the WebRequest::Create method will also
	be able to handle File requests and any special requests that are added later.
	<b>Do not construct an HttpWebRequest directly</b>.
	
2. Modify the request as needed.  You can modify the request up to the time
	that it's actually sent off to the http server.  This happens at either
	the 'GetResponse()' method or the 'BeginGetResponse()' method.

3. Send the request up to your http server.  You can use the GetResponse() method
	which will wait until the server responds or use BeginGetResponse()/EndGetResponse()
	which work asynchronously.  For most WildTangent projects the Async version is
	the version to use since it lets you abort the download and get updates.

	You can also use 'BeginDownloadToFile (filename)' which will download to
	a specific file (in which case there will be no stream available)

4. Clean up afterwords by deleting the object.  If you were doing async work (which you
	probably should be doing) you must also call EndGetResponse().
*/
typedef SmartPointer<class CHttpWebRequest> HHttpWebRequest;
class CHttpWebRequest : public CWebRequest
{
public:
	WT_CLASS( CHttpWebRequest, CWebRequest );
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	char m_StartString[20];
#endif

private:
	/// Default constructor for the HttpWebRequest class; not useful.
	CHttpWebRequest();

	/// Useful constructor for the HttpWebRequest class; takes in a Uri which is 
	//// copied in.  However, you should call the static WebRequest::Create() method
	// to actually make an HttpWebRequest instead of calling the constructor.
	CHttpWebRequest(const CUri& Uri);

public:
	/// Standard destructor for HttpWebRequest.
	virtual ~CHttpWebRequest();

	/// The Abort() method will abort an asynchronous download.  In theory you
	/// can Abort() a synchronous download, too if you do it on another thread.
	/// But that would be silly.
	///
	/// Clarification: this can be a slow routine -- it sets the "please abort"
	/// flag and then, if the helper thread doesn't time out in a reasonable period,
	/// will force the thread to terminte (but will try to clean up).  This can
	/// take seconds and (possibly) mintutes of time to accomplish.
	void Abort (); 

	/// Adds an event callback.  This can be called at any time (except during a
	/// callback -- that might not work at all)
	void AddCallback (CAsyncCallback*);

	/// If Range is positive, adds a byte range 0-Range. If Range is negative,
	/// adds a byte range Range- (i.e., to the end);.
	/// See AddRange(const int32 RangeBegin, const int32 RangeEnd) for full details.
	void AddRange(const int64 Range);

	/// Modifies the request to only download a limited amount of data from the server.
	/// This only makes sense before the request is sent to the server (before GetResponse()
	/// or BeginGetResponse() has been called).
	/// There are four variants on AddRange; you should generally use the 
	/// AddRange(const int64 RangeBegin, const int64 RangeEnd) since it has the most obvious
	/// functionality.
	void AddRange(const int64 RangeBegin, const int64 RangeEnd);


	/// Just like AddRange (const int32 Range) but instead of
	/// adding in a "byte" range can add in any other type of range.  The types of
	/// ranges that your server are known only to your server administrator.
	/// See AddRange(const int64 RangeBegin, const int64 RangeEnd) for full details.
	void AddRange(const string& RangeSpecifier, const int64 Range);


	/// Just like AddRange (const int32 RangeBegin, const int32 RangeEnd) but instead of
	/// adding in a "byte" range can add in any other type of range.  The types of
	/// ranges that your server are known only to your server administrator.
	/// See AddRange(const int64 RangeBegin, const int64 RangeEnd) for full details.
	void AddRange(const string& RangeSpecifier, const int64 RangeBegin, const int64 RangeEnd);

	/// Similar to BeginGetResponse() but requires that AddCallback() and SetAsyncObj()
	/// be called first; similar to BeginDownloadToFile() but requires that SetDownloadFile()
	/// be called first.
	virtual void Begin ();

	/// Starts a download to a named file.  The 'stream' result will never be filled in.
	/// Overwise the same as BeginGetResponse
	virtual const IAsyncResult* BeginDownloadToFile (CAsyncCallback*, void* pObj, const string& Filename);

	/// Call BeginGetResponse() to start your async download.  You must pass in an 
	/// AsyncCallback (see the sample for how) and an Obj which can be anything.
	/// As the download progresses, you will get callbacks with the progress of the
	/// download. 
	/// Note that this method starts a thread; that thread has a limited stack space.
	/// Your callback will always be called in that context (I think) and therefore
	/// must be careful with its stack.
	/// When you are done you must call EndGetResponse(); however if the download is done
	/// you must call Abort() FIRST in order to stop the download thread.  Abort() can
	/// be a slow method.
	/// There is also a synchronous version of this called GetResponse().
	///
	/// DO NOT DELETE the object you get back!
	virtual const IAsyncResult* BeginGetResponse (CAsyncCallback*, void* pObj);


	/// If you used the BeginGetResponse()  you must call the EndGetResponse().
	virtual CWebResponse& EndGetResponse(const IAsyncResult*);

	/// Specialization of EndGetResponse that return a CHttpWebResponse directly
	CHttpWebResponse& EndGetHttpResponse(const IAsyncResult*);

	/// Gets the AbortOnDelete flag; see SetAbortOnDelete for details.
	const bool GetAbortOnDelet() const;

	/// Returns any changes you have made via SetAccept() or the 
	/// default value ("").  See SetAccept for details.
	const string& GetAccept() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value (true).  See the Set* method for details.
	bool GetAllowAutoRedirect() const;

	/// Returns the number of content bytes actually downloaded
	/// (does not include header bytes)
	int64 GetBytesDownloaded() const;

	/// Gets the number of bytes downloaded PLUS the number of
	/// bytes that were already in the cache
	int64 GetBytesDownloadedTotal() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("").  See the Set* method for details.
	/// Difference: default is "" instead of a null reference
	const string& GetConnection() const;

	/// Gets the content length (as it exists on the server)
	/// OR -1 if the content length is not known.
	/// This is a WildTangent extention to the .NET framework
	int64 GetContentLength() const;


	/// Gets the saved content length; see SetContentLengthSave for
	/// more information.
	int64 GetContentLengthSave() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("").  See the Set* method for details.
	const string& GetContentType() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value (null).  See the Set* method for details.
	WT::System::Net::CCookieContainer* GetCookieContainer();

	/// Returns the HTTTPS Certificate information when request type is HTTPS. 
	/// This information is only available after a successful request.
	/// It may look like this:
	/*
		Subject:
		US
		Washington
		Redmond
		"WildTangent, Inc."
		Internet Operations
		www.wildgames.com
		Issuer:
		ZA
		Western Cape
		Cape Town
		Thawte Consulting cc
		Certification Services Division
		Thawte Premium Server CA
		premium-server@thawte.com
		Effective Date: 2006-3-3 19:26:41
		Expiration Date:        2007-3-3 19:26:41
		Security Protocol:      (null)
		Signature Type: (null)
		Encryption Type:        (null)
		Privacy Strength:       High (128 bits)
	*/
	const string& GetHttpsCertificate() const;
	//Return the subject only.
	const string& GetHttpsSubject() const;
	//Return the issuer only.
	const string& GetHttpsIssuer() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value (64 meaning 64k).  See the Set* method for details.
	///
	/// Difference: WTPlatform simply uses 64k as the starting default; .NET allows
	/// for a configuration file that changes the value.  This value is in kilobytes
	/// (that is, the default 'actual' value is 64k so this routine returns 64.
	/// Values of 0 and -1 are not allowed.
	int32 GetDefaultMaximumResponseHeadersLength() const;

	/// Returns true when a download is done.  Technically, a download is
	// 'Ok' if GetPercentDone()>=100.0f AND GetStatusIsBad() is false 
	/// AND GetStatusCode() returns a value which is 200..299 (these are
	/// the values that are defined to be a good return code from http)
	///
	///
	bool DownloadIsOk() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("").  See the Set* method for details.
	const string& GetExpect() const;

	/// Returns true if a response has been received.  This is useless in Sync mode
	/// (GetResponse()).  Is set after the server first responds with, e.g., a
	/// content length but before the actual data is read.
	virtual bool GetHaveResponse() const;

	/// Returns a vector of strings.  You can add (or remove) items from this 
	/// list; when the request is sent to the server (from GetResponse() or
	/// BeginGetResponse()) the extra strings you set will be sent up to
	/// the server as headers.
	///
	/// Warning: no checking is done on the strings.  You must get
	/// them right or odd (and hard to debug) errors will occur.
	vector<string>& GetHeaders();

	/// Returns any changes you have made via the Set* method or the
	/// default value (DateTime::Now()).  See the Set* method for details.
	const string& GetIfModifiedSince() const;

	/// Combines GetPercentDone()==100 with the internal detail that
	/// the download thread has exited AND that there will be no
	/// more callbacks at all
	const bool GetIsDeletable() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value (64 meaning 64k).  See the Set* method for details.
	///
	/// Clarification: unless SetMaximumResponseHeadersLength is explicitly
	/// called, this routine will call the GetDefaultMaximumResponseHeadersLength
	/// method for the return value.  The value is in K (that is, 64==64k)
	int32 GetMaximumResponseHeadersLength() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("").  See the Set* method for details.
	///
	/// Change: default is "" instead of a null reference.
	const string& GetMediaType() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("GET").  See the Set* method for details.
	const string& GetMethod() const;

	// Returns the original CHttpWebResponse() that you will get from
	// EndGetHttpResponse() when the download is complete.  This is
	// included because just calling EndGetHttpResponse() isn't
	// the correct thing to do to get a response.  It returns the
	// CHttpWebResponse() because it make life easier.
	CHttpWebResponse& GetOriginalHttpResponse ();

	// Returns the original IAsyncResult that BeginGetResponse() returned.
	// This is a "practical" routine: it's here because .NET has a
	// (fairly useless) IAsyncResult object that we have to duplicate
	// and which must be passed in to various methods but which I don't
	// really want people to have to keep around.
	const IAsyncResult* GetOriginalAsyncResult ();

	/// Gets the percent done the download is at.  There are some
	/// special values:
	/// 0% = download has not started
	/// 1%..99% = download is in progress
	/// 100% = download is done (or aborted, or failed -- in any event,
	////    it's at a state such that it's not going to get more done.
	/// This is a WildTangent extention to the .NET framework
	float32 GetPercentDone() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("HTTP/1.1").  See the Set* method for details.
	///
	/// Change: returns a string ("HTTP/1.1") instead of a Version
	/// Clarification: no matter what the setting is, WinInet will use HTTP/1.1
	/// unless the user has UNchecked the 
	/// Control Panel(classic view)-->Internet Settings-->Advanced-->HTTP 1.1-->Use HTTP 1.1
	/// setting!  
	const string& GetProtocolVersion() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("").  See the Set* method for details.
	const string& GetReferer() const;

	/// Returns a stream that you can write to for POST requests.  Only the data
	/// that is written before the request is sent to the server (e.g., before
	/// GetResponse() or BeginGetResponse() is called) will be sent.  Technically,
	/// the data is always written to the server whether you are using a GET or 
	/// a POST; however it only makes sense to send data with a POST.
	WT::System::IO::CStream& GetRequestStream();

	/// Returns the Uri used to create the request -- this will be a copy of
	/// the Uri passed into to WebRequest::Create(Uri) was called or a copy of the Uri that
	/// was created when WebRequest::Create(string) was called.
	virtual const CUri& GetRequestUri() const;

	/*
	A cornerstone of the entire Http system!
	This method actually sends the request up to the server and waits for the answer
	in a synchronous fashion (see BeginGetResponse() for the async version).
	This will time out (eventually) on failure.  The return result is WebResponse,
	but more than that will always be an HttpWebResponse.  The following code shows
	how to convert one into the other:
	
	Example:
	<code>
		HttpWebResponse& WResponse = *WT::DynamicCast<HttpWebResponse>(&m_WRequest->EndGetResponse(AR));
	</code>

	Note that you can use the specialization GetHttpResponse() too.
	*/
	virtual CWebResponse& GetResponse();

	/// Specialization of the GetResponse() method; returns a reference
	/// to a CHttpWebResponse directly 
	virtual CHttpWebResponse& GetHttpResponse();

	/// Get the response stream of the download -- that is, the
	/// actual set of bytes in the download.
	/// This is a WildTangent extention to the .NET framework
	WT::System::IO::CStream* GetResponseStream() const;

	/// Gets the download status for a download.
	DownloadStatus::Enum GetStatus() const;

	/// Get the HTTP status code from the download.  The status is normally
	/// something like 200 (OK) or 404 (File not found) and matches the standard
	/// set of HTTP codes except that there's a -1 value for "not set".
	/// Future direction: all error codes that are not HTTP error codes will
	/// return a negative number.
	/// This is a WildTangent extention to the .NET framework.  
	HttpStatusCode::Enum GetStatusCode() const;

	/// Returns true if the value from GetStatusCode can't be trusted.
	/// For example, if a download starts to work, but then fails because
	/// of an ethernet issue, then GetStatusCode returns the normal value
	/// (like 200) AND it's reported as 100% done -- but that doesn't mean
	/// that the download worked.
	bool GetStatusIsBad() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value (-1).  See the Set* method for details.
	int32 GetTimeout() const;

	/// Returns any changes you have made via the Set* method or the
	/// default value ("").  See the Set* method for details.
	const string& GetTransferEncoding() const;


	/// Returns any changes you have made via the Set* method or the
	/// default value ("Mozilla/4.0 (compatible; MSIE 5.0; CDA 6.0)").  See the Set* method for details.
	/// Change: default is "Mozilla/4.0 (compatible; MSIE 5.0; CDA 6.0)" 
	/// instead of a null reference
	const string& GetUserAgent() const;

	/// Returns whatever user data had been set earlier.  The initial
	/// value is zero.
	void* GetUserData();

	/// Pauses the download.  When Wait is true, will wait for the
	/// pause to take effect.
	/// This is a WildTangent extension to .NET
	void Pause(const bool Wait);

	/// Resumes the download.  Pause and Resume are not counted -- that is,
	/// the method sequence Pause() Pause() Pause() Resume() will 
	/// end up with the download going.
	/// You will almost certainly want to call AddRange() before calling this!
	/// This is a WildTangent extension to .NET
	void Resume();

	/// When set, the DownloadRequest will Abort() downloads on delete.
	/// (Default is set)
	void SetAbortOnDelete(const bool Value);

	/// Sets a value for the 'Accept' header.
	/// Note that although the default value is "", the http code will automatically
	/// substitute */* for a blank string)
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetAccept(const string& Value);

	/// Sets a value for auto redirect.  When true (default), servers that respond
	/// with a redirect will automatically be listened to and a new request will
	/// be automatically issued.  When false, servers that respond with a redirect
	/// will be declared "done".
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetAllowAutoRedirect (const bool Value);

	/// Sets the async object -- the same one that's normally passed in
	/// to 'BeginGetResponse' or 'BeginDownloadToFile'
	void SetAsyncObj (void* pObj);

	/// Sets the "Connection: " header; default is "" (which means that the header is not sent)
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetConnection(const string& Value);

	/// A very specialized value for knowledable clients.  By setting this
	/// value, downloads that are restarted using a different download value
	/// will use this given value as the final download size.  This is added
	/// so that download percentages will be glitch-free as much as possible.
	void SetContentLengthSave (const int64 Value);

	/// Sets the Content-Type header; default is "" (which means that the header is not sent)
	void SetContentType(const string& Value);

	/// Sets a cookie container for this request.  Once the cookie container is set, you
	/// can query it to get the cookies that (a) will be sent by the request and (b) cookies
	/// that the server wants to be set.  Which one you get depends on your timing: checks
	/// done before GetResponse() or BeginGetRespose() will get the cookies to be sent;
	/// after the request is done (GetIsDone() returns true) you will get the cookies 
	/// that were returned.  Between these two times the values are in flux.
	void SetCookieContainer (CCookieContainer* Value);

	/// Sets the default for the maximum headers length; default is 64 which means 64k (64*1024).
	/// This value is used when  SetMaximumResponseHeadersLength() isn't called; see that
	/// method for full information.
	/// In general, you will never have to call this routine.
	void SetDefaultMaximumResponseHeadersLength(const int32 Value);

	/// If you call this, then the download will be automatically cached.
	void SetDoCache();

	/// Sets the filename to download to.  If blank, the filename is cleared
	/// and the download will be to a memory stream.  This cannot be set once
	/// 'Begin()' (or similar methods) have been called
	void SetDownloadFilename (const string& Filename);

	/// Sets the "Expect: " header; default is "" (which means that the header is not sent)
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetExpect(const string& Value);

	/// Sets the "If-Modified-Since: " header; default is Date::Now() but the header
	/// will only be sent if the value is changed (otherwise it wouldn't make any sense)
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetIfModifiedSince(const CDateTime& Value);

	/// Sets the "If-Modified-Since: " header; default is Date::Now() but the header
	/// will only be sent if the value is changed (otherwise it wouldn't make any sense)
	/// The string must be in the proper RFC 822 / RFC 1123 format -- but that's not 
	/// actually checked.
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetIfModifiedSince (const string& Value);

	/// Sets the maximum header length; default is -1 which means use the 
	/// "DefaultMaximumResponseHeadersLength" value which is 64 which in turn
	/// means 64k (64*1024).  This is used to size an internal buffer which is
	/// used for all internet reading and writing; if you size this buffer too
	/// small two bad things can happen:
	/// 1. If it's very small indeed, you might not be able to send all your headers
	///    and your server will behave very oddly
	/// 2. It will make reading from the internet less efficient.
	/// You will never normally have to set this value.
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetMaximumResponseHeadersLength(const int32 Value);

	/// Does bugger all.
	/// Change: the .NET documentation is astoundingly vague as to the exact nature of the
	/// 'MediaType' property.  Consequently WTPlatform lets you set and get this value
	/// but otherwise it does nothing.
	void SetMediaType(const string& Value);

	/// Sets the connection method; should be either "GET" (default) or "POST".
	/// <b>WARNING</b>: very very bad things will happen if this is anything but GET or POST.
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetMethod(const string& Value);

	/// Sets the version number part of the Http header; default is "HTTP/1.1" 
	/// <b>WARNING</b>: this will rarely do what you want!  Wininit, even when given
	/// an explicit value, will STILL use "HTTP/1.1" so long as an obscure checkbox
	/// is set in IE.
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetProtocolVersion(const string& Value);

	/// Sets the Referrer header; default is "".  Some servers demand certain values
	/// in order to work properly; others don't care.  
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetReferer(const string& Value);

	/// Generally useless timeout value which is used for the synchronous version
	/// of downloading (i.e., via GetResponse()).  It's useless for two reasons:
	/// firstly, it's hard to really know what a good value is, and secondly because
	/// the more common error scenario only cares about the time each individual 
	/// read takes.
	void SetTimeout(const int32 Value);

	/// Sets the "Transfer-Encoding: " header; default is "" (which means that the header is not sent)
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetTransferEncoding(const string& Value);

	/// Sets the "Connection: " header; default is "Mozilla/4.0 (compatible; MSIE 5.0; CDA 6.0)"
	/// A decent value must be set.  
	/// This value must be set before GetResponse() or BeginGetResponse() is called;
	/// otherwise the value may be set but it will have no effect.
	void SetUserAgent(const string& Value);

	/// Sets arbitrary "user data" for this connection.  Nothing in particular is
	/// done with this data.
	void SetUserData (void* ptr);

	friend CWebRequest;
	friend CHttpWebResponse;
	friend CHttpWebRequest_ThreadHelper;
	friend CHttpWebRequestAsyncResult;

private:
	// Will abort a download if the timeout values have been reached.  If the timeout
	// values have not been reached, do nothing.  This is generally a good routine
	// to call when you are downloading a file.
	// Checks both the regular timeout values AND the special KAbortIndividualReadTimeout value.
	bool AbortIfTimeout();

	bool AmDoingCache() const
	{
		const bool Retval = mtr_DLFilename != "" && mtr_FinalFilename != "";
		return Retval;
	}

	// Returns the current actual size of the cache file
	int64 CacheFileSize ();

	// Calls the callbacks from mtr_pAsyncEvent.
	// Callbacks are trickier than they look!  When this returns TRUE, the object
	// was DELETED during the callback.
	bool CallEventCallbacks();

	// Delete the cache file
	void DeleteCacheFile();

	// Deletes the file with meta-data about the download
	void DeleteFileDataFile();

	// Handles the finalizing work for a thread -- this is useful in the
	// case of a terminated thread because it can carefully clean up
	// all of the "junk" of a download.
	void EndDownloadWork();

	void* GetAsyncObj();
	bool GetIsDone() const;
	WT::int64 GetReadTime() const;
	WT::int64 GetThreadTime() const;

	bool GetShouldSaveFileData ();

	// Kills the helper thread outright; does not play nice at all.  Should only
	// be used if the thread is clearly not going to stop on its own.
	void KillWorkerThread();

	// Reads in data from the _filedata file
	void ReadFileData();

	// Replaced the original _Filename with the 
	// _Cachefilename.  Closes the cache file first.
	void ReplaceFile();

	// Saves download data into the _filedata file
	void SaveFileData (const char* const DateStamp, const int ContentLength, const char* const Url);



	// SetContentLength is here as it is part of the .NET classes.  However, it's
	// not a normally useful function in any sense since it's overriding the actual
	// work of the class (and it's setting a 'thread writable' variable outside of
	// the worker thread, which is a no-no)
	void SetContentLength(const int64 Value);
	void StartDownloadFromMainThread();
	void StartDownloadFromWorkerThread();
	void StartDownloadFromWorkerThread_h();

	//Compose headers into a single string.
	void ComposeHeaders(string& Ret);
	//Parse the response header string to WebResponse.
	void ParseResponseHeaders(int8* Buffer, int32 BufferLength);

	// m_ variables: thread neither read nor writes
	bool m_StartedDownload;
	bool m_AbortOnDelete;
	CHttpWebResponse m_Response;
	HANDLE m_hWorkerThread;
	int32 m_Timeout; // timeout for sync mode only
	int32 m_ReadTimeout; // timeout for the internal read call
	void* m_UserData;

	//
	// Guard Page
	//
#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	char m_GuardPage [8*1024];
#endif

	// m_tr variables: thread reads only, others write
	bool mtr_AbortRequested;
	bool mtr_AllowAutoRedirect;
	CAsyncEvent* mtr_pAsyncEvent;
	CHttpWebRequestAsyncResult mtr_AsyncResult;
	int32 mtr_DefaultMaximumResponseHeadersLength;
	string mtr_DLFilename; // when not "" will download to a file
	string mtr_HeaderAccept;
	string mtr_HeaderConnection;
	string mtr_HeaderContentType;
	string mtr_HeaderExpect;
	string mtr_HeaderIfModifiedSince;
	bool mtr_HeaderIfModifiedSinceSet;
	string mtr_HeaderMediaType;
	string mtr_HeaderProtocolVersion;
	string mtr_HeaderRangeSpecifier;
	int64 mtr_HeaderRangeBegin;
	int64 mtr_HeaderRangeEnd;
	string mtr_HeaderReferer;
	string mtr_HeaderTransferEncoding;
	string mtr_HeaderUserAgent;
	int32 mtr_MaximumResponseHeadersLength;
	string mtr_Method; // "GET" or "POST"
	bool mtr_PauseRequested;


	string mtr_FinalFilename;
	string mtr_FileDataFilename;

	bool mtw_DidSaveFileData;
	bool mtw_DidCheckFileDataExists;
	bool mtw_FileDataExists;

	// These are all 'filedata' information bits.
	// quite arguably they should be in their own
	// class instead of thrown in here.
	bool mtw_HaveDateStampSave;
	bool mtw_HaveOldContentLengthSave;
	string mtw_DateStampSave;
	int64 mtw_OldContentLengthSave;
	string mtw_UrlSave;

	WT::System::IO::CMemoryStream mtr_RequestStream; // data for POST commands
	CUri mtr_Uri;



	// mtw_ variables: thread writes, others read
	// (note: except that on initialization they are written)
	int64 mtw_ContentLength;
	int64 mtw_ContentLengthSave;
	bool mtw_HaveContentLength;
	int64 mtw_NBytesDownloaded;
	WT::System::IO::CStream* mtw_pStream;
	bool mtw_IsDone;
	bool mtw_HaveResponse;

	//Certificate information returned from server if request is HTTPS.
	string mtw_HttpsCert;
	string mtw_HttpsSubject;
	string mtw_HttpsIssuer;

	HINTERNET mtw_hInternet; // from InternetOpen
	HINTERNET mtw_hSite; // from InternetConnect
	HINTERNET mtw_hRequest; // from HttpOpenRequest
	CTimer* mtw_pReadTimer;
	bool mtw_StatusAbort;
	HttpStatusCode::Enum mtw_StatusCode;
	bool mtw_StatusError;
	bool mtw_StatusPause;
	CTimer* mtw_pThreadTimer;

	// 
	// mmt_ variables: variables that have no particular
	// threading model ('multiple thread' accesses); these can be 
	// both written to and read from both the main thread and the 
	// download thread. Yes, this is bad.
	WT::System::Net::CCookieContainer* mmt_pCookies;
	void* mmt_pAsyncObj;
	bool *mmt_pNotifyOnDelete;
	DownloadStatus::Enum mmt_Status;


	vector<string>	m_Headers;

#ifdef WT_SYSTEM_NET_HTTPREQUEST_GUARD
	char m_EndString[20];
#endif
};

} // end namespace Net
} // end namespace System
} // end namespace WT