//
//	Point3d.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Objects/Point3d.h"
#include "WT/Math/MathSerialization.h"


namespace WT
{
namespace Math
{

WT_DEFINE_CLASS(CPoint3d)

// ISerializable

void CPoint3d::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
							 System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "Center", Center );
}

void CPoint3d::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
							 System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "Center", Center );
}


} // namespace Math
} // namespace WT
