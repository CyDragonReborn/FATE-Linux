//
//	CSerializationInfo.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/Serialization/SerializationException.h"


namespace WT
{
namespace System
{
namespace Serialization
{


// int8 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const int8& value )
{
	m_Formatter->WriteInt8( value, name );
}

// int16 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const int16& value )
{
	m_Formatter->WriteInt16( value, name );
}

// int32 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const int32& value )
{
	m_Formatter->WriteInt32( value, name );
}

// int64 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const int64& value )
{
	m_Formatter->WriteInt64( value, name );
}

// uint8 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const uint8& value )
{
	m_Formatter->WriteUInt8( value, name );
}

// uint16 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const uint16& value )
{
	m_Formatter->WriteUInt16( value, name );
}

// uint32 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const uint32& value )
{
	m_Formatter->WriteUInt32( value, name );
}

// uint64 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const uint64& value )
{
	m_Formatter->WriteUInt64( value, name );
}

// float32 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const float32& value )
{
	m_Formatter->WriteFloat32( value, name );
}

// float64 AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const float64& value )
{
	m_Formatter->WriteFloat64( value, name );
}

// bool AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const bool& value )
{
	m_Formatter->WriteBool( value, name );
}

// DateTime AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const CDateTime& value )
{
	m_Formatter->WriteDateTime( value, name );
}

// TimeSpan AddValue() specilization
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const CTimeSpan& value )
{
	m_Formatter->WriteTimeSpan( value, name );
}

// string AddValue() specilizations
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const string& value )
{
	m_Formatter->WriteString( value, name );
}

// string AddValue() specilizations
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, const wstring& value )
{
	m_Formatter->WriteWString( value, name );
}


// Object* AddValue() specilizations
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, CObject const & value )
{
	m_Formatter->WriteObjectByValue(value, name, value.GetType() );
}


// Object* AddValue() specilizations
template <> inline void CSerializationInfo::AddBaseTypeValue( const wstring& name, CObject* const & value )
{
	m_Formatter->WriteObjectByRef( const_cast<CObject*>(value), name, value ? value->GetType() : NULL );
}


// GetValue template specializations


template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, int8& value )
{
	value = m_Formatter->ReadInt8( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, int16& value )
{
	value = m_Formatter->ReadInt16( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, int32& value )
{
	value = m_Formatter->ReadInt32( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, int64& value )
{
	value = m_Formatter->ReadInt64( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, uint8& value )
{
	value = m_Formatter->ReadUInt8( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, uint16& value )
{
	value = m_Formatter->ReadUInt16( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, uint32& value )
{
	value = m_Formatter->ReadUInt32( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, uint64& value )
{
	value = m_Formatter->ReadUInt64( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, float32& value )
{
	value = m_Formatter->ReadFloat32( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, float64& value )
{
	value = m_Formatter->ReadFloat64( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, bool& value )
{
	value = m_Formatter->ReadBool( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, CDateTime& value )
{
	value = m_Formatter->ReadDateTime( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, CTimeSpan& value )
{
	value = m_Formatter->ReadTimeSpan( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, string& value )
{
	value = m_Formatter->ReadString( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, wstring& value )
{
	value = m_Formatter->ReadWString( name );
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, CObject& value )
{
	m_Formatter->ReadObjectByValue(name, value);
}

template <> inline void CSerializationInfo::GetValueForBasicType( const wstring& name, CObject*& value )
{
	value = m_Formatter->ReadObjectByRef( name );
}





// containers


// template implementations for containers (vector or list)
template <typename T>
inline void CSerializationInfo::AddContainerValue(const wstring& name, T const & container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	AddContainerBeginWithCount(name, static_cast<uint32>(container.size()) );

	T::const_iterator iter = container.begin();
	for (; iter != container.end(); iter++ )
	{ 
//		AddValue< TypeTraits<T::value_type>::TopType >("item", *iter);
		AddValue("item", *iter);
	}

	AddContainerEnd();
}

template <typename T>
inline void CSerializationInfo::GetContainerValue(const wstring& name, T& container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	int32 count = GetContainerBeginWithCount(name);

	container.resize(count);
	T::iterator iter = container.begin();
	for (; iter != container.end(); iter++ )
	{
//		GetValue< TypeTraits<T::value_type>::TopType >(L"item", *iter );
		GetValue(L"item", *iter);
	}

	GetContainerEnd();
}

/*
Specialize for std::vector<bool>. This is necessary because, per the C++ Standard, vector<bool> doesn't 
actually contain bool's and its iterators can't be used to get a pointer to a bool. We solve the 
problem here by reading into a local variable and copying it into the container. This implementation
won't work in general because CObject-derived types aren't filled in immediately and can't be
copied into the container.
*/
template <>
inline void CSerializationInfo::GetContainerValue(const wstring& name, std::vector<bool>& container)
{
	typedef std::vector<bool> T;

	int32 count = GetContainerBeginWithCount(name);

	container.resize(count);
	T::iterator iter = container.begin();
	for (; iter != container.end(); iter++ )
	{
//		GetValue< TypeTraits<T::value_type>::TopType >(L"item", *iter );
		T::value_type value;
		GetValue(L"item", value);
		*iter = value;
	}

	GetContainerEnd();
}

// specializations for AddValue()/GetValue() for std::list containers
template <typename T>
inline void CSerializationInfo::AddBaseTypeValue(const wstring& name, std::list<T> const & container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	AddContainerValue(name, container);
}

template <typename T>
inline void CSerializationInfo::GetValueForBasicType(const wstring& name, std::list<T>& container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	GetContainerValue(name, container);
}

// specializations for AddValue()/GetValue() for std::vector containers
template <typename T>
inline void CSerializationInfo::AddBaseTypeValue(const wstring& name, std::vector<T> const & container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	AddContainerValue(name, container);
}

template <typename T>
inline void CSerializationInfo::GetValueForBasicType(const wstring& name, std::vector<T>& container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	GetContainerValue(name, container);
}

// specializations for AddValue()/GetValue() for std::map containers
template <typename K, typename T>
inline void CSerializationInfo::AddBaseTypeValue(const wstring& name, std::map<K,T> const & container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfK = TypeTraits<K>::ToString();
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	AddContainerBeginWithCount(name, static_cast<uint32>(container.size()) );

	std::map<K,T>::const_iterator iter = container.begin();
	for (; iter != container.end(); iter++ )
	{
		AddContainerBegin("item");
		AddValue("key", iter->first);
		AddValue("value", iter->second);
		AddContainerEnd();
	}

	AddContainerEnd();
}

template <typename K, typename T>
inline void CSerializationInfo::GetValueForBasicType(const wstring& name, std::map<K,T>& container)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfK = TypeTraits<K>::ToString();
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	int32 count = GetContainerBeginWithCount(name);

	container.clear();
	std::map<K,T>::iterator iter = container.begin();
	for (int32 i = 0; i < count; i++ )
	{
		K key;
		T value;
		GetContainerBegin("item");
		GetValue("key", key );
		GetValue("value", value );
		container[key] = value;
		GetContainerEnd();
	}

	GetContainerEnd();
}








template <typename T>
inline void CSerializationInfo::AddValue(const wstring& name, T const & value)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG
	
	AddBaseTypeValue(name, static_cast< TypeTraits<T>::TopType const & >(value) );
}

template <typename T>
inline void CSerializationInfo::AddValue(const string& name, T const & value)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	AddValue( WT::AnsiToWide(name), value );
}



template <typename T>
inline void CSerializationInfo::GetValue3(const wstring& name, T& value, WT::FalseType*)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	GetValueForBasicType(name, value );
}

// GetValue (for Object*, objects by reference)
template <typename T>
inline void CSerializationInfo::GetValue4(const wstring& name, T& value, WT::TrueType*)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
	string typeNameOfTInnerType = TypeTraits< TypeTraits<T>::InnerType >::ToString();
#endif // _DEBUG

	CObject * pObj = NULL;
	GetValueForBasicType(name, pObj );
	value = DynamicCast< TypeTraits<T>::InnerType >(pObj);
	if (pObj != NULL && value == NULL)
	{
		std::ostringstream ss;
		ss << "CSerializationInfo::GetValue(): DynamicCast from Object* to ";
		ss << TypeTraits<TypeTraits<T>::InnerType>::ToString() << " Failed.";
		throw CSerializationException(ss.str());
	}
}


// GetValue (for Object&, objects by value)
template <typename T>
inline void CSerializationInfo::GetValue4(const wstring& name, T& value, WT::FalseType*)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG

	GetValueForBasicType(name, static_cast<CObject&>(value) );
}

template <typename T>
inline void CSerializationInfo::GetValue3(const wstring& name, T& value, WT::TrueType*)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
	string typeNameOfTIsPtrType = TypeTraits< TypeTraits<T>::IsPtr >::ToString();
#endif // _DEBUG

	GetValue4(name, value, static_cast<TypeTraits<T>::IsPtr*>(NULL) );
}




template <typename T>
inline void CSerializationInfo::GetValue(const wstring& name, T& value)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
	string typeNameOfTIsObjectType = TypeTraits< TypeTraits<T>::IsObject >::ToString();
#endif // _DEBUG

//	GetValueForBasicType(name, static_cast< TypeTraits<T>::TopType & >(value) );
	GetValue3(name, value, static_cast<TypeTraits<T>::IsObject*>(NULL) );
}

template <typename T>
inline void CSerializationInfo::GetValue(const string& name, T& value)
{
#ifdef _DEBUG 
	// this is to aid in debugging templates, when you are stepped into this method
	// you can examine the values of thes strings to determin the type that was used 
	// to instance this template
	string typeNameOfT = TypeTraits<T>::ToString();
#endif // _DEBUG
	GetValue( WT::AnsiToWide(name), value );
}


} // namespace Serialization
} // namespace System
} // namespace WT
