/*
	Uri.cpp

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#include "WT/stdafx.h"
#include "WT/System/Uri.h"

#include <algorithm> // for transform to do lowercasing
#include <assert.h>
#include <ctype.h>

using std::min;
#include <atlutil.h> // for the url cracker

namespace WT
{
namespace System
{

WT_DEFINE_CLASS(CUri)

// the transform_nodup is a weird transform: it exists in order to remove
// duplicate \ sequences from strings.  
//
// merge together the transform and replace_if to make a new algorithm, "transform_nodup"
// which calls _Pred() on each input item; the _Func() is called every time _Pred() is false
// AND on the first time _in a row_ that _Pred() is true.  When _Pred() is true two or more
// times in a row, _Func() is called only for the first one.
// 
template<class _InIt,
	class _CDest,
	class _Pr,
	class _Fn1> inline
	_CDest& transform_nodup(_InIt _First, _InIt _Last, _CDest& _Dest, _Pr _Pred, _Fn1 _Func)
	{	// transform [_First, _Last) with _Func
	int32 NDup=0;
	for (; _First != _Last; ++_First)
		if (_Pred(*_First)) { _Dest.push_back (_Func(*_First)); NDup=0; }
		else
		{
			//DBG: old codeif (NDup==0) { *_Dest = _Func(*_First); ++_Dest; }
			if (NDup==0) { _Dest.push_back (_Func(*_First)); }
			++NDup;
		}
	return (_Dest);
	}
bool IsNotSlash(const int8 Ch) 
{ 
	const bool Retval = (Ch!='/');
	return Retval;
}
int8 NoXform(int8 Ch) { return Ch; }

	// Url is scheme:://host[:port]/path[#fragment][?query]

CUri::CUri()
{
}

CUri::CUri (const string& URL)
{
	m_UserInfo="";
	Parse(URL);
}

CUri::~CUri ()
{
}

	// Properties
const string& CUri::GetAbsolutePath () const { return m_Path; }
string CUri::GetAuthority() const { return m_Host + ":" + m_Port; }
string CUri::GetFragment() const { if (m_Fragment == "") return ""; return "#" + m_Fragment; }
const string& CUri::GetHost() const { return m_Host; }
UriHostNameType::Enum CUri::GetHostNameType() const { return m_HostNameType; }
bool CUri::GetIsDefaultPort() const { return m_PortNum == 80; }
bool CUri::GetIsFile() const { return false; } // we only support http: right now, and those aren't files
bool CUri::GetIsLoopback () const
{
	const bool Retval = (m_Host == "127.0.0.1" || m_Host == "loopback" || m_Host == "localhost");
	return Retval;
}
bool CUri::GetIsUnc () const { return false; } // we don't support \\server\dir\file.ext names
string CUri::GetPathAndQuery() const { if (m_Query == "") return m_Path; return m_Path + "?" + m_Query; }
int32 CUri::GetPort() const { return m_PortNum; }
string CUri::GetQuery() const { if (m_Query=="") return ""; return "?" + m_Query; }
const string& CUri::GetScheme() const { return m_Scheme; }
vector<string> CUri::GetSegments() const
{
	vector<string> Retval;
	Retval.push_back (m_Scheme);
	Retval.push_back (m_Host);
	Retval.push_back (m_Port);
	Retval.push_back (m_Path);
	Retval.push_back (m_Fragment);
	Retval.push_back (m_Query);
	return Retval;
}
bool CUri::GetUserEscaped () const { return m_UserEscaped; }
const string& CUri::GetUserInfo() const { return m_UserInfo; }

// Methods
string CUri::ToString() const
{
	string Retval = m_Scheme + "://" + m_Host;
	if (m_PortNum != 80) // PortNum and Port are always in sync
	{
		Retval += ":" + m_Port;
	}
	Retval += m_Path;
	if (m_Fragment != "")
	{
		Retval += "#" + m_Fragment;
	}
	if (m_Query != "")
	{
		Retval += "?" + m_Query;
	}
	return Retval;
}

void CUri::Parse (const string& URL, const bool UserEscaped)
{
	m_UserEscaped = UserEscaped; // is "http://bn.com/my%20life" really ..my%2520life..?

	// Set up defaults for everything
	m_Port = "";
	m_PortNum = 0;
	m_Scheme = "";
	m_Host= "";
	m_HostNameType = UriHostNameType::Dns; 
	m_Path = "";
	m_Fragment = "";
	m_Query = "";

	CUrl tmp;
	tmp.CrackUrl (URL.c_str());
	m_Scheme = tmp.GetSchemeName ();
	transform (m_Scheme.begin(), m_Scheme.end(), m_Scheme.begin(), tolower);
	m_Host = tmp.GetHostName ();
	transform (m_Host.begin(), m_Host.end(), m_Host.begin(), tolower);
	m_PortNum = tmp.GetPortNumber ();
	int8 Buffer [32];
	itoa (m_PortNum, Buffer, 10);
	m_Port = Buffer;
	m_Path = "";
	const string RawPath = tmp.GetUrlPath ();
	// Fix up any extra slashes in the path
	transform_nodup (RawPath.begin(), RawPath.end(), m_Path, IsNotSlash, NoXform);
	const int8* const Extra = tmp.GetExtraInfo ();
	if(Extra && Extra[0])
	{
		assert (Extra[0] == '#' || Extra[0]=='?');
		if (Extra[0] == '#')
		{
			m_Fragment = Extra+1;
		}
		else if (Extra[0] == '?')
		{
			m_Query = Extra + 1;
		}
	}

}

} // end namespace System
} // end namespace WT