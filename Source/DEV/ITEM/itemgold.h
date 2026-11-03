/*
	itemgold.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _ITEMGOLD_H
#define _ITEMGOLD_H

#include "item.h"
class CLevel;
class CRefManager;
class CSettings;

// gold is a special sort of item that only exists in the level, and
// is goes into the player's gold pool when taken
class CItemGold:
		public CItem
{
public:

				CItemGold( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   CRefManager& pRefManager,				// pointer to the reference manager, for tracking submaterials
						   CSettings& pSettings,					// pointer to the settings manager, for keeping track of card/machine capabilities
						   CParticleManager& pParticleManager,					// particle manager
						   CGlowManager& pGlowManager,					// glow manager
						   CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
						   uint32 Value );										// gold value

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

protected:

};

#endif