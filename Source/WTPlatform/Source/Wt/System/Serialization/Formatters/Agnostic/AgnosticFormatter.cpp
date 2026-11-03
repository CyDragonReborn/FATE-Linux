#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/Agnostic/AgnosticFormatter.h"
#include "WT/System/Serialization/Formatters/Binary/BinaryFormatter.h"
#include "WT/System/Serialization/Formatters/Xml/XmlFormatter.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/ScopedClasses.h"
#include "WT/System/Serialization/SerializationException.h"




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

WT_DEFINE_CLASS(CAgnosticFormatter);


CAgnosticFormatter::CAgnosticFormatter() : 
	m_pDeserializationFormatter(NULL), 
	m_pSerializationFormatter(NULL)
{
}

CAgnosticFormatter::~CAgnosticFormatter()
{
	delete m_pDeserializationFormatter;
	m_pDeserializationFormatter = NULL;
	m_pSerializationFormatter = NULL;
}

CFormatter* CAgnosticFormatter::GetSelectedFormatter()
{
	return m_pDeserializationFormatter;
}

void CAgnosticFormatter::SetSerializationFormatter(CFormatter* pSerializationFormatter)
{
	m_pSerializationFormatter = pSerializationFormatter;
}


void CAgnosticFormatter::Serialize ( IO::CStream* serializationStream , CObject* graph, CSerializationContext* pContext )
{
	if (m_pSerializationFormatter)
	{
		m_pSerializationFormatter->Serialize( serializationStream , graph, pContext );
	}
	else if (m_pDeserializationFormatter)
	{
		m_pDeserializationFormatter->Serialize( serializationStream , graph, pContext );
	}
	else
	{
		throw CSerializationException("No Serialization Formatter specified. AgnosticFormatter could not serialize.");
	}	
}

HObject CAgnosticFormatter::Deserialize ( IO::CStream* serializationStream, CSerializationContext* pContext )
{
	if (m_pDeserializationFormatter)
	{
		delete m_pDeserializationFormatter;
	}
	m_pDeserializationFormatter = NULL;

	if (!serializationStream->CanSeek())
	{
		throw CSerializationException("CAgnosticFormatter::Deserialize(): should only be passed seekable streams.");
	}

	int32 pos = serializationStream->GetPosition();
	uint8 headerBuf[5];
	int32 count = serializationStream->Read(headerBuf, sizeof(headerBuf));

	// reset to the original stream position

	serializationStream->SetPosition(pos);
	if (count == sizeof(headerBuf) && memcmp(headerBuf, "WTBIN", 5) == 0)
	{		
		m_pDeserializationFormatter = new CBinaryFormatter();
	}
	else
	{
		// this is not a binary file, assume it is xml
		m_pDeserializationFormatter = new CXmlFormatter();
	}

	return m_pDeserializationFormatter->Deserialize(serializationStream, pContext);
}

	// When overridden in a derived class, writes a Boolean value to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteBool(bool value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteBool(value, name);
}

	// When overridden in a derived class, writes an 8-bit unsigned integer to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteUInt8(uint8 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteUInt8(value, name);
}


	// When overridden in a derived class, writes a DateTime value to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteDateTime(const CDateTime& value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteDateTime(value, name);
}


	// When overridden in a derived class, writes an 8-bit signed integer to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteInt8(int8 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteInt8(value, name);
}


	// When overridden in a derived class, writes a 16-bit signed integer to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteInt16(int16 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteInt16(value, name);
}


	// When overridden in a derived class, writes a 32-bit signed integer to the stream. 
void CAgnosticFormatter::WriteInt32(int32 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteInt32(value, name);
}


	// When overridden in a derived class, writes a 64-bit signed integer to the stream. 
void CAgnosticFormatter::WriteInt64(int64 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteInt64(value, name);
}


	// When overridden in a derived class, writes a 32-bit float to the stream. 
void CAgnosticFormatter::WriteFloat32(float32 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteFloat32(value, name);
}


	// When overridden in a derived class, writes a 64-bit float to the stream. 
void CAgnosticFormatter::WriteFloat64(float64 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteFloat64(value, name);
}


	// When overridden in a derived class, writes a TimeSpan value to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteTimeSpan(const CTimeSpan& value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteTimeSpan(value, name);
}


	// When overridden in a derived class, writes a 16-bit unsigned integer to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteUInt16(uint16 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteUInt16(value, name);
}


	// When overridden in a derived class, writes a 32-bit unsigned integer to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteUInt32(uint32 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteUInt32(value, name);
}


	// When overridden in a derived class, writes a 64-bit unsigned integer to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteUInt64(uint64 value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteUInt64(value, name);
}


	// When overridden in a derived class, writes a string to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteString(const string& value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteString(value, name);
}


	// When overridden in a derived class, writes a wide string to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteWString(const wstring& value, const wstring& name)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteWString(value, name);
}


	// When overridden in a derived class, writes an object reference to the stream already attached to the formatter. 
void CAgnosticFormatter::WriteObjectByRef(CObject* obj, const wstring& name, const CType* type)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteObjectByRef(obj, name, type);
}


void CAgnosticFormatter::WriteObjectByValue(CObject const & obj, const wstring& name, const CType* type)
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->WriteObjectByValue(obj, name, type);
}



bool CAgnosticFormatter::ReadBool(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadBool(name);
}

uint8 CAgnosticFormatter::ReadUInt8(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadUInt8(name);
}

CDateTime CAgnosticFormatter::ReadDateTime(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadDateTime(name);
}

int8 CAgnosticFormatter::ReadInt8(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadInt8(name);
}

int16 CAgnosticFormatter::ReadInt16(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadInt16(name);
}

int32 CAgnosticFormatter::ReadInt32(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadInt32(name);
}

int64 CAgnosticFormatter::ReadInt64(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadInt64(name);
}

float32 CAgnosticFormatter::ReadFloat32(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadFloat32(name);
}

float64 CAgnosticFormatter::ReadFloat64(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadFloat64(name);
}

CTimeSpan CAgnosticFormatter::ReadTimeSpan(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadTimeSpan(name);
}

uint16 CAgnosticFormatter::ReadUInt16(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadUInt16(name);
}

uint32 CAgnosticFormatter::ReadUInt32(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadUInt32(name);
}

uint64 CAgnosticFormatter::ReadUInt64(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadUInt64(name);
}

string CAgnosticFormatter::ReadString(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadString(name);
}

wstring CAgnosticFormatter::ReadWString(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadWString(name);
}

HObject CAgnosticFormatter::ReadObjectByRef(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadObjectByRef(name);
}

void CAgnosticFormatter::ReadObjectByValue(const wstring& name, CObject& rObj)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadObjectByValue(name, rObj);
}


CStream* CAgnosticFormatter::BeginStreamBlock(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->BeginStreamBlock(name);
}

void CAgnosticFormatter::EndStreamBlock()
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->EndStreamBlock();
}


void CAgnosticFormatter::WriteContainerBegin(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->WriteContainerBegin(name);
}

void CAgnosticFormatter::WriteContainerBeginWithCount(const wstring& name, uint32 count)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->WriteContainerBeginWithCount(name, count);
}

void CAgnosticFormatter::WriteContainerEnd()
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->WriteContainerEnd();
}

void CAgnosticFormatter::ReadContainerBegin(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadContainerBegin(name);
}

uint32 CAgnosticFormatter::ReadContainerBeginWithCount(const wstring& name)
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadContainerBeginWithCount(name);
}

void CAgnosticFormatter::ReadContainerEnd()
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ReadContainerEnd();
}


bool CAgnosticFormatter::IgnoresNames()
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->IgnoresNames();
}

uint32 CAgnosticFormatter::GetCurrentObjectFormatVersion() const
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->GetCurrentObjectFormatVersion();
}

void CAgnosticFormatter::SetCurrentObjectFormatVersion( uint32 Version )
{
	assert(m_pDeserializationFormatter);
	m_pDeserializationFormatter->SetCurrentObjectFormatVersion( Version );
}

bool CAgnosticFormatter::ObjectIsFinalized(CObject const * pObject) const
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->ObjectIsFinalized(pObject);
}

CSerializationContext* CAgnosticFormatter::GetSerializationContext()
{
	assert(m_pDeserializationFormatter);
	return m_pDeserializationFormatter->GetSerializationContext();
}

} // Serialization namespace
} // System namespace
} // WT namespace
