/*
	itemgold.cpp

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
#include "../UTILITIES/loadlocalizedstring.h"

#include "../MEDIA/refmanager.h"

#include "../GAMEOBJECTS/soundbank.h"

#include "../LEVEL/level.h"

#include "item.h"
#include "itemgold.h"


CItemGold::CItemGold( LPDIRECT3DDEVICE8 pD3DDevice,						// direct3d device
				  CRefManager& pRefManager,							// pointer to the reference manager, for tracking submaterials
				  CSettings& pSettings,								// pointer to the settings manager, for keeping track of card/machine capabilities
				  CParticleManager& pParticleManager,				// particle manager
				  CGlowManager& pGlowManager,						// glow manager
				  CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
				  uint32 Value ) :									// gold value
				  CItem( pD3DDevice,
						 pRefManager,
						 pSettings,
						 pParticleManager,
						 pGlowManager,
						 pPositionalAudioManager )

{
	m_Type = KItemGold;
	m_Category = KCategoryGold;

	m_Value = Value;

	m_Identified = kTrue;

	m_BaseName = "Gold";
	m_Name = LoadLocalizedString1Variable(994, IntegerToString( m_Value ) );
	m_Collideable = kFalse;
	m_Shadow = kFalse;
	m_Takeable = kTrue;
	m_FliesWhenDropped = kFalse;

	m_pModel = new CGenericModel( pD3DDevice, 
								  m_pRefManager, 
								  m_pSettings,
								  "ITEMS/gold.mdl",
								  "" );

	if( m_pModel->AnimationExists( "IDLE" ) )
	{
		m_pModel->PlayAnimation( "IDLE", kTrue );
	}
	else if( m_pModel->GetAnimationCount() == 0 )
	{
		m_pModel->PlayAnimation( 0 , kFalse );
	}

	for( uint32 i = 0; i < m_pModel->GetAnimationCount(); i++ )
	{
		std::string Name = StringUpper( m_pModel->GetAnimationName( i ) );
		if( Name.length() >= 4 && Name.substr( 0, 4 ) == "IDLE" )
		{
			m_IdleAnimations.push_back( i );
		}
	}

	m_pCullingBounds = new CCullingBounds( m_pModel->GetMinBounds(),
										   m_pModel->GetMaxBounds() );

	D3DXMatrixIdentity( &m_OrientationMatrix );

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	D3DXMATRIX RotationMatrix;

	float32 Angle( RandomBetween( 0, 360 ) );

	float32 Scale( (float32)Value * .004f );
	Scale += .5f;
	if( Scale > 1.5f )
	{
		Scale = 1.5f;
	}

	D3DXMatrixRotationY( &RotationMatrix, (float32)KDeg2Rad * Angle );


	D3DXMATRIX ScaleMatrix;
	D3DXMatrixScaling( &ScaleMatrix, 
					   Scale,
					   Scale,
					   Scale );

	m_OrientationMatrix = ScaleMatrix * RotationMatrix * m_OrientationMatrix;

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );

	m_pTakeSound = m_pRefManager.AddSample( "SOUNDS/ITEMS/gold.wav", kFalse );

	m_pDropSound = m_pRefManager.AddSample( "SOUNDS/ITEMS/gold.wav", kFalse );

	FindParticles();

} // CItemGold::CItemGold()

