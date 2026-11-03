//
//	CDisc.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Objects/Disc.h"
#include "WT/Math/MathSerialization.h"


namespace WT
{
namespace Math
{

WT_DEFINE_CLASS( CDisc )

bool __fastcall CDisc::MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	//calculate our next movement position

	CVector3f End ( Start + Velocity * FrameTime );

	// calculate old and new distances from the disc : dist(pos,plane) = n * pos + d
	// m_RealRadius stores -n*p, which is d.

	float32 OldDistance = Start.DotProduct ( Normal ) + RealRadiusSqr;
	float32 NewDistance = End.DotProduct ( Normal ) + RealRadiusSqr;
	
	if ( OldDistance * NewDistance >= 0 )
		return false;

	// Find the crossing point by using p(t) = start + velocity * t -> t = -(n * p + D) / (n * v)

	float32 nv = Normal.DotProduct ( Velocity );
	float32 CollisionTime = -OldDistance / nv;
	
	// Actual hit point p(t) = start + velocity * t

	CVector3f HitPos ( Start + Velocity * CollisionTime );
	
	// Offset from origin in plane, hit - center

	CVector3f HitPosDistance ( HitPos - Center );
	
	float32 r = HitPosDistance.LengthSquared();
	
	if ( r > RadiusSqr || r < Radius2Sqr )
		return false;
	
	//we got a hit here, so compute tangential and normal components of velocity

	NormalVelocity = Normal * nv;
	TangentVelocity = Velocity - NormalVelocity;

	return true;
}


// ISerializable

void CDisc::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						  System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "Center", Center );
	pInfo->AddValue( "Normal", Normal );
	pInfo->AddValue( "Radius", Radius );
	pInfo->AddValue( "Radius2", Radius2 );
	pInfo->AddValue( "RadiusSqr", RadiusSqr );
	pInfo->AddValue( "Radius2Sqr", Radius2Sqr );
	pInfo->AddValue( "RealRadiusSqr", RealRadiusSqr );
	pInfo->AddValue( "U", U );
	pInfo->AddValue( "V", V );
}

void CDisc::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						  System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "Center", Center );
	pInfo->GetValue( "Normal", Normal );
	pInfo->GetValue( "Radius", Radius );
	pInfo->GetValue( "Radius2", Radius2 );
	pInfo->GetValue( "RadiusSqr", RadiusSqr );
	pInfo->GetValue( "Radius2Sqr", Radius2Sqr );
	pInfo->GetValue( "RealRadiusSqr", RealRadiusSqr );
	pInfo->GetValue( "U", U );
	pInfo->GetValue( "V", V );
}


} // namespace Math
} // namespace WT
