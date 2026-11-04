//
//	predicates.h
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{

void exactinit();
double orient2d( double* pa, double* pb, double* pc );
double orient3d( double* pa, double* pb, double* pc, double* pd );

//predicates.cpp depends requires double precision (not extended precision) FPU operation
class CPrecisionOverride
{
public:
	CPrecisionOverride()
		: m_Precision( _controlfp( 0, 0 ) )
	{
		_controlfp(_PC_53, MCW_PC); //set FPU control word for double precision
	}

	~CPrecisionOverride()
	{
		_controlfp( m_Precision, MCW_PC ); //Restore FPU control word
	}

private:
	int32 m_Precision;
};

//Check to see if the FPU precision is equivalent to that assigned by a CPrecisionOverride object
inline bool CheckPrecision()
{
	int32 const m_Precision = _controlfp( 0, 0 );
	return (m_Precision & MCW_PC) == _PC_53;
}

inline void ExactInit()
{
	static bool bExactInit = false;
	if (!bExactInit)
	{
		CPrecisionOverride PrecisionOverride;
		exactinit();
		bExactInit = true;
	}
}

}
}
