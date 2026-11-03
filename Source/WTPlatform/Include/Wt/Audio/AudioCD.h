/***************************************************************************************
 *
 * AudioCD.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/18/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/AudioTypes.h"
#include "WT/Audio/AudioProperties.h"
#include "WT/Audio/Channel.h"


namespace WT
{
namespace Audio
{

// CD Track End Callback
class CAudioCD;
typedef int8 (__cdecl *AudioCDTrackEndCallback) (CAudioCD *Cd, int32 Param );

#define	KMaxCdTracks	100

typedef System::SmartPointer<class CAudioCD> HAudioCD;
class CAudioCD : public System::CObject
{
public:

	WT_CLASS_NO_CREATOR( CAudioCD, System::CObject );

	CAudioCD( const std::string& driveLetter, uint32 BufferMs );
	virtual ~CAudioCD( );

    virtual HChannel	Play( int32 Track,
							  ELoopMode LoopMode,
							  EStartMode StartMode,
							  CChannel** ppChannel,
							  CAudioProperties* Props );

	virtual int32			GetTrack( ) const;
	virtual int32			GetTrackCount( ) const;
	virtual int32			GetTrackLengthMs( int32 Track ) const;

	virtual void			SetEndCallback(	AudioCDTrackEndCallback Cb,
											int32 UserData );
    
	virtual void			GetEndCallback(	AudioCDTrackEndCallback* pCb,
											int32* pUserData );

	FSOUND_STREAM*			GetStream()	{ return m_pCd; }

private:

	bool					DidCdOpenError( FSOUND_STREAM* pStream );

private:

	FSOUND_STREAM*	m_pCd;
	int32			m_Track;

	AudioCDTrackEndCallback	m_EndCb;
	int32						m_EndCbUserData;

	int32			m_NumTracks;
	int32			m_TrackLength[ KMaxCdTracks ];
};

}	// namespace Audio
}	// namespace WT

