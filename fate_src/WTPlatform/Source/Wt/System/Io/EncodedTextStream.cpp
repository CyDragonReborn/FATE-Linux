#include "WT/stdafx.h"

#include "WT/System/IO/EncodedTextStream.h"
#include "WT/System/ScopedClasses.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CEncodedTextStream);


CEncodedTextStream::CEncodedTextStream(CTextEncoder* pEncoder, CStream* pBaseStream) :
	m_pEncoder(pEncoder),
	m_pBaseStream(pBaseStream),
	m_WriteByteIsCached(false),
	m_CachedWriteByte(0),
	m_MaxBytesPerChar( m_pEncoder->GetMaxByteCount( 1 ) ),
	m_pCachedReadCharBytes(NULL),
	m_NumCachedReadBytes(0),
	m_DecodedWCharsByteReadOffset(0)	
{
	assert(m_pEncoder != NULL);
	assert(m_pBaseStream != NULL);

	// this class assumes that wstring is using wide characters that are exactly two bytes in size
	assert(sizeof(wstring::value_type) == 2);

	m_pCachedReadCharBytes = new uint8[m_MaxBytesPerChar];
}

CEncodedTextStream::~CEncodedTextStream()
{
	m_pEncoder = NULL;
	m_pBaseStream = NULL;
	delete [] m_pCachedReadCharBytes;
}

CTextEncoder* CEncodedTextStream::GetTextEncoder()
{
	return m_pEncoder;
}

CStream* CEncodedTextStream::GetBaseStream()
{
	return m_pBaseStream;
}

// Stream methods

bool CEncodedTextStream::CanRead() const
{
	return m_pBaseStream->CanRead();
}

bool CEncodedTextStream::CanSeek() const
{
	return m_pBaseStream->CanSeek();
}

bool CEncodedTextStream::CanWrite() const
{
	return m_pBaseStream->CanWrite();
}

int32 CEncodedTextStream::GetLength() const
{
	return m_pBaseStream->GetLength();
}

void CEncodedTextStream::SetPosition( int32 position )
{
	m_pBaseStream->SetPosition(position);
}

int32 CEncodedTextStream::GetPosition() const
{
	return m_pBaseStream->GetPosition();
}

//BeginRead
//BeginWrite
void CEncodedTextStream::Close()
{
	Flush();
	m_pBaseStream->Close();

	if (m_WriteByteIsCached)
	{
		throw CIOException("CEncodedTextStream::Close() called but end of writen data was not a complete character. It was only a partial character. Cannot write out final character");
	}
}

//EndRead
//EndWrite
void CEncodedTextStream::Flush()
{
	// Note: there may be a cached byte, we can't flush this out until we have another byte
	// that will allow us to encode it.

	m_pBaseStream->Flush();
}

int32 CEncodedTextStream::Read ( uint8 buffer[], int32 count )
{
	for (int32 i = 0; i < count; i++)
	{
		int32 b = ReadByte();
		if (b == -1)
		{
			return i;
		}
		buffer[i] = static_cast<uint8>( b );
	}

	return count;
}

//Returns the read byte [0,255] or -1 on failure.
int32 CEncodedTextStream::ReadByte ()
{
	if (m_CachedDecodedWChars == L"")
	{
		// attempt to decode some more bytes into the wchar cache
		if (!FillDecodedCache())
		{
			return -1;
		}
	}

	return GetCachedReadByte();
}

int32 CEncodedTextStream::Seek ( int32 offset , SeekOrigin::Enum origin )
{
	return m_pBaseStream->Seek(offset, origin);
}

void CEncodedTextStream::SetLength ( int32 length )
{
	m_pBaseStream->SetLength(length);
}

void CEncodedTextStream::Write ( uint8 const buffer[], int32 count )
{
	if (count == 0)
	{
		return;
	}

	uint8 const* pBuffer = buffer;

	// if there is already a cached byte (partial wchar) from a previous
	// write call, then pick the first byte from this buffer and write
	// out a complete character
	if (m_WriteByteIsCached)
	{
		WriteWChar(BuildWChar(m_CachedWriteByte, *pBuffer));
		m_WriteByteIsCached = false;

		++pBuffer;
		--count;

		// the buffer may only have had one byte left, if there are no bytes left now
		// then just return
		if (count == 0)
		{
			return;
		}

	}

	// Now check to see if we need to cache the last byte of this buffer. This buffer
	// may only have a partial wchar at the end
	if (count % sizeof(wstring::value_type) == 1)
	{
		// yes, we need to cache the last byte
		m_CachedWriteByte = pBuffer[count-1];
		m_WriteByteIsCached = true;

		count--;

		// the buffer may only have had one byte left, if there are no bytes left now
		// then just return
		if (count == 0)
		{
			return;
		}
	}

	// buffer represents the contents of a wstring
	std::wstring wstr( reinterpret_cast<wstring::value_type const *>(pBuffer), count/sizeof(wstring::value_type));
	int32 encodedByteCount = m_pEncoder->GetByteCount(wstr);

	uint8* pBuf = new uint8[encodedByteCount];
	CScopedDeleteArray<uint8> scopedDeleteBuf(pBuf);

	m_pEncoder->GetBytes(wstr, pBuf, encodedByteCount, 0);

	m_pBaseStream->Write(pBuf, encodedByteCount);
}

void CEncodedTextStream::WriteByte ( uint8 value )
{
	if (m_WriteByteIsCached)
	{
		WriteWChar(BuildWChar(m_CachedWriteByte, value));
		m_WriteByteIsCached = false;
	}
	else
	{
		m_CachedWriteByte = value;
		m_WriteByteIsCached = true;
	}
}


CEncodedTextStream::CEncodedTextStream( ) :
	m_pEncoder(NULL),
	m_pBaseStream(NULL),
	m_CachedWriteByte(0),
	m_WriteByteIsCached(false),
	m_MaxBytesPerChar(0),
	m_pCachedReadCharBytes(NULL),
	m_NumCachedReadBytes(0),
	m_DecodedWCharsByteReadOffset()
{
	assert(false);
}

void CEncodedTextStream::WriteWChar(wstring::value_type wc)
{
	std::wstring wstr( &wc, 1);

	// we're assuming there will be no encoding which takes more 
	// than 20 bytes per wide character when encoded.
	// we will assert if this assumtion is ever false.
	// If it turns out to be false we can switch to dynamically allocating this buffer.
	const int32 MaxEncodedCharBytes = 20;
	uint8 buf[MaxEncodedCharBytes];
	assert(m_pEncoder->GetMaxByteCount(1) <= sizeof(buf)); 

	int32 numBytes = m_pEncoder->GetBytes(wstr, (uint8*)&buf, sizeof(buf), 0);
	m_pBaseStream->Write((uint8*)&buf, numBytes);
}

// Returns a wide character given two bytes. Byte1 is the high order byte, 
// byte2 is the low order byte.
wstring::value_type CEncodedTextStream::BuildWChar(uint8 byte1, uint8 byte2)
{
	wstring::value_type wc = (byte1 << 8) & 0xFF00;
	wc |= (0x00FF & (byte2) );
	return wc;
}


uint8 CEncodedTextStream::GetCachedReadByte()
{
	assert(m_CachedDecodedWChars != L"");
	assert(m_DecodedWCharsByteReadOffset < 2);
	wstring::value_type wc = m_CachedDecodedWChars[0];
	if (m_DecodedWCharsByteReadOffset == 0)
	{
		m_DecodedWCharsByteReadOffset++;
		return static_cast<uint8>((wc >> 8) & 0x00FF);
	}
	else 
	{
		assert(m_DecodedWCharsByteReadOffset == 1);

		uint8 b = static_cast<uint8>(wc & 0x00FF);

		// we've finished with the second byte of the first character.
		// We should remove the wchar from the front of the cached string of decoded characters
		// and reset out byte read offset to 0.

		if (m_CachedDecodedWChars.size() > 1)
		{
			m_CachedDecodedWChars = wstring(m_CachedDecodedWChars, 1);
		}
		else
		{
			// this was the last character, at this point we have no more cached decoded characters
			m_CachedDecodedWChars = L"";
		}

		m_DecodedWCharsByteReadOffset = 0;

		return b;
	}
}

// returns false if no more wchars could be decoded into the cache
bool CEncodedTextStream::FillDecodedCache()
{
	assert(m_CachedDecodedWChars == L"");

	do {
		int32 numBytesRead = m_pBaseStream->Read(m_pCachedReadCharBytes + m_NumCachedReadBytes, m_MaxBytesPerChar-m_NumCachedReadBytes);
		m_NumCachedReadBytes += numBytesRead;

		if (numBytesRead == 0)
		{
			return false;
		}

		int32 numBytesDecoded = 0;
		m_CachedDecodedWChars = m_pEncoder->GetChars(m_pCachedReadCharBytes, 0, m_NumCachedReadBytes, numBytesDecoded);
		m_DecodedWCharsByteReadOffset = 0;

		if (m_NumCachedReadBytes != numBytesDecoded)
		{
			// there were some bytes left in the buffer. We need to shift those remaining
			// bytes to the front of the buffer.
			::memcpy(m_pCachedReadCharBytes, &(m_pCachedReadCharBytes[numBytesDecoded]), m_NumCachedReadBytes-numBytesDecoded);
			m_NumCachedReadBytes -= numBytesDecoded;
		}
		else
		{
			// we consumed every byte in the buffer
			m_NumCachedReadBytes = 0;
		}
	}
	while (m_CachedDecodedWChars == L"");

	return true;
}


void CEncodedTextStream::WriteByteOrderMarker()
{
	int32 markerSize = m_pEncoder->GetByteOrderMarkerSize();
	if (markerSize > 0)
	{
		uint8* pMarkerBuf = new uint8[markerSize];
		CScopedDeleteArray<uint8> scopedDeleteMarkerBuffer(pMarkerBuf);

		m_pEncoder->GetByteOrderMarker(pMarkerBuf, markerSize);

		m_pBaseStream->Write((uint8*)pMarkerBuf, markerSize);
	}
}


}
}
}