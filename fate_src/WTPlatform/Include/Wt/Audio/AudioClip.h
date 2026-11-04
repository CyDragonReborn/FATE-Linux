/***************************************************************************************
 *
 * AudioClip.h
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
#include "WT/Audio/AudioProperties.h"
#include "WT/Audio/Channel.h"

struct FSOUND_SAMPLE;

namespace WT
{
namespace Audio
{

typedef System::SmartPointer<class CAudioClip> HAudioClip;
class CAudioClip : public System::CObject
{
public:
    
	WT_CLASS_NO_CREATOR( CAudioClip, System::CObject );

	CAudioClip( const std::string& Filename, AudioModeFlags Flags );
	virtual ~CAudioClip( );
	
	virtual HChannel		Play(	ELoopMode LoopMode,
										EStartMode StartMode,
										CChannel** ppChannel,
										CAudioProperties* Props );

	virtual void			SetLoopPoints( uint32 StartPoint, uint32 EndPoint );
	virtual void			GetLoopPoints( uint32* StartPoint, uint32* EndPoint ) const;
	virtual uint32				GetLength() const;
	
private:

	FSOUND_SAMPLE*	m_pSample;
};

}	// namespace Audio
}	// namespace WT

