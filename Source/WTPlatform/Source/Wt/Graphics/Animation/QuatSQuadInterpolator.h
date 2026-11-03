/*
	QuatSQuadInterpolator.h

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

class CQuatSQuadKey
{
public:
	CQuatSQuadKey(float32 ATime, const CQuaternion& AValue, const CQuaternion& AQuadrangleCorner)
		: Time(ATime), Value(AValue), QuadrangleCorner(AQuadrangleCorner)
	{
	}

	CQuatSQuadKey ()
	{
	}

	float32		Time;
	CQuaternion	Value;
	CQuaternion	QuadrangleCorner;	
};

typedef CKeyInterpolator<CQuaternion, CQuatSQuadKey> CQuatSQuadInterpolator;


} // namespace Animation
} // namespace Graphics
} // namespace WT

//Include specialized member functions
#include "WT/Graphics/Animation/QuatSQuadInterpolatorImpl.h"
