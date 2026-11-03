#pragma once

#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/ObjectFinalizer.h"
#include "WT/System/Serialization/SerializationSurrogate.h"
#include "WT/System/Serialization/SerializationException.h"

#include <iostream>
#include <sstream>
#include <map>

namespace WT
{
namespace System
{
namespace Serialization
{


CObjectFinalizeData::CObjectFinalizeData() : 
	m_pObject(NULL), 
	m_pSerializer(NULL),
	m_DeleteSerializer(false), 
	m_FinalizeStatus(FinalizeStatus::NotFinalized)
{
}


CObjectFinalizeData::CObjectFinalizeData(CObject* pObj) : 
	m_pObject(pObj), 
	m_DeleteSerializer(false), 
	m_FinalizeStatus(FinalizeStatus::NotFinalized)
{
	m_pSerializer = DynamicCast<ISerializable>(m_pObject);
	if (m_pObject != NULL && m_pSerializer == NULL)
	{
		std::ostringstream ssmsg;
		ssmsg << "Couldn't cast object (" << m_pObject->GetType()->GetFullName() << ") to ISerializable";
		throw CSerializationException(ssmsg.str());
	}
}

CObjectFinalizeData::CObjectFinalizeData(CObject* pObj, ISerializable* pSerializable, bool deleteSerializer) :
	m_pObject(pObj), 
	m_pSerializer(pSerializable),
	m_DeleteSerializer(deleteSerializer), 
	m_FinalizeStatus(FinalizeStatus::NotFinalized)
{
}

CObjectFinalizeData::~CObjectFinalizeData()
{
	m_pSerializer = NULL;
	m_pObject = NULL;
	m_FinalizeStatus = FinalizeStatus::NotFinalized;
}

void CObjectFinalizeData::AddReference(CObject* pObjectRef)
{
	if (pObjectRef == NULL)
	{
		// don't waste space storing NULL refereces, 
		// they can't be finalized so we don't need to know about them
		return;
	}

	m_References.push_back( pObjectRef );
}

void CObjectFinalizeData::Finalize(CObjectFinalizer* pFinalizer, CSerializationInfo* si, CSerializationContext* sc)
{
	if ( m_FinalizeStatus == FinalizeStatus::NotFinalized)
	{
		m_FinalizeStatus = FinalizeStatus::InProgress;
		FinalizeReferences(pFinalizer, si, sc);
		m_pSerializer->FinalizeDeserialization(si, sc);
		if (m_DeleteSerializer)
		{
			delete m_pSerializer;
			m_pSerializer = NULL;
		}
		m_FinalizeStatus = FinalizeStatus::Finalized;	
	}
}

FinalizeStatus::Type CObjectFinalizeData::GetFinalizeStatus() const
{
	return m_FinalizeStatus;
}

CObject* CObjectFinalizeData::GetSerializedObject() const
{
	return m_pObject;
}

ISerializable* CObjectFinalizeData::GetSerializer() const
{
	return m_pSerializer;
}

void CObjectFinalizeData::FinalizeReferences(CObjectFinalizer* pFinalizer, CSerializationInfo* si, CSerializationContext* sc)
{
	std::vector<CObject*>::const_iterator i;
	for (i = m_References.begin(); i != m_References.end(); i++)
	{
		pFinalizer->FinalizeObject(*i, si, sc);
	}
}

void CObjectFinalizer::AddFinalizableObject(CObject* pObj)
{
	m_FinalizableObjects[pObj] = CObjectFinalizeData(pObj);
}

void CObjectFinalizer::AddFinalizableObject(CObject* pObj, ISerializable* pSerializable, bool deleteSerializer)
{
	m_FinalizableObjects[pObj] = CObjectFinalizeData(pObj, pSerializable, deleteSerializer);
}

void CObjectFinalizer::AddObjectFinalizableObjectReference(CObject* pObj, CObject* pReferencedObject)
{
	std::map<HObject, CObjectFinalizeData>::iterator i = m_FinalizableObjects.find(pObj);
	if (i == m_FinalizableObjects.end())
	{
		throw CSerializationException("Unexpected Error, AddObjectFinalizableObjectReference() called but couldn't find object. It must not have been added using AddObjectFinalizeData()");
	}

	i->second.AddReference(pReferencedObject);
}

void CObjectFinalizer::FinalizeAllObjects(CSerializationInfo* si, CSerializationContext* sc)
{
	std::map<HObject, CObjectFinalizeData>::iterator i;
	for (i = m_FinalizableObjects.begin(); i != m_FinalizableObjects.end(); i++)
	{
		i->second.Finalize(this, si, sc);
	}
}

bool CObjectFinalizer::ObjectIsFinalized(CObject const* pObject) const
{
	std::map<HObject, CObjectFinalizeData>::const_iterator i = 
		m_FinalizableObjects.find( const_cast<CObject*>( pObject ) );
	if (i == m_FinalizableObjects.end())
	{
		// couldn't even find the object, so no, it hasn't been finalized yet
		return false;
	}

	return i->second.GetFinalizeStatus() == FinalizeStatus::Finalized;
}

void CObjectFinalizer::Reset()
{
	m_FinalizableObjects.clear();
}

ISerializable* CObjectFinalizer::GetSerializer(CObject* pObject) const
{
	std::map<HObject, CObjectFinalizeData>::const_iterator i = m_FinalizableObjects.find(pObject);
	if (i == m_FinalizableObjects.end())
	{
		throw CSerializationException("Unexpected error. Could not find object in m_FinalizableObjects map.");
	}

	return i->second.GetSerializer();
}


void CObjectFinalizer::FinalizeObject(CObject* pObj, CSerializationInfo* si, CSerializationContext* sc)
{
	std::map<HObject, CObjectFinalizeData>::iterator i = m_FinalizableObjects.find(pObj);
	if (i == m_FinalizableObjects.end())
	{
		throw CSerializationException("Unexpected error. Could not find object in m_FinalizableObjects map.");
	}

	i->second.Finalize(this, si, sc);
}






} // Serialization namespace
} // System namespace
} // WT namespace

