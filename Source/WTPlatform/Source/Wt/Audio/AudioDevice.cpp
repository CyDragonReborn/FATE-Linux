/***************************************************************************************
 *
 * AudioDevice.cpp
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 12/08/2003: Jason G:	Creation
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "windows.h"
#include "WT/Audio/AudioDevice.h"
#include "WT/Audio/AudioSystem.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CAudioDevice)

///
CAudioDevice::CAudioDevice( const CAudioSystemProperties& InitProps ) :
	m_MaxSoftwareChannels( 0 ),
	m_MasterVolume( 1.0f ),
	m_StreamBufferMs( 200 )
{
	// TODO: Do initialization here

	FSOUND_SetBufferSize( InitProps.GetMixBufferMs() );
	HWND hwnd = InitProps.GetWindowHandle();
	FSOUND_SetHWND( hwnd ); //InitProps.GetWindowHandle() );

	if( FSOUND_Init(	InitProps.GetMixRate(),
						InitProps.GetMaxSoftwareChannels(),
						InitProps.GetFlags() ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Init failure" );
	}

	m_hDspEngine = new CDspEngine();

	m_MaxSoftwareChannels = InitProps.GetMaxSoftwareChannels();
	SetMasterVolume( m_MasterVolume );	// override FMOD default
}

///
CAudioDevice::~CAudioDevice( )
{
	FSOUND_Close();
}

///
void CAudioDevice::Update()
{
	FSOUND_Update();
}

///
void CAudioDevice::SetSpeakerMode( ESpeakerMode SpeakerMode )
{
	unsigned int mode;

	switch( SpeakerMode )
	{
	case KSpeakerModeHeadphones:
		mode = FSOUND_SPEAKERMODE_HEADPHONES;
		break;

	case KSpeakerModeMono:
		mode = FSOUND_SPEAKERMODE_MONO;
		break;

	case KSpeakerModeQuad:
		mode = FSOUND_SPEAKERMODE_QUAD;
		break;

	case KSpeakerModeSurround:
		mode = FSOUND_SPEAKERMODE_SURROUND;
		break;

	case KSpeakerModeDolby:
		mode = FSOUND_SPEAKERMODE_DOLBYDIGITAL;
		break;

	case KSpeakerModeStereo:
	default:
		mode = FSOUND_SPEAKERMODE_STEREO;
		break;
	}

	FSOUND_SetSpeakerMode( mode );
}

///
void CAudioDevice::SetPanSeparation( float32 PanSeparation )
{
	// Valid Range: 0 <= PanSeparation <= 1.0
	// Fmod clips the value so we won't do it twice
	FSOUND_SetPanSeperation( PanSeparation );
}

///
void CAudioDevice::SetMasterVolume( float32 Volume )
{
	m_MasterVolume = std::min( std::max(0.0f, Volume), 1.0f );
	int32 iVolume = Float32ToInt32( m_MasterVolume * m_kMaxVolume );
	FSOUND_SetSFXMasterVolume( static_cast<int>( iVolume ) );
}

///
float32 CAudioDevice::GetMasterVolume( ) const
{
	return m_MasterVolume;
}

///
HAudioClip CAudioDevice::LoadAudioClip(	const std::string& Filename,
													AudioModeFlags Flags )
{
	return new CAudioClip( Filename, Flags );
}

///
HAudioStream CAudioDevice::OpenAudioStream(	const std::string& Filename,
														AudioModeFlags Flags,
														uint32 BufferMs )
{
	return new CAudioStream( Filename, Flags, BufferMs );
}

///
HAudioCD CAudioDevice::OpenAudioCD( const std::string& driveLetter, uint32 BufferMs )
{
	return new CAudioCD( driveLetter, BufferMs );
}

///
HMusic CAudioDevice::LoadMusic( const std::string& Filename, AudioModeFlags Flags )
{
	return new CMusic( Filename, Flags );
}

///
CDspEngine* CAudioDevice::GetDspEngine( )
{
	return m_hDspEngine;
}

///
HChannel CAudioDevice::CreateReservedChannel( )
{
	HChannel hChannel = new CChannel();

	// reserve a software channel if available
	for( int32 i = 0; i < m_MaxSoftwareChannels; i++ )
	{
		hChannel->SetChannel( i );
		hChannel->SetStream( NULL );
		if( hChannel->IsPlaying() == false && hChannel->GetReserved() == false )
		{
			hChannel->SetReserved( true );
			return hChannel;
		}
	}

	return NULL;
}


}	// namespace Audio
}	// namespace WT
