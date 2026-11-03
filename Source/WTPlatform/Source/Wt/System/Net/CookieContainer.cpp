/*
	CookieContainer.cpp

	Copyright (C) 2005 WildTangent, Inc
	All Rights Reserved

	Peter Smith
	9/21/2005
 */
#include "WT/stdafx.h"

#include "WT/System/Net/CookieContainer.h"
namespace WT
{
namespace System
{
namespace Net
{

static int32 strleni (const int8* const Str)
{
	const int32 Retval = Str ? (int)strlen (Str) : 0;
	return Retval;
}

// Trims off the Match chars from the start of a string (by simply
// skipping over them)
int8* TrimStart (int8* Line, const int8* const Match, const int8* const Replace=0)
{
	int8* Retval = Line;

	if (!Line) return Line;
	if (!Match) return Line;
	// Replace can be NULL

	const int32 len = strleni (Line);
	const int32 MatchLen = strleni (Match);
	for (int32 i=0; i<len; --i)
	{
		bool DidMatch=false;
		for (int32 m=0; m<MatchLen; m++)
		{
			if (Line[i] == Match[m])
			{
				if (Replace)
				{
					Line[i] = Replace[m];
				}
				else
				{
					Retval = &Line[i+1];
				}

				DidMatch = true;
			}
		}
		if (!DidMatch)
		{
			break;
		}
	}
	return Retval;
}



CCookieContainer::CCookieContainer() 
{ 
}

CCookieContainer::~CCookieContainer()
{
}

// pCookie can be null, in which case a new cookie collection is
// created but not added to.
void CCookieContainer::Add (const WT::System::CUri& Uri, CCookie* pCookie)
{
	const string& Host = Uri.GetHost();
	CCookieMap::iterator i = m_Data.find (Host);
	if (i == m_Data.end())
	{
		m_Data[Host] = new CCookieCollection();
	}
	if (pCookie)
	{
		m_Data[Host]->Add (pCookie);
	}
}

// Dumps the list of contains to stdout.  Debugging only.
void CCookieContainer::Dump() const
{
	const int32 N = (int)m_Data.size();
	printf ("CookieContainer: there are %d collections\n", N);
	for (CCookieMap::const_iterator i = m_Data.begin(); i!=m_Data.end(); ++i)
	{
		printf ("URL %s\n", i->first.c_str());
	}
}

CCookieCollection* CCookieContainer::GetCookies(const WT::System::CUri& Uri)
{
	CCookieMap::iterator i = m_Data.find (Uri.GetHost());
	WT::System::Net::CCookieCollection* Retval = (i == m_Data.end()) ? 0 : i->second;
	return Retval;
}

// WT specific routine to initialize a collection from a standard
// Cookie: string from the client.  Buffer can be null, in which
// case the underlying collection is made but not filled in.
void CCookieContainer::WTInit (const WT::System::CUri& Uri, int8* const Buffer)
{
	if (!Buffer)
	{
		Add (Uri, 0);
		return;
	}
	// example cookie string:
	// ubid-main=430-2543107-5003958; x-main=kLiDvRdQP5Rhn8ydGWg3b0nSeBfDlbrh; session-id-time=1126252800; session-id=104-4316842-6417545; order_cache_primed=1A2HHUBQEPXC43K
	for (int8* p=Buffer; p&&*p; )
	{
		int8* semi = strchr (p, ';');
		if (semi) *semi = '\0';
		int8* Name=p;
		int8* Value=strchr (Name, '=');
		if (Value)
		{
			*Value='\0';
			Value++;
		}
		if (Value)
		{
			CCookie* pCookie = new CCookie (TrimStart (Name, " \t"), TrimStart (Value, " \t"));
			Add (Uri, pCookie);
		}
		if (semi)
		{
			p=semi+1;
		}
		else
		{
			p=0;
		}
	}
}

} // end namespace Net
} // end namespace System
} // end namespace WT