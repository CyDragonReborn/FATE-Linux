#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/ScopedClasses.h"
#include "WT/System/IO/TextEncoder.h"

namespace WT
{
namespace System
{
namespace IO
{

typedef SmartPointer<class CEncodedTextStream> HEncodedTextStream;
class CEncodedTextStream : public CStream
{
public:
	WT_CLASS(CEncodedTextStream, CStream);

	CEncodedTextStream(CTextEncoder* pEncoder, CStream* pBaseStream);
	virtual ~CEncodedTextStream();

	virtual CTextEncoder* GetTextEncoder();
	virtual CStream* GetBaseStream();

	// Stream methods

	virtual bool CanRead() const;
	virtual bool CanSeek() const;
	virtual bool CanWrite() const;
	virtual int32 GetLength() const;
	virtual void SetPosition( int32 position );
	virtual int32 GetPosition() const;

	//BeginRead
	//BeginWrite
	virtual void Close();
	//EndRead
	//EndWrite
	virtual void Flush();
	virtual int32 Read ( uint8 buffer[], int32 count );

	//Returns the read byte [0,255] or -1 on failure.
	virtual int32 ReadByte ();
	virtual int32 Seek ( int32 offset , SeekOrigin::Enum origin );
	virtual void SetLength ( int32 length );
	virtual void Write ( uint8 const buffer[], int32 count );
	virtual void WriteByte ( uint8 value );

	void WriteByteOrderMarker();
private: // private methods
	CEncodedTextStream( );

	void WriteWChar(wstring::value_type wc);

	// Returns a wide character given two bytes. Byte1 is the high order byte, 
	// byte2 is the low order byte.
	wstring::value_type BuildWChar(uint8 byte1, uint8 byte2);

	bool FillDecodedCache();
	uint8 GetCachedReadByte();

private:
	CTextEncoder* m_pEncoder;
	CStream* m_pBaseStream;

	// fields used when writing/encoding
	int8 m_CachedWriteByte;
	bool m_WriteByteIsCached;

	// fields use when reading/decoding
	wstring m_CachedDecodedWChars;
	int32 m_DecodedWCharsByteReadOffset;

	int32 m_MaxBytesPerChar;
	uint8* m_pCachedReadCharBytes;
	int32 m_NumCachedReadBytes;
	
};


}
}
}