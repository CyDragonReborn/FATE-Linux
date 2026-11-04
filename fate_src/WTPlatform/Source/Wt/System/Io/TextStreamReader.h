#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/IO/EncodedTextStream.h"

namespace WT
{
namespace System
{
namespace IO
{

typedef SmartPointer<class CTextStreamReader> HTextStreamReader;
class CTextStreamReader : public CObject
{
public:
	WT_CLASS( CTextStreamReader, CObject );

	CTextStreamReader(CEncodedTextStream* BaseStream);
	virtual ~CTextStreamReader();

	// all of these methods can throw UnexpectedEndOfDataException()
	void SkipWhitespace();
	void Skip(int32 numChars = 1);
	wstring::value_type PeekChar();
	wstring::value_type ReadChar();

	// returns a string of characters from the current read location up to but
	// no including the first instance of stopChar.
	wstring ReadToNext(wstring::value_type stopChar);

	uint32 GetLineNum();
	uint32 GetColumnNum();

protected:
	CTextStreamReader();
	void UpdateLineAndColumn(wstring::value_type k);
private:
	CEncodedTextStream* m_BaseStream;
	int32 m_LineNum;
	int32 m_ColumnNum;
	bool m_PeekCharInUse; // is true if m_PeekChar contains a peeked character, false otherwise
	wstring::value_type m_PeekChar; 
};

}
}
}