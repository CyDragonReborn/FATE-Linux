/*
	Timeline.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/InterpolatorCollection.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

typedef System::SmartPointer<class CTimeline> HTimeline;
class CTimeline : public System::CObject
{
public:
	WT_CLASS(CTimeline, System::CObject)

	CTimeline();
	CTimeline(CInterpolatorCollection * pInterpolators);
	virtual ~CTimeline();

	virtual CInterpolatorCollection * GetInterpolators()
	{
		return m_pInterpolators;
	}

	virtual void SetInterpolators(CInterpolatorCollection * pInterpolators)
	{
		m_pInterpolators = pInterpolators;
	}

	virtual void SetLength(float32 Time)
	{
		m_Length = Time;
	}

	virtual float32 GetLength()
	{
		return m_Length;
	}

	virtual void Play ()
	{
		m_Playing = true;
		m_Reverse = false;
	}

	virtual bool IsPlaying()
	{
		return m_Playing;
	}

	virtual void SetSpeed (float32 Speed)
	{
		m_Speed = Speed;
	}

	virtual float32 GetSpeed()
	{
		return m_Speed;
	}

	virtual void PlayReverse()
	{
		m_Playing = true;
		m_Reverse = true;
	}

	virtual void JumpTo(float32 Time)
	{
		m_CurrentPosition = Time;
		m_pInterpolators->Update(m_CurrentPosition);
	}

	virtual void Pause()
	{
		m_Playing = false;
	}

	virtual float32 GetPosition()
	{
		return m_CurrentPosition;
	}

	virtual void Animate(float32 FrameTime);

private:	// data

	CInterpolatorCollection *	m_pInterpolators;
	float32						m_Length;
	float32						m_CurrentPosition;
	bool						m_Playing;
	bool						m_Reverse;
	float32						m_Speed;
};


} // namespace Animation
} // namespace Graphics
} // namespace WT
