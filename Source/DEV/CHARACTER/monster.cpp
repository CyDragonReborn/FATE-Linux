/*
	monster.cpp

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
#include "../MEDIA/material.h"

#include "../GAMEOBJECTS/soundbank.h"

#include "../PARTICLES/particlemanager.h"
#include "../GLOWS/glowmanager.h"

#include "../LEVEL/level.h"

#include "../ITEM/spelltemplate.h"

#include "charactertemplate.h"
#include "character.h"
#include "inventory.h"
#include "monster.h"


CMonster::CMonster( LPDIRECT3DDEVICE8 pD3DDevice,						// direct3d device
				    CRefManager& pRefManager,							// pointer to the reference manager, for tracking submaterials
					CSettings& pSettings,								// pointer to the settings manager, for keeping track of card/machine capabilities
					CSpellTemplate& pSpellTemplate,						// spell template for casting
					CParticleManager& pParticleManager,					// particle manager
					CGlowManager& pGlowManager,						// particle manager
					CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
					CCharacterDescription& pDescription ) :				// character description
															CCharacter( pRefManager, 
																		pSettings, 
																		pSpellTemplate,
																		pParticleManager, 
																		pGlowManager,
																		pPositionalAudioManager ),
															m_CurrentUniqueIdle ( "" )
{

	m_MaxStamina = -1;
	ApplyDescription( pD3DDevice,
					  pDescription );
	m_TemplateName = pDescription.Name();
	m_OriginalTemplateName = m_TemplateName;

} // CMonster::CMonster()



void CMonster::Update( const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
					   float32 TimeElapsed, 	// time elapsed, in seconds
					   CLevel& pLevel )			// a scene to collide with
{
	CCharacter::Update( FocalPoint, TimeElapsed, pLevel );
} // CMonster::Update()

void CMonster::UpdateAI( float32 TimeElapsed, 	// time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{
	if( Target() != NULL &&
		( !Target()->Alive() ||
		  !Target()->InActiveRange() ) )
	{
		SetTarget( NULL );
	}

	if( Target() != NULL ||
		m_pCurrentAttack == NULL )
	{
		SelectAttack();
	}

	switch( m_AIState )
	{
	case KAIDying :
		DyingAI( TimeElapsed, pLevel );
		break;
	case KAIOperateItem :
		OperateItemAI( TimeElapsed, pLevel );
		break;
	case KAIInactive :
		SetAIState( KAIWander );
		break;
	case KAIReturningToTown :
		ReturnToTownAI( TimeElapsed, pLevel );
		break;
	case KAIWander :
		WanderAI( TimeElapsed, pLevel );
		break;
	case KAIFlee :
		FleeAI( TimeElapsed, pLevel );
		break;
	case KAIGetRange :
		GetRangeAI( TimeElapsed, pLevel );
		break;
	case KAIAttack :
		AttackAI( TimeElapsed, pLevel );
		break;
	case KAIApproach :
		ApproachAI( TimeElapsed, pLevel );
		break;
	case KAIHunt :
		HuntAI( TimeElapsed, pLevel );
		break;
	case KAIGetItem :
		GetItemAI( TimeElapsed, pLevel );
		break;
	case KAICast :
		CastAI( TimeElapsed, pLevel );
		break;
	case KAIUniqueIdle :
		UniqueIdleAI( TimeElapsed, pLevel );
		break;
	}

	if( IsMimic() )
	{
		if( AIState() == KAIWander ||
			AIState() == KAIUniqueIdle ||
			AIState() == KAIInactive )
		{
			m_DisplayedAsItem = kTrue;
		}
		else
		{
			m_DisplayedAsItem = kFalse;
		}
	}

} // CMonster::UpdateAI()

void CMonster::ApproachAI( float32 TimeElapsed, 	// time elapsed, in seconds
							 CLevel& pLevel )		// a scene to collide with
{
	SetRunning( kTrue );
	CCharacter::ApproachAI( TimeElapsed, pLevel );
} // CMonster::ApproachAI()


void CMonster::OperateItemAI( float32 TimeElapsed, 	// time elapsed, in seconds
							  CLevel& pLevel )		// a scene to collide with
{

	SetRunning( kTrue );

	CCharacter::OperateItemAI( TimeElapsed, pLevel );

} // CMonster::OperateItemAI()

void CMonster::AttackAI( float32 TimeElapsed, 	// time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{
	if( TargetItem() != NULL )
	{
		SetTargetItem( NULL );
	}

	if( Target() == NULL ||
		!Target()->Alive() ||
		IsPetNearDeath() ||
		!Target()->AvailableForCommand() )
	{
		SetAIState( KAIWander );
		SetTarget( NULL );
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
			return;
		}
	}

	D3DXVECTOR3 Delta = Target()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance >= m_FollowRadius &&
		!HasMaster() )
	{
		StopPathing();
		SetAIState( KAIWander );
		SetTarget( NULL );
		return;
	}

	
	if( HP() < (float32)MaxHP() / 2.0f )
	{
		SelectHealSpell();
		if( ActiveSpell() != NULL &&
			rand() %1000 < 5 )
		{
			if( ActiveSpell()->ManaCost() <= Mana() )
			{
				CastSpell( pLevel, 
						   ActiveSpell(),
						   NULL,
						   this,
						   D3DXVECTOR3( 0, 0, 0 ) );
				return;
			}
		}
	}
	SelectAttackSpell();
	if( ActiveSpell() != NULL &&
		rand() %1000 < 2 )
	{
		if( ActiveSpell()->ManaCost() <= Mana() )
		{
			CastSpell( pLevel, 
					ActiveSpell(),
					NULL,
					Target(),
					D3DXVECTOR3( 0, 0, 0 ) );
			return;
		}
	}

	SelectDefenseSpell();
	if( ActiveSpell() != NULL &&
		rand() %1000 < 2 )
	{
		if( ActiveSpell()->ManaCost() <= Mana() )
		{
			CastSpell( pLevel, 
						ActiveSpell(),
						NULL,
						this,
					    D3DXVECTOR3( 0, 0, 0 )  );
				return;
		}
	}

	if( Pets() < 1 )
	{
		SelectSummonSpell();

		if( ActiveSpell() != NULL &&
			rand() % 2000 < 2 )
		{
			if( ActiveSpell()->ManaCost() <= Mana() )
			{
				CastSpell( pLevel, 
							ActiveSpell(),
							NULL,
							Target(),
							D3DXVECTOR3( 0, 0, 0 ) );
				return;
			}
		}
	}
	if( Target() != NULL &&
		( Target()->Pets() > 1 ||
		  Target()->IsSummoned() ) )
	{
		SelectBanishSpell();

		if( ActiveSpell() != NULL &&
			rand() % 2000 < 5 )
		{
			if( Target()->IsSummoned() &&
				Target()->HasMaster() )
			{
				SetTarget( Target()->Master() );
			}
			if( Target() == NULL )
			{
				return;
			}
			if( ActiveSpell()->ManaCost() <= Mana() )
			{
				CastSpell( pLevel, 
							ActiveSpell(),
							NULL,
							Target(),
							D3DXVECTOR3( 0, 0, 0 ) );
				return;
			}
		}
	}

	if( !InAttackRange() ||
		!ValidLineOfSight( pLevel ) ) 
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
		TurnTowardPosition( Target()->Position(), TimeElapsed );
		if( !PerformingAttack() )
		{
			// see if we're too close for comfort
			if( RandomIntegerBetween( 0, 100 ) < 40 && 
				m_TimeSincePath > 1 && 
				Target() != NULL &&
				TooCloseForAttack() )
			{
				SetAIState( KAIGetRange );
				float32 Range( AttackRange() );
				D3DXVECTOR3 Destination = pLevel.RandomOpenPosition( Target()->Position(),
																	 Range * .75f,
																	 Range );
				SetDestination( pLevel, Destination.x, Destination.z );
			}
			else
			{
				if( FacingTarget() )
				{
					Attack();
					SelectAttack();
				}
			}
		}
	}

	// make sure we haven't wandered too far afield as a pet
	if( HasMaster() )
	{
		D3DXVECTOR3 Delta;
		
		Delta = Master()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		// run if we're far away
		if( Distance > KPetAttackRadius )
		{
			SetTarget( NULL, kFalse );
			SetAIState( KAIWander );
		}
	}

} // CMonster::AttackAI()

void CMonster::SelectAttackSpell( void )
{
	SetActiveSpell( NULL );
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[KMagicAttack][i] != NULL )
		{
			if( rand() % 1000 < 200 )
			{
				SetActiveSpell( m_KnownSpells[KMagicAttack][i] );
				return;
			}
		}
	}
} // CMonster::SelectAttackSpell()

void CMonster::SelectSummonSpell( void )
{
	SetActiveSpell( NULL );
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[KMagicCharm][i] != NULL &&
			m_KnownSpells[KMagicCharm][i]->IsSummonSpell() )
		{
			if( rand() % 1000 < 300 &&
				m_pActiveSpell == NULL )
			{
				SetActiveSpell( m_KnownSpells[KMagicCharm][i] );
				return;
			}
		}
	}
} // CMonster::SelectSummonSpell()

void CMonster::SelectBanishSpell( void )
{
	SetActiveSpell( NULL );
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[KMagicCharm][i] != NULL &&
			m_KnownSpells[KMagicCharm][i]->Name() == "Banish" )
		{
			if( rand() % 1000 < 300 &&
				m_pActiveSpell == NULL )
			{
				SetActiveSpell( m_KnownSpells[KMagicCharm][i] );
				return;
			}
		}
	}
} // CMonster::SelectBanishSpell()


void CMonster::SelectDefenseSpell( void )
{
	SetActiveSpell( NULL );
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[KMagicDefense][i] != NULL &&
			!m_KnownSpells[KMagicDefense][i]->IsHealSpell() )
		{
			if( rand() % 1000 < 200 )
			{
				SetActiveSpell( m_KnownSpells[KMagicDefense][i] );
				return;
			}
		}
	}
} // CMonster::SelectDefenseSpell()

void CMonster::SelectHealSpell( void )
{
	SetActiveSpell( NULL );
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[KMagicDefense][i] != NULL &&
			m_KnownSpells[KMagicDefense][i]->IsHealSpell() )
		{
			if( rand() % 1000 < 200 )
			{
				SetActiveSpell( m_KnownSpells[KMagicDefense][i] );
				return;
			}
		}
	}
} // CMonster::SelectHealSpell()

void CMonster::SelectHealMinionSpell( void )
{
	SetActiveSpell( NULL );
	for( uint32 i = 0; i < KMaximumSpellsPerSphere; i++ )
	{
		if( m_KnownSpells[KMagicDefense][i] != NULL &&
			m_KnownSpells[KMagicDefense][i]->IsHealSpell() &&
			m_KnownSpells[KMagicDefense][i]->Target() == KTargetCharacter )
		{
			if( rand() % 1000 < 200 )
			{
				SetActiveSpell( m_KnownSpells[KMagicDefense][i] );
				return;
			}
		}
	}
} // CMonster::SelectHealMinionSpell()

void CMonster::HuntAI( float32 TimeElapsed, 	// time elapsed, in seconds
					   CLevel& pLevel )			// a scene to collide with
{
	if( Target() == NULL ||
		!Target()->Alive() ||
		IsPetNearDeath() ||
		Target()->IsPetNearDeath() )
	{
		SetAIState( KAIWander );
		SetTarget( NULL );
		return;
	}
	if( Target()->Alignment() == Alignment() ||
		Target()->Alignment() == KNeutral )
	{
		SetAIState( KAIInactive );
		SetTarget( NULL );
		return;
	}

	// run if possible
	SetRunning( kTrue );

	D3DXVECTOR3 Delta = Target()->Position() - Position();
	Delta.y = 0;

	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance >= m_FollowRadius &&
		!HasMaster() )
	{
		StopPathing();
		SetAIState( KAIWander );
		SetTarget( NULL );
		return;
	}

	bool LineOfSightValid( ValidLineOfSight( pLevel ) );

	// mages don't try to get in close. They only fight when appropached
	if( m_Tactics == KTacticsMage )
	{
		if( m_TimeSincePath > 1 )
		{
			Delta = Target()->Position() - m_FinalTargetPosition;
			Delta.y = 0;

			float32 Distance = D3DXVec3Length( &Delta );
			if( Distance >= m_AttentionRadius * .6f ||
				RandomIntegerBetween( 0, 100 ) < 1 )
			{
				D3DXVECTOR3 Destination = pLevel.RandomOpenPosition( Target()->Position(),
																	m_AttentionRadius * .25f,
																	m_AttentionRadius * .6f );
				SetDestination( pLevel,
								Destination.x,
								Destination.z );
				// if pathing failed, then stop attacking this character
				if( !Pathing() )
				{
					CCharacter::SetTarget( NULL );
					CCharacter::SetAIState( KAIWander );

				}
			}
		}
	}
	else
	{
		if( ( !InAttackRange() ||
			!LineOfSightValid ) &&
			m_TimeSincePath > 1 )
		{
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
				return;
			}
		}
	}

	if( HP() < (float32)MaxHP() / 2.0f )
	{
		SelectHealSpell();
		if( ActiveSpell() != NULL &&
			rand() %1000 < 5 )
		{
			if( ActiveSpell()->ManaCost() <= Mana() )
			{
				CastSpell( pLevel, 
						ActiveSpell(),
						NULL,
						this,
						D3DXVECTOR3( 0, 0, 0 )  );
				return;
			}
		}
	}

	// heal any minions, if applicable
	for( uint32 i = 0; i < Pets(); i++ )
	{
		if( Pet( i )->HP() < (float32)Pet( i )->MaxHP() / 2.0f )
		{
			SelectHealMinionSpell();
			if( ActiveSpell() != NULL &&
				rand() %1000 < 5 )
			{
				if( ActiveSpell()->ManaCost() <= Mana() )
				{
					CastSpell( pLevel, 
							ActiveSpell(),
							NULL,
							Pet( i ),
						    D3DXVECTOR3( 0, 0, 0 )  );
					return;
				}
			}
		}
	}

	SelectAttackSpell();
	if( ActiveSpell() != NULL &&
		rand() %1000 < 2 )
	{
		if( ActiveSpell()->ManaCost() <= Mana() )
		{
			CastSpell( pLevel, 
					ActiveSpell(),
					NULL,
					Target(),
					D3DXVECTOR3( 0, 0, 0 )  );
			return;
		}
	}


	SelectDefenseSpell();
	if( ActiveSpell() != NULL &&
		rand() %1000 < 2 )
	{
		if( ActiveSpell()->ManaCost() <= Mana() )
		{
			CastSpell( pLevel, 
						ActiveSpell(),
						NULL,
						this,
					    D3DXVECTOR3( 0, 0, 0 )  );
				return;
		}
	}

	if( Pets() < 1 )
	{
		SelectSummonSpell();

		if( ActiveSpell() != NULL &&
			rand() % 2000 < 2 )
		{
			if( ActiveSpell()->ManaCost() <= Mana() )
			{
				CastSpell( pLevel, 
							ActiveSpell(),
							NULL,
							Target(),
							D3DXVECTOR3( 0, 0, 0 ) );
				return;
			}
		}
	}

	if( Target() != NULL &&
		( Target()->Pets() > 1 ||
		  Target()->IsSummoned() ) )
	{
		
		SelectBanishSpell();

		if( ActiveSpell() != NULL &&
			rand() % 2000 < 5 )
		{
			if( Target()->IsSummoned() &&
				Target()->HasMaster() )
			{
				SetTarget( Target()->Master() );
			}
			if( Target() == NULL )
			{
				return;
			}

			if( ActiveSpell()->ManaCost() <= Mana() )
			{
				CastSpell( pLevel, 
							ActiveSpell(),
							NULL,
							Target(),
							D3DXVECTOR3( 0, 0, 0 ) );
				return;
			}
		}
	}


	// if in range of our hunted character, start attacking
	if( InAttackRange() &&
		LineOfSightValid )
	{
		SetAIState( KAIAttack );
		StopPathing();
		return;
	}

	// make sure we haven't wandered too far afield as a pet
	if( HasMaster() && Target() != NULL )
	{
		D3DXVECTOR3 Delta;
		
		Delta = Master()->Position() - Target()->Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		// run if we're far away
		if( Distance > KPetAttackRadius )
		{
			SetTarget( NULL );
			SetAIState( KAIWander );
		}
	}

} // CMonster::HuntAI()

void CMonster::WanderAI( float32 TimeElapsed, 	// time elapsed, in seconds
						 CLevel& pLevel )		// a scene to collide with
{
	if( IsMerchant() && 
		Target() != NULL
		&& Target()->AIState() != KAIApproach )
	{
		SetTarget( NULL );
	}

	SetRunning( kFalse );
	if( rand() %1000 < 100 &&
		Alignment() != KNeutral &&
		!IsPetNearDeath() )
	{
		EAlignment TargetAlignment( KGood );
		if( Alignment() == KGood )
		{
			TargetAlignment = KEvil;
		}
		float32 AttentionRadius( m_AttentionRadius );
		if( HasMaster() )
		{
			AttentionRadius = KPetAttackRadius;
		}
		CCharacter* pCharacter = pLevel.FindCharacterWithinView( DisplayMatrix(),
																 TargetAlignment,
																 m_MotionRadius,
																 m_ViewRadius,
																 AttentionRadius );

		// look in the master's view cone to see what THEY sey
		if( HasMaster() && pCharacter == NULL )
		{
			 pCharacter = pLevel.FindCharacterWithinView( Master()->DisplayMatrix(),
														  TargetAlignment,
														  m_MotionRadius,
														  m_ViewRadius,
														  AttentionRadius );
		}
		// make sure the target is near the master
		if( HasMaster() && pCharacter != NULL )
		{
			D3DXVECTOR3 Delta;
			
			Delta = Master()->Position() - pCharacter->Position();
			Delta.y = 0;

			float32 Distance = D3DXVec3Length( &Delta );
			// run if we're far away
			if( Distance > KPetAttackRadius )
			{
				pCharacter = NULL;
			}
		}

		if( pCharacter != NULL )
		{			
			if( pCharacter->HasMaster() )
			{
				// 10% chance we take on the master instead
				if( RandomIntegerBetween( 0, 100 ) < 10 )
				{
					pCharacter = pCharacter->Master();
				}
			}
			StopPathing();
			SetAIState( KAIHunt );
			SetTarget( pCharacter );

			//CCharacter::FollowCharacter( pCharacter,
			//							 pLevel );

			if( m_TimeSinceIdleSound > .5f )
			{
				if( rand() % 1000 < 800 )
				{
					m_pCharacterSounds->PlaySample( KSoundRoar,
													m_Position,
													255,
													m_SoundRadius );
					m_TimeSinceIdleSound = 0;
				}
			}

			return;
		}
	}
	if( WalkingSpeed() == 0 )
	{
		return;
	}
	if( !Pathing() && rand() %1000 < 2 )
	{
		// if we're a pet, only wander around our master
		if( HasMaster() )
		{
			float32 MaximumDistance = KPetFollowRadius * .9f;
			SetDestination( pLevel,
							RandomBetween( Master()->Position().x - MaximumDistance, 
										   Master()->Position().x + MaximumDistance ),
							RandomBetween( Master()->Position().z - MaximumDistance, 
										   Master()->Position().z + MaximumDistance ) );
		}
		else
		{
			SetDestination( pLevel,
							RandomBetween( m_Position.x - 100, m_Position.x + 100 ),
							RandomBetween( m_Position.z - 100, m_Position.z + 100 ) );
		}
	}

	if( HasMaster() )
	{
		EAlignment TargetAlignment( KGood );
		if( Alignment() == KGood )
		{
			TargetAlignment = KEvil;
		}
		SetRunning( Master()->Running() );
		if( Master()->StaminaFloat() < 1 )
		{
			SetRunning( kFalse );
		}
		if( Master()->Target() != NULL &&
			Master()->Target()->Alignment() == TargetAlignment &&
			!IsPetNearDeath() )
		{

			// make sure the target is near the master
			D3DXVECTOR3 Delta;
				
			Delta = Master()->Position() - Master()->Target()->Position();
			Delta.y = 0;

			float32 Distance = D3DXVec3Length( &Delta );
			// run if we're far away
			if( Distance <= KPetFollowRadius )
			{
				SetTarget( Master()->Target() );
				SetAIState( KAIHunt );
				return;
			}
		}
		
		D3DXVECTOR3 Delta;
		
		Delta = Master()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		// run if we're far away
		if( Distance > KPetFollowRadius + 2 )
		{
			SetRunning( kTrue );
		}
		else if( Distance <= KPetFollowRadius )
		{
			if( !Master()->Pathing() )
			{
				SetRunning( kFalse );
			}
		}

		if( Distance > KPetTeleportRadius )
		{
			D3DXVec3Normalize( &Delta, &Delta );
			Delta *= 20;
			Delta += Position();
			SetPosition( pLevel.RandomOpenPosition( Delta, 12 ) );
			DropToGround( pLevel );
		}

		if( IsPetNearDeath() )
		{
			SetRunning( kTrue );
		}

		if( rand() %1000 < 100 ||
			IsPetNearDeath() )
		{
			if( Pathing() )
			{
				Delta = Master()->Position() - m_FinalTargetPosition;
			}
			else
			{
				Delta = Master()->Position() - Position();
			}
			Delta.y = 0;
			Distance = D3DXVec3Length( &Delta );

			// make sure we haven't wandered too far from our master
			if( ( IsPetNearDeath() && !Pathing() ) ||
				( Distance > KPetFollowRadius && 
				  m_TimeSincePath > .25f ) )
			{
				SetRunning( kTrue );
				float32 MaximumDistance = KPetFollowRadius * .9f;
				SetDestination( pLevel,
								RandomBetween( Master()->Position().x - MaximumDistance, 
												Master()->Position().x + MaximumDistance ),
								RandomBetween( Master()->Position().z - MaximumDistance, 
												Master()->Position().z + MaximumDistance ) );			}
		}
	}

	if( !Pathing() && AIState() == KAIWander )
	{
		if( rand() % 1000 < 2 )
		{
			if( m_UniqueIdles.size() > 0 && m_TimeSinceUniqueIdle > 10 )
			{
				int32 Idle = RandomIntegerBetween( 0, m_UniqueIdles.size() - 1 );
				if( m_pModel->AnimationExists( m_UniqueIdles[Idle] ) )
				{
					m_CurrentUniqueIdle = m_UniqueIdles[Idle];
					BlendAnimation( m_UniqueIdles[Idle], kFalse, .25f );
					QueueBlendAnimation( "IDLE", kTrue, .25f );
					SetAIState( KAIUniqueIdle );

					m_pCharacterSounds->PlaySample( KSoundUniqueIdle,
													m_Position,
													255,
													m_SoundRadius );
					m_TimeSinceIdleSound = 0;
					m_TimeSinceUniqueIdle = 0;

				}
			} 
		}
	}

} // CMonster::WanderAI()

void CMonster::FleeAI( float32 TimeElapsed, 	// time elapsed, in seconds
					   CLevel& pLevel )		// a scene to collide with
{
	SetTarget( NULL, kFalse );
	if( IsMerchant() && 
		Target() != NULL &&
		Target()->AIState() != KAIApproach )
	{
		SetTarget( NULL );
	}
	SetRunning( kTrue );

	if( !Pathing() )
	{
		if( RandomIntegerBetween( 0, 100 ) < 50 )
		{
			SetAIState( KAIWander );
		}
		// if we're a pet, only wander around our master
		if( HasMaster() )
		{
			float32 MaximumDistance = KPetFollowRadius * 2.0f;
			SetDestination( pLevel,
							RandomBetween( Master()->Position().x - MaximumDistance, 
										   Master()->Position().x + MaximumDistance ),
							RandomBetween( Master()->Position().z - MaximumDistance, 
										   Master()->Position().z + MaximumDistance ) );
		}
		else
		{
			SetDestination( pLevel,
							RandomBetween( m_Position.x - 100, m_Position.x + 100 ),
							RandomBetween( m_Position.z - 100, m_Position.z + 100 ) );
		}
	}

} // CMonster::FleeAI()

void CMonster::GetRangeAI( float32 TimeElapsed, 	// time elapsed, in seconds
					   CLevel& pLevel )		// a scene to collide with
{
	SetRunning( kTrue );

	if( !Pathing() )
	{
		if( Target() != NULL )
		{
			SetAIState( KAIAttack );
		}
		else
		{
			SetAIState( KAIWander );
		}
	}

} // CMonster::GetRangeAI()

void CMonster::UniqueIdleAI( float32 TimeElapsed, 	// time elapsed, in seconds
							 CLevel& pLevel )		// a scene to collide with
{
	if( !Alive() )
	{
		return;
	}

	if( !m_pModel->AnimationPlaying( m_CurrentUniqueIdle ) )
	{
		SetAIState( KAIWander );
	}

	if( HasMaster() )
	{
		D3DXVECTOR3 Delta;
		
		Delta = Master()->Position() - Position();
		Delta.y = 0;

		float32 Distance = D3DXVec3Length( &Delta );
		// run if we're far away
		if( Distance > KPetFollowRadius )
		{
			SetAIState( KAIWander );
			BlendAnimation( "IDLE", kTrue, .1f );
		}
	}

} // CMonster::UniqueIdleAI()

void CMonster::RespondToAttack( CCharacter* pAttacker )	// attacking character
{

	if( !Alive() )
	{
		return;
	}
	if( AIState() == KAIInTown ||
		AIState() == KAIReturningToTown )
	{
		return;
	}

	if( pAttacker == NULL ||
		pAttacker == this )
	{
		return;
	}
	if( AIState() == KAIFlee )
	{
		return;
	}
	if( MaxHP() > 0 )
	{
		if( ( (float32)HP() / (float32)MaxHP() ) < ( 1.0f - m_Bravery ) )
		{
			if( RandomIntegerBetween( 0, 100 ) < 50 )
			{
				SetAIState( KAIFlee );
				return;
			}
		}
	}

	if( !pAttacker->Alive() ||
		pAttacker->Alignment() == Alignment() ||
		pAttacker->Alignment() == KNeutral  )
	{
		return;
	}

	if( Target() == NULL )
	{
		if( !PerformingCast() &&
			pAttacker->Alive() )
		{
			if( AIState() == KAIUniqueIdle )
			{
				BlendAnimation( "IDLE", kTrue, .1f );
			}
			SetAIState( KAIHunt );

			// recognize masters and attack them
			if( pAttacker->HasMaster() &&
				RandomIntegerBetween( 0, 100 ) < 10 )
			{
				SetTarget( pAttacker->Master() );
			}
			else
			{
				SetTarget( pAttacker );
			}
		}
	}
	else if( !PerformingAttack() &&
			 !PerformingCast() )
	{
		if( rand() %1000 < 50 )
		{
			SetAIState( KAIAttack );
			// recognize masters and attack them
			if( pAttacker->HasMaster() &&
				RandomIntegerBetween( 0, 100 ) < 10 )
			{
				SetTarget( pAttacker->Master() );
			}
			else
			{
				SetTarget( pAttacker );
			}
		}
		else if( AIState() == KAIInactive ||
				 AIState() == KAIWander )
		{
			SetAIState( KAIAttack );
			// recognize masters and attack them
			if( pAttacker->HasMaster() &&
				RandomIntegerBetween( 0, 100 ) < 10 )
			{
				SetTarget( pAttacker->Master() );
			}
			else
			{
				SetTarget( pAttacker );
			}
		}
	}
} // CMonster::RespondToAttack()