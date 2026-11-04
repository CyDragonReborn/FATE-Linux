/*
	Directory.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#include "WT/stdafx.h"
#include "WT/System/IO/IOException.h"
#include "Directory.h"
#include "FilePath.h"
#include "File.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CDirectory)

static void ValidatePath( const std::string& Path )
{
	if( Path.empty() )
	{
		throw CArgumentException( "Empty path" );
	}

	if( Path.length() > MAX_PATH )
	{
		throw CPathTooLongException();
	}

	std::string::const_iterator it = Path.begin();
	while( it != Path.end() )
	{
		char ch = *it;

		if( ! CFilePath::IsValidPathCharacter( ch ) )
		{
			throw CArgumentException( "Path contains one or more invalid characters" );
		}

		if( ! ::isspace( ch ) )
			return;		
		++it;
	}	
	throw CArgumentException( "Path contains only whitespace characters" );
}

static void GetFileAttributeData( const std::string& Path, WIN32_FILE_ATTRIBUTE_DATA& AttrData )
{
	if( FALSE == ::GetFileAttributesEx( Path.c_str(), GetFileExInfoStandard, (LPVOID) &AttrData ) )
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
}

// Extracts the name of the parent directory for a given path
static std::string GetParentPath( const std::string Path )
{
	// Start before the last char, so that if the last char is "\", we can ignore it
	for( int32 i = (int32)Path.size() - 2; i >= 0; i-- )
	{
		char c = Path[i];
		if (c == '/' || c == '\\')
		{
			//in case \\computer1
			if (i > 0 && (Path[i-1] == '/' || Path[i-1] =='\\'))
				i--;
			return Path.substr(0, i);
		}
	}
	return "";
}

	
// Creates all directories and subdirectories as specified by the given path
CDirectoryInfo WTCall CDirectory::CreateDirectory( const std::string& Path )
{
	ValidatePath( Path );

	if( FALSE == ::CreateDirectoryA( Path.c_str(), NULL ) )
	{
		DWORD dwError = ::GetLastError();
		
		switch( dwError )
		{
		default:
			throw CException();
		case ERROR_ALREADY_EXISTS:
			throw CIOException( "Directory already exists" );
		case ERROR_PATH_NOT_FOUND:
			try
			{
				CDirectory::CreateDirectory( GetParentPath( Path ) );
				CDirectory::CreateDirectory( Path );
			}
			catch(...)
			{
				throw;
			}
			break;
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		case ERROR_INVALID_PARAMETER:
			throw CArgumentException();
		}
	}

	return CDirectoryInfo( Path );
}

// Creates all directories and subdirectories as specified by the given path
CDirectoryInfo WTCall CDirectory::CreateDirectoryToFile( const std::string& Filename )
{
	std::string Path = GetParentPath( Filename );
	if (Exists (Path))
	{
		return CDirectoryInfo (Path);
	}

	return CreateDirectory (Path);
}


// Deletes an empty directory from a specified path
void WTCall CDirectory::Delete( const std::string& Path )
{
	Delete( Path, false );
}

// Deletes the specified directory and, if indicated, any subdirectories in the directory
void WTCall CDirectory::Delete( const std::string& Path, bool Recursive )
{
	ValidatePath( Path );

	if( ! CDirectory::Exists( Path ) )
	{
		throw CDirectoryNotFoundException();
	}

	if( ::GetFileAttributes( Path.c_str() ) & FILE_ATTRIBUTE_READONLY )
	{
		throw CIOException( "Directory was read-only" );
	}
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = ::FindFirstFile( Path.c_str(), &FindFileData );
    while( hFind != INVALID_HANDLE_VALUE )
	{
		if( ! (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&	// not a directory
			FindFileData.cFileName[0] != '.' )								// not a 'special' entry
		{
			throw CIOException( "Directory not empty" );
		}
		if( ! ::FindNextFile( hFind, &FindFileData ) )
			break;
	}
	::FindClose( hFind );

	if( Recursive )
	{		
		const vector<string> Directories = CDirectory::GetDirectories( Path );
		vector<string>::const_iterator it = Directories.begin();
		while( it != Directories.end() )
		{
			std::string Directory = Path;
			if( Path.at( Path.length()-1 ) != '\\' &&
				Path.at( Path.length()-1 ) != '/' )
			{
				Directory += '\\';
			}
			Directory += *it;

			CDirectory::Delete( Directory, true );

			++it;
		}               
	}

	if( FALSE == ::RemoveDirectory( Path.c_str() ) )
	{
		DWORD dwError = ::GetLastError();
		
		switch( dwError )
		{
		default:
			throw CException();
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		}
	}
}

// Determines whether the given path refers to an existing directory on disk
bool WTCall CDirectory::Exists( const std::string& Path )
{
	DWORD dwFileAttributes = ::GetFileAttributes( Path.c_str() );
	
	if( (dwFileAttributes != INVALID_FILE_ATTRIBUTES) && // valid attributes
		(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	 // contains directory flag
	{
		return true;
	}
	return false;
}

int64 MAKELONGLONG( uint32 low, uint32 high )
{
	return (static_cast<uint64>(high) << 32) + low;
}

// Gets the creation date and time of a directory
CDateTime WTCall CDirectory::GetCreationTime( const std::string& Path )
{
	ValidatePath( Path );

	WIN32_FILE_ATTRIBUTE_DATA AttrData;
	GetFileAttributeData( Path, AttrData );
	
	const int64 FileTime = MAKELONGLONG( AttrData.ftCreationTime.dwLowDateTime,
										 AttrData.ftCreationTime.dwHighDateTime );

	return CDateTime::FromFileTime( FileTime );
}

// Gets the creation date and time, in coordinated universal time (UTC) format, of a directory
CDateTime WTCall CDirectory::GetCreationTimeUtc( const std::string& Path )
{
	return CDirectory::GetCreationTime( Path ).ToUniversalTime();
}

// Gets the current working directory of the application
std::string WTCall CDirectory::GetCurrentDirectory()
{
	CHAR Dir[MAX_PATH];
	if( 0L == ::GetCurrentDirectory( MAX_PATH, Dir ) )
	{
		throw CUnauthorizedAccessException();
	}
	Dir[MAX_PATH-1] = '\0';
	return Dir;
}

// Gets the names of subdirectories in the specified directory
vector<string> WTCall CDirectory::GetDirectories( const std::string& Path )
{
	return GetDirectories( Path, "*" );
}

// Gets an array of directories matching the specified search pattern from the current directory
vector<string> WTCall CDirectory::GetDirectories( const std::string& Path, const std::string& SearchPattern )
{
	ValidatePath( Path );

	if( ! CDirectory::Exists( Path ) )
	{
		throw CDirectoryNotFoundException();
	}

	DWORD dwFileAttributes = ::GetFileAttributes( Path.c_str() );

	if( ! (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
	{
		throw CIOException( "Path is a file name" );
	}

	std::string Search = Path;
	if( Path.at( Path.length()-1 ) != '\\' &&
		Path.at( Path.length()-1 ) != '/' )
	{
		Search += '\\';
	}
    Search += SearchPattern;

	WIN32_FIND_DATA FindFileData;
	vector<string> Directories;

	HANDLE hFind = ::FindFirstFile( Search.c_str(), &FindFileData );
    while( hFind != INVALID_HANDLE_VALUE )
	{
		if( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&	// is a directory
			FindFileData.cFileName[0] != '.' )								// not a 'special' entry
		{
			Directories.push_back( FindFileData.cFileName );
		}
		if( ! ::FindNextFile( hFind, &FindFileData ) )
			break;
	}
	::FindClose( hFind );

	return Directories;
}

// Returns the volume information, root information, or both for the specified path
std::string WTCall CDirectory::GetDirectoryRoot( const std::string& Path )
{
	ValidatePath( Path );

	return CFilePath::GetPathRoot( Path );
}

// Returns the names of files in the specified directory
vector<string> WTCall CDirectory::GetFiles( const std::string Path )
{
	return GetFiles( Path, "*" );
}

// Returns the names of files in the specified directory that match the specified search pattern
vector<string> WTCall CDirectory::GetFiles( const std::string& Path, const std::string& SearchPattern )
{
	ValidatePath( Path );

	if( ! CDirectory::Exists( Path ) )
	{
		throw CDirectoryNotFoundException();
	}

	DWORD dwFileAttributes = ::GetFileAttributes( Path.c_str() );

	if( ! (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
	{
		throw CIOException( "Path is a file name" );
	}

	std::string Search = Path;
	if( Path.at( Path.length()-1 ) != '\\' &&
		Path.at( Path.length()-1 ) != '/' )
	{
		Search += '\\';
	}
    Search += SearchPattern;

	WIN32_FIND_DATA FindFileData;
	vector<string> Files;

	HANDLE hFind = ::FindFirstFile( Search.c_str(), &FindFileData );
    while( hFind != INVALID_HANDLE_VALUE )
	{
		if( ! (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	// not a directory
		{
			Files.push_back( FindFileData.cFileName );
		}
		if( ! ::FindNextFile( hFind, &FindFileData ) )
			break;
	}
	::FindClose( hFind );

	return Files;
}

// Returns the names of all files and subdirectories in the specified directory
vector<string> WTCall CDirectory::GetFileSystemEntries( const std::string& Path )
{
	return GetFileSystemEntries( Path, "*" );
}

// Returns an array of file system entries matching the specified search criteria
vector<string> WTCall CDirectory::GetFileSystemEntries( const std::string& Path, const std::string& SearchPattern )
{
	ValidatePath( Path );

	if( ! CDirectory::Exists( Path ) )
	{
		throw CDirectoryNotFoundException();
	}

	DWORD dwFileAttributes = ::GetFileAttributes( Path.c_str() );

	if( ! (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
	{
		throw CIOException( "Path is a file name" );
	}

	std::string Search = Path;
	if( Path.at( Path.length()-1 ) != '\\' &&
		Path.at( Path.length()-1 ) != '/' )
	{
		Search += '\\';
	}
    Search += SearchPattern;

	WIN32_FIND_DATA FindFileData;
	vector<string> Entries;

	HANDLE hFind = ::FindFirstFile( Search.c_str(), &FindFileData );
    while( hFind != INVALID_HANDLE_VALUE )
	{
		// Here, we include 'special' entries such as '.' and '..'
		Entries.push_back( FindFileData.cFileName );

		if( ! ::FindNextFile( hFind, &FindFileData ) )
			break;
	}
	::FindClose( hFind );

	return Entries;
}

// Returns the date and time the specified file or directory was last accessed
CDateTime WTCall CDirectory::GetLastAccessTime( const std::string& Path )
{
	ValidatePath( Path );

	WIN32_FILE_ATTRIBUTE_DATA AttrData;
	GetFileAttributeData( Path, AttrData );

	const int64 FileTime = MAKELONGLONG( AttrData.ftLastAccessTime.dwLowDateTime,
										 AttrData.ftLastAccessTime.dwHighDateTime );

	return CDateTime::FromFileTime( FileTime );
}

// Returns the date and time, in universal coordinated time (UTC) format, that the specified file or directory was last accessed
CDateTime WTCall CDirectory::GetLastAccessTimeUtc( const std::string& Path )
{
	return CDirectory::GetLastAccessTime( Path ).ToUniversalTime();
}

// Returns the date and time the specified file or directory was last written to
CDateTime WTCall CDirectory::GetLastWriteTime( const std::string& Path )
{
	ValidatePath( Path );

	WIN32_FILE_ATTRIBUTE_DATA AttrData;
	GetFileAttributeData( Path, AttrData );

	const int64 FileTime = MAKELONGLONG( AttrData.ftLastWriteTime.dwLowDateTime,
										 AttrData.ftLastWriteTime.dwHighDateTime );

	return CDateTime::FromFileTime( FileTime );
}

// Returns the date and time, in universal coordinated time (UTC) format, that the specified file or directory was last written to
CDateTime WTCall CDirectory::GetLastWriteTimeUtc( const std::string& Path )
{
	return CDirectory::GetLastWriteTime( Path ).ToUniversalTime();
}

// Retrieves the names of the logical drives on this computer in the form "<drive letter>:\"
vector<string> WTCall CDirectory::GetLogicalDrives()
{
	// The Win32 ::GetLogicalDrives function retrieves a bitmask representing the currently available disk drives
	DWORD dwDrivesMask = ::GetLogicalDrives();

	if( dwDrivesMask == 0L )
	{
		DWORD dwError = ::GetLastError();

        switch( dwError )
		{
		default:
			throw CIOException();
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		}
	}

	vector<string> Drives;
	char Drive = 'A';
	while( dwDrivesMask )
	{
		if( dwDrivesMask & 1 )
		{
			char Buffer[16];
			::sprintf( Buffer, "%c:\\", Drive );
			Drives.push_back( Buffer );
		}

		dwDrivesMask >>= 1;
		++Drive;
	}

	return Drives;
}

// Retrieves the parent directory of the specified path, including both absolute and relative paths
CDirectoryInfo WTCall CDirectory::GetParent( const std::string& Path )
{
	std::string ParentPath;

	if( ! CFilePath::IsPathRooted( Path ) )	
	{
		ParentPath = CFilePath::Combine( CDirectory::GetCurrentDirectory(), GetParentPath( ParentPath ) );
	}
	else
	{
		ParentPath = GetParentPath( ParentPath );
	}

	ValidatePath( ParentPath );

	if( ! CDirectory::Exists( ParentPath ) )
	{
		throw CDirectoryNotFoundException();
	}

	return CDirectoryInfo( ParentPath );
}

// Moves a file or a directory and its contents to a new location
void WTCall CDirectory::Move( const std::string& SourceDirName, const std::string& DestDirName )
{
	ValidatePath( SourceDirName );
	ValidatePath( DestDirName );

	if( SourceDirName.length() == DestDirName.length() )
	{
		if( 0 == ::strcmpi( SourceDirName.c_str(), DestDirName.c_str() ) )
		{
			throw CIOException( "Source and destination parameters refer to the same file or directory" );
		}
	}

	if( CDirectory::Exists( DestDirName ) )
	{
		throw CIOException( "Directory already exists" );
	}

	if( !CFilePath::IsPathRooted( SourceDirName ) || !CFilePath::IsPathRooted( DestDirName ) )
	{
		throw CDirectoryNotFoundException();
	}

	if( 0 == ::strcmpi( CFilePath::GetPathRoot( SourceDirName ).c_str(), CFilePath::GetPathRoot( DestDirName ).c_str() ) )
	{
		throw CIOException( "An attempt was made to move a directory to a different volume" );
	}
	
	std::string strSource;		// Source file
	std::string strDestination; // Destination file

	const std::string strPattern = CFilePath::Combine( SourceDirName, "*.*" );

	// Create destination directory
	CDirectory::CreateDirectory( DestDirName );

	WIN32_FIND_DATA FindData; // File information
	HANDLE hFind = ::FindFirstFile( strPattern.c_str(), &FindData );
	if( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( FindData.cFileName[0] != '.' )
			{
				strSource.erase();
				strDestination.erase();

				strSource = CFilePath::Combine( SourceDirName, FindData.cFileName );
				strDestination = CFilePath::Combine( DestDirName, FindData.cFileName );

				if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					// Copy subdirectory
					CDirectory::Move( strSource, strDestination );
				}
				else
				{
					// Copy file
					CFile::Move( strSource, strDestination );
				}
			}
		}	while( ::FindNextFile( hFind, &FindData ) );
	} 

	// Close handle
	::FindClose( hFind );

	DWORD dwError = ::GetLastError();
	if( dwError != ERROR_NO_MORE_FILES )
	{
		// TODO:
	}
}

// Sets the application's current working directory to the specified directory
void WTCall CDirectory::SetCurrentDirectory( const std::string& Path )
{
	ValidatePath( Path );

	if( ! CDirectory::Exists( Path ) )
	{
		throw CDirectoryNotFoundException();
	}

	if( FALSE == ::SetCurrentDirectory( Path.c_str() ) )
	{
		DWORD dwError = ::GetLastError();

        switch( dwError )
		{
		default:
			throw CIOException();
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		}
	}
}

}	// IO
}	// System
}	// WT