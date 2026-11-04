#pragma once

#include "WT/System/Platform.h"
#include <set>

namespace WT
{
namespace Graphics
{
namespace NvTriStrip
{

class VertexCache
{
public:
	VertexCache(int size);
	~VertexCache() {}
	
	bool VertexCache::InCache(int entry)
	{
		return m_Multiset.find( entry ) != m_Multiset.end();
	}

	int AddEntry(int entry);

private:
	std::vector<int32> m_Verts; //Vector of vertex indices which is treated as a fixed-size queue
	int32 m_Front; //Index of the front of the queue
	std::multiset<int32> m_Multiset; //Same data as m_Verts, but speeds up searching
};

} //namespace NvTriStrip
} //namespace Graphics
} //namespace WT
