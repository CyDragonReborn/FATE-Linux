/***************************************************************************************
	Log.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved 

	Description:  Logging system. The WordPad code is written by Dennis Clark and
	transported to WTPlatform.

	History:
		2004		Dennis Clark	Created
		10/06/2005	Rui Jiang		Rewritten with simplification.

To enable logging, you need to define WTLOG_ENABLED either at the project level or 
before including the header. 

The macros in the header provides an easy way to log to Debug output, and/or to a opened
wordpad file. To log to wordpad, you need to add something like:
	
	LOG_SET_WORDPAD(true, "log.doc");

at the beginning of your program. Then when you are debugging, open or create a new 
file called "log.doc" by WordPad. You will see logs will be inserted into the WordPad.

Each log has several properties:
	1. Level. Usually describe how important this message is. 0 is the most important. 
		and 1 is less important. You can set a filter level so that all logs less than
		the filter level will be discarded. see: LOG_SET_FILTERLEVEL.
	2. Color. Only useful for wordpad logging. By default it is black.
	3. Style. Only useful for wordpad logging. By default it is normal font.

You can also choose whether you want to log source information like source file name,
line number, function name etc by LOG_SET_OPTIONS.

The one log macro LOG is defined as creating a temporary object, that has overloaded 
member functions so that it is easier to specify logging styles. Following overloads 
are available:

	void __cdecl operator()(std::string sFmt, ...) const;
	void __cdecl operator()(int Level, std::string sFmt, ...) const;
	void __cdecl operator()(ELogColor::Enum Color, std::string sFmt, ...) const;
	void __cdecl operator()(ELogStyle::Enum Style, std::string sFmt, ...) const;
	void __cdecl operator()(int Level, ELogColor::Enum Color, std::string sFmt, ...) const;
	void __cdecl operator()(int Level, ELogStyle::Enum Style, std::string sFmt, ...) const;
	void __cdecl operator()(ELogColor::Enum Color, ELogStyle::Enum Style, std::string sFmt, ...) const;
	void __cdecl operator()(int Level, ELogColor::Enum Color, ELogStyle::Enum Style, std::string sFmt, ...) const;

So in case you don't care anything, you can just write:
	LOG("Hello, World! %d", 2005)
or you can combine level, colors and styles like:
	LOG(4, ELogColor::Red, ELogStyle::BoldAndUnderline, "Even more fancier message.");

Example:

	LOG_SET_WORDPAD(true, "log.doc");
	LOG_SET_FILTERLEVEL(99);
	LOG_SET_OPTIONS(ELogOption::LogLevel | ELogOption::LogFunction);

	LOG("Hello, World");
	LOG("Hello, World, %d", 2005);

	LOG(ELogColor::Blue, "Blue");
	LOG(ELogStyle::Bold, "Bold");
	LOG(ELogColor::Blue, ELogStyle::Bold, "Blue and Bold");
	LOG(ELogColor::Blue, ELogStyle::All, "Blue and Bold and Italic and Underline");

	LOG(0, "This is a level 0 message");
	LOG(1, "This is a level 1 message");
	LOG(2, "This is a level 2 message");
	LOG(2, ELogColor::Silver, "This is a level 2 message With Silver");
	LOG(3, ELogStyle::BoldAndUnderline, "More fancier message.");
	LOG(4, ELogColor::Red, ELogStyle::BoldAndUnderline, "Even more fancier message.");
	LOG(5, "Back to normal message. %.3f", 0.53434);

***************************************************************************************/
#pragma once

#include <string>

namespace WT
{
namespace System
{
namespace Logging
{

//*************************************************************************************************
// Some log targets, notably wordpad, can support colors
//
namespace ELogColor
{   
	enum Enum
	{
		Black,
		Maroon,
		Green,
		Olive,
		Navy,
		Purple,
		Teal,
		Gray,
		Silver,
		Red,
		Lime,
		Yellow,
		Blue,
		Fuchsia,
		Aqua,
		White,
		Automatic,

		Count,

		FirstValid = 0,
		LastValid  = Count - 1,
		Invalid    = -1
	};
};

//*************************************************************************************************
//  Some log targets, notably wordpad can support font styles
//
namespace ELogStyle
{
	enum Enum
	{
		None = 0x0,
		Bold = 0x1,
		Italic = 0x2,
		Underline = 0x4,
		BoldAndItalic = Bold | Italic,
		BoldAndUnderline = Bold | Underline,
		ItalicAndUnderline = Italic | Underline,
		All = Bold | Italic | Underline,
	};
};

//Verbose level of a log.
namespace ELogLevel
{
	enum Enum
	{
		Level0,	//output under any circumstances, when logging is on.
		Level1,
		Level2
	};
};

namespace ELogOption
{
	enum Enum
	{
		LogLevel		= 1 << 0,
		LogFile			= 1 << 1,
		LogLine			= 1 << 2,
		LogFunction		= 1 << 3,


		LogDebugger		= 1 << 4, //Flag for calling OutputDebugString()
		LogToFile		= 1 << 5,
		LogAll			= (1 << 5) - 1, // not really all!
		LogAllFileOnly	= 0x2f,
	};
};


#define LOGDEFAULT_LEVEL WT::System::Logging::ELogLevel::Level0
#define LOGDEFAULT_COLOR WT::System::Logging::ELogColor::Black
#define LOGDEFAULT_STYLE WT::System::Logging::ELogStyle::None
#define LOGDEFAULT_OPTION WT::System::Logging::ELogOption::LogAll

void __cdecl SetUseWordPad(bool UseWordPad, const std::string& DocName);
void __cdecl SetFilterLevel(int NewLevel);
int __cdecl GetOptions();
void __cdecl SetOptions( unsigned int Options);

class CLogTraceInfo
{
public:
	CLogTraceInfo
		(
			const char *pszFileName,
			int nLineNo,
			const char* pszFunctionName
		)
	{
		m_FileName = pszFileName;
		m_LineNo = nLineNo;
		m_FunctionName = pszFunctionName;
		m_Color = LOGDEFAULT_COLOR;
		m_Style = LOGDEFAULT_STYLE;
	}
	CLogTraceInfo
		(
			const char *pszFileName,
			int nLineNo,
			const char* pszFunctionName,
			ELogColor::Enum Color,
			ELogStyle::Enum Style
		)
	{
		m_FileName = pszFileName;
		m_LineNo = nLineNo;
		m_FunctionName = pszFunctionName;
		m_Color = Color;
		m_Style = Style;
	}

	// Note: sFmt is NOT passed by reference because of the way the compiler needs a stack-addressed
	// argument preceeeding the variable argument list.
	void __cdecl operator()(std::string sFmt, ...) const;
	void __cdecl operator()(int Level, std::string sFmt, ...) const;
	void __cdecl operator()(ELogColor::Enum Color, std::string sFmt, ...) const;
	void __cdecl operator()(ELogStyle::Enum Style, std::string sFmt, ...) const;

	void __cdecl operator()(int Level, ELogColor::Enum Color, std::string sFmt, ...) const;
	void __cdecl operator()(int Level, ELogStyle::Enum Style, std::string sFmt, ...) const;
	void __cdecl operator()(ELogColor::Enum Color, ELogStyle::Enum Style, std::string sFmt, ...) const;

	void __cdecl operator()(int Level, ELogColor::Enum Color, ELogStyle::Enum Style, std::string sFmt, ...) const;
private:
	std::string		m_FileName;
	int				m_LineNo;
	std::string		m_FunctionName;
	ELogColor::Enum m_Color;
	ELogStyle::Enum m_Style;
};

#ifdef WTLOG_ENABLED

//This is the trick so that we can accept variable arguments.
#define LOG WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__)

// These are just some shortcut macros for some of the most common color/style combos that I've used
#define LOG_BOLD           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Black  ,WT::System::Logging::ELogStyle::Bold  )
#define LOG_ITALIC         WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Black  ,WT::System::Logging::ELogStyle::Italic)

#define LOG_BLACK          WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Black  , WT::System::Logging::ELogStyle::None )
#define LOG_MAROON         WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Maroon , WT::System::Logging::ELogStyle::None )
#define LOG_GREEN          WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Green  , WT::System::Logging::ELogStyle::None )
#define LOG_OLIVE          WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Olive  , WT::System::Logging::ELogStyle::None )
#define LOG_NAVY           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Navy   , WT::System::Logging::ELogStyle::None )
#define LOG_PURPLE         WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Purple , WT::System::Logging::ELogStyle::None )
#define LOG_TEAL           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Teal   , WT::System::Logging::ELogStyle::None )
#define LOG_GRAY           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Gray   , WT::System::Logging::ELogStyle::None )
#define LOG_SILVER         WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Silver , WT::System::Logging::ELogStyle::None )
#define LOG_RED            WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Red    , WT::System::Logging::ELogStyle::None )
#define LOG_LIME           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Lime   , WT::System::Logging::ELogStyle::None )
#define LOG_YELLOW         WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Yellow , WT::System::Logging::ELogStyle::None )
#define LOG_BLUE           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Blue   , WT::System::Logging::ELogStyle::None )
#define LOG_FUCHSIA        WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Fuchsia, WT::System::Logging::ELogStyle::None )
#define LOG_AQUA           WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Aqua   , WT::System::Logging::ELogStyle::None )

#define LOG_BOLD_BLACK     WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Black  , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_MAROON    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Maroon , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_GREEN     WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Green  , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_OLIVE     WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Olive  , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_NAVY      WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Navy   , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_PURPLE    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Purple , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_TEAL      WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Teal   , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_GRAY      WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Gray   , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_SILVER    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Silver , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_RED       WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Red    , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_LIME      WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Lime   , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_YELLOW    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Yellow , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_BLUE      WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Blue   , WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_FUCHSIA   WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Fuchsia, WT::System::Logging::ELogStyle::Bold )
#define LOG_BOLD_AQUA      WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Aqua   , WT::System::Logging::ELogStyle::Bold )

#define LOG_ITALIC_BLACK   WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Black  , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_MAROON  WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Maroon , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_GREEN   WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Green  , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_OLIVE   WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Olive  , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_NAVY    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Navy   , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_PURPLE  WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Purple , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_TEAL    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Teal   , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_GRAY    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Gray   , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_SILVER  WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Silver , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_RED     WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Red    , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_LIME    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Lime   , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_YELLOW  WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Yellow , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_BLUE    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Blue   , WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_FUCHSIA WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Fuchsia, WT::System::Logging::ELogStyle::Italic )
#define LOG_ITALIC_AQUA    WT::System::Logging::CLogTraceInfo(__FILE__, __LINE__, __FUNCTION__,WT::System::Logging::ELogColor::Aqua   , WT::System::Logging::ELogStyle::Italic )

//To turn on/off WordPad logging. By default it's off. 
//Example: LOG_SET_WORDPAD(true, "log.doc")
#define LOG_SET_WORDPAD( UseIt, DocName ) WT::System::Logging::SetUseWordPad(UseIt, DocName)

//Set filter level. If the logged level of a message is greater than filter level, then
//the log message will be discarded. By default, the filter level is 99999999.
#define LOG_SET_FILTERLEVEL( Level ) WT::System::Logging::SetFilterLevel(Level)

//Set options as to whether to log level, source file, line number or function name.
//Can be one or more of ELogOption::Enum
//Example: LOG_SET_OPTIONS(ELogOption::LogLevel | ELogOption::LogFunction);
#define LOG_SET_OPTIONS(Options) WT::System::Logging::SetOptions(Options)

#else

#define LOG __noop

#define LOG_BOLD    __noop
#define LOG_ITALIC  __noop

#define LOG_BLACK   __noop
#define LOG_MAROON  __noop
#define LOG_GREEN   __noop
#define LOG_OLIVE   __noop
#define LOG_NAVY    __noop
#define LOG_PURPLE  __noop
#define LOG_TEAL    __noop
#define LOG_GRAY    __noop
#define LOG_SILVER  __noop
#define LOG_RED     __noop
#define LOG_LIME    __noop
#define LOG_YELLOW  __noop
#define LOG_BLUE    __noop
#define LOG_FUCHSIA __noop
#define LOG_AQUA    __noop

#define LOG_BOLD_BLACK   __noop
#define LOG_BOLD_MAROON  __noop
#define LOG_BOLD_GREEN   __noop
#define LOG_BOLD_OLIVE   __noop
#define LOG_BOLD_NAVY    __noop
#define LOG_BOLD_PURPLE  __noop
#define LOG_BOLD_TEAL    __noop
#define LOG_BOLD_GRAY    __noop
#define LOG_BOLD_SILVER  __noop
#define LOG_BOLD_RED     __noop
#define LOG_BOLD_LIME    __noop
#define LOG_BOLD_YELLOW  __noop
#define LOG_BOLD_BLUE    __noop
#define LOG_BOLD_FUCHSIA __noop
#define LOG_BOLD_AQUA    __noop

#define LOG_ITALIC_BLACK   __noop
#define LOG_ITALIC_MAROON  __noop
#define LOG_ITALIC_GREEN   __noop
#define LOG_ITALIC_OLIVE   __noop
#define LOG_ITALIC_NAVY    __noop
#define LOG_ITALIC_PURPLE  __noop
#define LOG_ITALIC_TEAL    __noop
#define LOG_ITALIC_GRAY    __noop
#define LOG_ITALIC_SILVER  __noop
#define LOG_ITALIC_RED     __noop
#define LOG_ITALIC_LIME    __noop
#define LOG_ITALIC_YELLOW  __noop
#define LOG_ITALIC_BLUE    __noop
#define LOG_ITALIC_FUCHSIA __noop
#define LOG_ITALIC_AQUA    __noop

#define LOG_SET_WORDPAD( UseIt, DocName )
#define LOG_SET_FILTERLEVEL( Level )
#define LOG_SET_OPTIONS(Options)

#endif


}
}
}