/*
	MeshVtxPropCol.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	7/19/2004

*/

#include "WT/stdafx.h"

#include "WT/Mesh/Mesh.h"
#include "WT/Mesh/MeshVtxProp.h"
#include "WT/Mesh/MeshVtxPropCol.h"

using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

CMeshVtxProp* CMeshVtxPropCol::GetProperty(int32 Index)
{
    if (Index < 0 || static_cast<uint32>( Index ) >= m_Props.size())
    {
		throw std::out_of_range( "property index" );
    }

    return m_Props[Index];
}

bool CMeshVtxPropCol::ClearProperties()
{
	for (size_t i = 0; i < m_Props.size(); ++i)
	{
		delete m_Props[i];
	}

    m_Props.clear();
    return true;
}

CMeshVtxProp* CMeshVtxPropCol::GetProperty(int32 Index) const
{
    if (Index < 0 || static_cast<uint32>( Index ) >= m_Props.size())
    {
		throw std::out_of_range( "property index" );
    }

    return m_Props[Index];
}

bool CMeshVtxPropCol::SetPropertyCount(int32 PropertyCount)
{
    if (PropertyCount < 0 || PropertyCount > KMaxPropertyCount)
    {
        return false;
    }

	size_t const OldSize = m_Props.size();

	for (int32 i = static_cast<int32>( OldSize - 1 ); i >= PropertyCount; --i)
	{
		delete m_Props[i];
	}

    m_Props.resize( static_cast<uint32>( PropertyCount ) );

	for (size_t i = OldSize; i < m_Props.size(); ++i)
	{
		m_Props[i] = new CMeshVtxProp();
	}

    return true;
}

bool CMeshVtxPropCol::RemoveProperty(int32 Index)
{
    if (Index < 0 || static_cast<uint32>( Index ) >= m_Props.size())
    {
        return false;
    }

	delete m_Props[Index];
    m_Props.erase( m_Props.begin() + Index );

    return true;
}

bool CMeshVtxPropCol::SetProperty(int32 Index, CMeshVtxProp* pProperty)
{
    if (Index < 0 || static_cast<uint32>( Index ) >= m_Props.size())
    {
        return false;
    }

	if (m_Props[Index] != pProperty)
	{
		delete m_Props[Index];
	    m_Props[Index] = pProperty;
	}

    return true;
}

CMeshVtxPropCol::~CMeshVtxPropCol()
{
	ClearProperties();
}

void CMeshVtxPropCol::Write( CSerializationInfo* sInfo )
{
    uint8 PropertyCount;
	assert( m_Props.size() <= KMaxPropertyCount );
    PropertyCount = static_cast<uint8>( m_Props.size() );
	sInfo->AddValue( "PropertyCount", PropertyCount );

    for (uint32 i = 0; i < m_Props.size(); ++i)
    {
        if (m_Props[i] == NULL)
        {
			throw std::invalid_argument( "NULL mesh property pointer" );
        }

		m_Props[i]->Write( sInfo );
    }
}

void CMeshVtxPropCol::Read( CSerializationInfo* sInfo )
{
    uint8 PropertyCount;
	sInfo->GetValue( "PropertyCount", PropertyCount );

    SetPropertyCount( PropertyCount );

    for (uint32 i = 0; i < m_Props.size(); ++i)
    {
		m_Props[i]->Read( sInfo );
    }
}

bool CMeshVtxPropCol::GetValue(int32 PropIndex, int32 VertexIndex, float32* pValue) const
{
    if (PropIndex < 0 || static_cast<uint32>( PropIndex ) >= m_Props.size())
    {
        return false;
    }

    if (m_Props[PropIndex] == NULL)
    {
        return false;
    }

    return m_Props[PropIndex]->GetValue( VertexIndex, pValue );
}

bool CMeshVtxPropCol::SetValue(int32 PropIndex, int32 VertexIndex, float32 Value)
{
    if (PropIndex < 0 || static_cast<uint32>( PropIndex ) >= m_Props.size())
    {
        return false;
    }

    if (m_Props[PropIndex] == NULL)
    {
        return false;
    }

    return m_Props[PropIndex]->SetValue( VertexIndex, Value );
}

} //namespace Graphics
} //namespace WT
