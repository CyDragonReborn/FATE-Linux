//#include "basetypes.h"

#include "constants.h"
#include <shlobj.h>
#include <assert.h>
#include "vistafileutilities.h"
#include "windows.h"
#include "SaveFile.h"

using namespace WT::System::IO;

std::string CVistaFileUtilities::s_strWorkingPath = FindWorkingPath();

const std::string CVistaFileUtilities::FindWorkingPath()
{
	int8*	pStr, szPath[_MAX_PATH ]; 
	GetModuleFileName( NULL, szPath, _MAX_PATH ); 
	pStr = strrchr( szPath, '\\' ); 
	if ( pStr != NULL )
	{
		*( ++pStr ) = '\0';
	}	
	s_strWorkingPath = szPath;
	return s_strWorkingPath;
}

const std::string CVistaFileUtilities::GetWorkingPath()
{
	return s_strWorkingPath;
}

void CVistaFileUtilities::SetWorkingPath( const std::string &path )
{
	s_strWorkingPath = path;
}

const std::string CVistaFileUtilities::GetSharedDataPath( EFileLocationPrefs pref )
{
	switch (pref)
	{
		case KFileLocation_SharedCache:
			return CSaveFile::GetRootPath( KTypeCache );
		break;
	}

	return CSaveFile::GetRootPath( KTypePersistent );
}


std::string CVistaFileUtilities::GetLoadPathFromRelativePath( const std::string &path, EFileLocationPrefs pref )
{
	// If the path actually happens to be an absolute drive path (i.e. it has a colon in it),
	// then just return it right away.  This is a bit of a hack.  It's either this or re-write
	// large chunks of loading code.
	if( s_strWorkingPath == "" )
	{
		s_strWorkingPath = FindWorkingPath();
	}

	if( path.find( ":" ) != std::string::npos )
		return path;

	if( path == "" )
	{
		return s_strWorkingPath;
	}

	std::string strPath = "";

	std::string cleanPath = path;
		// flip all slashes
	if( path.find( "\\" ) == 0 )
	{
		cleanPath = path.substr( 1, path.length() );
	}

	// Use the CSaveFile library functions to retreive the path
	FILE *f;
	std::string strRoot = CSaveFile::GetRootPath( KTypePersistent );

	switch( pref )
	{
	case KFileLocation_SharedCache:
		strRoot = CSaveFile::GetRootPath( KTypeCache );
		strPath = strRoot + "\\" + cleanPath;
		break;
	case KFileLocation_Application:
		// Use the app local path
		strPath =  s_strWorkingPath + cleanPath;
		break;

	case KFileLocation_Shared:
		strPath = strRoot + "\\" + cleanPath;
		break;

	case KFileLocation_Profile:
	case KFileLocation_Automatic:
		// When loading a file, we first look in the application directory
		strPath = strRoot + "\\" + cleanPath;
		f = fopen( strPath.c_str(), "r" );
		if( f )
		{
			fclose( f );
			break;
		}

		strRoot = CSaveFile::GetRootPath( KTypeCache );
		strPath = strRoot + "\\" + cleanPath;
		f = fopen( strPath.c_str(), "r" );
		if( f )
		{
			fclose( f );
			break;
		}
		else if( (GetFileAttributes(strPath.c_str()) | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
		{
			// The file didn't exist, but maybe it's a directory...
			break;
		}
		// If it wasn't there, we try the app data directory.
		strPath = s_strWorkingPath + "\\" + cleanPath;
		break;

	default:
		strPath =  s_strWorkingPath + "\\" + cleanPath;
		break;
	}

	strPath = CleanFilePath( strPath );

#ifdef _MYTEST
	FILE *file = fopen( "z:\\test.txt", "at" );
	if( file )
	{	
		fprintf( file, "loading file: %s\n", strPath.c_str() );		
		fclose( file );
	}
#endif

	return strPath;
}

void CVistaFileUtilities::CreateTempFile( const std::string &path )
{
	std::string pathTo = path;
	pathTo = CleanFilePath( pathTo );

	if( pathTo.find( "." ) != std::string::npos )
		pathTo = pathTo.substr( 0, pathTo.find_last_of( "/" ) );
	
	if( (GetFileAttributes(pathTo.c_str()) | FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
	{
		for(uint32 t = 1; t < (uint32)pathTo.size(); t++ )//testing
		{
			if( pathTo[ t ] == '/' )
			{
				std::string directory = pathTo.substr( 0, t );	//testing
				::CreateDirectory ( directory.c_str(), NULL );
			}
		}
		::CreateDirectory ( pathTo.c_str(), NULL );
	}
}

std::string CVistaFileUtilities::GetSavePathFromRelativePath( const std::string &path, EFileLocationPrefs pref )
{
	std::string strPath = "";

	if( path == "" )
	{
		return s_strWorkingPath;
	}

	std::string cleanPath = path;
	// flip all slashes
	if( path.find( "\\" ) == 0 )
	{
		cleanPath = path.substr( 1, path.length() );
	}

	CSaveFile* pSaveFile  = NULL;

	switch( pref )
	{
	case KFileLocation_SharedCache:
		// For automatic mode, we should first check in a shared location
		// TODO: Figure out how to access a shared location
		// Next, we check the users's profile location
		try
		{
			pSaveFile = new CSaveFile( cleanPath, KTypeCache );
			HANDLE hSaveFile = pSaveFile->Open( GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, OPEN_ALWAYS );
			if( INVALID_HANDLE_VALUE == hSaveFile )
			{
				// This is a bad thing... We've failed to open the file...
				strPath = CSaveFile::GetRootPath( KTypeCache ) + "\\" + cleanPath;
			}
			else
			{
				// We opened (perhaps created) the file.  Set permissions
				strPath = pSaveFile->GetPath();
				CloseHandle( hSaveFile );
			}
		}
		catch( ... )
		{
			// This is a bad thing... We've failed to open the file...
			strPath = CSaveFile::GetRootPath( KTypeCache ) + "\\" + cleanPath;
		}
		break;
	case KFileLocation_Automatic:
	case KFileLocation_Shared:
	case KFileLocation_Profile:
	default:
		// For automatic mode, we should first check in a shared location
		// TODO: Figure out how to access a shared location
		// Next, we check the users's profile location
		try
		{
			pSaveFile = new CSaveFile( cleanPath, KTypePersistent );
			HANDLE hSaveFile = pSaveFile->Open( GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, OPEN_ALWAYS );
			if( INVALID_HANDLE_VALUE == hSaveFile )
			{
				// This is a bad thing... We've failed to open the file...
				strPath = CSaveFile::GetRootPath( KTypePersistent ) + "\\" + cleanPath;
			}
			else
			{
				// We opened (perhaps created) the file.  Set permissions
				strPath = pSaveFile->GetPath();
				CloseHandle( hSaveFile );
			}
		}
		catch( ... )
		{
			// This is a bad thing... We've failed to open the file...
			strPath = CSaveFile::GetRootPath( KTypePersistent ) + "\\" + cleanPath;
		}

		break;
	}

	if( pSaveFile )
		delete pSaveFile;

	strPath = CleanFilePath( strPath );
	//create directories
	std::string pathTo = strPath;
	if( pathTo.find( "." ) != std::string::npos )
		pathTo = pathTo.substr( 0, pathTo.find_last_of( "/" ) );
	
	if( (GetFileAttributes(pathTo.c_str()) | FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY )
	{
		for(uint32 t = 1; t < (uint32)pathTo.size(); t++ )//testing
		{
			if( pathTo[ t ] == '/' )
			{
				std::string directory = pathTo.substr( 0, t );	//testing
				::CreateDirectory( directory.c_str(), NULL );
			}
		}

		::CreateDirectory( pathTo.c_str(), NULL );
	}


#ifdef _MYTEST
	FILE *file = fopen( strPath.c_str(), "ab" );
	if( file )
	{
		fclose( file );
		FILE *f = fopen( "z:\\test.txt", "at" );
		if( f )
		{	
			fprintf( f, "saving file: %s\n", strPath.c_str() );		
			fclose( f );
		}
	}
#endif

	return strPath;
}

std::string CVistaFileUtilities::CleanFilePath( const std::string& Path )	// path to clean
{
	std::string NewPath = Path;

	// flip all slashes
	while( NewPath.find( "\\" ) != -1 )
	{
		int32 Index = (int32) NewPath.find( "\\" );
		NewPath.replace( Index, 1, "/" );
	}

	// and remove duplicates
	while( NewPath.find( "//" ) != -1 )
	{
		int32 Index = (int32) NewPath.find( "//" );
		NewPath.replace( Index, 2, "/" );
	}

	return NewPath;
} // std::string CleanPath()
