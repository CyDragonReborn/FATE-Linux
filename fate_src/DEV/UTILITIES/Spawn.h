/*
	spawn.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Pete Siamidis
	8/24/2004

*/

#ifndef SPAWN_H
#define SPAWN_H

#pragma warning ( disable : 4201 4214 4115 )
#include <windows.h>
#pragma warning ( default : 4201 4214 4115 )

#ifdef __cplusplus
	extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////////////
//	Spawn styles
////////////////////////////////////////////////////////////////////////////////////////
typedef enum ESpawnStyle
{
	KSpawnAsync,
	KSpawnSync
};


////////////////////////////////////////////////////////////////////////////////////////
//	Prototypes
////////////////////////////////////////////////////////////////////////////////////////

// spawn a new app
extern HANDLE SpawnApp( char	*LaunchString,		// app to launch
						const char	*Parameters,	// parameters
						ESpawnStyle	Style );	// spawn style

// determines if an app is still active
extern bool SpawnActive( HANDLE	hApp );	// handle to app that we will query


#ifdef __cplusplus
	}
#endif

#endif
