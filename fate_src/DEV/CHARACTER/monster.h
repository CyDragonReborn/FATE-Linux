/*
	monster.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _MONSTER_H
#define _MONSTER_H

#include "d3dx8_shim.h"

#include "character.h"
class CLevel;
class CRefManager;
class CSettings;
class CCharacterDescription;
class CSpellTemplate;

// monsters extend the base character, and add their own
// AI routines
class CMonster:
		public CCharacter
{
public:

				CMonster( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						  CRefManager& pRefManager,		// pointer to the reference manager, for tracking submaterials
						  CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
						  CSpellTemplate& pSpellTemplate,							// spell template for casting
						  CParticleManager& pParticleManager,					// particle manager
						  CGlowManager& pGlowManager,					// glow manager
						  CPositionalAudioManager& pPositionalAudioManager,		// positional audio manager
						  CCharacterDescription& pDescription );				// character description

	void		Update( const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
						float32 TimeElapsed, 		// time elapsed, in seconds
						CLevel& pLevel );			// a scene to collide with

	void		UpdateAI( float32 TimeElapsed, 		// time elapsed, in seconds
						  CLevel& pLevel );			// a scene to interact with

	void		WanderAI( float32 TimeElapsed, 	// time elapsed, in seconds
						  CLevel& pLevel );		// a scene to collide with

	void		FleeAI( float32 TimeElapsed, 	// time elapsed, in seconds
						CLevel& pLevel );		// a scene to collide with

	void		GetRangeAI( float32 TimeElapsed, 	// time elapsed, in seconds
							CLevel& pLevel );		// a scene to collide with

	void		AttackAI( float32 TimeElapsed, 	// time elapsed, in seconds
						  CLevel& pLevel );		// a scene to collide with

	void		ApproachAI( float32 TimeElapsed, 	// time elapsed, in seconds
							CLevel& pLevel );		// a scene to collide with


	void		HuntAI( float32 TimeElapsed, 	// time elapsed, in seconds
						CLevel& pLevel );		// a scene to collide with

	void		UniqueIdleAI( float32 TimeElapsed, 	// time elapsed, in seconds
							  CLevel& pLevel );		// a scene to collide with

	void		OperateItemAI( float32 TimeElapsed, 		// time elapsed, in seconds
							   CLevel& pLevel );			// a scene to interact with

	void		RespondToAttack( CCharacter* pAttacker );	// attacking character

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

protected:

	std::string		m_CurrentUniqueIdle;

	void	SelectAttackSpell( void );

	void	SelectSummonSpell( void );

	void	SelectBanishSpell( void );

	void	SelectDefenseSpell( void );

	void	SelectHealSpell( void );

	void	SelectHealMinionSpell( void );
};

#endif