#include "WT/stdafx.h"


#include "WT/System/Platform.h"
#include "WT/System/UserPropertyCollection.h"

namespace WT
{
namespace System
{

using namespace Serialization;

//TODO consider changing the name of CUserPropertyCollection; it's not just
//for users. CExplicitPropertyCollection would be better.
WT_DEFINE_CLASS(CUserPropertyCollection)

void CUserPropertyCollection::GetObjectData(CSerializationInfo* sInfo, CSerializationContext* sContext) const
{
	SerializePropertyValues( this, sInfo, sContext );
}

void CUserPropertyCollection::SetObjectData(CSerializationInfo* sInfo, CSerializationContext* sContext)
{
	DeserializePropertyValues( this, sInfo, sContext );
}

PropertyType::Type CUserPropertyCollection::GetType( string const& name ) const
{
	CProperty* pProp = const_cast<CUserPropertyCollection*>(this)->GetItem( name );
	if (pProp == NULL)
	{
		throw std::invalid_argument( "property name" );
	}

	CType const* pType = pProp->GetType();
	assert( pType != NULL );
	if (*pType == *CExplicitTypedProperty<int8>::GetClassType()) {
		return PropertyType::Int8;
	} else if (*pType == *CExplicitTypedProperty<int16>::GetClassType()) {
		return PropertyType::Int16;
	} else if (*pType == *CExplicitTypedProperty<int32>::GetClassType()) {
		return PropertyType::Int32;
	} else if (*pType == *CExplicitTypedProperty<int64>::GetClassType()) {
		return PropertyType::Int64;
	} else if (*pType == *CExplicitTypedProperty<uint8>::GetClassType()) {
		return PropertyType::UInt8;
	} else if (*pType == *CExplicitTypedProperty<uint16>::GetClassType()) {
		return PropertyType::UInt16;
	} else if (*pType == *CExplicitTypedProperty<uint32>::GetClassType()) {
		return PropertyType::UInt32;
	} else if (*pType == *CExplicitTypedProperty<uint64>::GetClassType()) {
		return PropertyType::UInt64;
	} else if (*pType == *CExplicitTypedProperty<float32>::GetClassType()) {
		return PropertyType::Float32;
	} else if (*pType == *CExplicitTypedProperty<float64>::GetClassType()) {
		return PropertyType::Float64;
	} else if (*pType == *CExplicitTypedProperty<bool>::GetClassType()) {
		return PropertyType::Bool;
	} else if (*pType == *CExplicitTypedProperty<string>::GetClassType()) {
		return PropertyType::String;
	} else if (*pType == *CExplicitTypedProperty<wstring>::GetClassType()) {
		return PropertyType::WString;
	} else if (*pType == *CExplicitTypedProperty<CDateTime>::GetClassType()) {
		return PropertyType::DateTime;
	} else if (*pType == *CExplicitTypedProperty<CTimeSpan>::GetClassType()) {
		return PropertyType::TimeSpan;
	} else if (*pType == *CExplicitTypedProperty<CObject*>::GetClassType()) {
		return PropertyType::ObjectPtr;
	} else if (*pType == *CExplicitTypedProperty<std::vector<int8> >::GetClassType()) {
		return PropertyType::Int8Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<int16> >::GetClassType()) {
		return PropertyType::Int16Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<int32> >::GetClassType()) {
		return PropertyType::Int32Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<int64> >::GetClassType()) {
		return PropertyType::Int64Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<uint8> >::GetClassType()) {
		return PropertyType::UInt8Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<uint16> >::GetClassType()) {
		return PropertyType::UInt16Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<uint32> >::GetClassType()) {
		return PropertyType::UInt32Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<uint64> >::GetClassType()) {
		return PropertyType::UInt64Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<float32> >::GetClassType()) {
		return PropertyType::Float32Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<float64> >::GetClassType()) {
		return PropertyType::Float64Array;
	} else if (*pType == *CExplicitTypedProperty<std::vector<bool> >::GetClassType()) {
		return PropertyType::BoolArray;
	} else if (*pType == *CExplicitTypedProperty<std::vector<string> >::GetClassType()) {
		return PropertyType::StringArray;
	} else if (*pType == *CExplicitTypedProperty<std::vector<wstring> >::GetClassType()) {
		return PropertyType::WStringArray;
	} else if (*pType == *CExplicitTypedProperty<std::vector<CDateTime> >::GetClassType()) {
		return PropertyType::DateTimeArray;
	} else if (*pType == *CExplicitTypedProperty<std::vector<CTimeSpan> >::GetClassType()) {
		return PropertyType::TimeSpanArray;
	} else if (*pType == *CExplicitTypedProperty<std::vector<CObject*> >::GetClassType()) {
		return PropertyType::ObjectPtrArray;
	}

	return PropertyType::Unknown;
}

void CUserPropertyCollection::SetType( string const& name, PropertyType::Type Type )
{
	try
	{
		PropertyType::Type CurrentType = GetType( name );
		if (CurrentType == Type)
		{
			return;
		}
	} catch (...) {
	}

	CProperty* pProp = NULL;
	
	switch (Type)
	{
	case PropertyType::Int8:			pProp = new CExplicitTypedProperty<int8>( 0 ); break;
	case PropertyType::Int16:			pProp = new CExplicitTypedProperty<int16>( 0 ); break;
	case PropertyType::Int32:			pProp = new CExplicitTypedProperty<int32>( 0 ); break;
	case PropertyType::Int64:			pProp = new CExplicitTypedProperty<int64>( 0 ); break;
	case PropertyType::UInt8:			pProp = new CExplicitTypedProperty<uint8>( 0 ); break;
	case PropertyType::UInt16:			pProp = new CExplicitTypedProperty<uint16>( 0 ); break;
	case PropertyType::UInt32:			pProp = new CExplicitTypedProperty<uint32>( 0 ); break;
	case PropertyType::UInt64:			pProp = new CExplicitTypedProperty<uint64>( 0 ); break;
	case PropertyType::Float32:			pProp = new CExplicitTypedProperty<float32>( 0.0f ); break;
	case PropertyType::Float64:			pProp = new CExplicitTypedProperty<float64>( 0.0 ); break;
	case PropertyType::Bool:			pProp = new CExplicitTypedProperty<bool>( false ); break;
	case PropertyType::String:			pProp = new CExplicitTypedProperty<string>; break;
	case PropertyType::WString:			pProp = new CExplicitTypedProperty<wstring>; break;
	case PropertyType::DateTime:		pProp = new CExplicitTypedProperty<CDateTime>; break;
	case PropertyType::TimeSpan:		pProp = new CExplicitTypedProperty<CTimeSpan>; break;
	case PropertyType::ObjectPtr:		pProp = new CExplicitTypedProperty<CObject*>; break;
	case PropertyType::Int8Array:		pProp = new CExplicitTypedProperty<std::vector<int8> >; break;
	case PropertyType::Int16Array:		pProp = new CExplicitTypedProperty<std::vector<int16> >; break;
	case PropertyType::Int32Array:		pProp = new CExplicitTypedProperty<std::vector<int32> >; break;
	case PropertyType::Int64Array:		pProp = new CExplicitTypedProperty<std::vector<int64> >; break;
	case PropertyType::UInt8Array:		pProp = new CExplicitTypedProperty<std::vector<uint8> >; break;
	case PropertyType::UInt16Array:		pProp = new CExplicitTypedProperty<std::vector<uint16> >; break;
	case PropertyType::UInt32Array:		pProp = new CExplicitTypedProperty<std::vector<uint32> >; break;
	case PropertyType::UInt64Array:		pProp = new CExplicitTypedProperty<std::vector<uint64> >; break;
	case PropertyType::Float32Array:	pProp = new CExplicitTypedProperty<std::vector<float32> >; break;
	case PropertyType::Float64Array:	pProp = new CExplicitTypedProperty<std::vector<float64> >; break;
	case PropertyType::BoolArray:		pProp = new CExplicitTypedProperty<std::vector<bool> >; break;
	case PropertyType::StringArray:		pProp = new CExplicitTypedProperty<std::vector<string> >; break;
	case PropertyType::WStringArray:	pProp = new CExplicitTypedProperty<std::vector<wstring> >; break;
	case PropertyType::DateTimeArray:	pProp = new CExplicitTypedProperty<std::vector<CDateTime> >; break;
	case PropertyType::TimeSpanArray:	pProp = new CExplicitTypedProperty<std::vector<CTimeSpan> >; break;
	case PropertyType::ObjectPtrArray:	pProp = new CExplicitTypedProperty<std::vector<CObject*> >; break;
	default:
		throw std::invalid_argument( "property type" );
	}

	assert( pProp != NULL );
	Set( name, pProp );
}

int32 CUserPropertyCollection::GetArrayElementCount( string const& name ) const
{
	CProperty const* pProp = GetItem( name );

	size_t size = 0;
	switch (GetType( name ))
	{
		case PropertyType::Int8Array:		size = static_cast< CExplicitTypedProperty<std::vector<int8> > const* >( pProp )->Get().size(); break;
		case PropertyType::Int16Array:		size = static_cast< CExplicitTypedProperty<std::vector<int16> > const* >( pProp )->Get().size(); break;
		case PropertyType::Int32Array:		size = static_cast< CExplicitTypedProperty<std::vector<int32> > const* >( pProp )->Get().size(); break;
		case PropertyType::Int64Array:		size = static_cast< CExplicitTypedProperty<std::vector<int64> > const* >( pProp )->Get().size(); break;
		case PropertyType::UInt8Array:		size = static_cast< CExplicitTypedProperty<std::vector<uint8> > const* >( pProp )->Get().size(); break;
		case PropertyType::UInt16Array:		size = static_cast< CExplicitTypedProperty<std::vector<uint16> > const* >( pProp )->Get().size(); break;
		case PropertyType::UInt32Array:		size = static_cast< CExplicitTypedProperty<std::vector<uint32> > const* >( pProp )->Get().size(); break;
		case PropertyType::UInt64Array:		size = static_cast< CExplicitTypedProperty<std::vector<uint64> > const* >( pProp )->Get().size(); break;
		case PropertyType::Float32Array:	size = static_cast< CExplicitTypedProperty<std::vector<float32> > const* >( pProp )->Get().size(); break;
		case PropertyType::Float64Array:	size = static_cast< CExplicitTypedProperty<std::vector<float64> > const* >( pProp )->Get().size(); break;
		case PropertyType::BoolArray:		size = static_cast< CExplicitTypedProperty<std::vector<bool> > const* >( pProp )->Get().size(); break;
		case PropertyType::StringArray:		size = static_cast< CExplicitTypedProperty<std::vector<string> > const* >( pProp )->Get().size(); break;
		case PropertyType::WStringArray:	size = static_cast< CExplicitTypedProperty<std::vector<wstring> > const* >( pProp )->Get().size(); break;
		case PropertyType::DateTimeArray:	size = static_cast< CExplicitTypedProperty<std::vector<CDateTime> > const* >( pProp )->Get().size(); break;
		case PropertyType::TimeSpanArray:	size = static_cast< CExplicitTypedProperty<std::vector<CTimeSpan> > const* >( pProp )->Get().size(); break;
		case PropertyType::ObjectPtrArray:	size = static_cast< CExplicitTypedProperty<std::vector<CObject*> > const* >( pProp )->Get().size(); break;
		default:
			throw std::invalid_argument( "property type" );
	}

	return static_cast<int32>( size );
}

void CUserPropertyCollection::SetArrayElementCount( string const& name, int32 Count )
{
	CProperty* pProp = GetItem( name );

	switch (GetType( name ))
	{
		case PropertyType::Int8Array:		static_cast< CExplicitTypedProperty<std::vector<int8> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::Int16Array:		static_cast< CExplicitTypedProperty<std::vector<int16> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::Int32Array:		static_cast< CExplicitTypedProperty<std::vector<int32> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::Int64Array:		static_cast< CExplicitTypedProperty<std::vector<int64> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::UInt8Array:		static_cast< CExplicitTypedProperty<std::vector<uint8> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::UInt16Array:		static_cast< CExplicitTypedProperty<std::vector<uint16> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::UInt32Array:		static_cast< CExplicitTypedProperty<std::vector<uint32> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::UInt64Array:		static_cast< CExplicitTypedProperty<std::vector<uint64> >* >( pProp )->Get().resize( Count, 0 ); break;
		case PropertyType::Float32Array:	static_cast< CExplicitTypedProperty<std::vector<float32> >* >( pProp )->Get().resize( Count, 0.0f ); break;
		case PropertyType::Float64Array:	static_cast< CExplicitTypedProperty<std::vector<float64> >* >( pProp )->Get().resize( Count, 0.0 ); break;
		case PropertyType::BoolArray:		static_cast< CExplicitTypedProperty<std::vector<bool> >* >( pProp )->Get().resize( Count, false ); break;
		case PropertyType::StringArray:		static_cast< CExplicitTypedProperty<std::vector<string> >* >( pProp )->Get().resize( Count ); break;
		case PropertyType::WStringArray:	static_cast< CExplicitTypedProperty<std::vector<wstring> >* >( pProp )->Get().resize( Count ); break;
		case PropertyType::DateTimeArray:	static_cast< CExplicitTypedProperty<std::vector<CDateTime> >* >( pProp )->Get().resize( Count ); break;
		case PropertyType::TimeSpanArray:	static_cast< CExplicitTypedProperty<std::vector<CTimeSpan> >* >( pProp )->Get().resize( Count ); break;
		case PropertyType::ObjectPtrArray:	static_cast< CExplicitTypedProperty<std::vector<CObject*> >* >( pProp )->Get().resize( Count ); break;
		default:
			throw std::invalid_argument( "property type" );
	}
}

}
}
