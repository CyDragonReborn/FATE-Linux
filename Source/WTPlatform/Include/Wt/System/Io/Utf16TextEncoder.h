#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/IO/TextEncoder.h"

namespace WT
{
namespace System
{
namespace IO
{

typedef SmartPointer<class CUtf16TextEncoder> HUtf16TextEncoder;
class CUtf16TextEncoder : public CTextEncoder
{
public:
	WT_CLASS( CUtf16TextEncoder, CTextEncoder );

	CUtf16TextEncoder();
	virtual ~CUtf16TextEncoder();

	// Overloaded. Overridden. Calculates the number of bytes required to store the results of encoding a set of characters from a specified String or Unicode character array. 
	virtual int32 GetByteCount(wstring const & wstr);

	// Overloaded. Overridden. Encodes a specified range of characters from a Unicode character array or a String and stores the results in a specified byte array. 
	virtual int32 GetBytes(wstring const & wstr, int32 charIdx, int32 charCount, uint8* pBuf, int32 bufSize, int32 byteIdx);
	virtual int32 GetBytes(wstring const & wstr, uint8* pBuf, int32 bufSize, int32 byteIdx);

	// Overloaded. Overridden. Calculates the number of characters that would result from decoding a set of elements in a byte array. 
	virtual int32 GetCharCount(uint8 const * pBuf, int32 idx, int32 count);
	// from encoding:
	virtual int32 GetCharCount(uint8 const * pBuf);

	// Overloaded. Overridden. Decodes a set of elements from a specified byte array and stores the result into a specified set of elements in a Unicode character array. 
	// from encoding:
	virtual wstring GetChars(uint8 const * pBuf, int32& rNumBytesDecoded);
	virtual wstring GetChars(uint8 const * pBuf, int32 idx, int32 count, int32& rNumBytesDecoded);

	// Overridden. Calculates the maximum number of bytes required to encode a specified number of characters. 
	virtual int32 GetMaxByteCount( int32 charCount );

	// Overridden. Calculates the maximum number of characters that can result from decoding a specified number of bytes. 
	virtual int32 GetMaxCharCount( int32 byteCount );

	virtual int32 GetByteOrderMarkerSize();
	virtual int32 GetByteOrderMarker(uint8* pMarkerBuf, int32 markerBufSize);
private:
	uint32 ReadChar(uint8 const * pBuf, int32 bufSize, uint32& bytesRead, uint32& charByteWidth);
	int32 ConvertWCharToUtf8Bytes(wstring::value_type wc, uint8* pBuf);

};


}
}
}