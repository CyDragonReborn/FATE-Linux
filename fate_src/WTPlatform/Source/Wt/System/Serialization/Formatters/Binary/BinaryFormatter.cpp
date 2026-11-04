#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/Binary/BinaryFormatter.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationException.h"
#include "WT/System/ScopedClasses.h"
#include "WT/System/Serialization/Formatters/SurrogateSelector.h"

#include <iostream>
#include <sstream>

using namespace WT::System::IO;

namespace WT
{
namespace System
{
namespace Serialization
{

const uint8 FORMAT_HEADER_ID_VALUE_STR[] = "WTBIN";

// - Moved to binary format version 2 when strings and uints in object refs were changed to compact formats
// - Moved to binary format version 3 when object references after the first one were made much smaller.
const int32 CURRENT_VERSION_VALUE_INT = 3;

// this is the version number to assume a file is if no "version" attribute is present.
// This should always be 1.
const int32 ASSUMED_VERSION_VALUE_INT = 1; 


WT_DEFINE_CLASS(CBinaryFormatter)

CBinaryFormatter::CBinaryFormatter() :
	m_TypeStream(NULL), m_StreamOnLoan(false),
	m_pSInfo(NULL), m_pSContext(NULL)
{
}


void CBinaryFormatter::Serialize ( CStream* serializationStream , CObject* graph, CSerializationContext* pContext )
{
	CSerializationInfo sinfo(this);
	CSerializationContext AutoContext;
	if (pContext == NULL)
	{
		pContext = &AutoContext;
	}

	assert(serializationStream != NULL);
	assert(m_TypeStream == NULL);
	assert(m_ObjectQueue.empty());
	assert(!m_StreamOnLoan);

	SetSerializationInfoAndContext(&sinfo, pContext);
	SetCurrentObjectAndFormatVersion(NULL, 0);
	m_StreamOnLoan = false;

	m_IdGenerator.Reset();

	m_TypeStream = new CTypeStream(serializationStream);
	CScopedDeleteAndClear<CTypeStream> ScopedDeleteTypeStream(m_TypeStream);

	WriteHeaderAndVersion();

	Schedule(graph);

	HObject hNextObj;
	CObjectIDGenerator::ObjectID objID = 0;

	while ((hNextObj = GetNext(&objID)) != NULL)
	{
		WriteObjectDefinition( hNextObj, objID );
	}

	SetSerializationInfoAndContext(NULL, NULL);
}

HObject CBinaryFormatter::Deserialize ( CStream* serializationStream, CSerializationContext* pContext )
{
	CSerializationInfo sinfo(this);
	CSerializationContext AutoContext;
	if (pContext == NULL)
	{
		pContext = &AutoContext;
	}

	assert(serializationStream != NULL);
	assert(m_TypeStream == NULL);
	assert(m_ObjectQueue.empty());
	assert(!m_StreamOnLoan);

	m_StreamOnLoan = false;

	m_IdGenerator.Reset();
	m_ObjectFinalizer.Reset();

	SetSerializationInfoAndContext(&sinfo, pContext);
	SetCurrentObjectAndFormatVersion(NULL, 0);

	m_TypeStream = new CTypeStream(serializationStream);
	CScopedDeleteAndClear<CTypeStream> ScopedDeleteTypeStream(m_TypeStream);

	ReadHeaderAndVersion();

	CObjectTypeRef rootObjectTypeInfo = ReadObjectTypeRef();

	HObject DeserializedRootObj;
	if (!rootObjectTypeInfo.IsNull())
	{

		DeserializedRootObj = rootObjectTypeInfo.GetRefType()->CreateInstance();
		// TODO: throw if DeserializedRootObj == NULL, type is not createable

		SetCurrentObjectAndFormatVersion(DeserializedRootObj, rootObjectTypeInfo.GetObjectFormatVersion());

		Schedule(DeserializedRootObj);

		// we have an object now, now we need to fill it in.

		HObject hNextObj;
		CObjectIDGenerator::ObjectID objID = 0;

		while ((hNextObj = GetNext( &objID )) != NULL)
		{
			CObjectTypeRef objectTypeInfo;
			if (DeserializedRootObj == hNextObj)
			{
				objectTypeInfo = rootObjectTypeInfo;
			}
			else
			{
				objectTypeInfo = ReadObjectTypeRef();
			}
			ReadObjectDefinition( hNextObj, objID, &objectTypeInfo );
		}
	}
	else
	{
		// a NULL object was serialized out, so we'll just leave 
		// the DeserializedObj to be NULL and return it.

		SetCurrentObjectAndFormatVersion(NULL, rootObjectTypeInfo.GetObjectFormatVersion());
	}

	m_ObjectFinalizer.FinalizeAllObjects(GetSerializationInfo(), GetSerializationContext());

	SetSerializationInfoAndContext(NULL, NULL);

	return DeserializedRootObj;
}



// When overridden in a derived class, writes a Boolean value to the stream already attached to the formatter. 
void CBinaryFormatter::WriteBool(bool value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );

	m_TypeStream->WriteByte( static_cast<uint8>(value ? 0x01 : 0x00) );
}

// When overridden in a derived class, writes an 8-bit unsigned integer to the stream already attached to the formatter. 
void CBinaryFormatter::WriteUInt8(uint8 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );

	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a DateTime value to the stream already attached to the formatter. 
void CBinaryFormatter::WriteDateTime(const CDateTime& value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );

	m_TypeStream->WriteValue( value.Ticks() );
}

// When overridden in a derived class, writes an 8-bit signed integer to the stream already attached to the formatter. 
void CBinaryFormatter::WriteInt8(int8 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 16-bit signed integer to the stream already attached to the formatter. 
void CBinaryFormatter::WriteInt16(int16 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 32-bit signed integer to the stream. 
void CBinaryFormatter::WriteInt32(int32 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 64-bit signed integer to the stream. 
void CBinaryFormatter::WriteInt64(int64 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 32-bit float to the stream. 
void CBinaryFormatter::WriteFloat32(float32 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 64-bit float to the stream. 
void CBinaryFormatter::WriteFloat64(float64 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a TimeSpan value to the stream already attached to the formatter. 
void CBinaryFormatter::WriteTimeSpan(const CTimeSpan& value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value.Ticks() );
}


// When overridden in a derived class, writes a 16-bit unsigned integer to the stream already attached to the formatter. 
void CBinaryFormatter::WriteUInt16(uint16 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 32-bit unsigned integer to the stream already attached to the formatter. 
void CBinaryFormatter::WriteUInt32(uint32 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a 64-bit unsigned integer to the stream already attached to the formatter. 
void CBinaryFormatter::WriteUInt64(uint64 value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteValue( value );
}

// When overridden in a derived class, writes a string to the stream already attached to the formatter. 
void CBinaryFormatter::WriteString(const string& value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteCompactValue( value );
}

// When overridden in a derived class, writes a wide string to the stream already attached to the formatter. 
void CBinaryFormatter::WriteWString(const wstring& value, const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	m_TypeStream->WriteCompactValue( value );
}

// When overridden in a derived class, writes an object reference to the stream already attached to the formatter. 
void CBinaryFormatter::WriteObjectByRef(CObject* obj, const wstring& name, const CType* type)
{
	//TODO: avy, what is "type" supposed to used for?

	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );

	CObjectIDGenerator::ObjectID objectId = 0;
	bool firstReference = false;
	if (obj != NULL)
	{
		// if this object isn't already in the m_IdGenerator (has an Id associated with it)
		// then this is the first reference we've encountered with regard to this object.
		firstReference = ! this->m_IdGenerator.HasID(obj);
		objectId = Schedule(obj);
	}
	CObjectTypeRef objTypeInfo(obj, objectId);

	// indicate to WriteObjectTypeRef whether or not this is the first reference
	// to this object. WriteObjectTypeRef() will output additional data needed for object construction
	// if this is the first reference.
	WriteObjectTypeRef(objTypeInfo, true, firstReference);
}

void CBinaryFormatter::WriteObjectByValue(CObject const & rObj, const wstring& name, const CType* type)
{
	bool firstTime;
	CObjectIDGenerator::ObjectID objectId = m_IdGenerator.GetID( const_cast<CObject*>( &rObj ), firstTime );
	assert(firstTime);

	WriteObjectDefinition( &rObj, objectId );	
}


bool CBinaryFormatter::ReadBool(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetBool( );
}

uint8 CBinaryFormatter::ReadUInt8(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetUInt8( );
}

CDateTime CBinaryFormatter::ReadDateTime(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return CDateTime( m_TypeStream->GetInt64( ) );
}

int8 CBinaryFormatter::ReadInt8(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetInt8( );
}

int16 CBinaryFormatter::ReadInt16(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetInt16();
}

int32 CBinaryFormatter::ReadInt32(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetInt32();
}

int64 CBinaryFormatter::ReadInt64(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetInt64();
}

float32 CBinaryFormatter::ReadFloat32(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetFloat32();
}

float64 CBinaryFormatter::ReadFloat64(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetFloat64();
}

CTimeSpan CBinaryFormatter::ReadTimeSpan(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return CTimeSpan( m_TypeStream->GetInt64() );
}

uint16 CBinaryFormatter::ReadUInt16(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetUInt16();
}

uint32 CBinaryFormatter::ReadUInt32(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetUInt32();
}

uint64 CBinaryFormatter::ReadUInt64(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetUInt64();
}

string CBinaryFormatter::ReadString(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	if (this->m_DeserializingFormatVersion > 1)
	{
		return m_TypeStream->GetCompactString();
	}
	else
	{
		return m_TypeStream->GetString();
	}
}

wstring CBinaryFormatter::ReadWString(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetCompactWString();
}

HObject CBinaryFormatter::ReadObjectByRef(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );

	CObjectTypeRef objTypeInfo = ReadObjectTypeRef();
	
	HObject hObject;
	if (!objTypeInfo.IsNull())
	{			
		hObject = m_IdGenerator.GetObjectByID( objTypeInfo.GetObjectID() );
		if ( hObject == NULL )
		{
			// no object with the ID has been found,
			// So now construct an object of the correct type,
			// add it to the map and return the object.
			assert(objTypeInfo.GetRefType() != NULL);

			hObject = objTypeInfo.GetRefType()->CreateInstance();
			
			assert(hObject != NULL); // TODO: throw if obj == NULL, type is not createable

			CObjectIDGenerator::ObjectID objId = Schedule( hObject );

			// since ObjectID are issues in the same manner during serialization 
			// as during deserialization... these two ID should match.
			objId;
			assert(objTypeInfo.GetObjectID() == objId); 

			// this Id was unassigned when we entered this block, it should be assigned now.
			assert( NULL != m_IdGenerator.GetObjectByID( objTypeInfo.GetObjectID() ) );
		}
	}

	m_ObjectFinalizer.AddObjectFinalizableObjectReference( GetCurrentObject(), hObject );

	// Warning: this object is being returned but it hasn't been deserialized into yet.
	return hObject;
}

void CBinaryFormatter::ReadObjectByValue(const wstring& name, CObject& rObject)
{
	bool firstTime;
	CObjectIDGenerator::ObjectID objectId = this->m_IdGenerator.GetID(&rObject, firstTime);
	assert(firstTime);
	CObjectTypeRef objectTypeRef = ReadObjectTypeRef();
	ReadObjectDefinition(&rObject, objectId, &objectTypeRef );
}

CStream* CBinaryFormatter::BeginStreamBlock(const wstring& name)
{
	assert(!m_StreamOnLoan);
	assert(m_TypeStream);
	m_StreamOnLoan = true;
	return m_TypeStream;
}

void CBinaryFormatter::EndStreamBlock()
{
	assert(m_StreamOnLoan);
	m_StreamOnLoan = false;
}

void CBinaryFormatter::WriteContainerBegin(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
}

void CBinaryFormatter::WriteContainerBeginWithCount(const wstring& name, uint32 count)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
	
	return m_TypeStream->WriteValue(count);
}

void CBinaryFormatter::WriteContainerEnd()
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );
}


void CBinaryFormatter::ReadContainerBegin(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
}

uint32 CBinaryFormatter::ReadContainerBeginWithCount(const wstring& name)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
	
	return m_TypeStream->GetUInt32();
}

void CBinaryFormatter::ReadContainerEnd()
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanRead() );
}



bool CBinaryFormatter::IgnoresNames()
{
	return true;
}

uint32 CBinaryFormatter::GetCurrentObjectFormatVersion() const
{
	return m_CurrentObjectFormatVersion;
}

void CBinaryFormatter::SetCurrentObjectFormatVersion( uint32 Version )
{
	m_CurrentObjectFormatVersion = Version;
}

void CBinaryFormatter::SetCurrentObjectAndFormatVersion(CObject* pObj, uint32 version)
{
	m_pCurrentObject = pObj;
	m_CurrentObjectFormatVersion = version;
}

CObject* CBinaryFormatter::GetCurrentObject()
{
	return m_pCurrentObject;
}

bool CBinaryFormatter::ObjectIsFinalized(CObject const * pObject) const
{
	return m_ObjectFinalizer.ObjectIsFinalized(pObject);
}

void CBinaryFormatter::WriteHeaderAndVersion()
{
	// write out the header id characters
	m_TypeStream->Write(static_cast<const uint8*>(FORMAT_HEADER_ID_VALUE_STR), sizeof(FORMAT_HEADER_ID_VALUE_STR));

	// write out the format version
	m_TypeStream->WriteValue(CURRENT_VERSION_VALUE_INT);
}

// may throw SerializationFormatVersionError
void CBinaryFormatter::ReadHeaderAndVersion()
{
	// assume format version 1 unless we determin otherwise
	m_DeserializingFormatVersion = ASSUMED_VERSION_VALUE_INT; 

	// there may or maynot be a header id. If this is a slightly old format it will not have one
	// first save the current position so that we can reset it if we don't find the header id
	int32 origPos = m_TypeStream->GetPosition();
	uint8 headerIdBuf[sizeof(FORMAT_HEADER_ID_VALUE_STR)];
	m_TypeStream->Read(static_cast<uint8*>(headerIdBuf), sizeof(FORMAT_HEADER_ID_VALUE_STR));
	if (memcmp(headerIdBuf, FORMAT_HEADER_ID_VALUE_STR, sizeof(FORMAT_HEADER_ID_VALUE_STR)) == 0)
	{
		// we found the header id, so read in the format version number
		m_DeserializingFormatVersion = m_TypeStream->GetInt32();
	}
	else
	{
		// didn't find the header id, so reset the stream position
		m_TypeStream->SetPosition(origPos);
	}

	// check to be sure we can read this format version
	if (m_DeserializingFormatVersion > CURRENT_VERSION_VALUE_INT)
	{
		// Something is wrong. This must be an older formatter, and
		// we are trying to read in a format that was generated by a
		// newer formater. We will not be able to deserialize this stream.
		std::ostringstream ssmsg;
		ssmsg << "Cannot deserialize stream. Stream is format version " << m_DeserializingFormatVersion ;
		ssmsg << " but this formatter ("<< this->GetType()->GetFullName() <<") can only handle formats up to and including version ";
		ssmsg << CURRENT_VERSION_VALUE_INT;
		throw CSerializationFormatVersionError(ssmsg.str());
	}
}


void CBinaryFormatter::SetSerializationInfoAndContext(CSerializationInfo* pSInfo, CSerializationContext* pSContext)
{
	m_pSInfo = pSInfo;
	m_pSContext = pSContext;
}

CSerializationInfo* CBinaryFormatter::GetSerializationInfo()
{
	return m_pSInfo;
}

CSerializationContext* CBinaryFormatter::GetSerializationContext()
{
	return m_pSContext;
}







namespace ObjectRefTypeFormat
{
	// This enum is uses a single byte to define type information formats.
	// Right now there are only two TypeFormats:
	//   NullType: indicates that there is no type information but the object 
	//             is a NULL pointer value.
	//
	//  ObjectFullClassNameStringType: indicates that the FullClassName of an 
	//           object will appear next in the stream as a string.
	//
	// Other formats might be added later such as base type identiers or other more 
	// compact Object Type formats.
	enum Type
	{
		NullType = 0x00,
		ExpandedObjectRefType = 0x01,
		CompactObjectRefType = 0x02,
		SecondaryCompactObjectRefType = 0x03,
	};
}

namespace CompactObjectRefFlags // used only by the CompactObjectRefType format
{
	const uint16 SurrogateTypePresent	= 0x0001;
	const uint16 FormatVersionPresent	= 0x0002;
}

CObjectTypeRef CBinaryFormatter::ReadObjectTypeRef() const
{
	ObjectRefTypeFormat::Type typeFormat = static_cast<ObjectRefTypeFormat::Type>(m_TypeStream->GetUInt8());
	switch (typeFormat)
	{
	case ObjectRefTypeFormat::NullType:
		return CObjectTypeRef(NULL, 0);
	case ObjectRefTypeFormat::ExpandedObjectRefType:
		{
			string FullClassName = m_TypeStream->GetString();
			CObjectIDGenerator::ObjectID objectID = m_TypeStream->GetInt64();
			return CObjectTypeRef(FullClassName, objectID, 1); // assume object format version of one, records in this format were in very early versions
		}
	case ObjectRefTypeFormat::CompactObjectRefType:
		{
			uint16 ObjectRefFlags = m_TypeStream->GetUInt16();

			CObjectIDGenerator::ObjectID objectID = m_TypeStream->GetCompactUIntType();
			
			string FullClassName;
			FullClassName = m_TypeStream->GetCompactString();

			string SurrogateClassName;
			if ( CompactObjectRefFlags::SurrogateTypePresent & ObjectRefFlags )
			{
				SurrogateClassName = m_TypeStream->GetCompactString();
			}

			uint32 objectFormatVersion = 1;
			if ( CompactObjectRefFlags::FormatVersionPresent & ObjectRefFlags )
			{
				objectFormatVersion = static_cast<uint32>(m_TypeStream->GetCompactUIntType());			
			}

			if (SurrogateClassName == "")
			{
				return CObjectTypeRef(FullClassName, objectID, objectFormatVersion);
			}
			else
			{
				return CObjectTypeRef(FullClassName, objectID, SurrogateClassName, objectFormatVersion);
			}
		}
	case ObjectRefTypeFormat::SecondaryCompactObjectRefType:
		{
			CObjectIDGenerator::ObjectID objectID = m_TypeStream->GetCompactUIntType();
			CObject const * pObj = this->m_IdGenerator.GetObjectByID(objectID);
			pObj;
			assert(pObj); // should exist because this is not the first reference to this instance.

			// pass 0 for the object format version since we have no way of knowing 
			// what the object format version should be. 0 is an invalid format version
			// and CObjectTypeRef::GetObjectFormatVersion() will assert rather than 
			// return 0. This allows use to be confident that this version here is never being
			// accessed.
			return CObjectTypeRef(pObj->GetClassType()->GetFullName(), objectID, 0 );
		}
	default:
		{
			// Read in an Unknown TypeFormat value
			std::ostringstream ssmsg;
			ssmsg << "Invalid data. Encountered an unknown TypeFormat when reading an ObjectTypeInfo record. TypeFormat value: ";
			ssmsg << typeFormat;
			throw CSerializationException( ssmsg.str() );
		}
	}
}

void CBinaryFormatter::WriteObjectTypeRef(CObjectTypeRef const & objectTypeRef, bool isRef, bool isFirstReference)
{
	assert( m_TypeStream );
	assert( m_TypeStream->CanWrite() );

	if (objectTypeRef.IsNull())
	{
		uint8 typeFormat = static_cast<uint8>(ObjectRefTypeFormat::NullType);
		m_TypeStream->WriteValue(typeFormat);
	}
	else if (!isRef || isFirstReference)
	{
		uint16 ObjectRefFlags = 0x0000;
		uint8 typeFormat = static_cast<uint8>(ObjectRefTypeFormat::CompactObjectRefType);
		m_TypeStream->WriteValue(typeFormat);
		
		if (! isRef)
		{
			if (objectTypeRef.HasSurrogate())
			{
				ObjectRefFlags |= CompactObjectRefFlags::SurrogateTypePresent;
			}

			ObjectRefFlags |= CompactObjectRefFlags::FormatVersionPresent;
		}

		m_TypeStream->WriteValue(ObjectRefFlags);
		m_TypeStream->WriteCompactValue(objectTypeRef.GetObjectID());
		m_TypeStream->WriteCompactValue(objectTypeRef.GetRefType()->GetFullName());

		if (! isRef)
		{
			if (objectTypeRef.HasSurrogate())
			{
				m_TypeStream->WriteCompactValue(objectTypeRef.GetSurrogateType()->GetFullName());
			}
			m_TypeStream->WriteCompactValue(objectTypeRef.GetObjectFormatVersion());
		}
	}
	else
	{
		uint8 typeFormat = static_cast<uint8>(ObjectRefTypeFormat::SecondaryCompactObjectRefType);
		m_TypeStream->WriteValue(typeFormat);
		m_TypeStream->WriteCompactValue(objectTypeRef.GetObjectID());
	}
}


void CBinaryFormatter::WriteObjectDefinition(CObject const * pObject, CObjectIDGenerator::ObjectID objID)
{
	// save previous object and object format version
	CObject* pPreviousObject = GetCurrentObject();
	uint32 previousObjectVersion = GetCurrentObjectFormatVersion();

	ISerializable* serializableObj = DynamicCast<ISerializable>(pObject);
	if (serializableObj)
	{
		// we need to first write out some type information about this object
		CObjectTypeRef objTypeInfo(pObject, objID);
		WriteObjectTypeRef(objTypeInfo, false);

		uint32 objVersion  = serializableObj->GetObjectFormatVersion();
		SetCurrentObjectAndFormatVersion(const_cast<CObject*>(pObject), objVersion);

		// now we'll let the object inform us how to serialize it out
		serializableObj->GetObjectData(GetSerializationInfo(), GetSerializationContext());
	}
	else
	{
		// the object does not implement ISerializable so try to
		// get a surrogate to handle serialization of this object
		CSerializationSurrogate* pSurrogate = CSurrogateSelector::SelectSurrogateOrThrow(pObject);
		pSurrogate->SetInnerObject(const_cast<CObject*>(pObject));

		CObjectTypeRef objTypeInfo(pObject, objID, pSurrogate);
		WriteObjectTypeRef(objTypeInfo, false);

		
		uint32 objVersion = pSurrogate->GetObjectFormatVersion();
		SetCurrentObjectAndFormatVersion(const_cast<CObject*>(pObject), objVersion);

		// now we'll let the object inform us how to serialize it out
		pSurrogate->GetObjectData(GetSerializationInfo(), GetSerializationContext());
	}

	// restore previous current object and object format version
	SetCurrentObjectAndFormatVersion(pPreviousObject, previousObjectVersion);
}

void CBinaryFormatter::ReadObjectDefinition(CObject* pObject, CObjectIDGenerator::ObjectID expectedObjID, 
										   CObjectTypeRef const * objectTypeInfo)
{
	// save previous object and object format version
	CObject* pPreviousObject = GetCurrentObject();
	uint32 previousObjectVersion = GetCurrentObjectFormatVersion();

	SetCurrentObjectAndFormatVersion(pObject, objectTypeInfo->GetObjectFormatVersion());

	if ( objectTypeInfo->IsNull() )
	{
		throw CSerializationException("Invalid Data. Unexpected Error, ObjectTypeInfo specified a NULL object but prior reference specified non-NULL.");
	}

	if ( objectTypeInfo->GetObjectID() != expectedObjID )
	{
		throw CSerializationException("Invalid Data. Unexpected Error, ObjectTypeInfo ObjectID does not match ObjectID of instance to be deserialized into.");
	}

	if ( pObject->GetType() != objectTypeInfo->GetRefType() )
	{
		throw CSerializationException("Invalid Data. Unexpected Error, ObjectTypeInfo Type data does not match Type data of instance to be deserialized into.");
	}

	ISerializable* serializableObj = DynamicCast<ISerializable>(pObject);
	if (serializableObj == NULL)
	{
		if ( !objectTypeInfo->HasSurrogate() )
		{
			throw CSerializationException( "Object does not implement ISerializable and no surrogate attribute was found. Cannot Deserialize object. Object Type: " + pObject->GetType()->GetFullName() );
		}

		CSerializationSurrogate* pSurrogate = NULL;
		HObject hObject = objectTypeInfo->GetSurrogateType()->CreateInstance();
		if (hObject != NULL)
		{
			pSurrogate = DynamicCast<CSerializationSurrogate>( hObject.get() );
		}

		if (pSurrogate == NULL)
		{
			std::ostringstream ssmsg;
			ssmsg << "Unexpected error. Failed to construct serialization surrogate (" << objectTypeInfo->GetSurrogateType()->GetFullName() << ")";
			ssmsg << "Found type object, but CreateInstance() returned NULL or the returned object did not implement ISerializationSurrogate";
			throw CSerializationException(ssmsg.str());
		}

		pSurrogate->SetInnerObject( hObject );
		m_ObjectFinalizer.AddFinalizableObject( hObject, pSurrogate, true ); // pass ownership of Surrogate to ObjectFinalizer
		pSurrogate->SetObjectData(GetSerializationInfo(), GetSerializationContext());
	}
	else
	{
		m_ObjectFinalizer.AddFinalizableObject( pObject );
		serializableObj->SetObjectData(GetSerializationInfo(), GetSerializationContext());
	}

	// restore previous current object and object format version
	SetCurrentObjectAndFormatVersion(pPreviousObject, previousObjectVersion);
}



} // Serialization namespace
} // System namespace
} // WT namespace
