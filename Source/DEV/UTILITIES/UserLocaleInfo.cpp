/*
	UserLocaleInfo.cpp

	Copyright (C) 2007 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	10/01/2007

*/

//#include "WT\stdafx.h"
#include "../UTILITIES/UserLocaleInfo.h"  		//commented by TDC for Fate
#include "windows.h"

//namespace WT		//commented by TDC for Fate
//{		//commented by TDC for Fate
//namespace System		//commented by TDC for Fate
//{		//commented by TDC for Fate

class CModule
{
public:
	CModule( HMODULE hModule )
		: m_hModule( hModule )
	{
	}

	~CModule()
	{
		if (m_hModule != NULL)
		{
			::FreeLibrary( m_hModule );
		}
	}

	HMODULE GetHModule() const
	{
		return m_hModule;
	}

private:
	HMODULE m_hModule;
};

CUserLocaleInfo::CUserLocaleInfo()
	: m_wsLocaleName( L"en-us" )
{
	typedef int (WINAPI* PGetGeoInfoFunc)(GEOID   GeoId,			// location identifier
										GEOTYPE GeoType,			// type of information requested
										LPWSTR  lpGeoData,			// buffer for results
										int     cchData,			// size of buffer
										LANGID  language);			// language id
	typedef GEOID (WINAPI *PGetUserGeoID)( GEOCLASS  GeoClass );	// location class

	PGetGeoInfoFunc pGetGeoInfo = NULL;
	PGetUserGeoID	pGetUserGeoID = NULL;

    CModule hModKernel32( ::LoadLibrary( "KERNEL32.DLL" ) );
    if (hModKernel32.GetHModule() != NULL)
    {
	    pGetGeoInfo = reinterpret_cast<PGetGeoInfoFunc>( ::GetProcAddress( hModKernel32.GetHModule(), "GetGeoInfoW" ) );
		pGetUserGeoID = reinterpret_cast<PGetUserGeoID>( ::GetProcAddress( hModKernel32.GetHModule(), "GetUserGeoID" ) );
	}

	if (pGetGeoInfo != NULL)
	{
		WCHAR Buffer[MAX_PATH] = {L'\0'};

		GEOID const GeoId = (*pGetUserGeoID)( GEOCLASS_NATION );			// location identifier
		GEOTYPE const GeoType = GEO_RFC1766;						// type of information requested
		int const cchData = sizeof Buffer / sizeof *Buffer - 1;		// size of buffer
		LANGID const language = ::GetUserDefaultUILanguage();		// language id

		int const Result = (*pGetGeoInfo)( GeoId, GeoType, Buffer, cchData, language );
		if (Result)
		{
			_wcslwr( Buffer );
			m_wsLocaleName = Buffer;
		}
	}
}

CUserLocaleInfo::~CUserLocaleInfo()
{
}

std::wstring CUserLocaleInfo::GetLocaleName() const
{
	return m_wsLocaleName;
}
//
//} //namespace System		//commented by TDC for Fate
//} //namespace WT		//commented by TDC for Fate
