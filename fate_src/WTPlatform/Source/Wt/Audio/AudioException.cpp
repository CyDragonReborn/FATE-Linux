/***************************************************************************************
 *
 * AudioException.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 4/19/2006: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include <sstream>
#include "WT/Audio/AudioException.h"
#include "fmod.h"

namespace WT
{
namespace Audio
{

WT_DEFINE_CLASS( CAudioException )

CAudioException::CAudioException( std::string const& Message,
	std::string const& File, int32 Line, std::string const& Function )
	: System::CException( Message, File, Line, Function )
{
	m_FMODError = FSOUND_GetError();
	
	switch (m_FMODError)
	{
		case FMOD_ERR_NONE             : m_FMODErrorString = "No errors"; break;
		case FMOD_ERR_BUSY             : m_FMODErrorString = "Cannot call this command after FSOUND_Init.  Call FSOUND_Close first."; break;
		case FMOD_ERR_UNINITIALIZED    : m_FMODErrorString = "This command failed because FSOUND_Init or FSOUND_SetOutput was not called"; break;
		case FMOD_ERR_INIT             : m_FMODErrorString = "Error initializing output device."; break;
		case FMOD_ERR_ALLOCATED        : m_FMODErrorString = "Error initializing output device, but more specifically, the output device is already in use and cannot be reused."; break;
		case FMOD_ERR_PLAY             : m_FMODErrorString = "Playing the sound failed."; break;
		case FMOD_ERR_OUTPUT_FORMAT    : m_FMODErrorString = "Soundcard does not support the features needed for this soundsystem (16bit stereo output)"; break;
		case FMOD_ERR_COOPERATIVELEVEL : m_FMODErrorString = "Error setting cooperative level for hardware."; break;
		case FMOD_ERR_CREATEBUFFER     : m_FMODErrorString = "Error creating hardware sound buffer."; break;
		case FMOD_ERR_FILE_NOTFOUND    : m_FMODErrorString = "File not found"; break;
		case FMOD_ERR_FILE_FORMAT      : m_FMODErrorString = "Unknown file format"; break;
		case FMOD_ERR_FILE_BAD         : m_FMODErrorString = "Error loading file"; break;
		case FMOD_ERR_MEMORY           : m_FMODErrorString = "Not enough memory or resources"; break;
		case FMOD_ERR_VERSION          : m_FMODErrorString = "The version number of this file format is not supported"; break;
		case FMOD_ERR_INVALID_PARAM    : m_FMODErrorString = "An invalid parameter was passed to this function"; break;
		case FMOD_ERR_NO_EAX           : m_FMODErrorString = "Tried to use an EAX command on a non EAX enabled channel or output."; break;
		case FMOD_ERR_CHANNEL_ALLOC    : m_FMODErrorString = "Failed to allocate a new channel"; break;
		case FMOD_ERR_RECORD           : m_FMODErrorString = "Recording is not supported on this machine"; break;
		case FMOD_ERR_MEDIAPLAYER      : m_FMODErrorString = "Windows Media Player not installed so cannot play wma or use internet streaming."; break;
		case FMOD_ERR_CDDEVICE         : m_FMODErrorString = "An error occured trying to open the specified CD device"; break;
		default						   : m_FMODErrorString = "(invalid FMOD error code detected)"; break;
	}

	std::ostringstream o;
	o << GetMessage() << "; FMOD error " << m_FMODError << ": \"" << m_FMODErrorString << '"';
	SetMessage( o.str() );
}

} //namespace Audio
} //namespace WT
