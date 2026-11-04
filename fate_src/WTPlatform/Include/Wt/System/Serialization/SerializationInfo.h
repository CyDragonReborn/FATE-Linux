//
//	SerializationInfo.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/System/Object.h"
#include "WT/System/Platform.h"
#include "WT/System/DateTime.h"
#include "WT/System/Serialization/Formatters/Formatter.h"
#include "WT/System/TypeTraits.h"

#include <vector>

namespace WT
{
namespace System
{
namespace Serialization
{

typedef SmartPointer<class CSerializationInfo> HSerializationInfo;
class CSerializationInfo : 
	public CObject
{
public:
	WT_CLASS( CSerializationInfo, CObject );

	CSerializationInfo(CFormatter* FormatterObj);
	virtual ~CSerializationInfo();

	uint32 GetObjectFormatVersion() const;

	// this method can be called in ISerializable::FinalizeDeserialization() to determine if another
	// object's FinalizeDeserialization() methods has been called yet.
	bool ObjectIsFinalized(CObject const * pObject) const;

	// call this method in DeserializationComplete() proir to using an object reference
//	void Referencing(Object* pObject);

	// Overloaded. Adds a value into the CSerializationInfo. 
	template <typename T>
	void AddValue(const wstring& name, T const & value);

	template <typename T>
	void AddValue(const string& name, T const & value);


	// Overloaded. Gets a value from the CSerializationInfo
	template <typename T>
	void GetValue(const wstring& name, T& value);

	template <typename T>
	void GetValue(const string& name, T& value);


	// Block operatorations (streams & containers)

	// Once BeginStreamBlock() has been called do not call AddValue() or GetValue() 
	// until after EndStreamBlock() has been called.
	IO::CStream* BeginStreamBlock(const wstring& name);
	IO::CStream* BeginStreamBlock(const string& name);
	void EndStreamBlock();

	void AddContainerBegin(const wstring& name);
	void AddContainerBegin(const string& name);
	void AddContainerBeginWithCount(const wstring& name, uint32 count);
	void AddContainerBeginWithCount(const string& name, uint32 count);
	void AddContainerEnd();

	void GetContainerBegin(const wstring& name);
	void GetContainerBegin(const string& name);
	uint32 GetContainerBeginWithCount(const wstring& name);
	uint32 GetContainerBeginWithCount(const string& name);
	void GetContainerEnd();


	template <typename T>
	void AddBaseClass( wstring const& name, T& value )
	{
		uint32 const PreviousVersion = m_Formatter->GetCurrentObjectFormatVersion();
		uint32 const NewVersion = T::_BaseClass::GetClassFormatVersion();

		AddContainerBegin( name );
		AddValue( "version", NewVersion );

		AddContainerBegin( "data" );
		m_Formatter->SetCurrentObjectFormatVersion( NewVersion );
		value.T::_BaseClass::GetObjectData( this, m_Formatter->GetSerializationContext() );
		m_Formatter->SetCurrentObjectFormatVersion( PreviousVersion );
		AddContainerEnd();

		AddContainerEnd();
	}

	template <typename T>
	void GetBaseClass( wstring const& name, T& value )
	{
		uint32 const PreviousVersion = m_Formatter->GetCurrentObjectFormatVersion();

		GetContainerBegin( name );
		uint32 NewVersion;
		GetValue( "version", NewVersion );

		GetContainerBegin( "data" );
		m_Formatter->SetCurrentObjectFormatVersion( NewVersion );
		value.T::_BaseClass::SetObjectData( this, m_Formatter->GetSerializationContext() );
		m_Formatter->SetCurrentObjectFormatVersion( PreviousVersion );
		GetContainerEnd();

		GetContainerEnd();
	}

protected:

	// AddBaseTypeValue is a template method which has specializations for 
	// all supported top level types: all base types,
	// and Object pointers and references (but not specializations to handle classes derived 
	// from Object AddValue() handles casting classes up to Object if needed.
	//
	// This method is also overloaded to support three container types: std::list, std::vector
	// and std::map 
	template <typename T>
	void AddBaseTypeValue(const wstring& name, T const & value);


	template <> inline void AddBaseTypeValue( const wstring& name, const int8& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const int16& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const int32& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const int64& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const uint8& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const uint16& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const uint32& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const uint64& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const float32& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const float64& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const bool& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const CDateTime& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const CTimeSpan& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const string& value );
	template <> inline void AddBaseTypeValue( const wstring& name, const wstring& value );


	template <typename T>
	void AddObjectValue(const wstring& name, T const & value);

	template <> inline void AddObjectValue( const wstring& name, CObject const & value );
	template <> inline void AddObjectValue( const wstring& name, CObject* const & value );


	template <typename T, typename IsBaseType>
	void AddBaseTypeVsNonBaseTypeValue(const wstring& name, T const & value, IsBaseType* );




	// makes distinctions between Object and base type values
	template <typename T, typename IsObjectType>
	void GetValue3(const wstring& name, T& value, IsObjectType*);

	template <typename T> void GetValue3(const wstring& name, T& value, WT::FalseType* pIsObj);
	template <typename T> void GetValue3(const wstring& name, T& value, WT::TrueType* pIsObj);


	// makes distinctions between GetValue for Objects by reference and Object by value
	template <typename T, typename IsObjectPtrType>
	void GetValue4(const wstring& name, T& value, IsObjectPtrType*);

	template <typename T> void GetValue4(const wstring& name, T& value, WT::TrueType*);
	template <typename T> void GetValue4(const wstring& name, T& value, WT::FalseType*);


	template <typename T>
	void GetValueForBasicType(const wstring& name, T& value);



	// partial template specializations for stl/std containers (vector, list and map)
	template <typename T>
	void AddBaseTypeValue(const wstring& name, std::list<T> const & container);

	template <typename T>
	void GetValueForBasicType(const wstring& name, std::list<T>& container);


	template <typename T>
	void AddBaseTypeValue(const wstring& name, std::vector<T> const & container);

	template <typename T>
	void GetValueForBasicType(const wstring& name, std::vector<T>& container);


	template <typename K, typename T>
	void AddBaseTypeValue(const wstring& name, std::map<K,T> const & container);

	template <typename K, typename T>
	void GetValueForBasicType(const wstring& name, std::map<K,T>& container);


protected:
	CSerializationInfo();

	void SerializeValueType(wstring const & name, CObject const * value);
	void DeserializeValueType(wstring const & name, CObject* value);

	// helpers for serializing out vectors and lists
	template <typename T>
	void AddContainerValue(const wstring& name, T const & container);

	template <typename T>
	void GetContainerValue(const wstring& name, T& container);

private:
	CFormatter* m_Formatter;
};

} // namespace Serialization
} // namespace System
} // namespace WT

#include "WT/System/Serialization/SerializationInfo.inl"

