#pragma once

#include "WT/System/IO/Stream.h"
#include <string>

namespace WT
{
namespace System
{
namespace IO
{

namespace KConstants
{
	enum Enum { KDefaultCur=0, KWriteCur=1, KMaxCur=10 };
}

typedef SmartPointer<class CMemoryStream> HMemoryStream;
class CMemoryStream : public CStream
{
public:
	WT_CLASS( CMemoryStream , CStream )

	CMemoryStream();
	CMemoryStream( uint8 Buffer[], uint32 Length );
	CMemoryStream( std::vector<uint8>& Buffer );

	virtual ~CMemoryStream();

	virtual bool CanRead() const;
	virtual bool CanSeek() const;
	virtual bool CanWrite() const;
	virtual void* GetHandle();
	virtual bool IsAsync() const;
	virtual int32 GetLength() const;
	virtual void SetPosition( int32 Position );
	virtual int32 GetPosition() const;
	string const& GetName() const;

	//BeginRead
	//BeginWrite
	virtual void Close();
	//EndRead
	//EndWrite
	virtual void Flush();

	// Gets the allocated size for the memory stream.  In all cases this
	// is at least as long as the Length.  See SetCapacity() for a
	// full discussion.
	int32 GetCapacity() const;

	virtual int32 Read ( uint8 buffer[], int32 count );
	virtual int32 ReadByte ();
	virtual int32 Seek ( int32 offset , SeekOrigin::Enum origin );

	// Sets the Capacity of the memory stream.  This is strictly an
	// efficiency measure and will never (apart from memory effects)
	// have any effect on the actual functionality of the stream.
	// Some memory stream writers can guess, ahead of time, how large a
	// stream will be.  They can use this guess to set the capacity of 
	// a stream -- that is, now many bytes can be written before the
	// stream has to increase in size.  
	//
	// Note that if you set the Capacity of a stream, and then the stream
	// needs to grow even larger, it will grow larger using the normal methods.
	//
	// You cannot set the capacity to smaller than the length of the 
	// stream.  
	void SetCapacity (const int32 Value);
	virtual void SetLength ( int32 length );
	virtual void Write ( uint8 const buffer[], int32 count );
	virtual void WriteByte ( uint8 value );

	// Additional routines for handling streams which can be written
	// and read from at the same time
	virtual void SetPositionCur( int32 Cur, int32 Position );
	virtual int32 GetPositionCur( int32 Cur) const;
	virtual int32 ReadCur ( int32 Cur, uint8 buffer[], int32 count );
	virtual int32 ReadByteCur (int32 Cur );
	virtual int32 SeekCur ( int32 Cur, int32 offset , SeekOrigin::Enum origin );
	virtual void WriteCur ( int32 Cur, uint8 const buffer[], int32 count );
	virtual void WriteByteCur ( int32 Cur, uint8 value );

private:
	uint8* m_pBuffer;
	int32 m_Capacity;
	int32 m_Length;
	bool m_bOwnBuffer;
	int32 m_Cur[KConstants::KMaxCur];

	// Will change the capacity of the buffer without
	// changing the conceptual length (unless you make the
	// capacity smaller than the original length;
	// then the length is strunk to meet the new
	// capacity)
	void ChangeCapacityTo ( const int32 Value );
};

}
}
}
