#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/Formatter.h"

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CFormatter)

//  Returns the next object to serialize, from the formatter's internal work queue. 
HObject CFormatter::GetNext(CObjectIDGenerator::ObjectID* id )
{
	if (m_ObjectQueue.empty())
	{
		return NULL;
	}

	HObject hObject = m_ObjectQueue.front();
	m_ObjectQueue.pop_front();
	if (id != NULL)
	{
		bool firstTime;
		*id = m_IdGenerator.GetID( hObject, firstTime );
	}
	return hObject;
}

// Schedules an object for later serialization. 
CObjectIDGenerator::ObjectID CFormatter::Schedule(CObject* obj)
{
	assert(obj != NULL);

	bool firstTime;
	CObjectIDGenerator::ObjectID id = m_IdGenerator.GetID(obj, firstTime);
	if ( firstTime )
	{
		// this is the first time this object has existed in the system, we better place it
		// on the queue for serialization.
		m_ObjectQueue.push_back(obj);
	}

	return id;
}


} // Serialization namespace
} // System namespace
} // WT namespace