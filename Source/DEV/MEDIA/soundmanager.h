/*
	soundmanager

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef _SOUNDMANAGER_H
#define _SOUNDMANAGER_H


#include "fmod.h"
#include "../UTILITIES/constants.h"

class CSettings;


const uint32 KMaximumMusicSearchDepth( 3 );
const uint32 KMaximumMusicFileChecks( 500 );

class CSoundManager
{
public:


					CSoundManager( CSettings& pSettings,				// pointer to a settings manager
								   std::string ExecutablePath );	// the base Path of this executable
					~CSoundManager();

	void			ShutdownFMOD();

	void			Update( void );

	void			StartMusic( std::string Path,				// path to music stream
								bool Looping = kTrue );			// loop the music stream?

	bool			StartMusicFromFolder( std::string Path );	// path to the music folder to look in
	
	void			GetMusicFromFolder( std::string Path,					// path to the music folder to look in
									    std::vector< std::string >& Paths,	// list of paths to add to
										uint32& Depth,						// subfolder depth
										uint32& FilesChecked );				// number of files checked


	void			StopMusic();
	void			StartCDMusic();
	void			StartCDMusicContinuous();
	void			StartCDMusic( int32 Track );	// Track to begin playing
	bool			MusicPlaying();
	void			SetMasterVolume( int32 Volume );// volume to set at ( 0-100% )
	void			SetSFXVolume( int32 Volume );	// volume to set at ( 0-100% )
	void			SetMusicVolume( int32 Volume );	// volume to set at ( 0-100% )
	void			UpdateVolume( void );
	void			DetectCD( void );
	void			Mute( void );
	void			UnMute( void );
	uint32			CDTracks( void );

	bool			HasCD( void )			{	return m_HasMusicCD;	};

private:
	CSettings&		m_pSettings;

	bool			m_CDMusicPlaying;
	bool			m_Muted;
	bool			m_Initialized;

	std::string		m_ExecutablePath;
	int32			m_MusicChannel;

	int32			m_MasterVolume;
	int32			m_SFXVolume;
	int32			m_MusicVolume;

	bool			m_HasMusicCD;
	int32			m_CDTracks;
	int32			m_CurrentTrack;

	FSOUND_STREAM*	m_MusicStream;

};

#endif