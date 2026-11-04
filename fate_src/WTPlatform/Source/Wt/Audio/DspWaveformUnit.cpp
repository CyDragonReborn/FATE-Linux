/***************************************************************************************
 *
 * DspWaveformUnit.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/17/2004: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/System/SystemException.h"
#include "WT/Audio/DspWaveformUnit.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspWaveformUnit)

#pragma warning (push)
#pragma warning (disable: 4312)
#pragma warning (disable: 4100)

///
void* __cdecl DspWaveformUnitCallback_Int16StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspWaveformUnit* pDspUnit = reinterpret_cast<CDspWaveformUnit*>(Param);
	if( pDspUnit != NULL )
	{
		int16* pWaveformData[ KNumChannels ];
		pWaveformData[KLeftChannel] = reinterpret_cast<int16*>(pDspUnit->GetWaveform( KLeftChannel ));
		pWaveformData[KRightChannel] = reinterpret_cast<int16*>(pDspUnit->GetWaveform( KRightChannel ));
		pWaveformData[KCenterChannel] = reinterpret_cast<int16*>(pDspUnit->GetWaveform( KCenterChannel ));

		int16* sample = reinterpret_cast<int16*>(NewBuffer);
		int16 l,r;
		int32 c;
		for( int32 i = 0; i < Length; i++ )
		{
			l = *sample++;
			r = *sample++;
			c = l - r;
			// Clip
			if(c > 32767)	c = 32767;
			if(c < -32768)	c = -32768;

			c = ((l - c) + (r - c)) / 2;
			// Clip
			if(c > 32767)	c = 32767;
			if(c < -32768)	c = -32768;

			pWaveformData[KLeftChannel][i] = l;
			pWaveformData[KRightChannel][i] = r;
			pWaveformData[KCenterChannel][i] = static_cast<int16>(c);
		}
	}
    return NewBuffer;
}

void* __cdecl DspWaveformUnitCallback_Float32StereoSample(void *OriginalBuffer,
										void *NewBuffer,
										int32 Length,
										int32 Param)
{
	CDspWaveformUnit* pDspUnit = reinterpret_cast<CDspWaveformUnit*>(Param);
	if( pDspUnit != NULL )
	{
		float32* pWaveformData[ KNumChannels ];
		pWaveformData[KLeftChannel] = reinterpret_cast<float32*>(pDspUnit->GetWaveform( KLeftChannel ));
		pWaveformData[KRightChannel] = reinterpret_cast<float32*>(pDspUnit->GetWaveform( KRightChannel ));
		pWaveformData[KCenterChannel] = reinterpret_cast<float32*>(pDspUnit->GetWaveform( KCenterChannel ));

		float32* sample = reinterpret_cast<float32*>(NewBuffer);
		float32 l,r;
		float32 c;
		for( int32 i = 0; i < Length; i++ )
		{
			l = *sample++;
			r = *sample++;
			c = l - r;
			c = ((l - c) + (r - c)) / 2;

			pWaveformData[KLeftChannel][i] = l;
			pWaveformData[KRightChannel][i] = r;
			pWaveformData[KCenterChannel][i] = c;
		}
	}
    return NewBuffer;
}

#pragma warning (pop)

#pragma warning (push)
#pragma warning (disable: 4311)
CDspWaveformUnit::CDspWaveformUnit( CDspSystem* pDspSystem, int32 Priority, CObject* pStreamOrCD )
{
	if (pDspSystem == NULL)
	{
		WT_THROW( System::CArgumentException, "pDspSystem" );
	}

	memset( m_WaveformData, 0, sizeof m_WaveformData );
	assert( m_WaveformData[0] == NULL );

	// allocate memory first
	int32 buflen = pDspSystem->GetDspBufferLength();
	int32 samplebytes;
	DspCallback cb;

	if( pDspSystem->GetMixerSampleType() == KInt16StereoSample )
	{
		samplebytes = sizeof(int16);
		cb = DspWaveformUnitCallback_Int16StereoSample;
	}
	else // KFloat32StereoSample
	{
		samplebytes = sizeof(float32);
		cb = DspWaveformUnitCallback_Float32StereoSample;
	}

	int8* pBuf = new int8[buflen * samplebytes * 3];

	m_pCustomDspUnit = new CDspCustomUnit(	cb,
											Priority,
											reinterpret_cast<int32>(this),
											pStreamOrCD );
	m_WaveformData[0] = reinterpret_cast<void*>(pBuf);
	m_WaveformData[1] = reinterpret_cast<void*>(&pBuf[buflen*samplebytes]);
	m_WaveformData[2] = reinterpret_cast<void*>(&pBuf[buflen*samplebytes*2]);
}
#pragma warning (pop)

///
CDspWaveformUnit::~CDspWaveformUnit()
{
	delete[] m_WaveformData[0];
}

///
void CDspWaveformUnit::SetActive( bool Active )
{
	m_pCustomDspUnit->SetActive( Active );
}

///
void CDspWaveformUnit::SetPriority( int32 Priority )
{
	m_pCustomDspUnit->SetPriority( Priority );
}

///
bool CDspWaveformUnit::GetActive() const
{
	return m_pCustomDspUnit->GetActive();
}

///
int32 CDspWaveformUnit::GetPriority() const
{
	return m_pCustomDspUnit->GetPriority();
}

///
const void* CDspWaveformUnit::GetWaveform( EAudioChannel Channel ) const
{
	return m_WaveformData[Channel];
}

///
void* CDspWaveformUnit::GetWaveform( EAudioChannel Channel )
{
	return m_WaveformData[Channel];
}


}	// namespace Audio
}	// namespace WT

