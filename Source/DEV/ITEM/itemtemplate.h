/*
	Itemtemplate.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _ITEMTEMPLATE_H
#define _ITEMTEMPLATE_H


#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../GAMECLIENT/gamerules.h"

#include "effect.h"


class CDataFile;

// Item description contains item value and property description information

class CItemDescription
{

public:

	CItemDescription( const std::string& ModelPath,					// path to Item model
						   const std::string& CollisionPath,		// path to collision model
						   const std::string& Name,					// name of Item
						   EItemType Type,							// item type
						   int32 Value,								// gold value
						   bool Collideable,						// collideable?
						   bool Takeable,							// takeable?
						   bool Destructible,						// destructible?
						   bool FliesWhenDropped,					// flips in air when dropped?
						   bool Shadow,								// casts shadow?
						   float32 ScaleVariation ) :				// variation in base scale
														m_ModelPath( ModelPath ),
														m_OpenModelPath( "" ),
														m_CloseModelPath( "" ),
														m_CollisionPath( CollisionPath ),
														m_Name( Name ),
														m_Value( Value ),
														m_Category( KItemCategory[Type] ),
														m_Type( Type ),
														m_Collideable( Collideable ),
														m_Shadow( Shadow ),
														m_HasLightPool( kFalse ),
														m_Useable( kFalse ),
														m_Takeable( Takeable ),
														m_Destructible( Destructible ),
														m_ScaleVariation( ScaleVariation ),
														m_FliesWhenDropped( FliesWhenDropped ),
														m_MerchantMinimum( 0 ),
														m_MerchantMaximum( 100 ),
														m_ActivationRadius( 16 ),
														m_DeactivationRadius( 16 ),
														m_BaseSoundRadius( 32 ),
														m_BaseSoundVolume( 255 ),
														m_IconWidth( 48 ),
														m_IconHeight( 48 ),
														m_AttackRadius( 0 ),
														m_StrikeRadius( 0 ),
														m_MinimumArmorBonus( 0 ),
														m_MaximumArmorBonus( 0 ),
														m_MinimumDamage( 0 ),
														m_MaximumDamage( 0 ),
														m_ToHitBonus( 0 ),
														m_Grade( KGradeNormal ),
														m_Speed( KAttackNormal ),
														m_Uses( KUnlimitedUses ),
														m_AmbientSoundPath( "" ),
														m_AnimatedModelPath( "" ),
														m_IconPath( "" ),
														m_IconAlphaPath( "" ),
														m_UseDescription( "" ),
														m_Target( KTargetUser ),
														m_Identified( kFalse ),
														m_Purchaseable( kTrue ),
														m_IsUnique( kFalse ),
														m_IsArtifact( kFalse ),
														m_MinimumDepth( 1 ),
														m_MaximumDepth( 32000 ),
														m_MinimumFishingDepth( 1 ),
														m_MaximumFishingDepth( 32000 ),
														m_Rarity( 1 ),
														m_FishingRarity( 1000 ),
														m_CollisionMaskRadius( 1 ),
														m_Sockets( 0 ),
														m_LightPoolType ( KLightFlicker ),
														m_LightPoolRadius ( 0.0f ),
														m_EnterableName ( "" ),
														m_LightPoolPath ( "" ),
														m_TakeSoundPath ( "" ),
														m_DropSoundPath ( "" ),
														m_LandSoundPath ( "" ),
														m_UseSoundPath ( "" )
	{

		while( m_Name.find( "\\n" ) != -1 )
		{
			int32 Index = m_Name.find( "\\n" );
			m_Name.replace( Index, 2, "\n" );
		}

		m_EnterableName = m_Name;

		// remove any carriage returns so this can wrap properly
		while( m_EnterableName.find( "\n" ) != -1 )
		{
			int32 Index = m_EnterableName.find( "\n" );
			m_EnterableName.replace( Index, 1, " " );
		}

	}

	~CItemDescription( void )
	{
		for( uint32 i = 0; i < m_pEffects.size(); i++ )
		{
			DELETE_SAFELY( m_pEffects[i] );
		}
		m_pEffects.clear();

		for( i = 0; i < m_pSFX.size(); i++ )
		{
			DELETE_SAFELY( m_pSFX[i] );
		}
		m_pSFX.clear();
	}

	
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void	AddDamageBonus( EDamageType Type,	// type of damage
							int32 Value )		// value
						  {	
							  m_DamageBonus.push_back( Type );
							  m_DamageBonusValue.push_back( Value );
						  }

	void	AddActivationSound( const std::string& Path )	
	{	
		m_ActivationSoundPath.push_back( Path );	
	};

	void	AddDeactivationSound( const std::string& Path )	
	{	
		m_DeactivationSoundPath.push_back( Path );	
	};

	void	AddAttackSound( const std::string& Path )	
	{	
		m_AttackSoundPath.push_back( Path );	
	};

	void	AddStrikeSound( const std::string& Path )	
	{	
		m_StrikeSoundPath.push_back( Path );	
	};

	void	AddMissSound( const std::string& Path )		
	{	
		m_MissSoundPath.push_back( Path );		
	};

	void	SetLightPool( const std::string& Path,	// path to light texture
						  float32 Radius,			// radius of light pool
						  ELightPoolType Type )		// flicker or pulse?
	{
		m_HasLightPool = kTrue;
		m_LightPoolRadius = Radius;
		m_LightPoolPath = Path;
		m_LightPoolType = Type;
	}

	void	SetCollisionMaskRadius( int32 Radius )				{	m_CollisionMaskRadius = Radius;	};

	void	SetOpenModelPath( const std::string& Text )			{	m_OpenModelPath = Text;			};

	void	SetCloseModelPath( const std::string& Text )		{	m_CloseModelPath = Text;			};

	void	SetUseDescription( const std::string& Text )		{	m_UseDescription = Text;			};

	void	SetAnimatedModelPath( const std::string& Path )		{	m_AnimatedModelPath = Path;			};

	void	AddSFX( ESFXType Type,
					const std::string& Path,
					float32 Duration,
					bool Looping,
					float32 Speed,
					float32 TurnRate,
					bool Exclusive,
					bool RespectBonuses )					
			{
				m_pSFX.push_back( new CSFXDescription( Type,
													   Path,
													   Duration,
													   Looping,
													   Speed,
													   TurnRate,
													   Exclusive,
													   RespectBonuses ) );
			};
	
	void	SetAmbientSoundPath( const std::string& Path )		{	m_AmbientSoundPath = Path;			};
	void	SetTakeSoundPath( const std::string& Path )			{	m_TakeSoundPath = Path;		};
	void	SetDropSoundPath( const std::string& Path )			{	m_DropSoundPath = Path;		};
	void	SetLandSoundPath( const std::string& Path )			{	m_LandSoundPath = Path;		};
	void	SetUseSoundPath( const std::string& Path )			{	m_UseSoundPath = Path;		};

	void	SetTarget( ETarget Target )							{	m_Target = Target;			};

	void	SetIdentified( bool Identified )					{	m_Identified = Identified;	};

	void	SetActivationRadius( float32 Radius )				{	m_ActivationRadius = Radius;		};
	void	SetDeactivationRadius( float32 Radius )				{	m_DeactivationRadius = Radius;		};

	void	SetBaseSoundRadius( float32 Radius )				{	m_BaseSoundRadius = Radius;			};
	
	void	SetBaseSoundVolume( uint32 Volume )					{	m_BaseSoundVolume = Volume;			};

	void	SetDamage( uint32 MinimumDamage, 
					   uint32 MaximumDamage )
	{
		m_MinimumDamage = MinimumDamage;
		m_MaximumDamage = MaximumDamage;
	}

	void	SetIsUnique( bool Unique )							{	m_IsUnique = Unique;			};

	void	SetIsArtifact( bool Artifact )						{	m_IsArtifact = Artifact;		};

	void	SetPurchaseable( bool Purchaseable )				{	m_Purchaseable = Purchaseable;		};

	void	SetArmorBonus( uint32 Minimum,
						   uint32 Maximum )
			{	
				m_MinimumArmorBonus = Minimum;				
				m_MaximumArmorBonus = Maximum;				
			};

	void	SetAttackRadius( float32 Radius )				{	m_AttackRadius = Radius;	};
	void	SetStrikeRadius( float32 Radius )				{	m_StrikeRadius = Radius;	};

	void	SetIcon( const std::string& IconPath,
					 const std::string& IconAlphaPath )		{	m_IconPath = IconPath; m_IconAlphaPath = IconAlphaPath;		};

	void	SetUses( int32 Uses )					{	m_Uses = Uses;						};

	void	SetUseable( bool Useable )				{	m_Useable = Useable;				};

	void	AddEffect( CEffect* pEffect )			{	m_pEffects.push_back( pEffect );	};

	void	SetSpeed( EAttackSpeed Speed )			{	m_Speed = Speed;					};

	void	SetToHitBonus( int32 Bonus )			{	m_ToHitBonus = Bonus;				};

	void	SetIconWidth( uint32 Width )			{	m_IconWidth = Width;				};
	void	SetIconHeight( uint32 Height )			{	m_IconHeight = Height;				};

	void	AddRequirement( EStatistic Statistic,	// statistic
							uint32 Value )			// requirement value
	{
		m_RequirementStatistic.push_back( Statistic );
		m_RequirementValue.push_back( Value );
	}


	void	SetGrade( EGrade Grade )				{	m_Grade = Grade;				};

	void	SetRarity( int32 Rarity )				{	m_Rarity = Rarity;				};

	void	SetFishingRarity( int32 Rarity )		{	m_FishingRarity = Rarity;				};

	void	SetMinimumDepth( int32 Depth )			{	m_MinimumDepth = Depth;			};

	void	SetMaximumDepth( int32 Depth )			{	m_MaximumDepth = Depth;			};

	void	SetMinimumFishingDepth( int32 Depth )	{	m_MinimumFishingDepth = Depth;			};

	void	SetMaximumFishingDepth( int32 Depth )	{	m_MaximumFishingDepth = Depth;			};

	void	SetSockets( uint32 Sockets )			
			{	
				m_Sockets = Sockets;			
			};


	void	SetMerchantMinimum( int32 Level )		{	m_MerchantMinimum = Level;		};

	void	SetMerchantMaximum( int32 Level )		{	m_MerchantMaximum = Level;		};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	EGrade		Grade( void )					{	return m_Grade;					};

	int32		MerchantMaximum( void )			{	return m_MerchantMaximum;		};

	int32		MerchantMinimum( void )			{	return m_MerchantMinimum;		};

	uint32		Sockets( void )					{	return m_Sockets;				};

	bool		IsUnique( void )				{	return m_IsUnique;				};

	bool		IsArtifact( void )				{	return m_IsArtifact;			};

	bool		Purchaseable( void )			{	return m_Purchaseable;			};

	int32		Rarity( void )					{	return m_Rarity;				};

	int32		FishingRarity( void )			{	return m_FishingRarity;			};

	int32		MinimumDepth( void )			{	return m_MinimumDepth;			};

	int32		MaximumDepth( void )			{	return m_MaximumDepth;			};

	int32		MinimumFishingDepth( void )		{	return m_MinimumFishingDepth;			};

	int32		MaximumFishingDepth( void )		{	return m_MaximumFishingDepth;			};

	int32		CollisionMaskRadius( void )		{	return m_CollisionMaskRadius;	};

	uint32				DamageBonuses( void )				{	return m_DamageBonus.size();			};

	EDamageType			DamageBonusType( uint32 Index )		{	return m_DamageBonus[Index];			};

	int32				DamageBonusValue( uint32 Index )	{	return m_DamageBonusValue[Index];		};

	int32				ToHitBonus( void )					{	return m_ToHitBonus;					};

	ETarget				Target( void )						{	return m_Target;						};

	bool				Identified( void )					{	return m_Identified;					};

	uint32				SFXItems( void )					{	return m_pSFX.size();					};

	CSFXDescription*	SFX( uint32 Index )					{	return m_pSFX[Index];					};

	const std::string&	UseDescription( void )				{	return m_UseDescription;				};

	uint32				Requirements( void )				{	return m_RequirementValue.size();	};

	EStatistic			RequirementStat( uint32 Index )		{	return m_RequirementStatistic[Index];	};

	uint32				RequirementValue( uint32 Index )	{	return m_RequirementValue[Index];		};

	std::vector< EStatistic >&	RequirementStats( void )	{	return m_RequirementStatistic;			};

	std::vector< uint32 >&		RequirementValues( void )	{	return m_RequirementValue;				};

	EAttackSpeed		Speed( void )				{	return m_Speed;						};

	uint32				Effects( void )				{	return m_pEffects.size();			};

	CEffect*			Effect( uint32 Index )		{	return m_pEffects[Index];			};

	int32				Uses( void )				{	return m_Uses;						};

	bool				Useable( void )				{	return m_Useable;					};

	uint32				ActivationSounds( void )	{	return m_ActivationSoundPath.size();	};
	uint32				DeactivationSounds( void )	{	return m_DeactivationSoundPath.size();	};

	const std::string&	ActivationSound( uint32 Index )		{	return m_ActivationSoundPath[Index];	};
	const std::string&	DeactivationSound( uint32 Index )	{	return m_DeactivationSoundPath[Index];	};

	uint32				AttackSounds( void )		{	return m_AttackSoundPath.size();	};
	uint32				StrikeSounds( void )		{	return m_StrikeSoundPath.size();	};
	uint32				MissSounds( void )			{	return m_MissSoundPath.size();		};

	const std::string&	AttackSound( uint32 Index )	{	return m_AttackSoundPath[Index];	};
	const std::string&	StrikeSound( uint32 Index )	{	return m_StrikeSoundPath[Index];	};
	const std::string&	MissSound( uint32 Index )	{	return m_MissSoundPath[Index];		};

	float32				AttackRadius( void )	{	return m_AttackRadius;		};
	float32				StrikeRadius( void )	{	return m_StrikeRadius;		};

	EItemType			Type( void )			{	return m_Type;				};

	EItemCategory		Category( void )		{	return m_Category;			};

	bool				HasLightPool( void )	{	return m_HasLightPool;		};

	float32				LightPoolRadius( void )	{	return m_LightPoolRadius;	};

	ELightPoolType		LightPoolType( void )	{	return m_LightPoolType;		};

	const std::string&	LightPoolPath( void )	{	return m_LightPoolPath;		};

	const std::string&	ModelPath( void )		{	return m_ModelPath;			};
	const std::string&	OpenModelPath( void )	{	return m_OpenModelPath;			};
	const std::string&	CloseModelPath( void )	{	return m_CloseModelPath;			};
	const std::string&	CollisionPath( void )	{	return m_CollisionPath;		};

	bool				Shadow( void )			{	return m_Shadow;		};

	bool				Collideable( void )		{	return m_Collideable;	};

	bool				Takeable( void )		{	return m_Takeable;		};

	bool				Destructible( void )	{	return m_Destructible;	};

	bool				FliesWhenDropped( void ){	return m_FliesWhenDropped;		};

	const std::string&	Name( void )				{	return m_Name;				};

	const std::string&	EnterableName( void )		{	return m_EnterableName;		};

	const std::string&	AnimatedModelPath( void )	{	return m_AnimatedModelPath;				};

	int32				Value( void )				{	return m_Value;			};

	uint32				MinimumArmorBonus( void )	{	return m_MinimumArmorBonus;	};
	uint32				MaximumArmorBonus( void )	{	return m_MaximumArmorBonus;	};

	uint32				MinimumDamage( void )		{	return m_MinimumDamage;		};
	uint32				MaximumDamage( void )		{	return m_MaximumDamage;		};

	float32				ActivationRadius( void )	{	return m_ActivationRadius;	};
	float32				DeactivationRadius( void )	{	return m_DeactivationRadius;	};

	float32				ScaleVariation( void )	{	return m_ScaleVariation;	};

	const std::string&	AmbientSoundPath( void )		{	return m_AmbientSoundPath;				};
	const std::string&	TakeSoundPath( void )			{	return m_TakeSoundPath;			};
	const std::string&	DropSoundPath( void )			{	return m_DropSoundPath;			};
	const std::string&	LandSoundPath( void )			{	return m_LandSoundPath;			};
	const std::string&	UseSoundPath( void )			{	return m_UseSoundPath;			};

	const std::string&	IconPath( void )				{	return m_IconPath;				};
	const std::string&	IconAlphaPath( void )			{	return m_IconAlphaPath;			};

	uint32				IconWidth( void )				{	return m_IconWidth;				};
	uint32				IconHeight( void )				{	return m_IconHeight;			};

	bool				HasIcon( void )					{	return m_IconPath.length() > 0;	};


	float32				BaseSoundRadius( void )		{	return m_BaseSoundRadius;	};
	uint32				BaseSoundVolume( void )		{	return m_BaseSoundVolume;	};


private:

	EGrade				m_Grade;

	std::string			m_ModelPath;
	std::string			m_OpenModelPath;
	std::string			m_CloseModelPath;
	std::string			m_CollisionPath;

	std::string			m_Name;

	std::string			m_EnterableName;

	std::string			m_UseDescription;

	std::string			m_AnimatedModelPath;

	bool				m_IsUnique;
	bool				m_IsArtifact;

	bool				m_Collideable;

	bool				m_Useable;

	bool				m_Purchaseable;

	float32				m_ScaleVariation;

	bool				m_Shadow;

	EItemType			m_Type;

	EItemCategory		m_Category;

	int32				m_CollisionMaskRadius;

	int32				m_ToHitBonus;

	int32				m_Value;

	int32				m_Uses;

	uint32				m_MinimumArmorBonus;
	uint32				m_MaximumArmorBonus;

	uint32				m_MinimumDamage;
	uint32				m_MaximumDamage;

	int32				m_MerchantMinimum;

	int32				m_MerchantMaximum;

	bool				m_Takeable;

	bool				m_Destructible;

	bool				m_Identified;

	ETarget				m_Target;

	bool				m_FliesWhenDropped;

	std::string			m_LightPoolPath;
	ELightPoolType		m_LightPoolType;	
	float32				m_LightPoolRadius;
	bool				m_HasLightPool;

	float32				m_ActivationRadius;
	float32				m_DeactivationRadius;

	EAttackSpeed		m_Speed;

	float32				m_AttackRadius;
	float32				m_StrikeRadius;

	std::string			m_AmbientSoundPath;
	std::string			m_TakeSoundPath;
	std::string			m_DropSoundPath;
	std::string			m_LandSoundPath;
	std::string			m_UseSoundPath;

	float32				m_BaseSoundRadius;
	uint32				m_BaseSoundVolume;

	std::vector< std::string >		m_ActivationSoundPath;
	std::vector< std::string >		m_DeactivationSoundPath;

	std::vector< std::string >		m_AttackSoundPath;
	std::vector< std::string >		m_StrikeSoundPath;
	std::vector< std::string >		m_MissSoundPath;

	std::vector< CEffect* >			m_pEffects;

	std::vector< EStatistic >		m_RequirementStatistic;
	std::vector< uint32 >			m_RequirementValue;

	std::string			m_IconPath;
	std::string			m_IconAlphaPath;

	uint32				m_IconWidth;
	uint32				m_IconHeight;

	std::vector< CSFXDescription* >		m_pSFX;

	std::vector< EDamageType >		m_DamageBonus;
	std::vector< int32 >			m_DamageBonusValue;

	int32				m_Rarity;

	int32				m_FishingRarity;

	int32				m_MinimumDepth;
	int32				m_MaximumDepth;

	int32				m_MinimumFishingDepth;
	int32				m_MaximumFishingDepth;

	uint32				m_Sockets;
};


class CItemInstance
{
public:
	CItemInstance( CItemDescription& pDescription ) :		// Item this is an instance of
														m_pDescription( pDescription ),
														m_Value( pDescription.Value() ),
														m_Angle ( RandomBetween( 0, 360 )),
														m_Scale ( 1.0f + RandomBetween( 0, m_pDescription.ScaleVariation()))

	{
	}

	~CItemInstance( void )
	{
	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void				SetValue( int32 Value )	{	m_Value = Value;		};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	int32				Value( void )		{	return m_Value;			};

	float32				Angle( void )		{	return m_Angle;			};
	float32				Scale( void )		{	return m_Scale;			};

	CItemDescription&	Description( void )	{	return m_pDescription;		};

private :

	CItemDescription&	m_pDescription;

	float32			m_Angle;
	float32			m_Scale;

	int32			m_Value;
};


class CItemTemplate
{

public:

	CItemTemplate( const std::string& Path );		// path to template
	~CItemTemplate( void );
	
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////


	uint32					Items( void )							{	return m_pItemDescription.size();			};

	uint32					Items( EItemType Type )					{	return m_pItemsByType[Type].size();			};

	uint32					Items( EItemCategory Category )			{	return m_pItemsByCategory[Category].size();	};

	uint32					TakeableItems( void )					{	return m_pTakeableItems.size();		};

	uint32					ContainerItems( void )					{	return m_pContainers.size();		};

	CItemDescription*		ItemDescription( std::string Name );	// name of item to retrieve

	CItemDescription*		ItemDescriptionEnterableName( std::string Name );	// name of item to retrieve

	CItemDescription*		ItemDescription( uint32 Index )			{	return m_pItemDescription[Index];	};

	CItemDescription*		ItemDescription( EItemType Type, 
											 uint32 Index )			{	return m_pItemsByType[Type][Index];	};

	CItemDescription*		ItemDescription( EItemCategory Category, 
											 uint32 Index )			{	return m_pItemsByCategory[Category][Index];	};

	CItemDescription*		TakeableItemDescription( uint32 Index )	{	return m_pTakeableItems[Index];		};

	CItemDescription*		ContainerItemDescription( uint32 Index ){	return m_pContainers[Index];		};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

private:

	void	ParseTemplate( const std::string& Path,			// path to template
						   ERank Rank = KRankNormal );		// rank to convert items to


	std::vector< CItemDescription* >	m_pItemDescription;

	std::vector< CItemDescription* >	m_pContainers;

	std::vector< CItemDescription* >	m_pTakeableItems;

	std::vector< CItemDescription* >	m_pItemsByType[KItemTypes];

	std::vector< CItemDescription* >	m_pItemsByCategory[KItemCategories];
};

#endif