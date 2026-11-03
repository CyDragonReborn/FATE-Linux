#include "WT/stdafx.h"
#include "WT/System/Serialization/PropertySerializationSurrogate.h"

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CPropertySerializationSurrogate)


CPropertySerializationSurrogate::CPropertySerializationSurrogate()
{
}

CPropertySerializationSurrogate::CPropertySerializationSurrogate( CObject* pInnerObject ) :
	CSerializationSurrogate( pInnerObject )
{
}

CPropertySerializationSurrogate::~CPropertySerializationSurrogate()
{
}

// ISerializable
void CPropertySerializationSurrogate::GetObjectData(CSerializationInfo* SerialInfo, CSerializationContext* sContext) const
{
	assert(m_hObject != NULL);
	CPropertyCollection* pPropCollection = m_hObject->GetType()->GetProperties();
	assert(pPropCollection);
	pPropCollection->SerializePropertyValues(m_hObject, SerialInfo, sContext);
}

void CPropertySerializationSurrogate::SetObjectData(CSerializationInfo* SerialInfo, CSerializationContext* sContext)
{
	assert(m_hObject != NULL);
	CPropertyCollection* pPropCollection = m_hObject->GetType()->GetProperties();
	assert(pPropCollection);
	pPropCollection->DeserializePropertyValues(m_hObject, SerialInfo, sContext);
}

void CPropertySerializationSurrogate::GetSupportedVersionRange( uint32 &MinVersion, uint32 &MaxVersion ) const
{
	MinVersion = 1;
	MaxVersion = 1;
}




} // Serialization namespace
} // System namespace
} // WT namespace