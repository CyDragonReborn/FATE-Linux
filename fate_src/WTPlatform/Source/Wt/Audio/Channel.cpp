/***************************************************************************************
 *
 * Channel.cpp
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
#include "WT/Audio/Channel.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CChannel)

CChannel::CChannel() :
	m_Channel( KInvalidChannel ),
	m_Pan( KCenterPan ),
	m_Volume( 1.0f ),
	m_pStream( NULL )
{
}

///
CChannel::CChannel( int32 Channel, FSOUND_STREAM* pStream ) :
	m_Channel( Channel ),
	m_Pan( KCenterPan ),
	m_Volume( 1.0f ),
	m_pStream( pStream )
{
	int32 const realchnl( Channel & 0xfff );

	if (realchnl < 0 || realchnl >= FSOUND_GetMaxChannels())
	{
		WT_THROW( System::CArgumentOutOfRangeException, "Channel & 0xfff" );
	}
}

///
CChannel::~CChannel()
{
}

///
bool CChannel::IsPlaying( ) const
{
	return ( (FSOUND_IsPlaying( m_Channel ) == TRUE) ? true : false );
}

///
void CChannel::Stop( )
{
	signed char ret;
	if( m_pStream != NULL )
	{
		ret = FSOUND_Stream_Stop( m_pStream );
		if( !ret )
		{
			WT_THROW( CAudioException, "FSOUND_Stream_Stop failure" );
		}
	}
	else
	{
		ret = FSOUND_StopSound( m_Channel );
		if( !ret )
		{
			WT_THROW( CAudioException, "FSOUND_StopSound failure" );
		}
	}
}

///
void CChannel::SetFrequencyHz( int32 Frequency )
{
	int32 freq( std::max<int32>( 0, Frequency ) );	// clip < 0 to 0
	if( ! FSOUND_SetFrequency( m_Channel, freq ) )
	{
		WT_THROW( CAudioException, "FSOUND_SetFrequency failure" );
	}
}

///
void CChannel::SetLoopMode( ELoopMode LoopMode )
{
	signed char ret;
	unsigned int mode = ((LoopMode == KLoopNormal) ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF );

	if( m_pStream != NULL )
	{
		ret = FSOUND_Stream_SetMode( m_pStream, mode );
		if (!ret)
		{
			WT_THROW( CAudioException, "FSOUND_Stream_SetMode failure" );
		}
	}
	else
	{
		ret = FSOUND_SetLoopMode( m_Channel, mode );
		if (!ret)
		{
			WT_THROW( CAudioException, "FSOUND_SetLoopMode failure" );
		}
	}
}

///
void CChannel::SetMute( bool Mute )
{
	signed char mute( ( Mute == true ) ? TRUE : FALSE );
	if (!FSOUND_SetMute( m_Channel, mute ))
	{
		WT_THROW( CAudioException, "FSOUND_SetMute failure" );
	}
}

// -1.0f(left) to 1.0f(right)
///
void CChannel::SetPan( float32 Pan )
{
	int iPan;

	if( Pan == KStereoPan )		iPan = FSOUND_STEREOPAN;
	else if( Pan < KLeftPan )		iPan = 0;
	else if( Pan > KRightPan )	iPan = 255;
	else if( Pan <= KCenterPan )	iPan = static_cast<int>( Float32ToInt32( (Pan + 1.0f) * 128 ) );
	else							iPan = static_cast<int>( Float32ToInt32( (Pan * 127) + 128 ) );

	if( FSOUND_SetPan( m_Channel, iPan ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_SetPan failure" );
	}

	if( iPan == 0 )			m_Pan = KLeftPan;
	else if( iPan == 255 )	m_Pan = KRightPan;
	else					m_Pan = Pan;
}

///
void CChannel::SetPaused( bool Paused )
{
	signed char paused( ( Paused == true ) ? TRUE : FALSE );
	if (!FSOUND_SetPaused( m_Channel, paused ))
	{
		WT_THROW( CAudioException, "FSOUND_SetPaused failure" );
	}
}

///
void CChannel::SetPriority( int32 Priority )
{
	// 0 to 255
	int32 pri = std::min<int32>( std::max<int32>( 0, Priority ), 255 );
	if (!FSOUND_SetPriority( m_Channel, pri ))
	{
		WT_THROW( CAudioException, "FSOUND_SetPriority failure" );
	}
}

///
void CChannel::SetReserved( bool Reserved )
{
	signed char reserved( ( Reserved == true ) ? TRUE : FALSE );
	if (!FSOUND_SetReserved( m_Channel, reserved ))
	{
		WT_THROW( CAudioException, "FSOUND_SetReserved failure" );
	}
}


///
void CChannel::SetVolume( float32 Volume )
{
	return SetVolume( Volume, false );
}

// absolute defaults to false, true means absolute volume which overrides Master volumes affect.
///
void CChannel::SetVolume( float32 Volume, bool Absolute )
{
	Volume = std::min( std::max( 0.0f, Volume ), 1.0f );
	signed char ret;
	int vol = static_cast<int>( Float32ToInt32( 255 * Volume ) );
	if( Absolute == FALSE )
	{
		ret = FSOUND_SetVolume( m_Channel, vol );
		if (ret == FALSE)
		{
			WT_THROW( CAudioException, "FSOUND_SetVolume failure" );
		}
	}
	else
	{
		ret = FSOUND_SetVolumeAbsolute( m_Channel, vol );
		if (ret == FALSE)
		{
			WT_THROW( CAudioException, "FSOUND_SetVolumeAbsolute failure" );
		}
	}

	m_Volume = Volume;
}

///
void CChannel::SetPosition( uint32 SamplePosition )
{
	if (!FSOUND_SetCurrentPosition( m_Channel, SamplePosition ))
	{
		WT_THROW( CAudioException, "FSOUND_SetCurrentPosition failure" );
	}
}

///
int32 CChannel::GetFrequencyHz( ) const
{
	return static_cast<int32>( FSOUND_GetFrequency( m_Channel ) );
}

///
ELoopMode CChannel::GetLoopMode( ) const
{
	unsigned int mode = 0;
	if( m_pStream != NULL )
	{
		mode = FSOUND_Stream_GetMode( m_pStream );
	}
	else
	{
		mode = FSOUND_GetLoopMode( m_Channel );
	}

	return( ( mode & FSOUND_LOOP_NORMAL ) ? KLoopNormal : KLoopOff );
}

///
bool CChannel::GetMute( ) const
{
	return( ( FSOUND_GetMute( m_Channel ) == TRUE ) ? true : false );
}

///
float32 CChannel::GetPan( ) const
{
	return m_Pan;
}

///
bool CChannel::GetPaused( ) const
{
	return( ( FSOUND_GetPaused( m_Channel ) == TRUE ) ? true : false );
}

///
int32 CChannel::GetPriority( ) const
{
	return static_cast<int32>( FSOUND_GetPriority( m_Channel) );
}

///
bool CChannel::GetReserved( ) const
{
	return( ( FSOUND_GetReserved( m_Channel ) == TRUE ) ? true : false );
}

// this gives you the volume, but doesn't provide if it's absolute or not (is this important?)
///
float32 CChannel::GetVolume( ) const
{
	return m_Volume;
}

// possibly we should return Result and pass a param for returning the position.
///
uint32 CChannel::GetPosition( ) const
{
	return static_cast<uint32>( FSOUND_GetCurrentPosition( m_Channel ) );
}

void CChannel::SetChannel( int32 iChannel )
{
	int32 realchnl( iChannel & 0xfff );
	if( realchnl < 0 || realchnl >= FSOUND_GetMaxChannels())
	{
		WT_THROW( System::CArgumentOutOfRangeException, "Channel & 0xfff" );
	}

	m_Channel = iChannel;
}

void CChannel::SetStream( FSOUND_STREAM* pStream )
{
	m_pStream = pStream;
}

///
int32 CChannel::GetChannel() const
{
	// Fmod absolute channel value
	if( m_Channel != KInvalidChannel )
	{
		return (m_Channel & 0xfff);
	}
	return KInvalidChannel;
}


}	// namespace Audio
}	// namespace WT
