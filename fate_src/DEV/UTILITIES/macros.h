/*
	macros.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/


#ifndef _MACROS_H
#define _MACROS_H

#define DELETE_SAFELY(o)	   { if(o) { delete (o);	 (o)=NULL; } }
#define DELETE_ARRAY_SAFELY(o) { if(o) { delete[] (o);   (o)=NULL; } }
#define RELEASE_SAFELY(o)      { if(o) { (o)->Release(); (o)=NULL; } }


#endif