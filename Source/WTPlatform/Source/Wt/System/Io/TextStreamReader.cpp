#include "WT/stdafx.h"

#include "WT/System/IO/TextStreamReader.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CTextStreamReader)


CTextStreamReader::CTextStreamReader(CEncodedTextStream* BaseStream) :
	m_BaseStream(BaseStream),
	m_PeekChar(L'\0'),
	m_PeekCharInUse(false),
	m_LineNum(1),
	m_ColumnNum(1)
{
	assert(m_BaseStream);
}

CTextStreamReader::~CTextStreamReader()
{
	m_BaseStream = NULL;
}

void CTextStreamReader::SkipWhitespace()
{
	static wstring::value_type const * const s_SpaceChars = L" \t\r\n";

	// as long as k is a space character, comsume the character
	// and continue peeking
	for (;;)
	{
		wstring::value_type k;
		try
		{
			k = PeekChar();
		} catch (CUnexpectedEndOfDataException const&) {
			break; //End of file; all whitespace skipped
		}

		if (!wcschr(s_SpaceChars, k))
		{
			break; //Not whitespace character
		}

		// consume k
		ReadChar();
	}
}

void CTextStreamReader::Skip(int32 numChars)
{
	wstring::value_type k;
	for (int32 i = 0; i < numChars; i++)
	{
		k = ReadChar();
	}
}

wstring::value_type CTextStreamReader::PeekChar()
{
	wstring::value_type k = ReadChar();
	m_PeekChar = k;
	m_PeekCharInUse = true;
	return k;
}

wstring::value_type CTextStreamReader::ReadChar()
{
	wstring::value_type k;
	// check to see if the user has peeked ahead one byte
	if (m_PeekCharInUse)
	{
		// there is a peek character.
		// instead of reading from the stream we need to return this character that
		// was previously read and clear the one-character peek buffer.
		k = m_PeekChar;
		m_PeekCharInUse = false;
		return k;
	}

	int32 byte1 = m_BaseStream->ReadByte();
	int32 byte2 = m_BaseStream->ReadByte();
	
	if ( -1 == byte1 || -1 == byte2 )
	{
		throw CUnexpectedEndOfDataException("CTextStreamReader::ReadChar() attempted to read a byte, but reached EOF..");
	}

	assert( sizeof(wstring::value_type) == 2);
	k =  static_cast<wstring::value_type>(0x0000FFFF & (((0x000000FF & byte1) << 8) | (0x000000FF & byte2)));

	UpdateLineAndColumn(k);
	return k;
}

wstring CTextStreamReader::ReadToNext(wstring::value_type stopChar)
{
	std::wostringstream wss;
	wstring::value_type k;
	k = PeekChar();
	while (k != stopChar)
	{
		wss << k;
		ReadChar();
		k = PeekChar();
	}
	return wss.str();
}

uint32 CTextStreamReader::GetLineNum()
{
	return m_LineNum;
}

uint32 CTextStreamReader::GetColumnNum()
{
	return m_ColumnNum;
}

void CTextStreamReader::UpdateLineAndColumn(wstring::value_type k)
{
	if (k == L'\n')
	{
		m_ColumnNum = 1;
		m_LineNum++;
	}
	else
	{
		m_ColumnNum++;
	}
}

//----------------------------------------------
// protected methods:

CTextStreamReader::CTextStreamReader() :
	m_BaseStream(NULL),
	m_PeekChar(L'\0'),
	m_PeekCharInUse(false),
	m_LineNum(1),
	m_ColumnNum(1)
{
	assert(0);
}


}
}
}