/***************************************************************************************
 *
 * DspGraphicEQUnit.cpp
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
#include "WT/Audio/DspGraphicEQUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspGraphicEQUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspGraphicEQUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspGraphicEQUnit* pDspUnit = reinterpret_cast<CDspGraphicEQUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CGraphicEqualizerFilter* pLFilter = pDspUnit->GetGraphicEQFilterLeft();
		CGraphicEqualizerFilter* pRFilter = pDspUnit->GetGraphicEQFilterRight();

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

void* __cdecl DspGraphicEQUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspGraphicEQUnit* pDspUnit = reinterpret_cast<CDspGraphicEQUnit*>(Param);
	if( pDspUnit != NULL )
	{
		CGraphicEqualizerFilter* pLFilter = pDspUnit->GetGraphicEQFilterLeft();
		CGraphicEqualizerFilter* pRFilter = pDspUnit->GetGraphicEQFilterRight();

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
CDspGraphicEQUnit::CDspGraphicEQUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		cb = DspGraphicEQUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		cb = DspGraphicEQUnitCallback_Float32StereoSample;
	}

	m_pCustomDspUnit = new CDspCustomUnit(
																cb,
																Priority,
																reinterpret_cast<int32>(this),
																pStreamOrCD );
}
#pragma warning (pop)

///
CDspGraphicEQUnit::~CDspGraphicEQUnit()
{
}

///
void CDspGraphicEQUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspGraphicEQUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspGraphicEQUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspGraphicEQUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

///
void CDspGraphicEQUnit::SetEqualizerSetting(int32 Index, float32 val)
{
	m_GraphicEQFilterLeft.SetEqualizerSetting( Index, val );
	return m_GraphicEQFilterRight.SetEqualizerSetting( Index, val );
}

///
int32 CDspGraphicEQUnit::GetNumBands( ) const
{
	return static_cast<int32>(m_GraphicEQFilterLeft.eNumBands);
}

///
void CDspGraphicEQUnit::GetEqualizerSetting(int32 Index, float32& val) const
{
	return m_GraphicEQFilterLeft.GetEqualizerSetting( Index, val );
}

}	// namespace Audio
}	// namespace WT

