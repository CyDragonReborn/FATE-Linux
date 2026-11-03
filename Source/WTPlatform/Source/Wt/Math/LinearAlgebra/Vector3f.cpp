//
//	CVector3f.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "Vector3f.h"

#include <cassert>


namespace WT
{
namespace Math
{


namespace
{
	float32 const KEpsilon = 0.000173f;
} //anonymous namespace

CVector3f const CVector3f::gm_Zero( 0, 0, 0);
CVector3f const CVector3f::gm_XAxis(1, 0, 0);
CVector3f const CVector3f::gm_YAxis(0, 1, 0);
CVector3f const CVector3f::gm_ZAxis(0, 0, 1);
CVector3f const CVector3f::gm_NegXAxis(-1, 0, 0);
CVector3f const CVector3f::gm_NegYAxis(0, -1, 0);
CVector3f const CVector3f::gm_NegZAxis(0, 0, -1);
CVector3f const CVector3f::gm_UnitScale(1, 1, 1);

//
// CVector3f	APIs
//

bool __fastcall CVector3f::Compare( const CVector3f& V2, float32 Tolerance) const
{
	assert( Tolerance >= 0.0f );

	if (fabs(V2.x - x) > Tolerance)
		return false;
	if (fabs(V2.y - y) > Tolerance)
		return false;
	if (fabs(V2.z - z) > Tolerance)
		return false;

	return true;
}

bool __fastcall CVector3f::IsNormalized( ) const
{
	float32 const Error = LengthSquared( ) - 1.0f;
	return fabsf( Error ) <= KEpsilon * 2.0f;
}

float32 __fastcall CVector3f::TriangleNormal(const CVector3f& V1, const CVector3f& V2,const CVector3f& V3)
{
	CVector3f A( false ), B( false );

	A = V2 - V1;
	B = V3 - V2;
	CrossProduct( A, B );
	return Normalize( );
}

float32 __fastcall CVector3f::TriangleArea(const CVector3f& V2, const CVector3f& V3) const
{
	CVector3f A( false ), B( false );
	
	A = V2 - *this;
	B = V3 - V2;
	A.CrossProduct( A, B );
	return A.Length( );
}

CVector3f __fastcall CVector3f::Perp() const
{
	CVector3f Result( false );

	if (fabsf( x ) < KEpsilon)
	{
		Result.x = 1.0f;
		Result.y = 0.0f;
	} else {
		Result.x = y;
		Result.y = -x;
	}

	Result.z = 0.0f;

	return Result;
}

CVector3f __fastcall CVector3f::UnitPerp() const
{
	CVector3f Result( false );
	
	if (fabsf( x ) < KEpsilon)
	{
		Result.x = 1.0f;
		Result.y = 0.0f;
	} else {
		float32 const Factor = 1.0f / Math_Hypot( x, y );
		Result.x = y * Factor;
		Result.y = -x * Factor;
	}

	Result.z = 0.0f;
	
	return Result;
}

} // namespace Math
} // namespace WT
