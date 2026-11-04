/*
	soundmanager

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004


	Change History

	Travis Baldree
	6/17/2004		-		all volumes were being extracted from master volume settings

*/

#include <windows.h>
#include <math.h>
#include <string>
#include "fmod.h"

using namespace std;

#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "soundmanager.h"



CSoundManager::CSoundManager( CSettings& pSettings,					// pointer to a settings manager
							  std::string ExecutablePath ) :	// the base Path of this executable
							  m_pSettings( pSettings ),
							  m_ExecutablePath( ExecutablePath ),
							  m_Muted( kFalse ),
							  m_Initialized( kFalse ),
							  m_HasMusicCD( kFalse ),
							  m_CDMusicPlaying( kFalse ),
							  m_CDTracks( 0 ),
							  m_CurrentTrack( 1 ),
							  m_MusicChannel( -1 ),
							  m_MusicStream( NULL ),
							  m_MasterVolume( 60 ),
							  m_SFXVolume( 100 ),
							  m_MusicVolume( 100 )
{
	uint32	Retries( 0 );

	FSOUND_SetBufferSize( 100 );
	while ( !FSOUND_Init( 44100, 16, 0 ) && Retries < 20 )
	{
		Sleep( 100 );
		Retries++;
	}

	SetMasterVolume( m_MasterVolume );

	m_Initialized = kTrue;
} // CSoundManager::CSoundManager()

// shut down FMOD and stop any playing m_MusicStream
CSoundManager::~CSoundManager( void )
{
	StopMusic();
	ShutdownFMOD();
} // CSoundManager::~CSoundManager()

void CSoundManager::DetectCD( void )
{
	m_CDTracks = FSOUND_CD_GetNumTracks( 0 );
	if ( m_CDTracks > 1 )
	{
		m_HasMusicCD = kTrue;
	}
	else
	{
		m_HasMusicCD = kFalse;
	}
} // CSoundManager::DetectCD()

// shut down FMOD
void CSoundManager::ShutdownFMOD( void )
{
	if ( m_Initialized )
	{
		FSOUND_Close();
	}
} // CSoundManager::ShutdownFMOD()

// start a CD playing - will choose a random track when done with the current track
void CSoundManager::StartCDMusic( void )
{
	if ( !m_HasMusicCD || m_MusicVolume == 0 )
	{
		return;
	}
	int32	Track( 1 );
	if ( m_CDTracks > 1 )
	{
		Track = rand() % ( m_CDTracks - 1 ) + 1;
	}
	if ( Track > m_CDTracks )
	{
		Track = m_CDTracks;
	}

	m_CurrentTrack = Track;

	float32	MusicVolume	= ( float32 ) m_MusicVolume / 100.0f;

	FSOUND_CD_Stop( 0 );
	FSOUND_CD_SetPlayMode( 0, FSOUND_CD_PLAYRANDOM );
	FSOUND_CD_Play( 0, Track );
	FSOUND_CD_SetVolume( 0, (int32)floor( MusicVolume * 255 + .5f ) );
	UpdateVolume();
	m_CDMusicPlaying = kTrue;
} // CSoundManager::StartCDMusic()

// start a CD playing - will automatically skip to the next track when done
void CSoundManager::StartCDMusicContinuous( void )
{
	if ( !m_HasMusicCD || m_MusicVolume == 0 )
	{
		return;
	}
	int32	Track( m_CurrentTrack );
	if ( Track > m_CDTracks )
	{
		Track = 1;
	}

	m_CurrentTrack = Track;

	float32	MusicVolume	= ( float32 ) m_MusicVolume / 100.0f;

	FSOUND_CD_Stop( 0 );
	FSOUND_CD_SetPlayMode( 0, FSOUND_CD_PLAYCONTINUOUS );
	FSOUND_CD_Play( 0, Track );
	FSOUND_CD_SetVolume( 0, (int32)floor( MusicVolume * 255 + .5f ) );
	m_CurrentTrack++;
	if ( m_CurrentTrack > m_CDTracks )
	{
		m_CurrentTrack = 1;
	}
	UpdateVolume();
	m_CDMusicPlaying = kTrue;
} // CSoundManager::StartCDMusicContinuous()

// play a specific track on the cd
void CSoundManager::StartCDMusic( int32 Track )		// track to play
{
	if ( !m_HasMusicCD || m_MusicVolume == 0 )
	{
		return;
	}
	srand( GetTickCount() );

	if ( Track == -1 && m_CDTracks > 1 )
	{
		Track = rand() % ( m_CDTracks - 1 ) + 1;
	}
	if ( Track > m_CDTracks )
	{
		Track = m_CDTracks;
	}

	float32	MusicVolume	= ( float32 ) m_MusicVolume / 100.0f;

	FSOUND_CD_Stop( 0 );
	FSOUND_CD_SetPlayMode( 0, FSOUND_CD_PLAYCONTINUOUS );
	FSOUND_CD_Play( 0, Track );
	FSOUND_CD_SetVolume( 0, (int32)floor( MusicVolume * 255 + .5f ) );
	UpdateVolume();
	m_CDMusicPlaying = kTrue;
} // CSoundManager::StartCDMusic()

// return how many tracks are available on CD
uint32	CSoundManager::CDTracks( void )
{
	return m_CDTracks;
} // CSoundManager::CDTracks()

// start a new music stream playing
void CSoundManager::StartMusic( std::string Path,	// path to the music file to stream
							    bool Looping )		// loop the music track?
{
	if ( m_MusicVolume == 0 )
	{
		return;
	}

	StopMusic();
	Path = AssembleAbsolutePath( m_ExecutablePath, Path );
	if( Looping )
	{
		m_MusicStream = FSOUND_Stream_Open( ( Path ).c_str(),
										FSOUND_LOOP_NORMAL |
										FSOUND_NORMAL |
										FSOUND_2D,
										0,
										0 );
	}
	else
	{
		m_MusicStream = FSOUND_Stream_Open( ( Path ).c_str(),
										FSOUND_LOOP_OFF |
										FSOUND_NORMAL |
										FSOUND_2D,
										0,
										0 );
	}
	m_MusicChannel = FSOUND_Stream_Play( FSOUND_FREE, m_MusicStream );

	UpdateVolume();
} // CSoundManager::StartMusic()

bool CSoundManager::StartMusicFromFolder( std::string Path )	// path to the music folder to look in
{		
	if( Path.length() == 0 )
	{
		return kFalse;
	}
	srand( GetTickCount() );
	uint32 Depth( 0 );
	uint32 FilesChecked( 0 );

	// first, find all MP3 files in this folder.
	std::vector< std::string > MusicFiles;
        
	GetMusicFromFolder( Path,
						MusicFiles,
						Depth,
						FilesChecked );

	// if we found music, pick a random track, and play it
	if( MusicFiles.size() > 0 )
	{
		// now select a random mp3 file.
		uint32 Index = rand() % MusicFiles.size();

		if( Index >= MusicFiles.size() )
		{
			Index = MusicFiles.size();
		}
		StartMusic( MusicFiles[Index], kFalse );
		return kTrue;
	}
	else
	{
		return kFalse;
	}
} // CSoundManager::StartMusicFromFolder()

void CSoundManager::GetMusicFromFolder( std::string Path,					// path to the music folder to look in
									    std::vector< std::string >& Paths,	// list of paths to add to
										uint32& Depth,						// subfolder depth
										uint32& FilesChecked )				// number of files checked
{
	Depth++;
	if( Depth > KMaximumMusicSearchDepth )
	{
		Depth--;
		return;
	}
	if( FilesChecked > KMaximumMusicFileChecks )
	{
		return;
	}

	WIN32_FIND_DATA FindFileData;
	HANDLE Find;

	Find = FindFirstFile( ( Path + "/*.*" ).c_str(), &FindFileData );

	BOOL Done( kFalse );
       
	while ( Find != INVALID_HANDLE_VALUE && !Done )
    {
		std::string Name = FindFileData.cFileName;
		FilesChecked++;
		if( FilesChecked > KMaximumMusicFileChecks )
		{
			return;
		}
		
		if( !( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			if( Name.length() > 4 )
			{
				std::string Extension = StringUpper( Name.substr( Name.length() - 3, 3 ) );
				if( Extension == "MP3" || Extension == "OGG" )
				{
					Paths.push_back( Path + "/" + Name );

				}
			}
		}
		else
		{
			if( Name != "." && Name != ".." )
			{
				GetMusicFromFolder( Path + "/" + Name,
									Paths,
									Depth,
									FilesChecked );
			}
		}
		Done = !FindNextFile( Find, &FindFileData );
	}
	Depth --;

	FindClose( Find );
} // CSoundManager::GetMusicFromFolder()

// is any music playing?
bool CSoundManager::MusicPlaying( void )
{
	if ( m_MusicStream != NULL )
	{
		return kTrue;
	}
	if( m_CDMusicPlaying )
	{
		return kTrue;
	}
	return kFalse;
} // CSoundManager::MusicPlaying()

// stop all playing music, both music streams and CDs
void CSoundManager::StopMusic( void )
{
	if ( m_HasMusicCD )
	{
		if ( m_CDMusicPlaying )
		{
			m_CDMusicPlaying = kFalse;
			FSOUND_CD_Stop( 0 );
		}
	}

	if ( m_MusicStream != NULL )
	{
		FSOUND_Stream_Close( m_MusicStream );
		m_MusicStream = NULL;
	}
	m_MusicChannel = -1;
} // CSoundManager::StopMusic()

// make sure that our volume is in sync with the settings
void CSoundManager::Update( void )
{
	int32 MasterVolume( m_pSettings.GetSettings( KSetMasterVolume ) );
	int32 SFXVolume( m_pSettings.GetSettings( KSetSoundFXVolume ) );
	int32 MusicVolume( m_pSettings.GetSettings( KSetMusicVolume ) );

	if( MasterVolume != m_MasterVolume ||
		SFXVolume != m_SFXVolume ||
		MusicVolume != m_MusicVolume )
	{
		SetMasterVolume( MasterVolume );
		SetSFXVolume( SFXVolume );
		SetMusicVolume( MusicVolume );
		UpdateVolume();
	}

	if ( m_MusicStream != NULL )
	{
		int32 StreamPos = FSOUND_Stream_GetTime( m_MusicStream );
		if( StreamPos >= FSOUND_Stream_GetLengthMs( m_MusicStream ) )
		{
			FSOUND_Stream_Close( m_MusicStream );
			m_MusicStream = NULL;
		}
	}

} // CSoundManager::Update

void CSoundManager::UpdateVolume( void )	// apply all volume settings to active streams and sounds
{
	if ( m_Muted )
	{
		return;
	}
	float32	MasterVolume( ( float32 ) m_MasterVolume / 100.0f );
	float32	SFXVolume( ( float32 ) m_SFXVolume / 100.0f );
	float32	MusicVolume( ( float32 ) m_MusicVolume / 100.0f );

	SFXVolume *= MasterVolume;
	MusicVolume *= MasterVolume;

	FSOUND_SetSFXMasterVolume( (int32)floor( SFXVolume * 255 + .5f ) );
	if ( m_MusicChannel != -1 )
	{
		FSOUND_SetVolumeAbsolute( m_MusicChannel, (int32)floor( MusicVolume * 255 + .5f ) );
	}
	if ( m_MusicVolume == 0 )
	{
		StopMusic();
	}
	if ( m_HasMusicCD )
	{
		FSOUND_CD_SetVolume( 0, (int32)floor( MusicVolume * 255 + .5f ) );
	}

} // CSoundManager::UpdateVolume()

void CSoundManager::SetMasterVolume( int32 Volume )	// volume to set at ( 0-100% )
{
	m_MasterVolume = Volume;
	UpdateVolume();
} // CSoundManager::SetMasterVolume()

void CSoundManager::SetSFXVolume( int32 Volume )	// volume to set at ( 0-100% )
{
	m_SFXVolume = Volume;
	UpdateVolume();
} // CSoundManager::SetSFXVolume()

void CSoundManager::SetMusicVolume( int32 Volume )	// volume to set at ( 0-100% )
{
	m_MusicVolume = Volume;
	UpdateVolume();
} // CSoundManager::SetMusicVolume()

// mute all sounds
void CSoundManager::Mute( void )
{
	m_Muted = kTrue;
	if ( m_MusicChannel != -1 )
	{
		FSOUND_SetVolumeAbsolute( m_MusicChannel, 0 );
	}
	if ( m_HasMusicCD && m_CDMusicPlaying )
	{
		FSOUND_CD_SetPaused( 0, kTrue );
	}
	FSOUND_SetSFXMasterVolume( 0 );
} // CSoundManager::Mute()

// unmute all sounds
void CSoundManager::UnMute( void )
{
	m_Muted = kFalse;
	UpdateVolume();
	if ( m_HasMusicCD )
	{
		if ( FSOUND_CD_GetPaused( 0 ) )
		{
			FSOUND_CD_SetPaused( 0, kFalse );
		}
	}
} // CSoundManager::UnMute()