/*
	constants.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/
#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#pragma warning(disable: 4786) 

#define 	kTrue   			1 
#define 	kFalse  			0

#ifndef NULL
	#define NULL    0
#endif

typedef 	char			int8; 
typedef 	short			int16; 
typedef 	long			int32; 
typedef 	unsigned char	uint8; 
typedef 	unsigned short	uint16; 
typedef 	unsigned long	uint32; 
typedef 	float			float32; 
typedef 	double			float64; 
//typedef     bool  			  boolean; 
typedef 	size_t			PtrSz; 

#endif