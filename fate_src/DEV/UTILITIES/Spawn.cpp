/*
	spawn.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Pete Siamidis
	8/24/2004

*/

#pragma warning ( disable : 4514 )
#include <string.h>
#include <assert.h>
#include "../UTILITIES/constants.h"
#include "Spawn.h"


HANDLE SpawnApp( char	*LaunchString,			// app to launch
				 const char	*Parameters,	// parameters
				 ESpawnStyle	Style )			// spawn style
{

	// locals
	SHELLEXECUTEINFO	ShellInfo;

	// init ShellInfo struct
	memset( &ShellInfo, 0, sizeof( SHELLEXECUTEINFO ) );
	ShellInfo.cbSize = sizeof( SHELLEXECUTEINFO );
	ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShellInfo.nShow = SW_SHOWNORMAL;
	ShellInfo.lpFile = LaunchString;
	ShellInfo.lpParameters = Parameters;

	// launch new process
	if ( ShellExecuteEx( &ShellInfo ) == 0 )
	{
		return NULL;
	}

	// if the launch string is not an exe file, then ShellInfo.hProcess may be null,
	// so we need to get a handle to the app that was loaded
	if ( ShellInfo.hProcess == NULL )
	{

		// get app handle from hinstance
		//undone how do i do this?

	}

	// if its a synchronous spawn, then wait for the app to finish then null its handle
	if ( Style == KSpawnSync )
	{
		while ( SpawnActive( ShellInfo.hProcess ) );
		ShellInfo.hProcess = NULL;
	}
	::ShowWindow( ShellInfo.hwnd, SW_SHOWNORMAL );
	::SetForegroundWindow( ShellInfo.hwnd );
	::SetFocus( ShellInfo.hwnd );


	// return spawed app handle process
	return ShellInfo.hProcess;

} // SpawnApp()



////////////////////////////////////////////////////////////////////////////////////////
//
//	SpawnActive()
//
//	Determines if an app is still active.
//
//	Returns
//		kTrue
//			App is still active.
//		kFalse
//			App is not active.
//
////////////////////////////////////////////////////////////////////////////////////////
bool SpawnActive( HANDLE	hApp )	// handle to app that we will query
{

	// locals
	BOOL	AppQuery;
	DWORD	Status = 0;

	// fail if we have a bad handle
	if ( hApp == NULL )
	{
		assert( 0 );
		return kFalse;
	}

	// get app status
	AppQuery = GetExitCodeProcess( hApp, &Status );
	if ( AppQuery == kFalse )
	{
		return kFalse;
	}

	// return app status
	if ( Status == STILL_ACTIVE )
	{
		return kTrue;
	}
	return kFalse;

} // Spawn_IsActive()
