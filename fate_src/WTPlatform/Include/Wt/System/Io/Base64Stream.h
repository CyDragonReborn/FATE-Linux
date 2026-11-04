#pragma once

#include "WT/System/IO/Stream.h"
#include "WT/System/IO/Base64Encoder.h"
#include <string>

namespace WT
{
namespace System
{
namespace IO
{

// This Stream wraps other Streams. 
// When it is asked to write data it takes that data
// and converts it to base64 encoding before writing it to
// its wrapped stream.
//

typedef SmartPointer<class CBase64Stream> HBase64Stream;
class CBase64Stream : public CStream
{
public:
	WT_CLASS( CBase64Stream, CStream );

	// Base64Stream will not claim ownership of BaseStream. 
	// it is up to the user to ensure that BaseStream stays alive for the duration
	// of Base64Stream's existence.
	CBase64Stream(CStream* BaseStream);

	virtual ~CBase64Stream();

	//
	// Stream methods
	//

	virtual bool CanRead() const;
	virtual bool CanSeek() const;
	virtual bool CanWrite() const;
	virtual int32 GetLength() const;
	virtual void SetPosition( int32 Position );
	virtual int32 GetPosition() const;

	//BeginRead
	//BeginWrite

	// Close() will not close the wrapped stream, but it will generate the final closing
	// characters to make the base64 encoded block a valid base64 block.
	virtual void Close();
	//EndRead
	//EndWrite
	virtual void Flush();
	virtual int32 Read ( uint8 buffer[], int32 count );
	virtual int32 ReadByte ();
	virtual int32 Seek ( int32 offset , SeekOrigin::Enum origin );
	virtual void SetLength ( int32 length );
	virtual void Write ( uint8 const buffer[], int32 count );
	virtual void WriteByte ( uint8 value );

protected: // methods

	// No one should directly create a Base64Stream from the default constructor
	CBase64Stream();

private: // methods
	// if encoded data is available this will write it to the base stream
	void WriteEncodedDataToBaseStream();
private: // data
	CStream* m_BaseStream;
	CBase64Encoder m_Encoder;
};

} // IO namespace
} // System namespace
} // WT namespace

