#pragma once

#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/IOException.h"
#include "WT/System/ScopedClasses.h"

namespace WT
{
namespace System
{
namespace IO
{

// Handles encoding text from wstring to format X and decoding from format X to wstring
typedef SmartPointer<class CTextEncoder> HTextEncoder;
class CTextEncoder : public CObject
{
public:
	WT_ABSTRACT_CLASS( CTextEncoder, CObject );

	virtual int32 GetByteCount(wstring const & wstr) = 0;

	virtual int32 GetBytes(wstring const & wstr, int32 charIdx, int32 charCount, uint8* pBuf, int32 bufSize, int32 byteIdx) = 0;
	virtual int32 GetBytes(wstring const & wstr, uint8* pBuf, int32 bufSize, int32 byteIdx) = 0;

	virtual int32 GetCharCount(uint8 const* pBuf, int32 idx, int32 count) = 0;
	virtual int32 GetCharCount(uint8 const* pBuf) = 0;

	virtual wstring GetChars(uint8 const * pBuf, int32& rNumBytesDecoded) = 0;
	virtual wstring GetChars(uint8 const* pBuf, int32 idx, int32 count, int32& rNumBytesDecoded) = 0;

	virtual int32 GetMaxByteCount( int32 charCount ) = 0;

	virtual int32 GetMaxCharCount( int32 byteCount ) = 0;

	virtual int32 GetByteOrderMarkerSize() = 0;
	virtual int32 GetByteOrderMarker(uint8* pMarkerBuf, int32 markerBufSize) = 0;
};

}
}
}