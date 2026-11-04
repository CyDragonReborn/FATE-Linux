
/*
	dx8app.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _GETDXVER_H
#define _GETDXVER_H
HRESULT GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );

HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
HRESULT GetDirectXVerionViaFileVersions( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
HRESULT GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion );
ULARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d );
int CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 );





#endif
