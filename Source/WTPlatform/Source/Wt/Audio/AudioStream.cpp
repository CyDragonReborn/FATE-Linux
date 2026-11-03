/***************************************************************************************
 *
 * AudioStream.cpp
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
#include "WT/Audio/AudioStream.h"
#include "WT/Audio/Channel.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CAudioStream)

#pragma warning (push)
#pragma warning (disable: 4100)
#pragma warning (disable: 4312)
///
signed char F_CALLBACKAPI myAudioStreamEndCallback( FSOUND_STREAM *stream,
												  void *buff,
												  int len,
												  int param )
{
	int8 ret(0);
	CAudioStream* pStream = reinterpret_cast<CAudioStream*>(param);
	if( pStream != NULL )
	{
		AudioStreamEndCallback EndCb(NULL);
		int32 EndCbUserData(0);
		pStream->GetEndCallback( &EndCb, &EndCbUserData );
		if( EndCb != NULL )
		{
//			IAudioStream* pIStream = static_cast<AudioStream*>(IAudioStream::Promote( pStream ));
			ret = EndCb( pStream, EndCbUserData );
		}
	}
	return (ret ? TRUE : FALSE);
}

///
signed char F_CALLBACKAPI myAudioStreamMarkerCallback(	FSOUND_STREAM *stream,
															void *buff,
															int len,
															int param )
{
	int8 ret(0);
	CAudioStream* pStream = reinterpret_cast<CAudioStream*>(param);
	if( pStream != NULL )
	{
		AudioStreamMarkerCallback MarkerCb(NULL);
		int32 MarkerCbUserData(0);
		pStream->GetMarkerCallback( &MarkerCb, &MarkerCbUserData );
		if( MarkerCb != NULL )
		{
			//IAudioStream* pIStream = static_cast<IAudioStream*>(IAudioStream::Promote( pStream ));
			std::string MarkerName( reinterpret_cast<char*>(buff) );
			uint32 MarkerOffset( static_cast<uint32>(len) );

			ret = MarkerCb( pStream, MarkerCbUserData, MarkerName, MarkerOffset );
		}
	}
	return (ret ? TRUE : FALSE);
}

#pragma warning (pop)

///
CAudioStream::CAudioStream( const std::string& Filename, AudioModeFlags Flags, uint32 BufferMs ) :
	m_EndCb(NULL),
	m_EndCbUserData(0)
{
#pragma message( "TODO: Make stream opening non-blocking in " __FILE__ )

	if (Flags != KAudioMode_Default)
	{
		WT_THROW( System::CArgumentException, "Flags not KAudioMode_Default" );
	}

	FSOUND_Stream_SetBufferSize( static_cast<int>(BufferMs) );

	m_pStream = FSOUND_Stream_Open(	Filename.c_str(),
													FSOUND_LOOP_OFF,
													0,
													0 );

	if (m_pStream == NULL)
	{
		WT_THROW( CAudioException, "FSOUND_Stream_Open failure" );
	}
}

///
CAudioStream::~CAudioStream()
{
	// Closing Non-Blocking Streams shouldn't be done in the destructor
	assert( m_pStream != NULL );
	FSOUND_Stream_Close( m_pStream );
}

///
HChannel CAudioStream::Play(	ELoopMode LoopMode,
										EStartMode StartMode,
										CChannel** ppChannel,
										CAudioProperties* Props )
{
	assert( m_pStream != NULL );

	HChannel hChannel;
	int chnl( FSOUND_FREE );

	if (ppChannel != NULL)
	{
		hChannel = *ppChannel;
		chnl = hChannel->GetChannel();
	}
	else
	{
		hChannel = new CChannel();
	}

	if (hChannel != NULL)
	{
		int retChannel = FSOUND_Stream_PlayEx( chnl, m_pStream, NULL, TRUE );
		if( retChannel != -1 )
		{
			hChannel->SetChannel( retChannel );
			hChannel->SetStream( m_pStream );
			{
				if( Props != NULL )
				{
					hChannel->SetFrequencyHz( Props->GetFrequencyHz() );
					hChannel->SetPan( Props->GetPan() );
					hChannel->SetPriority( Props->GetPriority() );
					hChannel->SetVolume( Props->GetVolume() );
				}
				else
				{
					hChannel->SetPan( hChannel->GetPan() );
					hChannel->SetVolume( hChannel->GetVolume() );
				}

				hChannel->SetLoopMode( LoopMode );

				if( StartMode == KStartPlaying )
				{
					hChannel->SetPaused( false );
				}

				return hChannel;
			}
		}
	}

	return NULL;
}

///
#pragma warning( push )
#pragma warning( disable: 4311 )
void CAudioStream::SetEndCallback(	AudioStreamEndCallback Cb,
												int32 UserData )
{
	assert( m_pStream != NULL );

	if( FSOUND_Stream_SetEndCallback(	m_pStream,
										myAudioStreamEndCallback,
										reinterpret_cast<int>(this) ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_SetEndCallback failure" );
	}

	m_EndCb = Cb;
	m_EndCbUserData = UserData;
}
#pragma warning( pop )

///
void CAudioStream::AddMarker(	const std::string& MarkerName,
										uint32 MarkerOffset )
{
	assert( m_pStream != NULL );

	FSOUND_SYNCPOINT* pMarker = FSOUND_Stream_AddSyncPoint(	m_pStream,
		static_cast<unsigned int>(MarkerOffset), MarkerName.c_str() );

	if( pMarker != NULL )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_AddSyncPoint failure" );
	}
}

///
void CAudioStream::DeleteMarker( int32 MarkerIndex )
{
	assert( m_pStream != NULL );

	if (FSOUND_Stream_GetNumSyncPoints( m_pStream ) <= 0 )	// required check to fix fmod bug
	{
		WT_THROW( CAudioException, "FSOUND_Stream_GetNumSyncPoints() <= 0" );
	}

	FSOUND_SYNCPOINT* pMarker = FSOUND_Stream_GetSyncPoint( m_pStream, static_cast<int>(MarkerIndex+1) );
	if( pMarker == NULL )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_GetSyncPoint failure" );
	}

	if( FSOUND_Stream_DeleteSyncPoint( pMarker ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_DeleteSyncPoint failure" );
	}
}

///
int32 CAudioStream::GetNumMarkers() const
{
	assert( m_pStream != NULL );
	return static_cast<int32>( FSOUND_Stream_GetNumSyncPoints( m_pStream ) );
}

// pass null for markerName or offset to ignore value. returns true on success
///
void CAudioStream::GetMarkerInfo(	int32 MarkerIndex,
											std::string* MarkerName,
											uint32* MarkerOffset )
{
	assert( m_pStream != NULL );

	if (MarkerIndex < 0 ||
		MarkerIndex >= FSOUND_Stream_GetNumSyncPoints( m_pStream ))
	{
		WT_THROW( System::CArgumentOutOfRangeException, "MarkerIndex" );
	}

	FSOUND_SYNCPOINT* pMarker = FSOUND_Stream_GetSyncPoint( m_pStream, static_cast<int>(MarkerIndex+1) );
	if( pMarker == NULL )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_GetSyncPoint failure" );
	}

	unsigned int offset;
	char* name = FSOUND_Stream_GetSyncPointInfo( pMarker, &offset );
	if (name == NULL)
	{
		WT_THROW( CAudioException, "FSOUND_Stream_GetSyncPointInfo failure" );
	}

	if( MarkerName != NULL )	*MarkerName = name;
	if( MarkerOffset != NULL )	*MarkerOffset = static_cast<uint32>(offset);
}

///
#pragma warning( push )
#pragma warning( disable: 4311 )
void CAudioStream::SetMarkerCallback(	AudioStreamMarkerCallback Cb,
												int32 UserData )
{
	assert( m_pStream != NULL );

	if( FSOUND_Stream_SetSyncCallback(	m_pStream,
										myAudioStreamEndCallback,
										reinterpret_cast<int>(this) ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_SetSyncCallback failure" );
	}

	m_MarkerCb = Cb;
	m_MarkerCbUserData = UserData;
}
#pragma warning( pop )

///
void CAudioStream::GetEndCallback( AudioStreamEndCallback* pCb, int32* pUserData )
{
	assert( pCb != NULL && pUserData != NULL );
	*pCb = m_EndCb;
	*pUserData = m_EndCbUserData;
}

///
void CAudioStream::GetMarkerCallback( AudioStreamMarkerCallback* pCb, int32* pUserData )
{
	assert( pCb != NULL && pUserData != NULL );
	*pCb = m_MarkerCb;
	*pUserData = m_MarkerCbUserData;
}


}	// namespace Audio
}	// namespace WT
