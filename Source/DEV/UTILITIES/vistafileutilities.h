#pragma once
#include <string>

enum EFileLocationPrefs
{
	KFileLocation_Automatic = 0,
	KFileLocation_Shared,
	KFileLocation_Profile,
	KFileLocation_Application,
	KFileLocation_SharedCache
};

class CVistaFileUtilities
{
public:

static const std::string GetWorkingPath();
static void SetWorkingPath( const std::string &path );
static const std::string GetSharedDataPath( EFileLocationPrefs pref = KFileLocation_Automatic );

static std::string CleanFilePath( const std::string &path );

static void CreateTempFile( const std::string &path );
static std::string GetLoadPathFromRelativePath( const std::string &path, EFileLocationPrefs pref = KFileLocation_Automatic );
static std::string GetSavePathFromRelativePath( const std::string &path, EFileLocationPrefs pref = KFileLocation_Automatic );


private:

static const std::string FindWorkingPath();

// PRIVATE VARIABLES
static std::string s_strWorkingPath;

};
