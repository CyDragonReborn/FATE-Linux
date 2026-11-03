/*
	QuatLerpInterpolator.h

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

class CQuatLerpKey
{
public:
	CQuatLerpKey(float32 ATime, const CQuaternion& AValue)
		: Time(ATime), Value(AValue)
	{
	}

	CQuatLerpKey ()
	{
	}

	float32		Time;
	CQuaternion	Value;
};


typedef CKeyInterpolator<CQuaternion, CQuatLerpKey> CQuatLerpInterpolator;


} // namespace Animation
} // namespace Graphics
} // namespace WT

//Include specialized member functions
#include "WT/Graphics/Animation/QuatLerpInterpolatorImpl.h"
