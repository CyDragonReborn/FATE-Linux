#include "WT/stdafx.h"


#include "WT/System/IO/Stream.h"
#include "WT/System/Xml/XmlReader.h"
#include "WT/System/Xml/XmlException.h"
#include "WT/System/IO/Utf8TextEncoder.h"
#include "WT/System/IO/Utf16TextEncoder.h"

#include <iostream>
#include <sstream>

//TODO Find out what is "wrong" here. Why do we have wrong code??
#define FAST_BUT_WRONG

namespace WT
{
namespace System
{
using namespace IO;

namespace Xml
{
#ifdef FAST_BUT_WRONG
	// CTextStreamUtf8Reader takes in a CStream which is known to be
	// in UTF-8 format AND which is also known to be an an XML file 
	// and provides facilities so that the real XML parser
	// can read the XML file in quickly.  By combining the two features
	// together the code can be made much faster (the alternate, a
	// CTextStreamReader + Utf8TextEncoder, has been measured to be
	// significanly slower in release mode and painfully slow in 
	// debug mode.  Measurements taken of a 1.5 meg XML file with 
	// large BASE64 text fields)
	//
	//
	class CTextStreamUtf8Reader 
	{
	public:
		CTextStreamUtf8Reader(IO::CStream* stream)
		{
			Init();
			m_pStream=stream;
		}

		virtual ~CTextStreamUtf8Reader()
		{
		}

		static inline bool IsWS(const char ch)
		{
			const bool Retval = ch==' '||ch=='\t'||ch=='\r'||ch=='\n';
			return Retval;
		}

		// some of these methods can throw UnexpectedEndOfDataException()

		void SkipWhitespace()
		{
			// Note: I'm allowed to peek directly at the chars
			// because each of the whitespace chars in question
			// is a 7-bit ascii char.  No byte of a UTF-8
			// buffer is a 7-bit char except for real 7-bit chars.
			for (;;)
			{
				QFillBuffer();
				if (!NInBuffer() || !IsWS( m_Buffer[m_BIndex] ))
				{
					break;
				}
				++m_BIndex;
			}
		}

		void Skip()
		{
			ReadChar();
		}

		wstring::value_type PeekChar()
		{
			QFillBuffer();
			return GetChar (false); //false==do not advance
		}

		wstring::value_type ReadChar()
		{
			QFillBuffer();
			return GetChar (true); //true==do advance
		}

		// returns a string of characters from the current read location up to but
		// not including the first instance of stopChar.
		wstring ReadToNext(wstring::value_type stopChar)
		{
			const int KMaxBuffer=100;
			wstring::value_type Buffer [KMaxBuffer];
			int BIndex = 0;

			wstring::value_type k;
			k = PeekChar();
			while (k != stopChar)
			{
				Buffer[BIndex++]=k;
				Skip();
				// Too big; switch to a more correct but slower implementation
				if (BIndex >= KMaxBuffer-1)
				{
					Buffer[BIndex] = 0; // NUL terminate the buffer
					return ReadToNextBig (Buffer, BIndex, stopChar);
				}
				k = PeekChar();
			}
			Buffer[BIndex] = 0; // NUL terminate the buffer
			return Buffer;
		}
		wstring CTextStreamUtf8Reader::ReadToNextBig (wstring::value_type* OrigBuffer, const int OrigSize, wstring::value_type stopChar)
		{
			const int KMaxBuffer=2000000;
			wstring::value_type* Buffer = new wstring::value_type[KMaxBuffer];
			int BIndex = OrigSize;
			memcpy (Buffer, OrigBuffer, sizeof (Buffer[0])*OrigSize);

			wstring::value_type k;
			k = PeekChar();
			while (k != stopChar)
			{
				Buffer[BIndex++]=k;
				Skip();
				// Too big; switch to the most correct (but slowest) implementation
				if (BIndex >= KMaxBuffer-1)
				{
					Buffer[BIndex] = 0; // NUL terminate the buffer
					std::wstring Tmp (Buffer);
					return ReadToNextCorrect (Tmp, stopChar);
				}
				k = PeekChar();
			}
			Buffer[BIndex++] = 0; // NUL terminate the buffer
			wstring Retval(Buffer);
			delete[] Buffer;
			return Retval;
		}
		wstring CTextStreamUtf8Reader::ReadToNextCorrect (wstring& StartingBuffer, wstring::value_type stopChar)
		{
			wstring::value_type k;
			k = PeekChar();
			while (k != stopChar)
			{
				StartingBuffer.push_back (k);
				Skip();
				k = PeekChar();
			}
			return StartingBuffer;
		}

		uint32 GetLineNum() 
		{
			return m_Row;
		}

		uint32 GetColumnNum()
		{
			return m_Col;
		}


	private:
		IO::CStream* m_pStream;
		uint8 m_Buffer[1024];
		int m_BIndex; // index to the next char to read
		int m_BLast; // index to just beyond the buffer.  There are m_BLast-m_BIndex bytes to read.
		int m_Col;
		int m_Row;

		void Init()
		{
			m_pStream=0;
			m_Col=0;
			m_Row=0;
			m_BIndex=0;
			m_BLast=0;
		}
		inline int32 NInBuffer() const { return m_BLast-m_BIndex; }
		inline bool LotsInBuffer() const { return NInBuffer() >= 6; }
		inline void QFillBuffer() { if (!LotsInBuffer()) FillBuffer(); }

		void FillBuffer()
		{
			const int NByte = m_BLast - m_BIndex;
			if (NByte <= 6)
			{
				if (NByte>0 && m_BIndex>0)
				{
					memcpy (&m_Buffer[0], &m_Buffer[m_BIndex], NByte);
					m_BLast-=m_BIndex;
					m_BIndex=0;
				}
				const int32 Avail = m_pStream->GetLength();
				const int32 Space = sizeof (m_Buffer) - m_BLast;
				const int32 NToRead = std::min( Avail, Space );
				const int32 NRead = m_pStream->Read (m_Buffer + m_BLast, NToRead);
				m_BLast += NRead;
			}
			if (m_BIndex >= m_BLast)
			{
				m_BIndex=m_BLast;
 				m_Buffer[m_BIndex] = '\0';
 				m_Buffer[m_BIndex+1] = '\0';
				//throw CUnexpectedEndOfDataException("CTextStreamUtf8Reader::QFillBuffer() attempted to read a byte, but reached EOF..");
			}
		}

		wstring::value_type GetChar (const bool Advance)
		{
			if (NInBuffer() < 1)
			{
				throw CUnexpectedEndOfDataException("CTextStreamUtf8Reader::GetChar() - insufficient bytes in buffer.");
			}

			int32 NBytes = 1;
			wstring::value_type Retval = m_Buffer[m_BIndex];
			if ((wcscmp ((wchar_t*)&m_Buffer[m_BIndex], L"wstring") == 0) || (Retval == L'w' && m_Buffer[m_BIndex+1] ==L's'))
			{
				//printf ("got wstring!\n");
			}
			if (Retval & 0x80)
			{
				const uint32 TopThreeBits = Retval & 0xE0;
				const uint32 TopFourBits = Retval & 0xF0;
				const uint32 TopFiveBits = Retval & 0xF8;
				if (TopThreeBits == 0xC0)  // C0-->1100 0000
				{
					if (NInBuffer() < 2)
					{
						throw CUnexpectedEndOfDataException("CTextStreamUtf8Reader::GetChar() - insufficient bytes in buffer.");
					}

					// output is range 0x80-->0x7FF
					// bit pattern 110zzzzx 10xxxxxx
					// retval<--zzzzxxxxxxxx (4 z, 8 x)
					Retval = ((m_Buffer[m_BIndex]&0x1F)<<6) | (m_Buffer[m_BIndex+1]&0x3F);
					NBytes = 2;
					assert (Retval >= 0x80);
					assert (Retval <= 0x7FF);
				}
				else if (TopFourBits == 0xE0) // E0-->1110 0000
				{
					if (NInBuffer() < 3)
					{
						throw CUnexpectedEndOfDataException("CTextStreamUtf8Reader::GetChar() - insufficient bytes in buffer.");
					}

					// output is range 0x800-->0xFFFF
					// bit pattern is 1110zzzz 10zxxxxx 10xxxxxx
					// retval <- zzzzzxxx xxxxxxxx
					Retval = ((m_Buffer[m_BIndex]&0x0F)<<12) | ((m_Buffer[m_BIndex+1]&0x3F)<<6) | (m_Buffer[m_BIndex+2]&0x3F);
					NBytes=3;
					assert (Retval >= 0x800);
					assert (Retval <= 0xFFFF);
				}
				else if (TopFiveBits == 0xF0) // F0-->1111 0000
				{
					if (NInBuffer() < 4)
					{
						throw CUnexpectedEndOfDataException("CTextStreamUtf8Reader::GetChar() - insufficient bytes in buffer.");
					}

					// Extended char -- something to remember!
					// bit pattern is 11110zzz 10zzxxxx 10xxxxxx 10xxxxxx
					// there's no good return value
					assert (false);
					Retval = '!'; // return bogus data
					NBytes=4;
				}
			}
			if (Advance)
			{
				m_BIndex+=NBytes;
			}
			return Retval;
		}
	};

#define GETLINENUM() m_TextStreamUtf8Reader ? m_TextStreamUtf8Reader->GetLineNum() : m_TextStreamReader->GetLineNum();
#define PEEKCHAR() m_TextStreamUtf8Reader ? m_TextStreamUtf8Reader->PeekChar() : m_TextStreamReader->PeekChar()
#define READCHAR() m_TextStreamUtf8Reader ? m_TextStreamUtf8Reader->ReadChar() : m_TextStreamReader->ReadChar()					
#define READTONEXT(_CH) m_TextStreamUtf8Reader ? m_TextStreamUtf8Reader->ReadToNext(_CH) : m_TextStreamReader->ReadToNext (_CH)
#define SKIP() m_TextStreamUtf8Reader ? m_TextStreamUtf8Reader->Skip() : m_TextStreamReader->Skip();
#define SKIPWHITESPACE()\
		if (m_TextStreamUtf8Reader) \
			m_TextStreamUtf8Reader->SkipWhitespace();\
		else \
			m_TextStreamReader->SkipWhitespace();
#else
	class CTextStreamUtf8Reader
	{
	};
#define GETLINENUM() m_TextStreamReader->GetLineNum();
#define PEEKCHAR() m_TextStreamReader->PeekChar()
#define READCHAR() m_TextStreamReader->ReadChar()					
#define READTONEXT(_CH) m_TextStreamReader->ReadToNext (_CH)
#define SKIP() m_TextStreamReader->Skip();
#define SKIPWHITESPACE() m_TextStreamReader->SkipWhitespace();
#endif // #ifdef FAST_BUT_WRONG

WT_DEFINE_CLASS(CXmlReader)

//Decode characters that were previously encoded using XmlEncode. This function checks for 
//XML entities beginning with the ampersand character and writes corresponding special characters 
//to the result.
wstring XmlDecode( wstring const& s )
{
	wstring Result;
	Result.reserve( s.size() );

	wchar_t const* const End = &*s.end();
	for (wchar_t const* p = &*s.begin(); p < End;)
	{
		wchar_t const c = *p++;

		//Check for XML entities which correspond to special characters
		if (c == '&')
		{
			//wstring is not terminated by NULL character, so check if we're near the end of the string.
			if (p + 3 <= End && !wcsncmp( p, L"lt;",   3 )) {Result.push_back( L'<'  ); p += 3; continue;}
			if (p + 3 <= End && !wcsncmp( p, L"gt;",   3 )) {Result.push_back( L'>'  ); p += 3; continue;}
			if (p + 4 <= End && !wcsncmp( p, L"amp;",  4 )) {Result.push_back( L'&'  ); p += 4; continue;}
			if (p + 4 <= End && !wcsncmp( p, L"quot;", 5 )) {Result.push_back( L'\"' ); p += 5; continue;}
			if (p + 5 <= End && !wcsncmp( p, L"apos;", 5 )) {Result.push_back( L'\'' ); p += 5; continue;}
		}

		Result.push_back( c );
	}

	return Result;
}

CXmlReader::CXmlReader ( IO::CStream* stream ) :
		m_BaseStream(stream),
		m_pTextEncoder(NULL),
		m_pEncodedStream(NULL),
		m_TextStreamReader(NULL),
		m_TextStreamUtf8Reader(NULL),
		m_NodeType(NodeType::Error)
{
	assert(m_BaseStream);
	ClearNodeData();

	int32 origPos = stream->GetPosition();
	int32 byte1 = stream->ReadByte();
	int32 byte2 = stream->ReadByte();
	int32 byte3 = stream->ReadByte();
	stream->SetPosition(origPos);

	StringEncoding::Type encoding = StringEncoding::UTF8;

	if (byte1 == 0xFF && byte2 == 0xFE)
	{
		//UTF16LE byte order marker
		encoding = StringEncoding::UTF16;
	} else if (byte1 == 0xEF && byte2 == 0xBB && byte3 == 0xBF) {
		//UTF8 byte order marker
		stream->SetPosition( origPos + 3 );
	}

	if (encoding == StringEncoding::UTF8)
	{
#ifdef FAST_BUT_WRONG
		m_pTextEncoder=0;
		m_pEncodedStream=0;
		m_TextStreamReader = 0;
		m_TextStreamUtf8Reader = new CTextStreamUtf8Reader (m_BaseStream);
		return; // quick return
#else
		m_pTextEncoder = new CUtf8TextEncoder();
#endif
	}
	else if (encoding == StringEncoding::UTF16)
	{
		m_pTextEncoder = new CUtf16TextEncoder();
	}
	else
	{
		assert(0);
	}

	m_pEncodedStream = new CEncodedTextStream(m_pTextEncoder, m_BaseStream);

	m_TextStreamReader = new CTextStreamReader(m_pEncodedStream);
}

CXmlReader::~CXmlReader()
{
	delete m_pEncodedStream;
	delete m_pTextEncoder;
	delete m_TextStreamReader;
	delete m_TextStreamUtf8Reader;
}

IO::CEncodedTextStream* CXmlReader::GetEncodedTextStream()
{
	return m_pEncodedStream;
}


// Gets the number of attributes on the current node. 
int32 CXmlReader::AttributeCount() const
{
	return static_cast<int32>(m_AttribNames.size());
}

// Gets the depth of the current node in the XML document. 
int32 CXmlReader::Depth() const
{
	return static_cast<int32>(m_ElementNameStack.size());
}

// Gets a value indicating whether the current node has any attributes. 
bool CXmlReader::HasAttributes() const
{
	return !m_AttribNames.empty();
}

// Gets the qualified name of the current node. 
wstring const& CXmlReader::Name() const
{
	return m_NodeName;
}

// Gets the type of the current node. 
NodeType::Type CXmlReader::NodeType() const
{
	return m_NodeType;
}

// Gets the text value of the current node. 
wstring const& CXmlReader::Value() const
{
	if (m_NodeType != NodeType::TextNode)
	{
		throw CXMLReaderException("XmlReader::Value() called on node type that does not have a value.");
	}
	return m_TextNodeStr;
}

// Overloaded. When overridden in a derived class, gets the value of an attribute. 
// returns true if the attribute existed and was placed into outValue
bool CXmlReader::GetAttribute(wstring const & attribName, wstring& outValue) const
{
	std::map<wstring, wstring>::const_iterator i = m_ElementNodeAttribNames2Values.find(attribName);
	if ( i == m_ElementNodeAttribNames2Values.end() )
	{
		// attribute does not exist
		return false;
	}

	outValue = i->second;
	return true;
}

// returns the value (in outValue) of the attribute at the given index
// return false if there was no attribute at that index
bool CXmlReader::GetAttributeValue(int32 attribIdx, wstring& outValue) const
{
	if (attribIdx < 0 || attribIdx >= static_cast<int32>(m_AttribNames.size()))
	{
		return false;
	}
	
	std::map<wstring, wstring>::const_iterator it = m_ElementNodeAttribNames2Values.find( m_AttribNames[attribIdx] );
	assert( it != m_ElementNodeAttribNames2Values.end() );

	outValue = it->second;
	return true;
}

// returns the name (in outName) of the attribute at the given index
// return false if there was no attribute at that index
bool CXmlReader::GetAttributeName(int32 attribIdx, wstring& outName) const
{
	if (attribIdx < 0 || attribIdx >= static_cast<int32>(m_AttribNames.size()))
	{
		return false;
	}
	outName = m_AttribNames[attribIdx];
	return true;
}


// Overloaded. Tests if the current content node is a start tag. 
bool CXmlReader::IsStartElement() const
{
	return m_NodeType == NodeType::StartElementNode;
}

// When overridden in a derived class, reads the next node from the stream. 
bool CXmlReader::Read()
{
	ReadNextNode();
	if (m_NodeType == NodeType::NoMoreNodes || m_NodeType == NodeType::Error)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CXmlReader::Skip()
{
	int32 origDepth = Depth();
	while (Read())
	{
		if (Depth() <= origDepth)
		{
			break;
		}
	}
}

//---------------------------------------------
// Protected Methods


// TODO: refactor this method. Break it down into smaller methods
// TODO: this method does not know how to skip xml comments (<!-- -->)
NodeType::Type CXmlReader::ReadNextNode()
{
	if (m_AutoCloseElement)
	{
		m_AutoCloseElement = false;

		assert( !m_ElementNameStack.empty() ); // there is a bug if this happens

		//TODO: test this
		InitEndElementNode(m_ElementNameStack.back()); // may throw
		return m_NodeType;
	}

	SKIPWHITESPACE();

	try
	{
		//Check if we are at the end of file
		PEEKCHAR();
	}
	catch (CUnexpectedEndOfDataException const& eodExcept)
	{
		// This may be fine we had to get to the end of the stream at some point.
		// Verify that everything is consistent for being at the end of the file.

		
		//assert(m_AutoCloseElement == true);

		if (!m_ElementNameStack.empty())
		{
			m_NodeType = NodeType::Error;

			// re-throw the exception, we shouldn't hit the end of the
			// stream and still have open elements or an auto close element still
			// active.
			throw eodExcept;
		}
			
		ClearNodeData();
		m_NodeType = NodeType::NoMoreNodes;
		return m_NodeType;
	}


	wstring::value_type k = PEEKCHAR(); //m_TextStreamReader->PeekChar();
	if (k == L'<')
	{
		// skip the last character we just peeked
		SKIP(); // m_TextStreamReader->Skip();

		// this is the start of an element (start or end element? we don't know yet)
		k = PEEKCHAR(); // m_TextStreamReader->PeekChar();
		NodeType::Type prelimNodeType = NodeType::StartElementNode;
		if (k == L'/')
		{
			// skip the last character we just peeked
			SKIP(); // m_TextStreamReader->Skip();

			// this is an ending element
			prelimNodeType = NodeType::EndElementNode;
		}
		else if (k == L'?')
		{
			// skip the last character we just peeked
			SKIP(); // m_TextStreamReader->Skip();

			prelimNodeType = NodeType::PINode;
		}

		SKIPWHITESPACE(); //m_TextStreamReader->SkipWhitespace();

		// this holds the name of the element or processing instruction
		wstring name = ReadXmlName(); // may throw
		
		if (prelimNodeType != NodeType::EndElementNode)
		{
			// We now think this is either a start element or a PI node

			if (prelimNodeType == NodeType::StartElementNode)
			{
				InitStartElementNode(name); // may throw
			}
			else if (prelimNodeType == NodeType::PINode)
			{
				InitPINode(name); // may throw
			}
			else
			{
				assert(0);
			}

			// loop picking up attributes, if we get to the end of the start element
			// this will be detected and we will return from this method.
			// if errors are encountered an exception will be thrown. (XMLReaderException 
			// or UnexpectedEndOfDataException)
			for (;;)
			{
				SKIPWHITESPACE(); //m_TextStreamReader->SkipWhitespace();
				// parse attributes and closing '>' or "/>" or "?>"
				k = PEEKCHAR(); //m_TextStreamReader->PeekChar();
				if (k == L'/' && prelimNodeType == NodeType::StartElementNode)
				{
					SKIP();//m_TextStreamReader->Skip();
					m_AutoCloseElement = true;
					k = READCHAR();//m_TextStreamReader->ReadChar();
					if (k != L'>')
					{
						// something is wrong
						// we read something that looked like this: "<eltname /", but 
						// the next character wasn't '>'
						m_NodeType = NodeType::Error;
						std::ostringstream ss;
						ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
						ss << ", While parsing end element ("<<WideToAnsi(name)<<")";
						ss << " found '/' character and expected to find '>' next, but found '"<<k<<"' instead.";
						throw CXMLReaderException(ss.str());
					}

					return m_NodeType;
				}
				else if (k == L'>' && prelimNodeType == NodeType::StartElementNode)
				{
					SKIP(); //m_TextStreamReader->Skip();
					// we're done with this start element node. Everything looks valid.
					return m_NodeType;
				}
				else if (k == L'?' && prelimNodeType == NodeType::PINode)
				{
					// the next character should be a '>' then we're done
					SKIP(); //m_TextStreamReader->Skip();

					k = READCHAR(); //m_TextStreamReader->ReadChar();
					if (k != L'>')
					{
						// something is wrong
						// we read something that looked like this: "<?piname ?", but 
						// the next character wasn't '>'
						m_NodeType = NodeType::Error;
						std::ostringstream ss;
						ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
						ss << ", While parsing processing instruction (<?"<<WideToAnsi(name)<<" ...)";
						ss << " found '?' character and expected to find '>' next, but found '"<<k<<"' instead.";
						throw CXMLReaderException(ss.str());
					}

					return m_NodeType;
				}
				else
				{
					// we have to assume we've encountered an attribute
					wstring attribName = ReadXmlName(); // may throw
					k = READCHAR(); //m_TextStreamReader->ReadChar();
					if (k != L'=')
					{
						// invalid xml
						// we read an attribute name, but it wasn't immediately
						// followed by a '=' character
						m_NodeType = NodeType::Error;
						std::ostringstream ss;
						ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
						ss << ", While parsing start element ("<<WideToAnsi(name)<<") read in attribute name ("<<WideToAnsi(attribName)<<")";
						ss << ", expected to find '=' character next, but found '"<<k<<"' instead.";
						throw CXMLReaderException(ss.str());
					}

					k = READCHAR(); //m_TextStreamReader->ReadChar();
					if (k != L'\"') // TODO: can valid XML also use the ' character to specify attribute values?
					{
						m_NodeType = NodeType::Error;
						std::ostringstream ss;
						ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
						ss << ", While parsing start element ("<<WideToAnsi(name)<<") read in attribute ("<<WideToAnsi(attribName)<<"=)";
						ss << ", expected to find '\"' character next, but found '"<<k<<"' instead.";
						throw CXMLReaderException(ss.str());
					}
					wstring attribValue = READTONEXT(L'\"'); //m_TextStreamReader->ReadToNext(L'\"');

					// consume the '"' character
					SKIP(); //m_TextStreamReader->Skip();

					if ( ::wcschr(attribValue.c_str(),L'<') != NULL ||
						 ::wcschr(attribValue.c_str(),L'>') != NULL )
					{
						throw CXMLReaderException("found illegal characters in attrbiute value. Attribute values should be Xml Encoded.");
					}

					attribValue = XmlDecode( attribValue );

					AddAttribute(attribName, attribValue); // may throw
				}
			}

		}
		else
		{
			assert(prelimNodeType == NodeType::EndElementNode);

			InitEndElementNode(name); // may throw
			SKIPWHITESPACE(); //m_TextStreamReader->SkipWhitespace();
			k = READCHAR(); //m_TextStreamReader->ReadChar();
			if ( k != L'>')
			{
				m_NodeType = NodeType::Error;
				std::ostringstream ss;
				ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
				ss << ", While parsing end element (" << WideToAnsi(name) << ") expected to find '>' character, but found '" << k << "' instead.";
				throw CXMLReaderException(ss.str());
			}
			return m_NodeType;
		}

	}
	else
	{
		// this should be a text node
		wstring text = READTONEXT(L'<'); //m_TextStreamReader->ReadToNext(L'<');
		if (wcschr (text.c_str(), L'&'))
		{
			text = XmlDecode( text );
		}
		InitTextNode(text); // may throw
		return m_NodeType;
	}

}

void CXmlReader::ClearNodeData()
{
	m_NodeType = NodeType::Unset;
	m_NodeName = L"";
	m_TextNodeStr = L"";
	m_ElementNodeAttribNames2Values.clear();
	m_AttribNames.clear();
	m_AutoCloseElement = false;

}

void CXmlReader::InitPINode(wstring const & name)
{
	ClearNodeData();
	m_NodeType = NodeType::PINode;
	m_NodeName = name;
}

void CXmlReader::InitStartElementNode(wstring const & name)
{
	ClearNodeData();
	m_NodeType = NodeType::StartElementNode;
	m_NodeName = name;
	m_ElementNameStack.push_back(name);
}

void CXmlReader::InitEndElementNode(wstring const & name)
{
	ClearNodeData();
	if (m_ElementNameStack.size() == 0)
	{
		m_NodeType = NodeType::Error;
		std::ostringstream ss;
		ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
		ss << ", Extra end element found (" << WideToAnsi(name) << ").";
		throw CXMLReaderException(ss.str());
	}
	else if (m_ElementNameStack.back() != name)
	{
		m_NodeType = NodeType::Error;
		std::ostringstream ss;
		ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum();
		ss << ", End element found (" << WideToAnsi(name) << "), but it does not match the name of the corresponding start element ("<<WideToAnsi(m_ElementNameStack.back())<<").";
		throw CXMLReaderException(ss.str());
	}

	m_NodeType = NodeType::EndElementNode;
	m_NodeName = name;
	m_ElementNameStack.pop_back();
}

void CXmlReader::InitTextNode(wstring const & text)
{
	ClearNodeData();

	if (m_ElementNameStack.size() == 0)
	{
		// text nodes can only happen inside other elements
		m_NodeType = NodeType::Error;

		std::ostringstream ss;
		ss << "Line #: "<< GETLINENUM(); //m_TextStreamReader->GetLineNum() ;
		ss << ", Text data found outside of any elements";
		ss << ", Text Data: [" << WideToAnsi(text) << "]";
		throw CXMLReaderException(ss.str());
	}

	m_NodeType = NodeType::TextNode;
	const size_t Idx=wcscspn (text.c_str(), L" \t\r\n");
	if (text[Idx]==L'\0')
	{
		m_TextNodeStr = text;
	}
	else
	{
		m_TextNodeStr = Trim(text); // This is measured to take 1/5 of my program startup with a 1.5meg 'patchmanifest.xml' file
	}
}

void CXmlReader::AddAttribute(wstring const & attribName, wstring const & attribValue)
{
	// Only start elements and processing instruction nodes can have attributes.
	if (m_NodeType != NodeType::StartElementNode && 
		m_NodeType != NodeType::PINode)
	{
		// can only add attributes to StartElement nodes.
		// This should never happen, even if invalid xml is supplied.
		// If we get here there is a bug in XmlReader.

		// TODO: throw
		assert(0);
		return;
	}

	std::map<wstring, wstring>::const_iterator i = m_ElementNodeAttribNames2Values.find(attribName);
	if (i != m_ElementNodeAttribNames2Values.end())
	{
		// The same attribute appeared in this element more than once.
		// This is invalid
		m_NodeType = NodeType::Error;

		// throw an exception
		std::ostringstream ss;
		ss << "Line #:" << GETLINENUM(); //m_TextStreamReader->GetLineNum();
		ss << ", Same attribute ("<< WideToAnsi(attribName) <<") found more than once in the one start element ("<< WideToAnsi(m_NodeName) <<").";
		throw CXMLReaderException(ss.str());
	}

	m_ElementNodeAttribNames2Values[attribName] = attribValue;
	m_AttribNames.push_back(attribName);
}



wstring CXmlReader::ReadXmlName()
{
	// any space character, bracket, question mark quote or equal sign indicates that an xml name is finished ()
	// NOTE: do not consume characters from this set. They are not part of the name
	static wstring::value_type const * const g_NameEndChars = L" \t\r\n<>/?=\"\'"; 

	// This is the set of valid characters that can make up the first character of an xml name
	static wstring::value_type const * const g_ValidFirstChars = L"_:abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// this is the set of valid characters that can be after the first character in an xml name
	static wstring::value_type const * const g_ValidNameChars = L".-_:0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	wstring::value_type k;
	k = READCHAR(); //m_TextStreamReader->ReadChar();
	if ( ::wcschr(g_ValidFirstChars, k) == NULL)
	{
		// invalid first character
		std::ostringstream ssmsg;
		ssmsg << "Line #:" << GETLINENUM(); //m_TextStreamReader->GetLineNum();
		ssmsg << ", Attempted to read an xml name, but first character was a '"<<k<<"' character. Valid xml names must begin with '_' or ':' or a letter.";
		throw CXMLReaderException(ssmsg.str());
	}

	const int KMaxBLen=1024;
	wstring::value_type Buffer [KMaxBLen];
	int BIndex=0;

	//std::wostringstream wss;
	//wss << k;
	Buffer[BIndex++]=k;
	k = PEEKCHAR(); //m_TextStreamReader->PeekChar();
	while ( ::wcschr(g_NameEndChars, k) == NULL)
	{
		// consume the peeked character
		SKIP(); //m_TextStreamReader->Skip();
		if (::wcschr(g_ValidNameChars, k) == NULL)
		{
			// this is an invalid character in an xml name (even if this isn't the first character)
			Buffer[BIndex]=L'\0';
			std::ostringstream ssmsg;
			ssmsg << "Line #:" << GETLINENUM(); //m_TextStreamReader->GetLineNum();
			ssmsg << ", Attempted to read an xml name (so far: "<<WideToAnsi(Buffer)<<"), but encountered an invalid xml name character '"<<k<<"'.";
			throw CXMLReaderException(ssmsg.str());
		}
		if (BIndex>KMaxBLen-2)
		{
			Buffer[BIndex]=L'\0';
			std::ostringstream ssmsg;
			ssmsg << "Line #:" << GETLINENUM(); //m_TextStreamReader->GetLineNum();
			ssmsg << ", Attempted to read an xml name (so far: "<<WideToAnsi(Buffer)<<"), but name was too long (over 1000 chars)";
			throw CXMLReaderException(ssmsg.str());
		}
		//wss << k;
		Buffer[BIndex++]=k;
		k =PEEKCHAR(); //m_TextStreamReader->PeekChar();
	}
	Buffer[BIndex]='\0';

	return Buffer;
	//return wss.str();
}

} // namespace Xml
} // namespace System
} // namespace WT