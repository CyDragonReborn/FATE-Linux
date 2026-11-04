/*
	Timeline.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/

#include "WT/stdafx.h"
#include "WT/Graphics/Animation/Timeline.h"

using namespace WT::System;


namespace WT
{
namespace Graphics
{
namespace Animation
{

WT_DEFINE_CLASS(CTimeline)

CTimeline::CTimeline() : m_pInterpolators(NULL)
{
	m_Playing = false;
	m_Reverse = false;
	m_Speed = 1.0f;
}

CTimeline::CTimeline(CInterpolatorCollection * pInterpolators) : m_pInterpolators(pInterpolators)
{
	m_Playing = false;
	m_Reverse = false;
	m_Speed = 1.0f;
}

CTimeline::~CTimeline()
{
}

void CTimeline::Animate(float32 FrameTime)
{
	if (!m_Playing)
		return;

	float32 NewPosition = 0;
	if (!m_Reverse)
		 NewPosition = m_CurrentPosition + FrameTime * m_Speed;
	else
		m_CurrentPosition -= FrameTime * m_Speed;

	m_pInterpolators->Update(NewPosition);
	m_CurrentPosition = NewPosition;
}


} // namespace Animation
} // namespace Graphics
} // namespace WT
