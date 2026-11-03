//
// Frustum.cpp
//
// Copyright (C) 2005 WildTangent, Inc.
// All Rights Reserved
//

#include "WT/stdafx.h"

#include "WT/System/Platform.h"
#include "WT/Graphics/Frustum.h"
#include "WT/Graphics/Camera.h"
#include "WT/Math/Utils/MathUtil.h"

using namespace WT::System;
using namespace WT::Math;


namespace WT
{
namespace Graphics
{

void CFrustum::Update( CCamera* Camera )
{
	//Compute frustum verts
	m_Verts[EBoxVert::XNegYNegZNeg] = Camera->TransformScreenToWorld( CVector3f( -1.0f, +1.0f, 0.0f ) );
	m_Verts[EBoxVert::XPosYNegZNeg] = Camera->TransformScreenToWorld( CVector3f( +1.0f, +1.0f, 0.0f ) );
	m_Verts[EBoxVert::XNegYPosZNeg] = Camera->TransformScreenToWorld( CVector3f( -1.0f, -1.0f, 0.0f ) );
	m_Verts[EBoxVert::XPosYPosZNeg] = Camera->TransformScreenToWorld( CVector3f( +1.0f, -1.0f, 0.0f ) );
	m_Verts[EBoxVert::XNegYNegZPos] = Camera->TransformScreenToWorld( CVector3f( -1.0f, +1.0f, 1.0f ) );
	m_Verts[EBoxVert::XPosYNegZPos] = Camera->TransformScreenToWorld( CVector3f( +1.0f, +1.0f, 1.0f ) );
	m_Verts[EBoxVert::XNegYPosZPos] = Camera->TransformScreenToWorld( CVector3f( -1.0f, -1.0f, 1.0f ) );
	m_Verts[EBoxVert::XPosYPosZPos] = Camera->TransformScreenToWorld( CVector3f( +1.0f, -1.0f, 1.0f ) );

	//Compute frustum planes
	CMatrix4f const& WorldToScreen = Camera->GetWorldToScreen();

	m_Planes[EBoxPlane::XNeg].Normal.x = -WorldToScreen.RowX.x - WorldToScreen.RowX.w; 
	m_Planes[EBoxPlane::XNeg].Normal.y = -WorldToScreen.RowY.x - WorldToScreen.RowY.w; 
	m_Planes[EBoxPlane::XNeg].Normal.z = -WorldToScreen.RowZ.x - WorldToScreen.RowZ.w; 
	m_Planes[EBoxPlane::XNeg].Distance = -WorldToScreen.RowT.x - WorldToScreen.RowT.w;

	m_Planes[EBoxPlane::XPos].Normal.x =  WorldToScreen.RowX.x - WorldToScreen.RowX.w; 
	m_Planes[EBoxPlane::XPos].Normal.y =  WorldToScreen.RowY.x - WorldToScreen.RowY.w; 
	m_Planes[EBoxPlane::XPos].Normal.z =  WorldToScreen.RowZ.x - WorldToScreen.RowZ.w; 
	m_Planes[EBoxPlane::XPos].Distance =  WorldToScreen.RowT.x - WorldToScreen.RowT.w;

	m_Planes[EBoxPlane::YNeg].Normal.x =  WorldToScreen.RowX.y - WorldToScreen.RowX.w; 
	m_Planes[EBoxPlane::YNeg].Normal.y =  WorldToScreen.RowY.y - WorldToScreen.RowY.w; 
	m_Planes[EBoxPlane::YNeg].Normal.z =  WorldToScreen.RowZ.y - WorldToScreen.RowZ.w; 
	m_Planes[EBoxPlane::YNeg].Distance =  WorldToScreen.RowT.y - WorldToScreen.RowT.w;

	m_Planes[EBoxPlane::YPos].Normal.x = -WorldToScreen.RowX.y - WorldToScreen.RowX.w; 
	m_Planes[EBoxPlane::YPos].Normal.y = -WorldToScreen.RowY.y - WorldToScreen.RowY.w; 
	m_Planes[EBoxPlane::YPos].Normal.z = -WorldToScreen.RowZ.y - WorldToScreen.RowZ.w; 
	m_Planes[EBoxPlane::YPos].Distance = -WorldToScreen.RowT.y - WorldToScreen.RowT.w;

	m_Planes[EBoxPlane::ZNeg].Normal.x = -WorldToScreen.RowX.z; 
	m_Planes[EBoxPlane::ZNeg].Normal.y = -WorldToScreen.RowY.z; 
	m_Planes[EBoxPlane::ZNeg].Normal.z = -WorldToScreen.RowZ.z; 
	m_Planes[EBoxPlane::ZNeg].Distance = -WorldToScreen.RowT.z;

	m_Planes[EBoxPlane::ZPos].Normal.x =  WorldToScreen.RowX.z - WorldToScreen.RowX.w; 
	m_Planes[EBoxPlane::ZPos].Normal.y =  WorldToScreen.RowY.z - WorldToScreen.RowY.w; 
	m_Planes[EBoxPlane::ZPos].Normal.z =  WorldToScreen.RowZ.z - WorldToScreen.RowZ.w; 
	m_Planes[EBoxPlane::ZPos].Distance =  WorldToScreen.RowT.z - WorldToScreen.RowT.w;

	//Normalize frustum planes
	for (int32 i = 0; i < EBoxPlane::Count; ++i)
	{
		m_Planes[i].Normalize();
	}

	//Compute frustum bounding box
	m_BoundingBox.Reset();
	for (uint32 i = 0; i < EBoxVert::Count; ++i)
	{
		m_BoundingBox.ExtendToEnclose( m_Verts[i] );
	}

	//Compute frustum bounding sphere.
	CVector3f const Center = Camera->TransformScreenToWorld( CVector3f( 0.0f, 0.0f, 0.5f ) );

	//Frustum can have strange shape (e.g. -NearClip > FarClip), so check all points to 
	//determine radius.
	float32 MaxR2 = 0.0f;
	for (uint32 i = 0; i < EBoxVert::Count; ++i)
	{
		float32 const R2 = Center.DistanceBetweenSquared( m_Verts[i] );
		if (MaxR2 < R2)
		{
			MaxR2 = R2;
		}
	}

	m_BoundingSphere.Set( Center.x, Center.y, Center.z, sqrtf( MaxR2 ) );
}

bool CFrustum::Contains( CVector3f const& Point ) const
{
	if (!m_BoundingSphere.IsInside( Point ))
	{
		return false;
	}

	if (!m_BoundingBox.ContainsPoint( Point ))
	{
		return false;
	}

	for (uint32 i = 0; i < EBoxPlane::Count; ++i)
	{
		if (!m_Planes[i].IsInside( Point ))
		{
			return false;
		}
	}

	return true;
}

} //namespace Graphics
} //namespace WT
