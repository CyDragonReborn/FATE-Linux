/*
	Skeleton.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	4/5/2005
*/

#include "WT/stdafx.h"
#include "WT/Graphics/Animation/Skeleton.h"

using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{
namespace Animation
{

WT_DEFINE_CLASS(CSkeleton)

/*virtual*/ 
void CSkeleton::GetObjectData(CSerializationInfo* Info, CSerializationContext* /*sContext*/) const
{
//	Info->AddValue( "Bones", static_cast<std::vector<CBone>&>( m_Bones ) );

	Info->AddContainerBeginWithCount( "Bones", static_cast<uint32>( m_Bones.size() ) );
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		Info->AddValue( "item", m_Bones[i] );
	}
	Info->AddContainerEnd();
}

/*virtual*/ 
void CSkeleton::SetObjectData(CSerializationInfo* Info, CSerializationContext* /*sContext*/)
{
//	Info->GetValue( "Bones", static_cast<std::vector<CBone>&>( m_Bones ));

	uint32 NumBones = Info->GetContainerBeginWithCount( "Bones" );
	m_Bones.resize( NumBones );
	for (size_t i = 0; i < m_Bones.size(); ++i)
	{
		Info->GetValue( "item", m_Bones[i] );
	}
	Info->GetContainerEnd();
}

} // Animation
} // Graphics
} // WT
