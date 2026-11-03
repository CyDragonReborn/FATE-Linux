/*
	GameTracking.h

	Game play tracking support class 
	Copyright (C) 2006 WildTangent, Inc. 
	All Rights Reserved
	Original Author: D.Clark 
*/

#ifndef __GameTracking_H
#define __GameTracking_H

namespace GameTracking
{	
	void IncrementGamePlayCount( void );
	void SendTrackingReport( void );
	void SetUnLocked();
};

#endif
