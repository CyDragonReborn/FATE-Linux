#include "WT/stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>

#include "File.h"
#include "FilePath.h"
#include "IOException.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CFile)

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
		if( ! CFilePath::IsValidPathCharacter( *it ) )
		{
			throw CArgumentException( "Path contains one or more invalid characters" );
		}

		if( ! ::isspace( *it ) )
			return;		
		++it;
	}	
	throw CArgumentException( "Path contains only whitespace characters" );
}

void WTCall CFile::Copy ( const string& sourceFileName , const string& destFileName , bool overwrite )
{
	//TODO:
	assert (false);
}

void WTCall CFile::Delete ( const string& FileName )
{
	unlink (FileName.c_str());
}

bool WTCall CFile::Exists( const string& Path )
{
	DWORD dwFileAttributes = ::GetFileAttributes( Path.c_str() );
	
	if( (dwFileAttributes != INVALID_FILE_ATTRIBUTES) && // valid attributes
		!(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	 // NOT contains directory flag
	{
		return true;
	}
	return false;
}

CFileStream* WTCall CFile::Open ( const string& path , FileMode::Enum mode )
{
	return new CFileStream( path, mode );
}

CFileStream* WTCall CFile::Open ( const string& path , FileMode::Enum mode , FileAccess::Enum access , FileShare::Enum share,
								  uint32 Flags)
{
	return new CFileStream( path, mode, access, share, Flags );
}

// Moves a specified file to a new location, providing the option to specify a new file name
void WTCall CFile::Move( const std::string& SourceFileName, const std::string& DestFileName )
{
	ValidatePath( SourceFileName );
	ValidatePath( DestFileName );

	if( FALSE == ::MoveFile( SourceFileName.c_str(), DestFileName.c_str() ) )
	{
		const DWORD dwError = ::GetLastError();

		switch( dwError )
		{
		default:
			throw CIOException();
		case ERROR_FILE_NOT_FOUND:
			throw CFileNotFoundException();
		case ERROR_PATH_NOT_FOUND:
			throw CDirectoryNotFoundException();
		case ERROR_ACCESS_DENIED:
			throw CUnauthorizedAccessException();
		case ERROR_INVALID_DRIVE:
			throw CIOException( "The system cannot find the drive specified" );
		case ERROR_WRITE_PROTECT:
			throw CIOException( "The media is write protected" );
		case ERROR_SHARING_VIOLATION:
			throw CIOException( "The process cannot access the file because it is being used by another process" );
		case ERROR_ALREADY_EXISTS:
			throw CIOException( "Cannot create a file when that file already exists" );			
		case ERROR_FILE_EXISTS:
			throw CIOException( "The file exists" );
		case ERROR_NOT_READY:
			throw CIOException( "The device is not ready" );		
		}
	}
}

int32 WTCall CFile::Size (const std::string& Path)
{
	int32 Retval = -1;
	struct _stat filestat;
	const int Status = _stat (Path.c_str(), &filestat);
	if (Status == 0)
	{
		Retval = filestat.st_size;
	}
	return Retval;
}


}
}
}