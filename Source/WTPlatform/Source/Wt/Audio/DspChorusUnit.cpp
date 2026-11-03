/***************************************************************************************
 *
 * DspChorusUnit.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/19/2004: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/System/SystemException.h"
#include "WT/Audio/DspChorusUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspChorusUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspChorusUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspChorusUnit* pDspUnit = reinterpret_cast<CDspChorusUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterChorus* pLFilter = pDspUnit->GetChorusFilterLeft();
		CAudioFilterChorus* pRFilter = pDspUnit->GetChorusFilterRight();

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

void* __cdecl DspChorusUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspChorusUnit* pDspUnit = reinterpret_cast<CDspChorusUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterChorus* pLFilter = pDspUnit->GetChorusFilterLeft();
		CAudioFilterChorus* pRFilter = pDspUnit->GetChorusFilterRight();

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
CDspChorusUnit::CDspChorusUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	// We must set sample rate in case it differs from default
	m_ChorusFilterLeft.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );
	m_ChorusFilterRight.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspChorusUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspChorusUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
																cb,
																Priority,
																reinterpret_cast<int32>(this),
																pStreamOrCD );
}
#pragma warning (pop)

///
CDspChorusUnit::~CDspChorusUnit()
{
}

///
void CDspChorusUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspChorusUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspChorusUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspChorusUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

///
void CDspChorusUnit::SetBufferTime( float32 Seconds )
{
	m_ChorusFilterLeft.SetBufferedTime( Seconds );
	return m_ChorusFilterRight.SetBufferedTime( Seconds );
}

///
void CDspChorusUnit::SetDelayTime( float32 Seconds )
{
	m_ChorusFilterLeft.SetDelayTime( Seconds );
	return m_ChorusFilterRight.SetDelayTime( Seconds );
}

///
void CDspChorusUnit::SetGain( float32 Gain )
{
	m_ChorusFilterLeft.SetMagnitude( Gain );
	return m_ChorusFilterRight.SetMagnitude( Gain );	
}

///
float32 CDspChorusUnit::GetBufferTime( ) const
{
	// both left and right values will be the same
	return m_ChorusFilterLeft.GetBufferedTime();
}

float32 CDspChorusUnit::GetDelayTime( ) const
{
	// both left and right values will be the same
	return m_ChorusFilterLeft.GetDelayTime();
}

///
float32 CDspChorusUnit::GetGain( ) const
{
	// both left and right values will be the same
	return m_ChorusFilterLeft.GetMagnitude();
}

}	// namespace Audio
}	// namespace WT

