#include "WT/stdafx.h"

#include "WT/System/IO/Base64Stream.h"
#include "WT/System/IO/H2N.h"
#include "WT/System/IO/IOException.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CBase64Stream)


// Base64Stream will not claim ownership of WrappedStream. 
// it is up to the user to ensure that WrappedStream stays alive for the duration
// of Base64Stream's existence.
CBase64Stream::CBase64Stream(CStream* BaseStream) :
	m_BaseStream(BaseStream)
{
	assert(m_BaseStream);
}

CBase64Stream::~CBase64Stream()
{
	// TODO: may need to flush the remaining bits from our base64 accumulator and
	// write out the '=' terminating base64 encoding character
}

//
// Stream methods
//

bool CBase64Stream::CanRead() const
{
	// TODO: right now Base64Stream only supports writting base64. We need to support reading also
	//return m_BaseStream->CanRead();
	return false;
}

bool CBase64Stream::CanSeek() const
{
	// TODO: right now we don't support seeking in base64 streams. This would be a pain.
	return false;
}

bool CBase64Stream::CanWrite() const
{
	return m_BaseStream->CanWrite();
}

int32 CBase64Stream::GetLength() const
{
	return m_BaseStream->GetLength();
}

void CBase64Stream::SetPosition( int32 Position )
{
	// TODO: not implemented. when we change CanSeek() to return something other than false, then we need to implement this
	assert(0);
}

int32 CBase64Stream::GetPosition() const
{
	return m_BaseStream->GetPosition();
}

void CBase64Stream::Close()
{
	// force the generation of the final chunk.
	wstring encodedStr = m_Encoder.FetchFinalEncodedChunk();
	m_BaseStream->Write((uint8*)encodedStr.c_str(), (int32)encodedStr.size()*sizeof(wstring::value_type));

	// force a flush to the base stream.
	m_BaseStream->Flush();
}

void CBase64Stream::Flush()
{
	// Not sure if there is a better way to implement flush.
	// there still may be data in the Base64Encoder that hasn't been
	// converted to an encoded chunk (more data much be added before a chunk is complete)
	// how to we flush this data out? There is not good solution to this.
	m_BaseStream->Flush();
}

int32 CBase64Stream::Read ( uint8 buffer[] , int32 count )
{
	// TODO: do not support reading base64 yet
	assert(0);
	return 0;
}

int32 CBase64Stream::ReadByte ()
{
	// TODO: do not support reading base64 yet
	assert(0);
	return 0;
}

int32 CBase64Stream::Seek ( int32 offset , SeekOrigin::Enum origin )
{
	// TODO: do not support seeking yet
	assert(0);
	return 0;
}

void CBase64Stream::SetLength ( int32 length )
{
	m_BaseStream->SetLength(length);
}

void CBase64Stream::Write ( uint8 const buffer[], int32 count )
{
	m_Encoder.AddBuffer((uint8*)buffer, 0, count);
	WriteEncodedDataToBaseStream();
}
void CBase64Stream::WriteByte ( uint8 value )
{
	m_Encoder.AddByte(value);
	WriteEncodedDataToBaseStream();
}

//-----------------------------------------------------------
// private methods
//

// No one should directly create a Base64Stream from the default constructor
CBase64Stream::CBase64Stream() :
	m_BaseStream(NULL)
{
	assert(0);
}

void CBase64Stream::WriteEncodedDataToBaseStream()
{
	if (m_Encoder.EncodedChunkReady())
	{
		wstring encodedStr = m_Encoder.FetchEncodedChunk();
		m_BaseStream->Write((uint8*)encodedStr.c_str(), (int32)encodedStr.size()*sizeof(wstring::value_type));
	}
}

} // IO namespace
} // System namespace
} // WT namespace