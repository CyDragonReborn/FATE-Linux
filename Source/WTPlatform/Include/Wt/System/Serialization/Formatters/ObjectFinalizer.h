#pragma once

#include "WT/System/Object.h"
#include "WT/System/Serialization/ObjectIDGenerator.h"
#include "WT/System/Serialization/ISerializable.h"

namespace WT
{
namespace System
{
namespace Serialization
{


//TODO: avy, everything is leaked (in the general deserialization system) when an exception thrown). 
// How do we want to clean up.


class CObjectFinalizer;

class CObjectFinalizeData
{
public:
	CObjectFinalizeData();
	CObjectFinalizeData(CObject* pObj);
	CObjectFinalizeData(CObject* pObj, ISerializable* pSerializable, bool deleteSerializer);
	~CObjectFinalizeData();
	void AddReference(CObject* ObjectRef);
	void Finalize(CObjectFinalizer* pFinalizer, CSerializationInfo* si, CSerializationContext* sc);
	FinalizeStatus::Type GetFinalizeStatus() const;

	CObject* GetSerializedObject() const;
	ISerializable* GetSerializer() const;
private:
	void FinalizeReferences(CObjectFinalizer* pFinalizer, CSerializationInfo* si, CSerializationContext* sc);

	CObject* m_pObject; // the object being deserialized
	ISerializable* m_pSerializer; // the object handling deserialization. May be different from m_pObject if a surrogate is used
	bool m_DeleteSerializer; // indicates whether the serializer should be deleted after finalization (this is needed for some surrogates)
	
	FinalizeStatus::Type  m_FinalizeStatus;
	vector<CObject*> m_References;
};



// This object is during used deserialization by formaters to keep track of top level objects
// and to order the calls to ISerializable::FinalizeDeserialization()
typedef SmartPointer<class CObjectFinalizer> HObjectFinalizer;
class CObjectFinalizer :
		public CObject
{
public:
	WT_CLASS(CObjectFinalizer, CObject);

	void AddFinalizableObject(CObject* pObj);
	void AddFinalizableObject(CObject* pObj, ISerializable* pSerializer, bool deleteSerializer);

	void AddObjectFinalizableObjectReference(CObject* pObj, CObject* pReferencedObject);
	void FinalizeAllObjects(CSerializationInfo* si, CSerializationContext* sc);

	bool ObjectIsFinalized(CObject const* pObject) const;
	void Reset();

	ISerializable* GetSerializer(CObject* pObject) const;
private:
	friend class CObjectFinalizeData;
	void FinalizeObject(CObject* pObj, CSerializationInfo* si, CSerializationContext* sc);

	std::map<HObject, CObjectFinalizeData> m_FinalizableObjects;
};


} // Serialization namespace
} // System namespace
} // WT namespace

