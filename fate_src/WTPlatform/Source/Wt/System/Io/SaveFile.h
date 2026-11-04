/*
	SaveFile.h

	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	3/28/2006
*/

#pragma once

#include <string>
#include <winnt.h>

namespace WT
{
namespace System
{
namespace IO
{

//Forward declarations
namespace FileMode {enum Enum;}
namespace FileAccess {enum Enum;}
namespace FileShare {enum Enum;}

/*
CSaveFile provides an API for dealing with save files. CSaveFile enforces the correct 
policies including save file location and permissions. Using SaveFile.h requires linking 
to WTPlatform1(D).lib.

Each CSaveFile object you create represents a path to a file or directory - the file may 
or may not exist. Use the Open() call to return a file HANDLE, and call 
::CloseHandle( hFile ) when you are finished using the handle.

Save files are stored in a common directory for this application independent of
the current user. Save files will be readable, writable, and deletable by any 
Windows user.

When constructing a CSaveFile object, you may specify a save file type:
KTypeCache or KTypePersistent. The default value is KTypePersistent.

The value CSaveFile::KTypeCache is used for cached files that will always be deleted on 
game uninstall. Such files will be placed under the "Cache" directory.

The value CSaveFile::KTypePersistent is used for saved data that may survive a 
game uninstall. High score data is an example of this kind of file. Such files will
be placed under the "Persistent" directory.
*/

class CSaveFile
{
public:

	//A type of save file
	enum EType
	{
		KTypeCache,		 //A cache file, which is always deleted on game uninstall.
		KTypePersistent, //A persistent file, which may survive a game uninstall.

		KTypeCount		 //The number of EType choices - not a valid choice itself
	};

	//Construct a save file with the specified FileName, which must be 
	//one or more file/directory names separated by slashes or backslashes. 
	//An example of FileName is "player3\\save6".
	explicit CSaveFile( std::string const& FileName, EType Type = KTypePersistent );

	//Simple open call - this can open, create, truncate, or append to a file. This
	//uses constants found in WT/System/IO/FileStream.h. This returns a handle to the save 
	//file. On failure, INVALID_HANDLE_VALUE is returned; otherwise a valid handle is 
	//returned and you must call ::CloseHandle() when you are finished using it. Note 
	//that requesting to create a file will automatically result in creation of the 
	//necessary directories.
	HANDLE Open( FileMode::Enum Mode ) const;

	//More flexible open call. uses constants found in WT/System/IO/FileStream.h. This 
	//returns a handle to the specified save file. On failure, INVALID_HANDLE_VALUE is 
	//returned; otherwise a valid handle is returned and you must call ::CloseHandle() 
	//when you are finished using it. Note that requesting to create a file will 
	//automatically result in creation of the necessary directories.
	HANDLE Open( 
		FileMode::Enum Mode, 
		FileAccess::Enum Access, 
		FileShare::Enum Share = static_cast<FileShare::Enum>( 1 ) /*FileShare::Read*/,
		unsigned int Flags = 0, bool Async = false ) const;

	//Create a handle to the specified save file. This function uses parameters similar to 
	//Windows' CreateFile. If Inheritable is true, then the security attributes can be 
	//inherited by other processes.	On failure, INVALID_HANDLE_VALUE is returned; otherwise a 
	//valid handle is returned and you must call ::CloseHandle() when you are finished using 
	//it. Note that requesting to create a file will automatically result in creation of the 
	//necessary directories.
	HANDLE Open(
		DWORD dwDesiredAccess, 
		DWORD dwShareMode, 
		DWORD dwCreationDisposition, 
		DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL,
		bool Inheritable = false ) const;

	//Check if a save file or directory exists.
	bool FileExists() const;

	//Create as a directory. Supporting directories will be created,
	//if necessary. If the path already exists (as a file or
	//as a directory), then an exception occurs.
	void CreateDirectory() const;

	//Remove a file or directory. Removing a directory will destroy
	//all of its contents. Removing the last element in a directory will 
	//automatically trigger the destruction of empty supporting directories,
	//as appropriate.
	void Delete() const;

	//Set security attributes on the file or directory. If it is a directory, 
	//and if Recurse is true, then attributes are set on all descendants of 
	//the directory. Note: using CSaveFile::Open() to create files and directories 
	//will automatically result in correct security attributes; this method 
	//is intended to help set correct attributes on externally created files.
	//This function has no effect on OS's below Windows 2000.
	void SetAttributes( bool Recurse = true ) const;

	//Get the path to the directory containing all save files with
	//the specified CSaveFile::EType type.
	static std::string __fastcall GetRootPath( EType Type );

	//
	// Accessors
	//

	// Get the file name that this save file object was constructed with.
	std::string GetFileName() const;

	// Get the type of save file.
	EType GetType() const;

	// Get the full path to the save file.
	std::string GetPath() const;

	//
	// Miscellaneous - copy constructor, operators, and destructor
	//
	CSaveFile( CSaveFile const& SaveFile );
	CSaveFile& operator=( CSaveFile const& SaveFile );

	//Check for equivalence. Two CSaveFile objects are equivalent
	//if their full paths are the same, even though the supplied file 
	//names may be different (for example, "file" and "dir/../file").
	bool operator==( CSaveFile const& SaveFile ) const;
	bool operator!=( CSaveFile const& SaveFile ) const;

	virtual ~CSaveFile();

private:
	std::string m_FileName;
	std::string m_Path;
	int			m_Depth;
	EType		m_Type;
};

}
}
}
