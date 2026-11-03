/******************************************************************************
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved

	History:
	4/27/2006	Rui Jiang	Created

	Description:  License reader interface for game developers.

	To use the libray, first you need to set the default location. This is 
	a unique identifier of the game, and it should have been created by 
	game installer. All subsequent calls will query your default location 
	for values. e.g.:

	LMSetDefaultLocation("{E816AC5C-F1EB-469d-9B01-B8B19D9B2E30}");
	
	if (LMGetDisplayAds())
	{
		...
	}

	You need to link your project with the provided library files:
	LMReaderGameDev.lib and SVSGameDev.lib
	or LMReaderGameDevd.lib and SVSGameDevd.lib for debug configuration.
	Code generation is "Multi-Thread" or "Multi-Thread Debug"

	Note that you must call LMSetDefaultLocation during your program's initialization.
	See below for the reason. 
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

//You need to call this function before calling any other functions.
/*Per our discussion:

1. Protector set the actual locationid of a game in a shared location.
2. When a game starts up , it calls LMSetDefaultLocation with a preset value. 

Inside LMSetDefaultLocation, first it will check if there is already a locationid 
set in shared location. 
	If there is, use that as the default location. 
	Otherwise, use the locationid passed in. 

This way the game can run without a protector, with a "test" locationid 
where we could change value for testing. 
*/
void  __cdecl LMSetDefaultLocation(LPCSTR lpLocationId);

//Should the game display in game ads? 0 if false. nonzero if true.
DWORD __cdecl LMGetDisplayAds();

#ifdef __cplusplus
};
#endif