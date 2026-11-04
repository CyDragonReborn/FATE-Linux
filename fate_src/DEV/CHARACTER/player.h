/*
	player.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _PLAYER_H
#define _PLAYER_H

#include "character.h"
class CLevel;
class CRefManager;
class CSettings;
class CCharacterDescription;
class CSpellTemplate;

// player extend the base character, and add their own
// loading/saving routines
class CPlayer:
		public CCharacter
{
public:

				CPlayer( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						  CRefManager& pRefManager,		// pointer to the reference manager, for tracking submaterials
						  CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
						  CSpellTemplate& pSpellTemplate,							// spell template for casting
						  CParticleManager& pParticleManager,					// particle manager
						  CGlowManager& pGlowManager,						// particle manager
						  CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
						  EGender Gender,									// gender of player
						  int32 HeadIndex,									// head index
						  int32 HairIndex );									// hair index


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void		SetAIState( EAIState State );	// ai state to set

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	int32		BaseMaxHP( void );
protected:
	void		CalculateMaxHP( void );

private:

	void		SetBaseDescription( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	// base HP modifier
	int32		m_BaseHP;

};

#endif