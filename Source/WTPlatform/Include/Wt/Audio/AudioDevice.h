/***************************************************************************************
 *
 * AudioDevice.h
 *
 * Copyright (C) 2002 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 11/27/2002: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/Audio/AudioSystemProperties.h"
#include "WT/Audio/DspEngine.h"
#include "WT/Audio/AudioClip.h"
#include "WT/Audio/AudioStream.h"
#include "WT/Audio/AudioCD.h"
#include "WT/Audio/Music.h"
#include "WT/Audio/Channel.h"
#include "WT/Audio/DspEngine.h"


namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CAudioDevice> HAudioDevice;
class CAudioDevice : public System::CObject
{
public:
    
	WT_CLASS_NO_CREATOR( CAudioDevice, System::CObject );

	CAudioDevice( const CAudioSystemProperties& InitProps );
	virtual ~CAudioDevice( );
	
	virtual void			Update();		// call once per frame
	
	virtual void			SetSpeakerMode( enum ESpeakerMode SpeakerMode );
	virtual void			SetPanSeparation( float32 PanSeparation );
	virtual void			SetMasterVolume( float32 Volume );

	virtual float32			GetMasterVolume( ) const;

	// Clip support
	virtual HAudioClip		LoadAudioClip( const std::string& Filename, AudioModeFlags Flags);

	// Stream support
	virtual HAudioStream	OpenAudioStream( const std::string& Filename, AudioModeFlags Flags, uint32 BufferMs );

	// Audio CD stream support
	virtual HAudioCD		OpenAudioCD( const std::string& driveLetter, uint32 BufferMs );

	// Music support
	virtual HMusic			LoadMusic( const std::string& Filename, AudioModeFlags Flags);

	// 3D sound functions

	// Dsp functions
	virtual CDspEngine*		GetDspEngine( );

	// Channel functions
	virtual HChannel		CreateReservedChannel( );	// creates a reserved channel object that can be passed to play methods for clips and streams

private:
	
	static const int32 m_kMaxVolume = 255;
	static const uint32 m_kMinStreamBufferMs = 50;

private:

	int32		m_MaxSoftwareChannels;
	float32		m_MasterVolume;

	uint32		m_StreamBufferMs;

	HDspEngine	m_hDspEngine;
};

}	// namespace Audio
}	// namespace WT

