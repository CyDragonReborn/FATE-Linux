/***************************************************************************************
 *
 * AudioCD.cpp
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
#include "WT/Audio/AudioCD.h"
#include "WT/Audio/Channel.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CAudioCD)

#pragma warning (push)
#pragma warning (disable: 4100)
#pragma warning (disable: 4312)
///
signed char F_CALLBACKAPI AudioCDEndCallback(	FSOUND_STREAM *stream,
														void *buff,
														int len,
														int param )
{
	int8 ret(0);
	CAudioCD* pCd = reinterpret_cast<CAudioCD*>(param);
	if( pCd != NULL )
	{
		AudioCDTrackEndCallback EndCb(NULL);
		int32 EndCbUserData(0);
		pCd->GetEndCallback( &EndCb, &EndCbUserData );
		if( EndCb != NULL )
		{			
			ret = EndCb( pCd, EndCbUserData );
		}
	}
	return (ret ? TRUE : FALSE);
}

#pragma warning (pop)


///
CAudioCD::CAudioCD( const std::string& driveLetter, uint32 BufferMs ) :
	m_Track( -1 ),
	m_EndCb( NULL ),
	m_EndCbUserData( 0 ),
	m_NumTracks( 0 )
{
	/*	Notes
	FSOUND_Stream_Open("d:", 0, 0, 0);		// open drive d: for playback
	FSOUND_Stream_Open("d:*?", 0, 0, 0);	// open drive d: for playback and get device info
	FSOUND_Stream_Open("d:*!", 0, 0, 0);	// quick open drive d: for fast track queries, no playback allowed.
	FSOUND_Stream_Open("d:*j", 0, 0, 0);	// j turns off jitter correction
	FSOUND_Stream_Open("d:*?!j", 0, 0, 0);	// combined is okay
	*/

	char drivestr[32];

	FSOUND_Stream_SetBufferSize( static_cast<int>(BufferMs) );

	// Quick Open and get track info
	strcpy(drivestr,"c:*!");
	drivestr[0] = driveLetter[0];
	FSOUND_STREAM* pStream = FSOUND_Stream_Open(drivestr,0,0,0);
	if (pStream == NULL)
	{
		WT_THROW( CAudioException, string( "FSOUND_Stream_Open failure for" ) + drivestr );
	}

	if (DidCdOpenError( pStream ))
	{
		FSOUND_Stream_Close( pStream );
		WT_THROW( CAudioException, string( "DidCdOpenError for " ) + drivestr );
	}

	int trackcnt = FSOUND_Stream_GetNumSubStreams( pStream );
	m_NumTracks = static_cast<int32>(trackcnt);
	for( int i=0; i<trackcnt; i++ )
	{
		if( FSOUND_Stream_SetSubStream( pStream, i ) == TRUE )
		{
			m_TrackLength[i] = static_cast<int32>( FSOUND_Stream_GetLengthMs( pStream ) );
		}
		else
		{
			m_TrackLength[i] = -1;
		}
	}
	FSOUND_Stream_Close( pStream );

	strcpy(drivestr,"c:");
	drivestr[0] = driveLetter[0];
	pStream = FSOUND_Stream_Open( drivestr, 0, 0, 0 );
	if (pStream == NULL)
	{
		WT_THROW( CAudioException, string( "FSOUND_Stream_Open failure for " ) + drivestr );
	}

	if (DidCdOpenError( pStream ))
	{
		FSOUND_Stream_Close( pStream );
		WT_THROW( CAudioException, string( "DidCdOpenError for " ) + drivestr );
	}

	m_pCd = pStream;
}

///
CAudioCD::~CAudioCD()
{
	// Closing Non-Blocking Streams shouldn't be done in the destructor
	assert( m_pCd != NULL );
	FSOUND_Stream_Close( m_pCd );
}

///
HChannel CAudioCD::Play(	int32 Track,
								   ELoopMode LoopMode,
								   EStartMode StartMode,
								   CChannel** ppChannel,
								   CAudioProperties* Props )
{
	assert( m_pCd != NULL );

	HChannel hChannel;

	if (FSOUND_Stream_SetSubStream( m_pCd, static_cast<int>(Track) ) == TRUE)
	{
		int chnl( FSOUND_FREE );

		if( ppChannel != NULL )
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
			int retChannel = FSOUND_Stream_PlayEx( chnl, m_pCd, NULL, TRUE );
			if( retChannel != -1 )
			{
				hChannel->SetChannel( static_cast<int32>(retChannel) );
				hChannel->SetStream( m_pCd );
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

					m_Track = Track;
					return hChannel;
				}
			}
		}
	}

	m_Track = -1;
	return NULL;
}

///
int32 CAudioCD::GetTrack( ) const
{
	return m_Track;
}

///
int32 CAudioCD::GetTrackCount( ) const
{
	return m_NumTracks;
}

///
int32 CAudioCD::GetTrackLengthMs( int32 Track ) const
{
	// TODO: code this
	if( Track >= 0 && Track < m_NumTracks )
	{
		return m_TrackLength[ Track ];
	}
	return 0;
}

#pragma warning (push)
#pragma warning (disable: 4311)
///
void CAudioCD::SetEndCallback(	AudioCDTrackEndCallback Cb,
											int32 UserData )
{
	assert( m_pCd != NULL );

	if( FSOUND_Stream_SetEndCallback(	m_pCd,
										AudioCDEndCallback,
										reinterpret_cast<int>(this) ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Stream_SetEndCallback failure" );
	}

	m_EndCb = Cb;
	m_EndCbUserData = UserData;
}
#pragma warning (pop)

///
void CAudioCD::GetEndCallback( AudioCDTrackEndCallback* pCb, int32* pUserData )
{
	assert( pCb != NULL && pUserData != NULL );
	*pCb = m_EndCb;
	*pUserData = m_EndCbUserData;
}

bool CAudioCD::DidCdOpenError( FSOUND_STREAM* pStream )
{
	if( pStream != NULL )
	{
		int tagcnt(0);
		if( FSOUND_Stream_GetNumTagFields( pStream, &tagcnt ) == TRUE )
		{
			int tagtype;
			char* tagname(NULL);
			void* tagvalue;
			int taglen;

			for( int i=0; i<tagcnt; i++ )
			{
				if( FSOUND_Stream_GetTagField( pStream, i, &tagtype, &tagname, &tagvalue, &taglen ) == TRUE )
				{
					if( ::strncmp( tagname, "CD_ERROR", 8 ) == 0 )
					{
						return true;	// CD_ERROR occured
					}
				}
			}
		}
		return false;
	}
	return true;
}


}	// namespace Audio
}	// namespace WT
