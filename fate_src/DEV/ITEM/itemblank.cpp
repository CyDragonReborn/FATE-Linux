/*
	itemblank.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004


*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/refmanager.h"

#include "../GAMEOBJECTS/soundbank.h"

#include "../LEVEL/level.h"

#include "item.h"
#include "itemblank.h"


CItemBlank::CItemBlank( LPDIRECT3DDEVICE8 pD3DDevice,					// direct3d device
				  CRefManager& pRefManager,							// pointer to the reference manager, for tracking submaterials
				  CSettings& pSettings,								// pointer to the settings manager, for keeping track of card/machine capabilities
				  CParticleManager& pParticleManager,				// particle manager
				  CGlowManager& pGlowManager,						// glow manager
				  CPositionalAudioManager& pPositionalAudioManager, // positional audio manager
				  const std::string& Name,							// name of blank item
				  const D3DXVECTOR3& Position,						// worldspace position
				  const D3DXVECTOR3& MinBounds,						// min bounds of click area
				  const D3DXVECTOR3& MaxBounds,						// max bounds of click area
				  EItemType Type ) :								// type of blank item
				  CItem( pD3DDevice,
						 pRefManager,
						 pSettings,
						 pParticleManager,
						 pGlowManager,
						 pPositionalAudioManager )

{
	m_Operateable = kTrue;

	m_Identified = kTrue;

	m_Type = Type;
	m_Category = KCategoryBlank;

	m_Name = Name;
	m_Collideable = kFalse;
	m_Shadow = kFalse;
	m_Takeable = kFalse;
	m_FliesWhenDropped = kFalse;

	m_Position = Position;

	m_pCullingBounds = new CCullingBounds( MinBounds,
										   MaxBounds );

	D3DXMatrixIdentity( &m_OrientationMatrix );

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );


	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );


} // CItemBlank::CItemBlank()

