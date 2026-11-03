/*
	DirectoryInfo.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/IO/FileSystemInfo.h"

namespace WT
{
namespace System
{
namespace IO
{

typedef SmartPointer<class CDirectoryInfo> HDirectoryInfo;
class CDirectoryInfo : public CFileSystemInfo
{
public:

	WT_CLASS( CDirectoryInfo , CFileSystemInfo )

	// Gets the name of the last directory in the hierarchy if a hierarchy exists	
	std::string GetName() const;

	// Gets the parent directory of a specified subdirectory
	CDirectoryInfo GetParent() const;

	// Gets the root portion of a path
	CDirectoryInfo GetRoot() const;

    CDirectoryInfo( const std::string& Path );
	virtual ~CDirectoryInfo();

private:

	CDirectoryInfo();

};

}	// IO
}	// System
}	// WT