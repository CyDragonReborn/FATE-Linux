/***************************************************************************************
	Log.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved 

	Description:  Logging system. The WordPad code is written by Dennis Clark and
	transported to WTPlatform.

	History:
		2004		Dennis Clark	Created
		10/06/2005	Rui Jiang		Rewritten
***************************************************************************************/

#include "WT\stdafx.h"

#include "Log.h"
#include <shlobj.h>
#include "WT/System/Threading.h"
#include "WT/System/Util/WinUtil.h"

using namespace std;

namespace WT
{
namespace System
{
namespace Logging
{

void OutputLogMsg(const std::string& FileName, int LineNumber, const std::string& FunctionName, 
				  int LogLevel, ELogColor::Enum LogColor, ELogStyle::Enum LogStyle, std::string sFmt, va_list ptr);

void __cdecl CLogTraceInfo::operator()(std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, LOGDEFAULT_LEVEL, m_Color, m_Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(int Level, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, Level, m_Color, m_Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(ELogColor::Enum Color, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, LOGDEFAULT_LEVEL, Color, m_Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(ELogStyle::Enum Style, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, LOGDEFAULT_LEVEL, m_Color, Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(int Level, ELogColor::Enum Color, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, Level, Color, m_Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(int Level, ELogStyle::Enum Style, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, Level, m_Color, Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(int Level, ELogColor::Enum Color, ELogStyle::Enum Style, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, Level, Color, Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

void __cdecl CLogTraceInfo::operator()(ELogColor::Enum Color, ELogStyle::Enum Style, std::string sFmt, ...) const
{
	va_list ptr; va_start(ptr, sFmt);
	OutputLogMsg( m_FileName, m_LineNo, m_FunctionName, LOGDEFAULT_LEVEL, Color, Style, sFmt.c_str(), ptr );
	va_end(ptr);
}

//*************************************************************************************************
// Well, you can send text to WordPad, if you know some magic details, which these functions take
// care of for us.
//
static HWND const FindWordpadHWndFrame( std::string const &filename )
{
	static char const kszClassName[]   = "WordPadClass";
	static char const kszTitleSuffix[] = " - WordPad";

	std::string winTitle = filename + kszTitleSuffix;
	HWND const hWndMain = ::FindWindow( kszClassName, winTitle.c_str() );

	if( hWndMain != NULL )
	{
		// The edit window is the last child of WordPad's frame window. The frame window is
		// the mainwindow's first child

		HWND const hWndFrame = ::GetWindow( hWndMain, GW_CHILD );
		return hWndFrame;
	}

	return NULL; // There isn't a WordPad instance open to the given file
}

//-------------------------------------------------------------------------------------------------
// The rich-edit window happens to be the frame window's last child
//
static HWND const FindWordpadHwndEdit( HWND const hWndFrame )
{
	assert( hWndFrame != NULL );
		
	HWND const hWndEdit = ::GetWindow( hWndFrame, GW_HWNDLAST );

	return hWndEdit;
}

//*************************************************************************************************
// values collected from WordPad's resource.h
//
class CColorTableEntry
{
public:
	CColorTableEntry( ELogColor::Enum const color, WPARAM const wParam ):
	m_color ( color  ),
	m_wParam( wParam )
	{
	}

	ELogColor::Enum const GetColor ( void ) const { return m_color;  }
	WPARAM    const GetWParam( void ) const { return m_wParam; }

private:
   ELogColor::Enum  m_color;
   WPARAM     m_wParam;
};

//------------------------------------------------------------------------------------------------
// Color constants are from the Wordpad source code
//
static CColorTableEntry const kColorIdTable[ELogColor::Count] =
{
	CColorTableEntry( ELogColor::Black    , 32782 ),
	CColorTableEntry( ELogColor::Maroon   , 32783 ),
	CColorTableEntry( ELogColor::Green    , 32784 ),
	CColorTableEntry( ELogColor::Olive    , 32785 ),
	CColorTableEntry( ELogColor::Navy     , 32786 ),
	CColorTableEntry( ELogColor::Purple   , 32787 ),
	CColorTableEntry( ELogColor::Teal     , 32788 ),
	CColorTableEntry( ELogColor::Gray     , 32789 ),
	CColorTableEntry( ELogColor::Silver   , 32790 ),
	CColorTableEntry( ELogColor::Red      , 32791 ),
	CColorTableEntry( ELogColor::Lime     , 32792 ),
	CColorTableEntry( ELogColor::Yellow   , 32793 ),
	CColorTableEntry( ELogColor::Blue     , 32794 ),
	CColorTableEntry( ELogColor::Fuchsia  , 32795 ),
	CColorTableEntry( ELogColor::Aqua     , 32796 ),
	CColorTableEntry( ELogColor::White    , 32797 ),
	CColorTableEntry( ELogColor::Automatic, 32798 ),
};

//------------------------------------------------------------------------------------------------
// These are from the Wordpad source code
//
static WPARAM const ID_CHAR_BOLD      = 32799;
static WPARAM const ID_CHAR_ITALIC    = 32800;
static WPARAM const ID_CHAR_COLOR     = 32801;
static WPARAM const ID_CHAR_UNDERLINE = 32802;

//*************************************************************************************************
static void ApplyStylesToWordpad( HWND const hWndFrame, DWORD styles )
{  
	assert( hWndFrame != NULL );
	if (styles & ELogStyle::Bold)		::SendMessage( hWndFrame, WM_COMMAND, ID_CHAR_BOLD     , LPARAM(0) ); 
	if (styles & ELogStyle::Italic)	::SendMessage( hWndFrame, WM_COMMAND, ID_CHAR_ITALIC   , LPARAM(0) ); 
	if (styles & ELogStyle::Underline)	::SendMessage( hWndFrame, WM_COMMAND, ID_CHAR_UNDERLINE, LPARAM(0) ); 
}

//*************************************************************************************************
static void ApplyColorToWordpad( HWND const hWndFrame, ELogColor::Enum const color )
{  
	assert( color >= ELogColor::FirstValid );
	assert( color <= ELogColor::LastValid  );
	assert( hWndFrame != NULL );

	WPARAM const wParam = kColorIdTable[color].GetWParam();
	::SendMessage( hWndFrame, WM_COMMAND, wParam, LPARAM(0) );
}

class CLog
{
public:
	static CLog& GetInstance()				{static CLog g_Log; return g_Log;}
	DWORD	GetLogOptions()					{return m_LogOptions;}
	void	SetLogOptions(DWORD NewOptions) {m_LogOptions = NewOptions;}

	int		GetLogLevel()					{return m_LogLevel;}
	void	SetLogLevel(int NewOptions)		{m_LogLevel = NewOptions;}
	
	bool			LogWordPad()			{return m_LogWordPad;}
	const string&	GetWordPadDocName()		{return m_WordPadDocName;}
	void			SetLogWordPad(bool Log, const string& DocName) { m_LogWordPad = Log, m_WordPadDocName = DocName; }

	CCriticalSection& GetCriticalSection() {return m_CriticalSection;}

protected:
	DWORD				m_LogOptions;
	bool				m_LogWordPad;
	string				m_WordPadDocName;
	int					m_LogLevel;
	CCriticalSection	m_CriticalSection;

private:
	CLog()
	{
		m_LogOptions = LOGDEFAULT_OPTION;
		m_LogWordPad = false;
		m_LogLevel = 99999999;
	}
};

void OutputMsgToWordPad(const string& DocName, ELogColor::Enum LogColor, DWORD LogStyle, const string& Msg)
{
	HWND const hWndFrame = FindWordpadHWndFrame( DocName );
	if ( hWndFrame != NULL )
	{
		// Change styles by sending a message to WordPad's frame window
		ApplyStylesToWordpad( hWndFrame, LogStyle );
		ApplyColorToWordpad ( hWndFrame, LogColor );

		// Add logging text by sending a message to WordPad's rich-edit window
		HWND const hWndEdit = FindWordpadHwndEdit( hWndFrame );
		::SendMessage( hWndEdit, EM_REPLACESEL, 0, (LPARAM)(Msg.c_str()) );
	}
}

std::string itostr(int n)
{
	char buf[30];
	sprintf(buf, "%d", n);
	return buf;
}

class RAIIfclose
{
public:
	RAIIfclose ():m_f(0)
	{
	}
	~RAIIfclose ()
	{
		End();
	}
	void Init(FILE* f)
	{
		End();
		m_f = f;
	}
	void End ()
	{
		if (m_f)
		{
			fclose (m_f);
		}
		m_f = 0;
	}

private:
	FILE* m_f;
};
static FILE* g_logf = 0;
static RAIIfclose g_autoclose_logf;

template <typename T> class CAutoSet
{
public:
	CAutoSet( T& Obj, T const& TempValue )
		: m_Obj( Obj )
		, m_OrigValue( m_Obj )
	{
		m_Obj = TempValue;
	}

	~CAutoSet()
	{
		m_Obj = m_OrigValue;
	}
private:
	T& m_Obj;
	T m_OrigValue;
	CAutoSet& operator=( CAutoSet const& ); //Unimplemented
};

void OutputLogMsg(const std::string& FileName, int LineNumber, const std::string& FunctionName, 
				  int LogLevel, ELogColor::Enum LogColor, ELogStyle::Enum LogStyle, std::string sFmt, va_list ptr)
{
	if (LogLevel > CLog::GetInstance().GetLogLevel())
	{
		return;
	}

	static bool g_LogEnabled = true;
	if (!g_LogEnabled)
	{
		return;
	}

	//Serialize output
	CAutoClassLock<CCriticalSection>( &CLog::GetInstance().GetCriticalSection() );
	
	//Disable recursive logging
	CAutoSet<bool> AutoSet( g_LogEnabled, false );

	string s;

	DWORD const LogOptions = CLog::GetInstance().GetLogOptions();

	if (LogOptions & ELogOption::LogLevel)
		s += string("L") + itostr(LogLevel) + " ";
	if (LogOptions & ELogOption::LogFile)
		s += FileName + " ";
	if (LogOptions & ELogOption::LogLine)
		s += itostr(LineNumber) + " ";
	if (LogOptions & ELogOption::LogFunction)
		s += FunctionName + ": ";

	const int MaxBufferSize = 2024;
	char buf[MaxBufferSize];
	_vsnprintf(buf, MaxBufferSize, sFmt.c_str(), ptr);
	buf[MaxBufferSize - 1] = 0;
	s += buf;
	s += "\n";

	if (LogOptions & ELogOption::LogDebugger)
	{
		OutputDebugString(s.c_str());
	}

	if (LogOptions & ELogOption::LogToFile)
	{
		if (!g_logf)
		{
			std::string FilePath = Util::GetCommonAppDataPath();
			FilePath += "\\WildTangent\\wtplatformlog.txt";

			g_logf = fopen( FilePath.c_str(), "a" );
			g_autoclose_logf.Init (g_logf);
		}

		if (g_logf)
		{
			fputs (s.c_str(), g_logf);
			//fflush (g_logf);
		}


	}

	if (CLog::GetInstance().LogWordPad())
	{
		OutputMsgToWordPad(CLog::GetInstance().GetWordPadDocName(), LogColor, LogStyle, s);
	}
}

void __cdecl SetUseWordPad(bool UseWordPad, const std::string& DocName)
{
	CLog::GetInstance().SetLogWordPad(UseWordPad, DocName);
}

void __cdecl SetFilterLevel(int NewLevel)
{
	CLog::GetInstance().SetLogLevel(NewLevel);
}

int __cdecl GetOptions()
{
	return CLog::GetInstance().GetLogOptions();
}

void __cdecl SetOptions( unsigned int Options)
{
	CLog::GetInstance().SetLogOptions(Options);
}

}
}
}
