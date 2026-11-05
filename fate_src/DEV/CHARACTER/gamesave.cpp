/*
	GameSave.m

	Copyright (C) 2013 WildTangent, Inc. 
	All Rights Reserved

	[BLG] Juan Jalil
	11/06/2013
*/

#include "gamesave.h"
#include "../UTILITIES/vistafileutilities.h"

#define MAX_GAME_SAVES	100

CGameSave::CGameSave()
{
#ifdef NO_STEAM
    m_pSteamRemoteStorage = NULL;
#else
    m_pSteamRemoteStorage = SteamRemoteStorage();
#endif
}

bool CGameSave::SyncProfiles(std::string cachePath, std::string localization, 
							 std::vector< std::string > ProfileFiles)
{
	
	if(!m_pSteamRemoteStorage || !m_pSteamRemoteStorage->IsCloudEnabledForAccount()) return false;

	int32 files = 0;
	
	DeleteLocalProfiles(ProfileFiles);

	BOOL foundSlot( kFalse );
	//check for existing slot files
	while (files <= MAX_GAME_SAVES )
    {
		foundSlot = m_pSteamRemoteStorage->FileExists(("SAVE//" +  
			localization + "//" + IntegerToString( files ) + "." + KSaveExtension).c_str());
		if(foundSlot)
		{
			GetSlotDataFromCloud(files, 
				(cachePath + IntegerToString( files ) + "." + KSaveExtension).c_str(), localization);
		}
		files++;
	}

	return true;
}
/*int32 CGameSave::HasGameSaves(std::string localization)
{
	if(!m_pSteamRemoteStorage->IsCloudEnabledForAccount) return false;

	return m_pSteamRemoteStorage->GetFileCount() > 0;
}*/

int32 CGameSave::HasGameSaves(std::string localization)
{
	if(!m_pSteamRemoteStorage || !m_pSteamRemoteStorage->IsCloudEnabledForAccount()) return -1;

	int32 i = 0;	
	
	BOOL found( kFalse );
	//check for existing slot files
	while (i <= MAX_GAME_SAVES )
    {
		found = m_pSteamRemoteStorage->FileExists(("SAVE//" + localization + "//" + IntegerToString( i ) + "." + KSaveExtension).c_str());
		if (found)
		{
			return 1;
		}
		i++;
	}

	return 0;
}
int32 CGameSave::NextAvailableSlot(std::string localization)
{
	/*m_pSteamRemoteStorage->FileDelete("harry.dat");
	m_pSteamRemoteStorage->FileDelete("message2.dat");
	m_pSteamRemoteStorage->FileDelete("message3.dat");
	m_pSteamRemoteStorage->FileDelete("message123.dat");
	m_pSteamRemoteStorage->FileDelete("pepito.dat");
	m_pSteamRemoteStorage->FileDelete("pepito2.dat");
	m_pSteamRemoteStorage->FileDelete("SAVE//1.ffd");
	m_pSteamRemoteStorage->FileDelete("SAVE//2.ffd");
	return 1;*/
	
	if(!m_pSteamRemoteStorage || !m_pSteamRemoteStorage->IsCloudEnabledForAccount()) return -1;
	
	int32 files = 0;	
	
	BOOL found( kFalse );
	//check for existing slot files
	while (files <= MAX_GAME_SAVES )
    {
		found = m_pSteamRemoteStorage->FileExists(("SAVE//" + localization + "//" + IntegerToString( files ) + "." + KSaveExtension).c_str());
		if (!found)
		{
			return files;
		}
		files++;
	}

	return files -1;
}
bool CGameSave::StoreSlotOnCloud(int32 SlotIndex, std::string cachePath, std::string localization)
{
	if(!m_pSteamRemoteStorage || !m_pSteamRemoteStorage->IsCloudEnabledForAccount()) return false;
	
	FILE *pFileRead;
	pFileRead = fopen ( cachePath.c_str(), "rb" );
	fseek(pFileRead, 0, SEEK_END);
	long fsize = ftell(pFileRead);
	fseek(pFileRead, 0, SEEK_SET);

	char *buffer = (char*)malloc(fsize + 1);
	fread(buffer, fsize, 1, pFileRead);	

	fclose(pFileRead);
	
	buffer[fsize] = 0;

	bool bRet = m_pSteamRemoteStorage->FileWrite( ("SAVE//" + localization + "//" + IntegerToString( SlotIndex ) + "." + KSaveExtension).c_str(), buffer, fsize );
	return bRet;
}

bool CGameSave::GetSlotDataFromCloud(int32 SlotIndex, std::string cachePath, std::string localization)
{
	if(!m_pSteamRemoteStorage || !m_pSteamRemoteStorage->IsCloudEnabledForAccount()) return false;
	
	std::string savePath = "SAVE//" + localization + "//" + IntegerToString( SlotIndex ) + "." + KSaveExtension;
	//get file size
	int32 fileSize = m_pSteamRemoteStorage->GetFileSize(savePath.c_str());

	//char *bufferRead = (char*)malloc(fileSize);
	//char *bufferRead = (char*)malloc (sizeof (char) * fileSize);
	char* bufferRead = new char[fileSize];
	//char bufferRead[1000000];

	int32 cubRead = m_pSteamRemoteStorage->FileRead( savePath.c_str(), bufferRead, fileSize );
	//bufferRead[cubRead] = 0;

	CVistaFileUtilities::CreateTempFile(cachePath);
	FILE *pFileWrite;
	pFileWrite = fopen ( cachePath.c_str(), "wb" );
	fwrite( bufferRead, cubRead, 1, pFileWrite );
	fclose( pFileWrite );
	return cubRead > 0;
}

bool CGameSave::DeleteProfile(int32 SlotIndex, std::string localization)
{
	if(!m_pSteamRemoteStorage || !m_pSteamRemoteStorage->IsCloudEnabledForAccount()) return false;

	return m_pSteamRemoteStorage->FileDelete( ("SAVE//" + localization + "//" + IntegerToString( SlotIndex ) + "." + KSaveExtension).c_str());
}

//this method avoids ghost files on game's directory, on any case where 
//a game save is deleted on other computer
void CGameSave::DeleteLocalProfiles(std::vector< std::string >	ProfileFiles)
{
	for( uint32 i = 0; i < ProfileFiles.size(); i++ )
	{
		DeleteFile(ProfileFiles[i].c_str());
	}
}