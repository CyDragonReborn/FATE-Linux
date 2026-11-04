#include "WT/stdafx.h"
#include "WT/System/Serialization/SerializationSurrogate.h"

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CSerializationSurrogate)

CSerializationSurrogate::CSerializationSurrogate(CObject* pObj) :
	m_hObject(pObj)
{
}

CSerializationSurrogate::~CSerializationSurrogate()
{
}

// SerializationSurrogate 

// the object to deserialize into.
// ownership is not passed to SerializationSurrogate
void CSerializationSurrogate::SetInnerObject(CObject* pObject)
{
	assert(pObject);
	m_hObject = pObject;
}

// returns the Object that will be/has been deserialized into
CObject* CSerializationSurrogate::GetInnerObject()
{
	assert(m_hObject != NULL);
	return m_hObject;
}


} // Serialization namespace
} // System namespace
} // WT namespace