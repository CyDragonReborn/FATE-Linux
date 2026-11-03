//
//	Line.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Objects/Line.h"
#include "WT/Math/MathSerialization.h"


namespace WT
{
namespace Math
{

WT_DEFINE_CLASS( CLine )


// ISerializable

void CLine::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						  System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "StartPoint", StartPoint );
	pInfo->AddValue( "EndPoint", EndPoint );
}

void CLine::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						  System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "StartPoint", StartPoint );
	pInfo->GetValue( "EndPoint", EndPoint );
}


} // namespace Math
} // namespace WT
