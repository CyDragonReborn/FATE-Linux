/*
	level.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <assert.h>
#include <D3D8.h>
#include <algorithm>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/capabilities.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/quadtree.h"
#include "../UTILITIES/loadlocalizedstring.h"

#include "../UTILITIES/llist.h"

#include "../UI/sprite.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../SCENE/scene.h"
#include "../SCENE/collisionscene.h"

#include "../MODEL/facelist.h"
#include "../GAMEOBJECTS/genericmodel.h"

#include "../GAMEOBJECTS/lightmanager.h"

#include "../PARTICLES/particlemanager.h"

#include "../GLOWS/glowmanager.h"

#include "../POSITIONALAUDIO/positionalaudiomanager.h"

#include "../ITEM/item.h"
#include "../ITEM/itemblank.h"
#include "../ITEM/itemgold.h"

#include "../CHARACTER/charactertemplate.h"
#include "../CHARACTER/character.h"
#include "../CHARACTER/player.h"
#include "../CHARACTER/monster.h"
#include "../CHARACTER/inventory.h"

#include "../EFFECTS/arrowtrail.h"
#include "../EFFECTS/eventtext.h"
#include "../EFFECTS/sfx.h"

#include "../NAMES/namegenerator.h"

#include "../MENU/loadingmenu.h"

#include "../GAMECLIENT/quest.h"
#include "../GAMECLIENT/questdescriptions.h"
#include "../GAMECLIENT/tips.h"

#include "maze.h"
#include "levelprop.h"
#include "levelcollision.h"
#include "level.h"
#include "levelmesh.h"
#include "levelhistory.h"
#include "template.h"

#include "../GAMECLIENT/demorules.h" //marsh

#include "../UTILITIES/vistafileutilities.h"

struct SortItem
{
     bool operator()( CItem* Start, CItem* End)
     {
  
		    if( Start->Category() > End->Category() )
			{
				return kTrue;
			}
			else if( Start->Category() < End->Category() )
			{
				return kFalse;
			}
		    if( Start->SlotsTall() > End->SlotsTall() )
			{
				return kTrue;
			}
			else if( Start->SlotsTall() < End->SlotsTall() )
			{
				return kFalse;
			}
		    if( Start->Type() > End->Type() )
			{
				return kTrue;
			}
			else if( Start->Type() < End->Type() )
			{
				return kFalse;
			}

			if( !( Start->BaseModifiedName() == End->BaseModifiedName() ) )
			{
				if( Start->BaseModifiedName() > End->BaseModifiedName() )
				{
					return kTrue;
				}
			}
			return false;
     }
};


CLevel::CLevel( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
			    CRefManager& pRefManager,			// pointer to the reference manager, for tracking media
				CSettings& pSettings,				// pointer to the settings manager, for keeping track of card/machine capabilities
				CQuestDescriptions& pQuestDescriptions,	// random quest descriptions
				CCharacterTemplate& pCharacterManifest,	// character template for summoning
				CItemTemplate& pItemManifest,			// item list to use for generation
				CSpellTemplate& pSpellManifest,		// spell template for casting
				CParticleManager& pParticleManager,	// particle manager
				CGlowManager& pGlowManager,			// glow manager
				CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
				CNameGenerator& pNameGenerator,		// random name generator
				CTips& pTips,						// tip manager
				EDifficulty Difficulty,				// game difficulty
				const std::string& TemplatePath ) :	// path to template to use
													m_pD3DDevice( pD3DDevice ),
													m_pTips( pTips ),
													m_Difficulty( Difficulty ),
													m_pRefManager( pRefManager ),
													m_pCharacterManifest( pCharacterManifest ),
													m_pSettings( pSettings ),
													m_pParticleManager( pParticleManager ),
													m_pGlowManager( pGlowManager ),
													m_pQuestDescriptions( pQuestDescriptions ),
													m_pSpellManifest( pSpellManifest ),
													m_pItemManifest( pItemManifest ),
													m_pPositionalAudioManager( pPositionalAudioManager ),
													m_pNameGenerator( pNameGenerator ),
													m_VisibilityChanged( kFalse ),
													m_WorldRenderState( KWRSRenderAll ),
													m_Torchlit( kTrue ),
													m_RequestingTeleport( kFalse ),
													m_pMaze( NULL ),
													m_pTemplate( NULL ),
													m_pPropQuadTree( NULL ),
													m_pLevelCollision( NULL ),
													m_MapPassabilityData( NULL ),
													m_ObjectPassabilityData( NULL ),
													m_pLevelHistory( NULL ),
													m_pLevelMesh( NULL ),
													m_pLevelScene( NULL ),
													m_pLevelCollisionScene( NULL ),
													m_LevelChangeSilent( kFalse ),
													m_RequestingMessage( kFalse ),
													m_RequestingTip( kFalse ),
													m_RequestingEventMessage( kFalse ),
													m_RequestingEnchantMessage( kFalse ),
													m_VisibilityData( NULL ),
													m_VisibilityCenter( -999, -999, -999 ),
													m_CellWidth( 16 ),
													m_VisibilityCellWidth( 16 ),
													m_PropsRendered( 0 ),
													m_ItemsRendered( 0 ),
													m_CharactersRendered( 0 ),
													m_TorchLightFlicker( 0 ),
													m_LevelChangeRequest( 0 ),
													m_LevelDepth( 0 ),
													m_pTorchLight( NULL ),
													m_pEnemyTorchLight( NULL ),
													m_pEnemyTorchHardLight( NULL ),
													m_pShadowFade( NULL ),
													m_pArrowTrailMaterial( NULL ),
													m_pMiniMapIconSprite( NULL ),
													m_pTorchLightSprite( NULL ),
													m_pArrowTrailModel( NULL ),
													m_PathingCellsWide( 0 ),
													m_PathingCellsHigh( 0 ),
													m_VisibilityCellsWide( 0 ),
													m_VisibilityCellsHigh( 0 ),
													m_PathingCellsPerTile( 0 ),
													m_DebugText( "" ),
													m_LevelChangeMessage( "" ),
													m_MessageText( "" ),
													m_YesMessageText( "" ),
													m_NoMessageText( "" ),
													m_TileWidth ( 0.0f ),
													m_TileHeight ( 0.0f )
{
	ZeroMemory ( &m_PropFrustum, sizeof ( m_PropFrustum ));
	ZeroMemory ( &m_ProjectorBias, sizeof ( m_ProjectorBias ));
	ZeroMemory ( &m_StartPosition, sizeof ( m_StartPosition ));
	ZeroMemory ( &m_EnchantPosition, sizeof ( m_EnchantPosition ));

	m_pCharacterList = new CLList< CCharacter >();
	m_pItemList = new CLList< CItem >();
	m_pOnscreenItems = new CLList< CItem >();
	m_pOnscreenCharacters = new CLList< CCharacter >();
	m_pArrowTrailList = new CLList< CArrowTrail >();
	m_pEventTextList = new CLList< CEventText >();
	m_pSFXList = new CLList< CSFX >();


	if( TemplatePath.length() > 0 )
	{
		m_pTemplate = new CTemplate( m_pD3DDevice, pRefManager, TemplatePath );
		m_TileWidth = m_pTemplate->TileWidth();
		m_TileHeight = m_pTemplate->TileHeight();

		m_PathingCellsPerTile = (uint32)floor( m_pTemplate->TileWidth() / KPathingCellWidth );

		m_CellWidth = m_TileWidth / m_PathingCellsPerTile;
		m_VisibilityCellWidth = m_pTemplate->TileWidth() / KVisibilityCellsPerTile;
	}
	else
	{
		m_TileWidth = 8;
		m_TileHeight = 10;
		m_CellWidth = KPathingCellWidth;
	}

	m_pArrowTrailModel = m_pRefManager.AddModel( m_pD3DDevice,
												 "EFFECTS/arrowtrail.mdl",
												 "",
												 kTrue,
												 kTrue );
	m_pArrowTrailMaterial = m_pRefManager.AddMaterial( m_pD3DDevice, 
														"",
														"",
														"EFFECTS/arrowtrail.png",
														"",
														"" );

	for( uint32 i = 0; i < KMiniMapIcons; i++ )
	{
		m_pMiniMapIconMaterial[i] = m_pRefManager.AddMaterialAlpha( pD3DDevice, 
																	KMiniMapTextures[i],
																	KMiniMapAlphas[i] );
	}

	m_pMiniMapIconSprite = new CSprite( pD3DDevice, 
										m_pMiniMapIconMaterial[0],
									    8,
									    8,
									    24,
									    16,
									    0,
									    0,
									    1,
									    1,
									    KSpriteCenter );

	// no texture compression on the primary torchlight - looks too yucky.
	m_pTorchLight = m_pRefManager.AddMaterial( m_pD3DDevice, "IMAGES//torchlight.png", "", "", "", "", kFalse, kFalse );
	m_pEnemyTorchLight = m_pRefManager.AddMaterial( m_pD3DDevice, "IMAGES//redlight.png" );
	m_pEnemyTorchHardLight = m_pRefManager.AddMaterial( m_pD3DDevice, "IMAGES//redshadow.png" );
	m_pShadowFade = m_pRefManager.AddMaterialAlpha( m_pD3DDevice, 
													"IMAGES//shadowfade.png",
													"IMAGES//shadowfadea.png" );
	m_pTorchLightSprite = new CSprite( m_pD3DDevice, 
									   m_pTorchLight,
									   128,
									   128,
									   1,
									   1,
									   0,
									   0,
									   1,
									   1,
									   KSpriteCenter,
									   256,
									   256 );

	// set up a projector bias for use in the default reflection projections
	float32	Offset	= .5f + ( .5f / 256 );

	m_ProjectorBias._11 = 0.5f; 
	m_ProjectorBias._12 = 0.0f; 
	m_ProjectorBias._13 = 0.0f; 
	m_ProjectorBias._14 = 0;

	m_ProjectorBias._21 = 0.0f; 
	m_ProjectorBias._22 = -0.5f; 
	m_ProjectorBias._23 = 0.0f; 
	m_ProjectorBias._24 = 0;

	m_ProjectorBias._31 = 0.0f; 
	m_ProjectorBias._32 = 0.0f; 
	m_ProjectorBias._33 = 1.0f; 
	m_ProjectorBias._34 = 0;

	m_ProjectorBias._41 = Offset; 
	m_ProjectorBias._42 = Offset; 
	m_ProjectorBias._43 = 0.0f; 
	m_ProjectorBias._44 = 1.0f;

} // CLevel::CLevel()

CLevel::~CLevel( void )
{

	if( m_VisibilityData != NULL )
	{
		for( uint32 i = 0; i < m_VisibilityCellsWide; i++ )
		{
			DELETE_ARRAY_SAFELY( m_VisibilityData[i] );
		}
	}
	DELETE_ARRAY_SAFELY( m_VisibilityData );

	m_pOnscreenItems->Clear();
	m_pOnscreenCharacters->Clear();

	DELETE_SAFELY( m_pMiniMapIconSprite );
	DELETE_SAFELY( m_pEventTextList );
	DELETE_SAFELY( m_pSFXList );
	DELETE_SAFELY( m_pArrowTrailList );
	DELETE_SAFELY( m_pCharacterList );
	DELETE_SAFELY( m_pItemList );
	DELETE_SAFELY( m_pOnscreenItems );
	DELETE_SAFELY( m_pOnscreenCharacters );

	m_pRefManager.RemoveModel( m_pArrowTrailModel );
	m_pRefManager.RemoveMaterial( m_pArrowTrailMaterial );

	m_pRefManager.RemoveMaterial( m_pTorchLight );
	m_pRefManager.RemoveMaterial( m_pEnemyTorchLight );
	m_pRefManager.RemoveMaterial( m_pEnemyTorchHardLight );
	m_pRefManager.RemoveMaterial( m_pShadowFade );

	for( uint32 i = 0; i < KMiniMapIcons; i++ )
	{
		m_pRefManager.RemoveMaterial( m_pMiniMapIconMaterial[i] );
		m_pMiniMapIconMaterial[i] = NULL;
	}

	DELETE_SAFELY( m_pTorchLightSprite );

	for( int32 i = 0; i < m_pLevelProps.size(); i++ )
	{
		DELETE_SAFELY( m_pLevelProps[i] );
	}
	m_pLevelProps.clear();

	m_pSortedProps.clear();

	m_pRefManager.RemoveScene( m_pLevelScene );
	m_pRefManager.RemoveCollisionScene( m_pLevelCollisionScene );

	DELETE_SAFELY( m_pLevelMesh );
	for( int32 i = 0; i < m_PathingCellsWide; i++ )
	{
		DELETE_ARRAY_SAFELY( m_MapPassabilityData[i] );
		DELETE_ARRAY_SAFELY( m_ObjectPassabilityData[i] );
	}

	DELETE_SAFELY( m_pLevelHistory );

	DELETE_ARRAY_SAFELY( m_MapPassabilityData );
	DELETE_ARRAY_SAFELY( m_ObjectPassabilityData );
	DELETE_SAFELY( m_pMaze );
	DELETE_SAFELY( m_pTemplate );
	DELETE_SAFELY( m_pPropQuadTree );
	DELETE_SAFELY( m_pLevelCollision );
} // CLevel::~CLevel()

void CLevel::ChangeWorldRenderState( void )
{
	uint32 RS = (uint32)m_WorldRenderState;
	RS++;
	if( RS >= KWRSStates )
	{
		RS = 0;
	}
	m_WorldRenderState = (EWorldRenderState)RS;
} // CLevel::ChangeWorldRenderState()

void CLevel::RequestMessage( const std::string& Message )
{
	m_MessageText = Message;
	m_RequestingMessage = kTrue;
} // CLevel::RequestMessage()

void CLevel::ClearPendingMessage( void )
{
	m_RequestingMessage = kFalse;
} // CLevel::ClearPendingMessage()

void CLevel::RequestTip( const std::string& Message )
{
	m_TipText.push_back( Message );
	m_RequestingTip = kTrue;
} // CLevel::RequestTip()

void CLevel::ClearPendingTip( void )
{
	if( m_TipText.size() > 1 )
	{
		for( uint32 i = 0; i < m_TipText.size() - 1; i++ )
		{
			m_TipText[i] = m_TipText[i + 1];
		}
	}
	if( m_TipText.size() > 0 )
	{
		m_TipText.pop_back();
	}
	if( m_TipText.size() == 0 )
	{
		m_RequestingTip = kFalse;
	}
} // CLevel::ClearPendingTip()


void CLevel::RequestEventMessage( const std::string& Message,	// message to pop up
								  const std::string& YesMessage,	// yes text
								  const std::string& NoMessage )	// no text
{
	m_MessageText = Message;
	m_YesMessageText = YesMessage;
	m_NoMessageText = NoMessage;
	m_RequestingEventMessage = kTrue;
} // CLevel::RequestEventMessage()

void CLevel::ClearPendingEventMessage( void )
{
	m_RequestingEventMessage = kFalse;
} // CLevel::ClearPendingEventMessage()

void CLevel::RequestEnchantMessage( const std::string& Message,	// message to pop up
								    const D3DXVECTOR3& Position,	// position of enchanter
								    const std::string& YesMessage,	// yes text
								    const std::string& NoMessage )	// no text
{
	m_MessageText = Message;
	m_YesMessageText = YesMessage;
	m_NoMessageText = NoMessage;
	m_RequestingEnchantMessage = kTrue;
	m_EnchantPosition = Position;
} // CLevel::RequestEnchantMessage()

void CLevel::ClearPendingEnchantMessage( void )
{
	m_RequestingEnchantMessage = kFalse;
} // CLevel::ClearPendingEnchantMessage()


void CLevel::RequestLevelChange( int32 Direction,	// go up or down?
								 bool Silent )		// silent? ( no confirmation )
{
		
		

		if( IsGameUnlocked() ||		//marsh - check for demo mode
			Direction <= 0 ||		//going up. Always can go up
		    ( Direction > 0 && ( ( LevelDepth() + Direction ) <= KDemoDungeonLevel ) ) ) //I'm going down and I'm below the current level and locked it's ok		    
		{

			m_LevelChangeSilent = Silent;
			if( Direction < 0 && LevelDepth() <= 0 )
			{
				Direction = 0;
			}
			m_RequestingTeleport = kFalse;
			if( Direction < 0 )
			{
				if( LevelDepth() == 1 )
				{
					m_LevelChangeMessage = LoadLocalizedString(996);
				}
				else
				{
					m_LevelChangeMessage = LoadLocalizedString(997);
				}
			}
			else
			{
				if( LevelDepth() == 0 )
				{
					m_LevelChangeMessage = LoadLocalizedString(998);
				}
				else
				{
					m_LevelChangeMessage = LoadLocalizedString(999);
				}
			}
			m_LevelChangeRequest = LevelDepth() + Direction;
			if( m_LevelChangeRequest < 0 )
			{
				m_LevelChangeRequest = 0;
			}
	}else{  //must be in demo mode and going below the maxium dugneon level
		m_LevelChangeMessage = LoadLocalizedString1Variable(1000, IntegerToString( KDemoDungeonLevel ) );
		RequestMessage( m_LevelChangeMessage );	
	}
} // CLevel::RequestLevelChange()

void CLevel::RequestLevelTeleport( int32 Depth )	// new level depth
{
	if( Depth == 0 )
	{
		m_LevelChangeMessage = LoadLocalizedString(996);
	}
	else
	{
		m_LevelChangeMessage = LoadLocalizedString(1001);
	}
	m_LevelChangeRequest = Depth;
	m_RequestingTeleport = kTrue;
} // CLevel::RequestLevelChange()

void CLevel::AddEventText( CCharacter* pParent,				// parent character
						   CCharacter* pTarget,				// target character
						   const D3DXVECTOR3& Position,		// position shot from
						   const std::string& Text,			// text
						   float32 Scale,					// scale of text
						   bool Bolded,						// text bolded?
						   float32 FadeRate,				// rate of fade
						   bool Secondary )
{
	m_pEventTextList->Queue( new CEventText( pParent,
											 pTarget,
											 Position,	
											 Text,
											 Scale,
											 Bolded,
											 FadeRate,
											 Secondary ) );
} // CLevel::AddEventText()

void CLevel::AddArrowTrail( const D3DXVECTOR3& Position,		// position shot from
							const D3DXVECTOR3& Target )			// terminating position
{
	D3DXVECTOR3 Delta = Target - Position;
	Delta.y = 0;
	float32 Distance = D3DXVec3Length( &Delta );
	D3DXMATRIX Transformation;
	D3DXMatrixLookAtLH( &Transformation,
						&Position,
						&( Position + Delta ),
						&D3DXVECTOR3( 0, 1, 0 ) );

	D3DXMatrixTranspose( &Transformation, &Transformation );
	Transformation._41 = Position.x;
	Transformation._42 = Position.y;
	Transformation._43 = Position.z;
	Transformation._14 = 0;
	Transformation._24 = 0;
	Transformation._34 = 0;
	D3DXMATRIX Scale;
	D3DXMatrixScaling( &Scale, 1, 1, Distance );
	Transformation = Scale * Transformation;

	m_pArrowTrailList->Queue( new CArrowTrail( Transformation, Distance ) );

} // CLevel::AddArrowTrail() 

CSFX* CLevel::AddSFX( CSFXDescription& pDescription,	// sfx to add
					  CCharacter* pOwner,				// owner
					  CCharacter* pTarget,				// target
					  const D3DXVECTOR3& Position,		// startting position
					  const D3DXVECTOR3& TargetPosition,	// target position
					  float32 DurationBonus,				// time duration bonus
					  CSpellDescription* pPayloadSpell )	// payload spell 
{
	if( pDescription.m_Exclusive )
	{
		CLLNode<CSFX>*	pSFX	= m_pSFXList->GetFirst();
		CLLNode<CSFX>*	pNext;
		while ( pSFX != NULL )
		{
			pNext = pSFX->Next();

			// if we find a dupe, we kill the dupe
			if( pSFX->Data()->Type() == pDescription.m_Type &&
				pTarget == pSFX->Data()->Target() &&
				pDescription.m_Path == pSFX->Data()->Path() )
			{
				pSFX->Data()->SetTime( 0 );
			}
			pSFX = pNext;
		}

	}
	CSFX* pSFX =  m_pSFXList->Queue( new CSFX( pDescription,
												m_pD3DDevice,
												m_pRefManager,
												m_pSettings,
												m_pParticleManager,
												m_pGlowManager,
												pOwner,
												pTarget,
												Position,
												TargetPosition,
												pPayloadSpell ) );

	if( pDescription.m_Looping &&
		pDescription.m_RespectBonuses )
	{
		pSFX->SetTime( pDescription.m_Duration + DurationBonus );
	}
	return pSFX;
} // CLevel::AddSFX() 

CCharacter* CLevel::AddCharacter( CCharacter* pCharacter,			// character to add to the list
								  const D3DXVECTOR3 &Position )		// position to add at
{
	m_pCharacterList->Queue( pCharacter );

	pCharacter->RemoveFromAvoidanceMap( *this );

	pCharacter->SetPosition( Position );

	pCharacter->DropToGround( *this );

	pCharacter->AddToAvoidanceMap( *this );

	pCharacter->CreatePathfinder( *this );

	return pCharacter;
} // CLevel::AddCharacter()

CCharacter* CLevel::AddPlayer( const D3DXVECTOR3 &Position )		// position to add at
{

	CCharacter* pPlayer =  new CPlayer( m_pD3DDevice,
										m_pRefManager,
										m_pSettings,
										m_pSpellManifest,
										m_pParticleManager,
										m_pGlowManager,
										m_pPositionalAudioManager,
										KMale,
										0,
										0 );

	m_pCharacterList->Queue( pPlayer );

	pPlayer->SetPosition( Position );

	pPlayer->DropToGround( *this );

	pPlayer->AddToAvoidanceMap( *this );

	pPlayer->CreatePathfinder( *this );

	pPlayer->SetMaximumTreeDepth( 800 );

	return pPlayer;
} // CLevel::AddPlayer()

CCharacter* CLevel::AddMonster( CCharacterDescription& pDescription,	// character description
							    const D3DXVECTOR3 &Position )			// position to add at
{
	CMonster* pMonster = new CMonster( m_pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pSpellManifest,
									   m_pParticleManager,
									   m_pGlowManager,
									   m_pPositionalAudioManager,
									   pDescription );

	pMonster->SetPosition( Position );

	pMonster->DropToGround( *this );

	pMonster->AddToAvoidanceMap( *this );

	pMonster->CreatePathfinder( *this );

	m_pCharacterList->Queue( pMonster );


	return pMonster;
} // CLevel::AddMonster()

CCharacter* CLevel::AddMerchant( CCharacterDescription& pDescription,	// character description
							     const D3DXVECTOR3 &Position,			// position to add at
								 EMerchant Type,						// type of merchant
								 int32 PlayerLevel )					// player level, for selecting inventory
{
	int32 CurrentLevelDepth = m_LevelDepth;
	if( CurrentLevelDepth == 0 )
	{
		CurrentLevelDepth = PlayerLevel;
		if( CurrentLevelDepth < 5 )
		{
			CurrentLevelDepth = 5;
		}
	}
	CMonster* pMonster = new CMonster( m_pD3DDevice,
									   m_pRefManager,
									   m_pSettings,
									   m_pSpellManifest,
									   m_pParticleManager,
									   m_pGlowManager,
									   m_pPositionalAudioManager,
									   pDescription );

	pMonster->SetIsMerchant( kTrue );

	pMonster->SetMerchantType( Type );

	pMonster->SetInvincible( kTrue );

	pMonster->SetPosition( Position );

	pMonster->DropToGround( *this );

	pMonster->AddToAvoidanceMap( *this );

	pMonster->CreatePathfinder( *this );

	m_pCharacterList->Queue( pMonster );

	std::vector< CItem* >	pItemList;

	if( Type == KMerchantFish )
	{
		// always have 1 fishing pole
		uint32 Item = RandomIntegerBetween( 0, m_pItemManifest.Items( KItemFishingPole ) - 1 );
		CItem* pItem = new CItem( m_pD3DDevice,
		 						  m_pRefManager,
								  m_pSettings,
								  m_pParticleManager,
								  m_pGlowManager,
								  m_pPositionalAudioManager,
								  *m_pItemManifest.ItemDescription( KItemFishingPole, Item ) );
			pItemList.push_back( pItem );
			pMonster->GiveItem( *this,
								pItem );

	}
	if( Type == KMerchantTraveling ||
		Type == KMerchantGoods )
	{
		// always have 1 fishing pole
		uint32 Item = RandomIntegerBetween( 0, m_pItemManifest.Items( KItemFishingPole ) - 1 );
		CItem* pItem = new CItem( m_pD3DDevice,
		 						  m_pRefManager,
								  m_pSettings,
								  m_pParticleManager,
								  m_pGlowManager,
								  m_pPositionalAudioManager,
								  *m_pItemManifest.ItemDescription( KItemFishingPole, Item ) );
			pItemList.push_back( pItem );
			pMonster->GiveItem( *this,
								pItem );

		// always have some potions on hand
		for( uint32 i = 0; i < 8; i++ )
		{
			uint32 Item = RandomIntegerBetween( 0, m_pItemManifest.Items( KItemPotion ) - 1 );
			CItem* pItem = new CItem( m_pD3DDevice,
									m_pRefManager,
									m_pSettings,
									m_pParticleManager,
									m_pGlowManager,
									m_pPositionalAudioManager,
									*m_pItemManifest.ItemDescription( KItemPotion, Item ) );
			pItemList.push_back( pItem );
			pMonster->GiveItem( *this,
								pItem );

		}
		// always have some scrolls too
		for( int32 i = 0; i < 4; i++ )
		{
			uint32 Item = RandomIntegerBetween( 0, m_pItemManifest.Items( KItemScroll ) - 1 );
			CItem* pItem = new CItem( m_pD3DDevice,
									m_pRefManager,
									m_pSettings,
									m_pParticleManager,
								    m_pGlowManager,
									m_pPositionalAudioManager,
									*m_pItemManifest.ItemDescription( KItemScroll, Item ) );
			pItemList.push_back( pItem );
			pMonster->GiveItem( *this,
								pItem );

		}
		pItem = new CItem( m_pD3DDevice,
						   m_pRefManager,
						   m_pSettings,
						   m_pParticleManager,
						   m_pGlowManager,
						   m_pPositionalAudioManager,
						   *m_pItemManifest.ItemDescription( LoadLocalizedString(1137) ) ); // not directly displayed, but this item is added based on the en-US name and crashes the game in localizations
		pItemList.push_back( pItem );
		pMonster->GiveItem( *this,
							pItem );

		// always have some books too
		for( int32 i = 0; i < 2; i++ )
		{
			uint32 Item = RandomIntegerBetween( 0, m_pItemManifest.Items( KItemBook ) - 1 );
			CItem* pItem = new CItem( m_pD3DDevice,
									m_pRefManager,
									m_pSettings,
									m_pParticleManager,
									m_pGlowManager,
									m_pPositionalAudioManager,
									*m_pItemManifest.ItemDescription( KItemBook, Item ) );
			pItemList.push_back( pItem );
			pMonster->GiveItem( *this,
								pItem );

		}
	}

	// add a few gems to a magic merchant
	if( Type == KMerchantMagic )
	{
		for( int32 i = 0; i < 6; i++ )
		{
			uint32 Item = RandomRareGemItemIndex();
			CItemDescription* Description = m_pItemManifest.TakeableItemDescription( Item );
			if( Description->Purchaseable() )
			{
				CItem* pItem = new CItem( m_pD3DDevice,
										m_pRefManager,
										m_pSettings,
										m_pParticleManager,
										m_pGlowManager,
										m_pPositionalAudioManager,
										*Description );
				pItemList.push_back( pItem );
				pMonster->GiveItem( *this,
									pItem );
			}
			else
			{
				i--;
			}
		}
	}

	uint32 Items = RandomIntegerBetween( 50, 60 );
	// the Stash doesn't actually 'sell' items - so it is empty
	if( Type == KMerchantStash ||
		Type == KMerchantBreakGems ||
		Type == KMerchantBreakWeapons )
	{
		Items = 0;
	}

	for( uint32 i = 0; i < Items; i++ )
	{
		uint32 Percent = RandomIntegerBetween( 0, 100 );

		uint32 Item = RandomIntegerBetween( 0, m_pItemManifest.TakeableItems() - 1 );

		// if they run out of room, stop adding items
		if( pMonster->Inventory()->FindFreeSlot( 2, 4 ) == -1 )
		{
			break;
		}
		bool SkipItem( kFalse );
		CItemDescription* pDescription = m_pItemManifest.TakeableItemDescription( Item );
		if( ( pDescription->Type() == KItemGem ) &&
			RandomIntegerBetween( 0, 100 ) > 5 )
		{
			SkipItem = kTrue;
		}
		if( pDescription->Type() == KItemPetFood &&
			RandomIntegerBetween( 0, 100 ) > 2 )
		{
			SkipItem = kTrue;
		}

		// already got one of these if we need it
		if( pDescription->Type() == KItemFishingPole )
		{
			SkipItem = kTrue;
		}

		uint32 EnchantChance( 40 );
		uint32 UniqueChance( 10 );
		int32 SocketChance( 15 );

		if( !pDescription->Purchaseable() )
		{
			SkipItem = kTrue;
		}
		int32 FishDepth( LevelDepth() );
		FishDepth -= 10;
		if( FishDepth < 1 )
		{
			FishDepth = 1;
		}
		switch( Type )
		{
		case KMerchantFish :
			if( pDescription->Type() != KItemPetFood )
			{
				SkipItem = kTrue;
			}
			else
			{
				SkipItem = kFalse;
			}
			
			if( !( pDescription->MinimumDepth() <= FishDepth &&
				   pDescription->MaximumDepth() >= FishDepth + 1 ) )
			{
				SkipItem = kTrue;
			}
			break;
		case KMerchantGambler :
			if( pDescription->Category() != KCategoryWeapon &&
				pDescription->Category() != KCategoryArmor &&
				pDescription->Category() != KCategoryJewelry )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel + 3 ) )
			{
				SkipItem = kTrue;
			}
			UniqueChance = 70;
			EnchantChance = 100;
			SocketChance = 25;
			break;
		case KMerchantTraveling :
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel + 2 ) )
			{
				SkipItem = kTrue;
			}
			break;
		case KMerchantGoods :
			EnchantChance = 10;
			UniqueChance = 1;
			if( pDescription->Category() == KCategoryWeapon ||
				pDescription->Category() == KCategoryArmor ||
				pDescription->Type() == KItemSpell ||
				pDescription->Type() == KItemGem ||
				pDescription->Type() == KItemNecklace ||
				pDescription->Type() == KItemRing )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel ) )
			{
				SkipItem = kTrue;
			}
			break;
		case KMerchantWeapons :
			EnchantChance = 20;
			UniqueChance = 5;
			if( pDescription->Category() != KCategoryWeapon )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel ) )
			{
				SkipItem = kTrue;
			}

			break;
		case KMerchantArmor :
			EnchantChance = 20;
			UniqueChance = 5;
			if( pDescription->Category() != KCategoryArmor )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel ) )
			{
				SkipItem = kTrue;
			}

			break;
		case KMerchantWeaponsArmor :
			EnchantChance = 20;
			UniqueChance = 5;
			if( pDescription->Category() != KCategoryWeapon &&
				pDescription->Category() != KCategoryArmor )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel ) )
			{
				SkipItem = kTrue;
			}

			break;
		case KMerchantMagic :
			if( pDescription->Type() != KItemPotion &&
				pDescription->Type() != KItemScroll &&
				pDescription->Type() != KItemSpell &&
				pDescription->Type() != KItemStaff &&
				pDescription->Type() != KItemGem &&
				pDescription->Type() != KItemNecklace &&
				pDescription->Type() != KItemRing &&
				pDescription->Type() != KItemPetFood &&
				pDescription->Type() != KItemBook )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel + 5 ) )
			{
				SkipItem = kTrue;
			}

			break;
		case KMerchantPotions :
			if( pDescription->Type() != KItemPotion &&
				pDescription->Type() != KItemScroll &&
				pDescription->Type() != KItemBook )
			{
				SkipItem = kTrue;
			}
			if( !( pDescription->MerchantMaximum() >= PlayerLevel &&
				   pDescription->MerchantMinimum() <= PlayerLevel ) )
			{
				SkipItem = kTrue;
			}
			break;
		}

		// don't need duplicate spells
		if( pDescription->Type() == KItemSpell &&
			pMonster->Inventory()->HasItems( pDescription->Name(), 1 ) )
		{
			SkipItem = kTrue;
		}

		if( !SkipItem )
		{
			CItem* pItem = new CItem( m_pD3DDevice,
									m_pRefManager,
									m_pSettings,
									m_pParticleManager,
									m_pGlowManager,
									m_pPositionalAudioManager,
									*pDescription );
			pItemList.push_back( pItem );

			pMonster->GiveItem( *this,
								pItem );

			if( Type != KMerchantGambler )
			{
				pItem->SetIdentified( kTrue );
			}

			if( Percent <= EnchantChance )
			{
				int32 MinimumPower = CurrentLevelDepth;
				int32 MaximumPower = CurrentLevelDepth * 5;
				if( MaximumPower > CurrentLevelDepth + 20 )
				{
					MaximumPower = CurrentLevelDepth + 20;
				}
				if( CurrentLevelDepth == 0 )
				{
					MinimumPower = 1;
					MaximumPower = 30;
				}
				if( Type == KMerchantGambler )
				{
					MinimumPower = 20;
					if( CurrentLevelDepth > MinimumPower )
					{
						MinimumPower = CurrentLevelDepth;
					}
					MaximumPower = 70 + CurrentLevelDepth;
				}


				if( Percent < UniqueChance )
				{
					// we don't allow cursed items in the store
					pItem->BestowUniquePowers( m_pNameGenerator,
											   RandomIntegerBetween( MinimumPower, MaximumPower ),
											   kFalse );
				}
				else
				{
					// we don't allow cursed items in the store - unless gambling!
					pItem->BestowPowers( RandomIntegerBetween( MinimumPower, MaximumPower ), Type == KMerchantGambler );
				}
				// gamblers don't have identified objects!
				if( Type != KMerchantGambler )
				{
					pItem->SetIdentified( kTrue );
				}
				else
				{
					pItem->SetIdentified( kFalse );
					// make sure that gambled items have effects, no matter what!
					while( pItem->Effects( KActivationPassive ) == 0 &&
						   pItem->DamageBonuses() == 0 )
					{
						pItem->BestowPowers( RandomIntegerBetween( MinimumPower, MaximumPower ), kTrue );
					}
				}
			}
			if( RandomIntegerBetween( 0, 100 ) < SocketChance &&
				!pItem->IsUnique() )
			{
				pItem->BestowSockets();
			}
			if( RandomIntegerBetween( 0, 1000 ) < 100 )
			{
				pItem->BestowGrade( Difficulty() );
			}
		}
		else
		{
			Items++;
		}

	}

	// OK, now we sort all these items so it doesn't look ugly in the inventory
	std::sort( pItemList.begin(), pItemList.end(), SortItem() );

	// first take them back out of the inventory to clear room
	for( int32 i = 0; i < pItemList.size(); i++ )
	{
		pMonster->Inventory()->RemoveItem( pItemList[i] );
	}

	// now put them back in order
	for( int32 i = 0; i < pItemList.size(); i++ )
	{
		pMonster->GiveItem( *this, pItemList[i] );
	}

	pItemList.clear();

	return pMonster;
} // CLevel::AddMerchant()

bool CLevel::CharacterExists( CCharacter* pSelectedCharacter )	// charater to check for
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data() == pSelectedCharacter )
		{
			return kTrue;
		}
		pCharacter = pCharacter->Next();
	}
	return kFalse;
} // CLevel::CharacterExists()

bool CLevel::ItemExists( CItem* pSelectedItem )	// charater to check for
{
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data() == pSelectedItem )
		{
			return kTrue;
		}
		pItem = pItem->Next();
	}
	return kFalse;
} // CLevel::ItemExists()

void CLevel::DeletePortals( void )
{
	CLLNode<CItem>*	pItem;
	CLLNode<CItem>*	pItemNext;

	pItem = m_pOnscreenItems->GetFirst();
	while ( pItem != NULL )
	{
		pItemNext = pItem->Next();
		if( pItem->Data()->Type() == KItemTownPortal ||
			pItem->Data()->Type() == KItemDungeonPortal )
		{
			m_pOnscreenItems->RemoveNoDelete( pItem );
		}
		pItem = pItemNext;
	}

	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		pItemNext = pItem->Next();
		if( pItem->Data()->Type() == KItemTownPortal ||
			pItem->Data()->Type() == KItemDungeonPortal )
		{
			pItem->Data()->SetInWorld( kFalse );
			pItem->Data()->RemoveFromAvoidanceMap( *this );
			NotifyOfDeletion( pItem->Data() );
			m_pItemList->Remove( pItem );
		}
		pItem = pItemNext;
	}



	CLLNode<CSFX>*	pSFX	= m_pSFXList->GetFirst();
	CLLNode<CSFX>*	pNext;
	while ( pSFX != NULL )
	{
		pNext = pSFX->Next();

		if( pSFX->Data()->Type() == KSFXStationaryPortal )
		{
			m_pSFXList->Remove( pSFX );
		}
		pSFX = pNext;
	}

} // CLevel::DeletePortals()

// removes an item from the scene WITHOUT deleting it
void CLevel::RemoveItem( CItem* pSelectedItem )	// item to remove
{
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data() == pSelectedItem )
		{
			pItem->Data()->SetInWorld( kFalse );
			pItem->Data()->RemoveFromAvoidanceMap( *this );
			m_pItemList->RemoveNoDelete( pItem );
			NotifyOfDeletion( pSelectedItem );
			break;
		}
		pItem = pItem->Next();
	}

	pItem = m_pOnscreenItems->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data() == pSelectedItem )
		{
			m_pOnscreenItems->RemoveNoDelete( pItem );
			break;
		}
		pItem = pItem->Next();
	}

} // CLevel::RemoveItem()

// deletes an item from the scene
void CLevel::DeleteItem( CItem* pSelectedItem )	// item to remove
{
	CLLNode<CItem>*	pItem;
	CLLNode<CItem>*	pItemNext;

	pItem = m_pOnscreenItems->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data() == pSelectedItem )
		{
			m_pOnscreenItems->RemoveNoDelete( pItem );
			break;
		}
		pItem = pItem->Next();
	}

	pItem = m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		pItemNext = pItem->Next();
		if( pItem->Data() == pSelectedItem )
		{
			pItem->Data()->SetInWorld( kFalse );
			pItem->Data()->RemoveFromAvoidanceMap( *this );
			NotifyOfDeletion( pSelectedItem );
			m_pItemList->Remove( pItem );
			return;
		}
		pItem = pItemNext;
	}



} // CLevel::DeleteItem()

// notifies characters of a character's death
void CLevel::NotifyOfDeath( CCharacter* pSelectedCharacter )	// Character which has died
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data() != pSelectedCharacter )
		{
			pCharacter->Data()->NotifyOfDeath( *this,
											   pSelectedCharacter );
		}
		pCharacter = pCharacter->Next();
	}
} // CLevel::NotifyOfDeath()

// removes an Character from the scene WITHOUT deleting it
void CLevel::RemoveCharacter( CCharacter* pSelectedCharacter )	// Character to remove
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data() == pSelectedCharacter )
		{
			pCharacter->Data()->RemoveFromAvoidanceMap( *this );
			m_pCharacterList->RemoveNoDelete( pCharacter );
			NotifyOfDeletion( pSelectedCharacter );
			break;
		}
		pCharacter = pCharacter->Next();
	}

	pCharacter = m_pOnscreenCharacters->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data() == pSelectedCharacter )
		{
			m_pOnscreenCharacters->RemoveNoDelete( pCharacter );
			break;
		}
		pCharacter = pCharacter->Next();
	}
} // CLevel::RemoveCharacter()

// deletes an Character from the scene
void CLevel::DeleteCharacter( CCharacter* pSelectedCharacter )	// Character to remove
{

	CLLNode<CCharacter>*	pCharacter;
	pCharacter = m_pOnscreenCharacters->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data() == pSelectedCharacter )
		{
			m_pOnscreenCharacters->RemoveNoDelete( pCharacter );
			break;
		}
		pCharacter = pCharacter->Next();
	}

	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data() == pSelectedCharacter )
		{
			pCharacter->Data()->RemoveFromAvoidanceMap( *this );
			NotifyOfDeletion( pSelectedCharacter );
			m_pCharacterList->Remove( pCharacter );
			return;
		}
		pCharacter = pCharacter->Next();
	}


} // CLevel::DeleteCharacter()

void CLevel::GenerateRandomItems( void )
{
	if( m_pLevelMesh == NULL )
	{
		return;
	}
	// first add containers
	int32 Area = m_pMaze->Width() * m_pMaze->Height();

	uint32 Events = 0;
	if( RandomIntegerBetween( 0, 100 ) < 75 )
	{
		Events = RandomIntegerBetween( 1, 3 );
	}

	int32 ModifiedLevelDepth = LevelDepth();
	ModifiedLevelDepth = (int32)ceil( (float32)ModifiedLevelDepth * KDifficultyMultiplier[Difficulty()] );
	ModifiedLevelDepth += (int32)KDifficultyLevelBase[Difficulty()];
	if( ModifiedLevelDepth < 1 )
	{
		ModifiedLevelDepth = 1;
	}

	if( m_pItemManifest.Items( KCategoryEvent ) > 0 )
	{
		for( uint32 i = 0; i < Events; i++ )
		{
			D3DXVECTOR3 Position;
			Position = RandomOpenPositionInRoom();

			int32 ItemIndex = RandomEventItemIndex();
			CItemDescription* pEventDescription = m_pItemManifest.ItemDescription( KCategoryEvent, ItemIndex );

			if( pEventDescription != NULL )
			{
				CItem* pItem = AddItem( *pEventDescription,
										Position );
			}
		}
	}

	uint32 Containers = RandomIntegerBetween( Area / 40 , Area / 30 );

	if( m_pItemManifest.ContainerItems() > 0 )
	{
		for( uint32 i = 0; i < Containers; i++ )
		{
			D3DXVECTOR3 Position;
			if( RandomIntegerBetween( 0, 100 ) < 70 )
			{
				Position = RandomOpenPositionNearWall();
			}
			else
			{
				Position = RandomOpenPosition();
			}

			CItem* pItem;
			uint32 Percent = RandomIntegerBetween( 0, 100 );
			uint32 ItemIndex( 0 );
			uint32 Bunch( 1 );	// default, only 1
			CItemDescription* pContainerDescription;
			bool IsBarrel( kFalse );
			if( Percent < 80 )	// 80% chance it will be a normal boring container (barrel )
			{

				IsBarrel = kTrue;
				ItemIndex = RandomBarrel();
				pContainerDescription = m_pItemManifest.ItemDescription( KCategoryContainer, ItemIndex );
				Bunch = RandomIntegerBetween( 1, 3 );
				// we can have clusters of these
			}
			else	// otherwise, take ANY kind of container ( including chests )
			{
				ItemIndex = RandomContainer();
				pContainerDescription = m_pItemManifest.ItemDescription( KCategoryContainer, ItemIndex );
				if( RandomIntegerBetween( 0, 100 ) < 50 )
				{
					Position = RandomOpenPosition();
				}
			}

			for( uint32 p = 0; p < Bunch; p++ )
			{
				if( IsBarrel )	// change up the bunches of barrels/urns
				{
					ItemIndex = RandomBarrel();
					pContainerDescription = m_pItemManifest.ItemDescription( KCategoryContainer, ItemIndex );
				}
				pItem = AddItem( *pContainerDescription,
								RandomOpenPosition( Position, 0, 5 ) );
				// if it's a plain (non-chest) container,
				if( pItem->Type() == KItemContainer )
				{
					// we don't trap items below level two. Go easy on the newbies
					if( ModifiedLevelDepth > 2 )
					{
						// 10% chance it is trapped!
						if( RandomIntegerBetween( 0, 100 ) < 10 )
						{
							// 80% are explosions, and the other 20 are poison
							if( RandomIntegerBetween( 0, 100 ) < 80 )
							{
								pItem->SetTrapType( KTrapExplosion );
							}
							else
							{
								pItem->SetTrapType( KTrapPoison );
							}
						}
					}
					// 0-2 items in barrels
					uint32 Items = RandomIntegerBetween( 0, 2 );
					for( uint32 j = 0; j < Items; j++ )
					{
						// let's figure out what kind of item to put in there
						uint32 Percent = RandomIntegerBetween( 0, 100 );
						if( Percent < 15 )
						{
							pItem->AddGold( m_pD3DDevice,
											RandomIntegerBetween( 1 * ModifiedLevelDepth, 4 * ModifiedLevelDepth ) );
						}
						else if( Percent < 25 )
						{
							uint32 SubItem = RandomTakeableItemIndex();

							CItem* pSubItem = pItem->AddItem( m_pD3DDevice,
															 *m_pItemManifest.TakeableItemDescription( SubItem ) );
							if( Percent * KDifficultyItemMultiplier[Difficulty()] > 92 )
							{
								pSubItem->BestowPowers( ModifiedLevelDepth );
							}

						}

						if( RandomIntegerBetween( 0, 1000 ) < 3 * KDifficultyItemMultiplier[Difficulty()] )
						{
							pItem->BestowGrade( Difficulty() );
						}
						// there's a 60 percent chance nothing is in there
					}

				}
				else	// chests get better loot
				{
					// we don't trap items on level one. Go easy on the newbies
					if( ModifiedLevelDepth > 2 &&
						pItem->Type() != KItemWeaponRack )
					{
						// 8% chance it is trapped
						if( RandomIntegerBetween( 0, 100 ) < 6 )
						{
							// 80% are magic swords, and the other 20 are poison
							if( RandomIntegerBetween( 0, 100 ) < 80 )
							{
								pItem->SetTrapType( KTrapSword );
							}
							else
							{
								pItem->SetTrapType( KTrapPoison );
							}
						}
					}
					uint32 MaxItems = 0;
					uint32 MinItems = 0;
					uint32 ItemLevelDepth = ModifiedLevelDepth;
					switch( pItem->Type() )
					{
					case KItemWeaponRack:
						MinItems = 1;
						MaxItems = 2;
						break;
					case KItemChestSmall:
						MaxItems = 1;
						break;
					case KItemChestMedium:
						MaxItems = 2;
						break;
					case KItemChestLarge:
						MinItems = 2;
						MaxItems = 5;
						ItemLevelDepth += 2;
						break;
					}
					uint32 Items = RandomIntegerBetween( MinItems, MaxItems );
					for( uint32 j = 0; j < Items; j++ )
					{
						// let's figure out what kind of item to put in there
						uint32 Percent = RandomIntegerBetween( 0, 100 );
						if( Percent < 55 && pItem->Type() != KItemWeaponRack )
						{
							pItem->AddGold( m_pD3DDevice,
											RandomIntegerBetween( 1 * ItemLevelDepth, 4 * ItemLevelDepth ) );
						}
						else
						{
							uint32 SubItem = RandomTakeableItemIndex();
							CItem* pSubItem;
							if( pItem->Type() == KItemWeaponRack )
							{
								SubItem = RandomWeaponItemIndex();
								pSubItem = pItem->AddItem( m_pD3DDevice,
															  *m_pItemManifest.ItemDescription( KCategoryWeapon, SubItem ) );
							}
							else
							{
								pSubItem = pItem->AddItem( m_pD3DDevice,
															  *m_pItemManifest.TakeableItemDescription( SubItem ) );
							}
							if( Percent * KDifficultyItemMultiplier[Difficulty()] > 86 )
							{
								if( Percent * KDifficultyItemMultiplier[Difficulty()] > 98 )
								{
									pSubItem->BestowUniquePowers( m_pNameGenerator,
																  ItemLevelDepth );
								}
								else
								{
									pSubItem->BestowPowers( ItemLevelDepth );
								}
							}
							if( RandomIntegerBetween( 0, 1000 ) < 50 * KDifficultyItemMultiplier[Difficulty()] )
							{
								pItem->BestowGrade( Difficulty() );
							}
							if( RandomIntegerBetween( 0, 100 ) < 12 * KDifficultyItemMultiplier[Difficulty()] &&
								!pSubItem->IsUnique() )
							{
								pSubItem->BestowSockets();
							}
						}
					}
				}
			}
		}
	}

	// add random gold piles
	uint32 GoldPiles = RandomIntegerBetween( 0, 10 );
	for( uint32 i = 0; i < GoldPiles; i++ )
	{

		AddGold( RandomIntegerBetween( 1 * ModifiedLevelDepth, 4 * ModifiedLevelDepth ),
				 RandomOpenPosition() );
	}

	// add some random potions
	uint32 Potions = RandomIntegerBetween( 2, 5 );
	if( m_pItemManifest.Items( KItemPotion ) > 0 )
	{
		for( int32 i = 0; i < Potions; i++ )
		{

			uint32 Item = RandomPotion();
			AddItem( *m_pItemManifest.ItemDescription( KItemPotion, Item ),
					 RandomOpenPosition() );
		}
	}

	// find possible treasure rooms
	for( int32 i = 0; i < m_pMaze->Rooms(); i++ )
	{
		// if the room only has 1 exit, and is below a certain size,
		// then it is likely to become a treasure room...
		if( m_pMaze->RoomExits( i ) < 2 &&
			m_pMaze->RoomWidth( i ) < 7 &&
			m_pMaze->RoomHeight( i ) < 7 &&
			RandomIntegerBetween( 0, 100 ) < 70 )
		{
			// ok! this is a treasure room
			for( uint32 y = 0; y < m_pMaze->Height(); y++ )
			{
				for( uint32 x = 0; x < m_pMaze->Width(); x++ )
				{
					if( RandomIntegerBetween( 0, 100 ) < 70 &&
						!m_pMaze->HasCollideableProp( x, y ) )
					{


						// for every cell in this room, we dump treasure!
						if( m_pMaze->RoomIndexData( x, y ) == i &&
							m_pMaze->CellHeight( x, y ) == 0 )
						{
							D3DXVECTOR3 Position( PositionFromCellPosition( x, y ) );

							// might dump a monster to defend the goods!
							if( RandomIntegerBetween( 0, 100 ) < 40 )
							{
								bool IsUnique( kFalse );
								// one in 100 chance that we get a superunique monster
								if( rand() %1000 < 10 * KDifficultyItemMultiplier[Difficulty()] )
								{
									IsUnique = kTrue;
								}


								uint32 Character = RandomMonsterIndex();
								if( IsUnique )
								{
									Character = RandomUniqueMonsterIndex();
								}
								if( m_pCharacterManifest.CharacterDescription( Character )->Alignment() == KEvil )
								{
									CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( Character );

									uint32 PackCount( 1 );

									CCharacter* pMonster = AddMonster( *pDescription,
																	   RandomOpenPosition( Position, 0, 15 ) );

									OutfitCharacter( *pDescription,
													 *pMonster,
													 IsUnique );

									i += GenerateMinions( *pDescription,
														  *pMonster);
									if( RandomIntegerBetween( 0, 100 ) < 50 &&
										!pMonster->IsUnique() )
									{
										pMonster->SetAllowDropEquipment( kFalse );
									}
								}

							}


							// add random gold piles on this cell
							uint32 GoldPiles = RandomIntegerBetween( 0, 4 );
							for( uint32 i = 0; i < GoldPiles; i++ )
							{

								AddGold( RandomIntegerBetween( 2 * ModifiedLevelDepth, 8 * ModifiedLevelDepth ),
										 RandomOpenPosition( Position, 6 ) );
							}

							// 50/50 we'll add a jewel
							if( RandomIntegerBetween( 0, 100 ) < 30 )
							{

								uint32 SubItem = RandomJewelryItemIndex();

								CItem* pSubItem = AddItem( *m_pItemManifest.TakeableItemDescription( SubItem ),
														   RandomOpenPosition( Position, 0, 5 ) );
								if( RandomIntegerBetween( 0, 100 ) * KDifficultyItemMultiplier[Difficulty()] > 75 )
								{
									pSubItem->BestowPowers( ModifiedLevelDepth );
								}
							}

							// 50/50 we'll add a chest
							if( RandomIntegerBetween( 0, 100 ) < 50 )
							{
								int32 ItemIndex = RandomIntegerBetween( 0, m_pItemManifest.ContainerItems() - 1 );
								CItemDescription* pContainerDescription = m_pItemManifest.ContainerItemDescription( ItemIndex );


								CItem* pItem = AddItem( *pContainerDescription,
														RandomOpenPosition( Position, 0, 5 ) );

								uint32 Items = RandomIntegerBetween( 1, 3 );
								for( uint32 j = 0; j < Items; j++ )
								{
									// let's figure out what kind of item to put in there
									uint32 Percent = RandomIntegerBetween( 0, 100 );
									if( Percent < 75 && pItem->Type() != KItemWeaponRack )
									{
										pItem->AddGold( m_pD3DDevice,
														RandomIntegerBetween( 1 * ModifiedLevelDepth, 4 * ModifiedLevelDepth ) );
									}
									else
									{
										CItem* pSubItem;

										if( pItem->Type() == KItemWeaponRack )
										{
											uint32 SubItem = RandomWeaponItemIndex();
											pSubItem = pItem->AddItem( m_pD3DDevice,
																		*m_pItemManifest.ItemDescription( KCategoryWeapon, SubItem ) );
										}
										else
										{
											uint32 SubItem = RandomTakeableItemIndex();

											pSubItem = pItem->AddItem( m_pD3DDevice,
																		*m_pItemManifest.TakeableItemDescription( SubItem ) );
										}
										if( Percent > 65 * KDifficultyItemMultiplier[Difficulty()] )
										{
											if( Percent > 95 * KDifficultyItemMultiplier[Difficulty()] )
											{
												pSubItem->BestowUniquePowers( m_pNameGenerator,
																			  ModifiedLevelDepth );
											}
											else
											{
												pSubItem->BestowPowers( ModifiedLevelDepth );
											}
										}
										if( RandomIntegerBetween( 0, 100 ) < 17 * KDifficultyItemMultiplier[Difficulty()] &&
											!pSubItem->IsUnique() )
										{
											pSubItem->BestowSockets();
										}
										if( RandomIntegerBetween( 0, 1000 ) < 30 * KDifficultyItemMultiplier[Difficulty()] )
										{
											pItem->BestowGrade( Difficulty() );
										}

									}
								}

							}

						}
					}
				}
			}
		}
	}
} // CLevel::GenerateRandomItems()

void CLevel::GenerateRandomMonsters( void )
{
	if( m_pLevelMesh == NULL )
	{
		return;
	}
	if( m_pCharacterManifest.Families() == 0 )
	{
		return;
	}

	if( RandomBetween( 0, 100 ) < 30 )
	{
		// add a traveling salesman!
		CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( LoadLocalizedString(1002) );
		if( pDescription != NULL )
		{
			CCharacter* pSalesman = AddMerchant( *pDescription,
												 RandomOpenPositionInRoom(),
												 KMerchantTraveling,
												 LevelDepth() );
		}
	}

	if( RandomBetween( 0, 100 ) <= 20 )
	{
		// add a traveling fish salesman!
		CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( LoadLocalizedString(1003) );
		if( pDescription != NULL )
		{
			CCharacter* pSalesman = AddMerchant( *pDescription,
												 RandomOpenPositionInRoom(),
												 KMerchantFish,
												 LevelDepth() );
		}
	}

	// we make a 'safe zone' around the stairwell so that we don't attack players immediately
	D3DXVECTOR3 StairsPosition = PositionNearStairsUp();
	StairsPosition.y = 0;
	float32 SafetyRadius = 42;


	int32 Area = m_pMaze->Width() * m_pMaze->Height();
	uint32 Monsters = RandomIntegerBetween( Area / 30 , Area / 20 );

	Monsters -= m_pCharacterList->Nodes();

	for( uint32 i = 0; i < Monsters; i++ )
	{
		bool IsUnique = kFalse;
		// one in 100 chance that we get a superunique monster
		if( rand() %1000 < 10 * KDifficultyAppearanceMultiplier[Difficulty()] )
		{
			IsUnique = kTrue;
		}


		uint32 Character = RandomMonsterIndex();
		if( IsUnique )
		{
			Character = RandomUniqueMonsterIndex();
		}
		if( m_pCharacterManifest.CharacterDescription( Character )->Alignment() == KEvil )
		{
			CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( Character );

			uint32 PackCount( 1 );
			if( RandomIntegerBetween( 0, 100 ) < 10 &&
				pDescription->Minions() == 0 &&
				!IsUnique )
			{
				PackCount = RandomIntegerBetween( 2, 4 );
			}

			D3DXVECTOR3 Position = RandomOpenPosition();
			int32 Tries( 0 );
			while( D3DXVec3Length( &( Position - StairsPosition ) ) < SafetyRadius &&
				   Tries < 100 )
			{
				Position = RandomOpenPosition();
				Tries++;
			}
			for( uint32 j = 0; j < PackCount; j++ )
			{
				CCharacter* pMonster = AddMonster( *pDescription,
												   RandomOpenPosition( Position, 0, 15 ) );

				OutfitCharacter( *pDescription,
								 *pMonster,
								 IsUnique );

				i += GenerateMinions( *pDescription,
									  *pMonster);
				if( RandomIntegerBetween( 0, 100 ) < 60 &&
					!pMonster->IsUnique() )
				{
					pMonster->SetAllowDropEquipment( kFalse );
				}
			}
		}
	}
} // CLevel::GenerateRandomMonsters()

uint32 CLevel::GenerateMinions( CCharacterDescription& pDescription,	// character description
								CCharacter& pCharacter )				// character to outfit
{
	uint32 TotalMinions( 0 );
	if( pDescription.Minions() > 0 )
	{
		for( uint32 i = 0; i < pDescription.Minions(); i++ )
		{
			uint32 Minions = RandomIntegerBetween( pDescription.MinimumMinions( i ),
												   pDescription.MaximumMinions( i ) );

			for( uint32 j = 0; j < Minions; j++ )
			{
				CCharacterDescription* pMinionDescription = m_pCharacterManifest.CharacterDescription( pDescription.MinionName( i ) );
				if( pMinionDescription != NULL )
				{
					TotalMinions ++;
					CCharacter* pMinion = AddMonster( *pMinionDescription,
													  RandomOpenPosition( pCharacter.Position(), 12 ) ); 
					OutfitCharacter( *pMinionDescription,
									 *pMinion,
									 kFalse );
					pCharacter.AddPet( pMinion );
					pMinion->SetAllowDropInventory( kFalse );
					pMinion->SetAllowDropEquipment( kFalse );
				}
			}
		}
	}
	return TotalMinions;
} // CLevel::GenerateMinions()

void CLevel::OutfitCharacter( CCharacterDescription& pDescription,	// character description
							  CCharacter& pCharacter,				// character to outfit
							  bool IsUnique )					// allow unique creation?
{
	int32 MinimumLevel = LevelDepth() - 2;
	if( MinimumLevel < 1 )
	{
		MinimumLevel = 1;
	}
	uint32 TargetLevel = RandomIntegerBetween( MinimumLevel, LevelDepth() );
	if( LevelDepth() <= 1 && TargetLevel > 2 )
	{
		TargetLevel = 2;
	}
	TargetLevel = (int32)ceil( (float32)TargetLevel * KDifficultyMultiplier[Difficulty()] );
	TargetLevel += (int32)KDifficultyLevelBase[Difficulty()];
	if( TargetLevel < 1 )
	{
		TargetLevel = 1;
	}

	if( IsUnique )
	{
		pCharacter.MakeUnique( m_pNameGenerator,
							   TargetLevel,
							   kFalse );
	}
	else
	{
		pCharacter.TweakToLevel( TargetLevel );
	}
	
	
	GiveEquipment( pDescription,
				   pCharacter );

} // CLevel::OutfitCharacter()

void CLevel::GiveEquipment( CCharacterDescription& pDescription,	// character description
							CCharacter& pCharacter )				// character to outfit
{
	int32 MinimumLevel = LevelDepth() - 2;
	if( MinimumLevel < 1 )
	{
		MinimumLevel = 1;
	}
	uint32 TargetLevel = RandomIntegerBetween( MinimumLevel, LevelDepth() + 3 );
	if( LevelDepth() == 1 && TargetLevel > 2 )
	{
		TargetLevel = 2;
	}
	TargetLevel = (int32)ceil( (float32)TargetLevel * KDifficultyItemMultiplier[Difficulty()] );

	uint32 EquipmentLevel = TargetLevel;
	if( pCharacter.IsUnique() )
	{
		EquipmentLevel += 10;
	}

	// let's figure out what kind of item to put in there
	uint32 Percent = RandomIntegerBetween( 0, 100 );
	if( Percent < 5 )
	{
		pCharacter.GiveGold( RandomIntegerBetween( 1 * TargetLevel, 4 * TargetLevel ) );
	}
	if( pCharacter.IsMimic() )
	{
		pCharacter.GiveGold( RandomIntegerBetween( 2 * TargetLevel, 15 * TargetLevel ) );
		pCharacter.GiveGold( RandomIntegerBetween( 2 * TargetLevel, 15 * TargetLevel ) );
		pCharacter.GiveGold( RandomIntegerBetween( 2 * TargetLevel, 15 * TargetLevel ) );
	}

	Percent = RandomIntegerBetween( 0, 100 );
	if( Percent < 5 ||
		pCharacter.IsMimic() )
	{
		uint32 Item = RandomTakeableItemIndex();

		CItem* pItem = new CItem( m_pD3DDevice,
								  m_pRefManager,
								  m_pSettings,
								  m_pParticleManager,
								  m_pGlowManager,
								  m_pPositionalAudioManager,
								  *m_pItemManifest.TakeableItemDescription( Item ) );
		pCharacter.GiveItem( *this,
							pItem );

		Percent = RandomIntegerBetween( 0, 100 );
		if( Percent * KDifficultyItemMultiplier[Difficulty()]  > 90 || 
			( pCharacter.IsUnique() && Percent > 50 ) )
		{
			if( RandomIntegerBetween( 0, 100 )  * KDifficultyItemMultiplier[Difficulty()] > 80 )
			{
				pItem->BestowUniquePowers( m_pNameGenerator,
										   TargetLevel,
										   !pCharacter.IsUnique() );
			}
			else
			{
				pItem->BestowPowers( EquipmentLevel,
									 !pCharacter.IsUnique()  );
			}
		}
		if( RandomIntegerBetween( 0, 100 ) < 8  * KDifficultyItemMultiplier[Difficulty()] &&
			!pItem->IsUnique() )
		{
			pItem->BestowSockets();
		}
		if( RandomIntegerBetween( 0, 1000 ) < 50  * KDifficultyItemMultiplier[Difficulty()] )
		{
			pItem->BestowGrade( Difficulty() );
		}

	}

	// extra gold and a guaranteed unique item for unique monster
	if( pCharacter.IsUnique() )
	{
		pCharacter.GiveGold( RandomIntegerBetween( 4 * m_LevelDepth, 10 * m_LevelDepth ) );

		uint32 Item = RandomRareTakeableItemIndex();

		CItem* pItem = new CItem( m_pD3DDevice,
								  m_pRefManager,
								  m_pSettings,
								  m_pParticleManager,
								  m_pGlowManager,
								  m_pPositionalAudioManager,
								  *m_pItemManifest.TakeableItemDescription( Item ) );
		pCharacter.GiveItem( *this,
							pItem );

		if( RandomIntegerBetween( 0, 100 ) > 95 )
		{
			// unique characters don't get cursed
			pItem->BestowUniquePowers( m_pNameGenerator,
									   TargetLevel,
									   !pCharacter.IsUnique()  );
		}
		else
		{
			pItem->BestowPowers( EquipmentLevel,
								 !pCharacter.IsUnique()  );
		}
		if( RandomIntegerBetween( 0, 100 ) < 15 &&
			!pItem->IsUnique() )
		{
			pItem->BestowSockets();
		}
		if( RandomIntegerBetween( 0, 1000 ) < 50 )
		{
			pItem->BestowGrade( Difficulty() );
		}

	}


	// outfit the monster

	int32 ItemLevelDepth( LevelDepth() );
	if( pCharacter.IsUnique() )
	{
		ItemLevelDepth = (int32)( ItemLevelDepth * 1.5 );
		ItemLevelDepth += 5;
	}
	// can hold a weapon -
	if( pCharacter.Inventory()->CanEquip( KSlotRightHand ) )
	{
		Percent = RandomIntegerBetween( 0, 100 );
		CItemDescription* pWeaponDescription = NULL;

		if( pDescription.PreferredWeapons() > 0 && rand() % 1000 < 500 )
		{
			uint32 Weapon = RandomIntegerBetween( 0, pDescription.PreferredWeapons() - 1 );
			pWeaponDescription = m_pItemManifest.ItemDescription( pDescription.PreferredWeapon( Weapon ) );
			// make sure the weapon we wanted is allowed at this level, or we are unique
			if( pWeaponDescription != NULL &&
				pWeaponDescription->MinimumDepth() > ItemLevelDepth )
			{
				pWeaponDescription = NULL;
			}
		}

		if( pWeaponDescription == NULL )
		{
			if( pCharacter.IsUnique() )	// uniques can have better loot
			{

				uint32 RandomWeapon = RandomRareWeaponItemIndex();

				pWeaponDescription = m_pItemManifest.ItemDescription( KCategoryWeapon, 
																	  RandomWeapon );
			}
			else
			{
				// first select a random weapon
				if( Percent < 70 )
				{
					uint32 RandomWeapon = RandomWeaponItemIndex();
					pWeaponDescription = m_pItemManifest.ItemDescription( KCategoryWeapon, 
																		  RandomWeapon );
				}
			}
		}

		if( pWeaponDescription != NULL )
		{
			CItem* pItem = new CItem( m_pD3DDevice,
									  m_pRefManager,
									  m_pSettings,
									  m_pParticleManager,
									  m_pGlowManager,
									  m_pPositionalAudioManager,
									  *pWeaponDescription );
			pCharacter.GiveItem( *this,
								pItem );

			Percent = RandomIntegerBetween( 0, 100 );
			if( Percent > 90 || ( pCharacter.IsUnique() && Percent > 60 ) )
			{
				if( RandomIntegerBetween( 0, 100 ) > 90 )
				{
					pItem->BestowUniquePowers( m_pNameGenerator,
											   TargetLevel,
											   !pCharacter.IsUnique()  );
				}
				else
				{
					pItem->BestowPowers( EquipmentLevel,
										 !pCharacter.IsUnique()  );
				}
			}
			if( RandomIntegerBetween( 0, 100 ) < 15 &&
				!pItem->IsUnique() )
			{
				pItem->BestowSockets();
			}
			if( RandomIntegerBetween( 0, 1000 ) < 50 )
			{
				pItem->BestowGrade( Difficulty() );
			}
		}
	}


	// can hold a weapon -
	if( pDescription.DualWielding() &&
		pCharacter.Inventory()->CanEquip( KSlotLeftHand ) )
	{
		Percent = RandomIntegerBetween( 0, 100 );
		CItemDescription* pWeaponDescription = NULL;

		if( pDescription.PreferredWeapons() > 0 && RandomIntegerBetween( 0, 100 ) < 50 )
		{
			uint32 Weapon = RandomIntegerBetween( 0, pDescription.PreferredWeapons() - 1 );
			pWeaponDescription = m_pItemManifest.ItemDescription( pDescription.PreferredWeapon( Weapon ) );

			// make sure the weapon we wanted is allowed at this level, or we are unique
			if( pWeaponDescription != NULL &&
				pWeaponDescription->MinimumDepth() > ItemLevelDepth )
			{
				pWeaponDescription = NULL;
			}
		}

		if( pWeaponDescription == NULL )
		{
			if( pCharacter.IsUnique() )	// uniques can have anything
			{
				uint32 RandomWeapon = RandomRareWeaponItemIndex();
				pWeaponDescription = m_pItemManifest.ItemDescription( KCategoryWeapon, 
																	  RandomWeapon );
			}
			else
			{
				// first select a random weapon
				if( Percent < 80 )
				{
					uint32 RandomWeapon = RandomWeaponItemIndex();
					pWeaponDescription = m_pItemManifest.ItemDescription( KCategoryWeapon, 
																		  RandomWeapon );
				}
			}
		}

		if( pWeaponDescription != NULL )
		{
			CItem* pItem = new CItem( m_pD3DDevice,
									  m_pRefManager,
									  m_pSettings,
									  m_pParticleManager,
									  m_pGlowManager,
									  m_pPositionalAudioManager,
									  *pWeaponDescription );
			pCharacter.GiveItem( *this,
								pItem );

			Percent = RandomIntegerBetween( 0, 100 );
			if( Percent > 90 || pCharacter.IsUnique() )
			{
				if( RandomIntegerBetween( 0, 100 ) > 90 )
				{
					pItem->BestowUniquePowers( m_pNameGenerator,
											   TargetLevel,
											   !pCharacter.IsUnique()  );
				}
				else
				{
					pItem->BestowPowers( EquipmentLevel );
				}
			}
			if( RandomIntegerBetween( 0, 100 ) < 3 &&
				!pItem->IsUnique() )
			{
				pItem->BestowSockets();
			}
			if( RandomIntegerBetween( 0, 1000 ) < 20 )
			{
				pItem->BestowGrade( Difficulty() );
			}
		}
	}

	bool NeedsShield( kTrue );
	if( pDescription.DualWielding() && rand() %1000 < 500 )
	{
		NeedsShield = kFalse;
	}

	// can hold a shield -
	if( pCharacter.Inventory()->CanEquip( KSlotLeftArm ) &&
		NeedsShield )
	{
		Percent = RandomIntegerBetween( 0, 100 );
		CItemDescription* pArmorDescription = NULL;

		if( pDescription.PreferredShields() > 0 && rand() % 1000 < 500 )
		{
			uint32 Shield = RandomIntegerBetween( 0, pDescription.PreferredShields() - 1 );
			pArmorDescription = m_pItemManifest.ItemDescription( pDescription.PreferredShield( Shield ) );
			// make sure the armor we wanted is allowed at this level, or we are unique
			if( pArmorDescription != NULL &&
				pArmorDescription->MinimumDepth() > ItemLevelDepth )
			{
				pArmorDescription = NULL;
			}

		}

		if( pArmorDescription == NULL &&
			m_pItemManifest.Items( KItemShield ) > 0 )
		{
			
			if( pCharacter.IsUnique() )	// uniques can have anything
			{
				uint32 RandomArmor = RandomRareShieldItemIndex();
				pArmorDescription = m_pItemManifest.ItemDescription( KItemShield, 
																	 RandomArmor );
			}
			else
			{
				if( Percent < 50 )
				{
					uint32 RandomArmor = RandomShieldItemIndex();
					pArmorDescription = m_pItemManifest.ItemDescription( KItemShield, 
																		 RandomArmor );
				}
			}
		}

		if( pArmorDescription != NULL )
		{
			CItem* pItem = new CItem( m_pD3DDevice,
									  m_pRefManager,
									  m_pSettings,
									  m_pParticleManager,
									  m_pGlowManager,
									  m_pPositionalAudioManager,
									  *pArmorDescription );
			pCharacter.GiveItem( *this,
								pItem );

			Percent = RandomIntegerBetween( 0, 100 );
			if( Percent > 90 || ( pCharacter.IsUnique() && Percent > 50 ) )
			{
				if( RandomIntegerBetween( 0, 100 ) > 90 )
				{
					pItem->BestowUniquePowers( m_pNameGenerator,
											   TargetLevel,
											   !pCharacter.IsUnique()  );
				}
				else
				{
					pItem->BestowPowers( EquipmentLevel,
										 !pCharacter.IsUnique()  );
				}
			}
			if( RandomIntegerBetween( 0, 100 ) < 3 &&
				!pItem->IsUnique() )
			{
				pItem->BestowSockets();
			}
			if( RandomIntegerBetween( 0, 1000 ) < 50 )
			{
				pItem->BestowGrade( Difficulty() );
			}
		}
	}

	pCharacter.Inventory()->EquipBest( *this );


	if( pDescription.DualWielding() )
	{
		pCharacter.Inventory()->EquipBestDualWieldWeapon( *this );
	}

} // CLevel::GiveEquipment()

CItem* CLevel::AddItem( CItemDescription& pDescription,		// item description
						const D3DXVECTOR3 &Position )		// position to add at
{

	CItem* pItem =  new CItem( m_pD3DDevice,
							   m_pRefManager,
							   m_pSettings,
							   m_pParticleManager,
							   m_pGlowManager,
							   m_pPositionalAudioManager,
							   pDescription );

	pItem->SetInWorld( kTrue );

	m_pItemList->Queue( pItem );

	pItem->SetPosition( *this,
						Position );

	pItem->DropToGround( *this );
	pItem->AddToAvoidanceMap( *this );
	return pItem;
} // CLevel::AddItem()

void CLevel::AddItemToRandomContainer( CItem* pAddItem )
{
	bool MustAdd( kFalse );
	bool Done( kFalse );
	uint32 Containers( 0 );
	while( !Done )
	{
		CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
		while ( pItem != NULL )
		{
			if( pItem->Data()->IsContainer() &&
				!pItem->Data()->CanInteract() )
			{
				if( RandomIntegerBetween( 0, 100 ) < 10 ||
					MustAdd )
				{
					pItem->Data()->AddItem( pAddItem );
					return;
				}
				Containers++;
			}
			pItem = pItem->Next();
		}
		if( Containers == 0 )
		{
			AddItem( pAddItem, RandomOpenPosition() );
			return;
		}
		MustAdd = kTrue;
	}
} // CLevel::AddItemToRandomContainer()

CItem* CLevel::AddGold( uint32 Value,					// amount of gold
						const D3DXVECTOR3 &Position )	// position to add at
{

	CItem* pItem =  new CItemGold( m_pD3DDevice,
								   m_pRefManager,
								   m_pSettings,
								   m_pParticleManager,
								   m_pGlowManager,
								   m_pPositionalAudioManager,
								   Value );
	pItem->SetInWorld( kTrue );

	m_pItemList->Queue( pItem );

	pItem->SetPosition( *this, Position );

	pItem->DropToGround( *this );

	return pItem;
} // CLevel::AddGold()

CItem* CLevel::AddItem( CItem* pItem,						// item to add
						const D3DXVECTOR3 &Position )		// position to add at
{

	pItem->RemoveFromAvoidanceMap( *this );
	m_pItemList->Queue( pItem );

	pItem->SetInWorld( kTrue );

	pItem->SetPosition( *this, Position );

	pItem->DropToGround( *this );

	pItem->AddToAvoidanceMap( *this );
	return pItem;
} // CLevel::AddItem()

void CLevel::UpdateCharacters( const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
							   float32 TimeElapsed )			// time elapsed in seconds
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	CLLNode<CCharacter>*	pNext;
	while ( pCharacter != NULL )
	{
		pNext = pCharacter->Next();
		if( pCharacter->Data()->DeletionRequested() )
		{
			DeleteCharacter( pCharacter->Data() );
		}
		else
		{
			pCharacter->Data()->Update( FocalPoint,
										TimeElapsed,
										*this);
			if( pCharacter->Data()->InActiveRange() &&
				pCharacter->Data()->Alive() )
			{
				UpdateProximity( pCharacter->Data()->Position() );
			}
		}
		pCharacter = pNext;
	}
} // CLevel::UpdateCharacters()

void CLevel::UpdateCharacterDeaths( void )
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	CLLNode<CCharacter>*	pNext;
	while ( pCharacter != NULL )
	{
		pNext = pCharacter->Next();
		if( pCharacter->Data()->AIState() == KAIDying )
		{
			pCharacter->Data()->CompleteDeath( *this );
		}
		pCharacter = pNext;
	}
} // CLevel::UpdateCharacterDeaths()

void CLevel::UpdateCharacterAnimation(  float32 TimeElapsed )			// time elapsed in seconds
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		pCharacter->Data()->UpdateAnimation( TimeElapsed);
		pCharacter = pCharacter->Next();
	}
} // CLevel::UpdateCharacterAnimation()

void CLevel::UpdateArrowTrails( float32 TimeElapsed )	// time elapsed in seconds
{
	if( TimeElapsed == 0 )
	{
		return;
	}
	CLLNode<CArrowTrail>*	pArrowTrail	= m_pArrowTrailList->GetFirst();
	CLLNode<CArrowTrail>*	pNext;
	while ( pArrowTrail != NULL )
	{
		pNext = pArrowTrail->Next();

		pArrowTrail->Data()->Update( TimeElapsed );
		if( pArrowTrail->Data()->Opacity() <= 0 )
		{
			m_pArrowTrailList->Remove( pArrowTrail );
		}
		pArrowTrail = pNext;
	}
} // CLevel::UpdateArrowTrails()

void CLevel::UpdateSFX( float32 TimeElapsed )	// time elapsed in seconds
{
	if( TimeElapsed == 0 )
	{
		return;
	}
	CLLNode<CSFX>*	pSFX	= m_pSFXList->GetFirst();
	CLLNode<CSFX>*	pNext;
	while ( pSFX != NULL )
	{
		pNext = pSFX->Next();

		pSFX->Data()->Update( *this, TimeElapsed );
		if( pSFX->Data()->Complete() )
		{
			m_pSFXList->Remove( pSFX );
		}
		pSFX = pNext;
	}
} // CLevel::UpdateSFX()

void CLevel::UpdateItems( float32 TimeElapsed )	// time elapsed in seconds
{
	if( TimeElapsed == 0 )
	{
		return;
	}
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	CLLNode<CItem>*	pNext;
	while ( pItem != NULL )
	{
		pNext = pItem->Next();

		pItem->Data()->Update( TimeElapsed,
							   *this);
		pItem = pNext;
	}
} // CLevel::UpdateItems()

void CLevel::NotifyOfDeletion( CCharacter* pDeletedCharacter )	// character being deleted
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		pCharacter->Data()->NotifyOfDeletion( pDeletedCharacter );
		pCharacter = pCharacter->Next();
	}

	CLLNode<CSFX>*	pSFX	= m_pSFXList->GetFirst();
	CLLNode<CSFX>*	pNext;
	while ( pSFX != NULL )
	{
		pNext = pSFX->Next();

		pSFX->Data()->NotifyOfDeletion( pDeletedCharacter );
		pSFX = pNext;
	}

} // CLevel::NotifyOfDeletion()

void CLevel::NotifyOfDeletion( CItem* pItem )	// item being deleted
{
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		pCharacter->Data()->NotifyOfDeletion( pItem );
		pCharacter = pCharacter->Next();
	}
} // CLevel::NotifyOfDeletion()

void CLevel::CalculateLocalLight( const D3DXVECTOR3& FocalPoint,	// focal point of light
								  float32 TimeElapsed,				// for smoothing
								  float32 LightRadius )				// light falloff
{
	// update local light for all characters
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		pCharacter->Data()->CalculateLocalLight( IsTorchlit(),
												 TimeElapsed,
												 FocalPoint,
												 LightRadius );
		pCharacter = pCharacter->Next();
	}

	// update local light for all Items
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		pItem->Data()->CalculateLocalLight( IsTorchlit(),
											FocalPoint,
											LightRadius );
		pItem = pItem->Next();
	}
}

void CLevel::ClearEvents( void )
{
	// clear any floating events from the previous tick
	m_pEventTextList->Destroy();

} // CLevel:ClearEvents()

void CLevel::UpdateLevelWideVisibility( void )
{
	if( m_pLevelMesh != NULL )
	{
		m_pLevelMesh->UpdateLevelWideVisibility( *this,
												m_pD3DDevice );
	}
} // CLevel::UpdateLevelWideVisibility()

void CLevel::Update( float32 TimeElapsed )	// time elapsed in seconds
{
	if( TimeElapsed == 0 )
	{
		return;
	}
	if( m_pLevelMesh != NULL )
	{
		m_pLevelMesh->Update( TimeElapsed );
	}

	for( uint32 i = 0; i < m_pLevelProps.size(); i++ )
	{
		m_pLevelProps[i]->Update( TimeElapsed );
	}

	m_TorchLightFlicker += TimeElapsed * RandomBetween( 2, 5 );
	if( RandomBetween( 0, 10 ) > 7 )
	{
		m_TorchLightFlicker += RandomBetween( -.2f, .2f );
	}
	while( m_TorchLightFlicker >= 2.0f * (float32)KPI )
	{
		m_TorchLightFlicker -= 2.0f * (float32)KPI;
	}

	UpdateItems( TimeElapsed );

	UpdateArrowTrails( TimeElapsed );

	UpdateSFX( TimeElapsed );

	EmitParticles( );
} // CLevel::Update()

void CLevel::UpdateProximity( const D3DXVECTOR3& Position )	// position of mobile object
{
	for( uint32 i = 0; i < m_pLevelProps.size(); i++ )
	{
		m_pLevelProps[i]->UpdateProximity( Position );
	}

} // CLevel::UpdateProximity()

void CLevel::ClearPassabilityData( void )
{
	for( uint32 i = 0; i < m_PathingCellsWide; i++ )
	{
		for( uint32 j = 0; j < m_PathingCellsHigh; j++ )
		{
			m_MapPassabilityData[i][j] = 0;
			m_ObjectPassabilityData[i][j] = 0;
		}
	}
} // CLevel::ClearPassabilityData()

void CLevel::ClearVisibilityData( void )
{
	m_pLevelHistory->ClearVisibilityData();
	ClearFogVisibilityData();
} // CLevel::ClearVisibilityData()

void CLevel::ClearFogVisibilityData( void )
{
	if( m_VisibilityData != NULL )
	{
		for( uint32 i = 0; i < m_VisibilityCellsWide; i++ )
		{
			memset( m_VisibilityData[i], kFalse, m_VisibilityCellsHigh );
/*			for( uint32 j = 0; j < m_VisibilityCellsHigh; j++ )
			{
				m_VisibilityData[i][j] = kFalse;
			}*/
		}
	}
} // CLevel::ClearFogVisibilityData()

void CLevel::DiscoverAll( void )
{
	m_pLevelHistory->DiscoverAll();
	UpdateLevelWideVisibility();
} // CLevel::DiscoverAll()

void CLevel::DumpToFile( void )
{
	if( m_pMaze != NULL )
	{
		m_pMaze->DumpToFile( CVistaFileUtilities::GetSharedDataPath() + "//maze.txt" );
	}
} // CLevel::DumpToFile()

void CLevel::Create( CLoadingMenu& pLoadingMenu,	// loading menu ( so that we can update our loading bar )
					 int32 Seed,		// dungeon seed
					 uint32 Depth )		// level depth
{
	DELETE_SAFELY( m_pLevelMesh );
	DELETE_SAFELY( m_pLevelCollision );
	DELETE_SAFELY( m_pPropQuadTree );
	DELETE_SAFELY( m_pMaze );

	m_LevelDepth = Depth;
	m_LevelChangeRequest = Depth;

	// depth 0 is town!
	if( m_LevelDepth == 0 )
	{
		m_pRefManager.RemoveMaterial( m_pShadowFade );
		m_pShadowFade = m_pRefManager.AddMaterialAlpha( m_pD3DDevice, 
														"IMAGES//dayshadowfade.png",
														"IMAGES//dayshadowfadea.png" );
		m_Torchlit = kFalse;

		m_pLevelCollision = new CLevelCollision();
		m_pLevelCollision->CreateForScene();

		m_pLevelHistory = new CLevelHistory( LevelDepth(),
											 0,
											 0 );

		m_pLevelScene = m_pRefManager.AddScene( m_pD3DDevice,
												"TOWN/" + CGame::GetLocaleString() + "/town.scn", // TDC 2008_02_05
												"",
												kTrue,
												kFalse,
												KSortQuadtree );

		pLoadingMenu.SetProgressPercentage( 25 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelCollisionScene = m_pRefManager.AddCollisionScene( "TOWN/" + CGame::GetLocaleString() + "/town.col",
																  KSortQuadtree );

		pLoadingMenu.SetProgressPercentage( 30 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		D3DXMATRIX Identity;
		D3DXMatrixIdentity( &Identity );
		m_pLevelCollision->AddFaceList( m_pLevelCollisionScene->FaceList(), Identity );

		m_pLevelCollision->Finalize();

		float32 Width = m_pLevelCollisionScene->GetMaxBounds().x - m_pLevelCollisionScene->GetMinBounds().x;
		float32 Height = m_pLevelCollisionScene->GetMaxBounds().z - m_pLevelCollisionScene->GetMinBounds().z;
		m_PathingCellsWide = (uint32)( Width / KPathingCellWidth );
		m_PathingCellsHigh = (uint32)( Height / KPathingCellWidth );

		m_MapPassabilityData = new int16*[m_PathingCellsWide];
		m_ObjectPassabilityData = new int16*[m_PathingCellsWide];
		for( uint32 i = 0; i < m_PathingCellsWide; i++ )
		{
			m_MapPassabilityData[i] = new int16[m_PathingCellsHigh];
			m_ObjectPassabilityData[i] = new int16[m_PathingCellsHigh];
		}

		m_pLevelProps.clear();

		m_pPropQuadTree = new CQuadtree< CLevelProp* >( m_pLevelCollisionScene->GetMinBounds(),
														m_pLevelCollisionScene->GetMaxBounds(),
														1 );
		pLoadingMenu.SetProgressPercentage( 32 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		CreateProps();

		pLoadingMenu.SetProgressPercentage( 35 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		CreatePropCollision();

		pLoadingMenu.SetProgressPercentage( 37 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelCollision->Finalize();

		pLoadingMenu.SetProgressPercentage( 40 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		CreateLevelAudio();

		FindParticles();

		pLoadingMenu.SetProgressPercentage( 45 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

	}
	else
	{
		m_pMaze = new CMaze( *m_pTemplate, Seed, Depth );

		pLoadingMenu.SetProgressPercentage( 25 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelCollision = new CLevelCollision();
		m_pLevelCollision->Create( *this, *m_pMaze, m_TileWidth, m_TileHeight );

		m_PathingCellsWide = m_PathingCellsPerTile * m_pMaze->Width();
		m_PathingCellsHigh = m_PathingCellsPerTile * m_pMaze->Height();

		m_VisibilityCellsWide = KVisibilityCellsPerTile * m_pMaze->Width();
		m_VisibilityCellsHigh = KVisibilityCellsPerTile * m_pMaze->Height();

		m_MapPassabilityData = new int16*[m_PathingCellsWide];
		m_ObjectPassabilityData = new int16*[m_PathingCellsWide];
		for( uint32 i = 0; i < m_PathingCellsWide; i++ )
		{
			m_MapPassabilityData[i] = new int16[m_PathingCellsHigh];
			m_ObjectPassabilityData[i] = new int16[m_PathingCellsHigh];
		}

		m_pLevelHistory = new CLevelHistory( LevelDepth(),
											m_VisibilityCellsWide,
											m_VisibilityCellsHigh );

		m_VisibilityData = new bool*[m_VisibilityCellsWide];
		for( int32 i = 0; i < m_VisibilityCellsWide; i++ )
		{
			m_VisibilityData[i] = new bool[m_VisibilityCellsHigh];
		}

		ClearVisibilityData();

		m_pLevelProps.clear();

		D3DXVECTOR3 MinBounds( -m_TileWidth,
							0,
							-m_TileWidth * ( m_pMaze->Height() + 2 ) );
		D3DXVECTOR3 MaxBounds( m_TileWidth * ( m_pMaze->Width() + 2 ),
							100,
							m_TileWidth );
		m_pPropQuadTree = new CQuadtree< CLevelProp* >( MinBounds,
														MaxBounds,
														1 );

		pLoadingMenu.SetProgressPercentage( 30 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelMesh = new CLevelMesh( m_pD3DDevice,
									m_pRefManager,
									m_pSettings,
									*m_pTemplate,
									m_pMaze->Width(),
									m_pMaze->Height(),
									m_TileWidth );

		pLoadingMenu.SetProgressPercentage( 35 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		for( uint32 y = 0; y < m_pMaze->Height(); y++ )
		{
			for( uint32 x = 0; x < m_pMaze->Width(); x++ )
			{
				if( m_pMaze->IsCliff( x, y ) )
				{
					D3DXVECTOR3 Position( PositionFromCellPosition( x, y ) );

					uint32	  SubWallType = m_pMaze->WallType( x, y );
					EWallType Type = m_pMaze->WallTypeNW( x, y );
					int16	  CellHeight = m_pMaze->CellHeight( x, y );

					if( CellHeight <= 0 )
					{
						if( CellHeight < 0 )
						{
							Position.y += m_TileHeight * CellHeight;
						}
						Position.y -= m_TileHeight;
					}
					else if( CellHeight >= 2 )
					{
						Position.y += m_TileHeight;
					}
					// 
					uint32 SubType = 0;
					if( m_pMaze->IsFlat( x, y ) )
					{	
						SubType = RandomIntegerBetween( 0, m_pTemplate->TileTemplates( SubWallType, Type ) - 1 );
					}
					if( Type == KWTypeOpen )
					{
						Type = KWTypeNWSolid;
					}
					if( Type != KWTypeOpen &&
						Type != KWTypeNWSolid )
					{
						AddWall( SubWallType, Type, SubType, Position );
					}
					Type = m_pMaze->WallTypeNE( x, y );
					if( Type == KWTypeOpen )
					{
						Type = KWTypeNESolid;
					}
					if( Type != KWTypeOpen &&
						Type != KWTypeNESolid )
					{
						AddWall( SubWallType, Type, SubType, Position );
					}
					Type = m_pMaze->WallTypeSW( x, y );
					if( Type == KWTypeOpen )
					{
						Type = KWTypeSWSolid;
					}
					if( Type != KWTypeOpen  &&
						Type != KWTypeSWSolid )
					{
						AddWall( SubWallType, Type, SubType, Position );
					}
					Type = m_pMaze->WallTypeSE( x, y );
					if( Type == KWTypeOpen )
					{
						Type = KWTypeSESolid;
					}
					if( Type != KWTypeOpen &&
						Type != KWTypeSESolid )
					{
						AddWall( SubWallType, Type, SubType, Position );
					}
				}
			}
		}

		pLoadingMenu.SetProgressPercentage( 37 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelMesh->SupplyCollision( *m_pLevelCollision->FaceList() );

		pLoadingMenu.SetProgressPercentage( 40 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelMesh->Finalize( *this, m_pD3DDevice );

		pLoadingMenu.SetProgressPercentage( 42 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		m_pLevelCollision->Finalize();

		pLoadingMenu.SetProgressPercentage( 44 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		CreateProps();

		pLoadingMenu.SetProgressPercentage( 46 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		CreatePropCollision();

		pLoadingMenu.SetProgressPercentage( 48 );
		pLoadingMenu.RenderUI( m_pD3DDevice, kTrue );

		CreateLevelAudio();

		m_pLevelCollision->Finalize();
	}

	CalculatePassability();

} // CLevel::Create()

void CLevel::FindCharacters( CCharacter* pPlayerCharacter )	// player character
{
	if( m_pLevelScene == NULL )
	{

		return;
	}
	int32 PlayerLevel( 1 );
	if( pPlayerCharacter != NULL )
	{
		PlayerLevel = pPlayerCharacter->Level();
	}

	bool StashExists( kFalse );
	// find out if there is a stash in the level, so that we don't create a new one 
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data()->IsMerchant() &&
			pCharacter->Data()->MerchantType() == KMerchantStash )
		{
			StashExists = kTrue;
		}
		pCharacter = pCharacter->Next();
	}

	for( uint32 i = 0; i < m_pLevelScene->GetPointCount(); i++ )
	{
		CDataPoint& Point = m_pLevelScene->GetPoint( i );
		std::string Name = StringUpper( Point.Name() );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "MERCHANT" )
		{
			std::string UserData = StringUpper( Point.UserData() );
			queue<std::string>	Tokens;
			std::string Name( "" );
			EMerchant Type( KMerchantTraveling );
			TokenizeString( &Tokens, UserData, "\n=\t\r", '#' );	

			// parse all user elements of the positional audio
			while( Tokens.size() > 0 )
			{
				std::string Key = Tokens.front();
				Tokens.pop();
				if( Key == "NAME" )	
				{
					if( Tokens.size() > 0 )
					{
						Name = Tokens.front();
					}
				}
				if( Key == "TYPE" )	
				{
					if( Tokens.size() > 0 )
					{
						std::string TypeName = StringUpper( Tokens.front() );
						for( uint32 j = 0; j < KMerchantTypes; j++ )
						{
							if( KMerchantName[j] == TypeName )
							{
								Type = (EMerchant)j;
								break;
							}
						}
					}
				}
				if( Tokens.size() > 0 )
				{
					Tokens.pop();
				}
			}

			if( !( Type == KMerchantStash && StashExists ) )
			{
				// add a salesman!
				CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( Name );
				if( pDescription != NULL )
				{
					CCharacter* pSalesman = AddMerchant( *pDescription,
														 RandomOpenPosition( Point.Position(), 4 ),
														 Type,
														 PlayerLevel );
					pSalesman->SetOrientation( Point.Orientation() );

				}
			}
		}
		else if( Name.length() >= 10 && Name.substr( 0, 10 ) == "QUESTGIVER" )
		{
			std::string UserData = StringUpper( Point.UserData() );
			queue<std::string>	Tokens;
			std::string Name( "" );
			EMerchant Type( KMerchantTraveling );
			TokenizeString( &Tokens, UserData, "\n=\t\r", '#' );	

			// parse all user elements of the positional audio
			while( Tokens.size() > 0 )
			{
				std::string Key = Tokens.front();
				Tokens.pop();
				if( Key == "NAME" )	
				{
					if( Tokens.size() > 0 )
					{
						Name = Tokens.front();
					}
				}
				if( Tokens.size() > 0 )
				{
					Tokens.pop();
				}
			}

			// add a quest-giver!
			CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( Name );
			if( pDescription != NULL )
			{
				CCharacter* pQuestGiver = AddMonster( *pDescription,
													 RandomOpenPosition( Point.Position(), 4 ) );

				pQuestGiver->SetIsQuestGiver( kTrue );

				if( !pPlayerCharacter->HasQuestsForCharacter( pQuestGiver->Name() ) )
				{
					int32 DeepestLevel( 0 );
					for( uint32 q = 0; q < pPlayerCharacter->LevelHistories(); q++ )
					{
						if( pPlayerCharacter->GetLevelHistoryByIndex( q )->Level() >
							DeepestLevel )
						{
							DeepestLevel = pPlayerCharacter->GetLevelHistoryByIndex( q )->Level();
						}
					}
					// give them a random quest
					CQuest* pQuest = GenerateQuest( m_pQuestDescriptions,
													pQuestGiver->Name(),
													DeepestLevel );
					pQuestGiver->GiveQuest( *pQuest );
					DELETE_SAFELY( pQuest );
				}

			}
		}
		else if( Name.length() >= 16 && Name.substr( 0, 16 ) == "MASTERQUESTGIVER" )
		{
			std::string UserData = StringUpper( Point.UserData() );
			queue<std::string>	Tokens;
			std::string Name( "" );
			EMerchant Type( KMerchantTraveling );
			TokenizeString( &Tokens, UserData, "\n=\t\r", '#' );	

			// parse all user elements of the positional audio
			while( Tokens.size() > 0 )
			{
				std::string Key = Tokens.front();
				Tokens.pop();
				if( Key == "NAME" )	
				{
					if( Tokens.size() > 0 )
					{
						Name = Tokens.front();
					}
				}
				if( Tokens.size() > 0 )
				{
					Tokens.pop();
				}
			}

			// add a quest-giver!
			CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( Name );
			if( pDescription != NULL )
			{
				CCharacter* pQuestGiver = AddMonster( *pDescription,
													 RandomOpenPosition( Point.Position(), 4 ) );

				pQuestGiver->SetIsQuestGiver( kTrue );

			}
		}

	}

} // CLevel::FindCharacters()

CQuest* CLevel::GenerateQuest( CQuestDescriptions& pQuestDescriptions,	// random quest descriptions
							   const std::string& GiverName,			// name of quest giver
							   uint32 Level )							// level (difficulty) of quest
{
	EQuestType Type = KQuestKillMonster;

	CItem* pItem = NULL;
	CItem* pQuestItem = NULL;
	uint32 QuestType = RandomIntegerBetween( 0, KQuestMasterQuest - 1 );
	Type = (EQuestType)QuestType;

	if( RandomIntegerBetween( 0, 100 ) < 70 )
	{
		int32 MinLevel = Level * 2;
		int32 MaxLevel = Level * 2 + 5;
		uint32 Item = RandomQuestItemIndex();
		pItem = new CItem( m_pD3DDevice,
						   m_pRefManager,
						   m_pSettings,
						   m_pParticleManager,
						   m_pGlowManager,
						   m_pPositionalAudioManager,
						   *m_pItemManifest.ItemDescription( Item ) );

		// guarantee that these item types DO get a bonus. Period.
		if( pItem->Category() == KCategoryWeapon ||
			pItem->Category() == KCategoryArmor ||
			pItem->Category() == KCategoryJewelry )
		{
			 while( pItem->Effects( KActivationPassive ) == 0 &&
					pItem->DamageBonuses() == 0 )
			 {
				pItem->BestowPowers( MaxLevel, kFalse, MinLevel );
			 }
		}

		if( RandomIntegerBetween( 0, 100 ) < 20 )
		{
			pItem->BestowUniquePowers( m_pNameGenerator, MaxLevel, kFalse, MinLevel );
		}
		if( RandomIntegerBetween( 0, 100 ) < 10 &&
			!pItem->IsUnique() )
		{
			pItem->BestowSockets();
		}
		if( RandomIntegerBetween( 0, 1000 ) < 100 )
		{
			pItem->BestowGrade( Difficulty() );
		}
		pItem->SetIdentified( kTrue );
	}

	if( Type == KQuestMonsterItem ||
		Type == KQuestArmyItem ||
		Type == KQuestFindItem )
	{
		int32 MinLevel = Level * 2;
		int32 MaxLevel = Level * 2 + 2;
		uint32 Item = RandomQuestItemIndex();
		pQuestItem = new CItem( m_pD3DDevice,
						   m_pRefManager,
						   m_pSettings,
						   m_pParticleManager,
						   m_pGlowManager,
						   m_pPositionalAudioManager,
						   *m_pItemManifest.ItemDescription( Item ) );

		// guarantee that these item types DO get a bonus. Period.
		if( pQuestItem->Category() == KCategoryWeapon ||
			pQuestItem->Category() == KCategoryArmor ||
			pQuestItem->Category() == KCategoryJewelry )
		{
			 while( pQuestItem->Effects( KActivationPassive ) == 0 &&
					pQuestItem->DamageBonuses() == 0 )
			 {
				pQuestItem->BestowPowers( MaxLevel, kFalse, MinLevel );
			 }
		}

		//if( RandomIntegerBetween( 0, 100 ) < 10 )
		{
			pQuestItem->BestowUniquePowers( m_pNameGenerator, MaxLevel, kFalse, MinLevel );
		}
		if( RandomIntegerBetween( 0, 100 ) < 10 &&
			!pQuestItem->IsUnique() )
		{
			pQuestItem->BestowSockets();
		}
		if( RandomIntegerBetween( 0, 1000 ) < 100 )
		{
			pQuestItem->BestowGrade( Difficulty() );
		}
		pQuestItem->SetIdentified( kTrue );
	}


	CQuest* pQuest = new CQuest( m_pNameGenerator,
								 pQuestDescriptions,
								 m_pCharacterManifest,
								 m_pItemManifest,
								 Level + 1,
								 Type,
								 pQuestItem,
								 pItem,
								 Difficulty(),
								 GiverName );

	DELETE_SAFELY( pItem );
	DELETE_SAFELY( pQuestItem );

	return pQuest;
} // CLevel::GenerateQuest()

void CLevel::FindParticles( void )
{
	if( m_pLevelScene == NULL )
	{
		return;
	}
	for( uint32 i = 0; i < m_pLevelScene->GetPointCount(); i++ )
	{
		CDataPoint& Point = m_pLevelScene->GetPoint( i );
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
			if( GlowIndex != -1 )
			{
				D3DXVECTOR3 Position = Point.Position();
				m_pGlowManager.AddGlow( GlowIndex, Position );
			}
		}
	}

} // CLevel::FindParticles()

void CLevel::EmitParticles( void )
{
	for( uint32 i = 0; i < m_ParticleType.size(); i++ )
	{
		D3DXMATRIX Transform = m_ParticleOrientation[i];
		Transform._41 = m_ParticlePosition[i].x;
		Transform._42 = m_ParticlePosition[i].y;
		Transform._43 = m_ParticlePosition[i].z;
		m_pParticleManager.AddParticle( m_ParticleType[i], Transform );
	}

} // CLevel::EmitParticles()

// generate tables to bias item/monster creation by rarity
void CLevel::CreateRandomizationTables( int32 PlayerLevel )	// level of player for quest item generation
{

	int32 AcceptableDepth( LevelDepth() );

	AcceptableDepth = (int32)ceil( (float32)AcceptableDepth * KDifficultyItemMultiplier[Difficulty()] );

	if( AcceptableDepth > 100 )
	{
		AcceptableDepth = 100;
	}

	int32 AcceptableMonsterDepth( LevelDepth() );

	AcceptableMonsterDepth = (int32)ceil( (float32)AcceptableMonsterDepth * KDifficultyAppearanceMultiplier[Difficulty()] );

	if( AcceptableMonsterDepth > 98 )
	{
		AcceptableMonsterDepth = 98;
	}

	for( uint32 i = 0; i < m_pCharacterManifest.Characters(); i++ )
	{
		CCharacterDescription* pDescription = m_pCharacterManifest.CharacterDescription( i );
		if( pDescription->Alignment() == KEvil &&
			pDescription->MinimumDepth() <= AcceptableMonsterDepth &&
			pDescription->MaximumDepth() >= AcceptableMonsterDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_MonsterRandomizer.AddChoice( i, Rarity );
			}
		}
		if( pDescription->Alignment() == KEvil &&
			pDescription->MinimumDepth() <= AcceptableMonsterDepth + 1 &&
			pDescription->MaximumDepth() >= AcceptableMonsterDepth + 2 )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_UniqueMonsterRandomizer.AddChoice( i, Rarity );
			}
		}
	}
	for( int32 i = 0; i < m_pItemManifest.TakeableItems(); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.TakeableItemDescription( i );


		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_TakeableItemRandomizer.AddChoice( i, Rarity );
				m_RareTakeableItemRandomizer.AddChoice( i, Rarity );
				if( pDescription->Type() == KItemGem ||
					pDescription->Category() == KCategoryJewelry )
				{
					m_JewelryRandomizer.AddChoice( i, Rarity );
				}
				if( pDescription->Type() == KItemGem )
				{
					m_GemRandomizer.AddChoice( i, Rarity );

					Rarity = 1000 - ( pDescription->Rarity() / 2 );
					if( Rarity < 0 )
					{
						Rarity = 0;
					}
					if( Rarity > 0 )
					{
						m_RareGemRandomizer.AddChoice( i, Rarity );
					}
				}
			}
		}
		else if( pDescription->MinimumDepth() <= AcceptableDepth + 11 &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_RareTakeableItemRandomizer.AddChoice( i, Rarity );
			}
			if( pDescription->Type() == KItemGem )
			{
				Rarity = 1000 - ( pDescription->Rarity() / 2 );
				if( Rarity < 0 )
				{
					Rarity = 0;
				}
				if( Rarity > 0 )
				{
					m_RareGemRandomizer.AddChoice( i, Rarity );
				}
			}
		}
	}

	for( int32 i = 0; i < m_pItemManifest.Items( KCategoryContainer ); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( KCategoryContainer, i );

		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity <= 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				if( pDescription->Type() == KItemContainer )
				{
					m_BarrelRandomizer.AddChoice( i, Rarity );
				}
				else
				{
					m_ContainerRandomizer.AddChoice( i, Rarity );
				}
			}
		}
	}

	for( int32 i = 0; i < m_pItemManifest.Items( KItemPotion ); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( KItemPotion, i );

		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity <= 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_PotionRandomizer.AddChoice( i, Rarity );
			}
		}
	}

	for( int32 i = 0; i < m_pItemManifest.Items( KCategoryWeapon ); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( KCategoryWeapon, i );

		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_WeaponRandomizer.AddChoice( i, Rarity );
				m_RareWeaponRandomizer.AddChoice( i, Rarity );
			}
		}
		else if( pDescription->MinimumDepth() <= AcceptableDepth + 5 &&
			pDescription->MaximumDepth() >= AcceptableDepth + 5 )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_RareWeaponRandomizer.AddChoice( i, Rarity );
			}
		}
	}

	for( int32 i = 0; i < m_pItemManifest.Items( KCategoryEvent ); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( KCategoryEvent, i );

		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_EventRandomizer.AddChoice( i, Rarity );
			}
		}
	}


	for( int32 i = 0; i < m_pItemManifest.Items( KCategoryArmor ); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( KCategoryArmor, i );

		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_ArmorRandomizer.AddChoice( i, Rarity );
			}
		}
	}

	for( int32 i = 0; i < m_pItemManifest.Items( KItemShield ); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( KItemShield, i );

		if( pDescription->MinimumDepth() <= AcceptableDepth &&
			pDescription->MaximumDepth() >= AcceptableDepth )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_ShieldRandomizer.AddChoice( i, Rarity );
				m_RareShieldRandomizer.AddChoice( i, Rarity );
			}
		}
		else if( pDescription->MinimumDepth() <= AcceptableDepth + 5 &&
				  pDescription->MaximumDepth() >= AcceptableDepth + 5 )
		{
			int32 Rarity = 1000 - pDescription->Rarity();
			if( Rarity < 0 )
			{
				Rarity = 0;
			}
			if( Rarity > 0 )
			{
				m_RareShieldRandomizer.AddChoice( i, Rarity );
			}
		}
	}

	for( int32 i = 0; i < m_pItemManifest.Items(); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( i );

		if( pDescription->MinimumDepth() <= PlayerLevel * 2 + 3 &&
			pDescription->MaximumDepth() >= PlayerLevel * 2 + 8 )
		{
			if( ( pDescription->Category() == KCategoryJewelry ||
				  pDescription->Category() == KCategoryWeapon ||
				  pDescription->Category() == KCategoryArmor ) &&
				  !pDescription->IsUnique() )
			{
				int32 Rarity = 1000 - pDescription->Rarity();
				if( Rarity < 0 )
				{
					Rarity = 0;
				}
				if( Rarity > 0 )
				{
					m_QuestRandomizer.AddChoice( i, Rarity );
				}
			}
		}
	}

	int32 FishingDepth = AcceptableDepth;
	if( FishingDepth == 0 )
	{
		FishingDepth = 3;
	}
	for( int32 i = 0; i < m_pItemManifest.Items(); i++ )
	{
		CItemDescription* pDescription = m_pItemManifest.ItemDescription( i );

		if( pDescription->MinimumFishingDepth() <= FishingDepth &&
			pDescription->MaximumFishingDepth() >= FishingDepth )
		{
			if( pDescription->FishingRarity() < 1000 )
			{

				int32 ItemRarity = 1000 - pDescription->FishingRarity();
				if( ItemRarity < 0 )
				{
					ItemRarity = 0;
				}
				if( ItemRarity > 0 )
				{
					m_FishRandomizer.AddChoice( i, ItemRarity );
				}
			}
		}
	}

} // CLevel::CreateRandomizationTables()

void CLevel::CreateLevelAudio( void )
{
	if( m_pMaze != NULL )
	{
		for( uint32 i = 0; i < m_pMaze->PositionalAudioItems(); i++ )
		{
		
			D3DXVECTOR3 Position( PositionFromCellPosition( (uint32)m_pMaze->PositionalAudioPosition( i ).x, 
															(uint32)m_pMaze->PositionalAudioPosition( i ).z ) );
			m_pPositionalAudioManager.AddAudio( Position,
												m_pMaze->PositionalAudioPath( i ),
												255,
												m_pMaze->PositionalAudioRadius( i ) );
		}
	}

	if( m_pLevelScene != NULL )
	{
		for( uint32 i = 0; i < m_pLevelScene->GetPointCount(); i++ )
		{
			CDataPoint& Point = m_pLevelScene->GetPoint( i );
			std::string PointName = StringUpper( Point.Name() );
			std::string UserData = Point.UserData();
			if( PointName.length() >= 5 &&
				PointName.substr( 0, 5 ) == "SOUND" )
			{
				queue<std::string>	Tokens;
				std::string Path( "" );
				float32 Radius( 100 );
				uint32 BaseVolume( 255 );
				TokenizeString( &Tokens, UserData, "\n=\t\r", '#' );	

				// parse all user elements of the positional audio
				while( Tokens.size() > 0 )
				{
					std::string Key = Tokens.front();
					Tokens.pop();
					if( Key == "PATH" )	// path to sample
					{
						if( Tokens.size() > 0 )
						{
							Path = Tokens.front();
						}
					}
					else if( Key == "RADIUS" )	// max radius at which sound can be heard
					{
						if( Tokens.size() > 0 )
						{
							Radius = (float32) atof( Tokens.front().c_str() );
						}
					}
					else if( Key == "VOLUME" )	// base volume (0-255) of sound sample
					{
						if( Tokens.size() > 0 )
						{
							BaseVolume = atoi( Tokens.front().c_str() );
						}
					}
					if( Tokens.size() > 0 )
					{
						Tokens.pop();
					}
				}
				m_pPositionalAudioManager.AddAudio( Point.Position(),
													Path,
													BaseVolume,
													Radius );
			}
		}
	}
} // CLevel::CreateLevelAudio()

void CLevel::CreateProps( void )
{

	if( m_pLevelScene != NULL )
	{
		for( uint32 i = 0; i < m_pLevelScene->GetPointCount(); i++ )
		{
			std::string Name = StringUpper( m_pLevelScene->GetPoint( i ).Name() );
			std::string UserData = StringUpper( m_pLevelScene->GetPoint( i ).UserData() );
			D3DXVECTOR3 Position = m_pLevelScene->GetPoint( i ).Position();
			if( Name.length() >= 5 && Name.substr( 0, 5 ) == "START" )
			{
				m_StartPosition =Position;
				Position.y = 0;
			}
			else if( Name.length() >= 15 && Name.substr( 0, 15 ) == "DUNGEONENTRANCE" )
			{
				Position.y = 0;
				CItem* pItem =  new CItemBlank( m_pD3DDevice,
												m_pRefManager,
												m_pSettings,
												m_pParticleManager,
											    m_pGlowManager,
												m_pPositionalAudioManager,
												LoadLocalizedString(1157),
												Position,
												D3DXVECTOR3( -m_TileWidth * .3f, 0, -m_TileWidth * .3f ),
												D3DXVECTOR3( m_TileWidth * .3f, m_TileHeight, m_TileWidth *.3f ),
												KItemStairsDown );
				pItem->SetInWorld( kTrue );
				m_pItemList->Queue( pItem );
			}
			else if( Name.length() >= 11 && Name.substr( 0, 11 ) == "FISHINGHOLE" )
			{
				CItem* pItem =  new CItemBlank( m_pD3DDevice,
												m_pRefManager,
												m_pSettings,
												m_pParticleManager,
											    m_pGlowManager,
												m_pPositionalAudioManager,
												LoadLocalizedString(1147),
												Position,
												D3DXVECTOR3( -m_TileWidth * .2f, 0, -m_TileWidth * .2f ),
												D3DXVECTOR3( m_TileWidth * .2f, m_TileHeight * .5f, m_TileWidth *.2f ),
												KItemFishingHole );
				pItem->SetInWorld( kTrue );
				m_pItemList->Queue( pItem );

				CSFXDescription* pSFX = new CSFXDescription( KSFXStationaryOwner,
															"SPELLS\\EFFECTS\\FIREFLY\\firefly.ams",
															-1,
															kTrue,
															0,
															0,
															kFalse,
															kFalse );

				AddSFX( *pSFX,
						NULL,
						NULL,
						Position,
						Position );

				DELETE_SAFELY( pSFX );

			}
			else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "PROP" )
			{
				queue<std::string>	Tokens;
				std::string Path( "" );
				std::string CollisionPath( "" );
				bool Collideable( kFalse );
				bool Shadow( kFalse );
				TokenizeString( &Tokens, UserData, "\n=\t\r", '#' );	

				// parse all user elements of the positional audio
				while( Tokens.size() > 0 )
				{
					std::string Key = Tokens.front();
					Tokens.pop();
					if( Key == "PATH" )	
					{
						if( Tokens.size() > 0 )
						{
							Path = Tokens.front();
						}
					}
					if( Key == "COLLISIONPATH" )	
					{
						if( Tokens.size() > 0 )
						{
							CollisionPath = Tokens.front();
						}
					}
					else if( Key == "COLLIDEABLE" )
					{
						if( Tokens.size() > 0 )
						{
							Collideable = atoi( Tokens.front().c_str() ) == 1;
						}
					}
					else if( Key == "SHADOW" )
					{
						if( Tokens.size() > 0 )
						{
							Shadow = atoi( Tokens.front().c_str() ) == 1;
						}
					}
					if( Tokens.size() > 0 )
					{
						Tokens.pop();
					}
				}

				CPropTemplate* pPropTemplate = new CPropTemplate( "",
																  Path,
																  CollisionPath,
																  Collideable,
																  Shadow,
																  kFalse,
																  1,
																  1,
																  D3DXVECTOR3( 0, 0, 0 ),
																  D3DXVECTOR3( 0, 0, 0 ),
																  0,
																  0 );
				CLevelProp* pProp = new CLevelProp( m_pD3DDevice,
													m_pRefManager,
													m_pSettings,
													m_pParticleManager,
													m_pGlowManager,
													m_pPositionalAudioManager,
													*pPropTemplate,
													Position,
													1,
													0 );

				D3DXMATRIX Transformation = m_pLevelScene->GetPoint( i ).Orientation();
				Transformation._41 = Position.x;
				Transformation._42 = Position.y;
				Transformation._43 = Position.z;
				pProp->SetTransformation( Transformation );

				DELETE_SAFELY( pPropTemplate );

				m_pPropQuadTree->AddData( pProp,
										  pProp->CullingBounds()->GetMinBounds(),
										  pProp->CullingBounds()->GetMaxBounds() );
				
				if( pProp->Collideable() )
				{
					int32 Material = KImpassableMaterialID;
					m_pLevelCollision->AddFaceList( pProp->FaceList(),
													pProp->Transformation(),
													Material ); 
				}

				m_pLevelProps.push_back( pProp );

			}

		}
	}
	else if( m_pMaze != NULL )
	{
		if( m_pTemplate->Props() == 0 )
		{
			return;
		}
		for( uint32 i = 0; i < m_pMaze->Props(); i++ )
		{
			CPropInstance* pProp = m_pMaze->Prop( i );

			D3DXVECTOR3 Position( PositionFromCellPosition( pProp->CellX(), pProp->CellY() ) );
			Position += pProp->Offset();
			Position.y = m_TileHeight * m_pMaze->CellHeight( pProp->CellX(), pProp->CellY() ) + FloorHeight( Position );
			AddProp( *pProp,
					Position );
		}

		for( uint32 x = 0; x < m_pMaze->Width(); x++ )
		{
			for( uint32 y = 0; y < m_pMaze->Height(); y++ )
			{
				if( m_pMaze->IsFishable( x, y ) &&
					m_pMaze->CellHeight( x, y ) < 0 )
				{
					D3DXVECTOR3 Position( PositionFromCellPosition( x, y ) );
					Position.y = 0;
					CItem* pItem =  new CItemBlank( m_pD3DDevice,
													m_pRefManager,
													m_pSettings,
													m_pParticleManager,
												    m_pGlowManager,
													m_pPositionalAudioManager,
													LoadLocalizedString(1147),
													Position,
													D3DXVECTOR3( -m_TileWidth * .2f, 0, -m_TileWidth * .2f ),
													D3DXVECTOR3( m_TileWidth * .2f, m_TileHeight * .5f, m_TileWidth *.2f ),
													KItemFishingHole );
					pItem->SetInWorld( kTrue );
					m_pItemList->Queue( pItem );

					CSFXDescription* pSFX = new CSFXDescription( KSFXStationaryOwner,
																"SPELLS\\EFFECTS\\FIREFLY\\firefly.ams",
																-1,
																kTrue,
																0,
																0,
																kFalse,
																kFalse );

					AddSFX( *pSFX,
							NULL,
							NULL,
							Position,
							Position );
					DELETE_SAFELY( pSFX );
				}
			}
		}
	}
} // CLevel::CreateProps()

void CLevel::CreatePropCollision( void )
{
	for( uint32 i = 0; i < m_pLevelProps.size(); i++ )
	{
		if( m_pLevelProps[i]->Collideable() )
		{
			m_pLevelCollision->AddFaceList( m_pLevelProps[i]->FaceList(),
											m_pLevelProps[i]->Transformation(),
											KImpassableMaterialID ); 
		}
	}

} // CLevel::CreatePropCollision()

void CLevel::AddWall( uint32 WallType,				// wallset being used
					  EWallType Type,				// type of model to add
					  uint32 SubType,				// subtype
					  const D3DXVECTOR3& Position )	// worldspace position
{
	uint32 Templates = m_pTemplate->TileTemplates( WallType, Type );
	if( SubType >= m_pTemplate->TileTemplates( WallType, Type ) ||
		SubType < 0 )
	{
		SubType = 0;
	}

	m_pLevelMesh->AddWall( *this,
						   WallType,
						   Position,
						   m_pTemplate->TileTemplate( WallType, Type, SubType ),
						   m_pTemplate->TileCollisionTemplate( WallType, Type, SubType ) );

} // CLevel::AddCell()

void CLevel::AddProp( CPropInstance& pPropInstance,	// prop being used
					  D3DXVECTOR3 Position )		// worldspace position
{

	if( pPropInstance.Template().HasPlacementRay() )
	{
		D3DXVECTOR3 Impact;
		D3DXVECTOR3 Normal;
		D3DXVECTOR3 Color;
		uint32 Material;
		D3DXVECTOR3 Start = Position;
		Start.y = pPropInstance.Template().PlacementRayHeight();
		D3DXVECTOR3 End = Start + pPropInstance.Template().PlacementRay() * 3.0f;

		if( RayCollision( Start, End, Impact, Normal, Material, Color, NULL, kFalse ) )
		{
			Impact -= Start;
			Impact -= pPropInstance.Template().PlacementRay();
			Impact.y = 0;
			Position += Impact;
		}
		else
		{
			Position += D3DXVECTOR3( pPropInstance.Template().PlacementRay().x, 0, pPropInstance.Template().PlacementRay().z );
		}
	}

	CLevelProp* pProp = new CLevelProp( m_pD3DDevice,
										m_pRefManager,
										m_pSettings,
										m_pParticleManager,
										m_pGlowManager,
										m_pPositionalAudioManager,
										pPropInstance.Template(),
										Position,
										pPropInstance.Scale(),
										pPropInstance.Angle() );
	m_pPropQuadTree->AddData( pProp,
							  pProp->CullingBounds()->GetMinBounds(),
							  pProp->CullingBounds()->GetMaxBounds() );
	

	if( pPropInstance.Template().IsStairsUp() )
	{
		CItem* pItem =  new CItemBlank( m_pD3DDevice,
										m_pRefManager,
										m_pSettings,
										m_pParticleManager,
									    m_pGlowManager,
										m_pPositionalAudioManager,
										LoadLocalizedString(1148),
										Position,
										D3DXVECTOR3( -m_TileWidth * .3f, 0, -m_TileWidth * .3f ),
										D3DXVECTOR3( m_TileWidth * .3f, m_TileHeight, m_TileWidth *.3f ),
										KItemStairsUp );
		pItem->SetInWorld( kTrue );
		m_pItemList->Queue( pItem );
	}
	else if( pPropInstance.Template().IsStairsDown() )
	{
		CItem* pItem =  new CItemBlank( m_pD3DDevice,
										m_pRefManager,
										m_pSettings,
										m_pParticleManager,
									    m_pGlowManager,
										m_pPositionalAudioManager,
										LoadLocalizedString(1149),
										Position,
										D3DXVECTOR3( -m_TileWidth * .3f, 0, -m_TileWidth * .3f ),
										D3DXVECTOR3( m_TileWidth * .3f, m_TileHeight, m_TileWidth *.3f ),
										KItemStairsDown );
		pItem->SetInWorld( kTrue );
		m_pItemList->Queue( pItem );
	}


	m_pLevelProps.push_back( pProp );
} // CLevel::AddProp())

void CLevel::Sort( CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	if( m_pLevelMesh != NULL )
	{
		m_pLevelMesh->Sort( pFrustum );
	}
	else
	{
		m_pLevelScene->Sort( *pFrustum );
	}
} // CLevel::Sort()


void CLevel::Render( const D3DXVECTOR3& FocalPoint,			// point where the camera is directed
					 CLightManager* pLightManager,			// light manager - NULL for no light manager
					 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
					 CFrustum* pFrustum,					// frustum to cull with - NULL for none
					 D3DXMATRIX* LightMatrix,				// shadow projector matrix
					 CMaterial* pShadowMaterial,			// shadow material layer
					 CMaterial* pLightMaterial )			// shadow material layer
{
	if( m_pLevelMesh != NULL )
	{
		if( m_VisibilityChanged  )
		{
			m_pLevelMesh->UpdateVisibility();
		}

		if( m_pSettings.GetSettings( KSetShowFPS ) )
		{
			float32 OffsetX( FocalPoint.x / m_TileWidth );
			float32 OffsetY( -FocalPoint.z / m_TileWidth );
			uint32 X = (uint32)floor( OffsetX );
			uint32 Y = (uint32)floor( OffsetY );
			ECellType Type = m_pMaze->CellType( X, Y );
			uint32 Height = m_pMaze->CellHeight( X, Y );
			uint32 HeightN = m_pMaze->CellHeight( X, Y - 1 );
			uint32 HeightS = m_pMaze->CellHeight( X, Y + 1 );
			uint32 HeightE = m_pMaze->CellHeight( X + 1, Y );
			uint32 HeightW = m_pMaze->CellHeight( X - 1, Y );
			m_DebugText = "\nSeed: " + IntegerToString( m_pMaze->Seed() ) + "\n" + KCellNames[Type] + "\n" + IntegerToString( Height ) + "\n" +
				"Items - " + IntegerToString( m_ItemsRendered ) + "\n" +
				"Characters - " + IntegerToString( m_CharactersRendered ) + "\n" +
				"N - " + IntegerToString( HeightN ) + "\n" +
				"S - " + IntegerToString( HeightS ) + "\n" +
				"E - " + IntegerToString( HeightE ) + "\n" +
				"W - " + IntegerToString( HeightW ) + "\n" +
				"Models - " + IntegerToString( m_pRefManager.CountModels() ) + "\n" + 
				"Animated Models - " + IntegerToString( m_pRefManager.CountAnimatedMeshes() ) + "\n"+ 
				"Animated Geometries - " + IntegerToString( m_pRefManager.CountAnimatedGeometries() ) + "\n"+ 
				"Skinned Models - " + IntegerToString( m_pRefManager.CountSkinnedMeshes() ) + "\n"+ 
				"Skinned Geometries - " + IntegerToString( m_pRefManager.CountSkinnedGeometries() ) + "\n"+ 
				"Skinned Animations - " + IntegerToString( m_pRefManager.CountSkinnedAnimations() ) + "\n"+ 
				"Scenes - " + IntegerToString( m_pRefManager.CountScenes() ) + "\n" + 
				"Samples - " + IntegerToString( m_pRefManager.CountSounds() ) + "\n" + 
				"Movies - " + IntegerToString( m_pRefManager.CountMovies() ) ;
		}

		if( m_WorldRenderState == KWRSRenderAll ||
			m_WorldRenderState != KWRSNoRenderScene )
		{
			m_pLevelMesh->Render( *this, m_pD3DDevice, pLightManager, ViewportMatrix, pFrustum );

			if( m_pSettings.Capabilities().IsCapable( KCanRenderToTexture ) &&
				m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
			{
				if( pShadowMaterial != NULL &&
					m_pSettings.GetSettings( KSetProjectedShadows ) &&
					m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
				{
					m_pLevelMesh->RenderShadowPass( m_pD3DDevice, ViewportMatrix, *LightMatrix, pShadowMaterial, pLightMaterial );
				}
			}
			else
			{
				if(	m_pSettings.Capabilities().IsCapable( KCanDoOrthoShadowTextureProjection ) )
				{
					m_pLevelMesh->RenderShadowPass( m_pD3DDevice, ViewportMatrix, *LightMatrix, m_pTorchLight, m_pTorchLight );
				}
			}
		}


		if( m_WorldRenderState == KWRSRenderAll ||
			m_WorldRenderState != KWRSNoRenderProps )
		{
			RenderProps( FocalPoint, pLightManager, ViewportMatrix, pFrustum );

			if( m_pSettings.Capabilities().IsCapable( KCanRenderToTexture ) &&
				m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
			{
				if( pShadowMaterial != NULL &&
					m_pSettings.GetSettings( KSetProjectedShadows ) &&
					m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
				{
					RenderShadowPassProps( ViewportMatrix, *LightMatrix, pLightMaterial, pFrustum );
				}
			}
			else
			{
				if(	m_pSettings.Capabilities().IsCapable( KCanDoOrthoShadowTextureProjection ) )
				{
					RenderShadowPassProps( ViewportMatrix, *LightMatrix, m_pTorchLight, pFrustum );
				}
			}
		}

		RenderCharactersBehindWall( pLightManager, ViewportMatrix, pFrustum );

		if( m_WorldRenderState == KWRSRenderAll ||
			m_WorldRenderState != KWRSNoRenderWater )
		{
			m_pLevelMesh->RenderWater( m_pD3DDevice, pLightManager, ViewportMatrix );
		}
	}
	else
	{
		m_pLevelScene->Sort( *pFrustum );
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

		m_pLevelScene->SetRenderAlpha( kFalse );
		m_pLevelScene->RenderSorted( m_pD3DDevice,
							   ViewportMatrix );
		m_pLevelScene->SetRenderAlpha( kTrue );

		if( pShadowMaterial != NULL &&
			m_pSettings.GetSettings( KSetProjectedShadows ) &&
			m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
		{
			m_pD3DDevice->SetRenderState( D3DRS_ZBIAS, 16 );
			m_pD3DDevice->SetTexture( 0, pShadowMaterial->GetDynamicTexture() );
			m_pLevelScene->RenderShadowPass( m_pD3DDevice, ViewportMatrix, *LightMatrix, NULL, kTrue );
			m_pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );
		}

		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );

		D3DXMATRIX Identity;
		D3DXMatrixIdentity( &Identity );
		pLightManager->UpdateLights( m_pD3DDevice, Identity );

		RenderCharactersBehindWall( pLightManager, ViewportMatrix, pFrustum );

		if( m_WorldRenderState == KWRSRenderAll ||
			m_WorldRenderState != KWRSNoRenderProps )
		{
			RenderProps( FocalPoint, pLightManager, ViewportMatrix, pFrustum );

			if( pShadowMaterial != NULL &&
				m_pSettings.GetSettings( KSetProjectedShadows ) &&
				m_pSettings.GetSettings( KSetShadowDetail ) > 0 )
			{
				RenderShadowPassProps( ViewportMatrix, *LightMatrix, pShadowMaterial, pFrustum );
			}
		}



	}

	if( m_WorldRenderState == KWRSRenderAll ||
		m_WorldRenderState != KWRSNoRenderItems )
	{
		RenderItems( pLightManager, ViewportMatrix, pFrustum );
	}


} // CLevel::Render()

void CLevel::RenderDetail( const D3DXVECTOR3& FocalPoint,		// point where the camera is directed
						   CLightManager* pLightManager,		// light manager - NULL for no light manager
						   const D3DXMATRIX& ViewportMatrix,	// viewport matrix
						   CFrustum* pFrustum,					// frustum to cull with - NULL for none
						   D3DXMATRIX* LightMatrix,				// shadow projector matrix
						   CMaterial* pShadowMaterial,			// shadow material layer
						   CMaterial* pLightMaterial )			// shadow material layer
{
	if( m_pLevelMesh != NULL )
	{
	}
	else
	{
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

		m_pLevelScene->SetRenderOpaque( kFalse );
		m_pLevelScene->RenderSorted( m_pD3DDevice,
									 ViewportMatrix );
		m_pLevelScene->SetRenderOpaque( kTrue );

		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
	}

} // CLevel::RenderDetail()

void CLevel::RenderArrowTrails( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
								CFrustum* pFrustum )				// frustum to cull with - NULL for none
{
	bool CanFade( kFalse );
	// make sure we're alpha blending, and turn on additive blending
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	if( m_pSettings.GetSettings( KSetAlphaFade ) )
	{
		CanFade = kTrue;

		// turn on fading
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
	}
	m_pD3DDevice->SetTexture( 0, m_pArrowTrailMaterial->GetIlluminationTexture() );

	// render all characters
	CLLNode<CArrowTrail>*	pTrail	= m_pArrowTrailList->GetFirst();
	while ( pTrail != NULL )
	{
		if( pFrustum == NULL || pTrail->Data()->InFrustum( *pFrustum ) )
		{
			if( CanFade )
			{
				uint32 Fade = (uint32)floor( pTrail->Data()->Opacity() + .5f );
				m_pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 
											D3DCOLOR_RGBA(0, 0, 0, Fade));			
			}
			
			m_pArrowTrailModel->RenderNoTexture( m_pD3DDevice, pTrail->Data()->Transformation() * ViewportMatrix );
		}
		pTrail = pTrail->Next();
	}

	if( CanFade )
	{
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	}

	// back to standard settings
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
} // CLevel::RenderArrowTrails()

void CLevel::RenderSFX( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
						CFrustum* pFrustum )				// frustum to cull with - NULL for none
{
	bool CanFade( kFalse );
	// make sure we're alpha blending, and turn on additive blending
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	/*CanFade = kFalse;
	if( m_pSettings.GetSettings( KSetAlphaFade ) )
	{
		CanFade = kTrue;

		// turn on fading
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
	}*/
	CanFade = kFalse;
	m_pD3DDevice->SetTexture( 0, m_pArrowTrailMaterial->GetIlluminationTexture() );

	// render all characters
	CLLNode<CSFX>*	pSFX	= m_pSFXList->GetFirst();
	while ( pSFX != NULL )
	{
		if( pFrustum == NULL || pSFX->Data()->InFrustum( *pFrustum ) )
		{
			/*if( CanFade )
			{
				uint32 Fade = (uint32)floor( pSFX->Data()->Opacity() + .5f );
				m_pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 
											D3DCOLOR_RGBA(0, 0, 0, Fade));			
			}*/
			
			pSFX->Data()->Render( m_pD3DDevice, ViewportMatrix );
		}
		pSFX = pSFX->Next();
	}

	if( CanFade )
	{
		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

		m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	}

	// back to standard settings
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
} // CLevel::RenderSFX()


void CLevel::RenderCharacters( CLightManager* pLightManager,		// light manager - NULL for no light manager
							   const D3DXMATRIX& ViewportMatrix,	// viewport matrix
							   CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	m_CharactersRendered = 0;
	if( m_WorldRenderState == KWRSRenderAll ||
		m_WorldRenderState != KWRSNoRenderCharacters )
	{
		// render all characters
		CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
		while ( pCharacter != NULL )
		{
			pCharacter->Data()->SetOnscreen( kFalse );
			pCharacter->Data()->Render( m_pD3DDevice, 
										pLightManager, 
										ViewportMatrix,
										pFrustum );
			if( pCharacter->Data()->Onscreen() )
			{
				m_CharactersRendered ++;
			}
			pCharacter = pCharacter->Next();
		}
	}

} // CLevel::RenderCharacters()

void CLevel::RenderCharactersBehindWall( CLightManager* pLightManager,		// light manager - NULL for no light manager
										 const D3DXMATRIX& ViewportMatrix,	// viewport matrix
										 CFrustum* pFrustum )				// frustum to cull with - NULL for none
{
	if( m_WorldRenderState == KWRSRenderAll ||
		m_WorldRenderState != KWRSNoRenderCharacters )
	{

		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER );
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );


		// render all characters
		CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
		while ( pCharacter != NULL )
		{
			pCharacter->Data()->RenderBehindWall( m_pD3DDevice, 
												pLightManager, 
												ViewportMatrix,
												pFrustum );
			pCharacter = pCharacter->Next();
		}

		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
		m_pD3DDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

	}
} // CLevel::RenderCharactersBehindWall()

void CLevel::RenderCharacterShadows( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
									 CFrustum* pFrustum )				// frustum to cull with - NULL for none
{
	// render all characters
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		pCharacter->Data()->RenderShadow( m_pD3DDevice, 
										  ViewportMatrix,
										  pFrustum );
		pCharacter = pCharacter->Next();
	}
} // CLevel::RenderCharacterShadows()

void CLevel::RenderItems( CLightManager* pLightManager,		// light manager - NULL for no light manager
						  const D3DXMATRIX& ViewportMatrix,	// viewport matrix
						  CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	m_ItemsRendered = 0;
	// render all Items
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		pItem->Data()->SetOnscreen( kFalse );
		pItem->Data()->Render( m_pD3DDevice, 
								    pLightManager, 
									ViewportMatrix,
									pFrustum );
		if( pItem->Data()->Onscreen() )
		{
			m_ItemsRendered++;
		}
		pItem = pItem->Next();
	}
} // CLevel::RenderItems()

void CLevel::RenderItemShadows( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
								CFrustum* pFrustum )				// frustum to cull with - NULL for none
{
	// render all Items
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data()->CastsShadow() )
		{
			pItem->Data()->RenderShadow( m_pD3DDevice, 
										 ViewportMatrix,
										 pFrustum );
		}
		pItem = pItem->Next();
	}
} // CLevel::RenderItemShadows()

void CLevel::RenderGlowPass( const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{
	if( m_pLevelMesh != NULL )
	{
		m_pLevelMesh->RenderGlowPass( m_pD3DDevice, ViewportMatrix  );
	}
	else
	{
		m_pLevelScene->RenderGlowPass( m_pD3DDevice, ViewportMatrix, kFalse );
	}
} // CLevel::RenderGlowPass()

void CLevel::RenderProps( const D3DXVECTOR3& FocalPoint,		// point where the camera is directed
						  CLightManager* pLightManager,			// light manager - NULL for no light manager
						  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						  CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	for( uint32 i = 0; i < m_pLevelProps.size(); i++ )
	{
		m_pLevelProps[i]->SetOnscreen( kFalse );
	}

	m_pSortedProps.resize( 0 );
	m_pPropQuadTree->Sort( *pFrustum,
						   m_pSortedProps );
	m_PropsRendered = 0;

	for( int32 i = 0; i < m_pSortedProps.size(); i++ )
	{
		m_pSortedProps[i]->Render( m_pD3DDevice,
								   pLightManager,
								   ViewportMatrix,
								   pFrustum );
		if( m_pSortedProps[i]->Onscreen() )
		{
			m_pSortedProps[i]->CalculateLocalLight( IsTorchlit(),
													FocalPoint, 
													(float32)KMaxVisibilityRadius * m_VisibilityCellWidth );
			m_PropsRendered++;
		}
	}


} // CLevel::RenderProps()

void CLevel::RenderShadowPassProps( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								    const D3DXMATRIX& LightMatrix,			// matrix for the shadow caster
								    CMaterial* pShadowMaterial,				// projected shadow material
									CFrustum* pFrustum,						// frustum to cull with - NULL for none
								    bool Orthographic )						// false by default
{
	if( pShadowMaterial->IsDynamicTextureValid() )
	{
		m_pD3DDevice->SetTexture( 0, pShadowMaterial->GetDynamicTexture() );
	}
	else
	{
		m_pD3DDevice->SetTexture( 0, pShadowMaterial->GetTexture() );
	}

	D3DXMATRIX ProjectionMatrix;
	D3DXMATRIX InverseView;

	float32 FOV		= ( float32 )( 90 * KDeg2Rad );
	if( !m_pSettings.Capabilities().IsCapable( KCanDoShadowTextureProjection ) )
	{
		Orthographic = kTrue;
	}
	if( Orthographic  )
	{
		if( m_pSettings.GetSettings( KSetShadowDetail ) == 0 )
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								125,
								125,
								.01f,
								2000 );	// far clip
		}
		else
		{
			D3DXMatrixOrthoLH ( &ProjectionMatrix, 
								50,
								50,
								.01f,
								2000 );	// far clip
		}
	}
	else
	{
		if( m_pSettings.GetSettings( KSetShadowDetail ) == 0 )
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										( float32 )( 120 * KDeg2Rad ), 
										1.0f, 
										.01f, 
										2000 );
		}
		else
		{
			D3DXMatrixPerspectiveFovLH( &ProjectionMatrix, 
										FOV, 
										1.0f, 
										.01f, 
										2000 );
		}
	}

	D3DXMatrixInverse( &InverseView, NULL, &ViewportMatrix );

	InverseView = InverseView * LightMatrix * ProjectionMatrix * m_ProjectorBias;
		
	m_pD3DDevice->SetTransform( D3DTS_TEXTURE0, &InverseView );

	// now we need to set up layer 0 to do the sort of projection we want when it comes time

	m_pD3DDevice->SetTextureStageState( 0,
										D3DTSS_ADDRESSU,
										D3DTADDRESS_CLAMP );
	m_pD3DDevice->SetTextureStageState( 0,
										D3DTSS_ADDRESSV,
										 D3DTADDRESS_CLAMP );
		
	if( Orthographic )
	{
		m_pD3DDevice->SetTextureStageState( 0,
											D3DTSS_TEXTURETRANSFORMFLAGS,
											D3DTTFF_COUNT3 );
	}
	else
	{
		m_pD3DDevice->SetTextureStageState( 0,
											D3DTSS_TEXTURETRANSFORMFLAGS,
											D3DTTFF_COUNT3 |
											D3DTTFF_PROJECTED );
	}
	m_pD3DDevice->SetTextureStageState( 0,
										 D3DTSS_TEXCOORDINDEX,
										 D3DTSS_TCI_CAMERASPACEPOSITION |
										 1 );



	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	m_pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );

	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );

	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );

	for( uint32 i = 0; i < m_pSortedProps.size(); i++ )
	{
		if( m_pSortedProps[i]->Structural() )
		{
			m_pSortedProps[i]->RenderShadow( m_pD3DDevice,
											 ViewportMatrix,
											 pFrustum );
		}
	}

	m_pD3DDevice->SetRenderState( D3DRS_ZBIAS, 0 );
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, kTrue );
	m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

	m_pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );

	m_pD3DDevice->SetTextureStageState( 0,
										 D3DTSS_TEXTURETRANSFORMFLAGS,
										 D3DTTFF_DISABLE );
	m_pD3DDevice->SetTextureStageState( 0,
										 D3DTSS_ADDRESSU,
										 D3DTADDRESS_WRAP );
	m_pD3DDevice->SetTextureStageState( 0,
										 D3DTSS_ADDRESSV,
										 D3DTADDRESS_WRAP );

} // CLevel::RenderShadowPassProps()

void CLevel::RenderPropShadows( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	m_pSortedProps.resize( 0 );
	m_pPropQuadTree->Sort( *pFrustum,
						   m_pSortedProps );

	for( uint32 i = 0; i < m_pSortedProps.size(); i++ )
	{
		if( m_pSortedProps[i]->CastsShadow() )
		{
			m_pSortedProps[i]->RenderShadow( m_pD3DDevice,
											ViewportMatrix,
											pFrustum );
		}
	}

} // CLevel::RenderPropShadows()

void CLevel::RenderPropLights( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	m_pSortedProps.resize( 0 );
	m_pPropQuadTree->Sort( *pFrustum,
						   m_pSortedProps );
	D3DXMATRIX Rotation;
	D3DXMatrixRotationX( &Rotation, (float32)KDeg2Rad * 90 );
	D3DXMATRIX YRotation;
	for( uint32 i = 0; i < m_pSortedProps.size(); i++ )
	{
		if( m_pSortedProps[i]->HasLightPool() )
		{
			D3DXMatrixRotationY( &YRotation, m_pSortedProps[i]->LightPoolAngle() );
			D3DXMATRIX Orientation;
			D3DXMatrixIdentity( &Orientation );
			Orientation = Rotation * YRotation * Orientation;
			D3DXMATRIX Translation;
			D3DXMatrixTranslation( &Translation, 
								   m_pSortedProps[i]->Position().x,
								   0,
								   m_pSortedProps[i]->Position().z );
			D3DXMATRIX Scaling;
			D3DXMatrixScaling( &Scaling, 
							   m_pSortedProps[i]->LightPoolScale(),
							   m_pSortedProps[i]->LightPoolScale(),
							   1 );
			m_pD3DDevice->SetTexture( 0, m_pSortedProps[i]->LightPoolMaterial()->GetTexture() );


			m_pTorchLightSprite->RenderNoTexture( m_pD3DDevice, Scaling * Orientation * Translation * ViewportMatrix );

		}
	}

} // CLevel::RenderPropLights()

void CLevel::RenderItemLights( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	D3DXMATRIX Rotation;
	D3DXMatrixRotationX( &Rotation, (float32)KDeg2Rad * 90 );
	D3DXMATRIX YRotation;

	// render all Items
	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data()->HasLightPool() &&
			pItem->Data()->Onscreen() )
		{
			D3DXMatrixRotationY( &YRotation, pItem->Data()->LightPoolAngle() );
			D3DXMATRIX Orientation;
			D3DXMatrixIdentity( &Orientation );
			Orientation = Rotation * YRotation * Orientation;
			D3DXMATRIX Translation;
			D3DXMatrixTranslation( &Translation, 
									pItem->Data()->Position().x,
									0,
									pItem->Data()->Position().z );
			D3DXMATRIX Scaling;
			D3DXMatrixScaling( &Scaling, 
								pItem->Data()->LightPoolScale(),
								pItem->Data()->LightPoolScale(),
								1 );
			m_pD3DDevice->SetTexture( 0, pItem->Data()->LightPoolMaterial()->GetTexture() );


			m_pTorchLightSprite->RenderNoTexture( m_pD3DDevice, Scaling * Orientation * Translation * ViewportMatrix );

		}
		pItem = pItem->Next();
	}
} // CLevel::RenderItemLights()

void CLevel::RenderCharacterLights( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
									CFrustum* pFrustum )					// frustum to cull with - NULL for none
{


	D3DXMATRIX Rotation;
	D3DXMatrixRotationX( &Rotation, (float32)KDeg2Rad * 90 );
	D3DXMATRIX YRotation;

		// render all characters
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( ( pCharacter->Data()->IsUnique() ||
			  pCharacter->Data()->HasLightPool() ) &&
			pCharacter->Data()->Alive() )
		{
			if( pCharacter->Data()->Onscreen() )
			{
				D3DXMatrixRotationY( &YRotation, 0 );
				D3DXMATRIX Orientation;
				D3DXMatrixIdentity( &Orientation );
				Orientation = Rotation * YRotation * Orientation;
				D3DXMATRIX Translation;
				D3DXMatrixTranslation( &Translation, 
									   pCharacter->Data()->Position().x,
									   0,
									   pCharacter->Data()->Position().z );

				float32 Flicker( fabs( sin( m_TorchLightFlicker + pCharacter->Data()->Position().x ) ) * 5 );
				D3DXMATRIX Scaling;
				D3DXMatrixScaling( &Scaling, 
								   15 + Flicker,
								   15 + Flicker,
								   15 + Flicker );


				if( pCharacter->Data()->HasLightPool() )
				{
					m_pD3DDevice->SetTexture( 0, pCharacter->Data()->LightPoolMaterial()->GetTexture() );
				}
				else
				{
					m_pD3DDevice->SetTexture( 0, m_pEnemyTorchLight->GetTexture() );
				}
				m_pTorchLightSprite->RenderNoTexture( m_pD3DDevice, Scaling * Orientation * Translation * ViewportMatrix );
			}
		}
		pCharacter = pCharacter->Next();
	}
	
} // CLevel::RenderCharacterLights()


void CLevel::RenderHardCharacterLights( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
										CFrustum* pFrustum )					// frustum to cull with - NULL for none
{


	D3DXMATRIX Rotation;
	D3DXMatrixRotationX( &Rotation, (float32)KDeg2Rad * 90 );
	D3DXMATRIX YRotation;

		// render all characters
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data()->IsUnique() &&
			pCharacter->Data()->Alive()  )
		{
			if( pCharacter->Data()->Onscreen() )
			{
				D3DXMatrixRotationY( &YRotation, 0 );
				D3DXMATRIX Orientation;
				D3DXMatrixIdentity( &Orientation );
				Orientation = Rotation * YRotation * Orientation;
				D3DXMATRIX Translation;
				D3DXMatrixTranslation( &Translation, 
									   pCharacter->Data()->Position().x,
									   0,
									   pCharacter->Data()->Position().z );

				float32 Flicker( fabs( sin( m_TorchLightFlicker + pCharacter->Data()->Position().x ) ) * 5 );

				D3DXMATRIX Scaling;
				D3DXMatrixScaling( &Scaling, 
								   15 + Flicker,
								   15 + Flicker,
								   15 + Flicker );

				m_pD3DDevice->SetTexture( 0, m_pEnemyTorchHardLight->GetTexture() );
				m_pTorchLightSprite->RenderNoTexture( m_pD3DDevice, Scaling * Orientation * Translation * ViewportMatrix );
			}
		}
		pCharacter = pCharacter->Next();
	}

} // CLevel::RenderHardCharacterLights()


void CLevel::RenderTorchLight( void )
{
	// set up an othographic projection for rendering the main torchlight
	D3DXMATRIX Identity;
	D3DXMATRIX Orthographic;
	D3DXMatrixIdentity( &Identity );
	D3DXMatrixOrthoLH ( &Orthographic, 
						256,
						256,
						1.0f,
						10.0f );

	m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &Orthographic );
	m_pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );
	m_pD3DDevice->SetTransform( D3DTS_VIEW, &Identity );

	float32 Flicker( fabs( sin( m_TorchLightFlicker ) ) * 50 );

	D3DXMATRIX Scaling;
	D3DXMatrixScaling( &Scaling, 
					   400 + Flicker,
					   400 + Flicker,
					   1 );

	D3DXMATRIX	Rotation;
	D3DXMatrixRotationZ( &Rotation, (float32) KDeg2Rad * ( sin( m_TorchLightFlicker ) * 10.0f ) );

	m_pTorchLightSprite->Render( m_pD3DDevice, Scaling * Rotation * m_pTorchLightSprite->Transformation() );

} // CLevel::RenderTorchLight()

void CLevel::RenderShadowFade( void )
{
	if( IsTorchlit() )
	{
		return;
	}
	// set up an othographic projection for rendering the main torchlight
	D3DXMATRIX Identity;
	D3DXMATRIX Orthographic;
	D3DXMatrixIdentity( &Identity );
	D3DXMatrixOrthoLH ( &Orthographic, 
						256,
						256,
						1.0f,
						10.0f );

	m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &Orthographic );
	m_pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );
	m_pD3DDevice->SetTransform( D3DTS_VIEW, &Identity );

	m_pD3DDevice->SetTexture( 0, m_pShadowFade->GetTexture() );

	D3DXMATRIX Scaling;
	D3DXMatrixScaling( &Scaling, 
					   256,
					   256,
					   1 );

	m_pTorchLightSprite->RenderNoTexture( m_pD3DDevice, Scaling * m_pTorchLightSprite->Transformation() );

} // CLevel::RenderShadowFade()

void CLevel::RenderVisibility( const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{
	if( m_WorldRenderState == KWRSRenderAll ||
		m_WorldRenderState != KWRSNoRenderVisibility )
	{
		if( m_pLevelMesh != NULL )
		{
			m_pLevelMesh->RenderVisibility( m_pD3DDevice, ViewportMatrix );
		}
	}
} // CLevel::RenderVisibility()

void CLevel::RenderMiniMap( const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						    CCharacter* pPlayerCharacter )			// player character
{
	if( m_pLevelMesh != NULL )
	{
		m_pLevelMesh->RenderMiniMap( m_pD3DDevice, ViewportMatrix, *this );
	}

	D3DXMATRIX Rotation;
	D3DXMatrixRotationX( &Rotation, (float32)KDeg2Rad * 90 );
	D3DXMATRIX YRotation;

	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data()->Alive() &&
			pCharacter->Data()->Alignment() != KEvil &&
			pCharacter->Data() != pPlayerCharacter &&
			pCharacter->Data()->AIState() != KAIInTown )
		{
			if( Visible( (int32)( pCharacter->Data()->Position().x / VisibilityCellWidth() ), 
						 (int32)( -pCharacter->Data()->Position().z / VisibilityCellWidth() ) ) )
			{
				D3DXMatrixRotationY( &YRotation, (float32)KDeg2Rad * -45 );
				D3DXMATRIX Orientation;
				D3DXMatrixIdentity( &Orientation );
				Orientation = Rotation * YRotation * Orientation;
				D3DXMATRIX Translation;
				D3DXMatrixTranslation( &Translation, 
										pCharacter->Data()->Position().x,
										0,
										pCharacter->Data()->Position().z );
				if( pCharacter->Data()->Alignment() == KNeutral )
				{
					if( pCharacter->Data()->IsQuestGiver() )
					{
						bool IsCompleted( kFalse );
						for( uint32 i = 0; i < pPlayerCharacter->Quests(); i++ )
						{
							if( pPlayerCharacter->Quest( i )->Completed() &&
								pPlayerCharacter->Quest( i )->GiverName() == pCharacter->Data()->Name() )
							{
								IsCompleted = kTrue;
							}
						}
						
						if( IsCompleted )
						{
							m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapQuestGiverDone]->GetTexture() );
						}
						else if( pCharacter->Data()->Quests() > 0 )
						{
							m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapQuestGiver]->GetTexture() );
						}
						else
						{
							m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapNeutral]->GetTexture() );
						}
					}
					else
					{
						m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapNeutral]->GetTexture() );
					}
				}
				else
				{
					m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapCharacter]->GetTexture() );
				}
				m_pMiniMapIconSprite->RenderNoTexture( m_pD3DDevice, Orientation * Translation * ViewportMatrix );
			}
		}
		pCharacter = pCharacter->Next();
	}


	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data()->Type() == KItemTownPortal ||
			pItem->Data()->Type() == KItemDungeonPortal )
		{
			int32 X = (int32)( pItem->Data()->Position().x / VisibilityCellWidth() );
			int32 Y = (int32)( -pItem->Data()->Position().z / VisibilityCellWidth() );
			if( Visible( X, Y ) ||
				Visible( X, Y - 1 ) ||
				Visible( X, Y + 1 ) || 
				Visible( X - 1, Y ) ||
				Visible( X + 1, Y ) )
			{
				D3DXMatrixRotationY( &YRotation, (float32)KDeg2Rad * -45 );
				D3DXMATRIX Orientation;
				D3DXMatrixIdentity( &Orientation );
				Orientation = Rotation * YRotation * Orientation;
				D3DXMATRIX Translation;
				D3DXMatrixTranslation( &Translation, 
										pItem->Data()->Position().x,
										0,
										pItem->Data()->Position().z );
				m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapPortal]->GetTexture() );
				m_pMiniMapIconSprite->RenderNoTexture( m_pD3DDevice, Orientation * Translation * ViewportMatrix );
			}
		}
		else if( pItem->Data()->Type() == KItemStairsUp )
		{
			int32 X = (int32)( pItem->Data()->Position().x / VisibilityCellWidth() );
			int32 Y = (int32)( -pItem->Data()->Position().z / VisibilityCellWidth() );
			if( Visible( X, Y ) ||
				Visible( X, Y - 1 ) ||
				Visible( X, Y + 1 ) || 
				Visible( X - 1, Y ) ||
				Visible( X + 1, Y ) )
			{
				D3DXMatrixRotationY( &YRotation, (float32)KDeg2Rad * -45 );
				D3DXMATRIX Orientation;
				D3DXMatrixIdentity( &Orientation );
				Orientation = Rotation * YRotation * Orientation;
				D3DXMATRIX Translation;
				D3DXMatrixTranslation( &Translation, 
										pItem->Data()->Position().x,
										0,
										pItem->Data()->Position().z );
				m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapAscend]->GetTexture() );
				m_pMiniMapIconSprite->RenderNoTexture( m_pD3DDevice, Orientation * Translation * ViewportMatrix );
			}
		}
		else if( pItem->Data()->Type() == KItemStairsDown )
		{
			if( Visible( (int32)( pItem->Data()->Position().x / VisibilityCellWidth() ), 
						 (int32)( -pItem->Data()->Position().z / VisibilityCellWidth() ) ) )
			{
				D3DXMatrixRotationY( &YRotation, (float32)KDeg2Rad * -45 );
				D3DXMATRIX Orientation;
				D3DXMatrixIdentity( &Orientation );
				Orientation = Rotation * YRotation * Orientation;
				D3DXMATRIX Translation;
				D3DXMatrixTranslation( &Translation, 
										pItem->Data()->Position().x,
										0,
										pItem->Data()->Position().z );
				m_pD3DDevice->SetTexture( 0, m_pMiniMapIconMaterial[KMiniMapDescend]->GetTexture() );
				m_pMiniMapIconSprite->RenderNoTexture( m_pD3DDevice, Orientation * Translation * ViewportMatrix );
			}
		}
		pItem = pItem->Next();
	}


} // CLevel::RenderMiniMap()


void CLevel::RenderVisibilityMask( const D3DXMATRIX& ViewportMatrix )		// viewport matrix
{
	if( m_pLevelMesh != NULL )
	{
		m_pLevelMesh->RenderVisibilityMask( m_pD3DDevice, ViewportMatrix );
	}
} // CLevel::RenderVisibilityMask()

CItem* CLevel::FindDestructibleItemWithinView( const D3DXMATRIX& Transformation,	// position and direction
											   float32 ConeRadius,					// cone of view radius, in degrees
											   float32 Radius )					// valid sight radius
{
	D3DXMATRIX Inverse;
	D3DXVECTOR3 LocalPosition;
	D3DXMatrixInverse( &Inverse, NULL, &Transformation );

	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( pItem->Data()->Destructible() &&
			pItem->Data()->IsContainer() &&
			pItem->Data()->CanInteract() )
		{
			D3DXVec3TransformCoord( &LocalPosition, 
									&pItem->Data()->Position(),
									&Inverse );
			LocalPosition.y = 0;

			float32 Distance = D3DXVec3Length( &LocalPosition );

			float32		Angle	= (float32) KRad2Deg * atan2( LocalPosition.x, 
															  LocalPosition.z );

			if( fabs( Angle ) <= ConeRadius )
			{
				if( Distance <= Radius )
				{
					return pItem->Data();
				}
			}
		}
		pItem = pItem->Next();
	}

	return NULL;
} // CLevel::FindDestructibleItemWithinView()

CCharacter*	CLevel::FindCharacterWithinView( const D3DXMATRIX& Transformation,	// position and direction
											 EAlignment Alignment,				// alignment the character must be
											 float32 MotionRadius,				// valid motion sense radius
											 float32 ConeRadius,				// cone of view radius, in degrees
											 float32 Radius )					// valid sight radius
{
	D3DXMATRIX Inverse;
	D3DXVECTOR3 LocalPosition;
	D3DXMatrixInverse( &Inverse, NULL, &Transformation );
	// render all characters
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data()->Alive() &&
			pCharacter->Data()->InActiveRange() && 
			pCharacter->Data()->Alignment() == Alignment &&
			!pCharacter->Data()->IsPetNearDeath() )
		{
			D3DXVec3TransformCoord( &LocalPosition, 
									&pCharacter->Data()->Position(),
									&Inverse );
			LocalPosition.y = 0;

			float32 Distance = D3DXVec3Length( &LocalPosition );

			bool Visible( kFalse );
			if( Distance <= MotionRadius )
			{
				Visible = kTrue;
			}
			float32		Angle	= (float32) KRad2Deg * atan2( LocalPosition.x, 
															  LocalPosition.z );

			if( fabs( Angle ) <= ConeRadius )
			{
				if( Distance <= Radius )
				{
					Visible = kTrue;
				}
			}

			if( Visible &&
				!( pCharacter->Data()->IsMimic() && pCharacter->Data()->DisplayedAsItem() ) )	// don't find mimics right away
			{
				// try to make sure line-of-sight is valid
				D3DXVECTOR3 Start( Transformation._41, Transformation._42, Transformation._43 );
				D3DXVECTOR3 End( pCharacter->Data()->Position() );
				End.y +=5;
				Start.y += 5;

				if( SightUnobstructed( Start, End ) )
				{
					return pCharacter->Data();
				}
			}

		}
		pCharacter = pCharacter->Next();
	}

	return NULL;
} // CLevel::FindCharacterWithinView()

void CLevel::FindCharactersInArea( const D3DXVECTOR3& Position,						// centerpoint
								   float32 Range,									// radius
								   std::vector< CCharacter* >&	CharacterList )		// list to add characters to
{
	D3DXVECTOR3 LocalPosition;
	// render all characters
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		LocalPosition = pCharacter->Data()->Position() - Position;
		LocalPosition.y = 0;

		float32 Distance = D3DXVec3Length( &LocalPosition );

		if( Distance <= Range )
		{
			CharacterList.push_back( pCharacter->Data() );
		}

		pCharacter = pCharacter->Next();
	}

} // CLevel::FindCharactersInArea()

CCharacter*	CLevel::FindCharacterAtScreenCoordinates( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
													  const D3DXMATRIX& ProjectionMatrix,// projection matrix
													  float32 ScreenX,					// screen coordinates in worldspace
													  float32 ScreenY,					// screen coordinates in worldspace
													  CCharacter* pIgnoreCharacter )	// character to ignore
{
	D3DXMATRIX FinalMatrix;
	D3DXMatrixMultiply( &FinalMatrix, &ViewportMatrix, &ProjectionMatrix );

	D3DXVECTOR3 LocalPosition;

	D3DXVECTOR3 ScreenMinBounds;
	D3DXVECTOR3 ScreenMaxBounds;

	CCharacter* pBestCharacter = NULL;
	float32 BestDistance( 99999 );

	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data()->Alive() &&
			pCharacter->Data()->AvailableForKeyCommand() &&
			pCharacter->Data() != pIgnoreCharacter )
		{

			D3DXVec3TransformCoord( &LocalPosition, 
									&( pCharacter->Data()->DisplayPosition() ),
									&FinalMatrix );
			LocalPosition /= LocalPosition.z;

			if( LocalPosition.x > - 1.1f &&
				LocalPosition.x <= 1.1f &&
				LocalPosition.y >= -1.1f &&
				LocalPosition.y <= 1.1f )
			{
				D3DXVECTOR3 MinBounds( pCharacter->Data()->GetMinBounds() );
				D3DXVECTOR3 MaxBounds( pCharacter->Data()->GetMaxBounds() );
				MinBounds.x *= .5f;
				MaxBounds.x *= .5f;
				MinBounds.z *= .5f;
				MaxBounds.z *= .5f;
				MinBounds.y *= .75f;
				MaxBounds.y *= .75f;
				if( MinBounds.x > -3 )
				{
					MinBounds.x = -3;
				}
				if( MaxBounds.x < 3 )
				{
					MaxBounds.x = 3;
				}
				if( MinBounds.z > -3 )
				{
					MinBounds.z = -3;
				}
				if( MaxBounds.z < 3 )
				{
					MaxBounds.z = 3;
				}
				if( MaxBounds.y < 3 )
				{
					MaxBounds.y = 3;
				}

				if( MaxBounds.y > 4 )
				{
					MaxBounds.y = 4;
				}

				if( pCharacter->Data()->IsMerchant() ||
					pCharacter->Data()->IsQuestGiver() )
				{
					MaxBounds *= .6f;
					MinBounds *= .6f;
				}

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + MinBounds ),
										&FinalMatrix );

			
				ScreenMinBounds = LocalPosition;
				ScreenMaxBounds = LocalPosition;	

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + MaxBounds ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + D3DXVECTOR3( MinBounds.x, MinBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + D3DXVECTOR3( MaxBounds.x, MinBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + D3DXVECTOR3( MaxBounds.x, MinBounds.y, MinBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + D3DXVECTOR3( MinBounds.x, MaxBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + D3DXVECTOR3( MaxBounds.x, MaxBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pCharacter->Data()->DisplayPosition() + D3DXVECTOR3( MaxBounds.x, MaxBounds.y, MinBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				if( ScreenMaxBounds.z > 0 )
				{
					ScreenMinBounds.x /= ScreenMinBounds.z;
					ScreenMinBounds.y /= ScreenMinBounds.z;
					ScreenMaxBounds.x /= ScreenMinBounds.z;
					ScreenMaxBounds.y /= ScreenMinBounds.z;
					float32 Distance = ScreenMinBounds.z;
					// if the character is a pet, bias it to the back of all
					// other characters
					if( pCharacter->Data()->HasMaster() &&
						pCharacter->Data()->Master() == pIgnoreCharacter )

					{
						Distance += 1000;
					}
					if( pCharacter->Data()->Alignment() == KNeutral )
					{
						Distance += 800;
					}
					if( Distance < BestDistance )
					{
						if( ScreenX >= ScreenMinBounds.x &&
							ScreenY >= ScreenMinBounds.y &&
							ScreenX <= ScreenMaxBounds.x &&
							ScreenY <= ScreenMaxBounds.y )
						{
							BestDistance = Distance;
							pBestCharacter = pCharacter->Data();
						}
					}
				}
			}
		}
		pCharacter = pCharacter->Next();
	}

	return pBestCharacter;
} // CLevel::FindCharacterAtScreenCoordinates()

CItem*	CLevel::FindItemAtScreenCoordinates( const D3DXMATRIX& ViewportMatrix,	// viewport matrix
											 const D3DXMATRIX& ProjectionMatrix,// projection matrix
											 float32 ScreenX,					// screen coordinates in worldspace
											 float32 ScreenY )					// screen coordinates in worldspace
{
	D3DXMATRIX FinalMatrix;
	D3DXMatrixMultiply( &FinalMatrix, &ViewportMatrix, &ProjectionMatrix );

	D3DXVECTOR3 LocalPosition;

	D3DXVECTOR3 ScreenMinBounds;
	D3DXVECTOR3 ScreenMaxBounds;

	CItem* pBestItem = NULL;
	float32 BestDistance( 99999 );

	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( !( pItem->Data()->IsContainer() && !pItem->Data()->CanInteract() ) &&
			!( pItem->Data()->Category() == KCategoryEvent && !pItem->Data()->Operateable() ) &&
			!pItem->Data()->Dropping() )
		{
			D3DXVec3TransformCoord( &LocalPosition, 
									&( pItem->Data()->Position() ),
									&FinalMatrix );
			LocalPosition /= LocalPosition.z;
			if( LocalPosition.x > - 1.1f &&
				LocalPosition.x <= 1.1f &&
				LocalPosition.y >= -1.1f &&
				LocalPosition.y <= 1.1f )
			{
				D3DXVECTOR3 MinBounds( pItem->Data()->GetMinBounds() );
				D3DXVECTOR3 MaxBounds( pItem->Data()->GetMaxBounds() );

				if( pItem->Data()->Category() != KCategoryBlank )
				{
					if( MinBounds.x < -3 )
					{
						MinBounds.x = -3;
					}
					if( MaxBounds.x > 3 )
					{
						MaxBounds.x = 3;
					}
					if( MinBounds.z < -3 )
					{
						MinBounds.z = -3;
					}
					if( MaxBounds.z > 3 )
					{
						MaxBounds.z = 3;
					}
					if( MaxBounds.y > 5 )
					{
						MaxBounds.y = 5;
					}
				}

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + MinBounds ),
										&FinalMatrix );


			
				ScreenMinBounds = LocalPosition;
				ScreenMaxBounds = LocalPosition;	

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + MaxBounds ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + D3DXVECTOR3( MinBounds.x, MinBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + D3DXVECTOR3( MaxBounds.x, MinBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + D3DXVECTOR3( MaxBounds.x, MinBounds.y, MinBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + D3DXVECTOR3( MinBounds.x, MaxBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + D3DXVECTOR3( MaxBounds.x, MaxBounds.y, MaxBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				D3DXVec3TransformCoord( &LocalPosition, 
										&( pItem->Data()->Position() + D3DXVECTOR3( MaxBounds.x, MaxBounds.y, MinBounds.z ) ),
										&FinalMatrix );
				
				ExpandBounds( ScreenMinBounds, ScreenMaxBounds, LocalPosition );

				if( ScreenMaxBounds.z > 0 )
				{
					ScreenMinBounds.x /= ScreenMinBounds.z;
					ScreenMinBounds.y /= ScreenMinBounds.z;
					ScreenMaxBounds.x /= ScreenMinBounds.z;
					ScreenMaxBounds.y /= ScreenMinBounds.z;

					float32 Distance = ScreenMinBounds.z;
					// events are biased to the far back
					if( pItem->Data()->Category() == KCategoryBlank ||
						( pItem->Data()->Category() == KCategoryEvent ) )
					{
						Distance += 1000;
					}
					if( Distance < BestDistance )
					{

						if( ScreenX >= ScreenMinBounds.x &&
							ScreenY >= ScreenMinBounds.y &&
							ScreenX <= ScreenMaxBounds.x &&
							ScreenY <= ScreenMaxBounds.y )
						{
							BestDistance = Distance;
							pBestItem = pItem->Data();
						}
					}
				}
			}
		}
		pItem = pItem->Next();
	}

	return pBestItem;
} // CLevel::FindItemAtScreenCoordinates()

void CLevel::FindItemsOnscreen( const D3DXMATRIX& ViewportMatrix,	 // viewport matrix
								const D3DXMATRIX& ProjectionMatrix ) // projection matrix
{
	m_pOnscreenItems->Clear();
	D3DXMATRIX FinalMatrix;
	D3DXMatrixMultiply( &FinalMatrix, &ViewportMatrix, &ProjectionMatrix );

	D3DXVECTOR3 LocalPosition;

	D3DXVECTOR3 ScreenMinBounds;
	D3DXVECTOR3 ScreenMaxBounds;


	CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
	while ( pItem != NULL )
	{
		if( ( pItem->Data()->Takeable() ||
			  ( pItem->Data()->IsContainer() && pItem->Data()->CanInteract() ) ) &&
			!pItem->Data()->Dropping() )
		{
			D3DXVec3TransformCoord( &LocalPosition, 
									&( pItem->Data()->Position() ),
									&FinalMatrix );
			LocalPosition /= LocalPosition.z;
			if( LocalPosition.x > - 1.0f &&
				LocalPosition.x <= 1.0f &&
				LocalPosition.y >= -1.0f &&
				LocalPosition.y <= 1.0f &&
				LocalPosition.z >= 0 )
			{
				m_pOnscreenItems->Queue( pItem->Data() );
			}
		}
		pItem = pItem->Next();
	}
} // CLevel::FindItemsOnscreen()

void CLevel::FindCharactersOnscreen( const D3DXMATRIX& ViewportMatrix,	 // viewport matrix
									 const D3DXMATRIX& ProjectionMatrix ) // projection matrix
{
	m_pOnscreenCharacters->Clear();
	D3DXMATRIX FinalMatrix;
	D3DXMatrixMultiply( &FinalMatrix, &ViewportMatrix, &ProjectionMatrix );

	D3DXVECTOR3 LocalPosition;

	D3DXVECTOR3 ScreenMinBounds;
	D3DXVECTOR3 ScreenMaxBounds;


	// go through our linked list of Characters, and reload data each one
	CLLNode<CCharacter>*	pCharacter	= m_pCharacterList->GetFirst();
	while ( pCharacter != NULL )
	{
		if( pCharacter->Data()->Alive() )
		{
			D3DXVec3TransformCoord( &LocalPosition, 
									&( pCharacter->Data()->Position() ),
									&FinalMatrix );
			LocalPosition /= LocalPosition.z;
			if( LocalPosition.x > - 1.0f &&
				LocalPosition.x <= 1.0f &&
				LocalPosition.y >= -1.0f &&
				LocalPosition.y <= 1.0f &&
				LocalPosition.z >= 0 )
			{
				m_pOnscreenCharacters->Queue( pCharacter->Data() );
			}
		}
		pCharacter = pCharacter->Next();
	}
} // CLevel::FindCharactersOnscreen()

void CLevel::CalculatePassability( void )
{
	ClearPassabilityData();

	D3DXVECTOR3 MinBounds;
	D3DXVECTOR3 MaxBounds;

	D3DXVECTOR3 ImpactPoint;
	D3DXVECTOR3 ImpactNormal;
	D3DXVECTOR3 Start;
	D3DXVECTOR3 End;
	D3DXVECTOR3 Final;
	D3DXVECTOR3 Color;
	uint32		Material;

	float32 HalfCellWidth = m_CellWidth * .5f;

	float32 MinThreshhold( -5 );
	float32 MaxThreshhold( 4 );

	float32 TopLeftX( 0 );
	float32 TopLeftZ( 0 );
	if( m_pLevelCollisionScene != NULL )
	{
		MinThreshhold = -12;
		MaxThreshhold = 5;
		TopLeftX = m_pLevelCollisionScene->GetMinBounds().x + HalfCellWidth;
		TopLeftZ = m_pLevelCollisionScene->GetMaxBounds().z - HalfCellWidth;
	}



	for( uint32 i = 0; i < m_PathingCellsWide; i++ )
	{
		MinBounds = D3DXVECTOR3( TopLeftX + i * m_CellWidth - HalfCellWidth - 1, -100, TopLeftZ - HalfCellWidth * 2 - m_PathingCellsHigh * m_CellWidth - 1 );	
		MaxBounds = D3DXVECTOR3( TopLeftX + i * m_CellWidth + HalfCellWidth * 2 + 1, 100, TopLeftZ + HalfCellWidth + 1 );	
		SortForCollision( MinBounds, MaxBounds );
		for( uint32 j = 0; j < m_PathingCellsHigh; j++ )
		{
			End.x = (float32)( i * m_CellWidth ) + HalfCellWidth + TopLeftX;
			End.z = (float32)( j * -m_CellWidth ) - HalfCellWidth + TopLeftZ;
			End.y = -50;

			Start.x = (float32)( i * m_CellWidth ) + HalfCellWidth + TopLeftX;
			Start.z = (float32)( j * -m_CellWidth ) - HalfCellWidth + TopLeftZ;
			Start.y = 50;

			if( PreSortedRayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color, NULL, kFalse ) )
			{
				if( ( ImpactPoint.y > MaxThreshhold || 
					  ImpactPoint.y < MinThreshhold ||
					  Material == KImpassableMaterialID )
					  && Material != KFloorMaterialID )
				{
					m_MapPassabilityData[i][j] = 1;
				}
			}
			else
			{
				if( m_pLevelCollisionScene != NULL )
				{
					m_MapPassabilityData[i][j] = 1;
				}
				else
				{
					if( PositionCellHeight( Start ) != 0 )
					{
						m_MapPassabilityData[i][j] = 1;
					}
				}
			}
		}
	}
} // CLevel::CalculatePassability()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevel::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
					   	   const D3DXVECTOR3& End,		// end of collision ray
						   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
						   D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	D3DXVECTOR3 Color;
	uint32 Material;
	return RayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color, NULL );
} // CLevel::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevel::SphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
							  const D3DXVECTOR3& End,		// end of collision ray
							  float32 Radius,				// radius of sphere to check along collision ray
							  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							  D3DXVECTOR3& ImpactPoint,	// impact point to be filled
							  D3DXVECTOR3& ImpactNormal )	// impact normal to be filled
{
	D3DXVECTOR3 Color;
	uint32 Material;
	return SphereCollision( Start, 
							End, 
							Radius, 
							FinalPosition, 
							ImpactPoint, 
							ImpactNormal, 
							Material, 
							Color,
							NULL );

} // CLevel::SphereCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevel::PreSortedRayCollision( const D3DXVECTOR3& Start,	// start of collision ray
									const D3DXVECTOR3& End,		// end of collision ray
									D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								    uint32& FaceMaterial,		// material id of the impacted face
									D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
									CCharacter** pCharacter,	// character hit?
									bool AllowObjectCollision)	// collide with objects?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;

	if( AllowObjectCollision &&
		ObjectRayCollision( Start,
							 End,
	 						 BestImpactPoint,
							 BestImpactNormal,
							 FaceMaterial,
							 pCharacter ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );

		BestDistance = Distance;
		ImpactPoint = BestImpactPoint;
		ImpactNormal = BestImpactNormal;
		AverageColor = D3DXVECTOR3( 1, 1, 1 );
	}

	if( m_pLevelCollision->PreSortedRayCollision( Start,
								    End,
									BestImpactPoint,
									BestImpactNormal,
									FaceMaterial,
									AverageColor ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
		if( Distance < BestDistance )
		{
			BestDistance = Distance;
			ImpactPoint = BestImpactPoint;
			ImpactNormal = BestImpactNormal;
		}
	}
	return( BestDistance != KCollisionInvalid );
} // CLevel::PreSortedRayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevel::PreSortedSphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
									   const D3DXVECTOR3& End,		// end of collision ray
									   float32 Radius,				// radius of sphere to check along collision ray
									   D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									   uint32& FaceMaterial,		// material id of the impacted face
									   D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
									   CCharacter** pCharacter,		// character hit?
									   bool AllowObjectCollision	)	// collide with objects?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestFinalPosition;
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;

	if( AllowObjectCollision &&
		ObjectSphereCollision( Start,
								End,
								Radius,
								BestFinalPosition,
								BestImpactPoint,
								BestImpactNormal,
								FaceMaterial,
								pCharacter ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );

		BestDistance = Distance;
		FinalPosition = BestFinalPosition;
		ImpactPoint = BestImpactPoint;
		ImpactNormal = BestImpactNormal;
		AverageColor = D3DXVECTOR3( 1, 1, 1 );
	}

	if( m_pLevelCollision->PreSortedSphereCollision( Start,
									   End,
									   Radius,
									   BestFinalPosition,
									   BestImpactPoint,
									   BestImpactNormal,
									   FaceMaterial,
									   AverageColor ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
		if( Distance < BestDistance )
		{
			BestDistance = Distance;
			FinalPosition = BestFinalPosition;
			ImpactPoint = BestImpactPoint;
			ImpactNormal = BestImpactNormal;
		}
	}
	return( BestDistance != KCollisionInvalid );
} // CLevel::PreSortedSphereCollision()

// check for collision along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevel::RayCollision( const D3DXVECTOR3& Start,	// start of collision ray
						   const D3DXVECTOR3& End,		// end of collision ray
						   D3DXVECTOR3& ImpactPoint,	// impact point to be filled
						   D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
						   uint32& FaceMaterial,			// material id of the impacted face
						   D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
						   CCharacter** pCharacter,		// character hit?
						   bool AllowObjectCollision	)	// collide with objects?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;

	if( AllowObjectCollision &&
		ObjectRayCollision( Start,
							End,
	 						BestImpactPoint,
							BestImpactNormal,
							FaceMaterial,
							pCharacter ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );

		BestDistance = Distance;
		ImpactPoint = BestImpactPoint;
		ImpactNormal = BestImpactNormal;
		AverageColor = D3DXVECTOR3( 1, 1, 1 );
	}

	if( m_pLevelCollision->RayCollision( Start,
						   End,
						   BestImpactPoint,
						   BestImpactNormal,
						   FaceMaterial,
						   AverageColor ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
		if( Distance < BestDistance )
		{
			BestDistance = Distance;
			ImpactPoint = BestImpactPoint;
			ImpactNormal = BestImpactNormal;
		}
	}
	return( BestDistance != KCollisionInvalid );
} // CLevel::RayCollision()

// check the collision of a sphere along the given ray
// return kTrue if collision occurs and fill the ImpactPoint and ImpactNormal, kFalse if not
bool CLevel::SphereCollision( const D3DXVECTOR3& Start,		// start of collision ray
							  const D3DXVECTOR3& End,		// end of collision ray
							  float32 Radius,				// radius of sphere to check along collision ray
							  D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
							  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
							  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
							  uint32& FaceMaterial,			// material id of the impacted face
							  D3DXVECTOR3& AverageColor,	// averaged colors of the face's verts
							  CCharacter** pCharacter,		// character hit?
							  bool AllowObjectCollision	)	// collide with objects?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestFinalPosition;
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;
	if( AllowObjectCollision &&
		ObjectSphereCollision( Start,
								End,
								Radius,
								BestFinalPosition,
								BestImpactPoint,
								BestImpactNormal,
								FaceMaterial,
								pCharacter ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );

		BestDistance = Distance;
		FinalPosition = BestFinalPosition;
		ImpactPoint = BestImpactPoint;
		ImpactNormal = BestImpactNormal;
		AverageColor = D3DXVECTOR3( 1, 1, 1 );
	}

	if( m_pLevelCollision->SphereCollision( Start,
							  End,
							  Radius,
							  BestFinalPosition,
							  BestImpactPoint,
							  BestImpactNormal,
							  FaceMaterial,
							  AverageColor ) )
	{
		float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
		if( Distance < BestDistance )
		{
			BestDistance = Distance;
			FinalPosition = BestFinalPosition;
			ImpactPoint = BestImpactPoint;
			ImpactNormal = BestImpactNormal;
		}
	}
	return( BestDistance != KCollisionInvalid );
} // CLevel::SphereCollision()


// presorts faces for collision later.
// this is excellent to use if you are going to perform many collisions in a set area
// (for instance, colliding with four points on the ground to place a shadow )
// call SortForCollision, and then either PreSortedRayCollision or PreSortedSphereCollision
// to collide with this data.
void CLevel::SortForCollision( const D3DXVECTOR3& MinBounds,	// min bounds of area to sort
							  const D3DXVECTOR3& MaxBounds )	// max bounds of area to sort
{
	m_pLevelCollision->SortForCollision( MinBounds, MaxBounds );

} // CLevel::SortForCollision()

bool CLevel::ObjectSphereCollision( const D3DXVECTOR3& Start,	// start of collision ray
									const D3DXVECTOR3& End,		// end of collision ray
									float32 Radius,				// radius of sphere to check along collision ray
									D3DXVECTOR3& FinalPosition,	// final position of sphere after impact to be filled
									D3DXVECTOR3& ImpactPoint,	// impact point to be filled
									D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
									uint32& FaceMaterial,		// material id of the impacted face
									CCharacter** pCharacter )	// character hit?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestFinalPosition;
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;

	// go through our linked list of Characters, and reload data each one
	CLLNode<CCharacter>*	Character	= m_pCharacterList->GetFirst();
	while ( Character != NULL )
	{
		if( Character->Data()->Collideable() &&
			Character->Data()->SphereCollision( Start,
												End,
												Radius,
												BestFinalPosition,
												BestImpactPoint,
												BestImpactNormal ) )
		{
			float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
			if( Distance < BestDistance )
			{
				if( pCharacter != NULL )
				{
					*pCharacter = Character->Data();
				}
				FaceMaterial = KCharacterMaterialID;

				BestDistance = Distance;
				FinalPosition = BestFinalPosition;
				ImpactPoint = BestImpactPoint;
				ImpactNormal = BestImpactNormal;
			}
		}

		Character = Character->Next();
	}

	// go through our linked list of Items, and reload data each one
	CLLNode<CItem>*	Item	= m_pItemList->GetFirst();
	while ( Item != NULL )
	{
		if( Item->Data()->Collideable() &&
			Item->Data()->SphereCollision( Start,
										   End,
										 	Radius,
												BestFinalPosition,
												BestImpactPoint,
												BestImpactNormal ) )
		{
			float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
			if( Distance < BestDistance )
			{
				FaceMaterial = KImpassableMaterialID;

				BestDistance = Distance;
				FinalPosition = BestFinalPosition;
				ImpactPoint = BestImpactPoint;
				ImpactNormal = BestImpactNormal;
			}
		}

		Item = Item->Next();
	}
	return( BestDistance != KCollisionInvalid );
} // CLevel::ObjectSphereCollision()

bool CLevel::ObjectRayCollision( const D3DXVECTOR3& Start,		// start of collision ray
								  const D3DXVECTOR3& End,		// end of collision ray
								  D3DXVECTOR3& ImpactPoint,		// impact point to be filled
								  D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								  uint32& FaceMaterial,		// material id of the impacted face
								  CCharacter** pCharacter )		// character hit?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;

	// go through our linked list of Characters, and reload data each one
	CLLNode<CCharacter>*	Character	= m_pCharacterList->GetFirst();
	while ( Character != NULL )
	{
		if( Character->Data()->Collideable() &&
		    Character->Data()->RayCollision( Start,
										   End,
										   BestImpactPoint,
										   BestImpactNormal ) )
		{
			float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
			if( Distance < BestDistance )
			{
				if( pCharacter != NULL )
				{
					*pCharacter = Character->Data();
				}

				FaceMaterial = KCharacterMaterialID;

				BestDistance = Distance;
				ImpactPoint = BestImpactPoint;
				ImpactNormal = BestImpactNormal;
			}
		}

		Character = Character->Next();
	}

	// go through our linked list of Items
	CLLNode<CItem>*	Item	= m_pItemList->GetFirst();
	while ( Item != NULL )
	{
		if( Item->Data()->Collideable() &&
		    Item->Data()->RayCollision( Start,
										End,
										BestImpactPoint,
										BestImpactNormal ) )
		{
			float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
			if( Distance < BestDistance )
			{
				FaceMaterial = KImpassableMaterialID;
				BestDistance = Distance;
				ImpactPoint = BestImpactPoint;
				ImpactNormal = BestImpactNormal;
			}
		}

		Item = Item->Next();
	}

	return( BestDistance != KCollisionInvalid );
} // CLevel::ObjectRayCollision()

bool CLevel::CharacterRayCollision( const D3DXVECTOR3& Start,		// start of collision ray
								    const D3DXVECTOR3& End,		// end of collision ray
								    D3DXVECTOR3& ImpactPoint,		// impact point to be filled
								    D3DXVECTOR3& ImpactNormal,	// impact normal to be filled
								    uint32& FaceMaterial,		// material id of the impacted face
								    CCharacter** pCharacter )		// character hit?
{
	float32 BestDistance( KCollisionInvalid );
	D3DXVECTOR3 BestImpactPoint;
	D3DXVECTOR3 BestImpactNormal;

	// go through our linked list of Characters, and reload data each one
	CLLNode<CCharacter>*	Character	= m_pCharacterList->GetFirst();
	while ( Character != NULL )
	{
		if( Character->Data()->Collideable() &&
		    Character->Data()->RayCollision( Start,
										   End,
										   BestImpactPoint,
										   BestImpactNormal ) )
		{
			float32 Distance = D3DXVec3Length( &( BestImpactPoint - Start ) );
			if( Distance < BestDistance )
			{
				if( pCharacter != NULL )
				{
					*pCharacter = Character->Data();
				}

				FaceMaterial = KCharacterMaterialID;

				BestDistance = Distance;
				ImpactPoint = BestImpactPoint;
				ImpactNormal = BestImpactNormal;
			}
		}

		Character = Character->Next();
	}


	return( BestDistance != KCollisionInvalid );
} // CLevel::ObjectRayCollision()

float32 CLevel::TopLeftX( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		return m_pLevelCollisionScene->GetMinBounds().x;
	}
	else
	{
		return 0;
	}
} // CLevel::TopLeftX()

float32 CLevel::TopLeftY( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		return m_pLevelCollisionScene->GetMaxBounds().z;
	}
	else
	{
		return 0;
	}
} // CLevel::TopLeftY()

float32 CLevel::WaterHeight( const D3DXVECTOR3& Position ) // position to check at
{
	if( m_pLevelScene != NULL )
	{
		// this is the water-level in the town
		return -5.5f;
	}
	else
	{
		float32 Height = Template()->WaterDepth();
		Height += sin( m_pLevelMesh->WaterOffset() + Position.x * 1.5f );
		Height += cos( m_pLevelMesh->WaterOffset() + Position.z * 1.5f );
		return Height;
	}
} // CLevel::WaterHeight()

float32 CLevel::FloorHeight( const D3DXVECTOR3& Position ) // position to check at
{
	return FloorHeight( Position.x, Position.z );
} // CLevel::FloorHeight()

float32 CLevel::FloorHeight( float32 WorldspaceX,		// x worldspace position
							 float32 WorldspaceY )		// y worldspace position
{		

	if( m_pLevelCollisionScene != NULL )
	{
		D3DXVECTOR3 ImpactPoint;
		D3DXVECTOR3 ImpactNormal;
		D3DXVECTOR3 Start;
		D3DXVECTOR3 End;
		D3DXVECTOR3 Color;
		uint32		Material;

		End.x = WorldspaceX;
		End.z = WorldspaceY;
		End.y = -100;

		Start.x = WorldspaceX;
		Start.z = WorldspaceY;
		Start.y = 100;

		if( RayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color, NULL, kFalse ) )
		{
			return ImpactPoint.y;
		}
		return 0;
	}

	float32 OffsetX( WorldspaceX / m_TileWidth );
	float32 OffsetY( -WorldspaceY / m_TileWidth );

	uint32 X = (uint32)floor( OffsetX );
	uint32 Y = (uint32)floor( OffsetY );

	OffsetX -= X;
	OffsetY -= Y;

	if( OffsetX < .1f )
	{
		OffsetX = 0;
	}
	else if( OffsetX > m_TileWidth - .1f )
	{
		OffsetX = 1;
	}
	if( OffsetY < .1f )
	{
		OffsetY = 0;
	}



	// Compute height at nearest grid points, and bilinear blend
	// the results.
	float32	Point1 = m_pMaze->FloorHeight( X, Y );
	float32	Point2 = m_pMaze->FloorHeight( X + 1 , Y );
	float32	Point3 = m_pMaze->FloorHeight( X, Y + 1 );
	float32	Point4 = m_pMaze->FloorHeight( X + 1, Y + 1 );
		
	return ( Point1 * ( 1 - OffsetX ) * ( 1 - OffsetY )
			+ Point2 * ( OffsetX ) * ( 1 - OffsetY )
			+ Point3 * ( 1 - OffsetX ) * ( OffsetY )
			+ Point4 * ( OffsetX ) * ( OffsetY ) );
} // CLevel::FloorHeight()

float32 CLevel::FloorOffsetX( const D3DXVECTOR3& Position ) // position to check at
{
	return FloorOffsetX( Position.x, Position.z );
} // CLevel::XOffset()

float32 CLevel::FloorOffsetX( float32 WorldspaceX,		// x worldspace position
							  float32 WorldspaceY )		// y worldspace position
{		

	float32 OffsetX( WorldspaceX / m_TileWidth );
	float32 OffsetY( -WorldspaceY / m_TileWidth );

	uint32 X = (uint32)floor( OffsetX );
	uint32 Y = (uint32)floor( OffsetY );

	OffsetX -= X;
	OffsetY -= Y;

	if( OffsetX < .1f )
	{
		OffsetX = 0;
	}
	else if( OffsetX > m_TileWidth - .1f )
	{
		OffsetX = 1;
	}
	if( OffsetY < .1f )
	{
		OffsetY = 0;
	}

	// Compute heat at nearest grid points, and bilinear blend
	// the results.
	float32	Point1 = m_pMaze->XOffset( X, Y );
	float32	Point2 = m_pMaze->XOffset( X + 1 , Y );
	float32	Point3 = m_pMaze->XOffset( X, Y + 1 );
	float32	Point4 = m_pMaze->XOffset( X + 1, Y + 1 );
		
	return ( Point1 * ( 1 - OffsetX ) * ( 1 - OffsetY )
			+ Point2 * ( OffsetX ) * ( 1 - OffsetY )
			+ Point3 * ( 1 - OffsetX ) * ( OffsetY )
			+ Point4 * ( OffsetX ) * ( OffsetY ) );

} // CLevel::XOffset()

float32 CLevel::FloorOffsetY( const D3DXVECTOR3& Position ) // position to check at
{
	return FloorOffsetY( Position.x, Position.z );
} // CLevel::YOffset()

float32 CLevel::FloorOffsetY( float32 WorldspaceX,		// x worldspace position
							  float32 WorldspaceY )		// y worldspace position
{		
	float32 OffsetX( WorldspaceX / m_TileWidth );
	if( OffsetX < .1f )
	{
		OffsetX = 0;
	}
	else if( OffsetX > m_TileWidth - .1f )
	{
		OffsetX = 1;
	}
	float32 OffsetY( -WorldspaceY / m_TileWidth );
	if( OffsetY < .1f )
	{
		OffsetY = 0;
	}

	uint32 X = (uint32)floor( OffsetX );
	uint32 Y = (uint32)floor( OffsetY );

	OffsetX -= X;
	OffsetY -= Y;

	// Compute heat at nearest grid points, and bilinear blend
	// the results.
	float32	Point1 = m_pMaze->YOffset( X, Y );
	float32	Point2 = m_pMaze->YOffset( X + 1 , Y );
	float32	Point3 = m_pMaze->YOffset( X, Y + 1 );
	float32	Point4 = m_pMaze->YOffset( X + 1, Y + 1 );
		
	return ( Point1 * ( 1 - OffsetX ) * ( 1 - OffsetY )
			+ Point2 * ( OffsetX ) * ( 1 - OffsetY )
			+ Point3 * ( 1 - OffsetX ) * ( OffsetY )
			+ Point4 * ( OffsetX ) * ( OffsetY ) );

} // CLevel::YOffset()

D3DXVECTOR3 CLevel::FloorNormal( const D3DXVECTOR3& Position ) // position to check at
{		

     float32 DeltaX = FloorHeight( Position.x + m_TileWidth, Position.z ) - 
				  FloorHeight( Position.x - m_TileWidth, Position.z );

     float32 DeltaY = FloorHeight( Position.x, Position.z + m_TileWidth ) - 
				  FloorHeight( Position.x, Position.z - m_TileWidth );

	 D3DXVECTOR3 Normal( -DeltaX, 2.0f, DeltaY );
	 // we reduce the y component of the y to accentuate perpendicular normals,
	 // for more interesting lighting on the floors
	// Normal.y *= .75f;
	 D3DXVec3Normalize( &Normal, &Normal );

	 return Normal;
} // CLevel::FloorNormal()

D3DXVECTOR3 CLevel::PositionFromCellPosition( uint32 X,		// x cell location
											  uint32 Y )	// y cell location
{
	D3DXVECTOR3 Position( (float32)X * m_TileWidth + m_TileWidth * .5f, 
						  0, 
						  (float32)Y * -m_TileWidth - m_TileWidth * .5f );

	return Position;
} // CLevel::PositionFromCellPosition()

D3DXVECTOR3 CLevel::PositionNearStairsUp( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		D3DXVECTOR3 Position = m_pLevelCollisionScene->GetMaxBounds() - m_pLevelCollisionScene->GetMinBounds();
		Position *= .5f;
		Position += m_pLevelCollisionScene->GetMinBounds();
		Position.y = 0;
		return Position;
	}
	else
	{
		for( uint32 x = 0; x < m_pMaze->Width(); x++ )
		{
			for( uint32 y = 0; y < m_pMaze->Height(); y++ )
			{
				if( m_pMaze->CellType( x, y ) == KCTypeStairwellUp )
				{
					return PositionFromCellPosition( x, y );
				}
			}
		}
	}
	return RandomOpenPosition();

} // CLevel::PositionNearStairsUp()

D3DXVECTOR3 CLevel::PositionNearStairsDown( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		CLLNode<CItem>*	pItem	= m_pItemList->GetFirst();
		while ( pItem != NULL )
		{
			if( StringUpper( pItem->Data()->Name() ) == "DESCEND INTO DUNGEON" )
			{
				return RandomOpenPosition( pItem->Data()->Position(), 10 );
			}
			pItem = pItem->Next();
		}

		D3DXVECTOR3 Position = m_pLevelCollisionScene->GetMaxBounds() - m_pLevelCollisionScene->GetMinBounds();
		Position *= .5f;
		Position += m_pLevelCollisionScene->GetMinBounds();
		Position.y = 0;
		return Position;
	}
	else
	{
		for( uint32 x = 0; x < m_pMaze->Width(); x++ )
		{
			for( uint32 y = 0; y < m_pMaze->Height(); y++ )
			{
				if( m_pMaze->CellType( x, y ) == KCTypeStairwellDown )
				{

					if( m_pMaze->CellOpenIgnoreDoors( x - 1, y ) )
					{
						return PositionFromCellPosition( x - 1, y );
					}
					if( m_pMaze->CellOpenIgnoreDoors( x + 1, y ) )
					{
						return PositionFromCellPosition( x + 1, y );
					}
					if( m_pMaze->CellOpenIgnoreDoors( x, y - 1 ) )
					{
						return PositionFromCellPosition( x, y - 1 );
					}
					if( m_pMaze->CellOpenIgnoreDoors( x, y + 1 ) )
					{
						return PositionFromCellPosition( x, y + 1 );
					}
				}
			}
		}
	}


	return RandomOpenPosition();

} // CLevel::PositionNearStairsDown()

D3DXVECTOR3 CLevel::RandomOpenPositionNearWall( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		bool Done( kFalse );
		while( !Done )
		{
			D3DXVECTOR3 Position( RandomBetween( m_pLevelCollisionScene->GetMinBounds().x, m_pLevelCollisionScene->GetMaxBounds().x ),
								0,
								RandomBetween( m_pLevelCollisionScene->GetMinBounds().z, m_pLevelCollisionScene->GetMaxBounds().z ) );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) )
			{
				return Position;
			}

		}
	}
	else
	{
		bool Done( kFalse );
		while( !Done )
		{
			D3DXVECTOR3 Position( RandomBetween( 0, m_pMaze->Width() * m_TileWidth ),
								0,
								RandomBetween( 0, m_pMaze->Height() * -m_TileWidth ) );

			ECellType Type = PositionType( Position );

			if( Type == KCTypeRoom ||
				Type == KCTypeHall )
			{
				if( Type == KCTypeHall ||
					PositionType( Position + D3DXVECTOR3( 0, 0, m_TileWidth ) ) == KCTypeRoomWall  ||
					PositionType( Position + D3DXVECTOR3( 0, 0, -m_TileWidth ) ) == KCTypeRoomWall  ||
					PositionType( Position + D3DXVECTOR3( -m_TileWidth, 0, 0 ) ) == KCTypeRoomWall  ||
					PositionType( Position + D3DXVECTOR3( m_TileWidth, 0, 0 ) ) == KCTypeRoomWall  )
				{
					if( PositionPassable( Position ) &&
						PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
						PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
						PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
						PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
						Type != KCTypeNSDoor && 
						Type != KCTypeEWDoor )
					{
						return Position;
					}
				}
			}
		}
	}
	return RandomOpenPosition();

} // CLevel::RandomOpenPositionNearWall()

D3DXVECTOR3 CLevel::RandomOpenPositionInRoom( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		bool Done( kFalse );
		while( !Done )
		{
			D3DXVECTOR3 Position( RandomBetween( m_pLevelCollisionScene->GetMinBounds().x, m_pLevelCollisionScene->GetMaxBounds().x ),
								0,
								RandomBetween( m_pLevelCollisionScene->GetMinBounds().z, m_pLevelCollisionScene->GetMaxBounds().z ) );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) )
			{
				return Position;
			}

		}
	}
	else
	{
		uint32 Tries( 0 );
		bool Done( kFalse );
		while( !Done &&
			   Tries < 10000 )
		{
			D3DXVECTOR3 Position( RandomBetween( 0, m_pMaze->Width() * m_TileWidth ),
								0,
								RandomBetween( 0, m_pMaze->Height() * -m_TileWidth ) );

			ECellType Type = PositionType( Position );

			int32 X = (uint32)floor( Position.x / m_TileWidth );
			int32 Y = (uint32)floor( -Position.z / m_TileWidth );
			if( !m_pMaze->HasCollideableProp( X, Y ) )
			{

				if( Type == KCTypeRoom )
				{
					if( PositionType( Position + D3DXVECTOR3( 0, 0, m_TileWidth ) ) == KCTypeRoom  &&
						PositionType( Position + D3DXVECTOR3( 0, 0, -m_TileWidth ) ) == KCTypeRoom  &&
						PositionType( Position + D3DXVECTOR3( -m_TileWidth, 0, 0 ) ) == KCTypeRoom  &&
						PositionType( Position + D3DXVECTOR3( m_TileWidth, 0, 0 ) ) == KCTypeRoom  )
					{
						if( PositionPassable( Position ) &&
							PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
							PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
							PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
							PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) )  )
						{
							return Position;
						}
					}
				}
			}
			Tries++;
		}
	}
	return RandomOpenPosition();

} // CLevel::RandomOpenPositionInRoom()

D3DXVECTOR3 CLevel::RandomOpenPosition( void )
{
	if( m_pLevelCollisionScene != NULL )
	{
		bool Done( kFalse );
		while( !Done )
		{
			D3DXVECTOR3 Position( RandomBetween( m_pLevelCollisionScene->GetMinBounds().x, m_pLevelCollisionScene->GetMaxBounds().x ),
								0,
								RandomBetween( m_pLevelCollisionScene->GetMinBounds().z, m_pLevelCollisionScene->GetMaxBounds().z ) );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) )
			{
				return Position;
			}

		}
	}
	else
	{
		bool Done( kFalse );
		while( !Done )
		{
			D3DXVECTOR3 Position( RandomBetween( 0, m_pMaze->Width() * m_TileWidth ),
								0,
								RandomBetween( 0, m_pMaze->Height() * -m_TileWidth ) );

			ECellType Type = PositionType( Position );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				Type != KCTypeNSDoor && 
				Type != KCTypeEWDoor )
			{
				return Position;
			}

		}
	}
	return D3DXVECTOR3( 0, 0, 0 );

} // CLevel::RandomOpenLocation()

D3DXVECTOR3 CLevel::RandomOpenPosition( const D3DXVECTOR3& StartPosition,	// generation position
										float32 Radius )				// random radius
{
	if( m_pLevelCollisionScene != NULL )
	{
		bool Done( kFalse );
		uint32 Tries( 0 );
		while( !Done && Tries < 500 )
		{
			D3DXVECTOR3 Position( RandomBetween( StartPosition.x - Radius, StartPosition.x + Radius ),
								0,
								RandomBetween( StartPosition.z - Radius, StartPosition.z + Radius ) );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) )
			{
				return Position;
			}

			Tries++;

		}
	}
	else
	{
		bool Done( kFalse );
		uint32 Tries( 0 );
		while( !Done && Tries < 500 )
		{
			D3DXVECTOR3 Position( RandomBetween( StartPosition.x - Radius, StartPosition.x + Radius ),
								0,
								RandomBetween( StartPosition.z - Radius, StartPosition.z + Radius ) );

			ECellType Type = PositionType( Position );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				Type != KCTypeNSDoor && 
				Type != KCTypeEWDoor  )
			{
				return Position;
			}

			Tries++;

		}
	}
	// just keep expanding if we failed
	return RandomOpenPosition( StartPosition, Radius + 10 );

} // CLevel::RandomOpenPosition()

D3DXVECTOR3 CLevel::RandomOpenPosition( const D3DXVECTOR3& StartPosition,	// generation position
									    float32 MinimumRadius,			// minimum radius
										float32 MaximumRadius )			// maximum radius
{
	D3DXVECTOR3 Position;
	if( m_pLevelCollisionScene != NULL )
	{
		bool Done( kFalse );
		uint32 Tries( 0 );
		while( !Done && Tries < 500 )
		{
			D3DXMATRIX Rotation;
			D3DXMatrixRotationY( &Rotation, (float32)KDeg2Rad * ( RandomBetween( 0, 360 ) ) );
			D3DXVECTOR3 Ray( 0, 0, RandomBetween( MinimumRadius, MaximumRadius ) );
			D3DXVec3TransformNormal( &Ray, &Ray, &Rotation );
			D3DXVECTOR3 Position = StartPosition + Ray;
			
			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) )
			{
				return Position;
			}

			Tries++;

		}
	}
	else
	{
		bool Done( kFalse );
		uint32 Tries( 0 );
		while( !Done && Tries < 500 )
		{
			D3DXMATRIX Rotation;
			D3DXMatrixRotationY( &Rotation, (float32)KDeg2Rad * ( RandomBetween( 0, 360 ) ) );
			D3DXVECTOR3 Ray( 0, 0, RandomBetween( MinimumRadius, MaximumRadius ) );
			D3DXVec3TransformNormal( &Ray, &Ray, &Rotation );
			D3DXVECTOR3 Position = StartPosition + Ray;

			ECellType Type = PositionType( Position );

			if( PositionPassable( Position ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( 0, 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, 0 ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, PassabilityCellWidth() ) ) &&
				PositionPassable( Position + D3DXVECTOR3( -PassabilityCellWidth(), 0, -PassabilityCellWidth() ) ) &&
				Type != KCTypeNSDoor && 
				Type != KCTypeEWDoor  )
			{
				return Position;
			}

			Tries++;

		}
	}
	return StartPosition;

} // CLevel::RandomOpenPosition()

D3DXVECTOR3 CLevel::RandomOpenItemPosition( const D3DXVECTOR3& Position,	// generation position
											float32 Radius )				// random radius
{
	bool Done( kFalse );
	uint32 Tries( 0 );
	while( !Done && Tries < 500 )
	{
		D3DXVECTOR3 Position( RandomBetween( Position.x - Radius, Position.x + Radius ),
							  0,
							  RandomBetween( Position.z - Radius, Position.z + Radius ) );

		if( MapPassable( Position ) )
		{
			return Position;
		}

		Tries++;

	}
	return Position;

} // CLevel::RandomOpenItemPosition()

ECellType CLevel::PositionType( const D3DXVECTOR3& Position )	// position to check
{
	int32 X = (uint32)floor( Position.x / m_TileWidth );
	int32 Y = (uint32)floor( -Position.z / m_TileWidth );
	if( X < 0 || 
		Y < 0 ||
		X >= (int32)m_pMaze->Width() ||
		Y >= (int32)m_pMaze->Height() )
	{
		return KCTypeSolid;
	}

	return m_pMaze->CellType( X, Y );
} // CLevel::PositionType()

int32 CLevel::PositionCellHeight( const D3DXVECTOR3& Position )	// position to check
{
	int32 X = (uint32)floor( Position.x / m_TileWidth );
	int32 Y = (uint32)floor( -Position.z / m_TileWidth );
	if( X < 0 || 
		Y < 0 ||
		X >= (int32)m_pMaze->Width() ||
		Y >= (int32)m_pMaze->Height() )
	{
		return 1;
	}

	return m_pMaze->CellHeight( X, Y );
} // CLevel::PositionType()

bool CLevel::PassableBetween( D3DXVECTOR3 StartPosition,			// position to check
							  const D3DXVECTOR3& EndPosition )		// position to check
{
	D3DXVECTOR3 Delta = EndPosition - StartPosition;
	float32 Length = D3DXVec3Length( &Delta );
	D3DXVec3Normalize( &Delta, &Delta );
	float32 Step = m_CellWidth * .5f;
	Delta *= Step;
	float32 Distance( 0 );
	while( Distance <= Length )
	{
		if( !PositionPassable( StartPosition ) )
		{
			return kFalse;
		}
		StartPosition += Delta;
		Distance += Step;
	}
	return kTrue;

} // CLevel::PassableBetween()

bool CLevel::PositionPassable( const D3DXVECTOR3& Position )	// position to check
{
	// check the passability map
	int32 CellX( (int32)floor( ( Position.x - TopLeftX() ) / m_CellWidth ) );
	int32 CellY( (int32)floor( -( Position.z - TopLeftY() ) / m_CellWidth ) );


	if( CellX < 0 || 
		CellY < 0 ||
		CellX >= (int32)m_PathingCellsWide ||
		CellY >= (int32)m_PathingCellsHigh )
	{
		return kFalse;
	}

	if( m_MapPassabilityData[CellX][CellY] != 0 )
	{
		return kFalse;
	}
	if( m_ObjectPassabilityData[CellX][CellY] > 0 )
	{
		return kFalse;
	}

	return kTrue;
} // CLevel::PositionPassable()

bool CLevel::MapPassable( const D3DXVECTOR3& Position )	// position to check
{
	// check the passability map
	int32 CellX( (int32)floor( ( Position.x - TopLeftX() ) / m_CellWidth ) );
	int32 CellY( (int32)floor( -( Position.z - TopLeftY() ) / m_CellWidth ) );


	if( CellX < 0 || 
		CellY < 0 ||
		CellX >= (int32)m_PathingCellsWide ||
		CellY >= (int32)m_PathingCellsHigh )
	{
		return kFalse;
	}

	if( m_MapPassabilityData[CellX][CellY] != 0 )
	{
		return kFalse;
	}

	return kTrue;
} // CLevel::MapPassable()


void CLevel::IncrementObjectPassability( const D3DXVECTOR3& Position )	// position to check
{
	// check the passability map
	int32 CellX( (int32)floor( ( Position.x - TopLeftX() ) / m_CellWidth ) );
	int32 CellY( (int32)floor( -( Position.z - TopLeftY() ) / m_CellWidth ) );


	if( CellX < 0 || 
		CellY < 0 ||
		CellX >= (int32)m_PathingCellsWide ||
		CellY >= (int32)m_PathingCellsHigh )
	{
		return;
	}


	m_ObjectPassabilityData[CellX][CellY]++;
} // CLevel::IncrementObjectPassability()

void CLevel::DecrementObjectPassability( const D3DXVECTOR3& Position )	// position to check
{
	// check the passability map
	int32 CellX( (int32)floor( ( Position.x - TopLeftX() ) / m_CellWidth ) );
	int32 CellY( (int32)floor( -( Position.z - TopLeftY() ) / m_CellWidth ) );


	if( CellX < 0 || 
		CellY < 0 ||
		CellX >= (int32)m_PathingCellsWide ||
		CellY >= (int32)m_PathingCellsHigh )
	{
		return;
	}

	m_ObjectPassabilityData[CellX][CellY]--;


} // CLevel::DecrementObjectPassability()

float32 CLevel::VisibilityLevel( int32 X,	// visibility cell to check x
								 int32 Y )	// visibility cell to check y	
{
	if( !m_pLevelHistory->HasFog() )
	{
		return 0;
	}
	float32 Level( 0 );
	if( FogVisible( X, Y ) )
	{
		Level++;
	}
	if( FogVisible( X - 1, Y ) )
	{
		Level++;
	}
	if( FogVisible( X + 1, Y ) )
	{
		Level++;
	}
	if( FogVisible( X, Y - 1 ) )
	{
		Level++;
	}
	if( FogVisible( X, Y + 1 ) )
	{
		Level++;
	}
	if( FogVisible( X - 1, Y - 1 ) )
	{
		Level++;
	}
	if( FogVisible( X + 1, Y - 1 ) )
	{
		Level++;
	}
	if( FogVisible( X - 1, Y + 1 ) )
	{
		Level++;
	}
	if( FogVisible( X + 1, Y + 1 ) )
	{
		Level++;
	}
	Level /= 9.0f;
	
	return 1.0f - Level;
} // CLevel::VisibilityLevel()

bool CLevel::Visible( int32 X,	// visibility cell to check x
					  int32 Y )	// visibility cell to check y	
{
	if( !m_pLevelHistory->HasFog() )
	{
		return kTrue;
	}
	return m_pLevelHistory->Visible( X, Y );
} // CLevel::Visible()

bool CLevel::FogVisible( int32 X,	// visibility cell to check x
						 int32 Y )	// visibility cell to check y	
{
	if( !m_pLevelHistory->HasFog() )
	{
		return kTrue;
	}
	if( X >= 0 && Y >= 0 && X < (int32)m_VisibilityCellsWide && Y < (int32)m_VisibilityCellsHigh )
	{
		return m_VisibilityData[X][Y];
	}
	return kFalse;
} // CLevel::FogVisible()

void CLevel::SetVisibility( const D3DXVECTOR3& Position,	// worldspace position
							bool Value )					// value to set
{
	if( !m_pLevelHistory->HasFog() )
	{
		return;
	}
	int32 X( (int32)floor( Position.x / m_VisibilityCellWidth ) );
	int32 Y( (int32)floor( -Position.z / m_VisibilityCellWidth ) );

	if( X >= 0 && Y >= 0 && X < (int32)m_VisibilityCellsWide && Y < (int32)m_VisibilityCellsHigh )
	{
		m_pLevelHistory->SetVisibility( X, Y, Value );
		SetFogVisibility( X, Y, Value );
	}

	X = (int32)floor( Position.x / m_VisibilityCellWidth + .5f );
	Y = (int32)floor( -Position.z / m_VisibilityCellWidth + .5f );

	if( X >= 0 && Y >= 0 && X < (int32)m_VisibilityCellsWide && Y < (int32)m_VisibilityCellsHigh )
	{
		m_pLevelHistory->SetVisibility( X, Y, Value );
		SetFogVisibility( X, Y, Value );
	}

} // CLevel::SetVisibility()

void CLevel::SetFogVisibility( const D3DXVECTOR3& Position,	// worldspace position
							   bool Value )	// visibility value
{
	int32 X( (int32)floor( Position.x / m_VisibilityCellWidth ) );
	int32 Y( (int32)floor( -Position.z / m_VisibilityCellWidth ) );
	SetFogVisibility( X, Y, Value );
} // CLevel::SetFogVisibility()

void CLevel::SetFogVisibility( int32 X,		// visibility cell to check x
							   int32 Y,		// visibility cell to check y	
							   bool Value )	// visibility value
{
	if( X >= 0 && Y >= 0 && X < (int32)m_VisibilityCellsWide - 1 && Y < (int32)m_VisibilityCellsHigh - 1 )
	{
		m_VisibilityData[X][Y] = Value;
	}
} // CLevel::SetFogVisibility()

void CLevel::UpdateVisibility( const D3DXVECTOR3& Position )	// position of light source
{
	if( m_pLevelMesh == NULL )
	{
		return;
	}
	int32 TileX( (int32)floor( Position.x / m_VisibilityCellWidth ) );
	int32 TileY( (int32)floor( -Position.z / m_VisibilityCellWidth ) );
	
	D3DXVECTOR3 VisibilityCenter( (float32)TileX, 0, (float32)TileY );

	if( VisibilityCenter == m_VisibilityCenter )
	{
		m_VisibilityChanged = kFalse;
		return;
	}

	// enable this for true line-of-sight
	ClearFogVisibilityData();

	D3DXVECTOR3 MinBounds( Position );
	D3DXVECTOR3 MaxBounds( Position );
	MinBounds.x -= (float32)KMaxVisibilityRadius * m_VisibilityCellWidth;
	MinBounds.z -= (float32)KMaxVisibilityRadius * m_VisibilityCellWidth;
	MaxBounds.x += (float32)KMaxVisibilityRadius * m_VisibilityCellWidth;
	MaxBounds.z += (float32)KMaxVisibilityRadius * m_VisibilityCellWidth;

	m_VisibilityCenter = VisibilityCenter;

	m_VisibilityChanged = kTrue;

	uint32 CastSteps( 36 );
	D3DXMATRIX Rotation;
	D3DXMatrixRotationY( &Rotation, (float32)KDeg2Rad * ( 360.0f / (float32)CastSteps ) );

	D3DXVECTOR3 Ray = D3DXVECTOR3( 0, 0, (float32)KMaxVisibilityRadius * m_VisibilityCellWidth );
	D3DXVECTOR3 Start( Position );
	D3DXVECTOR3 ImpactPoint;
	D3DXVECTOR3 ImpactNormal;
	D3DXVECTOR3 Color;
	uint32		Material;

	Start.y = m_TileHeight * .5f;
	D3DXVECTOR3 End( Start );

	for( uint32 i = 0; i < CastSteps; i++ )
	{
		End = Start + Ray;
		if( m_pLevelCollision->RayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color ) )
		{
			End = ImpactPoint;
		}
		float32 Length = D3DXVec3Length( &( End - Start ) );
		uint32 Steps = (uint32)floor( Length / m_VisibilityCellWidth + .5f );
		ImpactPoint = Start;
		End -= Start;
		D3DXVec3Normalize( &End, &End );
		End *= m_VisibilityCellWidth;
		for( uint32 j = 0; j < Steps; j++ )
		{
			SetVisibility( ImpactPoint, kTrue );
			//SetVisibility( ImpactPoint + D3DXVECTOR3( m_VisibilityCellWidth, 0, 0 ), kTrue );
			//SetVisibility( ImpactPoint + D3DXVECTOR3( -m_VisibilityCellWidth, 0, 0 ), kTrue );
			//SetVisibility( ImpactPoint + D3DXVECTOR3( 0, 0, m_VisibilityCellWidth ), kTrue );
			//SetVisibility( ImpactPoint + D3DXVECTOR3( 0, 0, -m_VisibilityCellWidth ), kTrue );
			ImpactPoint += End;
		}
		D3DXVec3TransformNormal( &Ray, &Ray, &Rotation );
	}

} // CLevel::UpdateVisiblity()

bool CLevel::SightUnobstructed( const D3DXVECTOR3& Start,	// start position
							    const D3DXVECTOR3& End )	// end position
{
	D3DXVECTOR3 ImpactPoint;
	D3DXVECTOR3 ImpactNormal;
	D3DXVECTOR3 Color;
	uint32		Material;
	
	if( m_pLevelCollision->RayCollision( Start, End, ImpactPoint, ImpactNormal, Material, Color ) )
	{
		return kFalse;
	}
	return kTrue;
} // CLevel::SightUnobstructed()


// contains the rules for determining what visual type a scrap of shadow is
ETerrainType CLevel::MiniMapTypeNW( int32 X,	// x location of cell to retrieve
								   int32 Y )	// y location of cell to retrieve
{		
	int32 Type( m_pMaze->CellHeight( X, Y ) );

	int32 TypeN( m_pMaze->CellHeight( X, Y - 1 ) );
	int32 TypeW( m_pMaze->CellHeight( X - 1, Y ) );

	int32 TypeNW( m_pMaze->CellHeight( X - 1, Y - 1 ) );

	if( Type > 1 ||
		TypeN > 1 ||
		TypeW > 1 ||
		TypeNW > 1 ||
		Type < -1 ||
		TypeN < -1 ||
		TypeW < -1 ||
		TypeNW < -1 )
	{
		return KTTypeOpen;
	}

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeN == Type &&
		TypeW == Type &&
		TypeNW == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeN == Type &&
			 TypeW == Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeNWDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeW != Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeNWDiagonal;
	}
	else if( TypeN == Type &&
			 TypeW != Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeWest;
	}
	else if( TypeN != Type &&
			 TypeW == Type &&
			 TypeNW != Type )
	{
		TerrainType = KTTypeNorth;
	}
	else if( TypeN != Type &&
			 TypeW != Type &&
			 TypeNW == Type )
	{
		TerrainType = KTTypeNWDiagonalJoin;
	}
	else if( TypeN == Type &&
			 TypeW != Type &&
			 TypeNW == Type )
	{
		TerrainType = KTTypeSWDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeW == Type &&
			 TypeNW == Type )
	{
		TerrainType = KTTypeNEDiagonalInner;
	}

	return TerrainType;

} // CLevel::MiniMapTypeNW()

// contains the rules for determining what visual type a scrap of shadow is
ETerrainType CLevel::MiniMapTypeNE( int32 X,		// x location of cell to retrieve
								   int32 Y )	// y location of cell to retrieve
{		
	int32 Type( m_pMaze->CellHeight( X, Y ) );

	int32 TypeN( m_pMaze->CellHeight( X, Y - 1 ) );
	int32 TypeE( m_pMaze->CellHeight( X + 1, Y ) );

	int32 TypeNE( m_pMaze->CellHeight( X + 1, Y - 1 ) );

	ETerrainType TerrainType( KTTypeEmpty );

	if( Type > 1 ||
		TypeN > 1 ||
		TypeE > 1 ||
		TypeNE > 1 ||
		Type < -1 ||
		TypeN < -1 ||
		TypeE < -1 ||
		TypeNE < -1 )
	{
		return KTTypeOpen;
	}

	if( TypeN == Type &&
		TypeE == Type &&
		TypeNE == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeN == Type &&
			 TypeE == Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeNEDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeE != Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeNEDiagonal;
	}
	else if( TypeN == Type &&
			 TypeE != Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeEast;
	}
	else if( TypeN != Type &&
			 TypeE == Type &&
			 TypeNE != Type )
	{
		TerrainType = KTTypeNorth;
	}
	else if( TypeN != Type &&
			 TypeE != Type &&
			 TypeNE == Type )
	{
		TerrainType = KTTypeNEDiagonalJoin;
	}
	else if( TypeN == Type &&
			 TypeE != Type &&
			 TypeNE == Type )
	{
		TerrainType = KTTypeSEDiagonalInner;
	}
	else if( TypeN != Type &&
			 TypeE == Type &&
			 TypeNE == Type )
	{
		TerrainType = KTTypeNWDiagonalInner;
	}

	return TerrainType;

} // CLevel::MiniMapTypeNE()

// contains the rules for determining what visual type a scrap of shadow is
ETerrainType CLevel::MiniMapTypeSW( int32 X,		// x location of cell to retrieve
								   int32 Y )	// y location of cell to retrieve
{		
	int32 Type( m_pMaze->CellHeight( X, Y ) );

	int32 TypeS( m_pMaze->CellHeight( X, Y + 1 ) );
	int32 TypeW( m_pMaze->CellHeight( X - 1, Y ) );

	int32 TypeSW( m_pMaze->CellHeight( X - 1, Y + 1 ) );

	if( Type > 1 ||
		TypeS > 1 ||
		TypeW > 1 ||
		TypeSW > 1 ||
		Type < -1 ||
		TypeS < -1 ||
		TypeW < -1 ||
		TypeSW < -1 )
	{
		return KTTypeOpen;
	}

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeS == Type &&
		TypeW == Type &&
		TypeSW == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeS == Type &&
			 TypeW == Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeSWDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeW != Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeSWDiagonal;
	}
	else if( TypeS == Type &&
			 TypeW != Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeWest;
	}
	else if( TypeS != Type &&
			 TypeW == Type &&
			 TypeSW != Type )
	{
		TerrainType = KTTypeSouth;
	}
	else if( TypeS != Type &&
			 TypeW != Type &&
			 TypeSW == Type )
	{
		TerrainType = KTTypeNEDiagonalJoin;
	}
	else if( TypeS == Type &&
			 TypeW != Type &&
			 TypeSW == Type )
	{
		TerrainType = KTTypeNWDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeW == Type &&
			 TypeSW == Type )
	{
		TerrainType = KTTypeSEDiagonalInner;
	}

	return TerrainType;

} // CLevel::MiniMapTypeSW()

// contains the rules for determining what visual type a scrap of shadow is
ETerrainType CLevel::MiniMapTypeSE( int32 X,		// x location of cell to retrieve
								    int32 Y )	// y location of cell to retrieve
{		
	int32 Type( m_pMaze->CellHeight( X, Y ) );

	int32 TypeS( m_pMaze->CellHeight( X, Y + 1 ) );
	int32 TypeE( m_pMaze->CellHeight( X + 1, Y ) );

	int32 TypeSE( m_pMaze->CellHeight( X + 1, Y + 1 ) );

	if( Type > 1 ||
		TypeS > 1 ||
		TypeE > 1 ||
		TypeSE > 1 ||
		Type < -1 ||
		TypeS < -1 ||
		TypeE < -1 ||
		TypeSE < -1 )
	{
		return KTTypeOpen;
	}

	ETerrainType TerrainType( KTTypeEmpty );

	if( TypeS == Type &&
		TypeE == Type &&
		TypeSE == Type )
	{
		TerrainType = KTTypeOpen;
	}
	else if( TypeS == Type &&
			 TypeE == Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeSEDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeE != Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeSEDiagonal;
	}
	else if( TypeS == Type &&
			 TypeE != Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeEast;
	}
	else if( TypeS != Type &&
			 TypeE == Type &&
			 TypeSE != Type )
	{
		TerrainType = KTTypeSouth;
	}
	else if( TypeS != Type &&
			 TypeE != Type &&
			 TypeSE == Type )
	{
		TerrainType = KTTypeNWDiagonalJoin;
	}
	else if( TypeS == Type &&
			 TypeE != Type &&
			 TypeSE == Type )
	{
		TerrainType = KTTypeNEDiagonalInner;
	}
	else if( TypeS != Type &&
			 TypeE == Type &&
			 TypeSE == Type )
	{
		TerrainType = KTTypeSWDiagonalInner;
	}

	return TerrainType;

} // CLevel::MiniMapTypeSE()
