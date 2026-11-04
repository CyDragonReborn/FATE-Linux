//
//	ObjectIDGenerator.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/System/Object.h"
#include "WT/System/Platform.h"
#include <map>

namespace WT
{
namespace System
{
namespace Serialization
{

// This is a helper class to aid in the creation of Formatters.
//
// WARNING: ObjectIDGenerator will keep a handle to EVERY object you pass to it.
// This can cause Object to exist for as long as ObjectIDGenerator exists. Make sure
// that ObjectIDGenerator is deleted/goes out of scope as soon as possible.
//
//------------------------------------------------------------------------------------
// From the .Net documentation:
//
// The ObjectIDGenerator keeps track of previously identified objects. When you ask for 
// the ID of an object, the ObjectIDGenerator knows whether to return the existing ID, 
// or generate and remember a new ID.
//
// The IDs are unique for the life of the ObjectIDGenerator instance. Generally, an 
// ObjectIDGenerator life lasts as long as the Formatter that created it. Object IDs have 
// meaning only within a given serialized stream, and are used for tracking which objects have 
// references to others within the serialized object graph.
//
// Using a hash table, the ObjectIDGenerator retains what ID is assigned to what object.
//
// Note   Object IDs are 64-bit numbers. Allocation starts from one, so zero is never a 
// valid object ID. A formatter can choose a zero value to represent an object reference 
// whose value is a null reference
//
typedef SmartPointer<class CObjectIDGenerator> HObjectIDGenerator;
class CObjectIDGenerator :
		public CObject
{
public:
	WT_CLASS( CObjectIDGenerator, CObject )

	typedef uint64 ObjectID;
	
	CObjectIDGenerator();
	virtual ~CObjectIDGenerator();

	ObjectID GetID(CObject* obj, bool& firstTime);
	bool HasID(CObject* obj) const;

	// This method is not part of the ObjectIDGenerator in .Net
	// but having it means that we don't have to write another object to id map class.
	CObject* GetObjectByID(const ObjectID& id) const;

	// Clears all state from the ObjectIDGenerator.
	// Formatters can call this and then reuse the same instance of ObjectIDGenerator for
	// multiple serialization/deserialization requests.
	void Reset();

private: // data
	typedef std::map<HObject, ObjectID> ObjectToIDTable;
	typedef std::map<ObjectID, HObject> IDToObjectTable;

	ObjectToIDTable m_Object2IDTable;
	IDToObjectTable m_ID2ObjectTable;
	ObjectID m_NextID;
};

}
}
}