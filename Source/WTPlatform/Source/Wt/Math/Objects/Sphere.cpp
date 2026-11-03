//
//	Sphere.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Objects/Sphere.h"
#include "WT/Math/MathSerialization.h"


#define EB_CLAMP(x, low, high) (((x)<(low))?(low):(((x)>(high))?(high):(x)))


namespace WT
{
namespace Math
{

WT_DEFINE_CLASS(CSphere)

void __fastcall CSphere::SetToPoint ( const CVector3f& Point )
{
	Center = Point;
	Radius = 0;
	Radius2 = 0;
}

CVector3f __fastcall CSphere::ComputeCenter( ) const
{
	return Center;
}

void __fastcall CSphere::ExtendToEnclose( const CVector3f& Point )
{
	float32 D = Point.DistanceBetweenSquared( Center );
	if(D > (Radius*Radius))
		Radius = sqrtf(D);
}

bool __fastcall CSphere::Intersects( const CSphere& B1) const
{
	float32 CDist = (B1.Radius + Radius);
	if( (CDist*CDist) > B1.Center.DistanceBetweenSquared(Center) )
		return true;
	else
		return false;
}

	
bool __fastcall CSphere::Intersection( const CSphere& B1, const CSphere& B2)
{
	return B1.Intersects( B2 );
}

void __fastcall CSphere::Union( const CSphere& B1, const CSphere& B2 )
{
	CVector3f a, b, rn;
	rn = B2.Center - B1.Center;
	
	float32 diam = rn.Normalize();
	diam += B1.Radius + B2.Radius;

	rn.Inverse();
	a = rn * B1.Radius;
	a += B1.Center;

	b = rn * diam;
	b += a;

	CVector3f newcenter = a + b;
	newcenter *= 0.5f;
	
	Center = newcenter;
	Radius = diam/2;
}

bool __fastcall CSphere::IsInside( const CVector3f& Point ) const
{
	float32 DistanceSqr = Center.DistanceBetweenSquared(Point);
	if( DistanceSqr > (Radius*Radius) || DistanceSqr < ( Radius2*Radius2 ) )
		return false;
	else
		return true;
}

CVector3f __fastcall CSphere::GetTranslation( ) const
{
	return Center;
}

void __fastcall CSphere::Translate( float32 DX,  float32 DY,  float32 DZ	)
{
	CVector3f VecDelta(DX, DY, DZ);
	Center += VecDelta;	
}

void __fastcall CSphere::SetTranslation( const CVector3f& NewCenter )
{
	Center = NewCenter;
}

CVector3f __fastcall CSphere::GetScaling( ) const
{
	return CVector3f( Radius, Radius, Radius );
}

void __fastcall CSphere::Scale( float32 Scale )
{
	Radius *= Scale;
	Radius2 *= Scale;
}

void __fastcall CSphere::SetScaling( float32 Scale )
{
	Radius = Scale;
}

void __fastcall CSphere::LinearSweep( const CSphere& BoxToSweep, const CVector3f& StartPoint, const CVector3f& EndPoint )
{
	CSphere Start, End;

	Start.Center = StartPoint;
	Start.Radius = BoxToSweep.Radius;

	End.Center = EndPoint;
	End.Radius = BoxToSweep.Radius;

	Union( Start, End );
}


bool __fastcall CSphere::RayCollision( const CVector3f& /*Start*/, const CVector3f& /*End*/, float32 * /*T*/, CVector3f* /*Normal*/ ) const
{
	//TODO: fill in this function
	return false;
}

void  __fastcall CSphere::Transform(const CMatrix4f& XF )
{
	XF.Transform( Center, Center );
}

float32 __fastcall CSphere::DistanceSquaredToPoint(const CVector3f& Pos) const
{
	float32 d = Center.DistanceBetween(Pos) - Radius;
	return d*d;
}

bool __fastcall CSphere::MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	//calculate our next movement position

	CVector3f End ( Start + Velocity * FrameTime );

	//first lets check if the end is inside, and early out, this will work for small DT, small velocities or
	//small spheres.
	if ( !IsInside ( End ))
		return false;

	CVector3f Normal ( Start - Center );
	Normal.Normalize();
		
	// Compute tangential and normal components of velocity

	float32 NormalMag = Velocity.DotProduct ( Normal );

	NormalVelocity = Normal * NormalMag;
	TangentVelocity = Velocity - NormalVelocity;

	return true;
}


// ISerializable

void CSphere::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
							System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "Center", Center );
	pInfo->AddValue( "Radius", Radius );
	pInfo->AddValue( "Radius2", Radius2 );
}

void CSphere::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
							System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "Center", Center );
	pInfo->GetValue( "Radius", Radius );
	pInfo->GetValue( "Radius2", Radius2 );
}


} // namespace Math
} // namespace WT
