/***************************************************************************************
 *
 * DspFlangeUnit.h
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
#include "WT/Audio/DspFlangeUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspFlangeUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspFlangeUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspFlangeUnit* pDspUnit = reinterpret_cast<CDspFlangeUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterFlange* pLFilter = pDspUnit->GetFlangeFilterLeft();
		CAudioFilterFlange* pRFilter = pDspUnit->GetFlangeFilterRight();

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

void* __cdecl DspFlangeUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspFlangeUnit* pDspUnit = reinterpret_cast<CDspFlangeUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CAudioFilterFlange* pLFilter = pDspUnit->GetFlangeFilterLeft();
		CAudioFilterFlange* pRFilter = pDspUnit->GetFlangeFilterRight();

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
CDspFlangeUnit::CDspFlangeUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	// We must set sample rate in case it differs from default
	m_FlangeFilterLeft.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );
	m_FlangeFilterRight.SetSampleRate( static_cast<float32>(pDspSystem->GetMixRateHz()) );

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspFlangeUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspFlangeUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
																cb,
																Priority,
																reinterpret_cast<int32>(this),
																pStreamOrCD );
}
#pragma warning (pop)

///
CDspFlangeUnit::~CDspFlangeUnit()
{
}


///
void CDspFlangeUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspFlangeUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspFlangeUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspFlangeUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

void CDspFlangeUnit::SetMinDelayTime(float32 MinDelayTime)
{
	m_FlangeFilterLeft.SetMinDelayTime( MinDelayTime );
	return m_FlangeFilterRight.SetMinDelayTime( MinDelayTime );
}

void CDspFlangeUnit::SetMaxDelayTime(float32 MaxDelayTime)
{
	m_FlangeFilterLeft.SetMaxDelayTime( MaxDelayTime );
	return m_FlangeFilterRight.SetMaxDelayTime( MaxDelayTime );
}

void CDspFlangeUnit::SetFrequency(float32 Frequency)
{
	m_FlangeFilterLeft.SetFrequency( Frequency );
	return m_FlangeFilterRight.SetFrequency( Frequency );
}

float32 CDspFlangeUnit::GetMinDelayTime() const
{
	// both left and right values will be the same
	return m_FlangeFilterLeft.GetMinDelayTime();
}

float32 CDspFlangeUnit::GetMaxDelayTime() const
{
	// both left and right values will be the same
	return m_FlangeFilterLeft.GetMaxDelayTime();
}

float32 CDspFlangeUnit::GetFrequency() const
{
	// both left and right values will be the same
	return m_FlangeFilterLeft.GetFrequency();
}

}	// namespace Audio
}	// namespace WT

