//
//	SerializationSurrogate.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/System/Object.h"
#include "WT/System/Platform.h"
#include "WT/System/Serialization/ISerializable.h"

namespace WT
{
namespace System
{
namespace Serialization
{

typedef SmartPointer<class CSerializationSurrogate> HSerializationSurrogate;
class CSerializationSurrogate :
	public CObject, 
	public ISerializable

{
public:
	WT_BEGIN_ABSTRACT_CLASS( CSerializationSurrogate, CObject );
		WT_INHERITS_INTERFACE( ISerializable );
	WT_END_CLASS( );

	CSerializationSurrogate( CObject* pObj = NULL );
	virtual ~CSerializationSurrogate();

	// SerializationSurrogate 

	// the object to deserialize into.
	// ownership is not passed to SerializationSurrogate
	virtual void SetInnerObject(CObject* pObject);

	// returns the Object that will be/has been deserialized into
	virtual CObject* GetInnerObject();

	// ISerializable
	virtual void GetObjectData(CSerializationInfo* SerialInfo, CSerializationContext* sContext) const = 0;
	virtual void SetObjectData(CSerializationInfo* SerialInfo, CSerializationContext* sContext) = 0;
protected:
	HObject m_hObject;
};



} // namespace Serialization
} // namespace System
} // namespace WT
