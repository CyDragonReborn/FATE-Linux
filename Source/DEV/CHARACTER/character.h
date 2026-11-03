/*
	Character.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

#ifndef _CHARACTER_H
#define _CHARACTER_H

#include "fmod.h"

#include "../UTILITIES/constants.h"
#include "../GAMECLIENT/gamerules.h"
#include "../GAMEOBJECTS/genericmodel.h"
#include "charactertemplate.h"
#include "../UTILITIES/cullingbounds.h"
#include "../ITEM/item.h"

class CModel;
class CRefManager;
class CSettings;
class CLevel;
class CLightManager;
class CCullingBounds;
class CFrustum;
class CMaterial;
class CCubeMap;
class CAStar;
class CPath;
class CParticleManager;
class CPositionalAudioManager;
class CPositionalAudio;
class CSoundBank;
class CItem;
class CInventory;
class CItemTemplate;
class CEffect;
class CLevelHistory;
class CSpellDescription;
class CSpellTemplate;
class CSpellDescription;
class CNameGenerator;
class CQuest;
class CGlow;
class CGlowManager;
class CCharacterTemplate;
class CSprite;
class CText;

const uint32  KMaximumActiveQuests( 3 );

const uint32  KMaximumSpellsPerSphere( 6 );

// maximum number of levels the player can remember
const uint32  KMaximumDiscoveryMemory( 20 );

const uint32  KMaximumSummonedCreatures( 7 );

const uint32  KMaximumPossibleSummonedCreatures( 10 );

const float32 KMaxTurnRate( 520 );

const uint32  KMaxInventorySlots( 40 );

const uint32  KMaxMerchantInventorySlots( 110 );

const float32 KPetTeleportRadius( 64 );

const float32 KPetFollowRadius( 20 );

const float32 KPetAttackRadius( 35 );

const float32 KHibernationRange( 64 );

const float32 KGetGoldProximity( 6 );

const enum EAttachmentPoint
{
	KAttachHead,
	KAttachChest,
	KAttachLeftHand,
	KAttachRightHand,
	KAttachLeftShoulder,
	KAttachRightShoulder,
	KAttachLeftArm,
	KAttachRightArm,
	KAttachNone,
	KAttachmentPoints
};

const std::string KAttachmentNames[KAttachmentPoints] =
{
	"HEAD",
	"CHEST",
	"LEFTHAND",
	"RIGHTHAND",
	"LEFTSHOULDER",
	"RIGHTSHOULDER",
	"LEFTARM",
	"RIGHTARM",
	"NONE"
};

const enum ECharacterSounds
{
	KSoundDie,
	KSoundStep,
	KSoundIdle,
	KSoundUniqueIdle,
	KSoundRoar,
	KSoundGreet,
	KSoundDispel,
	KSoundBlock,
	KSoundCritical,
	KSoundAttack,
	KSoundCast,
	KSoundCastSplash,
	KSoundPullSplash,
	KSoundBiteSplash,
	KSoundGold,
	KSoundLevelUp,
	KSoundHelpManaLow,
	KSoundHelpStaminaLow,
	KSoundHelpHealthLow,
	KSoundHelpDie,
	KSoundHelpLevelUp,
	KSoundHelpFameUp,
	KSoundHelpPetLevelUp,
	KSoundHelpPetFlee,
	KSoundHelpPetTransform,
	KSoundHelpPoison,
	KSoundHelpTrap,
	KSoundHelpLowSkill,
	KSoundHelpLowPower,
	KSoundHelpNewQuest,
	KSoundHelpQuestComplete,
	KSoundHelpSpellFail,
	KSoundHelpSpellMiscast,
	KSoundHelpNewSpell,
	KSoundHelpJourneyBegin,
	KSoundHelpJourneyEnd,
	KSoundHelpTemptFate,
	KSoundHelpFateSmiles,
	KSoundHelpPortal,
	KSoundHelpPetDepart,
	KSoundHelpPetReturn,
	KSoundHelpImpossible,
	KSoundHelpRefreshed,
	KSoundHelpFortified,
	KSoundHelpStronger,
	KSoundHelpHealthy,
	KSoundHelpLowGold,
	KSoundHelpPackFull,
	KSoundHelpChooseFate,
	KSoundHelpFateChosen,
	KSoundHelpNone,
	KCharacterSounds
};

const enum EAIState
{
	KAIInactive,
	KAIWander,
	KAIAttack,
	KAICast,
	KAIHunt,
	KAIDying,
	KAIDead,
	KAIGetItem,
	KAIOperateItem,
	KAIApproach,
	KAICollectReward,
	KAIReceiveQuest,
	KAIBuySell,
	KAISpeakMerchant,
	KAIUniqueIdle,
	KAIFlee,
	KAIGetRange,
	KAIFish,
	KAIPullFish,
	KAICancelFish,
	KAIReceiveFish,
	KAIInEvent,
	KAIReturningToTown,
	KAIInTown,
	KAISendPetToTown,
	KAIRetire,
	KAICompleteRetire,
	KAIStates
};


class CCharacterSaveInstance
{
public :
	CCharacterSaveInstance() :
						m_pMasterQuest( NULL )	
	{
	}
	~CCharacterSaveInstance();

	void Save( FILE* pFile );	// open file to load from

	void Load( FILE* pFile );	// open file to load from

	std::string	m_TemplateName;
	std::string	m_OriginalTemplateName;

	bool		m_IsPlayer;

	int32		m_JournalStats[KJournalStatistics];

	float32		m_ExistenceTime;

	float32		m_TransformationDuration;

	float32		m_LifeDuration;

	float32		m_Scale;

	float32		m_Bravery;

	std::string	m_Name;
	std::string	m_AncestorName;

	int32		m_Lineage;

	bool		m_IsMerchant;

	EMerchant	m_MerchantType;

	int32		m_MasterIndex;

	int32		m_DungeonSeed;

	float32		m_TownTimer;

	uint32		m_LastDungeonLevel;

	bool		m_Unique;

	EGender		m_Gender;

	int32		m_HairIndex;
	int32		m_HeadIndex;

	bool		m_HasOpenPortal;
	uint32		m_PortalDepth;
	D3DXVECTOR3	m_PortalPosition;

	D3DXVECTOR3	m_Position;
	D3DXMATRIX	m_Orientation;

	int32		m_Level;
	int32		m_Experience;
	int32		m_Fame;
	int32		m_FameRank;
	float32		m_HP;
	int32		m_MaxHP;

	float32		m_Stamina;
	int32		m_MaxStamina;

	float32		m_Mana;
	int32		m_MaxMana;

	int32		m_ToHitBonus;
	int32		m_OriginalToHitBonus;

	int32		m_NaturalArmor;
	int32		m_OriginalNaturalArmor;

	int32		m_ExperienceAward;
	int32		m_FameAward;

	int32		m_UnusedStatPoints;
	int32		m_UnusedSkillPoints;

	int32				m_DamageResistance[KDamageTypes];

	int32				m_SkillPoints[KSkills];
	std::string			m_KnownSpells[KMagicSpheres][KMaximumSpellsPerSphere];

	std::string	m_ActiveSpellName;

	int32				m_Strength;
	int32				m_OriginalStrength;
	int32				m_Dexterity;
	int32				m_OriginalDexterity;
	int32				m_Vitality;
	int32				m_OriginalVitality;
	int32				m_Magic;
	int32				m_OriginalMagic;

	float32				m_WalkingSpeed;
	float32				m_RunningSpeed;

	int32				m_Gold;

	std::vector< CItemSaveInstance* >	m_pItemInstances;
	std::vector< CEffect* >	m_pEffects[KActivationTypes];
	std::vector< CQuest* >	m_pQuests;

	CQuest*					m_pMasterQuest;

	EDifficulty				m_Difficulty;

	std::string	m_GemsCollectedList;
	std::string	m_FishCaughtList;
};


// this is just a simple class for maintaining location, orientation, and model references
// of an Character in 3d space
class CCharacter
{
public:

				CCharacter( CRefManager& pRefManager,								// pointer to the reference manager, for tracking submaterials
						    CSettings& pSettings,									// pointer to the settings manager, for keeping track of card/machine capabilities
							CSpellTemplate& pSpellTemplate,							// spell template for casting
							CParticleManager& pParticleManager,						// particle manager
							CGlowManager& pGlowManager,								// glow manager
							CPositionalAudioManager& pPositionalAudioManager );		// positional audio manager
				~CCharacter( void );

	virtual void		Update( const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
								float32 TimeElapsed, 		// time elapsed, in seconds
								CLevel& pLevel );			// a scene to collide with

	virtual void		UpdateAI( float32 TimeElapsed, 		// time elapsed, in seconds
								  CLevel& pLevel );			// a scene to interact with

	virtual void		DyingAI( float32 TimeElapsed, 		// time elapsed, in seconds
								 CLevel& pLevel );			// a scene to interact with

	virtual void		HuntAI( float32 TimeElapsed, 		// time elapsed, in seconds
							    CLevel& pLevel );			// a scene to interact with

	virtual void		ApproachAI( float32 TimeElapsed, 		// time elapsed, in seconds
								    CLevel& pLevel );			// a scene to interact with

	virtual void		GetItemAI( float32 TimeElapsed, 		// time elapsed, in seconds
								   CLevel& pLevel );			// a scene to interact with

	virtual void		OperateItemAI( float32 TimeElapsed, 		// time elapsed, in seconds
									   CLevel& pLevel );			// a scene to interact with

	virtual void		ReturnToTownAI( float32 TimeElapsed, 		// time elapsed, in seconds
									    CLevel& pLevel );			// a scene to interact with

	virtual void		AttackAI( float32 TimeElapsed, 		// time elapsed, in seconds
								  CLevel& pLevel );			// a scene to collide with

	virtual void		CastAI( float32 TimeElapsed, 		// time elapsed, in seconds
								CLevel& pLevel );			// a scene to collide with

	void				FishAI( float32 TimeElapsed, 		// time elapsed, in seconds
							    CLevel& pLevel );			// a scene to interact with

	void				PullFishAI( float32 TimeElapsed, 		// time elapsed, in seconds
								    CLevel& pLevel );			// a scene to interact with

	void				CancelFishAI( float32 TimeElapsed, 		// time elapsed, in seconds
									  CLevel& pLevel );			// a scene to interact with

	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						CLightManager* pLightManager,			// light manager - null for no light manager
					    const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderBehindWall( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									CLightManager* pLightManager,			// light manager - null for no light manager
									const D3DXMATRIX& ViewportMatrix,		// viewport matrix
									CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderShadow( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							  CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   const D3DXVECTOR3& LightDirection );	// direction the light is coming from

	bool		RemembersLevel( int32 LevelDepth );		// depth to check for

	void		CompleteDeath( CLevel& pLevel );			// a scene to interact with

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	virtual void SetAIState( EAIState State );	// ai state to set

	void		SetMouseOver( void )					{	m_MouseOver = kTrue;			};

	void		SetDifficulty( EDifficulty Difficulty )	{	m_Difficulty = Difficulty;		};

	// set the max depth of astar
	void		SetMaximumTreeDepth( uint32 Depth );	// tree depth for pathfinding

	void		ReturnToTown( CLevel& pLevel );			// active level

	void		NotifyOfItemReceipt( CItem* pItem,		// item we received
									 CLevel* pLevel );	// level to open items into

	void		PullFish( void );

	void		CancelFish( void );

	void		SetHasOpenPortal( bool State )			{	m_HasOpenPortal = State;		};

	void		FillInstance( CCharacterSaveInstance& pInstance,	// description to fill
							  CLevel& pLevel );						// current level

	void		ApplyInstance( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							   CLevel* pLevel,						// level we are in
							   CCharacterTemplate& pCharacterManifest,// character manifest
							   CCharacterSaveInstance& pInstance,	// description to fill
							   CItemTemplate& pItemManifest );		// item manifest to construct items from

	void		OpenPortal( CLevel& pLevel );			// level to open portal in 

	void		CreatePortals( CLevel& pLevel );		// level to open portal in 

	void		RememberDiscovery( CLevel& pLevel );	// level to remember discovery of

	void		ApplyInstantEffect( CLevel& pLevel,		// level to send event effects to
									CEffect* pEffect );	// effect to apply

	CEffect*	AddEffect( CLevel& pLevel,		// level to send text events to
						   CEffect* pEffect,	// effect to add
						   int32 Bonus = 0,		// bonuses to apply ( defaults to 0 )
						   CCharacter* pOwner = NULL,	// owner of the effect
						   bool DoSFX = kTrue );

	void		AddNaturalEffect( CEffect* pEffect,	// effect to add
								  int32 Bonus = 0,		// bonuses to apply ( defaults to 0 )
								  CCharacter* pOwner = NULL );// owner of the effect

	void		RemoveEffect( std::string Name );	// name of effect to remove

	bool		HasEffect( std::string Name );		// name of effect to check for

	bool		HasNaturalEffect( std::string Name );		// name of effect to check for

	void		CalculateEffectValues( void );

	float32		EffectValue( EEffectType Type );			// type of effect
	float32		CalculateEffectValue( EEffectType Type );	// type of effect

	uint32				Effects( EEffectActivation ActivationType );		// type to count

	CEffect*			Effect( EEffectActivation ActivationType,		// type to get
								uint32 Index );							// index within that type

	// don't copy it, just put the pointer in
	CEffect*	AddNewEffect( CLevel& pLevel,		// level to send text events to
							  CEffect* pEffect,		// effect to add
							  int32 Bonus = 0,		// bonuses to apply ( defaults to 0 )
							  CCharacter* pOwner = NULL,	// owner of the effect
							  bool DoSFX = kTrue );

	void		AddNewNaturalEffect( CEffect* pEffect,		// effect to add
									int32 Bonus = 0,		// bonuses to apply ( defaults to 0 )
									CCharacter* pOwner = NULL );// owner of the effect

	void		GetItem( CItem* pTargetItem,	// item to get ( assumes proximity is good )
						 CLevel& pLevel );		// a scene to collide with

	void		MakeUnique( CNameGenerator& pNameGenerator,	// generator to make random name
							int32 Level,					// level being generated for
						    bool SuperSize );				// super-sized?

	void		MakeUnique( const std::string& Name,		// name of unique monster
							int32 Level,					// level being generated for
						    bool SuperSize );				// super-sized?

	void		TweakToLevel( int32 Level );	// level to upgrade base statistics to

	void		AwardExperience( CLevel& pLevel,				// level to send text events to
								 CCharacter* pAwarder,	// xp awarder	
								 int32 Award,					// points to modify by
								 bool FromMaster  = kFalse );	// awarded by master character?

	void		AwardFame( CLevel& pLevel,				// level to post message to
						   CCharacter* pAwarder,		// xp awarder
						   int32 Award,					// points to modify by
						   bool FromMaster  = kFalse );	// awarded by master character?

	void		ModifyHP( float32 HP );

	void		ModifyMana( float32 Mana );

	void		ModifyStamina( float32 Stamina );

	void		SetOnscreen( bool State )				{	m_Onscreen = State;		};

	void		SetHoldPosition( bool State )			{	m_HoldPosition = State;		};

	void		SetName( const std::string& Name )		{	m_Name = Name;		};

	void		AddPet( CCharacter* pCharacter );		// character to set as pet

	void		RemovePet( CCharacter* pCharacter );	// character to remove as pet

	void		RemovePets( void );						// remove all pets

	void		SetMaster( CCharacter* pCharacter );	// character to set as master

	void		GiveGold( int32 Gold );		// amount to give

	void		TakeGold( uint32 Gold );	// amount to take

	void		SpendStrengthPoint( void );

	void		SpendVitalityPoint( void );

	void		SpendDexterityPoint( void );

	void		SpendMagicPoint( void );

	void		SpendSkillPoint( ESkill Skill );

	void		RemoveStrengthPoint( void );

	void		RemoveVitalityPoint( void );

	void		RemoveDexterityPoint( void );

	void		RemoveMagicPoint( void );

	void		RemoveSkillPoint( ESkill Skill );

	void		GiveItem( CLevel& pLevel,		// a scene to drop items into
						  CItem* pItem );		// item to give
	void		DropToGround( CLevel& pLevel );			// a scene to collide with

	void		CreatePathfinder( CLevel& pLevel );	// level to create pathfinder for

	void		SetDestination( CLevel& pLevel,	// a scene to collide with
								float32 X,		// worldspace x position
								float32 Y );	// worldspace y position

	void		StopPathing( void );


	void		SetPosition( const D3DXVECTOR3& Position );			// position to place character at

	void		LoadGloves( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const std::string& GlovesModelPath );	// path to animated gloves mesh

	void		LoadArmor( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const std::string& ArmorModelPath );		// path to animated armor mesh

	void		LoadBoots( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const std::string& BootsModelPath );		// path to animated Boots mesh

	void		UnloadModels( void );

	void		LoadPlayerModels( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		LoadModel( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   std::string ModelPath,			// path to the model we want to load
						   std::string UpperBodyModelPath,	// path to the upper body model we want to load (optional)
						   std::string HeadModelPath,		// path to the head model we want to load (optional)
						   std::string HairModelPath,		// path to the hair model we want to load (optional)
						   std::string TexturePath,			// path to textures ("" to use model path)
						   std::string CollisionPath,		// path to collision model
						   std::string AnimationPath );		// path to animation data file

	void		SetOrientation( const D3DXMATRIX& Orientation )		{	m_OrientationMatrix = Orientation;	};

	void		CalculateLocalLight( bool Torchlit,					//  is the level torchlit?
									 float32 TimeElapsed,				// for smoothing
									 const D3DXVECTOR3& FocalPoint,		// point where the camera is directed
									 float32 MaximumDistance );			// maximum distance from camera

	void		SetCollisionRadius( float32 Value )				{	m_CollisionRadius = Value;	};

	void		SetCollideable( bool State )					{	m_Collideable = State;		};

	void		NotifyOfDeletion( CCharacter* pCharacter );	// character being deleted

	void		NotifyOfDeletion( CItem* pItem );			// item being deleted

	void		NotifyOfDeath( CLevel& pLevel,				// active level
							   CCharacter* pCharacter );	// character which has died

	void		FollowCharacter( CCharacter* pCharacter,	// character to follow
								 CLevel& pLevel );			// a scene to collide with

	void		SetTarget( CCharacter* pCharacter,
						   bool TrimAttack = kTrue );

	void		SetTargetItem( CItem* pItem );

	void		AddUnarmedAttack( CAttackDescription* pAttack )		{	m_UnarmedAttacks.push_back( pAttack );	};

	void		RemoveFromAvoidanceMap( CLevel& pLevel );	// level which has the avoidance map

	void		AddToAvoidanceMap( CLevel& pLevel );		// level which has the avoidance map

	void		SetRunning( bool Running );					// running state

	void		SetIsMerchant( bool State );				// merchant or not?

	void		SetMerchantType( EMerchant Type );			// type of merchant

	void		SetIsQuestGiver( bool State )			{	m_IsQuestGiver = State;						};

	void		SetInvincible( bool State )				{	m_Invincible = State;						};

	void		LearnSpell( CSpellDescription* pSpellDescription,	// spell to learn
							uint32 Slot );							// slot within sphere
							
	void		UnLearnSpell( EMagicSphere Sphere,					// sphere of magic
							  uint32 Index );						// slot to unlearn

	void		CastSpell( CLevel& pLevel,							// level to remember discovery of
						   CSpellDescription* pSpellDescription,	// spell to learn
						   CItem* pItem,							// item to cast on
						   CCharacter* pTargetCharacter,			// character targeted
						   const D3DXVECTOR3& Position );			// worldspace target position

	void		CompleteSpell( CLevel& pLevel );					// level to remember discovery of

	void		CycleSpell( int32 Direction );						// direction

	void		SetActiveSpell( CSpellDescription* pSpellDescription ); // spell to set as active

	void		SetActiveSpell( const std::string& Name );				// spell to set as active

	void		EnsureSpellSelection( void );

	void		LevelUp( void );

	void		SetAllowDropEquipment( bool State )				{	m_AllowDropEquipment = State;		};

	void		SetAllowDropInventory( bool State )				{	m_AllowDropInventory = State;		};

	void		SetLifeDuration( float32 Duration )				{	m_LifeDuration = Duration;			};

	void		AddDiscoveryHistory( CLevelHistory* pHistory )	{	m_pDiscoveryHistory.push_back( pHistory );	};

	void		UpdateAnimation( float32 TimeElapsed );			// time elapsed in seconds

	void		GiveQuest( CQuest& pQuest );					// quest to give

	void		GiveMasterQuest( CQuest& pQuest );				// quest to give

	void		ReceiveReward( CQuest& pQuest,					// quest to receive reward from
							   LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
							   CLevel& pLevel,					// level to drop to if full
							   CItemTemplate& pItemManifest );	// item manifest to construct items from

	void		ClearQuests( void );

	void		ClearMasterQuest( void );

	void		RemoveQuest( uint32 Index );	// index of quest to remove

	void		EmitParticles( void );

	void		CompleteEvent( CNameGenerator& pNameGenerator,	// generator to make random name
							   CLevel& pLevel,					// active level
							   uint32 ConfirmationResult,		// result of the confirmation menu button press
							   CItem* pItem );					// event item

	void		ClearState( void );

	void		CreateNameForDisplay( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device)
									  CFontMetric* pFontMetric,				// font metric
									  CMaterial* pFontMaterial );			// font material

	void		Greet( const D3DXVECTOR3& Position );	// sound position

	void		PlayHelpSample( ECharacterSounds Sample,		// sample to put on the help queue
								bool HighPriority = kFalse );		// high priority?

	void		UpdateHelp( float32 TimeElapsed );		// time elapsed, in seconds

	void		ClearCurrentAttack( void );

	void		IncrementStat( EJournalStatistic Stat,			// stat to increment
							   int32 Value = 1 );				// value to increment

	void		FindParticles( void );

	void		ClearParticles( void );

	void		SetLineage( int32 Lineage )						{	m_Lineage = Lineage;				};

	void		ClearEffects( void );

	void		ClearNaturalEffects( void );

	virtual void	CalculateMaxHP( void );

	virtual void	RespondToAttack( CCharacter* pAttacker );	// attacking character

	void		SetGemsCollected( std::string Gems)			{	m_GemsCollectedList = Gems;		};
	void		SetGemsCollectedNumber( uint32 Gems)			{	m_gemTypesCollected = Gems;		};
	void		SetFishCaught( std::string Fish)			{	m_FishCaughtList = Fish;		};
	void		SetFishCaughtNumber( uint32 Fish)			{	m_fishTypesCollected = Fish;		};

	//Cheat System Control / Achievements
	void		SetDisabledCheatAchievements( bool disabledAchievements) { m_disabledAchievements = disabledAchievements; };

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	const std::string&	OriginalTemplateName( void )			{	return m_OriginalTemplateName;		};
	
	const std::string&	TemplateName( void )					{	return m_TemplateName;				};

	float32		Scale( void )									{	return m_Scale;						};

	ERank		Rank( void )									{	return m_Rank;						};

	float32		ExistenceTime( void )							{	return m_ExistenceTime;				};

	int32		JournalStat( EJournalStatistic Stat )			{	return m_JournalStats[Stat];			};

	EDifficulty	Difficulty( void )								{	return m_Difficulty;				};

	bool		HasGreetings( void )							{	return m_GreetMessages.size() > 0;	};

	const std::string&	Greeting( void );

	bool		DisplayedAsItem( void )							{	return m_DisplayedAsItem;			};

	bool		HasDisplayText( void )							{	return m_pCharacterName != NULL;	};

	CText*		DisplayText( void )								{	return m_pCharacterName;			};

	CSprite*	TextBackdrop( void )							{	return m_pCharacterBackdrop;		};

	float32		TownTimer( void )								{	return m_TownTimer;					};

	bool		Fishing( void )									{	return AIState() == KAIFish || AIState() == KAIPullFish;	};

	bool		HasQuestsForCharacter( std::string Name );	// character to check for quests for

	bool		HasQuestsForLevel( uint32 Depth );			// level depth to check for

	bool		HasMasterQuestForLevel( uint32 Depth );			// level depth to check for

	bool		QuestsFull( void )								{	return m_pQuests.size() >= KMaximumActiveQuests;	};

	uint32		Quests( void )									{	return m_pQuests.size();			};

	CQuest*		Quest( uint32 Index )							{	return m_pQuests[Index];			};

	CQuest*		MasterQuest( void )								{	return m_pMasterQuest;				};

	CLevelHistory*		GetLevelHistory( int32 LevelDepth );	// depth to check for

	uint32		LevelHistories( void )							{	return m_pDiscoveryHistory.size();	};

	CLevelHistory*	GetLevelHistoryByIndex( uint32 Index )		{	return m_pDiscoveryHistory[Index];	};

	bool		Onscreen( void )								{	return m_Onscreen;			};

	bool		IsSummoned( void )								{	return m_LifeDuration > 0;			};

	bool		IsUnique( void )								{	return m_IsUnique;					};

	bool		IsMimic( void )									{	return m_IsMimic;					};

	bool		HasLightPool( void )							{	return m_HasLightPool;		};

	CMaterial*	LightPoolMaterial( void )						{	return m_pLightPoolMaterial;	};

	float32		LightPoolScale( void )							{	return m_LightPoolScale;		};
	float32		LightPoolAngle( void )							{	return m_LightPoolAngle;		};

	int32		DamageResistance( EDamageType Type );			// type of resistance

	int32		FreeSpellSlot( EMagicSphere Sphere );	// sphere of magic 

	bool		HasSpell( CSpellDescription* pSpellDescription );	// spell to find

	bool		HasSpell( const std::string& SpellName );			// spell to check for
	
	bool		SpellInSlot( EMagicSphere Sphere,
							 uint32 Index )						{	return m_KnownSpells[Sphere][Index] != NULL;		};

	bool		OnAvoidanceMap( void )							{	return m_OnAvoidanceMap;					};

	CSpellDescription*	Spell( EMagicSphere Sphere,
							   uint32 Index )					{	return m_KnownSpells[Sphere][Index];		};

	CSpellDescription*	ActiveSpell( void )						{	return m_pActiveSpell;						};

	CItem*		ActiveWeapon( void )							{	return m_pActiveWeapon;						};

	uint32		Pets( void )							{	return m_pPetCharacters.size();				};

	int32		PetIndex( CCharacter* pCharacter );	// character to check for

	bool		HasPet( CCharacter* pCharacter );	// character to check for

	bool		IsDualWielding( void );

	bool		AvailableForCommand( void )				{	return ( !IsPetNearDeath() && 
																	 AIState() != KAIReturningToTown &&
																	 AIState() != KAIInTown &&
																	 Alive() );							};

	bool		AvailableForKeyCommand( void )			{	return ( AIState() != KAIReturningToTown &&
																	 AIState() != KAIInTown &&
																	 Alive() );							};

	bool		IsPetNearDeath( void )					{	return ( HasMaster() && Invincible() && HP() <= 1 );		};

	bool		Invincible( void )						{	return m_Invincible;						};

	bool		Poisoned( void )						{	return m_Poisoned;							};

	bool		IsMerchant( void )						{	return m_IsMerchant;						};

	EMerchant	MerchantType( void )				{	return m_MerchantType;						};

	bool		IsQuestGiver( void )					{	return m_IsQuestGiver;						};

	bool		RequirementMet( EStatistic Statistic,	// statistic to check
								int32 Value,			// minimum value
								bool AllowModification = kFalse );	// allow modification due to effects

	bool		InActiveRange( void )					{	return m_InActiveRange;						};

	CInventory*	Inventory( void )						{	return m_pInventory;						};

	float32		RunningSpeed( void );

	float32		WalkingSpeed( void );

	bool		FishBiting( void )						{	return m_FishBiting;						};

	bool		CaughtFish( void )						{	return m_CaughtFish;						};

	float32		CollisionRadius( void )					{	return m_CollisionRadius;					};

	bool		Running( void )							{	return m_Running;							};

	bool		CanRun( void )							{	return m_CanRun;							};

	bool		HasMaster( void )						{	return m_pMasterCharacter != NULL;			};

	CCharacter*	Master( void )							{	return m_pMasterCharacter;					};

	bool		HasPets( void )							{	return m_pPetCharacters.size() > 0;			};

	CCharacter*	Pet( uint32 Index )						{	return m_pPetCharacters[Index];				};

	bool		HasAttachment( EAttachmentPoint Point )	{	return m_AttachmentTagIndex[Point] != -1;	};

	bool		Alive( void );

	float32		HPPercentage( void )			{	if( MaxHP() == 0 ) return 0;  return (float32)HP() / (float32)MaxHP();	};

	int32		MinimumDamage( void );
	int32		MaximumDamage( void );

	int32		ElementalDamage( void );
	int32		ElementalDamage( EDamageType Type );

	float32		TransformationDuration( void )	{	return m_TransformationDuration;		};

	float32		LifeDuration( void )			{	return m_LifeDuration;					};

	int32		BaseMinimumDamage( void );
	int32		BaseMaximumDamage( void );

	int32		BaseMaxStamina( void );

	int32		MaxStamina( void );
	int32		Stamina( void );
	float32		StaminaFloat( void );

	int32		HP( void );
	float32		HPFloat( void );
	int32		MaxHP( void );
	int32		Strength( void );
	int32		Dexterity( void );
	int32		Vitality( void );
	int32		Magic( void );

	int32		Mana( void );
	int32		MaxMana( void );
	float32		ManaFloat( void );


	virtual int32		BaseMaxHP( void );
	int32		BaseStrength( void );
	int32		BaseDexterity( void );
	int32		BaseVitality( void );
	int32		BaseMagic( void );
	int32		BaseMaxMana( void );

	int32		LastDungeonLevel( void )		{	return m_LastDungeonLevel;			};

	int32		UnusedStatPoints( void )		{	return m_UnusedStatPoints;			};

	int32		UnusedSkillPoints( void )		{	return m_UnusedSkillPoints;			};

	int32		Level( void )					{	return m_Level;						};

	int32		Experience( void )				{	return m_Experience;				};

	int32		ExperienceAward( void )			{	return m_ExperienceAward;			};

	int32		Fame( void )					{	return m_Fame;						};

	int32		FameAward( void )				{	return m_FameAward;					};

	int32		FameRank( void )				{	return m_FameRank;					};

	int32		Lineage( void )					{	return m_Lineage;					};

	int32		NaturalArmor( void );

	int32		AC( void );

	int32		BaseAC( void );

	int32		BaseDexterityBonus( void );

	int32		DexterityBonus( void );

	int32		BaseArmorBonus( void );

	int32		ArmorBonus( void );

	int32		CharacterDamage( void );

	int32		BaseCharacterDamage( void );

	int32		RollWeaponDamage( void );

	bool		Blocked( void );

	int32		ToHit( void );

	int32		BaseToHit( void );

	int32		ToHitBonus( void );

	int32		BaseToHitBonus( void );

	int32		SkillPoints( ESkill Skill );

	int32		BaseSkillPoints( ESkill Skill );

	CCharacter*	Target( void )					{	return m_pTargetCharacter;			};

	CItem*	TargetItem( void )					{	return m_pTargetItem;				};

	const std::string&	Name( void )			{	return m_Name;						};

	const std::string&	MimicName( void )		{	return m_MimicName;					};

	const D3DXVECTOR3&	PortalPosition( void )	{	return m_PortalPosition;			};

	bool		ValidLineOfSight( CLevel& pLevel );	// a scene to collide with

	bool		FacingTarget( void );

	bool		FacingTarget( CCharacter* pTarget,	// target char
							  CItem* pTargetItem ); // target item

	bool		TooCloseForAttack( void );

	float32		AttackRange( void );

	bool		InAttackRange( void );
	
	bool		InAttackRange( CAttackDescription*	pCurrentAttack );	// attack to check

	bool		InStrikeRange( void );

	bool		InStrikeRange( CCharacter* pTarget,		// target
							   CItem* pTargetItem );	// target item

	bool		InInteractionRange( void );

	bool		InSpeakingRange( void );

	bool		InOperationRange( void );

	const D3DXVECTOR3&	GetMinBounds( void )	{	return m_pCullingBounds->GetLocalMinBounds();	};

	const D3DXVECTOR3&	GetMaxBounds( void )	{	return m_pCullingBounds->GetLocalMaxBounds();	};

	bool					IsPlayer( void )	{	return m_HeadIndex != -1;						};

	std::string		GetGemsCollected()			{	return m_GemsCollectedList;		};
	uint32			GetGemsCollectedNumber()	{	return m_gemTypesCollected;		};
	std::string		GetFishCaught()				{	return m_FishCaughtList;		};
	uint32			GetFishCaughtNumber()	{	return m_fishTypesCollected;		};

	//Cheat System Control / Achievements
	bool		GetDisabledCheatAchievements( void )			{	return m_disabledAchievements;		};

	bool					PerformingAttack( void );

	bool					PerformingCast( void );

	EAIState				AIState( void )				{	return m_AIState;				};

	EAlignment				Alignment( void );

	const D3DXMATRIX&		Orientation( void )			{	return m_OrientationMatrix;		};
	const D3DXMATRIX&		DisplayMatrix( void )		{	return m_DisplayMatrix;			};

	const D3DXVECTOR3&		DisplayPosition( void )		{	return  m_DisplayPosition;		};

	const D3DXVECTOR3&		Position( void )			{	return m_Position;				};

	const D3DXVECTOR3&		Up( void )					{	return m_Up;					};
	const D3DXVECTOR3&		Right( void )				{	return m_Right;					};
	const D3DXVECTOR3&		Forward( void )				{	return m_Forward;				};

	bool				DeletionRequested( void )		{	return m_DeletionRequested;	};


	bool				Pathing( void )					{	return m_Pathing;		};

	bool				HasReflection( void );
	bool				HasCubeMap( void );

	bool				InFrustum( CFrustum& pFrustum );		// frustum to cull with - NULL for none


	D3DXMATRIX			GetTransformedTag( uint32 Index );				// index of tag
	D3DXVECTOR3			GetTagPosition( uint32 Index );				// index of tag

	uint32				GetTagCount( void );

	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	const std::string&	GetTagUserData( uint32 Index );					// index of tag name to retrieve

	bool				Collideable( void )				{	return m_Collideable;	};

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


protected:

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////
	
	uint32				UnarmedAttacks( void )			{	return m_UnarmedAttacks.size();	};

	CAttackDescription*	UnarmedAttack( uint32 Index )	{	return m_UnarmedAttacks[Index];	};

	int32				ModifyDamage( int32 Damage,				// damage to modify by protection
									  EDamageType DamageType );	// type of damage being applied

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	virtual void	CalculateMaxStamina( void );

	virtual void	CalculateMaxMana( void );

	void			CalculateDropEffects( CLevel& pLevel,					// level to send events to
										  CCharacter& pTargetCharacter );	// character to calculate effects for

	void		Transform( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
						   CLevel* pLevel,						// level to send effects to
						   CCharacterTemplate& pCharacterManifest,// character manifest
						   const std::string& MonsterName,		// name of monster to transform into
						   float32 Duration,					// duration of change in seconds ( 0 for permanent )
						   bool ModifyStats );					// modify the stats?

	void		RevertTransformation( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									  CLevel& pLevel );						// level to send effects to

	void		ApplyDescription( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  CCharacterDescription& pDescription );// character description

	void		ApplyTransformationDescription( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
											    CLevel* pLevel,						// level to send effects to
											    CCharacterTemplate& pCharacterManifest,// character manifest
												CCharacterDescription& pDescription,	// character description
												bool ToNewForm,							// to a new form, or reverting?
												bool ModifyStats );						// modify the stats?

	void		VerifySpells( void );

	void		ClearSpellbook( void );

	void		RemoveDiscoveryHistory( uint32 Index );	// index within the history

	void		PerformAttack( CLevel& pLevel );	// level which has the avoidance map

	void		PerformCast( CLevel& pLevel );		// level which has the avoidance map

	void		UpdateStamina( CLevel& pLevel,			// level we are on
							  float32 TimeElapsed );	// time elapsed in seconds

	void		UpdateMana( float32 TimeElapsed );				// time elapsed in seconds

	void		UpdateHP( CLevel& pLevel,				// level to post message to
						  float32 TimeElapsed );		// time elapsed in seconds

	void		UpdateEffects( CLevel& pLevel,	// level to send event effects to
							   float32 TimeElapsed );			// time elapsed in seconds

	void		RemoveDeadEffects( void );

	void		CalculateActiveRange( const D3DXVECTOR3& FocalPoint );	// point where the camera is directed

	int32		FindRandomAnimation( const std::string& Prefix );		// prefix of animation to look for

	void		QueueBlendAnimation( const std::string& AnimationName,	//  animation to check for
									 bool Looping,						// looping anim?
									 float32 BlendTime,					// time to blend
									 float32 Speed = 1 );				// defaults to 1

	void		BlendAnimation( const std::string& AnimationName,	//  animation to check for
								bool Looping,						// looping anim?
								float32 BlendTime,					// time to blend
								float32 Speed = 1 );				// defaults to 1

	void		PlayAnimation( const std::string& AnimationName,	//  animation to check for
							   bool Looping,						// looping anim?
							   float32 Speed = 1 );					// defaults to 1

	void		QueueBlendAnimation( uint32 Index,						//  index of animation
									 bool Looping,						// looping anim?
									 float32 BlendTime,					// time to blend
									 float32 Speed = 1 );				// defaults to 1

	void		BlendAnimation( uint32 Index,						//  index of animation
								bool Looping,						// looiping anim?
								float32 BlendTime,					// time to blend
								float32 Speed = 1 );				// defaults to 1

	void		PlayAnimation( uint32 Index,						//  index of animation
							   bool Looping,						// looping anim?
							   float32 Speed = 1 );					// defaults to 1

	void		ClearQueuedAnimations( void );

	void		ClearAnimations( void );

	void		UpdateLocalLight( void );

	void		FindAttachmentPoints( void );

	void		UpdateAttachmentPoints( void );

	void		Die( CLevel& pLevel );	// level to drop items to

	void		ApplyDamage( CLevel& pLevel,		// level to send text events to
							 CCharacter* pAttacker,	// attacking character
							 float32 Damage,			// hp damage
							 bool AllowReflection = kTrue );	// allow damage reflection?

	void		SelectAttack( void );

	bool		RollAttack( CLevel& pLevel,			// level to send text events to
							CCharacter* pTarget );	// target to attack

	void		UpdateAttack( CLevel& pLevel );	// level to create pathfinder for

	void		UpdateCast( CLevel& pLevel );	// level to create pathfinder for

	void		UpdateTimers( float32 TimeElapsed, 	// time elapsed, in seconds
							  CLevel& pLevel );		// a scene to collide with

	void		UpdateTownTimer( float32 TimeElapsed, 	// time elapsed, in seconds
								 CLevel& pLevel );		// a scene to collide with

	void		Attack( void );

	void		Cast( void );

	void		ExtractOrientationVectors( void );

	void		UpdateMotion( float32 TimeElapsed, 		// time elapsed, in seconds
							  CLevel& pLevel );			// a scene to collide with

	void		UpdatePathing( float32 TimeElapsed );		// time elapsed in seconds

	void		UpdatePathNode( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	void		RenderPath( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const D3DXMATRIX& ViewportMatrix );	// viewport matrix

	void		TurnToward( const D3DXVECTOR3& TargetDirection,	// direction to face
						    float32 TimeElapsed );				// time elapsed in seconds

	void		TurnTowardPosition( const D3DXVECTOR3& TargetPosition,	// position to face
									float32 TimeElapsed );				// time elapsed in seconds
	
	void		RequestDeletion( void )		{	m_DeletionRequested = kTrue;		};
	
	void		Dispel( CLevel& pLevel );	// level to post effects to
	
	bool				m_OnGround;
	bool				m_DeletionRequested;

	float32				m_CollisionRadius;
	float32				m_VerticalOffset;

	float32				m_LifeDuration;		// lifetime of summoned creature
	float32				m_TransformationDuration;	// duration of transformation ( 0 for forever )

	float32		m_ExistenceTime;

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CSpellTemplate&				m_pSpellManifest;

	CParticleManager&			m_pParticleManager;
	CGlowManager&				m_pGlowManager;

	CPositionalAudioManager&	m_pPositionalAudioManager;

	CGenericModel*		m_pModel;
	CGenericModel*		m_pFishingPole;
	CGenericModel*		m_pFish;
	CGenericModel*		m_pUpperBodyModel;
	CGenericModel*		m_pGlovesModel;
	CGenericModel*		m_pArmorModel;
	CGenericModel*		m_pBootsModel;
	CGenericModel*		m_pHeadModel;
	CGenericModel*		m_pHairModel;

	CAStar*				m_pAStar;

	CModel*				m_pCollisionModel;
	CModel*				m_pPathNodeModel;

	CPath*				m_pPath;

	D3DXMATRIX			m_OrientationMatrix;
	D3DXMATRIX			m_DisplayMatrix;		// final orientation and translation
	D3DXVECTOR3			m_Position;
	D3DXVECTOR3			m_DisplayPosition;

	D3DXVECTOR3			m_GroundColor;

	// pathing positions
	D3DXVECTOR3			m_TargetPosition;
	D3DXVECTOR3			m_FinalTargetPosition;
	D3DXVECTOR3			m_CastingTargetPosition;

	D3DXVECTOR3			m_MinCollisionBounds;
	D3DXVECTOR3			m_MaxCollisionBounds;

	D3DXVECTOR3			m_LocalLight;

	D3DXVECTOR3			m_Up;
	D3DXVECTOR3			m_Forward;
	D3DXVECTOR3			m_Right;

	D3DXVECTOR3			m_Velocity;
	D3DXVECTOR3			m_KnockbackVelocity;

	CCullingBounds*		m_pCullingBounds;

	std::vector< CGlow* >		m_pGlowList;
	std::vector< CGlow* >		m_pModelGlowList;
	std::vector< uint32 >		m_GlowIndex;
	std::vector< D3DXVECTOR3 >	m_GlowPosition;


	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	std::vector< uint32 >		m_TagParticleIndex;
	std::vector< int32 >		m_TagParticleType;

	int32				m_DeathAnimationIndex;

	bool				m_Running;

	bool				m_CanRun;

	bool				m_Pathing;

	bool				m_Collideable;

	bool				m_OnAvoidanceMap;

	bool				m_Onscreen;

	float32				m_TurnRate;
	float32				m_WalkingSpeed;
	float32				m_RunningSpeed;

	float32				m_LastMovementSpeed;

	float32				m_ViewRadius;		// radius of view cone
	float32				m_MotionRadius;		// radius within which we can detect motion
	float32				m_AttentionRadius;	// maximum distance of sight
	float32				m_FollowRadius;		// maximum distance away at which we will follow

	float32				m_AttackTimer;

	float32				m_TownTimer;

	float32				m_CastTimer;

	float32				m_TimeSinceAvoidance;
	float32				m_TimeSincePath;
	float32				m_TimeSinceIdleSound;
	float32				m_TimeSinceGreetSound;
	float32				m_TimeSinceUniqueIdle;


	EAIState			m_AIState;

	EAlignment			m_Alignment;
	EAlignment			m_OriginalAlignment;	// alignment before becoming a pet
	EAlignment			m_PathNodeAlignment;	// alignment color of our path node

	CCharacter*			m_pTargetCharacter;	// of attacks/etc.

	CCharacter*			m_pMasterCharacter;	// master(owner) character.

	std::vector< CCharacter* >			m_pPetCharacters;	// pet characters
	bool				m_PetWasCollideable[KMaximumPossibleSummonedCreatures];

	CItem*				m_pTargetItem;	// item to pick up

	CItem*				m_pActiveWeapon;	// item currently wielding in attack

	std::string			m_AncestorName;
	std::string			m_Name;
	std::string			m_MimicName;
	std::string			m_Family;

	EGender				m_Gender;

	int32				m_HeadIndex;
	int32				m_HairIndex;

	// particles

	int32				m_BloodParticle;
	int32				m_StrikeParticle;
	int32				m_DustParticle;
	int32				m_DamageParticle[KDamageTypes];
	int32				m_DamageParticleSecondary[KDamageTypes];

	// display variation
	float32				m_Scale;

	// statistics

	int32				m_UnusedStatPoints;
	int32				m_UnusedSkillPoints;

	int32				m_LastDungeonLevel;

	int32				m_Level;
	int32				m_Experience;

	int32				m_Fame;
	int32				m_FameRank;

	int32				m_Strength;
	int32				m_OriginalStrength;
	int32				m_Dexterity;
	int32				m_OriginalDexterity;
	int32				m_Vitality;
	int32				m_OriginalVitality;
	int32				m_Magic;
	int32				m_OriginalMagic;

	float32				m_HP;
	int32				m_MaxHP;
	int32				m_MinHP;

	bool				m_PlayedStaminaLow;
	float32				m_Stamina;
	int32				m_MaxStamina;

	float32				m_Mana;
	int32				m_MaxMana;

	int32				m_ToHitBonus;
	int32				m_OriginalToHitBonus;

	int32				m_NaturalArmor;
	int32				m_OriginalNaturalArmor;

	int32				m_ExperienceAward;
	int32				m_FameAward;

	float32				m_SoundRadius;

	float32				m_ReachBonus;

	std::vector< CAttackDescription* >	m_UnarmedAttacks;

	std::vector< CEffect* >	m_pEffects[KActivationTypes];
	std::vector< CEffect* >	m_pNaturalEffects[KActivationTypes];

	CAttackDescription*	m_pCurrentAttack;

	CSoundBank*			m_pAttackSounds;
	CSoundBank*			m_pStrikeSounds;
	CSoundBank*			m_pMissSounds;
	CSoundBank*			m_pCharacterSounds;

	CInventory*			m_pInventory;

	CMaterial*			m_pOverrideMaterial;

	CMaterial*			m_pLightPoolMaterial;
	bool				m_HasLightPool;
	ELightPoolType		m_LightPoolType;
	float32				m_LightPoolRadius;
	float32				m_LightPoolScale;
	float32				m_LightPoolAngle;

	float32				m_TorchLightFlicker;

	int32				m_AttachmentTagIndex[KAttachmentPoints];
	D3DXMATRIX			m_AttachmentTagXForm[KAttachmentPoints];
	std::vector< int32 >m_MouthAttachments;

	bool				m_AllowDropEquipment;
	bool				m_AllowDropInventory;

	bool				m_IsMimic;

	bool				m_HoldPosition;

	bool				m_OnImpassableTerrain;

	bool				m_InActiveRange;

	bool				m_ReachedStrikePoint;		// has reached the strike point of an attack animation
	
	bool				m_IsMerchant;

	bool				m_IsQuestGiver;

	bool				m_LastAttackRight;

	bool				m_Invincible;

	bool				m_Poisoned;

	bool				m_BreathActive;

	bool				m_IsTurning;

	float32				m_EffectValue[KAllEffectTypes];

	std::vector< std::string >		m_UniqueIdles;

	std::vector< CLevelHistory* >	m_pDiscoveryHistory;

	std::vector< CQuest* >			m_pQuests;

	CQuest*							m_pMasterQuest;

	std::vector< std::string >		m_GreetMessages;

	int32				m_SkillPoints[KSkills];

	CSpellDescription*	m_pActiveSpell;
	CSpellDescription*	m_KnownSpells[KMagicSpheres][KMaximumSpellsPerSphere];

	CSpellDescription*	m_pCastingSpell;
	CItem*				m_pCastingItemTarget;
	CCharacter*			m_pCastingCharacterTarget;
	bool				m_ReachedCastPoint;

	int32				m_DamageResistance[KDamageTypes];

	bool				m_IsUnique;

	D3DMATERIAL8		m_D3DSilhouetteMaterial;

	ETactic				m_Tactics;

	bool				m_MouseOver;

	bool				m_MinionsCollideable;

	bool				m_FishBiting;
	bool				m_CaughtFish;
	float32				m_TimeSinceBite;
	float32				m_BiteSpeed;

	bool				m_DisplayedAsItem;		// character that does not show health/etc. ( stash/mimics )

	bool				m_HasOpenPortal;
	uint32				m_PortalDepth;
	D3DXVECTOR3			m_PortalPosition;

	float32				m_Bravery;

	std::string			m_TemplateName;
	std::string			m_OriginalTemplateName;	// before transformation

	EMerchant			m_MerchantType;

	float32				m_RepathPauseTime;

	CText*				m_pCharacterName;
	CSprite*			m_pCharacterBackdrop;

	float32				m_HelpTimer;
	std::deque< ECharacterSounds >	m_HelpQueue;
	int32				m_HelpChannel;

	bool				m_CanMatchSpeed;

	int32				m_Lineage;

	EDifficulty			m_Difficulty;

	ERank				m_Rank;

	int32				m_JournalStats[KJournalStatistics];


	int32				m_BaseLevel;		// level the monster's stats were created for

	ECharacterSounds	m_ActiveHelpSample;

	std::string			m_GemsCollectedList;
	std::string			m_FishCaughtList;
	uint32				m_gemTypesCollected;
	uint32				m_fishTypesCollected;

	//Cheat System Control / Achievements
	bool				m_disabledAchievements;
};

#endif