/*
	MeshVtxProp.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include <limits>
#include "WT/Mesh/Mesh.h"
#include "WT/System/IO/BitStream.h"
#include "WT/Mesh/MeshFVA.h"
#include "WT/Mesh/MeshVtxProp.h"

using namespace WT::System::IO;
using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

bool CMeshVtxProp::SetPropType( int32 PropType )
{
    if (PropType < 0 || PropType > KVtxPropTypeMax)
    {
        return false;
    }

    m_PropType = PropType;
    return true;
}

bool CMeshVtxProp::SetDimensionIndex( int32 DimensionIndex )
{
    if (DimensionIndex < 0 || DimensionIndex > 65535)
    {
        return false;
    }
    
    m_DimensionIndex = DimensionIndex;
    return true;
}

bool CMeshVtxProp::GetValue(int32 VertexIndex, float32* pValue) const
{
    if (m_hFVA == NULL)
    {
        return false;
    }

    float32 Value = m_hFVA->GetValue( VertexIndex + m_VectorIndex, m_DimensionIndex );

    if (pValue != NULL)
    {
        *pValue = Value;
    }

    return true;
}

bool CMeshVtxProp::SetValue(int32 VertexIndex, float32 Value)
{
    if (m_hFVA == NULL)
    {
        return false;
    }

    m_hFVA->SetValue( VertexIndex + m_VectorIndex, m_DimensionIndex, Value );
	return true;
}

CMeshVtxProp::CMeshVtxProp()
	: m_PropType( KVtxPropTypeMax )
    , m_VectorIndex( 0 )
    , m_DimensionIndex( 0 )
{
}

uint32 const KPropertyTypeBits = 16;

void CMeshVtxProp::Write( CSerializationInfo* sInfo )
{
    //First, write the property type
	CStream* pStream = sInfo->BeginStreamBlock( "PropType" );
	assert( (1 << KPropertyTypeBits) > KVtxPropTypeMax );
	CBitStream Bits( pStream );
    Bits.WriteIntegerLittleEndian( m_PropType, KPropertyTypeBits );
    Bits.FlushBits();
	sInfo->EndStreamBlock();

	sInfo->AddValue( "FVA", m_hFVA.get() );

	sInfo->AddValue( "VectorIndex", m_VectorIndex );

    uint16 DimensionIndex;
	assert( m_DimensionIndex <= std::numeric_limits<uint16>::max() );
    DimensionIndex = static_cast<uint16>( m_DimensionIndex );
	sInfo->AddValue( "DimensionIndex", DimensionIndex );
}

void CMeshVtxProp::Read( CSerializationInfo* sInfo )
{
    //First, read the property type
	CStream* pStream = sInfo->BeginStreamBlock( "PropType" );
	assert( (1 << KPropertyTypeBits) > KVtxPropTypeMax );
    CBitStream Bits( pStream );
    m_PropType = Bits.ReadIntegerLittleEndian( KPropertyTypeBits );
    Bits.FlushBits();
	sInfo->EndStreamBlock();

	CMeshFVA* pFVA; sInfo->GetValue( "FVA", pFVA ); m_hFVA = pFVA;

	sInfo->GetValue( "VectorIndex", m_VectorIndex );

	uint16 DimensionIndex;
	sInfo->GetValue( "DimensionIndex", DimensionIndex );
	m_DimensionIndex = DimensionIndex;
}

} //namespace Graphics
} //namespace WT
