/***************************************************************************************
 *
 * BitStream.h
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

#pragma once

#include "WT/System/IO/Stream.h"
#include "WT/System/IO/IOException.h"

namespace WT
{
namespace System
{
namespace IO
{

/*
CBitStream provides the ability to read and write individual bits to a 
stream. A CBitStream object is initialized with and uses an existing 
CStream object.

Note that it is generally unsafe to use multiple BitStream's based on a single
Stream or to use both a CBitStream and its underlying Stream simultaneously. If
you need to do this sort of thing, then Call FlushBits() after you are finished
using the BitStream and before you use the underlying CStream object another way.
*/
typedef SmartPointer<class CBitStream> HBitStream;
class CBitStream : public CObject
{
public:
	WT_CLASS( CBitStream, CObject );

	CBitStream( CStream* pStream = NULL)
	{
		m_pStream = pStream;
		ResetCachedBits();
	}

	CStream* GetStream()
	{
		return m_pStream;
	}

	CStream const* GetStream() const
	{
		return m_pStream;
	}

	void SetStream( CStream* pStream )
	{
		FlushBits();
		m_pStream = pStream;
	}

	virtual ~CBitStream()
	{
		FlushBits();
	}

	//Discard bits that were read in, or write out zero or more bits to align
	//to the stream to a byte boundary. Calling this permits safely using the
	//underlying stream object using a byte-based mechanism.
	void FlushBits()
	{
		if (m_bWriting && HaveCachedBits())
		{
			OnByteReady( m_Sum );
		}
		ResetCachedBits();
	}

	void WriteBit( bool Bit )
	{
		m_bWriting = true;

        if (Bit)
        {
            m_Sum = static_cast<uint8>( m_Sum + m_Mask );
        }

        m_Mask <<= 1;

        if (!m_Mask)
        {
			OnByteReady( m_Sum );
			ResetCachedBits();
        }
	}

	void SkipBit()
	{
        if (!HaveCachedBits())
        {
			m_Sum = OnByteNeeded();
        }

		m_Mask <<= 1;
		if (!m_Mask)
		{
			ResetCachedBits();
		}
	}

	bool ReadBit()
    {
        if (!HaveCachedBits())
        {
			m_Sum = OnByteNeeded();
        }

		bool Result = m_Sum & m_Mask ? true : false;

		m_Mask <<= 1;
		if (!m_Mask)
		{
			ResetCachedBits();
		}

		return Result;
    }

	//TODO create functions for handling little/big-endian formats and integers of various sizes.
	//Big-endian formats may be useful even if they are non-standard: in some cases, the size 
	//of an integer being read may be variable and may be deduced based on the values
	//of the most significant bits.
	//TODO consider float64 support.

	void WriteIntegerLittleEndian( uint32 Int, uint32 BitCount );
	uint32 ReadIntegerLittleEndian( uint32 BitCount );

    void WriteIntegerBigEndian( uint32 Int, uint32 BitCount );
	uint32 ReadIntegerBigEndian( uint32 BitCount );

	//Write the floating-point Value as a fixed-point quantity of BitCount bits.
	//Value is scaled automatically based on the Min and Max values you specify.
	//Values outside of the range [Min,Max] will be clamped. NaN's are not permitted.
	void WriteFixedLittleEndian( float32 Value, float32 Min, float32 Max, uint32 BitCount );
	float32 ReadFixedLittleEndian( float32 Min, float32 Max, uint32 BitCount );

	//Write a floating-point value using the specified number of bits. When BitCount is 
	//less than 32, precision will be lost. Before fraction bits are dropped, the 
	//floating-point value is rounded appropriately.
	//TODO check for a minimum precision - sign and exponent bits must not be affected.
	//
	//BitCount may be greater than 32, which will introduce superfluous zero bits.
	bool WriteFloatLittleEndian( float32 Value, uint32 BitCount = 32 );
	float32 ReadFloatLittleEndian( uint32 BitCount = 32 );

protected:

	void OnByteReady( uint8 Byte )
	{
		if (m_pStream != NULL)
		{
			m_pStream->WriteByte( Byte );
		}
	}

	uint8 OnByteNeeded()
	{
		assert( m_pStream != NULL );

		uint32 Byte = m_pStream->ReadByte();
		
		if (Byte < 0)
		{
			throw WT::System::IO::CIOException( "ReadByte() failed" );
		}

		return static_cast<uint8>( Byte );
	}

private:

	bool HaveCachedBits() const
	{
		return m_Mask != 1;
	}

	void ResetCachedBits()
	{
		m_bWriting = false;
		m_Mask = 1;
		m_Sum = 0;
	}

	CStream *	m_pStream;	//Underlying stream
	bool		m_bWriting; //True if we are writing bits
	uint8		m_Mask;		//Current bit mask (1 through 128).
    uint8		m_Sum;		//Byte that was read or is to be written
};

}
}
}
