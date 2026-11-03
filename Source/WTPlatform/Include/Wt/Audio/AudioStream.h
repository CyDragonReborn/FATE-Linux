/***************************************************************************************
 *
 * AudioStream.h
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
#include "WT/Audio/DspUnit.h"
#include "WT/Audio/Channel.h"

struct FSOUND_STREAM;

namespace WT
{
namespace Audio
{

// Stream End Callback
class CAudioStream;
typedef int8 (__cdecl *AudioStreamEndCallback) (CAudioStream *Stream, int32 Param );
// Stream Marker Callback
typedef int8 (__cdecl *AudioStreamMarkerCallback) (CAudioStream *Stream, int32 Param, const std::string& MarkerName, uint32 MarkerOffset );

typedef System::SmartPointer<class CAudioStream> HAudioStream;
class CAudioStream : public System::CObject
{
public:

	WT_CLASS_NO_CREATOR( CAudioStream, System::CObject );

    CAudioStream( const std::string& Filename, AudioModeFlags Flags, uint32 BufferMs );
	virtual ~CAudioStream();

	virtual HChannel		Play( ELoopMode LoopMode,
								  EStartMode StartMode,
								  CChannel** ppChannel,
								  CAudioProperties* Props );

	// Event support
	virtual void			SetEndCallback( AudioStreamEndCallback Cb, int32 UserData );

	// Marker support
	virtual void			AddMarker( const std::string& MarkerName, uint32 MarkerOffset );
	virtual void			DeleteMarker( int32 MarkerIndex );
	virtual int32				GetNumMarkers() const;
	// pass null for markerName or offset to ignore value. returns true on success
	virtual void			GetMarkerInfo( int32 MarkerIndex, std::string* MarkerName, uint32* MarkerOffset );
	virtual void			SetMarkerCallback( AudioStreamMarkerCallback Cb, int32 UserData );

    virtual void				GetEndCallback( AudioStreamEndCallback* pCb, int32* pUserData );
	virtual void				GetMarkerCallback( AudioStreamMarkerCallback* pCb, int32* pUserData );

	FSOUND_STREAM*				GetStream()	{ return m_pStream; }

private:

	FSOUND_STREAM*				m_pStream;

	AudioStreamEndCallback		m_EndCb;
	int32						m_EndCbUserData;

	AudioStreamMarkerCallback	m_MarkerCb;
	int32						m_MarkerCbUserData;
};

}	// namespace Audio
}	// namespace WT

