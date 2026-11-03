#pragma once

#include <list>
#include "WT/System/Object.h"
#include "WT/System/IO/Stream.h"
#include "WT/System/Serialization/ObjectIDGenerator.h"
#include "WT/System/DateTime.h"

namespace WT
{
namespace System
{
namespace Serialization
{

using namespace WT::System::IO;
class CSerializationInfo;
class CSerializationContext;

typedef SmartPointer<class CFormatter> HFormatter;
class CFormatter : public CObject
{
public:
	WT_ABSTRACT_CLASS(CFormatter, CObject)

	virtual void Serialize ( CStream* serializationStream , CObject* graph, CSerializationContext* pContext = NULL ) = 0;
	virtual HObject Deserialize (  CStream* serializationStream, CSerializationContext* pContext = NULL ) = 0;

	//  Returns the next object to serialize, from the formatter's internal work queue. 
	virtual HObject GetNext(CObjectIDGenerator::ObjectID* id );

	// Schedules an object for later serialization. 
	virtual CObjectIDGenerator::ObjectID Schedule(CObject* obj);

	virtual CSerializationContext* GetSerializationContext() = 0;

	// When overridden in a derived class, writes a Boolean value to the stream already attached to the formatter. 
	virtual void WriteBool(bool value, const wstring& name) = 0;

	// When overridden in a derived class, writes an 8-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt8(uint8 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a DateTime value to the stream already attached to the formatter. 
	virtual void WriteDateTime(const CDateTime& value, const wstring& name) = 0;

	// When overridden in a derived class, writes an 8-bit signed integer to the stream already attached to the formatter. 
	virtual void WriteInt8(int8 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 16-bit signed integer to the stream already attached to the formatter. 
	virtual void WriteInt16(int16 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 32-bit signed integer to the stream. 
	virtual void WriteInt32(int32 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 64-bit signed integer to the stream. 
	virtual void WriteInt64(int64 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 32-bit float to the stream. 
	virtual void WriteFloat32(float32 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 64-bit float to the stream. 
	virtual void WriteFloat64(float64 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a TimeSpan value to the stream already attached to the formatter. 
	virtual void WriteTimeSpan(const CTimeSpan& value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 16-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt16(uint16 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 32-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt32(uint32 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a 64-bit unsigned integer to the stream already attached to the formatter. 
	virtual void WriteUInt64(uint64 value, const wstring& name) = 0;

	// When overridden in a derived class, writes a string to the stream already attached to the formatter. 
	virtual void WriteString(const string& value, const wstring& name) = 0;

	// When overridden in a derived class, writes a wide string to the stream already attached to the formatter. 
	virtual void WriteWString(const wstring& value, const wstring& name) = 0;

	// When overridden in a derived class, writes an object reference to the stream already attached to the formatter. 
	virtual void WriteObjectByRef(CObject* obj, const wstring& name, const CType* type) = 0;

	virtual void WriteObjectByValue(CObject const & obj, const wstring& name, const CType* type) = 0;


	// These methods are not part of the .Net Serialization framework, but they seem necessary,
	// the alternative is to Derive a specialized instance of SerializationInfo for every Formatter

	virtual bool ReadBool(const wstring& name) = 0;
	virtual uint8 ReadUInt8(const wstring& name) = 0;
	virtual CDateTime ReadDateTime(const wstring& name) = 0;
	virtual int8 ReadInt8(const wstring& name) = 0;
	virtual int16 ReadInt16(const wstring& name) = 0;
	virtual int32 ReadInt32(const wstring& name) = 0;
	virtual int64 ReadInt64(const wstring& name) = 0;
	virtual float32 ReadFloat32(const wstring& name) = 0;
	virtual float64 ReadFloat64(const wstring& name) = 0;
	virtual CTimeSpan ReadTimeSpan(const wstring& name) = 0;
	virtual uint16 ReadUInt16(const wstring& name) = 0;
	virtual uint32 ReadUInt32(const wstring& name) = 0;
	virtual uint64 ReadUInt64(const wstring& name) = 0;
	virtual string ReadString(const wstring& name) = 0;
	virtual wstring ReadWString(const wstring& name) = 0;
	virtual HObject ReadObjectByRef(const wstring& name) = 0;
	virtual void ReadObjectByValue(const wstring& name, CObject& rObj) = 0;

	// These are block operations that work with formatters.
	// They are used to create containers and struct type datatypes
	// and open ended custom stream data

	virtual CStream* BeginStreamBlock(const wstring& name) = 0;
	virtual void EndStreamBlock() = 0;

	virtual void WriteContainerBegin(const wstring& name) = 0;
	virtual void WriteContainerBeginWithCount(const wstring& name, uint32 count) = 0;
	virtual void WriteContainerEnd() = 0;

	virtual void ReadContainerBegin(const wstring& name) = 0;
	virtual uint32 ReadContainerBeginWithCount(const wstring& name) = 0;
	virtual void ReadContainerEnd() = 0;


	// This is not part of the .NET API, but it is an efficeny enhancment for
	// WTPlatform. If a formater ignores the name arguments to the various SetValue() 
	// and Read*() methods then this will return true.
	//
	// This allows more efficent implmentation of template specializations in SerializationInfo's
	// 	 void AddValue(const string& name, const T& value);
	// and
	//   void GetValue(const string& name, T& fillValue );
	// methods because they do not have to perform string concatentations to create unique names
	// for sub-elements of special types if those names will be tossed away by the formatter anyway.
	virtual bool IgnoresNames() = 0;

	virtual uint32 GetCurrentObjectFormatVersion() const = 0;
	virtual void SetCurrentObjectFormatVersion( uint32 FormatVersion ) = 0;

	virtual bool ObjectIsFinalized(CObject const * pObject) const = 0;

protected: // data
	// Contains the ObjectIDGenerator used with the current formatter. 
	CObjectIDGenerator m_IdGenerator;

	// Contains a Queue of the objects left to serialize. 
	std::list<HObject> m_ObjectQueue; // used as a queue: push to the back and pop from the front.

	// SerializationInfo needs to have access to these methods of Formatter objects:
	friend class WT::System::Serialization::CSerializationInfo;

	// When overridden in a derived class, writes a value of the given type to the stream already attached to the formatter. 
//TODO:	virtual void WriteValueType(const Object* obj, string name, const Type* type) = 0;
	
    // TODO: this assumes boxing exists	
	// Inspects the type of data received, and calls the appropriate Write method to perform the write to the stream already attached to the formatter. 
	// virtual void WriteMember(string name, Object* obj) = 0;
};

} // Serialization namespace
} // System namespace
} // WT namespace

#include "WT/System/Serialization/SerializationContext.h"
