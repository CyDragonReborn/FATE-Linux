//
//	PropertySerializer.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/System/Object.h"
#include "WT/System/Platform.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/PropertyCollection.h"
#include "WT/System/Serialization/SerializationSurrogate.h"

namespace WT
{
namespace System
{
namespace Serialization
{

typedef SmartPointer<class CPropertySerializationSurrogate> HPropertySerializationSurrogate;
class CPropertySerializationSurrogate : 
	public CSerializationSurrogate
{
public:
	WT_CLASS( CPropertySerializationSurrogate, CSerializationSurrogate );

	CPropertySerializationSurrogate();
	CPropertySerializationSurrogate( CObject* pInnerObject );

	virtual ~CPropertySerializationSurrogate();

	// ISerializable
	void GetObjectData(CSerializationInfo* SerialInfo, CSerializationContext* sContext) const;
	void SetObjectData(CSerializationInfo* SerialInfo, CSerializationContext* sContext);
	virtual void GetSupportedVersionRange( uint32 &MinVersion, uint32 &MaxVersion ) const;
private:
};

} // namespace Serialization
} // namespace System
} // namespace WT
