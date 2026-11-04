//
//	BezierInterpolator.h
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


// Interpolate position for Bezier curve specified by four control points.
// This accepts control points P0, P1, P2, and P3; and passes through control points P0 and P3.
template< typename VT>
__forceinline void WTCall BezierInterpolate( float32 T, VT const& P0, VT const& P1, VT const& P2, VT const& P3, VT& Result )
{
	//P0 * (1 - t)^3 + P1 * 3t(1 - t)^2 + P2 * 3t^2(1 - t) + P3 * t^3

	float32 const OMT = 1 - T;
	float32 const OMT2 = OMT * OMT;
	float32 const OMT3 = OMT2 * OMT;

	float32 const T2 = T * T;
	float32 const T3 = T2 * T;

	Result = (P0 * OMT3) + (P1 * 3 * T * OMT2) + (P2 * 3 * T2 * OMT) + (P3 * T3);
}

// Interpolate derivative for Bezier curve specified by four control points.
template< typename VT>
__forceinline void WTCall BezierInterpolateDerivative( float32 T, VT const& P0, VT const& P1, VT const& P2, VT const& P3, VT& Result )
{
	//P0 [-3(1-t)^2] + P1 3[(1-t)^2 - 2t(1-t)] + P2 3[2t(1-t) - t^2] + P3 [3t^2]

	float32 const OMT = 1 - T;
	float32 const OMT2 = OMT * OMT;
	float32 const T2 = T * T;
	float32 const TOMT2 = T * OMT * 2.0f;

	Result = (P1 * (OMT2 - TOMT2) + P2 * (TOMT2 - T2) + P3 * T2 - P0 * OMT2) * 3.0f;
}

// Interpolate second derivative for Bezier curve specified by four control points.
template< typename VT>
__forceinline void WTCall BezierInterpolateCurvature( float32 T, VT const& P0, VT const& P1, VT const& P2, VT const& P3, VT& Result )
{
	//P0 6(1 - t) + P1 6(3t - 2) + P2 6(1 - 3t) + P3 6t

	Result = (P0 * (1 - T) + P1 * (3 * T - 2) + P2 * (1 - 3 * T) + P3 * T) * 6.0f;
}

} // namespace Math
} // namespace WT
