#include "WT/stdafx.h"

#include "WT/System/Object.h"
#include "WT/System/Type.h"
#include "WT/System/Property.h"
#include "WT/System/SmartPointer.h"

namespace WT
{
namespace System
{

using namespace std;

typedef std::map<std::string, SmartPointer<CType> > TypeMap;

TypeMap& GetTypeMap()
{
	static std::auto_ptr<TypeMap> g_pTypeMap;

	if (g_pTypeMap.get() == NULL)
	{
		g_pTypeMap.reset( new TypeMap() );
	}

	return *g_pTypeMap.get();
}

CType::CType ( const string& FullName , const CType* BaseType , std::auto_ptr<CCaster>& BaseCaster, 
			bool IsInterface, HObject (WTCall *CreateFunction)() ) :
	m_FullName ( FullName ) , m_BaseType ( BaseType ) , m_BaseCaster(BaseCaster) , m_CreateFunction ( CreateFunction ) , m_DefinePropertiesFunction ( NULL ), 
	m_IsInterfaceType(IsInterface), m_Properties (NULL)
{
	// if this is an interface then there should be no CreateFunction
	assert( (m_IsInterfaceType && m_CreateFunction == NULL) || !m_IsInterfaceType);

	// if there is a base class then there also better be a base caster
	assert( (m_BaseType && (m_BaseCaster.get() != NULL)) || !m_BaseType );

	InitName(m_FullName);
	AddType( m_FullName, this );
}

CType::CType ( const string& FullName , const CType* BaseType , std::auto_ptr<CCaster>& BaseCaster) :
	m_FullName ( FullName ) , m_BaseType ( BaseType ) ,  m_BaseCaster(BaseCaster) ,
	m_CreateFunction ( NULL ) , m_DefinePropertiesFunction ( NULL ), 
	m_IsInterfaceType( false ), m_Properties (NULL)
{
	// if this is an interface then there should be no CreateFunction
	assert( (m_IsInterfaceType && m_CreateFunction == NULL) || !m_IsInterfaceType);

	// if there is a base class then there also better be a base caster
	assert( (m_BaseType && (m_BaseCaster.get() != NULL)) || !m_BaseType );

	InitName(m_FullName);
	AddType( m_FullName, this );
}

CType::~CType()
{
	int32 const InheritedInterfaceCount = static_cast<int32>( m_InheritedInterfaces.size() );
	for (int32 i = 0; i < InheritedInterfaceCount; ++i)
	{
		delete m_InheritedInterfaces[i].m_InterfaceCaster;
	}

	delete m_Properties;
}



void CType::InitName(const string& FullName)
{
	m_FullName = FullName;

	string::size_type LastDotIndex = m_FullName.find_last_of("::");
	if ( LastDotIndex == string::npos )
		m_Name = m_FullName;
	else
		m_Name = m_FullName.substr ( LastDotIndex+1 );
}


void CType::AddInterface(const CType* InheritedInterface, std::auto_ptr<CCaster const>& pInterfaceCaster)
{
	m_InheritedInterfaces.push_back( InterfaceInfo(InheritedInterface, NULL ) );
	m_InheritedInterfaces.rbegin()->m_InterfaceCaster = pInterfaceCaster.release();
}

const string& CType::GetName() const
{
	return m_Name;
}
const string& CType::GetFullName() const
{
	return m_FullName;
}

const CType* CType::GetBaseType() const
{
	return m_BaseType;
}

bool CType::IsSubclassOf ( const CType& BaseClass ) const
{

	// This is a special case. If BaseClass is the same as
	// the Class itself then we still want to return true.
	//
	// Here's the scenario:
	//----------------------------------------------------------
	// class MyObject : public Object;
	//
	// void foo(Object* obj)
	// {
	//    bool isMyObject = obj->GetType()->IsSubClassOf( &MyObject::GetClassType() );
    //    // ... do something special if this is a MyObject
	// }
	//
	// MyObect* myObj = new MyObject();
	// Object* obj = myObj;
	// foo(obj);
	//----------------------------------------------------------
	//
	// In the above code if this special case is not tested for then
	// foo() will not be able to recognize that Object is a MyObject.
	//
	// TODO: technically MyObject would not a "SUBclass" of MyObject it "IS"
	// a MyObject. For this reason should we rename this method to something
	// other than IsSubClassOf()?

	if (this == &BaseClass)
	{
		return true;
	}

	// see if this is an interface
	for (vector<InterfaceInfo>::const_iterator citer = m_InheritedInterfaces.begin();
		citer != m_InheritedInterfaces.end(); citer++)
	{
		const CType* interfaceType = citer->m_InterfaceType;
		if ( interfaceType == &BaseClass || interfaceType->IsSubclassOf(BaseClass))
		{
			return true;
		}
	}

	const CType* ThisBaseType = m_BaseType;
	while ( ThisBaseType )
	{
		if ( ThisBaseType == &BaseClass )
			return true;

		// traverse up the interfaces at this level
		for (vector<InterfaceInfo>::const_iterator citer = ThisBaseType->m_InheritedInterfaces.begin();
			citer != ThisBaseType->m_InheritedInterfaces.end(); citer++)
		{
			const CType* interfaceType = citer->m_InterfaceType;
			if ( interfaceType == &BaseClass || interfaceType->IsSubclassOf(BaseClass))
			{
				return true;
			}
		}

		ThisBaseType = ThisBaseType->m_BaseType;
	}

	return false;
}

bool CType::IsInterface( ) const
{
	return m_IsInterfaceType;
}

bool CType::operator== ( const CType& OtherType ) const
{
	return this == &OtherType;
}

bool CType::operator!=( const CType& OtherType ) const
{
	return this != &OtherType;
}

CType* CType::GetType ( const string& TypeName , bool CaseSensitive )
{
	TypeMap::const_iterator i = GetTypeMap().find ( TypeName );
	if ( i == GetTypeMap().end() )
	{
		return NULL;
	}
	return i->second.get();
}

HObject CType::CreateInstance () const
{
	if ( m_CreateFunction )
		return m_CreateFunction();
	else
		return NULL;
}

HObject CType::CreateInstance ( const string& TypeName , bool CaseSensitive )
{
	CType* ObjectType = GetType ( TypeName , CaseSensitive );
	if ( ObjectType )
		return ObjectType->m_CreateFunction();
	else
		return NULL;
}

void CType::SetDefinePropertiesFunction( void (WTCall *DefinePropertiesFunction)( CPropertyCollection* ) )
{
	m_DefinePropertiesFunction = DefinePropertiesFunction;
}

void CType::FinalizeProperties()
{
	//Apply properties from interfaces
	uint32 const InterfaceCount = GetInterfaceCount();

	for (uint32 i = 0; i < InterfaceCount; ++i)
	{
		CType const* pInterfaceType = GetInterface( i );
		if (pInterfaceType && pInterfaceType->m_DefinePropertiesFunction != NULL)
		{
			if (m_Properties == NULL)
			{
				m_Properties = new CPropertyCollection();
			}

			pInterfaceType->DefineProperties( m_Properties );
		}
	}

	//Apply properties from this class and all of its base classes
	CType const* pCurrType = this;
	do 
	{
		if (pCurrType->m_DefinePropertiesFunction != NULL)
		{
			if (m_Properties == NULL)
			{
				m_Properties = new CPropertyCollection();
			}

			pCurrType->DefineProperties( m_Properties );
		}
	} while ((pCurrType = pCurrType->m_BaseType) != NULL);
}

CPropertyCollection* CType::GetProperties() const
{
	return m_Properties;
}

bool CType::HasProperties() const
{
	return m_Properties != NULL ? true : false;
}

void CType::DefineProperties (  CPropertyCollection* Properties ) const
{
	if ( m_DefinePropertiesFunction )
	{
		m_DefinePropertiesFunction ( Properties );
	}
}

string CType::ToString() const
{
	string str = "<type name=\"" + m_Name 
		+ "\" interface=\"" + string(m_IsInterfaceType ? "true" : "false") 
		+ "\" abstract=\"" + string( (m_CreateFunction == NULL) ? "true" : "false")
		+ "\">";
	str += "<fullname>" + m_FullName + "</fullname>";
	if (m_BaseType)
	{
		str += "<basetype>" + m_BaseType->ToString() + "</basetype>";
	}
	else
	{
		str += "<basetype />";
	}

	if (m_InheritedInterfaces.size() == 0)
	{
		str += "<interfaces />";
	}
	else
	{
		str += "<interfaces>";
		for (vector<InterfaceInfo>::const_iterator iter = m_InheritedInterfaces.begin(); 
			iter != m_InheritedInterfaces.end(); 
			iter++)
		{
			const CType* interfaceType = iter->m_InterfaceType;
			str += interfaceType->ToString();
		}
		str += "</interfaces>";
	}

	// TODO: Write Properties to string?

	str += "</type>";

	return str;
}

void* CType::CastToLowest(CObject const * obj) const
{
	// Interfaces are never the lowest
	assert(!m_IsInterfaceType);
	return NULL;
}

void* CType::CastUpBaseToTargetType(void* lowerObj, const CType* targetBaseType) const
{
	if (m_BaseType == NULL) 
	{
		return NULL;
	}

	// since there is a base class, we assume there is a base caster
	assert( this->m_BaseCaster.get()!= NULL );

	// cast to the base type
    void* castToBasePtr = this->m_BaseCaster->CastFromLowerClassUp(lowerObj);

	// if the immediate base class is our target cast then stop and return 
	// our current cast pointer
	if (m_BaseType == targetBaseType)
	{
		return castToBasePtr;
	}
	else
	{
		// this isn't our final cast target so continue up the base heirarchy
		return m_BaseType->CastUpBaseToTargetType(castToBasePtr, targetBaseType);
	}
}

void* CType::CastUpInterfaceToTargetType(void* lowerObj, 
										const CType* NextInterfaceType, 
										const CCaster* NextInterfaceCaster, 
										const CType* targetInterfaceType)
{
	// assume we don't have to follow base classes when we are looking for interfaces
	assert(NextInterfaceType->m_BaseType == NULL);

    void* castToInterfacePtr = NextInterfaceCaster->CastFromLowerClassUp(lowerObj);
	if (NextInterfaceType == targetInterfaceType)
	{
		return castToInterfacePtr;
	}
	else
	{
		for (vector<InterfaceInfo>::const_iterator iter = NextInterfaceType->m_InheritedInterfaces.begin(); 
				iter != NextInterfaceType->m_InheritedInterfaces.end(); iter++)
		{
			void* interfaceCast = NextInterfaceType->CastUpInterfaceToTargetType(castToInterfacePtr, iter->m_InterfaceType, iter->m_InterfaceCaster, targetInterfaceType);
			if (interfaceCast != NULL)
			{
				return interfaceCast;
			}
		}
		return NULL;
	}
}

void* CType::CastUpToInterface(void* thisObj, const CType* thisType, const CType* toType)
{
	for (vector<InterfaceInfo>::const_iterator iter = thisType->m_InheritedInterfaces.begin(); 
			iter != thisType->m_InheritedInterfaces.end(); iter++)
	{
		void* interfaceCast = CastUpInterfaceToTargetType(thisObj, iter->m_InterfaceType, iter->m_InterfaceCaster, toType);
		if (interfaceCast != NULL)
		{
			return interfaceCast;
		}
	}

	// couldn't find the target base interface by folling interfaces that this type directly
	// inherits from. It may derive from this interface via a base class, look there.
	if (thisType->m_BaseType != NULL) 
	{
		void* castToBasePtr = thisType->m_BaseCaster->CastFromLowerClassUp(thisObj);
		return CastUpToInterface(castToBasePtr, thisType->m_BaseType, toType);
	}

	return NULL;
}


void* CType::CastToType(CObject const * obj, CType const * toType) const
{
	assert(obj->GetType() == this);
	void* lowestObj = CastToLowest(obj);

	if (!toType->IsInterface())
	{
		// are we trying to cast to the bottom? If so then we are done.
		if (this == toType)
		{
			return lowestObj;
		}

		return CastUpBaseToTargetType(lowestObj, toType);
	}
	else
	{
		return CastUpToInterface(lowestObj, this, toType);		
	}
}


void CType::AddType( const string& Name, CType* pType )
{
	std::pair< std::string, SmartPointer<CType> > Element( 
		Name, SmartPointer<CType>( pType ) );
	GetTypeMap().insert( Element );

}

void CType::DeleteAllTypes()
{
	GetTypeMap().clear();
}

uint32 CType::GetInterfaceCount() const
{
	//VARIABLES
	vector< const CType* > Interfaces;
	const CType *Type = this;

	//TRAVERSE DOWN THE OBJECT TREE POPULATING THE INTERFACE LIST WITH ALL ROOT LEVEL INTERFACES
	while( Type != NULL )
	{
		for( vector< InterfaceInfo >::const_iterator I = Type->m_InheritedInterfaces.begin(); I != Type->m_InheritedInterfaces.end(); I++ )
		{
			if( I->m_InterfaceType != NULL )
			{
				bool FoundInterface = false;

				for( vector< const CType* >::iterator J = Interfaces.begin(); J != Interfaces.end(); J++ )
				{
					if( *J == I->m_InterfaceType )
					{
						FoundInterface = true;
						break;
					}
				}

				if( FoundInterface == false )
				{
					Interfaces.push_back( I->m_InterfaceType );
				}
			}
		}

		Type = Type->GetBaseType();
	}

	//RECURSIVELY TRAVERSE THE INTERFACE LIST POPULATING IT WITH ALL NON ROOT LEVEL INTERFACES
	for( uint32 I = 0; I < Interfaces.size(); I++ )
	{
		for( vector< InterfaceInfo >::const_iterator J = Interfaces[ I ]->m_InheritedInterfaces.begin(); J != Interfaces[ I ]->m_InheritedInterfaces.end(); J++ )
		{
			bool FoundInterface = false;

			for( vector< const CType* >::iterator K = Interfaces.begin(); K != Interfaces.end(); K++ )
			{
				if( J->m_InterfaceType == *K )
				{
					FoundInterface = true;
					break;
				}
			}

			if( FoundInterface == false )
			{
				Interfaces.push_back( J->m_InterfaceType );
			}
		}
	}

	//RETURN THE TOTAL NUMBER OF INTERFACES
	return (uint32)( Interfaces.size() );
}

const CType* CType::GetInterface( uint32 Index ) const
{
	//VARIABLES
	vector< const CType* > Interfaces;
	const CType *Type = this;

	//TRAVERSE DOWN THE OBJECT TREE POPULATING THE INTERFACE LIST WITH ALL ROOT LEVEL INTERFACES
	while( Type != NULL )
	{
		for( vector< InterfaceInfo >::const_iterator I = Type->m_InheritedInterfaces.begin(); I != Type->m_InheritedInterfaces.end(); I++ )
		{
			if( I->m_InterfaceType != NULL )
			{
				bool FoundInterface = false;

				for( vector< const CType* >::iterator J = Interfaces.begin(); J != Interfaces.end(); J++ )
				{
					if( *J == I->m_InterfaceType )
					{
						FoundInterface = true;
						break;
					}
				}

				if( FoundInterface == false )
				{
					Interfaces.push_back( I->m_InterfaceType );

					if( Index < Interfaces.size() )
					{
						return Interfaces[ Index ];
					}
				}
			}
		}

		Type = Type->GetBaseType();
	}

	//RECURSIVELY TRAVERSE THE INTERFACE LIST POPULATING IT WITH ALL NON ROOT LEVEL INTERFACES
	for( uint32 I = 0; I < Interfaces.size(); I++ )
	{
		for( vector< InterfaceInfo >::const_iterator J = Interfaces[ I ]->m_InheritedInterfaces.begin(); J != Interfaces[ I ]->m_InheritedInterfaces.end(); J++ )
		{
			bool FoundInterface = false;

			for( vector< const CType* >::iterator K = Interfaces.begin(); K != Interfaces.end(); K++ )
			{
				if( J->m_InterfaceType == *K )
				{
					FoundInterface = true;
					break;
				}
			}

			if( FoundInterface == false )
			{
				Interfaces.push_back( J->m_InterfaceType );

				if( Index < Interfaces.size() )
				{
					return Interfaces[ Index ];
				}
			}
		}
	}

	//IF THE FUNCTION HAS NOT EXITED THUS FAR, THEN THE INDEX WAS OUT OF RANGE
	throw CException( "CType::GetInterface(), Non-existent Index" );
}

void CType::GetSupportedTypes( std::vector<CType const*>& Types ) const
{
	for (CType const *pType = this; pType != NULL; pType = pType->GetBaseType())
	{
		Types.push_back( pType );
	}

	for( uint32 I = 0; I < GetInterfaceCount(); I++ )
	{
		Types.push_back( GetInterface( I ) );
	}
}

}
}