//
//	SerializationContext.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/System/Object.h"
#include "WT/System/Platform.h"

namespace WT
{
namespace System
{

class CUserPropertyCollection;

namespace Serialization
{

typedef SmartPointer<class CSerializationContext> HSerializationContext;
class CSerializationContext :
		public CObject
{
public:
	WT_BEGIN_CLASS(CSerializationContext, CObject);
//		WT_BEGIN_PROPERTIES();
//			WT_PROP_RO_OBJECT(UserProperties, UserPropertyCollection);
//		WT_END_PROPERTIES();
	WT_END_CLASS();

	CSerializationContext();
	virtual ~CSerializationContext();
	virtual CUserPropertyCollection* GetProperties() {return m_pContextProperties;}
	virtual CUserPropertyCollection const* GetProperties() const {return m_pContextProperties;}
private:
	CUserPropertyCollection* m_pContextProperties;
};

}
}
}

#include "WT/System/UserPropertyCollection.h"
