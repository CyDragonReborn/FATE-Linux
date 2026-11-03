#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace System
{
namespace IO
{

namespace SeekOrigin
{
	enum Enum
	{
		Begin,
		Current,
		End
	};
}

// Abstract representation of a stream.  Common streams include the
// FileStream and MemoryStream; other streams include the Base64Stream,
// BitStream, EncodedTextStream, and TypeStream.
//
typedef SmartPointer<class CStream> HStream;
class CStream : public CObject
{
public:
	WT_ABSTRACT_CLASS( CStream , CObject )

//	virtual ~CStream() =0;

	// When true you can read from the stream; when false attempts
	// to read will result in failure.
	virtual bool CanRead() const = 0;

	// When true you can seek in the stream; when false seeks will not work
	virtual bool CanSeek() const = 0;

	// When true you can write into the stream; when false you cannot.
	virtual bool CanWrite() const = 0;

	// Gets the length of the stream in bytes.  The length is the 
	// number of bytes (not chars) that you are currently able to
	// read OR, in the case of FileStream, the number of bytes in the file.
	virtual int32 GetLength() const = 0;

	// Sets the current cursor position in the stream for both reading
	// and writing.  The positions should be gotten from a corresponding
	// GetPosition call and should not be done "ad hoc".
	// See the Seek() method for another method that is useful here.
	virtual void SetPosition( int32 position ) = 0;
	virtual int32 GetPosition() const = 0;

	//BeginRead
	//BeginWrite

	// Closes the stream.
	virtual void Close() = 0;
	//EndRead
	//EndWrite

	// Flushes the stream.  For streams that go to files, 'Flush' means
	// 'treat all this data as precious and make sure that it ends up on
	// the disk'.
	virtual void Flush() = 0;

	// Reads data from the stream into a buffer. The buffer gets the data 
	// and count is the number of bytes to read.
	virtual int32 Read ( uint8 buffer[], int32 count ) = 0;

	//Returns the read byte [0,255] or -1 on failure.
	virtual int32 ReadByte () = 0; 

	// A fancier version of SetPosition()
	virtual int32 Seek ( int32 offset , SeekOrigin::Enum origin ) = 0;

	// Sets the length of the stream; can set the length to be smaller
	// than the original length (in which case all of the old positions
	// will be set to the length of the stream)
	virtual void SetLength ( int32 length ) = 0;

	// Writes 'count' bytes of data to the stream, into the buffer.  
	// If the current position is not the end of the stream then will 
	// overwrite the old data (that is: the old data will be overwritten, 
	// not 'shoved forward')
	virtual void Write ( uint8 const buffer[], int32 count ) = 0;

	// Writes the given string into the buffer -- is a simpler way of
	// calling Write ((const uint8*)String.c_str(), 0, (int32)String.Length());
	// especially because of the casting involved.  
	//
	// This cannot be Write () because most classes will overwrite
	// the main 'Write' method, which in turn would hide this method
	// if it used the same name.
	virtual void WriteString ( const std::string& String );

	// Alternate version of WriteString that uses a Stream
	virtual void WriteStream ( CStream& InStream );

	// Writes a single byte into the stream.  Internally is identical to
	// a call to Write (&value, 0, 1);
	virtual void WriteByte ( uint8 value );
};

}
}
}