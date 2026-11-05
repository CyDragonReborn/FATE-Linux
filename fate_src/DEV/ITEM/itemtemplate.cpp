/*
	itemtemplate.cpp

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

#include "effect.h"

#include "itemtemplate.h"

CItemTemplate::CItemTemplate( const std::string& Path ) 	// path to template
{
	ParseTemplate( Path );
	ParseTemplate( Path, KRankElite );
	ParseTemplate( Path, KRankLegendary );
} // CItemTemplate::CItemTemplate()

CItemTemplate::~CItemTemplate( void )
{
	for( uint32 i = 0; i < m_pItemDescription.size(); i++ )
	{
		DELETE_SAFELY( m_pItemDescription[i] );
	}
	m_pItemDescription.clear();

	// the rest do not have to be deleted, as they were pointers
	// to objects in the base item description list
	m_pContainers.clear();
	m_pTakeableItems.clear();
	for( int32 i = 0; i < KItemTypes; i++ )
	{
		m_pItemsByType[i].clear();
	}
	
	for( int32 i = 0; i < KItemCategories; i++ )
	{
		m_pItemsByCategory[i].clear();
	}
} // CItemTemplate::~CItemTemplate()

CItemDescription* CItemTemplate::ItemDescriptionEnterableName( std::string Name ) // name of item to retrieve
{

	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_pItemDescription.size(); i++ )
	{
		if( StringUpper( m_pItemDescription[i]->EnterableName() ) == Name )
		{
			return m_pItemDescription[i];
		}
	}
	return NULL;
} // CItemTemplate::ItemDescriptionEnterableName()

CItemDescription* CItemTemplate::ItemDescription( std::string Name ) // name of item to retrieve
{

	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_pItemDescription.size(); i++ )
	{
		if( StringUpper( m_pItemDescription[i]->Name() ) == Name )
		{
			return m_pItemDescription[i];
		}
	}
	return NULL;
} // CItemTemplate::ItemDescription()

void CItemTemplate::ParseTemplate( const std::string& Path,		// path to template
								   ERank Rank )					// rank to convert items to
{
	std::string BasePath = GetLocalPath();
	CDataFile* pDataFile = new CDataFile( Path );


	uint32 Items = pDataFile->DataGroups( "ITEM" );

	for( uint32 w = 0; w < Items; w++ )
	{
		std::string			ModelPath( "" );
		std::string			CollisionPath( "" );

		std::string			Name( "" );

		uint32				GoldValue( 0 );

		EItemType			ItemType( KItemGeneric );

		bool				Shadow( kTrue );

		bool				Collideable( kFalse );

		float32				ScaleVariation( 0 );

		bool				Takeable( kTrue );

		bool				FliesWhenDropped( kTrue );

		bool				Destructible( kFalse );

		// parse tile widths
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "ITEM", w );


		CDataGroup* pSubGroup = pMasterGroup->GetSubDataGroup( "TYPE" );
		if( pSubGroup != NULL )
		{
			std::string TypeVal = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KItemTypes; i++ )
			{
				if( KItemTypeName[i] == TypeVal )
				{
					ItemType = (EItemType)i;
				}
			}
		}

		// we skip any non-arms items when making high-rank items
		if( KItemCategory[ItemType] != KCategoryWeapon &&
			KItemCategory[ItemType] != KCategoryArmor &&
			Rank > KRankNormal )
		{
			continue;
		}

		if( Rank > KRankNormal )
		{
			// these low level items aren't worth eliting
			pSubGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_DEPTH" );
			if( pSubGroup != NULL )
			{
				int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
				if( Depth < 12 )
				{
					continue;
				}
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "NAME" );
		if( pSubGroup != NULL )
		{
			if(!CGame::GetLocaleString().compare("es")||!CGame::GetLocaleString().compare("it"))  // TDC 2008-02-26 Craig Miles
			{
				Name =  pSubGroup->Item( 0 ) + KRankName[Rank];
			}
			else
			{
				Name = KRankName[Rank] + pSubGroup->Item( 0 );
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MODEL" );
		if( pSubGroup != NULL )
		{
			ModelPath = pSubGroup->Item( 0 );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "COLLISION" );
		if( pSubGroup != NULL )
		{
			CollisionPath = pSubGroup->Item( 0 );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "COLLIDEABLE" );
		if( pSubGroup != NULL )
		{
			Collideable = atoi( pSubGroup->Item( 0 ).c_str() ) == 1;
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "VALUE" );
		if( pSubGroup != NULL )
		{
			GoldValue = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			GoldValue = (int32)( (float32)GoldValue * KRankPriceMultiplier[Rank] );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "SHADOW" );
		if( pSubGroup != NULL )
		{
			Shadow = atoi( pSubGroup->Item( 0 ).c_str() ) == 1;
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TAKEABLE" );
		if( pSubGroup != NULL )
		{
			Takeable = atoi( pSubGroup->Item( 0 ).c_str() ) == 1;
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "DESTRUCTIBLE" );
		if( pSubGroup != NULL )
		{
			Destructible = atoi( pSubGroup->Item( 0 ).c_str() ) == 1;
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "FLIES_WHEN_DROPPED" );
		if( pSubGroup != NULL )
		{
			FliesWhenDropped = atoi( pSubGroup->Item( 0 ).c_str() ) == 1;
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "SCALE_VARIATION" );
		if( pSubGroup != NULL )
		{
			ScaleVariation = (float32)atof( pSubGroup->Item( 0 ).c_str() );
		}

		CItemDescription* pItem = new CItemDescription( ModelPath,
														CollisionPath,
														Name,
														ItemType,
														GoldValue,
														Collideable,
														Takeable,
														Destructible,
														FliesWhenDropped,
														Shadow,
														ScaleVariation );

		m_pItemDescription.push_back( pItem );

		m_pItemsByType[ItemType].push_back( pItem );

		m_pItemsByCategory[KItemCategory[ItemType]].push_back( pItem );

		if( KItemCategory[ItemType] == KCategoryContainer )
		{
			m_pContainers.push_back( pItem );
		}
		else if( Takeable )
		{
			m_pTakeableItems.push_back( pItem );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TARGET" );

		if( pSubGroup != NULL )
		{
			std::string TargetTypeString = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 a = 0; a < KTargets; a++ )
			{
				if( TargetTypeString == KTargetNames[a] )
				{
					pItem->SetTarget( (ETarget)a );
					break;
				}
			}
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "OPEN_MODEL" );
		if( pSubGroup != NULL )
		{
			pItem->SetOpenModelPath( pSubGroup->Item( 0 ) );
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "CLOSE_MODEL" );
		if( pSubGroup != NULL )
		{
			pItem->SetCloseModelPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ARTIFACT" );
		if( pSubGroup != NULL )
		{
			pItem->SetIsArtifact( atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
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



			float32 PriceMultiplier( 0 );
			if( pItem->IsArtifact() )
			{
				if( Type >= KEffectPercentStrength )
				{


					if( sgn( (float32)Value  ) != sgn( (float32)KEffectMaximum[Type] ) )
					{
						PriceMultiplier = fabs( Value / KEffectMinimum[Type] ) * 5;
					}
					else
					{
						PriceMultiplier = fabs( Value / KEffectMaximum[Type] ) * 40;

					}

				}		
				else
				{
					if( sgn( (float32)Value  ) != sgn( (float32)KEffectMaximum[Type] ) )
					{
						PriceMultiplier = fabs( Value / KEffectMinimum[Type] ) * 8;
					}
					else
					{
						PriceMultiplier = fabs(  Value / KEffectMaximum[Type] ) * 60;
					}
				}	
			}

			CEffect* pItemEffect = new CEffect( Type,
											Positive,
											Activation,
											Duration,
											Value,
											PriceMultiplier );

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
				pItemEffect->SetDamageType( DamageType );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValue2( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
				if( pItemEffect->Value2() != pItemEffect->Value() )
				{
					pItemEffect->SetRangedValue( kTrue );
				}
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValue2Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE2_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValue2BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValueBonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValueBonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValue3( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValue3Bonus( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "VALUE3_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetValue3BonusPercent( (float32)atof( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetDurationBonus( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "DURATION_BONUS_PERCENT" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetDurationBonusPercent( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			} 

			pEffectGroup = pSubGroup->GetSubDataGroup( "CHANCE_OF_SUCCESS" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetChanceOfSuccess( (int32)atoi( StringUpper( pEffectGroup->Item( 0 ) ).c_str() ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "MESSAGE" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetMessage( pEffectGroup->Item( 0 ) );
			}


			pEffectGroup = pSubGroup->GetSubDataGroup( "NAME" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetName( pEffectGroup->Item( 0 ) );
			}

			pEffectGroup = pSubGroup->GetSubDataGroup( "EXCLUSIVE" );

			if( pEffectGroup != NULL )
			{
				pItemEffect->SetExclusive( atoi( pEffectGroup->Item( 0 ).c_str() ) == 1 );
			}


			pItem->AddEffect( pItemEffect );


		}

		uint32 Bonuses = pMasterGroup->SubDataGroups( "DAMAGE_BONUS" );

		for( uint32 b = 0; b < Bonuses; b++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "DAMAGE_BONUS", b );
			EDamageType DamageType( KDamageFire );
			std::string TypeString = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 a = 0; a < KDamageTypes; a++ )
			{
				if( TypeString == KDamageNames[a] )
				{
					DamageType = (EDamageType)a;
					break;
				}
			}
			uint32 Bonus = (int32)atoi( pSubGroup->Item( 1 ).c_str() );
			Bonus = (int32)( (float32)Bonus * KRankPowerMultiplier[Rank] );

			pItem->AddDamageBonus( DamageType, Bonus );
		}

		uint32 SFXItems = pMasterGroup->SubDataGroups( "SFX" );
		for( uint32 s = 0; s < SFXItems; s++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "SFX", s );

			std::string TypeString = StringUpper( pSubGroup->Item( 0 ) );
			ESFXType Type( KSFXStationaryTarget );
			for( uint32 a = 0; a < KSFXTypes; a++ )
			{
				if( TypeString == KSFXTypeName[a] )
				{
					Type = (ESFXType)a;
					break;
				}
			}
			float32 Duration = (float32)atof( pSubGroup->Item( 1 ).c_str() );
			bool Looping = atoi( pSubGroup->Item( 2 ).c_str() ) == 1;

			bool Exclusive = kFalse;
			if( pSubGroup->Items() >= 7 )
			{
				Exclusive = atoi( pSubGroup->Item( 6 ).c_str() ) == 1;
			}
			bool RespectBonuses = kFalse;
			if( pSubGroup->Items() >= 8 )
			{
				RespectBonuses = atoi( pSubGroup->Item( 7 ).c_str() ) == 1;
			}

			pItem->AddSFX( Type, 
						   pSubGroup->Item( 5 ),
						   Duration,
						   Looping,
						   (float32)atof( pSubGroup->Item( 3 ).c_str() ),
						   (float32)atof( pSubGroup->Item( 4 ).c_str() ),
						   Exclusive,
						   RespectBonuses );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ANIMATED_MODEL" );
		if( pSubGroup != NULL )
		{
			pItem->SetAnimatedModelPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "DESCRIPTION" );
		if( pSubGroup != NULL )
		{
			pItem->SetUseDescription( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ARMOR" );
		if( pSubGroup != NULL )
		{
			uint32 MinimumArmor = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			uint32 MaximumArmor = (int32)atoi( pSubGroup->Item( 1 ).c_str() );
			MinimumArmor = (int32)( (float32)MinimumArmor * KRankPowerMultiplier[Rank] );
			MaximumArmor = (int32)( (float32)MaximumArmor * KRankPowerMultiplier[Rank] );
			pItem->SetArmorBonus( MinimumArmor, MaximumArmor );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "DAMAGE" );
		if( pSubGroup != NULL )
		{
			uint32 MinimumDamage = (uint32)atoi( pSubGroup->Item( 0 ).c_str() );
			uint32 MaximumDamage = (uint32)atoi( pSubGroup->Item( 1 ).c_str() );
			MinimumDamage = (int32)( (float32)MinimumDamage * KRankPowerMultiplier[Rank] );
			MaximumDamage = (int32)( (float32)MaximumDamage * KRankPowerMultiplier[Rank] );
			pItem->SetDamage( MinimumDamage, MaximumDamage );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "COLLISION_MASK_RADIUS" );
		if( pSubGroup != NULL )
		{
			pItem->SetCollisionMaskRadius( (uint32)atoi( pSubGroup->Item( 0 ).c_str() ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TOHITBONUS" );
		if( pSubGroup != NULL )
		{
			pItem->SetToHitBonus( (uint32)( atof( pSubGroup->Item( 0 ).c_str() ) * KRankPowerMultiplier[Rank] ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "SOCKETS" );
		if( pSubGroup != NULL )
		{
			pItem->SetSockets( (uint32)( atof( pSubGroup->Item( 0 ).c_str() ) * KRankPowerMultiplier[Rank] ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "SPEED" );
		if( pSubGroup != NULL )
		{
			EAttackSpeed AttackSpeed = KAttackNormal;
			std::string Speed = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KAttackSpeeds; i++ )
			{
				if( Speed == StringUpper( KAttackSpeedName[i] ) )
				{
					AttackSpeed = (EAttackSpeed)i;
				}
			}
			pItem->SetSpeed( AttackSpeed );
		}

		uint32 Requirements = pMasterGroup->SubDataGroups( "REQUIRES" );

		for( uint32 Requirement = 0; Requirement < Requirements; Requirement++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "REQUIRES", Requirement );
			EStatistic Statistic = KStatStrength;
			std::string Stat = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KStatistics; i++ )
			{
				if( Stat == StringUpper( KStatisticName[i] ) )
				{
					Statistic = (EStatistic)i;
				}
			}
			pItem->AddRequirement( Statistic, (uint32)( atof( pSubGroup->Item( 1 ).c_str() ) * KRankRequirementMultiplier[Rank] ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "GRADE", Requirements );
		if( pSubGroup != NULL )
		{
			EGrade Grade = KGradeNormal;
			std::string GradeName = StringUpper( pSubGroup->Item( 0 ) );
			for( uint32 i = 0; i < KGrades; i++ )
			{
				if( GradeName == StringUpper( KGradeName[i] ) )
				{
					Grade = (EGrade)i;
				}
			}
			pItem->SetGrade( Grade );
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
			pItem->SetLightPool( LightPath, Radius, LightType );
			
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ICON" );
		if( pSubGroup != NULL )
		{
			std::string IconPath( pSubGroup->Item( 0 ) );
			std::string AlphaPath( "" );
			if( pSubGroup->Items() > 1 )
			{
				AlphaPath = pSubGroup->Item( 1 );
			}
			pItem->SetIcon( IconPath, AlphaPath );
			if( pSubGroup->Items() > 3 )
			{
				uint32 Width = (uint32)atoi( pSubGroup->Item( 2 ).c_str() );
				uint32 Height = (uint32)atoi( pSubGroup->Item( 3 ).c_str() );
				pItem->SetIconWidth( Width );
				pItem->SetIconHeight( Height );
			}
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ACTIVATE_RADIUS" );
		if( pSubGroup != NULL )
		{
			float32 Radius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
			pItem->SetActivationRadius( Radius );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "DEACTIVATE_RADIUS" );
		if( pSubGroup != NULL )
		{
			float32 Radius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
			pItem->SetDeactivationRadius( Radius );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "ATTACK_RADIUS" );
		if( pSubGroup != NULL )
		{
			float32 Radius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
			pItem->SetAttackRadius( Radius );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "STRIKE_RADIUS" );
		if( pSubGroup != NULL )
		{
			float32 Radius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
			pItem->SetStrikeRadius( Radius );
		}

		uint32 Sounds = pMasterGroup->SubDataGroups( "ACTIVATE_SOUND" );
		for( uint32 i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "ACTIVATE_SOUND", i );
			pItem->AddActivationSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "DEACTIVATE_SOUND" );
		for( int32 i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "DEACTIVATE_SOUND", i );
			pItem->AddDeactivationSound( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "TAKE_SOUND" );
		if( pSubGroup != NULL )
		{
			pItem->SetTakeSoundPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "DROP_SOUND" );
		if( pSubGroup != NULL )
		{
			pItem->SetDropSoundPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "LAND_SOUND" );
		if( pSubGroup != NULL )
		{
			pItem->SetLandSoundPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "USE_SOUND" );
		if( pSubGroup != NULL )
		{
			pItem->SetUseSoundPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "AMBIENT_SOUND" );
		if( pSubGroup != NULL )
		{
			pItem->SetAmbientSoundPath( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "BASE_SOUND_RADIUS" );
		if( pSubGroup != NULL )
		{
			float32 Radius = (float32)atof( pSubGroup->Item( 0 ).c_str() );
			pItem->SetBaseSoundRadius( Radius );
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "BASE_SOUND_VOLUME" );
		if( pSubGroup != NULL )
		{
			uint32 Volume = (uint32)atoi( pSubGroup->Item( 0 ).c_str() );
			pItem->SetBaseSoundVolume( Volume );
		}

		Sounds = pMasterGroup->SubDataGroups( "ATTACK_SOUND" );
		for( int32 i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "ATTACK_SOUND", i );
			pItem->AddAttackSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "STRIKE_SOUND" );
		for( int32 i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "STRIKE_SOUND", i );
			pItem->AddStrikeSound( pSubGroup->Item( 0 ) );
		}

		Sounds = pMasterGroup->SubDataGroups( "MISS_SOUND" );
		for( int32 i = 0; i < Sounds; i++ )
		{
			pSubGroup = pMasterGroup->GetSubDataGroup( "MISS_SOUND", i );
			pItem->AddMissSound( pSubGroup->Item( 0 ) );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "USES" );
		if( pSubGroup != NULL )
		{
			int32 Uses = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			pItem->SetUses( Uses );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "USEABLE" );
		if( pSubGroup != NULL )
		{
			bool Useable = atoi( pSubGroup->Item( 0 ).c_str() ) == 1;
			pItem->SetUseable( Useable );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "PURCHASEABLE" );
		if( pSubGroup != NULL )
		{
			pItem->SetPurchaseable( atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "UNIQUE" );
		if( pSubGroup != NULL )
		{
			pItem->SetIsUnique( atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "IDENTIFIED" );
		if( pSubGroup != NULL )
		{
			pItem->SetIdentified( atoi( pSubGroup->Item( 0 ).c_str() ) == 1 );
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "RARITY" );
		if( pSubGroup != NULL )
		{
			int32 Rarity = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			if( Rarity < 1000 )
			{

				Rarity = (int32)( (float32)Rarity * KRankItemRarityMultiplier[Rank] );
				if( Rarity >= 1000 )
				{
					Rarity = 999;
				}
			}
			pItem->SetRarity( Rarity );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "FISHING_RARITY" );
		if( pSubGroup != NULL )
		{
			int32 Rarity = (int32)atoi( pSubGroup->Item( 0 ).c_str() );
			if( Rarity < 1000 )
			{
				Rarity = (int32)( (float32)Rarity * KRankPowerMultiplier[Rank] );
				Rarity = (int32)( (float32)Rarity * .985f );
				if( Rarity >= 1000 )
				{
					Rarity = 999;
				}
			}
			pItem->SetFishingRarity( Rarity );
		}


		pSubGroup = pMasterGroup->GetSubDataGroup( "MINIMUM_DEPTH" );
		if( pSubGroup != NULL )
		{
			int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
			Depth += KRankLevelOffset[Rank];
			pItem->SetMinimumDepth( Depth );
			pItem->SetMinimumFishingDepth( Depth );

			if( Rank != KRankNormal )
			{

				int32 Renown;
				if( Rank == KRankLegendary )
				{
					Renown = ( Depth - 15 ) / 7;
					if( Renown > 10 )
					{
						Renown = 10;
					}
					if( Renown < 0 )
					{
						Renown = 0;
					}
					Renown = KFameLevels - 12 + Renown;
					if( Renown >= KFameLevels )
					{
						Renown = KFameLevels - 1;
					}
				}
				else
				{
					Renown = ( Depth - 15 ) / 10;
					if( Renown > 10 )
					{
						Renown = 10;
					}
					Renown = KFameLevels - 17 + Renown;
					if( Renown < 0 )
					{
						Renown = 0;
					}
					if( Renown >= KFameLevels )
					{
						Renown = KFameLevels - 1;
					}
				}
	
				pItem->AddRequirement( KStatRenown, Renown );
			}

		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_DEPTH" );
		if( pSubGroup != NULL )
		{
			int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
			if( Rank != KRankNormal )
			{
				Depth = 32000;
			}
			pItem->SetMaximumDepth( Depth );
			pItem->SetMaximumFishingDepth( Depth );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MINIMUM_FISHING_DEPTH" );
		if( pSubGroup != NULL )
		{
			int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
			Depth += KRankLevelOffset[Rank];
			pItem->SetMinimumFishingDepth( Depth );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_FISHING_DEPTH" );
		if( pSubGroup != NULL )
		{
			int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
			Depth += KRankLevelOffset[Rank];
			if( Rank != KRankNormal )
			{
				Depth = 32000;
			}
			pItem->SetMaximumFishingDepth( Depth );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MERCHANT_MINIMUM" );
		if( pSubGroup != NULL )
		{
			int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
			Depth += KRankLevelOffset[Rank];
			pItem->SetMerchantMinimum( Depth );
		}

		pSubGroup = pMasterGroup->GetSubDataGroup( "MERCHANT_MAXIMUM" );
		if( pSubGroup != NULL )
		{
			int32 Depth = (int32)atoi( pSubGroup->Item( 0 ).c_str()  );
			Depth += KRankLevelOffset[Rank];
			if( Rank != KRankNormal )
			{
				Depth = 32000;
			}
			pItem->SetMerchantMaximum( Depth );
		}

	}

	DELETE_SAFELY( pDataFile );
} // CItemTemplate::ParseTemplate()
