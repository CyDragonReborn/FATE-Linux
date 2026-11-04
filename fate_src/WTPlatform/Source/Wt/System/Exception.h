/*
	Exception.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/Object.h"
#include "WT/System/Logging/Log.h"

//Throw an exception of the specified type with the specified descriptive message.
//For example: if (x < 0) WT_THROW( WT::System::CArgumentOutOfRangeException, "x" ). The 
//exception will be logged, if appropriate. 
#define WT_THROW( ExceptionClass, Message ) { \
		ExceptionClass Exception( Message, __FILE__, __LINE__, __FUNCTION__ ); \
		Exception.Log(); \
		throw Exception; \
	}

namespace WT
{
namespace System
{

class CException : public CObject, public std::exception
{
public:

	WT_CLASS( CException , CObject )

	//Get and set details about whether and how exceptions are logged via CException::Log().
	//Note that exceptions are logged only if WTLOG_ENABLED is defined.
	static bool WTCall GetLogEnabled();
	static WT::System::Logging::ELogColor::Enum WTCall GetLogColor();
	static WT::System::Logging::ELogStyle::Enum WTCall GetLogStyle();
	static void WTCall SetLogEnabled( bool LogEnabled );
	static void WTCall SetLogColor( WT::System::Logging::ELogColor::Enum LogColor );
	static void WTCall SetLogStyle( WT::System::Logging::ELogStyle::Enum LogStyle );

	//
	//Constructors
	//
	explicit CException( std::string const& Message = "unknown", 
		std::string const& File = "unknown", 
		int32 Line = -1, std::string const& Function = "unknown" );

	//Destructor
	virtual ~CException();

	//Log the exception, including details such as the exception type, a descriptive 
	//message, and the file, line number, and function where the original exception 
	//object was constructed. Logging is contingent on the WTLOG_ENABLED preprocessor 
	//definition and a true result (the default) from CException::GetLogEnabled().
	void Log();

	// Get the message that describes the current exception
	std::string const& GetMessage() const {return m_Message;}
	void SetMessage( std::string const& Message ) {m_Message = Message;}

	// std::exception method
	virtual const char* what() const;

	// Get debug information about the exception.
	virtual std::string ToString();

private:
	static bool gm_LogEnabled;
	static WT::System::Logging::ELogColor::Enum gm_LogColor;
	static WT::System::Logging::ELogStyle::Enum gm_LogStyle;

	std::string m_Message;
	std::string m_File;
	int32		m_Line;
	std::string m_Function;
};

/*
Helper macro for defining exception classes that require no additional methods or fields.
This is a very common way to declare our basic exceptions.  For example, the
all of the IO exceptions are based off of a system exception; there are then
standard exceptions defined:
<pre>
<code>
WT_DECLARE_EXCEPTION_CLASS( CIOException, CSystemException, "IO Exception" )


WT_DECLARE_EXCEPTION_CLASS( CDirectoryNotFoundException,	CIOException, "Directory not found" )
WT_DECLARE_EXCEPTION_CLASS( CFileNotFoundException,			CIOException, "File not found" )
WT_DECLARE_EXCEPTION_CLASS( CPathTooLongException,			CIOException, "Path too long" )
WT_DECLARE_EXCEPTION_CLASS( CUnexpectedEndOfDataException,	CIOException, "End of stream encountered when more data was expected" )

</code>
</pre>

Working code can then throw these exceptions directly:
<pre>
</code>
	if( Path.length() > MAX_PATH )
	{
		throw CPathTooLongException();
	}
</code>
</pre>
*/

// Helper macros for defining exception classes in piece-meal fashion
#define WT_BEGIN_EXCEPTION_CLASS(ClassName, BaseClass) \
class ClassName : public BaseClass { \
public: 

#define WT_DECLARE_STANDARD_EXCEPTION_FUNCTIONS(ClassName, BaseClass, DefaultMessage) \
	WT_CLASS( ClassName, BaseClass ) \
	explicit ClassName( std::string const& Message = DefaultMessage, \
		std::string const& File = "unknown", \
		int32 Line = -1, std::string const& Function = "unknown" ) : BaseClass( Message, File, Line, Function ) {}

#define WT_END_EXCEPTION_CLASS()	};

#define WT_DECLARE_EXCEPTION_CLASS( ClassName, BaseClass, DefaultMessage ) \
	WT_BEGIN_EXCEPTION_CLASS( ClassName, BaseClass ) \
	WT_DECLARE_STANDARD_EXCEPTION_FUNCTIONS( ClassName, BaseClass, DefaultMessage ) \
	WT_END_EXCEPTION_CLASS()

}	// System
}	// WT
