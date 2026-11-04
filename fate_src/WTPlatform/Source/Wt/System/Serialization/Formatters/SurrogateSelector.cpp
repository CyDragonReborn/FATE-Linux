#include "WT/stdafx.h"

#include "WT/System/Serialization/Formatters/SurrogateSelector.h"
#include "WT/System/Serialization/SerializationException.h"
#include "WT/System/Serialization/PropertySerializationSurrogate.h"

#include <iostream>
#include <sstream>

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CSurrogateSelector)


// caller owns returned object
/*static*/ CSerializationSurrogate* CSurrogateSelector::SelectSurrogateOrThrow(CObject const * pObjectToSerialize)
{
	// The object does not implement ISerializable,
	// if this object has properties then we will serialize those out using the PropertySerializer
	// otherwise we'll throw a serialization exception.
	if ( !pObjectToSerialize->GetType()->HasProperties() )
	{
		throw CSerializationException( "Object does not implement ISerializable and does not have properties. Cannot Serialize object. Object Type: " + pObjectToSerialize->GetType()->GetFullName() );
	}

	return new CPropertySerializationSurrogate();
}



CSurrogateSelector::CSurrogateSelector() 
{
}


} // Serialization namespace
} // System namespace
} // WT namespace