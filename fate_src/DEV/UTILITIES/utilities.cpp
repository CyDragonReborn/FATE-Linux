/*
	utilities.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/
// disable the STL truncation warning
#pragma warning(disable: 4786) 

#include <shlobj.h>
#include <math.h>
#include <mmsystem.h>
// note that you must link in winmm.lib to get timer functionality

#include <windows.h>
#include <assert.h>
#include <queue>
#include <string>
#include <tchar.h>
#include <float.h>

using namespace std;

#include "utilities.h"

// ensure that a float is a real number
float32 VerifyFloat( float32 Value,		// value to test for validity
					 float32 Maximum )	// maximum value that the float may be
{
	if ( _isnan( Value ) )
	{
		Value = 0;
	}
	if ( !_finite( Value) )
	{
		Value = Maximum;
	}

	return Value;
} // VerifyFloat()

// ensure that a double is a real number
float64 VerifyDouble( float64 Value,		// value to test for validity
					  float64 Maximum )	// maximum value that the float may be
{
	if ( _isnan( Value ) )
	{
		Value = 0;
	}
	if ( !_finite( Value ) )
	{
		Value = Maximum;
	}

	return Value;
} // VerifyDouble()

// get the color depth of the desktop
uint32 GetDesktopBPP( HWND hWnd )	// hwnd is needed for device context
{
	uint32	BPP	= 16;
	HDC	hDC	= ::GetDC( hWnd );
	if ( hDC )
	{
		BPP = GetDeviceCaps( hDC, BITSPIXEL );

		::ReleaseDC( hWnd, hDC );
	}
	return BPP;
} // GetDesktopBPP()

// read a short from a binary file
int16 ReadShort( FILE* pFile )			// length of string to read
{
	int16	ShortValue;

	if ( fread( &ShortValue, sizeof( short ), 1, pFile ) != 1 )
	{
		return( kFalse );
	}
	return ShortValue;
} // ReadShort()

// read an int from a binary file
int32 ReadInt( FILE* pFile )			// length of string to read
{
	int32	IntValue;

	if ( fread( &IntValue, sizeof( int ), 1, pFile ) != 1 )
	{
		return( kFalse );
	}
	return IntValue;
} // ReadInt()

// read a float from a binary file
float32 ReadFloat( FILE* pFile )		// length of string to read
{
	float32	FloatValue;

	if ( fread( &FloatValue, sizeof( float ), 1, pFile ) != 1 )
	{
		return( kFalse );
	}

	return FloatValue;
} // ReadFloat()

// read a byte from a binary file
int32 ReadByte( FILE* pFile )			// length of string to read
{
	byte	Byte;
	fread( &Byte, 1, 1, pFile );
	return Byte;

} // ReadByte()

// read a bool from a binary file
bool ReadBool( FILE* pFile )			// length of string to read
{
	bool	Bool;
	fread( &Bool, 1, 1, pFile );
	return Bool;

} // ReadBool()

// read a string from a binary file of the specified length
std::string ReadString( FILE* pFile,	// pointer to binary file to read from
						uint32 Length )	// length of string to read
{
	std::string	Result;
	int8		Character;
	for ( uint32 i = 0; i < Length; i++ )
	{
		fread( &Character, 1, 1, pFile );
		Result = Result + Character;
	}
	return Result;
} // ReadString()

// read a string from a binary file until reaching a terminating character
std::string ReadString( FILE* pFile )	// pointer to binary file to read from		
{
	std::string	Result;
	int8		Character;
	for ( uint32 i = 0; ; i++ )
	{
		fread( &Character, 1, 1, pFile );
		if ( Character == '\0' )
		{
			break;
		}
		Result = Result + Character;
	}
	return Result;
} // ReadString()

void ClearQueue( std::queue<std::string >& pQueue )	// queue to clear
{
	while ( !pQueue.empty() )
	{
		pQueue.pop();
	}
} // ClearQueue()


// game timer - can be started and stopped, and returns elapsed millis from last tick
float64 __stdcall Game_Timer( ETimerCommand Command )	// command to send to the timer
{
	static bool		m_TimerInitialized	= kFalse;
	static bool		m_UsingQPF			= kFalse;
	static bool		m_TimerStopped		= kTrue;
	static LONGLONG	m_QPFTicksPerSec	= 0;

	// Initialize the timer
	if ( !m_TimerInitialized )
	{
		m_TimerInitialized = kTrue;

		// Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
		// not supported, we will timeGetTime() which returns milliseconds.
		LARGE_INTEGER	TicksPerSec;
		m_UsingQPF = QueryPerformanceFrequency( &TicksPerSec );
		if ( m_UsingQPF )
		{
			m_QPFTicksPerSec = TicksPerSec.QuadPart;
		}
	}

	if ( m_UsingQPF )
	{
		static LONGLONG	m_StopTime			= 0;
		static LONGLONG	m_LastElapsedTime	= 0;
		static LONGLONG	m_BaseTime			= 0;
		float64			Time;
		float64			ElapsedTime;
		LARGE_INTEGER	QWTime;

		// Get either the current time or the stop time, depending
		// on whether we're stopped and what Command was sent
		if ( m_StopTime != 0 &&
			 Command != KTimerStart &&
			 Command != KTimerGetAbsoluteTime )
		{
			QWTime.QuadPart = m_StopTime;
		}
		else
		{
			QueryPerformanceCounter( &QWTime );
		}

		// Return the elapsed time
		if ( Command == KTimerGetElapsedTime )
		{
			ElapsedTime = ( float64 )( QWTime.QuadPart - m_LastElapsedTime ) /
						   ( float64 )m_QPFTicksPerSec;

			m_LastElapsedTime = QWTime.QuadPart;
			return ( float64 ) ElapsedTime;
		}

		// Return the current time
		if ( Command == KTimerGetGameTime )
		{
			float64	fAppTime	= ( float64 )
								  ( QWTime.QuadPart - m_BaseTime ) /
								  ( float64 )
								  m_QPFTicksPerSec;
			return ( float64 ) fAppTime;
		}

		// Reset the timer
		if ( Command == KTimerReset )
		{
			m_BaseTime = QWTime.QuadPart;
			m_LastElapsedTime = QWTime.QuadPart;
			m_StopTime = 0;
			m_TimerStopped = kFalse;
			return 0.0f;
		}

		// Start the timer
		if ( Command == KTimerStart )
		{
			if ( m_TimerStopped )
			{
				m_BaseTime += QWTime.QuadPart - m_StopTime;
			}
			m_StopTime = 0;
			m_LastElapsedTime = QWTime.QuadPart;
			m_TimerStopped = kFalse;
			return 0.0f;
		}

		// Stop the timer
		if ( Command == KTimerStop )
		{
			m_StopTime = QWTime.QuadPart;
			m_LastElapsedTime = QWTime.QuadPart;
			m_TimerStopped = kTrue;
			return 0.0f;
		}

		// Advance the timer by 1/10th second
		if ( Command == KTimerAdvance )
		{
			m_StopTime += m_QPFTicksPerSec / 10;
			return 0.0f;
		}

		if ( Command == KTimerGetAbsoluteTime )
		{
			Time = QWTime.QuadPart / ( float64 ) m_QPFTicksPerSec;
			return ( float64 ) Time;
		}

		return -1.0f; // Invalid Command specified
	}
	else
	{
		// Get the time using timeGetTime()
		static float64	m_LastElapsedTime	= 0.0;
		static float64	m_BaseTime			= 0.0;
		static float64	m_StopTime			= 0.0;
		float64			Time;
		float64			ElapsedTime;

		// Get either the current time or the stop time, depending
		// on whether we're stopped and what Command was sent
		if ( m_StopTime != 0.0 &&
			 Command != KTimerStart &&
			 Command != KTimerGetAbsoluteTime )
		{
			Time = m_StopTime;
		}
		else
		{
			Time = timeGetTime() * 0.001;
		}

		// Return the elapsed time
		if ( Command == KTimerGetElapsedTime )
		{
			ElapsedTime = ( float64 ) ( Time - m_LastElapsedTime );
			m_LastElapsedTime = Time;
			return ( float64 ) ElapsedTime;
		}

		// Return the current time
		if ( Command == KTimerGetGameTime )
		{
			return ( float64 ) ( Time - m_BaseTime );
		}

		// Reset the timer
		if ( Command == KTimerReset )
		{
			m_BaseTime = Time;
			m_LastElapsedTime = Time;
			m_StopTime = 0;
			m_TimerStopped = kFalse;
			return 0.0f;
		}

		// Start the timer
		if ( Command == KTimerStart )
		{
			if ( m_TimerStopped )
			{
				m_BaseTime += Time - m_StopTime;
			}
			m_StopTime = 0.0f;
			m_LastElapsedTime = Time;
			m_TimerStopped = kFalse;
			return 0.0f;
		}

		// Stop the timer
		if ( Command == KTimerStop )
		{
			m_StopTime = Time;
			m_LastElapsedTime = Time;
			m_TimerStopped = kTrue;
			return 0.0f;
		}

		// Advance the timer by 1/10th second
		if ( Command == KTimerAdvance )
		{
			m_StopTime += 0.1f;
			return 0.0f;
		}

		if ( Command == KTimerGetAbsoluteTime )
		{
			return Time;
		}

		return -1.0f; // Invalid Command specified
	}
} // Game_Timer()

// copy text from the windows clipboard
std::string CopyFromClipboard()
{
	HANDLE		Handle;
	LPSTR		Text;
	std::string	PasteText	= "";
	if ( !OpenClipboard( NULL ) )
	{
		return PasteText;
	}

	if ( !( Handle = GetClipboardData( CF_TEXT ) ) )
	{
		CloseClipboard();
		return PasteText;
	}

	if ( Text = ( char * ) GlobalLock( Handle ) )
	{
		PasteText = Text;
		GlobalUnlock( Handle );
	}
	else
	{
		// this should NEVER happen!
		return PasteText;
	}

	CloseClipboard();

	return PasteText;
} // CopyFromClipboard()



// create a directory at the specified absolute path
bool CreateLocalDirectory( const std::string& Path )	// path to create
{
	std::string	SubDirectory	= Path;

	uint32			LastPos	= 0;
	uint32			Pos		= Path.find( '\\', 1 );    
	while ( Pos != -1 )
	{
		SubDirectory = Path.substr( 0, Pos );
		//if(! CreateLocalDirectory( SubDirectory )->Exists() )
		//{
		if ( !CreateDirectory( SubDirectory.c_str(), NULL ) &&
			 GetLastError() != ERROR_ALREADY_EXISTS )
		{
			return kFalse;
		}
		//}
		LastPos = Pos + 1;
		Pos = Path.find( '\\', LastPos );
	}

	if ( !::CreateDirectory( Path.c_str(), NULL ) &&
		 GetLastError() != ERROR_ALREADY_EXISTS )
	{
		return kFalse;
	}

	return S_OK;
} // CreateLocalDirectory()

// verify that a file exists at the specified absolute path
bool FileExists( const std::string& FileName )			// file name to check
{
	FILE*	pFile;
	pFile = fopen( FileName.c_str(), "rb" );

	if ( pFile )
	{
		fclose( pFile );
		return kTrue;
	}
	else
	{
		return kFalse;
	}

} // FileExists()


// assemble a valid absolute path given a base executable path and a local path
std::string AssembleAbsolutePath( const std::string& BasePath,		// base path of executable
								  const std::string& LocalPath )	// local path to assemble
{
	std::string ReturnPath = LocalPath;
	if ( LocalPath.length() > 0 )
	{
		// see if there's a ':' in the path
		int32	Index	= LocalPath.rfind( ":" );

		// if the file doesn't exist at its current path, and it has no ':' in it
		if ( !FileExists( LocalPath ) && Index <= 0 )
		{
			// prepend the base executable path
			ReturnPath = BasePath + LocalPath;
		}
	}
	return ReturnPath;
} // AssembleAbsolutePath()

// tokenize a string into a queue of tokens

void TokenizeString( std::queue<std::string>* pList, // queue to fill with tokens
					 std::string Line,				 // string to tokenize
					 const int8* Delimiters,		 // delimiters ... i.e. ",.:/"
					 int8 CommentCharacter )		 // comment line character '#' is default
{
	uint32 CommentLocation = Line.find( CommentCharacter, 0 );
	if ( CommentLocation != Line.npos )
	{
		Line = Line.erase( CommentLocation );
	}

	uint32 TokenStart = Line.find_first_not_of( ( int8 * ) Delimiters, 0 );
	uint32 TokenEnd;
	while ( TokenStart != Line.npos )
	{
		TokenEnd = Line.find_first_of( ( int8 * ) Delimiters, TokenStart );
		string	Token	= Line.substr( TokenStart, TokenEnd - TokenStart );
		pList->push( Token );

		TokenStart = Line.find_first_not_of( ( int8 * ) Delimiters, TokenEnd );
	}
} // TokenizeString()

bool GetSpecialFolder( HWND hwndParent,			// hwnd
					   INT Folder,				// id of folder we want to get
					   LPTSTR Buffer )			// string to fill with the folder's path
{

	LPITEMIDLIST	pIDL	= NULL;
	bool			OK		= ( ::SHGetSpecialFolderLocation( hwndParent,
															  Folder,
															  &pIDL ) == NOERROR );

	if ( OK )
	{
		OK = ::SHGetPathFromIDList( pIDL, Buffer ) ? kTrue : kFalse;
	}

	// Free up memory allocated by SHGetSpecialFolderLocation
	if ( pIDL != NULL )
	{
		LPMALLOC	pMalloc	= NULL;

		if ( ( ::SHGetMalloc( &pMalloc ) == NOERROR ) && pMalloc != NULL )
		{
			pMalloc->Free( pIDL );
			pMalloc->Release();
		}
	}

	return OK;

} // GetSpecialFolder()

std::string RemoveFileName( const std::string& Path )	// path to remove filename from (keeps trailing slash)
{
	std::string NewPath = Path;

	int32	LastSlash	= Path.rfind( "/" );
	if( LastSlash >= 0 )
	{
		NewPath = Path.substr( 0, LastSlash + 1 );
	}

	// perform the same check for the other slashes, just in case
	if ( NewPath.length() == Path.length() )
	{
		LastSlash	= Path.rfind( "\\" );
		if( LastSlash >= 0 )
		{
			NewPath = Path.substr( 0, LastSlash + 1 );
		}
	}
	return NewPath;
} // std::string RemoveFileName()

std::string CleanPath( const std::string& Path )	// path to remove filename from (keeps trailing slash)
{
	std::string NewPath = Path;

	// flip all slashes
	while( NewPath.find( "\\" ) != -1 )
	{
		int32 Index = NewPath.find( "\\" );
		NewPath.replace( Index, 1, "/" );
	}

	// and remove duplicates
	while( NewPath.find( "//" ) != -1 )
	{
		int32 Index = NewPath.find( "//" );
		NewPath.replace( Index, 2, "/" );
	}

	return NewPath;
} // std::string CleanPath()

std::string GetFileName( const std::string& Path )	// path to get filename from (keeps trailing slash)
{
	std::string NewPath = Path;

	int32	LastSlash	= Path.rfind( "/" );
	if( LastSlash >= 0 )
	{
		NewPath = Path.substr( LastSlash + 1, Path.length() - LastSlash );
	}

	// perform the same check for the other slashes, just in case
	if ( NewPath.length() == Path.length() )
	{
		LastSlash	= Path.rfind( "\\" );
		if( LastSlash >= 0 )
		{
			NewPath = Path.substr( LastSlash + 1, Path.length() - LastSlash );
		}
	}
	return NewPath;
} // std::string GetFileName()

// retrieve the local path to the executable;
std::string	GetLocalPath( void )
{
	std::string LocalPath = "";

	int8*	pStr, szPath[_MAX_PATH]; 
	GetModuleFileName( NULL, szPath, _MAX_PATH ); 
	pStr = strrchr( szPath, '\\' ); 
	if ( pStr != NULL )
	{
		*( ++pStr ) = '\0';
	}
	LocalPath = szPath;

	return LocalPath;
} // GetLocalPath()

float32 DecompressNormal( int32 AU, 
						  int32 AV, 
						  uint32 Component )
{
	float64 Multiplier = 3.14159 / 255.0;
	float64 Alpha, Beta;

	Alpha = AU * 2.0 * Multiplier;
	Beta = AV * 2.0 * Multiplier;

	switch( Component )
	{
	case 0:
		return (float32) (cos(Beta) * sin(Alpha));
	case 1:
		return (float32) (sin(Beta) * sin(Alpha));
	case 2:
		return (float32) cos(Alpha);
	}
	return 0;
} // DecompressNormal()

float32 RandomBetween( float32 Minimum,		// minimum range
					   float32 Maximum )	// maximum range
{
	if( Maximum == Minimum )
	{
		return Minimum;
	}
	float32 val =  Minimum + (float32)rand() / RAND_MAX  * ( Maximum - Minimum );
	return val;
} // RandomBetween()

int32 RandomIntegerBetween( int32 Minimum,	// minimum range
							int32 Maximum )	// maximum range
{
	if( Maximum <= Minimum )
	{
		return Minimum;
	}
	int32 val =  Minimum + rand() % ( Maximum - Minimum + 1 );
	if( val > Maximum )
	{
		val = Maximum;
	}
	return val;
} // RandomBetween()


std::string RomanNumeral( int32 Value )		// value to convert
{
	std::string Roman;

	while( Value >= 1000 )
	{
		Value -= 1000;
		Roman = Roman + "M";
	}
	if ( Value >= 900 )
	{
		Value-=900;
		Roman = Roman + "CM";
	}
	if( Value >= 500 )
	{
		Value -= 500;
		Roman = Roman + "D";
	}
	if ( Value >= 400 )
	{
		Value -= 400;
		Roman = Roman + "CD";
	}
	while( Value >= 100 )
	{
		Value = Value-100;
		Roman = Roman + "C";
	}
	if( Value >= 90 )
	{
		Value -= 90;
		Roman = Roman + "XC";
	}
	if( Value >= 50 )
	{
		Value -= 50;
		Roman = Roman + "L";
	}
	if( Value >= 40 )
	{
		Value -= 40;
		Roman = Roman + "XL";
	}
	while( Value >= 10 )
	{
		Value -= 10;
		Roman = Roman + "X";
	}
	if( Value == 9 )
	{
		Roman = Roman + "IX";
	}
	else
	{
		if( Value >= 5 )
		{
			Value -= 5;
			Roman = Roman + "V";
		}
		if( Value == 4 )
		{
			Roman = Roman + "IV";
		}
		else
		{
			while( Value > 0 )
			{
				Value -= 1;
				Roman = Roman + "I";
			}
		}
	}

	return Roman;
} // RomanNumeral()

