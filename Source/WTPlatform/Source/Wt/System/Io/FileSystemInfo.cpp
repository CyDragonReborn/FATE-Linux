/*
	FileSystemInfo.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#include "WT/stdafx.h"
#include "WT/System/IO/IOException.h"
#include "FileSystemInfo.h"
#include "Directory.h"
#include "FilePath.h"
#include "windows.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CFileSystemInfo)

// Sets the FileAttributes of the current CFileSystemInfo
// The Attributes parameter is expected to contain one or more logically ORed FileAttributes::Attributes flags
void CFileSystemInfo::SetAttribues( uint32 Attributes )
{
	DWORD dwFileAttributes = 0L;

	if( Attributes & FileAttributes::Archive )
		dwFileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
	if( Attributes & FileAttributes::Hidden )
		dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
	if( Attributes & FileAttributes::Normal )
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;
	if( Attributes & FileAttributes::NotContentIndexed )
		dwFileAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
	if( Attributes & FileAttributes::Offline )
		dwFileAttributes |= FILE_ATTRIBUTE_OFFLINE;
	if( Attributes & FileAttributes::ReadOnly )
		dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
	if( Attributes & FileAttributes::System )
		dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
	if( Attributes & FileAttributes::Temporary )
		dwFileAttributes |= FILE_ATTRIBUTE_TEMPORARY;

	if( dwFileAttributes == 0L )
	{
		throw CArgumentException( "Invalid attribute set" );
	}

	if( (dwFileAttributes & FILE_ATTRIBUTE_NORMAL) && (dwFileAttributes != FILE_ATTRIBUTE_NORMAL) )
	{
		dwFileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
	}

	if( FALSE == ::SetFileAttributes( m_FullPath.c_str(), dwFileAttributes ) )
	{
		DWORD dwError = ::GetLastError();

		switch( dwError )
		{
		default:
			throw CException();
		case ERROR_FILE_NOT_FOUND:
			throw CFileNotFoundException();
		case ERROR_PATH_NOT_FOUND:
			throw CDirectoryNotFoundException();
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		case ERROR_INVALID_PARAMETER:
			throw CArgumentException();
		}
	}

	Refresh();
}

// Retrieves the FileAttributes of the current CFileSystemInfo
// The return value will be one or more logically ORed FileAttributes::Attributes flags
uint32 CFileSystemInfo::GetAttribues() const
{
	DWORD dwFileAttributes = ::GetFileAttributes( m_FullPath.c_str() );

	if( dwFileAttributes == INVALID_FILE_ATTRIBUTES )
	{
		DWORD dwError = ::GetLastError();

		switch( dwError )
		{
		default:
			throw CException();
		case ERROR_FILE_NOT_FOUND:
			throw CFileNotFoundException();
		case ERROR_PATH_NOT_FOUND:
			throw CDirectoryNotFoundException();
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		case ERROR_INVALID_PARAMETER:
			throw CArgumentException();
		}
	}

	uint32 Attributes = 0;

	if( dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
		Attributes |= FileAttributes::Archive;
	if( dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED )
		Attributes |= FileAttributes::Compressed;
	if( dwFileAttributes & FILE_ATTRIBUTE_DEVICE )
		Attributes |= FileAttributes::Device;
	if( dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		Attributes |= FileAttributes::Directory;
	if( dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED )
		Attributes |= FileAttributes::Encrypted;
	if( dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
		Attributes |= FileAttributes::Hidden;
	if( dwFileAttributes & FILE_ATTRIBUTE_NORMAL )
		Attributes |= FileAttributes::Normal;
	if( dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED )
		Attributes |= FileAttributes::NotContentIndexed;
	if( dwFileAttributes & FILE_ATTRIBUTE_OFFLINE )
		Attributes |= FileAttributes::Offline;
	if( dwFileAttributes & FILE_ATTRIBUTE_READONLY )
		Attributes |= FileAttributes::ReadOnly;
	if( dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
		Attributes |= FileAttributes::ReparsePoint;
	if( dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE )
		Attributes |= FileAttributes::SparseFile;
	if( dwFileAttributes & FILE_ATTRIBUTE_SYSTEM )
		Attributes |= FileAttributes::System;
	if( dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY )
		Attributes |= FileAttributes::Temporary;

	return Attributes;
}

// Retrieves the creation time of the current CFileSystemInfo object
CDateTime CFileSystemInfo::GetCreationTime() const
{
	return CDirectory::GetCreationTime( m_FullPath );
}

// Retrieves the creation time, in coordinated universal time (UTC), of the current CFileSystemInfo object
CDateTime CFileSystemInfo::GetCreationTimeUtc() const
{
	return CDirectory::GetCreationTimeUtc( m_FullPath );
}

// Determines whether the underlying file system object exists
bool CFileSystemInfo::Exists() const
{
	return CDirectory::Exists( m_FullPath );
}

// Gets the string representing the extension part of the file
std::string CFileSystemInfo::GetExtension() const
{
	return CFilePath::GetExtension( m_FullPath );
}

// Gets the full path of the directory or file
std::string CFileSystemInfo::GetFullName() const
{
	return m_FullPath;
}

// Retrieves the time the current file or directory was last accessed
CDateTime CFileSystemInfo::GetLastAccessTime() const
{
	return CDirectory::GetLastAccessTime( m_FullPath );
}

// Retrieves the time when the current file or directory was last written to
CDateTime CFileSystemInfo::GetLastWriteTime() const
{
	return CDirectory::GetLastWriteTime( m_FullPath );
}

// Retrieves the time, in coordinated universal time (UTC), when the current file or directory was last written to
CDateTime CFileSystemInfo::GetLastWriteTimeUtc() const
{
	return CDirectory::GetLastWriteTimeUtc( m_FullPath );
}

// For files, gets the name of the file
// For directories, gets the name of the last directory in the hierarchy if a hierarchy exists	
std::string CFileSystemInfo::GetName() const
{
	assert( false );
	return std::string();
}

// Refreshes the state of the object
void CFileSystemInfo::Refresh()
{
	m_FullPath = CFilePath::GetFullPath( m_OriginalPath );
	if( m_FullPath.empty() )
	{
		throw CArgumentException( "The file was not found" ); 
	}
}

}	// IO
}	// System
}	// WT