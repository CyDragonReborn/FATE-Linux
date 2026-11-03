/*
	HttpWebResponse.cpp

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */

#include "WT/stdafx.h"

#include "WT/System/IO/TextStreamReader.h"

#include <iostream>
#include <sstream>
#include "WT/System/Net/WebResponse.h"
#include "WT/System/Net/HttpWebRequest.h"
#include "WT/System/Net/WebRequestEnum.h"

namespace WT
{
namespace System
{
namespace Net
{

WT_DEFINE_CLASS(CWebResponse)

//
// Note that it's not possible to make one of these without a valid m_pRequest.
//

int64 CHttpWebResponse::GetContentLength() const
{
	const int64 Retval = m_pRequest->GetContentLength();
	return Retval;
}

// GetHeaderValue (string("Last-Modified")) --> "Fri, 06 Jan 20-06 21:36:52 GMT"
const string CHttpWebResponse::GetHeaderValue(const string& Name, const string& DefaultValue) const
{
	const string StartsWith = Name + ": ";
	const size_t Len = StartsWith.length();
	for (vector<string>::const_iterator i=m_Headers.begin(); i!=m_Headers.end(); i++)
	{
		const string& Line=(*i);
		if (strncmp (Line.c_str(), StartsWith.c_str(), Len) == 0)
		{
			return string(Line.c_str() + Len);
		}
	}
	return DefaultValue;
}

bool CHttpWebResponse::GetIsDone() const
{
	const bool Retval = m_pRequest->GetIsDone();
	return Retval;
}

WT::System::IO::CStream* CHttpWebResponse::GetResponseStream ()
{
	WT::System::IO::CStream* const Retval = m_pRequest->GetResponseStream ();
	return Retval;
}

HttpStatusCode::Enum CHttpWebResponse::GetStatusCode() const
{ 
	const HttpStatusCode::Enum Retval = m_pRequest->GetStatusCode(); 
	return Retval;
}



} // end namespace Net
} // end namespace System
} // end namespace WT