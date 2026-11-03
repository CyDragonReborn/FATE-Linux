/*
	HermiteInterpolatorImpl.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/HermiteInterpolator.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

template<typename T>
void HermiteRecompute(vector<CHermiteKey<T> >& Keys, bool RecomputeAsAClosedLoop);

//
// Override the CKeyInterpolator base-class private Interpolate method
//

inline void CFloat32HermiteInterpolator::Interpolate(float32 BlendFactor, const CFloat32HermiteKey& Key1, const CFloat32HermiteKey& Key2, float32& Result) const
{
	Math::HermiteInterpolate(BlendFactor, Key1.Value, Key1.OutgoingTangent, Key2.Value, Key2.IncomingTangent, Result);
}

inline void CVector2fHermiteInterpolator::Interpolate(float32 BlendFactor, const CVector2fHermiteKey& Key1, const CVector2fHermiteKey& Key2, CVector2f& Result) const
{
	Math::HermiteInterpolate(BlendFactor, Key1.Value, Key1.OutgoingTangent, Key2.Value, Key2.IncomingTangent, Result);
}

inline void CVector3fHermiteInterpolator::Interpolate(float32 BlendFactor, const CVector3fHermiteKey& Key1, const CVector3fHermiteKey& Key2, CVector3f& Result) const
{
	Math::HermiteInterpolate(BlendFactor, Key1.Value, Key1.OutgoingTangent, Key2.Value, Key2.IncomingTangent, Result);
}

inline void CVector4fHermiteInterpolator::Interpolate(float32 BlendFactor, const CVector4fHermiteKey& Key1, const CVector4fHermiteKey& Key2, CVector4f& Result) const
{
	Math::HermiteInterpolate(BlendFactor, Key1.Value, Key1.OutgoingTangent, Key2.Value, Key2.IncomingTangent, Result);
}


//
// Override the CKeyInterpolator base-class private InternalRecompute method
//

inline void CFloat32HermiteInterpolator::InternalRecompute()
{
	HermiteRecompute<float32>(m_Keys, m_RecomputeAsAClosedLoop);
}

inline void CVector2fHermiteInterpolator::InternalRecompute()
{
	HermiteRecompute<CVector2f>(m_Keys, m_RecomputeAsAClosedLoop);
}

inline void CVector3fHermiteInterpolator::InternalRecompute()
{
	HermiteRecompute<CVector3f>(m_Keys, m_RecomputeAsAClosedLoop);
}

inline void CVector4fHermiteInterpolator::InternalRecompute()
{
	HermiteRecompute<CVector4f>(m_Keys, m_RecomputeAsAClosedLoop);
}


// compute the incoming and outgoing derivatives at each keyframe
template<typename T>
void HermiteRecompute(vector<CHermiteKey<T> >& Keys, bool RecomputeAsAClosedLoop)
{ 
	int32 KeyCount; // Number of Keys
	bool Looped = RecomputeAsAClosedLoop;

	// Compute derivatives at the keyframe points:
	// The derivative is the average of the source chord p[i]-p[i-1]
	// and the destination chord p[i+1]-p[i]
	//     (where i is Index1 in this function)
	//  D = 1/2 * ( p[i+1]-p[i-1] ) = 1/2 *( (p[i+1]-p[i]) + (p[i]-p[i-1]) )
	//  The very first and last chords are simply the 
	// destination and source derivative.
	//   These 'averaged' D's are adjusted for variences in the time scale
	// between the Keyframes.  To do this, the derivative at each keyframe
	// is split into two parts, an incoming ('source' DS) 
	// and an outgoing ('destination' DD) derivative.
	// DD[i] = DD[i] * 2 * N[i]  / ( N[i-1] + N[i] )   
	// DS[i] = DS[i] * 2 * N[i-1]/ ( N[i-1] + N[i] )
	//    where N[i] is time between keyframes i and i+1
	// Since the chord dealt with on a given chord between key[i] and key[i+1], only
	// one of the derivates are needed for each keyframe.  For key[i] the outgoing
	// derivative at is needed (DD[i]).  For key[i+1], the incoming derivative
	// is needed (DS[i+1])   ( note that  (1/2) * 2 = 1 )

	KeyCount = static_cast<int32>(Keys.size());
	
	if (KeyCount == 0)
		return;  //nothing to do

	if (KeyCount < 3)
	{
		Looped = false;	
		// cant compute slopes without a closed loop: 
		// so compute slopes as if it is not closed.
	}
	    	
	for (int32 i=0; i < KeyCount; i++)
	{
		int32 Index0 = i-1;
		int32 Index1 = i;
		int32 Index2 = i+1;
		float32 Time0;                          // Time at key[n-1] (Time at Key[Index0])
		float32 Time2;                          // Time at key[n+1] (Time at Key[Index2])
		float32 Time1 = Keys[Index1].Time;      // Time at key[n]   (Time at Key[Index1])

		if (Index1 == 0)
		{
			if (!Looped)
			{
				Index0 = 0;			
				Time0 = Keys[Index0].Time;
			}
			else
			{
				Index0 = KeyCount-2;
				Time0 = Time1 - (Keys[KeyCount-1].Time - Keys[KeyCount-2].Time);
			}
		}
		else
		{
			Time0 =  Keys[Index0].Time;
		}


		if (Index2 == KeyCount)
		{
			if (!Looped)
			{
				Index2 = KeyCount-1;
				Time2 = Keys[Index2].Time;
			}
			else
			{
				Index2 = 1;
				Time2 = Time1 + (Keys[1].Time - Keys[0].Time);
			}
		}
		else
		{
			Time2 = Keys[Index2].Time;
		}

		float32 N0 = (Time1 - Time0);    // N[n-1]; or time between keyframes n-1 and n
		float32 N1 = (Time2 - Time1);    // N[n];  or time between keyframes n and n+1

		if ( ( (!Looped) && (Index1 == 0)) || ( N0 <= KeyCutTime ) )
		{
			Keys[i].OutgoingTangent = Keys[i].IncomingTangent = 
				Keys[Index2].Value - Keys[Index1].Value;
		}
		else if ( ( (!Looped) && (Index1 == KeyCount-1) ) || (N1 <= KeyCutTime ) )
		{
			Keys[i].OutgoingTangent = Keys[i].IncomingTangent = 
				Keys[Index1].Value - Keys[Index0].Value;
		}
		else
		{
			T Slope = (Keys[Index2].Value - Keys[Index0].Value) / (N0 + N1);
			Keys[i].OutgoingTangent = Slope * N1;
			Keys[i].IncomingTangent = Slope * N0;
		}
	}	
}

} // namespace Animation
} // namespace Graphics
} // namespace WT
