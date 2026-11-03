/*
	GameSave.h

	Copyright (C) 2013 WildTangent, Inc. 
	All Rights Reserved

	[BLG] Juan Jalil
	11/06/2013
*/

#ifndef _GAMESAVE_H
#define _GAMESAVE_H

#include "character.h"
#include "../UTILITIES/constants.h"
#include "../GAMECLIENT/gamerules.h"
#include "steam_api.h"

class CGameSave
{
public :
	CGameSave();
	~CGameSave() {};

	int32 NextAvailableSlot(std::string localization);
	bool StoreSlotOnCloud(int32 SlotIndex, std::string cachePath, std::string localization);
	bool GetSlotDataFromCloud(int32 SlotIndex, std::string cachePath, std::string localization);
	bool SyncProfiles(std::string cachePath, std::string localization, std::vector< std::string > ProfileFiles);
	int32 HasGameSaves(std::string localization);
	bool DeleteProfile(int32 SlotIndex, std::string localization);

private:
	// Steam RemoteStorage interface
	ISteamRemoteStorage *m_pSteamRemoteStorage;

	void DeleteLocalProfiles(std::vector< std::string >	ProfileFiles);
};

#endif