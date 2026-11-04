#pragma once

#include "WT/System/IO/Stream.h"
#include <string>

namespace WT
{
namespace System
{
namespace IO
{

// This Stream wraps other Streams. 
// It expands the interface by adding a series overloaded methods for 
// writing various types. CTypeStream will convert all base types to network
// byte order before writing the data to the wrapped stream.
// 
// CTypeStream also provides a series of methods for fetching specific types 
// from the wrapped stream (for exmaple: GetInt32()). CTypeStream will convert
// the type to be read from network byte order to host byte order before returning 
// the value to the callee.
//
typedef SmartPointer<class CTypeStream> HTypeStream;
class CTypeStream : public CStream
{
public:
	WT_CLASS_NO_CREATOR( CTypeStream , CStream );

	// CTypeStream will not claim ownership of WrappedStream. 
	// it is up to the user to ensure that WrappedStream stays alive for the duration
	// of CTypeStream's existence.
	CTypeStream(CStream* WrappedStream);

	virtual ~CTypeStream();

	//
	// Stream methods
	//

	bool CanRead() const
	{
		assert(m_WrappedStream);
		return m_WrappedStream->CanRead();
	}

	bool CanSeek() const
	{
		assert(m_WrappedStream);
		return m_WrappedStream->CanSeek();
	}

	bool CanWrite() const
	{
		assert(m_WrappedStream);
		return m_WrappedStream->CanWrite();
	}

	int32 GetLength() const
	{
		assert(m_WrappedStream);
		return m_WrappedStream->GetLength();
	}


	void SetPosition( int32 position )
	{
		assert(m_WrappedStream);
		m_WrappedStream->SetPosition(position);
	}

	int32 GetPosition() const
	{
		assert(m_WrappedStream);
		return m_WrappedStream->GetPosition();
	}

	void Close()
	{
		assert(m_WrappedStream);
		return m_WrappedStream->Close();
	}

	void Flush()
	{
		assert(m_WrappedStream);
		return m_WrappedStream->Flush();
	}

	int32 Read ( uint8 buffer[], int32 count )
	{
		assert(m_WrappedStream);
		return m_WrappedStream->Read(buffer, count);
	}

	int32 ReadByte()
	{
		assert(m_WrappedStream);
		return m_WrappedStream->ReadByte();
	}


	int32 Seek ( int32 offset , SeekOrigin::Enum origin )
	{
		assert(m_WrappedStream);
		return m_WrappedStream->Seek(offset, origin);
	}

	void SetLength ( int32 length )
	{
		assert(m_WrappedStream);
		m_WrappedStream->SetLength(length);
	}

	void Write( uint8 const buffer[], int32 count )
	{
		assert(m_WrappedStream);
		m_WrappedStream->Write(buffer, count);
	}

	void WriteByte ( uint8 value )
	{
		assert(m_WrappedStream);
		m_WrappedStream->WriteByte(value);
	}

	//
	// CTypeStream specific methods
	//

	// These methods are only valid to call if CanWrite() returns true
	virtual void WriteValue(bool value);
	virtual void WriteValue(int8 value);
	virtual void WriteValue(int16 value);
	virtual void WriteValue(int32 value);
	virtual void WriteValue(int64 value);
	virtual void WriteValue(uint8 value);
	virtual void WriteValue(uint16 value);
	virtual void WriteValue(uint32 value);
	virtual void WriteValue(uint64 value);
	virtual void WriteValue(float32 value);
	virtual void WriteValue(float64 value);
	virtual void WriteValue(const string& value);
	virtual void WriteValue(const wstring& value);

	// These methods are only valid to call if CanRead() returns true

	// All of these methods will throw a UnexpectedEndOfDataException if
	// the end of the stream is encountered prior to reading all the
	// bytes necessary to fetch the requested type.
	virtual bool GetBool();
	virtual int8 GetInt8();
	virtual int16 GetInt16();
	virtual int32 GetInt32();
	virtual int64 GetInt64();
	virtual uint8 GetUInt8();
	virtual uint16 GetUInt16();
	virtual uint32 GetUInt32();
	virtual uint64 GetUInt64();
	virtual float32 GetFloat32();
	virtual float64 GetFloat64();
	virtual string GetString();
	virtual wstring GetWString();


	// additions to support compact type formats
	virtual void WriteCompactValue(int8 value);
	virtual void WriteCompactValue(int16 value);
	virtual void WriteCompactValue(int32 value);
	virtual void WriteCompactValue(int64 value);

	virtual void WriteCompactValue(uint8 value);
	virtual void WriteCompactValue(uint16 value);
	virtual void WriteCompactValue(uint32 value);
	virtual void WriteCompactValue(uint64 value);
	virtual void WriteCompactValue(string const & value);
	virtual void WriteCompactValue(wstring const & value);

	virtual int64  GetCompactIntType();
	virtual uint64 GetCompactUIntType();
	virtual string GetCompactString();
	virtual wstring GetCompactWString();

private: // methods

	// Read "count" bytes into the buffer.
	void ReadAll(uint8 buffer[], int32 count);

	int32 ReadByteOrThrow();
private: // data
	CStream* m_WrappedStream;
};

} // IO namespace
} // System namespace
} // WT namespace

