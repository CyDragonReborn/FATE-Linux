#pragma once

#include "WT/stdafx.h"
#include "WT/System/Platform.h"
#include "WT/System/SystemException.h"

#include <sstream>
#include <algorithm>
#include <windows.h>

namespace WT
{

//Convert float16 to float32.
float32 __fastcall Float16ToFloat32( float16 f16 )
{
	uint32 i = *reinterpret_cast<uint16*>( &f16 ); //Zero-extend

	uint32 const MantissaMask = (1 << 10) - 1;
	uint32 const ExponentMask = ((1 << 5) - 1) << 10;

	int32 bias = i & 0x8000u; //Sign
	i -= bias; //Absolute value

	if (i) //If not zero
	{
		bias <<= 16;

		if (i >= ExponentMask) 
		{
			bias += (255 - 31) << 23; //NAN - force resulting exponent to 255
		} else {
			bias += (127 - 15) << 23; //Exponent bias
		}

		while (i <= MantissaMask)
		{
			//A denormal

			//Plan to remove from the exponent later. Keep shifting until
			//the mantissa overflows and adds one onto the exponent.
			bias -= (1 << 23);
			i += i;
		}

		i <<= 13; //Move the exponent bits into place, and fill extra precision bits with zeroes
		i += bias; //Adjust exponent bias
	}

	return *reinterpret_cast<float32*>( &i );
}

//Convert float32 to float16. Precision will be lost.
float16 __stdcall Float32ToFloat16( float32 f )
{
	uint32 i = *reinterpret_cast<uint32*>( &f );
	
	uint32 const MantissaMask = (1 << 23) - 1;
	uint32 const ExponentMask = ((1 << 8) - 1) << 23;

	uint32 abs = i & 0x7fffffffu;

	//Check for zero, denormal float32, or float32 too small.
	//The smallest positive float16 is 2.0^(-15-9) = 2.0^(-24) which is (127 - 24) << 23
	//in the float32 representation.
	if (abs < (127 - 24) << 23)
	{
		return 0u;
	}

	uint16 const sign = static_cast<uint16>( (i >> 16) & 0x8000u );

	//Check for NaN, excluding infinities
	if (abs > ExponentMask)
	{
		//Destroy excess precision
		abs >>= 13;
		uint16 Result = static_cast<uint16>( abs ) + sign + 0x8000u /*Fix sign*/;

		if (Result == 0xfe00u || //Avoid returning indefinite
			Result == 0x7c00u || //Avoid returning +inf
			Result == 0xfc00u) //Avoid returning -inf
		{
			//Check for indefinite input
			if (i == 0xffc00000u)
			{
				assert( Result == 0xfe00u ); //The result should be indefinite
			} else {
				++Result; //Return a different NaN
			}
		}

		return Result;
	}

	//Check for infinity, or float32 too large.
	//The largest finite float16 is 2.0^(30-15) * (2.0f - 1.0/(1<<10)),
	//which is ((127 + 15) << 23) | (0x3ff << 13) in the float32 representation.
	if (abs > (((127 + 15) << 23) | (0x3ffu << 13)))
	{
		return 0x7c00u + sign;
	}

	//Check for denormal float16
	//The largest denormal float16 is 2.0^(-15) * (2.0f - 1.0/(1<<9)),
	//which is ((127 - 15) << 23) | (0x1ff << 14) in the float32 representation.
	if (abs <= (((127 - 15) << 23) | (0x1ffu << 14)))
	{
		//Prepend a 1 on the mantissa.
		//Then shift mantissa right until exponent becomes large enough
		
		//Add (1 << 23) to prepend a 1 on the mantissa.
		uint32 mant = (abs & MantissaMask) + (1 << 23);
		
		mant >>= (127 - 15 + 13 /*destroy excess precision*/) + ((mant - abs) >> 23);
		return static_cast<uint16>( mant ) + sign;
	}

	//Destroy excess precision
	abs >>= 13;
	return static_cast<uint16>( abs ) + sign + /*Adjust exponent*/ 0x4000u;
}

// Converts the given string to lower-case
std::string WTCall StringLower( std::string const& s )
{
	std::string Result;
	Result.resize( s.size() );
	std::transform( s.begin(), s.end(), Result.begin(), tolower );
	return Result;
}

// Converts the given string to lower-case
std::string WTCall StringUpper( std::string const& s )
{
	std::string Result;
	Result.resize( s.size() );
	std::transform( s.begin(), s.end(), Result.begin(), toupper );
	return Result;
}

wstring WTCall AnsiToWide( const string& AnsiString, StringEncoding::Type encoding)
{
    typedef wstring::value_type wide_type;

	if( AnsiString.length() == 0 )
		return wstring();		// empty string; nothing to convert

    // Ansi->Unicode conversion will take at most 2 bytes per ansi character
    const uint32 MaxLen = static_cast<uint32>(2 * ::strlen( AnsiString.c_str() ));
	
    wide_type* pWideBuffer = new wide_type[MaxLen];

	int nWChar = 0;
	switch( encoding )
	{
	case StringEncoding::DocumentEncoding:
		nWChar = (int)mbstowcs( pWideBuffer, AnsiString.c_str(), MaxLen );
		break;

	case StringEncoding::UTF8:
		nWChar= MultiByteToWideChar( CP_UTF8, 0, AnsiString.c_str(), (int) AnsiString.size(), pWideBuffer, MaxLen); 
		break;
	}
    wstring WideString( pWideBuffer, nWChar );
    delete [] pWideBuffer;
    return WideString;
}

string WTCall WideToAnsi( const wstring& WideString, StringEncoding::Type encoding)
{
    typedef string::value_type ansi_type;

	if( WideString.length() == 0 )
		return string();		// empty string; nothing to convert

    const uint32 ByteLen = static_cast<uint32>(WideString.length()*sizeof(wstring::value_type)*2+sizeof(wstring::value_type));
    ansi_type* pAnsiBuffer = new ansi_type[ByteLen];

 	int bytes = 0;
	switch( encoding )
	{
	case StringEncoding::DocumentEncoding:
		bytes = 
		(int)::wcstombs( pAnsiBuffer, WideString.c_str(), ByteLen ); 
		break;

	case StringEncoding::UTF8:
		bytes = WideCharToMultiByte ( CP_UTF8, 0, WideString.c_str(), (int)WideString.size(), pAnsiBuffer, ByteLen, NULL, NULL );
		break;
	}

	if (bytes == -1)
	{
		// there was an error in attempting to convert to ansi. Probably there was a wide character
		// that could not be converted.
		throw System::CSystemException("WideToAnsi(): Unable to convert one or more wide characters to ansi.");
	}

	string AnsiString( pAnsiBuffer, bytes );
    delete [] pAnsiBuffer;
    return AnsiString;
}

//Escape ", ', &, <, and > to make it a legal string to be used in XML.
// Ref: http://www.w3.org/TR/2004/REC-xml-20040204/
string WTCall EscapeStringForXml(const string& input)
{
	string Result;
	Result.reserve( input.size() );

	int32 const Size = static_cast<int32>( input.size() );
	for (int32 i = 0; i < Size; ++i)
	{
		char const c = input[i];

		switch (c)
		{
		case '\"': Result += "&quot;"; break;
		case '\'': Result += "&apos;"; break;
		case '&' : Result += "&amp;" ; break;
		case '<' : Result += "&lt;"  ; break;
		case '>':  Result += "&gt;"  ; break;
		default:
			Result += c;
		}
	}

	return Result;
}

wstring WTCall EscapeStringForXml(const wstring& input)
{
	wstring Result;
	Result.reserve( input.size() );

	int32 const Size = static_cast<int32>( input.size() );
	for (int32 i = 0; i < Size; ++i)
	{
		wchar_t const c = input[i];

		switch (c)
		{
		case L'\"': Result += L"&quot;"; break;
		case L'\'': Result += L"&apos;"; break;
		case L'&' : Result += L"&amp;" ; break;
		case L'<' : Result += L"&lt;"  ; break;
		case L'>':  Result += L"&gt;"  ; break;
		default:
			Result += c;
		}
	}

	return Result;
}

void WTCall UnEscapeXmlString(string& input)
{
	if (input.find('&') == string::npos) return;
	else
	{
		string res;
		res.reserve(input.size());
		int i;
		for(i = 0; i < (int)input.size(); i++)
		{
			if (input[i] == '&')
			{
				if (i+1 < (int)input.size())
				{
					if (input[i+1] == '#')
					{
						string::size_type pos = input.find(';', i+1);
						if (pos != string::npos)
						{							
							res += (char)atoi(input.c_str() + i + 2);
							i = (int)pos;
						}
						else res += input[i];
					}
					else
					{
						string::size_type pos = input.find(';', i+1);
						if (pos != string::npos && pos - i < 20)
						{
							string s = input.substr(i+1, pos - i - 1);
							char c = ' ';
							bool found = false;
							if (s == "quot")
							{
								c = '\"';
								found = true;
							}
							else if (s == "apos")
							{
								c = '\''; 
								found = true;
							}
							else if (s == "amp")
							{
								c = '&'; 
								found = true;
							}
							else if (s == "lt")
							{
								c = '<'; 
								found = true;
							}
							else if (s == "gt")
							{
								c = '>'; 
								found = true;
							}
							else if (s == "nbsp")
							{
								c = ' '; 
								found = true;
							}
							if (found)
							{
								res += c;
								i = (int)pos;
							}
							else
								res += input[i];
						}
						else res += input[i];
					}
				}
				else
					res += input[i];
			}
			else res += input[i];
		}
		input = res;
	}
}

//convert single \n to \r\n.
string WTCall ConvertToWindowsString(const string& s)
{
	string res;
	for(int i = 0; i < (int)s.size(); i++)
	{
		if (s[i] == '\r')
		{
			if (i+1 < (int)s.size() && s[i+1] == '\n')
			{
				res += "\r\n";
				i++;
			}
			else
				res += "\r\n";
		}
		else if (s[i] == '\n') //No \r?
		{
			res += "\r\n";
		}
		else res += s[i];
	}
	return res;
}

//Compare two strings if they are equal. Case insensative.
bool WTCall strieq(const string& s1, const string& s2)
{
	if (s1.size() != s2.size()) return false;
	for(int i = 0; i < (int)s1.size(); i++)
	{
		if (toupper(s1[i]) != toupper(s2[i]))
		{
			return false;
		}
	}
	return true;
}

string WTCall itostrA(int n)
{
	std::ostringstream o;
	o << n;
	return o.str();
}

wstring WTCall itostrW(int n)
{
	std::wostringstream o;
	o << n;
	return o.str();
}

}
