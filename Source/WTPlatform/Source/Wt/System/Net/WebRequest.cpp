/*
	HttpRequest.cpp

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#include "WT/stdafx.h"

#include "WT/System/IO/TextStreamReader.h"

#include <iostream>
#include <sstream>
#include <assert.h>
#include "WT/System/Net/WebRequest.h"
#include "WT/System/Net/HttpWebRequest.h"

namespace WT
{
namespace System
{
namespace Net
{

WT_DEFINE_CLASS(CWebRequest)
CWebRequest::CWebRequest()
{
}


CWebRequest::~CWebRequest()
{
}


CWebRequest* CWebRequest::Create (const string& URL)
{
	const CUri Uri (URL);
	return Create (Uri);
}


CWebRequest* CWebRequest::Create (const CUri& Uri)
{
	CWebRequest* Retval = 0;
	const string Scheme = Uri.GetScheme();
	if (Scheme == "http" || Scheme == "https")
	{
		Retval = new CHttpWebRequest (Uri);
	}
	else
	{
		assert (false);
	}
	assert (Retval);
	return Retval;
}

CHttpWebRequest* CWebRequest::CreateHttp (const string& URL)
{
	CWebRequest* RawRetval =  Create (CUri (URL));
	CHttpWebRequest* Retval = WT::DynamicCast< CHttpWebRequest > (RawRetval);
	return Retval;
}

CHttpWebRequest* CWebRequest::CreateHttp (const CUri& Uri)
{
	CWebRequest* RawRetval =  Create (Uri);
	CHttpWebRequest* Retval = WT::DynamicCast< CHttpWebRequest > (RawRetval);
	return Retval;
}

CHttpWebRequest* CWebRequest::CreateHttp (const string& Uri, const string& Filename, const int Flags, CAsyncCallback* Callback, void* pObj, void* pUserData)
{
	CWebRequest* RawRetval =  Create (Uri); // Uri==URL
	CHttpWebRequest* Retval = WT::DynamicCast< CHttpWebRequest > (RawRetval);
	Retval->SetDownloadFilename (Filename);
	if (Callback)
	{
		Retval->AddCallback (Callback);
	}
	Retval->SetAsyncObj (pObj);
	Retval->SetUserData (pUserData);
	if (Flags & KAutoCache)
	{
		Retval->SetDoCache ();
	}
	if (Flags & KSetAsyncThis)
	{
		Retval->SetAsyncObj (Retval);
	}
	if (Flags & KAutoBegin)
	{
		Retval->Begin();
	}
	return Retval;
}

} // end namespace Net
} // end namespace System
} // end namespace WT