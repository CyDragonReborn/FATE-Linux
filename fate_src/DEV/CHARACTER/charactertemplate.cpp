/*
	charactertemplate.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/datafile.h"

#include "../ITEM/effect.h"

#include "../PARTICLES/particlemanager.h"

#include "charactertemplate.h"

CCharacterTemplate::CCharacterTemplate( CParticleManager& pParticleManager,		// particle manager
										const std::string& Path )				// path to template
{
	ParseTemplate( pParticleManager, Path );
	ParseTemplate( pParticleManager, Path, KRankElite );
	ParseTemplate( pParticleManager, Path, KRankLegendary );
} // CCharacterTemplate::CCharacterTemplate()

CCharacterTemplate::~CCharacterTemplate( void )
{
	for( uint32 i = 0; i < m_pCharacterDescription.size(); i++ )
	{
		for( uint32 j = 0; j < m_pCharacterDescription[i].size(); j++ )
		{
			DELETE_SAFELY( m_pCharacterDescription[i][j] );
		}
	}
	m_pCharacterDescription.clear();
	m_pBaseCharacterDescription.clear();
} // CCharacterTemplate::~CCharacterTemplate()

void CCharacterTemplate::ParseTemplate( CParticleManager& pParticleManager,	// particle manager
										const std::string& Path,			// path to template
										ERank Rank )						// rank to convert items to
{
	std::string BasePath = GetLocalPath();
	CDataFile* pDataFile = new CDataFile( Path );


	uint32 Monsters = pDataFile->DataGroups( "MONSTER" );

	for( uint32 w = 0; w < Monsters; w++ )
	{
		std::string			ModelPath( "" );
		std::string			AnimationPath( "" );
		std::string			CollisionPath( "" );

		std::string			Name( "MONSTER" );
		std::string			Family( "MONSTER" );

		EAlignment			Alignment( KEvil );
		
		// mobility values
		float32				WalkingSpeed( 15 );
		float32				RunningSpeed( 20 );
		float32				TurnRate( 10 );
		
		// perception values
		float32				ViewRadius( 40 );
		float32				MotionRadius( 10 );
		float32				AttentionRadius( 50 );
		float32				FollowRadius( 72 );

		int32				ExperienceAward( 0 );
		int32				MinHP( 10 );
		int32				MaxHP( 16 );

		int32				NaturalArmor( 0 );

		std::string			UnarmedAttackAnimation( "ATTACK" );
		uint32				UnarmedAttackMinimumDamage( 1 );
		uint32				UnarmedAttackMaximumDamage( 6 );
		float32				AttackRadius( 6 );
		float32				StrikeRadius( 8 );

		// statistics
		int32				Strength( 10 );
		int32				Dexterity( 10 );
		int32				Vitality( 10 );
		int32				Magic( 10 );
		int32				ToHitBonus( 10 );
		int32				AttackToHitBonus( 0 );

		// parse tile widths
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "MONSTER", w );


		CDataGroup* pSubGroup = pMasterGroup->GetSubDataGroup( "FAMILY" );
		if( pSubGroup != NULL )
		{
			Family = StringUpper( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "NAME" );
		if( pSubGroup != NULL )
		{
			Name = KRankName[Rank] + pSubGroup->Item( 0 );
		}

		if( Rank > KRankNormal )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_DEPTH" );
			// we don't care about these low-level monsters for elite status
			if( pSubGroup != NULL )
			{
				if( (int32)atoi( pSubGroup->Item( 0 ).c_str() ) < 15 )
				{
					continue;
				}
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ALIGNMENT" );
		if( pSubGroup != NULL )
		{
			std::string Align = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KAlignments; i++ )
			{
				if( Align == KAlignmentName[i] )
				{
					Alignment = (EAlignment)i;
				}
			}
		}
	

		pSubGroup = pMasterGroup->GetSubDataGroup( "MODEL" );
		if( pSubGroup != NULL )
		{
			ModelPath = pSubGroup->Item( 0 );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "COLLISION_MODEL" );
		if( pSubGroup != NULL )
		{
			CollisionPath = pSubGroup->Item( 0 );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "WALKING_SPEED" );
		if( pSubGroup != NULL )
		{
			WalkingSpeed = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "RUNNING_SPEED" );
		if( pSubGroup != NULL )
		{
			RunningSpeed = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TURN_RATE" );
		if( pSubGroup != NULL )
		{
			TurnRate = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "VIEW_RADIUS" );
		if( pSubGroup != NULL )
		{
			ViewRadius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MOTION_RADIUS" );
		if( pSubGroup != NULL )
		{
			MotionRadius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ATTENTION_RADIUS" );
		if( pSubGroup != NULL )
		{
			AttentionRadius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "FOLLOW_RADIUS" );
		if( pSubGroup != NULL )
		{
			FollowRadius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "HP" );
		if( pSubGroup != NULL )
		{

			MinHP = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			MaxHP = (int32)atoi( pSubGroup->Item( 1 ).c_str() );
			MinHP = (int32)( (float32)MinHP * KRankToHitMultiplier[Rank] );
			MaxHP = (int32)( (float32)MaxHP * KRankToHitMultiplier[Rank] );

		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "EXPERIENCE" );
		if( pSubGroup != NULL )
		{
			ExperienceAward = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			ExperienceAward = (int32)( (float32)ExperienceAward * KRankExperienceMultiplier[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "NATURAL_ARMOR" );
		if( pSubGroup != NULL )
		{
			NaturalArmor = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			//NaturalArmor = (int32)( (float32)NaturalArmor * KRankPowerMultiplier[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "STRENGTH" );
		if( pSubGroup != NULL )
		{
			Strength = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			Strength = (int32)( (float32)Strength * KRankPowerMultiplier[Rank] );
		}
	
		pSubGroup = pMasterGroup->GetSubDataGroup( "DEXTERITY" );
		if( pSubGroup != NULL )
		{
			Dexterity = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			Dexterity = (int32)( (float32)Dexterity * KRankPowerMultiplier[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "VITALITY" );
		if( pSubGroup != NULL )
		{
			Vitality = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			Vitality = (int32)( (float32)Vitality * KRankPowerMultiplier[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MAGIC" );
		if( pSubGroup != NULL )
		{
			Magic = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			Magic = (int32)( (float32)Magic * KRankPowerMultiplier[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TOHIT" );
		if( pSubGroup != NULL )
		{
			ToHitBonus = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			ToHitBonus = (int32)( (float32)ToHitBonus * KRankToHitMultiplier[Rank] );
		}

		CCharacterDescription* pDescription = new CCharacterDescription( ModelPath,
																		 CollisionPath,
																		 Name,
																		 Family,
																		 Alignment,
																		 WalkingSpeed,
																		 RunningSpeed,
																		 TurnRate,
																		 ViewRadius,
																		 MotionRadius,
																		 AttentionRadius,
																		 FollowRadius,
																		 ExperienceAward,
																		 MinHP,
																		 MaxHP,
																		 ToHitBonus,
																		 NaturalArmor,
																		 Strength,
																		 Dexterity,
																		 Vitality,
																		 Magic );

		pSubGroup = pMasterGroup->GetSubDataGroup( "MIMIC_NAME" );
		if( pSubGroup != NULL )
		{
			pDescription->SetMimicName( pSubGroup->Item( 0 ) );
		}

		uint32 Resistances = pMasterGroup->SubDataGroups( "DAMAGE_RESISTANCE" );

		for( uint32 r = 0; r < Resistances; r++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "DAMAGE_RESISTANCE", r );

			EDamageType DamageType( KDamageSlashing );
			std::string Type = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KDamageTypes; i++ )
			{
				if( Type == KDamageNames[i] )
				{
					DamageType = (EDamageType)i;
					break;
				}
			}
			int32 Value = (int32)atoi( pSubGroup->Item( 1 ).c_str() );
			pDescription->SetDamageResistance( DamageType, Value );
		}		

		uint32 Effects = pMasterGroup->SubDataGroups( "EFFECT" );
		for( uint32 e = 0; e < Effects; e++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "EFFECT", e );


			CDataGroup* pEffectGroup;
			float32			Value( 0 );
			bool			Positive( kTrue );
			int32			Duration( KDurationInstant );
			EEffectType		Type( KEffectHP );
			EEffectActivation Activation( KActivationUsage );

			uint32 SubGroups = pSubGroup->SubDataGroups();

			pEffectGroup = pSubGroup->GetSubDataGroup( "TYPE" );

			if( pEffectGroup != NULL )
			{
				std::string EffectTypeString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KAllEffectTypes; a++ )
				{
					if( EffectTypeString == KEffectNames[a] )
					{
						Type = (EEffectType)a;
						break;
					}
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "ACTIVATION" );

			if( pEffectGroup != NULL )
			{
				std::string ActivationString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KActivationTypes; a++ )
				{
					if( ActivationString == KActivationNames[a] )
					{
						Activation = (EEffectActivation)a;
						break;
					}
				}

			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION" );

			if( pEffectGroup != NULL )
			{
				std::string DurationString = StringUpper( pEffectGroup->Item( 0 ) );
				if( DurationString == "INSTANT" )
				{
					Duration = KDurationInstant;
				}
				else if( DurationString == "ALWAYS" )
				{
					Duration = KDurationAlways;
				}
				else
				{
					Duration = (int32)atoi( DurationString.c_str() );
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE" );

			if( pEffectGroup != NULL )
			{
				Value = (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "POSITIVE" );

			if( pEffectGroup != NULL )
			{
				Positive = atoi( pEffectGroup->Item( 0 ).c_str() ) == 1;
			}



			CEffect* pBaseEffect = new CEffect( Type,
											Positive,
											Activation,
											Duration,
											Value,
											0 );

			uint32 SFXItems = pSubGroup->SubDataGroups( "SFX" );
			for( uint32 s = 0; s < SFXItems; s++ )
			{
				pEffectGroup = pSubGroup->GetSubDataGroup( "SFX", s );

				std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
				ESFXType Type( KSFXStationaryTarget );
				for( uint32 a = 0; a < KSFXTypes; a++ )
				{
					if( TypeString == KSFXTypeName[a] )
					{
						Type = (ESFXType)a;
						break;
					}
				}
				float32 Duration = (float32)atof( pEffectGroup->Item( 1 ).c_str() );
				bool Looping = atoi( pEffectGroup->Item( 2 ).c_str() ) == 1;
				bool Exclusive = kFalse;
				if( pEffectGroup->Items() >= 7 )
				{
					Exclusive = atoi( pEffectGroup->Item( 6 ).c_str() ) == 1;
				}

				pBaseEffect->AddSFX( Type, 
							     pEffectGroup->Item( 5 ),
							     Duration,
							     Looping,
								 (float32)atof( pEffectGroup->Item( 3 ).c_str() ),
								 (float32)atof( pEffectGroup->Item( 4 ).c_str() ),
								 Exclusive,
								 kFalse );

			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "DAMAGE_TYPE" );

			if( pEffectGroup != NULL )
			{
				EDamageType DamageType( KDamageSlashing );
				std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
				for( uint32 a = 0; a < KDamageTypes; a++ )
				{
					if( TypeString == KDamageNames[a] )
					{
						DamageType = (EDamageType)a;
						break;
					}
				}
				pBaseEffect->SetDamageType( DamageType );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValue2( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
				if( pBaseEffect->Value2() != pBaseEffect->Value() )
				{
					pBaseEffect->SetRangedValue( kTrue );
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValue2Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValue2BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValueBonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValueBonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValue3( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValue3Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetValue3BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetDurationBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetDurationBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetChanceOfSuccess( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "MESSAGE" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetMessage( pEffectGroup->Item( 0 ) );
			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "NAME" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetName( pEffectGroup->Item( 0 ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "EXCLUSIVE" );

			if( pEffectGroup != NULL )
			{
				pBaseEffect->SetExclusive( atoi( pEffectGroup->Item( 0 ).c_str() ) == 1 );
			}


			pDescription->AddNewEffect( pBaseEffect );
		}


		uint32 Skills = pMasterGroup->SubDataGroups( "SKILL" );

		for( r = 0; r < Skills; r++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "SKILL", r );

			ESkill SkillType( KSkillSword );
			std::string Type = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KSkills; i++ )
			{
				if( Type == KSkillNames[i] )
				{
					SkillType = (ESkill)i;
					break;
				}
			}
			int32 Value = (int32)atoi( pSubGroup->Item( 1 ).c_str() );
			Value = (int32)( (float32)Value * KRankPowerMultiplier[Rank] );
			pDescription->SetSkill( SkillType, Value );
		}		

		pSubGroup = pMasterGroup->GetSubDataGroup( "SOUND_RADIUS" );
		if( pSubGroup != NULL )
		{
			pDescription->SetSoundRadius( (float32)atof( pSubGroup->Item( 0 ).c_str() ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MINIMUM_DEPTH" );
		if( pSubGroup != NULL )
		{
			pDescription->SetMinimumDepth( (int32)atoi( pSubGroup->Item( 0 ).c_str() ) + KRankLevelOffset[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_DEPTH" );
		if( pSubGroup != NULL )
		{
			if( Rank > KRankElite )
			{
				pDescription->SetMaximumDepth( 1000 );
			}
			else
			{
				pDescription->SetMaximumDepth( (int32)atoi( pSubGroup->Item( 0 ).c_str() ) + ( KRankLevelOffset[Rank] * 2 ) );
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "VERTICAL_OFFSET" );
		if( pSubGroup != NULL )
		{
			pDescription->SetVerticalOffset( (float32)atof( pSubGroup->Item( 0 ).c_str() ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "RARITY" );
		if( pSubGroup != NULL )
		{
			int32 Rarity = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			if( Rarity < 1000 )
			{

				Rarity = (int32)( (float32)Rarity * KRankMonsterRarityMultiplier[Rank] );
				if( Rarity >= 1000 )
				{
					Rarity = 999;
				}
			}

			pDescription->SetRarity( Rarity );
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "SCALE" );
		if( pSubGroup != NULL )
		{
			pDescription->SetScaleRange( (float32)atof( pSubGroup->Item( 0 ).c_str() ),
										 (float32)atof( pSubGroup->Item( 1 ).c_str() ) );
		}

		uint32 SupportedEquipment = pMasterGroup->SubDataGroups( "CAN_EQUIP" );
		for( uint32 i = 0; i < SupportedEquipment; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "CAN_EQUIP", i );
			std::string TypeName = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 t = 0; t < KItemTypes; t++ )
			{
				if( KItemTypeName[t] == TypeName )
				{
					pDescription->AddSupportedEquipment( (EItemType)t );
					break;
				}
			}
		}

		uint32 Greetings = pMasterGroup->SubDataGroups( "GREET_MESSAGE" );
		for( i = 0; i < Greetings; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "GREET_MESSAGE", i );
			std::string Message = pSubGroup->Item( 0 );

			while( Message.find( "\\n" ) != -1 )
			{
				int32 Index = Message.find( "\\n" );
				Message.replace( Index, 2, "\n" );
			}
			while( Message.find( "\\b" ) != -1 )
			{
				int32 Index = Message.find( "\\b" );
				Message.replace( Index, 2, "\b" );
			}

			pDescription->AddGreetMessage( Message );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "BRAVERY" );
		if( pSubGroup != NULL )
		{
			pDescription->SetBravery( (float32)atof( pSubGroup->Item( 0 ).c_str() ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "REACH_BONUS" );
		if( pSubGroup != NULL )
		{
			pDescription->SetReachBonus( (float32)atof( pSubGroup->Item( 0 ).c_str() ) );
		}

		uint32 PreferredWeapons = pMasterGroup->SubDataGroups( "PREFERRED_WEAPON" );
		for( i = 0; i < PreferredWeapons; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "PREFERRED_WEAPON", i );
			pDescription->AddPreferredWeapon( StringUpper( pSubGroup->Item( 0 ) ) );
		}

		uint32 PreferredArmor = pMasterGroup->SubDataGroups( "PREFERRED_ARMOR" );
		for( i = 0; i < PreferredArmor; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "PREFERRED_ARMOR", i );
			pDescription->AddPreferredArmor( StringUpper( pSubGroup->Item( 0 ) ) );
		}

		uint32 PreferredShield = pMasterGroup->SubDataGroups( "PREFERRED_SHIELD" );
		for( i = 0; i < PreferredShield; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "PREFERRED_SHIELD", i );
			pDescription->AddPreferredShield( StringUpper( pSubGroup->Item( 0 ) ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MATERIAL_OVERRIDE" );
		if( pSubGroup != NULL )
		{
			pDescription->SetMaterialOverride( pSubGroup->Item( 0 ),
											   pSubGroup->Item( 1 ),
											   pSubGroup->Item( 2 ),
											   pSubGroup->Item( 3 ),
											   pSubGroup->Item( 4 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "BASE_LEVEL" );
		if( pSubGroup != NULL )
		{
			int32 Level = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			//Level += KRankLevelOffset[Rank];

			pDescription->SetBaseLevel( Level );
		}

		// parse extra attacks
		uint32 UnarmedAttacks = pMasterGroup->SubDataGroups( "UNARMED_ATTACK" );
		if( UnarmedAttacks > 0 )
		{
			for( uint32 i = 0; i < UnarmedAttacks; i++ )
			{
				AttackToHitBonus = 0;
				pSubGroup = pMasterGroup->GetSubDataGroup( "UNARMED_ATTACK", i );

				CDataGroup* pSubDataGroup = pSubGroup->GetSubDataGroup( "ANIMATION" );
				if( pSubDataGroup != NULL )
				{
					UnarmedAttackAnimation = pSubDataGroup->Item( 0 );
				}

				pSubDataGroup = pSubGroup->GetSubDataGroup( "TOHITBONUS" );
				if( pSubDataGroup != NULL )
				{
					AttackToHitBonus = (int32)atoi( pSubDataGroup->Item( 0 ).c_str() );
					AttackToHitBonus = (int32)( (float32)AttackToHitBonus * KRankToHitMultiplier[Rank] );
				}

				pSubDataGroup = pSubGroup->GetSubDataGroup( "RANGE" );
				if( pSubDataGroup != NULL )
				{
					AttackRadius = (float32)atof( pSubDataGroup->Item( 0 ).c_str() );
					StrikeRadius = (float32)atof( pSubDataGroup->Item( 1 ).c_str() );
				}

				pSubDataGroup = pSubGroup->GetSubDataGroup( "DAMAGE" );
				if( pSubDataGroup != NULL )
				{
					UnarmedAttackMinimumDamage = (int32)atoi( pSubDataGroup->Item( 0 ).c_str() );
					UnarmedAttackMaximumDamage = (int32)atoi( pSubDataGroup->Item( 1 ).c_str() );
					//UnarmedAttackMinimumDamage = (int32)( (float32)UnarmedAttackMinimumDamage * KRankToHitMultiplier[Rank] );
					//UnarmedAttackMaximumDamage = (int32)( (float32)UnarmedAttackMaximumDamage * KRankToHitMultiplier[Rank] );
				}

				CAttackDescription* pAttackDescription = new CAttackDescription( UnarmedAttackAnimation,
																		kFalse,
																		AttackRadius,
																		StrikeRadius,
																		UnarmedAttackMinimumDamage,
																		UnarmedAttackMaximumDamage,
																		AttackToHitBonus );

				pDescription->AddUnarmedAttack( pAttackDescription );


				pSubDataGroup = pSubGroup->GetSubDataGroup( "DAMAGE_TYPE" );

				if( pSubDataGroup != NULL )
				{
					EDamageType DamageType( KDamageSlashing );
					std::string TypeString = StringUpper( pSubDataGroup->Item( 0 ) );
					for( uint32 a = 0; a < KDamageTypes; a++ )
					{
						if( TypeString == KDamageNames[a] )
						{
							DamageType = (EDamageType)a;
							break;
						}
					}
					pAttackDescription->SetDamageType( DamageType );
				}

				uint32 BreathParticles = pSubGroup->SubDataGroups( "BREATH_PARTICLE" );
				for( uint32 b = 0; b < BreathParticles; b++ )
				{
					pSubDataGroup = pSubGroup->GetSubDataGroup( "BREATH_PARTICLE", b );
					int32 Particle = pParticleManager.GetParticleType( pSubDataGroup->Item( 0 ) );
					D3DXVECTOR3 Velocity( 0, 0, 0 );
					Velocity.x = (float32)atof( pSubDataGroup->Item( 1 ).c_str() );
					Velocity.y = (float32)atof( pSubDataGroup->Item( 2 ).c_str() );
					Velocity.z = (float32)atof( pSubDataGroup->Item( 3 ).c_str() );

					pAttackDescription->AddBreath( Velocity, Particle );
				}

				uint32 Effects = pSubGroup->SubDataGroups( "EFFECT" );
				for( uint32 e = 0; e < Effects; e++ )
				{
					pSubDataGroup = pSubGroup->GetSubDataGroup( "EFFECT", e );


					CDataGroup* pEffectGroup;
					float32			Value( 0 );
					int32			Duration( KDurationInstant );
					EEffectType		Type( KEffectHP );
					EEffectActivation Activation( KActivationUsage );

					uint32 SubGroups = pSubDataGroup->SubDataGroups();

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "TYPE" );

					if( pEffectGroup != NULL )
					{
						std::string EffectTypeString = StringUpper( pEffectGroup->Item( 0 ) );
						for( uint32 a = 0; a < KAllEffectTypes; a++ )
						{
							if( EffectTypeString == KEffectNames[a] )
							{
								Type = (EEffectType)a;
								break;
							}
						}
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "ACTIVATION" );

					if( pEffectGroup != NULL )
					{
						std::string ActivationString = StringUpper( pEffectGroup->Item( 0 ) );
						for( uint32 a = 0; a < KActivationTypes; a++ )
						{
							if( ActivationString == KActivationNames[a] )
							{
								Activation = (EEffectActivation)a;
								break;
							}
						}

					}


					pEffectGroup = pSubDataGroup->GetSubDataGroup( "DURATION" );

					if( pEffectGroup != NULL )
					{
						std::string DurationString = StringUpper( pEffectGroup->Item( 0 ) );
						if( DurationString == "INSTANT" )
						{
							Duration = KDurationInstant;
						}
						else if( DurationString == "ALWAYS" )
						{
							Duration = KDurationAlways;
						}
						else
						{
							Duration = (int32)atoi( DurationString.c_str() );
						}
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE" );

					if( pEffectGroup != NULL )
					{
						Value = (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() );
					}

					CEffect* pAttackEffect = new CEffect( Type,
													kTrue,
													Activation,
													Duration,
													Value,
													0 );


					uint32 SFXItems = pSubDataGroup->SubDataGroups( "SFX" );
					for( uint32 s = 0; s < SFXItems; s++ )
					{
						pEffectGroup = pSubDataGroup->GetSubDataGroup( "SFX", s );

						std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
						ESFXType Type( KSFXStationaryTarget );
						for( uint32 a = 0; a < KSFXTypes; a++ )
						{
							if( TypeString == KSFXTypeName[a] )
							{
								Type = (ESFXType)a;
								break;
							}
						}
						float32 Duration = (float32)atof( pEffectGroup->Item( 1 ).c_str() );
						bool Looping = atoi( pEffectGroup->Item( 2 ).c_str() ) == 1;
						bool Exclusive = kFalse;
						if( pEffectGroup->Items() >= 7 )
						{
							Exclusive = atoi( pEffectGroup->Item( 6 ).c_str() ) == 1;
						}
						pAttackEffect->AddSFX( Type, 
										pEffectGroup->Item( 5 ),
										Duration,
										Looping,
										(float32)atof( pEffectGroup->Item( 3 ).c_str() ),
										(float32)atof( pEffectGroup->Item( 4 ).c_str() ),
										Exclusive,
										kFalse );

					}
					pEffectGroup = pSubDataGroup->GetSubDataGroup( "DAMAGE_TYPE" );

					if( pEffectGroup != NULL )
					{
						EDamageType DamageType( KDamageSlashing );
						std::string TypeString = StringUpper( pEffectGroup->Item( 0 ) );
						for( uint32 a = 0; a < KDamageTypes; a++ )
						{
							if( TypeString == KDamageNames[a] )
							{
								DamageType = (EDamageType)a;
								break;
							}
						}
						pAttackEffect->SetDamageType( DamageType );
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE2" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValue2( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
						if( pAttackEffect->Value2() != pAttackEffect->Value() )
						{
							pAttackEffect->SetRangedValue( kTrue );
						}
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE2_BONUS" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValue2Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE2_BONUS_PERCENT" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValue2BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE3" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValue3( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE3_BONUS" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValue3Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE3_BONUS_PERCENT" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValue3BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE_BONUS" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValueBonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "VALUE_BONUS_PERCENT" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetValueBonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "DURATION_BONUS" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetDurationBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "DURATION_BONUS_PERCENT" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetDurationBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					} 

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetChanceOfSuccess( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "MESSAGE" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetMessage( pEffectGroup->Item( 0 ) );
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "NAME" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetName( pEffectGroup->Item( 0 ) );
					}

					pEffectGroup = pSubDataGroup->GetSubDataGroup( "EXCLUSIVE" );

					if( pEffectGroup != NULL )
					{
						pAttackEffect->SetExclusive( atoi( pEffectGroup->Item( 0 ).c_str() ) == 1 );
					}


					pAttackDescription->AddNewEffect( pAttackEffect );


				}

			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TACTICS" );
		if( pSubGroup != NULL )
		{
			std::string Tactics = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KTactics; i++ )
			{
				if( Tactics == KTacticNames[i] )
				{
					pDescription->SetTactics( (ETactic)i );
					break;
				}
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MINIONS_COLLIDE" );
		if( pSubGroup != NULL )
		{
			pDescription->SetMinionsCollideable( (int32)atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
		}
		pSubGroup = pMasterGroup->GetSubDataGroup( "DUAL_WIELDING" );
		if( pSubGroup != NULL )
		{
			pDescription->SetDualWielding( (int32)atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
		}
		pSubGroup = pMasterGroup->GetSubDataGroup( "MATCH_SPEED" );
		if( pSubGroup != NULL )
		{
			pDescription->SetCanMatchSpeed( (int32)atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
		}
		pSubGroup = pMasterGroup->GetSubDataGroup( "BLOOD_PARTICLE" );
		if( pSubGroup != NULL )
		{
			pDescription->SetBloodParticle( pSubGroup->Item( 0 ) );
		}
		uint32 Idles = pMasterGroup->SubDataGroups( "UNIQUE_IDLE" );
		for( i = 0; i < Idles; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "UNIQUE_IDLE", i );
			pDescription->AddUniqueIdle( pSubGroup->Item( 0 ) );
		}

		uint32 Spells = pMasterGroup->SubDataGroups( "SPELL" );
		for( i = 0; i < Spells; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "SPELL", i );
			pDescription->AddSpell( pSubGroup->Item( 0 ) );
		}

		uint32 Sounds = pMasterGroup->SubDataGroups( "DEATH_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "DEATH_SOUND", i );
			pDescription->AddDieSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "GREET_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "GREET_SOUND", i );
			pDescription->AddGreetSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "STEP_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "STEP_SOUND", i );
			pDescription->AddStepSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "IDLE_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "IDLE_SOUND", i );
			pDescription->AddIdleSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "UNIQUE_IDLE_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "UNIQUE_IDLE_SOUND", i );
			pDescription->AddUniqueIdleSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "ROAR_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "ROAR_SOUND", i );
			pDescription->AddRoarSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "ARMED_ATTACK_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "ARMED_ATTACK_SOUND", i );
			pDescription->AddArmedAttackSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "ATTACK_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "ATTACK_SOUND", i );
			pDescription->AddAttackSound( pSubGroup->Item( 0 ), pSubGroup->Item( 1 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "STRIKE_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "STRIKE_SOUND", i );
			pDescription->AddStrikeSound( pSubGroup->Item( 0 ), pSubGroup->Item( 1 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "MISS_SOUND" );
		for( i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "MISS_SOUND", i );
			pDescription->AddMissSound( pSubGroup->Item( 0 ), pSubGroup->Item( 1 ) );
		}

		// non-standard effects that may be applied
		pSubGroup = pMasterGroup->GetSubDataGroup( "LIGHT" );
		if( pSubGroup != NULL )
		{
			std::string LightPath =  pSubGroup->Item( 0 );
			float32 Radius = (float32)atof( pSubGroup->Item( 1 ).c_str() );
			ELightPoolType LightType( KLightFlicker );
			if( StringUpper( pSubGroup->Item( 2 ) ) == "PULSE" )
			{
				LightType = KLightPulse;
			}
			pDescription->SetLightPool( LightPath, Radius, LightType );
			
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ANIMATION_PATH" );
		if( pSubGroup != NULL )
		{
			pDescription->SetAnimationPath( AssembleAbsolutePath( GetLocalPath(), pSubGroup->Item( 0 ) ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MINION" );
		if( pSubGroup != NULL )
		{
			pDescription->AddMinion( pSubGroup->Item( 0 ),
									(int32)atoi( pSubGroup->Item( 1 ).c_str() ),
									(int32)atoi( pSubGroup->Item( 2 ).c_str() ) );
		}

		int32 FamilyIndex = FindFamily( Family );
		if( FamilyIndex == -1 )
		{
			FamilyIndex = AddFamily( Family );
		}
		m_pCharacterDescription[FamilyIndex].push_back( pDescription );

		m_pBaseCharacterDescription.push_back( pDescription );

	}

	DELETE_SAFELY( pDataFile );

} // CCharacterTemplate::ParseTemplate()

int32 CCharacterTemplate::FindFamily( const std::string& FamilyName )	// family name to find
{
	for( uint32 i = 0; i < m_CharacterFamily.size(); i++ )
	{
		if( m_CharacterFamily[i] == FamilyName )
		{
			return i;
		}
	}
	return -1;
} // CCharacterTemplate::FindFamily()

int32 CCharacterTemplate::AddFamily( const std::string& FamilyName )	// family name to add
{
	m_CharacterFamily.push_back( FamilyName );
	m_pCharacterDescription.resize( m_CharacterFamily.size() );
	return m_CharacterFamily.size() - 1;
} // CCharacterTemplate::AddFamily()

CCharacterDescription* CCharacterTemplate::CharacterDescription( std::string Name )	// name of character to retriev
{	
	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_pCharacterDescription.size(); i++ )
	{
		for( uint32 j = 0; j < m_pCharacterDescription[i].size(); j++ )
		{
			if( StringUpper( m_pCharacterDescription[i][j]->Name() ) == Name )
			{
				return m_pCharacterDescription[i][j];
			}
		}
	}

	return NULL;
} // CCharacter::CharacterDescription()