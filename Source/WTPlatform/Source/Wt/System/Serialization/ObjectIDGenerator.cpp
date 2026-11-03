#include "WT/stdafx.h"
#include "WT/System/Serialization/ObjectIDGenerator.h"


namespace WT
{
namespace System
{
namespace Serialization
{


WT_DEFINE_CLASS(CObjectIDGenerator)


CObjectIDGenerator::CObjectIDGenerator() : m_NextID(1)
{
}

CObjectIDGenerator::~CObjectIDGenerator()
{
}

CObjectIDGenerator::ObjectID CObjectIDGenerator::GetID( CObject* obj, bool& firstTime)
{
	assert(obj != NULL);

	ObjectID id;
	ObjectToIDTable::iterator iter = m_Object2IDTable.find( obj );
	firstTime = ( iter == m_Object2IDTable.end() );
	if (firstTime)
	{
		id = m_NextID++;
		m_Object2IDTable[obj] = id;
		m_ID2ObjectTable[id] = obj;
	}
	else
	{
		id = iter->second;
	}

	assert(id != 0);
	return id;
	
}

bool CObjectIDGenerator::HasID(CObject* obj) const
{	
	return m_Object2IDTable.find( obj ) != m_Object2IDTable.end();
}

CObject* CObjectIDGenerator::GetObjectByID(const ObjectID& id) const
{
	// quickly return NULL if we can see that the given ID us unassigned.
	if ( (id >= m_NextID) || (id <= 0) )
	{
		return NULL;
	}

	IDToObjectTable::const_iterator iter = m_ID2ObjectTable.find(id);
	assert(iter != m_ID2ObjectTable.end());
	return iter->second;
}

void CObjectIDGenerator::Reset()
{
	m_NextID = 1;
	m_Object2IDTable.clear();
	m_ID2ObjectTable.clear();
}

	

} // Serialization namespace
} // System namespace
} // WT namespace