/*
	LinearInterpolator.h

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
class CLinearKey
{
public:
	CLinearKey(float32 ATime, const ValueType& AValue) : Time(ATime), Value(AValue)
	{
	}
	CLinearKey ()
	{
	}

	float32		Time;
	ValueType	Value;
};


//
// float32 typed key, Interpolator
//

typedef CLinearKey<float32> CFloat32LinearKey;
typedef CKeyInterpolator<float32, CFloat32LinearKey> CFloat32LinearInterpolator;


//
// CVector2f typed key, Interpolator
//

typedef CLinearKey<CVector2f> CVector2fLinearKey;
typedef CKeyInterpolator<CVector2f, CVector2fLinearKey> CVector2fLinearInterpolator;


//
// CVector3f typed key, Interpolator
//

typedef CLinearKey<CVector3f> CVector3fLinearKey;
typedef CKeyInterpolator<CVector3f, CVector3fLinearKey> CVector3fLinearInterpolator;


//
// CVector4f typed key, Interpolator
//

typedef CLinearKey<CVector4f> CVector4fLinearKey;
typedef CKeyInterpolator<CVector4f, CVector4fLinearKey> CVector4fLinearInterpolator;


//
// CQuaternion linear key, Interpolator
//

typedef CLinearKey<CQuaternion> CQuatLinearKey;
typedef CKeyInterpolator<CQuaternion, CQuatLinearKey> CQuatLinearInterpolator;


} // namespace Animation
} // namespace Graphics
} // namespace WT
