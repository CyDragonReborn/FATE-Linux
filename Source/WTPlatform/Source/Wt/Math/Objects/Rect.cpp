//
//	Rect.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Objects/Rect.h"
#include "WT/Math/MathSerialization.h"

using namespace WT::Math;


namespace WT
{
namespace Math
{


bool __fastcall CRect::MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	//TODO: fill this function
	return false;
}


// ISerializable

void CRect::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						  System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "Origin", Origin );
	pInfo->AddValue( "Width", Width );
	pInfo->AddValue( "Height", Height );
	pInfo->AddValue( "Normal", Normal );
	pInfo->AddValue( "Distance", Distance );
	pInfo->AddValue( "WidthSqr", WidthSqr );
	pInfo->AddValue( "HeighSqr", HeightSqr );
}

void CRect::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						  System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "Origin", Origin );
	pInfo->GetValue( "Width", Width );
	pInfo->GetValue( "Height", Height );
	pInfo->GetValue( "Normal", Normal );
	pInfo->GetValue( "Distance", Distance );
	pInfo->GetValue( "WidthSqr", WidthSqr );
	pInfo->GetValue( "HeighSqr", HeightSqr );
}


} // namespace Math
} // namespace WT
