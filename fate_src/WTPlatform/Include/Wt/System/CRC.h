/*
	CRC.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Mike S:	Created  (adapted from crc32, originally by Charles Bloom)
	12/7/2002
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/IO/Stream.h"

namespace WT
{
namespace System
{
/*
Performs a cumulative CRC check on a set of data.  You should call
the various Add() methods to add more data for the CRC and then call
GetCRC() to get the resulting CRC.
 */
class CRC
{
public:
    CRC() : m_CheckSum((uint32)~0) {};
	CRC(WT::System::IO::CStream& Stream): m_CheckSum((uint32)~0) 
	{
		Add (Stream);
	}
    CRC(const uint8 *pBuffer, const uint32 Length) : m_CheckSum((uint32)~0) 
    { 
        Add(pBuffer, Length); 
    }
    CRC(const string& AString, const bool CaseSensitive) : m_CheckSum((uint32)~0) 
    { 
        Add(AString, CaseSensitive); 
    }
   
	void Add(WT::System::IO::CStream& Stream);
    void __forceinline __fastcall Add(const uint8 AByte) 
    { 
        m_CheckSum = CRC_Table[((int)m_CheckSum ^ (AByte)) & 0xff] ^ (m_CheckSum >> 8);
    }

    void __forceinline __fastcall Add(const uint16 AWord) 
    { 
        uint8 *ptr = (uint8 *)&AWord;
	    Add(ptr[0]);
	    Add(ptr[1]);
    }

    void __forceinline __fastcall Add(const uint32 ALong) 
    {
        uint8 *ptr = (uint8 *)&ALong;
	    Add(ptr[0]);
	    Add(ptr[1]);
	    Add(ptr[2]);
	    Add(ptr[3]);
    }

    void __forceinline __fastcall Add(const uint8 * pBuffer, const uint32 Length)
    {
        assert(pBuffer);
        
        const uint8 *Ptr= pBuffer;
        uint32 Len = Length;
	    while (Len--)
        { 
            Add(*Ptr++);
        }
    }
    
    void __forceinline __fastcall Add(const string& AString, const bool CaseSensitive)
    {
        int32 Length = static_cast<int32>(AString.length());
        if (CaseSensitive)
        {
            for (int32 i=0; i<Length; i++)
                Add( (uint8)AString[i] );
        }
        else
        {
            for (int32 i=0; i<Length; i++)
                Add( (uint8)toupper(AString[i]) );
        }
    }

    __forceinline void __fastcall Clear()
    { 
        m_CheckSum = (uint32)~0;
    }

    __forceinline uint32 __fastcall GetCRC() const 
    { 
        return (~m_CheckSum); 
    }

	// Calculates the CRC of the given file; if the file
	// cannot be opened will return 0 (technically: all CIOExceptions
	// are caught)
	static uint32 GetFileCRC (const std::string& FilePath);

private:
    uint32  m_CheckSum;   
	static const uint32 CRC_Table[256];
};


} // namespace System
} // namespace WT
