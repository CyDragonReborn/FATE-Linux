/*
	demorules.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Marsh Lefler
	2/16/05

	This sets the rules for the game in Demo mode

*/

#ifndef _DEMORULES_H
#define _DEMORULES_H
#ifdef USES_PAL
#include "Pal.h"
#endif
const enum EDemoStats
{
	KDemoPlayerLevel = 6,
	KDemoDungeonLevel = 3
};

inline bool IsGameUnlocked()
{
#ifdef USES_PAL
	return PAL_IsUnlocked();
#else
	return kTrue;
#endif
}

#endif