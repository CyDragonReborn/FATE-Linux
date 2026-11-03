/***************************************************************************************
 *
 * PointHelper.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          10/27/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Object.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Graphics/IMovable.h"
#include "WT/System/UserPropertyCollection.h"
#include "WT/System/IPropertied.h"

namespace WT
{
namespace Graphics
{

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/*
A CPointHelper consists of a transform in space and user-defined properties. When 
exported from 3ds max, point helpers become CPointHelper objects. A point helper's 
user-defined properties are parsed for specifications of the form "key=value". The 
key must not contain spaces, tabs, or the equal sign character. Each such 
specification establishes a CExplicitTypedProperty<std::string> property named 
based on the key and with the specified value.
*/
typedef System::SmartPointer<class CPointHelper> HPointHelper;
class CPointHelper : public System::CObject, public CMovableImpl, 
	public System::Serialization::ISerializable, public System::IPropertied
{
public:
	WT_BEGIN_CLASS( CPointHelper, System::CObject );
		WT_INHERITS_INTERFACE( IMovable );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( System::IPropertied );

		WT_BEGIN_PROPERTIES();
			WT_PROP_RW_BYREF( Name, string );
		WT_END_PROPERTIES();

	WT_END_CLASS();

	CPointHelper();

	std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	//
	// ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

	//
	// IPropertied interface
	//
	System::IPropertyCollection& GetProperties() {return *m_hProperties;}
	System::IPropertyCollection const& GetProperties() const {return *m_hProperties;}

private:
	std::string m_Name;
	System::HUserPropertyCollection m_hProperties;
};

}

template<> inline void WTCall WT::Using<Graphics::CPointHelper>()
{
	Graphics::CPointHelper::GetClassType();
	WT::Using<System::CExplicitTypedProperty<string> >();
}

}
