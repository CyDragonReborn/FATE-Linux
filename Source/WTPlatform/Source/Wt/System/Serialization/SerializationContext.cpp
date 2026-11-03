#include "WT/stdafx.h"
#include "WT/System/Serialization/SerializationContext.h"
#include "WT/System/UserPropertyCollection.h"

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DEFINE_CLASS(CSerializationContext)

CSerializationContext::CSerializationContext()
{
	m_pContextProperties = new CUserPropertyCollection();
}

CSerializationContext::~CSerializationContext()
{
	delete m_pContextProperties;
}

} // Serialization namespace
} // System namespace
} // WT namespace