/*
	utilities.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/


#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <windows.h>
#include <queue>
#include <string>
#include <sstream>
#include <tchar.h>

#include "constants.h"

// commands which may be executed on the game timer
enum ETimerCommand
{
	KTimerReset,
	KTimerStart,
	KTimerStop,
	KTimerAdvance,
	KTimerGetAbsoluteTime,
	KTimerGetGameTime,
	KTimerGetElapsedTime
};

// the game timer allows you to start, stop, and query a generic timer.
// very nice way to get elapsed time between frames -

// usage :
//
//	Game_Timer( KTimerStart );
//	float32 TimeElapsed = Game_Timer( KTimerGetElapsedTime );
//  and that's really it - just request elapsed time each frame after it's started,
//  and that's all there is to it.
float64 __stdcall	Game_Timer( ETimerCommand command );


inline std::string StringUpper( std::string String )
{

	// upper case each character
	for ( uint32 i = 0; i < String.length(); i++ )
	{

		String[i] = toupper( String[i] );
	}

	// return the new string
	return String;
}

inline std::string IntegerToString( int32 Integer ) // convert int to string
{
	std::stringstream	Stream;
	Stream << Integer;
	return Stream.str();
}

inline std::string FloatToString( float32 FloatVal ) // convert int to string
{
	std::stringstream	Stream;
	Stream << FloatVal;
	return Stream.str();
}

inline float32 sgn( float32 FloatVal )	// value to determine the sign of
{
	if( FloatVal < 0 )
	{
		return -1.0f;
	}
	else
	{
		return 1.0f;
	}
}

// make sure that a float is a valid number
float32			VerifyFloat( float32 Value,		// value to test for validity
							 float32 Maximum );// maximum value that the float may be

// make sure that a double is a valid number
float64			VerifyDouble( float64 Value,		// value to test for validity
							  float64 Maximum );// maximum value that the float may be

// get the desktop color depth
uint32			GetDesktopBPP( HWND hWnd );

void			TokenizeString( std::queue<std::string>* pList,		// queue to fill with tokens
								std::string Line,					// string to tokenize
								const int8* Delimiters,				// delimiters ... i.e. ",.:/"
								int8 CommentCharacter );			// comment line character '#' is default



void			ClearQueue( std::queue<std::string >& m_tokens );		// queue to clear



// read a bool from a binary file
bool			ReadBool( FILE* pFile );		// pointer to binary file to read from

// read an int from a binary file
int32			ReadInt( FILE* pFile );			// pointer to binary file to read from

// read a short from a binary file
int16			ReadShort( FILE* pFile );		// pointer to binary file to read from

// read a float from a binary file
float32			ReadFloat( FILE* pFile );		// pointer to binary file to read from

// read a byte from a binary file
int32			ReadByte( FILE* pFile );		// pointer to binary file to read from

// retrieve a string from the windows clipboard
std::string		CopyFromClipboard();

// create a directory at the specified path
bool			CreateLocalDirectory( const std::string& Path );	// path to create

// verify that a file exists at the given location
bool			FileExists( const std::string& FileName );			// filename to check

// assemble a valid absolute path given a base executable path and a local path
std::string AssembleAbsolutePath( const std::string& BasePath,		// base path of executable
								  const std::string& LocalPath );	// local path to assemble

std::string RemoveFileName( const std::string& Path );	// path to remove filename from (keeps trailing slash)

std::string	CleanPath(  const std::string& Path );	// path to clean slashes for

std::string GetFileName( const std::string& Path );	// path to get filename from (keeps trailing slash)

// read a string from a binary file of the specified length
std::string		ReadString( FILE* pFILE,		// pointer to binary file to read from
							uint32 Length );	// length of string to read

// read a string from a binary file until reaching a terminating character
std::string		ReadString( FILE* pFILE );		// pointer to binary file to read from

// get the location of a system folder
bool GetSpecialFolder( HWND hwndParent,			// hwnd
					   INT Folder,				// id of folder we want to get
					   LPTSTR Buffer );			// string to fill with the folder's path

// retrieve the local path to the executable;
std::string	GetLocalPath( void );

float32 DecompressNormal( int32 AU, 
						  int32 AV, 
						  uint32 Component );

float32 RandomBetween( float32 Minimum,		// minimum range
					   float32 Maximum );	// maximum range

int32 RandomIntegerBetween( int32 Minimum,		// minimum range
							int32 Maximum );	// maximum range

std::string RomanNumeral( int32 Value );		// value to convert
#endif
