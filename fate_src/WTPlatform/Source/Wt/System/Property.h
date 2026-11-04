#pragma once

#include "WT/System/Object.h"
#include "WT/System/Type.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationException.h"

#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <map>

#pragma pack( push, 1 )

namespace WT
{
namespace System
{

namespace Serialization
{
	class CSerializationContext;
	class CSerializationInfo;
}

using namespace WT::System::Serialization;

typedef SmartPointer<class CProperty> HProperty;
class CProperty : public CObject
{
public:
	WT_ABSTRACT_CLASS(CProperty, CObject);

	//Get the type of value held by the property
	virtual const CType* GetPropertyType() const = 0;

	virtual bool IsReadable() const = 0;
	virtual bool IsWriteable() const = 0;

	// Serialization support
	// We can't use GetObjectData/SetObjectData, because PropertyCollection needs extra
	// information about the instance to apply the property accessors to.

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* sContext) const = 0;
	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* sContext) = 0;
};

template<typename ValueType>
class CValueTypedProperty : public CProperty
{
public:
	WT_ABSTRACT_CLASS( CValueTypedProperty<ValueType>, CProperty );

	virtual const CType* GetPropertyType() const
	{
		return WT::TypeTraits<ValueType>::GetType();
	}

	virtual void GetValue( CObject const* pObject, ValueType& Value ) const = 0;
	virtual void SetValue( CObject* pObject, ValueType const& Value ) = 0;
};

template<typename ValueType>
class CExplicitTypedProperty : public CValueTypedProperty<ValueType>, public Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS(CExplicitTypedProperty<ValueType>, CValueTypedProperty<ValueType>);
		WT_INHERITS_INTERFACE( Serialization::ISerializable );
	WT_END_CLASS();

	CExplicitTypedProperty() 
	{
	}

	CExplicitTypedProperty( ValueType const& Value ) 
		: m_Value( Value )
	{
	}

	virtual void GetValue( CObject const* pObject, ValueType& Value ) const
	{
		pObject;
		Value = Get();
	}

	virtual void SetValue( CObject* pObject, ValueType const& Value )
	{
		pObject; 
		Set( Value );
	}

	void Set( const ValueType& Value )
	{
		m_Value = Value;
	}

	ValueType& Get()
	{
		return m_Value;
	}

	ValueType const& Get() const
	{
		return m_Value;
	}

	virtual bool IsReadable() const
	{
		return true;
	}

	virtual bool IsWriteable() const
	{
		return true;
	}

	// Serialization support

	virtual void GetObjectData(CSerializationInfo* sInfo, CSerializationContext* sContext) const
	{
		sContext;
		sInfo->AddValue( "value", m_Value );
	}

	virtual void SetObjectData(CSerializationInfo* sInfo, CSerializationContext* sContext)
	{
		sContext;
		sInfo->GetValue( "value", m_Value );
	}

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* ) const
	{
		Instance;
		SerialInfo->AddValue( "value", m_Value );
	}

	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* )
	{
		Instance;
		SerialInfo->GetValue("value", m_Value );
	}

private:
	ValueType m_Value;
};

template<class Class , typename ValueType>
class CByValTypedProperty : public CValueTypedProperty<ValueType>
{
public:
	typedef CByValTypedProperty<Class, ValueType> MyClassType;
	WT_CLASS_NO_CREATOR( MyClassType, CValueTypedProperty<ValueType> );

	typedef void ( Class::*SetMethodType ) ( ValueType );
	typedef ValueType (Class::*GetMethodType) () const;

	CByValTypedProperty ( SetMethodType SetMethod , GetMethodType GetMethod )
	{
		m_SetMethod = SetMethod;
		m_GetMethod = GetMethod;
	}

	virtual void GetValue( CObject const* pObject, ValueType& Value ) const
	{
		if (pObject == NULL) throw std::invalid_argument( "object" );
		Class* pInstance = WT::DynamicCast<Class>( pObject );
		if (pInstance == NULL) throw std::invalid_argument( "object" );
		Value = Get( pInstance );
	}

	virtual void SetValue( CObject* pObject, ValueType const& Value )
	{
		if (pObject == NULL) throw std::invalid_argument( "object" );
		Class* pInstance = WT::DynamicCast<Class>( pObject );
		if (pInstance == NULL) throw std::invalid_argument( "object" );
		Set( pInstance, Value );
	}

	bool IsReadable() const
	{
		return m_GetMethod != NULL;
	}

	bool IsWriteable() const
	{
		return m_SetMethod != NULL;
	}

	void Set ( Class* Instance , const ValueType& Value ) const
	{
		if (!IsWriteable()) throw std::runtime_error( "property not writable" );
		(Instance->*m_SetMethod) ( Value );
	}

	ValueType Get ( Class const* Instance ) const
	{
		if (!IsReadable()) throw std::runtime_error( "property not readable" );
		return (Instance->*m_GetMethod)();
	}

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* ) const
	{
		assert(Instance);
		Class* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		ValueType v = Get(pClassInstance);
		SerialInfo->AddValue("value", v );
	}

	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* )
	{
		assert(Instance);
		Class* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		ValueType v;
		SerialInfo->GetValue("value", v );
		Set(pClassInstance, v);
	}

	SetMethodType m_SetMethod;
	GetMethodType m_GetMethod;
};


template<class Class , typename ValueType>
class CByRefTypedProperty : public CValueTypedProperty<ValueType>
{
public:
	typedef CByRefTypedProperty<Class, ValueType> MyClassType;
	WT_CLASS_NO_CREATOR( MyClassType, CValueTypedProperty<ValueType> );

	typedef void ( Class::*SetMethodType ) ( ValueType const & );
	typedef ValueType const& (Class::*GetMethodType) () const;

	CByRefTypedProperty ( SetMethodType SetMethod , GetMethodType GetMethod )
	{
		m_SetMethod = SetMethod;
		m_GetMethod = GetMethod;
	}

	virtual void GetValue( CObject const* pObject, ValueType& Value ) const
	{
		if (pObject == NULL) throw std::invalid_argument( "object" );
		Class* pInstance = WT::DynamicCast<Class>( pObject );
		if (pInstance == NULL) throw std::invalid_argument( "object" );
		Value = Get( pInstance );
	}

	virtual void SetValue( CObject* pObject, ValueType const& Value )
	{
		if (pObject == NULL) throw std::invalid_argument( "object" );
		Class* pInstance = WT::DynamicCast<Class>( pObject );
		if (pInstance == NULL) throw std::invalid_argument( "object" );
		Set( pInstance, Value );
	}

	bool IsReadable() const
	{
		return m_GetMethod != NULL;
	}

	bool IsWriteable() const
	{
		return m_SetMethod != NULL;
	}

	void Set ( Class* Instance , const ValueType& Value ) const
	{
		if (!IsWriteable()) throw std::runtime_error( "property not writable" );
		(Instance->*m_SetMethod) ( Value );
	}

	ValueType const& Get( Class const* Instance ) const
	{
		if (!IsReadable()) throw std::runtime_error( "property not readable" );
		return (Instance->*m_GetMethod)();
	}

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* ) const
	{
		assert(Instance);
		Class const* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		ValueType const & v = Get( pClassInstance );
		SerialInfo->AddValue("value", v );
	}

	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* )
	{
		assert(Instance);
		Class* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		ValueType v;
		SerialInfo->GetValue("value", v );
		Set(pClassInstance, v);
	}

	SetMethodType m_SetMethod;
	GetMethodType m_GetMethod;
};


template<class Class , typename ValueType>
class CObjectProperty : public CValueTypedProperty<ValueType*>
{
public:
	typedef void ( Class::*SetMethodType ) ( ValueType* );
	typedef ValueType* (Class::*GetMethodType) () const;

	typedef CObjectProperty<Class, ValueType> MyClassType;
	WT_CLASS_NO_CREATOR( MyClassType, CValueTypedProperty<ValueType*> );

	CObjectProperty ( SetMethodType SetMethod , GetMethodType GetMethod )
	{
		m_SetMethod = SetMethod;
		m_GetMethod = GetMethod;
	}

	virtual void GetValue( CObject const* pObject, ValueType*& Value ) const
	{
		if (pObject == NULL) throw std::invalid_argument( "object" );
		Class* pInstance = WT::DynamicCast<Class>( pObject );
		if (pInstance == NULL) throw std::invalid_argument( "object" );
		Value = Get( pInstance );
	}

	virtual void SetValue( CObject* pObject, ValueType* const& Value )
	{
		if (pObject == NULL) throw std::invalid_argument( "object" );
		Class* pInstance = WT::DynamicCast<Class>( pObject );
		if (pInstance == NULL) throw std::invalid_argument( "object" );
		Set( pInstance, Value );
	}

	bool IsReadable() const
	{
		return m_GetMethod != NULL;
	}

	bool IsWriteable() const
	{
		return m_SetMethod != NULL;
	}

	void Set ( Class* Instance , ValueType* Value ) const
	{
		if (!IsWriteable()) throw std::runtime_error( "property not writable" );
		(Instance->*m_SetMethod) ( Value );
	}

	ValueType* Get( Class const* Instance ) const
	{
		if (!IsReadable()) throw std::runtime_error( "property not readable" );
		return (Instance->*m_GetMethod)();
	}

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* ) const
	{
		assert(Instance);
		Class const* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		ValueType* v = Get( pClassInstance );
		SerialInfo->AddValue("value", v );
	}

	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* )
	{
		assert(Instance);
		Class* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		ValueType* v = NULL;
		SerialInfo->GetValue("value", v );
		Set(pClassInstance, v);
	}

	SetMethodType m_SetMethod;
	GetMethodType m_GetMethod;
};

//================

// the GetPropValueType template argument is the return type to use for GetItem()
// this allows ByValueTypedArrayProperty and ByRefTypedArrayProperty to simply inherit from
// this class and pass ValueType for the ByValue implementation and ValueType const & for
// the ByRef implementation.
//
template<class Class , typename ValueType, typename GetPropValueType>
class CTypedArrayProperty : public CProperty
{
public:
	typedef CTypedArrayProperty<Class, ValueType, GetPropValueType> MyClassType;
	WT_CLASS_NO_CREATOR( MyClassType, CProperty);

	typedef void ( Class::*SetItemMethodType ) ( uint32 i, ValueType const & );
	typedef GetPropValueType (Class::*GetItemMethodType) (uint32 i) const;

	typedef void ( Class::*SetCountMethodType ) ( uint32 i );
	typedef uint32 (Class::*GetCountMethodType) () const;

	CTypedArrayProperty ( SetItemMethodType SetItemMethod , GetItemMethodType GetItemMethod, 
					SetCountMethodType SetCountMethod, GetCountMethodType GetCountMethod )
	{
		m_SetItemMethod = SetItemMethod;
		m_GetItemMethod = GetItemMethod;
		m_SetCountMethod = SetCountMethod;
		m_GetCountMethod = GetCountMethod;

		// if a SetItem method is null then the SetCount method should be also
		assert( (m_SetItemMethod == NULL && m_SetCountMethod == NULL) || ( m_SetItemMethod != NULL && m_SetCountMethod != NULL ) );

		// if a GetItem method is null then the GetCount method should be also
		assert( (m_GetItemMethod == NULL && m_GetCountMethod == NULL) || ( m_GetItemMethod != NULL && m_GetCountMethod != NULL ) );
	}

	bool IsReadable() const
	{
		return m_GetItemMethod != NULL;
	}

	bool IsWriteable() const
	{
		return m_SetItemMethod != NULL;
	}

	void SetItem ( Class* Instance , uint32 i, const ValueType& Value ) const
	{
		(Instance->*m_SetItemMethod) ( i, Value );
	}

	GetPropValueType GetItem ( Class const* Instance, uint32 i ) const
	{
		return (Instance->*m_GetItemMethod)(i);
	}

	void SetCount ( Class* Instance , uint32 i ) const
	{
		(Instance->*m_SetCountMethod) ( i );
	}

	uint32 GetCount ( Class const* Instance ) const
	{
		return (Instance->*m_GetCountMethod)();
	}

	virtual const CType* GetPropertyType() const
	{
		return WT::TypeTraits<ValueType>::GetType();
	}

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* ) const
	{
		assert(Instance);
		Class const* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		uint32 count = GetCount(pClassInstance);
		SerialInfo->AddContainerBeginWithCount("value", count);
		for (uint32 i = 0; i < count; i++)
		{
			GetPropValueType v = GetItem(pClassInstance, i);
			SerialInfo->AddValue("item", v );
		}
		SerialInfo->AddContainerEnd();
	}

	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* )
	{
		assert(Instance);
		Class* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		uint32 count = SerialInfo->GetContainerBeginWithCount("value");
		SetCount(pClassInstance, count);
		for (uint32 i = 0; i < count; i++)
		{
			ValueType v;
			SerialInfo->GetValue("item", v );
			SetItem(pClassInstance, i, v);
			
		}
		SerialInfo->GetContainerEnd();
	}

	SetItemMethodType m_SetItemMethod;
	GetItemMethodType m_GetItemMethod;
	SetCountMethodType m_SetCountMethod;
	GetCountMethodType m_GetCountMethod;
};




template<class Class , typename ValueType>
class CByValueTypedArrayProperty : public CTypedArrayProperty<Class, ValueType, ValueType>
{
public:
	typedef CTypedArrayProperty<Class, ValueType, ValueType> MyBaseClassType;
	typedef CByValueTypedArrayProperty<Class, ValueType> MyClassType;
	WT_CLASS_NO_CREATOR( MyClassType, MyBaseClassType);

	CByValueTypedArrayProperty ( SetItemMethodType SetItemMethod , 
						 GetItemMethodType GetItemMethod, 
						 SetCountMethodType SetCountMethod, 
						 GetCountMethodType GetCountMethod ) : 
		CTypedArrayProperty<Class, ValueType, ValueType>(SetItemMethod, GetItemMethod, SetCountMethod, GetCountMethod )
	{
	}
};

template<class Class , typename ValueType>
class CByRefTypedArrayProperty : public CTypedArrayProperty<Class, ValueType, ValueType const &>
{
public:
	typedef CTypedArrayProperty<Class, ValueType, ValueType const & > MyBaseClassType;
	typedef CByRefTypedArrayProperty<Class, ValueType> MyClassType;
	WT_CLASS_NO_CREATOR( MyClassType, MyBaseClassType);

	CByRefTypedArrayProperty ( SetItemMethodType SetItemMethod , 
						 GetItemMethodType GetItemMethod, 
						 SetCountMethodType SetCountMethod, 
						 GetCountMethodType GetCountMethod ) : 
		CTypedArrayProperty<Class, ValueType, ValueType const &>(SetItemMethod, GetItemMethod, SetCountMethod, GetCountMethod )
	{
	}
};




template<class Class , typename ValueType>
class CObjectArrayProperty : public CProperty
{
public:
	typedef void ( Class::*SetItemMethodType ) ( uint32 i, ValueType* );
	typedef ValueType* (Class::*GetItemMethodType) ( uint32 i ) const;

	typedef void ( Class::*SetCountMethodType ) ( uint32 i );
	typedef uint32 (Class::*GetCountMethodType) ( ) const;


	CObjectArrayProperty ( SetItemMethodType SetItemMethod , GetItemMethodType GetItemMethod,
					 SetCountMethodType SetCountMethod , GetCountMethodType GetCountMethod)
	{
		m_SetItemMethod = SetItemMethod;
		m_GetItemMethod = GetItemMethod;
		m_SetCountMethod = SetCountMethod;
		m_GetCountMethod = GetCountMethod;
		
		// if a SetItem method is null then the SetCount method should be also
		assert( (m_SetItemMethod == NULL && m_SetCountMethod == NULL) || ( m_SetItemMethod != NULL && m_SetCountMethod != NULL ) );

		// if a GetItem method is null then the GetCount method should be also
		assert( (m_GetItemMethod == NULL && m_GetCountMethod == NULL) || ( m_GetItemMethod != NULL && m_GetCountMethod != NULL ) );
	}

	bool IsReadable() const
	{
		return m_GetItemMethod != NULL;
	}

	bool IsWriteable() const
	{
		return m_SetItemMethod != NULL;
	}

	void SetItem ( Class* Instance , int32 i, ValueType* Value ) const
	{
		(Instance->*m_SetItemMethod) ( i, Value );
	}

	ValueType* GetItem ( Class const* Instance, int32 i ) const
	{
		return (Instance->*m_GetItemMethod)( i );
	}

	void SetCount ( Class* Instance , uint32 i ) const
	{
		(Instance->*m_SetCountMethod) ( i );
	}

	uint32 GetCount ( Class const* Instance ) const
	{
		return (Instance->*m_GetCountMethod)();
	}

	virtual const CType* GetPropertyType() const
	{
		return WT::TypeTraits<ValueType*>::GetType();
	}

	virtual void SerializePropertyValue(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* ) const
	{
		assert(Instance);
		Class const* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		uint32 count = GetCount( pClassInstance );
		SerialInfo->AddContainerBeginWithCount("value", count);
		for (uint32 i = 0; i < count; i++)
		{
			ValueType* v = GetItem(pClassInstance, i);
			SerialInfo->AddValue<CObject*>("item", v );
		}
		SerialInfo->AddContainerEnd();
	}

	virtual void DeserializePropertyValue(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* )
	{
		assert(Instance);
		Class* pClassInstance = DynamicCast<Class>(Instance);
		assert(pClassInstance != NULL);

		uint32 count = SerialInfo->GetContainerBeginWithCount("value");
		SetCount(pClassInstance, count);
		for (uint32 i = 0; i < count; i++)
		{
			CObject* pObj = NULL;
			SerialInfo->GetValue<CObject*>("item", pObj );
			ValueType* pItemType = DynamicCast<ValueType>(pObj);
			if (pObj != NULL && pItemType == NULL)
			{
				std::ostringstream ssmsg;
				ssmsg << "Unexpected deserialization error. Deserialized an unexpected object type when reading in an item of a property array.";
				throw CSerializationException(ssmsg.str());
			}
			SetItem(pClassInstance, i, pItemType);
			
		}
		SerialInfo->GetContainerEnd();
	}

	SetItemMethodType m_SetItemMethod;
	GetItemMethodType m_GetItemMethod;
	SetCountMethodType m_SetCountMethod;
	GetCountMethodType m_GetCountMethod;
};

//================

// These macros should be placed between WT_BEGIN_CLASS() and WT_END_CLASS() or
// WT_BEGIN_INTERFACE() and WT_END_INTERFACE() macros (declared in WT/System/Type.h).
#define WT_BEGIN_PROPERTIES() \
			g_pMyType->SetDefinePropertiesFunction(DefineProperties); \
			_ThisClass::_FinalizePropertiesChain( g_pMyType ); \
		} \
		return g_pMyType; \
	} \
	\
	static void WTCall DefineProperties ( WT::System::CPropertyCollection* _Properties ) \
	{ \

#define WT_END_PROPERTIES() \
	} \
	static WT::System::CType const* WTCall _FinalizePropertiesChain( WT::System::CType* g_pMyType ) \
	{ \
		{ \


#define WT_PROP_RW_BYVAL(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByValTypedProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName , &_ThisClass::Get##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_PROP_RW_BYREF(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByRefTypedProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName , &_ThisClass::Get##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_PROP_RW_OBJECT(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CObjectProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName , &_ThisClass::Get##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );



#define WT_PROP_RO_BYVAL(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByValTypedProperty<_ThisClass,ValueType > ( NULL , &_ThisClass::Get##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_PROP_RO_BYREF(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByRefTypedProperty<_ThisClass,ValueType > ( NULL , &_ThisClass::Get##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_PROP_RO_OBJECT(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CObjectProperty<_ThisClass,ValueType > ( NULL , &_ThisClass::Get##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );



#define WT_PROP_WO_BYVAL(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByValTypedProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName , NULL ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_PROP_WO_BYREF(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByRefTypedProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName , NULL ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_PROP_WO_OBJECT(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CObjectProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName , NULL ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );


// Array Properties

// RW ARRAY MACROS
#define WT_ARRAY_PROP_RW_BYVAL(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByValueTypedArrayProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName ,  &_ThisClass::Get##PropertyName, &_ThisClass::SetCount##PropertyName, &_ThisClass::GetCount##PropertyName); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_ARRAY_PROP_RW_BYREF(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByRefTypedArrayProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName ,  &_ThisClass::Get##PropertyName, &_ThisClass::SetCount##PropertyName, &_ThisClass::GetCount##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_ARRAY_PROP_RW_OBJECT(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CObjectArrayProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName ,  &_ThisClass::Get##PropertyName, &_ThisClass::SetCount##PropertyName, &_ThisClass::GetCount##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );


// Read-only ARRAY MACROS
#define WT_ARRAY_PROP_RO_BYVAL(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByValueTypedArrayProperty<_ThisClass,ValueType > ( NULL,  &_ThisClass::Get##PropertyName, NULL, &_ThisClass::GetCount##PropertyName); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_ARRAY_PROP_RO_BYREF(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByRefTypedArrayProperty<_ThisClass,ValueType > ( NULL,  &_ThisClass::Get##PropertyName, NULL, &_ThisClass::GetCount##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_ARRAY_PROP_RO_OBJECT(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CObjectArrayProperty<_ThisClass,ValueType > ( NULL,  &_ThisClass::Get##PropertyName, NULL, &_ThisClass::GetCount##PropertyName ); \
	_Properties->Add ( #PropertyName , PropertyName##Property );


// Write-only ARRAY MACROS
#define WT_ARRAY_PROP_WO_BYVAL(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByValueTypedArrayProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName ,  NULL, &_ThisClass::SetCount##PropertyName, NULL); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_ARRAY_PROP_WO_BYREF(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CByRefTypedArrayProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName ,  NULL, &_ThisClass::SetCount##PropertyName, NULL); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

#define WT_ARRAY_PROP_WO_OBJECT(PropertyName,ValueType) \
	WT::System::CProperty* PropertyName##Property = new WT::System::CObjectArrayProperty<_ThisClass,ValueType > ( &_ThisClass::Set##PropertyName ,  NULL, &_ThisClass::SetCount##PropertyName, NULL); \
	_Properties->Add ( #PropertyName , PropertyName##Property );

}
}

#pragma pack( pop )

#include "WT/System/PropertyCollection.h"
