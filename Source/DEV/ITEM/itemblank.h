/*
	itemblank.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _ITEMBLANK_H
#define _ITEMBLANK_H

#include "item.h"
class CLevel;
class CRefManager;
class CSettings;

// an invisible, but interactable item - (i.e. an invisible box around an exit
class CItemBlank : public CItem
{
public:

				CItemBlank( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   CRefManager& pRefManager,				// pointer to the reference manager, for tracking submaterials
						   CSettings& pSettings,					// pointer to the settings manager, for keeping track of card/machine capabilities
						   CParticleManager& pParticleManager,		// particle manager
						   CGlowManager& pGlowManager,				// glow manager
						   CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
						   const std::string& Name,					// name of blank item
						   const D3DXVECTOR3& Position,				// worldspace position
						   const D3DXVECTOR3& MinBounds,			// min bounds of click area
						   const D3DXVECTOR3& MaxBounds,			// max bounds of click area
						   EItemType Type );						// type of blank item


////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
		
	
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////


protected:

};

#endif