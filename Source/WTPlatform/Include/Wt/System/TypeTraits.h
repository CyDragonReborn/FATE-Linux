#pragma once

#include "WT/System/Platform.h"
#include "WT/System/DateTime.h"
#include "WT/System/SmartPointer.h"
#include "WT/System/Object.h"

#include <vector>
#include <list>
#include <map>

namespace WT
{

//CIsObject::Result is TrueType if a pointer to an object of type T can be cast to
//a CObject*. Otherwise, Result is FalseType.
template <typename T> struct CIsObject : CCanCastPointer<T, System::CObject>
{
};

// default implementation which assumes it is working 
// with something derived from Object. If T is not derived from
// CObject and you ended up here, then create a specialization of
// this template to deal with your specific type.
template <typename T>
struct TypeTraits
{
	typedef T ThisType;
	typedef NoType InnerType;
	typedef System::CObject TopType;
	typedef typename CIsObject<T>::Result IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef FalseType IsConst;
	
	static string ToString()
	{
		return ThisType::GetClassType()->GetFullName();
	}

	static wstring ToWString()
	{
		return AnsiToWide(ThisType::GetClassType()->GetFullName());
	}

	static WT::System::CType const* GetType() {return T::GetClassType();}
};

//Declare type traits explicitly for a non-CObject type, such as WT::int
//or std::vector<bool>. The argument to this macro should ideally include all
//namespace qualifiers as it will be stringized and used as the name for 
//the type.
#define DECLARE_VALUETYPE_TYPETRAIT(type) \
/* type */ \
template <> struct WT::TypeTraits<type > \
{ \
	typedef type ThisType; \
	typedef WT::NoType InnerType; \
	typedef ThisType TopType; \
	typedef WT::FalseType IsObject; \
	typedef WT::FalseType IsPtr; \
	typedef WT::FalseType IsRef; \
	typedef WT::FalseType IsConst; \
 \
	static std::string ToString() \
	{ \
		return #type; \
	} \
	static std::wstring ToWString() \
	{ \
		return AnsiToWide( ToString() ); \
	} \
 \
	static WT::System::CType const* GetType() \
	{ \
		static WT::System::CType* g_pType = NULL; \
		if (g_pType == NULL) \
		{ \
			std::auto_ptr<WT::System::CCaster> BaseCaster; \
			g_pType = new WT::System::CType( #type, NULL, BaseCaster ); \
		} \
		return g_pType; \
	} \
}; \
/* const type */ \
template <> struct WT::TypeTraits<type const> \
{ \
	typedef type const ThisType; \
	typedef WT::NoType InnerType; \
	typedef ThisType TopType; \
	typedef WT::FalseType IsObject; \
	typedef WT::FalseType IsPtr; \
	typedef WT::FalseType IsRef; \
	typedef WT::TrueType IsConst; \
 \
	static std::string ToString() \
	{ \
	return WT::TypeTraits<type >::ToString() + " const"; \
	} \
\
	static std::wstring ToWString() \
	{ \
		return AnsiToWide( ToString() ); \
	} \
\
	static WT::System::CType const* GetType() \
	{ \
		return WT::TypeTraits<type >::GetType(); \
	} \
};

DECLARE_VALUETYPE_TYPETRAIT(WT::int8);
DECLARE_VALUETYPE_TYPETRAIT(WT::int16);
DECLARE_VALUETYPE_TYPETRAIT(WT::int32);
DECLARE_VALUETYPE_TYPETRAIT(WT::int64);
DECLARE_VALUETYPE_TYPETRAIT(WT::uint8);
DECLARE_VALUETYPE_TYPETRAIT(WT::uint16);
DECLARE_VALUETYPE_TYPETRAIT(WT::uint32);
DECLARE_VALUETYPE_TYPETRAIT(WT::uint64);
DECLARE_VALUETYPE_TYPETRAIT(WT::float32);
DECLARE_VALUETYPE_TYPETRAIT(WT::float64);
DECLARE_VALUETYPE_TYPETRAIT(bool);
DECLARE_VALUETYPE_TYPETRAIT(std::string);
DECLARE_VALUETYPE_TYPETRAIT(std::wstring);
DECLARE_VALUETYPE_TYPETRAIT(WT::System::CDateTime);
DECLARE_VALUETYPE_TYPETRAIT(WT::System::CTimeSpan);
DECLARE_VALUETYPE_TYPETRAIT(WT::System::CObject*);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::int8>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::int16>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::int32>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::int64>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::uint8>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::uint16>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::uint32>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::uint64>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::float32>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::float64>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<bool>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<std::string>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<std::wstring>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::System::CDateTime>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::System::CTimeSpan>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::System::CObject*>);

// To aid in debugging
DECLARE_VALUETYPE_TYPETRAIT(WT::TrueType);
DECLARE_VALUETYPE_TYPETRAIT(WT::FalseType);
DECLARE_VALUETYPE_TYPETRAIT(WT::NoType);

// Object
template <>
struct TypeTraits<System::CObject>
{
	typedef System::CObject ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef TrueType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef FalseType IsConst;

	static string ToString()
	{
		return ThisType::GetClassType()->GetFullName();
	}

	static wstring ToWString()
	{
		return AnsiToWide(ThisType::GetClassType()->GetFullName());
	}

	static WT::System::CType const* GetType() {return WT::System::CObject::GetClassType();}
};

// const Object
template <>
struct TypeTraits<System::CObject const>
{
	typedef System::CObject const ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef TrueType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef TrueType IsConst;

	static string ToString()
	{
		return ThisType::GetClassType()->GetFullName() + " const ";
	}

	static wstring ToWString()
	{
		return AnsiToWide(ThisType::GetClassType()->GetFullName()) + L" const ";
	}

	static WT::System::CType const* GetType() {return WT::System::CObject::GetClassType();}
};



// vector
template <typename T>
struct TypeTraits< std::vector<T> >
{
	typedef std::vector<T> ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef FalseType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef FalseType IsConst;

	static string ToString()
	{
		return "vector<"+TypeTraits<T>::ToString()+">";
	}

	static wstring ToWString()
	{
		return L"vector<"+TypeTraits<T>::ToWString()+L">";
	}

};

// const vector
template <typename T>
struct TypeTraits<std::vector<T> const>
{
	typedef std::vector<T> const ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef FalseType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef TrueType IsConst;

	static string ToString()
	{
		return "vector<"+TypeTraits<T>::ToString()+"> const ";
	}

	static wstring ToWString()
	{
		return L"vector<"+TypeTraits<T>::ToWString()+L"> const ";
	}

};



// list
template <typename T>
struct TypeTraits< std::list<T> >
{
	typedef std::list<T> ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef FalseType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef FalseType IsConst;

	static string ToString()
	{
		return "list<"+TypeTraits<T>::ToString()+">";
	}

	static wstring ToWString()
	{
		return L"list<"+TypeTraits<T>::ToWString()+L">";
	}

};

// const list
template <typename T>
struct TypeTraits<std::list<T> const>
{
	typedef std::list<T> const ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef FalseType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef TrueType IsConst;

	static string ToString()
	{
		return "list<"+TypeTraits<T>::ToString()+"> const ";
	}

	static wstring ToWString()
	{
		return L"list<"+TypeTraits<T>::ToWString()+L"> const ";
	}

};


// map
template <typename K, typename T>
struct TypeTraits< std::map<K,T> >
{
	typedef std::map<K,T> ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef FalseType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef FalseType IsConst;

	static string ToString()
	{
		return "map<"+TypeTraits<K>::ToString()+", "+TypeTraits<T>::ToString()+">";
	}

	static wstring ToWString()
	{
		return L"map<"+TypeTraits<K>::ToWString()+L", "+TypeTraits<T>::ToWString()+L">";
	}

};

// const map
template <typename K, typename T>
struct TypeTraits< std::map<K,T> const>
{
	typedef std::map<K,T> const ThisType;
	typedef NoType InnerType;
	typedef ThisType TopType;
	typedef FalseType IsObject;
	typedef FalseType IsPtr;
	typedef FalseType IsRef;
	typedef TrueType IsConst;

	static string ToString()
	{
		return "map<"+TypeTraits<K>::ToString()+", "+TypeTraits<T>::ToString()+"> const ";
	}

	static wstring ToWString()
	{
		return L"map<"+TypeTraits<K>::ToWString()+L", "+TypeTraits<T>::ToWString()+L"> const ";
	}

};



// ptr
template <typename T>
struct TypeTraits<T*>
{
	typedef T* ThisType;
	typedef T InnerType;
	typedef typename TypeTraits<InnerType>::TopType* TopType;
	typedef typename TypeTraits<InnerType>::IsObject IsObject;
	typedef TrueType  IsPtr;
	typedef FalseType IsRef;
	typedef FalseType IsConst;

	static string ToString()
	{
		return TypeTraits<T>::ToString() + '*';
	}

	static wstring ToWString()
	{
		return TypeTraits<T>::ToWString() + L'*';
	}

};

// const ptr
template <typename T>
struct TypeTraits<T* const>
{
	typedef T* const ThisType;
	typedef T InnerType;
	typedef typename TypeTraits<InnerType>::TopType* const TopType;
	typedef typename TypeTraits<InnerType>::IsObject IsObject;
	typedef TrueType  IsPtr;
	typedef FalseType IsRef;
	typedef TrueType IsConst;

	static string ToString()
	{
		return TypeTraits<T>::ToString() + "* const ";
	}

	static wstring ToWString()
	{
		return TypeTraits<T>::ToWString() + L"* const ";
	}

};



// ref
template <typename T>
struct TypeTraits<T&>
{
	typedef T& ThisType;
	typedef T InnerType;
	typedef typename TypeTraits<InnerType>::TopType& TopType;
	typedef typename TypeTraits<InnerType>::IsObject IsObject;
	typedef FalseType IsPtr;
	typedef TrueType  IsRef;
	typedef FalseType IsConst;

	static string ToString()
	{
		return TypeTraits<T>::ToString() + "&";
	}

	static wstring ToWString()
	{
		return TypeTraits<T>::ToWString() + L"&";
	}

};

} // namespace WT




