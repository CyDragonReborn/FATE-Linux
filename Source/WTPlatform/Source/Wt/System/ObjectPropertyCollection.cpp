#include "WT/stdafx.h"


#include "WT/System/Platform.h"
#include "WT/System/ObjectPropertyCollection.h"
#include "WT/System/IPropertied.h"

namespace WT
{
namespace System
{

using namespace Serialization;

WT_DEFINE_CLASS(CObjectPropertyCollection)

CObjectPropertyCollection::CObjectPropertyCollection( CObject& Object )
	: m_pObject( &Object )
{
}

CObjectPropertyCollection::CObjectPropertyCollection( CObjectPropertyCollection const& ObjectPropertyCollection )
	: m_pObject( ObjectPropertyCollection.m_pObject )
{
}

/*virtual*/ CObjectPropertyCollection::~CObjectPropertyCollection()
{
}

CObjectPropertyCollection& CObjectPropertyCollection::operator=( CObjectPropertyCollection const& ObjectPropertyCollection )
{
	m_pObject = ObjectPropertyCollection.m_pObject;
	return *this;
}

IPropertyCollection* CObjectPropertyCollection::GetTypeProperties()
{
	assert( m_pObject != NULL );
	CType const *const pType = m_pObject->GetType();
	assert( pType != NULL );
	return pType->GetProperties();
}

IPropertyCollection const* CObjectPropertyCollection::GetTypeProperties() const
{
	assert( m_pObject != NULL );
	CType const *const pType = m_pObject->GetType();
	assert( pType != NULL );
	return pType->GetProperties();
}

IPropertyCollection* CObjectPropertyCollection::GetInstanceProperties()
{
	assert( m_pObject != NULL );

	IPropertied* const pPropertied = DynamicCast<IPropertied>( m_pObject );
	if (pPropertied == NULL)
	{
		return NULL;
	}

	return &pPropertied->GetProperties();
}

IPropertyCollection const* CObjectPropertyCollection::GetInstanceProperties() const
{
	assert( m_pObject != NULL );

	IPropertied const* const pPropertied = DynamicCast<IPropertied const>( m_pObject );
	if (pPropertied == NULL)
	{
		return NULL;
	}

	return &pPropertied->GetProperties();
}

/*virtual*/ bool CObjectPropertyCollection::Add( const string& Name, CProperty* Prop )
{
	assert( m_pObject != NULL );

	IPropertyCollection* pTypeProperties = GetTypeProperties();
	if (pTypeProperties != NULL && pTypeProperties->Exists( Name ))
	{
		return false;
	}

	IPropertyCollection* pInstanceProperties = GetInstanceProperties();
	if (pInstanceProperties != NULL)
	{
		if (pInstanceProperties->Exists( Name ))
		{
			return false;
		} else {
			return pInstanceProperties->Add( Name, Prop );
		}
	}

	if (pTypeProperties != NULL)
	{
		return pTypeProperties->Add( Name, Prop );
	}

	return false;
}

/*virtual*/ void CObjectPropertyCollection::Set( const string& Name, CProperty* Prop )
{
	assert( m_pObject != NULL );

	IPropertyCollection* pInstanceProperties = GetInstanceProperties();
	if (pInstanceProperties != NULL && pInstanceProperties->Exists( Name ))
	{
		pInstanceProperties->Set( Name, Prop );
		return;
	}

	IPropertyCollection* pTypeProperties = GetTypeProperties();
	if (pTypeProperties != NULL && pTypeProperties->Exists( Name ))
	{
		pTypeProperties->Set( Name, Prop );
		return;
	}

	if (pInstanceProperties != NULL)
	{
		pInstanceProperties->Set( Name, Prop );
		return;
	}

	if (pTypeProperties != NULL)
	{
		pTypeProperties->Set( Name, Prop );
		return;
	}
}

/*virtual*/ bool CObjectPropertyCollection::Erase( const string& Name )
{
	assert( m_pObject != NULL );

	bool Result = false;
	
	IPropertyCollection* pInstanceProperties = GetInstanceProperties();
	if (pInstanceProperties != NULL)
	{
		Result |= pInstanceProperties->Erase( Name );
	}

	IPropertyCollection* pTypeProperties = GetTypeProperties();
	if (pTypeProperties != NULL)
	{
		Result |= pTypeProperties->Erase( Name );
	}

	return Result;
}

/*virtual*/ bool CObjectPropertyCollection::Exists( const string& Name ) const
{
	assert( m_pObject != NULL );

	IPropertyCollection const* pTypeProperties = GetTypeProperties();
	if (pTypeProperties != NULL && pTypeProperties->Exists( Name ))
	{
		return true;
	}

	IPropertyCollection const* pInstanceProperties = GetInstanceProperties();
	if (pInstanceProperties != NULL && pInstanceProperties->Exists( Name ))
	{
		return true;
	}

	return false;
}

/*virtual*/ CProperty* CObjectPropertyCollection::GetItem( const string& Name )
{
	assert( m_pObject != NULL );

	CProperty* Result = NULL;
	
	IPropertyCollection* pInstanceProperties = GetInstanceProperties();
	if (pInstanceProperties != NULL)
	{
		Result = pInstanceProperties->GetItem( Name );
	}

	if (Result == NULL)
	{
		IPropertyCollection* pTypeProperties = GetTypeProperties();
		if (pTypeProperties != NULL)
		{
			Result = pTypeProperties->GetItem( Name );
		}
	}

	return Result;
}

/*virtual*/ CProperty const* CObjectPropertyCollection::GetItem( const string& Name ) const
{
	assert( m_pObject != NULL );

	CProperty const* Result = NULL;
	
	IPropertyCollection const* pInstanceProperties = GetInstanceProperties();
	if (pInstanceProperties != NULL)
	{
		Result = pInstanceProperties->GetItem( Name );
	}

	if (Result == NULL)
	{
		IPropertyCollection const* pTypeProperties = GetTypeProperties();
		if (pTypeProperties != NULL)
		{
			Result = pTypeProperties->GetItem( Name );
		}
	}

	return Result;
}

}
}
