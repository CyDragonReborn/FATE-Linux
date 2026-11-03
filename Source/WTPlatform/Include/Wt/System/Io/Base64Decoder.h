#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/MemoryStream.h"
#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace IO
{

typedef SmartPointer<class CBase64Decoder> HBase64Decoder;
class CBase64Decoder : public CObject
{
public:
	WT_CLASS( CBase64Decoder, CObject );

	CBase64Decoder(CStream* decodeToStream);
	virtual ~CBase64Decoder();

	// returns true if the base64 termination has been reached
	bool AddChar(int8 c);

	// returns true if the base64 termination has been reached
	bool AddCharBuffer(int8 const * buffer, int32 offset, int32 count);

	// returns the number of bytes decoded to the stream so far
	int32 GetBytesDecoded();

	// returns true if all the base64 data has been completely decoded
	// and all of the data has been passed in via AddChar or AddCharBuffer and 
	// that data has been decoded and written to the stream
	bool NoMoreData();
	
	// Call this to reset the decoder (if you want to resuse the same instace to perform multiple
	// decoding operations. The stream passed becomes the new destincation for the 
	// future decoded data
	void Reset(CStream* DecodeToStream);

	// returns number of bytes written to the stream
	// throws an IOException is the base64 stream appears to be invalid
	static int32 WTCall StringToStream(wstring const & base64Str, CStream* strm);

	// returns number of bytes written to the stream
	// throws an IOException is the base64 stream appears to be invalid
	static int32 WTCall StringToStream(string const & base64Str, CStream* strm);
protected:
	CBase64Decoder();
private:
	void DecodeAccumulator(bool final = false);
	

	uint32			m_24BitAccum;
	uint32			m_CharsInAccum;
	bool			m_DecodingComplete;
	int32           m_DecodedBytes;
	CStream*        m_DecodeToStream;

	// a table containing 128 entries (one for each printable char value).
	// the value stored at each index is the pattern of of 6 bits that that character maps to.
	// if the character is not part of the set of valid base64 characters then 0xFF is stored 
	// at that location in the table
	static uint8 gm_Base64Char2BitPattern[128];
	static bool gm_Base64Init;

	static uint8 WTCall ConvertBase64CharTo6bits(int8 c);
};

} // Xml namespace
} // System namespace
} // WT namespace
