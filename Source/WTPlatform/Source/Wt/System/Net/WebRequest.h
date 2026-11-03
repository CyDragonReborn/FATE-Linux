/*
	WebRequest.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
// Note: requires WinInet.lib

#pragma once

#include "WT/System/Object.h"
#include "WT/System/Uri.h"
#include "WT/System/Callback.h"
#include "WT/System/IAsyncResult.h"

namespace WT
{
namespace System
{
namespace Net
{

typedef CCallback1<const IAsyncResult*> CAsyncCallback;
typedef CAsyncCallback::Event CAsyncEvent;

class CHttpWebRequest;
/*
A very simply class that, given a URL, will return the correct
type of CWebRequest.  As of September 2005, this only supports
http and https connections via the CHttpWebRequest class.

Example:
<code>
	CHttpWebRequest* pClass = WT::DynamicCast<CHttpWebRequest>(CWebRequest::Create (Uri (Url)));
</code>
*/
typedef SmartPointer<class CWebRequest> HWebRequest;
class CWebRequest : public CObject
{
private:
	CWebRequest();
public:
	WT_CLASS_NO_CREATOR(CWebRequest, CObject);
	virtual ~CWebRequest(); 


	/*
	Creates a CWebRequest out of a string.  Example: "http://www.wildtangent.com/index.html"
	If a CWebRequest cannot be made, returns NULL.
	If the url is a http: or https: url, the actual return type is an CHttpWebRequest
	Example:
	<code>
		CHttpWebRequest* pClass = WT::DynamicCast<CHttpWebRequest>(CWebRequest::Create ("http://www.wildtangent.com/index.html"));
	</code>

	If you know what type you expect back, you can call specialized versions
	the return the expected type directly (e.g., CreateHttp)
	*/
	static CWebRequest* __fastcall Create (const string& URL);

	/*
	A specialization of the Create call.  Will use the 'Create' call and
	then does the correct WT::DynamicCast to get the return value.

	Example:
	<code>
		CHttpWebRequest* pClass = CWebRequest::CreateHttp ("http://www.wildtangent.com/index.html");
	</code>
	*/
	static CHttpWebRequest* __fastcall CreateHttp (const string& URL);

	/* 
	Better constructor for the Http object.  If the filename is blank, will download to
	a memory stream; otherwise downloads to the given file (it will append to the file).
	Simply call 'Begin()' to start the download.

	Example:
	<code>
	CHttpWebRequest* pHttp = CWebRequest::CreateHttp (
		"http://www.google.com", 
		"c:\\temp\\google.html", 
		CWebRequest::KAutoBegin|CWebRequest::KAutoCache, 
		0, // Callback
		0, // Async object
		0); // User object
	while (pHttp->GetPercentDone() < 100.0f)
	{
		Sleep (200);
	}
	const bool Ok = pHttp->DownloadIsOk();
	char Buffer [200];
	if (Ok)
	{
		sprintf (Buffer, "You have Google with %d", pHttp->GetStatusCode());
	}
	else
	{
		sprintf (Buffer, "You do not have the Google with %d", pHttp->GetStatusCode());
	}
	MessageBox (0, Buffer, "Download's done!", MB_OK);
	</code>
	 */
	enum EHttpFlags { KAutoCache=1, KSetAsyncThis=2, KAutoBegin=4};
	static CHttpWebRequest* __fastcall CreateHttp (const string& URL, const string& Filename, const int Flags, CAsyncCallback* Callback, void* pObj, void* pUserData);
	/*
	 Creates a new WebRequest out of Uri (which you can make from a string);
	If a WebRequest cannot be made, returns NULL.
	If the url is a http: or https: url, the actual return type is an HttpWebRequest
	Example:
	<code>
		CHttpWebRequest* pClass = WT::DynamicCast<CHttpWebRequest>(CWebRequest::Create (CUri ("http://server/file.ext")));
	</code>

	If you know what type you expect back, you can call specialized versions
	the return the expected type directly (e.g., CreateHttp)
	*/
	static CWebRequest* __fastcall Create (const CUri& Uri);

	/*
	A specialization of the Create call.  Will use the 'Create' call and
	then does the correct WT::DynamicCast to get the return value.

	Example:
	<code>
		CHttpWebRequest* pClass = CWebRequest::CreateHttp (CUri("http://www.wildtangent.com/index.html"));
	</code>
	*/
	static CHttpWebRequest* __fastcall CreateHttp (const CUri& Uri);

	friend CHttpWebRequest;
};

} // end namespace Net
} // end namespace System
} // end namespace WT