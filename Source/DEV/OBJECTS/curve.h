/*
	curve.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	5/15/2004

*/

#ifndef _CURVE_H
#define _CURVE_H

#include <d3dx8.h>

#include "../UTILITIES/utilities.h"

// a curve is a 2d collection of points defining a series of line
// segments. This is primarily intended to be used for car physics
// for storing torque curves and tire parameters

class CCurve
{

public:
				CCurve( const std::string& Path );	// path to curve file
				~CCurve();

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	float32		GetValue( float32 X );	// given this X Position, we will retrieve the approximate Y value

	float32		MaximumValue( void );	

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void		Multiply( float32 Multiplier );	// factor to multiply curve values by

private:

	void		LoadFromFile( const std::string& Path ); // path to curve file

	uint32		FindPointRight( float32 X );	// x value to find a point to the right of

	std::vector< D3DXVECTOR2 >	m_PointValues;	// actual point values

};

#endif