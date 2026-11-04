/*
	StatsAndAchievements.h

	Copyright (C) 2013 WildTangent, Inc. 
	All Rights Reserved

	[BLG] Ivan Fernandez
	10/22/2013

	Taken from Steamworks SDK Sample
*/

#include "StatsAndAchievements.h"

//#include "steam_api.h"

#define _ACH_ID( id ) { id, #id, 0 }

Achievement_t g_rgAchievements[] = 
{
	_ACH_ID( ACH_FATE_SEEKER		 ),
	_ACH_ID( ACH_DEEP_SEEKER		 ),
	_ACH_ID( ACH_GEM_COLLECTOR		 ),
	_ACH_ID( ACH_FATE_INSURED		 ),
	_ACH_ID( ACH_FISHER_KING		 ),
	_ACH_ID( ACH_WARRIOR_SPIRIT		 ),
	_ACH_ID( ACH_GOLDEN_EYES		 ),
	_ACH_ID( ACH_CONQUEROR_OF_DEATH	 ),
	_ACH_ID( ACH_PLEASANT_WARRIOR	 ),
	_ACH_ID( ACH_I_AM_LEGEND		 ),
	_ACH_ID( ACH_FLAWLESS_PET		 ),
	_ACH_ID( ACH_SPELL_MASTER		 ),
	_ACH_ID( ACH_THE_SMASHER		 ),
	_ACH_ID( ACH_SKILLFULL_WARRIOR	 ),
	_ACH_ID( ACH_ENERGIZED			 ),
};

CStatsAndAchievements* CStatsAndAchievements::m_pStatsAndAchievements = NULL;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
#pragma warning( push )
//  warning C4355: 'this' : used in base member initializer list
//  This is OK because it's warning on setting up the Steam callbacks, they won't use this until after construction is done
#pragma warning( disable : 4355 ) 
CStatsAndAchievements::CStatsAndAchievements( )
	: 
#ifndef NO_STEAM
	m_pSteamUser( NULL ),
	m_pSteamUserStats( NULL ),
	m_GameID( SteamUtils()->GetAppID() ),
	m_CallbackUserStatsReceived( this, &CStatsAndAchievements::OnUserStatsReceived ),
	m_CallbackUserStatsStored( this, &CStatsAndAchievements::OnUserStatsStored ),
	m_CallbackAchievementStored( this, &CStatsAndAchievements::OnAchievementStored ),
#endif
	m_pPlayer( NULL ),
	m_bAchivementsEnabled(true)
{
#ifndef NO_STEAM
	m_pSteamUser = SteamUser();
	m_pSteamUserStats = SteamUserStats();
#endif

	m_bRequestedStats = false;
	m_bStatsValid = false;
	m_bStoreStats = false;

	m_characterCustomized = kFalse;
	m_isWarriorSpirit = kFalse;
	m_dungeonLevel50Reached = kFalse;
	m_descendentCharacterCreated = kFalse;
	m_gameCompletedLegend = kFalse;
	m_flawlessPet = kFalse;
	m_spellMaster = 0;
	m_theSmasher = 0;
	m_skillfullWarrior = 0;
	m_gemTypesCollected = 0;
	m_fishTypesCollected = 0;
	m_openedTreasureChests = 0;
	m_resurrectionPillTypesUsed = 0;
	m_completedSideQuests = 0;
	m_energized = 0;
	m_pStatsAndAchievements = this;
}
#pragma warning( pop )

#ifndef NO_STEAM
void CStatsAndAchievements::Initialize()
{
	if ( !m_bRequestedStats )
	{		
		// Is Steam Loaded? if no, can't get stats, done
		if ( NULL == m_pSteamUserStats || NULL == m_pSteamUser )
		{
			m_bRequestedStats = true;
			return;
		}

		// If yes, request our stats
		bool bSuccess = m_pSteamUserStats->RequestCurrentStats();

		// This function should only return false if we weren't logged in, and we already checked that.
		// But handle it being false again anyway, just ask again later.
		m_bRequestedStats = bSuccess;
	}
}
#else
void CStatsAndAchievements::Initialize()
{
	m_bRequestedStats = true;
}
#endif

bool CStatsAndAchievements::Enabled()
{ 
	//Cheat System Control / Achievements
	if(m_bAchivementsEnabled && m_pPlayer)
		m_bAchivementsEnabled = !(m_pPlayer->GetDisabledCheatAchievements());

	return m_bAchivementsEnabled; 
}

//-----------------------------------------------------------------------------
// Purpose: Run a frame for the CStatsAndAchievements
//-----------------------------------------------------------------------------
void CStatsAndAchievements::Update()
{
	if ( !m_bStatsValid )
		return;

	if ( !Enabled() )
		return;

	// Check for dirty stats to process the achievements
	if( !m_bStoreStats )
	{
		return;
	}
	else
	{
		//Save stats just if needed
		StoreStatsIfNecessary();
	}

	// Evaluate achievements
	for ( int iAch = 0; iAch < ACH_COUNT; ++iAch )
	{
		EvaluateAchievement( g_rgAchievements[iAch] );
	}
}

//-----------------------------------------------------------------------------
// Purpose: see if we should unlock this achievement
//-----------------------------------------------------------------------------
void CStatsAndAchievements::EvaluateAchievement( Achievement_t &achievement )
{
	// Already have it?
	if ( achievement.m_bAchieved )
		return;

	switch ( achievement.m_eAchievementID )
	{
	case ACH_FATE_SEEKER:
		if (m_characterCustomized != kTrue) return;
		break;
	case ACH_DEEP_SEEKER:
		if (m_dungeonLevel50Reached != kTrue) return;
		break;
	case ACH_GEM_COLLECTOR:
		if (!m_pPlayer) return;
		if (m_pPlayer->GetGemsCollectedNumber() < 15) return;
		break;
	case ACH_FATE_INSURED:
		if (m_descendentCharacterCreated != kTrue) return;
		break;
	case ACH_FISHER_KING:
		if (!m_pPlayer) return;
		if (m_pPlayer->GetFishCaughtNumber() < 20) return;
		break;
	case ACH_WARRIOR_SPIRIT:
		if (m_isWarriorSpirit != kTrue) return;
		break;
	case ACH_GOLDEN_EYES:		
		if (!m_pPlayer) return;
		if (m_pPlayer->JournalStat(KJournalStatLargeChestsOpened) < 10) return;
		break;
	case ACH_CONQUEROR_OF_DEATH:
		if (m_resurrectionPillTypesUsed < 3) return;
		break;
	case ACH_PLEASANT_WARRIOR:
		if (m_completedSideQuests < 10) return;
		break;
	case ACH_I_AM_LEGEND:
		if (m_gameCompletedLegend != kTrue) return;
		break;
	case ACH_FLAWLESS_PET:
		if (m_flawlessPet != kTrue) return;
		break;
	case ACH_SPELL_MASTER:
		if (!m_pPlayer) return;
		if (m_pPlayer->Magic() < 25) return;
		break;
	case ACH_THE_SMASHER:
		if (!m_pPlayer) return;
		if (m_pPlayer->Strength() < 45) return;
		break;
	case ACH_SKILLFULL_WARRIOR:
		if (!m_pPlayer) return;
		if (m_pPlayer->Dexterity() < 40) return;
		break;
	case ACH_ENERGIZED:
		if (!m_pPlayer) return;
		if (m_pPlayer->Vitality() < 45) return;
		break;
	}

	//If we reach this point means the achievement must be unlocked
	UnlockAchievement( achievement );
}


//-----------------------------------------------------------------------------
// Purpose: Unlock this achievement
//-----------------------------------------------------------------------------
#ifndef NO_STEAM
void CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )
{
	achievement.m_bAchieved = true;

	// mark it down
	m_pSteamUserStats->SetAchievement( achievement.m_pchAchievementID );

	// Store stats end of frame
	m_bStoreStats = true;
}
#else
void CStatsAndAchievements::UnlockAchievement( Achievement_t &achievement )
{
	achievement.m_bAchieved = true;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Store stats in the Steam database
//-----------------------------------------------------------------------------
#ifndef NO_STEAM
void CStatsAndAchievements::StoreStatsIfNecessary()
{
	if ( m_bStoreStats )
	{
		// set stats
		/*m_pSteamUserStats->SetStat( "CollectedGemTypes", m_gemTypesCollected );
		m_pSteamUserStats->SetStat( "CollectedFishTypes", m_fishTypesCollected );
		m_pSteamUserStats->SetStat( "OpenedTreasureChests", m_openedTreasureChests );
		m_pSteamUserStats->SetStat( "UsedPillTypes", m_resurrectionPillTypesUsed );
		m_pSteamUserStats->SetStat( "CompletedSideQuests", m_completedSideQuests );*/

		bool bSuccess = m_pSteamUserStats->StoreStats();
		// If this failed, we never sent anything to the server, try
		// again later.
		m_bStoreStats = !bSuccess;
	}
}
#else
void CStatsAndAchievements::StoreStatsIfNecessary()
{
	m_bStoreStats = false;
}
#endif


#ifndef NO_STEAM
//-----------------------------------------------------------------------------
// Purpose: We have stats data from Steam. It is authoritative, so update
//			our data with those results now.
//-----------------------------------------------------------------------------
void CStatsAndAchievements::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	//SteamUserStats()->ResetAllStats(true);
	if ( !m_pSteamUserStats )
		return;

	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			m_bStatsValid = true;

			// load achievements
			for ( int iAch = 0; iAch < ACH_COUNT; ++iAch )
			{
				Achievement_t &ach = g_rgAchievements[iAch];
				m_pSteamUserStats->GetAchievement( ach.m_pchAchievementID, &ach.m_bAchieved );
			}

			// load stats			
			/*m_pSteamUserStats->GetStat( "CollectedGemTypes", &m_gemTypesCollected );
			m_pSteamUserStats->GetStat( "CollectedFishTypes", &m_fishTypesCollected );
			m_pSteamUserStats->GetStat( "OpenedTreasureChests", &m_openedTreasureChests );
			m_pSteamUserStats->GetStat( "UsedPillTypes", &m_resurrectionPillTypesUsed );
			m_pSteamUserStats->GetStat( "CompletedSideQuests", &m_completedSideQuests );*/
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Our stats data was stored!
//-----------------------------------------------------------------------------
void CStatsAndAchievements::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( k_EResultOK == pCallback->m_eResult )
		{
			//OutputDebugString( "StoreStats - success\n" );
		}
		else if ( k_EResultInvalidParam == pCallback->m_eResult )
		{
			// One or more stats we set broke a constraint. They've been reverted,
			// and we should re-iterate the values now to keep in sync.
			//OutputDebugString( "StoreStats - some failed to validate\n" );
			// Fake up a callback here so that we re-load the values.
			UserStatsReceived_t callback;
			callback.m_eResult = k_EResultOK;
			callback.m_nGameID = m_GameID.ToUint64();
			OnUserStatsReceived( &callback );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: An achievement was stored
//-----------------------------------------------------------------------------
void CStatsAndAchievements::OnAchievementStored( UserAchievementStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if ( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if ( 0 == pCallback->m_nMaxProgress )
		{
			//OutputDebugString( "Achievement Unlocked\n" );
		}
		else
		{
			//OutputDebugString( "Achievement progress callback\n" );
		}
	}
}

#endif // NO_STEAM

//------------------------------------------------------
// Stats Accessors
//------------------------------------------------------

void CStatsAndAchievements::IncrementTreasureChestOpenedCounter() 
{ 
	m_openedTreasureChests += 1; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::IncrementCompletedSideQuestsCounter() 
{ 
	m_completedSideQuests += 1;
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::FinishedCharacterCustomization() 
{ 
	m_characterCustomized = kTrue; 
	m_bStoreStats = kTrue;
	Update();
}
void CStatsAndAchievements::ReachDungeonLevel50() 
{ 
	m_dungeonLevel50Reached = kTrue; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::CreateDescendentCharacter() 
{ 
	m_descendentCharacterCreated = kTrue; 
	m_bStoreStats = kTrue;
	Update();
}
void CStatsAndAchievements::SetWarriorSpirit() 
{ 
	m_isWarriorSpirit = kTrue; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::SetGameCompletedInLegend() 
{ 
	m_gameCompletedLegend = kTrue; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::SetFlawlessPet() 
{ 
	m_flawlessPet = kTrue;
	m_bStoreStats = kTrue;
	Update();
}
void CStatsAndAchievements::SetCharacterMagicLevel(uint32 magicLevel) 
{ 
	m_spellMaster = magicLevel; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::SetCharacterStrengthLevel(uint32 strengthLevel) 
{ 
	m_theSmasher = strengthLevel; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::SetCharacterDesterityLevel(uint32 dexterityLevel) 
{ 
	m_skillfullWarrior = dexterityLevel; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::SetCharacterVitalityLevel(uint32 vitalityLevel) 
{ 
	m_energized = vitalityLevel; 
	m_bStoreStats = kTrue;
}
void CStatsAndAchievements::CheckStats() 
{ 
	m_bStoreStats = kTrue;
	Update();
}
void CStatsAndAchievements::Enable() 
{ 
	m_bAchivementsEnabled = true;
}
void CStatsAndAchievements::Disable() 
{ 
	m_bAchivementsEnabled = false;
}