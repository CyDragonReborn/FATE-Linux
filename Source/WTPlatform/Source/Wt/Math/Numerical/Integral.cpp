//
//	Integral.cpp
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//


#include "WT/stdafx.h"
#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{

float32 WTCall LegendreGaussQuadrature( float32 (WTCall *pIntegrand)(float32, void const*), void const* pIntegrandData, 
									    float32 L, float32 R )
{
	//Use 32-point Legendre-Gaussian quadrature
	int32 const KSampleCount = 32;
	static float32 const Samples[KSampleCount] =
	{
		-0.997263861849f,	-0.985611511545f,	-0.964762255588f,	-0.934906075938f,
		-0.896321155766f,	-0.849367613733f,	-0.794483795968f,	-0.73218211874f,
		-0.66304426693f,	-0.587715757241f,	-0.506899908932f,	-0.421351276131f,
		-0.331868602282f,	-0.239287362252f,	-0.144471961583f,	-0.0483076656877f,
		0.0483076656877f,	0.144471961583f,	0.239287362252f,	0.331868602282f,
		0.421351276131f,	0.506899908932f,	0.587715757241f,	0.66304426693f,
		0.73218211874f,		0.794483795968f,	0.849367613733f,	0.896321155766f,
		0.934906075938f,	0.964762255588f,	0.985611511545f,	0.997263861849f,
	};

	static float32 const Weights[KSampleCount] =
	{
		0.00701814576495f,	0.0162774265831f,	0.0253910098329f,	0.0342745478477f,
		0.0428359896785f,	0.0509978738117f,	0.0586839394615f,	0.0658220603578f,
		0.0723456094297f,	0.078193695762f,	0.083311711103f,	0.0876518688047f,
		0.0911736454878f,	0.0938441590423f,	0.0956384754512f,	0.0965398415811f,
		0.0965398415811f,	0.0956384754512f,	0.0938441590423f,	0.0911736454878f,
		0.0876518688047f,	0.083311711103f,	0.078193695762f,	0.0723456094297f,
		0.0658220603578f,	0.0586839394615f,	0.0509978738117f,	0.0428359896785f,
		0.0342745478477f,	0.0253910098329f,	0.0162774265831f,	0.00701814576495f,
	};

	//Let y(x) = Ax + B. Choose A and B such that y(-1) = L and y(1) = R.
	float32 const A = (R - L) * 0.5f;
	float32 const B = L + A;

	float32 Sum = 0.0f;
	for (int32 i = 0; i < KSampleCount; ++i)
	{
		float32 const y = A * Samples[i] + B;
		Sum += (*pIntegrand)( y, pIntegrandData ) * Weights[i];
	}
	return Sum * A;
}

namespace 
{
float32 AdaptiveIntegrateAux( float32 (WTCall *pIntegrand)(float32, void const*), void const* pIntegrandData, float32 L, float32 R,  
							  float32 TotalIntegral, float32 const ToleranceFactor, int32 Depth )
{
	int32 const KMaxDepth = 10;
	if (Depth >= KMaxDepth)
	{
		return TotalIntegral;
	}

	float32 const Mid = (L + R) * 0.5f;
	float32 LeftIntegral = LegendreGaussQuadrature( pIntegrand, pIntegrandData, L, Mid );
	float32 RightIntegral = LegendreGaussQuadrature( pIntegrand, pIntegrandData, Mid, R );

	if (fabsf( LeftIntegral + RightIntegral - TotalIntegral ) > ToleranceFactor * fabsf( TotalIntegral ))
	{
		LeftIntegral  = AdaptiveIntegrateAux( pIntegrand, pIntegrandData, L, Mid, LeftIntegral , ToleranceFactor, Depth + 1 );
		RightIntegral = AdaptiveIntegrateAux( pIntegrand, pIntegrandData, Mid, R, RightIntegral, ToleranceFactor, Depth + 1 );
	}

	return LeftIntegral + RightIntegral;
}
}

float32 WTCall AdaptiveIntegrate( float32 (WTCall *pIntegrand)(float32, void const*), void const* pIntegrandData, 
								  float32 L, float32 R, float32 ToleranceFactor )
{
	//Compute a rough value for the integral
	float32 TotalIntegral = LegendreGaussQuadrature( pIntegrand, pIntegrandData, L, R );
	float32 Result = AdaptiveIntegrateAux( pIntegrand, pIntegrandData, L, R, TotalIntegral, ToleranceFactor, 0 );
	return Result;
}

} // namespace Math
} // namespace WT
