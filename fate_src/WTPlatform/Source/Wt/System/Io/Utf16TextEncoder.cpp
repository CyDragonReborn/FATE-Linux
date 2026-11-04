#include "WT/stdafx.h"

#include "WT/System/IO/Utf16TextEncoder.h"
#include "WT/System/ScopedClasses.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CUtf16TextEncoder);


CUtf16TextEncoder::CUtf16TextEncoder()
{
}

CUtf16TextEncoder::~CUtf16TextEncoder()
{
}

int32 CUtf16TextEncoder::GetByteCount(wstring const & wstr)
{
	return ((int32)wstr.length())*sizeof(wstring::value_type);
}

int32	CUtf16TextEncoder::GetBytes(wstring const & wstr, int32 charIdx, int32 charCount, 
								  uint8* pBuf, int32 bufSize, int32 byteIdx)
{
	int32 numBytesToCopy = charCount*sizeof(wstring::value_type);
	if ( numBytesToCopy > bufSize-byteIdx)
	{
		throw CIOException("CUtf16TextEncoder::GetBytes(): buffer to small. In order to encode wstring a larger byte buffer is required.");
	}

	int32 bufIdx = byteIdx;
	for (int32 i = charIdx; i < charIdx+charCount; i++, bufIdx += 2)
	{
		*((wstring::value_type*)&(pBuf[bufIdx])) = wstr[i];
	}

	return numBytesToCopy;
}

int32	CUtf16TextEncoder::GetBytes(wstring const & wstr, uint8* pBuf, int32 bufSize, int32 byteIdx)
{
	return GetBytes(wstr, 0, (int32)(wstr.size()), pBuf, bufSize, byteIdx);
}

int32	CUtf16TextEncoder::GetCharCount(uint8 const * pBuf, int32 idx, int32 count)
{
	if (count % sizeof(wstring::value_type) != 0)
	{
		throw CIOException("Utf16Encoder::GetCharCount(): decoding an incomplete utf16 buffer, final bytes did not form a complete character.");
	}

	return count / sizeof(wstring::value_type);

}

int32	CUtf16TextEncoder::GetCharCount(uint8 const* pBuf)
{
	wstring wstr(reinterpret_cast<wstring::value_type const *>(pBuf) );
	const int32 Retval=(int32)(wstr.length());
	return Retval;
}

wstring	CUtf16TextEncoder::GetChars(uint8 const* pBuf, int32& rNumBytesDecoded)
{
	int32 count = (int32)(::strlen( reinterpret_cast<char const *>(pBuf) ));
	return GetChars(pBuf, 0, count, rNumBytesDecoded);
}

wstring CUtf16TextEncoder::GetChars(uint8 const* pBuf, int32 idx, int32 count, int32& rNumBytesDecoded)
{
	if (count % sizeof(wstring::value_type) != 0)
	{
		rNumBytesDecoded = count - 1;
	}
	else
	{
		rNumBytesDecoded = count;
	}

	std::wostringstream wss;
	for (int32 i = idx; i < idx+count; i+=2)
	{
		wstring::value_type* wc = (wstring::value_type*)&(pBuf[i]);
		if (*wc == 0xFEFF)
		{
			// skip this byte order marker
		}
		else if (*wc == 0xFFFE)
		{
			throw CIOException("Utf16TextEncoder does not support alternative chartacter byte orders.");
		}
		else
		{
			wss << *wc;
		}
	}

	//wstring wstr((wstring::value_type*)&(pBuf[idx]), 0, rNumBytesDecoded/sizeof(wstring::value_type));

	return wss.str();
}

int32	CUtf16TextEncoder::GetMaxByteCount( int32 charCount )
{
	// the maximum numbers of bytes a character can take when utf8 encoded is 6 bytes
	return sizeof(wstring::value_type)*charCount;
}

int32	CUtf16TextEncoder::GetMaxCharCount( int32 byteCount )
{
	// in utf-8 each byte is potentially a different character
	assert(sizeof(wstring::value_type)==2);

	return (byteCount/sizeof(wstring::value_type)) + (byteCount %sizeof(wstring::value_type));
}

int32 CUtf16TextEncoder::GetByteOrderMarkerSize()
{
	return 2;
}

int32 CUtf16TextEncoder::GetByteOrderMarker(uint8* pMarkerBuf, int32 markerBufSize)
{
	if (markerBufSize < 2)
	{
		throw CIOException("Utf16TextEncoder::GetByteOrderMarker() was passed a buffer that is too small to hold the Byte Order Marker.");
	}

	pMarkerBuf[0] = 0xFF;
	pMarkerBuf[1] = 0xFE;

	return 2;
}




}
}
}