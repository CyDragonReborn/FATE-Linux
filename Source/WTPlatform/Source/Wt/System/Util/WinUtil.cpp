/******************************************************************************
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	History:
	2/13/2006	Rui Jiang	Createed

	Description:  Windows Specific Utility functions/classes. 
******************************************************************************/
#include "WT/stdafx.h"
#include "WT/System/Platform.h"
#include "WT/System/Util/Util.h"
#include "WT/System/Logging/Log.h"

#include <accctrl.h>
#include <aclapi.h>
#include <shlobj.h>

//warning C4127: conditional expression is constant
#pragma warning(disable: 4127)

namespace WT
{
namespace System
{
namespace Util
{

/*
Create a security descriptor for a user. You need to free the *ppNewDacl that returned and 
also the returned security descriptor. example:
	SECURITY_ATTRIBUTES sa;
	PACL acl = NULL;
	memset(&sa, 0, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = false;
	sa.lpSecurityDescriptor = CreateSecurityDescriptorForUser(&acl, "EVERYONE", NO_INHERITANCE, 
		SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL);
	hFile = CreateFile(m_TrackFilePathName.c_str(), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		sa.lpSecurityDescriptor? &sa : NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (sa.lpSecurityDescriptor)
	{
		LocalFree(sa.lpSecurityDescriptor);
	}
	if (acl) LocalFree(acl);
Added by Rui 2/14/2006
*/
PSECURITY_DESCRIPTOR WTCall CreateSecurityDescriptorForUser(PACL* ppNewDacl, LPCSTR lpUserName, DWORD Inheritance, DWORD AccessPermissions)
{
	PACL newDacl = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	bool res = false;
	do
	{
		//Set registry security is only available on Win2000, XP. Change it later.
		EXPLICIT_ACCESS ea;
		memset(&ea, 0, sizeof(ea));
		ea.grfAccessPermissions = AccessPermissions;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = Inheritance;
		ea.Trustee.pMultipleTrustee = NULL;
		ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
		ea.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
		ea.Trustee.ptstrName = (LPSTR)lpUserName;

		if(SetEntriesInAcl(1, &ea, NULL, &newDacl) != ERROR_SUCCESS) break;
		// Initialize a security descriptor.  
		pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH); 
		if (NULL == pSD) break;

		if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) break;
		if (!SetSecurityDescriptorDacl(pSD, true, newDacl, false)) break;
		res = true;
	} while(false);

	//if (newDacl) LocalFree(newDacl);
	if (!res)
	{
		if (pSD) LocalFree(pSD);
		pSD = NULL;
		if (newDacl) LocalFree(newDacl);
		newDacl = NULL;
	}
	*ppNewDacl = newDacl;
	return pSD;
}

HANDLE WTCall CreateFileForEveryOne(LPCSTR lpFileName, 
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes
)
{
	SECURITY_ATTRIBUTES sa;
	PACL acl = NULL;
	memset(&sa, 0, sizeof(sa));
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = false;
	sa.lpSecurityDescriptor = CreateSecurityDescriptorForUser(&acl, "EVERYONE", NO_INHERITANCE, 
		SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL);
	HANDLE hFile = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, 
		sa.lpSecurityDescriptor? &sa : NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
	if (sa.lpSecurityDescriptor)
	{
		LocalFree(sa.lpSecurityDescriptor);
	}
	if (acl) LocalFree(acl);
	return hFile;
}

//Determine if the specified directory exist
//Added by Rui 2/14/2006
bool WTCall IsDirExist(const std::string & name)
{
      WIN32_FIND_DATA data;
      if (name.size() == 0) return false;
	  std::string s = name + "\\*.*";
      //Determine whether the source is a directory or file.
      HANDLE hFileFind = FindFirstFile(s.c_str(), &data);
      if (hFileFind == INVALID_HANDLE_VALUE)
      {
            return false;
      }

      FindClose(hFileFind);
      //return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
      return true;
}

//Recursively create directory.
//Added by Rui 2/14/2006
bool WTCall CreateDirRecursive(const std::string& name)
{
      if (name.size() == 0) return true;
      if (IsDirExist(name)) return true;

	  std::string parentDir = GetParentDir(name);
      if (parentDir.size() == name.size())
      {
            return CreateDirectory(name.c_str(), NULL) != FALSE;
      }
      else
      {
            if (!CreateDirRecursive(parentDir))
            {
                  return false;
            }
            else
            {
                  return CreateDirectory(name.c_str(), NULL) != FALSE;
            }
      }
      return true;
}

//One win98, there seems no commonly defined program files. But we could get it from reg.
// HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion, ProgramFilesDir
BOOL WTCall GetProgramFilesDirFromRegistry(LPSTR szBuf, DWORD dwBufSize)
{
	LONG lr;
	HKEY hKey = NULL;
	BOOL res = FALSE;
	do
	{
		lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_QUERY_VALUE, &hKey);
		if (lr != 0) break;
		DWORD dw = dwBufSize;
		DWORD dwType;
		lr = RegQueryValueEx(hKey, "ProgramFilesDir", NULL, &dwType, (LPBYTE)szBuf, &dw);
		if (lr != 0) break;
		if (dwBufSize) szBuf[dwBufSize - 1] = 0;
		res = TRUE;
	} while(false);	
	if (hKey) RegCloseKey(hKey);
	return res;
}

std::string WTCall GetSystemPath()
{
	char buf[MAX_PATH];
	std::string Result;

	//AppData first. 
	if (SHGetSpecialFolderPath(NULL, buf, CSIDL_SYSTEM, FALSE))
	{
		LOG("Using the system path at: %s", buf);
		Result = buf;
	}
	else if (GetSystemDirectory(buf, MAX_PATH))
	{
		Result = buf;
	}
	else if (GetWindowsDirectory(buf, MAX_PATH))
	{
		Result = buf;
	}
	else
	{
		//What the hell?
		Result = "C:";
	}

	return Result;
}

std::string WTCall GetProgramFilesPath()
{
	char buf[MAX_PATH];
	std::string Result;

	if (SHGetSpecialFolderPath(NULL, buf, CSIDL_PROGRAM_FILES, TRUE))
	{
		LOG("Using the program files folder at: %s", buf);
		Result = buf;
	}
	else if (GetProgramFilesDirFromRegistry(buf, MAX_PATH))
	{
		LOG("Got the program files folder from registry %s", buf);
		Result = buf;
	}
	else if (GetWindowsDirectory(buf, MAX_PATH))
	{
		Result = buf;
	}
	else
	{
		//What the hell?
		Result = "C:";
	}

	return Result;
}


std::string WTCall GetCommonAppDataPath()
{
	char buf[MAX_PATH];
	std::string Result;

	//AppData first. 
	if (SHGetSpecialFolderPath(NULL, buf, CSIDL_COMMON_APPDATA, TRUE))
	{
		LOG("Using the common appdata at: %s", buf);
		Result = buf;
	} else {
		Result = WT::System::Util::GetProgramFilesPath();
	}

	return Result;
}

}}} //namespace
