//
//	Timer2.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/System/Object.h"
#include "WT/System/Timer2.h"

namespace WT
{
namespace System
{

CTimer2::CTimer2()
{
    m_UsingQPF         = false;
    m_TimerStopped     = true;
    m_QPFTicksPerSec  = 0;

    m_StopTime        = 0;
    m_LastElapsedTime = 0;
    m_BaseTime        = 0;

    // Use QueryPerformanceFrequency() to get frequency of timer.  
    LARGE_INTEGER qwTicksPerSec;
    m_UsingQPF = (bool) (QueryPerformanceFrequency( &qwTicksPerSec ) != 0);
    m_QPFTicksPerSec = qwTicksPerSec.QuadPart;
}


//--------------------------------------------------------------------------------------
void CTimer2::Reset()
{
    if( !m_UsingQPF )
        return;

    // Get either the current time or the stop time
    LARGE_INTEGER qwTime;
    if( m_StopTime != 0 )
        qwTime.QuadPart = m_StopTime;
    else
        QueryPerformanceCounter( &qwTime );

    m_BaseTime        = qwTime.QuadPart;
    m_LastElapsedTime = qwTime.QuadPart;
    m_StopTime        = 0;
    m_TimerStopped     = FALSE;
}


//--------------------------------------------------------------------------------------
void CTimer2::Start()
{
    if( !m_UsingQPF )
        return;

    // Get the current time
    LARGE_INTEGER qwTime;
    QueryPerformanceCounter( &qwTime );

    if( m_TimerStopped )
        m_BaseTime += qwTime.QuadPart - m_StopTime;
    m_StopTime = 0;
    m_LastElapsedTime = qwTime.QuadPart;
    m_TimerStopped = FALSE;
}


//--------------------------------------------------------------------------------------
void CTimer2::Stop()
{
    if( !m_UsingQPF )
        return;

    if( !m_TimerStopped )
    {
        // Get either the current time or the stop time
        LARGE_INTEGER qwTime;
        if( m_StopTime != 0 )
            qwTime.QuadPart = m_StopTime;
        else
            QueryPerformanceCounter( &qwTime );

        m_StopTime = qwTime.QuadPart;
        m_LastElapsedTime = qwTime.QuadPart;
        m_TimerStopped = TRUE;
    }
}


//--------------------------------------------------------------------------------------
void CTimer2::Advance( float64 Value )
{
    if( !m_UsingQPF )
        return;

    m_StopTime += Float64ToInt64 ( m_QPFTicksPerSec * Value );
}


//--------------------------------------------------------------------------------------
float64 CTimer2::GetAbsoluteTime()
{
    if( !m_UsingQPF )
        return -1.0;

    // Get either the current time or the stop time
    LARGE_INTEGER qwTime;
    if( m_StopTime != 0 )
        qwTime.QuadPart = m_StopTime;
    else
        QueryPerformanceCounter( &qwTime );

    float64 fTime = qwTime.QuadPart / (float64) m_QPFTicksPerSec;

    return fTime;
}


//--------------------------------------------------------------------------------------
float64 CTimer2::GetTime()
{
    if( !m_UsingQPF )
        return -1.0;

    // Get either the current time or the stop time
    LARGE_INTEGER qwTime;
    if( m_StopTime != 0 )
        qwTime.QuadPart = m_StopTime;
    else
        QueryPerformanceCounter( &qwTime );

    float64 fAppTime = (float64) ( qwTime.QuadPart - m_BaseTime ) / (float64) m_QPFTicksPerSec;

    return fAppTime;
}


//--------------------------------------------------------------------------------------
float64 CTimer2::GetElapsedTime()
{
    if( !m_UsingQPF )
        return -1.0;

    // Get either the current time or the stop time
    LARGE_INTEGER qwTime;
    if( m_StopTime != 0 )
        qwTime.QuadPart = m_StopTime;
    else
        QueryPerformanceCounter( &qwTime );

    float64 fElapsedTime = (float64) ( qwTime.QuadPart - m_LastElapsedTime ) / (float64) m_QPFTicksPerSec;
    m_LastElapsedTime = qwTime.QuadPart;

    return fElapsedTime;
}


//--------------------------------------------------------------------------------------
bool CTimer2::IsStopped()
{
    return m_TimerStopped;
}


}	// namespace System
}	// namespace WT
