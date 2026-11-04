/*
	soundbank.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _SOUNDBANK_H
#define _SOUNDBANK_H

#include "fmod.h"

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/utilities.h"
#include "../POSITIONALAUDIO/positionalaudiomanager.h"
#include "../MEDIA/refmanager.h"



/// maintains a grouping of sounds, and allows random play of sounds in each bank -
/// useful for maintaining a variety of sounds for a single action

class CSoundBank
{
public:

	CSoundBank( CRefManager& pRefManager,	// pointer to the reference manager
				CSettings& pSettings,		// pointer to the settings manager, for keeping track of card/machine capabilities
				CPositionalAudioManager& pPositionalAudioManager ) :	// positional audio manager
																	m_pRefManager( pRefManager ),
																	m_pSettings( pSettings ),
																	m_pPositionalAudioManager( pPositionalAudioManager )
	{
	}
			
	~CSoundBank( void )
	{
		Clear();
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////
	uint32	Banks( void )			{	return m_pSoundBank.size();			};

	uint32	Samples( uint32 Bank )	{	return m_pSoundBank[Bank].size();	};

	FSOUND_SAMPLE*	Sample( uint32 Bank, uint32 Sample )	{	return m_pSoundBank[Bank][Sample];	};
	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void	Clear( void )
	{
		for( uint32 i = 0; i < m_pSoundBank.size(); i++ )
		{
			for( uint32 j = 0; j < m_pSoundBank[i].size(); j++ )
			{
				m_pRefManager.RemoveSample( m_pSoundBank[i][j] );
			}
		}
		m_pSoundBank.clear();
	}

	void		AddSample( int32 Bank,						// soundbank to add to
						   const std::string& SamplePath,	// path to sample
						   bool Looping = kFalse )			// looping?
	{
		if( Bank < 0 )
		{
			return;
		}

		FSOUND_SAMPLE* pSample = m_pRefManager.AddSample( SamplePath, Looping );
		if( pSample != NULL )
		{
			if( Banks() <= (uint32)Bank )
			{
				m_pSoundBank.resize( Bank + 1 );
			}
			m_pSoundBank[Bank].push_back( pSample );
		}
	}

	int32		PlaySample( int32 Bank,						// sound bank
							const D3DXVECTOR3& Position,	// worldspace position of audio generator
							uint32 BaseVolume,				// base volume (0-255)
							float32 Radius )				// maximum radius at which sound can be heard
	{
		if( (int32)Banks() <= Bank ||
			Bank < 0 )
		{
			return -1;
		}
		if( Samples( Bank ) == 0 )
		{
			return -1;
		}
		int32 SampleIndex = RandomIntegerBetween( 0, Samples( Bank ) - 1 );
		return m_pPositionalAudioManager.PlaySample( Sample( Bank, SampleIndex ),
													 Position,
													 BaseVolume,
													 Radius );
	}


private:

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;
	CPositionalAudioManager&	m_pPositionalAudioManager;

	std::vector< std::vector< FSOUND_SAMPLE* > >	m_pSoundBank;

};
#endif
