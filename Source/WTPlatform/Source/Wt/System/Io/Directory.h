/*
	Directory.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/IO/DirectoryInfo.h"

namespace WT
{
namespace System
{
namespace IO
{

#ifdef _WIN32
#undef CreateDirectory
#endif


typedef SmartPointer<class CDirectory> HDirectory;
class CDirectory : public CObject
{
public:

	WT_CLASS( CDirectory , CObject )

	// Creates all directories and subdirectories as specified by the given path
	static CDirectoryInfo WTCall CreateDirectory( const std::string& Path );

	// Creates all directories and subdirectories as specified by the given filename
	// For example, "c:\foo\bar.txt" will create c:\foo (unlike CreateDirectory
	// which will make a directory c:\foo\bar.txt\)
	static CDirectoryInfo WTCall CreateDirectoryToFile( const std::string& Path );

	// Deletes an empty directory from a specified path
	static void WTCall Delete( const std::string& Path );

	// Deletes the specified directory and, if indicated, any subdirectories in the directory
	static void WTCall Delete( const std::string& Path, bool Recursive );

	// Determines whether the given path refers to an existing directory on disk.
	// There is a corresponding CFile::Exists() method.
	static bool WTCall Exists( const std::string& Path );

	// Gets the creation date and time of a directory
	static CDateTime WTCall GetCreationTime( const std::string& Path );

	// Gets the creation date and time, in coordinated universal time (UTC) format, of a directory
	static CDateTime WTCall GetCreationTimeUtc( const std::string& Path );

	// Gets the current working directory of the application
	static std::string WTCall GetCurrentDirectory();

	// Gets the names of subdirectories in the specified directory
	static vector<string> WTCall GetDirectories( const std::string& Path );

	// Gets an array of directories matching the specified search pattern from the current directory
	static vector<string> WTCall GetDirectories( const std::string& Path, const std::string& SearchPattern );

	// Returns the volume information, root information, or both for the specified path
	static std::string WTCall GetDirectoryRoot( const std::string& Path );

	// Returns the names of files in the specified directory
	static vector<string> WTCall GetFiles( const std::string Path );

	// Returns the names of files in the specified directory that match the specified search pattern
	static vector<string> WTCall GetFiles( const std::string& Path, const std::string& SearchPattern );

	// Returns the names of all files and subdirectories in the specified directory
	static vector<string> WTCall GetFileSystemEntries( const std::string& Path );

	// Returns an array of file system entries matching the specified search criteria
	static vector<string> WTCall GetFileSystemEntries( const std::string& Path, const std::string& SearchPattern );

	// Returns the date and time the specified file or directory was last accessed
	static CDateTime WTCall GetLastAccessTime( const std::string& Path );

	// Returns the date and time, in universal coordinated time (UTC) format, that the specified file or directory was last accessed
	static CDateTime WTCall GetLastAccessTimeUtc( const std::string& Path );

	// Returns the date and time the specified file or directory was last written to
	static CDateTime WTCall GetLastWriteTime( const std::string& Path );

	// Returns the date and time, in universal coordinated time (UTC) format, that the specified file or directory was last written to
	static CDateTime WTCall GetLastWriteTimeUtc( const std::string& Path );

	// Retrieves the names of the logical drives on this computer in the form "<drive letter>:\"
	static vector<string> WTCall GetLogicalDrives();

	// Retrieves the parent directory of the specified path, including both absolute and relative paths
	static CDirectoryInfo WTCall GetParent( const std::string& Path );

	// Moves a file or a directory and its contents to a new location
	static void WTCall Move( const std::string& SourceDirName, const std::string& DestDirName );

	// Sets the application's current working directory to the specified directory
	static void WTCall SetCurrentDirectory( const std::string& Path );

};

}	// IO
}	// System
}	// WT