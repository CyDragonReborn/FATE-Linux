/*
	MeshFVA.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include <cmath>
#include <limits>
#include "WT/Mesh/Mesh.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/IO/BitStream.h"
#include "WT/System/IO/MemoryStream.h"
#include "WT/Mesh/VectorQuantization.h"
#include "WT/Mesh/MeshUA.h"
#include "WT/Mesh/MeshFVA.h"

using namespace WT::System::IO;
using namespace WT::System::Serialization;

namespace WT
{

namespace Graphics
{

WT_DEFINE_CLASS(CMeshFVA)

int32 const KMinNumBits = 1; //Minimum number of bits of precision for representing floats

int32 CMeshFVA::GetNumVectors() const
{
    return m_NumVectors;
}

int32 CMeshFVA::GetNumDimensions() const
{
    return m_NumDimensions;
}

bool CMeshFVA::SetNumVectors(int32 NumVectors)
{
    if (NumVectors < 0)
    {
        return false;
    }

    if (m_NumVectors != NumVectors)
    {
        m_NumVectors = NumVectors;

        if (m_NumDimensions != 0)
        {
            m_Data.resize( NumVectors * m_NumDimensions );
        
            //Zero out new data
            if (NumVectors > m_NumVectors)
            {
                memset( &m_Data[0] + m_NumVectors * m_NumDimensions, 0,
                    (NumVectors - m_NumVectors) * m_NumDimensions * sizeof(float32) );
            }

            m_NumVectors = NumVectors;
        }
    }

    return true;
}

bool CMeshFVA::RemoveVector(int32 VectorIndex)
{
    if (VectorIndex < 0 || VectorIndex >= m_NumVectors)
    {
        return false;
    }

    //For all vectors from the removed index to the next-to-last
    //one, copy the subsequent vector onto the current one.
    std::copy(&m_Data[(VectorIndex + 1) * m_NumDimensions], 
              &m_Data[m_NumVectors * m_NumDimensions],
              &m_Data[VectorIndex * m_NumDimensions]);

    m_Data.resize( --m_NumVectors * m_NumDimensions );

    return true;
}

bool CMeshFVA::CopyVector( int32 SourceIndex, int32 DestIndex )
{
	if (SourceIndex < 0 || SourceIndex >= m_NumVectors ||
		DestIndex < 0 || DestIndex >= m_NumVectors)
	{
		return false;
	}

	if (SourceIndex == DestIndex)
	{
		return true;
	}

	std::copy( &m_Data[SourceIndex * m_NumDimensions],
			   &m_Data[(SourceIndex + 1) * m_NumDimensions],
			   &m_Data[DestIndex * m_NumDimensions] );

	return true;
}

bool CMeshFVA::SetNumDimensions(int32 NumDimensions)
{
	if (NumDimensions < 0 || NumDimensions > std::numeric_limits<uint16>::max())
    {
        return false;
    }

    if (m_NumDimensions == NumDimensions)
    {
        return true;
    }

    //If the number of dimensions is shrinking, then compact array first
    if (m_NumDimensions > NumDimensions)
    {
        float32* pIn = &m_Data[0];
        float32* pOut = &m_Data[0];

        for (int32 i = 0; i < m_NumVectors; ++i)
        {
            for (int32 j = 0; j < m_NumDimensions; ++j)
            {
                *pOut++ = *pIn++;
            }

            pIn += (m_NumDimensions - NumDimensions);
        }
    }

    m_Data.resize( m_NumVectors * NumDimensions );

    //If the number of dimensions grew, then expand array
    if (m_NumDimensions < NumDimensions)
    {
        float32* pIn = &m_Data[m_NumVectors * m_NumDimensions];
        float32* pOut = &m_Data[m_NumVectors * NumDimensions];

        for (int32 i = 0; i < m_NumVectors; ++i)
        {
            int32 j;

            for (j = NumDimensions - 1; j >= m_NumDimensions; --j)
            {
                *--pOut = 0.0f;
            }

            for (; j >= 0; --j)
            {
                *--pOut = *--pIn;
            }
        }
    }

    m_NumDimensions = NumDimensions;

    return true;
}

bool CMeshFVA::RemoveDimension(int32 DimensionIndex)
{
    if (DimensionIndex < 0 || DimensionIndex >= m_NumDimensions)
    {
        return false;
    }

    //Rearrange data by moving some of the components up a dimension
    float32* pIn = &m_Data[0];
    float32* pOut = &m_Data[0];

    for (int32 i = 0; i < m_NumVectors; ++i)
    {
        for (int32 j = 0; j < m_NumDimensions; ++j)
        {
            if (j != DimensionIndex)
            {
                *pOut++ = *pIn;
            }
            ++pIn;
        }
    }

    --m_NumDimensions;
    m_Data.resize( m_NumVectors * m_NumDimensions );

    return true;
}

CMeshFVA::CMeshFVA()
	: m_StorageType( 0 )
	, m_NumVQDim( 0 )
	, m_Precision( 8 )
	, m_bUseVQ( true )
	, m_bAllowIndex( true )
{
    m_NumVectors = 0;
    m_NumDimensions = 0;
}

CMeshFVA::~CMeshFVA()
{
}

void CMeshFVA::SetObjectData( CSerializationInfo* sInfo, CSerializationContext* sContext )
{
	sInfo->GetValue( "StorageType", m_StorageType );

    switch (m_StorageType)
    {
    default:
		throw std::invalid_argument( "corrupt mesh FVA stream" );

    case 0: //Raw data
        {
			int32 NumVectors;
			uint16 NumDimensions;

			sInfo->GetValue( "NumVectors", NumVectors );
			sInfo->GetValue( "NumDimensions", NumDimensions );

			m_NumVectors = NumVectors;
			m_NumDimensions = NumDimensions;
           
            m_Data.resize(m_NumVectors * m_NumDimensions );

			sInfo->GetContainerBegin( "Data" );
			for (int32 i = 0; i < m_NumVectors * m_NumDimensions; ++i)
			{
				sInfo->GetValue( L"item", m_Data[i] );
			}
			sInfo->GetContainerEnd();
        }
        break;

    case 1: //Fixed-point data
        {
			int32 NumVectors;
			uint16 NumDimensions;

			sInfo->GetValue( "NumVectors", NumVectors );
			sInfo->GetValue( "NumDimensions", NumDimensions );

			m_NumVectors = NumVectors;
			m_NumDimensions = NumDimensions;

			std::vector<float32> Mins( m_NumDimensions );
			std::vector<float32> Maxs( m_NumDimensions );

			sInfo->GetContainerBegin( "Extents" );
			for (int32 i = 0; i < m_NumDimensions; ++i)
			{
				sInfo->GetValue( "Min", Mins[i] );
				sInfo->GetValue( "Max", Maxs[i] );
			}
			sInfo->GetContainerEnd();

			uint8 NumBits;
			sInfo->GetValue( "NumBits", NumBits );
            
            m_Data.resize( m_NumVectors * m_NumDimensions );

            //Compute the inverse of the maximum integer value representable with the number of bits
			CStream* pStream = sInfo->BeginStreamBlock( "Bits" );
            float32 FixedInverse = 1.0f / ((1 << NumBits) - 1);
            CBitStream Bits( pStream );
            for (int32 i = 0; i < m_NumVectors; ++i)
            {
                for (int32 j = 0; j < m_NumDimensions; ++j)
                {
                    uint32 Fixed = Bits.ReadIntegerLittleEndian( NumBits );

                    float32 Value = Fixed * FixedInverse; 
                    Value = Mins[j] + (Maxs[j] - Mins[j]) * Value;
                    m_Data[i * m_NumDimensions + j] = Value;
                }
            }
            Bits.FlushBits();
			sInfo->EndStreamBlock();
        }
        break;

    case 2: //Vector-quantized data
        {
			int32 NumVectors;
			uint16 NumDimensions;

			sInfo->GetValue( "NumVectors", NumVectors );
			sInfo->GetValue( "NumDimensions", NumDimensions );

			m_NumVectors = NumVectors;
            m_NumDimensions = NumDimensions;

			m_Mins.resize( m_NumDimensions );
			m_Maxs.resize( m_NumDimensions );

			sInfo->GetContainerBegin( "Extents" );
			for (int32 i = 0; i < m_NumDimensions; ++i)
			{
				sInfo->GetValue( "Min", m_Mins[i] );
				sInfo->GetValue( "Max", m_Maxs[i] );
			}
			sInfo->GetContainerEnd();

			sInfo->GetValue( "NumVQDim", m_NumVQDim );

			CMeshFVA* pCodebook;
			sInfo->GetValue( "Codebook", pCodebook );
			m_hCodebook = pCodebook;

			CMeshUA* pMap;
			sInfo->GetValue( "Map", pMap );
			m_hMap = pMap;
        }
        break;
    }
}

void CMeshFVA::FinalizeDeserialization( CSerializationInfo* sInfo, CSerializationContext* sContext )
{
	//_BaseClass::FinalizeDeserialization( sInfo, sContext );

	if (m_StorageType == 2)
	{
		if ((m_hCodebook != NULL && !sInfo->CSerializationInfo::ObjectIsFinalized( m_hCodebook )) ||
			(m_hMap		 != NULL && !sInfo->CSerializationInfo::ObjectIsFinalized( m_hMap )))
		{
			throw std::runtime_error( "serialization cycle error" );
		}

		if (m_hCodebook == NULL || m_hMap == NULL)
		{
			throw std::invalid_argument( "corrupt mesh FVA stream" );
		}

		if (m_hCodebook->GetNumDimensions() < m_NumVQDim)
		{
			throw std::invalid_argument( "corrupt mesh FVA stream" );
		}
	    
		if (m_hMap->GetNumValues() < (m_NumVectors * m_NumDimensions + m_NumVQDim - 1) / m_NumVQDim)
		{
			throw std::invalid_argument( "corrupt mesh FVA stream" );
		}

		m_Data.resize( m_NumVectors * m_NumDimensions );


		float32 const *const pMin = &m_Mins[0];
		float32 const *const pMax = &m_Maxs[0];

		std::vector<float32> Delta( m_NumDimensions );
		float32 *const pDelta = &Delta[0];
		for (int32 i = 0; i < m_NumDimensions; ++i)
		{
			pDelta[i] = pMax[i] - pMin[i];
		}

		float32* pOut = &m_Data[0];
		int32 OutputDim = 0;
		int32 MapIndex = 0;

		for (int32 OutputVector = 0; OutputVector < m_NumVectors;)
		{
			uint32 CodewordIndex = 0;
			m_hMap->GetValue( MapIndex++, &CodewordIndex );

			for (int32 j = 0; j < m_NumVQDim; ++j)
			{
				float32 const Value = m_hCodebook->GetValue( CodewordIndex, j );
				*pOut++ = pMin[OutputDim] + pDelta[OutputDim] * Value;

				if (++OutputDim >= m_NumDimensions)
				{
					OutputDim = 0;
					if (++OutputVector >= m_NumVectors)
					{
						break;
					}
				}
			}
		}

		//Release un-needed deserialization-related objects
		m_hMap = NULL;
		m_hCodebook = NULL;
	}
}

void CMeshFVA::GetObjectData( CSerializationInfo* sInfo, CSerializationContext* sContext ) const
{
	CMesh::SerializationParameters* pParameters = reinterpret_cast<CMesh::SerializationParameters*>( 
		sContext->GetProperties()->GetValue<uint64>( "MeshParameters" ) );
	assert( pParameters != NULL );

	if (m_bUseVQ)
    {
		WriteWithVectorQuantization( sInfo );
	} else if (m_Precision < std::numeric_limits<float32>::digits) {
		WriteFixedPoint( sInfo, pParameters );
	} else {
		WriteRaw( sInfo );
	}
}

void CMeshFVA::WriteWithVectorQuantization( CSerializationInfo* sInfo ) const
{
	//First compute minimum and maximum of values for each dimension
	std::vector<float32> Mins( m_NumDimensions, std::numeric_limits<float32>::max() );
	std::vector<float32> Maxs( m_NumDimensions, -std::numeric_limits<float32>::max() );

	for (int32 j = 0; j < m_NumDimensions; ++j)
	{
		for (int32 i = 0; i < m_NumVectors; ++i)
		{
			float32 Value = m_Data[i * m_NumDimensions + j];

			if (Mins[j] > Value)
			{
				Mins[j] = Value;
			}

			if (Maxs[j] < Value)
			{
				Maxs[j] = Value;
			}
		}
	}

	//Compute total number of floating point values
	int32 NumFloats = m_NumVectors * m_NumDimensions;

	//TODO choose number of VQ dimensions to work with
	int32 const VQDimensions = KVQDimensions;

	//Compute number of input VQ vectors
	int32 const NumVQInputVectors = (NumFloats + VQDimensions - 1) / VQDimensions;

	//TODO choose a codebook size
	int32 CodebookSize = Float32ToInt32( 4.0f * sqrt( static_cast<float32>( NumVQInputVectors ) ) );
	if (CodebookSize > NumVQInputVectors)
	{
		CodebookSize = NumVQInputVectors;
	}

	//Align data to 16-byte boundaries for SSE optimization
	std::vector<uint8> VQInputVectorRawData
		( NumVQInputVectors * VQDimensions * sizeof(float32) + (System::KSSEAlignmentSize - 1) );
	float32* VQInputVectorData = static_cast<float32*>( System::SSEAlign( &VQInputVectorRawData[0] ) );

	//Place scaled down floating-point data into the VQ input vector data array
	for (int32 i = 0; i < m_NumVectors; ++i)
	{
		for (int32 j = 0; j < m_NumDimensions; ++j)
		{
			float32 Value;

			if (Mins[j] == Maxs[j])
			{
				Value = 0.5f;
			} else {
				Value = m_Data[i * m_NumDimensions + j];
				Value = (Value - Mins[j]) / (Maxs[j] - Mins[j]);
			}

			VQInputVectorData[i * m_NumDimensions + j] = Value;
		}
	}

	//Fill the remaining portion of the VQ input vector data array with arbitrary values (e.g. 0.5)
	std::fill( VQInputVectorData + NumFloats, VQInputVectorData + NumVQInputVectors * VQDimensions, 0.5f );

	//Align data to 16-byte boundaries for SSE optimization
	std::vector<uint8> CodebookRaw( CodebookSize * VQDimensions * sizeof(float32) + (System::KSSEAlignmentSize - 1) );
	float32* Codebook = static_cast<float32*>( System::SSEAlign( &CodebookRaw[0] ) );

	/*float32 ResultDistortion =*/ QuantizeVectorsLBG(
		NumVQInputVectors, &VQInputVectorData[0], CodebookSize, &Codebook[0], 
			KDefaultLBGTolerance, KDefaultLBGMaxIter );

	//Create the codebook object
	HMeshFVA hCodebook = new CMeshFVA();
	hCodebook->SetPrecision( m_Precision );
	hCodebook->m_bAllowIndex = m_bAllowIndex;
	hCodebook->m_bUseVQ = false; //Disable vector quantization to avoid infinite loops

	hCodebook->SetNumVectors(CodebookSize);
	hCodebook->SetNumDimensions(VQDimensions);
	for (int32 i = 0; i < CodebookSize; ++i)
	{
		for (int32 j = 0; j < VQDimensions; ++j)
		{
			hCodebook->SetValue(i, j, Codebook[i * VQDimensions + j]);
		}
	}

	//Create the map object
	HMeshUA hMap = new CMeshUA();

	hMap->SetNumValues(NumVQInputVectors);
	for (int32 i = 0; i < NumVQInputVectors; ++i)
	{
		int32 CodewordIndex = 0;
		
		int32 BestDistortion;
		*(float32*)&BestDistortion = std::numeric_limits<float32>::max();

		float32 const* pInput = &VQInputVectorData[i * VQDimensions];
		float32 const* pCodebook = &Codebook[0];

		for (int32 j = CodebookSize - 1; j >= 0; --j)
		{
			int32 Distortion;
			*(float32*)&Distortion = ComputeDistortion( &pCodebook[j * VQDimensions], pInput );

			if (Distortion < BestDistortion)
			{
				CodewordIndex = j;
				BestDistortion = Distortion;
			}
		}

		hMap->SetValue(i, CodewordIndex);
	}

	uint8 StorageType = 2; //VQ compression
	sInfo->AddValue( "StorageType", StorageType );
	sInfo->AddValue( "NumVectors", m_NumVectors );

	uint16 NumDimensions;
	assert( m_NumDimensions <= std::numeric_limits<uint16>::max() );
	NumDimensions = static_cast<uint16>( m_NumDimensions );
	sInfo->AddValue( "NumDimensions", NumDimensions );

	//Write the extents
	sInfo->AddContainerBegin( "Extents" );
	for (int32 j = 0; j < m_NumDimensions; ++j)
	{
		sInfo->AddValue( "Min", Mins[j] );
		sInfo->AddValue( "Max", Maxs[j] );
	}
	sInfo->AddContainerEnd();

	uint16 OutVQDim;
	assert( VQDimensions <= std::numeric_limits<uint16>::max() );
	OutVQDim = static_cast<uint16>( VQDimensions );
	sInfo->AddValue( "NumVQDim", OutVQDim );

	//Write out the codebook, which also is an FVA
	sInfo->AddValue( "Codebook", hCodebook.get() );

	//Write out the map
	sInfo->AddValue( "Map", hMap.get() );
}

void CMeshFVA::WriteFixedPoint( CSerializationInfo* sInfo, CMesh::SerializationParameters* pParameters ) const
{
    uint8 NumBits = m_Precision;
	if (NumBits < KMinNumBits)
    {
        NumBits = KMinNumBits;
    }

	//Get minimum and maximum value for each dimension
	std::vector<float32> Mins( m_NumDimensions, std::numeric_limits<float32>::max() );
	std::vector<float32> Maxs( m_NumDimensions, -std::numeric_limits<float32>::max() );

    for (int32 j = 0; j < m_NumDimensions; ++j)
    {
        for (int32 i = 0; i < m_NumVectors; ++i)
        {
            float32 Value = m_Data[i * m_NumDimensions + j];

            if (Mins[j] > Value)
            {
                Mins[j] = Value;
            }

            if (Maxs[j] < Value)
            {
                Maxs[j] = Value;
            }
        }
    }

	std::vector<float32> NewData( m_NumVectors * m_NumDimensions );

	//Scale data to be in [0,1] range
    uint32 Factor = (1 << NumBits) - 1;
	float32 InverseFactor = 1.0f / Factor;
    for (int32 i = 0; i < m_NumVectors; ++i)
    {
        for (int32 j = 0; j < m_NumDimensions; ++j)
        {
			float32 Value = 0.5f;
            if (Maxs[j] != Mins[j])
            {
                Value = (m_Data[i * m_NumDimensions + j] - Mins[j]) / (Maxs[j] - Mins[j]);
            }

			//Take precision loss into account to enable more merging
			NewData[i * m_NumDimensions + j] = Float32ToInt32( Value * Factor ) * InverseFactor;
		}
	}

	//Check if merging duplicates pays off
    if (pParameters->bUseIndexedData && m_bAllowIndex )
	{
		int32 NumUniqueVectors;
		std::vector<int32> Remap;
		DoIndexing( &NewData[0], NumUniqueVectors, Remap );

		//Estimate whether the indexed data will be smaller than the map data
		//TODO refine this estimate
		if ((m_NumVectors - NumUniqueVectors) * m_NumDimensions /*size saved from codebook*/ >
			m_NumVectors /*int cost for extra map*/)
		{
			WriteFixedPointIndexed( sInfo, &Mins[0], &Maxs[0], &NewData[0], NumUniqueVectors, Remap );
			return;
		}
	}

	uint8 const StorageType = 1; //Fixed-point data
	sInfo->AddValue( "StorageType", StorageType );
	sInfo->AddValue( "NumVectors", m_NumVectors );
	assert( m_NumDimensions <= std::numeric_limits<uint16>::max() );
	uint16 NumDim = static_cast<uint16>( m_NumDimensions );
	sInfo->AddValue( "NumDimensions", NumDim );

	sInfo->AddContainerBegin( "Extents" );
	for (int32 j = 0; j < m_NumDimensions; ++j)
	{
		sInfo->AddValue( "Min", Mins[j] );
		sInfo->AddValue( "Max", Maxs[j] );
	}
	sInfo->AddContainerEnd();
	
	sInfo->AddValue( "NumBits", NumBits );

	CStream* pStream = sInfo->BeginStreamBlock( "Bits" );
	CBitStream Bits( pStream );
	for (int32 i = 0; i < m_NumVectors; ++i)
	{
		for (int32 j = 0; j < m_NumDimensions; ++j)
		{
            int32 iValue = Float32ToInt32( NewData[i * m_NumDimensions + j] * Factor );

            if (iValue < 0)
            {
                iValue = 0;
            } else if (static_cast<uint32>( iValue ) > Factor) {
                iValue = Factor;
            }

			Bits.WriteIntegerLittleEndian( iValue, NumBits );
		}
	}
	Bits.FlushBits();
	sInfo->EndStreamBlock();
}

void CMeshFVA::WriteRaw( CSerializationInfo* sInfo ) const
{
    uint8 const StorageType = 0; //Raw data
	sInfo->AddValue( "StorageType", StorageType );
    sInfo->AddValue( "NumVectors", m_NumVectors );
	assert( m_NumDimensions <= std::numeric_limits<uint16>::max() );
    uint16 NumDim = static_cast<uint16>( m_NumDimensions );
    sInfo->AddValue( "NumDimensions", NumDim );

	sInfo->AddContainerBegin( "Data" );
    for (int32 i = 0; i < m_NumVectors; ++i)
    {
        for (int32 j = 0; j < m_NumDimensions; ++j)
        {
            float32 Value = m_Data[i * m_NumDimensions + j];
			sInfo->AddValue( "item", Value );
        }
    }
	sInfo->AddContainerEnd();
}

void CMeshFVA::DoIndexing( float32* pNewData, int32& NumUniqueVectors, std::vector<int32>& Remap ) const
{
	assert( pNewData != NULL );

	//First, compute hash values for all vectors
	std::vector<int32> Hashes( m_NumVectors );

	for (int32 i = m_NumVectors - 1; i >= 0; --i)
	{
		int32 Hash = 0;
		float32 const* pData = &pNewData[i * m_NumDimensions];
		for (int32 j = m_NumDimensions - 1; j >= 0; --j)
		{
			Hash ^= *reinterpret_cast<int32 const*>( pData + j );
		}
		Hashes[i] = Hash;
	}

	//Next, find duplicates
	NumUniqueVectors = m_NumVectors;
	Remap.resize( m_NumVectors );
	for (int32 i = m_NumVectors - 1; i >= 0; --i)
	{
		Remap[i] = i;
		
		//Look for a duplicate of i
		float32 const* pData1 = &pNewData[i * m_NumDimensions];
		for (int32 j = i - 1; j >= 0; --j)
		{
			if (Hashes[i] == Hashes[j] &&
				std::equal( pData1, pData1 + m_NumDimensions, &pNewData[j * m_NumDimensions] ))
			{
				Remap[i] = j;
				--NumUniqueVectors;
				break;
			}
		}
	}

	Hashes.clear(); //No use for this anymore
}


void CMeshFVA::WriteFixedPointIndexed( CSerializationInfo* sInfo,
									  float32* pMin, float32* pMax, float32* pNewData, 
									  int32 NumUniqueVectors, std::vector<int32>& Remap ) const
{
	assert( pMin != NULL );
	assert( pMax != NULL );

	//Create the codebook object
	HMeshFVA hCodebook = new CMeshFVA();
	hCodebook->SetPrecision( m_Precision );
	hCodebook->SetUseVQ( m_bUseVQ );
	hCodebook->m_bAllowIndex = false; //Disable indexes to avoid infinite loops

	hCodebook->SetNumVectors( NumUniqueVectors );
	hCodebook->SetNumDimensions( m_NumDimensions );

	int32 NumVecsDeleted = 0;
	for (int32 i = 0; i < m_NumVectors; ++i)
	{
		if (Remap[i] == i)
		{
			Remap[i] -= NumVecsDeleted; //Adjust for deleted prior vectors

			//Move the preserved data into the codebook
			int32 DestIndex = Remap[i];
			for (int32 j = 0; j < m_NumDimensions; ++j)
			{
				hCodebook->SetValue( DestIndex, j, pNewData[i * m_NumDimensions + j] );
			}

		} else {
			++NumVecsDeleted; //All vectors numbered above i have to be adjusted further
			Remap[i] = Remap[Remap[i]]; //Get actual vector index
		}
	}

	//Create the map object
	HMeshUA hMap = new CMeshUA();

	hMap->SetNumValues( m_NumVectors );
	for (int32 i = 0; i < m_NumVectors; ++i)
	{
		hMap->SetValue( i, Remap[i] );
	}

	uint8 const StorageType = 2; //VQ compression
	sInfo->AddValue( "StorageType", StorageType );
	sInfo->AddValue( "NumVectors", m_NumVectors );

	uint16 NumDimensions;
	assert( m_NumDimensions <= std::numeric_limits<uint16>::max() );
	NumDimensions = static_cast<uint16>( m_NumDimensions );
	sInfo->AddValue( "NumDimensions", NumDimensions );

	//Write the extents
	sInfo->AddContainerBegin( "Extents" );
	for (int32 j = 0; j < m_NumDimensions; ++j)
	{
		sInfo->AddValue( "Min", pMin[j] );
		sInfo->AddValue( "Max", pMax[j] );
	}
	sInfo->AddContainerEnd();

	sInfo->AddValue( "NumVQDim", NumDimensions );

	//Write out the codebook, which is also an FVA
	sInfo->AddValue( "Codebook", hCodebook.get() );

	//Write out the map
	sInfo->AddValue( "Map", hMap.get() );
}

/// Compute an estimate of the serialized size in bytes.
int32 CMeshFVA::EstimateSerializedSize( CMesh::SerializationParameters* pParameters ) const
{
	assert(pParameters != NULL);
	{
		return 0;
	}

	uint32 TotalSize = 0;

	int32 NumDimensions = m_NumDimensions;
	int32 NumFloats = m_NumDimensions * m_NumVectors;

	//If vector quantization is used
	if (m_bUseVQ)
	{
		int32 const VQDimensions = KVQDimensions;

		//Compute number of input VQ vectors
		int32 const NumVQInputVectors = (NumFloats + VQDimensions - 1) / VQDimensions;

		//TODO choose a codebook size
		int32 CodebookSize = Float32ToInt32( 4.0f * sqrt( static_cast<float32>( NumVQInputVectors ) ) );
		if (CodebookSize > NumVQInputVectors)
		{
			CodebookSize = NumVQInputVectors;
		}

		//Add in the size for the vector quantization FVA chunk
		TotalSize += 29 + 8 * m_NumDimensions;

		//Compute the number of bits required to represent a codebook index
		int32 NumBits = 0;
		while ((1 << NumBits) < CodebookSize)
		{
			++NumBits;
		}

		//Add in the size of the UA chunk (which provides the codebook mapping)
		if (pParameters->bUseHuffmanCompression)
		{
			//Assume Huffman compression significantly reduces the number of bits of data required
			//even after including the code lengths array
			TotalSize += 29 + (NumVQInputVectors * NumBits + 7) / 16;
		} else {
			TotalSize += 18 + (NumVQInputVectors * NumBits + 7) / 8;
		}
		
		//The referenced FVA chunk uses a reduced number of floats
		NumDimensions = VQDimensions;
		NumFloats = VQDimensions * CodebookSize;
	} else if (pParameters->bUseIndexedData && m_bAllowIndex) {
		//If indexed data is to be used

		NumFloats >>= 1; //Assume only a portion of the vectors are unique
	}

	if (m_Precision < std::numeric_limits<float32>::digits)
	{
		//If fixed-point values will be used
		TotalSize += 20 + NumDimensions * 8 + (NumFloats * m_Precision + 7) / 8;
	} else {
		//Straight floating-point values will be written
		TotalSize += 19 + 4 * NumFloats;
	}

	return TotalSize;
}

} //namespace Graphics
} //namespace WT
