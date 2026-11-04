#include "WT/stdafx.h"

#include "WT/System/IO/Utf8TextEncoder.h"
#include "WT/System/ScopedClasses.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CUtf8TextEncoder);


CUtf8TextEncoder::CUtf8TextEncoder()
{
}

CUtf8TextEncoder::~CUtf8TextEncoder()
{
}

int32 CUtf8TextEncoder::GetByteCount(wstring const & wstr)
{
	int32 bytes = 0;
	for (uint32 i = 0; i < wstr.size(); i++)
	{
		wstring::value_type c = wstr.at(i);

		assert(sizeof(c) == 2); // assume wstring is for double byte character strings

		if (c <= 0x007F)
		{
			bytes += 1;
		}
		else if (c <= 0x07FF)
		{
			bytes += 2;
		}
		else
		{
			bytes += 3;
		}
	}

	return bytes;
}

int32	CUtf8TextEncoder::GetBytes(wstring const & wstr, int32 charIdx, int32 charCount, 
								  uint8* pBuf, int32 bufSize, int32 byteIdx)
{
	uint8 byteBuf[6];
	assert(sizeof(byteBuf) == GetMaxByteCount(1));

	uint8* pCurrBufPos = pBuf+byteIdx;
	uint8* pEndBufPos = pBuf+bufSize;

	for (int32 i = charIdx; i < charIdx + charCount; i++)
	{
		int32 numBytes = ConvertWCharToUtf8Bytes(wstr[i], byteBuf);

		if (pCurrBufPos + numBytes > pEndBufPos )
		{
			throw CIOException("CUtf8TextEncoder::GetBytes(): buffer to small. In order to encode wstring a larger byte buffer is required.");
		}

		::memcpy(pCurrBufPos, byteBuf, numBytes);
		pCurrBufPos += numBytes;
	}
	return (int32)(pCurrBufPos-(pBuf+byteIdx));
}

int32	CUtf8TextEncoder::GetBytes(wstring const & wstr, uint8* pBuf, int32 bufSize, int32 byteIdx)
{
	return GetBytes(wstr, 0, (int32)wstr.size(), pBuf, bufSize, byteIdx);
}

int32	CUtf8TextEncoder::GetCharCount(uint8 const * pBuf, int32 idx, int32 count)
{
	int32 charCount = 0;
	for (int32 i = idx; i < idx+count; i++)
	{
		uint8 b = pBuf[i];
		if ((0x80 & b) != 0x00 ) // 0xxxxxxb
		{
			// the high order bit is not 0 so this is a multi byte character

			// this is either a 2 or 3 byte utf8 character
			if ( (0xc0 & b) == 0xc0 ) // 110xxxxxb
			{
				// this is a 2 byte utf8 character

				// skip ahead 1 byte
				i += 1;
			}
			else if ( (0xe0 & b) == 0xe0 ) // 1110xxxxb
			{
				// this is a 3 byte utf8 character

				// skip ahead 2 bytes
				i += 2;
			}
			else
			{
				// this is more than a 3 byte utf8 character, this encoder decodes
				// to wstring which can only support 2 byte decoded characters utf8
				// characters that are more than 3 bytes decode to more than 2 bytes 
				// wide when decoded
				throw CIOException("Utf8Encoder::GetCharCount(): encountered character which would decode to more than 2 bytes wide.");
			}

			if (i > idx+count)
			{
				throw CIOException("Utf8Encoder::GetCharCount(): decoding an incomplete utf8 buffer, final bytes did not form a complete character.");
			}
		}

		charCount++;
	}

	return charCount;
}

int32	CUtf8TextEncoder::GetCharCount(uint8 const* pBuf)
{
	int32 count = (int32)::strlen( reinterpret_cast<char const *>(pBuf) );
	return GetCharCount(pBuf, 0, count);
}

wstring	CUtf8TextEncoder::GetChars(uint8 const* pBuf, int32& rNumBytesDecoded)
{
	int32 count = (int32)::strlen( reinterpret_cast<char const *>(pBuf) );
	return GetChars(pBuf, 0, count, rNumBytesDecoded);
}

wstring CUtf8TextEncoder::GetChars(uint8 const* pBuf, int32 idx, int32 count, int32& rNumBytesDecoded)
{
	uint32 charByteWidth;
	uint32 bytesRead;

	uint8 const * pCurrBufPos = &(pBuf[idx]);
	uint8 const * pEndBufPos  = pCurrBufPos+count;
	std::wostringstream wss;
	while (pCurrBufPos < pEndBufPos)
	{
		uint32 wChar = ReadChar(pCurrBufPos, (int32)(pEndBufPos-pCurrBufPos), bytesRead, charByteWidth);

		if (charByteWidth == 0)
		{
			// there was an incomplete character at the end of this buffer
			break;
		}

		if (charByteWidth > 2)
		{
			throw CIOException("Utf8Encoder::GetChars(): attempted to decode utf8 string that contained decoded characters of more than 2 bytes.");
		}

		pCurrBufPos += bytesRead;

		wss << static_cast<wstring::value_type>(wChar);
	}

	rNumBytesDecoded = (int32)(pCurrBufPos - (pBuf+idx));

	return wss.str();
}

int32	CUtf8TextEncoder::GetMaxByteCount( int32 charCount )
{
	// the maximum numbers of bytes a character can take when utf8 encoded is 6 bytes
	return 6*charCount;
}

int32	CUtf8TextEncoder::GetMaxCharCount( int32 byteCount )
{
	// in utf-8 each byte is potentially a different character
	return byteCount;
}

int32 CUtf8TextEncoder::GetByteOrderMarkerSize()
{
	return 0;
}

int32 CUtf8TextEncoder::GetByteOrderMarker(uint8* pMarkerBuf, int32 markerBufSize)
{
	throw CIOException("CUtf8TextEncoder does not implement a Byte Order Marker. GetByteOrderMarkerSize() returns 0 indicating that GetByteOrderMarker() should not be called");
}



//----------------------------------------------
// private methods:

// based on utf-8 documentation from http://www.cl.cam.ac.uk/~mgk25/unicode.html#unicode
uint32 CUtf8TextEncoder::ReadChar(uint8 const* pBuf, int32 bufSize, uint32& bytesRead, uint32& charByteWidth)
{
	assert(pBuf);
	bytesRead = 0;

	if (bufSize == 0)
	{
		charByteWidth = 0;
		return 0xFFFFFFFF;
	}

	int32 b = *pBuf;
	pBuf++;
	bytesRead++;

	uint8 oneByte = (uint8)(0x000000FF & b);
	uint32 firstByteBits = 0x00000000;
	int32 moreBytesToRead = 0;
	int32 numBitsInFirstBytes = 0;

	if ( (oneByte & 0x80) == 0x00) // has bit pattern: 0xxxxxxxb
	{
		// this is an ansi one byte character
		moreBytesToRead = 0;
		numBitsInFirstBytes = 7;
		firstByteBits = oneByte & 0x0000007F;
	}
	else if ( (oneByte & 0xE0) == 0xC0 ) // has bit pattern: 110xxxxxb
	{
		// this is encoded using two utf-8 bytes
		moreBytesToRead = 1;
		numBitsInFirstBytes = 5;
		firstByteBits = oneByte & 0x0000001F;
	}
	else if ( (oneByte & 0xF0) == 0xE0 ) // has bit pattern: 1110xxxxb
	{
		// this is encoded using three utf-8 bytes
		moreBytesToRead = 2;
		numBitsInFirstBytes = 4;
		firstByteBits = oneByte & 0x0000000F;
	}
	else if ( (oneByte & 0xF8) == 0xF0 ) // has bit pattern: 11110xxxb
	{
		// this is encoded using four utf-8 bytes
		moreBytesToRead = 3;
		numBitsInFirstBytes = 3;
		firstByteBits = oneByte & 0x00000007;
	}
	else if ( (oneByte & 0xFC) == 0xF8 ) // has bit pattern: 111110xxb
	{
		// this is encoded using five utf-8 bytes
		moreBytesToRead = 4;
		numBitsInFirstBytes = 2;
		firstByteBits = oneByte & 0x00000003;
	}
	else if ( (oneByte & 0xFE) == 0xFC ) // has bit pattern: 1111110xb
	{
		// this is encoded using six utf-8 bytes
		moreBytesToRead = 5;
		numBitsInFirstBytes = 1;
		firstByteBits = oneByte & 0x00000001;
	}


	// this is just a one byte ansi character
	if (moreBytesToRead == 0)
	{
		charByteWidth = 1;
		return firstByteBits;
	}

	uint8 moreBytesBuf[5];
	assert(moreBytesToRead <= sizeof moreBytesBuf);
	for (int32 i = 0; i < moreBytesToRead; i++)
	{
		moreBytesBuf[i] = *pBuf;
		if (bytesRead == (uint32)bufSize)
		{
			// the buffer doesn't contain all of the bytes we need in order to decode this character
			bytesRead = 0;
			charByteWidth = 0;
			return 0xFFFFFFFF;
		}
		pBuf++;
		bytesRead++;
	}

	uint32 accum = firstByteBits;
	for (int32 i = 0; i < moreBytesToRead; i++)
	{
		accum = accum << 6;
		accum |= 0x0000003F & moreBytesBuf[i];
	}

	uint8 totalBits = (uint8)(moreBytesToRead*6 + numBitsInFirstBytes);
	charByteWidth = totalBits / 8 + (totalBits % 8 > 0 ? 1 : 0); // character byte width is num total bits divided by 8 (and rounded up)
	return accum;
}


int32 CUtf8TextEncoder::ConvertWCharToUtf8Bytes(wstring::value_type wc, uint8* pBuf)
{
	// U-00000000 – U-0000007F: 	0xxxxxxx
	// U-00000080 – U-000007FF: 	110xxxxx 10xxxxxx
	// U-00000800 – U-0000FFFF: 	1110xxxx 10xxxxxx 10xxxxxx
	// U-00010000 – U-001FFFFF: 	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	// U-00200000 – U-03FFFFFF: 	111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	// U-04000000 – U-7FFFFFFF: 	1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

	assert(sizeof(wc) == 2);
	if ( ((~0x007F) & wc) == 0)
	{
		pBuf[0] = static_cast<uint8>( 0x007F & wc );
		return 1;
	}
	else if ( ((~0x07FF) & wc) == 0)
	{
		pBuf[0] = 0xc0 | static_cast<uint8>( (0x07c0 & wc) >> 6 );
		pBuf[1] = 0x80 | static_cast<uint8>( 0x003F & wc );
		return 2;
	}
	else
	{
		pBuf[0] = 0xe0 | static_cast<uint8>( ( 0xF000 & wc ) >> 12);
		pBuf[1] = 0x80 | static_cast<uint8>( ( 0x0Fc0 & wc ) >> 6 );
		pBuf[2] = 0x80 | static_cast<uint8>( 0x003F & wc );
		return 3;
	}
}


}
}
}