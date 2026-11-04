#pragma once

#include <cassert>
#include <string>
#include <vector>

/* Calling conventions for WTPlatform. Non-member and static member functions publicly 
exposed in WTPlatform are typically declared with this calling convention. (Non-static
member functions automatically use __thiscall under Microsoft Visual C++; hence 
specification for such functions are rarely needed.*/
#ifndef WTCall
#define WTCall __fastcall
#endif

namespace WT
{

using std::string;
using std::wstring;
using std::vector;

template<typename T>
inline void WTCall SafeDelete(T*& Pointer )
{
	if ( Pointer )
	{
		delete Pointer;
		Pointer = NULL;
	}
}

template<typename T>
inline void WTCall SafeDeleteArray(T*& Pointer )
{
	if ( Pointer )
	{
		delete[] Pointer;
		Pointer = NULL;
	}
}

template<typename T>
inline void WTCall SafeRelease(T*& Pointer )
{
	if ( Pointer )
	{
		Pointer->Release();
		Pointer = NULL;
	}
}

typedef	char				int8;
typedef	short				int16;
typedef	long				int32;
typedef	__int64				int64;
typedef	unsigned char		uint8;
typedef	unsigned short		uint16;
typedef	unsigned long		uint32;
typedef unsigned __int64    uint64;
typedef	float				float32;
typedef	double				float64;

//Partial precision float, a.k.a. fp16 or s10e5. These floating-point
//representations are often used in image data and in intermediate rendering
//calculations. float16 is a compact floating-point storange mechanism,
//and should not be used directly in arithmetic operations (even == is dangerous
//because of negative zero). The unusual unsigned __int16 type allows for templates 
//to be specialized for float16.
typedef unsigned __int16 float16;

//
//Type conversion functions.
//

//Fast conversion from a 32-bit floating point number to a 32-bit signed integer.
//The input is rounded to the nearest integer (although this behavior may
//be modified by altering the FPU control word).
inline int32 __stdcall Float32ToInt32( float32 f );

//Fast conversion from a 64-bit floating point number to a 64-bit signed integer.
//The input is rounded to the nearest integer (although this behavior may
//be modified by altering the FPU control word).
inline int64 WTCall Float64ToInt64( float64 f );

//Convert from float16 to float32.
float32 __fastcall Float16ToFloat32( float16 f16 );

//Convert from float32 to float16. Precision will be lost.
float16 __stdcall Float32ToFloat16( float32 f );

//
//Metaprogramming aids
//

struct TrueType
{
};

struct FalseType
{
};

class NoType
{
};

//CCanCastPointer<X,Y>::Result is TrueType if X is a Y-compatible type - that is, if an X*
//can be cast to a Y*. If not, Result is FalseType.
template <typename X, typename Y> struct CCanCastPointer
{
private:
	//Make sure X and Y are defined. If a compile error occurs, then there was probably an attempt 
	//to check CCanCastPointer<X,Y>::Result before X or Y was fully defined. The solution is to
	//define a specialization such as template<> struct CCanCastPointer<X,Y> {typedef TrueType Result;}; 
	//in the WT namespace using the appropriate result type (TrueType or FalseType).
	static int32 const SizeX = sizeof X;
	static int32 const SizeY = sizeof Y;

	//These functions help differentiate whether X* can be cast to a Y* or not.
	static uint8 foo( void const volatile * );
	static uint16 foo( Y* );

	template<int Size> struct SizeToType;
	template<> struct SizeToType<sizeof( uint8  )> {typedef FalseType Result;};
	template<> struct SizeToType<sizeof( uint16 )> {typedef TrueType  Result;};

public:
	typedef typename SizeToType<sizeof foo( static_cast<X*>( NULL ) )>::Result Result;
};

//
// Implementations
//

inline int32 __stdcall Float32ToInt32( float32 f )
{
	int32 Result;

	__asm
	{
		fld f
		fistp Result
	}

	return Result;
}

inline int64 WTCall Float64ToInt64( float64 f )
{
	int64 Result;

	__asm
	{
		fld f
		fistp Result
	}

	return Result;
}

//Convert float16 to float32.
float32 __fastcall Float16ToFloat32( float16 f16 );

//Convert float32 to float16. Precision will be lost.
float16 __stdcall Float32ToFloat16( float32 f );

// string helper functions
int8 const * const SPACES = " \t\r\n";

inline string WTCall TrimRight (const string & s, const string & t = SPACES)
{ 
	string d (s); 
	string::size_type i (d.find_last_not_of (t));
	if (i == string::npos)
	{
		return "";
	}
	else
	{
	return d.erase (d.find_last_not_of (t) + 1) ; 
	}
}

inline string WTCall TrimLeft (const string & s, const string & t = SPACES) 
{ 
	string d (s); 
	return d.erase (0, s.find_first_not_of (t)) ; 
}

inline string WTCall Trim (const string & s, const string & t = SPACES)
{ 
	string d (s); 
	return TrimLeft (TrimRight (d, t), t) ; 
}

// Converts the given string to lower-case
std::string WTCall StringLower( std::string const& s );

// Converts the given string to lower-case
std::string WTCall StringUpper( std::string const& s );

wchar_t const * const WSPACES = L" \t\r\n";

inline wstring WTCall TrimRight (const wstring & s, const wstring & t = WSPACES)
{ 
	wstring d (s); 
	wstring::size_type i (d.find_last_not_of (t));
	if (i == wstring::npos)
	{
		return L"";
	}
	else
	{
	return d.erase (d.find_last_not_of (t) + 1) ; 
	}
}

inline wstring WTCall TrimLeft (const wstring & s, const wstring & t = WSPACES) 
{ 
	wstring d (s); 
	return d.erase (0, s.find_first_not_of (t)) ; 
}

inline wstring WTCall Trim (const wstring & s, const wstring & t = WSPACES)
{ 
	wstring d (s); 
	return TrimLeft (TrimRight (d, t), t) ; 
}


namespace StringEncoding
{
	enum Type
	{
		DocumentEncoding = 0,
		UTF8,
		UTF16
	};
}

wstring WTCall AnsiToWide( const string& AnsiString, StringEncoding::Type encoding = StringEncoding::DocumentEncoding);
string WTCall WideToAnsi( const wstring& WideString, StringEncoding::Type encoding = StringEncoding::DocumentEncoding );

//helper template function to find if an element is contained within an stl container
template<typename C,typename T>
bool WTCall ContainerContains ( const C& Array , const T& Element )
{
	return find ( Array.begin() , Array.end() , Element ) != Array.end();
}

//Split string by small sub strings. e.g.: "How*_*Are*_*You?" split by "*_*" would be: 
// "How", "Are" and "You".
template <class _String, class _AggClass> int WTCall SplitByString(const _String& s, const _String& seq, _AggClass& v)
{
	assert(seq.size());
	_String::size_type start = 0;
	_String::size_type pos = 0;
	while(true)
	{
		pos = s.find(seq, start);
		if (pos == _String::npos)
		{
			if (s.size() > start)
			{
				v.push_back(s.substr(start, s.size() - start));
			}
			break;
		}
		else
		{
			if (pos > start)
				v.push_back(s.substr(start, pos - start));
			start = pos + seq.size();
		}
	}
	return v.size();
}

//Split string as symbols. One symbol consists of only [A-Z][a-z][0-9][_]. 
// e.g.: "Hello, World9! 987" would be: "Hello", ", ", "World9", "! ", "987"
template <class _String, class _AggClass> int WTCall SplitBySymbol(const _String& s, _AggClass& v)
{
	int symbolStart = -1;
	int nonSymbolStart = -1;
	for(int i = 0; i < (int)s.size(); i++)
	{
		char c = s[i];
		//Is this one of symbol char?
		if (isalpha(c) || isdigit(c) || c == '_')
		{			
			if (symbolStart < 0)
			{
				//End nonsymbol
				if (nonSymbolStart >= 0)
				{
					v.push_back(s.substr(nonSymbolStart, i - nonSymbolStart));
					nonSymbolStart = -1;
				}
				symbolStart = i;
			}
			else
			{
				//just continue.
			}
		}
		else
		{
			if (nonSymbolStart < 0)
			{
				if (symbolStart >= 0)
				{
					v.push_back(s.substr(symbolStart, i - symbolStart));
					symbolStart = -1;
				}
				nonSymbolStart = i;
			}
			else
			{
				//Just continue.
			}
		}
	}
	if (symbolStart >= 0)
		v.push_back(s.substr(symbolStart, s.size() - symbolStart + 1));
	if (nonSymbolStart >= 0)
		v.push_back(s.substr(nonSymbolStart, s.size() - nonSymbolStart + 1));
	return (int)v.size();
}

//Split string by specific characters. e.g.: "How*_&&Are??**^You?" split by "*_&^" would be: 
// "How", "Are" and "You".
template <class _String, class _AggClass> int WTCall SplitByChars(const _String& str, const _String& delim, _AggClass& v)
{
	_String::value_type* buf, *p;

	buf = new _String::value_type[str.size() + 1]; 
    strcpy(buf, str.c_str());
    p = strtok(buf, delim.c_str());
    while(p != NULL) 
    { 
        v.push_back(p); 
        p = strtok(NULL, delim.c_str()); 
    } 
    delete [] buf;
	return (int)v.size();
}

//Split string by on char with escape. e.g.: "A&B&&C&D" split by "&" would be: 
// "A", "B&C", "D"
template <class _String, class _AggClass> int WTCall SplitByCharWithEscape(const _String& str, int c, _AggClass& v)
{
	_String::size_type start = 0;
	_String tmp;
	for(_String::size_type pos = 0; pos < str.size(); pos++)
	{
		if (str[pos] == c)
		{
			if (pos + 1 == str.size() || str[pos+1] != c )
			{
				//Not empty string.
				if (tmp.size())
				{
					v.push_back(tmp);
					tmp.clear();
				}
			}
			else
			{
				//Skip this two char.
				tmp += '&';
				pos ++;
			}
		}
		else
		{
			tmp += str[pos];
		}
	}
	if (tmp.size())
	{
		v.push_back(tmp);
	}
	return (int)v.size();
}

//Split string by sequential characters. e.g.: "/AppConfig:Name=How:Are=You" split by ":=" would be: 
// "/AppConfig", "Name" and "How:Are=You".
template <class _String, class _AggClass> int WTCall SplitBySeqChars(const _String& str, const _String& delim, _AggClass& v)
{
	if(delim.size() == 0)
	{
		v.push_back(str);
	}
	else
	{
		_String::size_type start = 0;
		_String::size_type pos = 0;
		for(int delimPos = 0; delimPos < (int)delim.size(); delimPos ++)
		{
			pos = str.find(delim[delimPos], start);
			if (pos == _String::npos)
			{
				break;
			}
			else
			{
				if (pos > start)
					v.push_back(str.substr(start, pos - start));
				start = pos + 1;
			}
		}
		if (str.size() > start)
		{
			v.push_back(str.substr(start, str.size() - start));
		}
	}
	return (int)v.size();
}

//Seperate the string to 2 parts by a given char.
template<class _String> bool WTCall SplitStringByCharTwoParts(const _String & str, _String & left, _String & right, int c, bool reverse = false)
{
	_String::size_type pos = reverse? str.rfind(c) : str.find(c);
	if (pos != string::npos)
	{
		left = str.substr(0, pos);
		right = str.substr(pos+1, str.size() - pos - 1);
		return true;
	}
	else
	{
		left = str;
		right = _String();
		return false;
	}
}

//Extract string between two charactors, like: "....<ABDCDEE>"
template<class _String> _String WTCall ExtractStringBetween(const _String& str, int first, int second)
{
	_String::size_type pos1 = str.find((_String::value_type)first);
	if (pos1 != string::npos)
	{
		_String::size_type pos2 = str.find((_String::value_type)second, pos1+1);
		if (pos2 != string::npos) return str.substr(pos1 + 1, pos2 - pos1 - 1);
	}
	return _String();
}

//Separate command arguments by spaces, strip double quote. e.g.:
// -source:"c:\Program Files\test" -archive   ---> two strings: -source:c:\Program Files\test,  -archive
template<class _String> std::vector<_String> WTCall ParseCommandLine(const _String& cmd)
{
	//Separate cmdline by space.
	bool started = true, inquote = false;
	//args1 is those before :. args2 is after.
	std::vector<_String> args;
	string last;
	for(int i = 0; i < (int)cmd.size(); i++)
	{
		if (isspace(cmd[i]))
		{
			if (started)
			{
				if (inquote)
					last += cmd[i];
				else
				{
					started = false;
					if (last.size()) { args.push_back(last); last = ""; }
				}
			}
		}
		else
		{
			if (cmd[i] == '\"')
			{
				if (started)
					inquote = !inquote;
				else
				{
					started = true;
					inquote = true;
				}
			}
			else
			{
				if (!started)
					started = true;
				last += cmd[i];
			}
		}
	}

	if (last.size()) args.push_back(last);
	return args;
}

//Extract the parent dir name.
template<class _String> _String WTCall GetParentDir(const _String& s)
{
	int i;
	for(i = (int)s.size() - 1; i >= 0; i--)
	{
		int c = s[i];
		if (c == '/' || c == '\\')
		{
			//in case \\computer1
			if (i > 0 && (s[i-1] == '/' || s[i-1] =='\\'))
				i--;
			return s.substr(0, i);
		}
	}

	return "";
};

//T is usually a string, or wstring.
template<class T> T WTCall ConcatPath(const T& ParentDir, const T& FileName)
{
	T res = ParentDir;
	if (res.size() > 0 && res[res.size() - 1] != '\\' && res[res.size() - 1] != '/')
		res += "\\";
	res += FileName;
	return res;
}


//Escape ", ', &, <, and > to make it a legal string to be used in XML.
// Ref: http://www.w3.org/TR/2004/REC-xml-20040204/
string WTCall EscapeStringForXml(const string& input);
wstring WTCall EscapeStringForXml(const wstring& input);
void WTCall UnEscapeXmlString(string& input);

//convert single \n to \r\n.
string WTCall ConvertToWindowsString(const string& s);

//Compare two strings if they are equal. Case insensative.
bool WTCall strieq(const string& s1, const string& s2);

string WTCall itostrA(int n);
wstring WTCall itostrW(int n);

#ifdef UNICODE
#define itostr itostrW
#else
#define itostr itostrA
#endif


}
