/*
	StatsAndAchievements.h

	Copyright (C) 2013 WildTangent, Inc. 
	All Rights Reserved

	[BLG] Ivan Fernandez
	10/22/2013

	Taken from Steamworks SDK Sample
*/

#ifndef STATS_AND_ACHIEVEMENTS_H
#define STATS_AND_ACHIEVEMENTS_H

#include "../CHARACTER/character.h"
#include "steam_api.h"

enum EAchievements
{
	ACH_FATE_SEEKER		= 0,
	ACH_DEEP_SEEKER		= 1,
	ACH_GEM_COLLECTOR	= 2,
	ACH_FATE_INSURED	= 3,
	ACH_FISHER_KING		= 4,
	ACH_WARRIOR_SPIRIT	= 5,
	ACH_GOLDEN_EYES		= 6,
	ACH_CONQUEROR_OF_DEATH = 7,
	ACH_PLEASANT_WARRIOR = 8,
	ACH_I_AM_LEGEND		= 9,
	ACH_FLAWLESS_PET	= 10,
	ACH_SPELL_MASTER	= 11,
	ACH_THE_SMASHER		= 12,
	ACH_SKILLFULL_WARRIOR = 13,
	ACH_ENERGIZED		= 14,
	ACH_COUNT			= 15,
};

struct Achievement_t
{
	EAchievements m_eAchievementID;
	const char *m_pchAchievementID;
	bool m_bAchieved;
};

class CStatsAndAchievements
{
public:
	// Constructor
	CStatsAndAchievements();

	void Update();

	void Initialize();

	// Stats Accessors
	void IncrementTreasureChestOpenedCounter();
	void IncrementCompletedSideQuestsCounter();
	void FinishedCharacterCustomization();
	void ReachDungeonLevel50();
	void CreateDescendentCharacter();
	void SetWarriorSpirit();
	void SetGameCompletedInLegend();
	void SetFlawlessPet();
	void SetCharacterMagicLevel(uint32 magicLevel);
	void SetCharacterStrengthLevel(uint32 strengthLevel);
	void SetCharacterDesterityLevel(uint32 dexterityLevel);
	void SetCharacterVitalityLevel(uint32 vitalityLevel);
	void SetPlayer(CCharacter* pPlayer) { m_pPlayer = pPlayer; }
	void CheckStats();
	void Enable();
	void Disable();
	bool Enabled();

#ifndef NO_STEAM
	//Steam Callback Declarations
	STEAM_CALLBACK( CStatsAndAchievements, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );
	STEAM_CALLBACK( CStatsAndAchievements, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored );
	STEAM_CALLBACK( CStatsAndAchievements, OnAchievementStored, UserAchievementStored_t, m_CallbackAchievementStored );	
#endif

	static CStatsAndAchievements* GetInstance()
	{
		return m_pStatsAndAchievements;
	}

private:

	static CStatsAndAchievements*	m_pStatsAndAchievements;
	// Determine if we get this achievement now
	void EvaluateAchievement( Achievement_t &achievement );
	void UnlockAchievement( Achievement_t &achievement );

	// Store stats
	void StoreStatsIfNecessary();

#ifndef NO_STEAM
	// our GameID
	CGameID m_GameID;

	// Steam User interface
	ISteamUser *m_pSteamUser;

	// Steam UserStats interface
	ISteamUserStats *m_pSteamUserStats;
#endif

	// Did we get the stats from Steam?
	bool m_bRequestedStats;
	bool m_bStatsValid;

	// Should we store stats this frame?
	bool m_bStoreStats;

	// Achievement Stats Counters
	bool	m_flawlessPet;
	bool	m_isWarriorSpirit;
	bool	m_characterCustomized;
	bool	m_dungeonLevel50Reached;
	bool	m_descendentCharacterCreated;
	bool	m_gameCompletedLegend;
	int32	m_gemTypesCollected;
	int32	m_fishTypesCollected;
	int32	m_openedTreasureChests;
	int32	m_resurrectionPillTypesUsed;
	int32	m_completedSideQuests;
	int32	m_spellMaster;
	int32	m_theSmasher;
	int32  m_skillfullWarrior;
	int32	m_energized;

	bool	m_bAchivementsEnabled;

	CCharacter*		m_pPlayer;
};

#endif // STATS_AND_ACHIEVEMENTS_H