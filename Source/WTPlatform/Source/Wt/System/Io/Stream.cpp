#include "WT/stdafx.h"

#include "WT/System/IO/Stream.h"

namespace WT
{
namespace System
{
namespace IO
{

WT_DEFINE_CLASS(CStream)

//CStream::~CStream()
//{
//	Close();
//}

int32 CStream::ReadByte ()
{
	uint8 oneByteArray[1];
	int r = Read ( oneByteArray, 1 );
	if ( r==0 )
		return -1;
	return oneByteArray[0];
}

void CStream::WriteByte ( uint8 value )
{
	Write(&value, 1);
}

void CStream::WriteString ( const std::string& String)
{
	assert (String.length() <= 0x7FFFFFFF);
	Write ((const uint8*)String.c_str(), (int32)String.length());
}
void CStream::WriteStream ( CStream& InStream)
{
	const int32 KBufferLen = 16*1024;
	uint8* pBuffer = new uint8 [KBufferLen];

	int32 Len;
	while ((Len = InStream.Read (pBuffer, KBufferLen)) > 0)
	{
		Write (pBuffer, Len);
	}

	delete[] pBuffer;
}

} // namespace IO
} // namespace System
} // namespace WT