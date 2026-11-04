/*
	charactertemplate.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _CHARACTERTEMPLATE_H
#define _CHARACTERTEMPLATE_H

#include "d3dx8_shim.h"


#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../GAMECLIENT/gamerules.h"

#include "../ITEM/effect.h"

class CDataFile;
class CMaterial;
class CParticleManager;

// describes a natrual attack
class CAttackDescription
{
public :
	CAttackDescription( const std::string& Animation,	// animation name
						bool Armed,						// armed attack?
						float32 AttackRadius,			// radius to attack within
						float32 StrikeRadius,			// radius in which strikes are valid
						uint32 MinimumDamage,			// minimum damage points
						uint32 MaximumDamage,			// maximum damage points
						int32 ToHitBonus,				// bonus in to-hit calculation
						float32 Speed = 1 ) :			// default speed is 1
																m_Animation( Animation ),
																m_Armed( Armed ),
																m_ToHitBonus( ToHitBonus ),
																m_MinimumDamage( MinimumDamage ),
																m_MaximumDamage( MaximumDamage ),
																m_AttackRadius( AttackRadius ),
																m_StrikeRadius( StrikeRadius ),
																m_Speed( Speed ),
																m_AnimationIndex( 0 ),
																m_DamageType( KDamageSlashing )
	{
	}

	CAttackDescription( CAttackDescription* pDescription ) :	// attack to copy
																m_Animation( pDescription->Animation() ),
																m_MinimumDamage( pDescription->MinimumDamage() ),
																m_MaximumDamage( pDescription->MaximumDamage() ),
																m_AttackRadius( pDescription->AttackRadius() ),
																m_StrikeRadius( pDescription->StrikeRadius() ),
																m_Armed( pDescription->Armed() ),
																m_Speed( pDescription->Speed() ),
																m_ToHitBonus( pDescription->ToHitBonus() ),
																m_AnimationIndex( pDescription->AnimationIndex() ),
																m_DamageType( pDescription->DamageType() )
	{
		m_BreathParticle = pDescription->m_BreathParticle;
		m_BreathVelocity = pDescription->m_BreathVelocity;

		for( uint32 i = 0; i < KActivationTypes; i++ )
		{
			for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
			{
				DELETE_SAFELY( m_pEffects[i][j] );
			}
			m_pEffects[i].clear();
		}

		for( i = 0; i < KActivationTypes; i++ )
		{
			for( uint32 j = 0; j < pDescription->Effects( (EEffectActivation)i ); j++ )
			{
				AddEffect( pDescription->Effect( (EEffectActivation)i, j ) );
			}
		}

	}

	~CAttackDescription()
	{
		for( uint32 i = 0; i < KActivationTypes; i++ )
		{
			for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
			{
				DELETE_SAFELY( m_pEffects[i][j] );
			}
			m_pEffects[i].clear();
		}
	}
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	SetAnimationIndex( uint32 Index )	{	m_AnimationIndex = Index;	};

	void	SetDamageType( EDamageType Type )	{	m_DamageType = Type;		};

	void	SetDamageMinimumDamage( uint32 MinimumDamage, 
						   uint32 MaximumDamage )	
	{
		m_MinimumDamage = MinimumDamage;
		m_MaximumDamage = MaximumDamage;
	}

	void	AddNewEffect( CEffect* pEffect )	// effect to add
	{
		m_pEffects[pEffect->Activation()].push_back( pEffect );
	} 

	void	AddEffect( CEffect* pEffect )	// effect to add
	{
		m_pEffects[pEffect->Activation()].push_back( new CEffect( pEffect ) );
	} 

	void	AddBreath( const D3DXVECTOR3& Velocity,
					   int32 Particle )
	{
		m_BreathVelocity.push_back( Velocity );
		m_BreathParticle.push_back( Particle );
	}

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	EDamageType			DamageType( void )				{	return m_DamageType;				};

	int32				ToHitBonus( void )				{	return m_ToHitBonus;				};

	uint32				BreathParticles( void )			{	return m_BreathParticle.size();		};

	int32				BreathParticle( uint32 Index )	{	return m_BreathParticle[Index];		};

	const D3DXVECTOR3&	BreathVelocity( uint32 Index )	{	return m_BreathVelocity[Index];		};

	bool				Armed( void )				{	return m_Armed;				};

	const std::string&	Animation( void )			{	return m_Animation;			};

	uint32				MinimumDamage( void )			{	return m_MinimumDamage;				};
	uint32				MaximumDamage( void )			{	return m_MaximumDamage;				};

	float32				AttackRadius( void )	{	return m_AttackRadius;		};
	float32				StrikeRadius( void )	{	return m_StrikeRadius;		};

	float32				Speed( void )			{	return m_Speed;				};

	int32				AnimationIndex( void )	{	return m_AnimationIndex;	};

	uint32				Effects( EEffectActivation ActivationType )		// type to count
	{
		return m_pEffects[ActivationType].size();
	} 

	float32				EffectValue( EEffectActivation ActivationType,	// type of activation
									 EEffectType Type )					// type of effect
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
	} 

	CEffect*			Effect( EEffectActivation ActivationType,		// type to get
								uint32 Index )							// index within that type
	{
		return m_pEffects[ActivationType][Index];
	}
private :
	std::string m_Animation;

	bool	m_Armed;

	uint32	m_MinimumDamage;
	uint32	m_MaximumDamage;

	int32	m_AnimationIndex;

	float32 m_AttackRadius;
	float32 m_StrikeRadius;

	float32 m_Speed;
	
	int32	m_ToHitBonus;

	EDamageType	m_DamageType;

	std::vector< CEffect* >	m_pEffects[KActivationTypes];

	std::vector< int32 >		m_BreathParticle;

	std::vector< D3DXVECTOR3 >	m_BreathVelocity;

};

// character description contains monster description information

class CCharacterDescription
{

public:

	CCharacterDescription( const std::string& ModelPath,			// path to character model
						   const std::string& CollisionPath,		// path to collision model
						   const std::string& Name,					// name of character
						   const std::string& Family,				// monster family
						   EAlignment Alignment,					// alignment - good, evil, neutral
						   float32 WalkingSpeed,					// walking speed
						   float32 RunningSpeed,					// running speed
						   float32 TurnRate,						// turning rate
						   float32 ViewRadius,						// view cone radius, degrees
						   float32 MotionRadius,					// motion detection radius, feet
						   float32 AttentionRadius,					// visibility radius, feet
						   float32 FollowRadius,					// follow radius, feet
						   uint32 ExperienceAward,					// experience award for defeat
						   uint32 MinHP,							// min hp for level 1
						   uint32 MaxHP,							// max hp for level 1
						   uint32 ToHitBonus,						// to hit bonus
						   uint32 NaturalArmor,						// natural ac bonus
						   uint32 Strength,							// strength
						   uint32 Dexterity,						// dexterity
						   uint32 Vitality,							// vitality
						   uint32 Magic	) :							// magic
											m_DualWielding( kFalse ),
											m_HasLightPool( kFalse ),
											m_CanMatchSpeed( kTrue ),
											m_OverrideBase( "" ),
											m_OverrideAlpha( "" ),
											m_OverrideIllumination( "" ),
											m_OverrideNormal( "" ),
											m_OverrideSphere( "" ),
											m_AnimationPath( "" ),
											m_ModelPath( ModelPath ),
											m_CollisionPath( CollisionPath ),
											m_Name( Name ),
											m_MimicName( Name ),
											m_Family( Family ),
											m_Alignment( Alignment ),
											m_WalkingSpeed( WalkingSpeed ),
											m_RunningSpeed( RunningSpeed ),
											m_TurnRate( TurnRate ),
											m_ViewRadius( ViewRadius ),
											m_MotionRadius( MotionRadius ),
											m_AttentionRadius( AttentionRadius ),
											m_FollowRadius( FollowRadius ),
											m_ExperienceAward( ExperienceAward ),
											m_FameAward( 0 ),
											m_MinHP( MinHP ),
											m_MaxHP( MaxHP ),
											m_ToHitBonus( ToHitBonus ),
											m_NaturalArmor( NaturalArmor ),
											m_Strength( Strength ),
											m_Dexterity( Dexterity ),
											m_Vitality( Vitality ),
											m_Magic( Magic ),
											m_BloodParticle( "BLOOD" ),
											m_BaseLevel( 1 ),
											m_MinimumScale( 1 ),
											m_MaximumScale( 1 ),
											m_SoundRadius( 32 ),
											m_MinimumDepth( 1 ),
											m_MaximumDepth( 100 ),
											m_Rarity( 1 ),
											m_VerticalOffset( 0 ),
											m_MinionsCollideable( kFalse ),
											m_Tactics( KTacticsFighter ),
											m_Bravery( 1 ),
											m_ReachBonus( 0 ),
											m_LightPoolPath ( "" ),
											m_LightPoolType ( KLightFlicker ),
											m_LightPoolRadius ( 0.0f ),
											m_pOverrideMaterial( NULL )
	{
		for( uint32 i = 0; i < KDamageTypes; i++ )
		{
			m_DamageResistance[i] = 0;
		}
		for( i = 0; i < KSkills; i++ )
		{
			m_SkillPoints[i] = 0;
		}
	}

	~CCharacterDescription( void )
	{
		for( uint32 i = 0; i < KActivationTypes; i++ )
		{
			for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
			{
				DELETE_SAFELY( m_pEffects[i][j] );
			}
			m_pEffects[i].clear();
		}

		for( i = 0; i < m_UnarmedAttacks.size(); i++ )
		{
			DELETE_SAFELY( m_UnarmedAttacks[i] );
		}
		m_UnarmedAttacks.clear();
	}

	
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void	AddNewEffect( CEffect* pEffect )	// effect to add
	{
		m_pEffects[pEffect->Activation()].push_back( pEffect );
	} 

	void	AddEffect( CEffect* pEffect )	// effect to add
	{
		m_pEffects[pEffect->Activation()].push_back( new CEffect( pEffect ) );
	} 

	void	AddGreetMessage( const std::string& Message )	{	m_GreetMessages.push_back( Message );	};

	void	SetBravery( float32 Bravery )					{	m_Bravery = Bravery;		};

	void	SetTactics( ETactic Tactics )					{	m_Tactics = Tactics;		};

	void	SetMinionsCollideable( bool State )				{	m_MinionsCollideable = State;	};

	void	SetRarity( int32 Rarity )						{	m_Rarity = Rarity;			};

	void	SetLightPool( const std::string& Path,	// path to light texture
						  float32 Radius,			// radius of light pool
						  ELightPoolType Type )		// flicker or pulse?
	{
		m_HasLightPool = kTrue;
		m_LightPoolRadius = Radius;
		m_LightPoolPath = Path;
		m_LightPoolType = Type;
	}

	void	SetReachBonus( float32 Bonus )					{	m_ReachBonus = Bonus;		};

	void	SetDamageResistance( EDamageType Type,		// type of damage
								 int32 Value		)	// amount of resistance
							{	
								m_DamageResistance[Type] = Value;		
							};

	void	SetMinimumDepth( int32 Depth )					{	m_MinimumDepth = Depth;		};

	void	SetMaximumDepth( int32 Depth )					{	m_MaximumDepth = Depth;		};

	void	SetSoundRadius( float32 Radius )				{	m_SoundRadius = Radius;		};

	void	SetBaseLevel( uint32 Level )					{	m_BaseLevel = Level;		};

	void	SetBloodParticle( const std::string& Name )		{	m_BloodParticle = Name;		};

	void	AddUnarmedAttack( CAttackDescription* pAttack )
	{
		m_UnarmedAttacks.push_back( pAttack );
	}

	void	SetMimicName( const std::string& Name )		{	m_MimicName = Name;						};

	void	SetAnimationPath( const std::string& Path )	{	m_AnimationPath = Path;						};

	void	AddGreetSound( const std::string& Path )	{	m_GreetSoundPath.push_back( Path );		};
	void	AddDieSound( const std::string& Path )		{	m_DieSoundPath.push_back( Path );		};
	void	AddStepSound( const std::string& Path )		{	m_StepSoundPath.push_back( Path );		};
	void	AddIdleSound( const std::string& Path )		{	m_IdleSoundPath.push_back( Path );		};
	void	AddUniqueIdleSound( const std::string& Path )		{	m_UniqueIdleSoundPath.push_back( Path );		};
	void	AddRoarSound( const std::string& Path )		{	m_RoarSoundPath.push_back( Path );		};

	void	AddAttackSound( const std::string& Animation,	
							const std::string& Path )	
	{	
		m_AttackSoundAnimation.push_back( Animation );	
		m_AttackSoundPath.push_back( Path );	
	};

	void	AddArmedAttackSound( const std::string& Path )	
	{	
		m_ArmedAttackSoundPath.push_back( Path );	
	};

	void	AddStrikeSound( const std::string& Animation,	
							const std::string& Path )	
	{	
		m_StrikeSoundAnimation.push_back( Animation );	
		m_StrikeSoundPath.push_back( Path );	
	};

	void	AddMissSound( const std::string& Animation,	
						  const std::string& Path )		
	{	
		m_MissSoundAnimation.push_back( Animation );		
		m_MissSoundPath.push_back( Path );		
	};

	void	SetScaleRange( float32 Minimum,
						   float32 Maximum )
	{
		m_MinimumScale = Minimum;
		m_MaximumScale = Maximum;
	};

	void	SetMaterialOverride( const std::string& Base,
								 const std::string& Alpha,
								 const std::string& Illumination,
								 const std::string& Normal,
								 const std::string& Sphere )
	{
		if( StringUpper( Base ) != "NONE" )
		{
			m_OverrideBase = Base;
		}
		if( StringUpper( Alpha ) != "NONE" )
		{
			m_OverrideAlpha = Alpha;
		}
		if( StringUpper( Illumination ) != "NONE" )
		{
			m_OverrideIllumination = Illumination;
		}
		if( StringUpper( Normal ) != "NONE" )
		{
			m_OverrideNormal = Normal;
		}
		if( StringUpper( Sphere ) != "NONE" )
		{
			m_OverrideSphere = Sphere;
		}
	}

	void	SetVerticalOffset( float32 Offset )				{	m_VerticalOffset = Offset;				};

	void	SetDualWielding( bool State )					{	m_DualWielding = State;					};

	void	SetCanMatchSpeed( bool State )					{	m_CanMatchSpeed = State;				};

	void	AddSupportedEquipment( EItemType Type )			{	m_SupportedEquipment.push_back( Type );	};

	void	AddPreferredWeapon( const std::string& Name )	{	m_PreferredWeapon.push_back( Name );	};
	void	AddPreferredShield( const std::string& Name )	{	m_PreferredShield.push_back( Name );	};
	void	AddPreferredArmor( const std::string& Name )	{	m_PreferredArmor.push_back( Name );		};

	void	SetSkill( ESkill Skill,
					  uint32 Value )						{	m_SkillPoints[Skill] = Value;			};

	void	AddMinion( const std::string& Name,		// minion name
					   uint32 Minimum,				// minimum number
					   uint32 Maximum )				// maximum number
	{
		m_Minions.push_back( Name );
		m_MinionMinimum.push_back( Minimum );
		m_MinionMaximum.push_back( Maximum );
	}
	void	AddUniqueIdle( const std::string Name )
	{
		m_UniqueIdles.push_back( Name );
	}

	void	AddSpell( const std::string Name )
	{
		m_Spells.push_back( Name );
	}

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	uint32				Effects( EEffectActivation ActivationType )		// type to count
	{
		return m_pEffects[ActivationType].size();
	} 

	CEffect*			Effect( EEffectActivation ActivationType,		// type to get
								uint32 Index )							// index within that type
	{
		return m_pEffects[ActivationType][Index];
	}


	uint32				GreetMessages( void )					{	return m_GreetMessages.size();	};

	const std::string&	GreetMessage( uint32 Index )			{	return m_GreetMessages[Index];	};

	float32				ReachBonus( void )						{	return m_ReachBonus;			};

	float32				Bravery( void )							{	return m_Bravery;				};

	float32				VerticalOffset( void )					{	return m_VerticalOffset;		};

	int32				Rarity( void )							{	return m_Rarity;				};

	const std::vector< std::string >&	UniqueIdles( void )		{	return m_UniqueIdles;			};

	uint32				Spells( void )							{	return m_Spells.size();			};

	const std::string&	Spell( uint32 Index )					{	return m_Spells[Index];				};

	int32				DamageResistance( EDamageType Type )	{	return m_DamageResistance[Type];	};

	uint32				Minions( void )							{	return m_Minions.size();		};

	const std::string&	MinionName( uint32 Index )				{	return m_Minions[Index];		};
	int32				MinimumMinions( uint32 Index )			{	return m_MinionMinimum[Index];	};
	int32				MaximumMinions( uint32 Index )			{	return m_MinionMaximum[Index];	};

	int32	MinimumDepth( void )		{	return m_MinimumDepth;				};

	int32	MaximumDepth( void )		{	return m_MaximumDepth;				};

	float32 SoundRadius( void )			{	return m_SoundRadius;				};

	uint32	SupportedEquipment( void )	{	return m_SupportedEquipment.size();	};
	uint32	PreferredWeapons( void )	{	return m_PreferredWeapon.size();	};
	uint32	PreferredShields( void )	{	return m_PreferredShield.size();	};
	uint32	PreferredArmors( void )		{	return m_PreferredArmor.size();		};

	EItemType			SupportedEquipment( uint32 Index )	{	return m_SupportedEquipment[Index];	};
	const std::string&	PreferredWeapon( uint32 Index )		{	return m_PreferredWeapon[Index];	};
	const std::string&	PreferredShield( uint32 Index )		{	return m_PreferredShield[Index];	};
	const std::string&	PreferredArmor( uint32 Index )		{	return m_PreferredArmor[Index];		};


	bool	HasOverrideMaterial( void )
	{
		if( m_OverrideBase.length() > 0 ||
			m_OverrideAlpha.length() > 0 ||
			m_OverrideIllumination.length() > 0 ||
			m_OverrideNormal.length() > 0 ||
			m_OverrideSphere.length() > 0 )
		{
			return kTrue;
		}
		return kFalse;
	}

	const std::string&	OverrideBase( void )		{	return m_OverrideBase;				};
	const std::string&	OverrideAlpha( void )		{	return m_OverrideAlpha;				};
	const std::string&	OverrideIllumination( void ){	return m_OverrideIllumination;		};
	const std::string&	OverrideNormal( void )		{	return m_OverrideNormal;			};
	const std::string&	OverrideSphere( void )		{	return m_OverrideSphere;			};

	float32				MinimumScale( void )		{	return m_MinimumScale;				};
	float32				MaximumScale( void )		{	return m_MaximumScale;				};

	uint32				BaseLevel( void )			{	return m_BaseLevel;					};

	uint32				GreetSounds( void )			{	return m_GreetSoundPath.size();		};
	uint32				DieSounds( void )			{	return m_DieSoundPath.size();		};
	uint32				StepSounds( void )			{	return m_StepSoundPath.size();		};
	uint32				IdleSounds( void )			{	return m_IdleSoundPath.size();		};
	uint32				UniqueIdleSounds( void )	{	return m_UniqueIdleSoundPath.size();		};
	uint32				RoarSounds( void )			{	return m_RoarSoundPath.size();		};

	const std::string&	BloodParticle( void )		{	return m_BloodParticle;				};

	const std::string&	GreetSound( uint32 Index )	{	return m_GreetSoundPath[Index];		};
	const std::string&	DieSound( uint32 Index )	{	return m_DieSoundPath[Index];		};
	const std::string&	StepSound( uint32 Index )	{	return m_StepSoundPath[Index];		};
	const std::string&	IdleSound( uint32 Index )	{	return m_IdleSoundPath[Index];		};
	const std::string&	UniqueIdleSound( uint32 Index )	{	return m_UniqueIdleSoundPath[Index];		};
	const std::string&	RoarSound( uint32 Index )	{	return m_RoarSoundPath[Index];		};

	uint32				ArmedAttackSounds( void )	{	return m_ArmedAttackSoundPath.size();	};
	uint32				AttackSounds( void )		{	return m_AttackSoundPath.size();	};
	uint32				StrikeSounds( void )		{	return m_StrikeSoundPath.size();	};
	uint32				MissSounds( void )			{	return m_MissSoundPath.size();		};

	const std::string&	ArmedAttackSound( uint32 Index )	{	return m_ArmedAttackSoundPath[Index];	};
	const std::string&	AttackSound( uint32 Index )	{	return m_AttackSoundPath[Index];	};
	const std::string&	StrikeSound( uint32 Index )	{	return m_StrikeSoundPath[Index];	};
	const std::string&	MissSound( uint32 Index )	{	return m_MissSoundPath[Index];		};

	const std::string&	AttackSoundAnimation( uint32 Index )	{	return m_AttackSoundAnimation[Index];	};
	const std::string&	StrikeSoundAnimation( uint32 Index )	{	return m_StrikeSoundAnimation[Index];	};
	const std::string&	MissSoundAnimation( uint32 Index )		{	return m_MissSoundAnimation[Index];		};

	const std::string&	ModelPath( void )		{	return m_ModelPath;			};
	const std::string&	CollisionPath( void )	{	return m_CollisionPath;		};
	const std::string&	AnimationPath( void )	{	return m_AnimationPath;		};

	bool				HasLightPool( void )	{	return m_HasLightPool;		};

	float32				LightPoolRadius( void )	{	return m_LightPoolRadius;	};

	ELightPoolType		LightPoolType( void )	{	return m_LightPoolType;		};

	const std::string&	LightPoolPath( void )	{	return m_LightPoolPath;		};

	const std::string&	Name( void )			{	return m_Name;				};
	const std::string&	MimicName( void )		{	return m_MimicName;			};
	const std::string&	Family( void )			{	return m_Family;			};

	bool				DualWielding( void )	{	return m_DualWielding;		};

	bool				CanMatchSpeed( void )	{	return m_CanMatchSpeed;		};

	ETactic				Tactics( void )			{	return m_Tactics;			};

	bool				MinionsCollideable( void )	{	return m_MinionsCollideable;	};

	EAlignment			Alignment( void )		{	return m_Alignment;			};

	float32				WalkingSpeed( void )	{	return m_WalkingSpeed;		};
	float32				RunningSpeed( void )	{	return m_RunningSpeed;		};
	float32				TurnRate( void )		{	return m_TurnRate;			};

	float32				ViewRadius( void )		{	return m_ViewRadius;		};
	float32				MotionRadius( void )	{	return m_MotionRadius;		};
	float32				AttentionRadius( void )	{	return m_AttentionRadius;	};
	float32				FollowRadius( void )	{	return m_FollowRadius;		};


	int32				SkillPoints( ESkill Skill )	{	return m_SkillPoints[Skill];	};
	int32				ExperienceAward( void )	{	return m_ExperienceAward;	};

	int32				FameAward( void )		{	return m_FameAward;	};

	int32				MaxHP( void )			{	return m_MaxHP;				};
	int32				MinHP( void )			{	return m_MinHP;				};

	int32				ToHitBonus( void )		{	return m_ToHitBonus;		};

	int32				NaturalArmor( void )	{	return m_NaturalArmor;		};

	int32				Strength( void )				{	return m_Strength;		};
	int32				Dexterity( void )				{	return m_Dexterity;		};
	int32				Vitality( void )				{	return m_Vitality;		};
	int32				Magic( void )					{	return m_Magic;			};

	uint32				UnarmedAttacks( void )	{	return m_UnarmedAttacks.size();	};

	CAttackDescription*	UnarmedAttack( uint32 Index )	{	return m_UnarmedAttacks[Index];	};

	std::string			m_OverrideBase;
	std::string			m_OverrideAlpha;
	std::string			m_OverrideIllumination;
	std::string			m_OverrideNormal;
	std::string			m_OverrideSphere;

	std::vector< EItemType >	m_SupportedEquipment;
	std::vector< std::string >	m_PreferredWeapon;
	std::vector< std::string >	m_PreferredArmor;
	std::vector< std::string >	m_PreferredShield;

private:
	std::vector< CEffect* >	m_pEffects[KActivationTypes];

	int32				m_SkillPoints[KSkills];

	int32				m_Rarity;

	int32				m_DamageResistance[KDamageTypes];

	std::vector< std::string >	m_UniqueIdles;

	std::vector< std::string >	m_Spells;

	std::string			m_BloodParticle;

	std::string			m_ModelPath;
	std::string			m_AnimationPath;
	std::string			m_CollisionPath;

	std::string			m_Name;
	std::string			m_MimicName;
	std::string			m_Family;

	std::string			m_LightPoolPath;
	ELightPoolType		m_LightPoolType;	
	float32				m_LightPoolRadius;
	bool				m_HasLightPool;

	std::vector< std::string >	m_GreetMessages;

	EAlignment			m_Alignment;

	bool				m_DualWielding;
	bool				m_CanMatchSpeed;

	ETactic				m_Tactics;

	bool				m_MinionsCollideable;

	float32				m_ReachBonus;

	std::vector< std::string >		m_ArmedAttackSoundPath;
	std::vector< std::string >		m_AttackSoundPath;
	std::vector< std::string >		m_StrikeSoundPath;
	std::vector< std::string >		m_MissSoundPath;
	std::vector< std::string >		m_AttackSoundAnimation;
	std::vector< std::string >		m_StrikeSoundAnimation;
	std::vector< std::string >		m_MissSoundAnimation;

	std::vector< std::string >		m_GreetSoundPath;
	std::vector< std::string >		m_DieSoundPath;
	std::vector< std::string >		m_StepSoundPath;
	std::vector< std::string >		m_IdleSoundPath;
	std::vector< std::string >		m_UniqueIdleSoundPath;
	std::vector< std::string >		m_RoarSoundPath;


	std::vector< std::string >		m_Minions;
	std::vector< int32 >			m_MinionMinimum;
	std::vector< int32 >			m_MinionMaximum;

	// mobility values
	float32				m_WalkingSpeed;
	float32				m_RunningSpeed;
	float32				m_TurnRate;

	float32				m_MinimumScale;
	float32				m_MaximumScale;

	float32				m_Bravery;

	// perception values
	float32				m_ViewRadius;
	float32				m_MotionRadius;
	float32				m_AttentionRadius;
	float32				m_FollowRadius;

	float32				m_SoundRadius;

	int32				m_MinimumDepth;		// minimum dungeon depth at which it appears
	int32				m_MaximumDepth;		// maximum dungeon depth at which it appears

	int32				m_MinHP;
	int32				m_MaxHP;

	float32				m_VerticalOffset;

	int32				m_ExperienceAward;
	int32				m_FameAward;

	int32				m_ToHitBonus;

	int32				m_NaturalArmor;

	std::vector< CAttackDescription* >	m_UnarmedAttacks;

	// statistics
	int32				m_Strength;
	int32				m_Dexterity;
	int32				m_Vitality;
	int32				m_Magic;

	uint32				m_BaseLevel;

	CMaterial*			m_pOverrideMaterial;

};


class CCharacterTemplate
{

public:

	CCharacterTemplate( CParticleManager& pParticleManager,	// particle manager
						const std::string& Path );			// path to template
	~CCharacterTemplate( void );
	
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////



	uint32					Families( void )						{	return m_pCharacterDescription.size();				};
	uint32					Characters( uint32 Family )				{	return m_pCharacterDescription[Family].size();		};

	uint32					Characters( void )						{	return m_pBaseCharacterDescription.size();			};

	CCharacterDescription*	CharacterDescription( uint32 Index )	{	return m_pBaseCharacterDescription[Index];			};

	CCharacterDescription*	CharacterDescription( uint32 Family,
												  uint32 Index )		{	return m_pCharacterDescription[Family][Index];	};

	CCharacterDescription*	CharacterDescription( std::string Name );	// name of character to retrieve

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

private:

	int32	FindFamily( const std::string& FamilyName );	// family name to add

	int32	AddFamily( const std::string& FamilyName );		// family name to add

	void	ParseTemplate( CParticleManager& pParticleManager,	// particle manager
						   const std::string& Path,				// path to template
						   ERank Rank = KRankNormal );			// rank to convert items to


	std::vector< std::vector< CCharacterDescription* > >	m_pCharacterDescription;
	std::vector< std::string >								m_CharacterFamily;
	std::vector< CCharacterDescription* >					m_pBaseCharacterDescription;

};

#endif