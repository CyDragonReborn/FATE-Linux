/***************************************************************************************
 *
 * DspFFTUnit.cpp
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
#include "WT/Audio/AudioException.h"
#include "windows.h"
#include "WT/Audio/DspFFTUnit.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspFFTUnit)

static int32 KSpectrumBinCount = 512;

///
CDspFFTUnit::CDspFFTUnit()
{
	m_pDspUnit = FSOUND_DSP_GetFFTUnit();

	if( m_pDspUnit == NULL )
	{
		WT_THROW( CAudioException, "FSOUND_DSP_GetFFTUnit failure" );
	}
}

///
CDspFFTUnit::~CDspFFTUnit()
{
}

///
void CDspFFTUnit::SetActive( bool Active )
{
	assert( m_pDspUnit != NULL );
	signed char active((Active == true) ? TRUE : FALSE);
	FSOUND_DSP_SetActive( m_pDspUnit, active );
}

///
void CDspFFTUnit::SetPriority( int32 Priority )
{
	assert( m_pDspUnit != NULL );
    FSOUND_DSP_SetPriority( m_pDspUnit, static_cast<int>(Priority) );
}

///
bool CDspFFTUnit::GetActive() const
{
	assert( m_pDspUnit != NULL );
	return ((FSOUND_DSP_GetActive( m_pDspUnit ) == TRUE) ? true : false);
}

///
int32 CDspFFTUnit::GetPriority() const
{
	assert( m_pDspUnit != NULL );
	return static_cast<int32>(FSOUND_DSP_GetPriority( m_pDspUnit ));
}

///
int32 CDspFFTUnit::GetSpectrumBinCount()
{
	return KSpectrumBinCount;
}

///
const float32* CDspFFTUnit::GetSpectrum()
{
	return FSOUND_DSP_GetSpectrum();
}

}	// namespace Audio
}	// namespace WT
