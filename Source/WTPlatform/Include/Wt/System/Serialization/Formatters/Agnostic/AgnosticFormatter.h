#pragma once

#include "WT/System/Serialization/Formatters/Formatter.h"
#include "WT/System/Serialization/Formatters/ObjectTypeRef.h"
#include "WT/System/Xml/XmlWriter.h"
#include "WT/System/Xml/XmlReader.h"
#include "WT/System/Io/Base64Stream.h"
#include "WT/System/Serialization/SerializationSurrogate.h"
#include "WT/System/Serialization/Formatters/ObjectFinalizer.h"

namespace WT
{
namespace System
{
namespace Serialization
{

using namespace WT::System::IO;

typedef SmartPointer<class CAgnosticFormatter> HAgnosticFormatter;
class CAgnosticFormatter : public CFormatter
{
public:
	WT_CLASS(CAgnosticFormatter, CFormatter);

	CAgnosticFormatter();
	virtual ~CAgnosticFormatter();

	// After calling Deserialize() this method may be called to determine which formatter was selected.
	// Note, ownership of the formatter is not passed to the caller, the formatter may be deleted
	// when AgnosticFormatter is deleted or is Deserialize() is called.
	virtual CFormatter* GetSelectedFormatter();

	// Use this method to force AgnosticFormatter to use a specific formatter when serializing objects.
	// If no serialization formatter is specified then AgnosticFormatter will use the same formatter which it selected
	// for the previous deserialization operation. 
	// Note: the caller is not passing ownership of this formatter to AgnosticFormatter.
	virtual void SetSerializationFormatter(CFormatter* pSerializationFormatter);

	// Formatter Methods

	// If no formater was selected during deserialize and no formatter was 
	// set by calling SetSerializationFormatter() then calls to Serialize() will throw an exception.
	virtual void Serialize ( IO::CStream* serializationStream , CObject* graph, CSerializationContext* pContext = NULL );

	virtual HObject Deserialize ( IO::CStream* serializationStream, CSerializationContext* pContext = NULL );

	// When overridden in a derived class, writes a Boolean value to the stream already attached to the formatter. 
	virtual void WriteBool(bool value, const wstring& name);

	// When overridden in a derived class, writes an 8-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt8(uint8 value, const wstring& name);

	// When overridden in a derived class, writes a DateTime value to the stream already attached to the formatter. 
	virtual void WriteDateTime(const CDateTime& value, const wstring& name);

	// When overridden in a derived class, writes an 8-bit signed integer to the stream already attached to the formatter. 
	virtual void WriteInt8(int8 value, const wstring& name);

	// When overridden in a derived class, writes a 16-bit signed integer to the stream already attached to the formatter. 
	virtual void WriteInt16(int16 value, const wstring& name);

	// When overridden in a derived class, writes a 32-bit signed integer to the stream. 
	virtual void WriteInt32(int32 value, const wstring& name);

	// When overridden in a derived class, writes a 64-bit signed integer to the stream. 
	virtual void WriteInt64(int64 value, const wstring& name);

	// When overridden in a derived class, writes a 32-bit float to the stream. 
	virtual void WriteFloat32(float32 value, const wstring& name);

	// When overridden in a derived class, writes a 64-bit float to the stream. 
	virtual void WriteFloat64(float64 value, const wstring& name);

	// When overridden in a derived class, writes a TimeSpan value to the stream already attached to the formatter. 
	virtual void WriteTimeSpan(const CTimeSpan& value, const wstring& name);

	// When overridden in a derived class, writes a 16-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt16(uint16 value, const wstring& name);

	// When overridden in a derived class, writes a 32-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt32(uint32 value, const wstring& name);

	// When overridden in a derived class, writes a 64-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt64(uint64 value, const wstring& name);

	// When overridden in a derived class, writes a string to the stream already attached to the formatter. 
	virtual void WriteString(const string& value, const wstring& name);

	// When overridden in a derived class, writes a wide string to the stream already attached to the formatter. 
	virtual void WriteWString(const wstring& value, const wstring& name);

	// When overridden in a derived class, writes an object reference to the stream already attached to the formatter. 
	virtual void WriteObjectByRef(CObject* obj, const wstring& name, const CType* type);

	virtual void WriteObjectByValue(CObject const & obj, const wstring& name, const CType* type);

	// These methods are not part of the .Net Serialization framework, but they seem necessary,
	// the alternative is to Derive a specialized instance of SerializationInfo for every Formatter

	virtual bool ReadBool(const wstring& name);
	virtual uint8 ReadUInt8(const wstring& name);
	virtual CDateTime ReadDateTime(const wstring& name);
	virtual int8 ReadInt8(const wstring& name);
	virtual int16 ReadInt16(const wstring& name);
	virtual int32 ReadInt32(const wstring& name);
	virtual int64 ReadInt64(const wstring& name);
	virtual float32 ReadFloat32(const wstring& name);
	virtual float64 ReadFloat64(const wstring& name);
	virtual CTimeSpan ReadTimeSpan(const wstring& name);
	virtual uint16 ReadUInt16(const wstring& name);
	virtual uint32 ReadUInt32(const wstring& name);
	virtual uint64 ReadUInt64(const wstring& name);
	virtual string ReadString(const wstring& name);
	virtual wstring ReadWString(const wstring& name);
	virtual HObject ReadObjectByRef(const wstring& name);
	virtual void ReadObjectByValue(const wstring& name, CObject& rObj);

	virtual CStream* BeginStreamBlock(const wstring& name);
	virtual void EndStreamBlock();

	virtual void WriteContainerBegin(const wstring& name);
	virtual void WriteContainerBeginWithCount(const wstring& name, uint32 count);
	virtual void WriteContainerEnd();

	virtual void ReadContainerBegin(const wstring& name);
	virtual uint32 ReadContainerBeginWithCount(const wstring& name);
	virtual void ReadContainerEnd();

	virtual bool IgnoresNames();

	virtual uint32 GetCurrentObjectFormatVersion() const;
	virtual void SetCurrentObjectFormatVersion( uint32 Version );

	virtual bool ObjectIsFinalized(CObject const * pObject) const;

	virtual CSerializationContext* GetSerializationContext();

private:
	CStream*	m_BaseStream;
	CFormatter*  m_pDeserializationFormatter;
	CFormatter*  m_pSerializationFormatter;
};





} // Serialization namespace
} // System namespace
} // WT namespace
