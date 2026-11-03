/*
	WebResponse.h

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
class CHttpWebResponse;

/// WebResponse is a base class for all CSystem::CNet responses; as of September 2005
/// the only useful derived class is the HttpWebResponse that you get
/// from an HttpWebRequest.
class CWebResponse : public CObject
{
private:
	CWebResponse() { }
public:
	virtual ~CWebResponse() { }

	/// Gets the length that the downloaded file should be according to the server.
	/// If the server does not report file lengths, it will be set after the download
	/// is complete and successful based on the number of bytes actually downloaded.
	virtual int64 GetContentLength() const =0;

	/// Gets a list of headers that the server returned back to the client.
	/// Headers are often uninteresting, but can be useful for debugging.
	virtual const vector<string>& GetHeaders() const =0;

	/// Gets the results of the download in a stream format.  The downloader does
	/// not put data into files; you have to do that yourself.
	virtual WT::System::IO::CStream* GetResponseStream() = 0;

	friend CHttpWebResponse;
	friend CHttpWebRequest;
};

} // end namespace Net
} // end namespace System
} // end namespace WT