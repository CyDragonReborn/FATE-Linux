/*
	main.cpp

	Copyright (C) 2004 WildTangent, Inc.
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

//TODO: [BLG] IF (10/18/2013) Fix this, only set for development and testing purposes
#ifndef FATE_NO_STEAM
#define STEAM_BUILD
#endif


#define STRICT
#include <windows.h>
#include <string>

using namespace std;

#include "UTILITIES/constants.h"
#include "GAME/game.h"
#include "GAMECLIENT/demorules.h"
#include "RESOURCES/resources.h"

#include "UTILITIES/loadlocalizedstring.h"

// winmain - here we create our app and start it running
INT WINAPI WinMain( HINSTANCE hInst,
					HINSTANCE,
					LPSTR pCmdLine,
					INT )
{
//marsh added this for the PAL integration
	char AppTitle[] = "FATE";
	std::string ContentTitle = "FATE";
	std::string Params = pCmdLine;
	
#ifdef USES_PAL
		PAL_Init( ContentTitle.c_str(), NULL, "4B39DF83-1063-4fcc-B1B4-0E116120D387", pCmdLine, GetLocalPath().c_str());
#endif

	// create a 'game' object
	CGame* pGame = new CGame( Params );

	if(!pGame->InitializeSteam())
	{
/*		MessageBox( NULL,
					"Steam Initialization Failed",
					"Steam Error",
					MB_ICONERROR | MB_OK );*/
		return 0;
	}

	// create the game - this means creating all the d3d device, etc.
	if ( FAILED( pGame->Create( hInst ) ) )
	{
/*		MessageBox( NULL,
					"FAILED GAME CREATION",
					"Load Error",
					MB_ICONERROR | MB_OK );*/
		return 0;
	}

	std::string Flavor = "";



#ifdef USES_PAL
	Flavor = PAL_GetFlavor();
	Flavor = StringUpper( Flavor );

/*	MessageBox( NULL, 
				Flavor.c_str(), 
				"DETECTION ERROR", 
				MB_ICONERROR|MB_OK );*/

		if( !IsGameUnlocked() )
		{
			if(    ( (Flavor.length() >= 8 ) && (Flavor.substr( 0, 8  ) == "WIRE-DRM"   ) )
				|| ( (Flavor.length() >= 11) && (Flavor.substr( 0, 11 ) == "DEVELOPMENT") )
			  )
			{
				ShowWindow( pGame->GetHWND(), SW_HIDE );
				PAL_DisplayShell( kGameStartSession, NULL, AppTitle, LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIN_ICON ) ) );
				ShowWindow( pGame->GetHWND(), SW_SHOW );
			}

			if( IsGameUnlocked() )
			{
				pGame->SetFullscreenAfterPal();
			}
		}
#endif
	if( !IsGameUnlocked() )
	{
		pGame->SetFullscreenAfterPal();
	}
	int32 Result = pGame->Run();

if( !IsGameUnlocked() )
{

#ifdef USES_PAL //marsh

	if(    ( (Flavor.length() >= 8 ) && (Flavor.substr( 0, 8  ) == "WIRE-DRM"   ) )
		|| ( (Flavor.length() >= 11) && (Flavor.substr( 0, 11 ) == "DEVELOPMENT") )
	  )
	{
		ShowWindow( pGame->GetHWND(), SW_HIDE );
		PAL_DisplayShell( kGameEndSession, NULL, AppTitle, LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIN_ICON ) ) );
	}
	PAL_Done();

#endif
}


	DELETE_SAFELY( pGame );

	return Result;
} // WinMain()


