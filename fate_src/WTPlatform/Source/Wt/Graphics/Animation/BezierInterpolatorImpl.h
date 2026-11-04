/*
	BezierInterpolatorImpl.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/BezierInterpolator.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

// Override the CKeyInterpolator base-class private Interpolate method for Bezier interpolation

inline void CFloat32BezierInterpolator::Interpolate(float32 BlendFactor, const CFloat32BezierKey& Key1 , const CFloat32BezierKey& Key2, float32& Result) const
{
	Math::BezierInterpolate(BlendFactor, Key1.Value, Key1.RightValue, Key2.LeftValue, Key2.Value, Result);
}

inline void CVector2fBezierInterpolator::Interpolate(float32 BlendFactor, const CVector2fBezierKey& Key1, const CVector2fBezierKey& Key2, CVector2f& Result) const
{
	Math::BezierInterpolate(BlendFactor, Key1.Value, Key1.RightValue, Key2.LeftValue, Key2.Value, Result);
}

inline void CVector3fBezierInterpolator::Interpolate(float32 BlendFactor, const CVector3fBezierKey& Key1, const CVector3fBezierKey& Key2, CVector3f& Result) const
{
	Math::BezierInterpolate(BlendFactor, Key1.Value, Key1.RightValue, Key2.LeftValue, Key2.Value, Result);
}

inline void CVector4fBezierInterpolator::Interpolate(float32 BlendFactor, const CVector4fBezierKey& Key1, const CVector4fBezierKey& Key2, CVector4f& Result) const
{
	Math::BezierInterpolate(BlendFactor, Key1.Value, Key1.RightValue, Key2.LeftValue, Key2.Value, Result);
}


} // namespace Animation
} // namespace Graphics
} // namespace WT
