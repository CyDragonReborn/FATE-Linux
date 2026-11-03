/*
	HermiteInterpolator.h

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
class CHermiteKey
{
public:
	CHermiteKey(float32 ATime, const ValueType& AValue, const ValueType& AIncomingTangent, const ValueType& AOutgoingTangent)
		: Time(ATime), Value(AValue), IncomingTangent(AIncomingTangent)
		, RightValue(AOutgoingTangent)
	{
	}

	CHermiteKey ()
	{
	}

	float32		Time;
	ValueType	Value;
	ValueType	IncomingTangent;
	ValueType	OutgoingTangent;
};


//
// float32 typed key, Interpolator
//

typedef CHermiteKey<float32> CFloat32HermiteKey;
typedef CKeyInterpolator<float32, CFloat32HermiteKey> CFloat32HermiteInterpolator;


//
// CVector2f typed key, Interpolator
//

typedef CHermiteKey<Math::CVector2f> CVector2fHermiteKey;
typedef CKeyInterpolator<Math::CVector2f, CVector2fHermiteKey> CVector2fHermiteInterpolator;


//
// CVector3f typed key, Interpolator
//

typedef CHermiteKey<Math::CVector3f> CVector3fHermiteKey;
typedef CKeyInterpolator<Math::CVector3f, CVector3fHermiteKey> CVector3fHermiteInterpolator;


//
// CVector4f typed key, Interpolator
//

typedef CHermiteKey<Math::CVector4f> CVector4fHermiteKey;
typedef CKeyInterpolator<Math::CVector4f, CVector4fHermiteKey> CVector4fHermiteInterpolator;


} // namespace Animation
} // namespace Graphics
} // namespace WT

//Include specialized member functions
#include "WT/Graphics/Animation/HermiteInterpolatorImpl.h"
