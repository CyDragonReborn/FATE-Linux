/***************************************************************************************
 *
 * BitStream.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 6/28/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/System/IO/BitStream.h"

namespace WT
{

namespace System
{

namespace IO
{

WT_DEFINE_CLASS(CBitStream)

/*virtual*/ void CBitStream::WriteIntegerLittleEndian( uint32 Int, uint32 BitCount )
{
    for (uint32 i = 0; i < BitCount; ++i)
    {
        WriteBit( Int & (1 << i) ? true : false );
    }
}

/*virtual*/ uint32 CBitStream::ReadIntegerLittleEndian( uint32 BitCount )
{
    uint32 Result = 0;

	for (uint32 i = 0; i < BitCount; ++i)
	{
		uint32 Bit = static_cast<uint32>( ReadBit() );
	    Result += Bit << i;
	}

	return Result;
}

/*virtual*/ void CBitStream::WriteIntegerBigEndian( uint32 Int, uint32 BitCount )
{
    for (int32 i = BitCount - 1; i >= 0; --i)
    {
        WriteBit( Int & (1 << i) ? true : false );
    }
}

/*virtual*/ uint32 CBitStream::ReadIntegerBigEndian( uint32 BitCount )
{
    uint32 Result = 0;

    for (int32 i = BitCount - 1; i >= 0; --i)
    {
		uint32 const Bit = ReadBit();
        Result += Bit << i;
    }

	return Result;
}

/*virtual*/ void CBitStream::WriteFixedLittleEndian( float32 Value, float32 Min, float32 Max, uint32 BitCount )
{
	//TODO check for NaN Value and forbid it.
	//TODO check for excessive BitCount and forbid it or handle with dummy bits.

	float32 fScaled = 0.0f;
	if (Min != Max)
	{
		fScaled = (Value - Min) / (Max - Min);
	}

	uint64 MaxFixed = (1ui64 << BitCount) - 1;
	uint64 Fixed = static_cast<uint64>( fScaled * MaxFixed + 0.5f );
	if (Fixed < 0)
	{
		Fixed = 0;
	} else if (Fixed > MaxFixed) {
		Fixed = MaxFixed;
	}

	WriteIntegerLittleEndian( static_cast<uint32>( Fixed ), BitCount );
}

/*virtual*/ float32 CBitStream::ReadFixedLittleEndian( float32 Min, float32 Max, uint32 BitCount )
{
	//TODO check for excessive BitCount and forbid it or handle with dummy bits.
	uint32 Fixed = ReadIntegerLittleEndian( BitCount );
	float32 fScaled = static_cast<float32>( Fixed ) / ((1 << BitCount) - 1);
	return fScaled * (Max - Min) + Min;
}
	
//TODO fix this bug - writing a NaN with a fractional part of 1 may change
//to an infinity after reading & writing.
/*virtual*/ bool CBitStream::WriteFloatLittleEndian( float32 Value, uint32 BitCount )
{
	uint32 iValue = *reinterpret_cast<uint32*>( &Value );
	if (BitCount == 32)
	{
		WriteIntegerLittleEndian( iValue, BitCount );
	} else if (BitCount < 32) {
		iValue += 1 << (31 - BitCount); //Rounding
		iValue >>= (32 - BitCount);
		WriteIntegerLittleEndian( iValue, BitCount );
	} else {
		WriteIntegerLittleEndian( 0, BitCount - 32 );
		WriteIntegerLittleEndian( iValue, 32 );
	}

	return true;
}

/*virtual*/ float32 CBitStream::ReadFloatLittleEndian( uint32 BitCount )
{
	uint32 iValue;
	if (BitCount <= 32)
	{
		iValue = ReadIntegerLittleEndian( BitCount );
		iValue <<= (32 - BitCount );
	} else {
		//Read dummy bits
		for (uint32 i = 0; i < BitCount - 32; ++i)
		{
			SkipBit();
		}

		iValue = ReadIntegerLittleEndian( 32 );
	}

	return *reinterpret_cast<float32*>( &iValue );
}

}
}
}
