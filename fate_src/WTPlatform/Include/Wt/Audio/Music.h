/***************************************************************************************
 *
 * Music.h
 *
 * Copyright (C) 2003-2004 WildTangent, Inc.
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

struct FMUSIC_MODULE;

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CMusic> HMusic;
class CMusic : public System::CObject
{
public:
    
	WT_CLASS_NO_CREATOR( CMusic, System::CObject );

	CMusic( const std::string& Filename, AudioModeFlags Flags );
	virtual ~CMusic( );

	virtual void		Play( ELoopMode LoopMode );
	virtual void		Stop( );
	virtual bool			IsFinished() const;
	virtual bool			IsPlaying() const;

	virtual void		SetPaused( bool Paused );
	virtual void		SetLoopMode( ELoopMode LoopMode );
	virtual void		SetPlaybackRate( float32 Rate ); // 0 to 10.0f, default 1.0f normal
	virtual void		SetVolume( float32 Volume );	// master volume
	virtual void		SetPanSeparation( float32 PanSeparation );
	virtual void		SetOrder( int32 Order );

	virtual bool			GetPaused( ) const;
	virtual ELoopMode		GetLoopMode( ) const;
	virtual float32			GetPlaybackRate( ) const;
	virtual float32			GetVolume() const;
	virtual float32			GetPanSeparation( ) const;
	virtual int32			GetOrder() const;
	virtual int32			GetTime() const;
	virtual int32			GetBeatsPerMinute() const;
	virtual int32			GetNumOrders() const;
    
private:
	FMUSIC_MODULE*	m_pMusic;
	float32			m_Volume;
	float32			m_PanSeparation;
	ELoopMode		m_LoopMode;
	float32			m_PlaybackRate;
};

}	// namespace Audio
}	// namespace WT

