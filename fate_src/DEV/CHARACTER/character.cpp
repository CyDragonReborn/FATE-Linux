/*
	Character.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

	Change History

	Travis Baldree
	4/16/2004	-	added many methods for manipulating the character (moving, falling, etc.)
					expanded samples of motion blending for active character animation

	4/19/2004	-	gravity and physics were all screwed up, had forgotten to multiply velocity
					by timeslice. Fixed now


*/

#include <assert.h>
#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/cullingbounds.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"
#include "../MEDIA/cubemap.h"

#include "../PARTICLES/particlemanager.h"
#include "../GLOWS/glowmanager.h"
#include "../GLOWS/glowsystem.h"

#include "../POSITIONALAUDIO/positionalaudiomanager.h"

#include "../MODEL/model.h"

#include "../OBJECTS/path.h"

#include "../LEVEL/level.h"

#include "../GAMECLIENT/gamerules.h"
#include "../GAMECLIENT/gameui.h"
#include "../GAMECLIENT/tips.h"

#include "../ITEM/item.h"
#include "../ITEM/itemblank.h"
#include "../ITEM/spelltemplate.h"
#include "../ITEM/effect.h"

#include "../UI/text.h"
#include "../UI/sprite.h"

#include "../GAMEOBJECTS/genericmodel.h"
#include "../GAMEOBJECTS/lightmanager.h"
#include "../GAMEOBJECTS/soundbank.h"

#include "../LEVEL/level.h"
#include "../LEVEL/LevelHistory.h"

#include "../PATHFINDING/astar.h"

#include "../NAMES/namegenerator.h"

#include "../GAMECLIENT/quest.h"

#include "character.h"
#include "charactertemplate.h"
#include "inventory.h"

#include "../GAMECLIENT/demorules.h" //marsh

#include "../UTILITIES/StatsAndAchievements.h"

CCharacter::CCharacter( CRefManager& pRefManager,						// pointer to the reference manager, for tracking submaterials
						CSettings& pSettings,							// pointer to the settings manager, for keeping track of card/machine capabilities
						CSpellTemplate& pSpellManifest,							// spell template for casting
						CParticleManager& pParticleManager,						// particle manager
						CGlowManager& pGlowManager,						// particle manager
						CPositionalAudioManager& pPositionalAudioManager ) :	// positional audio manager
																				m_pRefManager( pRefManager ),
																				m_pSettings( pSettings ),
																				m_pParticleManager( pParticleManager ),
																				m_pGlowManager( pGlowManager ),
																				m_pSpellManifest( pSpellManifest ),
																				m_pPositionalAudioManager( pPositionalAudioManager ),
																				m_Position( 0, 0, 0 ),
																				m_Velocity( 0, 0, 0 ),
																				m_KnockbackVelocity( 0, 0, 0 ),
																				m_GroundColor( 1, 1, 1 ),
																				m_LocalLight( 1, 1, 1 ),
																				m_Up( 0, 1, 0 ),
																				m_Forward( 0, 0, 1 ),
																				m_Right( 1, 0, 0 ),
																				m_BaseLevel( 1 ),
																				m_WalkingSpeed( 15 ),
																				m_RunningSpeed( 22 ),
																				m_TurnRate( 10 ),
																				m_ViewRadius( 40 ),
																				m_MotionRadius( 10 ),
																				m_AttentionRadius( 48 ),
																				m_FollowRadius( 72 ),
																				m_CollisionRadius( 1.5f ),
																				m_TimeSinceAvoidance( 10 ),
																				m_TimeSinceIdleSound( 10 ),
																				m_TimeSinceGreetSound( 10 ),
																				m_TimeSinceUniqueIdle( 0 ),
																				m_ReachBonus( 0 ),
																				m_AttackTimer( 0 ),
																				m_TownTimer( 0 ),
																				m_CastTimer( 0 ),
																				m_TimeSincePath( 10 ),
																				m_SoundRadius( 32 ),
																				m_HeadIndex( -1 ),
																				m_HairIndex( -1 ),
																				m_HelpChannel( -1 ),
																				m_Gender( KMale ),
																				m_pActiveSpell( NULL ),
																				m_OnGround( kFalse ),
																				m_Pathing( kFalse ),
																				m_MouseOver( kFalse ),
																				m_HasOpenPortal( kFalse ),
																				m_DisplayedAsItem( kFalse ),
																				m_PlayedStaminaLow( kFalse ),
																				m_pCharacterName( NULL ),
																				m_pCharacterBackdrop( NULL ),
																				m_pCastingSpell( NULL ),
																				m_pCastingItemTarget( NULL ),
																				m_pCastingCharacterTarget( NULL ),
																				m_pInventory( NULL ),
																				m_pAttackSounds( NULL ),
																				m_pStrikeSounds( NULL ),
																				m_pMissSounds( NULL ),
																				m_pCharacterSounds( NULL ),
																				m_pCollisionModel( NULL ),
																				m_pModel( NULL ),
																				m_pUpperBodyModel( NULL ),
																				m_pGlovesModel( NULL ),
																				m_pArmorModel( NULL ),
																				m_pBootsModel( NULL ),
																				m_pHeadModel( NULL ),
																				m_pHairModel( NULL ),
																				m_pCullingBounds( NULL ),
																				m_pFishingPole( NULL ),
																				m_pFish( NULL ),
																				m_pPath( NULL ),
																				m_pPathNodeModel( NULL ),
																				m_pTargetCharacter( NULL ),
																				m_pMasterCharacter( NULL ),
																				m_pTargetItem( NULL ),
																				m_pActiveWeapon( NULL ),
																				m_pCurrentAttack( NULL ),
																				m_pAStar( NULL ),
																				m_pOverrideMaterial( NULL ),
																				m_pLightPoolMaterial( NULL ),
																				m_pMasterQuest( NULL ),
																				m_Rank( KRankNormal ),
																				m_CanMatchSpeed( kFalse ),
																				m_IsMimic( kFalse ),
																				m_Onscreen( kFalse ),
																				m_IsTurning( kFalse ),
																				m_HoldPosition( kFalse ),
																				m_Collideable( kFalse ),
																				m_OnAvoidanceMap( kFalse ),
																				m_DeletionRequested( kFalse ),
																				m_Running( kFalse ),
																				m_CanRun( kFalse ),
																				m_InActiveRange( kTrue ),
																				m_ReachedStrikePoint( kFalse ),
																				m_FishBiting( kFalse ),
																				m_ReachedCastPoint( kFalse ),
																				m_IsMerchant( kFalse ),
																				m_IsQuestGiver( kFalse ),
																				m_LastAttackRight( kFalse ),
																				m_Invincible( kFalse ),
																				m_Poisoned( kFalse ),
																				m_BreathActive( kFalse ),
																				m_HasLightPool( kFalse ),
																				m_OnImpassableTerrain( kFalse ),
																				m_MinionsCollideable( kFalse ),
																				m_IsUnique( kFalse ),
																				m_AllowDropEquipment( kTrue ),
																				m_AllowDropInventory( kTrue ),
																				m_CaughtFish( kFalse ),
																				m_AIState( KAIInactive ),
																				m_Difficulty( KDifficultyAdventurer ),
																				m_Alignment( KGood ),
																				m_Name( "UNKNOWN" ),
																				m_AncestorName( "UNKNOWN" ),
																				m_RepathPauseTime( 2 ),
																				m_ExistenceTime( 0 ),
																				m_LastMovementSpeed( 1 ),
																				m_Bravery( 1 ),
																				m_Scale( 1 ),
																				m_HP( 16 ),
																				m_MaxHP( 16 ),
																				m_Strength( 16 ),
																				m_Dexterity( 16 ),
																				m_Vitality( 16 ),
																				m_Magic( 16 ),
																				m_Level( 1 ),
																				m_Experience( 0 ),
																				m_ExperienceAward( 0 ),
																				m_Fame( 0 ),
																				m_FameRank( 1 ),
																				m_FameAward( 0 ),
																				m_UnusedStatPoints( 0 ),
																				m_UnusedSkillPoints( 0 ),
																				m_ToHitBonus( 20 ),
																				m_MaxStamina( 50 ),
																				m_Stamina( 50 ),
																				m_Mana( 0 ),
																				m_MaxMana( 0 ),
																				m_NaturalArmor( 0 ),
																				m_VerticalOffset( 0 ),
																				m_LifeDuration( 0 ),
																				m_HelpTimer( 0 ),
																				m_TransformationDuration( 0 ),
																				m_DeathAnimationIndex( 0 ),
																				m_TorchLightFlicker( 0 ),
																				m_LightPoolAngle( 0 ),
																				m_Lineage( 0 ),
																				m_Tactics( KTacticsFighter ),
																				m_TemplateName( "" ),
																				m_OriginalTemplateName( "" ),
																				m_MerchantType( KMerchantTraveling ),
																				m_PortalPosition( 0, 0, 0 ),
																				m_DisplayPosition( 0, 0, 0 ),
																				m_TargetPosition( 0, 0, 0 ),
																				m_FinalTargetPosition( 0, 0, 0 ),
																				m_CastingTargetPosition( 0, 0, 0 ),
																				m_MinCollisionBounds( 0, 0, 0 ),
																				m_MaxCollisionBounds( 0, 0, 0 ),
																				m_OriginalAlignment( KNeutral ),
																				m_PathNodeAlignment( KNeutral ),
																				m_BloodParticle( 0 ),
																				m_StrikeParticle( 0 ),
																				m_DustParticle( 0 ),
																				m_LastDungeonLevel( 0 ),
																				m_OriginalStrength( 0 ),
																				m_OriginalDexterity( 0 ),
																				m_OriginalVitality( 0 ),
																				m_OriginalMagic( 0 ),
																				m_MinHP( 0 ),
																				m_OriginalToHitBonus( 0 ),
																				m_OriginalNaturalArmor( 0 ),
																				m_PortalDepth( 0 ),
																				m_LightPoolRadius( 0 ),
																				m_LightPoolScale( 0 ),
																				m_TimeSinceBite( 0 ),
																				m_BiteSpeed( 0 ),
																				m_LightPoolType( KLightFlicker ),
																				m_ActiveHelpSample( KSoundIdle ),
																				m_MimicName( "" ),
																				m_Family( "" ),
																				m_FishCaughtList ( "" ),
																				m_GemsCollectedList ( "" ),
																				m_gemTypesCollected(0),
																				m_fishTypesCollected(0),
																				m_disabledAchievements(kFalse)
{
	ZeroMemory( &m_D3DSilhouetteMaterial, sizeof( D3DMATERIAL8 ) );
	ZeroMemory( &m_OrientationMatrix, sizeof( m_OrientationMatrix ) );
	ZeroMemory( &m_DisplayMatrix, sizeof( m_DisplayMatrix ) );

	m_D3DSilhouetteMaterial.Diffuse.r	= 0.0f;					
	m_D3DSilhouetteMaterial.Diffuse.g	= 0.0f;
	m_D3DSilhouetteMaterial.Diffuse.b	= 0.0f;
	m_D3DSilhouetteMaterial.Diffuse.a	= 0.0f;

	m_D3DSilhouetteMaterial.Specular.r	= 0.0F;
	m_D3DSilhouetteMaterial.Specular.g	= 0.0F;
	m_D3DSilhouetteMaterial.Specular.b	= 0.0F;

	m_D3DSilhouetteMaterial.Power		= 0.0f;

	m_D3DSilhouetteMaterial.Emissive.r	= 1.0F;
	m_D3DSilhouetteMaterial.Emissive.g	= 1.0F;
	m_D3DSilhouetteMaterial.Emissive.b	= 1.0F;

	m_D3DSilhouetteMaterial.Ambient.r	= 0.0f;						
	m_D3DSilhouetteMaterial.Ambient.g	= 0.0f;
	m_D3DSilhouetteMaterial.Ambient.b	= 0.0f;
	m_D3DSilhouetteMaterial.Ambient.a	= 0.0f;


	for( uint32 i = 0; i < KSkills; i++ )
	{
		m_SkillPoints[i] = 0;
	}

	for( int32 i = 0; i < KJournalStatistics; i++ )
	{
		m_JournalStats[i] = 0;
	}

	if( m_pSettings.GetSettings( KSetRedBlood ) )
	{
		m_BloodParticle = m_pParticleManager.GetParticleType( "BLOOD" );
	}
	else
	{
		m_BloodParticle = m_pParticleManager.GetParticleType( "STARBLOOD" );
	}
	m_StrikeParticle = m_pParticleManager.GetParticleType( "HITSPARK" );
	m_DustParticle = m_pParticleManager.GetParticleType( "DUST" );

	for( int32 i = 0; i < KDamageTypes; i++ )
	{
		if( KDamageParticles[i].length() > 0 )
		{
			m_DamageParticle[i] = m_pParticleManager.GetParticleType( KDamageParticles[i] );
		}
		else
		{
			m_DamageParticle[i] = -1;
		}
		if( KDamageParticlesSecondary[i].length() > 0 )
		{
			m_DamageParticleSecondary[i] = m_pParticleManager.GetParticleType( KDamageParticlesSecondary[i] );
		}
		else
		{
			m_DamageParticleSecondary[i] = -1;
		}
	}

	m_pInventory = new CInventory( *this,
								   KMaxInventorySlots );
	D3DXMatrixRotationY( &m_OrientationMatrix, (float32)KDeg2Rad * 180.0f );

	m_pPath = new CPath( "", kFalse, D3DXVECTOR3( 0, 0, 0 ) );

	m_pAttackSounds = new CSoundBank( m_pRefManager,
									  m_pSettings,
									  m_pPositionalAudioManager );

	m_pStrikeSounds = new CSoundBank( m_pRefManager,
									  m_pSettings,
									  m_pPositionalAudioManager );

	m_pMissSounds = new CSoundBank( m_pRefManager,
									m_pSettings,
									m_pPositionalAudioManager );

	m_pCharacterSounds = new CSoundBank( m_pRefManager,
									     m_pSettings,
									     m_pPositionalAudioManager );

	m_pCharacterSounds->AddSample( KSoundDispel,
								   "SOUNDS\\SPELLS\\fail.wav" );

	m_pCharacterSounds->AddSample( KSoundBlock,
								   "SOUNDS\\BATTLE\\block.wav" );

	m_pCharacterSounds->AddSample( KSoundCritical,
								   "SOUNDS\\BATTLE\\critical.wav" );

	m_pCharacterSounds->AddSample( KSoundGold,
								   "SOUNDS\\ITEMS\\gold.wav" );

	m_pCharacterSounds->AddSample( KSoundLevelUp,
								   "SOUNDS\\FX\\levelup.wav" );
	ClearSpellbook();

	CalculateEffectValues();

	// clear default resistances
	for( int32 i = 0; i < KDamageTypes; i++ )
	{
		m_DamageResistance[i] = 0;
	}
	// default EVERYTHING is 100% resistant to undead damage
	m_DamageResistance[KDamageUndead] = 100;
} // CCharacter::CCharacter()

CCharacter::~CCharacter( void )
{

	ClearQuests();
	ClearMasterQuest();

	for( uint32 i = 0; i < m_pDiscoveryHistory.size(); i++ )
	{
		DELETE_SAFELY( m_pDiscoveryHistory[i] );
	}
	m_pDiscoveryHistory.clear();

	ClearEffects();

	ClearNaturalEffects();

	DELETE_SAFELY( m_pCharacterName );
	DELETE_SAFELY( m_pCharacterBackdrop );
	DELETE_SAFELY( m_pPath );
	DELETE_SAFELY( m_pInventory );
	DELETE_SAFELY( m_pAttackSounds );
	DELETE_SAFELY( m_pStrikeSounds );
	DELETE_SAFELY( m_pMissSounds );
	DELETE_SAFELY( m_pCharacterSounds );

	for( int32 i = 0; i < m_UnarmedAttacks.size(); i++ )
	{
		DELETE_SAFELY( m_UnarmedAttacks[i] );
	}
	m_UnarmedAttacks.clear();

	DELETE_SAFELY( m_pAStar );

	m_pRefManager.RemoveMaterial( m_pLightPoolMaterial );

	m_pRefManager.RemoveMaterial( m_pOverrideMaterial );

	m_pRefManager.RemoveModel( m_pCollisionModel );
	m_pRefManager.RemoveModel( m_pPathNodeModel );

	DELETE_SAFELY( m_pModel );
	DELETE_SAFELY( m_pFishingPole );
	DELETE_SAFELY( m_pFish );
	DELETE_SAFELY( m_pUpperBodyModel );
	DELETE_SAFELY( m_pGlovesModel );
	DELETE_SAFELY( m_pArmorModel );
	DELETE_SAFELY( m_pBootsModel );
	DELETE_SAFELY( m_pHeadModel );
	DELETE_SAFELY( m_pHairModel );

	DELETE_SAFELY( m_pCullingBounds );

	ClearParticles();

} // CCharacter::~CCharacter()

const std::string& CCharacter::Greeting( void )
{
	
	return m_GreetMessages[RandomIntegerBetween( 0, m_GreetMessages.size() - 1 )];
} // CCharacter::Greeting()

void CCharacter::CreateNameForDisplay( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device)
								  CFontMetric* pFontMetric,				// font metric
								  CMaterial* pFontMaterial )			// font material
{
	DELETE_SAFELY( m_pCharacterName );
	DELETE_SAFELY( m_pCharacterBackdrop );

	m_pCharacterName = new CText( pD3DDevice,
								 pFontMaterial,
								 pFontMetric,
								 MimicName(),
								 512,
								 48,
								 .8f,
								 kFalse,
								 KJustifyCenter );



	// first resize our description shadow to fit
	float32 Width = m_pCharacterName->MaxBounds().x -
					m_pCharacterName->MinBounds().x;
	float32 Height = m_pCharacterName->MaxBounds().y -
					 m_pCharacterName->MinBounds().y;

	Width *= .5f;
	Height *= .5f;
	Width += 6;
	Height += 6;

	m_pCharacterBackdrop = new CSprite( pD3DDevice,
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

} // CCharacter::CreateNameForDisplay()

void CCharacter::IncrementStat( EJournalStatistic Stat,		// stat to increment
							    int32 Value )				// value to increment
{
	if( IsPlayer() )
	{
		m_JournalStats[Stat] += Value;
		CStatsAndAchievements::GetInstance()->CheckStats();
	}
} // CCharacter::IncrementStat()

void CCharacter::ClearCurrentAttack( void )
{
	m_pActiveWeapon = NULL;
	m_pCurrentAttack = NULL;
	m_AttackTimer = 0;
	if( AIState() == KAIAttack )
	{
		SetAIState( KAIInactive );
	}
} // CCharacter::ClearCurrentAttack( void )

void CCharacter::ClearState( void )
{
	SetAIState( KAIInactive );
	SetTarget( NULL );
	SetTargetItem( NULL );

	m_pCastingCharacterTarget = NULL;
	m_pCastingItemTarget = NULL;

	m_pActiveWeapon = NULL;
	m_pCurrentAttack = NULL;

	m_CastTimer = 0;
	m_AttackTimer = 0;
	m_TimeSinceAvoidance = 10;
	m_TimeSincePath = 10;

	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)m_pEffects[i].size(); j++ )
		{
			if( m_pEffects[i][j]->Owner() != this )
			{
					
				m_pEffects[i][j]->SetOwner( NULL );
			}
		}
	}
	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)m_pNaturalEffects[i].size(); j++ )
		{
			if( m_pNaturalEffects[i][j]->Owner() != this )
			{
					
				m_pNaturalEffects[i][j]->SetOwner( NULL );
			}
		}
	}
} // CCharacter::ClearState()

void CCharacter::ClearEffects( void )
{
	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
		{
			DELETE_SAFELY( m_pEffects[i][j] );
		}
		m_pEffects[i].clear();
	}

} // CCharacter::ClearEffects()

void CCharacter::ClearNaturalEffects( void )
{
	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pNaturalEffects[i].size(); j++ )
		{
			DELETE_SAFELY( m_pNaturalEffects[i][j] );
		}
		m_pNaturalEffects[i].clear();
	}

} // CCharacter::ClearEffects()

void CCharacter::ClearSpellbook( void )
{
	for( uint32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			m_KnownSpells[i][j] = NULL;
		}
	}
} // CCharacter::ClearSpellbook()

void CCharacter::UnLearnSpell( EMagicSphere Sphere,					// sphere of magic
							   uint32 Index )						// slot to unlearn
{
	if( Index >= KMaximumSpellsPerSphere )
	{
		return;
	}
	if( m_pActiveSpell != NULL &&
		m_pActiveSpell == m_KnownSpells[Sphere][Index] )
	{
		m_KnownSpells[Sphere][Index] = NULL;

		m_pActiveSpell = NULL;
		for( uint32 i = 0; i < KMagicSpheres; i++ )
		{
			for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				if( m_KnownSpells[i][j] != NULL &&
					m_pActiveSpell == NULL )
				{
					m_pActiveSpell = m_KnownSpells[i][j];
					break;
				}
			}
		}
	}
	m_KnownSpells[Sphere][Index] = NULL;

} // CCharacter::UnLearnSpell()

void CCharacter::SetActiveSpell( CSpellDescription* pSpellDescription )	// spell to set as active
{
	if( pSpellDescription == NULL )
	{
		m_pActiveSpell = NULL;
		return;
	}
	if( HasSpell( pSpellDescription ) )
	{
		m_pActiveSpell = pSpellDescription;
	}
} // CCharacter::SetActiveSpell()

void CCharacter::SetActiveSpell( const std::string& Name )	// spell to set as active
{
	if( Name.length() == 0 )
	{
		return;
	}
	for( uint32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( m_KnownSpells[i][j] != NULL &&
				m_KnownSpells[i][j]->Name() == Name )
			{
				SetActiveSpell( m_KnownSpells[i][j] );
			}
		}
	}
} // CCharacter::SetActiveSpell()

void CCharacter::EnsureSpellSelection( void )
{
	if( m_pActiveSpell == NULL )
	{
		for( uint32 i = 0; i < KMagicSpheres; i++ )
		{
			for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				if( m_KnownSpells[i][j] != NULL &&
					m_KnownSpells[i][j]->RequirementsMet( *this ) )
				{
					m_pActiveSpell = m_KnownSpells[i][j];
					return;
				}
			}
		}
	}
} // CCharacter::EnsureSpellSelection()

void CCharacter::LearnSpell( CSpellDescription* pSpellDescription,	// spell to learn
							 uint32 Slot )							// slot within sphere
{
	if( !HasSpell( pSpellDescription ) )
	{
		m_KnownSpells[pSpellDescription->Sphere()][Slot] = pSpellDescription;
		if( m_pActiveSpell == NULL )
		{
			m_pActiveSpell = pSpellDescription;
		}
	}
} // CCharacter::LearnSpell()

void CCharacter::CastSpell( CLevel& pLevel,							// level to send effects to
							CSpellDescription* pSpellDescription,	// spell to learn
						    CItem* pItem,							// item to cast on
						    CCharacter* pTargetCharacter,			// character targeted
							const D3DXVECTOR3& Position )			// worldspace target position
{
	m_CastingTargetPosition = Position;
	m_pCastingSpell = pSpellDescription;
	m_pCastingItemTarget = pItem;
	m_pCastingCharacterTarget = pTargetCharacter;

	if( !pSpellDescription->RequirementsMet( *this ) )
	{
		return;
	}
	if( pSpellDescription->CastTime() == 0 )
	{
		CompleteSpell( pLevel );
	}
	else
	{
		pSpellDescription->Chant( pLevel, this, pTargetCharacter );
		Cast();
		SetAIState( KAICast );
	}
} // CCharacter::CastSpell()

void CCharacter::CompleteSpell( CLevel& pLevel )	// level to send effects to
{
	uint32 i( 0 );
	if( !Alive() )
	{
		return;
	}
	EAlignment TargetAlignment = m_pCastingSpell->TargetAlignment();
	if( Alignment() == KEvil )
	{
		if( TargetAlignment == KGood )
		{
			TargetAlignment = KEvil;
		}
		else if( TargetAlignment == KEvil )
		{
			TargetAlignment = KGood;
		}
	}

	std::vector< CCharacter* > CharacterList;

	D3DXVECTOR3 TargetPosition;
	// only do it with sufficient mana
	if( Mana() >= (float32)m_pCastingSpell->ManaCost() )
	{
		bool Success( kFalse );
		switch( m_pCastingSpell->Target() )
		{
		case KTargetArea :
		case KTargetProjectile :
			if( m_pCastingCharacterTarget != NULL &&
				m_pCastingCharacterTarget->Alive() &&
				( m_pCastingCharacterTarget->Alignment() == TargetAlignment ||
				  TargetAlignment == KAllAlignments ) )
			{
				if( m_pCastingSpell->CanCastOnTarget( this, m_pCastingCharacterTarget ) )
				{
					m_pCastingSpell->Cast( pLevel, this, m_pCastingCharacterTarget );
				}
				else
				{
					pLevel.AddEventText( this,
										m_pCastingCharacterTarget,
										m_pCastingCharacterTarget->Position() + D3DXVECTOR3( 0, 9, 0 ),
										LoadLocalizedString(995),
										.75f,
										kFalse,
										1 );
				}
			}
			else
			{
				TargetPosition = m_CastingTargetPosition;//Position() + Forward() * 100;
				m_pCastingSpell->Cast( pLevel, this, TargetPosition );
			}
			Success = kTrue;
			break;
		case KTargetCharacter :
			if( m_pCastingCharacterTarget != NULL &&
				m_pCastingCharacterTarget->Alive() &&
				( m_pCastingCharacterTarget->Alignment() == TargetAlignment ||
				  TargetAlignment == KAllAlignments ) )
			{
				if( m_pCastingSpell->CanCastOnTarget( this, m_pCastingCharacterTarget ) )
				{
					m_pCastingSpell->Cast( pLevel, this, m_pCastingCharacterTarget );
					Success = kTrue;
				}
				else
				{
					pLevel.AddEventText( this,
										m_pCastingCharacterTarget,
										m_pCastingCharacterTarget->Position() + D3DXVECTOR3( 0, 9, 0 ),
										LoadLocalizedString(995),
										.75f,
										kFalse,
										1 );
				}
				if( m_pCastingCharacterTarget->Alignment() != Alignment() )
				{
					m_pCastingCharacterTarget->RespondToAttack( this );
				}

			}
			else
			{
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(818),
									 .75f,
									 kFalse,
									 .6f );
			}
			break;
		case KTargetCharacterArea :
			if( m_pCastingCharacterTarget != NULL &&
				m_pCastingCharacterTarget->Alive() &&
				( m_pCastingCharacterTarget->Alignment() == TargetAlignment ||
				  TargetAlignment == KAllAlignments ) )
			{
				m_pCastingSpell->PerformFX( pLevel, this, this );
				Success = kTrue;
				pLevel.FindCharactersInArea( m_pCastingCharacterTarget->Position(), 
											m_pCastingSpell->Range(),
											CharacterList );
				for( int32 i = 0; i < CharacterList.size(); i++ )
				{
					if( CharacterList[i]->Alive() &&
						( CharacterList[i]->Alignment() == TargetAlignment ||
						TargetAlignment == KAllAlignments ) )
					{
						if( m_pCastingSpell->CanCastOnTarget( this, CharacterList[i] ) )
						{
							m_pCastingSpell->Cast( pLevel, this, CharacterList[i], kFalse );
						}
						else
						{
							pLevel.AddEventText( this,
												CharacterList[i],
												CharacterList[i]->Position() + D3DXVECTOR3( 0, 9, 0 ),
												LoadLocalizedString(995),
												.75f,
												kFalse,
												1 );
						}

						if( CharacterList[i]->Alignment() != Alignment() )
						{
							CharacterList[i]->RespondToAttack( this );
						}

					}
				}
			}
			else
			{
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(818),
									 .75f,
									 kFalse,
									 .6f );
			}
			break;
		case KTargetUserArea :
			Success = kTrue;
			pLevel.FindCharactersInArea( Position(), 
										m_pCastingSpell->Range(),
										CharacterList );
			m_pCastingSpell->PerformFX( pLevel, this, this );
			for( int32 i = 0; i < CharacterList.size(); i++ )
			{
				if( CharacterList[i]->Alive() &&
					( CharacterList[i]->Alignment() == TargetAlignment ||
					TargetAlignment == KAllAlignments ) )
				{
					if( m_pCastingSpell->CanCastOnTarget( this, CharacterList[i] ) )
					{
						m_pCastingSpell->Cast( pLevel, this, CharacterList[i], kFalse );
					}
					else
					{
						pLevel.AddEventText( this,
											CharacterList[i],
											CharacterList[i]->Position() + D3DXVECTOR3( 0, 9, 0 ),
											LoadLocalizedString(995),
											.75f,
											kFalse,
											1 );
					}
					if( CharacterList[i]->Alignment() != Alignment() )
					{
						CharacterList[i]->RespondToAttack( this );
					}

				}
			}			
			break;
		case KTargetUser :
			if( m_pCastingSpell->SufficientLevelToCast( pLevel, this ) )
			{
				m_pCastingSpell->Cast( pLevel, this, this );
				Success = kTrue;
			}
			else
			{
				pLevel.AddEventText( this,
									 NULL,
									 Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString1Variable(1082, KMagicSphereShortName[m_pCastingSpell->Sphere()] ),
									 .75f,
									 kFalse,
									 .8f );
			}
			break;
		case KTargetUserAndPets :
			m_pCastingSpell->Cast( pLevel, this, this );
			for( int32 i = 0; i < Pets(); i++ )
			{
				if( Pet( i )->AvailableForKeyCommand() )
				{
					m_pCastingSpell->Cast( pLevel, this, Pet( i ) );
				}
			}
			Success = kTrue;
			break;
		case KTargetUserPets :
			for( int32 i = 0; i < Pets(); i++ )
			{
				if( Pet( i )->AvailableForKeyCommand() )
				{
					m_pCastingSpell->Cast( pLevel, this, Pet( i ) );
					Success = kTrue;
				}
			}
			break;
		case KTargetCharacterAndPets :
			if( m_pCastingCharacterTarget != NULL &&
				m_pCastingCharacterTarget->Alive() &&
				( m_pCastingCharacterTarget->Alignment() == TargetAlignment ||
				  TargetAlignment == KAllAlignments ) )
			{
				if( m_pCastingSpell->CanCastOnTarget( this, m_pCastingCharacterTarget ) )
				{
					m_pCastingSpell->Cast( pLevel, this, m_pCastingCharacterTarget );
					Success = kTrue;

				}
				else
				{
					pLevel.AddEventText( this,
										m_pCastingCharacterTarget,
										m_pCastingCharacterTarget->Position() + D3DXVECTOR3( 0, 9, 0 ),
										LoadLocalizedString(995),
										.75f,
										kFalse,
										1 );
				}
				Success = kTrue;

				if( m_pCastingCharacterTarget->Alignment() != Alignment() )
				{
					m_pCastingCharacterTarget->RespondToAttack( this );
				}

				for( int32 i = 0; i < m_pCastingCharacterTarget->Pets(); i++ )
				{
					if( m_pCastingCharacterTarget->Pet( i )->AvailableForKeyCommand() )
					{
						m_pCastingSpell->Cast( pLevel, this, m_pCastingCharacterTarget->Pet( i ) );
						Success = kTrue;

						if( m_pCastingCharacterTarget->Pet( i )->Alignment() != Alignment() )
						{
							m_pCastingCharacterTarget->Pet( i )->RespondToAttack( this );
						}
					}
				}

			}
			else
			{
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(818),
									 .75f,
									 kFalse,
									 .9f );
			}
			break;
		case KTargetCharacterPets :
			if( m_pCastingCharacterTarget != NULL &&
				m_pCastingCharacterTarget->Alive() &&
				( m_pCastingCharacterTarget->Alignment() == TargetAlignment ||
				  TargetAlignment == KAllAlignments ) )
			{

				for( int32 i = 0; i < m_pCastingCharacterTarget->Pets(); i++ )
				{
					m_pCastingSpell->Cast( pLevel, this, m_pCastingCharacterTarget->Pet( i ) );
					Success = kTrue;

					if( m_pCastingCharacterTarget->Pet( i )->Alignment() != Alignment() )
					{
						m_pCastingCharacterTarget->Pet( i )->RespondToAttack( this );
					}
				}

			}
			else
			{
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(818),
									 .75f,
									 kFalse,
									 .9f );
			}
			break;
		case KTargetItem :
			if( m_pCastingItemTarget != NULL )
			{
				m_pCastingSpell->Cast( pLevel, this, m_pCastingItemTarget );
				Success = kTrue;
			}
			else
			{
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 7, 0 ),
									 LoadLocalizedString(1083),
									 .75f,
									 kFalse,
									 .9f );
			}
			break;
		}

		if( Success )
		{
			IncrementStat( KJournalStatSpellsCast );
			ModifyMana( -(float32)m_pCastingSpell->ManaCost() );
		}
		else
		{
			m_pCastingSpell->FailCast( pLevel, this, m_pCastingCharacterTarget );
		}
	}

} // CCharacter::CompleteSpell()

bool CCharacter::HasSpell( CSpellDescription* pSpellDescription )	// spell to learn
{
	for( uint32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( m_KnownSpells[i][j] != NULL &&
				m_KnownSpells[i][j]->Name() == pSpellDescription->Name() )
			{
				return kTrue;
			}
		}
	}
	return kFalse;
} // CCharacter::HasSpell()

bool CCharacter::HasSpell( const std::string& SpellName )	// spell to check for
{
	for( uint32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( m_KnownSpells[i][j] != NULL &&
				m_KnownSpells[i][j]->Name() == SpellName )
			{
				return kTrue;
			}
		}
	}
	return kFalse;
} // CCharacter::HasSpell()

void CCharacter::CycleSpell( int32 Direction )	// cycle direction
{
	bool FoundSpell( m_pActiveSpell == NULL );
	uint32 Spells( 0 );
	if( Direction == 1 )
	{
		for( int32 i = 0; i < KMagicSpheres; i++ )
		{
			for( int32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				if( m_KnownSpells[i][j] != NULL )
				{
					Spells++;
					if( FoundSpell )
					{
						SetActiveSpell( m_KnownSpells[i][j] );
						return;
					}
					else
					{
						if( m_KnownSpells[i][j] == m_pActiveSpell )
						{
							FoundSpell = kTrue;
						}
					}
				}
			}
		}
		// we wrapped around, so take the first spell
		if( Spells > 1 )
		{
			for( int32 i = 0; i < KMagicSpheres; i++ )
			{
				for( int32 j = 0; j < KMaximumSpellsPerSphere; j++ )
				{
					if( m_KnownSpells[i][j] != NULL )
					{
						SetActiveSpell( m_KnownSpells[i][j] );
						return;
					}
				}
			}
		}
	}
	else
	{
		for( int32 i = KMagicSpheres - 1; i >= 0; i-- )
		{
			for( int32 j = KMaximumSpellsPerSphere - 1; j >= 0; j-- )
			{
				if( m_KnownSpells[i][j] != NULL )
				{
					Spells++;
					if( FoundSpell )
					{
						SetActiveSpell( m_KnownSpells[i][j] );
						return;
					}
					else
					{
						if( m_KnownSpells[i][j] == m_pActiveSpell )
						{
							FoundSpell = kTrue;
						}
					}
				}
			}
		}
		// we wrapped around, so take the first spell
		if( Spells > 1 )
		{
			for( int32 i = KMagicSpheres - 1; i >= 0; i-- )
			{
				for( int32 j = KMaximumSpellsPerSphere - 1; j >= 0; j-- )
				{
					if( m_KnownSpells[i][j] != NULL )
					{
						SetActiveSpell( m_KnownSpells[i][j] );
						return;
					}
				}
			}
		}
	}
} // CCharacter::CycleSpell()

int32 CCharacter::FreeSpellSlot( EMagicSphere Sphere )	// sphere of magic 
{
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[Sphere][i] == NULL )
		{
			return i;
		}
	}
	return -1;
} // CCharacter::FreeSpellSlot()

void CCharacter::ClearQuests( void )
{
	for( uint32 i = 0; i < m_pQuests.size(); i++ )
	{
		DELETE_SAFELY( m_pQuests[i] );
	}
	m_pQuests.clear();
} // CCharacter::ClearQuests()

void CCharacter::ClearMasterQuest( void )
{
	DELETE_SAFELY( m_pMasterQuest );
} // CCharacter::ClearMasterQuest()

void CCharacter::GiveQuest( CQuest& pQuest )	// quest to accept
{
	if( Quests() < KMaximumActiveQuests )
	{
		PlayHelpSample( KSoundHelpNewQuest, kTrue );

		CQuest* pNewQuest = new CQuest( pQuest );
		m_pQuests.push_back( pNewQuest );
	}
} // CCharacter::GiveQuest()

void CCharacter::GiveMasterQuest( CQuest& pQuest )	// quest to accept
{
	DELETE_SAFELY( m_pMasterQuest );

	m_pMasterQuest = new CQuest( pQuest );
} // CCharacter::GiveMasterQuest()

bool CCharacter::HasQuestsForCharacter( std::string Name )	// character to check for quests for
{
	Name = StringUpper( Name );
	for( uint32 i = 0; i < Quests(); i++ )
	{
		if( StringUpper( Quest( i )->GiverName() ) == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::HasQuestsForCharacter()


bool CCharacter::HasQuestsForLevel( uint32 Depth )	// depth to check for
{
	for( uint32 i = 0; i < Quests(); i++ )
	{
		if( Quest( i )->Level() == Depth )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::HasQuestsForLevel()

bool CCharacter::HasMasterQuestForLevel( uint32 Depth )	// depth to check for
{
	if( m_pMasterQuest != NULL )
	{
		if( m_pMasterQuest->Level() == Depth )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::HasMasterQuestForLevel()

void CCharacter::RemoveQuest( uint32 Index )	// index of quest to remove
{
	DELETE_SAFELY( m_pQuests[Index] );

	m_pQuests[Index] = m_pQuests[m_pQuests.size() - 1];
	m_pQuests.pop_back();
} // CCharacter::RemoveQuest()

void CCharacter::ReceiveReward( CQuest& pQuest,					// quest to receive reward from
							    LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							    CLevel& pLevel,					// level to drop to if full
							    CItemTemplate& pItemManifest )	// item manifest to construct items from
{
	if( pQuest.ItemReward() != NULL )
	{
		CItemDescription* pDescription = pItemManifest.ItemDescription( pQuest.ItemReward()->m_BaseName );
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
								  *pQuest.ItemReward(),
								  pItemManifest );

			GiveItem( pLevel, pItem );
		}
	}


	GiveGold( pQuest.GoldReward() );
	AwardExperience( pLevel, NULL, pQuest.ExperienceReward() );
	AwardFame( pLevel, NULL, pQuest.FameReward() );

} // CCharacter::ReceiveReward()

void CCharacter::RememberDiscovery( CLevel& pLevel )	// level to remember discovery of
{
	float32 LastPopulationTime( -1 );
	if( RemembersLevel( pLevel.LevelDepth() ) )
	{
		for( uint32 i = 0; i < m_pDiscoveryHistory.size(); i++ )
		{
			if( m_pDiscoveryHistory[i]->Level() == pLevel.LevelDepth() )
			{
				LastPopulationTime = m_pDiscoveryHistory[i]->LastPopulationTime();
				RemoveDiscoveryHistory( i );
				break;
			}
		}
	}

	if( pLevel.HasLevelHistory() )
	{
		CLevelHistory *pHistory = new CLevelHistory( *pLevel.LevelHistory() );
		m_pDiscoveryHistory.push_back( pHistory );
		if( LastPopulationTime != -1 )
		{
			pHistory->SetLastPopulationTime( LastPopulationTime );
		}
		else
		{
			pHistory->SetLastPopulationTime( m_ExistenceTime );
		}

		CLLNode<CItem>*	pItem	= pLevel.Items()->GetFirst();
		while ( pItem != NULL )
		{
			if( pItem->Data()->Category() != KCategoryBlank &&
				pLevel.LevelDepth() != 0 )
			{
				pHistory->AddItem( *pItem->Data() );
			}
			pItem = pItem->Next();
		}

		std::vector< CCharacter* > pMasterList;
		std::vector< uint32 > pMasterIndex;
		// first store all non-minion chars
		CLLNode<CCharacter>*	pCharacter	= pLevel.Characters()->GetFirst();
		while ( pCharacter != NULL )
		{
			if( pCharacter->Data() != this &&
				pCharacter->Data()->Master() != this &&
				( ( ( !pCharacter->Data()->IsMerchant() || pLevel.LevelDepth() > 0 ) ) ||
				   pCharacter->Data()->MerchantType() == KMerchantStash ) &&
				!pCharacter->Data()->IsQuestGiver() &&
				pCharacter->Data()->Alive() &&
				!pCharacter->Data()->HasMaster() )
			{
				if( pCharacter->Data()->Pets() > 0 )
				{
					pMasterList.push_back( pCharacter->Data() );
					pMasterIndex.push_back( pHistory->Characters() );
				}
				pHistory->AddCharacter( *pCharacter->Data(), pLevel );
			}
			pCharacter = pCharacter->Next();
		}

		// now store minion chars
		pCharacter	= pLevel.Characters()->GetFirst();
		while ( pCharacter != NULL )
		{
			if( pCharacter->Data() != this &&
				pCharacter->Data()->Master() != this &&
				!pCharacter->Data()->IsMerchant() &&
				!pCharacter->Data()->IsQuestGiver() &&
				pCharacter->Data()->Alive() &&
				pCharacter->Data()->HasMaster() )
			{
				int32 MasterIndex( -1 );
				for( uint32 i = 0; i < pMasterList.size(); i++ )
				{
					if( pMasterList[i] == pCharacter->Data()->Master() )
					{
						MasterIndex = pMasterIndex[i];
					}
				}
				pHistory->AddCharacter( *pCharacter->Data(), 
									    pLevel );
				pHistory->GetCharacter( pHistory->Characters() - 1 )->m_MasterIndex = MasterIndex;
			}
			pCharacter = pCharacter->Next();
		}
		
	}

	if( m_pDiscoveryHistory.size() > KMaximumDiscoveryMemory )
	{
		for( uint32 i = 0; i < m_pDiscoveryHistory.size(); i++ )
		{
			// don't remove town history
			// and don't remove history for a level we have quests pending for
			if( m_pDiscoveryHistory[i]->Level() != 0 &&
				!HasQuestsForLevel( m_pDiscoveryHistory[i]->Level() &&
				!HasMasterQuestForLevel( m_pDiscoveryHistory[i]->Level() ) ) )
			{
				RemoveDiscoveryHistory( i );
				break;
			}
		}
	}
	for( int32 i = 0; i < (int32)m_pDiscoveryHistory.size(); i++ )
	{
		// don't remove town history
		// and don't remove history for a level we have quests pending for
		if( m_ExistenceTime > m_pDiscoveryHistory[i]->LastPopulationTime() + KMaximumLevelRespawnTime 
			&& m_pDiscoveryHistory[i]->Level() != pLevel.LevelDepth() &&
			m_pDiscoveryHistory[i]->Level() != 0 &&
			!HasQuestsForLevel( m_pDiscoveryHistory[i]->Level() ) &&
			!HasMasterQuestForLevel( m_pDiscoveryHistory[i]->Level() ) &&
			!( m_HasOpenPortal && m_PortalDepth == m_pDiscoveryHistory[i]->Level() ))
		{
			RemoveDiscoveryHistory( i );
			i--;
		}
	}
} // CCharacter::RememberDiscovery()

void CCharacter::RemoveDiscoveryHistory( uint32 Index )	// index within the history
{
	for( int32 i = Index; i < (int32)m_pDiscoveryHistory.size() - 1; i++ )
	{
		m_pDiscoveryHistory[i] = m_pDiscoveryHistory[i+1];
	}
	m_pDiscoveryHistory.pop_back();

} // CCharacter::RemoveDiscoveryHistory()

bool CCharacter::RemembersLevel( int32 LevelDepth )	// depth to check for
{
	for( uint32 i = 0; i < m_pDiscoveryHistory.size(); i++ )
	{
		if( m_pDiscoveryHistory[i]->Level() == LevelDepth )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::RemembersLevel()

CLevelHistory*	CCharacter::GetLevelHistory( int32 LevelDepth )	// depth to check for
{
	for( uint32 i = 0; i < m_pDiscoveryHistory.size(); i++ )
	{
		if( m_pDiscoveryHistory[i]->Level() == LevelDepth )
		{
			return m_pDiscoveryHistory[i];
		}
	}
	return NULL;
} // CCharacter::GetLevelHistory()

void CCharacter::SetMaster( CCharacter* pCharacter )	// character to set as master
{
	if( pCharacter == NULL )
	{
		if( m_pMasterCharacter != NULL )
		{
			m_pMasterCharacter = NULL;
			m_Alignment = m_OriginalAlignment;
		}
	}
	else
	{
		if( m_pMasterCharacter == NULL )
		{
			m_OriginalAlignment = m_Alignment;
		}

		m_pMasterCharacter = pCharacter;
		if( m_pMasterCharacter->IsPlayer() )
		{
			m_RepathPauseTime = .5f;
		}
		// switch to the alignment of the master
		m_Alignment = m_pMasterCharacter->Alignment();
	}


	if( Master() != NULL &&
		Master()->IsPlayer() )
	{
		// pets can equip necklaces and rings.
		// and that's it.
		// players can use EVERYTHING by default
		for( uint32 i = 0; i < KItemTypes; i++ )
		{
			m_pInventory->SetSupportsItemType( (EItemType)i, kFalse );
		}

		m_pInventory->SetSupportsItemType( KItemRing, kTrue );
		m_pInventory->SetSupportsItemType( KItemNecklace, kTrue );
	}

	// clear out state, in case we are attacking something we now shouldn't be
	if( AvailableForCommand() )
	{
		SetAIState( KAIInactive );
	}
	SetTarget( NULL );
	SetTargetItem( NULL );
} // CCharacter::SetMaster()

void CCharacter::RemovePet( CCharacter* pCharacter )	// character to set as pet
{
	int32 Index = PetIndex( pCharacter );
	if( Index != -1 )
	{
		m_pPetCharacters[Index] = m_pPetCharacters[m_pPetCharacters.size() - 1 ];
		m_pPetCharacters.pop_back();
	}
} // CCharacter::RemovePet()

void CCharacter::RemovePets( void )	// character to set as pet
{
	m_pPetCharacters.clear();
} // CCharacter::RemovePets()

void CCharacter::AddPet( CCharacter* pCharacter )	// character to set as pet
{
	if( pCharacter == NULL )
	{
		return;
	}
	else
	{
		if( !HasPet( pCharacter ) )
		{

			m_pPetCharacters.push_back( pCharacter );
			// notify the character that they have become our pet
			pCharacter->SetMaster( this );
		}
	}

} // CCharacter::SetPet()

bool CCharacter::HasPet( CCharacter* pCharacter )	// character to check for
{
	return PetIndex( pCharacter ) != -1;
} // CCharacter::HasPet()

int32 CCharacter::PetIndex( CCharacter* pCharacter )	// character to check for
{
	for( uint32 i = 0; i < m_pPetCharacters.size(); i++ )
	{
		if( m_pPetCharacters[i] == pCharacter )
		{
			return i;
		}
	}
	return -1;
} // CCharacter::PetIndex()

void CCharacter::SetIsMerchant( bool State )	// merchant or not?
{	
	m_IsMerchant = State;		
	if( m_IsMerchant )
	{
		m_pInventory->Resize( KMaxMerchantInventorySlots );
	}
} // CCharacter::SetIsMerchant()

void CCharacter::SetMerchantType( EMerchant Type )	// type of merchant
{	
	m_MerchantType = Type;					
	if( m_MerchantType == KMerchantStash )
	{
		m_DisplayedAsItem = kTrue;
	}
} // CCharacter::SetMerchantType()

// notify us if a character was deleted, in case we have them targeted
void CCharacter::NotifyOfDeletion( CCharacter* pCharacter )	// character being deleted
{
	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)m_pEffects[i].size(); j++ )
		{
			if( m_pEffects[i][j]->Owner() == pCharacter )
			{
					
				m_pEffects[i][j]->SetOwner( NULL );
			}
		}
	}
	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)m_pNaturalEffects[i].size(); j++ )
		{
			if( m_pNaturalEffects[i][j]->Owner() == pCharacter )
			{
					
				m_pNaturalEffects[i][j]->SetOwner( NULL );
			}
		}
	}

	if( m_pCastingCharacterTarget == pCharacter )
	{
		m_pCastingCharacterTarget = NULL;
	}
	if( m_pMasterCharacter == pCharacter )
	{
		SetMaster( NULL );
	}
	if( HasPet( pCharacter ) )
	{
		RemovePet( pCharacter );
	}
	if( Target() == pCharacter )
	{
		SetTarget( NULL, kFalse );
	}
} // CCharacter::NotifyOfDeletion()



// notify us if a character has died, in case they were important to us ( quests )
void CCharacter::NotifyOfDeath( CLevel& pLevel,				// active level
								CCharacter* pCharacter )	// character which has died
{
	for( uint32 i = 0; i < Quests(); i++ )
	{
		if( Quest( i )->Type() == KQuestKillMonster ||
			Quest( i )->Type() == KQuestKillArmy ||
			Quest( i )->Type() == KQuestArmyItem ||
			Quest( i )->Type() == KQuestMonsterItem ||
			Quest( i )->Type() == KQuestKillMonsters )
		{
			if( StringUpper( Quest( i )->QuestMonsterName() ) ==
				StringUpper( pCharacter->Name() ) &&
				Quest( i )->Level() ==
				pLevel.LevelDepth() )
			{
				Quest( i )->IncreaseMonstersCompleted();
			}
			if( StringUpper( Quest( i )->QuestMinionMonsterName() ) ==
				StringUpper( pCharacter->Name() ) &&
				Quest( i )->Level() ==
				pLevel.LevelDepth() )
			{
				Quest( i )->IncreaseMinionsCompleted();
			}
		}
	}

	if( m_pMasterQuest != NULL )
	{
		if( m_pMasterQuest->Type() == KQuestMasterQuest )
		{
			if( StringUpper( m_pMasterQuest->QuestMonsterName() ) ==
				StringUpper( pCharacter->Name() ) &&
				m_pMasterQuest->Level() ==
				pLevel.LevelDepth() )
			{
				m_pMasterQuest->IncreaseMonstersCompleted();
			}
			if( StringUpper( m_pMasterQuest->QuestMinionMonsterName() ) ==
				StringUpper( pCharacter->Name() ) &&
				m_pMasterQuest->Level() ==
				pLevel.LevelDepth() )
			{
				m_pMasterQuest->IncreaseMinionsCompleted();
			}
		}
	}

} // CCharacter::NotifyOfDeletion()

// notify us if an item was deleted, in case we have it targeted
void CCharacter::NotifyOfDeletion( CItem* pItem )	// item being deleted
{
	if( m_pCastingItemTarget == pItem )
	{
		m_pCastingItemTarget = NULL;
	}
	if( m_pActiveWeapon == pItem )
	{
		m_pActiveWeapon = NULL;
	}
	if( TargetItem() == pItem )
	{
		SetTargetItem( NULL );
	}
} // CCharacter::NotifyOfDeletion()

void CCharacter::SetMaximumTreeDepth( uint32 Depth )	// pathfinding tree depth
{
	m_pAStar->SetMaximumTreeDepth( Depth );
} // CCharacter::SetMaximumTreeDepth()

void CCharacter::CreatePathfinder( CLevel& pLevel )	// level to create pathfinder for
{
	DELETE_SAFELY( m_pAStar );
	m_pAStar = new CAStar( pLevel.TopLeftX(),
						   pLevel.TopLeftY(),
						   pLevel.PassabilityWidth(), 
						   pLevel.PassabilityHeight(),
						   pLevel.MapPassability(),
						   pLevel.ObjectPassability(),
						   pLevel.PassabilityCellWidth() );
	StopPathing();
} // CCharacter::CreatePathfinder()

EAlignment CCharacter::Alignment( void )
{
	if( !IsPlayer() &&
		!( HasMaster() && Master()->IsPlayer() ) &&
		EffectValue( KEffectTurnAlignment ) > Level() )
	{
		if( m_Alignment == KEvil )
		{
			return KGood;
		}
		else if( m_Alignment == KGood )
		{
			return KEvil;
		}
		return m_Alignment;
	}
	else
	{
		return m_Alignment;	
	}

} // CCharacter::Alignment()

void CCharacter::SetRunning( bool RunningState )	// running state - on or off
{
	if( CanRun() ||
		( HasMaster() && m_pMasterCharacter->CanRun() ) )
	{
		m_Running = RunningState;
	}
	else
	{
		m_Running = kFalse;
	}
} // CCharacter::SetRunning()

void CCharacter::CalculateActiveRange( const D3DXVECTOR3& FocalPoint )	// point where the camera is directed
{
	if( HasMaster() &&
		Master()->InActiveRange() )
	{
		m_InActiveRange = kTrue;
		return;
	}

	if( AIState() == KAIDying )
	{
		m_InActiveRange = kTrue;
		return;
	}
	D3DXVECTOR3 Delta = FocalPoint - m_Position;
	Delta.y = 0;
	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance > KHibernationRange )
	{
		m_InActiveRange = kFalse;
	}
	else
	{
		m_InActiveRange = kTrue;
	}
} // CCharacter::CalculateActiveRange()

// verify that all present spells are still 'knowable'
void CCharacter::VerifySpells( void )
{
	if( ActiveSpell() != NULL )
	{
		if( !ActiveSpell()->RequirementsMet( *this ) )
		{
			SetActiveSpell( NULL );
		}
	}
} // CCharacter::VerifySpells()

void CCharacter::Update( const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
						 float32 TimeElapsed, 		// time elapsed, in seconds
						 CLevel& pLevel )			// a scene to collide with
{
	if( AIState() == KAIDead )
	{
		SetCollideable( kFalse );
		RemoveFromAvoidanceMap( pLevel );
		return;
	}

	if( AIState() == KAIInTown )
	{
		RemoveFromAvoidanceMap( pLevel );
		UpdateTownTimer( TimeElapsed,
						 pLevel );
		return;
	}

	if( IsPlayer() &&
		pLevel.LevelDepth() > 0 &&
		m_JournalStats[KJournalStatLevelsExplored] < pLevel.LevelDepth() )
	{
		m_JournalStats[KJournalStatLevelsExplored] = pLevel.LevelDepth();
	}

	CalculateActiveRange( FocalPoint );

	m_ExistenceTime += TimeElapsed;

	// characters outside effective activity range are not updated,
	// for performance reasons( 100s of monsters per level!)
	if( !InActiveRange() )
	{
		SetCollideable( kFalse );
		RemoveFromAvoidanceMap( pLevel );
		SetTarget( NULL );
		return;
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

	UpdateHelp( TimeElapsed );

	// reset the check for whether we are turning in place
	m_IsTurning = kFalse;

	UpdateEffects( pLevel, TimeElapsed );
	CalculateEffectValues();

	// make sure the selected spell exists
	VerifySpells();
	// then make sure that we don't have blank spell selection if we have spells
	EnsureSpellSelection();

	// calculate max hp based upon vitality - only really happens to player characters
	CalculateMaxHP();
	CalculateMaxStamina();
	CalculateMaxMana();

	UpdateHP( pLevel, TimeElapsed );
	UpdateMana( TimeElapsed );
	UpdateStamina( pLevel,
				   TimeElapsed );

	if( IsPlayer() )
	{
		if( HP() < 15 )
		{
			// pop up a 'health low' tip?
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipHealthLow ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipHealthLow ) );
				pLevel.Tips().SetTipSeen( KTipHealthLow, kTrue );
			}
		}
		if( UnusedStatPoints() > 0 )
		{
			// pop up a 'level up' tip?
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipLevelUp ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipLevelUp ) );
				pLevel.Tips().SetTipSeen( KTipLevelUp, kTrue );
			}
		}
	}
	if( IsPetNearDeath() &&
		HasMaster() &&
		Master()->IsPlayer() )
	{
		// remove poison effects once pets hit 0 health
		RemoveEffect( "POISON" );
		// tip about fleeing pets
		if( m_pSettings.GetSettings( KSetShowTips ) &&
			!pLevel.Tips().TipSeen( KTipPetFlee ) )
		{
			pLevel.RequestTip( pLevel.Tips().ContextTip( KTipPetFlee ) );
			pLevel.Tips().SetTipSeen( KTipPetFlee, kTrue );
		}
	}

	RemoveFromAvoidanceMap( pLevel );
	// make sure we don't collide with ourselves while updating
	SetCollideable( kFalse );
	if( HasPets() )
	{
		for( uint32 i = 0; i < Pets(); i++ )
		{
			m_pPetCharacters[i]->SetCollideable( kFalse );
		}
	}
	if( HasMaster() )
	{
		m_pMasterCharacter->SetCollideable( kFalse );
	}

	UpdateAI( TimeElapsed,
			  pLevel );

	RemoveDeadEffects();

	if( TimeElapsed == 0 )
	{
		// we aren't collideable when dead
		if( Alive() )
		{
			SetCollideable( kTrue );
			AddToAvoidanceMap( pLevel );
		}
		return;
	}


	for( uint32 i = 0; i < Pets(); i++ )
	{
		if( !m_pPetCharacters[i]->Alive() )
		{
			RemovePet( m_pPetCharacters[i] );
			i--;
		}
	}

	if( HasMaster() &&
		!m_pMasterCharacter->Alive() &&
		!m_pMasterCharacter->IsPlayer() )
	{
		SetMaster( NULL );
	}

	if( PerformingCast() &&
		!Pathing() &&
		m_pCastingSpell->Target() == KTargetItem &&
		m_pCastingItemTarget != NULL )
	{
		TurnTowardPosition( m_pCastingItemTarget->Position(), TimeElapsed );
	}
	else if( PerformingCast() &&
			 !Pathing() &&
			 m_pCastingSpell->Target() == KTargetCharacter &&
			 m_pCastingCharacterTarget != NULL )
	{
		TurnTowardPosition( m_pCastingCharacterTarget->Position(), TimeElapsed );
	}
	else if( PerformingCast() &&
			 !Pathing() &&
			 ( m_pCastingSpell->Target() == KTargetArea ||
			   m_pCastingSpell->Target() == KTargetProjectile ) )
	{
		if( m_pCastingCharacterTarget != NULL )
		{
			TurnTowardPosition( m_pCastingCharacterTarget->Position(), TimeElapsed );
		}
		else
		{
			TurnTowardPosition( m_CastingTargetPosition, TimeElapsed );
		}
	}
	else if( Target() != NULL  &&
			 !Pathing() )
	{
		TurnTowardPosition( Target()->Position(), TimeElapsed );
	}
	else if( TargetItem() != NULL  &&
			 !Pathing() )
	{
		TurnTowardPosition( TargetItem()->Position(), TimeElapsed );
	}
	else if( m_HoldPosition )
	{
		TurnTowardPosition( m_FinalTargetPosition, TimeElapsed );
	}

	if( m_HP <= 0 &&
		AIState() != KAIDying &&
		AIState() != KAIDead )
	{
		Die( pLevel );
	}


	UpdatePathing( TimeElapsed );

	UpdateMotion( TimeElapsed, pLevel );

	//UpdateAnimation( TimeElapsed );

	UpdateAttack( pLevel );

	UpdateCast( pLevel );

	UpdateTimers( TimeElapsed, pLevel );

	// we aren't collideable when dead
	if( Alive() )
	{
		SetCollideable( kTrue );
		AddToAvoidanceMap( pLevel );
	}
	if( HasPets() )
	{
		for( uint32 i = 0; i < Pets(); i++ )
		{
			if( m_pPetCharacters[i]->Alive() )
			{
				m_pPetCharacters[i]->SetCollideable( kTrue );
			}
		}
	}
	if( HasMaster() &&
		m_pMasterCharacter->Alive() )
	{
		m_pMasterCharacter->SetCollideable( kTrue );
	}

	if( AIState() != KAIDead )
	{
		EmitParticles();
	}

	// make sure that stat changes haven't rendered equipment unusable
	m_pInventory->VerifyEquipment( pLevel );

	m_pModel->ClearKeys();
} // CCharacter::Update()


void CCharacter::UpdateAI( float32 TimeElapsed, 	// time elapsed, in seconds
						   CLevel& pLevel )		// a scene to collide with
{
	if( Target() != NULL &&
		( !Target()->Alive() ||
		  !Target()->InActiveRange() ) )
	{
		SetTarget( NULL, kFalse );
	}

	SelectAttack();

	m_FishBiting = kFalse;

	switch( m_AIState )
	{

	case KAIDying :
		DyingAI( TimeElapsed, pLevel );
		break;
	case KAIAttack :
		AttackAI( TimeElapsed, pLevel );
		break;
	case KAIReturningToTown :
		ReturnToTownAI( TimeElapsed, pLevel );
		break;
	case KAIHunt :
		HuntAI( TimeElapsed, pLevel );
		break;
	case KAICast :
		CastAI( TimeElapsed, pLevel );
		break;
	case KAIApproach :
		ApproachAI( TimeElapsed, pLevel );
		break;
	case KAIGetItem :
		GetItemAI( TimeElapsed, pLevel );
		break;
	case KAIOperateItem :
		OperateItemAI( TimeElapsed, pLevel );
		break;
	case KAIFish :
		FishAI( TimeElapsed, pLevel );
		break;
	case KAIPullFish :
		PullFishAI( TimeElapsed, pLevel );
		break;
	case KAICancelFish :
		CancelFishAI( TimeElapsed, pLevel );
		break;
	}

} // CCharacter::UpdateAI()

void CCharacter::AttackAI( float32 TimeElapsed, // time elapsed, in seconds
						   CLevel& pLevel )		// a scene to collide with
{
	if( TargetItem() != NULL )
	{
		SetTargetItem( NULL );
	}
	if( Target() == NULL ||
		!Target()->Alive() )
	{
		SetAIState( KAIInactive );
		SetTarget( NULL, kFalse );
		return;
	}

	if( Target()->Alignment() == Alignment() ||
		Target()->Alignment() == KNeutral )
	{
		SetAIState( KAIInactive );
		SetTarget( NULL );
		return;
	}

	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			SetAIState( KAIInactive );
			SetTarget( NULL );
			return;
		}
	}

	D3DXVECTOR3 Delta = Target()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );


	if( ( !InAttackRange() ||
		  !ValidLineOfSight( pLevel ) ) &&
		!m_HoldPosition )
	{
		// if not mid-attack, then go back to hunting our target
		if( !PerformingAttack() )
		{
			SetAIState( KAIHunt );
		}
		return;
	}
	else	// in range - let's attack!
	{
		StopPathing();
		if( FacingTarget() )
		{
			Attack();
			SetAIState( KAIInactive );
		}
	}
} // CCharacter::AttackAI()

void CCharacter::CastAI( float32 TimeElapsed, // time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{

	if( !PerformingCast() )
	{
		SetAIState( KAIInactive );
	}
} // CCharacter::CastAI()

void CCharacter::SelectAttack( void )
{
	if( PerformingAttack() )
	{
		return;
	}
	m_pCurrentAttack = NULL;
	// first check for attacks with equipped weapons
	if( m_pInventory->SlotEquipped( KSlotRightHand ) )
	{
		if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL &&
			m_LastAttackRight )
				
		{
			m_pActiveWeapon = m_pInventory->GetItemFromSlot( KSlotLeftHand );
			if( m_pActiveWeapon != NULL &&
				m_pCurrentAttack != m_pActiveWeapon->LeftAttack() )
			{
				m_pCurrentAttack = m_pActiveWeapon->LeftAttack();
			}
			if( m_pCurrentAttack != NULL )
			{
				m_pCurrentAttack->SetAnimationIndex( FindRandomAnimation( m_pCurrentAttack->Animation() ) );
			}
		}
		else
		{
			m_pActiveWeapon = m_pInventory->GetItemFromSlot( KSlotRightHand );
			if( m_pActiveWeapon != NULL &&
				m_pCurrentAttack != m_pActiveWeapon->RightAttack() )
			{
				m_pCurrentAttack = m_pActiveWeapon->RightAttack();
			}
			if( m_pCurrentAttack != NULL )
			{
				m_pCurrentAttack->SetAnimationIndex( FindRandomAnimation( m_pCurrentAttack->Animation() ) );
			}
		}
	}
	else if( m_pInventory->SlotEquipped( KSlotLeftHand ) )
	{
		m_pActiveWeapon = m_pInventory->GetItemFromSlot( KSlotLeftHand );
		if( m_pActiveWeapon != NULL &&
			m_pCurrentAttack != m_pActiveWeapon->LeftAttack() )
		{
			m_pCurrentAttack = m_pActiveWeapon->LeftAttack();
		}
		if( m_pCurrentAttack != NULL )
		{
			m_pCurrentAttack->SetAnimationIndex( FindRandomAnimation( m_pCurrentAttack->Animation() ) );
		}
	}
	// then, if we had none of those, do an unarmed attack
	else
	{
		if( UnarmedAttacks() > 0 )
		{
			m_pActiveWeapon = NULL;
			uint32 AttackToUse = RandomIntegerBetween( 0, UnarmedAttacks() - 1 );
			m_pCurrentAttack = UnarmedAttack( AttackToUse );
		}
	}

} // CCharacter::SelectAttack()

void CCharacter::HuntAI( float32 TimeElapsed, 	// time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{
	if( Target() == NULL ||
		!Target()->Alive() )
	{
		SetAIState( KAIInactive );
		SetTarget( NULL, kFalse );
		return;
	}
	if( Target()->Alignment() == Alignment() ||
		Target()->Alignment() == KNeutral )
	{
		SetAIState( KAIInactive );
		SetTarget( NULL );
		return;
	}

	D3DXVECTOR3 Delta = Target()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance >= m_FollowRadius &&
		!m_HoldPosition )
	{
		StopPathing();
		SetAIState( KAIWander );
		SetTarget( NULL );
		return;
	}

	bool LineOfSightValid( ValidLineOfSight( pLevel ) );

	if( ( !InAttackRange() ||
		  !LineOfSightValid ) &&
		  !m_HoldPosition &&
		m_TimeSincePath > m_RepathPauseTime )
	{

		// follow them if we aren't already
		if( !Pathing() )
		{
			FollowCharacter( Target(),
							 pLevel );
		}
		else	//if already pathing, and the target has moved significantly,
				// we re-path
		{
			Delta = Target()->Position() - m_FinalTargetPosition;
			Delta.y = 0;

			float32 Distance = D3DXVec3Length( &Delta );
			if( Distance >= 16 )
			{
				FollowCharacter( Target(),
								 pLevel );
			}
		}

		// if pathing failed, then stop attacking this character
		if( !Pathing() )
		{
			SetTarget( NULL );
			SetAIState( KAIWander );

		}
	}

	// if in range of our hunted character, start attacking
	if( ( InAttackRange() &&
		  LineOfSightValid ) ||
		  m_HoldPosition )
	{
		SetAIState( KAIAttack );
		StopPathing();
		return;
	}
} // CCharacter::HuntAI()

void CCharacter::GetItemAI( float32 TimeElapsed, 	// time elapsed, in seconds
							CLevel& pLevel )		// a scene to collide with
{
	if( TargetItem() == NULL )
	{
		SetAIState( KAIInactive );
		return;
	}

	// if this is a pet retrieving items for its master, we want to run
	if( HasMaster() )
	{
		// run if possible
		SetRunning( kTrue );
	}

	D3DXVECTOR3 Delta = TargetItem()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	// if in range of the item, we're done
	if( TargetItem()->IsContainer() )
	{
		if( TargetItem()->Destructible() )
		{
			bool LineOfSightValid( ValidLineOfSight( pLevel ) );

			if( ( InAttackRange() && LineOfSightValid )
				|| InInteractionRange() )
			{
				StopPathing();
				if( FacingTarget() )
				{
					Attack();
					SetAIState( KAIInactive );
				}
				return;
			}
		}
		else
		{
			if( InInteractionRange() )
			{

				if( TargetItem()->Trapped() )
				{
					IncrementStat( KJournalStatTrapsSprung );
					PlayHelpSample( KSoundHelpTrap, kTrue );
				}

				IncrementStat( KJournalStatChestsOpened );
				if(TargetItem()->Name() == "Large Chest")
                    IncrementStat( KJournalStatLargeChestsOpened );

				TargetItem()->Open( pLevel );
				SetAIState( KAIInactive );
				return;
			}
		}

	}
	else if( InInteractionRange() )
	{
		SetAIState( KAIInactive );

		GetItem( TargetItem(), pLevel );
		return;
	}


	if( Distance > m_CollisionRadius )
	{

		// follow them if we aren't already
		if( !Pathing() )
		{
			SetDestination( pLevel,
							TargetItem()->Position().x, 
							TargetItem()->Position().z );
			RemoveFromAvoidanceMap( pLevel );
		}
	/*	else
		{
			D3DXVECTOR3 Delta = m_pTargetItem->Position() - m_TargetPosition;
			Delta.y = 0;

			Distance = D3DXVec3Length( &Delta );
			if( Distance > m_CollisionRadius )
			{
				SetDestination( pLevel,
								m_pTargetItem->Position().x, 
								m_pTargetItem->Position().z );
				RemoveFromAvoidanceMap( pLevel );
			}
		}*/

	}

	// if pathing failed, then stop trying to get the item
	if( !Pathing() )
	{
		SetTargetItem( NULL );
		SetAIState( KAIWander );
	}
} // CCharacter::GetItemAI()

void CCharacter::GetItem( CItem* pTargetItem,	// item to get ( assumes proximity is good )
						  CLevel& pLevel )		// a scene to collide with
{
	if( pTargetItem == NULL )
	{
		return;
	}

	pTargetItem->PlayTakeSound();
	bool Success( kFalse );

	if( pTargetItem->Type() == KItemGold &&
		HasMaster() &&
		Master()->Alive() )
	{
		Success = Master()->Inventory()->AddItem( pTargetItem, pLevel );
	}
	else
	{
		Success = m_pInventory->AddItem( pTargetItem, pLevel );
	}

	if( Success )
	{
		// gold gets deleted
		if( pTargetItem->Type() == KItemGold )
		{
			pLevel.AddEventText( this,
								 NULL,
								 pTargetItem->Position() + D3DXVECTOR3( 0, 7, 0 ),
								 pTargetItem->Name(),
								 .75f,
								 kFalse,
								 1 );
			pLevel.DeleteItem( pTargetItem );
			return;
		}
		else	// other items get moved to the inventory
		{
			pLevel.RemoveItem( pTargetItem );
		}
		switch( pTargetItem->Category() )
		{
		case KCategoryWeapon :
			if( !m_pInventory->SlotEquipped( KSlotRightHand ) &&
				!m_pInventory->SlotEquipped( KSlotLeftHand ) )
			{
				m_pInventory->EquipBestWeapon( pLevel, kFalse );
			}
			break;
		case KCategoryArmor :
			switch( pTargetItem->Type() )
			{
			case KItemShield :
				if( !m_pInventory->SlotEquipped( KSlotLeftHand ) &&
					!m_pInventory->SlotEquipped( KSlotLeftArm ) )
				{
					m_pInventory->EquipBestShield( pLevel, kFalse );
				}
				break;
			case KItemHelmet :
				if( !m_pInventory->SlotEquipped( KSlotHead ) )
				{
					m_pInventory->EquipBestHelmet( pLevel, kFalse );
				}
				break;
			case KItemBelt :
				if( !m_pInventory->SlotEquipped( KSlotBelt ) )
				{
					m_pInventory->EquipBestBelt( pLevel, kFalse );
				}
				break;
			case KItemBoots :
				if( !m_pInventory->SlotEquipped( KSlotFeet ) )
				{
					m_pInventory->EquipBestBoots( pLevel, kFalse );
				}
				break;
			case KItemShirt :
				if( !m_pInventory->SlotEquipped( KSlotChest ) )
				{
					m_pInventory->EquipBestShirt( pLevel, kFalse );
				}
				break;
			case KItemGloves :
				if( !m_pInventory->SlotEquipped( KSlotGloves ) )
				{
					m_pInventory->EquipBestGloves( pLevel, kFalse );
				}
				break;
			}
			break;
		}

		//m_pInventory->EquipEmpty( pLevel, kFalse );
		SetTargetItem( NULL );
	}
	else
	{
		if( HasMaster() &&
			Master()->IsPlayer() )
		{
			// tip about sending pets to town
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipPetFull ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipPetFull ) );
				pLevel.Tips().SetTipSeen( KTipPetFull, kTrue );
			}
		}
		else if( IsPlayer() )
		{
			PlayHelpSample( KSoundHelpPackFull );
			// tip about full inventory
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipInventoryFull ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipInventoryFull ) );
				pLevel.Tips().SetTipSeen( KTipInventoryFull, kTrue );
			}
		}

		pTargetItem->SetPosition( pLevel, m_Position );
		pTargetItem->Drop( pLevel );
		SetTargetItem( NULL );
	}

} // CCharacter::GetItem()

void CCharacter::CompleteEvent( CNameGenerator& pNameGenerator,	// generator to make random name
							    CLevel& pLevel,					// active level
								uint32 ConfirmationResult,		// result of the confirmation menu button press
								CItem* pItem )					// event item
{
	if( TargetItem() == NULL )
	{
		SetTargetItem( NULL );
		SetAIState( KAIInactive );
		return;
	}
	bool Success( kFalse );
	int32 Percent( 0 );
	int32 Area( 0 );
	int32 Bonus( 0 );
	int32 Gem( 0 );
	int32 Monster( 0 );
	int32 ModifiedLevelDepth( pLevel.LevelDepth() );
	ModifiedLevelDepth = (int32)ceil( (float32)ModifiedLevelDepth * KDifficultyMultiplier[pLevel.Difficulty()] );
	ModifiedLevelDepth += (int32)KDifficultyLevelBase[pLevel.Difficulty()];
	if( ModifiedLevelDepth < 1 )
	{
		ModifiedLevelDepth = 1;
	}
	std::string ResultParameter = "Empty";
	std::string ResultText = LoadLocalizedString(1084);
	CItemDescription* pItemDescription;
	CItem* pNewItem;
	CCharacterDescription* pCharacterDescription;
	CCharacter* pCharacter;
	CSFXDescription* pSFX;
	if( ConfirmationResult == KButtonEventYes )
	{
		switch( TargetItem()->Type() )
		{
		case KItemShrine :
			Percent = RandomIntegerBetween( 0, 100 );
			// 75/25 chance between stats and skills
			if( Percent < 75 )	// stats
			{
				Bonus = RandomIntegerBetween( 1, 3 );
				if( RandomIntegerBetween( 0, 100 ) < 20 )
				{
					Bonus *= -1;
				}
				Area = RandomIntegerBetween( 0, 3 );
				switch( Area )
				{
				case 0:
					//ResultParameter = "\bStrength\b";
					ResultParameter = "\b" + LoadLocalizedString(291) + "\b";
					m_Strength += Bonus;
					if( m_Strength < 1 )
					{
						m_Strength = 1;
					}
					break;
				case 1:
					//ResultParameter = "\bDexterity\b";
					ResultParameter = "\b" + LoadLocalizedString(292) + "\b";
					m_Dexterity += Bonus;
					if( m_Dexterity < 1 )
					{
						m_Dexterity = 1;
					}
					break;
				case 2:
					//ResultParameter = "\bVitality\b";
					ResultParameter = "\b" + LoadLocalizedString(293) + "\b";
					m_Vitality += Bonus;
					if( m_Vitality < 1 )
					{
						m_Vitality = 1;
					}
					break;
				case 3:
					//ResultParameter = "\bMagic\b";
					ResultParameter = "\b" + LoadLocalizedString(294) + "\b";
					m_Magic += Bonus;
					if( m_Magic < 1 )
					{
						m_Magic = 1;
					}
					break;
				}

				if( Bonus > 0 )
				{
					ResultText = LoadLocalizedString2Variables(1085, ResultParameter, IntegerToString( abs( Bonus ) ) );
					Success = kTrue;
				}
				else
				{
					ResultText = LoadLocalizedString2Variables(1086, ResultParameter, IntegerToString( abs( Bonus ) ) );
					Success = kFalse;
				}
				if( Success )
				{
					PlayHelpSample( KSoundHelpFateSmiles );
				}

				pLevel.RequestMessage( ResultText );
				m_pInventory->CalculateEffectValues();
			}
			else
			{
				Bonus = RandomIntegerBetween( 1, 3 );
				if( RandomIntegerBetween( 0, 100 ) < 20 )
				{
					Bonus *= -1;
				}
				Area = RandomIntegerBetween( 0, KSkills - 1 );

				if( m_SkillPoints[Area] == 0 )
				{
					Bonus = 0;
				}
				m_SkillPoints[Area] += Bonus;
				ResultParameter = KSkillDescription[Area];
				if( m_SkillPoints[Area] < 0 )
				{
					m_SkillPoints[Area] = 0;
				}

				if( Bonus > 0 )
				{
					ResultText = LoadLocalizedString2Variables(1085, ResultParameter, IntegerToString( abs( Bonus ) ) );
					Success = kTrue;
				}
				else if ( Bonus < 0 )
				{
					ResultText = LoadLocalizedString2Variables(1086, ResultParameter, IntegerToString( abs( Bonus ) ) );
					Success = kFalse;
				}
				else
				{
					ResultText = LoadLocalizedString(1084);
					Success = kFalse;
				}
				if( Success )
				{
					PlayHelpSample( KSoundHelpFateSmiles );
				}

				pLevel.RequestMessage( ResultText );
				m_pInventory->CalculateEffectValues();

			}
			break;
		case KItemStatue :
			Percent = RandomIntegerBetween( 0, 100 );
			// 50/50 chance between stats and skills
			if( Percent < 50 )	// we get gems
			{

				Gem = pLevel.RandomRareGemItemIndex();
				pItemDescription = pLevel.ItemManifest()->TakeableItemDescription( Gem );
				if( pItemDescription != NULL )
				{
					// two gemstones!
					pNewItem = pLevel.AddItem( *pItemDescription, pLevel.RandomOpenItemPosition( TargetItem()->Position(), 6 ) );
					pNewItem->Drop( pLevel );
					pNewItem = pLevel.AddItem( *pItemDescription, pLevel.RandomOpenItemPosition( TargetItem()->Position(), 6 ) );
					pNewItem->Drop( pLevel );
					ResultText = LoadLocalizedString1Variable(1087, pNewItem->Name() );
				}
				Success = kTrue;
			}
			else	// we get a nasty!
			{
				Monster = pLevel.RandomUniqueMonsterIndex();

				pCharacterDescription = pLevel.CharacterManifest()->CharacterDescription( Monster );

				if( pCharacterDescription != NULL )
				{
					pCharacter = pLevel.AddMonster( *pCharacterDescription,
													pLevel.RandomOpenPosition( TargetItem()->Position(), 0, 15 ) );

					pLevel.OutfitCharacter( *pCharacterDescription,
											*pCharacter,
											kTrue );
					pSFX = new CSFXDescription( KSFXFollowTarget,
												"SPELLS\\EFFECTS\\DISPEL\\dispel.mdl",
												1,
												kTrue,
												0,
												0,
												kFalse,
												kFalse );
														   
					pLevel.AddSFX( *pSFX,
									pCharacter,
									pCharacter,
									pCharacter->Position(),
									pCharacter->Position() );
					ResultText = LoadLocalizedString1Variable(1088, pCharacter->Name() );
					DELETE_SAFELY( pSFX );

				}
				Success = kFalse;
			}
			if( Success )
			{
				PlayHelpSample( KSoundHelpFateSmiles );
			}
			pLevel.RequestMessage( ResultText );
			break;
		case KItemMagicAnvil :
			if( pItem != NULL )
			{

				
				if( pItem->Category() != KCategoryWeapon &&
					pItem->Category() != KCategoryArmor &&
					pItem->Category() != KCategoryJewelry )
				{
					ResultText = LoadLocalizedString1Variable(1089, pItem->BaseModifiedName());
					Success = kFalse;
				}
				else if( pItem->Sockets() < (uint32)pItem->SlotsTall() &&
						 RandomIntegerBetween( 0, 100 ) < 20 )
				{
					pItem->BestowSockets( kTrue );
					ResultText = LoadLocalizedString1Variable(1090, pItem->BaseModifiedName() );
					Success = kTrue;
				}
				else if( RandomIntegerBetween( 0, 100 ) < 50 &&
						 !pItem->IsArtifact() )
				{
					pItem->BestowPowers( ModifiedLevelDepth,
									     kTrue );
					pItem->SetIdentified( kTrue );
					ResultText = LoadLocalizedString1Variable(1091, pItem->BaseModifiedName() );
					Success = kTrue;
					// make SURE it got something.
					while( pItem->Effects( KActivationPassive ) == 0 &&
							pItem->DamageBonuses() == 0 )
					{
						pItem->BestowPowers( ModifiedLevelDepth,
											 kTrue );
						pItem->SetIdentified( kTrue );
					}
				}
				else if( RandomIntegerBetween( 0, 100 ) < 25 &&
						 !pItem->IsUnique() )
				{
					pItem->BestowUniquePowers( pNameGenerator,
												 ModifiedLevelDepth,
												 kFalse );
					pItem->SetIdentified( kTrue );
					ResultText = LoadLocalizedString1Variable(1092, pItem->BaseModifiedName() );
					Success = kTrue;
				}
				else if( RandomIntegerBetween( 0, 100 ) < 6 &&
						 pItem->Effects( KActivationPassive ) > 0 )
				{
					pItem->RemovePowers();
					ResultText = LoadLocalizedString1Variable(1093, pItem->BaseModifiedName() );
					Success = kFalse;
				}
				else
				{
					ResultText = LoadLocalizedString1Variable(1094, pItem->BaseModifiedName() );
					Success = kFalse;
				}
				if( Success )
				{
					if( !pItem->IsCursed() )
					{
						PlayHelpSample( KSoundHelpFateSmiles );
					}
				}
				pLevel.RequestMessage( ResultText );
				m_pInventory->CalculateEffectValues();
			}
			break;
		}
	}
	else
	{
		SetTargetItem( NULL );
		SetAIState( KAIInactive );
		return;
	}

	TargetItem()->CompleteEvent( pLevel, Success );
	SetTargetItem( NULL );
	SetAIState( KAIInactive );

} // CCharacter::CompleteEvent()

void CCharacter::OperateItemAI( float32 TimeElapsed, 	// time elapsed, in seconds
								CLevel& pLevel )		// a scene to collide with
{
	if( TargetItem() == NULL )
	{
		SetAIState( KAIInactive );
		return;
	}
	if( !TargetItem()->Operateable() )
	{
		SetAIState( KAIInactive );
		SetTargetItem( NULL );
		return;
	}

	D3DXVECTOR3 Delta = TargetItem()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( InOperationRange() &&
		!Pathing() )
	{
		// try to go up or down if it is a stairwell we have activated
		switch( TargetItem()->Type() )
		{
		case KItemStairsUp :
			pLevel.RequestLevelChange( -1 );
			break;
		case KItemStairsDown :
			pLevel.RequestLevelChange( 1 );
			break;
		case KItemTownPortal :
			pLevel.RequestLevelTeleport( 0 );
			break;
		case KItemDungeonPortal :
			pLevel.RequestLevelTeleport( m_PortalDepth );
			break;
		case KItemWellnessFountain :
			if( IsPlayer() )
			{
				PlayHelpSample( KSoundHelpRefreshed );
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 6, 0 ),
									 LoadLocalizedString(206),
									 .75f,
									 kFalse,
									 .6f,
									 kTrue  );
			}
			ModifyHP( (float32)MaxHP() );
			ModifyStamina( (float32)MaxStamina() );
			ModifyMana( (float32)MaxMana() );
			TargetItem()->CompleteEvent( pLevel, kTrue );
			RemoveEffect( "POISON" );
			break;
		case KItemHealFountain :
			if( IsPlayer() )
			{
				PlayHelpSample( KSoundHelpHealthy );
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 6, 0 ),
									 LoadLocalizedString(1095),
									 .75f,
									 kFalse,
									 .6f,
									 kTrue );
			}
			ModifyHP( (float32)MaxHP() );
			TargetItem()->CompleteEvent( pLevel, kTrue );
			RemoveEffect( "POISON" );
			break;
		case KItemManaFountain :
			if( IsPlayer() )
			{
				PlayHelpSample( KSoundHelpRefreshed );
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 6, 0 ),
									 LoadLocalizedString(1096),
									 .75f,
									 kFalse,
									 .6f,
									 kTrue  );
			}
			ModifyMana( (float32)MaxMana() );
			TargetItem()->CompleteEvent( pLevel, kTrue );
			break;
		case KItemStaminaFountain :
			if( IsPlayer() )
			{
				PlayHelpSample( KSoundHelpFortified );
				pLevel.AddEventText( this,
									 this,
									 Position() + D3DXVECTOR3( 0, 6, 0 ),
									 LoadLocalizedString(1097),
									 .75f,
									 kFalse,
									 .6f,
									 kTrue );
			}
			ModifyStamina( (float32)MaxStamina() );
			TargetItem()->CompleteEvent( pLevel, kTrue );
			break;
		case KItemStatue :
			PlayHelpSample( KSoundHelpTemptFate );
			SetAIState( KAIInEvent );
			pLevel.RequestEventMessage( LoadLocalizedString(1098),
										LoadLocalizedString(1099),
										LoadLocalizedString(1100) );
			return;
		case KItemShrine :
			SetAIState( KAIInEvent );
			PlayHelpSample( KSoundHelpTemptFate );
			pLevel.RequestEventMessage( LoadLocalizedString(1101),
										LoadLocalizedString(171),
										LoadLocalizedString(172) );
			return;
		case KItemMagicAnvil :
			PlayHelpSample( KSoundHelpTemptFate );
			SetAIState( KAIInEvent );
			pLevel.RequestEnchantMessage( LoadLocalizedString(1102),
										   TargetItem()->Position(),
										  LoadLocalizedString(815),
										  LoadLocalizedString(1029) );
			return;
			break;
		case KItemFishingHole :
			if( m_pInventory->HasItems( KItemFishingPole, 1 ) )
			{
				SetAIState( KAIFish );
				BlendAnimation( "FISHCAST",
							kFalse,
							.1f );
				QueueBlendAnimation( "FISHIDLE",
								kTrue,
								.1f );
				m_pFishingPole->PlayAnimation( "FISHCAST",
											kFalse );
				m_pFishingPole->QueueBlendAnimation( "FISHIDLE",
													kTrue,
													.1f );
				m_pCharacterSounds->PlaySample( KSoundCast,
												m_Position,
												255,
												m_SoundRadius );
			}
			else
			{
				pLevel.RequestMessage( LoadLocalizedString(1103) );
				SetAIState( KAIInactive );
			}
			return;
		}
		SetAIState( KAIInactive );
		return;
	}

	if( Distance > m_CollisionRadius )
	{

		// path, if we aren't already
		if( !Pathing() )
		{
			SetDestination( pLevel,
							TargetItem()->Position().x, 
							TargetItem()->Position().z );
			RemoveFromAvoidanceMap( pLevel );
		}
	/*	else
		{
			D3DXVECTOR3 Delta = m_pTargetItem->Position() - m_TargetPosition;
			Delta.y = 0;

			Distance = D3DXVec3Length( &Delta );
			if( Distance > m_CollisionRadius )
			{
				SetDestination( pLevel,
								m_pTargetItem->Position().x, 
								m_pTargetItem->Position().z );
				RemoveFromAvoidanceMap( pLevel );
			}
		}*/

	}

	// if pathing failed, then stop trying to get the item
	if( !Pathing() )
	{
		SetTargetItem( NULL );
		SetAIState( KAIInactive );
	}
} // CCharacter::OperateItemAI()

void CCharacter::ReturnToTownAI( float32 TimeElapsed, 	// time elapsed, in seconds
								 CLevel& pLevel )		// a scene to collide with
{
	SetRunning( kTrue );
	if( !Pathing() )
	{
		SetAIState( KAIInTown );

		return;
	}
} // CCharacter::ReturnToTownAI()

void CCharacter::FishAI( float32 TimeElapsed, 	// time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{
	for( uint32 i = 0; i < m_pModel->KeysEncountered(); i++ )
	{
		if( m_pModel->KeyEncounteredName( i ) == "CASTSPLASH" )
		{
			m_pCharacterSounds->PlaySample( KSoundCastSplash,
											m_Position,
											255,
											m_SoundRadius );
			int32 ParticleIndex = m_pParticleManager.GetParticleType( "WATERSPLASH" );
			int32 PuddleParticleIndex = m_pParticleManager.GetParticleType( "FISHPUDDLE" );
			uint32 Tags = m_pFishingPole->GetTagCount();
			for( uint32 j = 0; j < Tags; j++ )
			{
				if( StringUpper( m_pFishingPole->GetTagName( j ) ) == "FISH" )
				{
					D3DXVECTOR3 Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position,  &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					for( uint32 k = 0; k < 20; k++ )
					{
						m_pParticleManager.AddParticle( ParticleIndex, Position );
					}

					Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
				}
			}
		}
	}
	m_CaughtFish = kFalse;
	if( RandomBetween( 0, 20000 )  < 16 )
	{
		if( !m_pModel->AnimationPlaying( "FISHBITE" ) &&
			!m_pModel->AnimationPlaying( "FISHCAST" ) )
		{
			m_TimeSinceBite = 0;
			m_BiteSpeed = RandomBetween( .2f, 1.0f );
			m_pCharacterSounds->PlaySample( KSoundBiteSplash,
											m_Position,
											255,
											m_SoundRadius );

			BlendAnimation( "FISHBITE", kFalse, .1f, m_BiteSpeed );
			QueueBlendAnimation( "FISHIDLE", kTrue, .1f );

			m_pFishingPole->BlendAnimation( "FISHBITE", kFalse, .1f, m_BiteSpeed );
			m_pFishingPole->QueueBlendAnimation( "FISHIDLE", kTrue, .1f );
		}
	}
	if( m_pModel->AnimationPlaying( "FISHBITE" ) )
	{
		m_FishBiting = kTrue;
		// make water ripples during a bite
		if( RandomIntegerBetween( 0, 1000 )  < 50 )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( "FISHPUDDLE" );
			uint32 Tags = m_pFishingPole->GetTagCount();
			for( uint32 j = 0; j < Tags; j++ )
			{
				if( StringUpper( m_pFishingPole->GetTagName( j ) ) == "FISH" )
				{
					D3DXVECTOR3 Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					m_pParticleManager.AddParticle( ParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
				}
			}
		}
		m_TimeSinceBite += TimeElapsed * m_BiteSpeed;
	}		
	else if( !m_pModel->AnimationPlaying( "FISHCAST" ) )
	{
			// make idle water ripples during idle
		if( RandomIntegerBetween( 0, 1000 )  < 20 )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( "FISHPUDDLE" );
			uint32 Tags = m_pFishingPole->GetTagCount();
			for( uint32 j = 0; j < Tags; j++ )
			{
				if( StringUpper( m_pFishingPole->GetTagName( j ) ) == "FISH" )
				{
					D3DXVECTOR3 Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					m_pParticleManager.AddParticle( ParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ), .25f );
				}
			}
		}
	}
} // CCharacter::FishAI()

void CCharacter::PullFishAI( float32 TimeElapsed, 	// time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{
	for( uint32 i = 0; i < m_pModel->KeysEncountered(); i++ )
	{
		if( m_pModel->KeyEncounteredName( i ) == "PULLSPLASH" )
		{
			m_pCharacterSounds->PlaySample( KSoundPullSplash,
											m_Position,
											255,
											m_SoundRadius );
			int32 ParticleIndex = m_pParticleManager.GetParticleType( "WATERSPLASH" );
			int32 PuddleParticleIndex = m_pParticleManager.GetParticleType( "FISHPUDDLE" );
			uint32 Tags = m_pFishingPole->GetTagCount();
			for( uint32 j = 0; j < Tags; j++ )
			{
				if( StringUpper( m_pFishingPole->GetTagName( j ) ) == "FISH" )
				{
					D3DXVECTOR3 Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position,  &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					for( uint32 k = 0; k < 20; k++ )
					{
						m_pParticleManager.AddParticle( ParticleIndex, Position );
					}

					Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );

				}
			}
		}
	}
	if( m_pModel->AnimationPlaying( "IDLE" )  )
	{
		SetAIState( KAIReceiveFish );
		SetTargetItem( NULL );
	}
} // CCharacter::PullFishAI()

void CCharacter::CancelFishAI( float32 TimeElapsed, 	// time elapsed, in seconds
							   CLevel& pLevel )			// a scene to collide with
{
	for( uint32 i = 0; i < m_pModel->KeysEncountered(); i++ )
	{
		if( m_pModel->KeyEncounteredName( i ) == "PULLSPLASH" )
		{
			m_pCharacterSounds->PlaySample( KSoundPullSplash,
											m_Position,
											255,
											m_SoundRadius );
			int32 ParticleIndex = m_pParticleManager.GetParticleType( "WATERSPLASH" );
			int32 PuddleParticleIndex = m_pParticleManager.GetParticleType( "FISHPUDDLE" );
			uint32 Tags = m_pFishingPole->GetTagCount();
			for( uint32 j = 0; j < Tags; j++ )
			{
				if( StringUpper( m_pFishingPole->GetTagName( j ) ) == "FISH" )
				{
					D3DXVECTOR3 Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position,  &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					for( uint32 k = 0; k < 20; k++ )
					{
						m_pParticleManager.AddParticle( ParticleIndex, Position );
					}

					Position = m_pFishingPole->GetTagPosition( j );
					D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
					Position.y = pLevel.WaterHeight( Position );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
					m_pParticleManager.AddParticle( PuddleParticleIndex, Position, D3DXVECTOR3( 0, .001f, .00001f ) );
				}
			}
		}
	}
	if( m_pModel->AnimationPlaying( "IDLE" )  )
	{
		SetAIState( KAIInactive );
		SetTargetItem( NULL );
	}
} // CCharacter::PullFishAI()


void CCharacter::PullFish( void )
{
	if( m_pModel->AnimationPlaying( "FISHBITE" ) )
	{
		float32 Chance = 1.0f - m_TimeSinceBite;
		if( Chance < 0 )
		{
			Chance = 0;
		}
		if( RandomBetween( .25f, 1.0f ) <= Chance )
		{
			m_CaughtFish = kTrue;
			m_pFish->PlayAnimation( 0, kTrue, RandomBetween( .9f, 1.5f ) );
		}
	}
	else
	{
		m_CaughtFish = kFalse;
	}
	BlendAnimation( "FISHPULL", kFalse, .1f );
	QueueBlendAnimation( "IDLE", kTrue, .1f );

	m_pFishingPole->BlendAnimation( "FISHPULL", kFalse, .1f );
	m_pFishingPole->QueueBlendAnimation( "IDLE", kTrue, .1f );
	SetAIState( KAIPullFish );
} // CCharacter::PullFish();


void CCharacter::CancelFish( void )
{
	BlendAnimation( "FISHPULL", kFalse, .1f );
	QueueBlendAnimation( "IDLE", kTrue, .1f );

	m_pFishingPole->BlendAnimation( "FISHPULL", kFalse, .1f );
	m_pFishingPole->QueueBlendAnimation( "IDLE", kTrue, .1f );
	SetAIState( KAICancelFish );
} // CCharacter::PullFish();



void CCharacter::ApproachAI( float32 TimeElapsed, 	// time elapsed, in seconds
							 CLevel& pLevel )		// a scene to collide with
{
	if( Target() == NULL )
	{
		SetAIState( KAIInactive );
		return;
	}
	if( !Target()->IsMerchant() &&
		!Target()->IsQuestGiver() )
	{
		SetAIState( KAIInactive );
		SetTarget( NULL );
		return;
	}

	Target()->SetTarget( this );

	D3DXVECTOR3 Delta = Target()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( !Pathing() && InSpeakingRange() )
	{
		if( HasMaster() )
		{
			Target()->Greet( Master()->Position() );
		}
		else
		{
			Target()->Greet( Position() );
		}
		if( Target()->IsMerchant() )
		{
			SetAIState( KAISpeakMerchant );
			return;
		}
		else if( Target()->IsQuestGiver() )
		{
			for( uint32 i = 0; i < Quests(); i++ )
			{
				if( StringUpper( Quest( i )->GiverName() ) ==
					StringUpper( Target()->Name() ) )
				{
					SetAIState( KAICollectReward );
					return;
				}
			}
			if( MasterQuest() != NULL )
			{
				if( StringUpper( MasterQuest()->GiverName() ) ==
					StringUpper( Target()->Name() ) )
				{
					SetAIState( KAICollectReward );
					return;
				}
			}
			// we can retire after the master quest is complete
			if( MasterQuest() == NULL &&
				Target()->Name() == "Bremen" )
			{
				SetAIState( KAIRetire );
				return;
			}

			SetAIState( KAIReceiveQuest );
			return;
		}

		SetAIState( KAIWander );
		return;
	}

	if( Distance > m_CollisionRadius )
	{

		// path, if we aren't already
		if( !Pathing() )
		{
			FollowCharacter( Target(),
							 pLevel );
		}
		else
		{
			D3DXVECTOR3 Delta = Target()->Position() - m_TargetPosition;
			Delta.y = 0;

			Distance = D3DXVec3Length( &Delta );
			if( Distance > m_CollisionRadius )
			{
				FollowCharacter( Target(),
								pLevel );
			}
		}

	}

	// if pathing failed, then stop trying to get the item
	if( !Pathing() )
	{
		SetTarget( NULL );
		SetAIState( KAIWander );
	}
} // CCharacter::ApproachAI()

void CCharacter::DyingAI( float32 TimeElapsed, 	// time elapsed, in seconds
						  CLevel& pLevel )		// a scene to collide with
{
	if( !m_pModel->AnimationPlaying( m_DeathAnimationIndex ) )
	{
		CompleteDeath( pLevel );		// a scene to collide with
	}
} // CCharacter::DyingAI()

void CCharacter::CompleteDeath( CLevel& pLevel )		// a scene to collide with
{
	// force it to play a death sound
	m_HelpQueue.clear();
	PlayHelpSample( KSoundHelpDie, kTrue );
	pLevel.NotifyOfDeath( this );
	SetAIState( KAIDead );
	RemoveFromAvoidanceMap( pLevel );
	SetCollideable( kFalse );
} // CCharacter::CompleteDeath()

void CCharacter::GiveGold( int32 Gold )		// amount to give
{	
	m_pInventory->AddGold( Gold );		
} // CCharacter::GiveGold()

void CCharacter::TakeGold( uint32 Gold )	// amount to remove
{	
	m_pInventory->RemoveGold( Gold );		
} // CCharacter::TakeGold()

void CCharacter::GiveItem( CLevel& pLevel,		// a scene to drop items into
						   CItem* pItem )		// item to give
{
	if( pItem == NULL )
	{
		return;
	}
	if( m_pInventory->AddItem( pItem, pLevel ) )
	{
		m_pInventory->EquipEmpty( pLevel, kFalse );
	}
	else
	{
		if( HasMaster() &&
			Master()->IsPlayer() )
		{
			// tip about sending pets to town
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipPetFull ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipPetFull ) );
				pLevel.Tips().SetTipSeen( KTipPetFull, kTrue );
			}
		}
		else if( IsPlayer() )
		{
			PlayHelpSample( KSoundHelpPackFull );
			// tip about full inventory
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipInventoryFull ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip( KTipInventoryFull ) );
				pLevel.Tips().SetTipSeen( KTipInventoryFull, kTrue );
			}
		}
		pLevel.AddItem( pItem, m_Position );
		pItem->SetPosition( pLevel, m_Position );
		pItem->Drop( pLevel );
	}
} // CCharacter::GiveItem()

void CCharacter::NotifyOfItemReceipt( CItem* pItem,		// item we received
									  CLevel* pLevel )	// level to open items into
{
	if( pItem == NULL )
	{
		return;
	}
	if( HasMaster() )
	{
		Master()->NotifyOfItemReceipt( pItem, pLevel );
	}
	for( uint32 i = 0; i < Quests(); i++ )
	{
		Quest( i )->NotifyOfItemReceipt( pItem );
	}

	if( pLevel != NULL &&
		IsPlayer() )
	{
		if( ( pItem->Effects( KActivationPassive ) > 0 ||
			  pItem->DamageBonuses() > 0 ) &&
			!pItem->Identified() )
		{
			// tip about item identification
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel->Tips().TipSeen( KTipIdentify ) )
			{
				pLevel->RequestTip( pLevel->Tips().ContextTip( KTipIdentify ) );
				pLevel->Tips().SetTipSeen( KTipIdentify, kTrue );
			}
		}
		else if(  pItem->Type() == KItemSpell )
		{
			// tip about spells
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel->Tips().TipSeen( KTipSpell ) )
			{
				pLevel->RequestTip( pLevel->Tips().ContextTip( KTipSpell ) );
				pLevel->Tips().SetTipSeen( KTipSpell, kTrue );
			}
		}
		else if(  pItem->Type() == KItemGem )
		{
			// tip about spells
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel->Tips().TipSeen( KTipGem ) )
			{
				pLevel->RequestTip( pLevel->Tips().ContextTip( KTipGem ) );
				pLevel->Tips().SetTipSeen( KTipGem, kTrue );
			}
		}
		else if(  pItem->Type() == KItemFishingPole )
		{
			// tip about spells
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel->Tips().TipSeen( KTipFish ) )
			{
				pLevel->RequestTip( pLevel->Tips().ContextTip( KTipFish ) );
				pLevel->Tips().SetTipSeen( KTipFish, kTrue );
			}
		}
	}

} // CCharacter::NotifyOfItemReceipt()

void CCharacter::UpdateTownTimer( float32 TimeElapsed, 	// time elapsed, in seconds
								  CLevel& pLevel )		// a scene to collide with
{
	// if the character is in town, count down until their return
	if( m_TownTimer > 0 )
	{
		m_TownTimer -= TimeElapsed;
		if( m_TownTimer <= 0 ||
			pLevel.LevelDepth() == 0 )
		{
			m_TownTimer = 0;
			SetAIState( KAIInactive );
			// only pets should be doing this, but just to be safe
			if( HasMaster() )
			{
				SetPosition( pLevel.RandomOpenPosition( Master()->Position(), 25, 40 ) );
				DropToGround( pLevel );
				Master()->PlayHelpSample( KSoundHelpPetReturn, kTrue );
			}


			// sell everything that isn't equipped!


			CLList<CItemRef>* pItems = m_pInventory->ItemList();

			CLLNode<CItemRef>* Root = pItems->GetFirst();
			CLLNode<CItemRef>* Next = pItems->GetFirst();
			uint32 Index( 0 );
			bool GotGold( kFalse );
			uint32 TotalGold( 0 );
			while( Root != NULL )
			{

				Next = Root->Next();
				bool IsQuestItem( kFalse );
				if( HasMaster() )
				{
					for( uint32 i = 0; i < Master()->Quests(); i++ )
					{
						if( Master()->Quest( i )->ItemMatches( Root->Data()->m_pItem ) )
						{
							IsQuestItem = kTrue;
							break;
						}
					}
				}

				// 'sell' everything we do not have equipped.
				if( !Root->Data()->m_Equipped &&
					!IsQuestItem )
				{
					CItem* pItem = m_pInventory->RemoveItem( Root->Data()->m_pItem );
					Master()->GiveGold( pItem->SellValue() );
					GotGold = kTrue;
					TotalGold += pItem->SellValue();
					DELETE_SAFELY( pItem );
				}
				Root = Next;
			}
			if( GotGold )
			{
				if( pLevel.LevelDepth() == 0 )
				{
					pLevel.RequestMessage( LoadLocalizedString1Variable(1104, IntegerToString( TotalGold ) ));
				}
				else
				{
					pLevel.RequestMessage( LoadLocalizedString1Variable(1105, IntegerToString( TotalGold )) );
				}

				m_pCharacterSounds->PlaySample( KSoundGold,
												Master()->Position(),
												255,
												m_SoundRadius );
			}
			else
			{
				if( pLevel.LevelDepth() == 0 )
				{
					pLevel.RequestMessage( LoadLocalizedString(1106) );
				}
				else
				{
					pLevel.RequestMessage( LoadLocalizedString(1107) );
				}
			}
			ModifyHP( (float32)MaxHP() );
			RemoveEffect( "POISON" );
		}
	}
} // CCharacter::UpdateTownTimer()

void CCharacter::UpdateTimers( float32 TimeElapsed, 	// time elapsed, in seconds
							   CLevel& pLevel )			// a scene to collide with
							   
{
	if( !Alive() )
	{
		return;
	}

	m_TimeSinceIdleSound += TimeElapsed;
	if( m_TimeSinceIdleSound > 100 )
	{
		m_TimeSinceIdleSound = 100;
	}
	m_TimeSinceGreetSound += TimeElapsed;
	if( m_TimeSinceGreetSound > 100 )
	{
		m_TimeSinceGreetSound = 100;
	}

	m_TimeSinceUniqueIdle += TimeElapsed;
	if( m_TimeSinceUniqueIdle > 100 )
	{
		m_TimeSinceUniqueIdle = 100;
	}

	m_TimeSincePath += TimeElapsed;
	if( m_TimeSincePath > 100 )
	{
		m_TimeSincePath = 100;
	}

	m_TimeSinceAvoidance += TimeElapsed;
	if( m_TimeSinceAvoidance > 100 )
	{
		m_TimeSinceAvoidance = 100;
	}

	if( m_AttackTimer > 0 )
	{
		m_AttackTimer -= TimeElapsed;
		if( m_AttackTimer <= 0 )
		{
			if( !m_ReachedStrikePoint )
			{
				m_ReachedStrikePoint = kTrue;
				PerformAttack( pLevel );
			}
			m_AttackTimer = 0;
		}
	}

	if( m_CastTimer > 0 )
	{
		m_CastTimer -= TimeElapsed;
		if( m_CastTimer <= 0 )
		{
			if( !m_ReachedCastPoint )
			{
				m_ReachedCastPoint = kTrue;
				PerformCast( pLevel );
			}
			m_CastTimer = 0;
		}
	}

	// find out if summoned creature has died
	if( m_LifeDuration > 0 )
	{
		m_LifeDuration -= TimeElapsed;
		if( m_LifeDuration <= 0 )
		{
			Dispel( pLevel );
		}
	}

	if( m_TransformationDuration > 0 )
	{
		m_TransformationDuration -= TimeElapsed;
		if( m_TransformationDuration <= 0 )
		{
			m_TransformationDuration = 0;
			RevertTransformation( pLevel.D3DDevice(),
								  pLevel );
		}
	}
} // CCharacter::UpdateTimers()

void CCharacter::Dispel( CLevel& pLevel )	// level to post effects to
{
	ClearEffects();
	ClearState();
	RemoveFromAvoidanceMap( pLevel );
	StopPathing();

	// de-summon any summoned creatures
	for( uint32 i = 0; i < Pets(); i++ )
	{
		if( Pet( i )->IsSummoned() )
		{
			Pet( i )->SetLifeDuration( .01f );
		}
	}

	SetAIState( KAIDead );

	m_LifeDuration = 0;
	m_HP = 0;
	pLevel.AddEventText( this,
						 this,
						 Position() + D3DXVECTOR3( 0, 10, 0 ),
						 LoadLocalizedString(1108),
						 .75f,
						 kTrue,
						 1 );
	m_pCharacterSounds->PlaySample( KSoundDispel,
									m_Position,
									255,
									m_SoundRadius * 2 );

	CSFXDescription* pSFX = new CSFXDescription( KSFXFollowTarget,
												"SPELLS\\EFFECTS\\DISPEL\\dispel.mdl",
												1,
												kTrue,
												0,
												0,
												kFalse,
												kFalse  );
										   
	pLevel.AddSFX( *pSFX,
					this,
					this,
					Position(),
					Position() );

	DELETE_SAFELY( pSFX );
	RequestDeletion();
} // CCharacter::Dispel()

void CCharacter::UpdateAnimation( float32 TimeElapsed )	// time elapsed in seconds
{
	if( AIState() == KAIDead ||
		AIState() == KAIInTown ||
		!InActiveRange() )
	{
		return;
	}

	bool CharacterFishing( kFalse );
	// fishing is a special state where we don't render weapons/shields, 
	// but DO render the pole
	if( AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish )
	{
		CharacterFishing = kTrue;
	}

	D3DXMATRIX TransformedMatrix;


	if( m_pFishingPole != NULL )
	{
		if( AIState() == KAIFish ||
			AIState() == KAICancelFish ||
			AIState() == KAIPullFish )
		{
			m_pFishingPole->UpdateAnimation( TimeElapsed );
			if( m_CaughtFish )
			{
				m_pFish->UpdateAnimation( TimeElapsed );
			}
		}
	}

	if( m_pModel != NULL )
	{
		m_pModel->UpdateAnimation( TimeElapsed );
	}
	if( m_pArmorModel != NULL )
	{
		m_pArmorModel->CopyAnimationState( *m_pModel );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->CopyAnimationState( *m_pModel );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->CopyAnimationState( *m_pModel );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->CopyAnimationState( *m_pModel );
	}
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->UpdateAnimation( TimeElapsed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->UpdateAnimation( TimeElapsed );
	}

	// inherit xforms for attachment points
	UpdateAttachmentPoints();


	D3DXMATRIX DisplayMatrix;
	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &DisplayMatrix,
						   m_DisplayPosition.x,
						   m_DisplayPosition.y,
						   m_DisplayPosition.z );

	D3DXMATRIX ScaleMatrix;
	D3DXMatrixScaling( &ScaleMatrix, 
					   m_Scale, 
					   m_Scale, 
					   m_Scale );

	DisplayMatrix = ScaleMatrix * m_OrientationMatrix * DisplayMatrix;

	// update animated attachments
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pInventory->SlotEquipped( (EItemSlot)i ) &&
			i != KSlotNeck &&
			i != KSlotLeftFinger &&
			i != KSlotRightFinger &&
			i != KSlotBelt )
		{
			if( !CharacterFishing ||
				( i != KSlotLeftHand &&
				  i != KSlotRightHand &&
				  i != KSlotLeftArm ) )
			{

				if( m_pInventory->EquipmentAttachment( (EItemSlot)i ) != KAttachNone &&
					HasAttachment( m_pInventory->EquipmentAttachment( (EItemSlot)i ) ) )
				{
					TransformedMatrix = m_AttachmentTagXForm[m_pInventory->EquipmentAttachment( (EItemSlot)i )] * DisplayMatrix;

					m_pInventory->GetItemFromSlot( (EItemSlot)i )->SetTransformation( TransformedMatrix );
				}

				m_pInventory->GetItemFromSlot( (EItemSlot)i )->UpdateAnimation( TimeElapsed );
				
				m_pInventory->GetItemFromSlot( (EItemSlot)i )->UpdateGlows();
			}
		}
	}

	// normal animations don't apply when attacking, dying, fishing
	if( PerformingAttack() ||
		PerformingCast() ||
		AIState() == KAIDead ||
		AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish ||
		AIState() == KAIDying ||
		AIState() == KAIUniqueIdle )
	{
		return;
	}

	if( TimeElapsed == 0 )
	{
		return;
	}

	float32 PercentSpeed = (float32)EffectValue( KEffectPercentSpeed );
	if( PercentSpeed > 100 )
	{
		PercentSpeed = 0;
	}
	if( PercentSpeed < -100 )
	{
		PercentSpeed = -100;
	}
	float32 SpeedMultiplier = 1.0f + PercentSpeed / 100.0f;

	if( HasMaster() )
	{
		// reset our velocity to walking/running speed
		if( Running() )
		{
			if( RunningSpeed() > 0 &&
				RunningSpeed() < m_pMasterCharacter->RunningSpeed() + 2.0f &&
				PercentSpeed >= 0 &&
				m_CanMatchSpeed )
			{
				if( RunningSpeed() > 0 )
				{
					SpeedMultiplier *= ( Master()->RunningSpeed() + 2.0f ) / RunningSpeed();
				}
				else
				{
					SpeedMultiplier = 0;
				}
			}
		}
		else
		{
			if( WalkingSpeed() > 0 &&
				WalkingSpeed() < Master()->WalkingSpeed() + 2.0f &&
				PercentSpeed >= 0 &&
				m_CanMatchSpeed )
			{
				if( WalkingSpeed() > 0 )
				{
					SpeedMultiplier *= ( m_pMasterCharacter->WalkingSpeed() + 2.0f ) / WalkingSpeed();
				}
				else
				{
					SpeedMultiplier = 0;
				}
			}
		}
	}

	if( SpeedMultiplier < .1f )
	{
		SpeedMultiplier = .1f;
	}

	// not moving and on the ground, blend to an idle animation
	if( !Pathing() )
	{ 
		if( m_IsTurning && 
			m_pModel->AnimationExists( "WALK" ) )
		{
			if( !m_pModel->AnimationPlaying( "WALK" ) ||
				SpeedMultiplier != m_LastMovementSpeed )
			{
				BlendAnimation( "WALK", kTrue, .2f, 1.0f / m_Scale * SpeedMultiplier );
			}
		}
		else if( !m_pModel->AnimationPlaying( "IDLE" ) ||
				 SpeedMultiplier != m_LastMovementSpeed )
		{
			if( !m_pModel->AnimationQueued( "IDLE" ) ||
				SpeedMultiplier != m_LastMovementSpeed )
			{
				BlendAnimation( "IDLE", kTrue, .1f, SpeedMultiplier );
				m_pCharacterSounds->PlaySample( KSoundStep,
												m_Position,
												255,
												m_SoundRadius );
			}
		}

		// play an idle sound occasionally
		if( m_TimeSinceIdleSound > 10 )
		{
			if( rand() % 1000 < 5 )
			{
				m_pCharacterSounds->PlaySample( KSoundIdle,
												m_Position,
												255,
												m_SoundRadius );
				m_TimeSinceIdleSound = 0;
			}
		}
	}
	else	// otherwise, we're running
	{
		if( Running() )
		{
			if( m_pModel->AnimationExists( "RUN" ) )
			{
				if( !m_pModel->AnimationPlaying( "RUN" ) ||
					SpeedMultiplier != m_LastMovementSpeed )
				{
					BlendAnimation( "RUN", kTrue, .2f, 1.0f / m_Scale * SpeedMultiplier );
				}
			}
			else
			{
				if( !m_pModel->AnimationPlaying( "WALK" ) ||
					SpeedMultiplier != m_LastMovementSpeed )
				{
					BlendAnimation( "WALK", kTrue, .2f, 1.0f / m_Scale * SpeedMultiplier );
				}
			}
		}
		else
		{
			if( !m_pModel->AnimationPlaying( "WALK" ) ||
				SpeedMultiplier != m_LastMovementSpeed )
			{
				BlendAnimation( "WALK", kTrue, .2f, 1.0f / m_Scale * SpeedMultiplier );
			}

			// play an idle sound occasionally
			if( m_TimeSinceIdleSound > 8 && AIState() == KAIWander )
			{
				if( rand() % 1000 < 5 )
				{
					m_pCharacterSounds->PlaySample( KSoundIdle,
													m_Position,
													255,
													m_SoundRadius );
					m_TimeSinceIdleSound = 0;
				}
			}

		}


		if( m_pModel != NULL )
		{

			// check any animation keys that were triggered during the run
			for( uint32 i = 0; i < m_pModel->KeysEncountered(); i++ )
			{
				// play a footstep if triggered
				if( m_pModel->KeyEncounteredName( i ) == "FOOTSTEP" )
				{
					IncrementStat( KJournalStatStepsTaken );
					m_pCharacterSounds->PlaySample( KSoundStep,
													m_Position,
													255,
													m_SoundRadius * .75f );
				}
			}
		}		
	}

	m_LastMovementSpeed = SpeedMultiplier;


} // CCharacter::UpdateAnimation()

void CCharacter::UpdatePathing( float32 TimeElapsed )	// time elapsed in seconds
{
	if( Pathing() && !PerformingAttack() && !PerformingCast() )
	{
		D3DXVECTOR3 NearestPoint;
		D3DXVECTOR3 BehindNearestPoint;
		D3DXVECTOR3 Perpendicular;
		uint32		Segment;
		float32		Distance;

		D3DXVECTOR3 ClosestPosition;
		// find the nearest point to our current location
		m_pPath->FindNearestPoint( m_Position, ClosestPosition, Perpendicular, Segment,
								   Distance );
		// if we have reached the end of the path, stop pathing
		if( Distance >= m_pPath->Length() )
		{
			StopPathing();
		}
		else	// otherwise, find a point ahead of us, and turn toward it.
		{
			float32 TargetDistance = Distance + 1.0f;

			if( TargetDistance > m_pPath->Length() )
			{
				TargetDistance = m_pPath->Length();
			}
			m_TargetPosition = m_pPath->GetPositionAtDistance( TargetDistance );


			BehindNearestPoint = m_pPath->GetSplinePositionAtDistance( TargetDistance );

			TargetDistance = Distance + 5.0f;

			if( TargetDistance > m_pPath->Length() )
			{
				TargetDistance = m_pPath->Length();
			}
			NearestPoint = m_pPath->GetSplinePositionAtDistance( TargetDistance );


			D3DXVECTOR3 Temp = NearestPoint - BehindNearestPoint;
			D3DXVec3Normalize( &Temp, &Temp );
			
			// turn toward the new heading
			TurnToward( Temp, TimeElapsed);

			float32 YComponent = m_Velocity.y;
			// set our velocity along this heading
			m_Velocity = m_TargetPosition - ClosestPosition;
			m_Velocity.y = 0;
			D3DXVec3Normalize( &m_Velocity, &m_Velocity );


			m_pPath->FindNearestPoint( NearestPoint, ClosestPosition, Perpendicular, Segment,
									   TargetDistance );

			if( TargetDistance <= Distance )
			{
				StopPathing();
			}

			if( HasMaster() )
			{
				// reset our velocity to walking/running speed
				if( Running() )
				{
					if( RunningSpeed() < m_pMasterCharacter->RunningSpeed() + 2.0f &&
						EffectValue( KEffectPercentSpeed ) >= 0 &&
						m_CanMatchSpeed )
					{
						m_Velocity *= m_pMasterCharacter->RunningSpeed() + 2.0f;
					}
					else
					{
						m_Velocity *= RunningSpeed();
					}
				}
				else
				{
					if( WalkingSpeed() < m_pMasterCharacter->WalkingSpeed() + 2.0f + .5f &&
						EffectValue( KEffectPercentSpeed ) >= 0 &&
						m_CanMatchSpeed )
					{
						m_Velocity *= m_pMasterCharacter->WalkingSpeed() + 2.0f + .5f;
					}
					else
					{
						m_Velocity *= WalkingSpeed();
					}
				}
			}
			else
			{
				// reset our velocity to walking/running speed
				if( Running() )
				{
					m_Velocity *= RunningSpeed();
				}
				else
				{
					m_Velocity *= WalkingSpeed();
				}
			}

			// non-player characters damp velocity when facing away from their desired
			// direction, for smoother looking motion
			if( !IsPlayer() )
			{
				// find the angle delta between direction currently facing, 
				// and desired direction
				D3DXVECTOR3 VelocityDirection;
				D3DXVec3Normalize( &VelocityDirection, &m_Velocity );
				float32 CurrentAngle = (float32)KRad2Deg * ( (float32)atan2( m_Forward.x, m_Forward.z ) );
				float32 TargetAngle  = (float32)KRad2Deg * ( (float32)atan2( VelocityDirection.x, VelocityDirection.z ) );

				TargetAngle -= CurrentAngle;

				if (TargetAngle > 180)
				{
					TargetAngle = TargetAngle - 360;
				}
				if (TargetAngle < -180) 
				{
					TargetAngle = TargetAngle + 360;
				}
				TargetAngle = fabs( TargetAngle );

				TargetAngle /= 180;
				if( TargetAngle > .9f )
				{
					TargetAngle = .9f;
				}
				TargetAngle = 1.0f - TargetAngle;
				m_Velocity *= TargetAngle;
			}

			m_Velocity.y = YComponent;
		}
	}

} // CCharacter::UpdatePathing()

void CCharacter::UpdateMotion( float32 TimeElapsed, 	// time elapsed, in seconds
							   CLevel& pLevel )			// a scene to collide with
{
	if( AIState() == KAIDead ||
		AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish )
	{
		m_Velocity *= 0;
		return;
	}

	// if the creature fell through the world, pick some random
	// spot and reset it.
	if( m_Position.y < -100 )
	{
		// try where we are first -
		SetPosition( pLevel.RandomOpenPosition( m_Position, 5 ) );
		DropToGround( pLevel );
		// if THAT didn't work, teleport anywhere!
		if( !m_OnGround )
		{
			SetPosition( pLevel.RandomOpenPosition() );
			DropToGround( pLevel );
		}
	}
	// accelerate downward (Gravity )
	D3DXVECTOR3 Gravity = D3DXVECTOR3( 0, -300.0f, 0 );

	// we don't do knockback on impassable terrain
	if( m_OnImpassableTerrain || !Alive() )
	{
		m_KnockbackVelocity = D3DXVECTOR3( 0, 0, 0 );
	}

	bool WasOnImpassableTerrain( m_OnImpassableTerrain );
	m_OnImpassableTerrain = kFalse;

	// do NOTHING if we're not moving. Save some cycles.
	if( D3DXVec3Length( &m_Velocity ) == 0 &&
		D3DXVec3Length( &m_KnockbackVelocity ) == 0 &&
		m_OnGround )
	{

		return;
	}

	D3DXVECTOR3 CumulativeVelocity = m_Velocity * TimeElapsed + 
									m_KnockbackVelocity * TimeElapsed +
				 					( Gravity * TimeElapsed * TimeElapsed * .5f );

	D3DXVECTOR3 NextPosition = m_Position + CumulativeVelocity;
							   
	D3DXVECTOR3 ImpactPoint;
	D3DXVECTOR3 ImpactNormal;
	D3DXVECTOR3 FinalPosition;
	D3DXVECTOR3 Color;
	D3DXVECTOR3 WallImpact;
	D3DXVECTOR3 CharacterImpact;
	uint32		Material;
	CCharacter* pCharacter = NULL;

	m_Velocity += Gravity * TimeElapsed;


	bool Knockback( kFalse );
	// add dust if this is a knockback
	if( D3DXVec3Length( &m_KnockbackVelocity ) > 0 )
	{
		Knockback = kTrue;
		if( D3DXVec3Length( &m_KnockbackVelocity ) > 2 )
		{
			D3DXVECTOR3 DustVelocity( RandomBetween( -5, 5 ), 0, RandomBetween( -5, 5 ) );
			D3DXVec3Normalize( &DustVelocity, &DustVelocity );
			DustVelocity *= RandomBetween( 5.0f, 8.0f );
			m_pParticleManager.AddParticle( m_DustParticle, Position(), DustVelocity );
		}
	}

	// keep track of our last state, so we can tell if we need to do any 'landing' work
	D3DXVECTOR3 DestinationPosition( NextPosition );
	D3DXVECTOR3 StartingPosition( m_Position );
	bool WasOnGround( m_OnGround );
	m_OnGround = kFalse;
	bool StruckCharacter( kFalse );
	bool StruckWall( kFalse );
	uint32 Collisions( 0 );
	uint32 MaxCollisions( 2 );
	D3DXVECTOR3 MinBounds( m_Position );
	D3DXVECTOR3 MaxBounds( m_Position );
	ExpandBounds( MinBounds, MaxBounds, DestinationPosition );
	MinBounds.x -= m_CollisionRadius * 2;
	MinBounds.y -= m_CollisionRadius * 2;
	MinBounds.z -= m_CollisionRadius * 2;
	MaxBounds.x += m_CollisionRadius * 2;
	MaxBounds.y += m_CollisionRadius * 2;
	MaxBounds.z += m_CollisionRadius * 2;
	pLevel.SortForCollision( MinBounds, MaxBounds );

	// check to see if we collided, until we are in a non-colliding state, or have
	// exceeded 2 collisions
	while( pLevel.PreSortedSphereCollision( m_Position,
										   DestinationPosition,
										   m_CollisionRadius,
										   FinalPosition,
										   ImpactPoint,
										   ImpactNormal,
										   Material,
										   Color,
										   &pCharacter,
										   Pathing() )  &&
		   Collisions < MaxCollisions )
	{
		if( pCharacter != NULL )
		{
			CharacterImpact = ImpactPoint;
			StruckCharacter = kTrue;
			if( MaxCollisions < 5 )
			{
				MaxCollisions = 5;
			}
		}

		// find the component of our velocity in the direction of the impact normal
		float32 Dot = D3DXVec3Dot( &m_Velocity, &ImpactNormal );

		// and remove it
		m_Velocity -= ImpactNormal * Dot;

		float32 Length = D3DXVec3Length( &m_KnockbackVelocity );
		Dot = D3DXVec3Dot( &m_KnockbackVelocity, &ImpactNormal );
		if( Dot < 0 )
		{
			m_KnockbackVelocity -= ImpactNormal * Dot;
		}

		// only 'land' if the ground is under a 45 degree grade
		if( !m_OnGround && 
			ImpactNormal.y > .2f )
		{
			m_OnGround = kTrue;
			m_GroundColor = Color;
		}

		if( ImpactNormal.y < .2f ||
			Material == KImpassableMaterialID )
		{
			WallImpact = ImpactPoint;
			StruckWall = kTrue;
			MaxCollisions = 10;
			//ImpactNormal.y = 0;

			//D3DXVec3Normalize( &ImpactNormal, &ImpactNormal );

			// remove the normal component of knockback velocity


			if( Material == KImpassableMaterialID &&
				ImpactNormal.y > .7f )
			{
				//ImpactNormal = -CumulativeVelocity;
				//D3DXVec3Normalize( &ImpactNormal, &ImpactNormal );
			}
			Length = D3DXVec3Length( &CumulativeVelocity );
			Dot = D3DXVec3Dot( &CumulativeVelocity, &ImpactNormal );
			if( Dot > 0 )
			{
				ImpactNormal = D3DXVECTOR3( 0, 0, 0 );
			}
			else
			{
				NextPosition = ImpactPoint + ImpactNormal * m_CollisionRadius;
			}



			D3DXVECTOR3 NewVelocity = CumulativeVelocity - ImpactNormal * Dot;
			D3DXVec3Normalize( &NewVelocity, &NewVelocity );
			NewVelocity *= Length;
			DestinationPosition = StartingPosition + NewVelocity;
			if( Collisions < MaxCollisions - 1 )
			{
				m_Position = FinalPosition;
				NextPosition = DestinationPosition;
			}
		}
		else
		{
			// set our next position to the impact point;
			NextPosition = FinalPosition;
		}

		if( ImpactNormal.y > .7f &&
			Material == KImpassableMaterialID )
		{
			m_OnImpassableTerrain = kTrue;
		}

		Collisions++;
	}


	if( !m_OnGround )
	{
		if( pLevel.PreSortedRayCollision( m_Position,
										 m_Position - D3DXVECTOR3( 0, m_CollisionRadius, 0 ),
										 ImpactPoint,
										 ImpactNormal,
										 Material,
										 Color,
										 NULL,
										 kFalse )  )
		{
			m_OnGround = kTrue;
			if( ImpactNormal.y > .7f &&
				Material == KImpassableMaterialID )
			{
				m_OnImpassableTerrain = kTrue;
			}
		}
	}

	if( m_OnImpassableTerrain &&
		!WasOnImpassableTerrain )
	{
		NextPosition = StartingPosition;
	}

	if( m_OnGround )
	{
		// and not supposed to be moving
		// then DON'T move. Prevents sliding on the ground
		if( ( !Pathing() &&
			  !Knockback ) ||
			  ( WalkingSpeed() == 0 && RunningSpeed() == 0 ) )
		{		
			NextPosition.x = StartingPosition.x;
			NextPosition.z = StartingPosition.z;
			if( WasOnGround )
			{
				NextPosition.y = StartingPosition.y;
			}
		}
	}
	
	// if we didn't collide with anything, we're not on the ground
	if( Collisions == 0)
	{
		m_OnGround = kFalse;
	}
	else
	{

		// if we are pathing, and a character is in our way
		if( StruckCharacter &&
			Pathing() )
		{
			// first see if they are standing in our destination!
			D3DXVECTOR3 Delta = pCharacter->Position() - m_FinalTargetPosition;
			Delta.y = 0;
			// if they are, just stop pathing
			if( D3DXVec3Length( &Delta ) <= m_CollisionRadius )
			{
				StopPathing();
			}
			else if( m_TimeSinceAvoidance > m_RepathPauseTime )	// otherwise, try to go around!
			{
				m_TimeSinceAvoidance = 0;
				//pLevel.IncrementObjectPassability( CharacterImpact );
				SetDestination( pLevel, m_FinalTargetPosition.x, m_FinalTargetPosition.z );
				RemoveFromAvoidanceMap( pLevel );
				//pLevel.DecrementObjectPassability( CharacterImpact );
			}
		}
		else if( StruckWall &&
				 Pathing() )	// otherwise, if hitting a wall, see if we're near the end and stop if so
		{
			// first see if they are standing in our destination!
			D3DXVECTOR3 Delta = Position() - m_FinalTargetPosition;
			Delta.y = 0;
			// if they are, just stop pathing
			if( D3DXVec3Length( &Delta ) <= m_CollisionRadius * 3.0f )
			{
				StopPathing();
			}
			else if( m_TimeSinceAvoidance > m_RepathPauseTime )
			{
				m_TimeSinceAvoidance = 0;
				pLevel.IncrementObjectPassability( WallImpact );
				SetDestination( pLevel, m_FinalTargetPosition.x, m_FinalTargetPosition.z );
				RemoveFromAvoidanceMap( pLevel );
				pLevel.DecrementObjectPassability( WallImpact );
				if( !m_pAStar->WasPerfectPath() )
				{
					StopPathing();
				}
			}
		}
	}
	
	m_Position = NextPosition;
	// since the pivot of the character is at the feet, we need to offset them downward by their collision radius
	m_DisplayPosition = m_Position - D3DXVECTOR3( 0, m_CollisionRadius, 0 );

	ExtractOrientationVectors();


	// if we're on the ground, clear out velocity -
	// it will be recalculated next frame based upon heading
	if( m_OnGround )
	{
		m_Velocity *= 0;
	}
	else // if in air, we only want to clear out perpendicular velocity
	{
		m_Velocity.x = 0;
		m_Velocity.z = 0;
	}

	m_MinCollisionBounds = m_DisplayPosition + 
						   m_pCollisionModel->GetMinBounds() * 2;
	m_MaxCollisionBounds = m_DisplayPosition + 
						   m_pCollisionModel->GetMaxBounds() * 2;
} // CCharacter::UpdateMotion()

void CCharacter::DropToGround( CLevel& pLevel )			// a scene to collide with
{

	D3DXVECTOR3 NextPosition( m_Position );

	m_Position.y = 50;
	NextPosition.y = -50;

	D3DXVECTOR3 ImpactPoint;
	D3DXVECTOR3 ImpactNormal;
	D3DXVECTOR3 FinalPosition;
	D3DXVECTOR3 Color;
	uint32 Material;

	m_OnGround = kFalse;


	if( pLevel.RayCollision( m_Position,
							 NextPosition,
							 ImpactPoint,
							 ImpactNormal,
							 Material,
							 Color,
							 NULL,
							 kFalse  ) )
	{
		m_OnGround = kTrue;

		// set our next position to the impact point;
		m_Position = ImpactPoint;
		m_Position.y += m_CollisionRadius;
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
								kFalse  ) )
		{
			m_OnGround = kTrue;

			// set our next position to the impact point;
			m_Position = ImpactPoint;
			m_Position.y += m_CollisionRadius;

		}
		else
		{
			m_Position.y = pLevel.FloorHeight( m_Position.x, m_Position.z );
			m_Position.y += m_CollisionRadius;
		}
	}
	// since the pivot of the character is at the feet, we need to offset them downward by their collision radius
	m_DisplayPosition = m_Position - D3DXVECTOR3( 0, m_CollisionRadius, 0 );

	m_MinCollisionBounds = m_DisplayPosition + 
						   m_pCollisionModel->GetMinBounds() * 2;
	m_MaxCollisionBounds = m_DisplayPosition + 
						   m_pCollisionModel->GetMaxBounds() * 2;

	ExtractOrientationVectors();
	EmitParticles();	// do this to make sure glows are positioned
} // CCharacter::DropToGround()


// extract forward, up, and right from the orientation matrix
void CCharacter::ExtractOrientationVectors( void )
{
	m_Forward.x = m_OrientationMatrix._31;
	m_Forward.y = m_OrientationMatrix._32;
	m_Forward.z = m_OrientationMatrix._33;

	m_Up.x = m_OrientationMatrix._21;
	m_Up.y = m_OrientationMatrix._22;
	m_Up.z = m_OrientationMatrix._23;

	m_Right.x = m_OrientationMatrix._11;
	m_Right.y = m_OrientationMatrix._12;
	m_Right.z = m_OrientationMatrix._13;

} // CCharacter::ExtractOrientationVectors()

void CCharacter::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						 CLightManager* pLightManager,			// light manager - NULL for no light manager
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						 CFrustum* pFrustum )					// frustum to cull with - NULL for none

{

	// no model, no render
	if( m_pModel == NULL ||
		AIState() == KAIInTown )
	{
		return;
	}

	
	D3DXMATRIX TransformedMatrix;
	
	D3DXMATRIX ScaleMatrix;

	D3DXMatrixScaling( &ScaleMatrix, 
					   m_Scale, 
					   m_Scale, 
					   m_Scale );

	// we have to normalize the normals of scaled meshes for proper lighting - which sucks.
	if( m_Scale != 1 )
	{
		pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, kTrue );
	}

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_DisplayMatrix,
						   m_DisplayPosition.x,
						   m_DisplayPosition.y,
						   m_DisplayPosition.z );

	m_DisplayMatrix = ScaleMatrix * m_OrientationMatrix * m_DisplayMatrix;
	TransformedMatrix = m_DisplayMatrix;


	if( pFrustum != NULL )
	{
		m_pCullingBounds->Update( TransformedMatrix );
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

	SetOnscreen( kTrue );

	UpdateLocalLight();

	D3DXVECTOR3 NormalLight( 0, -1, 0 );

	if( pLightManager != NULL )
	{
		pLightManager->UpdateLights( pD3DDevice, TransformedMatrix );

		D3DXMATRIX InverseTransformation;
		D3DXMatrixInverse( &InverseTransformation, NULL, &TransformedMatrix );

		D3DXVec3TransformNormal( &NormalLight, &pLightManager->LightDirection(), &InverseTransformation );

	}

	TransformedMatrix *= ViewportMatrix;

	if( m_pInventory->SlotEquipped( KSlotFeet ) )
	{
		LoadBoots( pD3DDevice,
				   m_pInventory->GetItemFromSlot( KSlotFeet )->AnimatedModelPath() );
	}

	bool Fishing( kFalse );
	// fishing is a special state where we don't render weapons/shields, 
	// but DO render the pole
	if( AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish )
	{
		Fishing = kTrue;
	}

	if( m_pInventory->SlotEquipped( KSlotFeet ) &&
		m_pBootsModel != NULL )
	{
		m_pBootsModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
	}
	else
	{

		if( m_pOverrideMaterial != NULL )
		{
			CMaterial* pOriginalMaterial = NULL;
			// override original material
			pOriginalMaterial = m_pModel->GetMaterialByIndex( 0 );
			m_pModel->SetMaterial( 0, m_pOverrideMaterial );

			m_pModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
			if( m_MouseOver )
			{
				pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
				m_pModel->RenderIlluminated( pD3DDevice, TransformedMatrix, NormalLight, kFalse );
				pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
			}
			// then put it back
			m_pModel->SetMaterial( 0, pOriginalMaterial );
		}
		else
		{
			m_pModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
			if( m_MouseOver )
			{
				pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
				m_pModel->RenderIlluminated( pD3DDevice, TransformedMatrix, NormalLight, kFalse );
				pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
			}
		}
	}
	if( Fishing )
	{
		m_pFishingPole->Render( pD3DDevice, TransformedMatrix, NormalLight );

		if( m_CaughtFish )
		{
			uint32 Tags = m_pFishingPole->GetTagCount();
			for( uint32 i = 0; i < Tags; i++ )
			{
				if( StringUpper( m_pFishingPole->GetTagName( i ) ) == "FISH" )
				{
					D3DXMATRIX XForm = m_pFishingPole->GetTransformedTag( i );
					XForm *= m_DisplayMatrix;
					XForm *= ViewportMatrix;
					m_pFish->Render( pD3DDevice, XForm, NormalLight );
				}
			}
		}

	}

	if( m_pInventory->SlotEquipped( KSlotChest ) )
	{
		LoadArmor( pD3DDevice,
				   m_pInventory->GetItemFromSlot( KSlotChest )->AnimatedModelPath() );
	}
	if( m_pInventory->SlotEquipped( KSlotChest ) &&
		m_pArmorModel != NULL )
	{
		m_pArmorModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
	}
	else if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
	}
	if( m_pInventory->SlotEquipped( KSlotGloves ) )
	{
		LoadGloves( pD3DDevice,
					m_pInventory->GetItemFromSlot( KSlotGloves )->AnimatedModelPath() );
		if( m_pGlovesModel != NULL )
		{
			m_pGlovesModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
		}
	}
	if( HasAttachment( KAttachHead ) )
	{
		TransformedMatrix = m_AttachmentTagXForm[KAttachHead] * m_DisplayMatrix;
		pLightManager->UpdateLights( pD3DDevice, TransformedMatrix );

		TransformedMatrix *= ViewportMatrix;
		if( m_pHeadModel != NULL )
		{
			m_pHeadModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
		}
		if( !m_pInventory->SlotEquipped( KSlotHead ) && 
			 m_pHairModel != NULL )
		{
			m_pHairModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
		}
	}

	// render attachments
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pInventory->SlotEquipped( (EItemSlot)i ) &&
			i != KSlotNeck &&
			i != KSlotLeftFinger &&
			i != KSlotRightFinger &&
			i != KSlotBelt )
		{
			if( !Fishing ||
				( i != KSlotLeftHand &&
				  i != KSlotRightHand &&
				  i != KSlotLeftArm ) )
			{
				if( m_pInventory->EquipmentAttachment( (EItemSlot)i ) != KAttachNone &&
					HasAttachment( m_pInventory->EquipmentAttachment( (EItemSlot)i ) ) )
				{
					TransformedMatrix = m_AttachmentTagXForm[m_pInventory->EquipmentAttachment( (EItemSlot)i )] * m_DisplayMatrix;
					pLightManager->UpdateLights( pD3DDevice, TransformedMatrix );

					//m_pInventory->GetItemFromSlot( (EItemSlot)i )->SetTransformation( TransformedMatrix );

					TransformedMatrix *= ViewportMatrix;
					m_pInventory->GetItemFromSlot( (EItemSlot)i )->Model()->Render( pD3DDevice, TransformedMatrix, NormalLight );

				}
			}
		}
	}

	// we have to normalize the normals of scaled meshes for proper lighting - which sucks.
	if( m_Scale != 1 )
	{
		pD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, kFalse );
	}

	m_MouseOver = kFalse;
} // CCharacter::Render()

void CCharacter::RenderBehindWall( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									CLightManager* pLightManager,			// light manager - NULL for no light manager
									const D3DXMATRIX& ViewportMatrix,		// viewport matrix
									CFrustum* pFrustum )					// frustum to cull with - NULL for none

{
	if( !Onscreen() )
	{
		return;
	}

	if( !Alive()  ||
		AIState() == KAIInTown )
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

	if( m_PathNodeAlignment != Alignment() )
	{
		UpdatePathNode( pD3DDevice );
	}

	D3DXVECTOR3 Light = m_LocalLight;
	Light.x -= .3f;
	Light.y -= .3f;
	Light.z -= .3f;

	if( Light.x <= 0 )
	{
		return;
	}
	D3DXMATRIX	Translation;


	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &Translation,
						   Position().x,
						   Position().y,
						   Position().z );
	for( uint32 i = 0; i < m_pPathNodeModel->GetMaterials(); i++ )
	{
		CMaterial*	Material = m_pPathNodeModel->GetMaterialByIndex( i );
		if( Material != NULL )
		{
			Material->D3DMaterial().Diffuse.r = Light.x;
			Material->D3DMaterial().Diffuse.g = Light.y;
			Material->D3DMaterial().Diffuse.b = Light.z;
			pD3DDevice->SetTexture( 0, Material->GetIlluminationTexture() );
		}
	}


	m_pPathNodeModel->RenderNoTexture( pD3DDevice, m_OrientationMatrix * Translation * ViewportMatrix );

} // CCharacter::RenderBehindWall()

void CCharacter::RenderShadow( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   CFrustum* pFrustum )						// frustum to cull with - NULL for none

{

	if( !Onscreen() )
	{
		return;
	}

	// no model, no render
	if( m_pModel == NULL  ||
		AIState() == KAIInTown )
	{
		return;
	}

	
	D3DXMATRIX TransformedMatrix;
	D3DXMATRIX ScaleMatrix;

	D3DXMatrixScaling( &ScaleMatrix, 
					   m_Scale, 
					   m_Scale, 
					   m_Scale );
	
	// since the pivot of the character is at the feet, we need to offset them downward by their collision radius
	D3DXVECTOR3	DisplayLocation = m_Position - m_Up * m_CollisionRadius  + m_Up * m_VerticalOffset;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_DisplayMatrix,
						   DisplayLocation.x,
						   DisplayLocation.y,
						   DisplayLocation.z );

	m_DisplayMatrix = ScaleMatrix * m_OrientationMatrix * m_DisplayMatrix;
	TransformedMatrix = m_DisplayMatrix;

	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

	bool Fishing( kFalse );
	// fishing is a special state where we don't render weapons/shields, 
	// but DO render the pole
	if( AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish )
	{
		Fishing = kTrue;
	}

	TransformedMatrix *= ViewportMatrix;

	m_pModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	if( Fishing )
	{
		m_pFishingPole->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}

	if( m_pInventory->SlotEquipped( KSlotChest ) &&
		m_pArmorModel != NULL )
	{
		m_pArmorModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}
	else if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}

	if( HasAttachment( KAttachHead ) )
	{
		TransformedMatrix = m_AttachmentTagXForm[KAttachHead] * m_DisplayMatrix;

		TransformedMatrix *= ViewportMatrix;
		if( m_pHeadModel != NULL )
		{
			m_pHeadModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
		}
		if( !m_pInventory->SlotEquipped( KSlotHead ) && 
			 m_pHairModel != NULL )
		{
			m_pHairModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
		}
	}
	// render attachments
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pInventory->SlotEquipped( (EItemSlot)i ) &&
			i != KSlotNeck &&
			i != KSlotLeftFinger &&
			i != KSlotRightFinger &&
			i != KSlotBelt )
		{
			if( !Fishing ||
				( i != KSlotLeftHand &&
				  i != KSlotRightHand &&
				  i != KSlotLeftArm ) )
			{
				if( m_pInventory->EquipmentAttachment( (EItemSlot)i ) != KAttachNone &&
					HasAttachment( m_pInventory->EquipmentAttachment( (EItemSlot)i ) ) )
				{
					TransformedMatrix = m_AttachmentTagXForm[m_pInventory->EquipmentAttachment( (EItemSlot)i )] * m_DisplayMatrix;

					TransformedMatrix *= ViewportMatrix;
					m_pInventory->GetItemFromSlot( (EItemSlot)i )->Model()->RenderNoTexture( pD3DDevice, TransformedMatrix );
				}
			}
		}
	}

} // CCharacter::RenderShadow()

void CCharacter::RenderPath( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{
	if( !Pathing()  ||
		AIState() == KAIInTown )
	{
		return;
	}
	D3DXMATRIX	Translation;

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );

	for( uint32 i = 0; i < m_pPath->Points(); i++ )
	{
		D3DXVECTOR3 Position = m_pPath->GetPoint( i );
		// calculate the translation matrix based upon our location
		D3DXMatrixTranslation( &Translation,
							   Position.x,
							   Position.y,
							   Position.z );

		m_pPathNodeModel->Render( pD3DDevice, Translation * ViewportMatrix );
	}

	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );

}

void CCharacter::UpdateLocalLight( void )
{

	if( m_pOverrideMaterial != NULL )
	{
		m_pOverrideMaterial->D3DMaterial().Diffuse.r = m_LocalLight.x;
		m_pOverrideMaterial->D3DMaterial().Diffuse.g = m_LocalLight.y;
		m_pOverrideMaterial->D3DMaterial().Diffuse.b = m_LocalLight.z;
	}
	// set local ambient light, based upon distance from camera
	if( m_pBootsModel != NULL )
	{
		// set local ambient light, based upon distance from camera
		for( uint32 i = 0; i < m_pBootsModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pBootsModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}

	{
		for( uint32 i = 0; i < m_pModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}

	if( m_pArmorModel != NULL )
	{
		// set local ambient light, based upon distance from camera
		for( uint32 i = 0; i < m_pArmorModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pArmorModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}
	if( m_pUpperBodyModel != NULL )
	{
		// set local ambient light, based upon distance from camera
		for( uint32 i = 0; i < m_pUpperBodyModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pUpperBodyModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}
	if( m_pGlovesModel != NULL )
	{
		// set local ambient light, based upon distance from camera
		for( uint32 i = 0; i < m_pGlovesModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pGlovesModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}
	if( m_pHeadModel != NULL )
	{
		// set local ambient light, based upon distance from camera
		for( uint32 i = 0; i < m_pHeadModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pHeadModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}
	if( m_pHairModel != NULL )
	{
		// set local ambient light, based upon distance from camera
		for( uint32 i = 0; i < m_pHairModel->GetMaterials(); i++ )
		{
			CMaterial*	Material = m_pHairModel->GetMaterialByIndex( i );
			if( Material != NULL )
			{
				Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
				Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
				Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
			}
		}
	}

	// render attachments
	for( uint32 j = 0; j < KEquipmentSlots; j++ )
	{
		if( m_pInventory->SlotEquipped( (EItemSlot)j ) &&
			j != KSlotNeck &&
			j != KSlotLeftFinger &&
			j != KSlotRightFinger &&
			j != KSlotBelt )
		{
			if( m_pInventory->EquipmentAttachment( (EItemSlot)j ) != KAttachNone &&
				HasAttachment( m_pInventory->EquipmentAttachment( (EItemSlot)j ) ) )
			{
				CGenericModel* pModel = m_pInventory->GetItemFromSlot( (EItemSlot)j )->Model();
				// set local ambient light, based upon distance from camera
				for( uint32 i = 0; i < pModel->GetMaterials(); i++ )
				{
					CMaterial*	Material = pModel->GetMaterialByIndex( i );
					if( Material != NULL )
					{
						Material->D3DMaterial().Diffuse.r = m_LocalLight.x * .5f;
						Material->D3DMaterial().Diffuse.g = m_LocalLight.y * .5f;
						Material->D3DMaterial().Diffuse.b = m_LocalLight.z * .5f;
					}
				}

			}
		}
	}
} // CCharacter::UpdateLocalLight()


void CCharacter::CalculateLocalLight( bool Torchlit,					//  is the level torchlit?
									  float32 TimeElapsed,				// for smoothing
									  const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
									  float32 MaximumDistance )			// maximum distance from camera

{
	if( !m_Onscreen )
	{
		return;
	}

	if( !Torchlit )
	{
		float32 DeltaTime = TimeElapsed * 5.0f;

		D3DXVECTOR3 Delta = m_GroundColor - m_LocalLight;
		if( DeltaTime > 1 )
		{
			DeltaTime = 1;
		}
		Delta *= DeltaTime;
		m_LocalLight += Delta;
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

} // CCharacter::CalculateLocalLight()

void CCharacter::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							 const D3DXVECTOR3& LightDirection )	// direction the light is coming from
{
	if( !Onscreen() )
	{
		return;
	}

	// no model, no render
	if( m_pModel == NULL ||
		AIState() == KAIInTown )
	{
		return;
	}

		
	D3DXMATRIX InverseMatrix;
	D3DXVECTOR3 LocalLightDirection = LightDirection;
	D3DXMatrixInverse( &InverseMatrix, NULL, &m_OrientationMatrix );
	D3DXVec3TransformNormal( &LocalLightDirection, &LocalLightDirection, &InverseMatrix );

	D3DXMATRIX TransformedMatrix;

	// since the pivot of the character is at the feet, we need to offset them downward by their collision radius
	D3DXVECTOR3	DisplayLocation = m_Position - m_Up * m_CollisionRadius + m_Up * m_VerticalOffset;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &TransformedMatrix,
						   DisplayLocation.x,
						   DisplayLocation.y,
						   DisplayLocation.z );

	TransformedMatrix = m_OrientationMatrix * TransformedMatrix;
	TransformedMatrix *= ViewportMatrix;


	m_pModel->RenderShadows( pD3DDevice, TransformedMatrix, LocalLightDirection );
} // CCharacter::RenderShadows()

void CCharacter::SetPosition( const D3DXVECTOR3& Position )			// position to place character at
{
	m_Position = Position;	
	m_TargetPosition = Position;
	m_DisplayPosition = m_Position - D3DXVECTOR3( 0, m_CollisionRadius, 0 );

	m_MinCollisionBounds = m_DisplayPosition + 
						   m_pCollisionModel->GetMinBounds() * 2;
	m_MaxCollisionBounds = m_DisplayPosition + 
						   m_pCollisionModel->GetMaxBounds() * 2;

} // CCharacter::SetPosition()

void CCharacter::SetDestination( CLevel& pLevel,// a scene to collide with
								 float32 X,		// worldspace x position
								 float32 Y )	// worldspace y position
{
	if(	AIState() == KAIDead ||
		AIState() == KAIDying ||
		AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish ||
		( WalkingSpeed() == 0 && RunningSpeed() == 0 ) ||
		m_HoldPosition )
	{
		StopPathing();
		if( m_HoldPosition )
		{
			Attack();
		}
		m_FinalTargetPosition = D3DXVECTOR3( X, 0, Y );
		return;
	}

	m_TimeSincePath = 0;
	m_TimeSinceAvoidance = 0;

	bool WasOnAvoidanceMap( OnAvoidanceMap() );
	RemoveFromAvoidanceMap( pLevel );

	float32 OriginalY( Y );
	m_FinalTargetPosition = D3DXVECTOR3( X, 0, Y );
	if( !m_pAStar->FindPath( m_Position.x, m_Position.z, X, Y ) )
	{
		if( WasOnAvoidanceMap )
		{
			AddToAvoidanceMap( pLevel );
		}
		return;
	}

	if( !m_pAStar->ReachedGoal() )
	{
		m_pPath->Clear();
		D3DXVECTOR3 CurrentPoint( 0, 0, 0 );
		D3DXVECTOR3 NextPoint( 0, 0, 0 );
		// throw out that first worthless node
		m_pAStar->AdvanceNextNode();
		//m_pPath->AddPoint( m_Position );
		while( !m_pAStar->ReachedGoal() )
		{
			bool Added( kFalse );
			NextPoint.x = m_pAStar->GetNodeX();
			NextPoint.z = m_pAStar->GetNodeY();
			if( m_pPath->Points() == 0 )
			{
				Added = kTrue;
				CurrentPoint = m_Position;
				m_pPath->AddPoint( CurrentPoint );
			}
			else if( !pLevel.PassableBetween( CurrentPoint, NextPoint ) )
			{
				Added = kTrue;
				CurrentPoint = NextPoint;
				m_pPath->AddPoint( CurrentPoint );
			}
			m_pAStar->AdvanceNextNode();
			if( m_pAStar->ReachedGoal() && !Added )
			{
				if( !pLevel.PassableBetween( CurrentPoint, m_FinalTargetPosition ) ||
					!pLevel.PositionPassable( m_FinalTargetPosition ) )
				{
					m_pPath->AddPoint( NextPoint );
				}
			}
		}
		if( m_pPath->Points() > 0 )
		{
			// if the actual location we clicked is passable, and reachable, then set
			// it as the last destination
			if( pLevel.PositionPassable( m_FinalTargetPosition ) &&
				pLevel.PassableBetween( m_pPath->GetPoint( m_pPath->Points() - 1 ), m_FinalTargetPosition ) )
			{
				m_pPath->AddPoint( D3DXVECTOR3( X, 0, Y ) );
				D3DXVECTOR3 Delta = m_FinalTargetPosition - m_pPath->GetPoint( m_pPath->Points() - 1 );
				if( D3DXVec3Length( &Delta ) < pLevel.TileWidth() )
				{
					m_pPath->AddPoint( D3DXVECTOR3( X, 0, Y ) );
				}
			}
			else
			{
				m_FinalTargetPosition = m_pPath->GetPoint( m_pPath->Points() - 1 );
			}
			m_Pathing = kTrue;
			m_TargetPosition = m_pPath->GetPoint( 0 );
		}
	}
	if( m_pPath->Length() <= 0 )
	{
		m_Pathing = kFalse;
	}
	if( WasOnAvoidanceMap )
	{
		AddToAvoidanceMap( pLevel );
	}
} // CCharacter::SetDestination()

void CCharacter::StopPathing( void )
{
	m_Pathing = kFalse;
	m_TargetPosition = m_Position;
} // CCharacter::StopPathing()


void CCharacter::RemoveFromAvoidanceMap( CLevel& pLevel )	// level which has the avoidance map
{

	if( m_OnAvoidanceMap )
	{
		for( uint32 i = 0; i < KMaximumPossibleSummonedCreatures; i++ )
		{
			m_PetWasCollideable[i] = kFalse;
		}
		// we don't want our pet to obstruct our motion
		if( HasPets() && !m_MinionsCollideable )
		{
			for( uint32 i = 0; i < Pets(); i++ )
			{
				m_PetWasCollideable[i] = m_pPetCharacters[i]->OnAvoidanceMap();
				m_pPetCharacters[i]->RemoveFromAvoidanceMap( pLevel );
			}
		}
		
		float32 StepSize = pLevel.PassabilityCellWidth();
		for( float32 x = m_Position.x - StepSize * 2.0f; x <= 
			 m_Position.x + StepSize * 2.0f; x+= StepSize )
		{
			for( float32 y = m_Position.z - StepSize * 2.0f; y <= 
				 m_Position.z + StepSize * 2.0f; y+= StepSize )
			{
				pLevel.DecrementObjectPassability( D3DXVECTOR3( x, 0, y ) );
			}
		}

		m_OnAvoidanceMap = kFalse;
	}
} // CCharacter::RemoveFromAvoidanceMap()

void CCharacter::AddToAvoidanceMap( CLevel& pLevel )	// level which has the avoidance map
{
	if( !m_OnAvoidanceMap && Alive() )
	{
		// we don't want our pet to obstruct our motion
		if( HasPets() && !m_MinionsCollideable )
		{
			for( uint32 i = 0; i < Pets(); i++ )
			{
				if( m_PetWasCollideable[i] )
				{
					m_pPetCharacters[i]->AddToAvoidanceMap( pLevel );
				}
			}
		}

		float32 StepSize = pLevel.PassabilityCellWidth();
		for( float32 x = m_Position.x - StepSize * 2.0f; x <= 
			 m_Position.x + StepSize * 2.0f; x+= StepSize )
		{
			for( float32 y = m_Position.z - StepSize * 2.0f; y <= 
				 m_Position.z + StepSize * 2.0f; y+= StepSize )
			{
				pLevel.IncrementObjectPassability( D3DXVECTOR3( x, 0, y ) );
			}
		}

		m_OnAvoidanceMap = kTrue;
	}
} // CCharacter::AddToAvoidanceMap()

void CCharacter::TurnTowardPosition( const D3DXVECTOR3& TargetPosition,	// Position to face
									 float32 TimeElapsed )				// time elapsed in seconds
{
	D3DXVECTOR3 Delta = TargetPosition - m_Position;
	D3DXVec3Normalize( &Delta, &Delta );

	TurnToward( Delta, TimeElapsed );
} // CCharacter::TurnTowardPosition()

void CCharacter::TurnToward( const D3DXVECTOR3& TargetDirection,	// direction to face
							 float32 TimeElapsed )					// time elapsed in seconds
{
	D3DXMATRIX  RotationMatrix;

	
	// find the angle delta between direction currently facing, 
	// and desired direction
	float32 CurrentAngle = (float32)KRad2Deg * ( (float32)atan2( m_Forward.x, m_Forward.z ) );
	float32 TargetAngle  = (float32)KRad2Deg * ( (float32)atan2( TargetDirection.x, TargetDirection.z ) );

	TargetAngle -= CurrentAngle;

	// make sure this is halved into left/right turns
	if (TargetAngle > 180)
	{
		TargetAngle = TargetAngle - 360;
	}
	if (TargetAngle < -180) 
	{
		TargetAngle = TargetAngle + 360;
	}
	
	// detect that we are turning
	if( fabs( TargetAngle ) > 5 )
	{
		m_IsTurning = kTrue;
	}

	float32 DegreesToTurn( 0 );


	// turn faster the further from the destination angle we are
	if( TargetAngle < 0 )
	{
		float32 Turn = -m_TurnRate * fabs( TargetAngle );
		if( Turn < -KMaxTurnRate )
		{
			Turn = -KMaxTurnRate;
		}
		DegreesToTurn = Turn * TimeElapsed;
		if( DegreesToTurn < TargetAngle )
		{
			DegreesToTurn = TargetAngle;
		}
	}
	else if( TargetAngle > 0 )
	{
		float32 Turn = m_TurnRate * fabs( TargetAngle );
		if( Turn > KMaxTurnRate )
		{
			Turn = KMaxTurnRate;
		}
		DegreesToTurn = Turn * TimeElapsed;
		if( DegreesToTurn > TargetAngle )
		{
			DegreesToTurn = TargetAngle;
		}

	}

	float32 PercentSpeed = (float32)EffectValue( KEffectPercentSpeed );
	if( PercentSpeed > 100 )
	{
		PercentSpeed = 100;
	}

	float32 SpeedMultiplier = 1.0f + PercentSpeed / 100.0f;

	// we cap the slowest turn rate to .5
	if( SpeedMultiplier < .5f )
	{
		SpeedMultiplier = .5f;
	}
	DegreesToTurn = (float32)KDeg2Rad * DegreesToTurn * SpeedMultiplier;
	DegreesToTurn = VerifyFloat( DegreesToTurn, (float32)KDeg2Rad * 1.0f );
	// perform rotation about the Y axis
	D3DXMatrixRotationY( &RotationMatrix, DegreesToTurn );

	m_OrientationMatrix *= RotationMatrix;

	ExtractOrientationVectors();

} // CCharacter::TurnToward()


bool CCharacter::HasReflection( void )
{
	return m_pModel->HasReflection();
} // CCharacter::HasReflection()

bool CCharacter::HasCubeMap( void )
{
	return m_pModel->HasCubeMap();
} // CCharacter::HasCubeMap()

bool CCharacter::InFrustum( CFrustum& pFrustum )					// frustum to cull with - NULL for none

{
	// no model, no render
	if( m_pModel == NULL)
	{
		return kFalse;
	}

	// if we're not in the frustum, don't render at all
	if( pFrustum.Intersects( *m_pCullingBounds ) == KFrustumOutside )
	{
		return kFalse;
	}

	return kTrue;
} // CCharacter::InFrustum()

void CCharacter::Transform( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							CLevel* pLevel,						// level to send effects to							
							CCharacterTemplate& pCharacterManifest,// character manifest
						    const std::string& MonsterName,		// name of monster to transform into
							float32 Duration,					// duration of change in seconds ( 0 for permanent )
							bool ApplyStats )					// apply stats?
{
	if( MonsterName != m_TemplateName )
	{
		CCharacterDescription* pCharDescription;

		pCharDescription = pCharacterManifest.CharacterDescription( MonsterName );

		if( pCharDescription != NULL )
		{			
			m_TransformationDuration = Duration;
			ApplyTransformationDescription( pD3DDevice,
											pLevel,
											pCharacterManifest,
											*pCharDescription,
											kTrue,
											ApplyStats );
			m_TemplateName = MonsterName;
			//Flawless pet achievement
			if(Duration < 0)
			{
				CStatsAndAchievements::GetInstance()->SetFlawlessPet();
			}
		}

	}
	else	// otherwise, just update the transformation duration
	{
		CCharacterDescription* pCharDescription;

		pCharDescription = pCharacterManifest.CharacterDescription( MonsterName );

		if( pCharDescription != NULL )
		{			
			m_TransformationDuration = Duration;
		}
	}
} // CCharacter::Transform()

void CCharacter::RevertTransformation( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									   CLevel& pLevel )						// level to send effects to
{
	CCharacterDescription* pCharDescription;

	pCharDescription = pLevel.CharacterManifest()->CharacterDescription( m_OriginalTemplateName );

	if( pCharDescription != NULL )
	{		
		m_TransformationDuration = 0;
		ApplyTransformationDescription( pD3DDevice,
										&pLevel,
										*pLevel.CharacterManifest(),
										*pCharDescription,
										kFalse,
										kTrue );
		m_TemplateName = m_OriginalTemplateName;

		CSFXDescription* pSFX = new CSFXDescription( KSFXFollowTarget,
													"SPELLS\\EFFECTS\\DISPEL\\dispel.mdl",
													1,
													kTrue,
													0,
													0,
													kFalse,
													kFalse  );
												   
		pLevel.AddSFX( *pSFX,
					   this,
					   this,
					   Position(),
					   Position() );

		DELETE_SAFELY( pSFX );
		m_pCharacterSounds->PlaySample( KSoundDispel,
										m_Position,
										255,
										m_SoundRadius * 2 );

	}
} // CCharacter::RevertTransformation()

void CCharacter::UnloadModels( void )

{	m_pRefManager.RemoveMaterial( m_pOverrideMaterial );
	m_pOverrideMaterial = NULL;

	m_pRefManager.RemoveModel( m_pCollisionModel );
	m_pCollisionModel = NULL;

	DELETE_SAFELY( m_pModel );
	m_pModel = NULL;
	DELETE_SAFELY( m_pUpperBodyModel );
	m_pUpperBodyModel = NULL;
	DELETE_SAFELY( m_pGlovesModel );
	m_pGlovesModel = NULL;
	DELETE_SAFELY( m_pArmorModel );
	m_pArmorModel = NULL;
	DELETE_SAFELY( m_pBootsModel );
	m_pBootsModel = NULL;
	DELETE_SAFELY( m_pHeadModel );
	m_pHeadModel = NULL;
	DELETE_SAFELY( m_pHairModel );
	m_pHairModel = NULL;
	DELETE_SAFELY( m_pCullingBounds );
	m_pCullingBounds = NULL;

} // CCharacter::UnloadModels()

void CCharacter::LoadModel( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							std::string BaseModelPath,			// path to the model we want to load
							std::string UpperBodyModelPath,		// path to the upper body model we want to load (optional)
							std::string HeadModelPath,			// path to the head model we want to load (optional)
							std::string HairModelPath,			// path to the hair model we want to load (optional)
							std::string TexturePath,			// path to textures ("" to use model path)
							std::string CollisionPath,			// path to collision model
							std::string AnimationPath )			// path to animation data file
{
	UpdatePathNode( pD3DDevice );

	m_pCollisionModel = m_pRefManager.AddModel( pD3DDevice, 
												CollisionPath,
												"",
												kTrue,
												kFalse );

	m_CollisionRadius = m_pCollisionModel->GetMaxBounds().x * .6f;

	m_pModel = new CGenericModel( pD3DDevice,
								  m_pRefManager,
								  m_pSettings,
								  BaseModelPath,
								  TexturePath,
								  AnimationPath );

	D3DXVECTOR3 MinBounds = m_pModel->GetMinBounds();
	D3DXVECTOR3 MaxBounds = m_pModel->GetMaxBounds();
	ExpandHorizontalCubicBounds( MinBounds, MaxBounds, D3DXVECTOR3( MinBounds.x, 0, MinBounds.z ) );
	ExpandHorizontalCubicBounds( MinBounds, MaxBounds, D3DXVECTOR3( MaxBounds.x, 0, MaxBounds.z ) );

	if( UpperBodyModelPath != "" )
	{
		m_pUpperBodyModel = new CGenericModel( pD3DDevice,
											   m_pRefManager,
											   m_pSettings,
											   UpperBodyModelPath,
											   TexturePath,
											   AnimationPath );
		ExpandBounds( MinBounds, MaxBounds, m_pUpperBodyModel->GetMinBounds() );
		ExpandBounds( MinBounds, MaxBounds, m_pUpperBodyModel->GetMaxBounds() );
	}

	if( HeadModelPath != "" )
	{
		m_pHeadModel = new CGenericModel( pD3DDevice,
										  m_pRefManager,
										  m_pSettings,
										  HeadModelPath,
										  TexturePath,
										  AnimationPath );
	}

	if( HairModelPath != "" )
	{
		m_pHairModel = new CGenericModel( pD3DDevice,
										  m_pRefManager,
										  m_pSettings,
										  HairModelPath,
										  TexturePath,
										  AnimationPath );
	}

	m_pCullingBounds = new CCullingBounds( MinBounds,
										   MaxBounds );

	if( m_pModel->AnimationExists( "RUN" ) )
	{
		m_CanRun = kTrue;
	}

	FindAttachmentPoints();

	FindParticles();

	PlayAnimation( "IDLE", kTrue );

	UpdateAnimation( 0 );

} // CCharacter::LoadModel()

void CCharacter::LoadPlayerModels( LPDIRECT3DDEVICE8 pD3DDevice )					// direct3d device
{
	if( m_HeadIndex == -1 )
	{
		return;
	}
	int32 HairIndex = m_HairIndex + 1;
	int32 HeadIndex = m_HeadIndex + 1;
	std::string HairPath;
	std::string HeadPath;
	std::string UpperPath;
	if( m_Gender == KMale )
	{
		UpperPath = "PLAYER/upper.sms";
		HairPath = "PLAYER/hair" + IntegerToString( HairIndex ) + ".mdl";
		HeadPath = "PLAYER/head" + IntegerToString( HeadIndex ) + ".mdl";
	}
	else
	{
		UpperPath = "PLAYER/femaleupper.sms";
		HairPath = "PLAYER/hairfemale" + IntegerToString( HairIndex ) + ".mdl";
		HeadPath = "PLAYER/headfemale" + IntegerToString( HeadIndex ) + ".mdl";
	}
	LoadModel( pD3DDevice,
			   "PLAYER/lower.sms",
			   UpperPath,
			   HeadPath,
			   HairPath,
			   "",
			   "PLAYER/collision.mdl",
			   "" );

} // CCharacter::LoadPlayerModels()

void CCharacter::UpdatePathNode( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	m_pRefManager.RemoveModel( m_pPathNodeModel );
	m_pPathNodeModel = NULL;
	if( m_pPathNodeModel == NULL )
	{
		m_PathNodeAlignment = Alignment();
		switch( Alignment() )
		{
		case KGood :
			m_pPathNodeModel = m_pRefManager.AddModel( pD3DDevice, 
													   "UI\\goodring.mdl",
													   "", 
													   kTrue, 
													   kTrue );
			break;
		case KEvil :
			m_pPathNodeModel = m_pRefManager.AddModel( pD3DDevice, 
													   "UI\\evilring.mdl",
													   "", 
													   kTrue, 
													   kTrue );
			break;
		default:
			m_pPathNodeModel = m_pRefManager.AddModel( pD3DDevice, 
													   "UI\\neutralring.mdl",
													   "", 
													   kTrue, 
													   kTrue );
			break;
		}
	}

} // CCharacter::UpdatePathNode()

void CCharacter::LoadGloves( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const std::string& GlovesModelPath )	// path to animated gloves mesh
{
	if( m_pGlovesModel != NULL &&
		GlovesModelPath == m_pGlovesModel->ModelPath() )
	{
		return;
	}
	DELETE_SAFELY( m_pGlovesModel );
	if( GlovesModelPath != "" )
	{
		m_pGlovesModel = new CGenericModel( pD3DDevice,
											m_pRefManager,
											m_pSettings,
											GlovesModelPath,
											"" );
		m_pGlovesModel->CopyAnimationState( *m_pModel );
	}
} // CCharacter::LoadGloves()

void CCharacter::LoadArmor( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const std::string& ArmorModelPath )	// path to animated gloves mesh
{
	if( m_pArmorModel != NULL &&
		ArmorModelPath == m_pArmorModel->ModelPath() )
	{
		return;
	}
	DELETE_SAFELY( m_pArmorModel );
	if( ArmorModelPath != "" )
	{
		m_pArmorModel = new CGenericModel( pD3DDevice,
											m_pRefManager,
											m_pSettings,
											ArmorModelPath,
											"" );
		m_pArmorModel->CopyAnimationState( *m_pModel );
	}
} // CCharacter::LoadArmor()

void CCharacter::LoadBoots( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 const std::string& BootsModelPath )	// path to animated gloves mesh
{
	if( m_pBootsModel != NULL &&
		BootsModelPath == m_pBootsModel->ModelPath() )
	{
		return;
	}
	DELETE_SAFELY( m_pBootsModel );
	if( BootsModelPath != "" )
	{
		m_pBootsModel = new CGenericModel( pD3DDevice,
											m_pRefManager,
											m_pSettings,
											BootsModelPath,
											"" );
		m_pBootsModel->CopyAnimationState( *m_pModel );
	}
} // CCharacter::LoadBoots()

void CCharacter::FindAttachmentPoints( void )
{
	m_MouthAttachments.clear();
	uint32 Tags = GetTagCount();
	for( uint32 i = 0; i < KAttachmentPoints; i++ )
	{
		m_AttachmentTagIndex[i] = -1;
		for( uint32 j = 0; j < Tags; j++ )
		{
			if( StringUpper( GetTagName( j ) ) == KAttachmentNames[i] )
			{
				m_AttachmentTagIndex[i] = j;
			}
		}
	}
	for( int32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( HasAttachment( KItemAttachments[i] ) )
		{
			m_pInventory->SetCanEquip( EItemSlot(i), kTrue );
		}
	}

	for( uint32 j = 0; j < Tags; j++ )
	{
		std::string Name = StringUpper( GetTagName( j ) );
		if( Name.length() >= 5 && Name.substr( 0, 5 ) == "MOUTH" )
		{
			m_MouthAttachments.push_back( j );
		}
		if( Name.length() >= 3 && Name.substr( 0, 3 ) == "EYE" )
		{
			m_MouthAttachments.push_back( j );
		}
	}

	m_pInventory->SetCanEquip( KSlotNeck, kTrue );
	m_pInventory->SetCanEquip( KSlotLeftFinger, kTrue );
	m_pInventory->SetCanEquip( KSlotRightFinger, kTrue );
	m_pInventory->SetCanEquip( KSlotBelt, kTrue );
	m_pInventory->SetCanEquip( KSlotGloves, kTrue );
	m_pInventory->SetCanEquip( KSlotFeet, kTrue );
	m_pInventory->SetCanEquip( KSlotChest, kTrue );
} // CCharacter::FindAttachmentPoints()

void CCharacter::ClearParticles( void )
{
	m_TagParticleIndex.clear();
	m_TagParticleType.clear();
	m_ParticlePosition.clear();
	m_ParticleOrientation.clear();
	m_ParticleType.clear();
	for( uint32 i = 0; i < m_pGlowList.size(); i++ )
	{
		m_pGlowList[i]->m_RequestingRemoval = kTrue;
	}
	for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
	{
		m_pModelGlowList[i]->m_RequestingRemoval = kTrue;
	}
	m_pGlowList.clear();
	m_pModelGlowList.clear();
	m_GlowIndex.clear();
	m_GlowPosition.clear();
} // CCharacter::ClearParticles()

void CCharacter::FindParticles( void )
{
	ClearParticles();

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
		std::string Name = StringUpper( GetTagName( i ) );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( GetTagUserData( i )  );
			if( ParticleIndex != -1 )
			{
				m_TagParticleIndex.push_back( i );
				m_TagParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGlowManager.GetGlowType( GetTagUserData( i ) );
			if( GlowIndex != -1 )
			{
				m_pGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, D3DXVECTOR3( 0, -10000, 0 ) ) );					
				m_GlowIndex.push_back( i );
			}
		}
	}
} // CCharacter::FindParticles()

void CCharacter::EmitParticles( void )
{
	bool CharacterFishing( kFalse );
	// fishing is a special state where we don't render weapons/shields, 
	// but DO render the pole
	if( AIState() == KAIFish ||
		AIState() == KAICancelFish ||
		AIState() == KAIPullFish )
	{
		CharacterFishing = kTrue;
	}

	// render attachments
	for( uint32 i = 0; i < KEquipmentSlots; i++ )
	{
		if( m_pInventory->SlotEquipped( (EItemSlot)i ) &&
			i != KSlotNeck &&
			i != KSlotLeftFinger &&
			i != KSlotRightFinger &&
			i != KSlotBelt )
		{
			if( !CharacterFishing ||
				( i != KSlotLeftHand &&
				  i != KSlotRightHand &&
				  i != KSlotLeftArm ) )
			{
				m_pInventory->GetItemFromSlot( (EItemSlot)i )->EmitParticles();
			}
		}
	}


	for( uint32 i = 0; i < m_ParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXVec3TransformCoord( &Position,  &m_ParticlePosition[i], &m_DisplayMatrix );
		D3DXMATRIX Transform = m_ParticleOrientation[i] * m_OrientationMatrix;
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pParticleManager.AddParticle( m_ParticleType[i], Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
	}

	for( int32 i = 0; i < m_TagParticleType.size(); i++ )
	{
		D3DXMATRIX Transform = GetTransformedTag( m_TagParticleIndex[i] );
		Transform *= m_DisplayMatrix;
		m_pParticleManager.AddParticle( m_TagParticleType[i], Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
	}

	for( int32 i = 0; i < m_pGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position = GetTagPosition( m_GlowIndex[i] );
		D3DXVec3TransformCoord( &Position, &Position, &m_DisplayMatrix );
		m_pGlowList[i]->m_Position = Position;
	}
	for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXVec3TransformCoord( &Position,  &m_GlowPosition[i], &m_DisplayMatrix );
		D3DXMATRIX Transform = m_OrientationMatrix;
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pModelGlowList[i]->m_Position = Position;
	}


	if( PerformingCast() &&
		m_pCastingSpell != NULL )
	{
		if( m_AttachmentTagIndex[KAttachLeftHand] != -1 )
		{
			D3DXMATRIX Transform = GetTransformedTag( m_AttachmentTagIndex[KAttachLeftHand] );
			Transform *= m_DisplayMatrix;
			for( uint32 i = 0; i < m_pCastingSpell->Particles(); i++ )
			{
				m_pParticleManager.AddParticle( m_pCastingSpell->Particle( i ), Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
			}
		}
		if( m_AttachmentTagIndex[KAttachRightHand] != -1 )
		{
			D3DXMATRIX Transform = GetTransformedTag( m_AttachmentTagIndex[KAttachRightHand] );
			Transform *= m_DisplayMatrix;
			for( uint32 i = 0; i < m_pCastingSpell->Particles(); i++ )
			{
				m_pParticleManager.AddParticle( m_pCastingSpell->Particle( i ), Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
			}
		}
	}

	if( !Fishing() )
	{
		if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
		{
			CItem* pItem = m_pInventory->GetItemFromSlot( KSlotLeftHand );
			D3DXMATRIX OriginalTransform = GetTransformedTag( m_AttachmentTagIndex[KAttachLeftHand] );
			for( int32 i = 0; i < pItem->DamageBonuses(); i++ )
			{
				D3DXMATRIX Transform = OriginalTransform * m_DisplayMatrix;
				if( m_DamageParticle[pItem->DamageBonusType( i )] != -1 )
				{
					float32 Offset = RandomBetween( pItem->Model()->GetMinBounds().y, pItem->Model()->GetMaxBounds().y );
					D3DXVECTOR3 PositionOffset = D3DXVECTOR3( Transform._21, Transform._22, Transform._23 ) * Offset;
					Transform._41 += PositionOffset.x;
					Transform._42 += PositionOffset.y;
					Transform._43 += PositionOffset.z;
					m_pParticleManager.AddParticle( m_DamageParticle[pItem->DamageBonusType( i )], Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
				}
				if( m_DamageParticleSecondary[pItem->DamageBonusType( i )] != -1 )
				{
					float32 Offset = RandomBetween( pItem->Model()->GetMinBounds().y, pItem->Model()->GetMaxBounds().y );
					D3DXVECTOR3 PositionOffset = D3DXVECTOR3( Transform._21, Transform._22, Transform._23 ) * Offset;
					Transform._41 += PositionOffset.x;
					Transform._42 += PositionOffset.y;
					Transform._43 += PositionOffset.z;
					m_pParticleManager.AddParticle( m_DamageParticleSecondary[pItem->DamageBonusType( i )], Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
				}
			}
		}

		if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
		{
			CItem* pItem = m_pInventory->GetItemFromSlot( KSlotRightHand );
			D3DXMATRIX OriginalTransform = GetTransformedTag( m_AttachmentTagIndex[KAttachRightHand] );
			for( int32 i = 0; i < pItem->DamageBonuses(); i++ )
			{
				D3DXMATRIX Transform = OriginalTransform * m_DisplayMatrix;
				if( m_DamageParticle[pItem->DamageBonusType( i )] != -1 )
				{
					float32 Offset = RandomBetween( pItem->Model()->GetMinBounds().y, pItem->Model()->GetMaxBounds().y );
					D3DXVECTOR3 PositionOffset = D3DXVECTOR3( Transform._21, Transform._22, Transform._23 ) * Offset;
					Transform._41 += PositionOffset.x;
					Transform._42 += PositionOffset.y;
					Transform._43 += PositionOffset.z;
					m_pParticleManager.AddParticle( m_DamageParticle[pItem->DamageBonusType( i )], Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
				}
				if( m_DamageParticleSecondary[pItem->DamageBonusType( i )] != -1 )
				{
					float32 Offset = RandomBetween( pItem->Model()->GetMinBounds().y, pItem->Model()->GetMaxBounds().y );
					D3DXVECTOR3 PositionOffset = D3DXVECTOR3( Transform._21, Transform._22, Transform._23 ) * Offset;
					Transform._41 += PositionOffset.x;
					Transform._42 += PositionOffset.y;
					Transform._43 += PositionOffset.z;
					m_pParticleManager.AddParticle( m_DamageParticleSecondary[pItem->DamageBonusType( i )], Transform, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
				}
			}
		}
	}

} // CCharacter::EmitParticles()

// update current xform of all valid attachment points
void CCharacter::UpdateAttachmentPoints( void )
{
	for( uint32 i = 0; i < KAttachmentPoints; i++ )
	{
		if( m_AttachmentTagIndex[i] != -1 )
		{
			m_AttachmentTagXForm[i] = GetTransformedTag( m_AttachmentTagIndex[i] );
		}
	}
} // CCharacter::UpdateAttachmentPoints()

uint32 CCharacter::GetTagCount( void )
{
	uint32 Tags = m_pModel->GetTagCount();
	if( m_pUpperBodyModel != NULL )
	{
		Tags += m_pUpperBodyModel->GetTagCount();
	}
	else if( m_pHeadModel != NULL )
	{
		Tags += m_pHeadModel->GetTagCount();
	}
	else if( m_pHairModel != NULL )
	{
		Tags += m_pHairModel->GetTagCount();
	}
	return Tags;
} // CCharacter::GetTagCount()

const std::string& CCharacter::GetTagName( uint32 Index )
{
	if( Index < m_pModel->GetTagCount() )
	{
		return m_pModel->GetTagName( Index );
	}
	Index -= m_pModel->GetTagCount();
	
	if( m_pUpperBodyModel != NULL )
	{
		if( Index < m_pUpperBodyModel->GetTagCount() )
		{
			return m_pUpperBodyModel->GetTagName( Index );
		}
		Index -= m_pUpperBodyModel->GetTagCount();
	}
	if( m_pHeadModel != NULL )
	{
		if( Index < m_pHeadModel->GetTagCount() )
		{
			return m_pHeadModel->GetTagName( Index );
		}
		Index -= m_pHeadModel->GetTagCount();
	}

	assert( m_pHairModel != NULL );
	if( m_pHairModel != NULL )
	{
		return m_pHairModel->GetTagName( Index );
	}
	return m_Name;
} // CCharacter::GetTagName()

const std::string& CCharacter::GetTagUserData( uint32 Index )
{
	if( Index < m_pModel->GetTagCount() )
	{
		return m_pModel->GetTagUserData( Index );
	}
	Index -= m_pModel->GetTagCount();
	
	if( m_pUpperBodyModel != NULL )
	{
		if( Index < m_pUpperBodyModel->GetTagCount() )
		{
			return m_pUpperBodyModel->GetTagUserData( Index );
		}
		Index -= m_pUpperBodyModel->GetTagCount();
	}
	if( m_pHeadModel != NULL )
	{
		if( Index < m_pHeadModel->GetTagCount() )
		{
			return m_pHeadModel->GetTagUserData( Index );
		}
		Index -= m_pHeadModel->GetTagCount();
	}

	assert( m_pHairModel != NULL );
	if( m_pHairModel != NULL )
	{
		return m_pHairModel->GetTagUserData( Index );
	}
	return m_Name;
} // CCharacter::GetTagUserData()

D3DXMATRIX CCharacter::GetTransformedTag( uint32 Index )					// index of tag
{
	if( Index < m_pModel->GetTagCount() )
	{
		return m_pModel->GetTransformedTag( Index );
	}
	Index -= m_pModel->GetTagCount();
	
	if( m_pUpperBodyModel != NULL )
	{
		if( Index < m_pUpperBodyModel->GetTagCount() )
		{
			return m_pUpperBodyModel->GetTransformedTag( Index );
		}
		Index -= m_pUpperBodyModel->GetTagCount();
	}
	if( m_pHeadModel != NULL )
	{
		if( Index < m_pHeadModel->GetTagCount() )
		{
			return m_pHeadModel->GetTransformedTag( Index );
		}
		Index -= m_pHeadModel->GetTagCount();
	}

	assert( m_pHairModel != NULL );
	if( m_pHairModel != NULL )
	{
		return m_pHairModel->GetTransformedTag( Index );
	}
	return Orientation();
} // CCharacter::GetTransformedTag()

D3DXVECTOR3 CCharacter::GetTagPosition( uint32 Index )					// index of tag							
{
	if( Index < m_pModel->GetTagCount() )
	{
		return m_pModel->GetTagPosition( Index );
	}
	Index -= m_pModel->GetTagCount();
	
	if( m_pUpperBodyModel != NULL )
	{
		if( Index < m_pUpperBodyModel->GetTagCount() )
		{
			return m_pUpperBodyModel->GetTagPosition( Index );
		}
		Index -= m_pUpperBodyModel->GetTagCount();
	}
	if( m_pHeadModel != NULL )
	{
		if( Index < m_pHeadModel->GetTagCount() )
		{
			return m_pHeadModel->GetTagPosition( Index );
		}
		Index -= m_pHeadModel->GetTagCount();
	}

	assert( m_pHairModel != NULL );
	if( m_pHairModel != NULL )
	{
		return m_pHairModel->GetTagPosition( Index );
	}

	return GetMaxBounds();
} // CCharacter::GetTagPosition()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCharacter::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
							   const D3DXVECTOR3& End,		// end of collision ray
							   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
							   D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	assert( m_pCollisionModel != NULL );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );

	ExpandBounds( MinBounds, MaxBounds, End );

	if( !BoundsIntersect( m_MinCollisionBounds,
						  m_MaxCollisionBounds,
						  MinBounds,
						  MaxBounds ) )
	{
		return kFalse;
	}

	// first we have to bring the collision ray into local space
	D3DXMATRIX InverseTransformation;
	D3DXMatrixInverse( &InverseTransformation, NULL, &m_OrientationMatrix );
	D3DXVECTOR3 LocalStart;
	D3DXVECTOR3 LocalEnd;
	LocalStart = Start - m_DisplayPosition;
	LocalEnd = End - m_DisplayPosition;
	//D3DXVec3TransformNormal( &LocalStart, &LocalStart, &InverseTransformation );
	//D3DXVec3TransformNormal( &LocalEnd, &LocalEnd, &InverseTransformation );

	if( m_pCollisionModel->RayCollision( LocalStart,
										 LocalEnd,
										 ImpactPoint,
										 ImpactNormal ) )
	{
		// for a successful collision, we have to send the results back to world space
		//D3DXVec3TransformNormal( &ImpactPoint, &ImpactPoint, &m_OrientationMatrix );
		//D3DXVec3TransformNormal( &ImpactNormal, &ImpactNormal, &m_OrientationMatrix );
		ImpactPoint += m_DisplayPosition;
		return kTrue;
	}
	return kFalse;
} // CCharacter::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CCharacter::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
								  const D3DXVECTOR3& End,			// end of collision ray
								  float32 Radius,					// radius of sphere to check along collision ray
								  D3DXVECTOR3& FinalPosition,		// final position of sphere after impact to be filled
								  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
								  D3DXVECTOR3& ImpactNormal )		// impact normal to be filled
{
	assert( m_pCollisionModel != NULL );

	D3DXVECTOR3	MinBounds		( Start );
	D3DXVECTOR3	MaxBounds		( Start );

	ExpandBounds( MinBounds, MaxBounds, End );
	MinBounds.x -= Radius;
	MinBounds.y -= Radius;
	MinBounds.z -= Radius;
	MaxBounds.x += Radius;
	MaxBounds.y += Radius;
	MaxBounds.z += Radius;
	
	if( !BoundsIntersect( m_MinCollisionBounds,
						  m_MaxCollisionBounds,
						  MinBounds,
						  MaxBounds ) )
	{
		return kFalse;
	}

	// first we have to bring the collision ray into local space
	D3DXMATRIX InverseTransformation;
	D3DXMatrixInverse( &InverseTransformation, NULL, &m_OrientationMatrix );
	D3DXVECTOR3 LocalStart;
	D3DXVECTOR3 LocalEnd;
	LocalStart = Start - m_DisplayPosition;
	LocalEnd = End - m_DisplayPosition;
	//D3DXVec3TransformNormal( &LocalStart, &LocalStart, &InverseTransformation );
	//D3DXVec3TransformNormal( &LocalEnd, &LocalEnd, &InverseTransformation );

	if( m_pCollisionModel->SphereCollision( LocalStart,
											LocalEnd,
											Radius,
											FinalPosition,
											ImpactPoint,
											ImpactNormal ) )
	{
		// for a successful collision, we have to send the results back to world space
		//D3DXVec3TransformNormal( &ImpactPoint, &ImpactPoint, &m_OrientationMatrix );
		//D3DXVec3TransformNormal( &FinalPosition, &FinalPosition, &m_OrientationMatrix );
		//D3DXVec3TransformNormal( &ImpactNormal, &ImpactNormal, &m_OrientationMatrix );
		ImpactPoint += m_DisplayPosition;
		FinalPosition += m_DisplayPosition;
		return kTrue;
	}
	return kFalse;
} // CCharacter::SphereCollision()

void CCharacter::ReturnToTown( CLevel& pLevel )
{
	if( pLevel.LevelDepth() == 0 )
	{
		// already in town!
		return;
	}
	// assume we are a pet
	if( HasMaster() )
	{
		Master()->PlayHelpSample( KSoundHelpPetDepart, kTrue );
		m_TownTimer = (float32)pLevel.LevelDepth() * 10;
		if( m_TownTimer > 120 )
		{
			m_TownTimer = 120;
		}
		SetTarget( NULL );
		SetTargetItem( NULL );
		SetAIState( KAIReturningToTown );
		D3DXVECTOR3 Target = pLevel.RandomOpenPosition( Master()->Position(), 20, 40 );
		SetDestination( pLevel, Target.x, Target.z );
	}
} // CCharacter::ReturnToTown()

void CCharacter::SetAIState( EAIState State )	// ai state to set
{
	// only players get to immediately repath after AI state changes.
	//m_TimeSincePath = 10;
	m_AIState = State;
} // CCharacter::SetAIState()

void CCharacter::FollowCharacter( CCharacter* pCharacter,	// character to attack
								  CLevel& pLevel )			// a scene to collide with
{
	if( pCharacter == NULL ||
		!pCharacter->Alive() )
	{
		return;
	}
	SetTarget( pCharacter );
	
	if( Target() == NULL )
	{
		return;
	}

	bool WasOnAvoidanceMap( Target()->OnAvoidanceMap() );
	Target()->RemoveFromAvoidanceMap( pLevel );
	
	SetDestination( pLevel,
					Target()->Position().x,
					Target()->Position().z );

	if( WasOnAvoidanceMap )
	{
		Target()->AddToAvoidanceMap( pLevel );
	}
} // CCharacter::FollowCharacter()

bool CCharacter::IsDualWielding( void )
{
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL &&
		m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		return kTrue;
	}
	return kFalse;
} // CCharacter::IsDualWielding()

void CCharacter::Attack( void )
{
	if( Target() == NULL &&
		TargetItem() == NULL &&
		!m_HoldPosition )
	{
		return;
	}
	if( UnarmedAttacks() == 0 )
	{
		return;
	}
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			return;
		}
	}
	if( m_pCurrentAttack->AnimationIndex() == -1 )
	{
		return;
	}
	if( !PerformingAttack() )
	{
		m_ReachedStrikePoint = kFalse;

		float32 AttackSpeed = 1.0f;
		AttackSpeed += (float32)EffectValue( KEffectPercentAttackSpeed ) / 100.0f;
		AttackSpeed *= m_pCurrentAttack->Speed();
		if( AttackSpeed < .2f )
		{
			AttackSpeed = .2f;
		}

		m_AttackTimer = m_pModel->GetAnimationLengthSeconds( m_pCurrentAttack->AnimationIndex() );
		// dual wielders get a speed bonus on their first attack for quick 1-2 combos
		if( IsDualWielding() )
		{
			m_AttackTimer *= .6f;
		}
		else
		{
			m_AttackTimer *= .8f;
		}
		m_AttackTimer /= AttackSpeed;
		BlendAnimation( m_pCurrentAttack->AnimationIndex(), kFalse, .1f, AttackSpeed );
		ClearQueuedAnimations();
		QueueBlendAnimation( "IDLE", kTrue, .1f );

		if( !m_pCurrentAttack->Armed() )
		{
			m_pAttackSounds->PlaySample( m_pCurrentAttack->AnimationIndex(),
										m_Position,
										255,
										m_SoundRadius );
		}
		else if( m_pActiveWeapon != NULL )
		{
			m_pCharacterSounds->PlaySample( KSoundAttack,
											m_Position,
											255,
											m_SoundRadius );

			m_pActiveWeapon->AttackSounds()->PlaySample( 0,
														 m_Position,
														 255,
														 m_SoundRadius );
		}
	}

} // CCharacter::Attack()

void CCharacter::UpdateAttack( CLevel& pLevel )	// level to create pathfinder for
{
	if( !PerformingAttack() )
	{
		m_BreathActive = kFalse;
		return;
	}

	if( m_pCurrentAttack == NULL )
	{
		return;
	}

	if( !Alive() )
	{
		return;
	}
	// check any animation keys that were triggered during the run
	for( uint32 i = 0; i < m_pModel->KeysEncountered(); i++ )
	{
		// begin breath activity
		if( m_pModel->KeyEncounteredName( i ) == "BREATHSTART"  )
		{
			m_BreathActive = kTrue;
		}
		// begin breath activity
		if( m_pModel->KeyEncounteredName( i ) == "BREATHEND"  )
		{
			m_BreathActive = kFalse;
		}

		// find a hit instant
		if( m_pModel->KeyEncounteredName( i ) == "HIT"  )
		{
			m_ReachedStrikePoint = kTrue;
			PerformAttack( pLevel );
		}
	}

	if( m_BreathActive )
	{
		for( uint32 b = 0; b < m_pCurrentAttack->BreathParticles(); b++ )
		{
			for( uint32 i = 0; i < m_MouthAttachments.size(); i++ )
			{
				D3DXVECTOR3 Velocity = m_pCurrentAttack->BreathVelocity( b );
				D3DXMATRIX Transform = GetTransformedTag( m_MouthAttachments[i] );

				Transform *= m_DisplayMatrix;
				m_pParticleManager.AddParticle( m_pCurrentAttack->BreathParticle( b ), 
												Transform,
												Velocity,
												m_Scale );
			}
		}
	}

} // CCharacter::UpdateAttack()

void CCharacter::PerformAttack( CLevel& pLevel )	// level to create pathfinder for
{
	bool StrikeObstructed( kFalse );
	m_LastAttackRight = !m_LastAttackRight;

	if( m_pCurrentAttack == NULL )
	{
		return;
	}
	

	CAttackDescription* pCurrentAttack = m_pCurrentAttack;
	CItem* pActiveWeapon = m_pActiveWeapon;

	// if our target item isn't attackable, then forget it
	if( TargetItem() != NULL &&
		!( TargetItem()->IsContainer() &&
		   TargetItem()->CanInteract() ) )
	{
		SetTargetItem( NULL );
	}

	CCharacter* pTarget = Target();
	CItem* pTargetItem = TargetItem();

	if( pTarget != NULL )
	{
		EAlignment TargetAlignment( KGood );
		if( Alignment() == KGood )
		{
			TargetAlignment = KEvil;
		}
		if( pTarget->Alignment() != TargetAlignment )
		{
			return;
		}


		pTargetItem = NULL;
	}
	// if nothing is targeted, we may be attacking in place -
	// so see if there is anything in the arc of our attack
	if( pTarget == NULL &&
		pTargetItem == NULL )
	{
		EAlignment TargetAlignment( KGood );
		if( Alignment() == KGood )
		{
			TargetAlignment = KEvil;
		}

		CCharacter* pViewTarget = pLevel.FindCharacterWithinView( DisplayMatrix(),
																  TargetAlignment,
																  0,
																  30,
																  pCurrentAttack->StrikeRadius() );

		if( pViewTarget != NULL )
		{
			pTarget = pViewTarget;
		}
		else 
		{
			CItem* pViewTargetItem = pLevel.FindDestructibleItemWithinView( DisplayMatrix(),
																		30,
																		pCurrentAttack->StrikeRadius() );
			if( pViewTargetItem != NULL )
			{
				pTargetItem = pViewTargetItem;
			}
		}

	}


	// add streaks for arrows,
	// and check for wall impacts
	if( pActiveWeapon != NULL && 
		( pActiveWeapon->Type() == KItemBow ||
		  pActiveWeapon->Type() == KItemCrossbow ) )
	{
		// we fire it from the left hand
		if( m_AttachmentTagIndex[KAttachLeftHand] != -1 )
		{
			D3DXVECTOR3 StartPosition;
			D3DXVec3TransformCoord( &StartPosition, &GetTagPosition( m_AttachmentTagIndex[KAttachLeftHand] ), &m_DisplayMatrix );
			D3DXVECTOR3 TargetPosition = StartPosition + m_Forward * pActiveWeapon->StrikeRadius();
			if( pTargetItem != NULL )
			{
				TargetPosition = pTargetItem->Position();
			}
			else if( pTarget != NULL )
			{
				TargetPosition = pTarget->Position();
			}
			TargetPosition.y = StartPosition.y;
			D3DXVECTOR3 StartRayPosition = Position();
			D3DXVECTOR3 TargetRayPosition = Position();
			StartRayPosition.y += 5;
			if( pTargetItem != NULL )
			{
				TargetRayPosition = pTargetItem->Position();
			}
			else if( pTarget != NULL )
			{
				TargetRayPosition = pTarget->Position();
			}
			TargetRayPosition.y = StartRayPosition.y;
			D3DXVECTOR3 Impact;
			D3DXVECTOR3 Normal;
			uint32 Material;
			D3DXVECTOR3 Color;
			if( pLevel.RayCollision( StartRayPosition, TargetRayPosition, Impact, Normal, Material, Color, NULL, kFalse ) )
			{
				StrikeObstructed = kTrue;
				TargetPosition = Impact;
				TargetPosition.y = StartPosition.y;
			}

			float32 Distance = D3DXVec3Length( &( TargetPosition - StartPosition ) );
			pLevel.AddArrowTrail( StartPosition, TargetPosition );
			TargetPosition -= StartPosition;
			D3DXVec3Normalize( &TargetPosition, &TargetPosition );
			TargetPosition *= .05f;
			int32 Steps = (int32)( Distance / .05f );
			// add elemental particles for arrow trails
			for( int32 j = 0; j < Steps; j++ )
			{
				if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
				{
					CItem* pItem = m_pInventory->GetItemFromSlot( KSlotLeftHand );
					for( uint32 i = 0; i < pItem->DamageBonuses(); i++ )
					{
						if( m_DamageParticle[pItem->DamageBonusType( i )] != -1 )
						{
							m_pParticleManager.AddParticle( m_DamageParticle[pItem->DamageBonusType( i )], StartPosition, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
						}
						if( m_DamageParticleSecondary[pItem->DamageBonusType( i )] != -1 )
						{
							m_pParticleManager.AddParticle( m_DamageParticleSecondary[pItem->DamageBonusType( i )], StartPosition, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
						}
					}
				}

				if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
				{
					CItem* pItem = m_pInventory->GetItemFromSlot( KSlotRightHand );
					for( uint32 i = 0; i < pItem->DamageBonuses(); i++ )
					{
						if( m_DamageParticle[pItem->DamageBonusType( i )] != -1 )
						{
							m_pParticleManager.AddParticle( m_DamageParticle[pItem->DamageBonusType( i )], StartPosition, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
						}
						if( m_DamageParticleSecondary[pItem->DamageBonusType( i )] != -1 )
						{
							m_pParticleManager.AddParticle( m_DamageParticleSecondary[pItem->DamageBonusType( i )], StartPosition, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
						}
					}
				}
				StartPosition += TargetPosition;
			}
		}
	}
	else // check wall obstruction for melee attacks
	{
		D3DXVECTOR3 StartRayPosition = Position();
		D3DXVECTOR3 TargetRayPosition = Position();
		StartRayPosition.y += 5;
		if( pTargetItem != NULL )
		{
			TargetRayPosition = pTargetItem->Position();
		}
		else if( pTarget != NULL )
		{
			TargetRayPosition = pTarget->Position();
		}
		TargetRayPosition.y = StartRayPosition.y;
		D3DXVECTOR3 Impact;
		D3DXVECTOR3 Normal;
		uint32 Material;
		D3DXVECTOR3 Color;
		if( pLevel.RayCollision( StartRayPosition, TargetRayPosition, Impact, Normal, Material, Color, NULL, kFalse ) )
		{
			StrikeObstructed = kTrue;
		}
		if( StrikeObstructed )
		{
			StartRayPosition -= Forward() * 5;
			StartRayPosition += Right() * 3;
			TargetRayPosition += Right() * 3;
			if( !pLevel.RayCollision( StartRayPosition, TargetRayPosition, Impact, Normal, Material, Color, NULL, kFalse ) )
			{
				StrikeObstructed = kFalse;
			}
			if( StrikeObstructed )
			{
				StartRayPosition -= Right() * 6;
				TargetRayPosition -= Right() * 6;
				if( !pLevel.RayCollision( StartRayPosition, TargetRayPosition, Impact, Normal, Material, Color, NULL, kFalse ) )
				{
					StrikeObstructed = kFalse;
				}
			}
		}
	}
	if( InStrikeRange( pTarget, pTargetItem ) &&
		FacingTarget( pTarget, pTargetItem ) )
	{
		bool Hit( kFalse );
		if( pTargetItem != NULL )
		{
			Hit = kTrue;
			// bash the target item
			if( !StrikeObstructed )
			{
				if( pTargetItem->Trapped() )
				{
					IncrementStat( KJournalStatTrapsSprung );
					PlayHelpSample( KSoundHelpTrap, kTrue );
				}
				pTargetItem->Destroy( pLevel );
				IncrementStat( KJournalStatBarrelsBroken );

			}

			if( pActiveWeapon != NULL && 
				!( pActiveWeapon->Type() == KItemBow ||
				  pActiveWeapon->Type() == KItemCrossbow ) )
			{
				float32 SwingRadius = 45;
				if( KItemDamageType[pActiveWeapon->Type()] == KDamagePiercing )
				{
					SwingRadius = 20;
				}
				bool Done = kFalse;
				float32 StrikeRadius( 0 );
				if( pCurrentAttack != NULL )
				{
					StrikeRadius = pCurrentAttack->StrikeRadius();
				}
				while( !Done )
				{
					Done = kFalse;

					CItem* pTargetSubItem = pLevel.FindDestructibleItemWithinView( DisplayMatrix(),
																				   SwingRadius,
																				   StrikeRadius );
					if( pTargetSubItem != NULL )
					{
						pTargetSubItem->Destroy( pLevel );
						IncrementStat( KJournalStatBarrelsBroken );
					}
					else
					{
						Done = kTrue;
					}
				}
			}


		}
		else if( pTarget != NULL )
		{
			if( !StrikeObstructed && RollAttack( pLevel, pTarget ) )
			{
				Hit = kTrue;
			}
		}


		if( Hit && !StrikeObstructed )
		{
			if( pCurrentAttack != NULL &&
				!pCurrentAttack->Armed() )
			{
				m_pStrikeSounds->PlaySample( pCurrentAttack->AnimationIndex(),
											m_Position,
											255,
											m_SoundRadius );
			}
			else if( pActiveWeapon != NULL )
			{
				pActiveWeapon->StrikeSounds()->PlaySample( 0,
															 m_Position,
															 255,
															 m_SoundRadius );
			}

		}
		else
		{
			if( pCurrentAttack != NULL &&
				!pCurrentAttack->Armed() )
			{
				m_pMissSounds->PlaySample( pCurrentAttack->AnimationIndex(),
										   m_Position,
										   255,
										   m_SoundRadius );
			}
			else if( pActiveWeapon != NULL )
			{
				pActiveWeapon->MissSounds()->PlaySample( 0,
														   m_Position,
														   255,
														   m_SoundRadius );
			}
		}
	}
} // CCharacter::PerformAttack()

bool CCharacter::PerformingAttack( void )
{
	if( m_pCurrentAttack == NULL )
	{
		return kFalse;
	}
	if( m_AttackTimer > 0 )
	{
		return kTrue;
	}
	return kFalse;
} // CCharacter::PerformingAttack()

void CCharacter::Cast( void )
{
	if( !PerformingCast() )
	{
		m_AttackTimer = 0;
		StopPathing();
		m_ReachedCastPoint = kFalse;

		m_CastTimer = m_pCastingSpell->CastTime();		

		float32 CastSpeedModifier = 1.0f;

		float32 SpellSkill = 50.0f / ( (float32)SkillPoints( KSkillSpellcasting ) * .03f + 1 );
		SpellSkill = 50 - SpellSkill;
		if( SpellSkill > 50 )
		{
			SpellSkill = 50;
		}
		SpellSkill /= 50.0f;
		SpellSkill *= .8f;

		CastSpeedModifier -= SpellSkill;
		CastSpeedModifier *= ( 1.0f - (float32)EffectValue( KEffectPercentCastSpeed ) / 100.0f );
		if( CastSpeedModifier < .2f )
		{
			CastSpeedModifier = .2f;
		}
		m_CastTimer *= CastSpeedModifier;

		std::string CastAnimation( "CAST" );
		if( m_CastTimer < 2.0f )
		{
			if( m_pModel->AnimationExists( "FASTCAST" ) )
			{
				CastAnimation = "FASTCAST";
			}
		}
		if( !m_pModel->AnimationExists( CastAnimation ) )
		{
			CastAnimation = "IDLE";
		}
		float32 CastSpeed = m_pModel->GetAnimationLengthSeconds( CastAnimation );
		
		CastSpeed /= m_CastTimer;

		BlendAnimation( CastAnimation, kFalse, .1f, CastSpeed );
		ClearQueuedAnimations();
		QueueBlendAnimation( "IDLE", kTrue, .1f );
	}

} // CCharacter::Cast()

void CCharacter::UpdateCast( CLevel& pLevel )	// level to create pathfinder for
{
	if( !PerformingCast() ||
		!Alive() )
	{
		return;
	}

	// check any animation keys that were triggered during the run
	for( uint32 i = 0; i < m_pModel->KeysEncountered(); i++ )
	{
		// find a hit instant
		if( m_pModel->KeyEncounteredName( i ) == "CAST" &&
			!m_ReachedCastPoint )
		{
			m_ReachedCastPoint = kTrue;
			PerformCast( pLevel );
		}
	}

} // CCharacter::UpdateCast()

void CCharacter::PerformCast( CLevel& pLevel )	// level to create pathfinder for
{
	CompleteSpell( pLevel );
} // CCharacter::PerformCast()

bool CCharacter::PerformingCast( void )
{
	if( m_CastTimer > 0 )
	{
		return kTrue;
	}
	return kFalse;
} // CCharacter::PerformingAttack()

bool CCharacter::InInteractionRange( void )
{
	if( TargetItem() == NULL )
	{
		return kFalse;
	}

	D3DXVECTOR3 Delta = TargetItem()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance > 7 )
	{
		return kFalse;
	}

	return kTrue;
} // CCharacter::InInteractionRange()

bool CCharacter::InSpeakingRange( void )
{
	if( Target() == NULL )
	{
		return kFalse;
	}

	D3DXVECTOR3 Delta = Target()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance > 8 )
	{
		return kFalse;
	}

	return kTrue;
} // CCharacter::InSpeakingRange()

bool CCharacter::InOperationRange( void )
{
	if( TargetItem() == NULL )
	{
		return kFalse;
	}

	D3DXVECTOR3 Delta = TargetItem()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance > 14 )
	{
		return kFalse;
	}

	return kTrue;
} // CCharacter::InOperationRange()

bool CCharacter::ValidLineOfSight( CLevel& pLevel )		// a scene to collide with
{
	bool LineOfSightValid( kTrue );

	//if( m_pActiveWeapon != NULL )
	{
		//if( m_pActiveWeapon->Type() == KItemBow ||
		//	m_pActiveWeapon->Type() == KItemCrossbow )
		{
			if( Target() != NULL )
			{
				if( !pLevel.SightUnobstructed( Position() + D3DXVECTOR3( 0, 5, 0 ),
											   Target()->Position() + D3DXVECTOR3( 0, 5, 0 ) ) )
				{
					LineOfSightValid = kFalse;
				}
			}
			else if( TargetItem() != NULL )
			{
				if( !pLevel.SightUnobstructed( Position() + D3DXVECTOR3( 0, 5, 0 ),
											   TargetItem()->Position() + D3DXVECTOR3( 0, 5, 0 ) ) )
				{
					LineOfSightValid = kFalse;
				}
			}
		}
	}
	return LineOfSightValid;
} // CCharacter::ValidLineOfSight()

bool CCharacter::FacingTarget( void )
{
	if( Target() == NULL &&
		TargetItem() == NULL )
	{
		return kFalse;
	}

	return FacingTarget( Target(), TargetItem() );
} // CCharacter::FacingTarget()

bool CCharacter::FacingTarget( CCharacter* pTarget,		// target
							   CItem* pTargetItem )		// target item
{
	if( pTarget == NULL &&
		pTargetItem == NULL )
	{
		return kFalse;
	}

	D3DXVECTOR3 Delta;
	if( pTarget != NULL )
	{
		Delta = pTarget->Position() - Position();
	}
	else
	{
		Delta = pTargetItem->Position() - Position();
	}
	Delta.y = 0;
	// now make sure we're pointing reasonably in the right direction
	D3DXVec3Normalize( &Delta, &Delta );
	// find the angle delta between direction currently facing, 
	// and desired direction
	float32 CurrentAngle = (float32)KRad2Deg * ( (float32)atan2( m_Forward.x, m_Forward.z ) );
	float32 TargetAngle  = (float32)KRad2Deg * ( (float32)atan2( Delta.x, Delta.z ) );

	TargetAngle -= CurrentAngle;

	if( fabs( TargetAngle ) > 30 )
	{
		return kFalse;
	}

	return kTrue;
} // CCharacter::FacingTarget()

bool CCharacter::InAttackRange( CAttackDescription*	pCurrentAttack )	// attack to check
{
	if( Target() == NULL &&
		TargetItem() == NULL )
	{
		return kFalse;
	}

	if( pCurrentAttack == NULL )
	{
		return kFalse;
	}

	if( Target() != NULL )
	{
		D3DXVECTOR3 Delta = Target()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		float32 TargetRadius = ( Target()->CollisionRadius() - 1 );
		if( TargetRadius < 0 )
		{
			TargetRadius = 0;
		}
		Distance -= TargetRadius;
		float32 MyRadius = ( CollisionRadius() - 1 );
		if( MyRadius < 0 )
		{
			MyRadius = 0;
		}
		Distance -= MyRadius;

		if( Distance > pCurrentAttack->AttackRadius() )
		{
			return kFalse;
		}

	}
	else
	{
		D3DXVECTOR3 Delta = TargetItem()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );

		float32 MyRadius = ( CollisionRadius() - 1 );
		if( MyRadius < 0 )
		{
			MyRadius = 0;
		}
		Distance -= MyRadius;

		if( Distance > pCurrentAttack->AttackRadius() )
		{
			return kFalse;
		}
	}
	return kTrue;
} // CCharacter::InAttackRange()

bool CCharacter::InAttackRange( void )
{
	if( Target() == NULL &&
		TargetItem() == NULL )
	{
		return kFalse;
	}
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		return kFalse;
	}
	float32 Radius= m_pCurrentAttack->AttackRadius();

	if( m_pActiveWeapon != NULL )
	{
		if( m_pInventory->SlotEquipped( KSlotRightHand ) &&
			m_pInventory->SlotEquipped( KSlotLeftHand ) )
		{
			// take the min radius for dual wielders
			if( m_pInventory->GetItemFromSlot( KSlotLeftHand )->LeftAttack()->AttackRadius() < Radius )
			{
				Radius = m_pInventory->GetItemFromSlot( KSlotLeftHand )->LeftAttack()->AttackRadius();
			}
			if( m_pInventory->GetItemFromSlot( KSlotRightHand )->RightAttack()->AttackRadius() < Radius )
			{
				Radius = m_pInventory->GetItemFromSlot( KSlotRightHand )->RightAttack()->AttackRadius();
			}
		}
		Radius += m_ReachBonus * m_Scale;
	}


	if( Target() != NULL )
	{
		D3DXVECTOR3 Delta = Target()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		float32 TargetRadius = ( Target()->CollisionRadius() - 1 );
		if( TargetRadius < 0 )
		{
			TargetRadius = 0;
		}
		Distance -= TargetRadius;
		float32 MyRadius = ( CollisionRadius() - 1 );
		if( MyRadius < 0 )
		{
			MyRadius = 0;
		}
		Distance -= MyRadius;

		if( Distance > Radius )
		{
			return kFalse;
		}
	}
	else if( TargetItem() != NULL )
	{
		D3DXVECTOR3 Delta = TargetItem()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		float32 MyRadius = ( CollisionRadius() - 1 );
		if( MyRadius < 0 )
		{
			MyRadius = 0;
		}
		Distance -= MyRadius;

		if( Distance > Radius )
		{
			return kFalse;
		}
	}
	return kTrue;
} // CCharacter::InAttackRange()

float32 CCharacter::AttackRange( void )
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		return 0;
	}
	float32 Radius= m_pCurrentAttack->AttackRadius();

	if( m_pActiveWeapon != NULL )
	{
		if( m_pInventory->SlotEquipped( KSlotRightHand ) &&
			m_pInventory->SlotEquipped( KSlotLeftHand ) )
		{
			// take the min radius for dual wielders
			if( m_pInventory->GetItemFromSlot( KSlotLeftHand )->LeftAttack()->AttackRadius() < Radius )
			{
				Radius = m_pInventory->GetItemFromSlot( KSlotLeftHand )->LeftAttack()->AttackRadius();
			}
			if( m_pInventory->GetItemFromSlot( KSlotRightHand )->RightAttack()->AttackRadius() < Radius )
			{
				Radius = m_pInventory->GetItemFromSlot( KSlotRightHand )->RightAttack()->AttackRadius();
			}
		}
		Radius += m_ReachBonus * m_Scale;
	}

	return Radius;

} // CCharacter::AttackRange()

bool CCharacter::TooCloseForAttack( void )
{
	if( Target() == NULL &&
		TargetItem() == NULL )
	{
		return kFalse;
	}
	float32 Radius = AttackRange();
	if( Radius < 10 )
	{
		return kFalse;
	}

	if( Target() != NULL )
	{
		D3DXVECTOR3 Delta = Target()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );

		if( Distance < Radius * .25f )
		{
			return kTrue;
		}
	}
	
	return kFalse;
} // CCharacter::TooCloseForAttack()

bool CCharacter::InStrikeRange( void )
{
	if( Target() == NULL &&
		TargetItem() == NULL )
	{
		return kFalse;
	}
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		return kFalse;
	}
	return InStrikeRange( Target(), TargetItem() );
} // CCharacter::InStrikeRange()

bool CCharacter::InStrikeRange( CCharacter* pTarget,	// target
							    CItem* pTargetItem )	// target item
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		return kFalse;
	}
	if( pTarget != NULL )
	{
		D3DXVECTOR3 Delta = pTarget->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		float32 TargetRadius = ( pTarget->CollisionRadius() - 1 );
		if( TargetRadius < 0 )
		{
			TargetRadius = 0;
		}
		Distance -= TargetRadius;
		float32 MyRadius = ( CollisionRadius() - 1 );
		if( MyRadius < 0 )
		{
			MyRadius = 0;
		}
		Distance -= MyRadius;

		float32 Radius = m_pCurrentAttack->StrikeRadius();
		if( m_pActiveWeapon != NULL )
		{
			Radius += m_ReachBonus * m_Scale;
		}

		if( Distance > Radius )
		{
			return kFalse;
		}
	}
	else if( pTargetItem != NULL )
	{
		D3DXVECTOR3 Delta = pTargetItem->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		float32 MyRadius = ( CollisionRadius() - 1 );
		if( MyRadius < 0 )
		{
			MyRadius = 0;
		}
		Distance -= MyRadius;

		if( Distance > m_pCurrentAttack->StrikeRadius() )
		{
			return kFalse;
		}
	}
	return kTrue;
} // CCharacter::InStrikeRange()

void CCharacter::CalculateDropEffects( CLevel& pLevel,					// level to send events to
									   CCharacter& pTargetCharacter )	// character to calculate effects for
{
	int32 GoldBonus = (int32)( pTargetCharacter.Inventory()->Gold() * (float32)EffectValue( KEffectPercentGoldDrop ) / 100.0f );
	if( GoldBonus < 0 )
	{
		pTargetCharacter.TakeGold( abs( GoldBonus ) );
	}
	else if( GoldBonus > 0 )
	{
		pTargetCharacter.GiveGold( abs( GoldBonus ) );
	}

	if( EffectValue( KEffectPercentMagicalDrop ) > 0 &&
		!pTargetCharacter.IsPlayer() )
	{
		float32 DropChance = EffectValue( KEffectPercentMagicalDrop );
		if( DropChance > 75 )
		{
			DropChance = 75;
		}
		int32 MinimumLevel = pLevel.LevelDepth() - 2;
		if( MinimumLevel < 1 )
		{
			MinimumLevel = 1;
		}
		MinimumLevel = (int32)ceil( (float32)MinimumLevel * KDifficultyMultiplier[ pLevel.Difficulty() ] );
		MinimumLevel += (int32)KDifficultyLevelBase[pLevel.Difficulty()];	
		if( MinimumLevel < 1 )
		{
			MinimumLevel = 1;
		}
		uint32 TargetLevel = RandomIntegerBetween( MinimumLevel, pLevel.LevelDepth() + 3 );
		if( pLevel.LevelDepth() == 1 && TargetLevel > 2 )
		{
			TargetLevel = 2;
		}
		TargetLevel = (int32)ceil( (float32)TargetLevel * KDifficultyMultiplier[ pLevel.Difficulty() ] );
		TargetLevel += (int32)KDifficultyLevelBase[pLevel.Difficulty()];
		if( TargetLevel < 1 )
		{
			TargetLevel = 1;
		}

		CLList<CItemRef>* pItems = pTargetCharacter.Inventory()->ItemList();
		CLLNode<CItemRef>* Root = pItems->GetFirst();
		uint32 Index( 0 );
		while( Root != NULL )
		{
			if( Root->Data()->m_pItem->Effects( KActivationPassive ) == 0 &&
				Root->Data()->m_pItem->DamageBonuses() == 0 )
			{
				int32 Percent = RandomIntegerBetween( 0, 100 );
				if( Percent < DropChance )
				{
					if( RandomIntegerBetween( 0, 100 ) > 80 )
					{
						Root->Data()->m_pItem->BestowUniquePowers( pLevel.NameGenerator(),
																   TargetLevel );
					}
					else
					{
						Root->Data()->m_pItem->BestowPowers( TargetLevel );
					}
				}
			}
			Root = Root->Next();
		}
	}

} // CCharacter::CalculateDropEffects()

bool CCharacter::RollAttack( CLevel& pLevel,			// level to send text events to
							 CCharacter* pTarget )		// target to attack
{
	if( pTarget == NULL ||
		!pTarget->Alive() )
	{
		return kFalse;
	}

	if( m_pCurrentAttack == NULL )
	{
		return kFalse;
	}
	CAttackDescription* pCurrentAttack = m_pCurrentAttack;
	CItem* pActiveWeapon = m_pActiveWeapon;

	bool CriticalHit( kFalse );
	
	float32 AttackRoll( RandomBetween( 0, 100 ) );
	
	float32 StrikeSkill = 50.0f / ( (float32)SkillPoints( KSkillCriticalStrike ) * .03f + 1 );
	StrikeSkill = 50 - StrikeSkill;
	if( StrikeSkill > 50 )
	{
		StrikeSkill = 50;
	}

//	StrikeSkill += EffectValue( KEffectPercentBlockChance );
	if( StrikeSkill > 50 )
	{
		StrikeSkill = 50;
	}
	float32 Percent = RandomBetween( 0, 100 );

	if( (int32)ceil( AttackRoll ) == 100 ||
		Percent < StrikeSkill )
	{
		CriticalHit = kTrue;
	}
	
	//int32 ToHitChance = ToHit() - pTarget->AC();
	float32 ToHitChance = 100.0f * 
						( (float32)ToHit() / ( (float32)ToHit() + (float32)pTarget->AC() ) ) ;


	if( ToHitChance < 5 )
	{
		ToHitChance = 5;
	}
	if( ToHitChance > 95 )
	{
		ToHitChance = 95;
	}

	/*pLevel.AddEventText( this,
						this,
						Position() + D3DXVECTOR3( 0, 12, 0 ),
						"HIT CHANCE : " + IntegerToString( (int32)ToHitChance ) + "%",
						.75f,
						kTrue,
						1,
						kFalse );*/

	// if our attack roll beats their defense, we hit!
	if( AttackRoll <= ToHitChance )
	{
		// check for block though!

		int32 Damage = CharacterDamage() + RollWeaponDamage();


		if( CriticalHit )
		{
			Damage += Damage;

			m_pCharacterSounds->PlaySample( KSoundCritical,
											m_Position,
											255,
											m_SoundRadius * 2 );
			pLevel.AddEventText( this,
								 pTarget,
								 pTarget->Position() + D3DXVECTOR3( 0, 10, 0 ),
								 LoadLocalizedString(1160),
								 .9f,
								 kFalse,
								 .9f,
								 kTrue );
		}


		EDamageType DamageType( pCurrentAttack->DamageType() );
		if( pActiveWeapon != NULL )
		{
			DamageType = KItemDamageType[pActiveWeapon->Type()];
		}

		Damage = pTarget->ModifyDamage( Damage, DamageType );

		if( pActiveWeapon != NULL )
		{
			for( uint32 i = 0; i < pActiveWeapon->DamageBonuses(); i++ )
			{
				int32 EffectDamage = pActiveWeapon->DamageBonusValue( i );
				EDamageType DamageType = pActiveWeapon->DamageBonusType( i );
				int32 Resistance = (int32)( (float32)pTarget->DamageResistance( DamageType ) / 100.0f * EffectDamage );
				EffectDamage -= Resistance;
				if( EffectDamage < 0 )
				{
					EffectDamage = 0;
				}
				Damage += EffectDamage;
			}
		}

		if( pTarget->Blocked() )
		{
			Damage = 0;

			m_pCharacterSounds->PlaySample( KSoundBlock,
											m_Position,
											255,
											m_SoundRadius * 2 );

			pLevel.AddEventText( this,
								 pTarget,
								 pTarget->Position() + D3DXVECTOR3( 0, 10, 0 ),
								 LoadLocalizedString(1164), //tdc 2008-02-26
								 .75f,
								 kTrue,
								 1 );
		}

		float32 FinalDamage( (float32)Damage );
		if( FinalDamage > pTarget->HP() )
		{
			FinalDamage = (float32)pTarget->HP();
		}

		float32 StealPercentage = (float32)EffectValue( KEffectPercentLifeStolen );
		if( StealPercentage > 100 )
		{
			StealPercentage = 100;
		}
		if( StealPercentage < -100 )
		{
			StealPercentage = -100;
		}
		float32 LifeSteal = (float32)FinalDamage * StealPercentage / 100.0f;
		if( LifeSteal > 0 )
		{
			ModifyHP( LifeSteal );
			pLevel.AddEventText( this,
								this,
								Position() + D3DXVECTOR3( 0, 10, 0 ),
								LoadLocalizedString1Variable(1109, IntegerToString( (int32)ceil( LifeSteal ) ) ),
								.75f,
								kFalse,
								1 );

		}
		else if( LifeSteal < 0 )
		{
			ModifyHP( LifeSteal );
			pLevel.AddEventText( this,
								this,
								Position() + D3DXVECTOR3( 0, 10, 0 ),
								LoadLocalizedString1Variable(1110, IntegerToString( (int32)ceil( LifeSteal ) ) ),
								.75f,
								kTrue,
								1,
								kTrue );

		}
		StealPercentage = (float32)EffectValue( KEffectPercentManaStolen );
		if( StealPercentage > 100 )
		{
			StealPercentage = 100;
		}
		if( StealPercentage < -100 )
		{
			StealPercentage = -100;
		}
		float32 ManaSteal = (float32)FinalDamage * StealPercentage / 100.0f;
		if( ManaSteal != 0 )
		{
			ModifyMana( ManaSteal );
		}

		float32 KnockbackValue = EffectValue( KEffectKnockback );
		KnockbackValue += pCurrentAttack->EffectValue( KActivationUsage, KEffectKnockback );
		if( pActiveWeapon != NULL )
		{
			KnockbackValue += pActiveWeapon->EffectValue( KActivationUsage, KEffectKnockback );
		}
		if( KnockbackValue > 0 )
		{

			CEffect* pEffect = new CEffect( KEffectKnockbackEffect,
											kFalse,
											KActivationUsage,
											(int32)( 2 + KnockbackValue * .5f ),
											2.0f + KnockbackValue * .1f,
											0 );

			pEffect->SetName( "KNOCKBACK" );
			D3DXVECTOR3 KnockbackDirection = pTarget->Position() - Position();
			KnockbackDirection.y = 0;
			D3DXVec3Normalize( &KnockbackDirection, &KnockbackDirection );
			
			pEffect->SetDirection( KnockbackDirection );

			pTarget->AddNewEffect( pLevel, pEffect );

		}

		float32 LastHP = (float32)pTarget->HP();

		pTarget->ApplyDamage( pLevel,
							   this,
							   (float32)Damage );



		if( pTarget != NULL )
		{
			D3DXVECTOR3 Delta = Position() - pTarget->Position();
			D3DXVec3Normalize( &Delta, &Delta );
			Delta *= pTarget->CollisionRadius();
			Delta += pTarget->Position();
			Delta.y = GetMaxBounds().y * .5f + Position().y;
			m_pParticleManager.AddParticle( m_StrikeParticle, Delta );
		}

		// if we just killed them, award ourselves their
		// experience award
		if( pTarget != NULL &&
			pTarget->Alive() &&
			pTarget->HP() <= 0 &&
			LastHP > 0 )
		{
			// do some magic/gold drop effects, if required

			CalculateDropEffects( pLevel,
								  *pTarget );
			AwardExperience( pLevel, pTarget, pTarget->ExperienceAward() );
			AwardFame( pLevel, pTarget, pTarget->FameAward() );
			if( pTarget->IsUnique() )
			{
				IncrementStat( KJournalStatFamousMonstersKilled );
				if( HasMaster() )
				{
					Master()->IncrementStat( KJournalStatFamousMonstersKilled );
				}
			}
			IncrementStat( KJournalStatMonstersKilled );
			if( HasMaster() )
			{
				Master()->IncrementStat( KJournalStatMonstersKilled );
			}

		}

		if( Damage > 0 &&
			pCurrentAttack != NULL )
		{

			// apply any attack effects because of successful strike

			for( uint32 i = 0; i < pCurrentAttack->Effects( KActivationUsage ); i++ )
			{
				int32 Chance = RandomIntegerBetween( 0, 100 );
				if( pCurrentAttack->Effect( KActivationUsage, i )->Type() != KEffectKnockback &&
					Chance <= pCurrentAttack->Effect( KActivationUsage, i )->ChanceOfSuccess() )
				{
					if( pTarget != NULL &&
						pTarget->Alive() )
					{
						CEffect* pEffect = pTarget->AddEffect( pLevel, pCurrentAttack->Effect( KActivationUsage, i ) );
						CSpellDescription* pSpell = pLevel.SpellManifest()->SpellDescription( pEffect->Name() );
						if( pSpell != NULL )
						{
							pEffect->SetParentSpell( pSpell );
						}
					}
					/*if( pCurrentAttack->Effect( KActivationUsage, i )->HasMessage() )
					{
						pLevel.AddEventText( this,
											 m_pTargetCharacter,
											 m_pTargetCharacter->Position() + D3DXVECTOR3( 0, 10, 0 ),
											 pCurrentAttack->Effect( KActivationUsage, i )->Message(),
											 .75f,
											 kTrue );
					}*/
				}
			}

			// apply any attack effects because of successful strike from a weapon

			if( pActiveWeapon != NULL )
			{
				for( int32 i = 0; i < pActiveWeapon->Effects( KActivationUsage ); i++ )
				{
					int32 Chance = RandomIntegerBetween( 0, 100 );
					if( pCurrentAttack->Effect( KActivationUsage, i )->Type() != KEffectKnockback &&
						Chance <= pActiveWeapon->Effect( KActivationUsage, i )->ChanceOfSuccess() )
					{
						if( pTarget != NULL &&
							pTarget->Alive() )
						{
							pTarget->AddEffect( pLevel, pActiveWeapon->Effect( KActivationUsage, i ) );
						}

					}
				}
			}
		}

		return kTrue;
	}

	if( pTarget != NULL )
	{
		pLevel.AddEventText( this,
							pTarget,
							pTarget->Position() + D3DXVECTOR3( 0, 9, 0 ),
							LoadLocalizedString(1111),
							.75f,
							kFalse,
							2.0f );
	}

	return kFalse;

} // CCharacter::RollAttack()

bool CCharacter::Blocked( void )
{
	// if a shield is equipped
	if( m_pInventory->GetItemFromSlot( KSlotLeftArm ) != NULL )
	{
		float32 BlockSkill = 50.0f / ( (float32)SkillPoints( KSkillShield ) * .03f + 1 );
		BlockSkill = 50 - BlockSkill;
		BlockSkill += EffectValue( KEffectPercentBlockChance );
		if( BlockSkill > 50 )
		{
			BlockSkill = 50;
		}
		int32 Percent = RandomIntegerBetween( 0, 100 );
		if( Percent != 100 && 
			Percent != 0 &&
			Percent < BlockSkill )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::Blocked()

// calculate to hit percentage
int32 CCharacter::ToHit( void )
{
	int32 ToHitBonusValue = 50 + Dexterity() / 2 + ToHitBonus() + m_Level;

	ToHitBonusValue = ToHitBonusValue + (int32)ceil( ToHitBonusValue * (float32)EffectValue( KEffectPercentToHitBonus ) / 100.0f );
	ToHitBonusValue += (int32)EffectValue( KEffectToHitBonus );

	return ToHitBonusValue;
} // CCharacter::ToHit()

// calculate to hit percentage
int32 CCharacter::BaseToHit( void )
{
	return 50 + BaseDexterity() / 2 + BaseToHitBonus() + m_Level;
} // CCharacter::BaseToHit()

int32 CCharacter::NaturalArmor( void )
{
	if( HasMaster() && 
		Master()->IsPlayer() &&
		!IsSummoned() )
	{
		// pet gets a natural armor boost to make them competitive,
		// since they can't wear armor. 
		return m_NaturalArmor + Level() * 3;
	}
	return m_NaturalArmor;
} // CCharacter::NaturalArmor()

// calculate armor class
int32 CCharacter::AC( void )
{
	return ArmorBonus() + DexterityBonus();
} // CCharacter::AC()

int32 CCharacter::BaseAC( void )
{
	return BaseArmorBonus() + BaseDexterityBonus();
} // CCharacter::BaseAC()

void CCharacter::CalculateMaxHP( void )
{
	// pet hp leveling
	if( Master() != NULL &&
		Master()->IsPlayer() &&
		!IsSummoned() )
	{
		m_MaxHP = 4 * Vitality() +
				  4 * m_Level +
				  18;

		if( m_HP > MaxHP() )
		{
			m_HP = (float32)MaxHP();
		}
	}
	
	if( m_HP > MaxHP() )
	{
		m_HP = (float32)MaxHP();
	}
} // CCharacter::CalculateMaxHP()

void CCharacter::CalculateMaxStamina( void )
{
	m_MaxStamina = 2 * Vitality() +
			  m_Level;

	if( m_Stamina > MaxStamina() )
	{
		m_Stamina = (float32)MaxStamina();
	}
} // CCharacter::CalculateMaxStamina()

void CCharacter::UpdateHP( CLevel& pLevel,			// level to post message to
						   float32 TimeElapsed )	// time elapsed in seconds
{
	if( !Alive() )
	{
		return;
	}

	// invincible pets heal slowly over time, so they aren't constantly fleeing
	if( Invincible() && Master() != NULL)
	{
		// effect values are per minute ( otherwise everything is WAY too fast
		ModifyHP( TimeElapsed * .032f );
	}

	// effect values are per minute ( otherwise everything is WAY too fast
	ModifyHP( EffectValue( KEffectHP ) * TimeElapsed * .016f );

	// if an effect killed us, see if there was an owner to that effect, so that we can give
	// them experience
	if( HP() <= 0 )
	{
		for( uint32 i = 0; i < KActivationTypes; i++ )
		{
			for( int32 j = 0; j < (int32)m_pEffects[i].size(); j++ )
			{
				// detect a poisoned state
				if( m_pEffects[i][j]->Type() == KEffectHP &&
					m_pEffects[i][j]->Value() < 0 &&
					m_pEffects[i][j]->Owner() != NULL )
				{
					
					CalculateDropEffects( pLevel,
										  *m_pEffects[i][j]->Owner() );
					m_pEffects[i][j]->Owner()->AwardExperience( pLevel, this, ExperienceAward() );
					m_pEffects[i][j]->Owner()->IncrementStat( KJournalStatMonstersKilled );
					if( HasMaster() )
					{
						Master()->IncrementStat( KJournalStatMonstersKilled );
					}
					if( IsUnique() )
					{
						m_pEffects[i][j]->Owner()->IncrementStat( KJournalStatFamousMonstersKilled );
						if( HasMaster() )
						{
							Master()->IncrementStat( KJournalStatFamousMonstersKilled );
						}
					}
					m_pEffects[i][j]->Owner()->AwardFame( pLevel, this, FameAward() );
					m_pEffects[i][j]->SetOwner( NULL );
					// don't want to do it more than once - only 1 person gets the XP!
					break;
				}
			}
		}
	}
} // CCharacter::UpdateHP()

void CCharacter::UpdateMana( float32 TimeElapsed )	// time elapsed in seconds
{
	if( !Alive() )
	{
		return;
	}
	ModifyMana( (float32)MaxMana() / 35 * TimeElapsed );
	ModifyMana( EffectValue( KEffectMana ) * TimeElapsed * .16f );
} // CCharacter::UpdateMana()

void CCharacter::UpdateStamina( CLevel& pLevel,			// level we are on
								float32 TimeElapsed )	// time elapsed in seconds
{
	if( !Alive() ||
		m_MaxStamina == -1 )
	{
		return;
	}
	float32 LastStamina( m_Stamina );

	if( pLevel.LevelDepth() == 0)
	{
		m_Stamina = (float32)MaxStamina();
	}

	if( Running() && Pathing() )
	{
		m_Stamina -= 2.0f * TimeElapsed;
		if( m_Stamina < 0 )
		{
			m_Stamina = 0;
		}
		if( m_Stamina != 0 )
		{
			ModifyStamina( EffectValue( KEffectStamina ) * TimeElapsed / 60.0f );
		}
	}
	else if( !Running() || !Pathing() )
	{
		if( m_Stamina > 0 || !Pathing() )
		{
			if( !Pathing() )
			{
				ModifyStamina( (float32)MaxStamina() / 25 * TimeElapsed );
			}
			else
			{
				ModifyStamina( (float32)MaxStamina() / 2 / 25 * TimeElapsed );
			}

			ModifyStamina( EffectValue( KEffectStamina ) * TimeElapsed / 60.0f );
		}
	}
	if( m_Stamina == 0 &&
		LastStamina > 0 &&
		!m_PlayedStaminaLow )
	{
		PlayHelpSample( KSoundHelpStaminaLow );
		m_PlayedStaminaLow = kTrue;
	}
	if( m_Stamina == 0 &&
		!HasMaster() &&
		IsPlayer() )
	{
		// tip about spells
		if( m_pSettings.GetSettings( KSetShowTips ) &&
			!pLevel.Tips().TipSeen( KTipStamina ) )
		{
			pLevel.RequestTip( pLevel.Tips().ContextTip(KTipStamina ) );
			pLevel.Tips().SetTipSeen( KTipStamina, kTrue );
		}

		SetRunning( kFalse );
	}

	if( m_Stamina > MaxStamina() / 2 )
	{
		m_PlayedStaminaLow = kFalse;
	}
} // CCharacter::UpdateStamina()

void CCharacter::CalculateMaxMana( void )
{
	m_MaxMana = 2 * Magic() +
			  m_Level;

	if( m_Mana > MaxMana() )
	{
		m_Mana = (float32)MaxMana();
	}
} // CCharacter::CalculateMaxMana()

bool CCharacter::RequirementMet( EStatistic Statistic,	// statistic to check
								 int32 Value,			// minimum value
								 bool AllowModification )	// allow modification due to effects
{

	int32 OriginalValue( Value );
	if( AllowModification )
	{
		float32 RequirementReduction = EffectValue( KEffectPercentItemRequirements );
		if( RequirementReduction > 75 )
		{
			RequirementReduction = 75;
		}
		Value = (int32)( (float32)Value - (float32)Value * RequirementReduction / 100.0f );
	}

	// satistics above the base ones map DIRECTLY to skills
	if( Statistic > KStatLineage )
	{
		ESkill Skill = (ESkill)((int32)Statistic - KStatSkillSword );
		if( SkillPoints( Skill ) < Value )
		{
			return kFalse;
		}
		return kTrue;
	}
	switch( Statistic )
	{
	case KStatStrength :
		if( Strength() < Value )
		{
			return kFalse;
		}
		break;
	case KStatDexterity :
		if( Dexterity() < Value )
		{
			return kFalse;
		}
		break;
	case KStatVitality :
		if( Vitality() < Value )
		{
			return kFalse;
		}
		break;
	case KStatMagic :
		if( Magic() < Value )
		{
			return kFalse;
		}
		break;
	case KStatLevel :
		if( Level() < Value )
		{
			return kFalse;
		}
		break;
	case KStatRenown :
		if( FameRank() < OriginalValue )
		{
			return kFalse;
		}
		break;
	case KStatLineage :
		if( Lineage() < OriginalValue )
		{
			return kFalse;
		}
		break;
	}
	return kTrue;
} // CCharacter::RequirementMet()

void CCharacter::UpdateEffects( CLevel& pLevel,			// level to send event effects to
								float32 TimeElapsed )	// time elapsed in seconds
{
	bool WasPoisoned( m_Poisoned );
	m_Poisoned = kFalse;

	m_KnockbackVelocity = D3DXVECTOR3( 0, 0, 0 );

	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)m_pEffects[i].size(); j++ )
		{

			m_pEffects[i][j]->UpdateTime( TimeElapsed );



			// detect a poisoned state
			if( m_pEffects[i][j]->Type() == KEffectHP &&
				m_pEffects[i][j]->Value() < 0 &&
				m_pEffects[i][j]->Duration() != KDurationInstant )
			{
				if( !WasPoisoned )
				{
					PlayHelpSample( KSoundHelpPoison, kTrue );
				}
				m_Poisoned = kTrue;
			}

			CEffect* pEffect = m_pEffects[i][j];
			if( m_pEffects[i][j]->Type() == KEffectKnockbackEffect &&
				m_pEffects[i][j]->Activation() == KActivationUsage )
			{
				m_KnockbackVelocity += m_pEffects[i][j]->Value() * m_pEffects[i][j]->Direction() * 10.0f;
				m_pEffects[i][j]->SetValue( m_pEffects[i][j]->Value() * (float32)pow( .01f, TimeElapsed ) );
			}

			if( m_pEffects[i][j]->Duration() == KDurationInstant )
			{
				ApplyInstantEffect( pLevel, m_pEffects[i][j] );
			}
		}
	}

	float32 KnockbackLength = D3DXVec3Length( &m_KnockbackVelocity );
	if( KnockbackLength > 100 )
	{
		D3DXVec3Normalize( &m_KnockbackVelocity, &m_KnockbackVelocity );
		m_KnockbackVelocity *= 100;
	}
} // CCharacter::UpdateEffects()

void CCharacter::RemoveDeadEffects( void )
{
	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)m_pEffects[i].size(); j++ )
		{
			if( m_pEffects[i][j]->Duration() == KDurationInstant ||
				m_pEffects[i][j]->Expired() )
			{
				DELETE_SAFELY( m_pEffects[i][j] );
				m_pEffects[i][j] = m_pEffects[i][ m_pEffects[i].size() - 1 ];
				m_pEffects[i].pop_back();
				j--;
			}
		}
	}
} // CCharacter::RemoveDeadEffects()

void CCharacter::ApplyInstantEffect( CLevel& pLevel,	// level to send event effects to
									 CEffect* pEffect )	// effect to apply
{
	m_pCurrentAttack = NULL;
	float32 Value = pEffect->Value();
	int32 Damage;
	int32 Iterator;
	uint32 Iterator2;
	uint32 i;
	CSpellDescription* pDescription;
	CCharacterDescription* pCharDescription;
	bool Bolded( kTrue );
	bool Secondary( kFalse );

	// select a value in range, if required
	if( pEffect->RangedValue() )
	{
		if( pEffect->Value2() < Value )
		{
			Value = RandomBetween( pEffect->Value2(), Value );
		}
		else 
		{
			Value = RandomBetween( Value, pEffect->Value2() );
		}
	}
	switch( pEffect->Type() )
	{
	case KEffectRemove :
		RemoveEffect( pEffect->Name() );
		break;
	case KEffectFlee :
		if( !IsPlayer() &&
			!( pEffect->DamageType() == KDamageUndead &&
			   DamageResistance( KDamageUndead ) >= 100 ) &&
			   Level() < Value )
		{
			StopPathing();
			SetDestination( pLevel,
							RandomBetween( m_Position.x - 100, m_Position.x + 100 ),
							RandomBetween( m_Position.z - 100, m_Position.z + 100 ) );

			SetAIState( KAIFlee );
			pLevel.AddEventText( this,
								this,
								Position() + D3DXVECTOR3( 0, 10, 0 ),
								LoadLocalizedString(1112),
								.8f,
								kTrue,
								.9f );
		}
		break;
	case KEffectHP :
		if( pEffect->Owner() != NULL &&
			pEffect->Owner()->IsPlayer() )
		{
			Bolded = kFalse;
			Secondary = kTrue;
		}
		else if( IsPlayer() ||
				( HasMaster() && Master()->IsPlayer() ) )
		{
			Secondary = kTrue;
		}
		if( Value < 0 )
		{
			Damage = ModifyDamage( (int32)fabs( Value ), pEffect->DamageType() );
			Damage *= -1;
			ModifyHP( (float32)Damage );
			pLevel.AddEventText( this,
								this,
								Position() + D3DXVECTOR3( 0, 10, 0 ),
								LoadLocalizedString1Variable(1113, IntegerToString( (int32)Damage ) ),
								.75f,
								Bolded,
								1,
								Secondary );
		}
		else
		{
			ModifyHP( Value );
			pLevel.AddEventText( this,
								this,
								Position() + D3DXVECTOR3( 0, 10, 0 ),
								LoadLocalizedString1Variable(1109, IntegerToString( (int32)Value ) ),
								.75f,
								kTrue,
								1 );
		}

		break;
	case KEffectMana :
		ModifyMana( Value );
		break;
	case KEffectStamina :
		ModifyStamina( Value );
		break;
	case KEffectRevertTransform :
		if( m_TransformationDuration != 0 )
		{
			m_TransformationDuration = .001f;
		}
		break;
	case KEffectTransform :

		Transform( pLevel.D3DDevice(),
				   &pLevel,
				   *pLevel.CharacterManifest(),
				   pEffect->Name(),
				   pEffect->Value(),
				   kTrue );
		break;
	case KEffectOpenPortal :
		OpenPortal( pLevel );
		break;
	case KEffectDiscover :
		pLevel.DiscoverAll();
		break;
	case KEffectWriteSpell :
		pDescription = m_pSpellManifest.SpellDescription( pEffect->Name() );
		if( pDescription != NULL )
		{
			int32 FreeSlot = FreeSpellSlot( pDescription->Sphere() );
			if( FreeSlot != -1 )
			{
				LearnSpell( pDescription, FreeSlot );
			}
		}
		if( IsPlayer() )
		{
			PlayHelpSample( KSoundHelpNewSpell );

			// tip about spells
			if( m_pSettings.GetSettings( KSetShowTips ) &&
				!pLevel.Tips().TipSeen( KTipWriteSpell ) )
			{
				pLevel.RequestTip( pLevel.Tips().ContextTip(KTipWriteSpell ) );
				pLevel.Tips().SetTipSeen( KTipWriteSpell, kTrue );
			}
		}
		break;
	case KEffectDispel :
		// de-summon any summoned creatures
		for( int32 i = 0; i < Pets(); i++ )
		{
			if( Pet( i )->IsSummoned() )
			{
				Pet( i )->SetLifeDuration( .01f );
			}
		}
		break;
	case KEffectSummon :
		pCharDescription = pLevel.CharacterManifest()->CharacterDescription( pEffect->Name() );

		if( pCharDescription != NULL )
		{
			for( Iterator = 0; Iterator < (int32)pEffect->Value(); Iterator++ )
			{
				if( Pets() < KMaximumSummonedCreatures )
				{
					CCharacter* pNewMonster = pLevel.AddMonster( *pCharDescription,
																pLevel.RandomOpenPosition( Position(), 12 ) );
					pNewMonster->TweakToLevel( (int32)pEffect->Value2() );
					pLevel.GiveEquipment( *pCharDescription,
										  *pNewMonster );

					pNewMonster->SetAllowDropInventory( kFalse );
					pNewMonster->SetName( LoadLocalizedString1Variable(1114, pNewMonster->Name() ));
					// make it a pet!
					AddPet( pNewMonster );
					pNewMonster->SetLifeDuration( pEffect->Value3() );

					for( Iterator2 = 0; Iterator2 < pEffect->SFXItems(); Iterator2++ )
					{
						pLevel.AddSFX( *pEffect->SFX( Iterator2 ), 
									   this,
									   pNewMonster,
									   Position(),
									   pNewMonster->Position() );
					}
				}
			}

		}
		break;
	}
} // CCharacter::ApplyInstantEffect()

void CCharacter::OpenPortal( CLevel& pLevel )	// level to open portal in 
{
	if( pLevel.LevelDepth() == 0 )
	{
		return;
	}
	IncrementStat( KJournalStatPortalsUsed );

	m_HasOpenPortal = kTrue;
	m_PortalDepth = pLevel.LevelDepth();
	m_PortalPosition = pLevel.RandomOpenPosition( Position(), 4, 12 );

	CreatePortals( pLevel );
} // CCharacter::OpenPortal()

void CCharacter::CreatePortals( CLevel& pLevel )	// level to open portal in 
{
	if( !m_HasOpenPortal )
	{
		return;
	}

	// if this isn't the level we opened it on, don't create it.
	if( pLevel.LevelDepth() != m_PortalDepth &&
		pLevel.LevelDepth() != 0 )
	{
		return;
	}

	pLevel.DeletePortals();
	D3DXVECTOR3 Position = m_PortalPosition;
	if( pLevel.LevelDepth() == 0 )
	{
		Position = pLevel.PositionNearStairsUp();
	}

	CItemBlank* pItem = NULL;
	if( pLevel.LevelDepth() == 0 )
	{
		pItem =  new CItemBlank( pLevel.D3DDevice(),
								 m_pRefManager,
								 m_pSettings,
								 m_pParticleManager,
							     m_pGlowManager,
								 m_pPositionalAudioManager,
								 LoadLocalizedString1Variable(1115, IntegerToString( m_PortalDepth )),
								 Position,
								 D3DXVECTOR3( -4, 0, -4 ),
								 D3DXVECTOR3( 4, 6, 4 ),
								 KItemDungeonPortal );
	}
	else
	{
		pItem =  new CItemBlank( pLevel.D3DDevice(),
								 m_pRefManager,
								 m_pSettings,
								 m_pParticleManager,
							     m_pGlowManager,
								 m_pPositionalAudioManager,
								 LoadLocalizedString(1116),
								 Position,
								 D3DXVECTOR3( -4, 0, -4 ),
								 D3DXVECTOR3( 4, 6, 4 ),
								 KItemTownPortal );
	}
	if( pItem != NULL )
	{
		pLevel.AddItem( pItem, Position );
	}

	CSFXDescription* pSFX = new CSFXDescription( KSFXStationaryPortal,
												 "SPELLS\\EFFECTS\\PORTAL\\portal.ams",
											     -1,
											     kTrue,
												 0,
												 0,
											     kFalse,
												 kFalse );

	pItem->SetAmbientPositionalAudio( "SOUNDS\\SPELLS\\portalloop.wav" );

	pLevel.AddSFX( *pSFX,
				   NULL,
				   NULL,
				   pItem->Position(),
				   pItem->Position() );

	DELETE_SAFELY( pSFX );


	pSFX = new CSFXDescription( KSFXStationaryPortal,
								"SPELLS\\EFFECTS\\DISPEL\\dispel.mdl",
								1,
								kTrue,
								0,
								0,
								kFalse,
								kFalse );

	pLevel.AddSFX( *pSFX,
				   NULL,
				   NULL,
				   pItem->Position(),
				   pItem->Position() );

	DELETE_SAFELY( pSFX );

} // CCharacter::CreatePortals()

void CCharacter::RemoveEffect( std::string Name )	// name of effect to remove
{
	Name = StringUpper( Name );
	for( int32 j = 0; j < (int32)m_pEffects[KActivationUsage].size(); j++ )
	{
		if( m_pEffects[KActivationUsage][j]->Type() != KEffectRemove &&
			m_pEffects[KActivationUsage][j]->HasName() &&
			StringUpper( m_pEffects[KActivationUsage][j]->Name() ) == Name )
		{
			DELETE_SAFELY( m_pEffects[KActivationUsage][j] );
			m_pEffects[KActivationUsage][j] = m_pEffects[KActivationUsage][ m_pEffects[KActivationUsage].size() - 1 ];
			m_pEffects[KActivationUsage].pop_back();
			j--;
		}

	}
} // CCharacter::RemoveEffect()

bool CCharacter::HasEffect( std::string Name )	// name of effect to remove
{
	Name = StringUpper( Name );
	for( int32 j = 0; j < (int32)m_pEffects[KActivationUsage].size(); j++ )
	{
		if( m_pEffects[KActivationUsage][j]->Type() != KEffectRemove &&
			m_pEffects[KActivationUsage][j]->HasName() &&
			StringUpper( m_pEffects[KActivationUsage][j]->Name() ) == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::HasEffect()

bool CCharacter::HasNaturalEffect( std::string Name )	// name of effect to remove
{
	Name = StringUpper( Name );
	for( int32 j = 0; j < (int32)m_pNaturalEffects[KActivationUsage].size(); j++ )
	{
		if( m_pNaturalEffects[KActivationUsage][j]->Type() != KEffectRemove &&
			m_pNaturalEffects[KActivationUsage][j]->HasName() &&
			StringUpper( m_pNaturalEffects[KActivationUsage][j]->Name() ) == Name )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CCharacter::HasNaturalEffect()

float32 CCharacter::CalculateEffectValue( EEffectType Type )				// type of effect
{
	float32 Value( 0 );

	for( uint32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
		{
			if( m_pEffects[i][j]->Type() == Type )
			{
				
				Value += m_pEffects[i][j]->Value();
			}
		}
	}

	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pNaturalEffects[i].size(); j++ )
		{
			if( m_pNaturalEffects[i][j]->Type() == Type )
			{
				Value += m_pNaturalEffects[i][j]->Value();
			}
		}
	}
	return Value;
} // CCharacter::CalculateEffectValue()

uint32 CCharacter::Effects( EEffectActivation ActivationType )
{
	return m_pEffects[ActivationType].size();
} // CCharacter::Effects()

CEffect* CCharacter::Effect( EEffectActivation ActivationType,	// type of activation
						uint32 Index )						// index within that type
{
	return m_pEffects[ActivationType][Index];
} // CCharacter::Effect()


float32 CCharacter::EffectValue( EEffectType Type )				// type of effect
{
	return m_EffectValue[Type];
} // CCharacter::EffectValue()

void CCharacter::CalculateEffectValues( void )
{
	for( uint32 i = 0; i < KAllEffectTypes; i++ )
	{
		m_EffectValue[i] = CalculateEffectValue( (EEffectType)i ) + m_pInventory->EffectValue( (EEffectType)i );
		// if this effect is susceptible to magical resistance, and it is negative,
		// then we reduce it appropriately
		if( KEffectIsMagic[i] &&
			sgn( (float32)m_EffectValue[i] ) != sgn( (float32)KEffectMaximum[i] ) )
		{
			m_EffectValue[i] -= (int32)( (float32)DamageResistance( KDamageMagical ) / 100.0f * (float32)m_EffectValue[i] );
		}

	}
} // CCharacter::CalculateEffectValues()

CEffect* CCharacter::AddEffect( CLevel& pLevel,		// level to send text events to
							CEffect* pEffect,	// effect to add
						    int32 Bonus,		// bonuses to apply
							CCharacter* pOwner, // owner of the effect
							bool DoSFX )		// perform sfx
{
	if( pEffect->Exclusive() && HasEffect( pEffect->Name() ) )
	{
		std::string Name = StringUpper( pEffect->Name() );
		for( int32 j = 0; j < (int32)m_pEffects[KActivationUsage].size(); j++ )
		{
			if( m_pEffects[KActivationUsage][j]->Type() != KEffectRemove &&
				m_pEffects[KActivationUsage][j]->HasName() &&
				StringUpper( m_pEffects[KActivationUsage][j]->Name() ) == Name )
			{
				// make it expire
				m_pEffects[KActivationUsage][j]->UpdateTime( (float32)m_pEffects[KActivationUsage][j]->Duration() );
			}
		}
	}
	return AddNewEffect( pLevel, new CEffect( pEffect ), Bonus, pOwner, DoSFX );
} // CCharacter::AddEffect()

CEffect* CCharacter::AddNewEffect( CLevel& pLevel,		// level to send text events to
							   CEffect* pEffect,	// effect to add
							   int32 Bonus,			// bonuses to apply
							   CCharacter* pOwner, // owner of the effect
								bool DoSFX )		// perform sfx
{
	if( pEffect->Exclusive() &&
		HasEffect( pEffect->Name() ) )
	{
		std::string Name = StringUpper( pEffect->Name() );
		for( int32 j = 0; j < (int32)m_pEffects[KActivationUsage].size(); j++ )
		{
			if( m_pEffects[KActivationUsage][j]->Type() != KEffectRemove &&
				m_pEffects[KActivationUsage][j]->HasName() &&
				StringUpper( m_pEffects[KActivationUsage][j]->Name() ) == Name )
			{
				// make it expire
				m_pEffects[KActivationUsage][j]->UpdateTime( (float32)m_pEffects[KActivationUsage][j]->Duration() );
			}
		}
	}
	pEffect->SetOwner( pOwner );
	pEffect->SetValue( pEffect->Value() + pEffect->ValueBonus() * (float32)Bonus );
	pEffect->SetValue2( pEffect->Value2() + pEffect->Value2Bonus() * (float32)Bonus );
	pEffect->SetValue3( pEffect->Value3() + pEffect->Value3Bonus() * (float32)Bonus );
	pEffect->SetDuration( pEffect->Duration() + pEffect->DurationBonus() * Bonus );
	pEffect->SetChanceOfSuccess( pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonus() * Bonus );

	pEffect->SetValue( pEffect->Value() + pEffect->Value() * pEffect->ValueBonusPercent() / 100.0f * (float32)Bonus );
	pEffect->SetValue2( pEffect->Value2() + pEffect->Value2() * pEffect->Value2BonusPercent() / 100.0f * (float32)Bonus );
	pEffect->SetValue3( pEffect->Value3() + pEffect->Value3() * pEffect->Value3BonusPercent() / 100.0f * (float32)Bonus );
	pEffect->SetDuration( pEffect->Duration() + (int32)( (float32)pEffect->Duration() * pEffect->DurationBonusPercent() / 100.0f * Bonus ) );
	pEffect->SetChanceOfSuccess( pEffect->ChanceOfSuccess() + (int32)( (float32)pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonusPercent() / 100.0f * Bonus ) );

	if( pEffect->HasMessage() )
	{
		pLevel.AddEventText( this,
							 this,
							 Position() + D3DXVECTOR3( 0, 9, 0 ),
							 pEffect->Message(),
							 1.0f,
							 kTrue,
							 .6f );
	}

	if( DoSFX )
	{
		for( uint32 i = 0; i < pEffect->SFXItems(); i++ )
		{
			float32 DurationBonus = pEffect->SFXDurationBonus() * Bonus;

			pLevel.AddSFX( *pEffect->SFX( i ), 
							this,
							this,
							Position(),
							Position(),
							DurationBonus );
		}
	}

	m_pEffects[pEffect->Activation()].push_back( pEffect );
	return pEffect;
} // CCharacter::AddNewEffect()

void CCharacter::AddNaturalEffect( CEffect* pEffect,	// effect to add
								   int32 Bonus,		// bonuses to apply
								   CCharacter* pOwner )// owner of the effect
{
	if( pEffect->Exclusive() && HasNaturalEffect( pEffect->Name() ) )
	{
		std::string Name = StringUpper( pEffect->Name() );
		for( int32 j = 0; j < (int32)m_pNaturalEffects[KActivationUsage].size(); j++ )
		{
			if( m_pNaturalEffects[KActivationUsage][j]->Type() != KEffectRemove &&
				m_pNaturalEffects[KActivationUsage][j]->HasName() &&
				StringUpper( m_pNaturalEffects[KActivationUsage][j]->Name() ) == Name )
			{
				// make it expire
				m_pNaturalEffects[KActivationUsage][j]->UpdateTime( (float32)m_pNaturalEffects[KActivationUsage][j]->Duration() );
			}
		}
	}

	AddNewNaturalEffect( new CEffect( pEffect ), Bonus, pOwner );
} // CCharacter::AddNaturalEffect()

void CCharacter::AddNewNaturalEffect( CEffect* pEffect,	// effect to add
									  int32 Bonus,			// bonuses to apply
									  CCharacter* pOwner )	// owner of the effect
{
	if( pEffect->Exclusive() &&
		HasNaturalEffect( pEffect->Name() ) )
	{
		std::string Name = StringUpper( pEffect->Name() );
		for( int32 j = 0; j < (int32)m_pNaturalEffects[KActivationUsage].size(); j++ )
		{
			if( m_pNaturalEffects[KActivationUsage][j]->Type() != KEffectRemove &&
				m_pNaturalEffects[KActivationUsage][j]->HasName() &&
				StringUpper( m_pNaturalEffects[KActivationUsage][j]->Name() ) == Name )
			{
				// make it expire
				m_pNaturalEffects[KActivationUsage][j]->UpdateTime( (float32)m_pNaturalEffects[KActivationUsage][j]->Duration() );
			}
		}
	}
	pEffect->SetOwner( pOwner );
	pEffect->SetValue( pEffect->Value() + pEffect->ValueBonus() * (float32)Bonus );
	pEffect->SetValue2( pEffect->Value2() + pEffect->Value2Bonus() * (float32)Bonus );
	pEffect->SetValue3( pEffect->Value3() + pEffect->Value3Bonus() * (float32)Bonus );
	pEffect->SetDuration( pEffect->Duration() + pEffect->DurationBonus() * Bonus );
	pEffect->SetChanceOfSuccess( pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonus() * Bonus );

	pEffect->SetValue( pEffect->Value() + pEffect->Value() * pEffect->ValueBonusPercent() / 100.0f * (float32)Bonus );
	pEffect->SetValue2( pEffect->Value2() + pEffect->Value2() * pEffect->Value2BonusPercent() / 100.0f * (float32)Bonus );
	pEffect->SetValue3( pEffect->Value3() + pEffect->Value3() * pEffect->Value3BonusPercent() / 100.0f * (float32)Bonus );
	pEffect->SetDuration( pEffect->Duration() + (int32)( (float32)pEffect->Duration() * pEffect->DurationBonusPercent() / 100.0f * Bonus ) );
	pEffect->SetChanceOfSuccess( pEffect->ChanceOfSuccess() + (int32)( (float32)pEffect->ChanceOfSuccess() + pEffect->ChanceOfSuccessBonusPercent() / 100.0f * Bonus ) );

	m_pNaturalEffects[pEffect->Activation()].push_back( pEffect );
} // CCharacter::AddNewNaturalEffect()

float32 CCharacter::RunningSpeed( void )					
{	
	float32 RunningSpeed = m_RunningSpeed + m_RunningSpeed * (float32)EffectValue( KEffectPercentSpeed ) / 100.0f;
	if( RunningSpeed < 0 )
	{
		RunningSpeed = 0;
	}
	return RunningSpeed;						
} // CCharacter::RunningSpeed()

float32	CCharacter::WalkingSpeed( void )					
{	
	float32 WalkingSpeed = m_WalkingSpeed + m_WalkingSpeed * (float32)EffectValue( KEffectPercentSpeed ) / 100.0f;
	if( WalkingSpeed < 0 )
	{
		WalkingSpeed = 0;
	}
	return WalkingSpeed;						
} // CCharacter::WalkingSpeed()


int32 CCharacter::DamageResistance( EDamageType Type )			// type of resistance
{
	// there are no modifiers for undead 'resistance'
	if( Type == KDamageUndead )
	{
		return m_DamageResistance[Type];
	}
	int32 Resistance = (int32)( m_DamageResistance[Type] + EffectValue( KDamageResistance[Type] ) );
	if( Resistance > 100 )
	{
		Resistance = 100;
	}
	if( Resistance < -100 )
	{
		Resistance = -100;
	}
		
	return Resistance;	
} // CCharacter::DamageResistance()

int32 CCharacter::SkillPoints( ESkill Skill )	// skill to retrieve value for
{	
	return m_SkillPoints[Skill] + (int32)EffectValue( EEffectType( KEffectSkillSword + Skill ) );
} //  CCharacter::SkillPoints()

int32 CCharacter::BaseSkillPoints( ESkill Skill )	// skill to retrieve value for
{	
	return m_SkillPoints[Skill];
} //  CCharacter::SkillPoints()

int32 CCharacter::HP( void )						
{	
	return (int32)floor( m_HP );			
} // CCharacter::HP()

int32 CCharacter::Stamina( void )						
{	
	return (int32)floor( m_Stamina );			
} // CCharacter::Stamina()

float32 CCharacter::HPFloat( void )						
{	
	return m_HP;			
} // CCharacter::HPFloat()

float32 CCharacter::StaminaFloat( void )						
{	
	return m_Stamina;			
} // CCharacter::StaminaFloat()

int32 CCharacter::MaxStamina( void )						
{	
	int32 MaxStamina = m_MaxStamina + (int32)ceil( m_MaxStamina * (float32)EffectValue( KEffectPercentStamina ) / 100.0f );
	MaxStamina += (int32)EffectValue( KEffectMaxStamina );

	return MaxStamina;			
} // CCharacter::MaxStamina()

int32 CCharacter::BaseMaxStamina( void )						
{	
	return 2 * BaseVitality() +
			  m_Level;
} // CCharacter::BaseMaxStamina()

int32 CCharacter::Mana( void )						
{	
	return (int32)floor( m_Mana );			
} // CCharacter::Mana()

float32 CCharacter::ManaFloat( void )						
{	
	return m_Mana;			
} // CCharacter::ManaFloat()

int32 CCharacter::MaxMana( void )						
{	
	int32 MaxMana = m_MaxMana + (int32)ceil( m_MaxMana * (float32)EffectValue( KEffectPercentMana ) / 100.0f );
	MaxMana += (int32)EffectValue( KEffectMaxMana );
	return MaxMana;		
} // CCharacter::MaxMana()

int32 CCharacter::MaxHP( void )					
{	
	int32 MaxHP = m_MaxHP + (int32)ceil( m_MaxHP * (float32)EffectValue( KEffectPercentHP ) / 100.0f );
	MaxHP += (int32)EffectValue( KEffectMaxHP );
	return MaxHP;		
} // CCharacter::MaxHP()

int32 CCharacter::Strength( void )				
{	
	int32 Strength = m_Strength + (int32)ceil( m_Strength * (float32)EffectValue( KEffectPercentStrength ) / 100.0f );
	Strength += (int32)EffectValue( KEffectStrength );
	return Strength;		
} // CCharacter::Strength()

int32 CCharacter::Dexterity( void )				
{	
	int32 Dexterity = m_Dexterity + (int32)ceil( m_Dexterity * (float32)EffectValue( KEffectPercentDexterity ) / 100.0f );
	Dexterity += (int32)EffectValue( KEffectDexterity );
	return Dexterity;		
} // CCharacter::Dexterity()

int32 CCharacter::Vitality( void )				
{	
	int32 Vitality = m_Vitality + (int32)ceil( m_Vitality * (float32)EffectValue( KEffectPercentVitality ) / 100.0f );
	Vitality += (int32)EffectValue( KEffectVitality );
	return Vitality;		
} // CCharacter::Vitality()

int32 CCharacter::Magic( void )					
{	
	int32 Magic = m_Magic + (int32)ceil( m_Magic * (float32)EffectValue( KEffectPercentMagic ) / 100.0f );
	Magic += (int32)EffectValue( KEffectMagic );
	return Magic;		
} // CCharacter::Magic()

int32 CCharacter::ArmorBonus( void )
{
	int32 Bonus = m_pInventory->ArmorBonus() + NaturalArmor();
	Bonus += (int32)ceil( (float32)Bonus * (float32)EffectValue( KEffectPercentArmorBonus ) / 100.0f );
	
	Bonus += (int32)EffectValue( KEffectArmorBonus );
	return Bonus;
} // CCharacter::ArmorBonus()

int32 CCharacter::DexterityBonus( void )
{
	return Dexterity() / 5;
} // CCharacter::DexterityBonus()

int32 CCharacter::BaseDexterityBonus( void )
{
	return BaseDexterity() / 5;
} // CCharacter::BaseDexterityBonus()

int32 CCharacter::ToHitBonus( void )
{
	int32 ToHitBonus = m_ToHitBonus;
	if( m_pActiveWeapon != NULL )
	{
		ToHitBonus += SkillPoints( KSkillModifier[m_pActiveWeapon->Type()] ) * 1;
	}
	if( m_pCurrentAttack != NULL )
	{
		ToHitBonus += m_pCurrentAttack->ToHitBonus();
	}
	return ToHitBonus;
} // CCharacter::ToHitBonus( void );

int32 CCharacter::BaseToHitBonus( void )
{
	int32 ToHitBonus = m_ToHitBonus;
	if( m_pActiveWeapon != NULL )
	{
		ToHitBonus += BaseSkillPoints( KSkillModifier[m_pActiveWeapon->Type()] ) * 1;
	}
	if( m_pCurrentAttack != NULL )
	{
		ToHitBonus += m_pCurrentAttack->ToHitBonus();
	}
	return ToHitBonus;
} // CCharacter::BaseToHitBonus( void );

int32 CCharacter::CharacterDamage( void )
{
	if( HasMaster() && 
		Master()->IsPlayer() &&
		!IsSummoned() )
	{
		// pet gets a natural damage boost to help them along
		return Level() * 1;
	}

	return 0;//Strength() * m_Level / 200;
} // CCharacter::CharacterDamage()

int32 CCharacter::BaseCharacterDamage( void )
{
	if( HasMaster() && 
		Master()->IsPlayer() &&
		!IsSummoned() )
	{
		// pet gets a natural damage boost to help them along
		return Level() * 1;
	}
	return 0;//BaseStrength() * m_Level / 200;
} // CCharacter::BaseCharacterDamage()

int32 CCharacter::MinimumDamage( void )
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			return 0;
		}
	}

	int32 Damage( 0 );
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotLeftHand )->MinimumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( Strength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotLeftHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 SecondarySkillReduction = 50.0f / ( (float32)SkillPoints( KSkillDualWield ) * .05f + 1 );
			SecondarySkillReduction = 50 - SecondarySkillReduction;
			if( SecondarySkillReduction > 50 )
			{
				SecondarySkillReduction = 50;
			}
			SecondarySkillReduction /= 50.0f;
			SecondarySkillReduction *= .5f;
			SecondarySkillReduction = .5f + SecondarySkillReduction;
			// dual wield proficiency reduces the penalty

			WeaponDamage =(int32)floor( (float32)WeaponDamage * SecondarySkillReduction );

		}

		WeaponDamage += (int32)ceil( (float32)WeaponDamage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
		WeaponDamage += (int32)EffectValue( KEffectDamageBonus );


		if( WeaponDamage < Damage ||
			Damage == 0 )
		{
			Damage = WeaponDamage;
		}
	}
	if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotRightHand )->MinimumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( Strength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotRightHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 PrimarySkillReduction = 50.0f / ( (float32)SkillPoints( KSkillDualWield ) * .05f + 1 );
			PrimarySkillReduction = 50 - PrimarySkillReduction;
			if( PrimarySkillReduction > 50 )
			{
				PrimarySkillReduction = 50;
			}
			PrimarySkillReduction /= 50.0f;
			PrimarySkillReduction *= .25f;
			PrimarySkillReduction = .75f + PrimarySkillReduction;
			// dual wield proficiency reduces the penalty
			WeaponDamage =(int32)floor( (float32)WeaponDamage * PrimarySkillReduction );

		}

		WeaponDamage += (int32)ceil( (float32)WeaponDamage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
		WeaponDamage += (int32)EffectValue( KEffectDamageBonus );

		if( WeaponDamage < Damage ||
			Damage == 0 )
		{
			Damage = WeaponDamage;
		}
	}
	if( Damage == 0 )
	{
		Damage = m_pCurrentAttack->MinimumDamage();
	}

	// unarmed attack - we need to boost by level delta
	if( m_pActiveWeapon == NULL )
	{
		if( Level() > m_BaseLevel )
		{
			Damage = (int32)( (float32)Damage * ( (float32)Level() - (float32)m_BaseLevel + 100.0f ) / 100.0f );
		}
		Damage = (int32)( (float32)Damage * ( Strength() + 100 ) / 100.0f );
		Damage += (int32)ceil( (float32)Damage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
		Damage += (int32)EffectValue( KEffectDamageBonus );
	}

	return Damage;
} // CCharacter::MinimumDamage()

int32 CCharacter::MaximumDamage( void )
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			return 0;
		}
	}

	int32 Damage( 0 );
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotLeftHand )->MaximumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( Strength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotLeftHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 SecondarySkillReduction = 50.0f / ( (float32)SkillPoints( KSkillDualWield ) * .05f + 1 );
			SecondarySkillReduction = 50 - SecondarySkillReduction;
			if( SecondarySkillReduction > 50 )
			{
				SecondarySkillReduction = 50;
			}
			SecondarySkillReduction /= 50.0f;
			SecondarySkillReduction *= .5f;
			SecondarySkillReduction = .5f + SecondarySkillReduction;
			// dual wield proficiency reduces the penalty

			WeaponDamage =(int32)floor( (float32)WeaponDamage * SecondarySkillReduction );

		}
		WeaponDamage += (int32)ceil( WeaponDamage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
		WeaponDamage += (int32)EffectValue( KEffectDamageBonus );

		if( WeaponDamage > Damage )
		{
			Damage = WeaponDamage;
		}
	}
	if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotRightHand )->MaximumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( Strength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotRightHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 PrimarySkillReduction = 50.0f / ( (float32)SkillPoints( KSkillDualWield ) * .05f + 1 );
			PrimarySkillReduction = 50 - PrimarySkillReduction;
			if( PrimarySkillReduction > 50 )
			{
				PrimarySkillReduction = 50;
			}
			PrimarySkillReduction /= 50.0f;
			PrimarySkillReduction *= .25f;
			PrimarySkillReduction = .75f + PrimarySkillReduction;
			// dual wield proficiency reduces the penalty
			WeaponDamage =(int32)floor( (float32)WeaponDamage * PrimarySkillReduction );

		}
		WeaponDamage += (int32)ceil( WeaponDamage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
		WeaponDamage += (int32)EffectValue( KEffectDamageBonus );
		if( WeaponDamage > Damage )
		{
			Damage = WeaponDamage;
		}
	}
	if( Damage == 0 )
	{
		Damage = m_pCurrentAttack->MaximumDamage();
	}

	// unarmed attack - we need to boost by level delta
	if( m_pActiveWeapon == NULL )
	{
		if( Level() > m_BaseLevel )
		{
			Damage = (int32)( (float32)Damage * ( (float32)Level() - (float32)m_BaseLevel + 100.0f ) / 100.0f );
		}
		Damage = (int32)( (float32)Damage * ( Strength() + 100 ) / 100.0f );
		Damage += (int32)ceil( Damage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
		Damage += (int32)EffectValue( KEffectDamageBonus );
	}
	return Damage;
} // CCharacter::MaximumDamage()

int32 CCharacter::ElementalDamage( void )
{
	int32 Damage( 0 );
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
	{
		for( uint32 i = 0; i < m_pInventory->GetItemFromSlot( KSlotLeftHand )->DamageBonuses(); i++ )
		{
			Damage += m_pInventory->GetItemFromSlot( KSlotLeftHand )->DamageBonusValue( i );
		}
	}
	if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		for( uint32 i = 0; i < m_pInventory->GetItemFromSlot( KSlotRightHand )->DamageBonuses(); i++ )
		{
			Damage += m_pInventory->GetItemFromSlot( KSlotRightHand )->DamageBonusValue( i );
		}
	}
	return Damage;
} // CCharacter::ElementalDamage()

int32 CCharacter::ElementalDamage( EDamageType Type )
{
	int32 Damage( 0 );
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
	{
		for( uint32 i = 0; i < m_pInventory->GetItemFromSlot( KSlotLeftHand )->DamageBonuses(); i++ )
		{
			if( m_pInventory->GetItemFromSlot( KSlotLeftHand )->DamageBonusType( i ) == Type )
			{
				Damage += m_pInventory->GetItemFromSlot( KSlotLeftHand )->DamageBonusValue( i );
			}
		}
	}
	if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		for( uint32 i = 0; i < m_pInventory->GetItemFromSlot( KSlotRightHand )->DamageBonuses(); i++ )
		{
			if( m_pInventory->GetItemFromSlot( KSlotRightHand )->DamageBonusType( i ) == Type )
			{
				Damage += m_pInventory->GetItemFromSlot( KSlotRightHand )->DamageBonusValue( i );
			}
		}
	}
	return Damage;
} // CCharacter::ElementalDamage()

int32 CCharacter::BaseMaxMana( void )						
{	
	return m_MaxMana;		
} // CCharacter::BaseMaxMana()

int32 CCharacter::BaseMaxHP( void )					
{	
	// pet hp leveling
	if( Master() != NULL &&
		Master()->IsPlayer() &&
		!IsSummoned() )
	{
		return 4 * BaseVitality() +
			   4 * m_Level +
			   18;
	
	}
	return m_MaxHP;		
} // CCharacter::BaseMaxHP()

int32 CCharacter::BaseStrength( void )				
{	
	return m_Strength;		
} // CCharacter::BaseStrength()

int32 CCharacter::BaseDexterity( void )				
{	
	return m_Dexterity;		
} // CCharacter::BaseDexterity()

int32 CCharacter::BaseVitality( void )				
{	
	return m_Vitality;
} // CCharacter::BaseVitality()

int32 CCharacter::BaseMagic( void )					
{	
	return m_Magic;		
} // CCharacter::BaseMagic()

int32 CCharacter::BaseArmorBonus( void )
{
	int32 Bonus = m_pInventory->ArmorBonus() + NaturalArmor();
	return Bonus;
} // CCharacter::BaseArmorBonus()


int32 CCharacter::BaseMinimumDamage( void )
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			return 0;
		}
	}

	int32 Damage( 0 );
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotLeftHand )->MinimumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( BaseStrength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotLeftHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 SecondarySkillReduction = 50.0f / ( (float32)BaseSkillPoints( KSkillDualWield ) * .05f + 1 );
			SecondarySkillReduction = 50 - SecondarySkillReduction;
			if( SecondarySkillReduction > 50 )
			{
				SecondarySkillReduction = 50;
			}
			SecondarySkillReduction /= 50.0f;
			SecondarySkillReduction *= .5f;
			SecondarySkillReduction = .5f + SecondarySkillReduction;
			// dual wield proficiency reduces the penalty

			WeaponDamage =(int32)floor( (float32)WeaponDamage * SecondarySkillReduction );

		}


		if( WeaponDamage < Damage ||
			Damage == 0 )
		{
			Damage = WeaponDamage;
		}
	}
	if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotRightHand )->MinimumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( BaseStrength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotRightHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 PrimarySkillReduction = 50.0f / ( (float32)BaseSkillPoints( KSkillDualWield ) * .05f + 1 );
			PrimarySkillReduction = 50 - PrimarySkillReduction;
			if( PrimarySkillReduction > 50 )
			{
				PrimarySkillReduction = 50;
			}
			PrimarySkillReduction /= 50.0f;
			PrimarySkillReduction *= .25f;
			PrimarySkillReduction = .75f + PrimarySkillReduction;
			// dual wield proficiency reduces the penalty
			WeaponDamage =(int32)floor( (float32)WeaponDamage * PrimarySkillReduction );

		}

		if( WeaponDamage < Damage ||
			Damage == 0 )
		{
			Damage = WeaponDamage;
		}
	}
	if( Damage == 0 )
	{
		Damage = m_pCurrentAttack->MinimumDamage();
	}

	// unarmed attack - we need to boost by level delta
	if( m_pActiveWeapon == NULL )
	{
		if( Level() > m_BaseLevel )
		{
			Damage = (int32)( (float32)Damage * ( (float32)Level() - (float32)m_BaseLevel + 100.0f ) / 100.0f );
		}
		Damage = (int32)( (float32)Damage * ( BaseStrength() + 100 ) / 100.0f );
	}

	return Damage;
} // CCharacter::BaseMinimumDamage()

int32 CCharacter::BaseMaximumDamage( void )
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			return 0;
		}
	}

	int32 Damage( 0 );
	if( m_pInventory->GetItemFromSlot( KSlotLeftHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotLeftHand )->MaximumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( BaseStrength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotLeftHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 SecondarySkillReduction = 50.0f / ( (float32)BaseSkillPoints( KSkillDualWield ) * .05f + 1 );
			SecondarySkillReduction = 50 - SecondarySkillReduction;
			if( SecondarySkillReduction > 50 )
			{
				SecondarySkillReduction = 50;
			}
			SecondarySkillReduction /= 50.0f;
			SecondarySkillReduction *= .5f;
			SecondarySkillReduction = .5f + SecondarySkillReduction;
			// dual wield proficiency reduces the penalty

			WeaponDamage =(int32)floor( (float32)WeaponDamage * SecondarySkillReduction );

		}

		if( WeaponDamage > Damage )
		{
			Damage = WeaponDamage;
		}
	}
	if( m_pInventory->GetItemFromSlot( KSlotRightHand ) != NULL )
	{
		int32 WeaponDamage = m_pInventory->GetItemFromSlot( KSlotRightHand )->MaximumDamage();

		WeaponDamage = (int32)( (float32)WeaponDamage * ( BaseStrength() + 100 ) / 100.0f );

		WeaponDamage += SkillPoints( KSkillModifier[m_pInventory->GetItemFromSlot( KSlotRightHand )->Type()] );

		if( IsDualWielding() )
		{
			float32 PrimarySkillReduction = 50.0f / ( (float32)BaseSkillPoints( KSkillDualWield ) * .05f + 1 );
			PrimarySkillReduction = 50 - PrimarySkillReduction;
			if( PrimarySkillReduction > 50 )
			{
				PrimarySkillReduction = 50;
			}
			PrimarySkillReduction /= 50.0f;
			PrimarySkillReduction *= .25f;
			PrimarySkillReduction = .75f + PrimarySkillReduction;
			// dual wield proficiency reduces the penalty
			WeaponDamage =(int32)floor( (float32)WeaponDamage * PrimarySkillReduction );

		}

		if( WeaponDamage > Damage )
		{
			Damage = WeaponDamage;
		}
	}
	if( Damage == 0 )
	{
		Damage = m_pCurrentAttack->MaximumDamage();
	}

	// unarmed attack - we need to boost by level delta
	if( m_pActiveWeapon == NULL )
	{
		if( Level() > m_BaseLevel )
		{
			Damage = (int32)( (float32)Damage * ( (float32)Level() - (float32)m_BaseLevel + 100.0f ) / 100.0f );
		}
		Damage = (int32)( (float32)Damage * ( BaseStrength() + 100 ) / 100.0f );
	}

	return Damage;
} // CCharacter::BaseMaximumDamage()

int32 CCharacter::RollWeaponDamage( void )
{
	if( m_pCurrentAttack == NULL )
	{
		SelectAttack();
		if( m_pCurrentAttack == NULL )
		{
			return 0;
		}
	}
	int32 Damage = RandomIntegerBetween( m_pCurrentAttack->MinimumDamage(),
										 m_pCurrentAttack->MaximumDamage() );

	// unarmed attack - we need to boost by level delta
	if( m_pActiveWeapon == NULL )
	{
		if( Level() > m_BaseLevel )
		{
			Damage = (int32)( (float32)Damage * ( (float32)Level() - (float32)m_BaseLevel + 100.0f ) / 100.0f );
		}
	}

	Damage = (int32)( (float32)Damage * ( Strength() + 100 ) / 100.0f );


	Damage += (int32)ceil( Damage * (float32)EffectValue( KEffectPercentDamageBonus ) / 100.0f );
	Damage += (int32)EffectValue( KEffectDamageBonus );

	if( m_pActiveWeapon != NULL )
	{
		Damage += SkillPoints( KSkillModifier[m_pActiveWeapon->Type()] );

		// calculate dual-wield penalties
		if( IsDualWielding() )
		{
			// left weapon receives the most penalty
			if( m_pActiveWeapon == m_pInventory->GetItemFromSlot( KSlotLeftHand ) )
			{
				float32 SecondarySkillReduction = 50.0f / ( (float32)SkillPoints( KSkillDualWield ) * .05f + 1 );
				SecondarySkillReduction = 50 - SecondarySkillReduction;
				if( SecondarySkillReduction > 50 )
				{
					SecondarySkillReduction = 50;
				}
				SecondarySkillReduction /= 50.0f;
				SecondarySkillReduction *= .5f;
				SecondarySkillReduction = .5f + SecondarySkillReduction;

				Damage =(int32)floor( (float32)Damage * SecondarySkillReduction );
			}
			else
			{
				float32 PrimarySkillReduction = 50.0f / ( (float32)SkillPoints( KSkillDualWield ) * .05f + 1 );
				PrimarySkillReduction = 50 - PrimarySkillReduction;
				if( PrimarySkillReduction > 50 )
				{
					PrimarySkillReduction = 50;
				}
				PrimarySkillReduction /= 50.0f;
				PrimarySkillReduction *= .25f;
				PrimarySkillReduction = .75f + PrimarySkillReduction;

				Damage =(int32)floor( (float32)Damage * PrimarySkillReduction );
			}
		}

	}


	return Damage;
} // CCharacter::RollWeaponDamage()

int32 CCharacter::ModifyDamage( int32 Damage,				// hp damage
							    EDamageType DamageType )	// type of damage being applied
{
	float32 DamageTakenPercent = (float32)EffectValue( KEffectPercentDamageTaken );
	if( DamageTakenPercent < -75 )
	{
		DamageTakenPercent = -75;
	}
	Damage += (int32)floor( Damage * DamageTakenPercent / 100.0f );
	Damage += (int32)EffectValue( KEffectDamageTaken );

	if( Damage < 1 )
	{
		Damage = 1;
	}

	int32 Resistance = (int32)ceil( (float32)DamageResistance( DamageType ) / 100.0f * (float32)Damage );
	Damage -= Resistance;

	if( Damage < 0 )
	{
		Damage = 0;
	}

	return Damage;
} // CCharacter::ModifyDamage()

void CCharacter::ApplyDamage( CLevel& pLevel,			// level to send text events to
							  CCharacter* pAttacker,	// attacking character
							  float32 Damage,				// hp damage
							  bool AllowReflection )	// allow reflection of damage?
{
	if( Damage == 0 ||
		!Alive() )
	{
		if( Alive() )
		{
			pLevel.AddEventText( this,
								pAttacker,
								Position() + D3DXVECTOR3( 0, 7, 0 ),
								LoadLocalizedString(1117),
								.75f,
								kFalse,
								1,
								kFalse );
		}
		return;
	}

	bool Bolded( kTrue );
	bool Secondary( kFalse );
	if( pAttacker->IsPlayer() )
	{
		Bolded = kFalse;
		Secondary = kTrue;
	}
	else if( IsPlayer() ||
			 ( HasMaster() && Master()->IsPlayer() ) )
	{
		Secondary = kTrue;
	}
	pLevel.AddEventText( this,
						 pAttacker,
						 Position() + D3DXVECTOR3( 0, 7, 0 ),
						 LoadLocalizedString1Variable(1118, IntegerToString( (int32)ceil( Damage ) )),
						 .75f,
						 Bolded,
						 1,
						 Secondary );

	ModifyHP( -Damage );

	if( pAttacker != NULL && AllowReflection && m_HP > 0 )
	{
		RespondToAttack( pAttacker );
	}

	if( AllowReflection &&
		pAttacker != NULL &&
		pAttacker->Alive() &&
		EffectValue( KEffectPercentDamageReflected ) != 0 )
	{
		float32 ReflectedDamage = (float32)Damage * (float32)EffectValue( KEffectPercentDamageReflected ) / 100.0f;

		float32 LastHP = (float32)pAttacker->HP();
		pAttacker->ApplyDamage( pLevel,
								this,
								ReflectedDamage,
								kFalse );	// don't allow reflection, or we can get in nasty pingpong matches!

		// if we just killed them, award ourselves their
		// experience award
		if( pAttacker->Alive() &&
			pAttacker->HP() <= 0 &&
			LastHP > 0 )
		{
			// do some magic/gold drop effects, if required

			CalculateDropEffects( pLevel,
								  *pAttacker );
			AwardExperience( pLevel, pAttacker, pAttacker->ExperienceAward() );
			IncrementStat( KJournalStatMonstersKilled );
			if( HasMaster() )
			{
				Master()->IncrementStat( KJournalStatMonstersKilled );
			}
			if( pAttacker->IsUnique() )
			{
				IncrementStat( KJournalStatFamousMonstersKilled );
				if( HasMaster() )
				{
					Master()->IncrementStat( KJournalStatFamousMonstersKilled );
				}
			}
			AwardFame( pLevel, pAttacker, pAttacker->FameAward() );
		}

	}


	D3DXVECTOR3 Delta = Position();
	if( pAttacker != NULL )
	{
		Delta = pAttacker->Position() - Delta;
		D3DXVec3Normalize( &Delta, &Delta );
		Delta *= CollisionRadius();
		Delta += Position();
		Delta.y = GetMaxBounds().y * .5f + Position().y;
	}

	for( uint32 i = 0; i < 8; i++ )
	{
		m_pParticleManager.AddParticle( m_BloodParticle, Delta, D3DXVECTOR3( 0, 0, 0 ), m_Scale );
	}

} // CCharacter::ApplyDamage()

void CCharacter::RespondToAttack( CCharacter* pAttacker )	// attacking character
{
	if( pAttacker == NULL ||
		pAttacker == this )
	{
		return;
	}
	for( uint32 i = 0; i < Pets(); i++ )
	{
		if( Pet( i )->Target() == NULL &&
			Pet( i )->AvailableForCommand() )
		{
			Pet( i )->StopPathing();
			Pet( i )->SetAIState( KAIHunt );
			Pet( i )->SetTarget( pAttacker );
		}
	}

} // CCharacter::RespondToAttack()

void CCharacter::MakeUnique( const std::string& Name,			// name of unique monster
							 int32 Level,						// level being generated for
							 bool SuperSize )					// super-sized?
{
	if( Level < 1 )
	{
		Level = 1;
	}
	m_Name = Name;

	m_IsUnique = kTrue;
	if( Level <= m_Level )
	{
		Level = m_Level + 1;
	}
	// uniques don't run away!
	m_Bravery = 1;



	int32 LevelDelta = Level - m_Level;

	m_ExperienceAward += (int32)( (float32)m_ExperienceAward * .75f * LevelDelta );
	m_ExperienceAward *= 2;
	if( m_ExperienceAward < 200 )
	{
		m_ExperienceAward = 200;
	}


	//m_ToHitBonus *= 2;
	m_ToHitBonus += LevelDelta * RandomIntegerBetween( 6, 8 );

	m_NaturalArmor += 10;

	m_NaturalArmor += LevelDelta * 6;

	m_Dexterity += LevelDelta * 6;
	m_Vitality += LevelDelta * 6;
	m_Strength += LevelDelta * 6;
	m_Magic += LevelDelta * 6;

	m_SkillPoints[KSkillCharmMagic] += LevelDelta / 2;
	m_SkillPoints[KSkillDefenseMagic] += LevelDelta / 2;
	m_SkillPoints[KSkillAttackMagic] += LevelDelta / 2;
	m_SkillPoints[KSkillSpellcasting] += LevelDelta / 2;
	if( m_SkillPoints[KSkillSpellcasting] > 10 )
	{
		m_SkillPoints[KSkillSpellcasting] = 10;
	}

	// boost base HP
	
	m_MaxHP += m_MinHP * 2;
	m_MaxHP += (int32)ceil( (float32)m_MinHP * (float32)LevelDelta * 2.0f );

	m_Level = Level;

	m_FameAward = RandomIntegerBetween( m_Level * 10, m_Level * 20 );

	// boost base HP

	if( m_MaxHP < 50 )
	{
		m_MaxHP = 50;
	}
	m_HP = (float32)m_MaxHP;
		
	// clear default resistances
	for( uint32 i = 0; i < KDamageTypes; i++ )
	{
	
		// don't fiddle with undead resistances - those are fixed
		if( i != KDamageUndead )
		{
			if( rand() %1000 < 500 )
			{
				int32 Resistance = RandomIntegerBetween( 0, 60 );
				if( Resistance > m_DamageResistance[i] )
				{
					m_DamageResistance[i] = Resistance;
				}
			}
		}
	}

	m_Scale += .2f;
	if( SuperSize )
	{
		m_Scale += .2f;
	}

	if( Magic() < 20 )
	{
		m_Magic = 20;
	}


	// learn a few random spells!
	uint32 Spells = RandomIntegerBetween( 0, 10 );
	for( int32 i = 0; i < Spells; i++ )
	{
		uint32 Sphere = RandomIntegerBetween( 0, KMagicSpheres - 1 );
		uint32 Spell = RandomIntegerBetween( 0, m_pSpellManifest.Spells( (EMagicSphere)Sphere ) - 1 );
		CSpellDescription* pSpellDescription;
		pSpellDescription = m_pSpellManifest.SpellDescription( (EMagicSphere)Sphere,
															   Spell );
		if( pSpellDescription != NULL &&
			pSpellDescription->RequirementsMet( *this ) )
		{
			int32 FreeSlot = FreeSpellSlot( pSpellDescription->Sphere() );
			if( FreeSlot != -1 )
			{
				LearnSpell( pSpellDescription, FreeSlot );
			}
		}
	}
}	// CCharacter::MakeUnique()

void CCharacter::MakeUnique( CNameGenerator& pNameGenerator,	// generator to make random name
							 int32 Level,						// level being generated for
							 bool SuperSize )					// super-sized?
{
	MakeUnique( pNameGenerator.GenerateName(), Level, SuperSize );
} // CCharacter::MakeUnique()

void CCharacter::TweakToLevel( int32 Level )		// level to upgrade base statistics to
{
	if( Level < 1 )
	{
		Level = 1;
	}
	if( Level <= m_Level )
	{
		return;
	}	
	int32 LevelDelta = Level - m_Level;

	m_ExperienceAward += (int32)( (float32)m_ExperienceAward * .08f * LevelDelta );

	m_ToHitBonus += LevelDelta * RandomIntegerBetween( 3, 5 );

	m_NaturalArmor += LevelDelta * 2;

	m_Dexterity += LevelDelta * 3;
	m_Vitality += LevelDelta * 3;
	m_Strength += LevelDelta * 3;
	m_Magic += LevelDelta * 3;
	m_SkillPoints[KSkillCharmMagic] += LevelDelta / 3;
	m_SkillPoints[KSkillDefenseMagic] += LevelDelta / 3;
	m_SkillPoints[KSkillAttackMagic] += LevelDelta / 3;
	m_SkillPoints[KSkillSpellcasting] += LevelDelta / 3;
	if( m_SkillPoints[KSkillSpellcasting] > 10 )
	{
		m_SkillPoints[KSkillSpellcasting] = 10;
	}

	// boost base HP
	
	m_MaxHP += (int32)ceil( (float32)m_MinHP * (float32)LevelDelta * .5f );
	m_HP = (float32)m_MaxHP;

	m_Level = Level;
} // CCharacter::TweakToLevel()

void CCharacter::SetTarget( CCharacter* pCharacter,
						    bool TrimAttack )		// cut short an attack?
{	
	if( pCharacter == NULL &&
		m_pTargetCharacter != NULL &&
		TrimAttack )
	{
		if( pCharacter != m_pTargetCharacter )
		{
			m_AttackTimer = 0;
		}
	}
	if( AIState() == KAIDying ||
		AIState() == KAIDead )
	{
		if( pCharacter == NULL )
		{
			m_pTargetCharacter = NULL;
		}
		return;
	}
	m_pTargetCharacter = pCharacter;		
} // CCharacter::SetTarget()

void CCharacter::SetTargetItem( CItem* pItem )
{	
	if( AIState() == KAIDying ||
		AIState() == KAIDead )
	{
		if( pItem == NULL )
		{
			m_pTargetItem = NULL;
		}
		return;
	}

	m_pTargetItem = pItem;		
} // CCharacter::SetTargetItem()

void CCharacter::ModifyHP( float32 HP )	// points to modify by
{
	float32 LastHP( m_HP );
	if( m_HP <= 0 && HP > 0 && HP < MaxHP() )
	{
		// already dead, can't heal, unless it's a full heal ( resurrection )
		return;
	}
	m_HP += HP;
	if( m_HP > MaxHP() )
	{
		m_HP = (float32)MaxHP();
	}
	if( Invincible() && m_HP < 1 )
	{
		RemoveEffect( "POISON" );
		m_HP = 1;
		if( LastHP > 1 &&
			HasMaster() )
		{
			Master()->PlayHelpSample( KSoundHelpPetFlee, kTrue );
		}
	}
	if( m_HP < 15 && m_HP > 0 && LastHP >= 15 )
	{
		PlayHelpSample( KSoundHelpHealthLow, kTrue );
	}
	if( m_HP < 0 )
	{
		m_HP = 0;
	}

} // CCharacter::ModifyHP()

void CCharacter::ModifyMana( float32 Mana )	// points to modify by
{
	m_Mana += Mana;
	if( m_Mana > MaxMana() )
	{
		m_Mana = (float32)MaxMana();
	}
	if( m_Mana < 0 )
	{
		m_Mana = 0;
	}
} // CCharacter::ModifyMana()

void CCharacter::ModifyStamina( float32 Stamina )	// points to modify by
{
	m_Stamina += Stamina;
	if( m_Stamina > MaxStamina() )
	{
		m_Stamina = (float32)MaxStamina();
	}
	if( m_Stamina < 0 )
	{
		m_Stamina = 0;
	}

} // CCharacter::ModifyStamina()

void CCharacter::AwardExperience( CLevel& pLevel,	// level to send text events to
								  CCharacter* pAwarder,	// xp awarder
								  int32 Award,		// points to modify by
								  bool FromMaster )	// experience awarded from master character?
{
	// owners give half of all experience earned to pets
	if( HasPets() )
	{
		for( uint32 i = 0; i < Pets(); i++ )
		{
			// fleeing pets or pets in town get no XP
			if( Pet( i )->AvailableForCommand() )
			{
				Pet( i )->AwardExperience( pLevel, pAwarder, (int32)( Award * .5f ) , kTrue );
			}
		}
	}

	// pets give earned experience to their masters
	if( HasMaster() &&
		!FromMaster )
	{
		Master()->AwardExperience( pLevel, pAwarder, Award );
	}
	else
	{
		m_Experience += Award;
		if( m_Experience < 0 )
		{
			m_Experience = 0;
		}

		if( IsPlayer() && Award > 0 )
		{
			if( pAwarder == NULL )
			{
				pAwarder = this;
			}
			pLevel.AddEventText( this,
								this,
								pAwarder->Position() + D3DXVECTOR3( 0, 12, 0 ),
								LoadLocalizedString1Variable(1119, IntegerToString( Award )),
								1.0f,
								kTrue,
								.5f,
								kFalse );
		}
	}



	if( !IsGameUnlocked() && Level() >= KDemoPlayerLevel + 1 ) //marsh
	{
		if( m_Experience >= ExperienceGate( Level() ) )
		{
			m_Experience = ExperienceGate( Level() ) - 1;
		}
	}

	if( m_Experience >= ExperienceGate( Level() ) &&
		Level() < 99 )
	{
		PlayHelpSample( KSoundHelpLevelUp, kTrue );

		if( IsPlayer() || 
			( HasMaster() && Master()->IsPlayer() ) )
		{
			CSFXDescription* pSFX = new CSFXDescription( KSFXFollowOwner,
														"SPELLS\\EFFECTS\\LEVELUP\\levelupring.ams",
														-1,
														kFalse,
														0,
														0,
														kFalse,
														kFalse );

			pLevel.AddSFX( *pSFX,
							this,
							this,
							Position(),
							Position() );

			DELETE_SAFELY( pSFX );

			pLevel.AddEventText( this,
								 this,
								 Position() + D3DXVECTOR3( 0, 8, 0 ),
								 LoadLocalizedString(1120),
								 1.0f,
								 kFalse,
								 .4f,
								 kTrue );

			m_pCharacterSounds->PlaySample( KSoundLevelUp,
											Position(),
											255,
											m_SoundRadius );

		}

		LevelUp();
	}
} // CCharacter::AwardExperience()

void CCharacter::AwardFame( CLevel& pLevel,		// level to post message to
						    CCharacter* pAwarder,	// xp awarder
							int32 Award,		// points to modify by
						    bool FromMaster )	// Fame awarded from master character?
{
	// pets give earned Fame to their masters
	if( HasMaster() &&
		!FromMaster )
	{
		m_pMasterCharacter->AwardFame( pLevel, pAwarder, Award );
	}
	else if( IsPlayer() )
	{
		if( Award > 0 )
		{
			if( pAwarder == NULL )
			{
				pAwarder = this;
			}
			pLevel.AddEventText( this,
								this,
								pAwarder->Position() + D3DXVECTOR3( 0, 14, 0 ),
								LoadLocalizedString1Variable(1121, IntegerToString( Award )),
								1.0f,
								kFalse,
								.5f,
								kTrue );
		}

		m_Fame += Award;
		if( m_Fame < 0 )
		{
			m_Fame = 0;
		}
	}


	bool PlayedSample( kFalse );
	while( m_Fame >= (int32)KFameGate[m_FameRank] &&
		m_FameRank < KFameLevels - 1 )
	{
		PlayHelpSample( KSoundHelpFameUp, !PlayedSample );
		PlayedSample = kTrue;
		m_FameRank++;
		pLevel.RequestMessage( LoadLocalizedString1Variable(1122, KFameName[FameRank()] ));
		m_UnusedSkillPoints += 4;
	}


} // CCharacter::AwardFame()

void CCharacter::LevelUp( void )
{
	m_Level++;

	m_HP = (float32)MaxHP();
	m_UnusedStatPoints += 5;
	m_UnusedSkillPoints += 2;
	// for now, do an auto-levelup for pets
	if( HasMaster() )
	{
		// pets get 1 extra stat
		m_UnusedStatPoints += 1;
		Master()->PlayHelpSample( KSoundHelpPetLevelUp, kTrue );

		SpendStrengthPoint();
		SpendStrengthPoint();
		SpendDexterityPoint();
		SpendDexterityPoint();
		SpendVitalityPoint();
		SpendVitalityPoint();
	}
} // CCharacter::LevelUp()

void CCharacter::SpendSkillPoint( ESkill Skill )	// skill to spend point on 
{
	if( m_UnusedSkillPoints > 0 )
	{
		m_SkillPoints[Skill] ++;
		m_UnusedSkillPoints --;
	}
} // CCharacter::SpendSkillPoint()

void CCharacter::SpendStrengthPoint( void )
{
	if( m_UnusedStatPoints > 0 )
	{
		m_Strength += 1;
		m_UnusedStatPoints --;
	}
	CStatsAndAchievements::GetInstance()->CheckStats();
} // CCharacter::SpendStrengthPoint()

void CCharacter::SpendDexterityPoint( void )
{
	if( m_UnusedStatPoints > 0 )
	{
		m_Dexterity += 1;
		m_UnusedStatPoints --;
	}
	CStatsAndAchievements::GetInstance()->CheckStats();
} // CCharacter::SpendDexterityPoint()

void CCharacter::SpendVitalityPoint( void )
{
	if( m_UnusedStatPoints > 0 )
	{
		m_Vitality += 1;
		m_UnusedStatPoints --;
	}
	CStatsAndAchievements::GetInstance()->CheckStats();
} // CCharacter::SpendVitalityPoint()

void CCharacter::SpendMagicPoint( void )
{
	if( m_UnusedStatPoints > 0 )
	{
		m_Magic += 1;
		m_UnusedStatPoints --;
	}
	CStatsAndAchievements::GetInstance()->CheckStats();
} // CCharacter::SpendMagicPoint()

void CCharacter::RemoveSkillPoint( ESkill Skill )	// skill to spend point on 
{
	if( m_SkillPoints[Skill] > 0 )
	{
		m_SkillPoints[Skill] --;
	}
} // CCharacter::RemoveSkillPoint()

void CCharacter::RemoveStrengthPoint( void )
{
	if( m_Strength > 2 )
	{
		m_Strength -= 1;
	}
} // CCharacter::RemoveStrengthPoint()

void CCharacter::RemoveDexterityPoint( void )
{
	if( m_Dexterity > 2 )
	{
		m_Dexterity -= 1;
	}
} // CCharacter::RemoveDexterityPoint()

void CCharacter::RemoveVitalityPoint( void )
{
	if( m_Vitality > 2 )
	{
		m_Vitality -= 1;
	}
} // CCharacter::RemoveVitalityPoint()

void CCharacter::RemoveMagicPoint( void )
{
	if( m_Magic > 2 )
	{
		m_Magic -= 1;
	}
} // CCharacter::RemoveMagicPoint()

void CCharacter::Die( CLevel& pLevel )	// level to drop items to
{
	IncrementStat( KJournalStatDeaths );

	m_pActiveWeapon = NULL;
	m_pCurrentAttack = NULL;
	ClearState();
	ClearEffects();
	ClearNaturalEffects();

	RemoveFromAvoidanceMap( pLevel );
	SetTarget( NULL );
	SetMaster( NULL );
	StopPathing();

	// de-summon any summoned creatures
	for( uint32 i = 0; i < Pets(); i++ )
	{
		if( Pet( i )->IsSummoned() )
		{
			Pet( i )->SetLifeDuration( .01f );
		}
	}

	if( m_LifeDuration > 0 )
	{
		Dispel( pLevel );
		return;
	}

	SetAIState( KAIDying );
	m_DeathAnimationIndex = FindRandomAnimation( "DIE" );
	BlendAnimation( m_DeathAnimationIndex, kFalse, .1f );
	m_pCharacterSounds->PlaySample( KSoundDie,
								    m_Position,
							        255,
							        m_SoundRadius );

	if( m_AllowDropInventory )
	{
		if( m_AllowDropEquipment )
		{
			m_pInventory->DropAll( pLevel );
		}
		else
		{
			m_pInventory->DropAllButEquipment( pLevel );
		}
	}

	for( int32 i = 0; i < 20; i++ )
	{
		m_pParticleManager.AddParticle( m_BloodParticle, m_Position );
	}


} // CCharacter::Die()

void CCharacter::UpdateHelp( float32 TimeElapsed )
{
	if( m_HelpTimer > 0 )
	{
		m_HelpTimer -= TimeElapsed;
	}
	if( m_HelpTimer <= 0 )
	{
		if( m_HelpQueue.size() > 0 )
		{
			m_HelpTimer = 2.5f;
			int32 Volume = m_pSettings.GetSettings( KSetSoundFXVolume ) - 60;
			if( Volume < 0 )
			{
				Volume = 0;
			}
			FSOUND_SetSFXMasterVolume( Volume );
			if( m_pSettings.GetSettings( KSetSoundFXVolume ) > 0 )
			{
				m_ActiveHelpSample = m_HelpQueue.front();

				m_HelpChannel = m_pCharacterSounds->PlaySample( m_HelpQueue.front(),
																Position(),
																255,
																10000 );
				int32 Volume = m_pSettings.GetSettings( KSetSoundFXVolume ) + 75;
				if( Volume > 255 )
				{
					Volume = 255;
				}
				if( m_HelpChannel != -1 )
				{
					FSOUND_SetVolumeAbsolute( m_HelpChannel, Volume );
				}
			}
			if( m_HelpChannel == -1 )
			{
				m_HelpChannel = 1;
			}
			m_HelpQueue.pop_front();
		}
		else
		{
			if( m_HelpChannel != -1 )
			{
				m_HelpChannel = -1;
				FSOUND_SetSFXMasterVolume( m_pSettings.GetSettings( KSetSoundFXVolume ) );
			}
			m_ActiveHelpSample = KSoundHelpNone;
		}

	}
	else
	{
			if( m_HelpChannel != -1 )
			{
				int32 Volume = m_pSettings.GetSettings( KSetSoundFXVolume ) + 75;
				if( Volume > 255 )
				{
					Volume = 255;
				}
				FSOUND_SetVolumeAbsolute( m_HelpChannel, Volume );
			}
	}

} // CCharacter::UpdateHelp()

void CCharacter::PlayHelpSample( ECharacterSounds Sample,	// sample to put on the help queue
								 bool HighPriority )		// high priority?
{
	if( IsPlayer() &&
		( HighPriority ||
		  ( m_HelpQueue.size() == 0 && m_HelpTimer <= 0 ) ) )
	{
		if( m_ActiveHelpSample == Sample )
		{
			return;
		}
		for( uint32 i = 0; i < m_HelpQueue.size(); i++ )
		{
			if( m_HelpQueue.at( i ) == Sample )
			{
				return;
			}
		}
		m_HelpQueue.push_back( Sample );
		UpdateHelp( 0 );
	}
} // CCharacter::PlayHelpSample()

void CCharacter::Greet( const D3DXVECTOR3& Position )	// sound position
{
	if( m_TimeSinceGreetSound < 3 )
	{
		return;
	}
	m_TimeSinceGreetSound = 0;
	int32 Channel = m_pCharacterSounds->PlaySample( KSoundGreet,
													Position,
													255,
													m_SoundRadius );

	int32 Volume = m_pSettings.GetSettings( KSetSoundFXVolume ) + 75;
	if( Volume > 255 )
	{
		Volume = 255;
	}

	if( Channel != -1 )
	{
		FSOUND_SetVolumeAbsolute( Channel, Volume );
	}

} // CCharacter::Greet()

bool CCharacter::Alive( void )
{
	if( AIState() == KAIDying ||
		AIState() == KAIDead )
	{
		return kFalse;
	}
	return kTrue;
} // CCharacter::Alive()

int32 CCharacter::FindRandomAnimation( const std::string& Prefix )	// prefix of animation to look for
{
	int32 AnimationIndex( -1 );
	uint32 Length = Prefix.length();
	for( uint32 a = 0; a < m_pModel->GetAnimationCount(); a++ )
	{
		if( m_pModel->GetAnimationName( a ).length() >= Length &&
			m_pModel->GetAnimationName( a ).substr( 0, Length ) == Prefix )
		{
			if( AnimationIndex == -1 )
			{
				AnimationIndex = a;
			}
			else if( rand() % 1000 < 500 )
			{
				AnimationIndex = a;
			}
		}
	}
	return AnimationIndex;
} // CCharacter::FindRandomAnimation()

void CCharacter::BlendAnimation( const std::string& AnimationName,	//  animation to check for
								 bool Looping,						// looping anim?
								 float32 BlendTime,					// time to blend
								 float32 Speed )					// defaults to 1
{
	if( m_pModel != NULL )
	{
		m_pModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
} // CCharacter::BlendAnimation()

void CCharacter::QueueBlendAnimation( const std::string& AnimationName,	//  animation to check for
									  bool Looping,						// looping anim?
									  float32 BlendTime,					// time to blend
									  float32 Speed )					// defaults to 1
{
	if( m_pModel != NULL )
	{
		m_pModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
} // CCharacter::QueueBlendAnimation()


void CCharacter::ClearAnimations( void )
{								    
	if( m_pModel != NULL )
	{
		m_pModel->ClearAnimations();
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->ClearAnimations();
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->ClearAnimations();
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->ClearAnimations();
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->ClearAnimations();
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->ClearAnimations();
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->ClearAnimations();
	}
} // CCharacter::ClearAnimations()

void CCharacter::ClearQueuedAnimations( void )
{								    
	if( m_pModel != NULL )
	{
		m_pModel->ClearQueuedAnimations();
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->ClearQueuedAnimations();
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->ClearQueuedAnimations();
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->ClearQueuedAnimations();
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->ClearQueuedAnimations();
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->ClearQueuedAnimations();
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->ClearQueuedAnimations();
	}
} // CCharacter::ClearQueuedAnimations()

void CCharacter::PlayAnimation( const std::string& AnimationName,	//  animation to check for
								bool Looping,						// looping anim?
								float32 Speed )						// defaults to 1
{								    
	if( m_pModel != NULL )
	{
		m_pModel->PlayAnimation( AnimationName, Looping, Speed );
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->PlayAnimation( AnimationName, Looping, Speed );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->PlayAnimation( AnimationName, Looping, Speed );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->PlayAnimation( AnimationName, Looping, Speed );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->PlayAnimation( AnimationName, Looping, Speed );
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->PlayAnimation( AnimationName, Looping, Speed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->PlayAnimation( AnimationName, Looping, Speed );
	}
} // CCharacter::PlayAnimation()

void CCharacter::BlendAnimation( uint32 Index,						//  index of animation
								 bool Looping,						// looping anim?
								 float32 BlendTime,					// time to blend
								 float32 Speed )					// defaults to 1
{
	if( m_pModel != NULL )
	{
		m_pModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
} // CCharacter::BlendAnimation()

void CCharacter::QueueBlendAnimation( uint32 Index,						//  index of animation
									  bool Looping,						// looping anim?
									  float32 BlendTime,				// time to blend
									  float32 Speed )					// defaults to 1

{
	if( m_pModel != NULL )
	{
		m_pModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
} // CCharacter::QueueBlendAnimation()

void CCharacter::PlayAnimation( uint32 Index,						//  index of animation
								bool Looping,						// looping anim?
								float32 Speed )						// defaults to 1
{								    
	if( m_pModel != NULL )
	{
		m_pModel->PlayAnimation( Index, Looping, Speed );
	}
	/*if( m_pArmorModel != NULL )
	{
		m_pArmorModel->PlayAnimation( Index, Looping, Speed );
	}
	if( m_pBootsModel != NULL )
	{
		m_pBootsModel->PlayAnimation( Index, Looping, Speed );
	}
	if( m_pUpperBodyModel != NULL )
	{
		m_pUpperBodyModel->PlayAnimation( Index, Looping, Speed );
	}
	if( m_pGlovesModel != NULL )
	{
		m_pGlovesModel->PlayAnimation( Index, Looping, Speed );
	}*/
	if( m_pHeadModel != NULL )
	{
		m_pHeadModel->PlayAnimation( Index, Looping, Speed );
	}
	if( m_pHairModel != NULL )
	{
		m_pHairModel->PlayAnimation( Index, Looping, Speed );
	}
} // CCharacter::PlayAnimation()

void CCharacter::FillInstance( CCharacterSaveInstance& pInstance,	// description to fill
							   CLevel& pLevel )						// current level
{
	pInstance.m_Difficulty = m_Difficulty;

	pInstance.m_MasterIndex = -1;

	pInstance.m_TownTimer = m_TownTimer;

	pInstance.m_Unique = m_IsUnique;

	for( uint32 i = 0; i < KDamageTypes; i++ )
	{
		pInstance.m_DamageResistance[i] = m_DamageResistance[i];
	}

	pInstance.m_IsPlayer = IsPlayer();

	for( int32 i = 0; i < KJournalStatistics; i++ )
	{
		pInstance.m_JournalStats[i] = m_JournalStats[i];
	}

	pInstance.m_ExistenceTime = m_ExistenceTime;

	pInstance.m_Position = Position();
	pInstance.m_Orientation = Orientation();

	pInstance.m_TemplateName = m_TemplateName;
	pInstance.m_OriginalTemplateName = m_OriginalTemplateName;

	pInstance.m_TransformationDuration = m_TransformationDuration;

	pInstance.m_LifeDuration = m_LifeDuration;

	pInstance.m_Gender = m_Gender;
	pInstance.m_HeadIndex = m_HeadIndex;
	pInstance.m_HairIndex = m_HairIndex;

	pInstance.m_Name = m_Name;

	pInstance.m_AncestorName = m_Name;

	pInstance.m_Lineage = m_Lineage;

	pInstance.m_IsMerchant = m_IsMerchant;

	pInstance.m_MerchantType = m_MerchantType;

	pInstance.m_Scale = m_Scale;

	pInstance.m_Bravery = m_Bravery;

	pInstance.m_LastDungeonLevel = pLevel.LevelDepth();

	pInstance.m_HasOpenPortal = m_HasOpenPortal;
	pInstance.m_PortalDepth = m_PortalDepth;
	pInstance.m_PortalPosition = m_PortalPosition;

	pInstance.m_Level = m_Level;
	pInstance.m_Experience = m_Experience;
	pInstance.m_HP = m_HP;
	pInstance.m_MaxHP = m_MaxHP;

	pInstance.m_Fame = m_Fame;
	pInstance.m_FameRank = m_FameRank;

	pInstance.m_Stamina = m_Stamina;
	pInstance.m_MaxStamina = m_MaxStamina;

	pInstance.m_Mana = m_Mana;
	pInstance.m_MaxMana = m_MaxMana;

	pInstance.m_ToHitBonus = m_ToHitBonus;
	pInstance.m_OriginalToHitBonus = m_OriginalToHitBonus;

	pInstance.m_NaturalArmor = m_NaturalArmor;
	pInstance.m_OriginalNaturalArmor = m_OriginalNaturalArmor;

	pInstance.m_ExperienceAward = m_ExperienceAward;

	pInstance.m_FameAward = m_FameAward;

	pInstance.m_UnusedStatPoints = m_UnusedStatPoints;
	pInstance.m_UnusedSkillPoints = m_UnusedSkillPoints;

	for( int32 i = 0; i < KSkills; i++ )
	{
		pInstance.m_SkillPoints[i] = m_SkillPoints[i];
	}
	for( int32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( m_KnownSpells[i][j] != NULL )
			{
				pInstance.m_KnownSpells[i][j] = m_KnownSpells[i][j]->Name();
			}
			else
			{
				pInstance.m_KnownSpells[i][j] = "";
			}
		}
	}

	pInstance.m_ActiveSpellName = "";
	std::string Name( "" );
	if( m_pActiveSpell != NULL )
	{
		pInstance.m_ActiveSpellName = m_pActiveSpell->Name();
	}

	pInstance.m_Strength = m_Strength;
	pInstance.m_OriginalStrength = m_OriginalStrength;
	pInstance.m_Dexterity = m_Dexterity;
	pInstance.m_OriginalDexterity = m_OriginalDexterity;
	pInstance.m_Vitality = m_Vitality;
	pInstance.m_OriginalVitality = m_OriginalVitality;
	pInstance.m_Magic = m_Magic;
	pInstance.m_OriginalMagic = m_OriginalMagic;

	pInstance.m_WalkingSpeed = m_WalkingSpeed;
	pInstance.m_RunningSpeed = m_RunningSpeed;

	pInstance.m_Gold = m_pInventory->Gold();

	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
		{
			pInstance.m_pEffects[i].push_back( new CEffect( m_pEffects[i][j] ) );
		}
	}

	CLList<CItemRef>* pItems = m_pInventory->ItemList();

	CLLNode<CItemRef>* Root = pItems->GetFirst();
	uint32 Index( 0 );
	while( Root != NULL )
	{
		CItemSaveInstance* pItemInstance = new CItemSaveInstance();
		Root->Data()->m_pItem->FillInstance( *pItemInstance, 
											 Root->Data()->m_SlotIndex, 
											 Root->Data()->m_Equipped );

		pInstance.m_pItemInstances.push_back( pItemInstance );
		Root = Root->Next();
	}

	for( int32 i = 0; i < Quests(); i++ )
	{
		pInstance.m_pQuests.push_back( new CQuest( *Quest( i ) ) );
	}

	if( m_pMasterQuest != NULL )
	{
		pInstance.m_pMasterQuest = new CQuest( *m_pMasterQuest );
	}

	if( IsPlayer() )
	{
		CStatsAndAchievements::GetInstance()->SetPlayer(this);
	}
} // CCharacter::FillInstance()

void CCharacter::ApplyInstance( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							    CLevel* pLevel,						// level we are in
							    CCharacterTemplate& pCharacterManifest,// character manifest
								CCharacterSaveInstance& pInstance,	// description to fill
							    CItemTemplate& pItemManifest )	// item manifest to construct items from
{
	
	CStatsAndAchievements::GetInstance()->Disable();

	ClearEffects();

	if( pLevel != NULL )
	{
		RemoveFromAvoidanceMap( *pLevel );
	}

	m_Difficulty = pInstance.m_Difficulty;

	m_ExistenceTime = pInstance.m_ExistenceTime;

	m_IsUnique = pInstance.m_Unique;

	m_TownTimer = pInstance.m_TownTimer;

	if( m_TownTimer > 0 )
	{
		SetAIState( KAIInTown );
	}

	for( uint32 i = 0; i < KDamageTypes; i++ )
	{
		m_DamageResistance[i] = pInstance.m_DamageResistance[i];
	}

	GiveGold( pInstance.m_Gold );

	for( int32 i = 0; i < KJournalStatistics; i++ )
	{
		m_JournalStats[i] = pInstance.m_JournalStats[i];
	}

	if( StringUpper( m_Family ) == "UNDEAD" )
	{
		m_DamageResistance[KDamageUndead] = 0;
	}
	else
	{
		m_DamageResistance[KDamageUndead] = 100;
	}

	m_Position = pInstance.m_Position;
	m_OrientationMatrix = pInstance.m_Orientation;


	if( _isnan( m_OrientationMatrix._11 ) )
	{
		// we got a bad orientation somehow
		// clean it up!
		D3DXMatrixRotationY( &m_OrientationMatrix, (float32)KDeg2Rad * 180.0f );
	}

	m_TemplateName = pInstance.m_TemplateName;
	m_OriginalTemplateName = pInstance.m_OriginalTemplateName;

	m_TransformationDuration = pInstance.m_TransformationDuration;

	m_LifeDuration = pInstance.m_LifeDuration;

	// summoned creatures do NOT drop their inventories!
	if( m_LifeDuration > 0 )
	{
		SetAllowDropInventory( kFalse );
	}

	m_Name = pInstance.m_Name;

	m_AncestorName = pInstance.m_Name;

	m_Lineage = pInstance.m_Lineage;

	SetIsMerchant( pInstance.m_IsMerchant );

	SetMerchantType( pInstance.m_MerchantType );

	m_Gender = pInstance.m_Gender;
	m_HeadIndex = pInstance.m_HeadIndex;
	m_HairIndex = pInstance.m_HairIndex;

	m_Scale = pInstance.m_Scale;

	m_Bravery = pInstance.m_Bravery;

	m_LastDungeonLevel = pInstance.m_LastDungeonLevel;

	m_HasOpenPortal = pInstance.m_HasOpenPortal;
	m_PortalDepth = pInstance.m_PortalDepth;
	m_PortalPosition = pInstance.m_PortalPosition;

	m_Level = pInstance.m_Level;
	m_Experience = pInstance.m_Experience;
	m_HP = pInstance.m_HP;
	m_MaxHP = pInstance.m_MaxHP;

	m_Fame = pInstance.m_Fame;
	m_FameRank = pInstance.m_FameRank;

	m_Stamina = pInstance.m_Stamina;
	m_MaxStamina = pInstance.m_MaxStamina;

	m_Mana = pInstance.m_Mana;
	m_MaxMana = pInstance.m_MaxMana;

	m_ToHitBonus = pInstance.m_ToHitBonus;
	m_OriginalToHitBonus = pInstance.m_OriginalToHitBonus;

	m_NaturalArmor = pInstance.m_NaturalArmor;
	m_OriginalNaturalArmor = pInstance.m_OriginalNaturalArmor;

	m_ExperienceAward = pInstance.m_ExperienceAward;

	m_FameAward = pInstance.m_FameAward;

	m_UnusedStatPoints = pInstance.m_UnusedStatPoints;
	m_UnusedSkillPoints = pInstance.m_UnusedSkillPoints;

	for( int32 i = 0; i < KSkills; i++ )
	{
		m_SkillPoints[i] = pInstance.m_SkillPoints[i];
	}

	ClearSpellbook();
	for( int32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( pInstance.m_KnownSpells[i][j].length() > 0 )
			{
				m_KnownSpells[i][j] = m_pSpellManifest.SpellDescription( pInstance.m_KnownSpells[i][j] );
			}
		}
	}

	m_pActiveSpell = NULL;
	for( int32 i = 0; i < KMagicSpheres; i++ )
	{
		for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
		{
			if( m_KnownSpells[i][j] != NULL &&
				StringUpper( m_KnownSpells[i][j]->Name() ) == StringUpper( pInstance.m_ActiveSpellName ) )
			{
				m_pActiveSpell = m_KnownSpells[i][j];
				break;
			}
		}
	}

	m_Strength = pInstance.m_Strength;
	m_OriginalStrength = pInstance.m_OriginalStrength;
	m_Dexterity = pInstance.m_Dexterity;
	m_OriginalDexterity = pInstance.m_OriginalDexterity;
	m_Vitality = pInstance.m_Vitality;
	m_OriginalVitality = pInstance.m_OriginalVitality;
	m_Magic = pInstance.m_Magic;
	m_OriginalMagic = pInstance.m_OriginalMagic;

	m_WalkingSpeed = pInstance.m_WalkingSpeed;
	m_RunningSpeed = pInstance.m_RunningSpeed;


	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( uint32 j = 0; j < pInstance.m_pEffects[i].size(); j++ )
		{
			m_pEffects[i].push_back( new CEffect( pInstance.m_pEffects[i][j] ) );
		}
	}

	for( int32 i = 0; i < pInstance.m_pItemInstances.size(); i++ )
	{
		CItemDescription* pDescription = pItemManifest.ItemDescription( pInstance.m_pItemInstances[i]->m_BaseName );
		CItem* pItem = NULL;
		if( pDescription != NULL )
		{
			int32 Slot( pInstance.m_pItemInstances[i]->m_SlotIndex );
			pItem = new CItem( pD3DDevice,
								m_pRefManager,
								m_pSettings,
								m_pParticleManager,
							    m_pGlowManager,
								m_pPositionalAudioManager,
								*pDescription );
			pItem->ApplyInstance( pD3DDevice,
								  *pInstance.m_pItemInstances[i],
								  pItemManifest );

			if( !m_pInventory->AddItem( pItem, Slot, pLevel ) )
			{
				DELETE_SAFELY( pItem );
				pItem = NULL;
			}
			if( pInstance.m_pItemInstances[i]->m_Equipped && pItem != NULL )
			{
				m_pInventory->SetSlotEquipped( (EItemSlot)Slot );
			}
		}
	}

	if( m_HeadIndex != -1 )
	{
		LoadPlayerModels( pD3DDevice );
	}

	// if this was saved in a transformed state, we need to change to the new form
	if( m_TransformationDuration != 0 )
	{
		std::string NewForm = m_TemplateName;
		m_TemplateName = m_OriginalTemplateName;
		Transform( pD3DDevice,
				   pLevel,
				   pCharacterManifest,
				   NewForm,
				   m_TransformationDuration,
				   kFalse );	// don't apply stat change - that's already done

	}

	for( int32 i = 0; i < pInstance.m_pQuests.size(); i++ )
	{
		m_pQuests.push_back( new CQuest( *pInstance.m_pQuests[i] ) );
	}

	if( pInstance.m_pMasterQuest != NULL )
	{
		m_pMasterQuest = new CQuest( *pInstance.m_pMasterQuest );
	}

	m_pInventory->CalculateEffectValues();

	CStatsAndAchievements::GetInstance()->Enable();

} // CCharacter::ApplyInstance()


void CCharacter::ApplyDescription( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								   CCharacterDescription& pDescription )	// character description
{
	ClearNaturalEffects();
	std::string BloodName = pDescription.BloodParticle();
	if( BloodName == "BLOOD" && !m_pSettings.GetSettings( KSetRedBlood ) )
	{
		BloodName = "STARBLOOD";
	}
	m_BloodParticle = m_pParticleManager.GetParticleType( BloodName );

	m_Level = pDescription.BaseLevel();

	m_CanMatchSpeed = pDescription.CanMatchSpeed();

	m_Alignment = pDescription.Alignment();
	m_OriginalAlignment = m_Alignment;
	m_Name = pDescription.Name();
	m_MimicName = pDescription.MimicName();
	m_Family = pDescription.Family();
	if( m_Family == "MIMIC" )
	{
		m_IsMimic = kTrue;
	}

	for( uint32 i = 0; i < KRanks; i++ )
	{
		if( m_Name.length() > KRankName[i].length() &&
			m_Name.substr( 0, KRankName[i].length() ) == KRankName[i] )
		{
			m_Rank = (ERank)( KRankNormal + i );
		}
	}

	m_ReachBonus = pDescription.ReachBonus();

	m_BaseLevel = pDescription.BaseLevel();

	m_MaxHP = RandomIntegerBetween( pDescription.MinHP(), pDescription.MaxHP() );
	m_HP = (float32)m_MaxHP;
	m_MinHP = pDescription.MinHP();

	float32 ScalePercentage( 1 );
	if( pDescription.MaxHP() - pDescription.MinHP() <= 0 )
	{
		 ScalePercentage = (float32)( m_MaxHP - pDescription.MinHP() ) / 
							  (float32)( pDescription.MaxHP() - pDescription.MinHP() );
		if( pDescription.MaxHP() - pDescription.MinHP() == 0 )
		{
			ScalePercentage = 0;
		}
	}
	ScalePercentage *= ( pDescription.MaximumScale() - pDescription.MinimumScale() );
	m_Scale = pDescription.MinimumScale() + ScalePercentage;

	m_Strength = pDescription.Strength();
	m_Dexterity = pDescription.Dexterity();
	m_Vitality = pDescription.Vitality();
	m_Magic = pDescription.Magic();

	CalculateMaxMana();
	m_Mana = (float32)m_MaxMana;

	m_VerticalOffset = pDescription.VerticalOffset();

	m_ExperienceAward = pDescription.ExperienceAward();

	m_FameAward = pDescription.FameAward();

	m_ToHitBonus = pDescription.ToHitBonus();

	m_NaturalArmor = pDescription.NaturalArmor();

	m_FollowRadius = pDescription.FollowRadius();
	m_AttentionRadius = pDescription.AttentionRadius();
	m_MotionRadius = pDescription.MotionRadius();
	m_ViewRadius = pDescription.ViewRadius();

	if( pDescription.HasLightPool() )
	{
		m_HasLightPool = kTrue;
		m_pLightPoolMaterial = m_pRefManager.AddMaterial( pD3DDevice, pDescription.LightPoolPath() );
		m_LightPoolRadius = pDescription.LightPoolRadius();
		m_LightPoolType = pDescription.LightPoolType();
	}

	m_TurnRate = pDescription.TurnRate();

	m_WalkingSpeed = pDescription.WalkingSpeed();
	m_RunningSpeed = pDescription.RunningSpeed();

	m_UniqueIdles = pDescription.UniqueIdles();

	m_MinionsCollideable = pDescription.MinionsCollideable();

	m_Tactics = pDescription.Tactics();

	m_Bravery = pDescription.Bravery();

	LoadModel( pD3DDevice, 
			   pDescription.ModelPath(),
			   "",
			   "",
			   "",
			   "",
			   pDescription.CollisionPath(),
			   pDescription.AnimationPath() );

	if( m_RunningSpeed <= m_WalkingSpeed )
	{
		m_RunningSpeed = m_WalkingSpeed;
		m_CanRun = kFalse;
	}

	for( int32 i = 0; i < pDescription.UnarmedAttacks(); i++ )
	{
		CAttackDescription* pAttack = new CAttackDescription( pDescription.UnarmedAttack( i ) );
		pAttack->SetAnimationIndex( m_pModel->GetAnimationIndex( pAttack->Animation() ) );
		assert( pAttack->AnimationIndex() != -1 );
		AddUnarmedAttack( pAttack );

	}

	for( int32 i = 0; i < KSkills; i++ )
	{
		m_SkillPoints[i] = pDescription.SkillPoints( (ESkill)i );
	}

	for( int32 i = 0; i < KDamageTypes; i++ )
	{
		m_DamageResistance[i] = pDescription.DamageResistance( (EDamageType)i );
	}
	if( StringUpper( m_Family ) == "UNDEAD" )
	{
		m_DamageResistance[KDamageUndead] = 0;
	}
	else
	{
		m_DamageResistance[KDamageUndead] = 100;
	}

	m_SoundRadius = pDescription.SoundRadius();

	for( int32 i = 0; i < pDescription.AttackSounds(); i++ )
	{
		assert( m_pModel->GetAnimationIndex( pDescription.AttackSoundAnimation( i ) ) != -1 );
		m_pAttackSounds->AddSample( m_pModel->GetAnimationIndex( pDescription.AttackSoundAnimation( i ) ),
									pDescription.AttackSound( i ) );
	}

	for( int32 i = 0; i < pDescription.StrikeSounds(); i++ )
	{
		assert( m_pModel->GetAnimationIndex( pDescription.StrikeSoundAnimation( i ) ) != -1 );
		m_pStrikeSounds->AddSample( m_pModel->GetAnimationIndex( pDescription.StrikeSoundAnimation( i ) ),
									pDescription.StrikeSound( i ) );
	}

	for( int32 i = 0; i < pDescription.MissSounds(); i++ )
	{
		assert( m_pModel->GetAnimationIndex( pDescription.MissSoundAnimation( i ) ) != -1 );
		m_pMissSounds->AddSample( m_pModel->GetAnimationIndex( pDescription.MissSoundAnimation( i ) ),
								  pDescription.MissSound( i ) );
	}

	for( int32 i = 0; i < pDescription.DieSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundDie,
									   pDescription.DieSound( i ) );
	}

	for( int32 i = 0; i < pDescription.GreetSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundGreet,
									   pDescription.GreetSound( i ) );
	}

	for( int32 i = 0; i < pDescription.StepSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundStep,
									   pDescription.StepSound( i ) );
	}

	for( int32 i = 0; i < pDescription.ArmedAttackSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundAttack,
									   pDescription.ArmedAttackSound( i ) );
	}

	for( int32 i = 0; i < pDescription.IdleSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundIdle,
									   pDescription.IdleSound( i ) );
	}

	for( int32 i = 0; i < pDescription.UniqueIdleSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundUniqueIdle,
									   pDescription.UniqueIdleSound( i ) );
	}

	for( int32 i = 0; i < pDescription.RoarSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundRoar,
									   pDescription.RoarSound( i ) );
	}

	m_GreetMessages.clear();
	for( int32 i = 0; i < pDescription.GreetMessages(); i++ )
	{
		m_GreetMessages.push_back( pDescription.GreetMessage( i ) );
	}


	if( pDescription.HasOverrideMaterial() )
	{
		m_pOverrideMaterial = m_pRefManager.AddMaterial( pD3DDevice,
														 pDescription.OverrideBase(),
														 pDescription.OverrideAlpha(),
														 pDescription.OverrideIllumination(),
														 pDescription.OverrideNormal(),
														 pDescription.OverrideSphere() );

		CMaterial* pOriginal = m_pModel->GetMaterialByIndex( 0 );
		if( pOriginal != NULL )
		{
			m_pOverrideMaterial->SetReflective( pOriginal->HasReflection() );
			m_pOverrideMaterial->SetAllowZWrite( pOriginal->AllowZWrite() );
			m_pOverrideMaterial->SetTransparentReflection( pOriginal->TransparentReflection() );
			m_pOverrideMaterial->SetAdditiveReflection( pOriginal->AdditiveReflection() );
			m_pOverrideMaterial->SetD3DMaterial( pOriginal->D3DMaterial() );
		}
	}

	for( int32 i = 0; i < pDescription.Spells(); i++ )
	{
		CSpellDescription* pSpellDescription;
		pSpellDescription = m_pSpellManifest.SpellDescription( pDescription.Spell( i ) );
		if( pSpellDescription != NULL )
		{
			int32 FreeSlot = CCharacter::FreeSpellSlot( pSpellDescription->Sphere() );
			if( FreeSlot != -1 )
			{
				LearnSpell( pSpellDescription, FreeSlot );
			}
		}
	}

	for( int32 i = 0; i < pDescription.SupportedEquipment(); i++ )
	{
		m_pInventory->SetSupportsItemType( pDescription.SupportedEquipment( i ), kTrue );
	}

	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)pDescription.Effects( (EEffectActivation)i ); j++ )
		{
			AddNaturalEffect( pDescription.Effect( (EEffectActivation)i, j ) );
		}
	}

} // CCharacter::ApplyDescription()

void CCharacter::ApplyTransformationDescription( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
												 CLevel* pLevel,						// level to send effects to
												 CCharacterTemplate& pCharacterManifest,// character manifest
												 CCharacterDescription& pDescription,	// character description
												 bool ToNewForm,						// to a new form, or reverting?
												 bool ModifyStats )						// modify the stats?
{	
	if( AIState() != KAIInTown &&
		AIState() != KAIBuySell &&
		AIState() != KAISpeakMerchant )
	{
		SetAIState( KAIInactive );
	}
	m_AttackTimer = 0;
	m_pCurrentAttack = NULL;
	ClearNaturalEffects();
	if(	AIState() != KAIBuySell &&
		AIState() != KAISpeakMerchant )
	{
		SetTarget( NULL );
	}
	CCharacterDescription* pCharDescription;
	m_BreathActive = kFalse;

	pCharDescription = pCharacterManifest.CharacterDescription( m_OriginalTemplateName );
	if( ToNewForm && pCharDescription == NULL )
	{
		return;
	}

	UnloadModels();

	// clear default resistances
	for( uint32 i = 0; i < KDamageTypes; i++ )
	{
		m_DamageResistance[i] = 0;
	}
	// default EVERYTHING is 100% resistant to undead damage
	m_DamageResistance[KDamageUndead] = 100;

	std::string BloodName = pDescription.BloodParticle();
	if( BloodName == "BLOOD" && !m_pSettings.GetSettings( KSetRedBlood ) )
	{
		BloodName = "STARBLOOD";
	}
	m_BloodParticle = m_pParticleManager.GetParticleType( BloodName );

	float32 ScalePercentage = 0;
	m_Scale = pDescription.MinimumScale() + ScalePercentage;
	m_VerticalOffset = pDescription.VerticalOffset();
	m_TurnRate = pDescription.TurnRate();
	m_WalkingSpeed = pDescription.WalkingSpeed();
	m_RunningSpeed = pDescription.RunningSpeed();
	m_UniqueIdles = pDescription.UniqueIdles();

	if( ModifyStats )
	{
		// new form?
		if( ToNewForm )
		{
			// if we're going from unnatural form to unnatural form, switch back to our
			// original form first as far as stats go
			if( pCharDescription != NULL &&
				m_TemplateName.length() > 0 &&
				m_TemplateName != m_OriginalTemplateName )
			{
				m_Strength += m_OriginalStrength;
				m_Dexterity += m_OriginalDexterity;
				m_Vitality += m_OriginalVitality;
				m_Magic += m_OriginalMagic;
				m_ToHitBonus += m_OriginalToHitBonus;
				m_NaturalArmor += m_OriginalNaturalArmor;
			}


			if( pCharDescription != NULL )
			{
				// if we are in original form, we store our original stats
				m_OriginalStrength =  pCharDescription->Strength() - pDescription.Strength();
				m_OriginalDexterity =  pCharDescription->Dexterity() - pDescription.Dexterity();
				m_OriginalVitality =  pCharDescription->Vitality() - pDescription.Vitality();
				m_OriginalMagic =  pCharDescription->Magic() - pDescription.Magic();
				m_OriginalToHitBonus =  pCharDescription->ToHitBonus() - pDescription.ToHitBonus();
				m_OriginalNaturalArmor =  pCharDescription->NaturalArmor() - pDescription.NaturalArmor();

				m_Strength += pDescription.Strength() - pCharDescription->Strength();
				m_Dexterity += pDescription.Dexterity() - pCharDescription->Dexterity();
				m_Vitality += pDescription.Vitality() - pCharDescription->Vitality();
				m_Magic += pDescription.Magic() - pCharDescription->Magic();
				m_ToHitBonus += pDescription.ToHitBonus() - pCharDescription->ToHitBonus();
				m_NaturalArmor += pDescription.NaturalArmor() - pCharDescription->NaturalArmor();
			}
			else
			{
				m_ToHitBonus += pDescription.ToHitBonus() - m_ToHitBonus;
				m_NaturalArmor += pDescription.NaturalArmor() - m_NaturalArmor;
			}

		}
		else	// nope, reverting to last state
		{
			m_Strength += m_OriginalStrength;
			m_Dexterity += m_OriginalDexterity;
			m_Vitality += m_OriginalVitality;
			m_Magic += m_OriginalMagic;
			m_ToHitBonus += m_OriginalToHitBonus;
			m_NaturalArmor += m_OriginalNaturalArmor;
		}
	}

	m_BaseLevel = pDescription.BaseLevel();

	m_CanMatchSpeed = pDescription.CanMatchSpeed();

	//m_Strength = pDescription.Strength();
	//m_Dexterity = pDescription.Dexterity();
	//m_Vitality = pDescription.Vitality();
	//m_Magic = pDescription.Magic();
	//m_ToHitBonus = pDescription.ToHitBonus();
	//m_NaturalArmor = pDescription.NaturalArmor();


	CalculateMaxMana();

	LoadModel( pD3DDevice, 
			   pDescription.ModelPath(),
			   "",
			   "",
			   "",
			   "",
			   pDescription.CollisionPath(),
			   pDescription.AnimationPath() );

	if( m_RunningSpeed <= m_WalkingSpeed )
	{
		m_RunningSpeed = m_WalkingSpeed;
		m_CanRun = kFalse;
	}

	for( int32 i = 0; i < m_UnarmedAttacks.size(); i++ )
	{
		DELETE_SAFELY( m_UnarmedAttacks[i] );
	}
	m_UnarmedAttacks.clear();

	for( int32 i = 0; i < pDescription.UnarmedAttacks(); i++ )
	{
		CAttackDescription* pAttack = new CAttackDescription( pDescription.UnarmedAttack( i ) );
		pAttack->SetAnimationIndex( m_pModel->GetAnimationIndex( pAttack->Animation() ) );
		assert( m_pModel->GetAnimationIndex( pAttack->Animation() ) != -1 );
		AddUnarmedAttack( pAttack );

	}

	for( int32 i = 0; i < KSkills; i++ )
	{
		m_SkillPoints[i] = pDescription.SkillPoints( (ESkill)i );
	}

	for( int32 i = 0; i < KDamageTypes; i++ )
	{
		m_DamageResistance[i] = pDescription.DamageResistance( (EDamageType)i );
	}

	if( StringUpper( m_Family ) == "UNDEAD" )
	{
		m_DamageResistance[KDamageUndead] = 0;
	}
	else
	{
		m_DamageResistance[KDamageUndead] = 100;
	}

	m_pAttackSounds->Clear();
	m_pStrikeSounds->Clear();
	m_pMissSounds->Clear();
	m_pCharacterSounds->Clear();

	m_pCharacterSounds->AddSample( KSoundGold,
								   "SOUNDS\\ITEMS\\gold.wav" );

	m_pCharacterSounds->AddSample( KSoundDispel,
								   "SOUNDS\\SPELLS\\fail.wav" );

	m_pCharacterSounds->AddSample( KSoundBlock,
								   "SOUNDS\\BATTLE\\block.wav" );

	m_pCharacterSounds->AddSample( KSoundCritical,
								   "SOUNDS\\BATTLE\\critical.wav" );

	m_pCharacterSounds->AddSample( KSoundLevelUp,
								   "SOUNDS\\FX\\levelup.wav" );

	for( int32 i = 0; i < pDescription.AttackSounds(); i++ )
	{
		assert( m_pModel->GetAnimationIndex( pDescription.AttackSoundAnimation( i ) ) != -1 );
		m_pAttackSounds->AddSample( m_pModel->GetAnimationIndex( pDescription.AttackSoundAnimation( i ) ),
									pDescription.AttackSound( i ) );
	}

	for( int32 i = 0; i < pDescription.StrikeSounds(); i++ )
	{
		assert( m_pModel->GetAnimationIndex( pDescription.StrikeSoundAnimation( i ) ) != -1 );
		m_pStrikeSounds->AddSample( m_pModel->GetAnimationIndex( pDescription.StrikeSoundAnimation( i ) ),
									pDescription.StrikeSound( i ) );
	}

	for( int32 i = 0; i < pDescription.MissSounds(); i++ )
	{
		assert( m_pModel->GetAnimationIndex( pDescription.MissSoundAnimation( i ) ) != -1 );
		m_pMissSounds->AddSample( m_pModel->GetAnimationIndex( pDescription.MissSoundAnimation( i ) ),
								  pDescription.MissSound( i ) );
	}

	for( int32 i = 0; i < pDescription.DieSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundDie,
									   pDescription.DieSound( i ) );
	}

	for( int32 i = 0; i < pDescription.GreetSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundGreet,
									   pDescription.GreetSound( i ) );
	}
	for( int32 i = 0; i < pDescription.StepSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundStep,
									   pDescription.StepSound( i ) );
	}

	for( int32 i = 0; i < pDescription.ArmedAttackSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundAttack,
									   pDescription.ArmedAttackSound( i ) );
	}

	for( int32 i = 0; i < pDescription.IdleSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundIdle,
									   pDescription.IdleSound( i ) );
	}

	for( int32 i = 0; i < pDescription.UniqueIdleSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundUniqueIdle,
									   pDescription.UniqueIdleSound( i ) );
	}

	for( int32 i = 0; i < pDescription.RoarSounds(); i++ )
	{
		m_pCharacterSounds->AddSample( KSoundRoar,
									   pDescription.RoarSound( i ) );
	}


	m_GreetMessages.clear();
	for( int32 i = 0; i < pDescription.GreetMessages(); i++ )
	{
		m_GreetMessages.push_back( pDescription.GreetMessage( i ) );
	}

	m_pRefManager.RemoveMaterial( m_pOverrideMaterial );
	m_pOverrideMaterial = NULL;

	// unlearn all spells
	ClearSpellbook();
	// learn any spells unique to this character type
	for( int32 i = 0; i < pDescription.Spells(); i++ )
	{
		CSpellDescription* pSpellDescription;
		pSpellDescription = m_pSpellManifest.SpellDescription( pDescription.Spell( i ) );
		if( pSpellDescription != NULL )
		{
			int32 FreeSlot = CCharacter::FreeSpellSlot( pSpellDescription->Sphere() );
			if( FreeSlot != -1 )
			{
				LearnSpell( pSpellDescription, FreeSlot );
			}
		}
	}

	if( pDescription.HasOverrideMaterial() )
	{
		m_pOverrideMaterial = m_pRefManager.AddMaterial( pD3DDevice,
														 pDescription.OverrideBase(),
														 pDescription.OverrideAlpha(),
														 pDescription.OverrideIllumination(),
														 pDescription.OverrideNormal(),
														 pDescription.OverrideSphere() );

		CMaterial* pOriginal = m_pModel->GetMaterialByIndex( 0 );
		if( pOriginal != NULL )
		{
			m_pOverrideMaterial->SetReflective( pOriginal->HasReflection() );
			m_pOverrideMaterial->SetAllowZWrite( pOriginal->AllowZWrite() );
			m_pOverrideMaterial->SetTransparentReflection( pOriginal->TransparentReflection() );
			m_pOverrideMaterial->SetAdditiveReflection( pOriginal->AdditiveReflection() );
			m_pOverrideMaterial->SetD3DMaterial( pOriginal->D3DMaterial() );
		}
	}

	for( int32 i = 0; i < KActivationTypes; i++ )
	{
		for( int32 j = 0; j < (int32)pDescription.Effects( (EEffectActivation)i ); j++ )
		{
			AddNaturalEffect( pDescription.Effect( (EEffectActivation)i, j ) );
		}
	}

	// don't need this, as it happens every frame
	/*if( pLevel != NULL )
	{
		m_pInventory->VerifyEquipment( *pLevel );
	}*/
} // CCharacter::ApplyTransformationDescription()


///////////////////////////////////////
// CHARACTER INSTANCE METHODS
///////////////////////////////////////

CCharacterSaveInstance::~CCharacterSaveInstance( void )
	{
		for( uint32 i = 0; i < KActivationTypes; i++ )
		{
			for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
			{
				DELETE_SAFELY( m_pEffects[i][j] );
			}
			m_pEffects[i].clear();
		}

		for( int32 i = 0; i < m_pItemInstances.size(); i++ )
		{
			DELETE_SAFELY( m_pItemInstances[i] );
		}
		m_pItemInstances.clear();


		for( int32 i = 0; i < m_pQuests.size(); i++ )
		{
			DELETE_SAFELY( m_pQuests[i] );
		}
		m_pQuests.clear();
} // CCharacterSaveInstance::CCharacterSaveInstance()


void CCharacterSaveInstance::Save( FILE* pFile )	// open file to save to
	{

		uint16 Size = m_TemplateName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_TemplateName.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_OriginalTemplateName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_OriginalTemplateName.c_str(), sizeof( char ) * Size, 1, pFile );

		fwrite( &m_ExistenceTime, sizeof( float32 ), 1, pFile );

		fwrite( &m_IsPlayer, sizeof( bool ), 1, pFile );

		if( m_IsPlayer )
		{
			for( uint32 i = 0; i < KJournalStatistics; i++ )
			{
				int32 Value = m_JournalStats[i];
				fwrite( &Value, sizeof( EJournalStatistic ), 1, pFile );
			}
		}

		fwrite( &m_TransformationDuration, sizeof( float32 ), 1, pFile );

		fwrite( &m_LifeDuration, sizeof( float32 ), 1, pFile );

		fwrite( &m_TownTimer, sizeof( float32 ), 1, pFile );

		fwrite( &m_Difficulty, sizeof( m_Difficulty ), 1, pFile );

		fwrite( &m_Scale, sizeof( float32 ), 1, pFile );

		fwrite( &m_Bravery, sizeof( float32 ), 1, pFile );

		fwrite( &m_MasterIndex, sizeof( int32 ), 1, pFile );

		fwrite( &m_DungeonSeed, sizeof( int32 ), 1, pFile );

		Size = m_Name.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_Name.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_AncestorName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_AncestorName.c_str(), sizeof( char ) * Size, 1, pFile );

		fwrite( &m_Lineage, sizeof( int32 ), 1, pFile );

		fwrite( &m_IsMerchant, sizeof( bool ), 1, pFile );

		fwrite( &m_MerchantType, sizeof( EMerchant ), 1, pFile );

		fwrite( &m_Gender, sizeof( EGender ), 1, pFile );

		fwrite( &m_HeadIndex, sizeof( int32 ), 1, pFile );

		fwrite( &m_HairIndex, sizeof( int32 ), 1, pFile );

		fwrite( &m_LastDungeonLevel, sizeof( int32 ), 1, pFile );

		fwrite( &m_HasOpenPortal, sizeof( bool ), 1, pFile );
		fwrite( &m_PortalDepth, sizeof( uint32 ), 1, pFile );
		fwrite( &m_PortalPosition, sizeof( D3DXVECTOR3 ), 1, pFile );

		fwrite( &m_Position, sizeof( D3DXVECTOR3 ), 1, pFile );

		fwrite( &m_Orientation, sizeof( D3DXMATRIX ), 1, pFile );

		fwrite( &m_Level, sizeof( int32 ), 1, pFile );
		fwrite( &m_Experience, sizeof( int32 ), 1, pFile );

		fwrite( &m_HP, sizeof( float32 ), 1, pFile );
		fwrite( &m_MaxHP, sizeof( int32 ), 1, pFile );

		fwrite( &m_Fame, sizeof( int32 ), 1, pFile );
		fwrite( &m_FameRank, sizeof( int32 ), 1, pFile );

		fwrite( &m_Stamina, sizeof( float32 ), 1, pFile );
		fwrite( &m_MaxStamina, sizeof( int32 ), 1, pFile );

		fwrite( &m_Mana, sizeof( float32 ), 1, pFile );
		fwrite( &m_MaxMana, sizeof( int32 ), 1, pFile );

		fwrite( &m_ToHitBonus, sizeof( int32 ), 1, pFile );
		fwrite( &m_OriginalToHitBonus, sizeof( int32 ), 1, pFile );

		fwrite( &m_NaturalArmor, sizeof( int32 ), 1, pFile );
		fwrite( &m_OriginalNaturalArmor, sizeof( int32 ), 1, pFile );

		fwrite( &m_ExperienceAward, sizeof( int32 ), 1, pFile );

		fwrite( &m_FameAward, sizeof( int32 ), 1, pFile );

		fwrite( &m_UnusedStatPoints, sizeof( int32 ), 1, pFile );
		fwrite( &m_UnusedSkillPoints, sizeof( int32 ), 1, pFile );

		fwrite( &m_Unique, sizeof( bool ), 1, pFile );

		for( uint32 i = 0; i < KDamageTypes; i++ )
		{
			fwrite( &m_DamageResistance[i], sizeof( int32 ), 1, pFile );
		}

		for( int32 i = 0; i < KSkills; i++ )
		{
			int32 Point = m_SkillPoints[i];
			fwrite( &Point, sizeof( int32 ), 1, pFile );
		}

		for( int32 i = 0; i < KMagicSpheres; i++ )
		{
			for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				std::string Name( m_KnownSpells[i][j] );
				uint16 Size = Name.length();
				fwrite( &Size, sizeof( uint16 ), 1, pFile );
				if( Size > 0 )
				{
					fwrite( Name.c_str(), sizeof( char ) * Size, 1, pFile );
				}
			}
		}

		Size = m_ActiveSpellName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			fwrite( m_ActiveSpellName.c_str(), sizeof( char ) * Size, 1, pFile );
		}

		fwrite( &m_Strength, sizeof( int32 ), 1, pFile );
		fwrite( &m_OriginalStrength, sizeof( int32 ), 1, pFile );
		fwrite( &m_Dexterity, sizeof( int32 ), 1, pFile );
		fwrite( &m_OriginalDexterity, sizeof( int32 ), 1, pFile );
		fwrite( &m_Vitality, sizeof( int32 ), 1, pFile );
		fwrite( &m_OriginalVitality, sizeof( int32 ), 1, pFile );
		fwrite( &m_Magic, sizeof( int32 ), 1, pFile );
		fwrite( &m_OriginalMagic, sizeof( int32 ), 1, pFile );

		fwrite( &m_WalkingSpeed, sizeof( float32 ), 1, pFile );
		fwrite( &m_RunningSpeed, sizeof( float32 ), 1, pFile );

		fwrite( &m_Gold, sizeof( int32 ), 1, pFile );

		for( uint32 j = 0; j < KActivationTypes; j++ )
		{
			int32 Effects( m_pEffects[j].size() );
			fwrite( &Effects , sizeof( int32 ), 1, pFile );
			for( uint32 i = 0; i < (uint32)Effects; i++ )
			{
				CEffect* pEffect = m_pEffects[j][i];

				Size = pEffect->Name().length();
				fwrite( &Size, sizeof( uint16 ), 1, pFile );
				fwrite( pEffect->Name().c_str(), sizeof( char ) * Size, 1, pFile );

				Size = pEffect->Message().length();
				fwrite( &Size, sizeof( uint16 ), 1, pFile );
				fwrite( pEffect->Message().c_str(), sizeof( char ) * Size, 1, pFile );

				bool Exclusive( pEffect->Exclusive() );
				fwrite( &Exclusive, sizeof( bool ), 1, pFile );

				EEffectType Type( pEffect->Type() );
				fwrite( &Type, sizeof( EEffectType ), 1, pFile );

				EDamageType DamageType( pEffect->DamageType() );
				fwrite( &DamageType, sizeof( EDamageType ), 1, pFile );

				bool Positive( pEffect->Positive() );
				fwrite( &Positive, sizeof( bool ), 1, pFile );
				EEffectActivation Activation( pEffect->Activation() );
				fwrite( &Activation, sizeof( EEffectActivation ), 1, pFile );

				int32 ChanceOfSuccess( pEffect->ChanceOfSuccess() );
				fwrite( &ChanceOfSuccess, sizeof( int32 ), 1, pFile );
				int32 ChanceOfSuccessBonus( pEffect->ChanceOfSuccessBonus() );
				fwrite( &ChanceOfSuccessBonus, sizeof( int32 ), 1, pFile );
				int32 ChanceOfSuccessBonusPercent( pEffect->ChanceOfSuccessBonusPercent() );
				fwrite( &ChanceOfSuccessBonusPercent, sizeof( int32 ), 1, pFile );

				int32 Duration( pEffect->Duration() );
				fwrite( &Duration, sizeof( int32 ), 1, pFile );
				int32 DurationBonus( pEffect->DurationBonus() );
				fwrite( &DurationBonus, sizeof( int32 ), 1, pFile );
				int32 DurationBonusPercent( pEffect->DurationBonusPercent() );
				fwrite( &DurationBonusPercent, sizeof( int32 ), 1, pFile );

				float32 Value( pEffect->Value() );
				fwrite( &Value, sizeof( int32 ), 1, pFile );
				float32 ValueBonus( pEffect->ValueBonus() );
				fwrite( &ValueBonus, sizeof( int32 ), 1, pFile );
				float32 ValueBonusPercent( pEffect->ValueBonusPercent() );
				fwrite( &ValueBonusPercent, sizeof( int32 ), 1, pFile );

				float32 Value2( pEffect->Value2() );
				fwrite( &Value2, sizeof( int32 ), 1, pFile );
				float32 Value2Bonus( pEffect->Value2Bonus() );
				fwrite( &Value2Bonus, sizeof( int32 ), 1, pFile );
				float32 Value2BonusPercent( pEffect->Value2BonusPercent() );
				fwrite( &Value2BonusPercent, sizeof( int32 ), 1, pFile );

				float32 Value3( pEffect->Value3() );
				fwrite( &Value3, sizeof( int32 ), 1, pFile );
				float32 Value3Bonus( pEffect->Value3Bonus() );
				fwrite( &Value3Bonus, sizeof( int32 ), 1, pFile );
				float32 Value3BonusPercent( pEffect->Value3BonusPercent() );
				fwrite( &Value3BonusPercent, sizeof( int32 ), 1, pFile );

				float32 PriceMultiplier( pEffect->PriceMultiplier() );
				fwrite( &PriceMultiplier, sizeof( float32 ), 1, pFile );
			}
		}

		uint32 SubItems = m_pItemInstances.size();
		fwrite( &SubItems , sizeof( uint32 ), 1, pFile );
		for( int32 j = 0; j < SubItems; j++ )
		{
			m_pItemInstances[j]->Save( pFile );
		}

		uint32 Quests = m_pQuests.size();
		fwrite( &Quests , sizeof( uint32 ), 1, pFile );
		for( int32 j = 0; j < Quests; j++ )
		{
			m_pQuests[j]->Save( pFile );
		}

		Quests = 0;
		if( m_pMasterQuest != NULL )
		{
			Quests = 1;
		}
		fwrite( &Quests , sizeof( uint32 ), 1, pFile );
		if( Quests == 1 )
		{
			m_pMasterQuest->Save( pFile );
		}

	} // CCharacterSaveInstance::Save()

	void CCharacterSaveInstance::Load( FILE* pFile )	// open file to load from
	{

		uint16 Size( 0 );

		fread( &Size, sizeof( uint16 ), 1, pFile );
		m_TemplateName = StringUpper( ReadString( pFile, Size ) );


		fread( &Size, sizeof( uint16 ), 1, pFile );
		m_OriginalTemplateName = StringUpper( ReadString( pFile, Size ) );

		fread( &m_ExistenceTime, sizeof( float32 ), 1, pFile );


		fread( &m_IsPlayer, sizeof( bool ), 1, pFile );

		if( m_IsPlayer )
		{
			for( uint32 i = 0; i < KJournalStatistics; i++ )
			{
				int32 Value;
				fread( &Value, sizeof( EJournalStatistic ), 1, pFile );
				m_JournalStats[i] = Value;
			}
		}

		fread( &m_TransformationDuration, sizeof( float32 ), 1, pFile );

		fread( &m_LifeDuration, sizeof( float32 ), 1, pFile );

		fread( &m_TownTimer, sizeof( float32 ), 1, pFile );

		fread( &m_Difficulty, sizeof( EDifficulty ), 1, pFile );

		fread( &m_Scale, sizeof( float32 ), 1, pFile );

		fread( &m_Bravery, sizeof( float32 ), 1, pFile );

		fread( &m_MasterIndex, sizeof( int32 ), 1, pFile );

		fread( &m_DungeonSeed, sizeof( int32 ), 1, pFile );

		fread( &Size, sizeof( uint16 ), 1, pFile );
		m_Name = ReadString( pFile, Size );

		fread( &Size, sizeof( uint16 ), 1, pFile );
		m_AncestorName = ReadString( pFile, Size );

		fread( &m_Lineage, sizeof( int32 ), 1, pFile );

		fread( &m_IsMerchant, sizeof( bool ), 1, pFile );

		fread( &m_MerchantType, sizeof( EMerchant ), 1, pFile );

		fread( &m_Gender, sizeof( EGender ), 1, pFile );

		fread( &m_HeadIndex, sizeof( int32 ), 1, pFile );

		fread( &m_HairIndex, sizeof( int32 ), 1, pFile );

		fread( &m_LastDungeonLevel, sizeof( int32 ), 1, pFile );

		fread( &m_HasOpenPortal, sizeof( bool ), 1, pFile );
		fread( &m_PortalDepth, sizeof( uint32 ), 1, pFile );
		fread( &m_PortalPosition, sizeof( D3DXVECTOR3 ), 1, pFile );

		fread( &m_Position, sizeof( D3DXVECTOR3 ), 1, pFile );

		fread( &m_Orientation, sizeof( D3DXMATRIX ), 1, pFile );

		fread( &m_Level, sizeof( int32 ), 1, pFile );
		fread( &m_Experience, sizeof( int32 ), 1, pFile );

		fread( &m_HP, sizeof( float32 ), 1, pFile );
		fread( &m_MaxHP, sizeof( int32 ), 1, pFile );

		fread( &m_Fame, sizeof( int32 ), 1, pFile );
		fread( &m_FameRank, sizeof( int32 ), 1, pFile );

		fread( &m_Stamina, sizeof( float32 ), 1, pFile );
		fread( &m_MaxStamina, sizeof( int32 ), 1, pFile );

		fread( &m_Mana, sizeof( float32 ), 1, pFile );
		fread( &m_MaxMana, sizeof( int32 ), 1, pFile );

		fread( &m_ToHitBonus, sizeof( int32 ), 1, pFile );
		fread( &m_OriginalToHitBonus, sizeof( int32 ), 1, pFile );

		fread( &m_NaturalArmor, sizeof( int32 ), 1, pFile );
		fread( &m_OriginalNaturalArmor, sizeof( int32 ), 1, pFile );

		fread( &m_ExperienceAward, sizeof( int32 ), 1, pFile );

		fread( &m_FameAward, sizeof( int32 ), 1, pFile );

		fread( &m_UnusedStatPoints, sizeof( int32 ), 1, pFile );
		fread( &m_UnusedSkillPoints, sizeof( int32 ), 1, pFile );

		fread( &m_Unique, sizeof( bool ), 1, pFile );

		for( uint32 i = 0; i < KDamageTypes; i++ )
		{
			fread( &m_DamageResistance[i], sizeof( int32 ), 1, pFile );
		}

		for( int32 i = 0; i < KSkills; i++ )
		{
			int32 Point;
			fread( &Point, sizeof( int32 ), 1, pFile );
			m_SkillPoints[i] = Point;
		}

		for( int32 i = 0; i < KMagicSpheres; i++ )
		{
			for( uint32 j = 0; j < KMaximumSpellsPerSphere; j++ )
			{
				uint16 Size( 0 );
				fread( &Size, sizeof( uint16 ), 1, pFile );
				if( Size > 0 )
				{
					std::string Name = ReadString( pFile, Size );
					m_KnownSpells[i][j] = Name;
				}
				else
				{
					m_KnownSpells[i][j] = "";
				}
			}
		}

		m_ActiveSpellName = "";
		fread( &Size, sizeof( uint16 ), 1, pFile );
		if( Size > 0 )
		{
			m_ActiveSpellName = ReadString( pFile, Size );
		}

		fread( &m_Strength, sizeof( int32 ), 1, pFile );
		fread( &m_OriginalStrength, sizeof( int32 ), 1, pFile );
		fread( &m_Dexterity, sizeof( int32 ), 1, pFile );
		fread( &m_OriginalDexterity, sizeof( int32 ), 1, pFile );
		fread( &m_Vitality, sizeof( int32 ), 1, pFile );
		fread( &m_OriginalVitality, sizeof( int32 ), 1, pFile );
		fread( &m_Magic, sizeof( int32 ), 1, pFile );
		fread( &m_OriginalMagic, sizeof( int32 ), 1, pFile );

		fread( &m_WalkingSpeed, sizeof( float32 ), 1, pFile );
		fread( &m_RunningSpeed, sizeof( float32 ), 1, pFile );

		fread( &m_Gold, sizeof( int32 ), 1, pFile );

		for( uint32 j = 0; j < KActivationTypes; j++ )
		{
			int32 Effects;
			fread( &Effects , sizeof( int32 ), 1, pFile );
			for( int32 i = 0; i < Effects; i++ )
			{
				fread( &Size, sizeof( uint16 ), 1, pFile );
				std::string Name = ReadString( pFile, Size );

				fread( &Size, sizeof( uint16 ), 1, pFile );
				std::string Message = ReadString( pFile, Size );

				bool Exclusive;
				fread( &Exclusive, sizeof( bool ), 1, pFile );

				EEffectType Type;
				fread( &Type, sizeof( EEffectType ), 1, pFile );

				EDamageType DamageType;
				fread( &DamageType, sizeof( EDamageType ), 1, pFile );

				bool Positive;
				fread( &Positive, sizeof( bool ), 1, pFile );
				EEffectActivation Activation;
				fread( &Activation, sizeof( EEffectActivation ), 1, pFile );

				int32 ChanceOfSuccess;
				fread( &ChanceOfSuccess, sizeof( int32 ), 1, pFile );
				int32 ChanceOfSuccessBonus;
				fread( &ChanceOfSuccessBonus, sizeof( int32 ), 1, pFile );
				int32 ChanceOfSuccessBonusPercent;
				fread( &ChanceOfSuccessBonusPercent, sizeof( int32 ), 1, pFile );

				int32 Duration;
				fread( &Duration, sizeof( int32 ), 1, pFile );
				int32 DurationBonus;
				fread( &DurationBonus, sizeof( int32 ), 1, pFile );
				int32 DurationBonusPercent;
				fread( &DurationBonusPercent, sizeof( int32 ), 1, pFile );

				float32 Value;
				fread( &Value, sizeof( float32 ), 1, pFile );
				float32 ValueBonus;
				fread( &ValueBonus, sizeof( float32 ), 1, pFile );
				float32 ValueBonusPercent;
				fread( &ValueBonusPercent, sizeof( float32 ), 1, pFile );

				float32 Value2;
				fread( &Value2, sizeof( float32 ), 1, pFile );
				float32 Value2Bonus;
				fread( &Value2Bonus, sizeof( float32 ), 1, pFile );
				float32 Value2BonusPercent;
				fread( &Value2BonusPercent, sizeof(float32 ), 1, pFile );


				float32 Value3;
				fread( &Value3, sizeof( float32 ), 1, pFile );
				float32 Value3Bonus;
				fread( &Value3Bonus, sizeof( float32 ), 1, pFile );
				float32 Value3BonusPercent;
				fread( &Value3BonusPercent, sizeof( float32 ), 1, pFile );

				float32 PriceMultiplier;
				fread( &PriceMultiplier, sizeof( float32 ), 1, pFile );

				CEffect* pEffect = new CEffect( Type,
												Positive,
												Activation,
												Duration,
												Value,
												PriceMultiplier );
				m_pEffects[j].push_back( pEffect );
				pEffect->SetExclusive( Exclusive );
				pEffect->SetName( Name );
				pEffect->SetMessage( Message );

				pEffect->SetDamageType( DamageType );
						
				pEffect->SetValueBonus( ValueBonus );
				pEffect->SetValueBonusPercent( ValueBonusPercent );

				pEffect->SetDurationBonus( DurationBonus );
				pEffect->SetDurationBonusPercent( DurationBonusPercent );

				pEffect->SetChanceOfSuccess( ChanceOfSuccess );
				pEffect->SetChanceOfSuccessBonus( ChanceOfSuccessBonus );
				pEffect->SetChanceOfSuccessBonusPercent( ChanceOfSuccessBonusPercent );

				pEffect->SetValue2( Value2 );
				pEffect->SetValue2Bonus( Value2Bonus );
				pEffect->SetValue2BonusPercent( Value2BonusPercent );

				pEffect->SetValue3( Value3 );
				pEffect->SetValue3Bonus( Value3Bonus );
				pEffect->SetValue3BonusPercent( Value3BonusPercent );
			}
		}

		uint32 SubItems( 0 );
		fread( &SubItems , sizeof( uint32 ), 1, pFile );
		for( int32 j = 0; j < SubItems; j++ )
		{
			CItemSaveInstance* pInstance = new CItemSaveInstance();
			m_pItemInstances.push_back( pInstance );
			pInstance->Load( pFile );
		}

		uint32 Quests( 0 );
		fread( &Quests , sizeof( uint32 ), 1, pFile );
		for( int32 j = 0; j < Quests; j++ )
		{
			CQuest* pQuest = new CQuest();
			m_pQuests.push_back( pQuest );
			pQuest->Load( pFile );
		}

		Quests = 0;
		fread( &Quests , sizeof( uint32 ), 1, pFile );
		if( Quests > 0 )
		{
			m_pMasterQuest = new CQuest();
			m_pMasterQuest->Load( pFile );
		}
	} // CCharacterSaveInstance::Load()

