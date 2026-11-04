/*
	DirectXVersionUtil.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

*/

#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Application
{

class CDirectXVersionUtil
{
public:

	/// Retrieves the currently installed DirectX version
	/// This method returns S_OK upon success, or E_FAIL if the DirectX version info couldn't be determined
	static HRESULT WTCall GetVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );

	/// Retrieves the currently installed DirectX version from DxDiag's COM interface
	/// This method returns S_OK upon success, or E_FAIL if the DirectX version info couldn't be determined
	static HRESULT WTCall GetVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );

	/// Retrieves the currently installed DirectX version by looking at DirectX file versions
	/// This method returns S_OK upon success, or E_FAIL if the DirectX version info couldn't be determined
	static HRESULT WTCall GetVersionViaFileVersions( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );

	/// Returns ULARGE_INTEGER with a file version of a file, or a failure code.
	static HRESULT WTCall GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion );

	/// Returns a ULARGE_INTEGER where a<<48|b<<32|c<<16|d<<0
	static ULARGE_INTEGER WTCall MakeInt64( WORD a, WORD b, WORD c, WORD d );

	/// Compares two ULARGE_INTEGER values
	/// This method returns:
	///	 1 if ullParam1 > ullParam2
	///  0 if ullParam1 = ullParam2
	/// -1 if ullParam1 < ullParam2
	static int WTCall CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 );
};

}
}
