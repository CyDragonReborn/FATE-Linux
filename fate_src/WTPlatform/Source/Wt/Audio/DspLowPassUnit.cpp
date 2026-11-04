/***************************************************************************************
 *
 * DspLowPassUnit.cpp
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
#include "WT/Audio/DspLowPassUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspLowPassUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspLowPassUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspLowPassUnit* pDspUnit = reinterpret_cast<CDspLowPassUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterLowPass* pLFilter = pDspUnit->GetLowPassFilterLeft();
		CAudioFilterLowPass* pRFilter = pDspUnit->GetLowPassFilterRight();

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

void* __cdecl DspLowPassUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspLowPassUnit* pDspUnit = reinterpret_cast<CDspLowPassUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterLowPass* pLFilter = pDspUnit->GetLowPassFilterLeft();
		CAudioFilterLowPass* pRFilter = pDspUnit->GetLowPassFilterRight();

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
CDspLowPassUnit::CDspLowPassUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	// We must set sample rate in case it differs from default
	m_LowPassFilterLeft.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );
	m_LowPassFilterRight.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspLowPassUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspLowPassUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
														cb,
														Priority,
														reinterpret_cast<int32>(this),
														pStreamOrCD );
}
#pragma warning (pop)

///
CDspLowPassUnit::~CDspLowPassUnit()
{
}

///
void CDspLowPassUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspLowPassUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspLowPassUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspLowPassUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

///
void CDspLowPassUnit::SetGain( float32 Gain )
{
	m_LowPassFilterLeft.SetMagnitude( Gain );
	return m_LowPassFilterRight.SetMagnitude( Gain );	
}

///
void CDspLowPassUnit::SetCutoffFrequency( float32 Frequency )
{
	m_LowPassFilterLeft.SetCutoffFrequency( Frequency );
	return m_LowPassFilterRight.SetCutoffFrequency( Frequency );
}

///
float32 CDspLowPassUnit::GetGain( ) const
{
	// both left and right values will be the same
	return m_LowPassFilterLeft.GetMagnitude();
}

///
float32 CDspLowPassUnit::GetCutoffFrequency( ) const
{
	// both left and right values will be the same
	return m_LowPassFilterLeft.GetCutoffFrequency();
}

}	// namespace Audio
}	// namespace WT

