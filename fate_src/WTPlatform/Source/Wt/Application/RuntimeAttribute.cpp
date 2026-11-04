//
// RuntimeAttribute.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//


#include <windows.h>

#include "WT/stdafx.h"
#include "WT/Application/RuntimeAttribute.h"
#include "WT/System/Exception.h"
#include <stdio.h>
#include <algorithm>
#include <locale>
#include <tchar.h>

#define MAX_KEY_LENGTH 255
#define SKU_REGISTRY_KEY_LOCATION "SOFTWARE\\WildTangent\\InstalledSKUs"
#define DEV_SKU_NAME				"TESTDEV0"
#define NO_SKU_NAME					"NOSKU"
#define INI_FILE_NAME				"SKUInfo.ini"
#define INI_HEADER_NAME				"SKUInfo"
#define SKU_KEY_NAME				"SKU"

namespace WT
{
namespace Application
{

CRuntimeAttribute::CRuntimeAttribute() 
	: m_DevSku (false)
	, m_UseLocalIniFile(false)
#ifndef NDEBUG
	, m_NoSku( false )
#endif
{
	m_IniFileName = GetApplicationDirectory() + std::string(INI_FILE_NAME);
	m_SKU = FindSKU ( );
}

CRuntimeAttribute::~CRuntimeAttribute()
{
}

std::string const CRuntimeAttribute::GetRegString ( const std::string &HKLM_KeyName, const std::string &StringName ) const
{
	std::string Result;

    HKEY hKey;
    RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,			// handle of open key 
		HKLM_KeyName.c_str(),		// address of name of subkey to open 
		0,							// reserved 
		KEY_READ,					// security access mask 
		&hKey						// address of handle of open key 
		);    

	try
	{
		TCHAR ReturnValue[MAX_KEY_LENGTH];
		DWORD ReturnValueBufferSize = MAX_KEY_LENGTH;
	    DWORD dwValueType = REG_SZ;

		LONG ret = RegQueryValueEx(
			hKey,						// handle of key to query 
			StringName.c_str(),			// address of name of value to query 
			0,							// reserved 
			&dwValueType,				// address of buffer for value type 
			(LPBYTE)ReturnValue,		// address of data buffer 
			&ReturnValueBufferSize		// address of data buffer size 
			);

		if ( ret != ERROR_SUCCESS )
		{
			//TODO: throw standard exception   throw System::CException( ErrorMessage );
			throw ( "Unable to get the value from the registry" );
		}

		Result = ReturnValue;
	}
	catch ( ... )
	{
		RegCloseKey(hKey);
		std::string ErrorMessage("Exception while querying the requested attribute: ");
		ErrorMessage+=StringName;
		throw ( ErrorMessage );
	}

    RegCloseKey(hKey);
	return Result;
}

std::string const CRuntimeAttribute::GetRegStringIfPresent ( const std::string &HKLM_KeyName, const std::string &StringName ) const
{
	std::string Result;

    HKEY hKey;
    RegOpenKeyEx(
		HKEY_LOCAL_MACHINE,			// handle of open key 
		HKLM_KeyName.c_str(),		// address of name of subkey to open 
		0,							// reserved 
		KEY_READ,					// security access mask 
		&hKey						// address of handle of open key 
		);    

	TCHAR ReturnValue[MAX_KEY_LENGTH];
	DWORD ReturnValueBufferSize = MAX_KEY_LENGTH;
	DWORD dwValueType = REG_SZ;

	LONG ret = RegQueryValueEx(
		hKey,						// handle of key to query 
		StringName.c_str(),			// address of name of value to query 
		0,							// reserved 
		&dwValueType,				// address of buffer for value type 
		(LPBYTE)ReturnValue,		// address of data buffer 
		&ReturnValueBufferSize		// address of data buffer size 
		);

	if ( ret != ERROR_SUCCESS )
	{
		Result = "";
	}
	else
	{
        Result = ReturnValue;
	}
	
	RegCloseKey(hKey);
	return Result;
}


bool CRuntimeAttribute::IniFileExists( ) const
{
	bool Result = false;

	HANDLE hFile = ::CreateFile( m_IniFileName.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, 0, NULL );
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle( hFile );
		Result = true;
	}

	return Result;
}


std::string const CRuntimeAttribute::GetIniFileString ( const std::string &KeyName ) const
{
	try
	{
		if (IniFileExists())
		{
			TCHAR s[MAX_KEY_LENGTH];
			GetPrivateProfileString(INI_HEADER_NAME,KeyName.c_str(),"",s,MAX_KEY_LENGTH,m_IniFileName.c_str());
			if (((int)_tcslen (s))>0)
			{
				return s;
			}
		}
		else
		{
			throw ( " file not found: " INI_FILE_NAME );
		}
	}
	catch ( ... )
	{
		std::string ErrorMessage("Exception while querying the INI file:" INI_FILE_NAME " for the requested attribute: ");
		ErrorMessage += KeyName;
		throw ( ErrorMessage );
	}
	throw ( "Unable to get the value from the ini file" );
	return "";
}

std::string const CRuntimeAttribute::GetPath( std::string FileName ) const
{
	size_t cpos = FileName.find_last_of('\\');
	if (cpos>=0)
	{
		FileName.erase(cpos+1);
	}
	return FileName;
}


std::string const CRuntimeAttribute::GetApplicationDirectory ( void ) const
{
	TCHAR ReturnValue[_MAX_PATH];
	GetModuleFileName(::GetModuleHandle(0), reinterpret_cast<LPTSTR>(ReturnValue), _MAX_PATH); 
	return GetPath( ReturnValue );
}

std::string const CRuntimeAttribute::GetApplicationExecutableName ( void ) const
{
	TCHAR ReturnValue[_MAX_PATH];
	GetModuleFileName(::GetModuleHandle(0), reinterpret_cast<LPTSTR>(ReturnValue), _MAX_PATH); 
	
	const _TCHAR* pStr;
	pStr = _tcsrchr(ReturnValue, '\\'); 
	if (pStr != NULL)
	{
		_tcsncpy ( ReturnValue, ++pStr, _MAX_PATH );
	}
	else
	{
		return "";
	}
	return ReturnValue;
}

std::string CRuntimeAttribute::GetSKU() const
{
	return m_SKU;
}

/*
	In order for a SKU to get data about itself from the registry, it needs to know what its SKU number is.  
	The most intelligent way of doing this is to have the WTGameLib parse all SKU’s in the registry 
	and look for the InstallDirectory string and the ExeHash string.  If the executing code’s hash matches 
	the ExeHash stored in the registry, and the install directory is the same, then we can be assured that 
	the data in that key is for the code which is running.
*/
std::string const CRuntimeAttribute::FindSKU ( void )
{
	std::string ApplicationDirectory = GetApplicationDirectory ( );

	

	bool TestDevSKUExists = false;
	HKEY		hKey;
    DWORD		cSubKeys=0;					// number of subkeys 

	if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, SKU_REGISTRY_KEY_LOCATION, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
	{
		DWORD retCode = RegQueryInfoKey(
			hKey,						// key handle 
			NULL,						// buffer for class name 
			NULL,						// size of class string 
			NULL,						// reserved 
			&cSubKeys,					// number of subkeys 
			NULL,						// longest subkey size 
			NULL,						// longest class string 
			NULL,						// number of values for this key 
			NULL,						// longest value name 
			NULL,						// longest value data 
			NULL,						// security descriptor 
			NULL);						// last write time 
	
		// Enumerate the subkeys, until RegEnumKeyEx fails.

		if (cSubKeys)
		{
			for (DWORD i = 0; i < cSubKeys; i++) 
			{ 
			    FILETIME	ftLastWriteTime;			// last write time 
				TCHAR		ReturnValue	[_MAX_PATH];
				DWORD		ReturnValueBufferSize = _MAX_PATH;

				retCode = RegEnumKeyEx(hKey, i,
						ReturnValue,
						&ReturnValueBufferSize,
						NULL,
						NULL,
						NULL,
						&ftLastWriteTime);
				std::string achKey = ReturnValue;
				if (achKey==DEV_SKU_NAME)
				{
					TestDevSKUExists = true;
				}

				if (retCode == ERROR_SUCCESS) 
				{
					std::string SKUInstallDirectory = GetRegString ( SKU_REGISTRY_KEY_LOCATION "\\" + achKey, "InstallDirectory" );
					if (		(StringLower(ApplicationDirectory) == StringLower(SKUInstallDirectory)) 
							||  (StringLower(ApplicationDirectory) == StringLower(SKUInstallDirectory) +"\\"))
					{
					// If we've found a sku in the registry that is installed into the folder we're running from
					// chances are, it's the one we want.

						// see if the registry key "RuntimeExeName" matches the current exe name
						//  RuntimeExeName may not be there, so don't except if it isn't
						if (   StringLower(GetRegStringIfPresent( SKU_REGISTRY_KEY_LOCATION "\\" + achKey, "RuntimeExeName"))
							== StringLower(GetApplicationExecutableName()))
						{
							RegCloseKey(hKey);
							TestDevSKUExists = false;
							return achKey;
						}

						// see if the registry key "ExeName" matches the current exe name
						if (   StringLower(GetRegString( SKU_REGISTRY_KEY_LOCATION "\\" + achKey, "ExeName"))
							== StringLower(GetApplicationExecutableName()))
						{
							RegCloseKey(hKey);
							TestDevSKUExists = false;
							return achKey;
						}
					}
				}
			}
		}
		RegCloseKey(hKey);
	}
	
	try
	{
		std::string ReturnValue = GetIniFileString (SKU_KEY_NAME );
		m_UseLocalIniFile=true;
		return ReturnValue;
	}
	catch ( ... )
	{
		// don't echo exceptions 
		m_UseLocalIniFile=false;
	}
	// If we've gotten here... we haven't found a regkey for this SKU.  If it there is a DEV regkey, return that instead.
	if ((cSubKeys) && (TestDevSKUExists == true))
	{
		m_DevSku = true;
		return DEV_SKU_NAME;
	}

#ifndef NDEBUG
	::MessageBox( NULL, "CRuntimeAttribute::FindSKU() failed. Ignoring for debug mode only.\n"
						"Running \\\\wtcfs01\\public\\wtplatform\\TESTDEV0_Setup.wsf will suppress this error.",
						"CRuntimeAttribute", MB_ICONEXCLAMATION | MB_OK );
	m_NoSku = true;
	return NO_SKU_NAME;
#else
	throw _T("CRuntimeAttribute::FindSKU - Could not find SKU info about this product.");
#endif
}

std::string const	CRuntimeAttribute::GetString	( const std::string &Name ) const
{
#ifndef NDEBUG
	if (m_NoSku)
	{
		if (Name == "DPCodeName")
		{
			static char const KDefaultDPCodeName[] = "wildgames";
			return KDefaultDPCodeName;
		}

		return "NOSKU";
	}
#endif

	if ( (m_DevSku == true) || (m_UseLocalIniFile==true) )
	{
		if ((Name == "ExeName") || (Name=="LaunchExeName") || (Name=="RuntimeExeName"))
		{
			return GetApplicationExecutableName();
		}
		if (Name == "InstallDirectory")
		{
			return GetApplicationDirectory();
		}
	}

	if (m_UseLocalIniFile)
	{
		return(GetIniFileString ( Name ));
	}
	else
	{
		std::string KeyName = SKU_REGISTRY_KEY_LOCATION + std::string("\\") + m_SKU;
		return( GetRegString( KeyName, Name ));
	}
}

int	CRuntimeAttribute::GetInt		( const std::string &Name ) const
{
	return _ttoi( GetString(Name).c_str() );
}

float CRuntimeAttribute::GetFloat	( const std::string &Name ) const
{
	return (float)_tcstod ( GetString(Name).c_str(), NULL );
}

bool CRuntimeAttribute::GetBool		( const std::string &Name ) const
{
	std::string BoolVal = StringLower(GetString( Name ));
	if (BoolVal == "true")
	{
		return true;
	}
	else if (BoolVal == "false")
	{
		return false;
	}
	else
	{
		throw _T("CRuntimeAttribute::GetBool - value not a boolean type.");
	}
}

}
}