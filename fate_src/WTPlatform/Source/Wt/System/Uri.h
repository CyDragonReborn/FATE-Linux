/*
	Uri.h

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#pragma once

#include "WT/System/Object.h"
#include <vector>


namespace WT
{
namespace System
{
namespace UriHostNameType
{
	enum Enum { Basic, Dns, IPv4, IPv6, Unknown };
}


// The Uri class is a simple class that, given a string of a url
// (like "http://www.wildtangent.com:80/index.html#main")
// will split it into its various parts.  Important because the
// download code only takes Uri and not strings.
//
// What's not handled: IPV4 and IPV6 hosts (http://127.0.0.1/index.html)
// and it doesn't convert urls into their canonical representations.
typedef SmartPointer<class CUri> HUri;
class CUri : public CObject
{
public:
	WT_CLASS( CUri, CObject );

	/// Creates a blank Uri.  There are no methods exposed for changing 
	/// a Uri once created, so this constructor isn't actually useful.
	CUri();

	/// Creates a Uri out of a string.  This is the primary constructor.
	/// An example of a proper URL is:
	/// http://www.wildtangent.com:80/index.html#main
	CUri(const std::string& URL);

	/// Creates a Uri as a "diff" between an existing Uri and a user string.
	/// NIY: as of 9/19/2005
	CUri(const CUri& BaseUri, const std::string& URL);

	/// Standard destructor for a Uri.
	virtual ~CUri();


	/// Returns the path for a URL.  For example, in 
	/// http://www.wildtangent.com:80/index.html#main
	/// the path is index.html
	const std::string& GetAbsolutePath() const;

	/// Returns the authority for a URL.  For example, in 
	/// http://www.wildtangent.com:80/index.html#main
	/// the authority is the host and port 
	/// www.wildtangent.com:80.  The port is given even
	/// if the original Url string in the constructor
	/// did not specify one; in that case the default
	/// port of 80 is used.
	std::string GetAuthority() const;


	/// Returns the fragment for a URL.  For example, in 
	/// http://www.wildtangent.com:80/index.html#main
	/// the fragment is the #host.  If no fragment was 
	/// specified, "" is returned.  If the fragment was
	/// specified, it will always be prefixed with a '#'.
	std::string GetFragment() const;

	/// Returns the host for a URL.  For example, in 
	/// http://www.wildtangent.com:80/index.html#main
	/// the host is www.wildtangent.com.  The host 
	/// does not include the port number (use either
	/// GetPort() or GetAuthority() for those).
	const std::string& GetHost() const;


	/// Returns the host name type; in the first version
	/// this will always be 'Dns' (it's part of the 
	/// UriHostNameType::Enum enumerated type).
	/// Future version may return, for example, 'IPv4'
	UriHostNameType::Enum GetHostNameType() const;

	/// Returns true if the port number is the default port number.
	/// As of 9/19/2005 only http and https are supported protocols;
	/// these each use 80 as their default port.  By definition if
	/// you did not supply a port number in your URL in the constructor
	/// the port number is the default port.  Also note that if you
	/// did supply a port number, and you supplied the default value,
	/// then GetIsDefaultPort() will return true since it is the default
	/// value.
	bool GetIsDefaultPort() const;

	/// Always returns false because only the http protocol is
	/// supported.  In the future will return true if the file:
	/// protocol was used.
	bool GetIsFile() const;

	/// Returns true if the host was either "127.0.0.1" OR "localhost"
	/// OR "loopback".
	bool GetIsLoopback() const;

	/// Always returns false because only the http protocol is
	/// supported.  In the future will return true if the file:
	/// protocol was used AND the path (GetAbsolutePath) included
	/// a UNC-style path ("\\server\dir\subdir\file.ext").
	bool GetIsUnc() const; 

	/// Returns the path and query for a URL.  For example, in 
	/// http://www.wildtangent.com:80/index.html?name=value
	/// the PathAndQuery is index.html?name=value.
	/// If no query was provided (either because there is
	/// nothing or because there is a fragment) then the returned
	/// value will not include a '?'.
	std::string GetPathAndQuery() const;


	/// Returns the port number for a URL.  For example, in 
	/// http://www.wildtangent.com:80/index.html#main
	/// the port number is '80'.  Ports are the tcp/ip
	/// address that the web page is at.  If no port number
	/// is given, the default is port 80 (and which will 
	/// be returned by GetPort).
	int32 GetPort() const;


	/// Returns the query portion of a URL.  For example, in
	/// http://www.wildtangent.com:80/index.html?name=value
	/// the query is the ?name=value part.
	/// Due to a limitation in our URL parser, URLs may either
	/// have a query or a fragment but not both.
	/// If no query was given, returns "".  The query string
	/// always includes the question mark.
	std::string GetQuery() const;


	/// Returns the scheme portion of a URL.  For example, in
	/// http://www.wildtangent.com:80/index.html#main
	/// the scheme is the 'http'.  Supported schemes, as of
	/// September 2005, are http and https.  All URLs must
	/// include a scheme; they are not optional.
	/// Historical note: there are many, many schemes that one
	/// company or another has implemented: for example, 'gopher'
	/// was an older and less capable version of http; 'ftp'
	/// is an old-fashioned (but very common) way to transfer
	/// files around.
	const std::string& GetScheme() const;

	/// Returns a vector with all of the part of the URL.
	/// In particular, returns: scheme, host, port, path,
	/// fragment, and query.  In the case of fragment
	/// and query, the strings will not include the '#'
	/// and '?' prefixes that GetFragment and GetQuery
	/// will return.
	std::vector<std::string> GetSegments() const;

	/// Returns the value passed into the (private) Parse method.
	/// Default is false
	bool GetUserEscaped() const;

	/// Always returns "".
	const std::string& GetUserInfo() const;

	/// Returns a string representation of the Uri.  In general the string
	/// can be cut-n-pasted into IE or FireFox.
	virtual std::string ToString() const;

private:
	int32 m_PortNum;
	std::string m_Port;
	std::string m_Scheme;
	std::string m_Host;
	UriHostNameType::Enum m_HostNameType;
	std::string m_Path;
	std::string m_Fragment;
	std::string m_Query;
	bool m_UserEscaped;
	std::string m_UserInfo;

	void Parse(const std::string& URL, const bool UserEscaped=false);

};

} // end namespace System
} // end namespace WT