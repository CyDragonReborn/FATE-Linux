#include "WT/stdafx.h"
#include "WT/System/IO/Base64Encoder.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

const uint32 First6BitMask  = 0x00FC0000;
const uint32 Second6BitMask = 0x0003F000;
const uint32 Third6BitMask  = 0x00000FC0;
const uint32 Fourth6BitMask = 0x0000003F;


CBase64Encoder::CBase64Encoder()
{
	Reset();
}

CBase64Encoder::~CBase64Encoder()
{
}

CBase64Encoder::CBase64Encoder( CBase64Encoder const& Encoder )
	: m_24BitAccum( Encoder.m_24BitAccum )
	, m_BytesInAccum( Encoder.m_BytesInAccum )
	, m_ChunkReady( Encoder.m_ChunkReady )
	, m_EncodingComplete( Encoder.m_EncodingComplete )
{
	m_CompletedChunk.copyfmt( Encoder.m_CompletedChunk );
	m_CompletedChunk.str() = Encoder.m_CompletedChunk.str();
}

CBase64Encoder& CBase64Encoder::operator=( CBase64Encoder const& Encoder )
{
	m_24BitAccum = Encoder.m_24BitAccum;
	m_BytesInAccum = Encoder.m_BytesInAccum;
	m_CompletedChunk.copyfmt( Encoder.m_CompletedChunk );
	m_CompletedChunk.str() = Encoder.m_CompletedChunk.str();
	m_ChunkReady = Encoder.m_ChunkReady;
	m_EncodingComplete = Encoder.m_EncodingComplete;
	return *this;
}

void CBase64Encoder::AddByte(int8 byte)
{
	assert(!m_EncodingComplete);

	AddByteToAccumulator(byte);
}

void CBase64Encoder::AddBuffer(uint8* buffer, int32 offset, int32 count)
{
	assert(!m_EncodingComplete);

	uint8* pNextByte = buffer + offset;
	uint8* pEnd = pNextByte + count;

	while (pNextByte != pEnd)
	{
		AddByteToAccumulator(*pNextByte);
		pNextByte++;
	}
}

bool CBase64Encoder::EncodedChunkReady()
{
	return m_ChunkReady;
}

wstring CBase64Encoder::FetchEncodedChunk()
{
	wstring chunk = m_CompletedChunk.str();
	m_CompletedChunk.str() = L"";
	m_ChunkReady = false;
	return chunk;
}

// will return a string even if StringChunkReady() returns false.
// This should be the final call to fetch any strings from the Base64Encoder() (until Reset() is called).
// This method will handle flushing out the final (possibly not a multiple of 3 bytes)
// bytes from the encoder and will add the final '=' terminating character of the 
// base64 block.
wstring CBase64Encoder::FetchFinalEncodedChunk()
{
	// for decoding of the accumulator, even if it isn't full
	DecodeAccumulator(true);
	return FetchEncodedChunk();
}

void CBase64Encoder::Reset()
{
	m_24BitAccum = 0;
	m_BytesInAccum = 0;
	m_CompletedChunk.str() = L"";
	m_ChunkReady = false;
	m_EncodingComplete = false;
}



// Implemented based on description from: http://en.wikipedia.org/wiki/Base64
wstring WTCall CBase64Encoder::BufferToWString(uint8* pBuffer, int32 offset, int32 count)
{
	CBase64Encoder encoder;
	encoder.AddBuffer(pBuffer, offset, count);
	return encoder.FetchFinalEncodedChunk();

}

string WTCall CBase64Encoder::BufferToString(uint8* pBuffer, int32 offset, int32 count)
{
	CBase64Encoder encoder;
	encoder.AddBuffer(pBuffer, offset, count);
	wstring WRetval = encoder.FetchFinalEncodedChunk();
	string Retval = "";

	// Yuck -- can anything be slower?  On the other hand,
	// it was very fast to code, and should be very stable.
	const int32 Len = (int32)WRetval.length();
	for (int i=0; i<Len; i++)
	{
		Retval.push_back ((char)WRetval[i]);
	}
	//Retval.push_back ('\0');
	return Retval;

}

void CBase64Encoder::AddByteToAccumulator(int8 byte)
{
	// indicates how many bits to shift depending on
	// if this is the 1st, 2nd or 3rd byte to be added to the accumulator
	static const int32 s_BitShiftAmt[] = {16, 8, 0};

	assert(m_BytesInAccum >= 0);
	assert(m_BytesInAccum <= 2);

	switch (m_BytesInAccum)
	{
	case 0:
		m_24BitAccum = 0x00FF0000 & ((byte) << 16);
		m_BytesInAccum++;
		break;
	case 1:
		m_24BitAccum |= 0x0000FF00 & ((byte) << 8);
		m_BytesInAccum++;
		break;
	case 2:
		m_24BitAccum |= 0x000000FF & (byte);
		m_BytesInAccum++;
		DecodeAccumulator(false);
		break;
	default:
		assert(0);
	}
}

void CBase64Encoder::DecodeAccumulator(bool final)
{
	static wchar_t const * const Base64Chars = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int32 idx1 = (m_24BitAccum & First6BitMask)  >> 18;
	int32 idx2 = (m_24BitAccum & Second6BitMask) >> 12;
	int32 idx3 = (m_24BitAccum & Third6BitMask)  >> 6;
	int32 idx4 = (m_24BitAccum & Fourth6BitMask);

	if (!final)
	{
		assert(m_BytesInAccum == 3);

		m_CompletedChunk << Base64Chars[idx1] << Base64Chars[idx2] << Base64Chars[idx3] << Base64Chars[idx4];
	}
	else
	{
		// this is the final decoding operation
		// the accumulator may not have 3 bytes in it.
		// It is possible that there are 0 bytes in the accumulator
		// then we only have to add the block terminating character.

		// since BytesInButBuffer is not zero, we know there is at least once bytes
		// in the buffer, This means we need to convert at least the first two indices
		// (the first 12 bits)
		if (m_BytesInAccum > 0)
		{
			m_CompletedChunk << Base64Chars[idx1] << Base64Chars[idx2];
		}

		// If we have at least a 2nd byte in the buffer then we need to write
		// out the 3rd indice
		if (m_BytesInAccum > 1)
		{
			m_CompletedChunk << Base64Chars[idx3];
		}

		// if we have 3 bytes in the buffer then write out the final indice also.
		if (m_BytesInAccum > 2)
		{
			m_CompletedChunk << Base64Chars[idx4];
		}

		// this is the end of the encoding so add the terimating character ('=')
		m_CompletedChunk << L'=';
		m_EncodingComplete = true;
	}

	m_BytesInAccum = 0;
	m_24BitAccum = 0;
}

} // Xml namespace
} // System namespace
} // WT namespace
