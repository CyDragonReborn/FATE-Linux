/*
	DirectoryInfo.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#include "WT/stdafx.h"
#include "WT/System/IO/IOException.h"
#include "DirectoryInfo.h"
#include "Directory.h"
#include "FilePath.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CDirectoryInfo)

// Gets the name of the last directory in the hierarchy if a hierarchy exists	
std::string CDirectoryInfo::GetName() const
{
	std::string ParentPath = CDirectory::GetParent( m_FullPath ).m_FullPath;
	return m_FullPath.substr( ParentPath.length()+1 );
}

// Gets the parent directory of a specified subdirectory
CDirectoryInfo CDirectoryInfo::GetParent() const
{
	return CDirectory::GetParent( m_FullPath );
}

// Gets the root portion of a path
CDirectoryInfo CDirectoryInfo::GetRoot() const
{
	return CFilePath::GetPathRoot( m_FullPath );
}


CDirectoryInfo::CDirectoryInfo()
{
}

CDirectoryInfo::CDirectoryInfo( const std::string& Path )
{
	m_OriginalPath = Path;

	if( ! CFilePath::IsPathRooted( Path ) )
	{
		m_FullPath = CFilePath::GetFullPath( Path );
	}
	else
	{
		m_FullPath = Path;
	}

	if( m_FullPath.length() > MAX_PATH )
	{
		throw CPathTooLongException();
	}
	
	for( uint32 i = 0; i < Path.length(); ++i )
	{
		if( ! CFilePath::IsValidPathCharacter( Path.at(i) ) )
		{
			throw CArgumentException( "Path contains invalid characters" );
		}
	}
}

CDirectoryInfo::~CDirectoryInfo()
{
}

}	// IO
}	// System
}	// WT