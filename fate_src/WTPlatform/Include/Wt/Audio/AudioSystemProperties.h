/***************************************************************************************
 *
 * AudioSystemProperties.h
 *
 * Copyright (C) 2004 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/26/2004: Jason Glover:	Creation
 *
 ***************************************************************************************/
#pragma once

#include "WT/System/Object.h"
#include "windows.h"

namespace WT
{
namespace Audio
{

// Note: this class is an Audio system property envelope.
// Member values are not range validated, this occurs during AudioSystem object construction
class CAudioSystemProperties
{
public:
	CAudioSystemProperties( HWND	hWnd,
							int32	MixRate = 44100,
							int32	MaxSoftwareChannels = 32,
							int32	MixBufferMs = 100,
							uint32	Flags = 0) :
			m_MixRate( MixRate ),
			m_MaxSoftwareChannels( MaxSoftwareChannels ),
			m_MixBufferMs( MixBufferMs ),
			m_hWnd( hWnd ),
			m_Flags( Flags ) {}
	CAudioSystemProperties(	int32	MixRate,
								int32	MaxSoftwareChannels,
								int32	MixBufferMs,
								HWND	hWnd,
								uint32	Flags ) :
			m_MixRate( MixRate ),
			m_MaxSoftwareChannels( MaxSoftwareChannels ),
			m_MixBufferMs( MixBufferMs ),
			m_hWnd( hWnd ),
			m_Flags( Flags ) {}

	void	SetMixRate( int32 MixRate )							{ m_MixRate = MixRate; }
	void	SetMaxSoftwareChannels( int32 MaxSoftwareChannels ) { m_MaxSoftwareChannels = MaxSoftwareChannels; }
	void	SetMixBufferMs( int32 MixBufferMs )					{ m_MixBufferMs = MixBufferMs; }
	void	SetWindowHandle( HWND hWnd )						{ m_hWnd = hWnd; }
	void	SetFlags( uint32 Flags )							{ m_Flags = Flags; }

	int32	GetMixRate() const				{ return m_MixRate; }
	int32	GetMaxSoftwareChannels() const	{ return m_MaxSoftwareChannels; }
	int32	GetMixBufferMs() const			{ return m_MixBufferMs; }
	HWND	GetWindowHandle() const			{ return m_hWnd; }
	uint32	GetFlags() const				{ return m_Flags; }

private:
	// methods
	CAudioSystemProperties() {};

private:
	// members
	int32	m_MixRate;
	int32	m_MaxSoftwareChannels;
	int32	m_MixBufferMs;
	HWND	m_hWnd;
	uint32	m_Flags;
};

}	// namespace Audio
}	// namespace WT

