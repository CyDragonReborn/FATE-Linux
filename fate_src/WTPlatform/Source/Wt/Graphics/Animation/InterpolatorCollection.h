/*
	InterpolatorCollection.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author: Paul Higinbotham (based on Chimera cmInterpolatorCollection class)
	7/8/2005
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/IInterpolator.h"
#include "WT/Graphics/Animation/LinearInterpolator.h"
#include "WT/Graphics/Animation/BezierInterpolatorImpl.h"
#include "WT/Graphics/Animation/HermiteInterpolatorImpl.h"
#include "WT/Graphics/Animation/QuatLerpInterpolatorImpl.h"
#include "WT/Graphics/Animation/QuatSLerpInterpolatorImpl.h"
#include "WT/Graphics/Animation/QuatSQuadInterpolatorImpl.h"
#include <vector>


namespace WT
{
namespace Graphics
{
namespace Animation
{

//
// Notes
// This is a simple collection class to hold interpolator objects.  The purpose of
// this class is to:
//   1)  Provide methods to create the different interpolator object types.
//   2)  Update all interpolator objects in collection with a single method.
//   3)  Own the interpolator objects:
//       !!! All interpolator objects contained in this class are destroyed when this
//           class is destroyed !!!
//

typedef System::SmartPointer<class CInterpolatorCollection> HInterpolatorCollection;
class CInterpolatorCollection : public System::CObject
{
public:
	WT_CLASS(CInterpolatorCollection, System::CObject)

	CInterpolatorCollection() {}
	virtual ~CInterpolatorCollection();

	// Accessors
	IInterpolator * operator[] (uint32 Index) { return GetItem(Index); }
	const IInterpolator * operator[] (uint32 Index) const { return GetItem(Index); }
	inline IInterpolator * GetItem(uint32 Index);
	inline const IInterpolator * GetItem(uint32 Index) const;

	// Interpolator creation methods
	inline CFloat32BezierInterpolator * AddFloat32BezierInterpolator();
	inline CVector2fBezierInterpolator * AddVector2fBezierInterpolator();
	inline CVector3fBezierInterpolator * AddVector3fBezierInterpolator();
	inline CVector4fBezierInterpolator * AddVector4fBezierInterpolator();

	inline CFloat32HermiteInterpolator * AddFloat32HermiteInterpolator();
	inline CVector2fHermiteInterpolator * AddVector2fHermiteInterpolator();
	inline CVector3fHermiteInterpolator * AddVector3fHermiteInterpolator();
	inline CVector4fHermiteInterpolator * AddVector4fHermiteInterpolator();

	inline CQuatLerpInterpolator * AddQuatLerpInterpolator();

	inline CQuatSLerpInterpolator * AddQuatSLerpInterpolator();

	inline CQuatSQuadInterpolator * AddQuatSQuadInterpolator();

	inline void Update(float32 Time);

private:
	std::vector<IInterpolator *> m_Array;
};


} // namespace Animation
} // namespace Graphics
} // namespace WT
