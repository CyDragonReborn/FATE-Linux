/*
	level.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVEL_H
#define _LEVEL_H

#include "maze.h"
#include "levelmesh.h"

#include "../ITEM/item.h"
#include "../CHARACTER/character.h"

#include "../UTILITIES/llist.h"
#include "../UTILITIES/randomizer.h"
#include "../UTILITIES/quadtree.h"

const enum EMiniMapCharacterIcon
{
	KMiniMapCharacter,
	KMiniMapNeutral,
	KMiniMapPortal,
	KMiniMapAscend,
	KMiniMapDescend,
	KMiniMapQuestGiver,
	KMiniMapQuestGiverDone,
	KMiniMapIcons
};

const std::string KMiniMapTextures[KMiniMapIcons] =
{
	"IMAGES/charactericon.png",
	"IMAGES/neutralicon.png",
	"IMAGES/portalicon.png",
	"IMAGES/ascendicon.png",
	"IMAGES/descendicon.png",
	"IMAGES/questgivericon.png",
	"IMAGES/questgiverdoneicon.png"
};

const std::string KMiniMapAlphas[KMiniMapIcons] =
{
	"IMAGES/charactericona.png",
	"IMAGES/charactericona.png",
	"IMAGES/portalicona.png",
	"IMAGES/ascendicona.png",
	"IMAGES/descendicona.png",
	"IMAGES/charactericona.png",
	"IMAGES/charactericona.png"
};

const float32 KPathingCellWidth( 1.2f );
//const uint32 KPathingCellsPerTile( floor( (float32)KDefaultCellSize / 1.2f ) );
const uint32 KVisibilityCellsPerTile( 4 );
const int32  KMaxVisibilityRadius( 9 );

const uint32 KImpassableMaterialID( 1000 );
const uint32 KFloorMaterialID( 1005 );
const uint32 KStairsUpMaterialID( 1001 );
const uint32 KStairsDownMaterialID( 1002 );
const uint32 KCharacterMaterialID( 2000 );

// 30 minutes until level is recharged
const float32 KMaximumLevelRespawnTime( 1800 );

//const uint32 KRandomizationTableSize( 5000 );

const uint32 KSmallRandomizationTableSize( 500 );
//
const enum EWorldRenderState
{
	KWRSRenderAll,
	KWRSNoRenderCharacters,
	KWRSNoRenderScene,
	KWRSNoRenderWater,
	KWRSNoRenderProps,
	KWRSNoRenderItems,
	KWRSNoRenderVisibility,
	KWRSStates
};

class CRefManager;
class CSettings;
class CMaze;
class CFrustum;
class CLevelProp;
class CTemplate;
class CLightManager;
class CLevelMesh;
class CSprite;
class CParticleManager;
class CGlowManager;
class CPositionalAudioManager;
class CLevelCollision;
class CCharacterDescription;
class CItemDescription;
class CArrowTrail;
class CEventText;
class CLevelHistory;
class CSpellTemplate;
class CSFX;
class CScene;
class CCollisionScene;
class CNameGenerator;
class CQuestDescriptions;
class CTips;
class CLoadingMenu;

// a level contains a maze, and an internal set of
// level 'cells' which make up the visual blocks of the level,
// and correspond to the layout of the maze

class CLevel
{


public:

			CLevel( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
					CRefManager& pRefManager,			// pointer to the reference manager, for tracking media
					CSettings& pSettings,				// pointer to the settings manager, for keeping track of card/machine capabilities
					CQuestDescriptions& pQuestDescriptiosn,	// random quest descriptions
					CCharacterTemplate& pCharacterManifest,	// character template for summoning
					CItemTemplate& pItemManifest,			// item list to use for generation
					CSpellTemplate& pSpellManifest,		// spell template for casting
					CParticleManager& pParticleManager,	// particle manager
					CGlowManager& pGlowManager,			// particle manager
					CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
					CNameGenerator& pNameGenerator,		// random name generator
					CTips& pTips,						// tip manager
					EDifficulty Difficulty,				// game difficulty
					const std::string& TemplatePath );	// path to template to use

			~CLevel( void );

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void			ChangeWorldRenderState( void );

	void			UpdateLevelWideVisibility( void );
			
	void			Update( float32 TimeElapsed );				// time elapsed in seconds

	void			UpdateCharacters( const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
									  float32 TimeElapsed );			// time elapsed in seconds

	void			UpdateCharacterDeaths( void );

	void			UpdateCharacterAnimation(  float32 TimeElapsed );	// time elapsed in seconds

	void			UpdateItems( float32 TimeElapsed );	// time elapsed in seconds

	void			UpdateArrowTrails( float32 TimeElapsed );	// time elapsed in seconds

	void			UpdateSFX( float32 TimeElapsed );	// time elapsed in seconds

	void			GenerateRandomItems( void );	
	
	void			GenerateRandomMonsters( void );	

	void			OutfitCharacter( CCharacterDescription& pDescription,	// character description
									 CCharacter& pCharacter,				// character to outfit
									 bool IsUnique = kFalse );			// allow unique creation?
									 
	void			GiveEquipment( CCharacterDescription& pDescription,	// character description
								   CCharacter& pCharacter );			// character to outfit

	uint32			GenerateMinions( CCharacterDescription& pDescription,	// character description
									 CCharacter& pCharacter );				// character to outfit

	CCharacter*		AddMonster( CCharacterDescription& pDescription,	// character description
								const D3DXVECTOR3 &Position );			// position to add at

	CCharacter*		AddMerchant( CCharacterDescription& pDescription,	// character description
								 const D3DXVECTOR3 &Position,			// position to add at
								 EMerchant Type,						// type of merchant
								 int32 PlayerLevel );					// player level, for selecting inventory

	CCharacter*		AddPlayer( const D3DXVECTOR3 &Position );	// position to add at

	CCharacter*		AddCharacter( CCharacter* pCharacter,			// character to add to the list
								  const D3DXVECTOR3 &Position );	// position to add at

	CItem*			AddItem( CItemDescription& pDescription,	// item description
							 const D3DXVECTOR3 &Position );		// position to add at

	void			AddItemToRandomContainer( CItem* pItem );	// item to add

	CItem*			AddItem( CItem* pItem,						// item to add
							 const D3DXVECTOR3 &Position );		// position to add at

	CItem*			AddGold( uint32 Value,						// amount of gold
							 const D3DXVECTOR3 &Position );		// position to add at

	void			AddArrowTrail( const D3DXVECTOR3& Position,		// position shot from
								   const D3DXVECTOR3& Target );	// terminating position

	CSFX*			AddSFX( CSFXDescription& pDescription,			// sfx to add
							CCharacter* pOwner,						// owner
							CCharacter* pTarget,					// target
							const D3DXVECTOR3& Position,			// startting position
							const D3DXVECTOR3& TargetPosition,		// target position
							float32 DurationBonus = 0,				// duration bonus
							CSpellDescription* pPayloadSpell = NULL );		// payload for projectile FX

	void			AddEventText( CCharacter* pParent,				// parent character
								   CCharacter* pTarget,				// target character
								  const D3DXVECTOR3& Position,		// position shot from
								  const std::string& Text,			// text
								  float32 Scale,					// scale of text
								  bool Bolded,						// text bolded?
								  float32 FadeRate,					// rate of fade
								  bool Secondary = kFalse );		// use secondary font?

	void			CalculateLocalLight( const D3DXVECTOR3& FocalPoint,	// focal point of light
										 float32 TimeElapsed,				// for smoothing
										 float32 LightRadius );			// light falloff

	void	RequestMessage( const std::string& Message );	// message to pop up

	void	RequestTip( const std::string& Message );	// message to pop up

	void	RequestEventMessage( const std::string& Message,	// message to pop up
								 const std::string& YesMessage,	// yes text
								 const std::string& NoMessage );	// no text

	void	RequestEnchantMessage( const std::string& Message,	// message to pop up
								   const D3DXVECTOR3& Position,	// position of enchanter
								   const std::string& YesMessage,	// yes text
								   const std::string& NoMessage );	// no text

	void	ClearPendingMessage( void );

	void	ClearPendingTip( void );

	void	ClearPendingEventMessage( void );

	void	ClearPendingEnchantMessage( void );

	void	RequestLevelChange( int32 Direction,			// up or down how many floors?
								bool Silent = kFalse );		// silent? ( no confirmation )

	void	RequestLevelTeleport( int32 Depth );	// new level depth

	void	UpdateProximity( const D3DXVECTOR3& Position );	// position of mobile object

	void	Create( CLoadingMenu& pLoadingMenu,	// loading menu ( so that we can update our loading bar )
					int32 Seed,			// dungeon seed
					uint32 Depth );		// level depth

	void	ClearEvents( void );

	void	UpdateVisibility( const D3DXVECTOR3& Position );	// position of light source

	bool	SightUnobstructed( const D3DXVECTOR3& Start,	// start position
							   const D3DXVECTOR3& End );	// end position

	void	Sort( CFrustum* pFrustum );						// frustum to cull with - NULL for none

	void	Render( const D3DXVECTOR3& FocalPoint,			// point where the camera is directed
					CLightManager* pLightManager,			// light manager - null for no light manager
				    const D3DXMATRIX& ViewportMatrix,		// viewport matrix
					CFrustum* pFrustum,						// frustum to cull with - NULL for none
					D3DXMATRIX* LightMatrix,				// shadow projector matrix
					CMaterial* pShadowMaterial,				// shadow material layer
					CMaterial* pLightMaterial );			// shadow material layer

	void	RenderDetail( const D3DXVECTOR3& FocalPoint,			// point where the camera is directed
							CLightManager* pLightManager,			// light manager - null for no light manager
							const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							CFrustum* pFrustum,						// frustum to cull with - NULL for none
							D3DXMATRIX* LightMatrix,				// shadow projector matrix
							CMaterial* pShadowMaterial,				// shadow material layer
							CMaterial* pLightMaterial );			// shadow material layer

	void	RenderCharacters( CLightManager* pLightManager,	// light manager - NULL for no light manager
							  const D3DXMATRIX& ViewportMatrix,// viewport matrix
							  CFrustum* pFrustum );			// frustum to cull with - NULL for none

	void	RenderCharactersBehindWall( CLightManager* pLightManager,	// light manager - NULL for no light manager
										const D3DXMATRIX& ViewportMatrix,// viewport matrix
										CFrustum* pFrustum );			// frustum to cull with - NULL for none

	void	RenderCharacterShadows( const D3DXMATRIX& ViewportMatrix,// viewport matrix
									CFrustum* pFrustum );			// frustum to cull with - NULL for none

	void	RenderItems( CLightManager* pLightManager,		// light manager - NULL for no light manager
						 const D3DXMATRIX& ViewportMatrix,	// viewport matrix
						 CFrustum* pFrustum );				// frustum to cull with - NULL for none

	void	RenderItemShadows( const D3DXMATRIX& ViewportMatrix,// viewport matrix
							   CFrustum* pFrustum );			// frustum to cull with - NULL for none

	void	RenderGlowPass( const D3DXMATRIX& ViewportMatrix );		// viewport matrix

	void	RenderProps( const D3DXVECTOR3& FocalPoint,			// point where the camera is directed
						 CLightManager* pLightManager,			// light manager - NULL for no light manager
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						 CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void	RenderShadowPassProps( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								    const D3DXMATRIX& LightMatrix,			// matrix for the shadow caster
								    CMaterial* pShadowMaterial,				// projected shadow material
									CFrustum* pFrustum,						// frustum to cull with - NULL for none
								    bool Orthographic = kFalse );					// false by default

	void	RenderPropShadows( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void	RenderPropLights( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							  CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void	RenderItemLights( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							  CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void	RenderCharacterLights( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
								   CFrustum* pFrustum );				// frustum to cull with - NULL for none

	void	RenderHardCharacterLights( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
									   CFrustum* pFrustum );				// frustum to cull with - NULL for none

	void	RenderArrowTrails( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
							   CFrustum* pFrustum );				// frustum to cull with - NULL for none

	void	RenderSFX( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
					   CFrustum* pFrustum );				// frustum to cull with - NULL for none

	void	RenderTorchLight( void );

	void	RenderShadowFade( void );

	void	RenderVisibility( const D3DXMATRIX& ViewportMatrix );		// viewport matrix

	void	RenderVisibilityMask( const D3DXMATRIX& ViewportMatrix );		// viewport matrix

	void	RenderMiniMap( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						   CCharacter* pPlayerCharacter );			// player character

	void	SortForCollision( const D3DXVECTOR3& MinBounds,		// min bounds of area to sort
							  const D3DXVECTOR3& MaxBounds );	// max bounds of area to sort

	bool	RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
						  const D3DXVECTOR3& End,		// end of collision ray
						  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
						  D3DXVECTOR3& ImpactNormal );		// impact normal to be filled

	bool	SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							 const D3DXVECTOR3& End,		// end of collision ray
							 float32 Radius,				// radius of sphere to check along collision ray
							 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							 D3DXVECTOR3& ImpactNormal );	// impact normal to be filled

	bool	RayCollision( const D3DXVECTOR3& Start,		// start of collision ray
						  const D3DXVECTOR3& End,		// end of collision ray
						  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
						  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
						  uint32& FaceMaterial,			// material id of the impacted face
						  D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
						  CCharacter** pCharacter,		// character hit?
						  bool AllowObjectCollision = kTrue	);	// collide with objects?

	bool	SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							 const D3DXVECTOR3& End,		// end of collision ray
							 float32 Radius,				// radius of sphere to check along collision ray
							 D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							 D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							 D3DXVECTOR3& ImpactNormal,		// impact normal to be filled
							 uint32& FaceMaterial,			// material id of the impacted face
							 D3DXVECTOR3& AverageColor,		// averaged colors of the face's verts
							 CCharacter** pCharacter,		// character hit?
							 bool AllowObjectCollision = kTrue	);	// collide with objects?

	bool	PreSortedRayCollision( const D3DXVECTOR3& Start,	// start of collision ray
								   const D3DXVECTOR3& End,		// end of collision ray
								   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
								   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								   uint32& FaceMaterial,		// material id of the impacted face
								   D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
								   CCharacter** pCharacter,		// character hit?
								   bool AllowObjectCollision = kTrue	);	// collide with objects?

	bool	PreSortedSphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
									  const D3DXVECTOR3& End,		// end of collision ray
									  float32 Radius,				// radius of sphere to check along collision ray
									  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
									  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									  uint32& FaceMaterial,			// material id of the impacted face
									  D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
									  CCharacter** pCharacter,		// character hit?
									  bool AllowObjectCollision = kTrue	);	// collide with objects?


	bool	ObjectSphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
									const D3DXVECTOR3& End,		// end of collision ray
									float32 Radius,				// radius of sphere to check along collision ray
									D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									uint32& FaceMaterial,		// material id of the impacted face
									CCharacter** pCharacter );	// character hit?

	bool	ObjectRayCollision( const D3DXVECTOR3& Start,	// start of collision ray
								const D3DXVECTOR3& End,		// end of collision ray
								D3DXVECTOR3& ImpactPoint,	// impact point to be filled
								D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								uint32& FaceMaterial,		// material id of the impacted face
								CCharacter** pCharacter );	// character hit?

	bool	CharacterRayCollision( const D3DXVECTOR3& Start,	// start of collision ray
								   const D3DXVECTOR3& End,		// end of collision ray
								   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
								   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								   uint32& FaceMaterial,		// material id of the impacted face
								   CCharacter** pCharacter );	// character hit?

	void	CreateRandomizationTables( int32 PlayerLevel );// level of player for quest item generation

	void	FindCharacters( CCharacter* pPlayerCharacter );	// player character

	void	NotifyOfDeath( CCharacter* pSelectedCharacter );	// Character which has died

	CQuest* GenerateQuest( CQuestDescriptions& pQuestDescriptions,	// random quest descriptions
						   const std::string& GiverName,			// name of quest giver
						   uint32 Level );							// level (difficulty) of quest

	void	DumpToFile( void );

	void	DiscoverAll( void );

	void	SetVisibility( const D3DXVECTOR3& Position,	// worldspace position
						   bool Value );				// value to set

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	const D3DXVECTOR3&	StartPosition( void )				{	return m_StartPosition;	};

	EDifficulty		Difficulty( void )						{	return m_Difficulty;	};

	CTips&			Tips( void )							{	return m_pTips;								};

	CNameGenerator&	NameGenerator( void )					{	return m_pNameGenerator;					};

	const D3DXVECTOR3&	EnchantPosition( void )				{	return m_EnchantPosition;					};

	bool			TipPending( void )						{	return m_RequestingTip;					};

	bool			MessagePending( void )					{	return m_RequestingMessage;					};

	bool			EventMessagePending( void )				{	return m_RequestingEventMessage;			};

	bool			EnchantMessagePending( void )			{	return m_RequestingEnchantMessage;			};

	const std::string& PendingTipText( void )				{	return m_TipText[0];		};

	const std::string& PendingMessageText( void )			{	return m_MessageText;						};

	const std::string& YesMessageText( void )				{	return m_YesMessageText;					};
	const std::string& NoMessageText( void )				{	return m_NoMessageText;						};

	const std::string&	LevelChangeMessage( void )			{	return m_LevelChangeMessage;				};

	uint32			RandomPotion( void )					{	return m_PotionRandomizer.GetRandom();		};

	uint32			RandomBarrel( void )					{	return m_BarrelRandomizer.GetRandom();		};

	uint32			RandomContainer( void )					{	return m_ContainerRandomizer.GetRandom();	};

	uint32			RandomEventItemIndex( void )			{	return m_EventRandomizer.GetRandom();		};

	uint32			RandomMonsterIndex( void )				{	return m_MonsterRandomizer.GetRandom();		};

	uint32			RandomUniqueMonsterIndex( void )		{	return m_UniqueMonsterRandomizer.GetRandom();};

	uint32			RandomTakeableItemIndex( void )			{	return m_TakeableItemRandomizer.GetRandom();};

	uint32			RandomRareTakeableItemIndex( void )		{	return m_RareTakeableItemRandomizer.GetRandom();};

	uint32			RandomWeaponItemIndex( void )			{	return m_WeaponRandomizer.GetRandom();		};

	uint32			RandomRareWeaponItemIndex( void )		{	return m_RareWeaponRandomizer.GetRandom();		};

	uint32			RandomArmorItemIndex( void )			{	return m_ArmorRandomizer.GetRandom();		};

	uint32			RandomShieldItemIndex( void )			{	return m_ShieldRandomizer.GetRandom();		};

	uint32			RandomRareShieldItemIndex( void )		{	return m_RareShieldRandomizer.GetRandom();		};

	uint32			RandomQuestItemIndex( void )			{	return m_QuestRandomizer.GetRandom();		};

	uint32			RandomFishItemIndex( void )				{	return m_FishRandomizer.GetRandom();		};

	uint32			RandomJewelryItemIndex( void )			{	return m_JewelryRandomizer.GetRandom();		};

	uint32			RandomGemItemIndex( void )				{	return m_GemRandomizer.GetRandom();			};

	uint32			RandomRareGemItemIndex( void )			{	return m_RareGemRandomizer.GetRandom();			};

	CCharacterTemplate*	CharacterManifest( void )			{	return &m_pCharacterManifest;	};

	CItemTemplate*	ItemManifest( void )					{	return &m_pItemManifest;	};

	CSpellTemplate*	SpellManifest( void )					{	return &m_pSpellManifest;			};

	CLevelHistory*	LevelHistory( void )					{	return m_pLevelHistory;			};

	bool			HasLevelHistory( void )				{	return m_pLevelHistory != NULL;	};

	bool			RequestingLevelChange( void )			{	return m_LevelChangeRequest != LevelDepth();	};

	bool			RequestingTeleport( void )				{	return m_RequestingTeleport;		};

	bool			IsLevelChangeSilent( void )				{	return m_LevelChangeSilent;			};

	int32			RequestedLevelChange( void )			{	return m_LevelChangeRequest;		};

	int32			LevelDepth( void )						{	return m_LevelDepth;			};

	bool			CharacterExists( CCharacter* pSelectedCharacter );	// charater to check for

	bool			ItemExists( CItem* pSelectedItem );	// item to check for

	void			RemoveItem( CItem* pSelectedItem );	// item to remove

	void			DeleteItem( CItem* pSelectedItem );	// item to delete

	void			DeletePortals( void );

	void			RemoveCharacter( CCharacter* pSelectedCharacter );	// charater to check for

	void			DeleteCharacter( CCharacter* pSelectedCharacter );	// charater to check for

	CCharacter*		FindCharacterAtScreenCoordinates( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
													  const D3DXMATRIX& ProjectionMatrix,// projection matrix
													  float32 ScreenX,					// screen coordinates in worldspace
													  float32 ScreenY,					// screen coordinates in worldspace
													  CCharacter* pIgnoreCharacter );	// character to ignore

	void			FindCharactersInArea( const D3DXVECTOR3& Position,						// centerpoint
										  float32 Range,									// radius
										  std::vector< CCharacter* >&	CharacterList );		// list to add characters to

	CItem*			FindItemAtScreenCoordinates( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
												 const D3DXMATRIX& ProjectionMatrix,// projection matrix
												 float32 ScreenX,					// screen coordinates in worldspace
												 float32 ScreenY );					// screen coordinates in worldspace
												 
	CCharacter*		FindCharacterWithinView( const D3DXMATRIX& Transformation,	// position and direction
											 EAlignment Alignment,				// alignment the character must be
											 float32 MotionRadius,				// valid motion sense radius
											 float32 ConeRadius,				// cone of view radius in degrees
											 float32 Radius );					// valid radius

	CItem*			FindDestructibleItemWithinView( const D3DXMATRIX& Transformation,	// position and direction
													float32 ConeRadius,					// cone of view radius, in degrees
													float32 Radius );					// valid sight radius

	void			FindItemsOnscreen( const D3DXMATRIX& ViewportMatrix,	 // viewport matrix
									   const D3DXMATRIX& ProjectionMatrix ); // projection matrix

	void			FindCharactersOnscreen( const D3DXMATRIX& ViewportMatrix,	 // viewport matrix
										    const D3DXMATRIX& ProjectionMatrix ); // projection matrix

	uint32			PatchesRendered( void )	{	if( m_pLevelMesh != NULL )
												{	
													return m_pLevelMesh->PatchesRendered();	
												}
												else
												{
													return 0;
												}
											};

	uint32			PropsRendered( void )	{	return m_PropsRendered;					};

	CTemplate*		Template( void )		{	return m_pTemplate;	};

	CMaze*			Maze( void )			{	return m_pMaze;		};

	float32			TopLeftX( void );

	float32			TopLeftY( void );

	float32			WaterHeight( const D3DXVECTOR3& Position ); // position to check at

	float32			FloorHeight( const D3DXVECTOR3& Position ); // position to check at

	float32			FloorHeight( float32 WorldspaceX,		// x worldspace position
								 float32 WorldspaceY );		// y worldspace position

	float32			FloorOffsetX( const D3DXVECTOR3& Position ); // position to check at

	float32			FloorOffsetX( float32 WorldspaceX,		// x worldspace position
								  float32 WorldspaceY );		// y worldspace position

	float32			FloorOffsetY( const D3DXVECTOR3& Position ); // position to check at

	float32			FloorOffsetY( float32 WorldspaceX,		// x worldspace position
								  float32 WorldspaceY );		// y worldspace position

	D3DXVECTOR3		FloorNormal( const D3DXVECTOR3& Position ); // position to check at

	float32			TileWidth( void )			{	return m_TileWidth;				};

	float32			TileHeight( void )			{	return m_TileHeight;			}

	float32			VisibilityCellWidth( void ) {	return m_VisibilityCellWidth;	};

	float32			PassabilityCellWidth( void ){	return m_CellWidth;				};

	int16**			MapPassability( void )		{	return m_MapPassabilityData;	};

	int16**			ObjectPassability( void )	{	return m_ObjectPassabilityData;	};

	uint32			PassabilityWidth( void )	{	return m_PathingCellsWide;		};

	uint32			PassabilityHeight( void )	{	return m_PathingCellsHigh;		};

	uint32			VisibilityWidth( void )		{	return m_VisibilityCellsWide;	};

	uint32			VisibilityHeight( void )	{	return m_VisibilityCellsHigh;	};

	D3DXVECTOR3		PositionFromCellPosition( uint32 X,		// x cell location
											  uint32 Y );	// y cell location

	D3DXVECTOR3		PositionNearStairsUp( void );

	D3DXVECTOR3		PositionNearStairsDown( void );

	D3DXVECTOR3		RandomOpenPosition( void );

	D3DXVECTOR3		RandomOpenPositionNearWall( void );

	D3DXVECTOR3		RandomOpenPositionInRoom( void );

	D3DXVECTOR3		RandomOpenPosition( const D3DXVECTOR3& Position,	// generation position
										float32 Radius );				// random radius

	D3DXVECTOR3		RandomOpenPosition( const D3DXVECTOR3& Position,	// generation position
										float32 MinimumRadius,			// random radius
										float32 MaximumRadius );		// random radius

	D3DXVECTOR3		RandomOpenItemPosition( const D3DXVECTOR3& Position,	// generation position
											float32 Radius );				// random radius

	bool			PositionPassable( const D3DXVECTOR3& Position );// position to check

	bool			PassableBetween( D3DXVECTOR3 StartPosition,			// position to check
									 const D3DXVECTOR3& EndPosition );	// position to check
	bool			MapPassable( const D3DXVECTOR3& Position );		// position to check

	ECellType		PositionType( const D3DXVECTOR3& Position );	// position to check

	int32			PositionCellHeight( const D3DXVECTOR3& Position );	// position to check

	void			DecrementObjectPassability( const D3DXVECTOR3& Position );	// position to check

	void			IncrementObjectPassability( const D3DXVECTOR3& Position );	// position to check

	ETerrainType	MiniMapTypeNW( int32 X,		// x location of cell to retrieve
								   int32 Y );	// y location of cell to retrieve

	ETerrainType	MiniMapTypeNE( int32 X,		// x location of cell to retrieve
								   int32 Y );	// y location of cell to retrieve

	ETerrainType	MiniMapTypeSW( int32 X,		// x location of cell to retrieve
								   int32 Y );	// y location of cell to retrieve

	ETerrainType	MiniMapTypeSE( int32 X,		// x location of cell to retrieve
								   int32 Y );	// y location of cell to retrieve

	bool	Visible( int32 X,	// visibility cell to check x
					 int32 Y );	// visibility cell to check y	

	bool	FogVisible( int32 X,	// visibility cell to check x
						int32 Y );	// visibility cell to check y	

	float32	VisibilityLevel( int32 X,	// visibility cell to check x
							 int32 Y );	// visibility cell to check y	

	CLList<CItem>*	OnscreenItems( void )				{	return m_pOnscreenItems;		};

	CLList<CCharacter>*	OnscreenCharacters( void )		{	return m_pOnscreenCharacters;	};

	CLList<CEventText>*	TextEvents( void )		{	return m_pEventTextList;	};

	const std::string&	DebugText( void )		{	return m_DebugText;			};

	bool			IsTorchlit( void )			{	return m_Torchlit;			};

	LPDIRECT3DDEVICE8 D3DDevice( void )			{	return m_pD3DDevice;		};

	CLList<CItem>*		Items( void )			{	return m_pItemList;			};
	CLList<CCharacter>*	Characters( void )		{	return m_pCharacterList;			};

	CLList<CSFX>*		SFXList( void )			{	return m_pSFXList;			};

private:

	void	NotifyOfDeletion( CCharacter* pDeletedCharacter );	// character being deleted

	void	NotifyOfDeletion( CItem* pDeletedItem );			// item being deleted

	void	SetFogVisibility( int32 X,					// visibility cell to check x
							  int32 Y,					// visibility cell to check y	
							  bool Value );				// visibility value

	void	SetFogVisibility( const D3DXVECTOR3& Position,	// worldspace position
							  bool Value );					// value to set

	void	CreateLevelAudio( void );

	void	CreateProps( void );

	void	CreatePropCollision( void );

	void	CalculateGroundCollision( void );

	void	CalculatePassability( void );

	void	ClearPassabilityData( void );

	void	ClearVisibilityData( void );

	void	ClearFogVisibilityData( void );

	void	FindParticles( void );

	void	EmitParticles( void );

	void	AddWall( uint32 WallType,				// wallset being used
					 EWallType Type,				// type of model to add
					 uint32 SubType,				// variation
					 const D3DXVECTOR3& Position );	// worldspace position

	void	AddProp( CPropInstance& pPropInstance,	// prop being used
					 D3DXVECTOR3 Position );		// worldspace position

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CNameGenerator&		m_pNameGenerator;

	CParticleManager&	m_pParticleManager;

	CQuestDescriptions&	m_pQuestDescriptions;

	CGlowManager&		m_pGlowManager;

	CSpellTemplate&		m_pSpellManifest;
	
	CCharacterTemplate&	m_pCharacterManifest;

	CItemTemplate&		m_pItemManifest;

	CPositionalAudioManager&	m_pPositionalAudioManager;

	CTemplate*			m_pTemplate;

	CMaze*				m_pMaze;

	CLevelMesh*			m_pLevelMesh;

	CScene*				m_pLevelScene;
	CCollisionScene*	m_pLevelCollisionScene;

	CLevelCollision*	m_pLevelCollision;

	float32				m_CellWidth;

	float32				m_VisibilityCellWidth;

	CQuadtree< CLevelProp* >*	m_pPropQuadTree;

	std::vector< CLevelProp* >	m_pLevelProps;

	std::vector< CLevelProp* >	m_pSortedProps;

	int16**		m_MapPassabilityData;
	int16**		m_ObjectPassabilityData;

	CLevelHistory*	m_pLevelHistory;

	bool		m_RequestingTip;
	bool		m_RequestingMessage;
	bool		m_RequestingEventMessage;
	bool		m_RequestingEnchantMessage;
	D3DXVECTOR3	m_EnchantPosition;
	std::vector< std::string >	m_TipText;
	std::string	m_MessageText;
	std::string	m_YesMessageText;
	std::string	m_NoMessageText;

	uint32		m_PathingCellsWide;
	uint32		m_PathingCellsHigh;

	uint32		m_VisibilityCellsWide;
	uint32		m_VisibilityCellsHigh;

	uint32		m_PathingCellsPerTile;

	int32		m_LevelDepth;

	int32		m_LevelChangeRequest;

	bool		m_LevelChangeSilent;

	bool		m_RequestingTeleport;

	std::vector < uint32 >		m_SortedFaces;		// a list of indices to all faces that 

	D3DXVECTOR3	m_VisibilityCenter;

	bool		m_VisibilityChanged;

	bool		m_Torchlit;

	float32		m_TileWidth;
	float32		m_TileHeight;

	uint32		m_PropsRendered;
	uint32		m_CharactersRendered;
	uint32		m_ItemsRendered;

	D3DXMATRIX	m_ProjectorBias;

	CMaterial*	m_pTorchLight;
	CMaterial*	m_pEnemyTorchLight;
	CMaterial*	m_pEnemyTorchHardLight;
	CMaterial*	m_pShadowFade;
	float32		m_TorchLightFlicker;
	CSprite*	m_pTorchLightSprite;

	CModel*		m_pArrowTrailModel;
	CMaterial*	m_pArrowTrailMaterial;

	std::string m_DebugText;

	CLList<CCharacter>*	m_pCharacterList;	// list of pointers to all active characters, player and monster

	CLList<CItem>*	m_pItemList;		// list of pointers to all active items

	CLList<CItem>*	m_pOnscreenItems;	// list of pointers to items onscreen

	CLList<CCharacter>*	m_pOnscreenCharacters;	// list of pointers to characters onscreen

	CLList<CArrowTrail>*	m_pArrowTrailList;	// list of pointers to all active arrow trails

	CLList<CEventText>*		m_pEventTextList;	// list of pointers to all floating text events

	CLList<CSFX>*			m_pSFXList;	// list of pointers to all active arrow trails

	LPDIRECT3DDEVICE8 m_pD3DDevice;

	EWorldRenderState		m_WorldRenderState;

	CRandomizer m_PotionRandomizer;
	CRandomizer m_BarrelRandomizer;
	CRandomizer m_ContainerRandomizer;
	CRandomizer m_MonsterRandomizer;
	CRandomizer m_UniqueMonsterRandomizer;
	CRandomizer m_QuestRandomizer;
	CRandomizer m_TakeableItemRandomizer;
	CRandomizer m_RareTakeableItemRandomizer;
	CRandomizer m_RareWeaponRandomizer;
	CRandomizer m_WeaponRandomizer;
	CRandomizer m_EventRandomizer;
	CRandomizer m_ArmorRandomizer;
	CRandomizer m_ShieldRandomizer;
	CRandomizer m_RareShieldRandomizer;
	CRandomizer m_FishRandomizer;
	CRandomizer m_JewelryRandomizer;
	CRandomizer m_GemRandomizer;
	CRandomizer m_RareGemRandomizer;

	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	std::string					m_LevelChangeMessage;

	CSprite*			m_pMiniMapIconSprite;
	CMaterial*			m_pMiniMapIconMaterial[KMiniMapIcons];

	CFrustum		m_PropFrustum;

	bool**			m_VisibilityData;

	CTips&			m_pTips;

	EDifficulty		m_Difficulty;

	D3DXVECTOR3		m_StartPosition;
};

#endif