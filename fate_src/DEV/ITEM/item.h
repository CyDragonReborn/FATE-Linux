/*
	item.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/16/2004

*/

#ifndef _ITEM_H
#define _ITEM_H

#include "d3dx8_shim.h"

#include "itemtemplate.h"
#include "fmod.h"
#include "../GAMEOBJECTS/genericmodel.h"
#include "../UTILITIES/cullingbounds.h"

class CRefManager;
class CSettings;
class CFrustum;
class CCullingBounds;
class CItemTemplate;
class CItemInstance;
class CMaterial;
class CModel;
class CParticleManager;
class CPositionalAudioManager;
class CPositionalAudio;
class CLevel;
class CPath;
class CAttackDescription;
class CSoundBank;
class CCharacter;
class CSprite;
class CText;
class CFontMetric;
class CNameGenerator;
class CGlowManager;
class CGlow;

const enum EItemSounds
{
	KSoundActivate,
	KSoundDeactivate,
	KItemSounds
};

class CItemSaveInstance
{
public :

	CItemSaveInstance( void )
	{
	};

	CItemSaveInstance( CItemSaveInstance& pInstance ) 
	{
		m_Position = pInstance.m_Position;
		m_Orientation = pInstance.m_Orientation;

		m_BaseName = pInstance.m_BaseName;

		m_BaseModifiedName = pInstance.m_BaseModifiedName;

		m_Name = pInstance.m_Name;

		m_ArmorBonus = pInstance.m_ArmorBonus;

		m_Uses = pInstance.m_Uses;

		m_Unique = pInstance.m_Unique;

		m_Artifact = pInstance.m_Artifact;

		m_Grade = pInstance.m_Grade;

		m_Open = pInstance.m_Open;

		m_CanInteract = pInstance.m_CanInteract;

		m_Operateable = pInstance.m_Operateable;

		m_Destructible = pInstance.m_Destructible;

		m_TrapType =  pInstance.m_TrapType; 

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

		for( i = 0; i < pInstance.m_pItems.size(); i++ )
		{
			m_pItems.push_back( new CItemSaveInstance( *pInstance.m_pItems[i] ) );
		}
	};

	~CItemSaveInstance()
	{
		for( uint32 i = 0; i < KActivationTypes; i++ )
		{
			for( uint32 j = 0; j < m_pEffects[i].size(); j++ )
			{
				DELETE_SAFELY( m_pEffects[i][j] );
			}
			m_pEffects[i].clear();
		}

		for( i = 0; i < m_pItems.size(); i++ )
		{
			DELETE_SAFELY( m_pItems[i] );
		}
		m_pItems.clear();
	}

	void Save( FILE* pFile )	// open file to save to
	{

		uint16 Size = m_BaseName.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_BaseName.c_str(), sizeof( char ) * Size, 1, pFile );

		Size = m_Name.length();
		fwrite( &Size, sizeof( uint16 ), 1, pFile );
		fwrite( m_Name.c_str(), sizeof( char ) * Size, 1, pFile );

		fwrite( &m_SlotIndex, sizeof( int32 ), 1, pFile );

		fwrite( &m_Equipped, sizeof( bool ), 1, pFile );

		fwrite( &m_Value, sizeof( int32 ), 1, pFile );

		fwrite( &m_Open , sizeof( bool ), 1, pFile );

		fwrite( &m_Position , sizeof( D3DXVECTOR3 ), 1, pFile );

		fwrite( &m_Orientation , sizeof( D3DXMATRIX ), 1, pFile );

		fwrite( &m_Uses, sizeof( int32 ), 1, pFile );

		fwrite( &m_Grade, sizeof( EGrade ), 1, pFile );

		fwrite( &m_Unique, sizeof( bool ), 1, pFile );

		fwrite( &m_Artifact, sizeof( bool ), 1, pFile );

		fwrite( &m_Identified, sizeof( bool ), 1, pFile );

		fwrite( &m_ArmorBonus, sizeof( uint32 ), 1, pFile );

		fwrite( &m_Sockets , sizeof( uint32 ), 1, pFile );

		fwrite( &m_CanInteract , sizeof( bool ), 1, pFile );

		fwrite( &m_Operateable , sizeof( bool ), 1, pFile );

		fwrite( &m_Destructible , sizeof( bool ), 1, pFile );

		fwrite( &m_TrapType , sizeof( ETrapType ), 1, pFile );


		uint32 Bonuses( m_DamageBonus.size() );
		fwrite( &Bonuses , sizeof( uint32 ), 1, pFile );

		for( uint32 j = 0; j < Bonuses; j++ )
		{
			EDamageType Type( m_DamageBonus[j] );
			fwrite( &Type, sizeof( EDamageType ), 1, pFile );

			int32 Value( m_DamageBonusValue[j] );
			fwrite( &Value, sizeof( int32 ), 1, pFile );
		}

		for( j = 0; j < KActivationTypes; j++ )
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


		uint32 SubItems = m_pItems.size();
		fwrite( &SubItems , sizeof( uint32 ), 1, pFile );
		for( j = 0; j < SubItems; j++ )
		{
			m_pItems[j]->Save( pFile );
		}
	}

	void Load( FILE* pFile )	// open file to load from
	{

		uint16 Size;
		fread( &Size, sizeof( uint16 ), 1, pFile );
		m_BaseName = ReadString( pFile, Size );

		fread( &Size, sizeof( uint16 ), 1, pFile );
		m_Name = ReadString( pFile, Size );

		fread( &m_SlotIndex, sizeof( int32 ), 1, pFile );

		fread( &m_Equipped, sizeof( bool ), 1, pFile );

		fread( &m_Value, sizeof( int32 ), 1, pFile );

		fread( &m_Open , sizeof( bool ), 1, pFile );

		fread( &m_Position , sizeof( D3DXVECTOR3 ), 1, pFile );

		fread( &m_Orientation , sizeof( D3DXMATRIX ), 1, pFile );

		fread( &m_Uses, sizeof( int32 ), 1, pFile );

		fread( &m_Grade, sizeof( EGrade ), 1, pFile );

		fread( &m_Unique, sizeof( bool ), 1, pFile );

		fread( &m_Artifact, sizeof( bool ), 1, pFile );

		fread( &m_Identified, sizeof( bool ), 1, pFile );

		fread( &m_ArmorBonus, sizeof( uint32 ), 1, pFile );

		fread( &m_Sockets, sizeof( uint32 ), 1, pFile );

		fread( &m_CanInteract , sizeof( bool ), 1, pFile );

		fread( &m_Operateable , sizeof( bool ), 1, pFile );

		fread( &m_Destructible , sizeof( bool ), 1, pFile );

		fread( &m_TrapType , sizeof( ETrapType ), 1, pFile );

		if( m_Grade != KGradeNormal )
		{
			m_BaseModifiedName = "\b" + KGradeName[m_Grade] + "\b" + m_BaseName;
		}
		else
		{
			m_BaseModifiedName = m_BaseName;
		}

		uint32 Bonuses;
		fread( &Bonuses, sizeof( uint32 ), 1, pFile );

		for( uint32 j = 0; j < Bonuses; j++ )
		{
			EDamageType Type( KDamageFire );
			fread( &Type, sizeof( EDamageType ), 1, pFile );
			m_DamageBonus.push_back( Type );

			int32 Value( 0 );
			fread( &Value, sizeof( int32 ), 1, pFile );
			m_DamageBonusValue.push_back( Value );
		}


		for( j = 0; j < KActivationTypes; j++ )
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
		for( j = 0; j < SubItems; j++ )
		{
			CItemSaveInstance* pInstance = new CItemSaveInstance();
			m_pItems.push_back( pInstance );
			pInstance->Load( pFile );
		}

	}


	uint32				m_SlotIndex;
	bool				m_Equipped;

	std::string			m_BaseName;
	std::string			m_BaseModifiedName;
	std::string			m_Name;

	int32				m_Uses;

	bool				m_Unique;
	bool				m_Artifact;

	ETrapType			m_TrapType;
	bool				m_Identified;

	bool				m_Open;

	bool				m_Operateable;

	bool				m_CanInteract;

	bool				m_Destructible;

	uint32				m_Sockets;

	int32				m_Value;

	uint32				m_ArmorBonus;

	EGrade				m_Grade;

	D3DXVECTOR3			m_Position;
	D3DXMATRIX			m_Orientation;

	std::vector< EDamageType >		m_DamageBonus;
	std::vector< int32 >			m_DamageBonusValue;

	std::vector< CEffect* >	m_pEffects[KActivationTypes];

	std::vector< CItemSaveInstance* >	m_pItems;	// items contained within
};

// an item is a takeable (or openable) object, or gold

class CItem
{


public:

	CItem( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
		   CRefManager& pRefManager,			// pointer to the reference manager, for tracking media
		   CSettings& pSettings,				// pointer to the settings manager, for keeping track of card/machine capabilities
		   CParticleManager& pParticleManager,	// particle manager
		   CGlowManager& pGlowManager,			// glow manager
		   CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
		   CItemDescription& pDescription );			// item description

	CItem( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
		   CRefManager& pRefManager,			// pointer to the reference manager, for tracking media
		   CSettings& pSettings,				// pointer to the settings manager, for keeping track of card/machine capabilities
		   CParticleManager& pParticleManager,	// particle manager
		   CGlowManager& pGlowManager,			// glow manager
		   CPositionalAudioManager& pPositionalAudioManager );	// positional audio manager

	~CItem( void );
	
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						CLightManager* pLightManager,			// light manager - null for no light manager
					    const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderShadow( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							  CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		Update( float32 TimeElapsed, 		// time elapsed, in seconds
						CLevel& pLevel );			// a scene to collide with

	void		UpdateAnimation( float32 TimeElapsed );	// time elapsed, in seconds

	void		UpdateDrop( float32 TimeElapsed );	// time elapsed, in seconds

	void		UpdateProximity( const D3DXVECTOR3& Position );	// position of nearby mobile object

	void		SetSockets( uint32 Sockets )			
				{	
					m_Sockets = Sockets;			
				};

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	EGrade	Grade( void )								{	return m_Grade;					};

	ERank	Rank( void )								{	return m_Rank;					};

	bool	Socketed( void )							{	return m_Sockets > 0;			};

	uint32	Sockets( void )								{	return m_Sockets;				};

	uint32	OpenSockets( void )
			{
				return m_Sockets - m_pItems.size();
			}

	uint32	Items( void )						{	return m_pItems.size();			};

	CItem*	SubItem( uint32 Index )				{	return m_pItems[Index];			};

	ETarget		Target( void )					{	return m_Target;				};

	bool		Identified( void )				{	return m_Identified;			};

	int32		Uses( void )					{	return m_Uses;					};

	bool		Dropping( void )				{	return m_Dropping;				};

	bool		HasDisplayText( void )			{	return m_pItemName != NULL &&
													m_NameIdentified == m_Identified; };

	CText*		DisplayText( void )				{	return m_pItemName;				};

	CSprite*	TextBackdrop( void )			{	return m_pItemBackdrop;			};

	bool		DefaultMagical( void )			{	return m_DefaultMagical;		};

	bool		IsUnique( void )				{	return m_IsUnique;				};	

	bool		IsArtifact( void )				{	return m_IsArtifact;			};

	uint32				Requirements( void )		{	return m_RequirementValue.size();	};

	EStatistic			RequirementStat( uint32 Index )		{	return m_RequirementStatistic[Index];	};

	uint32				RequirementValue( uint32 Index )	{	return m_RequirementValue[Index];		};

	std::string			EffectText( EEffectActivation ActivationType );	// type to return string for

	uint32				DamageBonuses( void )				{	return m_DamageBonus.size();			};

	EDamageType			DamageBonusType( uint32 Index )		{	return m_DamageBonus[Index];			};

	int32				DamageBonusValue( uint32 Index )	{	return m_DamageBonusValue[Index];		};

	uint32				Effects( EEffectActivation ActivationType );		// type to count

	float32				EffectValue( EEffectActivation ActivationType,	// type of activation
									 EEffectType Type );				// type of effect

	CEffect*			Effect( EEffectActivation ActivationType,		// type to get
								uint32 Index );							// index within that type

	bool				IsCursed( void );

	uint32				SFXItems( void )			{	return m_pSFX.size();				};

	CSFXDescription*	SFX( uint32 Index )			{	return m_pSFX[Index];				};

	const std::string&	AnimatedModelPath( void )	{	return m_AnimatedModelPath;				};

	int32				SlotsWide( void )		{	return m_SlotsWide;				};

	int32				SlotsTall( void )		{	return m_SlotsTall;				};

	bool				InWorld( void )			{	return m_InWorld;				};
	
	bool				Useable( void )			{	return m_Useable;				};

	bool				UsedUp( void )			{	return ( m_Uses <= 0 && m_Uses != KUnlimitedUses );	};

	bool				HasIcon( void )			{	return m_pIconMaterial != NULL;	};

	CSprite*			IconSprite( void )		{	return m_pIconSprite;			};

	CMaterial*			IconMaterial( void )	{	return m_pIconMaterial;			};

	bool				CanInteract( void )		{	return m_CanInteract;		};

	CSoundBank*			AttackSounds( void )	{	return m_pAttackSounds;		};
	CSoundBank*			StrikeSounds( void )	{	return m_pStrikeSounds;		};
	CSoundBank*			MissSounds( void )		{	return m_pMissSounds;		};

	CAttackDescription*	LeftAttack( void )		{	return m_pLeftAttack;		};
	CAttackDescription*	RightAttack( void )		{	return m_pRightAttack;		};

	float32				AttackRadius( void )	{	return m_AttackRadius;		};
	float32				StrikeRadius( void )	{	return m_StrikeRadius;		};

	uint32				Value( void )			{	return m_Value;				};

	uint32				PurchaseValue( void );

	uint32				SellValue( void );

	uint32				EnchantValue( void );

	uint32				GambleValue( void );

	int32				BaseLevel( void )		{	return m_BaseLevel;			};

	EAttackSpeed		Speed( void )			{	return m_Speed;				};

	uint32				ArmorBonus( void );

	uint32				MinimumDamage( void );
	uint32				MaximumDamage( void );

	const D3DXVECTOR3&	GetMinBounds( void )	{	return m_pCullingBounds->GetLocalMinBounds();	};

	const D3DXVECTOR3&	GetMaxBounds( void )	{	return m_pCullingBounds->GetLocalMaxBounds();	};

	bool				Onscreen( void )			{	return m_Onscreen;			};

	bool				Operateable( void )			{	return m_Operateable;		};

	bool				Takeable( void )			{	return m_Takeable;			};

	bool				Destructible( void )		{	return m_Destructible;		};

	bool				IsContainer( void )			{	return m_Category == KCategoryContainer;	};

	bool				CastsShadow( void )			{	return m_Shadow;			};

	EItemType			Type( void )				{	return m_Type;				};

	EItemCategory		Category( void )			{	return m_Category;			};

	CCullingBounds*		CullingBounds( void )		{	return m_pCullingBounds;	};

	CGenericModel*		Model( void )				{	return m_pModel;			};

	CGenericModel*		OpenModel( void )			{	return m_pOpenModel;			};

	CGenericModel*		CloseModel( void )			{	return m_pCloseModel;			};

	const D3DXVECTOR3&	Position( void )			{	return m_Position;			};

	const D3DXMATRIX&	Orientation( void ) const	{	return m_OrientationMatrix;	};

	const D3DXMATRIX&	Transformation( void ) const{	return m_TransformationMatrix;	};

	bool				Collideable( void )			{	return m_Collideable;		};

	bool				HasLightPool( void )		{	return m_HasLightPool;		};

	CMaterial*			LightPoolMaterial( void )	{	return m_pLightPoolMaterial;	};

	float32				LightPoolScale( void )		{	return m_LightPoolScale;		};
	float32				LightPoolAngle( void )		{	return m_LightPoolAngle;		};

	const std::string&	Name( void )				{	return m_Name;					};

	const std::string&	BaseModifiedName( void )	{	return m_BaseModifiedName;		};

	const std::string&	UseDescription( void )		{	return m_UseDescription;		};

	bool				RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
									  const D3DXVECTOR3& End,		// end of collision ray
									  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
									  D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

	bool				SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
										 const D3DXVECTOR3& End,		// end of collision ray
										 float32 Radius,				// radius of sphere to check along collision ray
										 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
										 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
										 D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

	bool RequirementsMet( CCharacter& pCharacter );	// character to check requirements for

	ETrapType	TrapType( void )						{	return m_TrapType;				};

	bool		Trapped( void )							{	return m_TrapType != KTrapNone;	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void	ApplyInstance( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   CItemSaveInstance& pInstance,		// description to fill
						   CItemTemplate& pItemManifest );	// item manifest to construct items from

	void	FillInstance( CItemSaveInstance& pInstance,		// description to fill
						  uint32 SlotIndex,					// slot it is in
						  bool Equipped );					// equipped?

	void	SetAmbientPositionalAudio( const std::string& Path );	// path to audio clip

	void	SetTrapType( ETrapType TrapType );	// type of trap

	void	AddDamageBonus( EDamageType Type,	// type of damage
							int32 Value )		// value
						  {	
							  for( uint32 i = 0; i < m_DamageBonus.size(); i++ )
							  {
								  if( m_DamageBonus[i] == Type )
								  {
									  m_DamageBonusValue[i] += Value;
									  return;
								  }
							  }
							  m_DamageBonus.push_back( Type );
							  m_DamageBonusValue.push_back( Value );
						  }

	void	RemoveDamageBonus( EDamageType Type,	// type of damage
								int32 Value )		// value
						  {	
							  for( uint32 i = 0; i < m_DamageBonus.size(); i++ )
							  {
								  if( m_DamageBonus[i] == Type )
								  {
									  m_DamageBonusValue[i] -= Value;
									  m_DamageBonus[i] = m_DamageBonus[m_DamageBonus.size()-1];
									  m_DamageBonusValue[i] = m_DamageBonusValue[m_DamageBonus.size()-1];
									  m_DamageBonus.pop_back();
									  m_DamageBonusValue.pop_back();
									  return;
								  }
							  }

						  }

	void	CreateNameForDisplay( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device)
								  CFontMetric* pFontMetric,				// font metric
								  CMaterial* pFontMaterial );			// font material

	void	SetBaseLevel( uint32 Value )						{	m_BaseLevel = Value;				};

	void	SetIdentified( bool State )							{	m_Identified = State;				};

	void	SetIsUnique( bool State )							{	m_IsUnique = State;					};

	void	SetName( const std::string& Name )					{	m_Name = Name;						};

	void	SetUses( int32 Uses )								{	m_Uses = Uses;						};

	void	SetAnimatedModelPath( const std::string& Path )		{	m_AnimatedModelPath = Path;			};

	void	BestowGrade( EDifficulty Difficulty );	// game difficulty - determines what can be had

	void	BestowSockets( bool AllowUnique = kFalse );

	void	RemovePowers( void );

	void	BestowPowers( int32 Level,					// level for baseline - 100 = best
						  bool AllowCurse = kTrue,		// allow cursed items?
						  int32 MinimumLevel = -1 );	// minimum level

	void	BestowUniquePowers( CNameGenerator& pNameGenerator,	// name generator for unique name
								int32 Level,					// level for baseline - 100 = best
								bool AllowCurse = kTrue,		// allow cursed items?
								int32 MinimumLevel = -1 );		// minimum level

	void	AddEffect( CEffect* pEffect );	// effect to add

	void	ClearEffects( void );
	
	// don't copy it, just put the pointer in
	void	AddNewEffect( CEffect* pEffect );	// effect to add

	void	SetInWorld( bool State );

	void	DropToGround( CLevel& pLevel );			// a scene to collide with

	void	SetTransformation( const D3DXMATRIX& Transformation );
	void	SetPosition( CLevel& pLevel,				// level which has the avoidance map
						 const D3DXVECTOR3& Position );	// position to set

	void	CalculateLocalLight( bool Torchlit,					//  is the level torchlit?
								 const D3DXVECTOR3& FocalPoint,		// point where the camera is directed
							     float32 MaximumDistance );			// maximum distance from camera

	void	SetOnscreen( bool State )		{	m_Onscreen = State;		};

	void	PlayTakeSound( void );

	void	PlayRemoveSound( void );
	
	void	Drop( CLevel& pLevel );	// level which has the avoidance map

	void	Use( CLevel& pLevel,		// level to add effects to
				 CCharacter* pUser,		// using character
				 CCharacter* pTarget );	// target character

	void	Use( CLevel& pLevel,		// level to add effects to
				 CCharacter* pUser,		// using character
				 CItem* pTarget );		// target item

	bool	CanUseOnItem( CItem* pTarget ); // target item

	bool	CanUse( CLevel& pLevel,			// level to post effects to
					CCharacter* pUser,		// using character
					CCharacter* pTarget );	// target character

	CItem*	AddItem( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					 CItemDescription& pDescription );		// item description

	CItem*	AddItem( CItem* pItem );		// item to add

	void	RemoveItem( CItem* pItem );		// item to remove

	CItem*	AddGold( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					 uint32 Value );						// amount of gold

	void	Open( CLevel& pLevel );	// level to open items into

	void	CompleteEvent( CLevel& pLevel,	// level to open items into
						   bool Success );	// successful completion?

	void	OpenToLevel( CLevel& pLevel );	// level to open items into

	void	Destroy( CLevel& pLevel );	// level to open items into

	void	RemoveFromAvoidanceMap( CLevel& pLevel );	// level which has the avoidance map

	void	AddToAvoidanceMap( CLevel& pLevel );	// level which has the avoidance map

	void	DeleteContainedItems( void );

	void	ApplyContainedEffects( void );

	void	RemoveEffect( CEffect* pEffect );	// effect to add

	void	DetermineName( void );

	void	EmitParticles( void );

	void	UpdateGlows( void );

protected:

	void				CalculateIdentifyLevel( void );

	void				BlendRandomIdleAnimation( void );

	void				FindParticles( void );

	bool				InFrustum( CFrustum& pFrustum );	// frustum to cull against

	D3DXVECTOR3			m_Position;
	
	D3DXVECTOR3			m_LocalLight;

	D3DXMATRIX			m_OrientationMatrix;
	D3DXMATRIX			m_TransformationMatrix;

	CCullingBounds*		m_pCullingBounds;

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CParticleManager&	m_pParticleManager;
	CGlowManager&		m_pGlowManager;

	CPositionalAudioManager&	m_pPositionalAudioManager;

	CGenericModel*		m_pModel;
	CGenericModel*		m_pOpenModel;
	CGenericModel*		m_pCloseModel;
	CModel*				m_pCollisionModel;

	bool				m_Operateable;

	bool				m_Identified;

	ETarget				m_Target;

	bool				m_Useable;
	bool				m_Takeable;
	bool				m_Collideable;
	bool				m_Shadow;
	bool				m_Destructible;

	CMaterial*			m_pLightPoolMaterial;
	bool				m_HasLightPool;
	ELightPoolType		m_LightPoolType;
	float32				m_LightPoolRadius;
	float32				m_LightPoolScale;
	float32				m_LightPoolAngle;

	float32				m_TorchLightFlicker;

	float32				m_ActivationRadius;
	float32				m_DeactivationRadius;

	float32				m_AttackRadius;
	float32				m_StrikeRadius;

	bool				m_Onscreen;

	bool				m_CanOpen;
	bool				m_CanClose;
	bool				m_Open;
	bool				m_OpenComplete;

	ETrapType			m_TrapType;

	bool				m_IsUnique;

	bool				m_IsArtifact;

	bool				m_ObjectNearby;

	std::vector< CGlow* >		m_pGlowList;
	std::vector< CGlow* >		m_pModelGlowList;
	std::vector< uint32 >		m_GlowIndex;
	std::vector< D3DXVECTOR3 >	m_GlowPosition;

	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	std::vector< uint32 >		m_TagParticleIndex;
	std::vector< int32 >		m_TagParticleType;

	CPositionalAudio*		m_pAmbientPositional;

	std::string				m_AmbientSoundPath;

	FSOUND_SAMPLE*			m_pTakeSound;
	FSOUND_SAMPLE*			m_pDropSound;
	FSOUND_SAMPLE*			m_pLandSound;
	FSOUND_SAMPLE*			m_pUseSound;

	uint32				m_BaseSoundVolume;
	float32				m_BaseSoundRadius;

	std::vector< uint32 >		m_IdleAnimations;


	uint32				m_Value;

	int32				m_ToHitBonus;

	uint32				m_ArmorBonus;
	uint32				m_MaximumArmorBonus;

	uint32				m_MinimumDamage;
	uint32				m_MaximumDamage;

	int32				m_SlotsWide;
	int32				m_SlotsTall;

	int32				m_BaseLevel;

	EAttackSpeed		m_Speed;

	int32				m_Uses;
	int32				m_OriginalUses;

	std::string			m_Name;
	std::string			m_BaseName;
	std::string			m_BaseModifiedName;

	std::string			m_UseDescription;

	std::string			m_AnimatedModelPath;

	EItemType			m_Type;
	EItemCategory		m_Category;

	bool				m_OnGround;
	bool				m_OnAvoidanceMap;

	std::vector< CItem* >	m_pItems;	// items contained within

	CPath*				m_pDropPath;
	float32				m_DropPosition;
	bool				m_Dropping;

	bool				m_FliesWhenDropped;

	bool				m_InWorld;		// actually in the level?

	bool				m_CanInteract;	// can be accessed with the mouse
										// for opening or bashing

	CAttackDescription*	m_pLeftAttack;
	CAttackDescription*	m_pRightAttack;

	CSoundBank*			m_pItemSounds;

	CSoundBank*			m_pAttackSounds;
	CSoundBank*			m_pStrikeSounds;
	CSoundBank*			m_pMissSounds;

	CSoundBank*			m_pTrapSounds;

	CMaterial*			m_pIconMaterial;

	CSprite*			m_pIconSprite;

	CText*				m_pItemName;
	CSprite*			m_pItemBackdrop;
	bool				m_NameIdentified;

	std::vector< CEffect* >	m_pEffects[KActivationTypes];

	std::vector< CSFXDescription* >	m_pSFX;

	std::vector< EStatistic >		m_RequirementStatistic;
	std::vector< uint32 >			m_RequirementValue;

	std::vector< EDamageType >		m_DamageBonus;
	std::vector< int32 >			m_DamageBonusValue;

	uint32							m_Sockets;

	float32							m_GambleMultiplier;
	int32							m_CollisionMaskRadius;

	EGrade							m_Grade;
	ERank							m_Rank;

	bool							m_DefaultMagical;
};

#endif