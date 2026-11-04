/*
	GameTracking.h

	Game play tracking support class 
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved
	Original Author: D.Clark 
*/
//#include "StdAfx.h"
#include <typeinfo.h>
#include "GameTracking.h"
#include "WT\GameTrack\GameTrack.h"
#include "WT\Application\RuntimeAttribute.h"
#include <time.h>
#include <Mmsystem.h>   // for timeGetTime  (also requires linking in winmm.lib)

namespace GameTracking
{
	static int s_iGamesPlayed = 0,
			   s_iUnLocked = 0;
	static DWORD s_dwStartTimeMs = timeGetTime();
	

void IncrementGamePlayCount( void )
{
	++s_iGamesPlayed;
}

void SetUnLocked( void )
{
	s_iUnLocked = 1;
}

void SendTrackingReport()
{
	WT::Application::CRuntimeAttribute RuntimeAttributes;
	bool const bDoTracking = RuntimeAttributes.GetUseTracking();
	if ( !bDoTracking )
	{
		return;
	}
	std::string const sProductCodeName = RuntimeAttributes.GetProductCodeName();
	
	std::string const sSKU = RuntimeAttributes.GetSKU();

	WT::Tracking::CGameTrack gt;
	gt.Init();
	std::string const sLoggerId = GAMETRACK_LOGGERID_WEBDDRIVER;
	
	DWORD dwTotalMs = timeGetTime() - s_dwStartTimeMs;
	if ( dwTotalMs < 0 )
	{
		dwTotalMs = timeGetTime();
	}
	DWORD const dwTotalSecs = dwTotalMs/1000;
	std::ostringstream o; 
	
	o<<"LC=1,TP="<<dwTotalSecs<<",GP="<<s_iGamesPlayed<<",UL="<<s_iUnLocked;
	
	std::string const sKeyVals = o.str();	
	gt.Log( sLoggerId, sSKU, sProductCodeName, sKeyVals );

	DWORD       const dwMaxCount = 100;
	bool        const bRepeat = false;
	bool        const bStopWhenNoRecords = true;
	DWORD       const dwSleepInterval = 0;
	DWORD       const dwNoRecordsSleepInterval = 60000;
	DWORD		const dwTimeout = 4000;

	bool const bUploadStarted = gt.StartUpload
		(
			dwMaxCount, bRepeat, bStopWhenNoRecords,
			dwSleepInterval, dwNoRecordsSleepInterval, dwTimeout
		);
	if ( bUploadStarted )
	{
		while( gt.IsUploading() )
		{
			DWORD dw = gt.GetLogBytes(); 
			Sleep(50);
		}
	}
	else
	{
	}
}

};
//*************************************************************************************************
