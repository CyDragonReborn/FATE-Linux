#include "WT/stdafx.h"
#include "WT/System/IO/Base64Decoder.h"
#include "WT/System/IO/IOException.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

static const int32 g_Base64Size (128);
uint8 CBase64Decoder::gm_Base64Char2BitPattern[g_Base64Size];
bool CBase64Decoder::gm_Base64Init = false;

uint8 WTCall CBase64Decoder::ConvertBase64CharTo6bits(int8 c)
{
	if (!gm_Base64Init)
	{
		gm_Base64Init=true;

		// the conversion table doesn't exist yet, build it.
		static int8 const * const Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		for (int32 j = 0; j < g_Base64Size; j++)
		{
			gm_Base64Char2BitPattern[j] = 0xFF;
		}

		uint8 i = 0; // i will always be representable by 6 bits
		uint32 len = static_cast<uint32>(::strlen(Base64Chars));
		while (i < len)
		{
			int8 k = Base64Chars[i];

			// ensure k stays within bounds that we can use to index into gm_Base64Char2BitPattern with
			assert(k > 0);
			assert(k < 128); 

			// verify that bits start within the first 6 bits of the byte
			assert( (i & 0xC0) == 0x00); 

			gm_Base64Char2BitPattern[k] = i;
			i++;
		}
	}

	return gm_Base64Char2BitPattern[c];
}

CBase64Decoder::CBase64Decoder(CStream* DecodeToStream)
{
	assert( DecodeToStream != NULL );
	Reset(DecodeToStream);
}

CBase64Decoder::~CBase64Decoder()
{
}

bool CBase64Decoder::AddChar(int8 k)
{
	if (k != '=')
	{
		uint8 bits = ConvertBase64CharTo6bits(k);
		if (bits == 0xFF)
		{
			// k was an invalid base64 character, we cannot decode this
			std::ostringstream ssmsg;
			ssmsg << "Invalid character found in base64 block ('"<<k<<"', ord: "<<static_cast<int32>(k)<<").";
			ssmsg << " Valid base64 characters are A-Z, a-z, 0-9, '+', and '/' and also '=' as a termination character.";
			throw CIOException(ssmsg.str());
		}

		switch (m_CharsInAccum)
		{
		case 0:
			m_24BitAccum =  0x00FC0000 & (bits << 18);
			m_CharsInAccum++;
			break;
		case 1:
			m_24BitAccum |= 0x0003F000 & (bits << 12);
			m_CharsInAccum++;
			break;
		case 2:
			m_24BitAccum |= 0x00000FC0 & (bits << 6);
			m_CharsInAccum++;
			break;
		case 3:
			m_24BitAccum |= 0x0000003F & bits;
			m_CharsInAccum++;
			DecodeAccumulator(false);
			break;
		default:
			assert(0);
		}
		return false;
	}
	else
	{
		DecodeAccumulator(true);
		return true;
	}
}

bool CBase64Decoder::AddCharBuffer(int8 const * buffer, int32 offset, int32 count)
{

	int8 const * pNext = buffer+offset;
	int8 const * pEnd = pNext+count;
	while (pNext != pEnd)
	{
		if (AddChar(*pNext))
		{
			return true;
		}
		pNext++;
	}
	return false;
}

int32 CBase64Decoder::GetBytesDecoded()
{
	return m_DecodedBytes;
}


bool CBase64Decoder::NoMoreData()
{
	if (!m_DecodingComplete)
	{
		return false;
	}
	else
	{
		return m_DecodedBytes == 0; //TODO: this is wrong
	}
}

void CBase64Decoder::Reset(CStream* DecodeToStream)
{
	assert( DecodeToStream != NULL );

	m_24BitAccum = 0;
	m_CharsInAccum = 0;
	m_DecodingComplete = false;

	m_DecodedBytes = 0;
	m_DecodeToStream = DecodeToStream;
}


int32 WTCall CBase64Decoder::StringToStream(wstring const & base64WStr, CStream* strm)
{
	string base64Str = WideToAnsi(base64WStr);
	CBase64Decoder decoder(strm);

	bool done = decoder.AddCharBuffer( base64Str.c_str(), 0, static_cast<int32>(base64Str.size()) );
	if (!done)
	{
		throw CIOException("Incomplete base64 block. No termination character ('=') found.");
	}
	return decoder.GetBytesDecoded();
}

int32 WTCall CBase64Decoder::StringToStream(string const & base64Str, CStream* strm)
{
	CBase64Decoder decoder(strm);

	bool done = decoder.AddCharBuffer( base64Str.c_str(), 0, static_cast<int32>(base64Str.size()) );
	if (!done)
	{
		throw CIOException("Incomplete base64 block. No termination character ('=') found.");
	}
	return decoder.GetBytesDecoded();
}

// Protected methods:

CBase64Decoder::CBase64Decoder()
{
	assert(0);
	Reset(NULL);
}

// Private methods:

void CBase64Decoder::DecodeAccumulator(bool final)
{
	if (!final)
	{
		// we have a full accumulator. Decode it into 3 bytes
		uint8 threeBytes[3];
		threeBytes[0] = static_cast<uint8>((m_24BitAccum & 0x00FF0000 ) >> 16);
		threeBytes[1] = static_cast<uint8>((m_24BitAccum & 0x0000FF00 ) >> 8);
		threeBytes[2] = static_cast<uint8>(m_24BitAccum &  0x000000FF);

		m_DecodeToStream->Write(threeBytes, 3);

		m_DecodedBytes += 3;

		// reset the accumulator
		m_CharsInAccum = 0;
		m_24BitAccum = 0;
	}
	else
	{
		m_DecodingComplete = true;

		uint8 b;
		if (m_CharsInAccum > 0)
		{
			if (m_CharsInAccum == 1)
			{
				// something is wrong, because the final accumulator should contain at least
				// 2 6bit values (see Base64Encoder::DecodeAccumulator() to see that this is always the case
				//
				throw CIOException("Invalid or corrupted base64 data.");
			}

			b = static_cast<uint8>((m_24BitAccum & 0x00FF0000) >> 16);
			m_DecodeToStream->WriteByte(b);
			m_DecodedBytes++;

			// Yes, this is correct, we skip the > 1 chars in accumulator case.

			if (m_CharsInAccum > 2)
			{
				b = static_cast<uint8>((m_24BitAccum & 0x0000FF00) >> 8);
				m_DecodeToStream->WriteByte(b);
				m_DecodedBytes++;

				if (m_CharsInAccum > 3)
				{
					b = static_cast<uint8>(m_24BitAccum & 0x000000FF);
					m_DecodeToStream->WriteByte(b);
					m_DecodedBytes++;
				}

			}
		}
	}
}

} // Xml namespace
} // System namespace
} // WT namespace
