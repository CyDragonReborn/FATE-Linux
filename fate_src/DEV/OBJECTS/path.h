
#include "d3dx8_shim.h"
#include <d3d8.h>
#include <d3d8types.h>
#include "d3dx8.h"
/*
	path.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/


#ifndef _PATH_H
#define _PATH_H


#include <d3dx8.h>

#include "../UTILITIES/utilities.h"

// a path is a spline object, which may be open or closed
// used for object guide paths.
// scene or collision objects may contain paths that were exported from MAX

class CPath
{
public:

						CPath( std::string PathName,				// name of path
							   bool PathClosed,						// is this a closed spline?
							   const D3DXVECTOR3& Position );		// worldspace location

						CPath( CPath& Path );						// path to copy from

						~CPath( void );
	
	void				CalculatePathWidth( CPath& LeftBorder,			// path to use as a left-hand border
											CPath& RightBorder );		// path to use as a right-hand border

	void				ClosePath( void );

	void				Resize( uint32 TargetPoints );					// target point count to resize to

	void				Clear( void );

	void				Reverse( void );

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	const std::string&	UserData( void )						{	return m_UserData;	};

	float32				Length( void )			{	return m_PathLength;		};	

	uint32				Points( void )			{	return m_Points.size();		};

	uint32				Segments( void )		{	return m_Points.size() - 1;	};

	bool				Closed( void )			{	return m_PathClosed;		};

	const std::string&	Name( void )			{	return m_PathName;			};

	D3DXVECTOR3			GetPoint( uint32 Index );					// index of point to retrieve

	float32				GetRadiusLeft( uint32 Index );	// index at which to retrieve radius
	float32				GetRadiusRight( uint32 Index );	// index at which to retrieve radius

	float32				GetTweenedRadiusLeft( float32 DistanceAlongPath );	// given this path position, retrieve tweened radius value
	float32				GetTweenedRadiusRight( float32 DistanceAlongPath ); // given this path position, retrieve tweened radius value

	D3DXVECTOR3			GetPositionAtDistance( float32 DistanceAlongPath );	// distance along path to calculate location

	D3DXVECTOR3			GetSplinePositionAtDistance( float32 DistanceAlongPath );	// distance along path to calculate location

	float32				GetAngleOverDistance( float32 StartDistance,		// beginning point at which to calculate angle deltas
											  float32 EndDistance );		// ending point at which to complete the calculation

	D3DXVECTOR3			GetSegmentPerpendicularY( uint32 Segment );			// segment to get the perpendicular of

	D3DXVECTOR3			GetPathSegment( uint32 Segment );				// segment to retrieve

	float32				GetPointDistance( uint32 Index );		// index of point to retrieve distance of
	float32				GetSegmentAngle( uint32 Index );		// index of segment to retrieve angle delta of

	bool				FindNearestPoint( const D3DXVECTOR3& Position,		// starting position to find nearest point from
										  D3DXVECTOR3& Point,				// resulting nearest point on path
										  D3DXVECTOR3& Perpendicular,		// perpendicular in the direction of the starting position
										  uint32& Segment,					// Segment of the path on which the resulting point resides
										  float32& Distance,				// how far along the path the resulting point is
										  float32 MinimumDistance = 0 );	// minimum distance along the path to check beyond (default 0)

	const D3DXVECTOR3&	MinBounds( void )	{	return m_MinBounds;	};
	const D3DXVECTOR3&	MaxBounds( void )	{	return m_MaxBounds;	};

	const D3DXVECTOR3&	Position( void )	{	return m_Position;	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void				SetUserData( const std::string& Data )	{	m_UserData = Data;	};

	void				AddPoint( const D3DXVECTOR3& Point,			// point to add to the path
								  float32 PathRadiusLeft = -60,		// left radius of the path at this point
								  float32 PathRadiusRight = 60 );	// rightradius of the path at this point

	void				SetRadiusLeft( uint32 Index,		// index at which to set radius
									   float32 Radius );	// radius value ( should be negative )
	void				SetRadiusRight( uint32 Index,		// index at which to set radius
										float32 Radius );	// radius value ( should be positive )

private:

	std::string		m_UserData;


	float32		m_PathLength;							// total length of the path
	
	D3DXVECTOR3						m_Position;			// worldspace location of path

	std::vector< D3DXVECTOR3 >		m_Points;			// position of each point along the path
	std::vector< float32 >			m_PathRadiusLeft;	// width of the path on the left-hand side, at a give point
	std::vector< float32 >			m_PathRadiusRight;	// width of the path on the right-hand side, at a given point
	std::vector< float32 >			m_SegmentAngle;		// difference in angle, in degrees, between one point and the next
	std::vector< float32 >			m_PointDistance;	// actual distance of a point along the entire length of the path
	
	bool		m_PathClosed;							// is this path closed?
	std::string	m_PathName;								// name of the path

	D3DXVECTOR3	m_MinBounds;
	D3DXVECTOR3 m_MaxBounds;
};


#endif