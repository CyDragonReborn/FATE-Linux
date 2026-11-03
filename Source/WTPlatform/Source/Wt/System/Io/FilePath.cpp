#include "WT/stdafx.h"

#include "WT/System/IO/FilePath.h"

namespace WT
{
namespace System
{
namespace IO
{

static uint32 GetRootLength ( const string& path );

string WTCall CFilePath::ChangeExtension ( const string& path , const string& extension )
{
	if (!path.empty())
	{
		string s = path;
		for (size_t i = path.size(); --i >= 0;)
		{
			char ch = path[i];
			if (ch == '.')
			{
				s = path.substr (0, i);
				break;
			}
			if (ch == DirectorySeparatorChar || ch == AltDirectorySeparatorChar || ch == VolumeSeparatorChar)
				break;
		}
		if ( !extension.empty() && path.size() != 0)
		{
			if (extension.size() == 0 || extension[0] != '.')
			{
				s = s + ".";
			}
			s += extension;
		}
		return s;
	}
	return "";
}

string WTCall CFilePath::GetDirectoryName ( const string& path )
{
	if (!path.empty())
	{
		size_t root = GetRootLength(path);
		size_t i = path.size();
		if (i > root)
		{
			i = path.size();
			if (i == root)
				return "";
			while (i > root && path[--i] != DirectorySeparatorChar && path[i] != AltDirectorySeparatorChar );
			return path.substr(0, i);
		}
	}
	return "";
}

uint32 GetRootLength ( const string& path )
{
	size_t i = 0;
	size_t length = path.size();

	if (length >= 1 && (CFilePath::IsDirectorySeparator(path[0])))
	{
		i = 1;
		if (length >= 2 && (CFilePath::IsDirectorySeparator(path[1])))
		{
			i = 2;
			int n = 2;
			while (i < length && ((path[i] != CFilePath::DirectorySeparatorChar && path[i] != CFilePath::AltDirectorySeparatorChar) || --n > 0))
				i++;
		}
	}
	else if (length >= 2 && path[1] == CFilePath::VolumeSeparatorChar)
	{
		i = 2;
		if (length >= 3 && (CFilePath::IsDirectorySeparator(path[2])))
			i++;
	}
	return (uint32) i;
}

bool WTCall CFilePath::IsDirectorySeparator( char PathCharacter )
{
	return PathCharacter == DirectorySeparatorChar || 
		PathCharacter == AltDirectorySeparatorChar;
}


string WTCall CFilePath::GetExtension ( const string& path )
{
	if (path.empty())
		return "";

	size_t length = path.size();
	for (size_t i = length; --i >= 0;)
	{
		char ch = path[i];
		if (ch == '.')
		{
			if (i != length - 1)
				return path.substr(i, length - i);
			else
				return "";
		}
		if (ch == DirectorySeparatorChar || ch == AltDirectorySeparatorChar || ch == VolumeSeparatorChar)
			break;
	}
	return "";
}

string WTCall CFilePath::GetFullPath ( const string& path )
{
	char buffer[MAX_PATH+1];
	LPTSTR filePart;
	if ( GetFullPathName ( path.c_str() , MAX_PATH , buffer , &filePart ) == 0 )
		return "";
	return buffer;
}
string WTCall CFilePath::GetFileName ( const string& path )
{
	if (!path.empty())
	{
		int32 length = static_cast<int32>(path.size());	// cast to signed so we can check for < 0
		for (int32 i = length; --i >= 0;)
		{
			char ch = path[i];
			if (ch == DirectorySeparatorChar || ch == AltDirectorySeparatorChar || ch == VolumeSeparatorChar)
				return path.substr(i + 1, length - i - 1);
		}
	}
	return path;
}

string WTCall CFilePath::GetFileNameWithoutExtension ( const string& path )
{
	string fileName = GetFileName(path);
	if (!fileName.empty())
	{
		size_t i;
		if ((i=fileName.find_last_of(".")) == string::npos)
			return fileName;
		else
			return fileName.substr(0,i);
	}
	return "";
}

string WTCall CFilePath::GetPathRoot ( const string& path )
{
	if (path.empty())
		return "";
	return path.substr(0, GetRootLength(path));
}

string WTCall CFilePath::GetTemporaryPath()
{
	char buffer[MAX_PATH+1];
	if ( GetTempPath(MAX_PATH , buffer ) == 0 )
		return "";
	return buffer;
}

string WTCall CFilePath::GetTemporaryFileName()
{
	char buffer[MAX_PATH+1];
	string path = GetTemporaryPath();
	if ( GetTempFileName(path.c_str(),"tmp",0,buffer ) == 0 )
		return "";
	return buffer;
}

bool WTCall CFilePath::HasExtension ( const string& path )
{
	if (!path.empty())
	{
		for (size_t i = path.size(); --i >= 0;)
		{
			char ch = path[i];
			if (ch == '.') {
				if ( i != path.size() - 1)
					return true;
				else
					return false;
			}
			if (ch == DirectorySeparatorChar || ch == AltDirectorySeparatorChar || ch == VolumeSeparatorChar)
				break;
		}
	}
	return false;
}
    
bool WTCall CFilePath::IsPathRooted ( const string& path )
{
	if (!path.empty())
	{
		size_t length = path.size();
		if (length >= 1 && (path[0] == DirectorySeparatorChar || path[0] == AltDirectorySeparatorChar) ||
			length >= 2 && path[1] == VolumeSeparatorChar)
			return true;
	}
	return false;
}

string WTCall CFilePath::Combine ( const string& path1, const string& path2 )
{
	if (path2.size() == 0)
		return path1;

	if (path1.size() == 0)
		return path2;
	    
	if (IsPathRooted(path2))
		return path2;

	char ch = path1[path1.size() - 1];
	if (ch != DirectorySeparatorChar && ch != AltDirectorySeparatorChar && ch != VolumeSeparatorChar) 
		return path1 + DirectorySeparatorChar + path2;
	return path1 + path2;
}

bool WTCall CFilePath::IsValidPathCharacter( char PathCharacter )
{
	static const char InvalidPathChars[] = { '\042', // ", 
											 '\074', // <
											 '\076', // > 
											 '\174', // | 
											 '\000', // \0 
											 '\010', // \b
									 		 '\020', '\021', '\022',
											 '\024', '\025', '\026', '\027', '\030', '\031',
	};

	char const* pEnd = &InvalidPathChars[sizeof InvalidPathChars];
	return std::find( InvalidPathChars, pEnd, PathCharacter ) == pEnd;
}

}
}
}
