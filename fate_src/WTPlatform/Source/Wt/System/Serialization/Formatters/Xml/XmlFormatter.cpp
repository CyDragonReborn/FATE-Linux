#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/Xml/XmlFormatter.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/ScopedClasses.h"
#include "WT/System/Serialization/SerializationException.h"
#include "WT/System/IO/MemoryStream.h"
#include "WT/System/IO/Base64Decoder.h"
#include "WT/System/Serialization/Formatters/SurrogateSelector.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>

using namespace WT::System::IO;

namespace WT
{
namespace System
{
namespace Serialization
{

using std::ostringstream;

const wstring TYPE_ATTRIB_NAME = L"type";
const wstring OBJECTID_ATTRIB_NAME = L"objectid";
const wstring OBJECTREF_ATTRIB_NAME = L"objectref";
const wstring COUNT_ATTRIB_NAME = L"size";
const wstring ENCODING_ATTRIB_NAME = L"encoding";
const wstring XMLNS_WT_ATTRIB_NAME = L"xmlns:wt";
const wstring VERSION_ATTRIB_NAME = L"version";
const wstring OBJECTVERSION_ATTRIB_NAME = L"classversion";

// if this attrib is present, then it contains the type name of the class used to serialize out this object
const wstring SURROGATE_ATTRIB_NAME = L"surrogate"; 

const wstring BOOL_TYPE_NAME     = TypeTraits<bool>::ToWString();
const wstring INT8_TYPE_NAME     = TypeTraits<int8>::ToWString();
const wstring INT16_TYPE_NAME    = TypeTraits<int16>::ToWString();
const wstring INT32_TYPE_NAME    = TypeTraits<int32>::ToWString();
const wstring INT64_TYPE_NAME    = TypeTraits<int64>::ToWString();
const wstring UINT8_TYPE_NAME    = TypeTraits<uint8>::ToWString();
const wstring UINT16_TYPE_NAME   = TypeTraits<uint16>::ToWString();
const wstring UINT32_TYPE_NAME   = TypeTraits<uint32>::ToWString();
const wstring UINT64_TYPE_NAME   = TypeTraits<uint64>::ToWString();
const wstring FLOAT32_TYPE_NAME  = TypeTraits<float32>::ToWString();
const wstring FLOAT64_TYPE_NAME  = TypeTraits<float64>::ToWString();
const wstring STRING_TYPE_NAME   = TypeTraits<string>::ToWString();
const wstring WSTRING_TYPE_NAME  = TypeTraits<wstring>::ToWString();
const wstring DATETIME_TYPE_NAME = TypeTraits<CDateTime>::ToWString();
const wstring TIMESPAN_TYPE_NAME = TypeTraits<CTimeSpan>::ToWString();

const wstring CONTAINER_TYPE_NAME = L"WT::System::Serialization::Container";
const wstring RAWSTREAM_TYPE_NAME = L"WT::System::Serialization::RawStreamData";

const wstring BASE64_ENCODING_NAME = L"base64";

const wstring TRUE_VALUE_STR = L"true";
const wstring FALSE_VALUE_STR = L"false";
const wstring NULL_VALUE_STR = L"null";
const wstring NULL_OBJECTID_VALUE_STR = L"0";
const wstring XMLNS_WT_URI_VALUE_STR = L"http://www.wildtangent.org/wtplatform/xml/ns";

const wstring ROOT_ELEMENT_NAME = L"wt:root";
const wstring OBJECT_ELEMENT_NAME = L"wt:object";


const int32 CURRENT_VERSION_VALUE_INT = 1;

// this is the version number to assume a file is if no "version" attribute is present.
// This should always be 1.
const int32 ASSUMED_VERSION_VALUE_INT = 1; 




WT_DEFINE_CLASS(CXmlFormatter)

CXmlFormatter::CXmlFormatter()
	: m_XmlWriter(NULL)
	, m_XmlReader(NULL)
	, m_LoanStream(NULL)
	, m_BaseStream(NULL)
	, m_pSInfo(NULL)
	, m_pSContext(NULL)
	, m_DeserializingFormatVersion( 0 )
	, m_SerializationEncoding(StringEncoding::UTF8)
	, m_CurrentObjectFormatVersion( 0 )
	, m_pCurrentObject( NULL )
{
}

CXmlFormatter::CXmlFormatter(StringEncoding::Type serializationEncoding) :
	m_XmlWriter(NULL), m_XmlReader(NULL), m_LoanStream(NULL), m_BaseStream(NULL),
	m_pSInfo(NULL), m_pSContext(NULL),
	m_SerializationEncoding(serializationEncoding)
{
}

CXmlFormatter::~CXmlFormatter()
{
	assert(m_BaseStream == NULL);
	assert(m_LoanStream == NULL);
	assert(m_XmlWriter == NULL);
	assert(m_XmlReader == NULL);
}

void CXmlFormatter::Serialize ( CStream* serializationStream , CObject* graph, CSerializationContext* pContext )
{
	CSerializationInfo sinfo(this);
	CSerializationContext AutoContext;
	if (pContext == NULL)
	{
		pContext = &AutoContext;
	}

	assert(serializationStream != NULL);
	assert(m_BaseStream == NULL);
	assert(m_LoanStream == NULL);
	assert(m_XmlWriter == NULL);
	assert(m_XmlReader == NULL);
	assert(m_ObjectQueue.empty());

	SetSerializationInfoAndContext(&sinfo, pContext);
	m_IdGenerator.Reset();

	m_BaseStream = serializationStream;
	// we don't own m_BaseStream, but we want to set it to NULL before returning from Serialize
	CScopedClear<CStream> scopedClearBaseStream(m_BaseStream); 

	m_XmlWriter = new CXmlWriter(serializationStream, m_SerializationEncoding);
	CScopedDeleteAndClear<CXmlWriter> scopedDeleteXmlWriter(m_XmlWriter);

	m_XmlWriter->WriteStartDocument();
	m_XmlWriter->WriteStartElement(ROOT_ELEMENT_NAME);

	WriteFormatVersion();

	CObjectTypeRef rootObjectTypeRef(graph, graph ? 1 : 0);
	WriteObjectTypeRef(rootObjectTypeRef, true);

	m_XmlWriter->WriteAttributeString( XMLNS_WT_ATTRIB_NAME, XMLNS_WT_URI_VALUE_STR);

	Schedule(graph);

	HObject hNextObj;
	CObjectIDGenerator::ObjectID objID = 0;


	// objID will be 1 for the 1st (root) object
	while ((hNextObj = GetNext(&objID)) != NULL)
	{
		WriteObjectDefinition( OBJECT_ELEMENT_NAME, hNextObj, objID );
	}

	m_XmlWriter->WriteEndElement(); // wt:root
	m_XmlWriter->WriteEndDocument();

	SetSerializationInfoAndContext(NULL, NULL);
}



HObject CXmlFormatter::Deserialize ( CStream* serializationStream, CSerializationContext* pContext )
{
	CSerializationInfo Info(this);
	CSerializationContext AutoContext;
	if (pContext == NULL)
	{
		pContext = &AutoContext;
	}

	assert(serializationStream != NULL);
	assert(m_BaseStream == NULL);
	assert(m_LoanStream == NULL);
	assert(m_XmlWriter == NULL);
	assert(m_XmlReader == NULL);
	assert(m_ObjectQueue.empty());

	SetSerializationInfoAndContext(&Info, pContext);
	m_IdGenerator.Reset();
	m_ObjectFinalizer.Reset();
	SetCurrentObjectAndFormatVersion(NULL, 0);

	m_BaseStream = serializationStream;
	CScopedClear<CStream> scopedClearBaseStream(m_BaseStream);

	m_XmlReader = new CXmlReader(serializationStream);
	CScopedDeleteAndClear<CXmlReader> scopedDeleteXmlReader(m_XmlReader);

	// advance the xml reader to the first start element, skip processing instructions.
	while (!m_XmlReader->IsStartElement())
	{
		AdvanceReaderOrThrow();
	}

	VerifyNodeTypeOrThrow(NodeType::StartElementNode);
	VerifyNodeNameOrThrow(ROOT_ELEMENT_NAME);

	ReadAndCheckFormatVersion(); // may throw SerializationFormatVersionError

	CObjectTypeRef objectTypeInfo = ReadObjectTypeRef();
	
	HObject DeserializedRootObj;
	if (objectTypeInfo.IsNull())
	{
		// a NULL object was serialized out, so we'll just leave 
		// the DeserializedObj to be NULL and return it.
		return NULL;
	}

	DeserializedRootObj = objectTypeInfo.GetRefType()->CreateInstance();
	// TODO: throw if DeserializedRootObj == NULL, type is not createable

	Schedule(DeserializedRootObj);

	// we have an object now, now we need to fill it in.

	HObject hNextObj;
	CObjectIDGenerator::ObjectID objID = 0;
	
	while ((hNextObj = GetNext(&objID)) != NULL)
	{
		ReadObjectDefinition( OBJECT_ELEMENT_NAME, hNextObj, objID );
	}

	m_ObjectFinalizer.FinalizeAllObjects(&Info, pContext);

	SetSerializationInfoAndContext(NULL, NULL);

	return DeserializedRootObj;
}

// When overridden in a derived class, writes a Boolean value to the stream already attached to the formatter. 
void CXmlFormatter::WriteBool(bool value, const wstring& name)
{
	WriteTypedElement(name, BOOL_TYPE_NAME, value ? L"true" : L"false");
}

// When overridden in a derived class, writes an 8-bit unsigned integer to the stream already attached to the formatter. 
void CXmlFormatter::WriteUInt8(uint8 value, const wstring& name)
{
	std::wostringstream wss;
	wss << static_cast<uint32>(value); // upcast so that we write it out as a number and not a character
	WriteTypedElement(name, UINT8_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a DateTime value to the stream already attached to the formatter. 
void CXmlFormatter::WriteDateTime(const CDateTime& value, const wstring& name)
{
	std::wostringstream wss;
	wss << value.Ticks(); // TODO: format this nicely, use a human readable datetime format
	WriteTypedElement(name, DATETIME_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes an 8-bit signed integer to the stream already attached to the formatter. 
void CXmlFormatter::WriteInt8(int8 value, const wstring& name)
{
	std::wostringstream wss;
	wss << static_cast<int32>(value); // upcast so that we write it out as a number and not a character
	WriteTypedElement(name, INT8_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 16-bit signed integer to the stream already attached to the formatter. 
void CXmlFormatter::WriteInt16(int16 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, INT16_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 32-bit signed integer to the stream. 
void CXmlFormatter::WriteInt32(int32 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, INT32_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 64-bit signed integer to the stream. 
void CXmlFormatter::WriteInt64(int64 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, INT64_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 32-bit float to the stream. 
void CXmlFormatter::WriteFloat32(float32 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, FLOAT32_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 64-bit float to the stream. 
void CXmlFormatter::WriteFloat64(float64 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, FLOAT64_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a TimeSpan value to the stream already attached to the formatter. 
void CXmlFormatter::WriteTimeSpan(const CTimeSpan& value, const wstring& name)
{
	std::wostringstream wss;
	wss << value.Ticks(); // TODO: can we format this better to make it more readable
	WriteTypedElement(name, TIMESPAN_TYPE_NAME, wss.str() );
}


// When overridden in a derived class, writes a 16-bit unsigned integer to the stream already attached to the formatter. 
void CXmlFormatter::WriteUInt16(uint16 value, const wstring& name)
{
	std::wostringstream wss;
	wss << static_cast<uint32>(value);
	WriteTypedElement(name, UINT16_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 32-bit unsigned integer to the stream already attached to the formatter. 
void CXmlFormatter::WriteUInt32(uint32 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, UINT32_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a 64-bit unsigned integer to the stream already attached to the formatter. 
void CXmlFormatter::WriteUInt64(uint64 value, const wstring& name)
{
	std::wostringstream wss;
	wss << value;
	WriteTypedElement(name, UINT64_TYPE_NAME, wss.str() );
}

// When overridden in a derived class, writes a string to the stream already attached to the formatter. 
void CXmlFormatter::WriteString(const string& value, const wstring& name)
{
	WriteTypedElement(name, STRING_TYPE_NAME, WT::AnsiToWide(value) );
}

// When overridden in a derived class, writes a wide string to the stream already attached to the formatter. 
void CXmlFormatter::WriteWString(const wstring& value, const wstring& name)
{
	WriteTypedElement(name, WSTRING_TYPE_NAME, value );
}

// When overridden in a derived class, writes an object reference to the stream already attached to the formatter. 
void CXmlFormatter::WriteObjectByRef(CObject* obj, const wstring& name, const CType* type)
{
	//TODO: what is "type" supposed to used for?

	assert( m_XmlWriter );

	CObjectIDGenerator::ObjectID objectId = 0;
	if (obj != NULL)
	{
		objectId = Schedule(obj);
	}

	m_XmlWriter->WriteStartElement(name);

	CObjectTypeRef objectTypeRef(obj, objectId);
	WriteObjectTypeRef(objectTypeRef, true);

	m_XmlWriter->WriteEndElement();
}

void CXmlFormatter::WriteObjectByValue(CObject const & rObj, const wstring& name, const CType* type)
{
	//TODO: what is "type" supposed to used for?

	assert( m_XmlWriter );

	bool firstTime;
	CObjectIDGenerator::ObjectID objectId = m_IdGenerator.GetID( const_cast<CObject*>( &rObj ), firstTime );
	assert(firstTime);

	WriteObjectDefinition(name, &rObj, objectId );
}


bool CXmlFormatter::ReadBool(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, BOOL_TYPE_NAME);
	if (valueStr == TRUE_VALUE_STR)
	{
		return true;
	}
	else if (valueStr == FALSE_VALUE_STR)
	{
		return false;
	}
	else
	{
		ostringstream ssmsg;
		ssmsg << "Attempted to read a "<< WideToAnsi(BOOL_TYPE_NAME);
		ssmsg << " typed element, but the text value was neither \"";
		ssmsg << WideToAnsi(TRUE_VALUE_STR) <<"\" or \""<< WideToAnsi(FALSE_VALUE_STR) <<"\", instead found: \"";
		ssmsg << WideToAnsi(valueStr) <<"\".";
		throw CSerializationException(ssmsg.str());
		return false;
	}
}

uint8 CXmlFormatter::ReadUInt8(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, UINT8_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return static_cast<uint8>(::_wtoi(valueStr.c_str()));
}

CDateTime CXmlFormatter::ReadDateTime(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, DATETIME_TYPE_NAME);
	// TODO: assuming this is ticks
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return CDateTime(::_wtoi64(valueStr.c_str()));
}

int8 CXmlFormatter::ReadInt8(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, INT8_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return static_cast<int8>(::_wtoi(valueStr.c_str()));
}

int16 CXmlFormatter::ReadInt16(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, INT16_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return static_cast<int16>(::_wtoi(valueStr.c_str()));
}

int32 CXmlFormatter::ReadInt32(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, INT32_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return ::_wtoi(valueStr.c_str());
}

int64 CXmlFormatter::ReadInt64(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, INT64_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return ::_wtoi64(valueStr.c_str());
}

float32 CXmlFormatter::ReadFloat32(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, FLOAT32_TYPE_NAME);
	VerifyIsFloatFormatOrThrow(name, valueStr);
	return static_cast<float32>(::_wtof(valueStr.c_str()));
}

float64 CXmlFormatter::ReadFloat64(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, FLOAT64_TYPE_NAME);
	VerifyIsFloatFormatOrThrow(name, valueStr);
	return ::_wtof(valueStr.c_str());
}

CTimeSpan CXmlFormatter::ReadTimeSpan(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, TIMESPAN_TYPE_NAME);
	// TODO: assuming this is ticks
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return CTimeSpan(::_wtoi64(valueStr.c_str()));
}

uint16 CXmlFormatter::ReadUInt16(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, UINT16_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return static_cast<uint16>(::_wtoi(valueStr.c_str()));
}

uint32 CXmlFormatter::ReadUInt32(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, UINT32_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	return static_cast<uint32>(::_wtoi64(valueStr.c_str()));
}

uint64 CXmlFormatter::ReadUInt64(const wstring& name)
{
	
	wstring valueStr = ReadSimpleTypeText(name, UINT64_TYPE_NAME);
	VerifyIsIntegerFormatOrThrow(name, valueStr);
	// TODO: !!! _atoi64() does not have the precision to convert to uint64, right?
	return static_cast<uint64>(::_wtoi64(valueStr.c_str()));
}

string CXmlFormatter::ReadString(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, STRING_TYPE_NAME);
	return WT::WideToAnsi(valueStr, StringEncoding::DocumentEncoding);
}

wstring CXmlFormatter::ReadWString(const wstring& name)
{
	wstring valueStr = ReadSimpleTypeText(name, WSTRING_TYPE_NAME);
	return valueStr;
}

HObject CXmlFormatter::ReadObjectByRef(const wstring& name)
{
	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::StartElementNode);
	VerifyNodeNameOrThrow(name);

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

	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::EndElementNode);
	VerifyNodeNameOrThrow(name);

	m_ObjectFinalizer.AddObjectFinalizableObjectReference( GetCurrentObject(), hObject );

	// Warning: this object is being returned but it hasn't been deserialized into yet.
	return hObject;
}

void CXmlFormatter::ReadObjectByValue(wstring const & name, CObject& rObject)
{
	assert(m_XmlReader != NULL);

	bool firstTime;
	CObjectIDGenerator::ObjectID objectId = this->m_IdGenerator.GetID(&rObject, firstTime);
	assert(firstTime);
	ReadObjectDefinition(name, &rObject, objectId);

	m_ObjectFinalizer.AddObjectFinalizableObjectReference( GetCurrentObject(), &rObject);
}

void CXmlFormatter::ReadObjectDefinition(wstring const & elementName, CObject* pObject, CObjectIDGenerator::ObjectID expectedObjID)
{
	assert(pObject != NULL);

	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::StartElementNode);
	VerifyNodeNameOrThrow(elementName);

	CObjectTypeRef objectTypeInfo = ReadObjectTypeRef();

	// save previous object version
	uint32 previousObjectVersion = GetCurrentObjectFormatVersion();
	CObject* pPreviousObject = GetCurrentObject();

	// set new object and object version
	SetCurrentObjectAndFormatVersion(pObject, objectTypeInfo.GetObjectFormatVersion());

	if ( objectTypeInfo.IsNull() )
	{
		throw CSerializationException("Invalid Data. Unexpected Error, ObjectTypeInfo specified a NULL object but prior reference specified non-NULL.");
	}

	if ( objectTypeInfo.GetObjectID() != expectedObjID )
	{
		throw CSerializationException("Invalid Data. Unexpected Error, ObjectTypeInfo ObjectID does not match ObjectID of instance to be deserialized into.");
	}

	if ( pObject->GetType() != objectTypeInfo.GetRefType() )
	{
		throw CSerializationException("Invalid Data. Unexpected Error, ObjectTypeInfo Type data does not match Type data of instance to be deserialized into.");
	}

	ISerializable* serializableObj = DynamicCast<ISerializable>(pObject);
	if (serializableObj == NULL)
	{
		// The object does not implement ISerializable,
		// look to see if there is a surrogate attribute. If there is, then construct a surrogate and 
		// use it to deserialize the object.
		// otherwise we'll throw a serialization exception.

		if ( !objectTypeInfo.HasSurrogate() )
		{
			throw CSerializationException( "Object does not implement ISerializable and no surrogate attribute was found. Cannot Deserialize object. Object Type: " + pObject->GetType()->GetFullName() );
		}

		CSerializationSurrogate* pSurrogate = NULL;
		HObject hObject = objectTypeInfo.GetSurrogateType()->CreateInstance();
		if (hObject != NULL)
		{
			pSurrogate = DynamicCast<CSerializationSurrogate>( hObject.get() );
		}

		if (pSurrogate == NULL)
		{
			ostringstream ssmsg;
			ssmsg << "Unexpected error. Failed to construct serialization surrogate (" << objectTypeInfo.GetSurrogateType()->GetFullName() << ")";
			ssmsg << "Found type object, but CreateInstance() returned NULL or returned object did not implement ISerializationSurrogate.";
			throw CSerializationException(ssmsg.str());
		}
		pSurrogate->SetInnerObject( hObject );

		// this must happen before we call SetObjectData(), because we can't add object reference relationships
		// without having the parent object already register with the ObjectFinalizer
		m_ObjectFinalizer.AddFinalizableObject( hObject, pSurrogate, true );  // pass ownership of Surrogate to ObjectFinalizer

		pSurrogate->SetObjectData(GetSerializationInfo(), GetSerializationContext());
	}
	else
	{
		// this must happen before we call SetObjectData(), because we can't add object reference relationships
		// without having the parent object already register with the ObjectFinalizer
		m_ObjectFinalizer.AddFinalizableObject( pObject );

		serializableObj->SetObjectData(GetSerializationInfo(), GetSerializationContext());
	}

	// restore previous object version
	SetCurrentObjectAndFormatVersion(pPreviousObject, previousObjectVersion);

	// error checking
	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::EndElementNode);
	VerifyNodeNameOrThrow(elementName);
}


CStream* CXmlFormatter::BeginStreamBlock(const wstring& name)
{
	assert(m_LoanStream == NULL);

	if (m_LoanStream)
	{
		ostringstream ssmsg;
		ssmsg << "BeginStreamBlock(\""<< WideToAnsi(name) <<"\") called while there is still an outstnading stream on loan.";
		ssmsg << " Call EndStreamBlock() before calling BeginStreamBlock() again.";
		throw CSerializationException( ssmsg.str() );
	}

	// have a try catch block here so that we can clean up m_LoanStream 
	// when there are exceptions thrown from below.
	try
	{
		if (m_XmlWriter)
		{
			// we are writing to the formater so write the necessary xml elements
			// that will surround the raw stream data written.

			m_XmlWriter->WriteStartElement(name);
			m_XmlWriter->WriteAttributeString(TYPE_ATTRIB_NAME, RAWSTREAM_TYPE_NAME);
			m_XmlWriter->WriteAttributeString(ENCODING_ATTRIB_NAME, BASE64_ENCODING_NAME);

			// force the xml writer to close theo start element and move on to 
			// position itself to write a "text node"
			m_XmlWriter->WriteString(L"");

			m_XmlWriter->Flush();

			m_LoanStream = new CBase64Stream(m_XmlWriter->GetEncodedTextStream());
		}
		else
		{
			// we are reading from the formatter, so setup read the xml elements 
			// bracketing the raw stream data

			assert (m_XmlReader);

			AdvanceReaderOrThrow();
			VerifyNodeTypeOrThrow(NodeType::StartElementNode);
			VerifyNodeNameOrThrow(name);
			VerifyAttributeValueOrThrow(TYPE_ATTRIB_NAME, RAWSTREAM_TYPE_NAME);
			VerifyAttributeValueOrThrow(ENCODING_ATTRIB_NAME, BASE64_ENCODING_NAME);

			AdvanceReaderOrThrow();
			VerifyNodeTypeOrThrow(NodeType::TextNode);
			
			// construct a memory stream, pass that to the Base64Decoder to decode into
			m_LoanStream = new CMemoryStream();
			CBase64Decoder::StringToStream(m_XmlReader->Value(), m_LoanStream);

			// we will return the memory stream as the loan stream, first moving the read 
			// position back to the begining
			m_LoanStream->SetPosition(0);
		}
	}
	catch (std::exception& e)
	{
		// clean up the loanStream
		SafeDelete(m_LoanStream);

		// then re-throw (rethrow) the exception
		throw e;
	}

	return m_LoanStream;
}

void CXmlFormatter::EndStreamBlock()
{
	assert(m_LoanStream);

	m_LoanStream->Close();
	SafeDelete(m_LoanStream);

	if (m_XmlWriter)
	{
		// we are writing to the formater so write the necessary xml elements
		// that will surround the raw stream data written.
		m_XmlWriter->WriteEndElement();
	}
	else
	{
		// we are reading from the formatter, so setup read the xml elements 
		// bracketing the raw stream data

		assert (m_XmlReader);
		AdvanceReaderOrThrow();
		VerifyNodeTypeOrThrow(NodeType::EndElementNode);
	}
}

void CXmlFormatter::WriteContainerBegin(const wstring& name)
{
	assert( m_XmlWriter );

	m_XmlWriter->WriteStartElement(name);
	m_XmlWriter->WriteAttributeString(TYPE_ATTRIB_NAME, CONTAINER_TYPE_NAME);
}

void CXmlFormatter::WriteContainerBeginWithCount(const wstring& name, uint32 count)
{
	assert( m_XmlWriter );

	std::wostringstream wss;
	wss << count;
	m_XmlWriter->WriteStartElement(name);
	m_XmlWriter->WriteAttributeString(TYPE_ATTRIB_NAME, CONTAINER_TYPE_NAME);
	m_XmlWriter->WriteAttributeString(COUNT_ATTRIB_NAME, wss.str());
}

void CXmlFormatter::WriteContainerEnd()
{
	m_XmlWriter->WriteEndElement();
}


void CXmlFormatter::ReadContainerBegin(const wstring& name)
{
	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::StartElementNode);
	VerifyNodeNameOrThrow(name);
	VerifyAttributeValueOrThrow(TYPE_ATTRIB_NAME, CONTAINER_TYPE_NAME);
}

uint32 CXmlFormatter::ReadContainerBeginWithCount(const wstring& name)
{
	ReadContainerBegin(name);

	wstring countStr = GetAttributeOrThrow(COUNT_ATTRIB_NAME);
	VerifyIsIntegerFormatOrThrow(name, countStr );
	return static_cast<uint32>(::_wtoi64(countStr.c_str()));
}

void CXmlFormatter::ReadContainerEnd()
{
	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::EndElementNode);
}



bool CXmlFormatter::IgnoresNames()
{
	return false;
}

uint32 CXmlFormatter::GetCurrentObjectFormatVersion() const
{
	return m_CurrentObjectFormatVersion;
}

void CXmlFormatter::SetCurrentObjectFormatVersion( uint32 Version )
{
	m_CurrentObjectFormatVersion = Version;
}

void CXmlFormatter::SetCurrentObjectAndFormatVersion(CObject* pObj, uint32 version)
{
	m_pCurrentObject = pObj;
	m_CurrentObjectFormatVersion = version;
}

CObject* CXmlFormatter::GetCurrentObject()
{
	return m_pCurrentObject;
}

bool CXmlFormatter::ObjectIsFinalized(CObject const * pObject) const
{
	return m_ObjectFinalizer.ObjectIsFinalized(pObject);
}


void CXmlFormatter::WriteTypedElement(wstring const & name, wstring const & typeStr, wstring const & value)
{
	assert( m_XmlWriter );

	m_XmlWriter->WriteStartElement(name);
	m_XmlWriter->WriteAttributeString(TYPE_ATTRIB_NAME, typeStr);
	m_XmlWriter->WriteString(value );
	m_XmlWriter->WriteEndElement();
}

// convert an ObjectId to a string
wstring CXmlFormatter::ObjectIdToString(CObjectIDGenerator::ObjectID objectId)
{
	std::wostringstream wss;
	wss << objectId;
	return wss.str();
}

CObjectTypeRef CXmlFormatter::ReadObjectTypeRef()
{	
	assert( m_XmlReader );
	assert( m_XmlReader->NodeType() == NodeType::StartElementNode );

	wstring typeStr;
	wstring objectRefStr;
	wstring objectIdStr;
	

	typeStr = GetAttributeOrThrow(TYPE_ATTRIB_NAME);
	objectRefStr = GetAttributeOrThrow(OBJECTREF_ATTRIB_NAME);
	objectIdStr = GetAttributeOrThrow(OBJECTID_ATTRIB_NAME);

	wstring objectFormatVersionStr; // assume object format version is 1 if it is not present
	if ( ! m_XmlReader->GetAttribute(OBJECTVERSION_ATTRIB_NAME, objectFormatVersionStr) )
	{
		objectFormatVersionStr = L"1"; // assume object format version is 1 if it is not present
	}

	int32 objectFormatVersion = ::_wtoi( objectFormatVersionStr.c_str() );
	CObjectIDGenerator::ObjectID objId = ::_wtoi64( objectIdStr.c_str() );

	wstring surrogateTypeStr;
	if ( m_XmlReader->GetAttribute(SURROGATE_ATTRIB_NAME, surrogateTypeStr) )
	{
		return CObjectTypeRef(WideToAnsi(typeStr), objId, WideToAnsi(surrogateTypeStr), objectFormatVersion );
	}
	else
	{
		return CObjectTypeRef(WideToAnsi(typeStr), objId, objectFormatVersion );		
	}
}

void CXmlFormatter::WriteObjectTypeRef(CObjectTypeRef& objectTypeRef, bool isRef)
{
	assert(m_XmlWriter);

	CObjectIDGenerator::ObjectID  objId = 0;
	if (!objectTypeRef.IsNull())
	{
		objId = objectTypeRef.GetObjectID();
	}

	wstring objectIdStr = ObjectIdToString(objId);

	wstring objectRefStr = isRef ? TRUE_VALUE_STR : FALSE_VALUE_STR;

	if (objectTypeRef.IsNull())
	{
		// can only have null references, not null definitions
		assert(isRef);
		assert(!objectTypeRef.HasSurrogate());

		m_XmlWriter->WriteAttributeString( TYPE_ATTRIB_NAME, NULL_VALUE_STR );
	}
	else
	{
		m_XmlWriter->WriteAttributeString( TYPE_ATTRIB_NAME,  WT::AnsiToWide(objectTypeRef.GetRefType()->GetFullName()) );
		
		if (objectTypeRef.HasSurrogate())
		{
			assert(!isRef);
			m_XmlWriter->WriteAttributeString( SURROGATE_ATTRIB_NAME, WT::AnsiToWide(objectTypeRef.GetSurrogateType()->GetFullName()) );
		}
	}

	m_XmlWriter->WriteAttributeString( OBJECTREF_ATTRIB_NAME, objectRefStr);
	m_XmlWriter->WriteAttributeString( OBJECTID_ATTRIB_NAME, objectIdStr );

	if (! isRef)
	{
		std::wostringstream wss;
		wss << objectTypeRef.GetObjectFormatVersion();
		m_XmlWriter->WriteAttributeString( OBJECTVERSION_ATTRIB_NAME, wss.str() );
	}
}


wstring CXmlFormatter::ReadSimpleTypeText(wstring const & expectedElementName, wstring const & expectedTypeStr)
{
	AdvanceReaderOrThrow();
	VerifyNodeTypeOrThrow(NodeType::StartElementNode);
	VerifyNodeNameOrThrow(expectedElementName);

	VerifyAttributeValueOrThrow(TYPE_ATTRIB_NAME, expectedTypeStr);

	AdvanceReaderOrThrow();
	wstring text;

	// the next node should be a text node, but if the string is the empty string 
	// then it might just be an EndElementNode
	if (m_XmlReader->NodeType() == NodeType::TextNode)
	{
		text = m_XmlReader->Value();
		AdvanceReaderOrThrow();
	}

	VerifyNodeTypeOrThrow(NodeType::EndElementNode);
	VerifyNodeNameOrThrow(expectedElementName);

	return text;
}




//TODO: this needs to throw exceptions with better messages
void CXmlFormatter::AdvanceReaderOrThrow()
{
	if ( ! m_XmlReader->Read() )
	{
		throw CSerializationException("Failed to read next node");
	}
}

void CXmlFormatter::VerifyNodeTypeOrThrow(NodeType::Type expectedNodeType)
{
	if ( m_XmlReader->NodeType() != expectedNodeType )
	{
		throw CSerializationException("Expected to read one node type. Got something else.");
	}
}

void CXmlFormatter::VerifyNodeNameOrThrow(wstring const & expectedElementName)
{
	if ( m_XmlReader->Name() != expectedElementName)
	{
		ostringstream ssmsg;
		ssmsg << "Expected to read <"<<WideToAnsi(expectedElementName)<<">. Instead read <"<<WideToAnsi(m_XmlReader->Name())<<">.";
		throw CSerializationException(ssmsg.str());
	}
}

wstring CXmlFormatter::GetAttributeOrThrow(wstring const & requiredAttributePresent)
{
	wstring valueStr;
	if ( ! m_XmlReader->GetAttribute(requiredAttributePresent, valueStr) )
	{
		ostringstream ssmsg;
		ssmsg << "Tag <"<<WideToAnsi(m_XmlReader->Name())<<"> was expected to have a \""<<WideToAnsi(requiredAttributePresent);
		ssmsg << "\" attribute. No such attribute found.";
		throw CSerializationException(ssmsg.str());
	}
	return valueStr;
}


void CXmlFormatter::VerifyAttributeValueOrThrow(
	wstring const & requiredAttributePresent, 
	wstring const & expectedAttributeValue)
{

	wstring valueStr = GetAttributeOrThrow(requiredAttributePresent);

	if ( valueStr !=  expectedAttributeValue )
	{
		ostringstream ssmsg;
		ssmsg << "In tag <"<<WideToAnsi(m_XmlReader->Name())<<"> Expected to find attribute "<<WideToAnsi(requiredAttributePresent);
		ssmsg << "=\""<<WideToAnsi(expectedAttributeValue);
		ssmsg << "\". Instead found "<<WideToAnsi(requiredAttributePresent)<<"=\""<<WideToAnsi(valueStr)<<"\".";
		throw CSerializationException(ssmsg.str());
	}
}

void CXmlFormatter::VerifyIsIntegerFormatOrThrow(wstring const & elementName, wstring const & valueStr)
{
	wstring trimmedStr = Trim(valueStr);
	int32 len = static_cast<int32>(trimmedStr.size());
	int32 startIdx = 0;
	
	// if the first character is a plus or minus then advance to the next character
	if (trimmedStr[startIdx] == '+' || trimmedStr[startIdx] == '-')
	{
		startIdx++;
	}

	bool badFormat = false;
	if (trimmedStr[startIdx] != '0' || startIdx == len-1) // only allow integers to start with 0 if it is the only digit
	{
		for (int32 i = startIdx; i < len; i++)
		{
			if ( !isdigit(trimmedStr[i]) )
			{
				badFormat = true;
				break;
			}
		}
	}
	else
	{
		badFormat = true;
	}

	if (badFormat)
	{
		ostringstream ssmsg;
		ssmsg << "Bad integer format. While parsing <" << WideToAnsi(elementName) << ">";
		ssmsg << " expected an integer value but got: " << WideToAnsi(trimmedStr) << ", integers may begin with +/-,";
		ssmsg << " but should then only contain digits. Integers may not start with 0.";
		throw CSerializationException(ssmsg.str());
	}
}

void CXmlFormatter::VerifyIsFloatFormatOrThrow(wstring const & elementName, wstring const & valueStr)
{

	// TODO: this is note really performing bullet proof validation its just checking for illegal characters
	// but valid characters might appear in illegal order and still get past this method.

	wstring trimmedStr = Trim(valueStr);
	int32 len = static_cast<int32>(trimmedStr.size());
	int32 startIdx = 0;

	bool badFormat = false;
	for (int32 i = startIdx; i < len; i++)
	{
		if ( !isdigit(trimmedStr[i]) &&  
			trimmedStr[i] != '.' && 
			trimmedStr[i] != '-' &&
			trimmedStr[i] != '+' &&
			trimmedStr[i] != 'E' &&
			trimmedStr[i] != 'e' )
		{
			badFormat = true;
			break;
		}
	}

	if (badFormat)
	{
		ostringstream ssmsg;
		ssmsg << "Bad floating format. While parsing <" << WideToAnsi(elementName) << ">";
		ssmsg << " expected a floating point value but got: " << WideToAnsi(trimmedStr) << ".";
		throw CSerializationException(ssmsg.str());
	}
	
}

void CXmlFormatter::WriteFormatVersion()
{
	std::wostringstream versionWSS;
	versionWSS << CURRENT_VERSION_VALUE_INT;
	m_XmlWriter->WriteAttributeString( VERSION_ATTRIB_NAME,  versionWSS.str() );
}

void CXmlFormatter::WriteObjectDefinition(wstring const & elementName, 
										 CObject const * pObject, 
										 CObjectIDGenerator::ObjectID objID)
{
	// save current object version
	uint32 previousObjectVersion = GetCurrentObjectFormatVersion();
	CObject* previousObject = GetCurrentObject();

	ISerializable const * serializableObj = DynamicCast<ISerializable>(const_cast<CObject*>(pObject));
	if (serializableObj != NULL)
	{
		SetCurrentObjectAndFormatVersion( const_cast<CObject*>(pObject), serializableObj->GetObjectFormatVersion());

		// we need to first write out some type information about this object
		m_XmlWriter->WriteStartElement(elementName);
		CObjectTypeRef objectTypeRef(pObject, objID);
		WriteObjectTypeRef(objectTypeRef, false);

		// now we'll let the object inform us how to serialize it out
		serializableObj->GetObjectData(GetSerializationInfo(), GetSerializationContext());

		m_XmlWriter->WriteEndElement();
	}
	else
	{
		CSerializationSurrogate* pSurrogate = CSurrogateSelector::SelectSurrogateOrThrow(pObject);
		CScopedDeleteAndClear<CSerializationSurrogate> scopedDeleteSurrogate(pSurrogate);

		pSurrogate->SetInnerObject( const_cast<CObject*>(pObject) );

		SetCurrentObjectAndFormatVersion( const_cast<CObject*>(pObject), pSurrogate->GetObjectFormatVersion());

		// we need to first write out some type information about this object
		m_XmlWriter->WriteStartElement(elementName);
		
		CObjectTypeRef objectTypeRef(pObject, objID, pSurrogate);
		WriteObjectTypeRef(objectTypeRef, false);

		// now we'll let the object inform us how to serialize it out
		pSurrogate->GetObjectData(GetSerializationInfo(), GetSerializationContext());

		m_XmlWriter->WriteEndElement();
	}

	// restore old object version
	SetCurrentObjectAndFormatVersion( previousObject, previousObjectVersion );
}


// may throw SerializationFormatVersionError
void CXmlFormatter::ReadAndCheckFormatVersion()
{
	m_DeserializingFormatVersion = ASSUMED_VERSION_VALUE_INT;
	wstring versionStr;
	if (m_XmlReader->GetAttribute(VERSION_ATTRIB_NAME, versionStr))
	{
		m_DeserializingFormatVersion  = _wtoi(versionStr.c_str());
	}

	// verify that we can handle this version
	if (m_DeserializingFormatVersion > CURRENT_VERSION_VALUE_INT)
	{
		// Something is wrong. This must be an older formatter, and
		// we are trying to read in a format that was generated by a
		// newer formater. We will not be able to deserialize this stream.
		ostringstream ssmsg;
		ssmsg << "Cannot deserialize stream. Stream is format version " << m_DeserializingFormatVersion ;
		ssmsg << " but this formatter ("<< this->GetType()->GetFullName() <<") can only handle formats up to and including version ";
		ssmsg << CURRENT_VERSION_VALUE_INT;
		throw CSerializationFormatVersionError(ssmsg.str());
	}
}


void CXmlFormatter::SetSerializationInfoAndContext(CSerializationInfo* pSInfo, CSerializationContext* pSContext)
{
	m_pSInfo = pSInfo;
	m_pSContext = pSContext;
}

CSerializationInfo* CXmlFormatter::GetSerializationInfo()
{
	assert(m_pSInfo);
	return m_pSInfo;
}

CSerializationContext* CXmlFormatter::GetSerializationContext()
{
	assert(m_pSContext);
	return m_pSContext;
}



} // Serialization namespace
} // System namespace
} // WT namespace
