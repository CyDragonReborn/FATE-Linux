#pragma once

#include <string>
#include <vector>
#include <typeinfo>
#include "WT/System/Platform.h"

namespace WT
{

namespace System
{

class CObject;
class IInterface;
class CPropertyCollection;
class CCaster;
template<typename T> class SmartPointer;
typedef SmartPointer<CObject> HObject;

class CType
{
public:
	CType ( const string& FullName , const CType* BaseType , std::auto_ptr<CCaster>& BaseCaster, bool IsInterface, HObject (WTCall *CreateFunction)() );
	CType ( const string& FullName , const CType* BaseType , std::auto_ptr<CCaster>& BaseCaster );
	virtual ~CType();
	virtual void AddInterface(const CType* InheritedInterface, std::auto_ptr<CCaster const>& interfaceCaster);
	virtual const string& GetName() const;
	virtual const string& GetFullName() const;
	virtual const CType* GetBaseType() const;
	virtual bool IsSubclassOf ( const CType& BaseClass ) const;
	virtual bool IsInterface( ) const;
	bool operator== ( const CType& OtherType ) const;
	bool operator!= ( const CType& OtherType ) const;
	HObject CreateInstance() const;
	virtual string ToString() const;


	// These functions are used by the Type definition macros to setup property mappings.
	virtual void SetDefinePropertiesFunction( void (WTCall *DefinePropertiesFunction)( CPropertyCollection* ) );
	// if SetDefinePropertiesFunction() hasn't been called when this is called then it will never happen,
	// this method causes the type to build a ProperiesCollection based on properties defined for this class
	// and for base classes.
	// TODO: what about properties defined by Interfaces? is this allowed?
	virtual void FinalizeProperties(); 
	virtual void DefineProperties ( CPropertyCollection* Properties ) const;

	// these methods are used to inquire about a Types defined properties
	virtual CPropertyCollection* GetProperties() const;
	virtual bool HasProperties() const;

	virtual uint32 GetInterfaceCount() const;
	virtual const CType* GetInterface( uint32 Index ) const;

	// Get a list of all supported types - the types that an object of this type
	// will always be dynamic-castable to. This includes the current type, all base
	// class types, and supported interface types.
	void GetSupportedTypes( std::vector<CType const*>& Types ) const;

	//These functions operate on the set of all type objects, rather than a particular type object.
	static void WTCall AddType ( const string& Name , CType* pType );
	static void WTCall DeleteAllTypes();
	static CType* WTCall GetType ( const string& TypeName , bool CaseSensitive = true );
	static HObject WTCall CreateInstance ( const string& TypeName , bool CaseSensitive = true );

	template <class castToType>
	static castToType* WTCall _DynamicCast(CObject const * castFromObj)
	{
		if (castFromObj != NULL)
		{
			const CType* fromType = castFromObj->GetType();
			const CType* toType = castToType::GetClassType();
			if (fromType != NULL && toType != NULL)
			{
				return reinterpret_cast<castToType*>(fromType->CastToType(castFromObj, toType));
			}
		}
		return NULL;
	}

protected:
	virtual void* CastToLowest(CObject const * obj) const;
	virtual void* CastToType(CObject const * obj, const CType* toType) const;

private:
	void InitName(const string& FullName);
	void* CastUpBaseToTargetType(void* lowerObj, const CType* targetBaseType) const;
	static void* CastUpInterfaceToTargetType(void* lowerObj, 
										const CType* NextInterfaceType, 
										const CCaster* NextInterfaceCaster, 
										const CType* targetInterfaceType);
	static void* CastUpToInterface(void* thisObj, const CType* thisType, const CType* toType);

private:

	struct InterfaceInfo
	{
		InterfaceInfo(const CType* interfaceType, CCaster const* interfaceCaster) :
			m_InterfaceType(interfaceType),
			m_InterfaceCaster(interfaceCaster)
			{
			}

		CType const *	m_InterfaceType;
		CCaster const*	m_InterfaceCaster;
	};

	CPropertyCollection* m_Properties;
	const CType* m_BaseType;
	std::auto_ptr<CCaster> m_BaseCaster;

	string m_Name;
	string m_FullName;
	HObject (WTCall *m_CreateFunction)();
	void (WTCall *m_DefinePropertiesFunction)( CPropertyCollection* );

	bool m_IsInterfaceType;
	vector<InterfaceInfo> m_InheritedInterfaces;
};

// This template class inherits from Type, but overrides the CastToLowest() method
// so that it correctly casts an object of the template type to that type.
//
// An example is in order:
//   class MyClass : public Object;
//
//   MyObject* myObj = new MyObject();
//   Object* obj = myObj;
//   MyObject* myObj2 = reinterpret_cast<MyObject*>(obj->GetType()->CastToLowest());
//
// CastToLowest() ensures that even if casting changes the object pointer location (as can
// happen in some circumstances... especially if virtual inheritance is ever used) to the
// lowest class in the inheritance chain.
//
// CastToLowest() is used to implement the templatized DynamicCast<>() methods in Object
// and IInterface.
//
//
template <class LowestType>
class CTypeTemplate : public CType
{
public:
	CTypeTemplate ( const std::string& FullName , const CType* BaseType , std::auto_ptr<CCaster>& BaseCaster, 
		bool IsInterface = false, HObject (WTCall *CreateFunction)() = NULL )
		: CType(FullName, BaseType, BaseCaster, IsInterface, CreateFunction)
	{
		// Interfaces should never be constructed as TemplateType... just use class Type 
		// because the CastToLowest() method is never called for Interface type objects.
		assert(!IsInterface);
	}

	virtual ~CTypeTemplate ( )
	{
	}

protected:
	virtual void* CastToLowest(CObject const * obj) const
	{
		assert(obj->GetType() == this);
		return const_cast<LowestType *>( static_cast<LowestType const *>(obj) );
	}
};





// This is a helper class. It is used by the WT_*CLASS macros and by 
// the Type class. No one else should have to know about this class.
//
// Caster is an interface for performing casting operations between two types
// in a typeless manner (void*). 
//
// Here is what that means:
//
// CasterTemplate<HigherClass, LowerClass> provide conrete implementations of 
// the Caster Class.
//
// Caster is used byt DynamicCast to perform up cast, down cast, and cross cast 
// operations. Without something like Caster we would loose the information needed
// to perform this casts at comile time. Caster provides a way to capture the compiler's
// casting/type information for use at runtime.
//
// For exmaple:
// 
// Class A {...}
// Class B : public A {...}
//
// Declare a caster that allows casting between these type by doing this:
// CasterTemplate<A,B> abCaster;
//
// A is the "higher" class and b is the "lower" class (with respect to the 
// inheritance heirarchy. abCaster will allow you to perform the following 
// operations (note that the pointers get case to void* early on):
//
// void* pUnknown1 = new A();
// void* pUnknown2 = new B();
//
// A* pA = reinterpret_cast<A*>(abCaster.CastFromLowerClassUp(pUnknown2));
// B* pB = reinterpret_cast<B*>(abCaster.CastFromHigherClassDown(pUnknown1));
//
// The above code, at first, may not seem very useful, but if you consider that
// static_cast<>() (which will perform pointer adjustment) is not something that
// can be targeted to new types at runtime, and reinterpret_cast() is the only option
// (but it will not change the pointer value) something like Caster is needed in order
// to cast between two types which are not initially known at compile time (the types
// are know, but the need and location of the cast are unknown)
//
class CCaster
{
public:
	virtual void* CastFromLowerClassUp(void* lowerObj) const = 0;
	virtual void* CastFromHigherClassDown(void* higherObj) const = 0;
};



template <class HigherClass, class LowerClass>
class CCasterTemplate : public CCaster
{
public:
	CCasterTemplate()
	{
	}

	virtual void* CastFromLowerClassUp(void* lowerObj) const
	{
		LowerClass* lower = reinterpret_cast<LowerClass*>(lowerObj);
		HigherClass* higher = static_cast<HigherClass*>(lower);
		return higher;
	}

	virtual void* CastFromHigherClassDown(void* higherObj) const
	{
		HigherClass* higher = reinterpret_cast<HigherClass*>(higherObj);
		LowerClass* lower = static_cast<LowerClass*>(higher);
		return lower;
	}
};


inline char const* WTCall NormalizeClassName( char const pFullClassName[] )
{
	assert( pFullClassName != NULL );

	if (pFullClassName[0] == 'c')
	{
		assert( !strncmp( pFullClassName, "class ", 6 ) );
		return pFullClassName + 6;
	} else {
		assert( !strncmp( pFullClassName, "struct ", 7 ) );
		return pFullClassName + 7;
	}
}

} // System namespace 


/*
DynamicCast is the WTPlatform version of the regular c++ dynamic_cast<>.
For various reasons, our home-grown run-time type information system
is not compatible with dynamic_cast<> and therefore you must use
DynamicCast() instead.

Example:
<pre><tt>
			CObject* pObj = NULL;
			SerialInfo->GetValue<CObject*>("item", pObj );
			ValueType* pItemType = DynamicCast<ValueType>(pObj);
</tt></pre>
*/
template <class castToType, class castFromType>
castToType* WTCall DynamicCast(castFromType const * castFromObj)
{
	if (castFromObj == NULL) 
	{
		return NULL;
	}
	return System::CType::_DynamicCast<castToType>(castFromObj->GetObjectPtr());
}


} // WT namespace

#define WT_BEGIN_ABSTRACT_CLASS(ClassName, BaseClass) \
	typedef ClassName _ThisClass; \
	typedef BaseClass _BaseClass; \
	virtual const WT::System::CType* GetType() const \
	{ \
		return GetClassType(); \
	} \
	static void WTCall Using() { _ThisClass::GetClassType(); } \
		virtual WT::System::CObject const * GetObjectPtr() const \
	{ \
		return this; \
	} \
	virtual WT::System::CObject* GetObjectPtr() \
	{ \
		return this; \
	} \
	static const WT::System::CType* WTCall GetClassType() \
	{ \
		static WT::System::CType* g_pMyType = NULL; \
		if (g_pMyType != NULL) \
		{ \
			return g_pMyType; \
		} \
		std::string FullClassName = WT::System::NormalizeClassName(typeid( _ThisClass ).name()); \
		g_pMyType = WT::System::CType::GetType(FullClassName); \
		if ( g_pMyType == NULL ) \
		{ \
		    /* This is an abstract class so there is no need to create a TemplateType object because this class will never be the lowest in the inheritance chance so CastToLowest() will never be called on this type. */ \
			std::auto_ptr<WT::System::CCaster> BaseCaster( new WT::System::CCasterTemplate<BaseClass,_ThisClass>() ); \
			g_pMyType = new WT::System::CType( FullClassName , BaseClass::GetClassType() , BaseCaster, false, NULL );

// TODO: this is only needed to place type information in the static type map
// for the purposes of deserialization. We probably want to rename this macro
// to indicate what it really is used for.
#define WT_DEFINE_CLASS(ClassName) \
	namespace { \
		template <typename T> struct InitClassType; \
		template <> struct InitClassType<ClassName > { \
			static WT::System::CType const* sClassType; \
		}; \
		WT::System::CType const* InitClassType<ClassName >::sClassType = ClassName::GetClassType(); \
	}
	
// Macros for Interfaces

//A macro used to define other macros - not intended to be called by the user
#define WT_BEGIN_CLASS_BASE_IMPLEMENTATION(ClassName, BaseClass) \
	typedef ClassName _ThisClass; \
	typedef BaseClass _BaseClass; \
	virtual const WT::System::CType* GetType() const \
	{ \
		return GetClassType(); \
	} \
	virtual WT::System::CObject const * GetObjectPtr() const \
	{ \
		return this; \
	} \
	virtual WT::System::CObject * GetObjectPtr() \
	{ \
		return this; \
	} \
	static void WTCall Using() { _ThisClass::GetClassType(); } \
	static const WT::System::CType* WTCall GetClassType() \
	{ \
		static WT::System::CType* g_pMyType = NULL; \
		if (g_pMyType != NULL) \
		{ \
			return g_pMyType; \
		} \
		std::string FullClassName = WT::System::NormalizeClassName(typeid( _ThisClass ).name()); \
		g_pMyType = WT::System::CType::GetType(FullClassName); \
		if ( g_pMyType == NULL ) \
		{ \
			std::auto_ptr<WT::System::CCaster> BaseCaster( new WT::System::CCasterTemplate<BaseClass,_ThisClass>() ); \
			g_pMyType = new WT::System::CTypeTemplate<_ThisClass>( FullClassName , BaseClass::GetClassType(), BaseCaster , false, _ThisClass::GetCreator() );

#define WT_BEGIN_CLASS(ClassName, BaseClass) \
	static WT::System::HObject WTCall Create() \
	{ \
		return new ClassName(); \
	} \
	static WT::System::HObject (WTCall * WTCall GetCreator()) () \
	{ \
		return &Create; \
	} \
	WT_BEGIN_CLASS_BASE_IMPLEMENTATION(ClassName, BaseClass )
	
//Define classes which have no creator - usually due to lack of a default constructor
#define WT_BEGIN_CLASS_NO_CREATOR(ClassName, BaseClass) \
	static WT::System::HObject (WTCall * WTCall GetCreator()) () \
	{ \
		return NULL; \
	} \
	WT_BEGIN_CLASS_BASE_IMPLEMENTATION(ClassName, BaseClass )

#define WT_END_CLASS() \
			g_pMyType->FinalizeProperties(); \
		} \
		return g_pMyType; \
	}

#define WT_BEGIN_INTERFACE( ClassName ) \
	typedef ClassName _ThisClass; \
	static void WTCall Using() { _ThisClass::GetClassType(); } \
	static const WT::System::CType* WTCall GetClassType() \
	{ \
		static WT::System::CType* g_pMyType = NULL; \
		if (g_pMyType != NULL) \
		{ \
			return g_pMyType; \
		} \
		std::string FullClassName = WT::System::NormalizeClassName(typeid( _ThisClass ).name()); \
		g_pMyType = WT::System::CType::GetType(FullClassName); \
		if ( g_pMyType == NULL ) \
		{ \
		    /* Interfaces do not need to use the TemplateType because Interfaces will never be the lowest class so CastToLowest never needs to work */ \
			std::auto_ptr<WT::System::CCaster> BaseCaster; \
			g_pMyType = new WT::System::CType( FullClassName, NULL, BaseCaster, true, NULL );


//Use WT_INHERITS_INTERFACE to indicate that your class inherits the specified interface.
//Derived classes should not repeat the WT_INHERITS_INTERFACE macro for this particular interface.
#define WT_INHERITS_INTERFACE( InterfaceClass ) \
			{ \
				std::auto_ptr<WT::System::CCaster const> pInterfaceCaster( new WT::System::CCasterTemplate<InterfaceClass, _ThisClass>() ); \
				g_pMyType->AddInterface( InterfaceClass::GetClassType(), pInterfaceCaster ); \
			} \

#define WT_END_INTERFACE() \
		} \
		return g_pMyType; \
	}

#define WT_CLASS(ClassName, BaseClass) \
	WT_BEGIN_CLASS(ClassName, BaseClass ) \
	WT_END_CLASS()

//Define classes which have no creator - usually due to lack of a default constructor
#define WT_CLASS_NO_CREATOR(ClassName, BaseClass) \
	WT_BEGIN_CLASS_NO_CREATOR(ClassName, BaseClass ) \
	WT_END_CLASS()

#define WT_ABSTRACT_CLASS(ClassName, BaseClass) \
	WT_BEGIN_ABSTRACT_CLASS(ClassName, BaseClass) \
	WT_END_CLASS()

#define WT_INTERFACE(ClassName) \
	WT_BEGIN_INTERFACE(ClassName) \
		WT_INHERITS_INTERFACE( WT::System::IInterface ) \
	WT_END_CLASS()
