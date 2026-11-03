/***************************************************************************************
 *
 * DspReverbUnit.cpp
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
#include "WT/Audio/DspReverbUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspReverbUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspReverbUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspReverbUnit* pDspUnit = reinterpret_cast<CDspReverbUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterReverberation* pLFilter = pDspUnit->GetReverbFilterLeft();
		CAudioFilterReverberation* pRFilter = pDspUnit->GetReverbFilterRight();

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

void* __cdecl DspReverbUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspReverbUnit* pDspUnit = reinterpret_cast<CDspReverbUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterReverberation* pLFilter = pDspUnit->GetReverbFilterLeft();
		CAudioFilterReverberation* pRFilter = pDspUnit->GetReverbFilterRight();

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

CDspReverbUnit::CDspReverbUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	// We must set sample rate in case it differs from default
	m_ReverbFilterLeft.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );
	m_ReverbFilterRight.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspReverbUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspReverbUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
														cb,
														Priority,
														reinterpret_cast<int32>(this),
														pStreamOrCD );
}
#pragma warning (pop)

///
CDspReverbUnit::~CDspReverbUnit()
{
}

///
void CDspReverbUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspReverbUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspReverbUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspReverbUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

///
void CDspReverbUnit::SetBufferTime( float32 Seconds )
{
	m_ReverbFilterLeft.SetBufferedTime( Seconds );
	return m_ReverbFilterRight.SetBufferedTime( Seconds );
}

///
void CDspReverbUnit::SetDelayTime( float32 Seconds )
{
	m_ReverbFilterLeft.SetDelayTime( Seconds );
	return m_ReverbFilterRight.SetDelayTime( Seconds );
}

///
void CDspReverbUnit::SetGain( float32 Gain )
{
	m_ReverbFilterLeft.SetMagnitude( Gain );
	return m_ReverbFilterRight.SetMagnitude( Gain );	
}

///
void CDspReverbUnit::SetDelayFactor( float32 DelayFactor )
{
	m_ReverbFilterLeft.SetDelayFactor( DelayFactor );
	return m_ReverbFilterRight.SetDelayFactor( DelayFactor );	
}

///
float32 CDspReverbUnit::GetBufferTime( ) const
{
	// both left and right values will be the same
	return m_ReverbFilterLeft.GetBufferedTime();
}

float32 CDspReverbUnit::GetDelayTime( ) const
{
	// both left and right values will be the same
	return m_ReverbFilterLeft.GetDelayTime();
}

///
float32 CDspReverbUnit::GetGain( ) const
{
	// both left and right values will be the same
	return m_ReverbFilterLeft.GetMagnitude();
}

///
float32 CDspReverbUnit::GetDelayFactor( ) const
{
	// both left and right values will be the same
	return m_ReverbFilterLeft.GetDelayFactor();
}

}	// namespace Audio
}	// namespace WT

