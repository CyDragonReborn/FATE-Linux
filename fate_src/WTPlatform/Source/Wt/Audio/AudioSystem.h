/***************************************************************************************
 *
 * AudioSystem.h
 *
 * Copyright (C) 2003 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 12/08/2003: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/AudioTypes.h"
#include "WT/Audio/AudioSystemProperties.h"
#include "WT/Audio/AudioClip.h"
#include "WT/Audio/AudioStream.h"
#include "WT/Audio/AudioDevice.h"
#include "WT/Audio/AudioCD.h"
#include "WT/Audio/Music.h"
#include "WT/Audio/DspSystem.h"

namespace WT
{
namespace Audio
{

class CAudioDevice;

///Speaker modes to be used with IAudioSystem::SetSpeakerMode
enum ESpeakerMode
{
	KSpeakerModeStereo,		///< Stereo speaker setup (default)
	KSpeakerModeHeadphones,	///< Headphone setup
	KSpeakerModeMono,			///< Mono speaker setup
	KSpeakerModeQuad,			///< Quadraphonic speaker setup
	KSpeakerModeSurround,		///< Surround sound speaker setup
	KSpeakerModeDolby			///< Dolby Digital output / speaker setup

};

typedef System::SmartPointer<class CAudioSystem> HAudioSystem;
class CAudioSystem : public System::CObject
{
public:

	WT_CLASS_NO_CREATOR( CAudioSystem, System::CObject );

	CAudioSystem( const CAudioSystemProperties& InitProps );
	virtual ~CAudioSystem();

	// System Init functions	
	virtual void					Update();		// call once per frame
	
	virtual void					SetSpeakerMode( ESpeakerMode SpeakerMode );
	virtual void					SetPanSeparation( float32 PanSeparation );
	virtual void					SetMasterVolume( float32 Volume );

	virtual float32					GetMasterVolume( ) const;

	// Clip support
	virtual HAudioClip			LoadAudioClip( const std::string& Filename, AudioModeFlags Mode );

	// Stream support
	virtual HAudioStream		OpenAudioStream( const std::string& Filename, AudioModeFlags Mode, uint32 BufferMs );

	// Audio CD stream support
	virtual HAudioCD			OpenAudioCD( const std::string& driveLetter, uint32 BufferMs );
	virtual void				FindFirstCdRomDriveLetter( std::string& driveLetter );
	virtual void				FindNextCdRomDriveLetter( std::string& driveLetter );

	// Music support
	virtual HMusic				LoadMusic( const std::string& Filename, AudioModeFlags Mode );

	// 3D sound functions

	// Dsp functions
	virtual CDspSystem*			GetDspSystem( );

	// Channel functions
	virtual HChannel			CreateReservedChannel( );	// creates a reserved channel object that can be passed to play methods for clips and streams

private:
    HAudioDevice		m_hAudioDevice;	// for now we need to get the device handle
											// from our audioRender dll.
	HDspSystem			m_hDspSystem;

	static const int8		KFindCdRomDriveMin = 2;	// 'A' + 2 == 'C'
	static const int8		KFindCdRomDriveMax = 26; // 'A' + 25 == 'Z'

	int8					m_FindCdRomDriveLetter;
};

}	// namespace Audio
}	// namespace WT

