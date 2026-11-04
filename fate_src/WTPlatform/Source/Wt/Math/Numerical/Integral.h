//
//	Integral.h
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{

//Rough numerical approximation to the integral of function pIntegrand over the interval [L,R],
//using Legendre-Gaussian quadrature.
float32 WTCall LegendreGaussQuadrature( float32 (WTCall *pIntegrand)(float32, void const*), void const* pIntegrandData, 
								 float32 L, float32 R );

//Accurate numerical approximation to the integral of function pIntegrand over the interval [L,R],
//using adaptive Legendre-Gaussian quadrature.
float32 WTCall AdaptiveIntegrate( float32 (WTCall *pIntegrand)(float32, void const*), void const* pIntegrandData, 
						   float32 L, float32 R, float32 ToleranceFactor = 1.0e-5f );

} // namespace Math
} // namespace WT
