//
//	MathUtil.h
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#pragma inline_recursion( on )
#pragma inline_depth( 16 )

#pragma warning(disable:301)
#pragma warning(disable:38)

#include <math.h>
#include <stdlib.h>
#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{


//
// Enum's/Const's
//

const float32 KPi				= 3.141592654f;
const float32 KTwoPi			= 6.283185307f;
const float32 KInvPi			= 0.318309886f;
const float32 KE				= 2.718281828f;
const float32 KSqrt2Pi			= 2.506628274631000502415765284811045253006f;
const float32 KOneOverSqrt2Pi	= 1.0f / KSqrt2Pi;

const float32 KMaxFloat32		= 3.40282e+038f;
const float32 KMinFloat32		= -3.40282e+038f;

const float32 KRadFromDegree	= KPi / 180.0f;
const float32 KDegreeFromRad	= 180.0f / KPi;


//
// Functions
//

__forceinline float64 __fastcall Math_Mod( float64 a, float64 b )
{
	return fmod(a,b);
}

__forceinline float32 __fastcall Math_Mod( float32 a, float32 b )
{
	return fmodf(a,b);
}

__forceinline int32 __fastcall Math_Mod( int32 a, int32 b )
{
	return a%b;
}

// random between a and b
template< class T >
__forceinline T	__fastcall Math_Rand( T MinRange, T MaxRange )
{
	return MinRange + static_cast<T>(rand() * (1.0f / RAND_MAX) * (MaxRange - MinRange));
}

__forceinline float32 __fastcall Math_Randf( float32 MinRange, float32 MaxRange )
{
	return MinRange + ( ((float32)rand())/((float32)RAND_MAX) * (MaxRange-MinRange));
}

// random a or b
template< class T >
__forceinline T	__fastcall Math_RandBinary( T a, T b )
{
	return (rand()&0x1)?a:b;
}

//a random between 0.0 and 1.0
__forceinline float32 __fastcall Math_Randf()
{
	return (((float32) rand())/((float32) RAND_MAX));
}

//TODO fix/delete this function, it may result in logf( 0 ) crash!
// Return a random number with a normal distribution.
__forceinline float32 NRand ( float Sigma = 1.0f )
{	
	const float32 Math_OneOverSigmaExp	= 1.0f / 0.7975f;

	if ( Sigma == 0 )
		return 0;
	
	float32 y;
	do
	{
		y = -logf ( Math_Randf() );
	} while ( Math_Randf() > expf(-(y - 1.0f) * (y - 1.0f) * 0.5f));
	
	if ( rand() & 0x1 )
		return y * Sigma * Math_OneOverSigmaExp;
	else
		return -y * Sigma * Math_OneOverSigmaExp;
}

//Use to seed the random number generator
#define Math_SRand48(x) srand(x)

template<class T>
__forceinline T __fastcall Math_Clamp( T val, T minval, T maxval )
{
	if( val < minval ) return minval;
	else if(val > maxval) return maxval;
	else return val;
}

//TODO: fix this - this function returns the wrong answer for Val = 0x80000000
//Consider taking uint32.
__forceinline bool __fastcall Math_IsPow2( int32 Val )
{
	return (Val&(Val-1)) == 0;
}

__forceinline float32 __fastcall Math_DegToRad( float32 Deg )
{
	return Deg * KRadFromDegree;
}

__forceinline float32 __fastcall Math_RadToDeg( float32 Rad )
{
	return Rad * KDegreeFromRad;
}

__forceinline float32 __fastcall Math_Fabs(float32 val)
{
	return fabsf(val);
}

__forceinline float32 __fastcall Math_Sin( float32 Rads )
{
	return sinf( Rads );
}

__forceinline float32 __fastcall Math_Cos( float32 Rads )
{
	return cosf( Rads );
}

__forceinline float32 __fastcall Math_Tan( float32 Rads )
{
	return tanf( Rads );
}

__forceinline float32 __fastcall Math_ArcSin( float32 Val )
{
	return asinf( Val );
}

__forceinline float32 __fastcall Math_ArcCos( float32 Val )
{
	return acosf( Val );
}

__forceinline float32 __fastcall Math_ArcTan( float32 Val )
{
	return atanf( Val );
}

__forceinline float32 __fastcall Math_Sinh( float32 Rads )
{
	return sinhf( Rads );
}

__forceinline float32 __fastcall Math_Cosh( float32 Rads )
{
	return coshf( Rads );
}

__forceinline float32 __fastcall Math_Tanh( float32 Rads )
{
	return tanhf( Rads );
}

__forceinline float32 __fastcall Math_Ceil( float32 Val )
{
	return ceilf( Val );
}

__forceinline float32 __fastcall Math_Floor( float32 Val )
{
	return floorf( Val );
}

__forceinline int32 __fastcall Math_Float32ToInt32( float32 val )
{
	return Float32ToInt32 ( val );
}

__forceinline float32 __fastcall Math_Sqrt(float32 val)
{
	return sqrtf(val);
}

__forceinline float32 __fastcall Math_Exp(float32 val)
{
	return expf(val);
}

__forceinline float32 __fastcall Math_Log(float32 val)
{
	return logf(val);
}

__forceinline float32 __fastcall Math_Log10(float32 val)
{
	return log10f(val);
}

__forceinline float32 __fastcall Math_Pow(float32 x, float32 y)
{
	return powf(x,y);
}

__forceinline float32 __fastcall Math_Hypot(float32 x, float32 y)
{
	return sqrtf(x * x + y * y);	//hypotf(x, y);
}

//Compute the cube root of x
__forceinline float32 __fastcall Math_cubert( float32 x )
{
	float32 Result = powf( fabsf( x ), 1.0f / 3.0f );
	if (x < 0.0f)
	{
		Result = -Result;
	}
	return Result;
}

} // namespace Math
} // namespace WT
