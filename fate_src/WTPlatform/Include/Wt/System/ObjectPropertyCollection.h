#pragma once

#include "WT/System/Platform.h"
#include "WT/System/DateTime.h"
#include "WT/System/Object.h"
#include "WT/System/SystemException.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationContext.h"
#include "WT/System/PropertyCollection.h"
#include "WT/System/UserPropertyCollection.h"

namespace WT
{
namespace System
{

// A virtual collection of properties that represents all properties for a CObject
// or CObject-derived instance. This is essentially a combination of an object's type's
// properties for the instance and, if the object supports IPropertied, the object's
// instance's properties.
class CObjectPropertyCollection : public CObject, public IPropertyCollection
{
public:
	WT_BEGIN_CLASS_NO_CREATOR( CObjectPropertyCollection, CObject )
		WT_INHERITS_INTERFACE( IPropertyCollection )
	WT_END_CLASS()

	//Constructors
	explicit CObjectPropertyCollection( CObject& Object );
	CObjectPropertyCollection( CObjectPropertyCollection const& ObjectPropertyCollection );

	virtual bool Add ( const string& Name, CProperty* Prop );
	virtual void Set ( const string& Name, CProperty* Prop );
	virtual bool Erase( const string& Name );
	virtual bool Exists( const string& Name ) const;
	virtual CProperty* GetItem ( const string& Name );
	virtual CProperty const* GetItem ( const string& Name ) const;

	//Helper functions to set both the property type and value
	template <typename T> void SetValue( string const& name, T const& value );

	//Get a property value value. This function throw if the property does not exist or if 
	//the property is of the wrong type.
	template <typename T> T GetValue( string const& name ) const;

	//For non-existent properties, this function returns the default value you specify.
	//Trying to get a wrongly typed value from a property will still throw.
	template <typename T> T GetValue( string const& name, T const& Default ) const;

	//Destructor
	~CObjectPropertyCollection();

	//Operators
	CObjectPropertyCollection& operator=( CObjectPropertyCollection const& ObjectPropertyCollection );

private:
	IPropertyCollection* GetTypeProperties();
	IPropertyCollection const* GetTypeProperties() const;
	IPropertyCollection* GetInstanceProperties();
	IPropertyCollection const* GetInstanceProperties() const;

	CObject* m_pObject;
};

template <typename T>
void CObjectPropertyCollection::SetValue( string const& name, T const& value )
{
	CProperty* pProp = GetItem( name );
	CValueTypedProperty<T>* pTypedProp = NULL;
	
	if (pProp != NULL)
	{
		pTypedProp = DynamicCast<CValueTypedProperty<T> >( pProp );
		if (pTypedProp == NULL)
		{
			pProp = NULL;
		}
	}

	if (pProp == NULL)
	{
		pTypedProp = new CExplicitTypedProperty<T>();
		Set( name, pTypedProp );
	}

	assert( pTypedProp != NULL );
	pTypedProp->SetValue( m_pObject, value );
}

template <typename T>
T CObjectPropertyCollection::GetValue( string const& name ) const
{
	CProperty const* pProp = GetItem( name );
	if (pProp == NULL)
	{
		throw std::invalid_argument( "property name" );
	}

	CValueTypedProperty<T>* pTypedProp = DynamicCast<CValueTypedProperty<T> >( pProp );

	if (pTypedProp == NULL)
	{
		throw std::runtime_error( "property type" );
	}

	T Value;
	pTypedProp->GetValue( m_pObject, Value );
	return Value;
}

template <typename T>
T CObjectPropertyCollection::GetValue( string const& name, T const& Default  ) const
{
	CProperty const* pProp = GetItem( name );
	if (pProp == NULL)
	{
		return Default;
	}

	CValueTypedProperty<T>* pTypedProp = DynamicCast<CValueTypedProperty<T> >( pProp );

	if (pTypedProp == NULL)
	{
		throw std::runtime_error( "property type" );
	}

	T Value;
	pTypedProp->GetValue( m_pObject, Value );
	return Value;
}

}
}
