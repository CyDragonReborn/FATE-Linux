#pragma once

#include "WT/System/Object.h"
#include "WT/System/Serialization/ObjectIDGenerator.h"
#include "WT/System/Serialization/SerializationSurrogate.h"

namespace WT
{
namespace System
{
namespace Serialization
{

class CObjectTypeRef : 
	public CObject
{
public:
	WT_CLASS(CObjectTypeRef, CObject);

	CObjectTypeRef();
	CObjectTypeRef(CObject const * object, CObjectIDGenerator::ObjectID objID);
	CObjectTypeRef(CObject const * object, CObjectIDGenerator::ObjectID objID, CSerializationSurrogate const * pSurrogate);
	CObjectTypeRef(string const & typeStr, CObjectIDGenerator::ObjectID objID, uint32 objectFormatVersion);
	CObjectTypeRef(string const & typeStr, CObjectIDGenerator::ObjectID objID, string const & surrogateTypeStr, uint32 objectFormatVersion);
	CObjectTypeRef(CObjectTypeRef const & right );
	virtual ~CObjectTypeRef();

	// returns true if the current object type info is a NULL reference 
	// (no type information and ObjectID of 0)
	bool IsNull() const;

	// Returns the Type object associated with the type specified in this object reference
	CType const * GetRefType() const;

	bool HasSurrogate() const;
	CType const * GetSurrogateType() const;

	// Returns the ObjectID of the object specified in this object reference.
	CObjectIDGenerator::ObjectID GetObjectID() const;

	uint32 GetObjectFormatVersion() const;
	
protected:

	void Init(CObject const * object, CObjectIDGenerator::ObjectID objID, CSerializationSurrogate const * pSurrogate);
	void Init(string const & typeStr, CObjectIDGenerator::ObjectID objID, uint32 objectFormatVersion);
	void Init(string const & typeStr, CObjectIDGenerator::ObjectID objID, string const & surrogateTypeStr, uint32 objectFormatVersion );

private:
	const CType* m_Type;
	CObjectIDGenerator::ObjectID m_ObjectID;
	const CType* m_SurrogateType;

	// format version number for the object used when serializing/deserializing. If a surrogate is 
	// being used then this versions come from the surrogate
	uint32 m_ObjectFormatVersion;
	
};


} // Serialization namespace
} // System namespace
} // WT namespace

