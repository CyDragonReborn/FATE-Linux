#include "WT/stdafx.h"
#include "WT/System/Exception.h"
#include "WT/System/SystemException.h"
#include "WT/System/IO/IOException.h"

namespace WT
{

namespace System
{
WT_DEFINE_CLASS(CException)
WT_DEFINE_CLASS(CSystemException)
WT_DEFINE_CLASS(CUnauthorizedAccessException)
WT_DEFINE_CLASS(CArgumentException)

namespace IO
{
WT_DEFINE_CLASS(CIOException)
WT_DEFINE_CLASS(CDirectoryNotFoundException)
WT_DEFINE_CLASS(CFileNotFoundException)
WT_DEFINE_CLASS(CPathTooLongException)
} //namespace IO

//
// Define static CException members
//
bool CException::gm_LogEnabled = true;
WT::System::Logging::ELogColor::Enum CException::gm_LogColor = WT::System::Logging::ELogColor::Red;
WT::System::Logging::ELogStyle::Enum CException::gm_LogStyle = WT::System::Logging::ELogStyle::None;

/*static*/ bool WTCall CException::GetLogEnabled()
{
	return gm_LogEnabled;
}

/*static*/ WT::System::Logging::ELogColor::Enum WTCall CException::GetLogColor()
{
	return gm_LogColor;
}

/*static*/ WT::System::Logging::ELogStyle::Enum WTCall CException::GetLogStyle()
{
	return gm_LogStyle;
}

/*static*/ void WTCall CException::SetLogEnabled( bool LogEnabled )
{
	gm_LogEnabled = LogEnabled;
}

/*static*/ void WTCall CException::SetLogColor( WT::System::Logging::ELogColor::Enum LogColor )
{
	gm_LogColor = LogColor;
}

/*static*/ void WTCall CException::SetLogStyle( WT::System::Logging::ELogStyle::Enum LogStyle )
{
	gm_LogStyle = LogStyle;
}

CException::CException( std::string const& Message, std::string const& File, int32 Line, std::string const& Function )
	: m_Message( Message )
	, m_File( File )
	, m_Line( Line )
	, m_Function( Function )
{
}

/*virtual*/ CException::~CException()
{
}

/*virtual*/ const char* CException::what() const
{
	return m_Message.c_str();
}

void CException::Log()
{
#ifdef WTLOG_ENABLED
	if (gm_LogEnabled)
	{
		std::string const Message = ToString();
		WT::System::Logging::CLogTraceInfo( m_File.c_str(), m_Line, m_Function.c_str(), 
			gm_LogColor, gm_LogStyle )( Message );
	}
#endif
}

/*virtual*/ std::string CException::ToString()
{
	std::string Result = CObject::ToString();
	Result += " '";
	Result += m_Message;
	Result += '\'';
	return Result;
}

} //namespace System
} //namespace WT
