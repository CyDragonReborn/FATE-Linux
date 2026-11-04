#include "WT/stdafx.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/Serialization/Formatters/Formatter.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationException.h"


namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CSerializationInfo)

CSerializationInfo::CSerializationInfo(CFormatter* FormatterObj) : 
	m_Formatter(FormatterObj)
{
}

CSerializationInfo::CSerializationInfo() : 
	m_Formatter(NULL)
{
	assert(m_Formatter != NULL);
}

CSerializationInfo::~CSerializationInfo()
{
}

uint32 CSerializationInfo::GetObjectFormatVersion() const
{
	return m_Formatter->GetCurrentObjectFormatVersion();
}

bool CSerializationInfo::ObjectIsFinalized(CObject const * pObject) const
{
	return m_Formatter->ObjectIsFinalized(pObject);
}


CStream* CSerializationInfo::BeginStreamBlock(const wstring& name)
{
	return m_Formatter->BeginStreamBlock(name);
}

CStream* CSerializationInfo::BeginStreamBlock(const string& name)
{
	return BeginStreamBlock(WT::AnsiToWide(name));
}

void CSerializationInfo::EndStreamBlock()
{
	m_Formatter->EndStreamBlock( );
}

void CSerializationInfo::AddContainerBeginWithCount(const wstring& name, uint32 count)
{
	return m_Formatter->WriteContainerBeginWithCount( name, count );
}

void CSerializationInfo::AddContainerBeginWithCount(const string& name, uint32 count)
{
	AddContainerBeginWithCount(WT::AnsiToWide(name), count);
}

void CSerializationInfo::AddContainerBegin(const wstring& name)
{
	return m_Formatter->WriteContainerBegin( name );
}

void CSerializationInfo::AddContainerBegin(const string& name)
{
	AddContainerBegin( WT::AnsiToWide(name) );
}

void CSerializationInfo::AddContainerEnd()
{
	return m_Formatter->WriteContainerEnd( );
}

void CSerializationInfo::GetContainerBegin(const wstring& name)
{
	m_Formatter->ReadContainerBegin( name );
}

void CSerializationInfo::GetContainerBegin(const string& name)
{
	GetContainerBegin( WT::AnsiToWide(name) );
}

uint32 CSerializationInfo::GetContainerBeginWithCount(const wstring& name)
{
	return m_Formatter->ReadContainerBeginWithCount( name );
}

uint32 CSerializationInfo::GetContainerBeginWithCount(const string& name)
{
	return GetContainerBeginWithCount( WT::AnsiToWide(name) );
}

void CSerializationInfo::GetContainerEnd()
{
	m_Formatter->ReadContainerEnd( );
}

//TODO: these value writters should reside in the formatters, right?
void CSerializationInfo::SerializeValueType(wstring const & name, CObject const * value)
{
	ISerializable const * pSerial = DynamicCast<ISerializable>(const_cast<CObject*>(value));

	if (pSerial == NULL)
	{
		throw CSerializationException("CSerializationInfo::SerializeValueType() passed an object value that does not implement ISerializable");
	}

	pSerial->GetObjectData(this, m_Formatter->GetSerializationContext() );
}

//TODO: these value writters should reside in the formatters, right?
void CSerializationInfo::DeserializeValueType(wstring const & name, CObject* value)
{
	ISerializable* pSerial = DynamicCast<ISerializable>(value);

	if (pSerial == NULL)
	{
		throw CSerializationException("CSerializationInfo::DeserializeValueType() passed an object value that does not implement ISerializable");
	}

	pSerial->SetObjectData(this, m_Formatter->GetSerializationContext() );
}


} // Serialization namespace
} // System namespace
} // WT namespace