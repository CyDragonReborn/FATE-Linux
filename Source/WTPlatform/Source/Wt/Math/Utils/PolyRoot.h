//
//	PolyRoot.h
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include <vector>

namespace WT
{
namespace Math
{

//Compute the real roots of a1 x + a0 = 0
void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a1, float32 a0 );

//Compute the real roots of a2 x^2 + a1 x + a0 = 0
void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a2, float32 a1, float32 a0 );

//Compute the real roots of a3 x^3 + a2 x^2 + a1 x + a0 = 0
void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a3, float32 a2, float32 a1, float32 a0 );

//Compute the real roots of a4 x^4 + a3 x^3 + a2 x^2 + a1 x + a0 = 0
void WTCall ComputeRealPolynomialRoots( std::vector<float32>& Results, float32 a4, float32 a3, float32 a2, float32 a1, float32 a0 );

} // namespace Math
} // namespace WT
