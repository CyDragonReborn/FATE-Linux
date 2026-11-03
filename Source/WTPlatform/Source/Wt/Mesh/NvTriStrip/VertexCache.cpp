#include "WT/stdafx.h"

#include "VertexCache.h"

namespace WT
{
namespace Graphics
{
namespace NvTriStrip
{

VertexCache::VertexCache( int size ) :
	m_Front( 0 )	
{
	m_Verts.resize( size );

	for (int i = 0; i < size; ++i)
	{
		m_Verts[i] = -1;
		m_Multiset.insert( -1 );
	}
}

int VertexCache::AddEntry( int entry )
{
	//Get the vertex index which is to be removed from the queue
	int Remove = m_Verts[ m_Front ];

	//Remove it from the multiset as well - it should be there
	std::multiset<int32>::iterator it = m_Multiset.find( Remove );
	if (it != m_Multiset.end())
	{
		m_Multiset.erase( it );
	}

	//Add the new vertex index
	m_Verts[ m_Front ] = entry;
	m_Multiset.insert( entry );

	//Advance the front of the queue
	if (++m_Front >= (int32)m_Verts.size())
	{
		m_Front = 0;
	}

	return Remove;
}

} //namespace NvTriStrip
} //namespace Graphics
} //namespace WT
