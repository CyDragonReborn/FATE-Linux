/*
	BezierInterpolator.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/KeyInterpolatorImpl.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

template <typename ValueType>
class CBezierKey
{
public:
	CBezierKey(float32 ATime ,const ValueType& AValue, const ValueType& ALeftValue, const ValueType& ARightValue )
		: Time (ATime), Value ( AValue ), LeftValue(ALeftValue)
		, RightValue(ARightValue)
	{
	}

	CBezierKey ()
	{
	}

	float32		Time;
	ValueType	Value;
	ValueType	LeftValue;
	ValueType	RightValue;
};


//
// float32 typed key, Interpolator
//

typedef CBezierKey<float32> CFloat32BezierKey;
typedef CKeyInterpolator<float32, CFloat32BezierKey> CFloat32BezierInterpolator;


//
// CVector2f typed key, Interpolator
//

typedef CBezierKey<CVector2f> CVector2fBezierKey;
typedef CKeyInterpolator<CVector2f, CVector2fBezierKey> CVector2fBezierInterpolator;


//
// CVector3f typed key, Interpolator
//

typedef CBezierKey<CVector3f> CVector3fBezierKey;
typedef CKeyInterpolator<CVector3f, CVector3fBezierKey> CVector3fBezierInterpolator;


//
// CVector4f typed key, Interpolator
//

typedef CBezierKey<CVector4f> CVector4fBezierKey;
typedef CKeyInterpolator<CVector4f, CVector4fBezierKey> CVector4fBezierInterpolator;


} // namespace Animation
} // namespace Graphics
} // namespace WT

//Include specialized member functions
#include "WT/Graphics/Animation/BezierInterpolatorImpl.h"
