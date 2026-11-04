/*
	QuatLerpInterpolatorImpl.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/Graphics/Animation/QuatLerpInterpolator.h"
#include "WT/Math/Interpolation/LinearInterpolator.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

// Override the CKeyInterpolator base-class private Interpolate method
inline void CQuatLerpInterpolator::Interpolate(float32 BlendFactor, const CQuatLerpKey& Key1, const CQuatLerpKey& Key2, CQuaternion& Result) const
{
	Math::LinearInterpolate(BlendFactor, Key1.Value, Key2.Value, Result);
	Result.Normalize();
}


} // namespace Animation
} // namespace Graphics
} // namespace WT
