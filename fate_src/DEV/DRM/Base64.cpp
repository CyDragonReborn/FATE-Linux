/*
	base64.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/24/2004

*/

#include "..\UTILITIES\constants.h"
#include "Base64.h"

const char				fillchar = '=';

static std::string       cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefghijklmnopqrstuvwxyz"
                          "0123456789+/";

std::string Base64::Encode( char* Data,		// char array of Data to encode
							uint32 Length )	// length of char array
{
    auto     std::string::size_type  i;
    auto     char               c;

	auto     std::string             ret;

    for( int32 i = 0; i < Length; ++i )
    {
        c = (Data[i] >> 2) & 0x3f;
        ret.append(1, cvt[c]);
        c = (Data[i] << 4) & 0x3f;
        if (++i < Length)
            c |= (Data[i] >> 4) & 0x0f;

        ret.append(1, cvt[c]);
        if (i < Length)
        {
            c = (Data[i] << 2) & 0x3f;
            if (++i < Length)
                c |= (Data[i] >> 6) & 0x03;

            ret.append(1, cvt[c]);
        }
        else
        {
            ++i;
            ret.append(1, fillchar);
        }

        if (i < Length)
        {
            c = Data[i] & 0x3f;
            ret.append(1, cvt[c]);
        }
        else
        {
            ret.append(1, fillchar);
        }
    }

    return(ret);
}

std::string Base64::Decode( const std::string& Data )	// string to decode
{
    auto     std::string::size_type  i;
    auto     char               c;
    auto     char               c1;
    auto     std::string::size_type  Length = Data.length();
    auto     std::string             ret;

    for( int32 i = 0; i < Length; ++i)
    {
        c = (char) cvt.find(Data[i]);
        ++i;
        c1 = (char) cvt.find(Data[i]);
        c = (c << 2) | ((c1 >> 4) & 0x3);
        ret.append(1, c);
        if (++i < Length)
        {
            c = Data[i];
            if (fillchar == c)
                break;

            c = (char) cvt.find(c);
            c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
            ret.append(1, c1);
        }

        if (++i < Length)
        {
            c1 = Data[i];
            if (fillchar == c1)
                break;

            c1 = (char) cvt.find(c1);
            c = ((c << 6) & 0xc0) | c1;
            ret.append(1, c);
        }
    }

    return(ret);
}


static char* _cpBase64Encoding = 
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";


void  Base64::Base64Encode( const char* cpInput,			// input char array
							char* cpOutput )				// output char array
{
 int nIdx[ 4 ];  // will contain the indices of coded letters in 
                  // _cpBase64Encoding std::string; valid values [0..64]; the value
                 // of 64 has special meaning - the padding symbol

 // process the Data (3 bytes of input provide 4 bytes of output)
 while ( '\0' != *cpInput )
 {
   nIdx[0] = ((*cpInput) & 0xFC)>>2;
   nIdx[1] = ((*cpInput) & 0x03)<<4;
   cpInput++;
   if ( '\0' != *cpInput )
   {
     nIdx[1] |= ((*cpInput) & 0xF0)>>4;
     nIdx[2]  = ((*cpInput) & 0x0F)<<2;
     cpInput++;
     if ( '\0' != (*cpInput) )
     {
       nIdx[2] |= ((*cpInput) & 0xC0) >> 6;
       nIdx[3]  = (*cpInput) & 0x3F;
       cpInput++;
     }
     else
       nIdx[3] = 64;
   }
   else
   { // refer to padding symbol '='
     nIdx[2] = 64;
     nIdx[3] = 64;
   }

   *(cpOutput+0) = *(_cpBase64Encoding + nIdx[0]);
   *(cpOutput+1) = *(_cpBase64Encoding + nIdx[1]);
   *(cpOutput+2) = *(_cpBase64Encoding + nIdx[2]);
   *(cpOutput+3) = *(_cpBase64Encoding + nIdx[3]);
   cpOutput += 4;
 }
 
 // set this to terminate output std::string
 *cpOutput = '\0';

 return;
}