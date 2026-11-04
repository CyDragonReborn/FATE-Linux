#pragma once

#include "WT/System/Object.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/System/Serialization/SerializationContext.h"
#include "WT/System/Property.h"
#include <map>

namespace WT
{
namespace System
{

typedef SmartPointer<class CProperty> HProperty;
class CProperty;

namespace Serialization
{
	class CSerializationContext;
	class CSerializationInfo;
}

using namespace WT::System::Serialization;

//
// An interface to a collection of properties.
//
class IPropertyCollection : public IInterface
{
public:
	WT_INTERFACE( IPropertyCollection )

	virtual bool Add ( const string& Name, CProperty* Prop ) = 0;
	virtual void Set ( const string& Name, CProperty* Prop ) = 0;
	virtual bool Erase( const string& Name ) = 0;
	virtual bool Exists( const string& Name ) const = 0;
	virtual CProperty* GetItem ( const string& Name ) = 0;
	virtual CProperty const* GetItem ( const string& Name ) const = 0;
};

//
// A collection of properties.
//
typedef SmartPointer<class CPropertyCollection> HPropertyCollection;
class CPropertyCollection : public CObject, public IPropertyCollection
{
public:
	WT_BEGIN_CLASS( CPropertyCollection, CObject )
		WT_INHERITS_INTERFACE( IPropertyCollection )
	WT_END_CLASS()

	virtual ~CPropertyCollection();
	virtual bool Add ( const string& Name, CProperty* Prop );
	virtual void Set ( const string& Name, CProperty* Prop );
	virtual bool Erase( const string& Name );
	virtual bool Exists( const string& Name ) const;
	virtual CProperty* GetItem ( const string& Name );
	virtual CProperty const* GetItem ( const string& Name ) const;

	typedef std::map<string,HProperty> PropertyMap;
	PropertyMap& GetProperties() {return Items;}
	PropertyMap const& GetProperties() const {return Items;}

	//
	// Serialization methods - these require an object instance so that the properties' values can be 
	// retrieved or set.
	//
	void SerializePropertyValues(CObject const* Instance, CSerializationInfo* SerialInfo, CSerializationContext* sContext) const;
	void DeserializePropertyValues(CObject* Instance, CSerializationInfo* SerialInfo, CSerializationContext* sContext);

private:
	PropertyMap Items;
};

}
}

#include "WT/System/Property.h"
