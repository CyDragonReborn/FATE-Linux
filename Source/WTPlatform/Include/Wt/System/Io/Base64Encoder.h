#pragma once

#include "WT/System/Object.h"
#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

//
// Example 1 shows how to encode a buffer of bytes into a base64 encoded string. This 
// exmaple does not demonstrate any streaming encoding. See example 2 to see how to encode
// bytes as they come in.
//
// Example #1:
//
// int8 bunchOfBytes[] = {0x01, 0x43, 0x67, 0x65, 0x22, 0x21};
// string base64Str = CBase64Encoder::BufferToWString( bunchOfBytes, 0, sizeof(bunchOfBytes) );
//
// Example #2:
//
// Stream* pInputStream = GetIStream(); // made-up function
// Stream* pOutStream = GetOStream(); // made-up function
// CBase64Encoder encoder;
//
// int32 byte;
// while (byte = pInputStream->ReadByte() != -1)
// {
//     encoder.AddByte(byte); // could use AddBuffer() if you had more than one byte
//     if (encoder.EncodedChunkReady())
//     {
//         string base64Str = WideToAnsi(encoder.FetchEncodedChunk());
//         pOutStream->Write(base64Str.c_str(), 0, base64Str.size());
//     }
// }
// string finalBase64Str = WideToAnsi(encoder.FetchFinalEncodedChunk());
// pOutStream->Write(finalBase64Str.c_str(), 0, finalBase64Str.size());
//
// 
typedef SmartPointer<class CBase64Encoder> HBase64Encoder;
class CBase64Encoder : public CObject
{
public:
	WT_CLASS( CBase64Encoder, CObject );

	CBase64Encoder();
	CBase64Encoder( CBase64Encoder const& Encoder );
	virtual ~CBase64Encoder();

	CBase64Encoder& operator=( CBase64Encoder const& Encoder );

	void AddByte(int8 byte);
	void AddBuffer(uint8* buffer, int32 offset, int32 count);
	bool EncodedChunkReady();
	wstring FetchEncodedChunk();

	// will return a string even if EncodedChunkReady() returns false.
	// This should be the final call to fetch any strings from the Base64Encoder() (until Reset() is called).
	// This method will handle flushing out the final (possibly not a multiple of 3 bytes)
	// bytes from the encoder and will add the final '=' terminating character of the 
	// base64 block.
	wstring FetchFinalEncodedChunk();

	// Call this to reset the encoder (if you want to resuse the same instace to perform multiple
	// encoding operations.
	void Reset();

	// This is a one shot encoder function.
	// It is not intended for streaming purposes, but it can handle the common case of
	// just encoding an existing buffer. The returned string will be a complete ('=' terminated)
	// base64 string.
	static wstring WTCall BufferToWString(uint8* pBuffer, int32 offset, int32 count);

	// This is a one shot encoder function.
	// It is not intended for streaming purposes, but it can handle the common case of
	// just encoding an existing buffer. The returned string will be a complete ('=' terminated)
	// base64 string.  It may be less efficient than BufferToWString
	static string WTCall BufferToString(uint8* pBuffer, int32 offset, int32 count);
private:
	void AddByteToAccumulator(int8 byte);
	void DecodeAccumulator(bool final = false);

	uint32				m_24BitAccum;
	uint32				m_BytesInAccum;
	std::wostringstream	m_CompletedChunk;
	bool				m_ChunkReady;
	bool                m_EncodingComplete;
};

} // Xml namespace
} // System namespace
} // WT namespace
