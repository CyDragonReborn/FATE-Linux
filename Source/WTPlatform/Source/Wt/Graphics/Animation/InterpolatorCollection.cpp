/*
	InterpolatorCollection.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author: Paul Higinbotham (based on Chimera cmInterpolatorCollection class)
	7/8/2005
*/

#include "WT/stdafx.h"
#include "WT/Graphics/Animation/InterpolatorCollection.h"

using namespace WT::System;
using std::vector;

namespace WT
{
namespace Graphics
{
namespace Animation
{

WT_DEFINE_CLASS(CInterpolatorCollection)

//
// Destructor
// 

CInterpolatorCollection::~CInterpolatorCollection()
{
	// Destroy all interpolator objects in collection
	vector<IInterpolator *>::iterator it = m_Array.begin();
	while (it != m_Array.end())
	{
		IInterpolator * pInterpolator = *it;
		delete pInterpolator;
		++it;
	}
}


//
// Accessors
// 

inline IInterpolator * CInterpolatorCollection::GetItem(uint32 Index)
{
	if ( Index < static_cast<uint32>(m_Array.size()) )
	{
		return m_Array[Index];
	}

	return NULL;
}

inline const IInterpolator * CInterpolatorCollection::GetItem(uint32 Index) const
{
	if ( Index < static_cast<uint32>(m_Array.size()) )
	{
		return m_Array[Index];
	}

	return NULL;
}


//
// Update interpolator objects
//

inline void CInterpolatorCollection::Update(float32 Time)
{
	vector<IInterpolator *>::iterator it = m_Array.begin();
	while (it != m_Array.end())
	{
		IInterpolator * pInterpolator = *it;
		pInterpolator->Update(Time);
	}
}


//
// Interpolator object creation
//

inline CFloat32BezierInterpolator * CInterpolatorCollection::AddFloat32BezierInterpolator()
{
	CFloat32BezierInterpolator * pInterpolator = new CFloat32BezierInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}
inline CVector2fBezierInterpolator * CInterpolatorCollection::AddVector2fBezierInterpolator()
{
	CVector2fBezierInterpolator * pInterpolator = new CVector2fBezierInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}
inline CVector3fBezierInterpolator * CInterpolatorCollection::AddVector3fBezierInterpolator()
{
	CVector3fBezierInterpolator * pInterpolator = new CVector3fBezierInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}
inline CVector4fBezierInterpolator * CInterpolatorCollection::AddVector4fBezierInterpolator()
{
	CVector4fBezierInterpolator * pInterpolator = new CVector4fBezierInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}

inline CFloat32HermiteInterpolator * CInterpolatorCollection::AddFloat32HermiteInterpolator()
{
	CFloat32HermiteInterpolator * pInterpolator = new CFloat32HermiteInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}
inline CVector2fHermiteInterpolator * CInterpolatorCollection::AddVector2fHermiteInterpolator()
{
	CVector2fHermiteInterpolator * pInterpolator = new CVector2fHermiteInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}
inline CVector3fHermiteInterpolator * CInterpolatorCollection::AddVector3fHermiteInterpolator()
{
	CVector3fHermiteInterpolator * pInterpolator = new CVector3fHermiteInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}
inline CVector4fHermiteInterpolator * CInterpolatorCollection::AddVector4fHermiteInterpolator()
{
	CVector4fHermiteInterpolator * pInterpolator = new CVector4fHermiteInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}

inline CQuatLerpInterpolator * CInterpolatorCollection::AddQuatLerpInterpolator()
{
	CQuatLerpInterpolator * pInterpolator = new CQuatLerpInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}

inline CQuatSLerpInterpolator * CInterpolatorCollection::AddQuatSLerpInterpolator()
{
	CQuatSLerpInterpolator * pInterpolator = new CQuatSLerpInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}

inline CQuatSQuadInterpolator * CInterpolatorCollection::AddQuatSQuadInterpolator()
{
	CQuatSQuadInterpolator * pInterpolator = new CQuatSQuadInterpolator();
	m_Array.push_back(pInterpolator);

	return pInterpolator;
}


} // Animation
} // Graphics
} // WT
