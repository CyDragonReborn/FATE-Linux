/*
	QuatSQuadInterpolatorImpl.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/Graphics/Animation/QuatSQuadInterpolator.h"
#include "WT/Math/Quaternion/Quaternion.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

// Override the CKeyInterpolator base-class private Interpolate method
inline void CQuatSQuadInterpolator::Interpolate(float32 BlendFactor, const CQuatSQuadKey& Key1, const CQuatSQuadKey& Key2, CQuaternion& Result) const
{
	CQuaternion SLerp1(false), SLerp2(false);

	SLerp1.SlerpNotShortest( Key1.Value, Key2.Value, BlendFactor );
	SLerp2.SlerpNotShortest( Key1.QuadrangleCorner, Key2.QuadrangleCorner, BlendFactor);
	Result.SlerpNotShortest( SLerp1, SLerp2, (2.0f * BlendFactor * (1.0f - BlendFactor)) );
}

// compute quadrangle corner for a keyframe containing Q1.
// Q0 and Q2 are the quaternions for the previous and next keyframes 
// corner is the newly computed quaternion
inline void FrameQuadrangleCorner(const CQuaternion& Q0, const CQuaternion& Q1, const CQuaternion& Q2, CQuaternion& Corner)
{
	Corner = (((Q2 / Q1).Ln() + (Q0 / Q1).Ln())* -0.25f).Exp() * Q1;
}

// Override the CKeyInterpolator base-class private InternalRecompute method
inline void CQuatSQuadInterpolator::InternalRecompute()
{
	int32 const KeyCount = static_cast<int32>(m_Keys.size()); // Number of Keys
	bool Looped = m_RecomputeAsAClosedLoop;

	for (int32 i = 1; i < KeyCount; i++)
	{
		if (m_Keys[i - 1].Value.Dot(m_Keys[i].Value) < 0.0f)
		{
			// negative half-sphere of Keys[i] is straight-line closer to Keys[i - 1] than positive half-sphere, 
			// so switch spheres - invert ALL components of Key[i]
			m_Keys[i].Value.Negate();
		}
	}

	if (KeyCount<3)
	{
		Looped = false;
		// cant compute 'slopes' without enough points to loop. 
		// so treat path as non-looped.
	}

	for (i =0; i< KeyCount; i++)
	{
		int32 Index0 = i-1;
		int32 Index1 = i;
		int32 Index2 = i+1;
		float32 Time0;                          // Time at key[n-1] (Time at Key[Index0])
		float32 Time2;                          // Time at key[n+1] (Time at Key[Index2])
		float32 Time1 = m_Keys[Index1].Time;    // Time at key[n]   (Time at Key[Index1])

		if (Index1 == 0)
		{
			if (!Looped)
			{
				Index0 = 0;
				Time0 = m_Keys[Index0].Time;
			}
			else
			{
				Index0 = KeyCount-2;
				Time0 = Time1 - (m_Keys[KeyCount-1].Time - m_Keys[KeyCount-2].Time);
			}
		}
		else
		{
			Time0 =  m_Keys[Index0].Time;
		}

		if (Index2 == KeyCount)
		{
			if (!Looped)
			{
				Index2 = KeyCount-1;
				Time2 = m_Keys[Index2].Time;
			}
			else
			{
				Index2 = 1;
				Time2 = Time1 + (m_Keys[1].Time - m_Keys[0].Time);
			}
		}
		else
		{
			Time2 = m_Keys[Index2].Time;
		}

		float32 N0 = (Time1 - Time0);    // N[n-1]; or time between keyframes n-1 and n
		float32 N1 = (Time2 - Time1);    // N[n];  or time between keyframes n and n+1

		if ( ( (!Looped) && (Index1 == 0)) || ( N0 <= KeyCutTime ) )
		{
			m_Keys[i].QuadrangleCorner = m_Keys[i].Value;
		}
		else if ( ( (!Looped) && (Index1 == KeyCount-1) ) || (N1 <= KeyCutTime ) )
		{
			m_Keys[i].QuadrangleCorner = m_Keys[i].Value;
		}
		else
		{
			FrameQuadrangleCorner ( m_Keys[Index0].Value, m_Keys[Index1].Value, m_Keys[Index2].Value, m_Keys[i].QuadrangleCorner );
		}
	}	
}


} // namespace Animation
} // namespace Graphics
} // namespace WT
