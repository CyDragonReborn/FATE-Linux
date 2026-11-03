/*
	curve.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	5/15/2004

*/

#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3d.h"
#include "../UTILITIES/textfile.h"

#include "curve.h"

CCurve::CCurve( const std::string& Path ) 	// path to curve file
{

	LoadFromFile( Path );

} // CCurve::CCurve()

CCurve::~CCurve()
{
} // CCurve::~CCurve()

// parse a curve file and extract the points
void CCurve::LoadFromFile( const std::string& Path ) // path to curve file
{
	CTextFile	File;
	File.Open( Path );
	std::queue<std::string>	Tokens;

	while( !File.IsEOF() )
	{
		File.TokenizeNextLine( &Tokens, " =", '#' );	

		if( Tokens.size() == 0 )
		{
			ClearQueue( Tokens );
			continue;
		}

		if( File.IsEOF() )
		{
			break;
		}

		std::string Token = Tokens.front();

		// if we've encountered a point -
		if( Tokens.size() == 1 && 
			Token.length() >= 5 &&
			Token.substr( 0, 5 ) == "point" )
		{
			ClearQueue( Tokens );

			// first line is an open bracket
			File.TokenizeNextLine( &Tokens );	
			ClearQueue( Tokens );

			// next should be the x component - we need to delimit = signs
			File.TokenizeNextLine( &Tokens, " =", '#' );	
			// first token will be 'x'
			Tokens.pop();
			// next token should be the value
			float32 X = (float32)atof( Tokens.front().c_str() );
			ClearQueue( Tokens );

			// next should be the y component - we need to delimit = signs
			File.TokenizeNextLine( &Tokens, " =", '#' );	
			// first token will be 'y'
			Tokens.pop();
			// next token should be the value
			float32 Y = (float32)atof( Tokens.front().c_str() );

			m_PointValues.push_back( D3DXVECTOR2( X, Y ) );
		}


		ClearQueue( Tokens );
	}
	File.Close();

} // CCurve::LoadFromFile()

// fint the point closest on the right side of our requested X value
// returns m_PointValues.size() if it is beyond the right-hand extents, and 0 if it
// is below the left-hand
uint32 CCurve::FindPointRight( float32 X ) // x value to find a point on the right of
{
	for ( uint32 i = 0; i < m_PointValues.size(); i++ )
	{
		if ( m_PointValues[i].x > X )
		{
			break;
		}
	}
	return i;
} // CCurve::FindPointRight()

float32 CCurve::GetValue( float32 X ) // given this X Position, we will retrieve the approximate Y value
{
	assert( m_PointValues.size() > 0 );

	// Find a point to the right of our X value
	uint32	PointRight( FindPointRight( X ) );

	// if we're to the left of the first point on the graph
	if ( PointRight == 0 )
	{
		// we use the Y value of the first point
		return m_PointValues[0].y;
	}
	else if ( PointRight == m_PointValues.size() )	// or if we're outside the bounds to the right
	{
		// we use the last point on the graph
		return m_PointValues[m_PointValues.size() - 1].y;
	}

	// Interpolate between 'rp' and previous point
	float32 DeltaX( m_PointValues[PointRight].x - 
					m_PointValues[PointRight - 1].x );

	float32 DeltaY( m_PointValues[PointRight].y - 
					m_PointValues[PointRight - 1].y );

	return ( X - m_PointValues[PointRight - 1].x ) / 
		   DeltaX * DeltaY + m_PointValues[PointRight - 1].y;
} // CCurve::GetValue()

void CCurve::Multiply( float32 Multiplier )	// factor to multiply curve values by
{
	for ( uint32 i = 0; i < m_PointValues.size(); i++ )
	{
		m_PointValues[i].y *= Multiplier;
	}
} // CCurve::Multiply()

float32 CCurve::MaximumValue( void )	
{
	float32 Maximum( 0 );
	for ( uint32 i = 0; i < m_PointValues.size(); i++ )
	{
		if( m_PointValues[i].y > Maximum )
		{
			Maximum = m_PointValues[i].y;
		}
	}
	return Maximum;
} // CCurve::MaximumValue()
