/*
	level.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _GAMECLIENT_H
#define _GAMECLIENT_H

#include "../IO/keyhandler.h"
#include "../IO/mousehandler.h"
#include "../UTILITIES/frustum.h"
#include "gamerules.h"
#include "gameui.h"
#include "../CHARACTER/gamesave.h" //cloud saves

const static float32	KBlurRate( .015f );

const static float32	KMinimumDolly( 30 );
const static float32	KMaximumDolly( 75 );
const static float32	KDefaultDolly( 60 );

class CRefManager;
class CSettings;
class CCamera;
class CFrustum;
class CLevel;
class CLightManager;
class CCharacter;
class CGameStateManager;
class CGameUI;
class CDataLight;
class CMaterial;
class CParticleManager;
class CGlowManager;
class CBlurLayer;
class CPositionalAudioManager;
class CCharacterTemplate;
class CItemTemplate;
class CItem;
class CTemplateManifest;
class CSpellTemplate;
class CSoundManager;
class CNameGenerator;
class CQuestDescriptions;
class CCharacterSaveInstance;
class CTips;
class CLoadingMenu;

// owns the in-game experience - 
// manages levels, objects, creatures, lights, and in-game UI
class CGameClient
{


public:

			CGameClient( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
						 CRefManager& pRefManager,					// pointer to the reference manager, for tracking media
						 CSettings& pSettings,						// pointer to the settings manager, for keeping track of card/machine capabilities
						 CGameStateManager& pGameStateManager,		// game manager
						 CLoadingMenu& pLoadingMenu,
						 CSoundManager& pSoundManager );

			~CGameClient( void );

	void	FinalizeCreation( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	PreRender( LPDIRECT3DDEVICE8 pD3DDevice );					// direct3d device

	void	DestroyLevel( void );

	void	StartMusic( void );

	void	StopMusic( void );

	void	GenerateLevel( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   int32 LevelDepth,						// dungeon level depth
						   bool Descending,							// have we just gone down?
						   bool IsTeleport,							// is this a teleport?
						   bool IsFateRespawn );					// is this a teleport?

	void	FlushAll( void );

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );						// direct3d device

	void	RenderUI( LPDIRECT3DDEVICE8 pD3DDevice );					// direct3d device

	void	RenderBlurLayer( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void	Update( LPDIRECT3DDEVICE8 pD3DDevice,			// d3d device
					HWND hWnd,								// hwnd of the active window
					float32 TimeElapsed );					// time elapsed, in seconds

	void	UpdateBlurRate( float32 FPS );	// frames/per/second

	void	ResetMouse( void );

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONUP, etc.
					    WPARAM WMParam );	// the wparam which contains mousewheel info

	void	SetDifficulty( EDifficulty Difficulty )			{	m_Difficulty = Difficulty;		};

	void	SetNewCharacterLineage( int32 Lineage )			{	m_NewCharacterLineage = Lineage;	};

	void	SetNewCharacterQuest( CQuest& pQuest );			// quest

	void	SetNewCharacterName( const std::string& Name )	{	m_NewCharacterName = Name;		};

	void	SetNewCharacterGender( EGender Gender )			{	m_NewCharacterGender = Gender;	};

	void	SetNewCharacterHeadIndex( int32 Index )			{	m_NewCharacterHeadIndex = Index;	};

	void	SetNewCharacterHairIndex( int32 Index )			{	m_NewCharacterHairIndex = Index;	};

	void	SetNewCharacterPetName( const std::string& Name )	{	m_NewCharacterPetName = Name;		};

	void	SetNewPetIsDog( bool IsDog )					{	m_NewPetIsDog = IsDog;			};

	void	SetSaveGamePath( const std::string& Path )		{	m_SaveGamePath = Path;			};

	void	SetCurrentSlotIndex( int32 index)		{	m_pSlotIndex = index;			};

	void	SaveCharacter( void );

	void	UpdateShadowResolution( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	Pause( void )									{	m_pGameUI->Pause( *m_pActiveLevel );				};

	
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	int32				RetirementLineage( void )	{	return m_RetirementLineage;		};

	const std::string&	RetirementText( void )		{	return m_RetirementText;		};

	EDifficulty			Difficulty( void )		{	return m_Difficulty;	};

	CTips*				Tips( void )			{	return m_pTips;			};

	CCamera*			Camera( void )			{	return m_pCamera;		};

	const std::string&	DebugText( void )		{	return m_DebugText;		};

	CLevel*				Level( void )			{	return m_pActiveLevel;	};

	CCharacterTemplate*	CharacterManifest( void )	{	return m_pCharacterManifest;	};

	CItemTemplate*		ItemManifest( void )		{	return m_pItemManifest;	};

	CSpellTemplate*		SpellManifest( void )		{	return m_pSpellManifest;	};

	CNameGenerator*		NameGenerator( void )		{	return m_pNameGenerator;	};

	CQuestDescriptions*	QuestDescriptions( void )	{	return m_pQuestDescriptions;	};

	CGlowManager*				GlowManager( void )				{	return m_pGlowManager;				};

	CParticleManager*			ParticleManager( void )			{	return m_pParticleManager;			};

	CPositionalAudioManager*	PositionalAudioManager( void )	{	return m_pPositionalAudioManager;	};

	CBlurLayer*					BlurLayer( void )				{	return m_pBlurLayer;				};

	CMaterial*					RenderedShadow( void )			{	return m_pRenderedShadow;			};

	CMaterial*					RenderedLight( void )			{	return m_pRenderedLight;			};

	CLightManager*				LightManager( void )			{	return m_pLightManager;				};

	bool	Paused( void )										{	return m_pGameUI->Paused();			};

	void						FindWorldLocation( D3DXVECTOR3& Location );	// location to fill

private:


	void	UpdateQuests( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	SpawnQuests( bool RemembersLevel );				// player knows this level already?

	void	LoadCharacter( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   const std::string& Path,			// path to file
						   std::vector< CCharacter* >& PetList,		// list of pets
						   std::vector< CCharacterSaveInstance* >& PetDescriptions );	// list of pets

	void	AutoGetGold( void );

	void	VerifyMouseOvers( void );

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						   bool Orthographic = kFalse );			// false by default

	void	UpdateMouseOver( void );

	void	Click( void );

	void	Hold( void );

	void	WalkToMouse( void );

	void	ProcessInput( HWND hWnd );		// hwnd of the active window

	CGameStateManager&	m_pGameStateManager;

	CSettings&		m_pSettings;
	CRefManager&	m_pRefManager;

	CSoundManager&	m_pSoundManager;

	CKeyHandler		m_KeyHandler;
	CMouseHandler	m_MouseHandler;

	CFrustum		m_ShadowFrustum;

	CLevel*			m_pActiveLevel;

	CCamera*		m_pCamera;

	CBlurLayer*		m_pBlurLayer;		// light motion blur layer
	float32			m_BlurTime;

	CLightManager*	m_pLightManager;

	CParticleManager*	m_pParticleManager;

	CGlowManager*	m_pGlowManager;

	CNameGenerator*	m_pNameGenerator;

	CCharacter*		m_pMouseOverCharacter;
	CItem*			m_pMouseOverItem;

	// positional audio manager
	CPositionalAudioManager*	m_pPositionalAudioManager;

	CCharacter*		m_pPlayer;

	CTemplateManifest*	m_pTemplateManifest;

	CQuestDescriptions*	m_pQuestDescriptions;

	CCharacterTemplate*	m_pCharacterManifest;
	CItemTemplate*		m_pItemManifest;
	CSpellTemplate*		m_pSpellManifest;

	CGameUI*		m_pGameUI;

	CMaterial*		m_pRenderedShadow;
	CMaterial*		m_pRenderedLight;
	CMaterial*		m_pBlack;
	CMaterial*		m_pWhite;


	D3DXMATRIX		m_ShadowViewMatrix;

	float32			m_Dolly;		// current camera dolly
	float32			m_TimeToRePath;	// timer that must be zero before the player can re-path

	std::string		m_DebugText;

	bool			m_HeldWhenInactive;

	int32			m_DungeonSeed;
	int32			m_ItemSeed;

	bool			m_NewPetIsDog;

	bool			m_ClickBroken;			// button was released

	std::string		m_NewCharacterName;		// name to give to newly created characters
	std::string		m_NewCharacterPetName;		// name to give to newly created characters
	EGender			m_NewCharacterGender;
	int32			m_NewCharacterHeadIndex;
	int32			m_NewCharacterHairIndex;
	std::string		m_SaveGamePath;			// path to active save game
	int32			m_pSlotIndex;			// current slot index to save on cloud

	CTips*			m_pTips;

	CLoadingMenu&	m_pLoadingMenu;

	FSOUND_SAMPLE*	m_pGoldSound;
	FSOUND_SAMPLE*	m_pRewardSound;
	FSOUND_SAMPLE*	m_pQuestSound;
	FSOUND_SAMPLE*	m_pErrorSound;
	FSOUND_SAMPLE*	m_pEnchantSound;
	FSOUND_SAMPLE*	m_pFailSound;

	float32			m_EventTime;
	float32			m_RenderTime;

	CQuest*			m_pMasterQuest;

	EDifficulty		m_Difficulty;

	CItemSaveInstance*	m_pHeirloomItem;
	int32				m_RetirementLineage;
	int32				m_NewCharacterLineage;

	std::string			m_RetirementText;

	CGameSave*			m_pGameSave;

	/* static HMODULE localizedDLL;
	static std::string localeString; */
};

#endif