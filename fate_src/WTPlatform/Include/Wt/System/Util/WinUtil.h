/******************************************************************************
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	History:
	2/13/2006	Rui Jiang	Createed

	Description:  Windows Specific Utility functions/classes. 
******************************************************************************/
#pragma once

#include "WT/System/Platform.h"

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
PSECURITY_DESCRIPTOR WTCall CreateSecurityDescriptorForUser(PACL* ppNewDacl, LPCSTR lpUserName, DWORD Inheritance, DWORD AccessPermissions);

//Create a file taht EVERYONE has full (read/write etc) access to it.
HANDLE WTCall CreateFileForEveryOne(LPCSTR lpFileName, 
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes
);

//Determine if the specified directory exist
//Added by Rui 2/14/2006
bool WTCall IsDirExist(const std::string & name);

//Recursively create directory.
//Added by Rui 2/14/2006
bool WTCall CreateDirRecursive(const std::string& name);

//One win98, there seems no commonly defined program files. But we could get it from reg.
// HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion, ProgramFilesDir
BOOL WTCall GetProgramFilesDirFromRegistry(LPSTR szBuf, DWORD dwBufSize);

std::string WTCall GetSystemPath();
std::string WTCall GetCommonAppDataPath();
std::string WTCall GetProgramFilesPath();

}}} //namespace
