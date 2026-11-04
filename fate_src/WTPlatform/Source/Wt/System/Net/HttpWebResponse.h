/*
	HttpWebResponse.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
// Note: requires WinInet.lib

#pragma once

#include "WT/System/Object.h"
#include "WT/System/Net/WebRequestEnum.h"
#include "WT/System/Net/CookieCollection.h"
namespace WT
{
namespace System
{
// Forward declarations
namespace IO
{
class CStream;
}

namespace Net
{

class CHttpWebRequest;

/// The HttpWebResponse contains the results of an http download.
/// You get one from the HttpWebRequest::GetResponse() method
/// (which means you can't get one until the download is done).
typedef SmartPointer<class CHttpWebResponse> HHttpWebResponse;
class CHttpWebResponse : public CWebResponse
{
public:
	WT_CLASS( CHttpWebResponse, CWebResponse );
private:
	CHttpWebResponse():m_pRequest(0),m_Cookies(0) { }
	CHttpWebResponse(CHttpWebRequest* Request):m_pRequest(Request),m_Cookies(0) { }
public:
	virtual ~CHttpWebResponse() { if (m_Cookies) delete m_Cookies; m_Cookies=0; }

	/// Gets the same cookie collection that was passed into SetCookieCollection. 
	/// If SetCookieCollection was not called, returns null.
	WT::System::Net::CCookieCollection* GetCookieCollection() { return m_Cookies; }

	/// Gets the length that the downloaded file should be according to the server.
	/// If the server does not report file lengths, it will be set after the download
	/// is complete and successful based on the number of bytes actually downloaded.
	int64 GetContentLength() const;

	/// Gets a list of headers that the server returned back to the client.
	/// Headers are often uninteresting, but can be useful for debugging.
	const vector<string>& GetHeaders() const { return m_Headers; }

	// Returns part of one header; returns DefaultValue if the requested
	// header isn't in the list.  
	// Example:
	// GetHeaderValue (string("Last-Modified")) --> "Fri, 06 Jan 20-06 21:36:52 GMT"
	// This is a WildTangent addition to the basic spec
	const string GetHeaderValue(const string& Name, const string& DefaultValue) const;

	/// Returns true when the download is as done as it will get -- either
	/// because it was aborted, the download failed, or because it worked.
	bool GetIsDone() const;

	/// Gets the results of the download in a stream format.  The downloader does
	/// not put data into files; you have to do that yourself.
	WT::System::IO::CStream* GetResponseStream();

	/// Gets the status code for the download.  Status codes are simply the status codes
	/// returned by the server (e.g., 404 for "page not found") except that if the download
	/// failed without reaching a server the code "NotSet" (-1) is returned.
	/// Probable future: other status codes that are not http status codes will also
	/// be negative values.
	HttpStatusCode::Enum GetStatusCode() const;

	/// Sets cookies for a download.  Ownership of the memory is transferred by 
	/// calling this routine and the cookie collection will be automatically deleted
	/// by this object  when it is deleted.
	void SetCookieCollection (WT::System::Net::CCookieCollection* Value) { m_Cookies = Value; }

	friend CHttpWebRequest;
private:
	WT::System::Net::CHttpWebRequest* m_pRequest;
	WT::System::Net::CCookieCollection* m_Cookies;
	vector<string>	m_Headers;

	//Add returned headers. Called by HttpWebRequest
	void AddHeader(const string& Value) { m_Headers.push_back(Value); }
	void ClearHeaders() { m_Headers.clear(); }
};

} // end namespace Net
} // end namespace System
} // end namespace WT