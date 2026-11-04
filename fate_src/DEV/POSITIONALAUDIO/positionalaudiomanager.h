/*
	positionalmanager.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	8/20/2004
	
*/

#ifndef _POSITIONALAUDIOMANAGER_H
#define _POSITIONALAUDIOMANAGER_H

#include "d3dx8_shim.h"

#include "fmod.h"

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"
#include "../MEDIA/refmanager.h"


class CSettings;
class CRefManager;


class CPositionalAudio
{
public:
			CPositionalAudio( CRefManager& pRefManager,		// the reference manager, for tracking media we create
							  FSOUND_SAMPLE* pSample,		// sound sample
							  const D3DXVECTOR3& Position,	// worldspace position of audio generator
							  uint32 BaseVolume,			// base volume (0-255)
							  float32 Radius ) :			// maximum radius at which sound can be heard
							  m_pSample( pSample ),
							  m_pRefManager( pRefManager ),
							  m_Position( Position ),
							  m_BaseVolume( BaseVolume ),
							  m_Radius( Radius ),
							  m_Channel( -1 )
			{
			}
			~CPositionalAudio( void )
			{
				if( m_Channel != -1 )
				{
					FSOUND_StopSound( m_Channel );
				}
				m_pRefManager.RemoveSample( m_pSample );
			}

	void		SetPosition( const D3DXVECTOR3& Position )	// position to move to 
	{
		m_Position = Position;
	}

	void		Update( const D3DXVECTOR3& POVPosition,		// worldspace location of the listener
						const D3DXMATRIX& POVOrientation,	// facing of the listener
						float32 TimeElapsed )				// time elapsed in seconds
	{
		float32 Distance = D3DXVec3Length( &( POVPosition - m_Position ) );
		// if the listener is in range,
		if( Distance <= m_Radius )
		{
			// calculate panning
			D3DXMATRIX Inverse;
			D3DXVECTOR3 LocalPosition( m_Position - POVPosition );
			D3DXMatrixInverse( &Inverse, NULL, &POVOrientation );
			D3DXVec3TransformNormal( &LocalPosition, &LocalPosition, &Inverse );

			float32 Pan = LocalPosition.x;
			Pan /= m_Radius;
			Pan *= 2.0f;
			if( Pan < -1 )
			{
				Pan = -1.0f;
			}
			if( Pan > 1 )
			{
				Pan = 1.0f;
			}
			Pan *= 128.0f;
			Pan += 128.0f;


			// play the sound if it isn't already started
			if( m_Channel == -1 )
			{
				m_Channel = FSOUND_PlaySound( FSOUND_FREE,
											  m_pSample );
			}
			// calculate current volume, and set it
			Distance /= m_Radius;
			Distance = 1.0f - Distance;
			Distance *= (float32)m_BaseVolume;
			if( Distance > 255 )
			{
				Distance = 255;
			}
			if( m_Channel != -1 )
			{
				FSOUND_SetVolume( m_Channel,
								( int16 )Distance );	
				FSOUND_SetPan( m_Channel,
							( int16 )Pan );
			}
		}
		else
		{
			if( m_Channel != -1 )
			{
				FSOUND_StopSound( m_Channel );
				m_Channel = -1;
			}
		}
	}

private:
	CRefManager&		m_pRefManager;

	FSOUND_SAMPLE*		m_pSample;

	float32				m_Radius;
	uint32				m_BaseVolume;

	int32				m_Channel;

	D3DXVECTOR3			m_Position;
};

// manager all the positional audio elements in a scene
class CPositionalAudioManager
{
public:


				CPositionalAudioManager( CRefManager& pRefManager,			// the reference manager, for tracking media we create
										 CSettings& pSettings );			// the settings manager, for keeping track of card/machine capabilities

				~CPositionalAudioManager( void );

	CPositionalAudio*	AddAudio( const D3DXVECTOR3& Position,	// worldspace position of audio generator
								  const std::string& SoundPath,	// path to sample to play
								  uint32 BaseVolume,			// base volume (0-255)
								  float32 Radius );				// maximum radius at which sound can be heard

	void		Update( const D3DXVECTOR3& POVPosition,		// worldspace location of the listener
						const D3DXMATRIX& POVOrientation,	// facing of the listener
						float32 TimeElapsed );				// time elapsed in seconds

	void		RemoveAudio( CPositionalAudio*	pPositionalAudio );	// audio item to remove

	int32		PlaySample( FSOUND_SAMPLE* pSample,			// sample to play
							const D3DXVECTOR3& Position,	// worldspace position of audio generator
							uint32 BaseVolume,				// base volume (0-255)
							float32 Radius );				// maximum radius at which sound can be heard

	void		Clear( void );

private:

	CLList<CPositionalAudio>*		m_pAudioList;	// list of pointers to all active positional audio items

	CSettings&			m_pSettings;
	CRefManager&		m_pRefManager;

	D3DXVECTOR3			m_POVPosition;
	D3DXMATRIX			m_POVOrientation;

};

#endif