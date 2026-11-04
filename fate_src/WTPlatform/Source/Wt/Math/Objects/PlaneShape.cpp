//
//	Plane.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/System/Platform.h"
#include "WT/Math/Objects/PlaneShape.h"
#include "WT/Math/MathSerialization.h"

using namespace WT::Math;

namespace WT
{
namespace Math
{

WT_DEFINE_CLASS( CPlaneShape )

//
//	CPlane::TypeFromUnitVector
//
EPlaneType::Type CPlaneShape::TypeFromUnitVector(const CVector3f *V1)
{
	float32	X, Y, Z;

	assert(V1);

	X = (float32)fabs(V1->x);
	Y = (float32)fabs(V1->y);
	Z = (float32)fabs(V1->z);

	if (X == 1.0f)			
		return EPlaneType::X;			// Axial aligned on X
	else if (Y == 1.0f)			
		return EPlaneType::Y;			// Axial aligned on Y
	else if (Z == 1.0f)
		return EPlaneType::Z;			// Axial aligned on Z
	else if (X >= Y && X >= Z)	
		return EPlaneType::AnyX;		// Non Axial, X dominent axis
	else if (Y >= X && Y >= Z)
		return EPlaneType::AnyY;		// Non Axial, Y dominent axis
	else
		return EPlaneType::AnyZ;		// Non Axial, Z dominent axis
}

// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
bool __fastcall CPlaneShape::MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	return m_Plane.MovingPointCollision( Start, Velocity, FrameTime, TangentVelocity, NormalVelocity );
}

// ISerializable

void CPlaneShape::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						   System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "Plane", m_Plane );
	pInfo->AddValue( "Type", static_cast<int32>(Type) );
}

void CPlaneShape::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						   System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "Plane", m_Plane );

	int32 nType=0;
	pInfo->GetValue( "Type", nType );
	Type = static_cast<EPlaneType::Type>( nType );
}


} // namespace Math
} // namespace WT
