//
//	Quaternion.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//


#include "WT/stdafx.h"
#include "WT/Math/Quaternion/Quaternion.h"
#include "WT/Math/Utils/MathUtil.h"
#include <math.h>

namespace WT
{
namespace Math
{


//
// CQuaternion	APIs
//

namespace
{
	float32 const KEpsilon = 0.000173f;
	float32 const KCosTolerance = 0.99999f; //Largest cosine from which accurate angle can be extracted
} //anonymous namespace


void __fastcall CQuaternion::SetFromAxisAngle(const CVector3f& Axis, float32 Theta)
	// set a quaternion from an axis and a rotation around the axis
{
	assert( Theta * Theta >= 0.0f );
	//assert( Axis.IsNormalized() );
	
	Theta *= 0.5f;
	w = cosf( Theta );
	float32 const sinTheta = sinf( Theta );
	x = sinTheta * Axis.x;
	y = sinTheta * Axis.y;
	z = sinTheta * Axis.z;
	//assert( IsUnit() );
}

void __fastcall CQuaternion::SetFromEulerAngles(const CVector3f& EulerAngles)
{
	CVector3f h( EulerAngles * 0.5f );
	float32 sx = sinf( h.x );
	float32 cx = cosf( h.x );
	float32 sy = sinf( h.y );
	float32 cy = cosf( h.y );
	float32 sz = sinf( h.z );
	float32 cz = cosf( h.z );
	
	//Result = Z rotation * X rotation * Y rotation = qz * qx * qy
	//qy = cy + j sy
	//qx = cx + i sx
	//qx * qy = cycx + icysx + jsycx - ksysx
	//qz = cz + k sz
	//qz * qx * qy = (cycxcz + sysxsz) + i(cysxcz+sycxsz) + j(sycxcz-cysxsz) + k(cycxsz-sysxcz)

	w = cy * cx * cz + sy * sx * sz;
	x = cy * sx * cz + sy * cx * sz;
	y = sy * cx * cz - cy * sx * sz;
	z = cy * cx * sz - sy * sx * cz;
}

void __fastcall CQuaternion::SetFromEulerAngles(float32 Pitch, float32 Yaw, float32 Roll)
{
	SetFromEulerAngles( CVector3f( Pitch, Yaw, Roll ) );
}

void __fastcall CQuaternion::SetAlign( CVector3f const& Source, CVector3f const& Dest )
{
	//The result is going to be a rotation about Source x Dest through angle
	//theta = acos( Source . Dest ). However, the quaternion components are based 
	//on cos(theta/2) and sin(theta/2), so we should instead do the cross and dot 
	//product operations with the "middle" vector (the normalized average).

	CVector3f Middle = Source + Dest;
	float32 const LengthSquared = Middle.LengthSquared();

	if (LengthSquared < KEpsilon)
	{
		//The two vectors are nearly opposite, so the middle vector is inaccurate.
		//Rotating around any perpendicular vector will suffice.
		w = 0.0f;
		*reinterpret_cast<CVector3f*>( &x ) = Source.UnitPerp();
	} else {
		Middle /= sqrtf( LengthSquared );
		w = Source.DotProduct( Middle );
		*reinterpret_cast<CVector3f*>( &x ) = Source.CrossProduct( Middle );
	}

	assert( (Dest - Rotate( Source )).LengthSquared() < 0.01f );
}

bool __fastcall CQuaternion::GetAxisAngle(CVector3f *Axis, float32 *Theta) const
{	
	assert( Axis != NULL );
	assert( Theta != NULL );
	//assert( IsUnit() );
	
	if (w <= KCosTolerance)
	{
		float32 const HalfTheta = acosf( w );
		float32 OneOverSinTheta = 1.0f / sinf( HalfTheta );
		Axis->x = OneOverSinTheta * x;
		Axis->y = OneOverSinTheta * y;
		Axis->z = OneOverSinTheta * z;
		*Theta = 2.0f * HalfTheta;
		assert( *Theta * *Theta >= 0.0f);
		return true;
	}
	else
	{
		Axis->x = Axis->y = Axis->z = 0.0f;
		*Theta = 0.0f;
		return false;
	}
}

void __fastcall CQuaternion::ComputeEulerAngles(CVector3f& EulerAngles) const
{
	float32 const xy = x * y;
	float32 const yz = y * z;
	float32 const wx = w * x;
	float32 const wz = w * z;
	
	float32 RowZy = yz - wx;

	//Pitch
	EulerAngles.x = RowZy <= -0.5f ? KPi * 0.5f :
					RowZy >= +0.5f ? KPi * -0.5f :
					-asinf( RowZy + RowZy );

	if (fabsf( RowZy ) <= 0.5f * (1.0f - KEpsilon))
	{
		float32 const xx = 0.5f - x * x;
		EulerAngles.y = atan2f( x * z + w * y, xx - y * y ); //Yaw
		EulerAngles.z = atan2f( xy + wz, xx - z * z ); //Roll
	} else {
		EulerAngles.y = atan2f( xy - wz, yz + wx ); //Yaw
		EulerAngles.z = 0.0f; //Roll
	}
}

void __fastcall CQuaternion::ComputeEulerAngles(float32* pPitch, float32* pYaw, float32* pRoll) const
{
	float32 const xy = x * y;
	float32 const yz = y * z;
	float32 const wx = w * x;
	float32 const wz = w * z;
	
	float32 RowZy = yz - wx;

	//Pitch
	if (pPitch != NULL)
	{
		*pPitch = RowZy <= -0.5f ? KPi * 0.5f :
				  RowZy >= +0.5f ? KPi * -0.5f :
			      -asinf( RowZy + RowZy );
	}

	if (fabsf( RowZy ) <= 0.5f * (1.0f - KEpsilon))
	{
		float32 const xx = 0.5f - x * x;
		if (pYaw != NULL)
		{
			*pYaw = atan2f( x * z + w * y, xx - y * y );
		}
		if (pRoll != NULL)
		{
			*pRoll = atan2f( xy + wz, xx - z * z );
		}
	} else {
		if (pYaw != NULL )
		{
			*pYaw = atan2f( xy - wz, yz + wx );
		}
		if (pRoll != NULL)
		{
			*pRoll = 0.0f;
		}
	}
}

void __fastcall CQuaternion::FromMatrix(const CMatrix4f& M)
	// takes upper 3 by 3 portion of matrix (rotation sub matrix) 
	// and generates a quaternion
{
	float32 trace,s;
	
	assert( M.IsOrthonormal() );
	
	trace = M.RowX.x + M.RowY.y + M.RowZ.z;
	if (trace > 0.0f)
	{
		s = sqrtf(trace + 1.0f);
		w = s * 0.5f;
		s = 0.5f / s;
		
		x = (M.RowY.z - M.RowZ.y) * s;
		y = (M.RowZ.x - M.RowX.z) * s;
		z = (M.RowX.y - M.RowY.x) * s;
	}
	else
	{
		enum {X,Y,Z};
		int32 biggest = X;

		if (M.RowY.y > M.RowX.x)
		{
			biggest = Y;
			if (M.RowZ.z > M.RowY.y)
			{
				biggest = Z;
			}
		} else if (M.RowZ.z > M.RowX.x) {
			biggest = Z;
		}
		
		// in the unusual case the original trace fails to produce a good sqrt, try others...
		switch (biggest)
		{
		case X:
			s = sqrtf( M.RowX.x - (M.RowY.y + M.RowZ.z) + 1.0f);
			x = s * 0.5f;
			s = 0.5f / s;
			w = (M.RowY.z - M.RowZ.y) * s;
			y = (M.RowX.y + M.RowY.x) * s;
			z = (M.RowX.z + M.RowZ.x) * s;
			break;
		case Y:
			s = sqrtf( M.RowY.y - (M.RowZ.z + M.RowX.x) + 1.0f);
			y = s * 0.5f;
			s = 0.5f / s;
			w = (M.RowZ.x - M.RowX.z) * s;
			z = (M.RowY.z + M.RowZ.y) * s;
			x = (M.RowY.x + M.RowX.y) * s;
			break;
		case Z:
			s = sqrtf( M.RowZ.z - (M.RowX.x + M.RowY.y) + 1.0f);
			z = s * 0.5f;
			s = 0.5f / s;
			w = (M.RowX.y - M.RowY.x) * s;
			x = (M.RowZ.x + M.RowX.z) * s;
			y = (M.RowZ.y + M.RowY.z) * s;
			break;
		default:
			assert( 0 );
		}
	}

	//assert( IsUnit() );
}

void __fastcall CQuaternion::ToMatrix( CMatrix4f* M ) const
// takes a unit quaternion and fills out an equivelant rotation
// portion of a xform
{
	//assert( IsUnit() );

	float32	xx, xy, xz;
	float32	yy, yz;
	float32	zz;
	float32	wx, wy, wz;
	float32	x2, y2, z2;
	
	x2 = x * 2.0F;
	y2 = y * 2.0F;
	z2 = z * 2.0F;
	
	xx = x * x2;
	xy = x * y2;
	xz = x * z2;
	
	yy = y * y2;
	yz = y * z2;
	
	zz = z * z2;
	
	wx = w * x2;
	wy = w * y2;
	wz = w * z2;
	
	// x axis
	M->RowX.x = 1.0F - (yy + zz);
	M->RowX.y = xy + wz;
	M->RowX.z = xz - wy;
	M->RowX.w = 0;
	
	// y axis
	M->RowY.x = xy - wz;
	M->RowY.y = 1.0F - (xx + zz);
	M->RowY.z = yz + wx;
	M->RowY.w = 0;
	
	// z axis
	M->RowZ.x = xz + wy;
	M->RowZ.y = yz - wx;
	M->RowZ.z = 1.0F - (xx + yy);
	M->RowZ.w = 0;

	M->RowT.x = M->RowT.y = M->RowT.z = 0.0f;
	M->RowT.w = 1;

	//assert( M->IsOrthonormal() );
}


void __fastcall CQuaternion::Slerp(
								 const CQuaternion& Q0, 
								 const CQuaternion& Q1, 
								 float32			T)
	// spherical interpolation between q0 and q1.   0<=t<=1 
	// resulting quaternion is 'between' q0 and q1
	// with t==0 being all q0, and t==1 being all q1.
{
	float32 omega,cosom,Scale0,Scale1;
	CQuaternion QL( false );
	assert( ( 0 <= T ) && ( T <= 1.0f + KEpsilon ) );
	//assert( Q0.IsUnit() );
	//assert( Q1.IsUnit() );
	
	cosom =	Q0.Dot( Q1 );
	
	if (cosom < 0)
	{
		cosom = -cosom;
		QL.x = -Q1.x;
		QL.y = -Q1.y;
		QL.z = -Q1.z;
		QL.w = -Q1.w;
	}
	else
	{
		QL = Q1;
	}
	
	
	if (cosom < 1.0f - KEpsilon)
	{
		omega  = acosf( cosom );
		float32 rsinom  = 1.0f / sinf( omega );
        float32 TAngle = T * omega;
		Scale0 = sinf( omega - TAngle ) * rsinom;
		Scale1 = sinf( TAngle ) * rsinom;
	}
	else
	{
		// has numerical difficulties around cosom == 1
		// in this case degenerate to linear interpolation
		
		Scale0 = 1.0f - T;
		Scale1 = T;
	}
	
	x = Scale0 * Q0.x + Scale1 * QL.x;
	y = Scale0 * Q0.y + Scale1 * QL.y;
	z = Scale0 * Q0.z + Scale1 * QL.z;
	w = Scale0 * Q0.w + Scale1 * QL.w;
	//assert( IsUnit() );
}


void __fastcall CQuaternion::SlerpNotShortest(
										  const CQuaternion& Q0, 
										  const CQuaternion& Q1, 
										  float32			  T)
	// spherical interpolation between q0 and q1.   0<=t<=1 
	// resulting quaternion is 'between' q0 and q1
	// with t==0 being all q0, and t==1 being all q1.
{
	float32 omega,cosom,Scale0,Scale1;
	assert( 0 <= T && T <= 1.0f );
	//assert( Q0.IsUnit() );
	//assert( Q1.IsUnit() );
	
	cosom =	Q0.Dot( Q1 );
	
    //There are three cases:
    //Case 1: Q0 and Q1 are nearly antiparallel, which
    //means we need to slerp with a perpendicular quaternion. 
    //Case 2: Q0 and Q1 are nearly parallel, which means we
    //need to degenerate to linear interpolation.
    //Case 3: This is the usual case. Straightforward spherical linear interpolation.

	if (cosom > -1.0f + KEpsilon )
	{
		if (cosom < 1.0f - KEpsilon)
		{
            //Case 3:

            //Note: omega and sinom here are independent of T, and could
            //be precomputed.
			omega  = acosf( cosom );
			float32 sinom  = sinf( omega );

            //We know cosom = 1 - e, where e > eps and e < 2 - eps. Now,
            //sinom = sqrt(1 - cosom*cosom) = sqrt(2e - e * e). We can prove
            //sinom > eps as follows: e < 2 - eps means eps < 2 - e. Thus
            //eps^2/eps < 2 - e. Since e > eps, eps^2/e < 2 - e. Thus,
            //eps^2 < 2e - e * e. Also, eps < sqrt(2e - e * e). Therefore, 
            //sinom > eps.
            assert(sinom > KEpsilon);

            float32 rsinom = 1.0f / sinom;
            float32 TAngle = T * omega;
			Scale0 = sinf( omega - TAngle) * rsinom;
			Scale1 = sinf( TAngle ) * rsinom;
		}
		else
		{
            //Case 2:
			// has numerical difficulties around cosom == 1
			// in this case degenerate to linear interpolation
			
			Scale0 = 1.0f - T;
			Scale1 = T;
		}
		x = Scale0 * Q0.x + Scale1 * Q1.x;
		y = Scale0 * Q0.y + Scale1 * Q1.y;
		z = Scale0 * Q0.z + Scale1 * Q1.z;
		w = Scale0 * Q0.w + Scale1 * Q1.w;
	}
	else
	{
        //Case 1:
        //(-Q0.y, Q0.x, -Q0.w, Q0.z) is perpendicular to (Q0.x, Q0.y, Q0.z, Q0.w)
        //and halfway towards Q1. Slerp towards the perpendicular quat with double 
        //the angle
        float32 TAngle = T * KPi;
		Scale0 = sinf( KPi * 0.5f - TAngle );
		Scale1 = sinf( TAngle );
		x = Scale0 * Q0.x - Scale1 * Q0.y;
		y = Scale0 * Q0.y + Scale1 * Q0.x;
		z = Scale0 * Q0.z - Scale1 * Q0.w;
		w = Scale0 * Q0.w + Scale1 * Q0.z;
	}

    //assert( IsUnit() );
}

float32 __fastcall CQuaternion::Normalize()
	// normalizes Q to be a unit CQuaternion
{
	assert( IsValid() );
	
	float32 const MagnitudeSquared = x * x + y * y + z * z + w * w;
	if (MagnitudeSquared == 0.0f)
	{
		return 0.0f;
	}

	float32 const Magnitude = sqrtf( MagnitudeSquared );
	operator/=( Magnitude );
	return Magnitude;
}

// ln(Q) for unit quaternion only!
CQuaternion __fastcall CQuaternion::Ln() const
{
	assert( IsValid() );
	//assert( IsUnit() );

	CQuaternion QL( false );
	if (w < 0.0f)
	{
		QL.x = -x;
		QL.y = -y;
		QL.z = -z;
		QL.w = -w;
	}
	else
	{
		QL = *this;
	}

	if (QL.w <= KCosTolerance)
	{
		float32 Theta = acosf( QL.w  );
		float32 Theta_Over_sin_Theta = Theta / sinf( Theta );
		QL.x *= Theta_Over_sin_Theta;
		QL.y *= Theta_Over_sin_Theta;
		QL.z *= Theta_Over_sin_Theta;
	}

	QL.w = 0.0f;
	return QL;
}
	
CQuaternion __fastcall CQuaternion::Exp() const
							  // exp(Q) for pure quaternion only!  (zero scalar part (W))
{
	float32 Theta;
	float32 sin_Theta_over_Theta;
	
	assert( IsValid() );
	assert( w == 0.0f );	//check a range?
	
	Theta = sqrtf( x * x + y * y + z * z );
	if (Theta > KEpsilon)
	{
		sin_Theta_over_Theta = sinf( Theta ) / Theta;
	}
	else
	{
		sin_Theta_over_Theta = 1.0f;
	}
	
	CQuaternion Result( false );
	Result.x = sin_Theta_over_Theta * x;
	Result.y = sin_Theta_over_Theta * y;
	Result.z = sin_Theta_over_Theta * z;
	Result.w = cosf( Theta );
	return Result;
}	

bool __fastcall CQuaternion::Compare( const CQuaternion& Q2, float32 Tolerance ) const
{
	assert( IsValid() );
	assert( Q2.IsValid() );
	assert( Tolerance >= 0.0f );
	
	if(	// they are the same but with opposite signs
		(	(fabsf(x + Q2.x) <= Tolerance )  
		  &&(fabsf(y + Q2.y) <= Tolerance )  
		  &&(fabsf(z + Q2.z) <= Tolerance )  
		  &&(fabsf(w + Q2.w) <= Tolerance )  
		)
		||  // they are the same with same signs
		(	(fabsf(x - Q2.x) <= Tolerance )  
		  &&(fabsf(y - Q2.y) <= Tolerance )  
		  &&(fabsf(z - Q2.z) <= Tolerance )  
		  &&(fabsf(w - Q2.w) <= Tolerance )  
		)
	  )
	{
		return true;
	}
	else
	{
		return false;
	}
}


} // namespace Math
} // namespace WT
