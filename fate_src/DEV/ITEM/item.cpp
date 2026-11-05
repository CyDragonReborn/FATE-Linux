/*
	item.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/17/2004

*/
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/cullingbounds.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../LEVEL/level.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../MODEL/model.h"

#include "../PARTICLES/particlemanager.h"
#include "../GLOWS/glowmanager.h"
#include "../GLOWS/glowsystem.h"

#include "../POSITIONALAUDIO/positionalaudiomanager.h"

#include "../GAMEOBJECTS/genericmodel.h"

#include "../GAMEOBJECTS/soundbank.h"

#include "../GAMEOBJECTS/lightmanager.h"

#include "../OBJECTS/path.h"

#include "../UI/sprite.h"
#include "../UI/fontmetric.h"
#include "../UI/text.h"

#include "../CHARACTER/charactertemplate.h"
#include "../CHARACTER/character.h"
#include "../CHARACTER/inventory.h"

#include "../NAMES/namegenerator.h"

#include "itemtemplate.h"
#include "item.h"
#include "itemgold.h"
#include "spelltemplate.h"

CItem::CItem( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
			  CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
			  CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
			  CParticleManager& pParticleManager,	// particle manager
			  CGlowManager& pGlowManager,			// glow manager
			  CPositionalAudioManager& pPositionalAudioManager ) :	// positional audio manager
																	m_pRefManager( pRefManager ),
																	m_pSettings( pSettings ),
																	m_pGlowManager( pGlowManager ),
																	m_pParticleManager( pParticleManager ),
																	m_pPositionalAudioManager( pPositionalAudioManager ),
																	m_AnimatedModelPath( "" ),
																	m_UseDescription( "" ),
																	m_AmbientSoundPath( "" ),
																	m_pModel( NULL ),
																	m_pOpenModel( NULL ),
																	m_pCloseModel( NULL ),
																	m_Onscreen( kFalse ),
																	m_pCullingBounds( NULL ),
																	m_pLightPoolMaterial( NULL ),
																	m_pCollisionModel( NULL ),
																	m_pAmbientPositional( NULL ),
																	m_pTakeSound( NULL ),
																	m_pDropSound( NULL ),
																	m_pUseSound( NULL ),
																	m_pLandSound( NULL ),
																	m_pDropPath( NULL ),
																	m_pRightAttack( NULL ),
																	m_pLeftAttack( NULL ),
																	m_pItemSounds( NULL ),
																	m_pAttackSounds( NULL ),
																	m_pTrapSounds( NULL ),
																	m_pStrikeSounds( NULL ),
																	m_pMissSounds( NULL ),
																	m_pIconMaterial( NULL ),
																	m_pIconSprite( NULL ),
																	m_pItemName( NULL ),
																	m_pItemBackdrop( NULL ),
																	m_Operateable( kFalse ),
																	m_Collideable( kFalse ),
																	m_Shadow( kTrue ),
																	m_HasLightPool( kFalse ),
																	m_CanOpen( kFalse ),
																	m_CanClose( kFalse ),
																	m_ObjectNearby( kFalse ),
																	m_Open( kFalse ),
																	m_OpenComplete( kFalse ),
																	m_Takeable( kTrue ),
																	m_Destructible( kFalse ),
																	m_OnGround( kFalse ),
																	m_OnAvoidanceMap( kFalse ),
																	m_Dropping( kFalse ),
																	m_FliesWhenDropped( kTrue ),
																	m_CanInteract( kFalse ),
																	m_Useable( kFalse ),
																	m_InWorld( kFalse ),
																	m_Grade( KGradeNormal ),
																	m_Rank( KRankNormal ),
																	m_CollisionMaskRadius( 1 ),
																	m_ToHitBonus( 0 ),
																	m_AttackRadius( 6 ),
																	m_StrikeRadius( 8 ),
																	m_DropPosition( 0 ),
																	m_TorchLightFlicker( 0 ),
																	m_LightPoolAngle( 0 ),
																	m_ActivationRadius( 16 ),
																	m_DeactivationRadius( 16 ),
																	m_BaseSoundVolume( 255 ),
																	m_BaseSoundRadius( 32 ),
																	m_ArmorBonus( 0 ),
																	m_MinimumDamage( 0 ),
																	m_MaximumDamage( 0 ),
																	m_SlotsWide( 1 ),
																	m_SlotsTall( 1 ),
																	m_BaseLevel( 1 ),
																	m_Sockets( 0 ),
																	m_GambleMultiplier( RandomBetween( .05f, .9f ) ),
																	m_Speed( KAttackNormal ),
																	m_Uses( KUnlimitedUses ),
																	m_OriginalUses( KUnlimitedUses ),
																	m_DefaultMagical( kFalse ),
																	m_Identified( kFalse ),
																	m_NameIdentified( kFalse ),
																	m_TrapType( KTrapNone ),
																	m_Target( KTargetUser ),
																	m_LightPoolType ( KLightFlicker ),
																	m_Type ( KItemNone ),
																	m_Category ( KCategoryBlank ),
																	m_IsUnique( kFalse ),
																	m_IsArtifact( kFalse ),
																	m_LightPoolRadius( 0.0f ),
																	m_LightPoolScale( 0.0f ),
																	m_Name( "" ),
																	m_BaseName( "" ),
																	m_BaseModifiedName( "" ),
																	m_Value( 0 ),
																	m_MaximumArmorBonus( 0 )
{
	ZeroMemory ( &m_Position, sizeof ( m_Position ));
	ZeroMemory ( &m_LocalLight, sizeof ( m_LocalLight ));
	ZeroMemory ( &m_OrientationMatrix, sizeof ( m_OrientationMatrix ));
	ZeroMemory ( &m_TransformationMatrix, sizeof ( m_TransformationMatrix ));
}




CItem::CItem( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
			  CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
			  CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
			  CParticleManager& pParticleManager,	// particle manager
			  CGlowManager& pGlowManager,			// glow manager
			  CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
			  CItemDescription& pItem ) :		// item description
																	m_pRefManager( pRefManager ),
																	m_pSettings( pSettings ),
																	m_pParticleManager( pParticleManager ),
																	m_pGlowManager( pGlowManager ),
																	m_pPositionalAudioManager( pPositionalAudioManager ),
																	m_AnimatedModelPath( "" ),
																	m_UseDescription( "" ),
																	m_AmbientSoundPath( "" ),
																	m_pModel( NULL ),
																	m_pOpenModel( NULL ),
																	m_pCloseModel( NULL ),
																	m_Onscreen( kFalse ),
																	m_pCullingBounds( NULL ),
																	m_pLightPoolMaterial( NULL ),
																	m_pCollisionModel( NULL ),
																	m_pAmbientPositional( NULL ),
																	m_pTakeSound( NULL ),
																	m_pDropSound( NULL ),
																	m_pLandSound( NULL ),
																	m_pUseSound( NULL ),
																	m_pDropPath( NULL ),
																	m_pLeftAttack( NULL ),
																	m_pRightAttack( NULL ),
																	m_Collideable( kFalse ),
																	m_pAttackSounds( NULL ),
																	m_pTrapSounds( NULL ),
																	m_pStrikeSounds( NULL ),
																	m_pItemSounds( NULL ),
																	m_pIconSprite( NULL ),
																	m_pMissSounds( NULL ),
																	m_pIconMaterial( NULL ),
																	m_pItemName( NULL ),
																	m_pItemBackdrop( NULL ),
																	m_Operateable( kFalse ),
																	m_Shadow( kTrue ),
																	m_HasLightPool( kFalse ),
																	m_CanOpen( kFalse ),
																	m_CanClose( kFalse ),
																	m_ObjectNearby( kFalse ),
																	m_Open( kFalse ),
																	m_OpenComplete( kFalse ),
																	m_Takeable( kTrue ),
																	m_Destructible( kFalse ),
																	m_OnGround( kFalse ),
																	m_OnAvoidanceMap( kFalse ),
																	m_Dropping( kFalse ),
																	m_FliesWhenDropped( kTrue ),
																	m_CanInteract( kFalse ),
																	m_Useable( kFalse ),
																	m_InWorld( kFalse ),
																	m_IsUnique( kFalse ),
																	m_IsArtifact( kFalse ),
																	m_Grade( KGradeNormal ),
																	m_Rank( KRankNormal ),
																	m_CollisionMaskRadius( 1 ),
																	m_ToHitBonus( 0 ),
																	m_AttackRadius( 6 ),
																	m_StrikeRadius( 8 ),
																	m_DropPosition( 0 ),
																	m_TorchLightFlicker( 0 ),
																	m_LightPoolAngle( 0 ),
																	m_ActivationRadius( 16 ),
																	m_DeactivationRadius( 16 ),
																	m_BaseSoundVolume( 255 ),
																	m_BaseSoundRadius( 32 ),
																	m_ArmorBonus( 0 ),
																	m_MinimumDamage( 0 ),
																	m_MaximumDamage( 0 ),
																	m_SlotsWide( 1 ),
																	m_SlotsTall( 1 ),
																	m_BaseLevel( 1 ),
																	m_Sockets( 0 ),
																	m_GambleMultiplier( RandomBetween( .1f, .9f ) ),
																	m_Speed( KAttackNormal ),
																	m_Uses( KUnlimitedUses ),
																	m_OriginalUses( KUnlimitedUses ),
																	m_DefaultMagical( kFalse ),
																	m_Identified( kFalse ),
																	m_NameIdentified( kFalse ),
																	m_TrapType( KTrapNone ),
																	m_Target( KTargetUser )

{
	m_pAttackSounds = new CSoundBank( m_pRefManager,
									  m_pSettings,
									  m_pPositionalAudioManager );

	m_pTrapSounds = new CSoundBank( m_pRefManager,
									m_pSettings,
									m_pPositionalAudioManager );

	m_pStrikeSounds = new CSoundBank( m_pRefManager,
									  m_pSettings,
									  m_pPositionalAudioManager );

	m_pMissSounds = new CSoundBank( m_pRefManager,
									m_pSettings,
									m_pPositionalAudioManager );

	m_pItemSounds = new CSoundBank( m_pRefManager,
									m_pSettings,
									m_pPositionalAudioManager );

	// add all sounds for weapons, if required
	for( uint32 i = 0; i < pItem.AttackSounds(); i++ )
	{
		m_pAttackSounds->AddSample( 0,
									pItem.AttackSound( i ) );
	}

	for( int32 i = 0; i < pItem.StrikeSounds(); i++ )
	{
		m_pStrikeSounds->AddSample( 0,
									pItem.StrikeSound( i ) );
	}

	for( int32 i = 0; i < pItem.MissSounds(); i++ )
	{
		m_pMissSounds->AddSample( 0,
								  pItem.MissSound( i ) );
	}

	for( int32 i = 0; i < pItem.ActivationSounds(); i++ )
	{
		m_pItemSounds->AddSample( KSoundActivate,
								  pItem.ActivationSound( i ) );
	}

	for( int32 i = 0; i < pItem.DeactivationSounds(); i++ )
	{
		m_pItemSounds->AddSample( KSoundDeactivate,
								  pItem.DeactivationSound( i ) );
	}

	// copy all effects from the item template
	for( int32 i = 0; i < pItem.Effects(); i++ )
	{
		AddEffect( pItem.Effect( i ) );

		m_DefaultMagical = kTrue;	// has magic effects inherent
	}

	CalculateIdentifyLevel();

	m_CollisionMaskRadius = pItem.CollisionMaskRadius();

	// copy requirements from the template
	m_RequirementStatistic = pItem.RequirementStats();
	m_RequirementValue = pItem.RequirementValues();

	m_ToHitBonus = pItem.ToHitBonus();

	m_Type = pItem.Type();
	m_Category = pItem.Category();

	m_Value = pItem.Value();

	m_Uses = pItem.Uses();
	m_OriginalUses = pItem.Uses();

	m_Useable = pItem.Useable();

	if( m_Category == KCategoryEvent )
	{
		m_Operateable = kTrue;
	}

	m_AnimatedModelPath = pItem.AnimatedModelPath();

	// containers are interactable until bashed or opened
	if( IsContainer() )
	{
		m_CanInteract = kTrue;
	}

	for( int32 i = 0; i < pItem.DamageBonuses(); i++ )
	{
		AddDamageBonus( pItem.DamageBonusType( i ), pItem.DamageBonusValue( i ) );
	}

	m_IsUnique = pItem.IsUnique();

	m_IsArtifact = pItem.IsArtifact();

	if( m_IsArtifact )
	{
		m_IsUnique = kTrue;
	}

	m_Identified = pItem.Identified();

	m_Target = pItem.Target();

	m_Speed = pItem.Speed();

	m_AttackRadius = pItem.AttackRadius();

	m_StrikeRadius = pItem.StrikeRadius();

	m_ArmorBonus = RandomIntegerBetween( pItem.MinimumArmorBonus(),
										 pItem.MaximumArmorBonus() );

	m_MaximumArmorBonus = pItem.MaximumArmorBonus();

	m_MinimumDamage = pItem.MinimumDamage();

	m_MaximumDamage = pItem.MaximumDamage();

	for( int32 i = 0; i < pItem.SFXItems(); i++ )
	{
		m_pSFX.push_back( new CSFXDescription( pItem.SFX( i ) ) );
	}

	// for weapons, we need to create a description of possible attacks
	m_pLeftAttack = new CAttackDescription( KWeaponAnimationLeft[m_Type],
											kTrue,
											m_AttackRadius,
											m_StrikeRadius,
											MinimumDamage(),
											MaximumDamage(),
											m_ToHitBonus,
											KAttackSpeedValue[m_Speed] );

	m_pRightAttack = new CAttackDescription( KWeaponAnimationRight[m_Type],
											 kTrue,
											 m_AttackRadius,
											 m_StrikeRadius,
											 MinimumDamage(),
											 MaximumDamage(),
											 m_ToHitBonus,
											 KAttackSpeedValue[m_Speed] );

	m_BaseName = pItem.Name();

	
	for( int32 i = 0; i < KRanks; i++ ) // modified by TDC 2008-04-22 Craig Miles
	{
		if(!CGame::GetLocaleString().compare("es")||!CGame::GetLocaleString().compare("it"))  
		{				
			if( m_BaseName.length() > KRankName[i].length() &&
				m_BaseName.substr( m_BaseName.length() - KRankName[i].length(), KRankName[i].length() ) == KRankName[i] )
			{
				m_Rank = (ERank)( KRankNormal + i );
			}
		}
		else
		{
			if( m_BaseName.length() > KRankName[i].length() &&
				m_BaseName.substr( 0, KRankName[i].length() ) == KRankName[i] )
			{
				m_Rank = (ERank)( KRankNormal + i );
			}
		}
	}
	m_Grade = pItem.Grade();
	if( m_Grade != KGradeNormal )
	{
		m_BaseModifiedName = "\b" + KGradeName[m_Grade] + "\b" + m_BaseName;
	}
	else
	{
		m_BaseModifiedName = m_BaseName;
	}
	m_Name = pItem.Name();

	m_UseDescription = pItem.UseDescription();

	m_Takeable = pItem.Takeable();

	m_Destructible = pItem.Destructible();

	// whether it flips in the air when dropped
	m_FliesWhenDropped = pItem.FliesWhenDropped();

	if( pItem.HasLightPool() )
	{
		m_HasLightPool = kTrue;
		m_pLightPoolMaterial = m_pRefManager.AddMaterial( pD3DDevice, pItem.LightPoolPath() );
		m_LightPoolRadius = pItem.LightPoolRadius();
		m_LightPoolScale = m_LightPoolRadius;
		m_LightPoolType = pItem.LightPoolType();
	}

	m_Collideable = pItem.Collideable();

	m_Shadow = pItem.Shadow();

	m_pModel = new CGenericModel( pD3DDevice, 
								  m_pRefManager, 
								  m_pSettings,
								  pItem.ModelPath(),
								  "" );

	if( pItem.OpenModelPath().length() > 0 )
	{
		m_pOpenModel = new CGenericModel( pD3DDevice, 
										  m_pRefManager, 
										  m_pSettings,
										  pItem.OpenModelPath(),
										  "" );
	}

	if( pItem.CloseModelPath().length() > 0 )
	{
		m_pCloseModel = new CGenericModel( pD3DDevice, 
										   m_pRefManager, 
										   m_pSettings,
										   pItem.CloseModelPath(),
										   "" );
	}

	if( m_pModel->AnimationExists( "IDLE" ) )
	{
		m_pModel->PlayAnimation( "IDLE", kTrue );
	}
	else if( m_pModel->GetAnimationCount() == 0 )
	{
		m_pModel->PlayAnimation( 0 , kFalse );
	}

	for( int32 i = 0; i < m_pModel->GetAnimationCount(); i++ )
	{
		std::string Name = StringUpper( m_pModel->GetAnimationName( i ) );
		if( Name.length() >= 4 && Name.substr( 0, 4 ) == "IDLE" )
		{
			m_IdleAnimations.push_back( i );
		}
	}

	// door-like motions 
	if( m_pModel->AnimationExists( "OPEN" ) )
	{
		m_CanOpen = kTrue;
	}
	if( m_pModel->AnimationExists( "CLOSE" ) )
	{
		m_CanClose = kTrue;
	}

	D3DXVECTOR3 MinBounds = m_pModel->GetMinBounds();
	D3DXVECTOR3 MaxBounds = m_pModel->GetMaxBounds();
	if( pItem.CollisionPath().length() > 0 )
	{
		m_pCollisionModel = m_pRefManager.AddModel( pD3DDevice,
													pItem.CollisionPath(),
													"",
													kTrue,
													kFalse );
		MinBounds = m_pCollisionModel->GetMinBounds();
		MaxBounds = m_pCollisionModel->GetMaxBounds();
	}
	MinBounds.x *= 2;
	MinBounds.z *= 2;
	MaxBounds.x *= 2;
	MaxBounds.z *= 2;
	ExpandBounds( MinBounds, MaxBounds, D3DXVECTOR3( -2, 0, -2 ) );
	ExpandBounds( MinBounds, MaxBounds, D3DXVECTOR3( 2, 2, 2 ) );
	m_pCullingBounds = new CCullingBounds( MinBounds,
										   MaxBounds );

	D3DXMatrixIdentity( &m_OrientationMatrix );

	// weapons need to be laid on their sides
	if( ( Category() == KCategoryWeapon ||
		  Type() == KItemShield ) &&
		Type() != KItemCrossbow )
	{
		D3DXMatrixRotationZ( &m_OrientationMatrix, (float32)KDeg2Rad * 90.0f );
	}


	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	D3DXMATRIX RotationMatrix;

	float32 Angle( RandomBetween( 0, 360 ) );

	if( m_Category == KCategoryEvent )
	{
		Angle = 0;
	}

	float32 Scale( RandomBetween( 0, pItem.ScaleVariation() ) );
	Scale += 1.0f;

	D3DXMatrixRotationY( &RotationMatrix, (float32)KDeg2Rad * Angle );


	D3DXMATRIX ScaleMatrix;
	D3DXMatrixScaling( &ScaleMatrix, 
					   Scale,
					   Scale,
					   Scale );

	m_OrientationMatrix = ScaleMatrix * m_OrientationMatrix * RotationMatrix;

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );

	if( m_pCollisionModel == NULL &&
		!m_pModel->HasCollisionData() )
	{
		m_Collideable = kFalse;
	}

	m_ActivationRadius = pItem.ActivationRadius();
	m_DeactivationRadius = pItem.DeactivationRadius();

	m_BaseSoundVolume = pItem.BaseSoundVolume();
	m_BaseSoundRadius = pItem.BaseSoundRadius();

	m_AmbientSoundPath = pItem.AmbientSoundPath();

	if( pItem.TakeSoundPath().length() > 0 )
	{
		m_pTakeSound = m_pRefManager.AddSample( pItem.TakeSoundPath(), kFalse );
	}
	if( pItem.DropSoundPath().length() > 0 )
	{
		m_pDropSound = m_pRefManager.AddSample( pItem.DropSoundPath(), kFalse );
	}
	if( pItem.LandSoundPath().length() > 0 )
	{
		m_pLandSound = m_pRefManager.AddSample( pItem.LandSoundPath(), kFalse );
	}
	if( pItem.UseSoundPath().length() > 0 )
	{
		m_pUseSound = m_pRefManager.AddSample( pItem.UseSoundPath(), kFalse );
	}

	FindParticles();

	if( pItem.HasIcon() )
	{
		m_pIconMaterial = m_pRefManager.AddMaterial( pD3DDevice,
													 pItem.IconPath(),
													 pItem.IconAlphaPath(),
													 "",
													 "",
													 "" );
		int32 Width = pItem.IconWidth();
		int32 Height = pItem.IconHeight();
		m_SlotsWide = (int32)floor( (float32)Width / (float32)KIconSlotSize );
		m_SlotsTall = (int32)floor( (float32)Height / (float32)KIconSlotSize );

		m_pIconSprite = new CSprite( pD3DDevice,
									 m_pIconMaterial,
									 0,
									 0,
									 (float32)Width,
									 (float32)Height,
									 0,
									 0,
									 1,
									 1,
									 KSpriteTopLeft );

	}
	int32 Sockets = pItem.Sockets();
	if( Sockets > m_SlotsTall )
	{
		Sockets = m_SlotsTall;
	}
	SetSockets( Sockets );


	if( !IsArtifact() &&
		!IsUnique() )
	{
		SetIdentified( kTrue );
	}

} // CItem::CItem()

CItem::~CItem( void )
{
	DELETE_SAFELY( m_pDropPath );
	DELETE_SAFELY( m_pItemName );
	DELETE_SAFELY( m_pItemBackdrop );
	DELETE_SAFELY( m_pIconSprite );
	DELETE_SAFELY( m_pAttackSounds );
	DELETE_SAFELY( m_pTrapSounds );
	DELETE_SAFELY( m_pStrikeSounds );
	DELETE_SAFELY( m_pMissSounds );
	DELETE_SAFELY( m_pItemSounds );

	DELETE_SAFELY( m_pLeftAttack );
	DELETE_SAFELY( m_pRightAttack );
	for( uint32 i = 0; i < m_pItems.size(); i++ )
	{
		DELETE_SAFELY( m_pItems[i] );
	}
	m_pItems.clear();

	for( int32 i = 0; i < m_pSFX.size(); i++ )
	{
		DELETE_SAFELY( m_pSFX[i] );
	}
	m_pSFX.clear();

	ClearEffects();

	m_pPositionalAudioManager.RemoveAudio( m_pAmbientPositional );

	m_pRefManager.RemoveSample( m_pTakeSound );
	m_pRefManager.RemoveSample( m_pDropSound );
	m_pRefManager.RemoveSample( m_pLandSound );
	m_pRefManager.RemoveSample( m_pUseSound );

	m_pRefManager.RemoveMaterial( m_pLightPoolMaterial );
	m_pRefManager.RemoveModel( m_pCollisionModel );

	m_pRefManager.RemoveMaterial( m_pIconMaterial );

	DELETE_SAFELY( m_pOpenModel );
	DELETE_SAFELY( m_pCloseModel );
	DELETE_SAFELY( m_pModel );

	DELETE_SAFELY( m_pCullingBounds );

	for( int32 i = 0; i < m_pGlowList.size(); i++ )
	{
		m_pGlowList[i]->m_RequestingRemoval = kTrue;
	}
	for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
	{
		m_pModelGlowList[i]->m_RequestingRemoval = kTrue;
	}

} // CItem::~CItem()

void CItem::CalculateIdentifyLevel( void )
{
	int32 BaseLevel( 1 );
	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
		{
			int32 Value =  (int32)m_pEffects[i][j]->Value();
			
			Value = (int32)( (float32)Value / (float32)KEffectMaximum[m_pEffects[i][j]->Type()] * 40.0f );
			if( Value > 100 )
			{
				Value = 100;
			}

			if( abs( Value ) > BaseLevel )
			{
				BaseLevel = abs( Value );
			}
		}
	}
	SetBaseLevel( BaseLevel );

} // CItem::CalculateIdentifyLevel()

void CItem::DeleteContainedItems( void )
{
	for( uint32 j = 0; j < m_pItems.size(); j++ )
	{

		for( uint32 i = 0; i < m_pItems[j]->Effects( KActivationPassive ); i++ )
		{
			CEffect* pEffect = m_pItems[j]->Effect( KActivationPassive, i );
			// elemental damage is added slightly differently from other effects
			if( pEffect->Type() == KEffectAddDamageType )
			{
				EDamageType DamageType( KDamageFire );
				std::string EffectName = StringUpper( pEffect->Name() );
				for( uint32 a = 0; a < KDamageTypes; a++ )
				{
					if( EffectName == KDamageNames[a] )
					{
						DamageType = (EDamageType)a;
					}
				}
				// non-weapon items get resistances instead of damage bonuses
				if( Category() != KCategoryWeapon )
				{
					CEffect* pNewEffect = new CEffect( pEffect );
					pNewEffect->SetType( KDamageResistance[DamageType] );
					pNewEffect->SetValue( pNewEffect->Value() * 4 );	// resistances get multiplied by 4x
					RemoveEffect( pNewEffect );
					DELETE_SAFELY( pNewEffect );
				}
				else
				{
					if( pEffect->Value() != 0 )
					{
						RemoveDamageBonus( DamageType, (int32)pEffect->Value() );
					}
				}
			}
			else
			{
				RemoveEffect( pEffect );
			}
		}

		DELETE_SAFELY( m_pItems[j] );
	}
	m_pItems.clear();
} // CItem::DeleteContainedItems()

void CItem::ApplyContainedEffects( void )
{
	for( uint32 j = 0; j < m_pItems.size(); j++ )
	{
		for( uint32 i = 0; i < m_pItems[j]->Effects( KActivationPassive ); i++ )
		{
			CEffect* pEffect = m_pItems[j]->Effect( KActivationPassive, i );
			// elemental damage is added slightly differently from other effects
			if( pEffect->Type() == KEffectAddDamageType )
			{
				EDamageType DamageType( KDamageFire );
				std::string EffectName = StringUpper( pEffect->Name() );
				for( uint32 a = 0; a < KDamageTypes; a++ )
				{
					if( EffectName == KDamageNames[a] )
					{
						DamageType = (EDamageType)a;
					}
				}
				// non-weapon items get resistances instead of damage bonuses
				if( Category() != KCategoryWeapon )
				{
					CEffect* pNewEffect = new CEffect( pEffect );
					pNewEffect->SetType( KDamageResistance[DamageType] );
					pNewEffect->SetValue( pNewEffect->Value() * 4 );	// resistances get multiplied by 4x
					AddEffect( pNewEffect );
					DELETE_SAFELY( pNewEffect );
				}
				else
				{
					if( pEffect->Value() != 0 )
					{
						AddDamageBonus( DamageType, (int32)pEffect->Value() );
					}
				}
			}
			else
			{
				AddEffect( pEffect );
			}
		}
	}
} // CItem::ApplyContainedEffects()

void CItem::FillInstance( CItemSaveInstance& pInstance,		// description to fill
						  uint32 SlotIndex,					// slot it is in
						  bool Equipped )					// equipped?
{

	pInstance.m_Position = Position();
	pInstance.m_Orientation = Orientation();

	pInstance.m_ArmorBonus = m_ArmorBonus;

	pInstance.m_SlotIndex = SlotIndex;
	pInstance.m_Equipped = Equipped;

	pInstance.m_BaseName = m_BaseName;
	pInstance.m_BaseModifiedName = m_BaseModifiedName;
	pInstance.m_Name = m_Name;

	pInstance.m_Value = m_Value;

	pInstance.m_TrapType = m_TrapType;

	pInstance.m_Uses = m_Uses;

	pInstance.m_Grade = m_Grade;

	pInstance.m_Unique = m_IsUnique;

	pInstance.m_Artifact = m_IsArtifact;

	pInstance.m_CanInteract = m_CanInteract;

	pInstance.m_Open = m_Open;

	pInstance.m_Operateable = m_Operateable;

	pInstance.m_Destructible = m_Destructible;

	pInstance.m_Identified = m_Identified;
	pInstance.m_Sockets = m_Sockets;

	pInstance.m_DamageBonus = m_DamageBonus;
	pInstance.m_DamageBonusValue = m_DamageBonusValue;


	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
		{
			pInstance.m_pEffects[i].push_back( new CEffect( m_pEffects[i][j] ) );
		}
	}
	

	for( int32 i = 0; i < m_pItems.size(); i++ )
	{
		CItemSaveInstance* pItemInstance = new CItemSaveInstance();
		m_pItems[i]->FillInstance( *pItemInstance, 0, kFalse );
		pInstance.m_pItems.push_back( pItemInstance );
	}


} // CItem::FillInstance()

void CItem::ApplyInstance( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   CItemSaveInstance& pInstance,	// description to fill
						   CItemTemplate& pItemManifest )	// item manifest to construct items from
{
	ClearEffects();
	m_Position = pInstance.m_Position;
	m_OrientationMatrix = pInstance.m_Orientation;

	m_BaseName = pInstance.m_BaseName;
	m_BaseModifiedName = pInstance.m_BaseModifiedName;

	m_Name = pInstance.m_Name;

	m_ArmorBonus = pInstance.m_ArmorBonus;

	m_Uses = pInstance.m_Uses;

	m_IsUnique = pInstance.m_Unique;

	m_IsArtifact = pInstance.m_Artifact;

	m_Grade = pInstance.m_Grade;
	if( m_Grade != KGradeNormal )
	{
		m_BaseModifiedName = "\b" + KGradeName[m_Grade] + "\b" + m_BaseName;
	}
	else
	{
		m_BaseModifiedName = m_BaseName;
	}

	m_Open = pInstance.m_Open;

	m_CanInteract = pInstance.m_CanInteract;

	m_Operateable = pInstance.m_Operateable;

	m_Destructible = pInstance.m_Destructible;

	SetTrapType( pInstance.m_TrapType ); 

	m_Identified = pInstance.m_Identified;
	m_Sockets = pInstance.m_Sockets;

	m_DamageBonus = pInstance.m_DamageBonus;
	m_DamageBonusValue = pInstance.m_DamageBonusValue;

	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < pInstance.m_pEffects[i].size(); j++ )
		{
			m_pEffects[i].push_back( new CEffect( pInstance.m_pEffects[i][j] ) );
		}
	}

	CalculateIdentifyLevel();

	for( int32 i = 0; i < pInstance.m_pItems.size(); i++ )
	{
		if( pInstance.m_pItems[i]->m_BaseName == "Gold" ) // problem spot?
		{
			AddGold( pD3DDevice, pInstance.m_pItems[i]->m_Value );
		}
		else
		{
			CItemDescription* pDescription = pItemManifest.ItemDescription( pInstance.m_pItems[i]->m_BaseName );
			CItem* pItem = NULL;
			if( pDescription != NULL )
			{
				pItem = new CItem( pD3DDevice,
								m_pRefManager,
								m_pSettings,
								m_pParticleManager,
								m_pGlowManager,
								m_pPositionalAudioManager,
								*pDescription );
				pItem->ApplyInstance( pD3DDevice,
									*pInstance.m_pItems[i],
									pItemManifest );

				m_pItems.push_back( pItem );
			}
		}
	}

	if( m_Open && m_CanOpen )
	{
		if( m_Destructible )
		{
			m_Collideable = kFalse;
		}
		m_pModel->PlayAnimation( "OPENED", kTrue, 0.0f );
	}
	if( m_Open )
	{
		m_OpenComplete = kTrue;
	}

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );

	// these events/containers have already been activated, so kill their effects
	if( ( Category() == KCategoryEvent &&
		  m_Open ) || 
		  ( IsContainer() && !CanInteract() ) )
	{
		m_HasLightPool = kFalse;
		if( m_pAmbientPositional != NULL )
		{
			m_pPositionalAudioManager.RemoveAudio( m_pAmbientPositional );
			m_pAmbientPositional = NULL;
		}
	}



} // CItem::ApplyInstance()


void CItem::SetAmbientPositionalAudio( const std::string& Path )	// path to audio clip
{
	m_pPositionalAudioManager.RemoveAudio( m_pAmbientPositional );

	if( Path.length() > 0 )
	{
		m_pAmbientPositional = m_pPositionalAudioManager.AddAudio( m_Position,
																	Path,
																	m_BaseSoundVolume,
																	m_BaseSoundRadius );

	}
} // CItem::SetAmbientPositionalAudio( void );

bool CItem::RequirementsMet( CCharacter& pCharacter )	// character to check requirements for
{
	bool RequirementsMet( kTrue );
	bool AllowsEffects( Category() == KCategoryWeapon ||
						Category() == KCategoryArmor ||
						Category() == KCategoryJewelry );
	for( uint32 i = 0; i < Requirements(); i++ )
	{
		if( !pCharacter.RequirementMet( RequirementStat( i ),
										RequirementValue( i ),
										AllowsEffects ) )
		{
			RequirementsMet = kFalse;
		}
	}
	return RequirementsMet;
} // CItem::RequirementsMet()

void CItem::ClearEffects( void )
{
	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
		{
			DELETE_SAFELY( m_pEffects[i][j] );
		}
		m_pEffects[i].clear();
	}

} // CItem::ClearEffects()

uint32 CItem::ArmorBonus( void )		
{	

	int32 Bonus;
	int32 Armor = m_ArmorBonus;

	if( m_Grade > KGradeNormal )
	{
		Armor = m_MaximumArmorBonus;
	}

	Bonus = (int32)ceil( (float32)Armor * KGradeBonus[m_Grade] / 100.0f );
	
	if( m_Grade > KGradeNormal && Bonus < 1 )
	{
		Bonus = 1;
	}
	return Armor + Bonus;		
} // CItem::ArmorBonus()


uint32 CItem::MinimumDamage( void )		
{	
	int32 Bonus = (int32)ceil( (float32)m_MaximumDamage * KGradeBonus[m_Grade] / 100.0f );		
	if( m_Grade > KGradeNormal && Bonus < 1 )
	{
		Bonus = 1;
	}

	return m_MinimumDamage + Bonus;
} // CItem::MinimumDamage()

uint32 CItem::MaximumDamage( void )		
{	
	int32 Bonus = (int32)ceil( (float32)m_MaximumDamage * KGradeBonus[m_Grade] / 100.0f );		
	if( m_Grade > KGradeNormal && Bonus < 1 )
	{
		Bonus = 1;
	}

	return m_MaximumDamage + Bonus;
} // CItem::MaximumDamage()

uint32 CItem::PurchaseValue( void )
{
	uint32 PurchaseValue = Value();
	
	uint32 EffectCount = Effects( KActivationPassive );
	uint32 EnchantmentCount( 0 );
	for( uint32 i = 0; i < EffectCount; i++ )
	{
		if( Effect( KActivationPassive, i )->PriceMultiplier() != 0 )
		{
			EnchantmentCount++;
		}
	}


	if( EnchantmentCount > 0 )
	{
		PurchaseValue *= EnchantmentCount;
		if( PurchaseValue < 1000 )
		{
			PurchaseValue /= 10;
			PurchaseValue += 1000;
		}
	}
	for( int32 i = 0; i < EffectCount; i++ )
	{
		PurchaseValue += (uint32)floor( Value() * Effect( KActivationPassive, i )->PriceMultiplier() );
	}

	for( int32 i = 0; i < DamageBonuses(); i++ )
	{
		PurchaseValue += (uint32)floor( (float32)Value() * (float32)DamageBonusValue( i ) / 10.0f );
	}

	PurchaseValue += Sockets() * 500;

	PurchaseValue += (uint32)( .5f * ( (float32)PurchaseValue * (float32)Sockets() ) );

	PurchaseValue += (int32)floor( (float32)PurchaseValue * KGradeBonus[m_Grade] / 20.0f );		

	// decrement cost by usage
	if( m_OriginalUses != KUnlimitedUses )
	{
		float32 Percentage = (float32)m_Uses / (float32)m_OriginalUses;
		PurchaseValue = (uint32)( Percentage * (float32)PurchaseValue );
	}
	return PurchaseValue;
} // CItem::PurchaseValue()

uint32 CItem::SellValue( void )
{
	if( !Identified() &&
		Effects( KActivationPassive ) > 0 )
	{
		return Value() / 8;
	}
		
	return PurchaseValue() / 8;
} // CItem::SellValue()

uint32 CItem::EnchantValue( void )
{

	int32 ItemValue = PurchaseValue() / 4;
	if( ItemValue < 1000 )
	{
		ItemValue = 1000;
	}
	if( Category() == KCategoryJewelry &&
		ItemValue < 10000 )
	{
		ItemValue = 10000;
	}
	return ItemValue;
} // CItem::EnchantValue()

uint32 CItem::GambleValue( void )
{

		
	return (int32)( (float32)PurchaseValue() * m_GambleMultiplier );
} // CItem::GambleValue()

void CItem::CreateNameForDisplay( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device)
								  CFontMetric* pFontMetric,				// font metric
								  CMaterial* pFontMaterial )			// font material
{
	DELETE_SAFELY( m_pItemName );
	DELETE_SAFELY( m_pItemBackdrop );
	if( Identified() )
	{
		std::string TextName = Name();
		if( Sockets() > 0 )
		{
			TextName = TextName + LoadLocalizedString(992);
		}
		m_pItemName = new CText( pD3DDevice,
								 pFontMaterial,
								 pFontMetric,
								 TextName,
								 512,
								 48,
								 .8f,
								 kFalse,
								 KJustifyCenter );
	}
	else
	{
		std::string TextName = BaseModifiedName();
		if( Sockets() > 0 )
		{
			TextName = TextName + LoadLocalizedString(992);
		}
		m_pItemName = new CText( pD3DDevice,
								 pFontMaterial,
								 pFontMetric,
								 TextName,
								 512,
								 48,
								 .8f,
								 kFalse,
								 KJustifyCenter );
	}


	if( Effects( KActivationPassive ) > 0 ||
		DamageBonuses() > 0 )
	{
		m_pItemName->SetBold( pD3DDevice, kTrue );
	}
	else
	{
		m_pItemName->SetBold( pD3DDevice, kFalse );
	}

	// first resize our description shadow to fit
	float32 Width = m_pItemName->MaxBounds().x -
					m_pItemName->MinBounds().x;
	float32 Height = m_pItemName->MaxBounds().y -
					 m_pItemName->MinBounds().y;

	Width *= .5f;
	Height *= .5f;
	Width += 6;
	Height += 6;

	m_pItemBackdrop = new CSprite( pD3DDevice,
							  	   NULL,
								   0,
								   0,
								   Width * 2,
								   Height * 2,
								   0,
								   0,
								   1,
								   1,
								   KSpriteCenter );

	m_NameIdentified = Identified();
} // CItem::CreateNameForDisplay()


void CItem::RemoveEffect( CEffect* pEffect )	// effect to add
{
	bool Changed = kTrue;
	if( pEffect->Activation() == KActivationPassive  )
	{
		for( int32 i = 0; i < (int32)Effects(  pEffect->Activation() ); i++ )
		{
			if( Effect( pEffect->Activation(), i )->Type() == pEffect->Type()  )
			{
				CEffect* pSubEffect = Effect( pEffect->Activation(), i );
				pSubEffect->SetValue( pSubEffect->Value() - pEffect->Value() );
				break;
			}
		}
	}
	for( int32 i = 0; i < (int32)Effects( KActivationPassive ); i++ )
	{
		if( Effect( KActivationPassive, i )->Value() == 0  )
		{
			DELETE_SAFELY( m_pEffects[KActivationPassive][i] );
			m_pEffects[KActivationPassive][i] = m_pEffects[KActivationPassive][Effects( KActivationPassive ) - 1];
			m_pEffects[KActivationPassive].pop_back();
			i--;
		}
	}

} // CItem::RemoveEffect()

void CItem::AddEffect( CEffect* pEffect )	// effect to add
{
	bool Merged( kFalse );
	if( pEffect->Activation() == KActivationPassive  )
	{
		for( uint32 i = 0; i < Effects(  pEffect->Activation() ); i++ )
		{
			if( Effect( pEffect->Activation(), i )->Type() == pEffect->Type()  )
			{
				Effect( pEffect->Activation(), i )->SetValue( pEffect->Value() + Effect( pEffect->Activation(), i )->Value() );
				Merged = kTrue;
				break;
			}
		}
	}
	if( !Merged )
	{
		m_pEffects[pEffect->Activation()].push_back( new CEffect( pEffect ) );
	}
} // CItem::AddEffect()

void CItem::AddNewEffect( CEffect* pEffect )	// effect to add
{
	bool Merged( kFalse );
	if( pEffect->Activation() == KActivationPassive )
	{
		for( uint32 i = 0; i < Effects(  pEffect->Activation() ); i++ )
		{
			if( Effect( pEffect->Activation(), i )->Type() == pEffect->Type()  )
			{
				Effect( pEffect->Activation(), i )->SetValue( pEffect->Value() + Effect( pEffect->Activation(), i )->Value() );
				Merged = kTrue;
				break;
			}
		}
	}
	if( !Merged )
	{
		m_pEffects[pEffect->Activation()].push_back( pEffect );
	}
	else
	{
		DELETE_SAFELY( pEffect );
	}
} // CItem::AddNewEffect()

void CItem::BestowGrade( EDifficulty Difficulty )	// game difficulty - determines what can be had
{
	if( !( Category() == KCategoryWeapon ||
		   Category() == KCategoryArmor ) )
	{
		return;
	}

	if( m_Grade != KGradeNormal )
	{
		return;
	}
	switch( Difficulty )
	{
	case KDifficultyPage :
		break;
	case KDifficultyAdventurer :
		m_Grade = KGradeSuperior;
		break;
	case KDifficultyHero :
		m_Grade = KGradeSuperior;
		if( RandomIntegerBetween( 0, 100 ) < 25 )
		{
			m_Grade = KGradeExceptional;
		}
		break;
	case KDifficultyLegend :
		m_Grade = KGradeSuperior;
		if( RandomIntegerBetween( 0, 100 ) < 30 )
		{
			m_Grade = KGradeExceptional;
		}
		else if( RandomIntegerBetween( 0, 100 ) < 50 )
		{
			m_Grade = KGradeFlawless;
		}
		break;
	}
	DetermineName();

	m_pLeftAttack->SetDamageMinimumDamage(MinimumDamage(), MaximumDamage());
	m_pRightAttack->SetDamageMinimumDamage(MinimumDamage(), MaximumDamage());



} // CItem::BestowGrade()

void CItem::BestowSockets( bool AllowUnique )	// allow unique items to be socketed
{
	// unique items don't get sockets!
	if( !AllowUnique && IsUnique() )
	{
		return;
	}

	if( Category() == KCategoryWeapon ||
		Category() == KCategoryArmor ||
		Category() == KCategoryJewelry )
	{
		if( Sockets() >= (uint32)m_SlotsTall )
		{
			return;
		}
		uint32 SocketCount = m_SlotsTall;

		SocketCount = RandomIntegerBetween( Sockets() + 1 , SocketCount );

		
		SetSockets( SocketCount );

		// clear out our 3d text description so it gets recreated.
		DELETE_SAFELY( m_pItemName );

	}

} // CItem::BestowSockets();

void CItem::RemovePowers( void )			// remove all powers
{
	if( Category() == KCategoryWeapon ||
		Category() == KCategoryArmor ||
		Category() == KCategoryJewelry )
	{
		ClearEffects();
		m_IsUnique = kFalse;
		m_IsArtifact = kFalse;

		m_DamageBonus.clear();
		m_DamageBonusValue.clear();

		ApplyContainedEffects();

		DetermineName();
	}
} // CItem::RemovePowers()

void CItem::BestowPowers( int32 Level,		// level for baseline - 100 = best
						  bool AllowCurse,	// allow cursed items?
						  int32 MinimumLevel )	
{
	if( IsArtifact() )
	{
		return;
	}
	bool AddedEffect( kFalse );
	if( Category() == KCategoryWeapon ||
		Category() == KCategoryArmor ||
		Category() == KCategoryJewelry )
	{

		if( Level > 100 )
		{
			Level = 100;
		}


		float32 LevelModifier = (float32)Level / 100;
		float32 Flatten = LevelModifier * 10;
		if( Flatten > 1 )
		{
			Flatten = 1;
		}
		LevelModifier *= Flatten;


		float32 MinimumLevelModifier = (float32)MinimumLevel / 100;
		Flatten = MinimumLevelModifier * 10;
		if( Flatten > 1 )
		{
			Flatten = 1;
		}
		MinimumLevelModifier *= Flatten;

		if( MinimumLevel == -1 )
		{
			MinimumLevelModifier = LevelModifier * .5f;
		}
		
		// bias the best items toward the end

		int32 Prefix( 0 );
		int32 Suffix( 0 );
		bool Done( kFalse );
		int32 Tries( 0 );
		bool AllowEffectStack( kFalse );
		if( RandomBetween( 0, 100 ) < 50 )
		{
			AllowEffectStack = kTrue;
		}
		while( !Done &&
				Tries < 10000 )
		{
			Done = kTrue;

			if( Category() == KCategoryWeapon )
			{
				Prefix	= RandomIntegerBetween( 0, 	KEffectKnockback );
			}
			else
			{
				Prefix	= RandomIntegerBetween( 0, 	KEffectSkillCharmMagic );
			}
			if( !AllowEffectStack )
			{
				for( uint32 i = 0; i < Effects( KActivationPassive ); i++ )
				{
					if( Effect( KActivationPassive, i )->Type() == Prefix )
					{
						Done = kFalse;
					}
				}
			}
			Tries++;
		}
		Done = kFalse;
		Tries = 0;
		while( !Done &&
				Tries < 10000 )
		{
			Done = kTrue;
			Suffix	= RandomIntegerBetween( KEffectPercentStrength, KEffectPercentItemRequirements );
			for( uint32 i = 0; i < Effects( KActivationPassive ); i++ )
			{
				if( Effect( KActivationPassive, i )->Type() == Suffix )
				{
					Done = kFalse;
				}
			}
			Tries++;
		}

		float32 SuffixPotency = RandomBetween( MinimumLevelModifier, LevelModifier );
		SuffixPotency *= sgn( (float32)KEffectMaximum[Suffix] );
		if( AllowCurse && RandomIntegerBetween( 0, 100 ) < 20 )
		{
			SuffixPotency *= -1.0f;
		}
		float32 PrefixPotency = RandomBetween( MinimumLevelModifier, LevelModifier );
		PrefixPotency *= sgn( (float32)KEffectMaximum[Prefix] );
		if( AllowCurse && RandomIntegerBetween( 0, 100 ) < 20 )
		{
			PrefixPotency *= -1.0f;
		}


		int32 Percentage = RandomIntegerBetween( 0, 100 );
		if( Percentage < 60 )
		{
			Prefix = -1;
		}
		else if( Percentage < 80 )
		{
			Suffix = -1;
		}

		int32 PrefixValue( 0 );
		int32 SuffixValue( 0 );
		if( Suffix >= 0 )
		{
			/*if( ( SuffixPotency < 0 && KEffectMinimum[Suffix] == 0 ) ||
				( SuffixPotency > 0 && KEffectMaximum[Suffix] == 0 ) )
			{
				SuffixPotency *= -1.0f;
			}*/

			bool Positive( kTrue );
			float32 PriceMultiplier( 1 );
			if( sgn( (float32)SuffixPotency  ) != sgn( (float32)KEffectMaximum[Suffix] ) )
			{
				PriceMultiplier = fabs( SuffixPotency ) * 5;
				Positive = kFalse;
				SuffixPotency = fabs( SuffixPotency );
				SuffixValue = (int32)( KEffectMinimum[Suffix] * SuffixPotency );
				if( SuffixValue == 0 )
				{
					SuffixValue = 1 * (int32)sgn( (float32)KEffectMinimum[Suffix] );
				}
				int32 SuffixIndex = (int32)floor( SuffixPotency * KNegativeSuffixes );
				if( SuffixIndex >= KNegativeSuffixes )
				{
					SuffixIndex = KNegativeSuffixes - 1;
				}
			}
			else
			{
				PriceMultiplier = fabs( SuffixPotency ) * 40;
				SuffixPotency = fabs( SuffixPotency );
				SuffixValue = (int32)( KEffectMaximum[Suffix] * SuffixPotency );
				if( SuffixValue == 0 )
				{
					SuffixValue = 1 * (int32)sgn( (float32)KEffectMaximum[Suffix] );
				}
				int32 SuffixIndex = (int32)floor( SuffixPotency * KPositiveSuffixes );
				if( SuffixIndex >= KPositiveSuffixes )
				{
					SuffixIndex = KPositiveSuffixes - 1;
				}

			}
			if( SuffixValue != 0 )
			{
				AddedEffect = kTrue;
				AddNewEffect( new CEffect( (EEffectType)Suffix,
											Positive,
											KActivationPassive,
											KDurationAlways,
											(float32)SuffixValue,
											PriceMultiplier ) );
			}

		}		
		if( Prefix >= 0 )
		{
			/*if( ( PrefixPotency < 0 && KEffectMinimum[Prefix] == 0 ) ||
				( PrefixPotency > 0 && KEffectMaximum[Prefix] == 0 ) )
			{
				PrefixPotency *= -1.0f;
			}*/

			bool Positive = kTrue;
			float32 PriceMultiplier( 1 );
			if( sgn( (float32)PrefixPotency  ) != sgn( (float32)KEffectMaximum[Prefix] ) )
			{
				PriceMultiplier = fabs( PrefixPotency ) * 8;

				Positive = kFalse;
				PrefixPotency = fabs( PrefixPotency );
				PrefixValue = (int32)( KEffectMinimum[Prefix] * PrefixPotency );
				if( PrefixValue < 1 )
				{
					PrefixValue = 1 * (int32)sgn( (float32)KEffectMinimum[Prefix] );
				}
				int32 PrefixIndex = (int32)floor( PrefixPotency * KNegativeSuffixes );
				if( PrefixIndex >= KNegativeSuffixes )
				{
					PrefixIndex = KNegativeSuffixes - 1;
				}
			}
			else
			{
				PriceMultiplier = fabs( PrefixPotency ) * 60;

				if( PrefixValue < 1 )
				{
					PrefixValue = 1 * (int32)sgn( (float32)KEffectMaximum[Prefix] );
				}
				PrefixPotency = fabs( PrefixPotency );
				PrefixValue = (int32)( KEffectMaximum[Prefix] * PrefixPotency );
				int32 PrefixIndex = (int32)floor( PrefixPotency * KPositiveSuffixes );
				if( PrefixIndex >= KPositiveSuffixes )
				{
					PrefixIndex = KPositiveSuffixes - 1;
				}

			}
			if( PrefixValue != 0 )
			{
				AddedEffect = kTrue;
				AddNewEffect( new CEffect( (EEffectType)Prefix,
										Positive,
										KActivationPassive,
										KDurationAlways,
										(float32)PrefixValue,
										PriceMultiplier ) );
			}

		}	
		
	}

	// give an elemental bonus?
	int32 Percent = RandomIntegerBetween( 0, 100 );
	if( Category() == KCategoryWeapon &&
		Percent < 20 )
	{
		int32 PossibleLevel( Level );
		if( PossibleLevel > 50 )
		{
			PossibleLevel = 50;
		}
		int32 Prefix = RandomIntegerBetween( KDamageFire, KDamageUndead );
		int32 Potency = RandomIntegerBetween( PossibleLevel / 2, PossibleLevel );
		if( Potency > 0 )
		{
			AddDamageBonus( (EDamageType)Prefix, Potency );
			AddedEffect = kTrue;
		}
	}

	if( !IsUnique() )
	{
		DetermineName();
	}

	if( AddedEffect )
	{
		SetIdentified( kFalse );
	}

	CalculateIdentifyLevel();
	
} // CItem::BestowPowers()

void CItem::BestowUniquePowers( CNameGenerator& pNameGenerator,	// name generator for unique name
								int32 Level,					// level for baseline - 100 = best
								bool AllowCurse,				// allow cursed items?
								int32 MinimumLevel )			// minimum level
{
	if( IsArtifact() )
	{
		return;
	}
	if( IsUnique() )
	{
		return;
	}

	bool AddedEffect( kFalse );

	if( Category() == KCategoryWeapon ||
		Category() == KCategoryArmor ||
		Category() == KCategoryJewelry )
	{
		Level += 5;



		if( Level > 100 )
		{
			Level = 100;
		}

		float32 LevelModifier = (float32)Level / 100;
		float32 Flatten = LevelModifier * 10;
		if( Flatten > 1 )
		{
			Flatten = 1;
		}
		LevelModifier *= Flatten;


		float32 MinimumLevelModifier = (float32)MinimumLevel / 100;
		Flatten = MinimumLevelModifier * 10;
		if( Flatten > 1 )
		{
			Flatten = 1;
		}
		MinimumLevelModifier *= Flatten;

		if( MinimumLevel == -1 )
		{
			MinimumLevelModifier = LevelModifier * .5f;
		}
		// bias the best items toward the end

		// can have many extra effects!
		uint32 EffectStacks = RandomIntegerBetween( 2, 4 );

		for( uint32 i = 0; i < EffectStacks; i++ )
		{
			int32 Prefix( 0 );
			int32 Suffix( 0 );
			bool Done( kFalse );
			int32 Tries( 0 );
			while( !Done &&
				   Tries < 10000 )
			{
				Done = kTrue;
				if( Category() == KCategoryWeapon )
				{
					Prefix	= RandomIntegerBetween( 0, 	KEffectKnockback );
				}
				else
				{
					Prefix	= RandomIntegerBetween( 0, 	KEffectSkillCharmMagic );
				}
				for( uint32 j = 0; j < Effects( KActivationPassive ); j++ )
				{
					if( Effect( KActivationPassive, j )->Type() == Prefix )
					{
						Done = kFalse;
					}
				}
			Tries++;

			}
			Tries = 0;
			Done = kFalse;
			while( !Done  &&
				   Tries < 10000 )
			{
				Done = kTrue;
				Suffix	= RandomIntegerBetween( KEffectPercentStrength, KEffectPercentItemRequirements );
				for( uint32 j = 0; j < Effects( KActivationPassive ); j++ )
				{
					if( Effect( KActivationPassive, j )->Type() == Suffix )
					{
						Done = kFalse;
					}
				}
				Tries++;
			}			
			float32 SuffixPotency = RandomBetween( MinimumLevelModifier, LevelModifier );
			SuffixPotency *= sgn( (float32)KEffectMaximum[Suffix] );
			if( AllowCurse && RandomIntegerBetween( 0, 100 ) < 5 )
			{
				SuffixPotency *= -1.0f;
			}
			float32 PrefixPotency = RandomBetween( MinimumLevelModifier, LevelModifier );
			PrefixPotency *= sgn( (float32)KEffectMaximum[Prefix] );
			if( AllowCurse && RandomIntegerBetween( 0, 100 ) < 5 )
			{
				PrefixPotency *= -1.0f;
			}


			int32 Percentage = RandomIntegerBetween( 0, 100 );
			if( Percentage < 40 )
			{
				Prefix = -1;
			}
			else if( Percentage < 60 )
			{
				Suffix = -1;
			}


			int32 PrefixValue( 0 );
			int32 SuffixValue( 0 );
			if( Suffix >= 0 )
			{
				/*if( ( SuffixPotency < 0 && KEffectMinimum[Suffix] == 0 ) ||
					( SuffixPotency > 0 && KEffectMaximum[Suffix] == 0 ) )
				{
					SuffixPotency *= -1.0f;
				}*/

				bool Positive( kTrue );
				float32 PriceMultiplier( 1 );
				if( sgn( (float32)SuffixPotency  ) != sgn( (float32)KEffectMaximum[Suffix] ) )
				{
					PriceMultiplier = fabs( SuffixPotency ) * 5;
					Positive = kFalse;
					SuffixPotency = fabs( SuffixPotency );
					SuffixValue = (int32)( KEffectMinimum[Suffix] * SuffixPotency );
					if( SuffixValue == 0 )
					{
						SuffixValue = 1 * (int32)sgn( (float32)KEffectMinimum[Suffix] );
					}
					int32 SuffixIndex = (int32)floor( SuffixPotency * KNegativeSuffixes );
					if( SuffixIndex >= KNegativeSuffixes )
					{
						SuffixIndex = KNegativeSuffixes - 1;
					}
				}
				else
				{
					PriceMultiplier = fabs( SuffixPotency ) * 40;
					SuffixPotency = fabs( SuffixPotency );
					SuffixValue = (int32)( KEffectMaximum[Suffix] * SuffixPotency );
					if( SuffixValue == 0 )
					{
						SuffixValue = 1 * (int32)sgn( (float32)KEffectMaximum[Suffix] );
					}
					int32 SuffixIndex = (int32)floor( SuffixPotency * KPositiveSuffixes );
					if( SuffixIndex >= KPositiveSuffixes )
					{
						SuffixIndex = KPositiveSuffixes - 1;
					}
				}
				if( SuffixValue != 0 )
				{
					AddedEffect = kTrue;
					AddNewEffect( new CEffect( (EEffectType)Suffix,
												Positive,
												KActivationPassive,
												KDurationAlways,
												(float32)SuffixValue,
												PriceMultiplier ) );
				}

			}		
			if( Prefix >= 0 )
			{
				/*if( ( PrefixPotency < 0 && KEffectMinimum[Prefix] == 0 ) ||
					( PrefixPotency > 0 && KEffectMaximum[Prefix] == 0 ) )
				{
					PrefixPotency *= -1.0f;
				}*/

				bool Positive = kTrue;
				float32 PriceMultiplier( 1 );
				if( sgn( (float32)PrefixPotency  ) != sgn( (float32)KEffectMaximum[Prefix] ) )
				{
					PriceMultiplier = fabs( PrefixPotency ) * 8;

					Positive = kFalse;
					PrefixPotency = fabs( PrefixPotency );
					PrefixValue = (int32)( KEffectMinimum[Prefix] * PrefixPotency );
					if( PrefixValue < 1 )
					{
						PrefixValue = 1 * (int32)sgn( (float32)KEffectMinimum[Prefix] );
					}
					int32 PrefixIndex = (int32)floor( PrefixPotency * KNegativeSuffixes );
					if( PrefixIndex >= KNegativeSuffixes )
					{
						PrefixIndex = KNegativeSuffixes - 1;
					}
				}
				else
				{
					PriceMultiplier = fabs( PrefixPotency ) * 60;

					if( PrefixValue < 1 )
					{
						PrefixValue = 1 * (int32)sgn( (float32)KEffectMaximum[Prefix] );
					}
					PrefixPotency = fabs( PrefixPotency );
					PrefixValue = (int32)( KEffectMaximum[Prefix] * PrefixPotency );
					int32 PrefixIndex = (int32)floor( PrefixPotency * KPositiveSuffixes );
					if( PrefixIndex >= KPositiveSuffixes )
					{
						PrefixIndex = KPositiveSuffixes - 1;
					}

				}
				if( PrefixValue != 0 )
				{
					AddedEffect = kTrue;
					AddNewEffect( new CEffect( (EEffectType)Prefix,
											Positive,
											KActivationPassive,
											KDurationAlways,
											(float32)PrefixValue,
											PriceMultiplier ) );
				}

			}	
		}

		// give an elemental bonus?
		int32 Percent = RandomIntegerBetween( 0, 100 );
		if( Category() == KCategoryWeapon &&
			Percent < 30 )
		{
			int32 PossibleLevel( Level );
			if( PossibleLevel > 50 )
			{
				PossibleLevel = 50;
			}
			int32 Prefix = RandomIntegerBetween( KDamageFire, KDamageUndead );
			int32 Potency = RandomIntegerBetween( PossibleLevel / 2, PossibleLevel );
			if( Potency > 0 )
			{
				AddedEffect = kTrue;
				AddDamageBonus( (EDamageType)Prefix, Potency );
			}
		}
		DetermineName();

		//m_Name = pNameGenerator.GenerateSimpleName() + "'s \n" + m_Name;
		m_Name = LoadLocalizedString2Variables(1150, pNameGenerator.GenerateSimpleName(), m_Name); // 1150

		m_IsUnique = kTrue;
	}
	CalculateIdentifyLevel();

	if( AddedEffect )
	{
		SetIdentified( kFalse );
	}

} // CItem::BestowUniquePowers()

void CItem::DetermineName( void )
{
	if( IsArtifact() )
	{
		return;
	}
	if( !( Category() == KCategoryWeapon ||
		   Category() == KCategoryArmor ||
		   Category() == KCategoryJewelry ) )
	{
		return;
	}
	std::string UniqueName( "" );
	if( m_Name.find( "\n" ) != -1 )
	{
		int32 Index = m_Name.find( "\n" );
		UniqueName = m_Name.substr( 0, Index - 1 );
		UniqueName = UniqueName + "\n";
	}
		std::string PrefixName( "" );
		std::string SuffixName( "" );
		// boost the level for superunique items
		float32 BestPrefixValue( -1 );
		float32 BestSuffixValue( -1 );
		std::string BestPrefixName( "" );
		std::string BestSuffixName( "" );
		for( uint32 k = 0; k < Effects( KActivationPassive ); k++ )
		{
			bool NamedEffect( kFalse );
			bool IsPrefix( kFalse );
			if( Effect( KActivationPassive, k )->Type() <= KEffectSkillCharmMagic )
			{
				NamedEffect = kTrue;
				IsPrefix = kTrue;
			}
			else if( Effect( KActivationPassive, k )->Type() >= KEffectPercentStrength && 
					 Effect( KActivationPassive, k )->Type() <= KEffectPercentItemRequirements )
			{
				NamedEffect = kTrue;
				IsPrefix = kFalse;
			}

			if( NamedEffect && IsPrefix )
			{
				if( fabs( Effect( KActivationPassive, k )->Value() ) > BestPrefixValue ||
					BestPrefixValue == -1 )
				{
					float32 PrefixValue = Effect( KActivationPassive, k )->Value();
					if( sgn( PrefixValue ) == sgn( (float32)KEffectMaximum[Effect( KActivationPassive, k )->Type()] ) )
					{
						float32 PrefixPotency = fabs( PrefixValue / KEffectMaximum[Effect( KActivationPassive, k )->Type()] );
						int32 PrefixIndex = (int32)floor( PrefixPotency * KPositiveSuffixes );
						if( PrefixIndex >= KPositiveSuffixes )
						{
							PrefixIndex = KPositiveSuffixes - 1;
						}
						//std::string PrefixName = KPositivePrefixSuffix[Effect( KActivationPassive, k )->Type()][PrefixIndex] + " ";
						std::string PrefixName = LoadLocalizedString1Variable(1156, KPositivePrefixSuffix[Effect( KActivationPassive, k )->Type()][PrefixIndex]);
						if( fabs( PrefixValue ) > BestPrefixValue )
						{
							BestPrefixValue = PrefixValue;
							BestPrefixName = PrefixName;
						}
					}
					else
					{
						float32 PrefixPotency = 0;
						if( KEffectMinimum[Effect( KActivationPassive, k )->Type()] != 0 )
						{
							PrefixPotency = fabs( PrefixValue / KEffectMinimum[Effect( KActivationPassive, k )->Type()] );
						}
						int32 PrefixIndex = (int32)floor( PrefixPotency * KNegativeSuffixes );
						if( PrefixIndex >= KNegativeSuffixes )
						{
							PrefixIndex = KNegativeSuffixes - 1;
						}
						std::string PrefixName = KNegativePrefixSuffix[Effect( KActivationPassive, k )->Type()][PrefixIndex] + " ";
						if( fabs( PrefixValue ) > BestPrefixValue )
						{
							BestPrefixValue = PrefixValue;
							BestPrefixName = PrefixName;
						}
					}

				}
			}
			if( NamedEffect && !IsPrefix )
			{
				if( fabs( Effect( KActivationPassive, k )->Value() ) > BestSuffixValue ||
					BestSuffixValue == -1 )
				{
					float32 SuffixValue = Effect( KActivationPassive, k )->Value();
					if( sgn( SuffixValue ) == sgn( (float32)KEffectMaximum[Effect( KActivationPassive, k )->Type()] ) )
					{
						float32 SuffixPotency = fabs( SuffixValue / KEffectMaximum[Effect( KActivationPassive, k )->Type()] );
						int32 SuffixIndex = (int32)floor( SuffixPotency * KPositiveSuffixes );
						if( SuffixIndex >= KPositiveSuffixes )
						{
							SuffixIndex = KPositiveSuffixes - 1;
						}
						//std::string SuffixName = " of " + KPositivePrefixSuffix[Effect( KActivationPassive, k )->Type()][SuffixIndex];
						std::string SuffixName = LoadLocalizedString1Variable(1151, KPositivePrefixSuffix[Effect( KActivationPassive, k )->Type()][SuffixIndex]); // 1151
						if( fabs( SuffixValue ) > BestSuffixValue )
						{
							BestSuffixValue = SuffixValue;
							BestSuffixName = SuffixName;
						}
					}
					else
					{
						float32 SuffixPotency = 0;
						if( KEffectMinimum[Effect( KActivationPassive, k )->Type()] != 0 )
						{
							SuffixPotency = fabs( SuffixValue / KEffectMinimum[Effect( KActivationPassive, k )->Type()] );
						}
						int32 SuffixIndex = (int32)floor( SuffixPotency * KNegativeSuffixes );
						if( SuffixIndex >= KNegativeSuffixes )
						{
							SuffixIndex = KNegativeSuffixes - 1;
						}
						//std::string SuffixName = " of " + KNegativePrefixSuffix[Effect( KActivationPassive, k )->Type()][SuffixIndex];
						std::string SuffixName = LoadLocalizedString1Variable(1151, KNegativePrefixSuffix[Effect( KActivationPassive, k )->Type()][SuffixIndex]); // 1151
						if( fabs( SuffixValue ) > BestSuffixValue )
						{
							BestSuffixValue = SuffixValue;
							BestSuffixName = SuffixName;
						}
					}

				}
			}
		}
		//m_Name = BestPrefixName + m_BaseName + BestSuffixName;
		m_Name = LoadLocalizedString3Variables(1152, BestPrefixName, m_BaseName, BestSuffixName);

		for( uint32 i = 0; i < DamageBonuses(); i++ )
		{
			
			m_Name = KDamageWeaponPrefix[DamageBonusType( i )] + m_Name;
		}
		if( m_Grade != KGradeNormal )
		{
			//m_Name =  "\b" + KGradeName[m_Grade] + "\b" + m_Name;	// apply grade name
			m_Name = LoadLocalizedString2Variables(1153, "\b" + KGradeName[m_Grade] + "\b", m_Name);
			if( IsUnique() )
			{
				m_Name = LoadLocalizedString2Variables(1153, UniqueName, m_Name);
			}
			//m_BaseModifiedName =  "\b" + KGradeName[m_Grade] + "\b" + m_BaseName;	// apply grade name
			m_BaseModifiedName =  LoadLocalizedString2Variables(1154, "\b" + KGradeName[m_Grade] + "\b", m_BaseName);
		}
		else
		{
			if( IsUnique() )
			{
				//m_Name = UniqueName + m_Name;
				m_Name = LoadLocalizedString2Variables(1155, UniqueName, m_Name);
			}
			m_BaseModifiedName =  m_BaseName;	// apply grade name
		}
	// clear out our 3d text description so it gets recreated.
	DELETE_SAFELY( m_pItemName );

} // CItem::DetermineName()

uint32 CItem::Effects( EEffectActivation ActivationType )
{
	return m_pEffects[ActivationType].size();
} // CItem::Effects()

float32 CItem::EffectValue( EEffectActivation ActivationType,	// type of activation
						  EEffectType Type )				// type of effect
{
	float32 Value( 0 );
	for( uint32 i = 0; i < m_pEffects[ActivationType].size(); i++ )
	{
		if( m_pEffects[ActivationType][i]->Type() == Type )
		{
			Value += m_pEffects[ActivationType][i]->Value();
		}
	}
	return Value;
} // CItem::EffectValue()

bool CItem::IsCursed( void )
{
	for( uint32 i = 0; i < m_pEffects[KActivationPassive].size(); i++ )
	{
		if( !m_pEffects[KActivationPassive][i]->Positive() )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CItem::IsCursed()

CEffect* CItem::Effect( EEffectActivation ActivationType,	// type of activation
						uint32 Index )						// index within that type
{
	return m_pEffects[ActivationType][Index];
} // CItem::Effect()

std::string CItem::EffectText( EEffectActivation ActivationType )
{
	std::string Text( "" );
	for( uint32 i = 0; i < m_pEffects[ActivationType].size(); i++ )
	{

		if( i != 0 )
		{
			Text = Text + "\n";
		}
		bool Positive( kTrue );
		int32 Value = (int32)m_pEffects[ActivationType][i]->Value();
		if( sgn( (float32)Value ) != sgn( (float32)KEffectMaximum[m_pEffects[ActivationType][i]->Type()] ) )
		{
			Positive = kFalse;
		}
		Value = abs( Value );
		if( Positive )
		{
			if( KEffectBonusType[m_pEffects[ActivationType][i]->Type()] == KBonusValue )
			{
				if( sgn( m_pEffects[ActivationType][i]->Value() ) > 0 )
				{
					Text = Text + "+" + IntegerToString( Value ) +
						" " + KEffectItemPositiveDescriptions[m_pEffects[ActivationType][i]->Type()];
				}
				else
				{
					Text = Text + IntegerToString( Value ) +
						" " + KEffectItemPositiveDescriptions[m_pEffects[ActivationType][i]->Type()];
				}
			}
			else
			{
				Text = Text + IntegerToString( Value ) +
					KEffectItemPositiveDescriptions[m_pEffects[ActivationType][i]->Type()];
			}
		}
		else
		{
			if( KEffectBonusType[m_pEffects[ActivationType][i]->Type()] == KBonusValue )
			{
				if( sgn( m_pEffects[ActivationType][i]->Value() ) < 0 )
				{
					Text = Text + "-" + IntegerToString( Value ) +
						" " + KEffectItemNegativeDescriptions[m_pEffects[ActivationType][i]->Type()];
				}
				else
				{
					Text = Text + IntegerToString( Value ) +
						" " + KEffectItemNegativeDescriptions[m_pEffects[ActivationType][i]->Type()];
				}
			}
			else
			{
				Text = Text + IntegerToString( Value ) +
					KEffectItemNegativeDescriptions[m_pEffects[ActivationType][i]->Type()];
			}
		}
	}
	return Text;
} // CItem::EffectText()

void CItem::BlendRandomIdleAnimation( void )
{
	if( m_IdleAnimations.size() == 1 &&
		!m_pModel->AnimationPlaying() )
	{
		m_pModel->PlayAnimation( 0, kTrue );
	}
	else
	{
		uint32 Index = (uint32)RandomBetween( 0, (float32)m_IdleAnimations.size() );
		if( Index >= m_IdleAnimations.size() )
		{
			Index = 0;
		}
		m_pModel->QueueBlendAnimation( Index, kFalse, 0.2f );
	}
} // CItem::BlendRandomIdleAnimation()

void CItem::FindParticles( void )
{
	for( uint32 i = 0; i < m_pModel->GetPointCount(); i++ )
	{
		CDataPoint& Point = m_pModel->GetPoint( i );
		std::string Name = StringUpper( Point.Name() );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( Point.UserData() );
			if( ParticleIndex != -1 )
			{
				D3DXVECTOR3 Position = Point.Position();
				m_ParticlePosition.push_back( Position );
				m_ParticleOrientation.push_back( Point.Orientation() );
				m_ParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGlowManager.GetGlowType( Point.UserData() );
			D3DXVECTOR3 Position = Point.Position();
			if( GlowIndex != -1 )
			{
				m_GlowPosition.push_back( Position );
				m_pModelGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, Point.Position() ) );					
			}
		}
	}

	for( int32 i = 0; i < m_pModel->GetTagCount(); i++ )
	{
		std::string Name = StringUpper( m_pModel->GetTagName( i ) );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( m_pModel->GetTagUserData( i )  );
			if( ParticleIndex != -1 )
			{
				m_TagParticleIndex.push_back( i );
				m_TagParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGlowManager.GetGlowType( m_pModel->GetTagUserData( i ) );
			if( GlowIndex != -1 )
			{
				m_pGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, D3DXVECTOR3( 0, -10000, 0 ) ) );					
				m_GlowIndex.push_back( i );
			}
		}
	}
} // CItem::FindParticles()

void CItem::EmitParticles( void )
{
	if( Category() == KCategoryEvent &&
		m_Open )
	{
		return;
	}
	if( IsContainer() && !CanInteract() )
	{
		return;
	}
	for( uint32 i = 0; i < m_ParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXVec3TransformCoord( &Position,  &m_ParticlePosition[i], &m_TransformationMatrix );
		D3DXMATRIX Transform = m_ParticleOrientation[i] * m_OrientationMatrix;
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pParticleManager.AddParticle( m_ParticleType[i], Transform );
	}

	for( int32 i = 0; i < m_TagParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXMATRIX Transform = m_pModel->GetTransformedTag( m_TagParticleIndex[i] );
		Transform *= m_TransformationMatrix;
		m_pParticleManager.AddParticle( m_TagParticleType[i], Transform );
	}


	UpdateGlows();

} // CItem::EmitParticles()

void CItem::UpdateGlows( void )
{
	if( Category() == KCategoryEvent &&
		m_Open )
	{
		return;
	}
	if( IsContainer() && !CanInteract() )
	{
		return;
	}

	for( uint32 i = 0; i < m_pGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position = m_pModel->GetTagPosition( m_GlowIndex[i] );
		D3DXVec3TransformCoord( &Position, &Position, &m_TransformationMatrix );
		m_pGlowList[i]->m_Position = Position;
	}
	for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXVec3TransformCoord( &Position,  &m_GlowPosition[i], &m_TransformationMatrix );
		D3DXMATRIX Transform = m_OrientationMatrix;
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pModelGlowList[i]->m_Position = Position;
	}

} // CItem::UpdateGlows()

void CItem::UpdateProximity( const D3DXVECTOR3& Position )	// position of nearby mobile object
{
	// if the object can perform no actions, we don't care about proximity
	if( !m_CanOpen && !m_CanClose )
	{
		return;
	}
	D3DXVECTOR3 Delta = Position - m_Position;
	Delta.y = 0;
	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance <= m_ActivationRadius )
	{
		m_ObjectNearby = kTrue;
	}
	else if ( Distance <= m_DeactivationRadius && m_Open )
	{
		m_ObjectNearby = kTrue;
	}

} // CItem::UpdateProximity()

void CItem::Update( float32 TimeElapsed,		// time elapsed in seconds
					CLevel& pLevel )			// a scene to collide with
{
	RemoveFromAvoidanceMap( pLevel );

	// if the creature fell through the world, pick some random
	// spot and reset it.
	if( m_Position.y < -100 ||
		VerifyFloat( m_Position.y, 1000 ) != m_Position.y )
	{
		SetPosition( pLevel, pLevel.RandomOpenPosition() );
		DropToGround( pLevel );
	}

	if( m_Dropping )
	{
		UpdateDrop( TimeElapsed );
	}


	if( Onscreen() )
	{
		UpdateAnimation( TimeElapsed );
		EmitParticles();
	}

	if( m_HasLightPool )
	{
		if( m_LightPoolType == KLightFlicker )
		{
			m_TorchLightFlicker += TimeElapsed * RandomBetween( 2, 5 );
			if( RandomBetween( 0, 10 ) > 7 )
			{
				m_TorchLightFlicker += RandomBetween( -.2f, .2f );
			}
		}
		else
		{
			m_TorchLightFlicker += TimeElapsed * 3.0f;
		}
		while( m_TorchLightFlicker >= 2.0f * (float32)KPI )
		{
			m_TorchLightFlicker -= 2.0f * (float32)KPI;
		}

		if( m_LightPoolType == KLightFlicker )
		{
			m_LightPoolScale = m_LightPoolRadius * 2.0f + fabs( sin( m_TorchLightFlicker ) ) * 10;
		}
		else
		{
			m_LightPoolScale = m_LightPoolRadius * 2.0f + fabs( sin( m_TorchLightFlicker ) ) * 15;
		}


		if( m_LightPoolType == KLightFlicker )
		{
			m_LightPoolAngle = (float32)KDeg2Rad * sin( sin( m_TorchLightFlicker ) * 10.0f );
		}
	}

	// if there is something in proximity
	if( m_ObjectNearby )
	{
		if( !m_Open && m_CanOpen )
		{
			Open( pLevel );
		}
	}
	else
	{
		if( m_Open && m_CanClose )
		{
			m_Open = kFalse;
			m_pModel->QueueBlendAnimation( "CLOSE", kFalse, 0.0f );
			m_pModel->QueueBlendAnimation( "IDLE", kTrue, 0.0f );
			m_pItemSounds->PlaySample( KSoundDeactivate,
									   m_Position,
									   m_BaseSoundVolume,
									   m_BaseSoundRadius );
		}
		else if( m_Open && !m_OpenComplete )
		{
			if( !m_pModel->AnimationPlaying( "OPEN" ) )
			{
				m_OpenComplete = kTrue;
			}
		}
	}

	m_ObjectNearby = kFalse;

	AddToAvoidanceMap( pLevel );

} // CItem::Update()

void CItem::UpdateAnimation( float32 TimeElapsed )
{
	if( !m_CanOpen &&
		 m_pModel->ActiveAnimations() == 1 )
	{
		if( rand() % 1000 < 5 ||
			!m_pModel->AnimationPlaying() )
		{
			BlendRandomIdleAnimation();
		}
	}
	m_pModel->UpdateAnimation( TimeElapsed );

} // CItem::UpdateAnimation()

void CItem::SetTrapType( ETrapType TrapType )		// type of trap
{
	m_TrapType = TrapType;
	switch( TrapType )
	{
	case KTrapExplosion :
		m_pTrapSounds->AddSample( 0,
								  "SOUNDS\\FX\\explode1.wav" );
		m_pTrapSounds->AddSample( 0,
								  "SOUNDS\\FX\\explode2.wav" );
		m_pTrapSounds->AddSample( 0,
								  "SOUNDS\\FX\\explode3.wav" );
		break;
	case KTrapPoison :
		m_pTrapSounds->AddSample( 0,
								  "SOUNDS\\FX\\poison1.wav" );
		m_pTrapSounds->AddSample( 0,
								  "SOUNDS\\FX\\poison2.wav" );
		break;
	case KTrapSword :
		m_pTrapSounds->AddSample( 0,
								  "SOUNDS\\FX\\cursedsword.wav" );
		break;
	}

} // CItem::SetTrapType()

void CItem::CompleteEvent( CLevel& pLevel,	// level to open items into
						   bool Success )	// successful completion?
{
	m_Open = kTrue;
	m_Operateable = kFalse;
	m_CanInteract = kFalse;
	m_HasLightPool = kFalse;
	if( Success )
	{
		m_pItemSounds->PlaySample( KSoundActivate,
								m_Position,
								m_BaseSoundVolume,
								m_BaseSoundRadius );
	}
	else
	{
		m_pItemSounds->PlaySample( KSoundDeactivate,
								m_Position,
								m_BaseSoundVolume,
								m_BaseSoundRadius );
	}

	if( m_pAmbientPositional != NULL )
	{
		m_pPositionalAudioManager.RemoveAudio( m_pAmbientPositional );
		m_pAmbientPositional = NULL;
	}

} // CItem::CompleteEvent()

void CItem::Open( CLevel& pLevel )			// a scene to collide with
{
	// once opened a container can no longer be interacted with
	if( IsContainer() )
	{
		m_CanInteract = kFalse;
		m_HasLightPool = kFalse;
	}
	if( Operateable() )
	{
		m_Operateable = kFalse;
	}
	m_Open = kTrue;
	m_pModel->QueueBlendAnimation( "OPEN", kFalse, 0.0f );
	m_pModel->QueueBlendAnimation( "OPENED", kTrue, 0.0f );
	m_pItemSounds->PlaySample( KSoundActivate,
							   m_Position,
							   m_BaseSoundVolume,
							   m_BaseSoundRadius );
	OpenToLevel( pLevel );

	bool Done( kFalse );
	CItem* pTargetItem = NULL;
	CEffect* pEffect = NULL;
	CSFXDescription* pSFX = NULL;
	CCharacterDescription* pCharDescription = NULL;
	if( Trapped() )
	{
		uint32 i;
		std::vector< CCharacter* > CharacterList;

		pLevel.AddEventText( NULL,
							 NULL,
							 Position() + D3DXVECTOR3( 0, 10, 0 ),
							 "TRAPPED!",
							 .75f,
							 kTrue,
							 1,
							 kTrue );

		m_pTrapSounds->PlaySample( 0, Position(), 255, 50 );

		switch ( m_TrapType )
		{
		case KTrapExplosion :
			 pSFX = new CSFXDescription( KSFXStationaryTarget,
										 "EFFECTS\\EXPLOSION\\explosion.ams",
										 -1,
										 kFalse,
										 0,
										 0,
										 kFalse,
										 kFalse );
			pLevel.AddSFX( *pSFX, 
						   NULL,
						   NULL,
						   Position(),
						   Position() );

			DELETE_SAFELY( pSFX );

			pLevel.FindCharactersInArea( Position(), 
										 8,
										 CharacterList );
			for( int32 i = 0; i < CharacterList.size(); i++ )
			{
				if( CharacterList[i]->Alive() &&
					CharacterList[i]->Alignment() != KNeutral )
				{
					if( CharacterList[i]->Blocked() )
					{
						pLevel.AddEventText( CharacterList[i],
											CharacterList[i],
											CharacterList[i]->Position() + D3DXVECTOR3( 0, 10, 0 ),
											LoadLocalizedString(993),
											.75f,
											kTrue,
											.9f );
					}
					else
					{
						CharacterList[i]->AddNewEffect( pLevel,
														new CEffect( KEffectHP,
																	kFalse,
																	KActivationPassive,
																	KDurationInstant,
																	-10,
																	0 ) );
					}
				}
			}

			Done = kFalse;
			while( !Done )
			{
				Done = kFalse;
				pTargetItem = pLevel.FindDestructibleItemWithinView( m_TransformationMatrix,
																	 360,
																	 8 );
				if( pTargetItem != NULL )
				{
					if( pTargetItem != this )
					{
						pTargetItem->Destroy( pLevel );
					}
				}
				else
				{
					Done = kTrue;
				}
			}


			break;
		case KTrapPoison :
			 pSFX = new CSFXDescription( KSFXStationaryTarget,
										 "EFFECTS\\POISON\\poison.ams",
										 -1,
										 kFalse,
										 0,
										 0,
										 kFalse,
										 kFalse );
			pLevel.AddSFX( *pSFX, 
						   NULL,
						   NULL,
						   Position(),
						   Position() );

			DELETE_SAFELY( pSFX );

			pLevel.FindCharactersInArea( Position(), 
										 8,
										 CharacterList );


			for( int32 i = 0; i < CharacterList.size(); i++ )
			{
				if( CharacterList[i]->Alive() &&
					CharacterList[i]->Alignment() != KNeutral  )
				{
					if( CharacterList[i]->Blocked() )
					{
						pLevel.AddEventText( CharacterList[i],
											CharacterList[i],
											CharacterList[i]->Position() + D3DXVECTOR3( 0, 10, 0 ),
											LoadLocalizedString(993),
											.75f,
											kTrue,
											.9f );
					}
					else
					{
						pEffect = new CEffect( KEffectHP,
											kFalse,
											KActivationUsage,
											20,
											-45,
											0 );
						pEffect->SetMessage( LoadLocalizedString(1125) );
						pEffect->SetName( "POISON" );
						// tie this to a poison spell, so that we will get a duration icon on any affected
						CSpellDescription* pSpell = pLevel.SpellManifest()->SpellDescription( "POISON" );
						if( pSpell != NULL )
						{
							pEffect->SetParentSpell( pSpell );
						}
						if( RandomIntegerBetween( 0, 100 ) < 50 )
						{
							CharacterList[i]->AddNewEffect( pLevel,
															pEffect );
						}
					}
				}
			}


			break;
		case KTrapSword :
			//pCharDescription = pLevel.CharacterManifest()->CharacterDescription( "Cursed Sword" );
			pCharDescription = pLevel.CharacterManifest()->CharacterDescription( LoadLocalizedString(1159) );

			if( pCharDescription != NULL )
			{
					CCharacter* pNewMonster = pLevel.AddMonster( *pCharDescription,
																 pLevel.RandomOpenPosition( Position(), 4 ) );
					pNewMonster->TweakToLevel( pLevel.LevelDepth() );
			}
			break;
		}
	}

	if( m_Destructible )
	{
		m_Collideable = kFalse;
	}
} // CItem::Open()

void CItem::UpdateDrop( float32 TimeElapsed )	// time elapsed in seconds
{
	m_DropPosition += TimeElapsed * 60.0f;
	if( m_DropPosition >= m_pDropPath->Length() )
	{
		m_DropPosition = m_pDropPath->Length();
		m_Dropping = kFalse;

		m_pPositionalAudioManager.PlaySample( m_pLandSound,
											  m_Position,
											  m_BaseSoundVolume,
											  m_BaseSoundRadius );
	}

	m_Position = m_pDropPath->GetSplinePositionAtDistance( m_DropPosition );
} // CItem::UpdateDrop()

void CItem::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						 CLightManager* pLightManager,			// light manager - NULL for no light manager
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						 CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	// no model, no render
	if( m_pModel == NULL )
	{
		return;
	}
		

	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

	D3DXMATRIX TransformedMatrix;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;
	TransformedMatrix = m_TransformationMatrix;

	SetOnscreen( kTrue );

	// set local ambient light, based upon distance from camera
	for( uint32 i = 0; i < m_pModel->GetMaterials(); i++ )
	{
		CMaterial*	Material = m_pModel->GetMaterial( i );
		if( Material != NULL )
		{
			Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
			Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
			Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
		}
	}


	D3DXVECTOR3 NormalLight( 0, -1, 0 );

	if( pLightManager != NULL )
	{
		pLightManager->UpdateLights( pD3DDevice, TransformedMatrix );

		D3DXMATRIX InverseTransformation;
		D3DXMatrixInverse( &InverseTransformation, NULL, &TransformedMatrix );

		D3DXVec3TransformNormal( &NormalLight, &pLightManager->LightDirection(), &InverseTransformation );

	}

	TransformedMatrix *= ViewportMatrix;

	if( !m_Open &&
		m_pOpenModel != NULL )
	{
		m_pOpenModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
	}
	else if( m_Open && m_OpenComplete && m_pCloseModel != NULL )
	{
		m_pCloseModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
	}
	else
	{
		m_pModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
	}

} // CItem::Render()

void CItem::RenderShadow( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							   const D3DXMATRIX& ViewportMatrix,	// viewport matrix
							   CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	if( !Onscreen() )
	{
		return;
	}
	// no model, no render
	if( m_pModel == NULL )
	{
		return;
	}

	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

		
	D3DXMATRIX TransformedMatrix;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;
	TransformedMatrix = m_TransformationMatrix;

	TransformedMatrix *= ViewportMatrix;

	if( !m_Open &&
		m_pOpenModel != NULL )
	{
		m_pOpenModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}
	else if( m_Open && m_OpenComplete && m_pCloseModel != NULL )
	{
		m_pCloseModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}
	else
	{
		m_pModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}

} // CItem::Render()


bool CItem::InFrustum( CFrustum& pFrustum )	// frustum to cull against					// frustum to cull with - NULL for none

{
	// no model, no render
	if( m_pModel == NULL )
	{
		return kFalse;
	}

	// if we're not in the frustum, don't render at all
	if( pFrustum.Intersects( *m_pCullingBounds ) == KFrustumOutside )
	{
		return kFalse;
	}

	return kTrue;
} // CItem::InFrustum()

void CItem::CalculateLocalLight( bool Torchlit,					//  is the level torchlit?
								 const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
								 float32 MaximumDistance )			// maximum distance from camera

{
	if( !Onscreen() )
	{
		return;
	}

	if( !Torchlit )
	{
		m_LocalLight = D3DXVECTOR3( 1, 1, 1 );
		return;
	}

	D3DXVECTOR3 Delta = FocalPoint - m_Position;
	Delta.y = 0;
	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance > MaximumDistance )
	{
		Distance = MaximumDistance;
	}

	// bring to the range of 0-1
	Distance /= MaximumDistance;

	// attenuate a little
	Distance *= Distance;

	// and invert
	Distance = 1.0f - Distance;

	m_LocalLight = D3DXVECTOR3( Distance, Distance, Distance );

} // CItem::CalculateLocalLight()

void CItem::DropToGround( CLevel& pLevel )			// a scene to collide with
{

	// weapons need to be laid on their sides
	if( ( Category() == KCategoryWeapon ||
		  Type() == KItemShield ) &&
		Type() != KItemCrossbow )
	{
		D3DXMatrixIdentity( &m_OrientationMatrix );
		D3DXMatrixRotationZ( &m_OrientationMatrix, (float32)KDeg2Rad * 90.0f );
	}

	D3DXVECTOR3 NextPosition( m_Position );

	m_Position.y = 100;
	NextPosition.y = -100;

	D3DXVECTOR3 ImpactPoint;
	D3DXVECTOR3 ImpactNormal;
	D3DXVECTOR3 FinalPosition;
	uint32 Material;
	D3DXVECTOR3 Color;
	m_OnGround = kFalse;


	if( pLevel.RayCollision( m_Position,
							 NextPosition,
							 ImpactPoint,
							 ImpactNormal,
							 Material,
							 Color,
							 NULL,
							 kFalse ) &&
							Material != KImpassableMaterialID )
	{
		m_OnGround = kTrue;

		// set our next position to the impact point;
		m_Position = ImpactPoint;

	}
	else
	{
		m_Position.x += .1f;
		NextPosition.x += .1f;
		m_Position.z += .15f;
		NextPosition.z += .15f;
		if( pLevel.RayCollision( m_Position,
								NextPosition,
								ImpactPoint,
								ImpactNormal,
								Material,
								Color,
								NULL,
								kFalse  ) &&
								Material != KImpassableMaterialID )
		{
			m_OnGround = kTrue;
			// set our next position to the impact point;
			m_Position = ImpactPoint;
		}
		else
		{
			if( m_Position.x < 10 &&
				m_Position.z < 10 )
			{
				// ok, this is definitely a bad location. Let's find a real spot for it
				m_Position = pLevel.RandomOpenPosition();
				DropToGround( pLevel );
				return;
			}
			m_OnGround = kTrue;
			m_Position.y = pLevel.FloorHeight( m_Position.x, m_Position.z );
		}
	}
	if( Category() != KCategoryEvent )
	{
		m_Position.y += .25f;
	}

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );

} // CItem::DropToGround()

void CItem::PlayTakeSound( void )
{
	m_pPositionalAudioManager.PlaySample( m_pTakeSound,
										  m_Position,
										  m_BaseSoundVolume,
										  m_BaseSoundRadius );
} // CItem::PlayTakeSound()

void CItem::PlayRemoveSound( void )
{
	m_pPositionalAudioManager.PlaySample( m_pLandSound,
										  m_Position,
										  m_BaseSoundVolume,
										  m_BaseSoundRadius );
} // CItem::PlayRemoveSound()

void CItem::SetTransformation( const D3DXMATRIX& Transformation )
{
	m_TransformationMatrix = Transformation;
	m_Position.x = Transformation._41;
	m_Position.x = Transformation._42;
	m_Position.x = Transformation._43;
	m_OrientationMatrix = Transformation;
	m_OrientationMatrix._41 = 0;
	m_OrientationMatrix._42 = 0;
	m_OrientationMatrix._43 = 0;

} // CItem::SetTransformation()

void CItem::SetPosition( CLevel& pLevel,				// level which has the avoidance map
						 const D3DXVECTOR3& Position )	// position to set
{
	bool WasOnAvoidanceMap = m_OnAvoidanceMap;

	RemoveFromAvoidanceMap( pLevel );
	m_Position = Position;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );

	if( WasOnAvoidanceMap )
	{
		AddToAvoidanceMap( pLevel );
	}

	if( m_pAmbientPositional != NULL )
	{
		m_pAmbientPositional->SetPosition( m_Position );
	}

} // CItem::SetPosition()

void CItem::Drop( CLevel& pLevel )	// level which has the avoidance map
{
	// re-rotate the object when dropped
	D3DXMATRIX RotationMatrix;

	float32 Angle( RandomBetween( 0, 360 ) );
	if( Category() == KCategoryEvent )
	{
		Angle = 0;
	}

	D3DXMatrixRotationY( &RotationMatrix, (float32)KDeg2Rad * Angle );

	m_OrientationMatrix = m_OrientationMatrix * RotationMatrix;

	if( !m_FliesWhenDropped )
	{
		D3DXVECTOR3 TargetPosition = pLevel.RandomOpenItemPosition( m_Position, 4 );
		TargetPosition.y = pLevel.FloorHeight( TargetPosition );
		TargetPosition.y += 1.0f;
		m_pPositionalAudioManager.PlaySample( m_pDropSound,
											  m_Position,
											  m_BaseSoundVolume,
											  m_BaseSoundRadius );
		SetPosition( pLevel, TargetPosition );
	}
	else
	{
		m_pPositionalAudioManager.PlaySample( m_pDropSound,
											  m_Position,
											  m_BaseSoundVolume,
											  m_BaseSoundRadius );

		int32 Tries( 0 );
		
		D3DXVECTOR3 TargetPosition = pLevel.RandomOpenItemPosition( m_Position, 6 );
		TargetPosition.y = pLevel.FloorHeight( TargetPosition ) + 1.0F;

		while( TargetPosition.y > 6 && Tries < 100 )
		{
			TargetPosition = pLevel.RandomOpenItemPosition( m_Position, 6 );
			TargetPosition.y = pLevel.FloorHeight( TargetPosition ) + 1.0F;
		}
		if( m_pDropPath == NULL )
		{
			m_pDropPath = new CPath( "DROP",
									  kFalse,
									  D3DXVECTOR3( 0, 0, 0 ) );
		}
		m_pDropPath->Clear();

		m_Dropping = kTrue;
		m_DropPosition = 0;

		D3DXVECTOR3 ApexPosition = TargetPosition + m_Position;
		ApexPosition *= .5f;
		ApexPosition.y += RandomBetween( 6, 10 );
	
		m_pDropPath->AddPoint( m_Position );
		m_pDropPath->AddPoint( ApexPosition );
		m_pDropPath->AddPoint( TargetPosition );

		if( m_pAmbientPositional != NULL )
		{
			m_pAmbientPositional->SetPosition( TargetPosition );
		}

	}
} // CItem::Drop()

void CItem::Use( CLevel& pLevel,		// level to add effects to
				 CCharacter* pUser,		// using character
				 CCharacter* pTarget )	// target character
{
	m_pPositionalAudioManager.PlaySample( m_pUseSound,
										  pUser->Position(),
										  m_BaseSoundVolume,
										  m_BaseSoundRadius );

	if( m_Uses != KUnlimitedUses )
	{
		m_Uses--;
	}

	for( uint32 i = 0; i < m_pEffects[KActivationUsage].size(); i++ )
	{
		pTarget->AddEffect( pLevel, m_pEffects[KActivationUsage][i] );
	}

	for( int32 i = 0; i < SFXItems(); i++ )
	{
		pLevel.AddSFX( *SFX( i ), 
					   pUser,
					   pTarget,
					   pUser->Position(),
					   pTarget->Position() );
	}

} // CItem::Use()

void CItem::Use( CLevel& pLevel,		// level to add effects to
				 CCharacter* pUser,		// using character
				 CItem* pTarget )		// target item
{
	m_pPositionalAudioManager.PlaySample( m_pUseSound,
										  pUser->Position(),
										  m_BaseSoundVolume,
										  m_BaseSoundRadius );



	for( uint32 i = 0; i < m_pEffects[KActivationUsage].size(); i++ )
	{
		switch( m_pEffects[KActivationUsage][i]->Type() )
		{
		case KEffectIdentify :
			pTarget->SetIdentified( kTrue );
			break;
		}
	}

	if( m_Uses != KUnlimitedUses )
	{
		m_Uses--;
	}

	for( int32 i = 0; i < SFXItems(); i++ )
	{
		pLevel.AddSFX( *SFX( i ), 
					   pUser,
					   pUser,
					   pUser->Position(),
					   pUser->Position() );
	}

} // CItem::Use()

bool CItem::CanUse( CLevel& pLevel,			// level to post effects to
					CCharacter* pUser,		// using character
					CCharacter* pTarget )	// target character
{
	bool CanUse( kTrue );
	for( uint32 i = 0; i < m_pEffects[KActivationUsage].size(); i++ )
	{
		switch( m_pEffects[KActivationUsage][i]->Type() )
		{
		case KEffectWriteSpell :
			CSpellDescription* pDescription;
			pDescription = pLevel.SpellManifest()->SpellDescription( m_pEffects[KActivationUsage][i]->Name() );
			if( pDescription != NULL )
			{
				if( pTarget != NULL &&
					pTarget->FreeSpellSlot( pDescription->Sphere()  ) == -1 )
				{
					CanUse = kFalse;
				}
				if( pTarget->HasSpell( pDescription ) )
				{
					CanUse = kFalse;
				}
			}
			break;
		}
	}
	return CanUse;
} // CItem::CanUse()

bool CItem::CanUseOnItem( CItem* pTarget ) // target item
{
	bool CanUse( kFalse );
	for( uint32 i = 0; i < m_pEffects[KActivationUsage].size(); i++ )
	{
		switch( m_pEffects[KActivationUsage][i]->Type() )
		{
		case KEffectIdentify :
			if( !pTarget->Identified() &&
				( pTarget->Effects( KActivationPassive ) > 0  ||
				pTarget->DamageBonuses() > 0 ) )
			{
				CanUse = kTrue;
			}
			break;
		}
	}
	return CanUse;
} // CItem::CanUseOnItem()

void CItem::RemoveFromAvoidanceMap( CLevel& pLevel )	// level which has the avoidance map
{
	if( m_OnAvoidanceMap )
	{
		float32 StepSize = pLevel.PassabilityCellWidth();
		for( float32 x = m_Position.x - StepSize * 1; x <= 
			 m_Position.x + StepSize * 1; x+= StepSize )
		{
			for( float32 y = m_Position.z - StepSize * 1; y <= 
				 m_Position.z + StepSize * 1; y+= StepSize )
			{
				pLevel.DecrementObjectPassability( D3DXVECTOR3( x, 0, y ) );
			}
		}
		m_OnAvoidanceMap = kFalse;
	}
} // CItem::RemoveFromAvoidanceMap()

void CItem::AddToAvoidanceMap( CLevel& pLevel )	// level which has the avoidance map
{
	if( InWorld() && m_Collideable && !m_OnAvoidanceMap )
	{
		float32 StepSize = pLevel.PassabilityCellWidth();
		for( float32 x = m_Position.x - StepSize * m_CollisionMaskRadius; x <= 
			 m_Position.x + StepSize * m_CollisionMaskRadius; x+= StepSize )
		{
			for( float32 y = m_Position.z - StepSize * m_CollisionMaskRadius; y <= 
				 m_Position.z + StepSize * m_CollisionMaskRadius ; y+= StepSize )
			{
				pLevel.IncrementObjectPassability( D3DXVECTOR3( x, 0, y ) );
			}
		}
		m_OnAvoidanceMap = kTrue;
	}
} // CItem::AddToAvoidanceMap()

CItem* CItem::AddItem( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					   CItemDescription& pDescription )		// item description
{

	CItem* pItem =  new CItem( pD3DDevice,
							   m_pRefManager,
							   m_pSettings,
							   m_pParticleManager,
							   m_pGlowManager,
							   m_pPositionalAudioManager,
							   pDescription );

	m_pItems.push_back( pItem );

	return pItem;
} // CItem::AddItem()


CItem* CItem::AddGold( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					   uint32 Value )					// amount of gold
{

	CItem* pItem =  new CItemGold( pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   m_pParticleManager,
								   m_pGlowManager,
								   m_pPositionalAudioManager,
								   Value );

	m_pItems.push_back( pItem );

	return pItem;
} // CItem::AddGold()

CItem* CItem::AddItem( CItem* pItem )	// item to add
{

	m_pItems.push_back( pItem );
	
	return pItem;
} // CItem::AddItem()

void CItem::RemoveItem( CItem* pItem )	// item to add
{

	for( int32 i = 0; i < (int32)m_pItems.size(); i++ )
	{
		if( m_pItems[i] == pItem )
		{
			m_pItems[i] = m_pItems[m_pItems.size() - 1];
			m_pItems.pop_back();
			return;
		}
	}
} // CItem::RemoveItem()

void CItem::OpenToLevel( CLevel& pLevel )	// level to open items into
{
	for( uint32 i = 0; i < m_pItems.size(); i++ )
	{
		pLevel.AddItem( m_pItems[i], m_Position );
		m_pItems[i]->Drop( pLevel );
	}
	m_pItems.clear();
} // CItem::OpenToLevel()

void CItem::Destroy( CLevel& pLevel )	// level to open items into
{
	Open( pLevel );
	m_Collideable = kFalse;
	RemoveFromAvoidanceMap( pLevel );
} // CItem::Destroy()

void CItem::SetInWorld( bool State )	// in world state
{
	m_InWorld = State;
	if( m_InWorld )
	{
		if( m_pAmbientPositional == NULL &&
			m_AmbientSoundPath.length() > 0 )
		{
			m_pAmbientPositional = m_pPositionalAudioManager.AddAudio( m_Position,
																	   m_AmbientSoundPath,
																	   m_BaseSoundVolume,
																	   m_BaseSoundRadius );
		}
	}
	else
	{
		for( uint32 i = 0; i < m_pGlowList.size(); i++ )
		{
			m_pGlowList[i]->m_Position = D3DXVECTOR3( -1000, -1000, -1000 );
		}
		for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
		{
			m_pModelGlowList[i]->m_Position = D3DXVECTOR3( -1000, -1000, -1000 );
		}

		if( m_pAmbientPositional != NULL )
		{
			m_pPositionalAudioManager.RemoveAudio( m_pAmbientPositional );
			m_pAmbientPositional = NULL;
		}
	}

} // CItem::SetInWorld()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CItem::RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
						  const D3DXVECTOR3& End,		// end of collision ray
						  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
						  D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	if( !Collideable() ||
		m_pCollisionModel == NULL )
	{
		return kFalse;
	}

	D3DXVECTOR3 MinBounds( Start );
	D3DXVECTOR3 MaxBounds( Start );
	ExpandBounds( MinBounds, MaxBounds, End );

	if( !BoundsIntersect( m_pCullingBounds->GetMinBounds(),
						  m_pCullingBounds->GetMaxBounds(),
						  MinBounds, MaxBounds ) )
	{
		return kFalse;
	}

	// first we have to bring the collision ray into local space
	D3DXMATRIX InverseTransformation;
	D3DXMatrixInverse( &InverseTransformation, NULL, &m_OrientationMatrix );
	D3DXVECTOR3 LocalStart;
	D3DXVECTOR3 LocalEnd;
	LocalStart = Start - m_Position;
	LocalEnd = End - m_Position;
	D3DXVec3TransformNormal( &LocalStart, &LocalStart, &InverseTransformation );
	D3DXVec3TransformNormal( &LocalEnd, &LocalEnd, &InverseTransformation );

	if( m_pCollisionModel->RayCollision( LocalStart,
										 LocalEnd,
										 ImpactPoint,
										 ImpactNormal ) )
	{
		// for a successful collision, we have to send the results back to world space
		D3DXVec3TransformNormal( &ImpactPoint, &ImpactPoint, &m_OrientationMatrix );
		D3DXVec3TransformNormal( &ImpactNormal, &ImpactNormal, &m_OrientationMatrix );
		ImpactPoint += m_Position;
		return kTrue;
	}
	return kFalse;
} // CItem::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CItem::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							 const D3DXVECTOR3& End,		// end of collision ray
							 float32 Radius,				// radius of sphere to check along collision ray
							 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							 D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	if( !Collideable() ||
		m_pCollisionModel == NULL )
	{
		return kFalse;
	}

	D3DXVECTOR3 MinBounds( Start );
	D3DXVECTOR3 MaxBounds( Start );
	ExpandBounds( MinBounds, MaxBounds, End );
	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;

	if( !BoundsIntersect( m_pCullingBounds->GetMinBounds(),
						  m_pCullingBounds->GetMaxBounds(),
						  MinBounds, MaxBounds ) )
	{
		return kFalse;
	}

	// first we have to bring the collision ray into local space
	D3DXMATRIX InverseTransformation;
	D3DXMatrixInverse( &InverseTransformation, NULL, &m_OrientationMatrix );
	D3DXVECTOR3 LocalStart;
	D3DXVECTOR3 LocalEnd;
	LocalStart = Start - m_Position;
	LocalEnd = End - m_Position;
	D3DXVec3TransformNormal( &LocalStart, &LocalStart, &InverseTransformation );
	D3DXVec3TransformNormal( &LocalEnd, &LocalEnd, &InverseTransformation );

	if( m_pCollisionModel->SphereCollision( LocalStart,
											LocalEnd,
											Radius,
											FinalPosition,
											ImpactPoint,
											ImpactNormal ) )
	{
		// for a successful collision, we have to send the results back to world space
		D3DXVec3TransformNormal( &ImpactPoint, &ImpactPoint, &m_OrientationMatrix );
		D3DXVec3TransformNormal( &FinalPosition, &FinalPosition, &m_OrientationMatrix );
		D3DXVec3TransformNormal( &ImpactNormal, &ImpactNormal, &m_OrientationMatrix );
		ImpactPoint += m_Position;
		FinalPosition += m_Position;
		return kTrue;
	}
	return kFalse;
} // CItem::SphereCollision()
