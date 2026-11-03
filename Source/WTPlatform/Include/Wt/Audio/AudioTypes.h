/***************************************************************************************
 *
 * AudioTypes.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/20/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "WT/Audio/AudioException.h"

namespace WT
{
namespace Audio
{

typedef uint32 AudioModeFlags;

enum ELoopMode
{
	KLoopOff,
	KLoopNormal
};

enum EStartMode
{
	KStartPlaying,
	KStartPaused
};

enum EAudioMode
{
	KAudioMode_Default	= 0x00000000
};



}	// namespace Audio
}	// namespace WT



