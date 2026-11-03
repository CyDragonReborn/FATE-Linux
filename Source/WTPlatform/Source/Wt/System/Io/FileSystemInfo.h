/*
	FileSystemInfo.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/DateTime.h"

namespace WT
{
namespace System
{
namespace IO
{

namespace FileAttributes
{
	enum Attributes
	{
		Archive = 32,		// Applications use this attribute to mark files for backup or removal
        Compressed = 2048,	// The file is compressed
		Device = 64,		// Reserved for future use
		Directory = 16,		// The file is a directory
		Encrypted = 16384,	// The file or directory is encrypted
							// For a file, this means that all data in the file is encrypted
							// For a directory, this means that encryption is the default for newly created files and directories 
		Hidden = 2,			// The file is hidden, and thus is not included in an ordinary directory listing
		Normal = 128,		// The file is normal and has no other attributes set
							// This attribute is valid only if used alone
		NotContentIndexed = 8192, // The file will not be indexed by the operating system's content indexing service
		Offline  = 4096,	// The file is offline. The data of the file is not immediately available
		ReadOnly = 1,		// The file is read-only
		ReparsePoint = 1024,// The file contains a reparse point, which is a block of user-defined data associated with a file or a directory
		SparseFile = 512,	// The file is a sparse file
							// Sparse files are typically large files whose data are mostly zeros
		System = 4,			// The file is a system file
							// The file is part of the operating system or is used exclusively by the operating system
		Temporary =  256,	// The file is temporary
							// File systems attempt to keep all of the data in memory for quicker access rather than flushing the data back to mass storage
							// A temporary file should be deleted by the application as soon as it is no longer needed
	};
}

typedef SmartPointer<class CFileSystemInfo> HFileSystemInfo;
class CFileSystemInfo : public CObject
{
public:

	WT_CLASS( CFileSystemInfo , CObject )

	// Sets the FileAttributes of the current CFileSystemInfo
	// The Attributes parameter is expected to contain one or more logically ORed FileAttributes::Attributes flags
	virtual void SetAttribues( uint32 Attributes );

	// Retrieves the FileAttributes of the current CFileSystemInfo
	// The return value will be one or more logically ORed FileAttributes::Attributes flags
	virtual uint32 GetAttribues() const;

	// Retrieves the creation time of the current CFileSystemInfo object
	virtual CDateTime GetCreationTime() const;

	// Sets the creation time, in coordinated universal time (UTC), of the current CFileSystemInfo object
	//TODO: NIY virtual void SetCreationTimeUtc( const CDateTime& CreationTime );

	// Retrieves the creation time, in coordinated universal time (UTC), of the current CFileSystemInfo object
	virtual CDateTime GetCreationTimeUtc() const;

	// Determines whether the underlying file system object exists
	virtual bool Exists() const;

	// Gets the string representing the extension part of the file
	virtual std::string GetExtension() const;

	// Gets the full path of the directory or file
	virtual std::string GetFullName() const;

	// Retrieves the time the current file or directory was last accessed
	virtual CDateTime GetLastAccessTime() const;

	// Retrieves the time, in coordinated universal time (UTC), that the current file or directory was last accessed
	//TODO: NIY: virtual CDateTime GetLastAccessTimeUtc() const;

	// Retrieves the time when the current file or directory was last written to
	virtual CDateTime GetLastWriteTime() const;

	// Retrieves the time, in coordinated universal time (UTC), when the current file or directory was last written to
	virtual CDateTime GetLastWriteTimeUtc() const;

	// For files, gets the name of the file
	// For directories, gets the name of the last directory in the hierarchy if a hierarchy exists	
	virtual std::string GetName() const;

	// Refreshes the state of the object
	virtual void Refresh();

protected:

	CFileSystemInfo()
	{
	}

	virtual ~CFileSystemInfo()
	{
	}

protected:

	std::string m_FullPath;	// the fully qualified path of the directory or file
	std::string m_OriginalPath; // the path originally specified by the user, whether relative or absolute

};

}	// IO
}	// System
}	// WT