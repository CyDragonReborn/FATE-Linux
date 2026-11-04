/***************************************************************************************
 *
 * AudioClip.cpp
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
#include "WT/Audio/AudioClip.h"
#include "WT/Audio/Channel.h"
#include "fmod.h"


namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS(CAudioClip)

///
CAudioClip::CAudioClip( const std::string& Filename, AudioModeFlags Flags )
{
	if (Flags != KAudioMode_Default)
	{
		WT_THROW( System::CArgumentException, "Flags not KAudioMode_Default" );
	}

	m_pSample = FSOUND_Sample_Load(	FSOUND_FREE,
		Filename.c_str(),
		FSOUND_LOOP_OFF,
		0,
		0 );

	if (m_pSample == NULL)
	{
		WT_THROW( CAudioException, "FSOUND_Sample_Load failure" );
	}
}

///
CAudioClip::~CAudioClip()
{
	assert( m_pSample != NULL );
	FSOUND_Sample_Free( m_pSample );
}

///
HChannel CAudioClip::Play(	ELoopMode LoopMode,
										EStartMode StartMode,
										CChannel** ppChannel,
										CAudioProperties* Props )
{
	assert( m_pSample != NULL );

	HChannel hChannel;

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
		int retChannel = FSOUND_PlaySoundEx( chnl, m_pSample, NULL, TRUE );
		if( retChannel != -1 )
		{
			hChannel->SetChannel( static_cast<int32>(retChannel) );
			hChannel->SetStream( NULL );
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
void CAudioClip::SetLoopPoints(	uint32 StartPoint,
											uint32 EndPoint )
{
	assert( m_pSample != NULL );

	if (StartPoint >= EndPoint)
	{
		WT_THROW( System::CArgumentException, "StartPoint >= EndPoint" );	
	}

	if (EndPoint >= GetLength())
	{
		WT_THROW( System::CArgumentOutOfRangeException, "EndPoint" );	
	}
	
	if (FSOUND_Sample_SetLoopPoints( m_pSample,
			static_cast<int>(StartPoint), static_cast<int>(EndPoint) ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Sample_SetLoopPoints failure" );
	}
}

///
void CAudioClip::GetLoopPoints(	uint32* StartPoint,
											uint32* EndPoint ) const
{
	assert( m_pSample != NULL );

	int loopstart, loopend;
    if (FSOUND_Sample_GetLoopPoints( m_pSample, &loopstart, &loopend ) != TRUE )
	{
		WT_THROW( CAudioException, "FSOUND_Sample_GetLoopPoints failure" );	
	}

	if( StartPoint != NULL)
	{
		*StartPoint = static_cast<uint32>(loopstart);
	}

	if( EndPoint != NULL)
	{
		*EndPoint = static_cast<uint32>(loopend);
	}
}

///
uint32 CAudioClip::GetLength() const
{
	assert( m_pSample != NULL );
	return static_cast<uint32>( FSOUND_Sample_GetLength( m_pSample ) );
}

}	// namespace Audio
}	// namespace WT
