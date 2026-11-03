#include "WT/stdafx.h"

#include "WT/System/IO/TypeStream.h"
#include "WT/System/IO/H2N.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/ScopedClasses.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CTypeStream)

CTypeStream::CTypeStream( CStream* WrappedStream )
	: m_WrappedStream( WrappedStream )
{
	assert(m_WrappedStream);
}

CTypeStream::~CTypeStream()
{
}

//
// CTypeStream specific methods
//

void CTypeStream::WriteValue(bool value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	m_WrappedStream->WriteByte( static_cast<uint8>(value ? 0x01 : 0x00) );
}

void CTypeStream::WriteValue(int8 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	m_WrappedStream->WriteByte(value);
}

void CTypeStream::WriteValue(int16 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int16 nValue = H2N(value);
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}


void CTypeStream::WriteValue(int32 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int32 nValue = H2N(value);
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(int64 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int64 nValue = H2N(value);
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(uint8 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	m_WrappedStream->WriteByte(value);
}

void CTypeStream::WriteValue(uint16 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int16 nValue = H2N(*reinterpret_cast<int16*>(&value));
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(uint32 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int32 nValue = H2N(*reinterpret_cast<int32*>(&value));
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(uint64 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int64 nValue = H2N(*reinterpret_cast<int64*>(&value));
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(float32 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int32 nValue = H2N(*reinterpret_cast<int32*>(&value));
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(float64 value)
{
	assert(m_WrappedStream);
	assert(CanWrite());
	int64 nValue = H2N(*reinterpret_cast<int64*>(&value));
	m_WrappedStream->Write(reinterpret_cast<uint8*>(&nValue), sizeof(nValue));
}

void CTypeStream::WriteValue(const string& value)
{
	assert(m_WrappedStream);
	assert(CanWrite());

	uint32 len = static_cast<uint32>(value.length());
	WriteValue(len);
	const uint8* pI8Buf = reinterpret_cast<const uint8*>(value.c_str());
	Write(pI8Buf, len+1);
}

void CTypeStream::WriteValue(const wstring& value)
{
	assert(m_WrappedStream);
	assert(CanWrite());

	uint32 len = static_cast<uint32>(value.length());
	WriteValue(len);
	const uint8* pI8Buf = reinterpret_cast<const uint8*>(value.c_str());
	Write(pI8Buf, (len+1) * sizeof(wstring::value_type));
}

bool CTypeStream::GetBool()
{
	return ReadByteOrThrow() ? true : false;
}

int8 CTypeStream::GetInt8()
{
	return static_cast<int8>(ReadByteOrThrow());
}

int16 CTypeStream::GetInt16()
{
	int16 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	return N2H(value);
}

int32 CTypeStream::GetInt32()
{
	int32 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	return N2H(value);
}

int64 CTypeStream::GetInt64()
{
	int64 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	return N2H(value);
}

uint8 CTypeStream::GetUInt8()
{
	return static_cast<uint8>(ReadByteOrThrow());
}

uint16 CTypeStream::GetUInt16()
{
	uint16 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	int16 h = N2H(*reinterpret_cast<int16*>(&value));
	return *reinterpret_cast<uint16*>(&h);
}

uint32 CTypeStream::GetUInt32()
{
	uint32 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	int32 h = N2H(*reinterpret_cast<int32*>(&value));
	return *reinterpret_cast<uint32*>(&h);
}

uint64 CTypeStream::GetUInt64()
{
	uint64 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	int64 h = N2H(*reinterpret_cast<int64*>(&value));
	return *reinterpret_cast<uint64*>(&h);
}

float32 CTypeStream::GetFloat32()
{
	float32 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	int32 h = N2H(*reinterpret_cast<int32*>(&value));
	return *reinterpret_cast<float32*>(&h);
}

float64 CTypeStream::GetFloat64()
{
	float64 value;
	assert(CanRead());
	ReadAll(reinterpret_cast<uint8*>(&value), sizeof(value));
	int64 h = N2H(*reinterpret_cast<int64*>(&value));
	return *reinterpret_cast<float64*>(&h);
}

string CTypeStream::GetString()
{
	assert(m_WrappedStream);
	assert(CanRead());

	uint32 len = GetUInt32();
	uint8* strBuf = new uint8[len+1];
	CScopedDeleteArray<uint8> scopedDeleteBuffer(strBuf);

	ReadAll(strBuf, len+1);

	string strResult( reinterpret_cast<char*>(strBuf) );

	return strResult;
}

wstring CTypeStream::GetWString()
{
	assert(m_WrappedStream);
	assert(CanRead());

	uint32 len = GetUInt32();
	int32 bufferLen = (len+1) * sizeof(wstring::value_type);
	
	uint8* strBuf = new uint8[bufferLen];
	CScopedDeleteArray<uint8> scopedDeleteBuffer(strBuf);

	ReadAll(strBuf, bufferLen);

	wstring strResult( reinterpret_cast<wstring::value_type*>(strBuf) );

	return strResult;
}

void CTypeStream::ReadAll(uint8 buffer[], int32 count)
{
	assert(m_WrappedStream);
	assert(m_WrappedStream->CanRead());

	if (count < 0)
	{
		throw std::out_of_range( "negative read count" );
	}

	for (int32 totalBytesRead = 0; totalBytesRead < count;)
	{
		int bytesRead = m_WrappedStream->Read( buffer + totalBytesRead, count);
		if (bytesRead == 0)
		{
			// Throw an expection if it is unable to read "count" bytes
			// the caller is expecting the data to be there (perhaps it
			// will require multiple calls to Read() but we should never get
			// a return value of 0 from Read();

			throw CUnexpectedEndOfDataException( );
		}

		totalBytesRead += bytesRead;
		assert( totalBytesRead <= count );
	}
}

namespace CompactByteWidths
{
	enum Type
	{
		Width1Byte = 1,
		Width2Byte = 2,
		Width4Byte = 4,
		Width8Byte = 8,
	};
}

namespace CompactStringTypes
{
	enum Type
	{
		CompactedLengthAnsi = 0,
		CompactedLengthWide = 1,
	};
}



void CTypeStream::WriteCompactValue(int8 value)
{
	WriteValue(static_cast<uint8>(CompactByteWidths::Width1Byte));
	WriteValue(value);
}

void CTypeStream::WriteCompactValue(int16 value)
{
	if ( static_cast<int8>(value) == value )
	{
		// we can shorten this to one byte
		WriteCompactValue(static_cast<int8>(value));
	}
	else
	{
		WriteValue(static_cast<uint8>(CompactByteWidths::Width2Byte));
		WriteValue(value);
	}
}

void CTypeStream::WriteCompactValue(int32 value)
{
	if ( static_cast<int16>(value) == value )
	{
		// we can shorten this to 2 bytes or maybe less
		WriteCompactValue(static_cast<int16>(value));
	}
	else
	{
		WriteValue(static_cast<uint8>(CompactByteWidths::Width4Byte));
		WriteValue(value);
	}
}

void CTypeStream::WriteCompactValue(int64 value)
{
	if ( static_cast<int32>(value) == value )
	{
		// we can shorten this to 4 bytes or maybe less
		WriteCompactValue(static_cast<int32>(value));
	}
	else
	{
		WriteValue(static_cast<uint8>(CompactByteWidths::Width8Byte));
		WriteValue(value);
	}
}



void CTypeStream::WriteCompactValue(uint8 value)
{
	WriteValue(static_cast<uint8>(CompactByteWidths::Width1Byte));
	WriteValue(value);
}

void CTypeStream::WriteCompactValue(uint16 value)
{
	if (value <= std::numeric_limits<uint8>::max())
	{
		// we can shorten this to one byte
		WriteCompactValue(static_cast<uint8>(value));
	}
	else
	{
		WriteValue(static_cast<uint8>(CompactByteWidths::Width2Byte));
		WriteValue(value);
	}
}


void CTypeStream::WriteCompactValue(uint32 value)
{
	if (value <= std::numeric_limits<uint16>::max())
	{
		// we can shorten this to 2 bytes or maybe less
		WriteCompactValue(static_cast<uint16>(value));
	}
	else
	{
		WriteValue(static_cast<uint8>(CompactByteWidths::Width4Byte));
		WriteValue(value);
	}
}

void CTypeStream::WriteCompactValue(uint64 value)
{
	if (value <= std::numeric_limits<uint32>::max())
	{
		// we can shorten this to 4 bytes or maybe less
		WriteCompactValue(static_cast<uint32>(value));
	}
	else
	{
		WriteValue(static_cast<uint8>(CompactByteWidths::Width8Byte));
		WriteValue(value);
	}
}


void CTypeStream::WriteCompactValue(string const & value)
{
	WriteValue(static_cast<uint8>(CompactStringTypes::CompactedLengthAnsi));
	assert (value.length() < 0x7FFFFFFF);
	const int32 len = (int32)value.length();
	WriteCompactValue(len);

	const uint8* pI8Buf = reinterpret_cast<const uint8*>(value.c_str());
	Write(pI8Buf, len); // don't write out the null terminator


}

void CTypeStream::WriteCompactValue(wstring const & value)
{
	WriteValue(static_cast<uint8>(CompactStringTypes::CompactedLengthWide));
	assert (value.length() < 0x7FFFFFFF);
	const int32 len = (int32)value.length();
	WriteCompactValue(len);

	const uint8* pI8Buf = reinterpret_cast<const uint8*>(value.c_str());
	Write(pI8Buf, len * sizeof(wstring::value_type)); // don't write out the null terminator
}

int64 CTypeStream::GetCompactIntType()
{
	CompactByteWidths::Type intWidthType = static_cast<CompactByteWidths::Type>(GetUInt8());

	int64 v = 0;
	switch (intWidthType)
	{
	case CompactByteWidths::Width1Byte:
		v = GetInt8();
		break;
	case CompactByteWidths::Width2Byte:
		v = GetInt16();
		break;
	case CompactByteWidths::Width4Byte:
		v = GetInt32();
		break;
	case CompactByteWidths::Width8Byte:
		v = GetInt64();
		break;
	default:
		throw CIOException("CTypeStream::GetCompactIntType() expected to read CompactByteWidth id, but got unknown value: " + intWidthType);
	}

	return v;
}

uint64 CTypeStream::GetCompactUIntType()
{
	CompactByteWidths::Type uintWidthType = static_cast<CompactByteWidths::Type>(GetUInt8());

	uint64 v = 0;
	switch (uintWidthType)
	{
	case CompactByteWidths::Width1Byte:
		v = GetUInt8();
		break;
	case CompactByteWidths::Width2Byte:
		v = GetUInt16();
		break;
	case CompactByteWidths::Width4Byte:
		v = GetUInt32();
		break;
	case CompactByteWidths::Width8Byte:
		v = GetUInt64();
		break;
	default:
		throw CIOException("CTypeStream::GetCompactUIntType() expected to read CompactByteWidth id, but got unknown value: " + uintWidthType);
	}

	return v;
}

string CTypeStream::GetCompactString()
{
	CompactStringTypes::Type stringType = static_cast<CompactStringTypes::Type>(GetUInt8());
	switch (stringType)
	{
	case CompactStringTypes::CompactedLengthAnsi:
		{
			const uint64 len = GetCompactUIntType();
			assert (len < 0x7FFFFFFF);
            const int32 ilen = (int32)len;

			uint8* strBuf = new uint8[ilen+1];
			CScopedDeleteArray<uint8> scopedDeleteBuffer(strBuf);

			ReadAll(strBuf, ilen);

			// must add our own null terminator
			strBuf[len] = '\0';


			string strResult( reinterpret_cast<char*>(strBuf) );

			return strResult;
		} break;
	default:
		throw CIOException("CTypeStream::GetCompactString() found unexpected CompactStringType: " + stringType);
	}
}

wstring CTypeStream::GetCompactWString()
{
	CompactStringTypes::Type stringType = static_cast<CompactStringTypes::Type>(GetUInt8());
	switch (stringType)
	{
	case CompactStringTypes::CompactedLengthWide:
		{
			const uint64 len = GetCompactUIntType();
			assert (len < 0x7FFFFFFF);
			const int32 ilen = (int32)len;

			const int32 bufferLen = ilen * sizeof(wstring::value_type);

			uint8* strBuf = new uint8[bufferLen+sizeof(wstring::value_type)];
			CScopedDeleteArray<uint8> scopedDeleteBuffer(strBuf);

			ReadAll(strBuf, bufferLen);

			// must add our own null terminator
			strBuf[bufferLen-1] = '\0';
			strBuf[bufferLen  ] = '\0';

			wstring strResult( reinterpret_cast<wstring::value_type*>(strBuf) );

			return strResult;
		} break;
	default:
		throw CIOException("TypeStream::GetCompactWString() found unexpected CompactStringType: " + stringType);
	}
}

int32 CTypeStream::ReadByteOrThrow()
{
	assert(m_WrappedStream);
	assert(CanRead());
	int32 b = m_WrappedStream->ReadByte();
	if (b < 0)
	{
		throw CUnexpectedEndOfDataException( );
	}
	return b;
}


} // IO namespace
} // System namespace
} // WT namespace
