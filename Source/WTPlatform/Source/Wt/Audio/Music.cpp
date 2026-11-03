/***************************************************************************************
 *
 * Music.cpp
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 2/5/2003: Jason Glover:	Created
 *
 ***************************************************************************************/

// -----------------------------------------------------------------
// Include Files
// -----------------------------------------------------------------
#include "WT/stdafx.h"
#include "windows.h"
#include "WT/System/SystemException.h"
#include "WT/Audio/Music.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CMusic)

///
CMusic::CMusic( const std::string& Filename, AudioModeFlags Flags ) :
	m_Volume(1.0f),
	m_PanSeparation(1.0f),
	m_LoopMode(KLoopNormal),
	m_PlaybackRate(1.0f)
{
	if (Flags != KAudioMode_Default)
	{
		WT_THROW( System::CArgumentException, "Flags not KAudioMode_Default" );
	}

	m_pMusic = FMUSIC_LoadSong( Filename.c_str() );
	if (m_pMusic == NULL)
	{
		WT_THROW( CAudioException, string( "FMUSIC_LoadSong failure with " ) + Filename );
	}
}

///
CMusic::~CMusic()
{
	assert( m_pMusic != NULL );
	FMUSIC_FreeSong( m_pMusic );
}

///
void CMusic::Play( ELoopMode LoopMode )
{
	assert( m_pMusic != NULL );

	if( FMUSIC_PlaySong( m_pMusic ) != TRUE )
	{
		WT_THROW( CAudioException, "FMUSIC_PlaySong failure" );
	}

	try
	{
		SetLoopMode( LoopMode );
	}
	catch( CAudioException const& )
	{
		FMUSIC_StopSong( m_pMusic );
		throw;
	}
}

///
void CMusic::Stop( )
{
	assert( m_pMusic != NULL );

	if( FMUSIC_IsPlaying( m_pMusic ) == TRUE )
	{
		FMUSIC_StopSong( m_pMusic );
	}
}

///
bool CMusic::IsFinished() const
{
	assert( m_pMusic != NULL );

	return FMUSIC_IsFinished( m_pMusic ) == TRUE;
}

///
bool CMusic::IsPlaying() const
{
	assert( m_pMusic != NULL );

	return FMUSIC_IsPlaying( m_pMusic ) == TRUE;
}

///
void CMusic::SetPaused( bool Paused )
{
	assert( m_pMusic != NULL );

	signed char pause( (Paused == true) ? TRUE : FALSE );
	if (FMUSIC_SetPaused( m_pMusic, pause ) != TRUE)
	{
		WT_THROW( CAudioException, "FMUSIC_SetPaused failure" );
	}
}

///
void CMusic::SetLoopMode( ELoopMode LoopMode )
{
	assert( m_pMusic != NULL );

	signed char looping = ((LoopMode == KLoopNormal) ? TRUE : FALSE );
	if( FMUSIC_SetLooping( m_pMusic, looping ) != TRUE )
	{
		WT_THROW( CAudioException, "FMUSIC_SetLooping failure" );
	}

	m_LoopMode = LoopMode;
}

// 0 to 10.0f, default 1.0f normal
///
void CMusic::SetPlaybackRate( float32 Rate )
{
	assert( m_pMusic != NULL );

	Rate = std::min( std::max( 0.0f, Rate), 10.0f );
	if(FMUSIC_SetMasterSpeed( m_pMusic, Rate ) != TRUE )
	{
		WT_THROW( CAudioException, "FMUSIC_SetMasterSpeed failure" );
	}

	m_PlaybackRate = Rate;
}

///
void CMusic::SetVolume( float32 Volume )
{
	assert( m_pMusic != NULL );

	Volume = std::min( std::max( 0.0f, Volume), 1.0f );
	int vol = static_cast<int>( Float32ToInt32( 256 * Volume ) );
	if (FMUSIC_SetMasterVolume( m_pMusic, vol ) != TRUE )
	{
		WT_THROW( CAudioException, "FMUSIC_SetMasterVolume failure" );
	}

	m_Volume = Volume;
}

///
void CMusic::SetPanSeparation( float32 PanSeparation )
{
	assert( m_pMusic != NULL );

	PanSeparation = std::min( std::max( 0.0f, PanSeparation), 1.0f );
	if (FMUSIC_SetPanSeperation( m_pMusic, PanSeparation ) != TRUE )
	{
		WT_THROW( CAudioException, "FMUSIC_SetPanSeperation failure" );
	}

	m_PanSeparation = PanSeparation;
}

///
void CMusic::SetOrder( int32 Order )
{
	assert( m_pMusic != NULL );

	if (FMUSIC_SetOrder( m_pMusic, static_cast<int>(Order) ) != TRUE )
	{
		WT_THROW( CAudioException, "FMUSIC_SetOrder failure" );
	}
}

///
bool CMusic::GetPaused( ) const
{
	assert( m_pMusic != NULL );

	return FMUSIC_GetPaused( m_pMusic ) == TRUE;
}

///
ELoopMode CMusic::GetLoopMode( ) const
{
	return m_LoopMode;
}

///
float32 CMusic::GetPlaybackRate( ) const
{
	return m_PlaybackRate;
}

///
float32 CMusic::GetVolume() const
{
	return m_Volume;
}

///
float32 CMusic::GetPanSeparation( ) const
{
	return m_PanSeparation;
}

///
int32 CMusic::GetOrder() const
{
	assert( m_pMusic != NULL );
	return static_cast<int32>(FMUSIC_GetOrder( m_pMusic ));
}

///
int32 CMusic::GetTime() const
{
	assert( m_pMusic != NULL );
	return static_cast<int32>(FMUSIC_GetTime( m_pMusic ));
}

///
int32 CMusic::GetBeatsPerMinute() const
{
	assert( m_pMusic != NULL );
	return static_cast<int32>(FMUSIC_GetBPM( m_pMusic ));
}

///
int32 CMusic::GetNumOrders() const
{
	assert( m_pMusic != NULL );
	return static_cast<int32>(FMUSIC_GetNumOrders( m_pMusic ));
}


}	// namespace Audio
}	// namespace WT
