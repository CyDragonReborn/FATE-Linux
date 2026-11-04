// GameTrackingSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "WT/System/Platform.h"
#include "WT/System/DateTime.h"
#include "WT/system/IO/FileStream.h"
#include "WT/System/Logging/Log.h"
#include "WT/GameTrack/GameTrack.h"

#include "WT/System/Util/WinUtil.h"

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "shlobj.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\..\\Build\\Debug\\WTPlatform1D.lib")
#else
#pragma comment(lib, "..\\..\\..\\..\\Build\\Release\\WTPlatform1.lib")
#endif

#pragma comment(lib, "wsock32.lib")

using namespace WT;
using namespace System;
using namespace IO;
using namespace Tracking;
using namespace Util;


void Test()
{
	CGameTrack gt;
	gt.Init();
	for(int i = 0; i < 10; i++)
	{
		gt.SetXorChar(0);
		//Webdriver
		gt.Log(GAMETRACK_LOGGERID_WEBDDRIVER, "ED9EB6E2-04A8-432B-B8F9-056AACA7BEAA", "WDTestGame", "LC=1,TP=276,GP=1,UL=1");
		//GameChannel
		gt.SetXorChar(0xB7);
		gt.Log(GAMETRACK_LOGGERID_GAMECHANNEL, "ED9EB6E2-04A8-432B-B8F9-056AACA7BEAA", "GCTestGame", "BC=1,DL=3");	
		//Other.
		gt.Log("{C360A165-B00D-481B-B562-82A301D9B87A}", "ED9EB6E2-04A8-432B-B8F9-056AACA7BEAA", "OTTestGame", "DD=0,CC=1");
	}
	printf("Begin uploading...\n");
	//Sleep(20000);
	if (gt.StartUpload(50, false, true, 0, 0, 0))
	{
		while(gt.IsUploading())
		{
			DWORD dw = gt.GetLogBytes(); 
			printf("%d bytes left\n", dw);
			//if (dw == 0) break;
			Sleep(1000);
			//gt.StopUpload(10000);
		}
	}
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
	char buf[MAX_PATH];

	LOG_SET_WORDPAD(true, "log.doc");
	//AppData first. That's better for Vista?
	if (SHGetSpecialFolderPath(NULL, buf, CSIDL_COMMON_APPDATA, TRUE))
	{
		printf("AppData: %s\n", buf);
	}
	if (SHGetSpecialFolderPath(NULL, buf, CSIDL_PROGRAM_FILES, TRUE))
	{
		printf("ProgramFiles: %s\n", buf);
	}
	if (GetProgramFilesDirFromRegistry(buf, MAX_PATH))
	{
		printf("Got ProgramFiles from registry: %s\n", buf);
	}
	if (GetWindowsDirectory(buf, MAX_PATH))
	{
		printf("Windows: %s\n", buf);
	}


	//_CrtSetBreakAlloc(321);
	_CrtMemState s1, s2, s3;


	LOG("Something");

	_CrtMemCheckpoint( &s1 );	
	
	Test();
	
	_CrtMemCheckpoint( &s2 );
	if ( _CrtMemDifference( &s3, &s1, &s2) )
	{
		LOG("DETECTED LEAKS:\n");
		_CrtMemDumpStatistics( &s3 );
		LOG("MEMORY LEAK DUMP:\n");
		_CrtDumpMemoryLeaks();
	}
	else
	{
		LOG("No Leaks detected\n");
	}


	return 0;
}

