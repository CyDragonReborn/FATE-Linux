/*
	main.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

#define STRICT
#include <windows.h>
#include <string>


using namespace std;

#include "UTILITIES/constants.h"
#include "GAME/game.h"
char *appTitle = "Snowboarding";

#include "pal.h"

// winmain - here we create our app and start it running
INT WINAPI WinMain( HINSTANCE hInst,
					HINSTANCE, 
					LPSTR pCmdLine, 
					INT )
{
	CGame* pGame = new CGame();

	pGame->SetWindowTitle( appTitle );

	if ( FAILED( pGame->Create( hInst ) ) )
	{
		return 0;
	}

#ifdef USES_PAL
	ShowWindow(pGame->GetWindowHandle(), SW_HIDE);
	PAL_Init(pGame->GetContentName().c_str(), pGame->GetContentId().c_str(), pGame->GetLicenseId().c_str(), pCmdLine, GetLocalPath().c_str());
	PAL_DisplayShell(kGameStartSession, NULL, appTitle, NULL);
	ShowWindow(pGame->GetWindowHandle(), SW_SHOW);
#endif

	int32 Result = pGame->Run();

#ifdef USES_PAL
	ShowWindow(pGame->GetWindowHandle(), SW_HIDE);
	PAL_DisplayShell(kGameEndSession, NULL, appTitle, NULL);
	PAL_Done();
#endif

	DELETE_SAFELY( pGame );
	return Result;
} // WinMain()


