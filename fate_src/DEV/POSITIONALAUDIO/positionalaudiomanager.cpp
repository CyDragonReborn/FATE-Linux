/*
	positionalaudiomanager.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	8/20/2004
	
*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"

#include "../UTILITIES/llist.h"

#include "../MEDIA/refmanager.h"


#include "positionalaudiomanager.h"

CPositionalAudioManager::CPositionalAudioManager( CRefManager& pRefManager,			// the reference manager, for tracking media we create
												  CSettings& pSettings ) :			// the settings manager, for keeping track of card/machine capabilities
																			m_pRefManager( pRefManager ),
																			m_pSettings( pSettings )
{
	ZeroMemory ( &m_POVPosition, sizeof ( m_POVPosition ));
	ZeroMemory ( &m_POVOrientation, sizeof ( m_POVOrientation ));

	// create a templatized linked list for all active samples
	m_pAudioList = new CLList< CPositionalAudio >();

} // CPositionalAudioManager::CPositionalAudioManager()

CPositionalAudioManager::~CPositionalAudioManager( void )
{
	DELETE_SAFELY( m_pAudioList );
} // CPositionalAudioManager::~CPositionalAudioManager()

void CPositionalAudioManager::Clear( void )
{
	m_pAudioList->Destroy();
} // CPositionalAudioManager::Clear()

CPositionalAudio* CPositionalAudioManager::AddAudio( const D3DXVECTOR3& Position,	// ground contact position of wheel
													 const std::string& SoundPath,	// path to sample to play
													 uint32 BaseVolume,			// base volume (0-255)
													 float32 Radius )				// maximum radius at which sound can be heard
{

	FSOUND_SAMPLE* pSample = m_pRefManager.AddSample( SoundPath, kTrue );

	CPositionalAudio* pAudio = m_pAudioList->Queue( new CPositionalAudio( m_pRefManager,
																		  pSample,
																		  Position,
																		  BaseVolume,
																		  Radius ) );
	return pAudio;
} // CPositionalAudioManager::AddAudio()

void CPositionalAudioManager::RemoveAudio( CPositionalAudio* pRemoveAudio )	// audio item to remove
{
	CLLNode<CPositionalAudio>*	pAudio	= m_pAudioList->GetFirst();
	while ( pAudio != NULL )
	{
		if( pAudio->Data() == pRemoveAudio )
		{
			m_pAudioList->Remove( pAudio );
			return;
		}
		pAudio = pAudio->Next();
	}
} // CPositionalAudioManager::AddAudio()

void CPositionalAudioManager::Update( const D3DXVECTOR3& POVPosition,	// worldspace position of listener
									  const D3DXMATRIX& POVOrientation,	// facing of the listener
									  float32 TimeElapsed )				// time elapsed in seconds
{
	m_POVPosition = POVPosition;
	m_POVOrientation = POVOrientation;
	CLLNode<CPositionalAudio>*	pAudio	= m_pAudioList->GetFirst();
	while ( pAudio != NULL )
	{
		pAudio->Data()->Update( POVPosition, POVOrientation, TimeElapsed );
		pAudio = pAudio->Next();
	}
} // CPositionalAudioManager::Update()

int32 CPositionalAudioManager::PlaySample( FSOUND_SAMPLE* pSample,		// sample to play
										  const D3DXVECTOR3& Position,	// worldspace position of audio generator
										  uint32 BaseVolume,			// base volume (0-255)
										  float32 Radius )				// maximum radius at which sound can be heard
{
	float32 Distance = D3DXVec3Length( &( m_POVPosition - Position ) );
	
	// if the listener is in range,
	if( Distance <= Radius )
	{
		// calculate panning
		D3DXMATRIX Inverse;
		D3DXVECTOR3 LocalPosition( Position - m_POVPosition );
		D3DXMatrixInverse( &Inverse, NULL, &m_POVOrientation );
		D3DXVec3TransformNormal( &LocalPosition, &LocalPosition, &Inverse );

		float32 Pan = LocalPosition.x;
		Pan /= Radius;
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

		int32 Channel = FSOUND_PlaySound( FSOUND_FREE,
										   pSample );

		// calculate current volume, and set it
		Distance /= Radius;
		Distance = 1.0f - Distance;
		Distance *= (float32)BaseVolume;
		if( Distance > 255 )
		{
			Distance = 255;
		}
			
		if( Channel != -1 )
		{
			FSOUND_SetVolume( Channel,
							( int16 )Distance );	
			FSOUND_SetPan( Channel,
						( int16 )Pan );
		}
		return Channel;

	}
	return -1;

} // CPositionalAudioManager::AddAudio()
