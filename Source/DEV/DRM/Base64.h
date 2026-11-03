/*
	base64.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/24/2004

*/

#ifndef BASE64_H
#define BASE64_H

#include <string>
using namespace std;
#include "..\UTILITIES\constants.h"


class Base64
{
public:
    static std::string Encode( char* Data,		// char array of data to encode
							   uint32 Length );	// length of char array

    static std::string Decode( const std::string& Data );	// string to decode

	static void Base64Encode( const char* cpInput,			// input char array
							  char* cpOutput );				// output char array
};

#endif
