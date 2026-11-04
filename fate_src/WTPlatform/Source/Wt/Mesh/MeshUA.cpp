/*
	CMeshUA.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include <limits>
#include "WT/Mesh/Mesh.h"
#include "WT/System/IO/BitStream.h"
#include "WT/System/IO/MemoryStream.h"
#include "WT/Mesh/HuffmanEncoding.h"
#include "WT/Mesh/MeshUA.h"

using namespace WT::System::IO;
using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS(CMeshUA)

int32 CMeshUA::GetNumValues() const
{
    return static_cast<int32>( m_Values.size() );
}

bool CMeshUA::GetValue(int32 Index, uint32* pValue) const
{
    if (Index < 0 || static_cast<uint32>(Index) >= m_Values.size())
    {
        return false;
    }

    if (pValue != NULL)
    {
        *pValue = m_Values[Index];   
    }

    return true;
}

bool CMeshUA::SetNumValues(int32 NumValues)
{
    if (NumValues < 0)
    {
        return false;
    }

    m_Values.resize(NumValues);
    return true;
}

bool CMeshUA::SetValue(int32 Index, uint32 Value)
{
    if (Index < 0 || static_cast<uint32>(Index) >= m_Values.size())
    {
        return false;
    }

    m_Values[Index] = Value;
    return true;
}

CMeshUA::CMeshUA()
{
}

CMeshUA::~CMeshUA()
{
}

void CMeshUA::GetObjectData( CSerializationInfo* sInfo, CSerializationContext* sContext ) const
{
	CMesh::SerializationParameters* pParameters = reinterpret_cast<CMesh::SerializationParameters*>( 
		sContext->GetProperties()->GetValue<uint64>( "MeshParameters" ) );
	assert( pParameters != NULL );

    //Compute the number of bits necessary to store any value
    uint32 MaxValue = 0;
    for (uint32 i = 0; i < m_Values.size(); ++i)
    {
        if (m_Values[i] > MaxValue)
        {
            MaxValue = m_Values[i];                            
        }
    }

    uint8 NumBits = 0;
    while ((1u << NumBits) <= MaxValue)
	{
		++NumBits;
	}

    if (pParameters->bUseHuffmanCompression)
    {
        //Use Huffman encoding
        uint32 LengthsLength = 0; //Number of bits of "lengths" data
        uint32 DataLength = 0; //Number of bits of Huffman-encoded data

		std::vector<uint8> Lengths( ((MaxValue + 1) * (6 + NumBits) + 7) / 8 );

		std::vector<uint8> OutData( (m_Values.size() * NumBits + 7) / 8 );

        {
			std::vector<int32> Frequencies( MaxValue + 1, 0 );
			std::vector<int32> CodeLengths( MaxValue + 1 );
			std::vector<int32> HuffCodes( MaxValue + 1 );

            for (uint32 i = 0; i < m_Values.size(); ++i)
            {
                assert( m_Values[i] >= 0 );
                assert( m_Values[i] <= MaxValue );
                ++Frequencies[m_Values[i]];
            }

            ComputeHuffmanCodes(MaxValue + 1, &Frequencies[0], &CodeLengths[0], &HuffCodes[0]);

//			pParameters->YieldGraph();

            //If there is more than a single value
            if (MaxValue)
            {
                //Write the length of each code, minus 1. Use a MTF (move-to-front) scheme.
                //Write bits based on the order of lengths in the MTF array as follows:
                //0 => 0
                //1 => 10
                //2 => 110
                //3 => 1110
                //4 => 11110
                //5 => 111110
                //anything else => 111111 followed by NumCodewordIndexBits bits
                //representing the position.

				std::vector<int32> MTFs( MaxValue + 1 );

                for (uint32 i = 0; i < MaxValue + 1; ++i)
                {
                    MTFs[i] = 1 + i;
                }

				CStream * pMemStream = new CMemoryStream( &Lengths[0], static_cast<uint32>(Lengths.size()) );
                CBitStream LengthsStream( pMemStream );
                for (uint32 i = 0; i <= MaxValue; ++i)
                {
                    uint32 k;
                    for (k = 0; k < MaxValue + 1; ++k)
                    {
                        if (MTFs[k] == CodeLengths[i]) break;
                    }

                    assert( k < (1u << NumBits) );

                    switch (k)
                    {
                    case 0: LengthsStream.WriteIntegerLittleEndian(0, 1); LengthsLength++; break;
                    case 1: LengthsStream.WriteIntegerLittleEndian(1, 2); LengthsLength += 2; break;
                    case 2: LengthsStream.WriteIntegerLittleEndian(3, 3); LengthsLength += 3; break;
                    case 3: LengthsStream.WriteIntegerLittleEndian(7, 4); LengthsLength += 4; break;
                    case 4: LengthsStream.WriteIntegerLittleEndian(15, 5); LengthsLength += 5; break;
                    case 5: LengthsStream.WriteIntegerLittleEndian(31, 6); LengthsLength += 6; break;
                    default: 
                        LengthsStream.WriteIntegerLittleEndian(63, 6); 
                        LengthsStream.WriteIntegerLittleEndian(k, NumBits); 
                        LengthsLength += 6 + NumBits;
                        break;
                    }

                    for (int32 m = k; m > 0; --m)
                    {
                        MTFs[m] = MTFs[m - 1];
                    }

                    MTFs[0] = CodeLengths[i];
                }
                LengthsStream.FlushBits();
				SafeDelete(pMemStream);
            }

            //Write the Huffman code for each input.
			CStream* pMemStream = new CMemoryStream( &OutData[0], static_cast<uint32>(OutData.size()) );
            CBitStream DataStream( pMemStream );
            for (uint32 i = 0; i < m_Values.size(); ++i)
            {
                uint32 Value = m_Values[i];
                DataStream.WriteIntegerBigEndian( HuffCodes[Value], CodeLengths[Value] );
                DataLength += CodeLengths[Value];
            }
            DataStream.FlushBits();
			SafeDelete(pMemStream);
        }

        //Convert from bits to bytes
        LengthsLength = (LengthsLength + 7) / 8;
        DataLength = (DataLength + 7) / 8;

        uint8 const StorageType = 1; //Compressed data
        sInfo->AddValue( "StorageType", StorageType );
        uint32 NumValues = static_cast<uint32>( m_Values.size() );
        sInfo->AddValue( "NumValues", NumValues );
        sInfo->AddValue( "MaxValue", MaxValue );

		CStream* pStream;

        sInfo->AddValue( "LengthsLength", LengthsLength );
		pStream = sInfo->BeginStreamBlock( "Lengths" );
		pStream->Write( &Lengths[0], LengthsLength );
		sInfo->EndStreamBlock();

        sInfo->AddValue( "DataLength", DataLength );
		pStream = sInfo->BeginStreamBlock( "Data" );
		pStream->Write( &OutData[0], DataLength );
		sInfo->EndStreamBlock();

    } else {

        uint8 const StorageType = 0; //Raw data
		sInfo->AddValue( "StorageType", StorageType );
        uint32 NumValues = static_cast<uint32>( m_Values.size() );
        sInfo->AddValue( "NumValues", NumValues );
        sInfo->AddValue( "NumBits", NumBits );

		CStream* pStream = sInfo->BeginStreamBlock( "Bits" );
        CBitStream Bits( pStream );
        for (uint32 i = 0; i < m_Values.size(); ++i)
        {
            Bits.WriteIntegerLittleEndian( m_Values[i], NumBits );
        }
        Bits.FlushBits();
		sInfo->EndStreamBlock();
    }
}

struct HuffmanNode
{
	HuffmanNode* m_Children[2];
	int32 m_Code;

	HuffmanNode() : m_Code( -1 )
	{
		m_Children[0] = NULL;
		m_Children[1] = NULL;
	}

	void Validate()
	{
		assert( (m_Children[0] == NULL) == (m_Children[1] == NULL) );
		if (m_Children[0] != NULL)
		{
			m_Children[0]->Validate();
			m_Children[1]->Validate();
		}
	}
};

void CMeshUA::SetObjectData( CSerializationInfo* sInfo, CSerializationContext* sContext )
{
	uint8 StorageType;
	sInfo->GetValue( "StorageType", StorageType );

	switch (StorageType)
    {
    default:
		throw std::out_of_range( "mesh UA storage type in stream" );

    case 0: //Raw data
        {
			uint32 NumValues;
            uint8 NumBits;

			sInfo->GetValue( "NumValues", NumValues );
			sInfo->GetValue( "NumBits", NumBits );

			m_Values.resize( NumValues );

			CStream* pStream = sInfo->BeginStreamBlock( "Bits" );
            CBitStream Bits( pStream );
            for (uint32 i = 0; i < NumValues; ++i)
            {
				m_Values[i] = Bits.ReadIntegerLittleEndian( NumBits );
            }
            Bits.FlushBits();
			sInfo->EndStreamBlock();
        }
        break;

    case 1: //Compressed data
        {
            uint32 NumValues;
            uint32 MaxValue;
            uint32 LengthsLength;

			sInfo->GetValue( "NumValues", NumValues );
			sInfo->GetValue( "MaxValue", MaxValue );

			sInfo->GetValue( "LengthsLength", LengthsLength );

			uint32 NumBits = 0;
            while ((1u << NumBits) <= MaxValue)
			{
				++NumBits;
			}

            //Extract the Huffman code lengths

			std::vector<uint32> HuffCodes( MaxValue + 1 );
			std::vector<uint32> HuffLengths( MaxValue + 1 );
            uint32 GreatestLength = 1; //Greatest length of code in bits
            uint32 LeastLength = std::numeric_limits<uint32>::max(); //Least length of code in bits

            //If there is more than a single value
            if (MaxValue)
            {
                //Element i shows number of occurances of length i - 1
				std::vector<uint32> NumLengths( MaxValue + 1, 0 );

				std::vector<uint32> MTFs( MaxValue + 1 );
                for (uint32 i = 0; i < MaxValue + 1; ++i)
                {
                    MTFs[i] = i + 1;
                }

				CStream* pStream = sInfo->BeginStreamBlock( "Lengths" );
                CBitStream Lengths( pStream );
                for (uint32 i = 0; i <= MaxValue; ++i)
                {
                    uint32 k;
                    for (k = 0; k < 6; k++)
                    {
                        if (!Lengths.ReadBit())
                        {
                            break;
                        }
                    }

                    if (k >= 6)
                    {
                        //Read explicit MTF position
						k = Lengths.ReadIntegerLittleEndian( NumBits );
                    }

                    HuffLengths[i] = MTFs[k];
                    for (int32 j = k; j > 0; --j)
                    {
                        MTFs[j] = MTFs[j - 1];
                    }
                    MTFs[0] = HuffLengths[i];

                    ++NumLengths[HuffLengths[i] - 1];
                    if (HuffLengths[i] > GreatestLength)
                    {
                        GreatestLength = HuffLengths[i];
                    }

                    if (HuffLengths[i] < LeastLength)
                    {
                        LeastLength = HuffLengths[i];
                    }
                }
                Lengths.FlushBits();
				sInfo->EndStreamBlock();

                //Compute cannonical Huffman codes
				std::vector<uint32> Firsts( MaxValue + 1 );
                Firsts[GreatestLength - 1] = 0;
                for (int32 i = GreatestLength - 2; i >= 0; --i)
                {
                    Firsts[i] = (Firsts[i + 1] + NumLengths[i + 1]) / 2;
                }

                for (uint32 i = 0; i <= MaxValue; ++i)
                {
                    HuffCodes[i] = Firsts[HuffLengths[i] - 1];
                    ++Firsts[HuffLengths[i] - 1];
                }

            } else {
                HuffLengths[0] = 0;
                HuffCodes[0] = 0;
                GreatestLength = 0;
            }
            
			uint32 const MaxNodes = 2 * (MaxValue + 1) - 1; //Maximum number of nodes for binary tree
			std::vector<HuffmanNode> HuffmanNodes( MaxNodes );
			uint32 NumNodes = 0;

			assert( NumNodes < MaxNodes );
			HuffmanNode* pRoot = &HuffmanNodes[NumNodes++];
			
			for (uint32 i = 0; i < MaxValue + 1; ++i)
			{
				uint32 const Code = HuffCodes[i];
				uint32 const Length = HuffLengths[i];
				HuffmanNode* pNode = pRoot;

				bool Created = true;
				for (uint32 j = 0; j < Length; ++j)
				{
					uint32 Bit;

					int32 Shift = Length - 1 - j;
					if (Shift >= 32)
					{
						Bit = 0;
					} else {
						Bit = (Code & (1 << Shift)) ? 1 : 0;
					}

					Created = false;
					if (pNode->m_Children[Bit] == NULL)
					{
						Created = true;
						assert( NumNodes < MaxNodes );
						pNode->m_Children[Bit] = &HuffmanNodes[NumNodes++];
					}

					pNode = pNode->m_Children[Bit];
				}

				assert( Created == true );
				pNode->m_Code = i;
			}

			pRoot->Validate();

			//Skip data length element
			uint32 DataLength;
			sInfo->GetValue( "DataLength", DataLength );

            m_Values.resize( NumValues );

			CStream* pStream = sInfo->BeginStreamBlock( "Data" );
            CBitStream Data( pStream );
            for (uint32 i = 0; i < NumValues; ++i)
            {
				HuffmanNode* pNode = pRoot;
				for (;;)
                {
					assert( (pNode->m_Children[0] == NULL) == (pNode->m_Children[1] == NULL) );
					if (pNode->m_Children[0] == NULL)
					{
						break;
					}

                    bool const Bit = Data.ReadBit();
					pNode = pNode->m_Children[Bit];
                }

				assert( pNode->m_Code != -1 );
                m_Values[i] = pNode->m_Code;
            }
            Data.FlushBits();
			sInfo->EndStreamBlock();
        }
        break;
    }
}

} //namespace Graphics
} //namespace WT
