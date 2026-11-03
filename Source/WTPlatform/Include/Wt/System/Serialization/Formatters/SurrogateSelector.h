#pragma once

#include "WT/System/Object.h"
#include "WT/System/Serialization/ObjectIDGenerator.h"
#include "WT/System/Serialization/SerializationSurrogate.h"

namespace WT
{
namespace System
{
namespace Serialization
{

// This is a helper class. This same code was in both the BinaryFormatter and the XmlFormatter
// This is a way to centralize the selection of SerializationSurrogates. 
//
// Right now all that SelectSurrogateOrThrow() does is look to see if the object
// has properties, if it does then it will construct an return an instance 
// of PropertySerializatonSurrogate.
typedef SmartPointer<class CSurrogateSelector> HSurrogateSelector;
class CSurrogateSelector : 
	public CObject
{
public:
	WT_CLASS(CSurrogateSelector, CObject);

	// caller owns returned object
	static CSerializationSurrogate* SelectSurrogateOrThrow(CObject const * pObjectToSerialize);

protected:
	CSurrogateSelector();

private:
};


} // Serialization namespace
} // System namespace
} // WT namespace

