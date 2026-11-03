//
//	Timer.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
// Overview:
//	This class provides timer services using various hardware services. It can measure time in either 
//	high accuracy or medium accuracy depending on what hardware resources are available on the computer.
//	All times are returned in milliseconds.
//
#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace System
{

// Precision types for the CTimer class
namespace ETimerResolution
{
	enum Resolution
	{
		High,
		Medium,
	};
}

class CTimer
{
public:

	CTimer();
	CTimer( ETimerResolution::Resolution TimerResolution );
	virtual ~CTimer();

	// Starts or resumes the timer
	// If the timer is currently active or stopped, it will be reset; paused timers be be resumed
	void Start();

	// Halts the timer, resetting all internal timer data
	void Stop();

	// Temporarily halts the timer
	// Subsequent calls to GetTotalTime will return the time at the point the timer was paused
	// Subsequent calls to GetElapsedTime will return 0 while the timer is paused
	// The timer can be later resumed by a call to Start
	// This method returns the current time (in milliseconds) at which the timer was paused
	int64 Pause();

	// Gets the time elapsed delta (in milliseconds) since the previous call to GetElapsedTime
	// of GetElapsedTimeSeconds
	// For paused or stopped timers, this method returns 0
	int64 GetElapsedTime();

	// Gets the time elapsed delta (in seconds) since the previous call to GetElapsedTime or
	// GetElapsedTimeSeconds
	// For paused or stopped timers, this method returns 0
	float32 GetElapsedTimeSeconds();

	// Gets the total time elapsed (in milliseconds) since the timer was started
	// For stopped timers, this method returns 0
	int64 GetTotalTime();

	// Gets the total time elapsed (in seconds) since the timer was started
	// For stopped timers, this method returns 0
	float32 GetTotalTimeSeconds();

	// Retrieves the resolution of the timer
	ETimerResolution::Resolution GetResolution() const;

private:	// methods

    // Sets the resolution of the timer.  This can only be specified at construction
	void SetResolution( const ETimerResolution::Resolution Res );

    // Retrieves the timer frequency based on the sent resolution
	int64 GetTimerFrequency( const ETimerResolution::Resolution Res );

    // Tests whether the rdtsc instruction is present
    bool IsRDTSCAvailable( );
    // Tests whether the rdtsc instruction is present and behaving like we expect
    bool IsRDTSCWorkingOK( );
    
	// Calculates the elapsed time since the given starting time
	int64 CalculateElapsedTime( int64 StartTime );

private:	// types

	typedef void (*GetTimeStamp)(int64* pResult);

	enum TimerState
	{
		EStopped  = 0,
		EActive   = 1,
		EPaused	  = 2,
	};

private:    // data

	ETimerResolution::Resolution m_Resolution;		  // the resolution of the timer
	GetTimeStamp		m_TimeStampFunc;              // pointer to GetTimeStamp function
	TimerState			m_State;					  // current state of the timer

	int64				m_StartTicks;                 // absolute time the timer was started
	int64				m_LastElapsedTicks;           // marker time set during each call to GetElapsedTime

	int64				m_PausedTicks;				  // the time the timer was paused
	int64				m_PausedTime;				  // the time the timer was paused
	int64				m_TotalPausedTicks;			  // total time spent in paused state

	// For all timers
	static int64		m_Frequencies[3];             // low, med, high

	// For high precision performance counting
	static bool			m_RtdscTested;                // flags whether the rtdsc instruction has been tested

	// For high and medium-performance counting
	float64				m_FrequencyInverse;           // 1000. over the number of frequency polls per second
};


}	// namespace System
}	// namespace WT
