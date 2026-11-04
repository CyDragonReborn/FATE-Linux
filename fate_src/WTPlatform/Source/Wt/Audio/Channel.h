/***************************************************************************************
 *
 * Channel.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/17/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/AudioTypes.h"

struct FSOUND_STREAM;

namespace WT
{
namespace Audio
{

static const float32 KLeftPan		= -1.0f;
static const float32 KCenterPan	= 0;
static const float32 KRightPan	= 1.0f;
static const float32 KStereoPan	= static_cast<float32>(0xFFFFFFFF);

typedef System::SmartPointer<class CChannel> HChannel;
class CChannel : public System::CObject
{
public:

	WT_CLASS( CChannel, System::CObject );

	CChannel();
	CChannel( int32 Channel, FSOUND_STREAM* pStream );
	virtual ~CChannel( );

	virtual bool		IsPlaying( ) const;
	virtual void		Stop( );

	virtual void		SetFrequencyHz( int32 Frequency );
	virtual void		SetLoopMode( ELoopMode LoopMode );
	virtual void		SetMute( bool Mute );
	virtual void		SetPan( float32 Pan );		// -1.0f(left) to 1.0f(right)
	virtual void		SetPaused( bool Paused );
	virtual void		SetPriority( int32 Priority );
	virtual void		SetReserved( bool Reserved );	// DO WE NEED OR WANT THIS ANYMORE?  OR SHOULD WE MANAGE CHANNEL RESERVATIONS?
	virtual void		SetVolume( float Volume );
	virtual void		SetVolume( float32 Volume, bool Absolute );	// absolute defaults to false, true means absolute volume which overrides Master volumes affect.
	virtual void		SetPosition( uint32 SamplePosition );

	virtual int32			GetFrequencyHz( ) const;
	virtual ELoopMode		GetLoopMode( ) const;
	virtual bool			GetMute( ) const;
	virtual float32			GetPan( ) const;
	virtual bool			GetPaused( ) const;
	virtual int32			GetPriority( ) const;
	virtual bool			GetReserved( ) const;
	virtual float32			GetVolume( ) const;	// this gives you the volume, but doesn't provide if it's absolute or not (is this important?)
	virtual uint32			GetPosition( ) const; // possibly we should return Result and pass a param for returning the position.
    
	virtual int32			GetChannel() const;
	virtual void			SetChannel( int32 iChannel );
	virtual void			SetStream( FSOUND_STREAM* pStream );

private:

	static const int32 KInvalidChannel = -1;

private:

	int32				m_Channel;
	float32				m_Pan;
	float32				m_Volume;
	FSOUND_STREAM*		m_pStream;
};

}	// namespace Audio
}	// namespace WT

