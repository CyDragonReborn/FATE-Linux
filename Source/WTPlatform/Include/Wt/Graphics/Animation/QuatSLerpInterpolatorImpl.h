/*
	QuatSLerpInterpolatorImpl.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/Graphics/Animation/QuatSLerpInterpolator.h"
#include "WT/Math/Quaternion/Quaternion.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

// Override the CKeyInterpolator base-class private Interpolate method
inline void CQuatSLerpInterpolator::Interpolate(float32 BlendFactor, const CQuatSLerpKey& Key1, const CQuatSLerpKey& Key2, CQuaternion& Result) const
{
	Result.SlerpNotShortest( Key1.Value, Key2.Value, BlendFactor);
}

// Override the CKeyInterpolator base-class private InternalRecompute method
inline void CQuatSLerpInterpolator::InternalRecompute()
{
	int32 const KeyCount = static_cast<int32>(m_Keys.size()); // Number of Keys

	for (int32 i = 1; i < KeyCount; i++)
	{
		if ( m_Keys[i - 1].Value.Dot(m_Keys[i].Value) < 0.0f )
		{
			// negative half-sphere of Keys[i] is straight-line closer to Keys[i - 1] than positive half-sphere, 
			// so switch spheres - invert ALL components of Key[i]
			m_Keys[i].Value.Negate();
		}
	}
}

} // namespace Animation
} // namespace Graphics
} // namespace WT
