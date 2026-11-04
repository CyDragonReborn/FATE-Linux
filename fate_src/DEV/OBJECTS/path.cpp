/*
	path.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	Change History

	Marsh Lefler
	5/15/2004  - fixed FindNearestPoint for non-closed paths


*/

#include "../UTILITIES/macros.h"
#include "../UTILITIES/math3d.h"

#include "path.h"


CPath::CPath( std::string PathName,				// name of path
			  bool PathClosed,				// is this a closed spline?
			  const D3DXVECTOR3& Position ) :	// worldspace location
			  m_Position( Position ),
			  m_PathClosed( PathClosed ),
			  m_PathName( PathName ),
			  m_PathLength( 0 ),
			  m_UserData ( "" )
{
	ZeroMemory ( &m_MinBounds, sizeof ( m_MinBounds ));
	ZeroMemory ( &m_MaxBounds, sizeof ( m_MaxBounds ));
} // CPath::CPath()

CPath::~CPath( void )
{
} // CPath::~CPath()

CPath::CPath( CPath& Path ) 	// path to copy from
{
	m_Position = Path.Position();
	m_PathClosed = Path.Closed();
	m_PathName = Path.Name();
	m_PathLength = Path.Length();
	m_UserData = Path.UserData();

	m_MaxBounds = Path.MaxBounds();
	m_MinBounds = Path.MinBounds();

	m_Points.resize( Path.Points() );
	m_PathRadiusLeft.resize( Path.Points() );
	m_PathRadiusRight.resize( Path.Points() );
	m_PointDistance.resize( Path.Points() );
	m_SegmentAngle.resize( Path.Points() );

	for( uint32 i = 0; i < Path.Points(); i++ )
	{
		m_Points[i] = Path.GetPoint( i ) - m_Position;
		m_PathRadiusLeft[i] = Path.GetRadiusLeft( i );
		m_PathRadiusRight[i] = Path.GetRadiusRight( i );
		m_PointDistance[i] = Path.GetPointDistance( i );
		m_SegmentAngle[i] = Path.GetSegmentAngle( i );
	}

} // CPath::CPath()

// remove all points
void CPath::Clear( void )
{
	m_PathLength = 0;
	m_Points.clear();
	m_SegmentAngle.clear();
	m_PathRadiusLeft.clear();
	m_PathRadiusRight.clear();
	m_PointDistance.clear();
} // CPath::Clear()

// add a point to the path - this must be done sequentially!
void CPath::AddPoint( const D3DXVECTOR3& Point,	// point to add to the path
					  float32 PathRadiusLeft,		// left radius of the path at this point
					  float32 PathRadiusRight )		// rightradius of the path at this point
{

	if( Points() == 0 )
	{
		m_MinBounds = Point;
		m_MaxBounds = Point;
	}
	else
	{
		ExpandBounds( m_MinBounds, m_MaxBounds, Point );
	}

	m_Points.push_back( Point );
	m_PathRadiusLeft.push_back( PathRadiusLeft );
	m_PathRadiusRight.push_back( PathRadiusRight );

	// no points, so position along the path is 0
	if( m_Points.size() < 2 )
	{
		m_PointDistance.push_back( 0 );
		m_SegmentAngle.push_back( 0 );
		m_PathLength = 0;
	}
	else
	{
		// calculate the delta between this point and the previous one, and store distance
		// and angle differences
		D3DXVECTOR3 TempVEC3D = m_Points[m_Points.size() - 1] - 
							    m_Points[m_Points.size() - 2];

		float32 Distance = D3DXVec3Length( &TempVEC3D );

		m_PathLength += Distance;

		Distance += m_PointDistance[m_PointDistance.size() - 1];

		m_PointDistance.push_back( Distance );

		// perform angle delta calculations
		m_SegmentAngle.push_back( 0 );
		if( m_Points.size() > 2 )
		{
			D3DXVECTOR3 TempVEC3D2 = m_Points[m_Points.size() - 2] - 
								     m_Points[m_Points.size() - 3];

			D3DXVec3Normalize( &TempVEC3D, &TempVEC3D );
			D3DXVec3Normalize( &TempVEC3D2, &TempVEC3D2 );

			float32	Angle	= (float32)( KRad2Deg * AngleBetween( TempVEC3D, TempVEC3D2 ) );
			m_SegmentAngle[m_Points.size() - 2] = Angle;
		}

	}
} // CPath::AddPoint()

// retrieve a point from the path
D3DXVECTOR3 CPath::GetPoint( uint32 Index )		// index of point to retrieve
{
	if ( Index < m_Points.size() )
	{
		return m_Points[Index] + m_Position;
	}
	else
	{
		return m_Position;
	}
} // CPath::GetPoint()

// retrieve the distance of a point down th epath
float32 CPath::GetPointDistance( uint32 Index )		// index of point to retrieve distance of
{
	if ( Index < m_PointDistance.size() )
	{
		return m_PointDistance[Index];
	}
	return 0;
} // CPath::GetPointDistance()

// retrieve the angle in degrees of a segment to the previous angle
float32 CPath::GetSegmentAngle( uint32 Index )		// index of segment to retrieve angle delta of
{
	if ( Index < m_SegmentAngle.size() )
	{
		return m_SegmentAngle[Index];
	}
	return 0;
} // CPath::GetSegmentAngle()


// calculates the full path length for a closed path - 
// it seams up the first and last points 
void CPath::ClosePath( void )
{
	D3DXVECTOR3 Start = GetPoint( 0 );
	Start -= GetPoint( Points() - 1 );
	m_PathLength += D3DXVec3Length( &Start );

} // CPath::ClosePath( void )

void CPath::Resize( uint32 TargetPoints )	// target point count to resize to
{
	if( Points() <= TargetPoints  )
	{
		return;
	}

	// copy our existing locations
	std::vector< D3DXVECTOR3 >	PointsCopy = m_Points;

	// calculate a step size based upon the ratio of old points to new
	float32 StepSize = (float32)Points() / 
					   (float32)TargetPoints;

	// clear out all the original data
	Clear();

	// and add back in only the points we need
	for( uint32 i = 0; i < TargetPoints; i++ )
	{
		AddPoint( PointsCopy[(uint32)floor( i * StepSize + .5f )] );
	}

	// remember to stitch back up the path if it is closed
	if( Closed() )
	{
		ClosePath();
	}

} // CPath::Resize()

void CPath::Reverse( void )
{

	// copy our existing locations
	std::vector< D3DXVECTOR3 >	PointsCopy = m_Points;

	uint32 Indices = Points();
	// clear out all the original data
	Clear();

	// and add back in only the points we need
	for( uint32 i = 0; i < Indices; i++ )
	{
		AddPoint( PointsCopy[Indices - 1 - i] );
	}

	// remember to stitch back up the path if it is closed
	if( Closed() )
	{
		ClosePath();
	}

} // CPath::Reverse()

// recalculate the radius of the path based upon a left and right bordering path
// in this way, your path can have variable with along its route
// this is especially useful for pathing and racing games, as you
// will not only be able to tell your location along a path, but whether you
// are within the path's arbitrary bounds
void CPath::CalculatePathWidth( CPath& LeftBorder,		// path to use as a left-hand border
								CPath& RightBorder )	// path to use as a right-hand border
{
	D3DXVECTOR3	Point, Perpendicular;
	uint32		Segment = 0;
	float32		Distance = 0;
	for ( uint32 i = 0; i < m_Points.size(); i++ )
	{

		if ( LeftBorder.FindNearestPoint( m_Points[i],
										  Point,
										  Perpendicular,
										  Segment,
										  Distance ) )
		{
			D3DXVec3Normalize( &Perpendicular, &Perpendicular );
			Point = Point - m_Points[i];
			Point.y = 0;

			m_PathRadiusLeft[i] = D3DXVec3Dot( &Point, &Perpendicular );
			if ( m_PathRadiusLeft[i] > 0 )
			{
				m_PathRadiusLeft[i] = 0;
			}
		}
		if ( RightBorder.FindNearestPoint( m_Points[i],
										   Point,
										   Perpendicular,
										   Segment,
										   Distance ) )
		{
			D3DXVec3Normalize( &Perpendicular, &Perpendicular );
			Perpendicular *= -1;
			Point = Point - m_Points[i];
			Point.y = 0;
			m_PathRadiusRight[i] = D3DXVec3Dot( &Point, &Perpendicular );
			if ( m_PathRadiusRight[i] < 0 )
			{
				m_PathRadiusRight[i] = 0;
			}
		}
	}
} // CPath::CalculatePathWidth()

float32 CPath::GetRadiusLeft( uint32 Index )	// index at which to retrieve radius
{
	if( m_PathClosed && Index >= m_PathRadiusLeft.size() )
	{
		Index -= m_PathRadiusLeft.size();
	}
	if( Index < m_PathRadiusLeft.size() )
	{
		return m_PathRadiusLeft[Index];
	}

	return 0;
} // CPath::GetRadiusLeft()

float32 CPath::GetRadiusRight( uint32 Index )	// index at which to retrieve radius
{
	if( m_PathClosed && Index >= m_PathRadiusRight.size() )
	{
		Index -= m_PathRadiusRight.size();
	}
	if( Index < m_PathRadiusRight.size() )
	{
		return m_PathRadiusRight[Index];
	}

	return 0;
} // CPath::GetRadiusRight()

void CPath::SetRadiusLeft( uint32 Index,		// index at which to set radius
						   float32 Radius )	// radius value ( should be negative )
{
	if( Index < m_PathRadiusLeft.size() )
	{
		m_PathRadiusLeft[Index] = Radius;
	}
} // CPath::SetRadiusLeft()

void CPath::SetRadiusRight( uint32 Index,		// index at which to set radius
							float32 Radius )	// radius value ( should be positive )
{
	if( Index < m_PathRadiusRight.size() )
	{
		m_PathRadiusRight[Index] = Radius;
	}
} // CPath::SetRadiusRight()


float32 CPath::GetTweenedRadiusRight( float32 DistanceAlongPath ) // given this path position, retrieve tweened radius value
{
	float32 Radius( 0 );

	while( DistanceAlongPath < 0 )
	{
		DistanceAlongPath += m_PathLength;
	}

	while( DistanceAlongPath > m_PathLength )
	{
		DistanceAlongPath -= m_PathLength;

	}

	for ( uint32 i = 0; i < m_Points.size() ; i++ )
	{
		uint32 NextPoint = i + 1;
		if( NextPoint >= m_Points.size() )
		{
			NextPoint -= m_Points.size();
		}

		if( m_PointDistance[NextPoint] > DistanceAlongPath ||
			NextPoint == 0 )
		{
			float32 Length = m_PointDistance[NextPoint] - m_PointDistance[i];
			if( NextPoint == 0 && m_PathClosed )
			{
				Length = m_PathLength - m_PointDistance[i];
			}
			float32 Percentage = DistanceAlongPath - m_PointDistance[i];
			Percentage /= Length;
			Radius = ( m_PathRadiusRight[NextPoint] - m_PathRadiusRight[i] ) * Percentage;
			Radius += m_PathRadiusRight[i];
			return Radius;
		}
	}
	return Radius;

} // CPath::GetTweenedRadiusRight()

float32 CPath::GetTweenedRadiusLeft( float32 DistanceAlongPath ) // given this path position, retrieve tweened radius value
{
	float32 Radius( 0 );

	while( DistanceAlongPath < 0 )
	{
		DistanceAlongPath += m_PathLength;
	}

	while( DistanceAlongPath > m_PathLength )
	{
		DistanceAlongPath -= m_PathLength;

	}

	for ( uint32 i = 0; i < m_Points.size() ; i++ )
	{
		uint32 NextPoint = i + 1;
		if( NextPoint >= m_Points.size() )
		{
			NextPoint -= m_Points.size();
		}

		if( m_PointDistance[NextPoint] > DistanceAlongPath ||
			NextPoint == 0 )
		{
			float32 Length = m_PointDistance[NextPoint] - m_PointDistance[i];
			if( NextPoint == 0 && m_PathClosed )
			{
				Length = m_PathLength - m_PointDistance[i];
			}
			float32 Percentage = DistanceAlongPath - m_PointDistance[i];
			Percentage /= Length;
			Radius = ( m_PathRadiusLeft[NextPoint] - m_PathRadiusLeft[i] ) * Percentage;
			Radius += m_PathRadiusLeft[i];
			return Radius;
		}
	}
	return Radius;

} // CPath::GetTweenedRadiusLeft()

// calculate the position at a given arbitrary distance along the path
D3DXVECTOR3 CPath::GetPositionAtDistance( float32 DistanceAlongPath )	// distance along path to calculate location
{
	D3DXVECTOR3 Position;
	while( DistanceAlongPath < 0 )
	{
		DistanceAlongPath += m_PathLength;
	}

	while( DistanceAlongPath > m_PathLength )
	{
		DistanceAlongPath -= m_PathLength;

	}
	if( DistanceAlongPath == m_PathLength )
	{
	    return m_Points[m_Points.size() - 1] + m_Position;
	}
	if( DistanceAlongPath == 0 )
	{
	    return m_Points[0] + m_Position;
	}

	for ( uint32 i = 0; i < m_Points.size() ; i++ )
	{
		uint32 NextPoint = i + 1;
		if( NextPoint >= m_Points.size() )
		{
			NextPoint -= m_Points.size();
		}

		if( m_PointDistance[NextPoint] > DistanceAlongPath ||
			NextPoint == 0 )
		{
			Position = m_Points[NextPoint] - m_Points[i];
			D3DXVec3Normalize( &Position, &Position );

			Position *= ( DistanceAlongPath - m_PointDistance[i] );
			Position += m_Points[i];
			return Position + m_Position ;
		}
	}
	return Position + m_Position;
} // CPath::GetPositionAtDistance()

// calculate the catmull-rom position at a given arbitrary distance along the path
D3DXVECTOR3 CPath::GetSplinePositionAtDistance( float32 DistanceAlongPath )	// distance along path to calculate location
{
	D3DXVECTOR3 Position;
	if( m_PathLength == 0 )
	{
		return m_Position;
	}
	while( DistanceAlongPath < 0 )
	{
		DistanceAlongPath += m_PathLength;
	}

	while( DistanceAlongPath > m_PathLength )
	{
		DistanceAlongPath -= m_PathLength;

	}
	// 4 points to be used to calculate the catmull rom location
	int32 A( 0 );
	int32 B( 0 );
	int32 C( 0 );
	int32 D( 0 );
	float32 Weight( 0 );

	for ( int32 i = m_Points.size() - 1 ; i >= 0 ; i-- )
	{
		if( m_PointDistance[i] < DistanceAlongPath )
		{
			B = i;
			break;
		}
	}

	A = B-1;
	C = B+1;
	D = B+2;

	int32 Size = m_Points.size();

	if( DistanceAlongPath == m_PathLength )
	{
	    return m_Points[Size - 1] + m_Position;
	}
	if( DistanceAlongPath == 0 )
	{
	    return m_Points[0] + m_Position;
	}

	if( A < 0 )
	{
		if( m_PathClosed )
		{
			A+= Size;
		}
		else
		{
			A = 0;
		}
	}
	if( C >= Size )
	{
		if( m_PathClosed )
		{
			C-= Size;
		}
		else
		{
			C = Size - 1;
		}
	}

	if( D >= Size )
	{
		if( m_PathClosed )
		{
			D -= Size;
		}
		else
		{
			D = Size - 1;
		}
	}

	Weight = m_PointDistance[C] - m_PointDistance[B];
	if( Weight < 0 )
	{
		Weight += m_PathLength;
	}
	float32 DistanceDelta = DistanceAlongPath - m_PointDistance[B];
	if( DistanceAlongPath < m_PointDistance[B] )
	{
		DistanceDelta += m_PathLength;
	}
	Weight = ( DistanceDelta ) / Weight;

	D3DXVec3CatmullRom( &Position,
					    &m_Points[A],
					    &m_Points[B],
					    &m_Points[C],
					    &m_Points[D],
					    Weight );

	return Position + m_Position;
} // CPath::GetSplinePositionAtDistance()

// calculates the change in angle of the path, in degrees, over a specified distance
// primarily useful for seeing how 'sharp' an upcoming corner is
float32 CPath::GetAngleOverDistance( float32 StartDistance,	// beginning point at which to calculate angle deltas
								     float32 EndDistance )	// ending point at which to complete the calculation
{
	if( EndDistance <= StartDistance )
	{
		return 0;
	}
	if( StartDistance < 0 )
	{
		StartDistance = 0;
	}
	if( EndDistance > m_PathLength )
	{
		EndDistance = m_PathLength;
	}

	int32		StartSegment	= -1;
	int32		EndSegment		= -1;

	// find the two segments we need to calculate angle deltas between
	for ( uint32 i = 0; i < m_Points.size() ; i++ )
	{
		if ( i > 0 && m_PointDistance[i] > StartDistance && StartSegment == -1 )
		{
			StartSegment = i;
		}
		if ( i > 0 &&  m_PointDistance[i] > EndDistance && EndSegment == -1 )
		{
			EndSegment = i;
		}
		if ( EndSegment != -1 && StartSegment != -1 )
		{
			break;
		}
	}

	float32	Angle = 0;

	StartSegment++;
	for ( ; StartSegment <= EndSegment; StartSegment++ )
	{
		if ( StartSegment == EndSegment )
		{
			float32 Delta = m_PointDistance[EndSegment] - m_PointDistance[EndSegment - 1];

			Delta = ( EndDistance - m_PointDistance[EndSegment - 1] ) / Delta;
			Angle += m_SegmentAngle[StartSegment] * Delta;
		}
		else
		{
			Angle += m_SegmentAngle[StartSegment];
		}
	}
	return Angle;
} // CPath::GetAngleOverDistance()

// get the perpendicular vector to the Y axis at a given point along the path
D3DXVECTOR3 CPath::GetSegmentPerpendicularY( uint32 Segment )				// Segment to get the perpendicular of
{
	D3DXVECTOR3 Perpendicular( 0, 0, 0 );
	if ( m_Points.size() <2 || 
		 Segment > m_Points.size() - 1 )
	{
		return Perpendicular;
	}

	uint32 NextPoint = Segment + 1;
	if( NextPoint == Points() &&
		Closed() )
	{
		NextPoint = 0;
	}

	Perpendicular = m_Points[NextPoint] - m_Points[Segment];

	Perpendicular.y = Perpendicular.z;
	Perpendicular.z = -Perpendicular.x;
	Perpendicular.x = Perpendicular.y;
	Perpendicular.y = 0;
	D3DXVec3Normalize( &Perpendicular, &Perpendicular );
	return Perpendicular;
} // CPath::GetSegmentPerpendicularY()

D3DXVECTOR3 CPath::GetPathSegment( uint32 Segment )				// Segment to retrieve
{
	D3DXVECTOR3 PathSegment( 0, 0, 0 );
	if ( m_Points.size() <2 || 
		 Segment > m_Points.size() - 2 )
	{
		return PathSegment;
	}

	PathSegment = m_Points[Segment + 1] - m_Points[Segment];
	return PathSegment;
} // CPath::GetPathSegment()

bool CPath::FindNearestPoint( const D3DXVECTOR3& Position,		// starting position to find nearest point from
							  D3DXVECTOR3& Point,				// resulting nearest point on path
							  D3DXVECTOR3& Perpendicular,		// perpendicular in the direction of the starting position
							  uint32& Segment,					// Segment of the path on which the resulting point resides
							  float32& Distance,				// how far along the path the resulting point is
							  float32 MinimumDistance )			// minimum distance along the path to check beyond (default 0)
{
	if ( m_Points.size() < 2 )
	{
		return kFalse;
	}

	if( !m_PathClosed && MinimumDistance > m_PathLength )
	{
		MinimumDistance = m_PathLength;
	}
	float32	BestDistance = KCollisionInvalid;

	float32		Length;
	float32		CurrentDistance;
	D3DXVECTOR3	Delta;
	D3DXVECTOR3	CurrentPosition;
	D3DXVECTOR3	CurrentPerpendicular;
	// bring the position into the local space of the path
	D3DXVECTOR3 LocalPosition = Position - m_Position;

	uint32 PathLength = m_Points.size();
	// for closed paths, we don't want to wrap around
	if( !m_PathClosed )
	{
		PathLength --;
	}

	// set our first, best distance to the first point on the spline.
	CurrentPosition = m_Points[0];
	CurrentPerpendicular = LocalPosition - CurrentPosition;
	Length = D3DXVec3Length( &CurrentPerpendicular );
	Distance = 0;
	Point = CurrentPosition;
	Perpendicular = CurrentPerpendicular;
	//BestDistance = Length;
	Segment = 0;

	for ( uint32 i = 0; i < PathLength; i++ )
	{
		if( BestDistance == KCollisionInvalid &&
			m_PointDistance[i] >= MinimumDistance )
		{
			CurrentPosition = m_Points[i];
			CurrentPerpendicular = LocalPosition - CurrentPosition;
			Length = D3DXVec3Length( &CurrentPerpendicular );
			if ( Length < BestDistance )
			{
				Distance = m_PointDistance[i];
				Point = CurrentPosition;
				Perpendicular = CurrentPerpendicular;
				BestDistance = Length;
				Segment = i;
			}
		}

		uint32 NextPoint = i + 1;
		if( NextPoint >= m_Points.size() )
		{
			NextPoint -= m_Points.size();
		}
		Delta = m_Points[NextPoint] - m_Points[i];
		Length = D3DXVec3Length( &Delta );

		float32	U	= ( ( ( LocalPosition.x - m_Points[i].x ) * 
						  ( m_Points[NextPoint].x - m_Points[i].x ) ) +
						( ( LocalPosition.z - m_Points[i].z ) * 
						  ( m_Points[NextPoint].z - m_Points[i].z ) ) +
						  ( ( LocalPosition.y - m_Points[i].y ) * 
						  ( m_Points[NextPoint].y - m_Points[i].y ) ) ) /
					    ( Length * Length );

		if ( !( U < 0 || U > 1 ) )
		{
			CurrentDistance = m_PointDistance[i] + Length * U;
			if ( CurrentDistance < MinimumDistance )
			{
				float32	Makeup	= MinimumDistance - CurrentDistance + 1.0f;
				Makeup = ( 1.0f / Length ) * Makeup;
				U += Makeup;
				if ( U > 1 )
				{
					U = 1;
				}
				CurrentDistance = m_PointDistance[i]  + Length * U;
			}

			if ( CurrentDistance >= MinimumDistance )
			{
				CurrentPosition = Delta * U + m_Points[i];
				CurrentPerpendicular = LocalPosition - CurrentPosition;
				Length = D3DXVec3Length( &CurrentPerpendicular );
				if ( Length < BestDistance )
				{
					Distance = CurrentDistance;
					Point = CurrentPosition;
					Perpendicular = CurrentPerpendicular;
					BestDistance = Length;
					Segment = i;
				}
			}
		}
		else
		{
			CurrentDistance = m_PointDistance[NextPoint] ;
			if( NextPoint == 0 )
			{
				CurrentDistance = m_PathLength;
			}

			if ( CurrentDistance < MinimumDistance )
			{
				float32	Makeup	= MinimumDistance - CurrentDistance + 1.0f;
				Makeup = ( 1.0f / Length ) * Makeup;
				U += Makeup;
				if ( U > 1 )
				{
					U = 1;
				}
				CurrentDistance = m_PointDistance[i] + Length * U;
			}


			if ( CurrentDistance >= MinimumDistance )
			{
				CurrentPosition = m_Points[NextPoint];
				CurrentPerpendicular = LocalPosition - CurrentPosition;
				Length = D3DXVec3Length( &CurrentPerpendicular );
				if ( Length < BestDistance )
				{
					Distance = CurrentDistance;
					Point = CurrentPosition;
					Perpendicular = CurrentPerpendicular;
					BestDistance = Length;
					Segment = i;
				}
			}
		}
	}
	if ( BestDistance != KCollisionInvalid )
	{
		// move the resulting point into world space
		Point += m_Position;
		return true;
	}
	return false;
} // CPath::FindNearestPoint()