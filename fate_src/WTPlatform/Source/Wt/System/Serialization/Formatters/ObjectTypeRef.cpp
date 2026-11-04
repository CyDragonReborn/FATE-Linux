#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/ObjectTypeRef.h"
#include "WT/System/Serialization/SerializationException.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CObjectTypeRef)

CObjectTypeRef::CObjectTypeRef(CObject const * object, CObjectIDGenerator::ObjectID objID) :
	m_Type(NULL),
	m_SurrogateType(NULL),
	m_ObjectID(objID),
	m_ObjectFormatVersion(0)
{
	Init(object, objID, NULL);
}

CObjectTypeRef::CObjectTypeRef(CObject const * object, CObjectIDGenerator::ObjectID objID, 
							 CSerializationSurrogate const * pSurrogate) :
	m_Type(NULL),
	m_SurrogateType(NULL),
	m_ObjectID(objID)
{
	Init(object, objID, pSurrogate);
}


CObjectTypeRef::CObjectTypeRef(string const & typeStr, CObjectIDGenerator::ObjectID objID, uint32 objectFormatVersion) :
	m_Type(NULL),
	m_SurrogateType(NULL),
	m_ObjectID(objID),
	m_ObjectFormatVersion(objectFormatVersion)
{
	Init(typeStr, objID, objectFormatVersion);
}

CObjectTypeRef::CObjectTypeRef(string const & typeStr, CObjectIDGenerator::ObjectID objID, 
							 string const & surrogateTypeStr, uint32 objectFormatVersion) :
	m_Type(NULL),
	m_SurrogateType(NULL),
	m_ObjectID(objID),
	m_ObjectFormatVersion(objectFormatVersion)
{
	Init(typeStr, objID, surrogateTypeStr, objectFormatVersion);
}

CObjectTypeRef::CObjectTypeRef(CObjectTypeRef const & right ) :
	m_Type(right.m_Type),
	m_SurrogateType(right.m_SurrogateType),
	m_ObjectID(right.m_ObjectID),
	m_ObjectFormatVersion(right.m_ObjectFormatVersion)
{
}

CObjectTypeRef::~CObjectTypeRef()
{
	m_Type = NULL;
	m_SurrogateType = NULL;
	m_ObjectID = 0;
	m_ObjectFormatVersion = 0;
}

// returns true if the current object type info is a NULL reference 
// (no type information and ObjectID of 0)
bool CObjectTypeRef::IsNull() const
{
	return m_Type == NULL;
}

// Returns the Type object associated with the type specified in this object reference
const CType* CObjectTypeRef::GetRefType() const
{
	return m_Type;
}

bool CObjectTypeRef::HasSurrogate() const
{
	return m_SurrogateType != NULL;
}

CType const * CObjectTypeRef::GetSurrogateType() const
{
	return m_SurrogateType;
}


// Returns the ObjectID of the object specified in this object reference.
CObjectIDGenerator::ObjectID CObjectTypeRef::GetObjectID() const
{
	assert(!IsNull());
	return m_ObjectID;
}

uint32 CObjectTypeRef::GetObjectFormatVersion() const
{
	assert(m_ObjectFormatVersion != 0);
	return m_ObjectFormatVersion;
}

//----------------------------------------
// Protected methods:

CObjectTypeRef::CObjectTypeRef() :
	m_Type(NULL),
	m_SurrogateType(NULL),
	m_ObjectID(0),
	m_ObjectFormatVersion(0)
{
}

void CObjectTypeRef::Init(CObject const * object, CObjectIDGenerator::ObjectID objID, CSerializationSurrogate const * pSurrogate)
{
	m_ObjectID = objID;
	if (object != NULL)
	{
		m_Type = object->GetType();
		if (m_Type == NULL)
		{
			throw CSerializationException( string("Unexpected error. object->GetType() returned NULL") );
		}
	}
	else
	{
		// the caller shouldn't pass in a NULL object but then a non-zero ObjectID.
		assert(objID == 0);
		m_ObjectID = 0;
	}

	if (pSurrogate)
	{
		m_SurrogateType = pSurrogate->GetType();
		m_ObjectFormatVersion = pSurrogate->GetObjectFormatVersion( );
	}
	else
	{
		m_SurrogateType = NULL;
		ISerializable* pSerializableObj = DynamicCast<ISerializable>(object);
		if (pSerializableObj != NULL)
		{
			m_ObjectFormatVersion = pSerializableObj->GetObjectFormatVersion( );
		}
		else
		{
			m_ObjectFormatVersion = 0; // cannot determine supported version number
		}
	}
}

void CObjectTypeRef::Init(string const & typeStr, CObjectIDGenerator::ObjectID objID, uint32 objectFormatVersion )
{
	m_ObjectFormatVersion = objectFormatVersion;
	m_ObjectID = objID;
	if (objID == 0)
	{
		// TODO: are we handling NULL types's properly. We should check that typeStr == "null" or something, right?
		return;
	}

	m_Type = CType::GetType(typeStr);
	if (m_Type == NULL)
	{
		/*
		If you get an exception here, and typeStr is nonsense, then the file is corrupt.
		
		If typeStr looks like a valid type, then probably you’re linking with /OPT:REF, which 
		eliminates unreferenced data and functions. You need to call something to make sure 
		the type is linked in and registered and therefore deserializable. Please try calling 
		WT::Using<T>() somewhere in your code (perhaps in WinMain()), where T is the type 
		described by typeStr.
		*/
		throw CSerializationTypeUnknownException( string("Type '") + typeStr + "' not found in static type map; try WT::Using<>()" );
	}

	m_SurrogateType = NULL;
}

void CObjectTypeRef::Init(string const & typeStr, CObjectIDGenerator::ObjectID objID, 
						 string const & surrogateTypeStr, uint32 objectFormatVersion )
{
	Init(typeStr, objID, objectFormatVersion);
	m_SurrogateType = CType::GetType(surrogateTypeStr);
	if (m_SurrogateType == NULL)
	{
		std::ostringstream ssmsg;
		ssmsg << "Surrogate attribute found (" << surrogateTypeStr;
		ssmsg << "). But could not find surrogate of that type in the static type map.";
		throw CSerializationTypeUnknownException( ssmsg.str() );
	}
}

} // Serialization namespace
} // System namespace
} // WT namespace