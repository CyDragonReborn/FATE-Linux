#include "WT/stdafx.h"

#include "WT/System/Serialization/SerializationException.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/PropertyCollection.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{

using namespace IO;
using namespace Serialization;

WT_DEFINE_CLASS(CPropertyCollection)

CPropertyCollection::~CPropertyCollection()
{
}

bool CPropertyCollection::Add ( const string& Name, CProperty* Prop )
{
	return Items.insert ( std::make_pair( Name, Prop ) ).second;
}

void CPropertyCollection::Set( const string& Name, CProperty* Prop )
{
	PropertyMap::iterator it = Items.find ( Name );

	if (it == Items.end())
	{
		Add( Name, Prop );
		return;
	}

	it->second = Prop;
}

bool CPropertyCollection::Erase( const string& Name )
{
	PropertyMap::iterator it = Items.find ( Name );

	if (it == Items.end())
	{
		return false;
	}

	Items.erase( it );
	return true;
}

bool CPropertyCollection::Exists( const string& Name ) const
{
	return Items.find( Name ) != Items.end();
}

CProperty* CPropertyCollection::GetItem ( const string& Name )
{
	PropertyMap::iterator Prop = Items.find ( Name );
	if ( Prop != Items.end() )
		return Prop->second;
	else
		return NULL;
}

CProperty const* CPropertyCollection::GetItem ( const string& Name ) const
{
	PropertyMap::const_iterator Prop = Items.find ( Name );
	if ( Prop != Items.end() )
		return Prop->second;
	else
		return NULL;
}

// serialize out all read/write properties (skip readonly or write only)
void CPropertyCollection::SerializePropertyValues(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* sContext) const
{
	// count how many read/write properties are here
	uint32 numReadWriteProps = 0;
	PropertyMap::const_iterator i = Items.begin();
	for(; i != Items.end(); i++)
	{
		if (i->second->IsWriteable() && i->second->IsReadable())
		{
			numReadWriteProps++;
		}
	}

	SerialInfo->AddContainerBeginWithCount("properties", numReadWriteProps);

	for(i = Items.begin(); i != Items.end(); i++)
	{
		CProperty* pProp = i->second;
		if (pProp->IsWriteable() && pProp->IsReadable())
		{
			SerialInfo->AddContainerBegin("property");
			SerialInfo->AddValue("name", i->first);

			ISerializable* pSerializable = DynamicCast<ISerializable>( pProp );
			if (pSerializable == NULL)
			{
				SerialInfo->AddValue( "serializable", false );
				pProp->SerializePropertyValue(Instance, SerialInfo, sContext);
			} else {
				SerialInfo->AddValue( "serializable", true );
				SerialInfo->AddValue( "item", pProp );
			}
			SerialInfo->AddContainerEnd();
		}
	}

	SerialInfo->AddContainerEnd();
}

// derialize in all read/write properties (skip readonly or write only)
void CPropertyCollection::DeserializePropertyValues(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* sContext)
{
	string propName;
	int32 count = SerialInfo->GetContainerBeginWithCount("properties");
	for (int32 i = 0; i < count; i++)
	{
		SerialInfo->GetContainerBegin("property");
		SerialInfo->GetValue("name", propName);
		bool bSerializable;
		SerialInfo->GetValue("serializable", bSerializable );

		if (!bSerializable)
		{
			PropertyMap::iterator propIter = Items.find(propName);
			if (propIter != Items.end())
			{
				if ( propIter->second->IsWriteable() )
				{
					propIter->second->DeserializePropertyValue(Instance, SerialInfo, sContext);
				}
				else
				{
					std::ostringstream ssmsg;
					ssmsg << "Cannot deserialize property (name: " << propName << ")";
					ssmsg << " on class \"" << Instance->GetType()->GetFullName() << "\",";
					ssmsg << " property is read-only, but is in the stream being deserialized from.";
					ssmsg << " Was it changed from a read/write property to a read-only property?";
					throw Serialization::CSerializationException( ssmsg.str() );
				}
			}
			else
			{
				// this property does not exist or no longer exists on this object.
				throw Serialization::CSerializationException("Property no longer exists: " + propName);
			}
		} else {
			CProperty* pProp = NULL;
			SerialInfo->GetValue( "item", pProp );
			Set( propName, pProp );
		}

		SerialInfo->GetContainerEnd();
	}

	SerialInfo->GetContainerEnd();
}


}
}