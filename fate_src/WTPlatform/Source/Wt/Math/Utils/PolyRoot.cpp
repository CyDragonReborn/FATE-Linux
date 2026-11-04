//
//	PolyRoot.cpp
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//


#include "WT/stdafx.h"
#include "WT/Math/Utils/MathUtil.h"
#include <math.h>

namespace WT
{
namespace Math
{

void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a1, float32 a0 )
{
	if (a1 != 0.0f)
	{
		Results.push_back( -a0 / a1 );
	}
}

void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a2, float32 a1, float32 a0 )
{
	if (a2 == 0.0f)
	{
		return ComputeRealPolynomialRoots( Results, a1, a0 );
	}

	float32 const D = a1 * a1 - 4.0f * a2 * a0;
	if (D >= 0.0f)
	{
		float32 const sD = sqrtf( D );
		float32 const Factor = -0.5f / a2;
		Results.push_back( Factor * (a1 + sD) );
		Results.push_back( Factor * (a1 - sD) );
	}
}

void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a3, float32 a2, float32 a1, float32 a0 )
{
	if (a3 == 0.0f)
	{
		return ComputeRealPolynomialRoots( Results, a2, a1, a0 );
	}

	float32 const ia3 = 1.0f / a3;
	a2 = a2 * ia3;
	a1 = a1 * ia3;
	a0 = a0 * ia3;

	float32 const a2a2 = a2 * a2;
	float32 const Q = 3.0f * a1 - a2a2;
	float32 const R = (4.5f * a1 - a2a2) * a2 - 13.5f * a0;
	float32 const D = Q * Q * Q + R * R;
	
	if (D <= 0.0f)
	{
		float32 sQ = sqrtf( -Q );
		float32 const theta = Q == 0.0f ? 0.0f : acosf( R / (Q * sQ) ) * (1.0f / 3.0f);
		sQ *= -2.0f / 3.0f;
		float32 const x1 = sQ * cosf( theta                          );
		float32 const x2 = sQ * cosf( theta + KTwoPi * (1.0f / 3.0f) );

		a2 *= -1.0f / 3.0f;
		Results.push_back( a2 + x2 );
		Results.push_back( a2 - x1 - x2 );
		Results.push_back( a2 + x1 );
	} else if (D > 0.0f) {
		float32 const sD = sqrtf( D );
		float32 const S = Math_cubert( R + sD );
		float32 const T = Math_cubert( R - sD );
		Results.push_back( (S + T - a2) * (1.0f / 3.0f) );
	}
}

void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a4, 
									    float32 a3, float32 a2, float32 a1, float32 a0 )
{
	if (a4 == 0.0f)
	{
		return ComputeRealPolynomialRoots( Results, a3, a2, a1, a0 );
	}

	float32 const ia4 = 1.0f / a4;
	a3 = a3 * ia4;
	a2 = a2 * ia4;
	a1 = a1 * ia4;
	a0 = a0 * ia4;

	float32 const a3a3 = a3 * a3;

	float32 y;
	{
		std::vector<float32> ys;
		ComputeRealPolynomialRoots( ys, 1.0f, -a2, a1 * a3 - 4.0f * a0, (4.0f * a2 - a3a3) * a0 - a1 * a1 );
		assert( !ys.empty() );
		y = ys[0];
	}

	float32 const average = a3 * (-1.0f / 4.0f);
	float32 const R2 = 0.25f * a3a3 - a2 + y;
	float32 const X  = 0.50f * a3a3 - a2 - y;
	float32 Y;
	float32 R;

	if (R2)
	{
		Y = a3 * a2 - a1 - a1 - 0.25f * a3a3 * a3;

		if (R2 < 0.0f)
		{
			if (Y == 0.0f && X == R2)
			{
				Results.push_back( average );
				Results.push_back( average );
			}
			return;
		}

		R = sqrtf( R2 );
		Y /= R;
	} else {
		R = 0.0f;
		Y = y * y - 4.0f * a0;
		if (Y < 0.0f)
		{
			return;
		}
		Y = 2.0f * sqrtf( Y );
	}

	if (X + Y >= 0.0f)
	{
		float32 const D = sqrtf( X + Y );
		Results.push_back( average + 0.5f * (R + D) );
		Results.push_back( average + 0.5f * (R - D) );
	}

	if (X - Y >= 0.0f)
	{
		float32 const E = sqrtf( X - Y );
		Results.push_back( average + 0.5f * (-R + E) );
		Results.push_back( average + 0.5f * (-R - E) );
	}
}


} // namespace Math
} // namespace WT
