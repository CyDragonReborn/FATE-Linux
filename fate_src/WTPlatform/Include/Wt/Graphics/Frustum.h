//
// Frustum.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "d3d9.h"
#include "WT/Math/Math.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Math/LinearAlgebra/Plane.h"
#include "WT/Math/Objects/AABox.h"
#include "WT/Math/Rects/Rect2i.h"

namespace WT
{
namespace Graphics
{

class CCamera;

/// A Frustum is a truncated rectangular pyramid consisting of 8 vertices and 6 faces.
/// The viewable volume of a camera is represented using a Frustum.
/// TODO additional intersect checks for Frustum are needed.

/// A vertex in a box or frustum.
//Note that the "Neg" and "Pos" designations refer to axis directions in camera space.
//YNeg, the bottom plane, actually corresponds to a postive screen y axis
//direction; while YPos, the top plane, corresponds to a negative screen y
//axis direction.
namespace EBoxVert
{
	enum Type
	{
		XNegYNegZNeg, //Left bottom near vertex
		XPosYNegZNeg, //Right Bottom near vertex
		XNegYPosZNeg, //Left top near vertex
		XPosYPosZNeg, //Right top near vertex
		XNegYNegZPos, //Left bottom far vertex
		XPosYNegZPos, //Right Bottom far vertex
		XNegYPosZPos, //Left top far vertex
		XPosYPosZPos, //Right top far vertex
		Count //The number of vertices in the frustum
	};
}

/// A plane in a box or frustum.
namespace EBoxPlane
{
	enum Type
	{
		XNeg, //Left frustum plane
		XPos, //Right frustum plane
		YNeg, //Bottom frustum plane
		YPos, //Top frustum plane
		ZNeg, //Near frustum plane
		ZPos, //Far frustum plane
		Count //The number of planes in the frustum
	};
}

namespace EBoxIntersectFlags
{
	enum Type
	{
		XNeg = 1 << EBoxPlane::XNeg, //Intersection test passed for left plane
		XPos = 1 << EBoxPlane::XPos, //Intersection test passed for right plane
		YNeg = 1 << EBoxPlane::YNeg, //Intersection test passed for bottom plane
		YPos = 1 << EBoxPlane::YPos, //Intersection test passed for top plane
		ZNeg = 1 << EBoxPlane::ZNeg, //Intersection test passed for near plane
		ZPos = 1 << EBoxPlane::ZPos, //Intersection test passed for far plane
		All  = XNeg | XPos | YNeg | YPos | ZNeg | ZPos //Intersection test passed for each plane
	};
}

typedef System::SmartPointer<class CFrustum> HFrustum;
class CFrustum : public System::CObject
{
public:
	WT_CLASS( CFrustum, System::CObject );

	//Note: Neg and Pos refer to axis directions in a coordinate system
	//aligned with the frustum.

	Math::CVector3f	m_Verts[EBoxVert::Count]; //Vertices
	Math::CPlane	m_Planes[EBoxPlane::Count]; //Planes
	Math::CAABox	m_BoundingBox;
	Math::CSphere	m_BoundingSphere;

	/// Update planes and bounding box based on camera.
	void Update( CCamera* Camera );

	/// Check whether the specified point lies within the frustum.
	bool Contains( Math::CVector3f const& Point ) const;

	/// Get EBoxIntersectFlags intersection flags for the specified point.
	inline uint32 ComputeIntersectFlags( Math::CVector3f const& Point ) const;
};

/// Get EBoxIntersectFlags intersection flags for the specified point.
uint32 CFrustum::ComputeIntersectFlags( Math::CVector3f const& Point ) const
{
	uint32 Result = 0;

	for (uint32 i = 0; i < EBoxPlane::Count; ++i)
	{
		if (m_Planes[i].IsInside( Point ))
		{
			Result |= 1 << i;
		}
	}

	return Result;
}

} //namespace Graphics
} //namespace WT
