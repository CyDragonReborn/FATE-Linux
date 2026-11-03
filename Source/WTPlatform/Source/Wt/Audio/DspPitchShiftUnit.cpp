/***************************************************************************************
 *
 * DspPitchShiftUnit.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/20/2004: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/System/SystemException.h"
#include "WT/Audio/DspPitchShiftUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspPitchShiftUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspPitchShiftUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspPitchShiftUnit* pDspUnit = reinterpret_cast<CDspPitchShiftUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterPitchShift* pLFilter = pDspUnit->GetPitchShiftFilterLeft();
		CAudioFilterPitchShift* pRFilter = pDspUnit->GetPitchShiftFilterRight();

		if( pLFilter && pRFilter )
		{
			float32 L, R;
			int32 LResult, RResult;
			int16* pBuffer = reinterpret_cast<int16*>(NewBuffer);
			int32 n = Length;
			while (n--)
			{
				L = pLFilter->Filter( static_cast<float32>(pBuffer[0]) );
				R = pRFilter->Filter( static_cast<float32>(pBuffer[1]) );

				LResult = Float32ToInt32( L );
				RResult = Float32ToInt32( R );

				if (LResult < -32768) LResult = -32768;
				else if (LResult > 32767) LResult = 32767;

				if (RResult < -32768) RResult = -32768;
				else if (RResult > 32767) RResult = 32767;

				*pBuffer++ = static_cast<int16>(LResult);
				*pBuffer++ = static_cast<int16>(RResult);
			}
		}
	}
    return NewBuffer;
}

void* __cdecl DspPitchShiftUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspPitchShiftUnit* pDspUnit = reinterpret_cast<CDspPitchShiftUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterPitchShift* pLFilter = pDspUnit->GetPitchShiftFilterLeft();
		CAudioFilterPitchShift* pRFilter = pDspUnit->GetPitchShiftFilterRight();

		if( pLFilter && pRFilter )
		{
			float32* pBuffer = reinterpret_cast<float32*>(NewBuffer);
			int32 n = Length;
			while (n--)
			{
				*pBuffer++ = pLFilter->Filter( *pBuffer );
				*pBuffer++ = pRFilter->Filter( *pBuffer );
			}
		}
	}
    return NewBuffer;
}

#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable: 4311)
CDspPitchShiftUnit::CDspPitchShiftUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	// We must set sample rate in case it differs from default
	m_PitchShiftFilterLeft.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );
	m_PitchShiftFilterRight.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspPitchShiftUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspPitchShiftUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
														cb,
														Priority,
														reinterpret_cast<int32>(this),
														pStreamOrCD );
}
#pragma warning (pop)

///
CDspPitchShiftUnit::~CDspPitchShiftUnit()
{
}

///
void CDspPitchShiftUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspPitchShiftUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspPitchShiftUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspPitchShiftUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

///
void CDspPitchShiftUnit::SetGain( float32 Gain )
{
	m_PitchShiftFilterLeft.SetMagnitude( Gain );
	return m_PitchShiftFilterRight.SetMagnitude( Gain );	
}

///
void CDspPitchShiftUnit::SetPitchFactor( float32 PitchFactor )
{
	m_PitchShiftFilterLeft.SetPitchFactor( PitchFactor );
	return m_PitchShiftFilterRight.SetPitchFactor( PitchFactor );	
}

///
void CDspPitchShiftUnit::SetLatency( float32 LatencyTime )
{
	//max latency in seconds
	m_PitchShiftFilterLeft.SetLatency( LatencyTime );
	return m_PitchShiftFilterRight.SetLatency( LatencyTime );	
}

///
float32 CDspPitchShiftUnit::GetGain( ) const
{
	// both left and right values will be the same
	return m_PitchShiftFilterLeft.GetMagnitude();
}

///
float32 CDspPitchShiftUnit::GetPitchFactor() const
{
	// both left and right values will be the same
	return m_PitchShiftFilterLeft.GetPitchFactor();
}

///
float32 CDspPitchShiftUnit::GetLatency() const
{
	// max latency in seconds
	// both left and right values will be the same
	return m_PitchShiftFilterLeft.GetLatency();
}

}	// namespace Audio
}	// namespace WT

