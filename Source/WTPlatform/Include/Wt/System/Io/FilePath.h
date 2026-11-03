#pragma once

#include "WT/System/Platform.h"
#include <string>

namespace WT
{
namespace System
{
namespace IO
{

class CFilePath
{
public:

	static const char DirectorySeparatorChar = '\\';
	static const char AltDirectorySeparatorChar = '/';
	static const char VolumeSeparatorChar = ':';
	static const char PathSeparator = ';';

	static string WTCall ChangeExtension ( const string& path , const string& extension );
	static string WTCall GetDirectoryName ( const string& path );
	static string WTCall GetExtension ( const string& path );
	static string WTCall GetFullPath ( const string& path );
	static string WTCall GetFileName ( const string& path );
	static string WTCall GetFileNameWithoutExtension ( const string& path );
	static string WTCall GetPathRoot ( const string& path );
	static string WTCall GetTemporaryPath();
	static string WTCall GetTemporaryFileName();
	static bool WTCall HasExtension ( const string& path );
	static bool WTCall IsPathRooted ( const string& path );
	static string WTCall Combine ( const string& path1 , const string& path2 );
	static bool WTCall IsValidPathCharacter( char PathCharacter );
	static bool WTCall IsDirectorySeparator( char PathCharacter );
};

}
}
}
