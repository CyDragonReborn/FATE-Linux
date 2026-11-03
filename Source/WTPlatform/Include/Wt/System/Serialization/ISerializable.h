//
//	ISerializable.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/System/Object.h"
#include "WT/System/Platform.h"
#include "WT/System/IInterface.h"

namespace WT
{
namespace System
{
namespace Serialization
{

class CSerializationInfo;
class CSerializationContext;

namespace FinalizeStatus
{
	enum Type
	{
		NotFinalized = 0,
		InProgress,
		Finalized,
	};
}


class ISerializable :
		public IInterface
{
public:
	WT_BEGIN_INTERFACE( ISerializable );
	  WT_INHERITS_INTERFACE(IInterface);
    WT_END_INTERFACE()

	/*
	To make your objects serializable, you have to implement two methods:
	GetObjectData and SetObjectData -- where GetObjectData is the
	'write to the file' and 'SetObjectData' is 'read in from
	the file'.  The actual reading and writing is handled by
	formatters; a sample of writing out an XML file is:

	<pre>
			const char* const Filename = "c:\\foo.xml";
			WT::System::IO::CFileStream outf (Filename, WT::System::IO::FileMode::OpenOrCreate);
			WT::System::Serialization::CXmlFormatter _XmlFormatter;
			_XmlFormatter.Serialize ( &outf, &Foo );
	</pre>

	<br/>
	And now here's a sample of GetObjectData:
	<pre>
	void CMyObject::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const
	{
		pInfo->AddValue ("Language", m_Language);
		pInfo->AddValue ("Value", m_Value);
	}
	</pre>
	*/


	virtual void GetObjectData(CSerializationInfo* sInfo, CSerializationContext* sContext) const = 0;

	/*
	Here's an example of using SetObjectData (a.k.a. 'Read data from file')
	<pre>
	void CMyObject::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext )
	{
		pInfo->GetValue ("Language", m_Language);
		pInfo->GetValue ("Value", m_Value);
	}
	</pre>

	<p/>
	This is different from .Net. .Net forces serializable object to implement a constructor
	with this signature. We can't do this in C++ because it would require reflection to 
	locate the constructor.
	*/
	virtual void SetObjectData(CSerializationInfo* sInfo, CSerializationContext* sContext) = 0;	

	// redefine these in derived objects if your object's data fields change
	static uint32 WTCall GetClassFormatVersion() {return 1;}
	virtual uint32 GetObjectFormatVersion( ) const {return GetClassFormatVersion();}

	// This method will be called on each object after every object in the object graph has had SetObjectData()
	// called on it. 
	//
	// FinalizeDeserialization() is used so that object which need to initalize themselves by refering to
	// object reference can do that in this method. Object references are not valid in SetObjectData().
	//
	// referencesFinalizeStatus indicates whether or not it was possible for the serialization system
	// to finalize all of this object's object references prior to finalizing the object.
	// If all references were finalized then referencesFinalizeStatus == FinalizeStatus::Finalized.
	// In some cases where there are circular object references there is no possible linear serialization of 
	// references. In this case referencesFinalizeStatus == FinalizeStatus::InProgess
	//
	// Note that, when a derived object's SetObjectData() calls CSerializationInfo::GetBaseClass()
	// to deserialize base class information, only one call to FinalizeDeserialization will be issued for 
	// the derived object. Typically, when you implement FinalizeDeserialization, you should call
	// _BaseClass::FinalizeDeserialization().
	//
	virtual void FinalizeDeserialization(CSerializationInfo* sInfo, CSerializationContext* sContext)
	{
		sInfo;
		sContext;
	}
};

}
}
}

#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/Serialization/SerializationContext.h"
