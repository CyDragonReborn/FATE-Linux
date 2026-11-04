//
//	Plane.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Plane.h"
#include "WT/Math/MathSerialization.h"

using namespace WT::Math;

namespace WT
{
namespace Math
{

namespace
{
	float32 const KEpsilon = 0.000173f;
} // anonymous namespace


//
//	CPlane	APIs
//

//
//	CPlane::SetFromVerts
//
 void __fastcall CPlane::SetFromVerts(const CVector3f& V1, const CVector3f& V2, CVector3f const& V3)
{
	// Get the 2 vectors to derive the normal
	CVector3f Vect1( V2 - V1 );
	CVector3f Vect2( V3 - V2 );
	
	// The normal is the cross between these 2 vectors
	Normal.CrossProduct( Vect1, Vect2);
	Normal.Normalize( );

	// Get the planes distance from the origin, by projecting a vert on the plane
	// along the plane normal, to the origin...
	Distance = -V1.DotProduct( Normal );
}

float32 __fastcall CPlane::Normalize()
{
	float32 const Length = Normal.Length();
	float32 const InvLength = 1.0f / Length;
	Normal *= InvLength;
	Distance *= InvLength;
	return Length;
}

void __fastcall CPlane::Inverse()
{
	Normal.Inverse( );
	Distance = -Distance;
}

//
//	CPlane::Rotate
//
 void __fastcall CPlane::Rotate(const CMatrix4f& XForm, CPlane& Out) const
{
	XForm.Rotate( Out.Normal, Normal);

	Out.Distance = Distance;
}

//
//	CPlane::Transform
//
  void __fastcall CPlane::Transform( const CMatrix4f& XForm, CPlane& Out) const
{
	// Rotate the normal
	XForm.Rotate( Out.Normal, Normal );

	// Correct the distance value
	Out.Distance = Distance - Out.Normal.DotProduct( XForm.RowT );
}

//
//	CPlane::TransformRenorm
//
 void __fastcall CPlane::TransformRenorm(const CMatrix4f& XForm, CPlane& Out) const
{
	CVector3f		PointOnPlane;
	CMatrix4f	NormalXF;

	// Put a point on the plane
	PointOnPlane = Normal * -Distance;
	// Transform the point
	XForm.Transform( PointOnPlane, PointOnPlane );
	// Rotate the plane

	// To transform the normals correctly we need to do this
	NormalXF = XForm.GetInverse( );
	NormalXF = NormalXF.GetTranspose( );
	NormalXF.Rotate( Out.Normal, Normal );
	
	Out.Normal.Normalize(); // if XForm isn't normalized, need to renorm here
	// Find the Distance of the new plane by projecting the transformed point on the new plane
	Out.Distance = -Out.Normal.DotProduct( PointOnPlane );
}

void __fastcall CPlane::ComputePlaneTransform(CMatrix4f& Out) const
{
	Out.SetIdentity( );
	Out.RowY = Normal;
	
	Out.RowX.Set( Normal.y, -Normal.x, Normal.z + Normal.x );
	Out.RowZ.Set( Normal.z, Normal.z + Normal.x, -Normal.y );
	
	Out.Orthonormalize( );

	Out.RowT = Normal * -Distance;

	Out = Out.GetInverse();
}

//
//	CPlane::Compare
//
bool __fastcall CPlane::Compare(const CPlane& Plane2, float32 NEpsilon, float32 DEpsilon) const
{
	if (Normal.Compare( Plane2.Normal, NEpsilon ) && 
		fabs(Distance - Plane2.Distance) <= DEpsilon)
			return true;

	return false;
}

//
//	CPlane::Snap
//
 void __fastcall CPlane::Snap(float32 Epsilon)
{
	int32		i;

	for (i=0 ; i<3 ; i++)
	{
		if ( fabs( Normal[i] - 1.0f) < Epsilon )
		{
			Normal.Clear();
			Normal[i] = 1.0f;
			break;
		}

		if ( fabs( Normal[i] - -1.0f) < Epsilon )
		{
			Normal.Clear();
			Normal[i] = -1.0f;
			break;
		}
	}
}

//
//	CPlane::IsEqual
//
bool CPlane::IsEqual(const CPlane& B) const
{
	return Compare( B, KEpsilon, KEpsilon );
}


//
//	CPlane::IsOpposite
//
bool __fastcall CPlane::IsOpposite(const CPlane& B) const
{
	float32 negDist = -Distance;
	if( negDist < (B.Distance - KEpsilon) || negDist > (B.Distance + KEpsilon) )
		return false;
	CVector3f InverseNormal = -Normal;
	InverseNormal.Inverse( );
	return InverseNormal.Compare( B.Normal, KEpsilon );
}

// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
bool __fastcall CPlane::MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	// calculate our next movement position

	CVector3f End ( Start + Velocity * FrameTime );

	float32 OldDistance = Start.DotProduct ( Normal ) + Distance;
	float32 NewDistance = End.DotProduct ( Normal ) + Distance;
	
	if ( OldDistance * NewDistance >= 0 )
		return false;
	
	float32 NormalMag = Velocity.DotProduct ( Normal );

	NormalVelocity = Normal * NormalMag;
	TangentVelocity = Velocity - NormalVelocity;

	return true;
}

} // namespace Math
} // namespace WT
