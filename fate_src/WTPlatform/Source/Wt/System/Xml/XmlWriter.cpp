#include "WT/stdafx.h"

#include "WT/System/Xml/XmlWriter.h"
#include "WT/System/Xml/XmlException.h"
#include "WT/System/IO/Base64Encoder.h"
#include "WT/System/IO/Utf8TextEncoder.h"
#include "WT/System/IO/Utf16TextEncoder.h"
#include "WT/System/IO/Stream.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
using namespace IO;

namespace Xml
{

// TODO: this class needs to do more error checking and throw XMLWriterExceptions when
// users attempt to construct invalid XML
// TODO make sure XML comments are legal; additional encoding may be necessary

WT_DEFINE_CLASS(CXmlWriter)

struct XmlStringPoolEntry
{
    void*  pData;
    uint32 Size;
};



template<class T>
class CXmlStringPool :
	public CObject
{
public:
	typedef CXmlStringPool<T> MyClassType;
	WT_CLASS(MyClassType, CObject);

	const XmlStringPoolEntry GetStringPoolEntry( const XmlEntryName::Type Name )
    {
        XmlStringPoolEntry E;
		std::map<XmlEntryName::Type, T>::iterator it = m_StringMap.find( Name );
        if( it != m_StringMap.end() )
        {            
            E.pData = (void*)it->second.c_str();
            E.Size = static_cast<uint32>(it->second.length() * sizeof T::value_type);
        }
        else
        {
            E.pData = (void*)m_Empty.c_str();
            E.Size = 0;
        }
        return E;
    }  

	void SetStringPoolEntry( const XmlEntryName::Type Name, const T& String )
    {
        m_StringMap[Name] = String;
    }

protected:
	std::map<XmlEntryName::Type, T> m_StringMap;
    T								m_Empty;        // empty string
};

typedef SmartPointer<class CXmlAnsiStringPool> HXmlAnsiStringPool;
class CXmlAnsiStringPool : 
	public CXmlStringPool<string>
{
public:
	typedef CXmlStringPool<string> MyBaseClassType;
	WT_CLASS(CXmlAnsiStringPool, MyBaseClassType);

    CXmlAnsiStringPool()         
    {
		SetStringPoolEntry( XmlEntryName::StartDocument1,   "<?xml version=\"1.0\" encoding=\"" );
		SetStringPoolEntry( XmlEntryName::Utf8EncodingName, "utf-8" );
		SetStringPoolEntry( XmlEntryName::Utf16EncodingName,"utf-16" );
		SetStringPoolEntry( XmlEntryName::StartDocument2,   "\" standalone=\"yes\"?>" );
		SetStringPoolEntry( XmlEntryName::Tab,              "\t" );
		SetStringPoolEntry( XmlEntryName::NodeStart,        "<" );
		SetStringPoolEntry( XmlEntryName::NodeEnd,          ">" );
		SetStringPoolEntry( XmlEntryName::EndSlash,         "/" );
		SetStringPoolEntry( XmlEntryName::Space,            " " );
		SetStringPoolEntry( XmlEntryName::Equals,           "=" );
		SetStringPoolEntry( XmlEntryName::DQuote,           "\"" );
		SetStringPoolEntry( XmlEntryName::Newline,          "\r\n" );
		SetStringPoolEntry( XmlEntryName::Bang,				"!" );
		SetStringPoolEntry( XmlEntryName::Dash,				"-" );
        
        m_Empty = "";
    }
};

typedef SmartPointer<class CXmlWideStringPool> HXmlWideStringPool;
class CXmlWideStringPool : public CXmlStringPool<wstring>
{
public:
	typedef CXmlStringPool<wstring> MyBaseClassType;
	WT_CLASS(CXmlWideStringPool, MyBaseClassType);


    CXmlWideStringPool() 
    {
		SetStringPoolEntry( XmlEntryName::StartDocument1,   L"<?xml version=\"1.0\" encoding=\"" );
		SetStringPoolEntry( XmlEntryName::Utf8EncodingName, L"utf-8" );
		SetStringPoolEntry( XmlEntryName::Utf16EncodingName,L"utf-16" );
		SetStringPoolEntry( XmlEntryName::StartDocument2,   L"\" standalone=\"yes\"?>" );
        SetStringPoolEntry( XmlEntryName::Tab,              L"\t" );
        SetStringPoolEntry( XmlEntryName::NodeStart,        L"<" );
        SetStringPoolEntry( XmlEntryName::NodeEnd,          L">" );
        SetStringPoolEntry( XmlEntryName::EndSlash,         L"/" );
        SetStringPoolEntry( XmlEntryName::Space,            L" " );
        SetStringPoolEntry( XmlEntryName::Equals,           L"=" );
        SetStringPoolEntry( XmlEntryName::DQuote,           L"\"" );
        SetStringPoolEntry( XmlEntryName::Newline,          L"\r\n" );
		SetStringPoolEntry( XmlEntryName::Bang,				L"!" );
		SetStringPoolEntry( XmlEntryName::Dash,				L"-" );
		

        m_Empty = L"";
    }
};


typedef CXmlStringPool<string> CXmlStringPool_String;
typedef CXmlStringPool<wstring> CXmlStringPool_WString;
WT_DEFINE_CLASS(CXmlStringPool_String)
WT_DEFINE_CLASS(CXmlStringPool_WString)
WT_DEFINE_CLASS(CXmlAnsiStringPool)
WT_DEFINE_CLASS(CXmlWideStringPool)


static CXmlAnsiStringPool g_XmlAnsiStringPoll;
static CXmlWideStringPool g_XmlWideStringPoll;



CXmlWriter::CXmlWriter ( CStream* stream, StringEncoding::Type encoding ) : 
	m_StartEltOpen(false), 
	m_EncodedTextStream(NULL), 
	m_pEncoder(NULL),
	m_IndentAmt(0),
	m_Encoding(encoding)
{

	if (m_Encoding == StringEncoding::UTF8)
	{
		m_pEncoder = new CUtf8TextEncoder();
	}
	else if (m_Encoding == StringEncoding::UTF16)
	{
		m_pEncoder = new CUtf16TextEncoder();
	}
	else
	{
		throw CSystemException("Unknown StringEncoding type passed to XmlWriter()");
	}

	m_EncodedTextStream = new IO::CEncodedTextStream(m_pEncoder, stream);
}

CXmlWriter::~CXmlWriter()
{
	delete m_EncodedTextStream;
	delete m_pEncoder;
}

IO::CEncodedTextStream* CXmlWriter::GetEncodedTextStream()
{
	return m_EncodedTextStream;
}

void CXmlWriter::Close()
{
	m_EncodedTextStream->Close();
}

void CXmlWriter::Flush()
{
	m_EncodedTextStream->Flush();
}

void CXmlWriter::WriteStartDocument()
{
	assert(!m_StartEltOpen);
	assert(m_IndentAmt == 0);

	m_EncodedTextStream->WriteByteOrderMarker();

	Write(XmlEntryName::StartDocument1);
	if (m_Encoding == StringEncoding::UTF8)
	{
		Write(XmlEntryName::Utf8EncodingName);
	}
	else
	{
		Write(XmlEntryName::Utf16EncodingName);
	}
	Write(XmlEntryName::StartDocument2);
	Write(XmlEntryName::Newline);
}

void CXmlWriter::WriteEndDocument()
{
	// close all open elements:
	while (!m_ElementInfoStack.IsEmpty())
	{
		WriteEndElement();
	}
}

// Closes one element and pops the corresponding namespace scope. 
void CXmlWriter::WriteEndElement()
{
	CloseOpenStartElement(false);
	DecrimentIndent();

	ElementInfo eltInfo = m_ElementInfoStack.Pop();

	// try to do better formating if we see that this is not a leaf element
	if (!eltInfo.m_Leaf)
	{
		WriteIndent();
	}
	Write(XmlEntryName::NodeStart);
	Write(XmlEntryName::EndSlash);
	Write(eltInfo.m_Name);
	Write(XmlEntryName::NodeEnd);
	Write(XmlEntryName::Newline);
}


void CXmlWriter::WriteStartElement(const wstring& eltName)
{
	CloseOpenStartElement(true);
	WriteIndent();
	Write(XmlEntryName::NodeStart);
	Write(eltName);

	m_ElementInfoStack.Push(eltName);
	IncrementIndent();

	m_StartEltOpen = true;
}

void CXmlWriter::WriteAttributeString(const wstring& attribName, const wstring& attribValue)
{

	if (!m_StartEltOpen)
	{
		throw new CXMLWriterException("Attempted to write attribute, but we are not in a start element.");
	}

	Write(XmlEntryName::Space);
	Write(attribName);
	Write(XmlEntryName::Equals);
	Write(XmlEntryName::DQuote);
	Write( EscapeStringForXml( attribValue ) );
	Write(XmlEntryName::DQuote);
}

void CXmlWriter::WriteBase64(uint8* pBuffer, uint32 offset, uint32 count)
{
	wstring base64Str = CBase64Encoder::BufferToWString(pBuffer, offset, count);

	CloseOpenStartElement(false);
	Write(base64Str);
}

// Writes out a comment <!--...--> containing the specified text. 

void CXmlWriter::WriteComment(wstring const &text)
{
	CloseOpenStartElement(true);
	Write(XmlEntryName::NodeStart);
	Write(XmlEntryName::Bang);
	Write(XmlEntryName::Dash);
	Write(XmlEntryName::Dash);
	Write( text );
	Write(XmlEntryName::Dash);
	Write(XmlEntryName::Dash);
	Write(XmlEntryName::NodeEnd);
}

void CXmlWriter::WriteString(wstring const & text)
{
	CloseOpenStartElement(false);
	Write( EscapeStringForXml( text ) );
}

//--------------------------------------------------
//
// Private Methods
//

void CXmlWriter::IncrementIndent()
{
	m_IndentAmt++;
}

void CXmlWriter::DecrimentIndent()
{
	assert(m_IndentAmt > 0);
	m_IndentAmt--;
}

void CXmlWriter::CloseOpenStartElement(bool TriggeredBySubElement)
{
	if (m_StartEltOpen)
	{
		m_StartEltOpen = false;
		Write(XmlEntryName::NodeEnd);
		if (TriggeredBySubElement)
		{
			Write(XmlEntryName::Newline);
		}
	}
}

void CXmlWriter::Write(XmlEntryName::Type entry)
{
	XmlStringPoolEntry E = g_XmlWideStringPoll.GetStringPoolEntry(entry);
    m_EncodedTextStream->Write( (uint8*)E.pData, E.Size );
}

void CXmlWriter::Write( const wstring& Text )
{
	uint32 Size = static_cast<uint32>(Text.length() * sizeof wstring::value_type);
	m_EncodedTextStream->Write( (uint8*)Text.c_str(), Size );
}


void CXmlWriter::WriteIndent()
{
	for (int32 i = 0; i < m_IndentAmt; i++)
	{
		Write(XmlEntryName::Tab);
	}
}

//----------------------------------------------
// ElementInfoStack methods:

void CXmlWriter::CElementInfoStack::Push(wstring const & name)
{
	if (!IsEmpty())
	{
		// this last element is clearly not a leaf because there is a sub element we are
		// addding right now.
		m_Stack.back().m_Leaf = false;
	}

	m_Stack.push_back(ElementInfo(name));
}

CXmlWriter::ElementInfo CXmlWriter::CElementInfoStack::Pop()
{
	ElementInfo info = m_Stack.back();
	m_Stack.pop_back();
	return info;
}

bool CXmlWriter::CElementInfoStack::IsEmpty()
{
	return m_Stack.begin() == m_Stack.end();
}



} // namespace Xml
} // namespace System
} // namespace WT