//
//	Timer.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/System/Timer.h"
#include <windows.h>

#pragma comment( lib, "winmm" )

namespace WT
{
namespace System
{

// Class static inits

int64 CTimer::m_Frequencies[] = { 0, 0, 0 };
bool CTimer::m_RtdscTested = false;


// -------------------------------------------------------------------------------------
// Typedefs, Forward Declarations
// -------------------------------------------------------------------------------------

// Retrieves the highest-level precision timestamp
static void FastGetTimestamp( int64 *pResult );
// Retrieves a medium-level precision timestamp
static void MedGetTimestamp( int64 *pResult );
// Retrieves a low-precision timestamp
static void LowGetTimestamp( int64 *pResult );


//
// Public Methods
//

CTimer::CTimer()
	: m_TimeStampFunc( NULL )
	, m_State( EStopped )
    , m_StartTicks( 0i64 )
	, m_LastElapsedTicks( 0i64 )
	, m_PausedTicks( 0i64 )
	, m_PausedTime( 0i64 )
	, m_TotalPausedTicks( 0i64 )
    , m_FrequencyInverse( 1.0 )
{
	SetResolution( ETimerResolution::Medium );
}

CTimer::CTimer( ETimerResolution::Resolution TimerResolution )
	: m_TimeStampFunc( NULL )
	, m_State( EStopped )
    , m_StartTicks( 0i64 )
	, m_LastElapsedTicks( 0i64 )
	, m_PausedTicks( 0i64 )
	, m_PausedTime( 0i64 )
	, m_TotalPausedTicks( 0i64 )
    , m_FrequencyInverse( 1.0 )
{
	SetResolution( TimerResolution );
}

CTimer::~CTimer()
{
	; // do nothing.
}

void CTimer::Start()
{
	assert( m_TimeStampFunc != NULL );

	// If we are currently paused, update the total amount of time spent paused
	// Also update the current elapsed marker, but don't adjust the start time
	if( m_State == EPaused )
	{
		// Get the current timestamp
		int64 Time;
		m_TimeStampFunc( &Time );

		// Update the total amount of time spent paused
		m_TotalPausedTicks += ( Time - m_PausedTicks );

		// Mark the current time as the elapsed marker 
		m_LastElapsedTicks = Time;
	}
	else
	{
		// Seed the absolute start time and elapsed marker
		m_TimeStampFunc( &m_StartTicks );
		m_LastElapsedTicks = m_StartTicks;
	}

	// Mark the timer as active
	m_State = EActive;
}

void CTimer::Stop( )
{
	// Update the current state
	m_State = EStopped;

	m_TotalPausedTicks = 0i64;
}

int64 CTimer::Pause( )
{
	assert( m_TimeStampFunc != NULL );

	if( m_State == EActive )
	{
		// Set the paused time to the absolute elapsed time
		m_PausedTime = GetTotalTime( );

		// Retrieve the current paused time
		m_TimeStampFunc( &m_PausedTicks );

		// Mark the state as paused
		m_State = EPaused;

		return m_PausedTime;
	}

	// Pause was called when the current state was either Paused or Stopped
	// Calling GetTotalTime will return the proper value
	return GetTotalTime( );
}

int64 CTimer::GetElapsedTime( )
{
	assert( m_TimeStampFunc != NULL );

	switch( m_State )
	{
	default:
		assert( 0 );
		return 0;

	case EPaused:
	case EStopped:
		// Timer is paused/stopped; "elapsed time" is invalid
		return 0;

	case EActive:
		{
			// Calculate the elapsed time since the last call to GetElapsedTime
			const int64 Elapsed = CalculateElapsedTime( m_LastElapsedTicks );
			
			// Update the elapsed tick count
			m_TimeStampFunc( &m_LastElapsedTicks );
			
			return Elapsed;
		}
	}
}

float32 CTimer::GetElapsedTimeSeconds()
{
	return GetElapsedTime() * 0.001f;
}

int64 CTimer::GetTotalTime( )
{
	switch( m_State )
	{
	default:
		assert( 0 );
		return 0;

	case EStopped:
		// Timer is stopped; no time has elapsed
		return 0;

	case EPaused:	
		// Return cached paused time
		return m_PausedTime;

	case EActive:
		// Calculate the elapsed time between the start time and now, factoring in the total time
		// spent paused
		return CalculateElapsedTime( m_StartTicks + m_TotalPausedTicks ); 
	}
}

float32 CTimer::GetTotalTimeSeconds( )
{
	return GetTotalTime() * 0.001f;
}

ETimerResolution::Resolution CTimer::GetResolution( ) const
{ 
	return m_Resolution; 
}


//
// Private Methods
//

void CTimer::SetResolution( const ETimerResolution::Resolution Res ) 
{
    int64 Freq;

    m_Resolution = Res;
    
    switch( m_Resolution )
    {
	case ETimerResolution::High:

        // Is there a high-performance counter available?
        if( !m_RtdscTested )
        {
            m_RtdscTested = true;
            if( !IsRDTSCAvailable( ) )
            {
				SetResolution( ETimerResolution::Medium );
                return;
            }
        }
      
        // Get the frequency of the timer
        Freq = CTimer::m_Frequencies[2];
        if( Freq == 0 )
        {
			Freq = GetTimerFrequency( ETimerResolution::High );
			CTimer::m_Frequencies[2] = Freq;
        }
        if( Freq != 0 )
        {
            // Set the function pointer to get the fast timestamp and set the frequency inverse
            m_TimeStampFunc = FastGetTimestamp;
            m_FrequencyInverse = (float64)( 1000. / (float64)Freq );    
        }
        else
        {
            // Try the medium res counter
			SetResolution( ETimerResolution::Medium );
            return;
        }

        // Is the rdtsc instruction working as we'd like?
        if( !IsRDTSCWorkingOK( ) )
        {
            // Try the medium res counter
			SetResolution( ETimerResolution::Medium );
            return;
        }
        break;

	case ETimerResolution::Medium:

        // Is there a medium-performance counter available?
        Freq = CTimer::m_Frequencies[1];
        if( Freq == 0 )
        {
			Freq = GetTimerFrequency( ETimerResolution::Medium );
			CTimer::m_Frequencies[1] = Freq;
        }
        if( Freq != 0 )
        {
            // Set the function pointer to get the medium-res timestamp and set the frequency inverse
            m_TimeStampFunc = MedGetTimestamp;
            m_FrequencyInverse = (float64)( 1000. / (float64)Freq );
        }
        else
        {
			// QueryPerformanceCounter not supported; this is extremely rare

            // Set the function pointer to get the low-precision timestamp and set the frequency inverse
			m_TimeStampFunc = LowGetTimestamp;
			// Frequency inverse doesn't matter because ::timeGetTime gives us the actual time rather than the
			// clock tick count
			m_FrequencyInverse = 1.;
            return;
        }
        break;
    }

    // Make sure the timer is primed and ready to go
    assert( m_TimeStampFunc );
    int64 Temp;
    m_TimeStampFunc( &Temp );
    m_TimeStampFunc( &Temp );
    m_TimeStampFunc( &Temp );
}

int64 CTimer::GetTimerFrequency( const ETimerResolution::Resolution Res )
{
	if( Res == ETimerResolution::High )
    {
        // Figure out the frequency
        int64 Freq = 0i64;
    
        {
            uint32 ProcPriority   = ::GetPriorityClass( ::GetCurrentProcess( ) );
            uint32 ThreadPriority = ::GetThreadPriority( ::GetCurrentThread( ) );
            ::SetPriorityClass( ::GetCurrentProcess( ), REALTIME_PRIORITY_CLASS );
            ::SetThreadPriority( ::GetCurrentThread( ), THREAD_PRIORITY_TIME_CRITICAL );
            
            FastGetTimestamp( &Freq );
            ::Sleep( 500 );                                // sleep for 500 ms
            int64 Now; FastGetTimestamp( &Now );
            Freq = Now - Freq;
            Freq *= 2;                                     // number of ticks per second

            ::SetPriorityClass( ::GetCurrentProcess( ), ProcPriority );
            ::SetThreadPriority( ::GetCurrentThread( ), ThreadPriority );
        }

        return Freq;
    }

	else if( Res == ETimerResolution::Medium )
    {
        int64 Freq;
        if( ::QueryPerformanceFrequency( (LARGE_INTEGER*)&Freq ) )
        {
            return Freq;
        }
        return 0i64;
    }

    else        // TimerLowRes
    {
        // "Timer frequency" is not used for the low-res timer-- just return 1.
        return 1i64;
    }
}

bool CTimer::IsRDTSCAvailable( )
{
    __try
    {
        // Test rdtsc and cpuid instructions
        __asm 
        {
            mov    eax, 1
            cpuid
            rdtsc
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {                
        return false;
    }       
    return true;
}

// This utility is provided to check cases where RDTSC is just NOT the thing to use.
// These cases include but are not limited to: Intel SpeedStep laptops, AMD K6-III laptops, and pre-RDTSC architectures.
bool CTimer::IsRDTSCWorkingOK( )
{
    enum RDTSC_IS
    {
        Untested,
        GoodWorkingOrder,
        Broken,
    };
    
    static RDTSC_IS RdtscState = Untested;
    
    if( RdtscState == Untested )
    {
        // cpuid check is enabled, as is the sleep test 
        // test cpuid (and get info for CPU detect)
        uint32 cpuid_eax;
        uint32 cpuid_ebx;
        __asm
        {
            mov         eax, 1
            cpuid
            mov         cpuid_eax, eax
            mov         cpuid_ebx, ebx
        }
        
        // Unless it's a speedstep proc
        // ftp://download.intel.com/design/mobile/datashts/p3_ds.pdf    (Mobile Intel PIII datasheet)   [brandID==2]
        // ftp://download.intel.com/design/mobile/datashts/28365403.pdf (Mobile Intel Celeron datasheet)[brandID==1]
        // ftp://download.intel.com/design/mobile/datashts/29834002.pdf
        uint32 ProcType    =  ((cpuid_eax >> 12) & 0x3);
        uint32 ProcFamily  =  ((cpuid_eax >> 8)  & 0xF);
        uint32 ProcModel   =  ((cpuid_eax >> 4)  & 0xF);
        uint32 ProcBrandID =  ((cpuid_ebx) & 0xFF);
        
        // CPU detect test
        if( ProcType == 0x0 )
        {
            if( ProcFamily == 0x6 )
            {
                if( ProcModel == 0x8 ) // Mobile pentium III and mobile celeron
                {
                    if( (ProcBrandID==0x2) ||(ProcBrandID==0x1) )
                    {
                        RdtscState = Broken;
                    }
                }
                if( ProcModel == 0xB ) // Mobile pentium III-M
                {
                    if (ProcBrandID==0x6) 
                    {
                        RdtscState = Broken;
                    }
                }
            }
        }
        // CPU speed test
        if( RdtscState != Broken )
        {
            float64 ObservedTime;
            const uint32 RealTime = 10;
            uint32 Strikes = 0;
        
            ::Sleep( 1 );    // warm up!
            for( uint32 index = 0; index < 5; index++ )
            {
                Start( );
                ::Sleep( RealTime );
                ObservedTime = (float64)GetElapsedTime( );

                // If we miss by 4 ms, that's a strike
                float64 Abs = ObservedTime - (float64)RealTime;
                if( Abs < 0 ) Abs = -Abs;
                if( Abs > 4.0 )
                {
                    Strikes++;
                }
            }
            if( Strikes >= 3 )
            {
                RdtscState = Broken;
            }
            else
            {
                RdtscState = GoodWorkingOrder;
            }
        }
    }

    return (bool)( RdtscState == GoodWorkingOrder);
}

int64 CTimer::CalculateElapsedTime( int64 StartTime )
{
	assert( m_TimeStampFunc != NULL );

	int64 Time;
    m_TimeStampFunc( &Time );

	return Float64ToInt64( (float64)(Time - StartTime) * m_FrequencyInverse );
}


// -------------------------------------------------------------------------------------
// Functions
// -------------------------------------------------------------------------------------

// rdtsc
inline void FastGetTimestamp( int64 *pResult ) 
{
    assert( pResult );

    int64 val;
    int32 *dest_low  = (int32*)(&val);
    int32 *dest_high = dest_low + 1;

    __asm 
    {
        rdtsc
        mov    ebx, dest_low
        mov    ecx, dest_high
        mov    [ebx], eax
        mov    [ecx], edx
    }

    *pResult = val;
}

// QueryPerformanceCounter
inline void MedGetTimestamp( int64* pResult )
{
    assert( pResult );

    // We won't worry about int64 rollover here; by my quick calculations it would take 
    // around 584,942,417 years for it to roll over. :)
    ::QueryPerformanceCounter( (LARGE_INTEGER*)pResult );
}

// timeGetTime
inline void LowGetTimestamp( int64* pResult )
{
    assert( pResult );

    int64 val = 0i64;
    int32 *dest_low  = (int32*)(&val);

    *dest_low = ::timeGetTime( );
    *pResult = val;
}


}	// namespace System
}	// namespace WT
