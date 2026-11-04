/*
	math3d.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/



#include "constants.h"
#include "math3d.h"

float32	Percentage;
float32	LineLength, DistanceFromPlane;
float32	DeltaX, DeltaY, DeltaZ, ToPlaneX, ToPlaneY, ToPlaneZ;

// used primarily for DOT3 operations
DWORD VectorToRGB( const D3DXVECTOR3& NormalVector )	// vector to convert to rgb
{
	DWORD R = (DWORD)( 127 * NormalVector.x + 128 );
	DWORD G = (DWORD)( 127 * NormalVector.y + 128 );
	DWORD B = (DWORD)( 127 * NormalVector.z + 128 );

	return (DWORD)( 0xff000000 + ( R << 16 ) + ( G << 8 ) + B );
} // VectorToRGB()


// determine if one bounds is contained within another
bool			BoundsContains( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								const D3DXVECTOR3& ObjectMax )			// max bounds to contain
{
	if( ObjectMax.x <= ContainingMax.x &&
		ObjectMax.y <= ContainingMax.y &&
		ObjectMax.z <= ContainingMax.z &&
		ObjectMin.x >= ContainingMin.x &&
		ObjectMin.y >= ContainingMin.y &&
		ObjectMin.z >= ContainingMin.z )
	{
		return kTrue;
	}

	return kFalse;
} // BoundsContains()

// determine if one bounds intersects another
bool			BoundsIntersect( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								 const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								 const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								 const D3DXVECTOR3& ObjectMax )			// max bounds to contain
{
	if( ObjectMax.x >= ContainingMin.x &&
		ObjectMax.y >= ContainingMin.y &&
		ObjectMax.z >= ContainingMin.z &&
		ObjectMin.x <= ContainingMax.x &&
		ObjectMin.y <= ContainingMax.y &&
		ObjectMin.z <= ContainingMax.z )
	{
		return kTrue;
	}

	return kFalse;
} // BoundsIntersect()

// determine if one bounds is contained within another
bool			BoundsContainsXY( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								  const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								  const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								  const D3DXVECTOR3& ObjectMax )			// max bounds to contain
{
	if( ObjectMax.x <= ContainingMax.x &&
		ObjectMax.z <= ContainingMax.z &&
		ObjectMin.x >= ContainingMin.x &&
		ObjectMin.z >= ContainingMin.z )
	{
		return kTrue;
	}

	return kFalse;
} // BoundsContainsXY()

// determine if one bounds intersects another
bool			BoundsIntersectXY( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								   const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								   const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								   const D3DXVECTOR3& ObjectMax )			// max bounds to contain
{
	if( ObjectMax.x >= ContainingMin.x &&
		ObjectMax.z >= ContainingMin.z &&
		ObjectMin.x <= ContainingMax.x &&
		ObjectMin.z <= ContainingMax.z )
	{
		return kTrue;
	}

	return kFalse;
} // BoundsIntersectXY()

// determine if one bounds is contained within another
bool			BoundsContains( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								 const D3DXVECTOR3& Position,			// position of sphere to contain
								 float32 Radius )						// radius of sphere to contain
{
	if( Position.x >= ContainingMin.x + Radius &&
		Position.y >= ContainingMin.y + Radius &&
		Position.z >= ContainingMin.z + Radius &&
		Position.x <= ContainingMax.x - Radius &&
		Position.y <= ContainingMax.y - Radius &&
		Position.z <= ContainingMax.z - Radius )
	{
		return kTrue;
	}

	return kFalse;
} // BoundsContains()

// determine if one bounds intersects another
bool			BoundsIntersect( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								 const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								 const D3DXVECTOR3& Position,			// position of sphere to contain
								 float32 Radius )						// radius of sphere to contain
{
	if( Position.x >= ContainingMin.x - Radius &&
		Position.y >= ContainingMin.y - Radius &&
		Position.z >= ContainingMin.z - Radius &&
		Position.x <= ContainingMax.x + Radius &&
		Position.y <= ContainingMax.y + Radius &&
		Position.z <= ContainingMax.z + Radius )
	{
		return kTrue;
	}

	return kFalse;
} // BoundsIntersect()

// quick way to expand a bounding box
void ExpandBounds( D3DXVECTOR3& Min,	// minimum bounds to expand
				   D3DXVECTOR3& Max,	// maximum bounds to expand
				   D3DXVECTOR3 Point )	// point to expand the bounds by
{
	if ( Point.x < Min.x )
	{
		Min.x = Point.x;
	}
	if ( Point.x > Max.x )
	{
		Max.x = Point.x;
	}
	if ( Point.y < Min.y )
	{
		Min.y = Point.y;
	}
	if ( Point.y > Max.y )
	{
		Max.y = Point.y;
	}
	if ( Point.z < Min.z )
	{
		Min.z = Point.z;
	}
	if ( Point.z > Max.z )
	{
		Max.z = Point.z;
	}
} // ExpandBounds()

// quick way to expand an equal-sided bounding box
void ExpandCubicBounds( D3DXVECTOR3& Min,	// minimum bounds to expand
						D3DXVECTOR3& Max,	// maximum bounds to expand
						D3DXVECTOR3 Point )	// point to expand the bounds by
{

	float32 LargestAxis( 0 );
	if ( fabs( Point.x ) > LargestAxis )
	{
		LargestAxis = fabs( Point.x );
	}
	if ( fabs( Point.y ) > LargestAxis )
	{
		LargestAxis = fabs( Point.y );
	}
	if ( fabs( Point.z ) > LargestAxis )
	{
		LargestAxis = fabs( Point.z );
	}

	ExpandBounds( Min, Max, D3DXVECTOR3( LargestAxis, LargestAxis, LargestAxis ) );
	ExpandBounds( Min, Max, D3DXVECTOR3( -LargestAxis, -LargestAxis, -LargestAxis ) );
} // ExpandCubicBounds()

// quick way to expand an equal-sided bounding box
void ExpandHorizontalCubicBounds( D3DXVECTOR3& Min,	// minimum bounds to expand
						D3DXVECTOR3& Max,	// maximum bounds to expand
						D3DXVECTOR3 Point )	// point to expand the bounds by
{

	float32 LargestAxis( 0 );
	if ( fabs( Point.x ) > LargestAxis )
	{
		LargestAxis = fabs( Point.x );
	}
	if ( fabs( Point.z ) > LargestAxis )
	{
		LargestAxis = fabs( Point.z );
	}

	ExpandBounds( Min, Max, D3DXVECTOR3( LargestAxis, 0, LargestAxis ) );
	ExpandBounds( Min, Max, D3DXVECTOR3( -LargestAxis, 0, -LargestAxis ) );
} // ExpandHorizontalCubicBounds()

// calculate the angle between two supplied vectors, in radians
float32 AngleBetween( D3DXVECTOR3 A,	// vector a
					  D3DXVECTOR3 B )	// vector b
{
	return ( float32 ) acos( A.x * B.x + A.y * B.y + A.z * B.z );
} // AngleBetween()

// rotate the supplied vector around the y axis
void RotateY( D3DXVECTOR3* Vector,	// vector to rotate 
			  float32 Angle )		// angle to rotate, in radians
{
	float32	X	= Vector->x;
	float32	Z	= Vector->z;
	Vector->x = ( float32 ) ( X * cos( Angle ) + Z * sin( Angle ) );
	Vector->z = ( float32 ) ( -X * sin( Angle ) + Z * cos( Angle ) );
} // RotateY()

// rotate the supplied vector around the x axis
void RotateX( D3DXVECTOR3* Vector,	// vector to rotate 
			  float32 Angle )		// angle to rotate, in radians
{
	float32	Y	= Vector->y;
	float32	Z	= Vector->z;
	Vector->y = ( float32 ) ( Y * cos( Angle ) + Z * sin( Angle ) );
	Vector->z = ( float32 ) ( -Y * sin( Angle ) + Z * cos( Angle ) );
} // RotateX()

// rotate the supplied vector around the z axis
void RotateZ( D3DXVECTOR3* Vector,	// vector to rotate 
			  float32 Angle )		// angle to rotate, in radians
{
	float32	Y	= Vector->y;
	float32	X	= Vector->x;
	Vector->x = ( float32 ) ( X * cos( Angle ) + Y * sin( Angle ) );
	Vector->y = ( float32 ) ( -X * sin( Angle ) + Y * cos( Angle ) );
} // RotateZ()

// transform a vector from world space to local space,
// and scale values along the z-axis for perspective correction
void WorldToLocalPerspective( D3DXVECTOR3& Destination,		// destination vector to fill 
							  const D3DXVECTOR3& Source,	// worldspace vector
							  const D3DXMATRIX& Matrix )	// local orientation Matrix
{
	float32	X	= Source.x;
	float32	Y	= Source.y;
	float32	Z	= Source.z;
	Destination.x = ( Matrix.m[0][0] * X +
					  Matrix.m[0][1] * Y +
					  Matrix.m[0][2] * Z );
	Destination.y = ( Matrix.m[1][0] * X +
					  Matrix.m[1][1] * Y +
					  Matrix.m[1][2] * Z );
	Destination.z = ( Matrix.m[2][0] * X +
					  Matrix.m[2][1] * Y +
					  Matrix.m[2][2] * Z );

	Z = Destination.z;
	if ( Z < 0 )
	{
		Z *= -1.0F;
	}
	Destination.x /= Z;
	Destination.y /= Z;
} // WorldToLocalPerspective()

// transform a vector from world space to local space
void WorldToLocal( D3DXVECTOR3& Destination,	// destination vector to fill 
				   const D3DXVECTOR3& Source,	// worldspace vector
				   const D3DXMATRIX& Matrix )	// local orientation Matrix
{
	float32	X	= Source.x;
	float32	Y	= Source.y;
	float32	Z	= Source.z;
	Destination.x = ( Matrix.m[0][0] * X +
					  Matrix.m[0][1] * Y +
					  Matrix.m[0][2] * Z );
	Destination.y = ( Matrix.m[1][0] * X +
					  Matrix.m[1][1] * Y +
					  Matrix.m[1][2] * Z );
	Destination.z = ( Matrix.m[2][0] * X +
					  Matrix.m[2][1] * Y +
					  Matrix.m[2][2] * Z );
} // WorldToLocal()

void ClosestPointOnTriangle( const D3DXVECTOR3& CornerA,	// corner a of the triangle
							 const D3DXVECTOR3& CornerB,	// corner b of the triangle 
							 const D3DXVECTOR3& CornerC,	// corner c of the triangle 
							 const D3DXVECTOR3& Point, 		// point to start from
							 D3DXVECTOR3& ClosestPoint )	// closest point to be filled
{
	D3DXVECTOR3	TempVEC3D;
	D3DXVECTOR3	DistanceVEC3D;
	float32		BestDistance, Distance;
	ClosestPointOnLine( CornerA, CornerB, Point, TempVEC3D );
	DistanceVEC3D = TempVEC3D - Point;
	BestDistance = D3DXVec3LengthSq( &DistanceVEC3D );
	ClosestPoint = TempVEC3D;

	ClosestPointOnLine( CornerB, CornerC, Point, TempVEC3D );
	DistanceVEC3D = TempVEC3D - Point;
	Distance = D3DXVec3LengthSq( &DistanceVEC3D );

	if ( Distance < BestDistance )
	{
		ClosestPoint = TempVEC3D;
		BestDistance = Distance;
	}

	ClosestPointOnLine( CornerC, CornerA, Point, TempVEC3D );
	DistanceVEC3D = TempVEC3D - Point;
	Distance = D3DXVec3LengthSq( &DistanceVEC3D );

	if ( Distance < BestDistance )
	{
		ClosestPoint = TempVEC3D;
		BestDistance = Distance;
	}
} // ClosestPointOnTriangle()



void ClosestPointOnLine( const D3DXVECTOR3& PointA,		// point a of line segment
						 const D3DXVECTOR3& PointB,		// point b of line segment
						 const D3DXVECTOR3& Point,		// point to check from
						 D3DXVECTOR3& ClosestPoint )	// resultant point to be filled
{
	float32		LineMagnitude;
	float32		U;
	D3DXVECTOR3	Intersection;

	Intersection = PointB - PointA;
	LineMagnitude = D3DXVec3LengthSq( &Intersection );

	U = ( ( ( Point.x - PointA.x ) * ( PointB.x - PointA.x ) ) +
		  ( ( Point.y - PointA.y ) * ( PointB.y - PointA.y ) ) +
		  ( ( Point.z - PointA.z ) * ( PointB.z - PointA.z ) ) ) /
		( LineMagnitude );

	if ( U < 0 )
	{
		ClosestPoint = PointA;
		return;
	}
	if ( U > 1 )
	{
		ClosestPoint = PointB;
		return;
	}

	Intersection.x = PointA.x + U * ( PointB.x - PointA.x );
	Intersection.y = PointA.y + U * ( PointB.y - PointA.y );
	Intersection.z = PointA.z + U * ( PointB.z - PointA.z );

	ClosestPoint = Intersection;
} // ClosestPointOnLine()

EPlaneClassification ClassifyPoint( const D3DXVECTOR3& Point,			// point to classify
									const D3DXVECTOR3& VertexOnPlane,	// a vertex on the plane
									const D3DXVECTOR3& Normal )			// the normal of the plane
{
	float32	Result	= ( VertexOnPlane.x - Point.x ) * Normal.x +
					  ( VertexOnPlane.y - Point.y ) * Normal.y +
					  ( VertexOnPlane.z - Point.z ) * Normal.z;
	if ( Result < -KEpsilon )
	{
		return KPlaneFront;
	}
	if ( Result > KEpsilon )
	{
		return KPlaneBack;
	}
	return KPlaneOn;
} // ClassifyPoint()

EPlaneClassification ClassifyPointForSphere( const D3DXVECTOR3& Point, 				// point to classify
											 const D3DXVECTOR3& VertexOnPlane, 		// a vertex on the plane
											 const D3DXVECTOR3& Normal, 			// the normal of the plane
											 float32 Radius )						// the radius of the sphere
{
	float32	Result	= ( VertexOnPlane.x - Point.x ) * Normal.x +
					  ( VertexOnPlane.y - Point.y ) * Normal.y +
					  ( VertexOnPlane.z - Point.z ) * Normal.z;
	Result += Radius;
	if ( Result < -KEpsilon )
	{
		return KPlaneFront;
	}
	if ( Result > KEpsilon )
	{
		return KPlaneBack;
	}
	return KPlaneOn;
} // ClassifyPointForSphere()	

bool GetLinePlaneIntersection( const D3DXVECTOR3& LineStart,		// line segment start 
											   const D3DXVECTOR3& LineEnd, 			// line segment end
											   const D3DXVECTOR3& VertexOnPlane,	// point on plane
											   const D3DXVECTOR3& Normal,			// normal of plane
											   D3DXVECTOR3& Intersection )			// intersection point to fill
{
	DeltaX = LineEnd.x - LineStart.x;
	DeltaY = LineEnd.y - LineStart.y;
	DeltaZ = LineEnd.z - LineStart.z;
	LineLength = DeltaX * Normal.x + DeltaY * Normal.y + DeltaZ * Normal.z;
	if ( fabs( LineLength ) < KEpsilon )
	{
		return kFalse;
	}
	ToPlaneX = VertexOnPlane.x - LineStart.x;
	ToPlaneY = VertexOnPlane.y - LineStart.y;
	ToPlaneZ = VertexOnPlane.z - LineStart.z;
	DistanceFromPlane = ToPlaneX * Normal.x + ToPlaneY * Normal.y + ToPlaneZ * Normal.z;

	Percentage = DistanceFromPlane / LineLength;
	if ( Percentage <0.0f || Percentage> 1.0f )
	{
		return kFalse;
	}

	Intersection.x = LineStart.x + DeltaX * Percentage;
	Intersection.y = LineStart.y + DeltaY * Percentage;
	Intersection.z = LineStart.z + DeltaZ * Percentage;
	return kTrue;
} // GetLinePlaneIntersection()

bool GetSpherePlaneIntersection( const D3DXVECTOR3& LineStart,		// line segment start
												 const D3DXVECTOR3& LineEnd,		// line segment end
												 const D3DXVECTOR3& VertexOnPlane,	// point on plane
												 const D3DXVECTOR3& Normal,			// normal of plane
												 D3DXVECTOR3& Intersection )		// intersection point to fill
{
	DeltaX = LineEnd.x - LineStart.x;
	DeltaY = LineEnd.y - LineStart.y;
	DeltaZ = LineEnd.z - LineStart.z;
	LineLength = DeltaX * Normal.x + DeltaY * Normal.y + DeltaZ * Normal.z;
	if ( fabs( LineLength ) < KEpsilon )
	{
		return kFalse;
	}
	ToPlaneX = VertexOnPlane.x - LineStart.x;
	ToPlaneY = VertexOnPlane.y - LineStart.y;
	ToPlaneZ = VertexOnPlane.z - LineStart.z;
	DistanceFromPlane = ToPlaneX * Normal.x + ToPlaneY * Normal.y + ToPlaneZ * Normal.z;

	Percentage = DistanceFromPlane / LineLength;

	Intersection.x = LineStart.x + DeltaX * Percentage;
	Intersection.y = LineStart.y + DeltaY * Percentage;
	Intersection.z = LineStart.z + DeltaZ * Percentage;
	return kTrue;
} // GetSpherePlaneIntersection()


float32 DistanceToPlane( const D3DXVECTOR3& RayOrigin,	
						 const D3DXVECTOR3& RayVector,
						 const D3DXVECTOR3& PlaneNormal,
						 float32 PlaneD )
 {
     float32 CosAlpha;
     float32 DeltaD;
 
 
     CosAlpha = D3DXVec3Dot( &RayVector,  &PlaneNormal );
 
     // parallel to the plane (alpha=90)
     if ( CosAlpha == 0) 
	 {
		 return -1.0f;
	 }
 
 
     DeltaD = PlaneD - D3DXVec3Dot( &RayOrigin, &PlaneNormal );
     
     return ( DeltaD / CosAlpha);
 }

void FillGammaRamp( D3DGAMMARAMP& Ramp,	// ramp to fill
					uint32 Gamma )		// gamma between 0-255 (90 is about right )
{
	float32 FloatGamma = (float32)Gamma / 255.0f;
	for ( uint32 i = 0; i < 256; i++ )
	{
		int16 RampVal = (int16)( 65535 * pow( i / 255.0f, FloatGamma ) );

		Ramp.red[i] = Ramp.green[i] = Ramp.blue[i] =
        RampVal;
    }
} // FillGammaRamp()
