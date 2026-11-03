/***************************************************************************************
 *
 * DspEngine.cpp
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
#include "windows.h"
#include "WT/Audio/DspEngine.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CDspEngine)

///
CDspEngine::CDspEngine()
{
}

///
CDspEngine::~CDspEngine()
{
}

///
int32 CDspEngine::GetMixRateHz( ) const
{
	return static_cast<int32>( FSOUND_GetOutputRate() );
}

///										
int32 CDspEngine::GetDspBufferLength( ) const
{
	//return value is in samples, not bytes
	return static_cast<int32>( FSOUND_DSP_GetBufferLength() );
}

///
EMixerSampleType CDspEngine::GetMixerSampleType( ) const
{
	int mixerType = FSOUND_GetMixer();
	switch( mixerType )
	{
	case FSOUND_MIXER_QUALITY_FPU:
		return KFloat32StereoSample;

	case FSOUND_MIXER_QUALITY_MMXP5:
	case FSOUND_MIXER_QUALITY_MMXP6:
		return KInt16StereoSample;

	default:
		// Theoretically impossible, if this happens there are much bigger issues
		// to worry about.  Will return a value just so this doesn't hang up anything.
		assert( 0 );
		return KInt16StereoSample;
	}
}


///
void CDspEngine::ClearMixBuffer( )
{
	FSOUND_DSP_ClearMixBuffer();
}

///
int32 CDspEngine::GetMixBufferSize( ) const
{
	//return value is in samples, not bytes
	return static_cast<int32>( FSOUND_DSP_GetBufferLengthTotal() );
}

///
void CDspEngine::MixBuffers(	void* DestBuffer,
										void* SrcBuffer,
										int32 Length,
										int32 Frequency,
										float32 Volume,
										float32 Pan,
										uint32 Mode )
{
	int iPan;
	int vol( static_cast<int>( Float32ToInt32( 255 * std::min( std::max( 0.0f, Volume ), 1.0f )) ) );

	if( Pan == KStereoPan )		iPan = FSOUND_STEREOPAN;
	else if( Pan < KLeftPan )		iPan = 0;
	else if( Pan > KRightPan )	iPan = 255;
	else							iPan = static_cast<int>( Float32ToInt32( (Pan + 1.0f) * 0.5f * 255 ) );

	if( FSOUND_DSP_MixBuffers(	DestBuffer,
								SrcBuffer,
								static_cast<int>( Length ),
								static_cast<int>( Frequency ),
								vol,
								iPan,
								static_cast<unsigned int>( Mode ) ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_DSP_MixBuffers failure" );
	}
}

}	// namespace Audio
}	// namespace WT
