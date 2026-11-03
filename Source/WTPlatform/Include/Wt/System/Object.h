#pragma once

#include "WT/System/Platform.h"
#include "WT/System/ReferenceCounted.h"
#include "WT/System/SmartPointer.h"
#include "WT/System/Type.h"

namespace WT
{
namespace System
{

typedef SmartPointer<class CObject> HObject;
class CObject : public CReferenceCounted
{
public:
	static const CType* WTCall GetClassType();
	virtual const CType* GetType() const {return GetClassType();}

	static HObject WTCall Create()
	{
		return new CObject();
	}

	// For consistency with IInterface
	virtual CObject const * GetObjectPtr() const;
	virtual CObject * GetObjectPtr();

	CObject()
	{
	}

	virtual ~CObject();

	static void WTCall Using() {GetClassType();}

	//Get debug information about the object. The information may just be the name of the object's type.
	virtual string ToString();
};

}

template <typename T> void WTCall Using()
{
	T::Using();
}

}
