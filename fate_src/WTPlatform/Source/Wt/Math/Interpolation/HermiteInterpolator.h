//
//	HermiteInterpolator.h
//	
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{


//
// A Hermite interpolation
//

template< typename VT>
__forceinline void HermiteInterpolate ( float32 T , const VT& P0 , const VT& Tangent0 , const VT& P1 , const VT& Tangent1 , VT& Result )
{
	float32	T2;			    // t squared
	float32	T3;			    // t cubed
	float32 H1,H2,H3,H4;	// hermite basis function coefficients

	T2 = T * T;
	T3 = T2 * T;

	H2 = -(T3 + T3) + T2*3.0f;
	H1 = 1.0f - H2;
	H4 = T3 - T2;
	H3 = H4 - T2 + T;   //t3 - 2.0f * t2 + t;

	Result = P0 * H1 + P1 * H2 + Tangent0 * H3 + Tangent1 * H4;
}


} // namespace Math
} // namespace WT
