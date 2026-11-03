//
//	Timer2.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
// Overview:
//	This class provides timer services using the high frequency performance counter.
//  This class is less robust than the CTimer (no support for really old machines),
//  but it is faster and smaller.
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"

namespace WT
{
namespace System
{

class CTimer2 : public CObject
{
public:

	CTimer2();

	// resets the timer
	void Reset();

	// starts the timer
	void Start();

	// stop (or pause) the timer
	void Stop();

	// advance the timer by a given amount (default of 0.1 seconds)
	void Advance( float64 Value = 0.1f );

	// get the absolute system time
	float64 GetAbsoluteTime();

	// get the current time
	float64 GetTime();

	// get the time that elapsed between GetElapsedTime() calls
	float64 GetElapsedTime(); 

	// returns true if timer stopped
	bool IsStopped();

protected:

	bool m_UsingQPF;
	bool m_TimerStopped;
	int64 m_QPFTicksPerSec;

	int64 m_StopTime;
	int64 m_LastElapsedTime;
	int64 m_BaseTime;
};

}	// namespace System
}	// namespace WT
