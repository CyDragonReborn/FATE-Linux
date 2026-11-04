/***************************************************************************************
 *
 * DspSystem.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/18/2004: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "WT/Audio/DspSystem.h"
#include "WT/Audio/DspWaveformUnit.h"
#include "WT/Audio/DspVocalEliminatorUnit.h"
#include "WT/Audio/DspChorusUnit.h"
#include "WT/Audio/DspFlangeUnit.h"
#include "WT/Audio/DspDelayUnit.h"
#include "WT/Audio/DspReverbUnit.h"
#include "WT/Audio/DspPitchShiftUnit.h"
#include "WT/Audio/DspRingModulatorUnit.h"
#include "WT/Audio/DspDistortionUnit.h"
#include "WT/Audio/DspHighPassUnit.h"
#include "WT/Audio/DspLowPassUnit.h"
#include "WT/Audio/DspGraphicEQUnit.h"
#include "WT/Audio/DspEngine.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspSystem)

///
CDspSystem::CDspSystem( CDspEngine* pDspEngine )
	: m_pDspEngine( pDspEngine )
{
}

///
CDspSystem::~CDspSystem()
{
}

#pragma warning (push)
#pragma warning (disable: 4100)

#pragma warning (pop)

int32 CDspSystem::GetMixRateHz( ) const
{
	return m_pDspEngine->GetMixRateHz();
}

///
int32 CDspSystem::GetDspBufferLength( ) const
{
	assert( m_pDspEngine != NULL );
	return m_pDspEngine->GetDspBufferLength();
}

///
EMixerSampleType CDspSystem::GetMixerSampleType( ) const
{
	assert( m_pDspEngine != NULL );
	return m_pDspEngine->GetMixerSampleType();
}

///
void CDspSystem::ClearMixBuffer( )
{
	assert( m_pDspEngine != NULL );
	m_pDspEngine->ClearMixBuffer();
}

///
int32 CDspSystem::GetMixBufferSize( ) const
{
	assert( m_pDspEngine != NULL );
	return m_pDspEngine->GetMixBufferSize();
}

///
void CDspSystem::MixBuffers( void* DestBuffer,
							void* SrcBuffer,
							int32 Length,
							int32 Frequency,
							float32 Volume,
							float32 Pan,
							uint32 Mode )
{
	assert( m_pDspEngine != NULL );
	return m_pDspEngine->MixBuffers( DestBuffer,
									SrcBuffer,
									Length,
									Frequency,
									Volume,
									Pan,
									Mode );
}

}	// namespace Audio
}	// namespace WT
