/***************************************************************************************
 *
 * AudioProperties.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/23/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace Audio
{

// Note: this class is an Audio property envelope.
// Member values are not range validated.  Range validation
// occurs when properties are set on channels.
class CAudioProperties
{
public:
	// freq, pan, volume, priority
	CAudioProperties(	int32	FrequencyHz,
						float32 Volume,
						float32 Pan,
						int32	Priority ) :
		m_FrequencyHz( FrequencyHz ),
		m_Volume( Volume ),
		m_Pan( Pan ),
		m_Priority( Priority ) {}

	void	SetFrequencyHz( int32 FrequencyHz )	{ m_FrequencyHz = FrequencyHz; }
	void	SetVolume( float32 Volume )			{ m_Volume = Volume; }
	void	SetPan( float32 Pan )				{ m_Pan = Pan; }
	void	SetPriority( int32 Priority )		{ m_Priority = Priority; }

	int32	GetFrequencyHz() const	{ return m_FrequencyHz; }
	float32	GetVolume() const		{ return m_Volume; }
	float32 GetPan() const			{ return m_Pan; }
	int32	GetPriority() const		{ return m_Priority; }

private:
	// members
	int32	m_FrequencyHz;
	float32 m_Volume;
	float32 m_Pan;
	int32	m_Priority;
};

}	// namespace Audio
}	// namespace WT

