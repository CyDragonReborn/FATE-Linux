#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/FileStream.h"

namespace WT
{
namespace System
{
namespace IO
{

typedef SmartPointer<class CFile> HFile;
class CFile : public CObject
{
public:

	WT_CLASS( CFile , CObject )

	//static AppendText();
	static void WTCall Copy( const string& sourceFileName , const string& destFileName , bool overwrite = false );

	// Deletes the given file
	static void WTCall Delete( const std::string& FileName);

	// Determines whether the given path refers to an existing file on disk.
	// If the 'Path' refers to a directory, the method returns false.
	// There is a corresponding CDirectory::Exists() method.
	static bool WTCall Exists( const std::string& Path );

	// Moves a specified file to a new location, providing the option to specify a new file name
	static void WTCall Move( const std::string& SourceFileName, const std::string& DestFileName );

	// Opens a file, returning a CFileStream
	static CFileStream* WTCall Open( const string& path , FileMode::Enum mode );

	// Opens a file, returning a CFileStream
	static CFileStream* WTCall Open( const string& path , FileMode::Enum mode , FileAccess::Enum access , 
		FileShare::Enum share = FileShare::None, uint32 Flags = 0 );

	// Gets the size of the file in bytes using the 'stat64' function.
	// If the file does not exist, returns -1.
	static int32 WTCall Size (const std::string& Path);

};

}
}
}