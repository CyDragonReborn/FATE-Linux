#include "WT/stdafx.h"

#include "windows.h"
#include "WT/System/Object.h"

namespace WT
{
namespace System
{

WT_DEFINE_CLASS( CObject )

CObject::~CObject()
{
}

const CType* CObject::GetClassType()
{
	static CType* g_pMyType = NULL;

	if (g_pMyType == NULL)
	{
		char const * typeidName = NormalizeClassName( typeid( CObject ).name() );
		std::auto_ptr<CCaster> BaseCaster;
		g_pMyType = new CTypeTemplate<CObject>( typeidName, NULL, BaseCaster, false, CObject::Create );
	}

	return g_pMyType;
}


CObject const * CObject::GetObjectPtr() const
{
	return this;
}

CObject * CObject::GetObjectPtr()
{
	return this;
}

//Get debug information about the object. The information may just be the name of the object's type.
//TODO consider making this const.
/*virtual*/ string CObject::ToString()
{
	return GetType()->GetName();
}

}
}
