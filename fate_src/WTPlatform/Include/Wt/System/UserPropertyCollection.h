#pragma once

#include "WT/System/Platform.h"
#include "WT/System/DateTime.h"
#include "WT/System/Object.h"
#include "WT/System/SystemException.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationContext.h"
#include "WT/System/PropertyCollection.h"

namespace WT
{
namespace System
{

namespace Serialization
{
	class CSerializationInfo;
	class CSerializationContext;
}

namespace PropertyType
{
	enum Type
	{
		//A mask used to form a Type value for property arrays
		ArrayMask = 1 << 31,
		
		Unknown = -1, //An unknown kind of property

		Int8 = 0,  //int8 storage
		Int16, //int16 storage
		Int32, //int32 storage
		Int64, //int64 storage
		UInt8, //uint8 storage
		UInt16, //uint16 storage
		UInt32, //uint32 storage
		UInt64, //uint64 storage
		Float32, //float32 storage
		Float64, //float64 storage
		Bool, //bool storage
		String, //string storage
		WString, //wstring storage
		DateTime, //CDateTime storage
		TimeSpan, //CTimeSpan storage
		ObjectPtr, //CObject* storage

		//Type values for property arrays
		Int8Array = Int8 + ArrayMask,
		Int16Array, //std::vector<int16> storage
		Int32Array, //std::vector<int32> storage
		Int64Array, //std::vector<int64> storage
		UInt8Array, //std::vector<uint8> storage
		UInt16Array, //std::vector<uint16> storage
		UInt32Array, //std::vector<uint32> storage
		UInt64Array, //std::vector<uint64> storage
		Float32Array, //std::vector<float32> storage
		Float64Array, //std::vector<float64> storage
		BoolArray, //std::vector<bool> storage
		StringArray, //std::vector<string> storage
		WStringArray, //std::vector<wstring> storage
		DateTimeArray, //std::vector<CDateTime> storage
		TimeSpanArray, //std::vector<CTimeSpan> storage
		ObjectPtrArray //std::vector<CObject*> storage
	};
}

// A simple collection of properties that facilitates holding explicit 
// properties (CExplicitTypedProperty<>) and supports serialization.
typedef SmartPointer<class CUserPropertyCollection> HUserPropertyCollection;
class CUserPropertyCollection : public CPropertyCollection, public Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS(CUserPropertyCollection, CPropertyCollection)
		WT_INHERITS_INTERFACE( Serialization::ISerializable )
	WT_END_CLASS()

	//Get the type of the named property. If no such property exists, then an exception will be thrown.
	//If the property is one that was manually inserted, and a type can't be deduced, PropertyType::Unknown
	//will be returned.
	PropertyType::Type GetType( string const& name ) const;

	//Add or set a property have the specified name and type. If the property already exists and has
	//the specified type, its value will be unaffected.
	void SetType( string const& name, PropertyType::Type Type );

	//Get the number of elements in the specified array property. If the specified
	//property is not a known type of array, then an exception will be thrown.
	int32 GetArrayElementCount( string const& name ) const;

	//Set the number of elements in the specified array property. If the specified
	//property is not a known type of array, then an exception will be thrown.
	void SetArrayElementCount( string const& name, int32 Count );

	//Helper functions to set both the property type and value
	template <typename T> void SetValue( string const& name, T const& value );

	//Set an array element
	template <typename T> void SetElement( string const& name, int32 Index, T const& value );

	//Get a property value or array property element value. These functions throw if
	//the property does not exist, if the property is of the wrong type, or if the
	//element index is out-of-bounds.
	template <typename T> T const& GetValue( string const& name ) const;
	template <typename T> T const& GetElement( string const& name, int32 Index ) const;

	//For non-existent properties, these functions return the default values you specify.
	//Trying to get a wrongly typed value from a property, or trying to use an out-of-bounds
	//element will still throw.
	template <typename T> T const& GetValue( string const& name, T const& Default ) const;
	template <typename T> T const& GetElement( string const& name, int32 Index, T const& Default ) const;

	//
	// ISerializable interface
	//
	virtual void GetObjectData(Serialization::CSerializationInfo* SerialInfo, Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(Serialization::CSerializationInfo* SerialInfo, Serialization::CSerializationContext* sContext);
};

template <typename T>
void CUserPropertyCollection::SetValue( string const& name, T const& value )
{
	CProperty* pProp = GetItem( name );
	CExplicitTypedProperty<T>* pTypedProp = NULL;
	if (pProp != NULL)
	{
		pTypedProp = DynamicCast<CExplicitTypedProperty<T> >( pProp );
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
	pTypedProp->Set( value );
}

template <typename T>
void CUserPropertyCollection::SetElement( string const& name, int32 Index, T const& value )
{
	if (Index < 0)
	{
		throw std::invalid_argument( "array property index" );
	}

	CProperty* pProp = GetItem( name );
	CExplicitTypedProperty<std::vector<T> >* pTypedProp = NULL;
	if (pProp != NULL)
	{
		pTypedProp = DynamicCast<CExplicitTypedProperty<std::vector<T> > >( pProp );
		if (pTypedProp == NULL)
		{
			pProp = NULL;
		}
	}

	if (pProp == NULL)
	{
		pTypedProp = new CExplicitTypedProperty<std::vector<T> >();
		Set( name, pTypedProp );
	}

	if (Index >= static_cast<int32>( pTypedProp->Get().size() ))
	{
		throw std::invalid_argument( "array property index" );
	}

	pTypedProp->Get()[Index] = value;
}

template <typename T>
T const& CUserPropertyCollection::GetValue( string const& name ) const
{
	CProperty const* pProp = GetItem( name );
	if (pProp == NULL)
	{
		throw std::invalid_argument( "property name" );
	}

	CExplicitTypedProperty<T>* pTypedProp = DynamicCast<CExplicitTypedProperty<T> >( pProp );

	if (pTypedProp == NULL)
	{
		throw std::runtime_error( "property type" );
	}

	return pTypedProp->Get();
}

template <typename T>
T const& CUserPropertyCollection::GetElement( string const& name, int32 Index ) const
{
	if (Index < 0)
	{
		throw std::invalid_argument( "array property index" );
	}

	CProperty const* pProp = GetItem( name );
	if (pProp == NULL)
	{
		throw std::invalid_argument( "property name" );
	}

	CExplicitTypedProperty<std::vector<T> >* pTypedProp = DynamicCast<CExplicitTypedProperty<std::vector<T> > >( pProp );

	if (pTypedProp == NULL)
	{
		throw std::runtime_error( "property type" );
	}

	if (Index >= static_cast<int32>( pTypedProp->Get().size() ))
	{
		throw std::invalid_argument( "array property index" );
	}

	return pTypedProp->Get()[Index];
}

template <typename T>
T const& CUserPropertyCollection::GetValue( string const& name, T const& Default  ) const
{
	CProperty const* pProp = GetItem( name );
	if (pProp == NULL)
	{
		return Default;
	}

	CExplicitTypedProperty<T>* pTypedProp = DynamicCast<CExplicitTypedProperty<T> >( pProp );

	if (pTypedProp == NULL)
	{
		throw std::runtime_error( "property type" );
	}

	return pTypedProp->Get();
}

template <typename T>
T const& CUserPropertyCollection::GetElement( string const& name, int32 Index, T const& Default ) const
{
	if (Index < 0)
	{
		throw std::invalid_argument( "array property index" );
	}

	CProperty const* pProp = GetItem( name );
	if (pProp == NULL)
	{
		return Default;
	}

	CExplicitTypedProperty<std::vector<T> >* pTypedProp = DynamicCast<CExplicitTypedProperty<std::vector<T> > >( pProp );

	if (pTypedProp == NULL)
	{
		throw std::runtime_error( "property type" );
	}

	if (Index >= static_cast<int32>( pTypedProp->Get().size() ))
	{
		throw std::invalid_argument( "array property index" );
	}

	return pTypedProp->Get()[Index];
}

}
}
