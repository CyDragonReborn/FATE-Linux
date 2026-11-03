#include "WT/stdafx.h"

#include "WT/System/IO/Utf8TextStream.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(Utf8TextStream)



Utf8TextStream::Utf8TextStream(Stream* BaseStream) :
	m_BaseStream(BaseStream)
{
	assert(m_BaseStream);
}

Utf8TextStream::~Utf8TextStream()
{
	m_BaseStream = NULL;
}




//----------------------------------------------
// protected methods:

Utf8TextStream::Utf8TextStream() :
	m_BaseStream(NULL)
{
	assert(0);
}


// based on utf-8 documentation from http://www.cl.cam.ac.uk/~mgk25/unicode.html#unicode
uint32 Utf8TextStream::ReadChar(uint32& charByteWidth)
{
	assert(m_BaseStream->CanRead());
	int32 b = m_BaseStream->ReadByte();
	if (b == -1)
	{
		charByteWidth = 0;
		return 0xFFFFFFFF;
	}
	uint8 oneByte = 0x000000FF & b;
	uint32 firstByteBits = 0x00000000;
	uint32 moreBytesToRead = 0;
	uint32 numBitsInFirstBytes = 0;

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
	m_BaseStream->Read(moreBytesBuf, 0, moreBytesToRead);

	uint32 accum = firstByteBits;
	for (int32 i = 0; i < moreBytesToRead; i++)
	{
		accum = accum << 6;
		accum |= 0x0000003F & moreBytesBuf[i];
	}

	uint8 totalBits = moreBytesToRead*6 + numBitsInFirstBytes;
	charByteWidth = totalBits / 8 + (totalBits % 8 > 0 ? 1 : 0); // character byte width is num total bits divided by 8 (and rounded up)
	return accum;
}



}
}
}