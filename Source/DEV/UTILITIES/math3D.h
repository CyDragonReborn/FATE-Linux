/*
	math3d.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

*/

#ifndef MATH3D_H
#define MATH3D_H

#include <D3D8.h>
#include <d3dx8.h>

#include <vector>
#include "constants.h"
using namespace std;

typedef 	std::vector <D3DXVECTOR3>	D3DXVECTOR3List; 

const float32	KCollisionInvalid( 99999 );

const enum EFaceClassification
{
	KNormalMaxX,
	KNormalMaxY,
	KNormalMaxZ
};

const enum ESortType
{
	KSortOctree,
	KSortQuadtree
};

const enum EPlaneClassification
{
	KPlaneFront,
	KPlaneBack,
	KPlaneOn,
	KPlaneSpanning
};

const float32	KEpsilon( .00001f );

const float64	KDeg2Rad	= 0.01745329252;
const float64	KRad2Deg	= 57.295827;

const float64	KPI			= 3.141592653;

DWORD			VectorToRGB( const D3DXVECTOR3& NormalVector );	// vector to convert to rgb


bool			BoundsContains( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								const D3DXVECTOR3& ObjectMax );			// max bounds to contain

bool			BoundsIntersect( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								 const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								 const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								 const D3DXVECTOR3& ObjectMax );		// max bounds to contain

bool			BoundsContainsXY( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								  const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								  const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								  const D3DXVECTOR3& ObjectMax );			// max bounds to contain

bool			BoundsIntersectXY( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								   const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								   const D3DXVECTOR3& ObjectMin,			// min bounds to contain
								   const D3DXVECTOR3& ObjectMax );		// max bounds to contain

bool			BoundsContains( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								 const D3DXVECTOR3& Position,			// position of sphere to contain
								 float32 Radius );						// radius of sphere to contain

bool			BoundsIntersect( const D3DXVECTOR3& ContainingMin,		// min bounds of containing space
								 const D3DXVECTOR3& ContainingMax,		// max bounds of containing space
								 const D3DXVECTOR3& Position,			// position of sphere to contain
								 float32 Radius );						// radius of sphere to contain

void			ExpandBounds( D3DXVECTOR3& Min,		// minimum bounds to expand
							  D3DXVECTOR3& Max,		// maximum bounds to expand
							  D3DXVECTOR3 Point );	// point to expand the bounds by

void			ExpandCubicBounds( D3DXVECTOR3& Min,		// minimum bounds to expand
								   D3DXVECTOR3& Max,		// maximum bounds to expand
								   D3DXVECTOR3 Point );		// point to expand the bounds by

void			ExpandHorizontalCubicBounds( D3DXVECTOR3& Min,		// minimum bounds to expand
										     D3DXVECTOR3& Max,		// maximum bounds to expand
											 D3DXVECTOR3 Point );		// point to expand the bounds by

float32			AngleBetween( D3DXVECTOR3 A,	// vector a
							  D3DXVECTOR3 B );	// vector b

void			RotateX( D3DXVECTOR3* Vector,	// vector to rotate 
						 float32 Angle );		// angle to rotate, in radians

void			RotateY( D3DXVECTOR3* Vector,	// vector to rotate 
						 float32 Angle );		// angle to rotate, in radians

void			RotateZ( D3DXVECTOR3* Vector,	// vector to rotate 
						 float32 Angle );		// angle to rotate, in radians

// transform a vector from world space to local space,
// and scale values along the z-axis for perspective correction
void			WorldToLocalPerspective( D3DXVECTOR3& Destination,		// destination vector to fill 
										 const D3DXVECTOR3& Source,		// worldspace vector
										 const D3DXMATRIX& Matrix );	// local orientation matrix

// transform a vector from world space to local space
void			WorldToLocal( D3DXVECTOR3& Destination,		// destination vector to fill 
							  const D3DXVECTOR3& Source,	// worldspace vector
							  const D3DXMATRIX& Matrix );	// local orientation matrix


void			ClosestPointOnLine( const D3DXVECTOR3& PointA,		// point a of line segment
									const D3DXVECTOR3& PointB,		// point b of line segment
									const D3DXVECTOR3& Point,		// point to check from
									D3DXVECTOR3& ClosestPoint );	// resultant point to be filled

void			ClosestPointOnTriangle( const D3DXVECTOR3& CornerA,		// corner a of the triangle
										const D3DXVECTOR3& CornerB,		// corner b of the triangle 
										const D3DXVECTOR3& CornerC,		// corner c of the triangle 
										const D3DXVECTOR3& Point, 		// point to start from
										D3DXVECTOR3& ClosestPoint );	// closest point to be filled

bool			GetLinePlaneIntersection( const D3DXVECTOR3& LineStart,			// line segment start 
										  const D3DXVECTOR3& LineEnd, 			// line segment end
										  const D3DXVECTOR3& VertexOnPlane,		// point on plane
										  const D3DXVECTOR3& Normal,			// normal of plane
										  D3DXVECTOR3& Intersection );			// intersection point to fill

bool			GetSpherePlaneIntersection( const D3DXVECTOR3& LineStart,		// line segment start
											const D3DXVECTOR3& LineEnd,			// line segment end
											const D3DXVECTOR3& VertexOnPlane,	// point on plane
											const D3DXVECTOR3& Normal,			// normal of plane
											D3DXVECTOR3& Intersection );		// intersection point to fill



EPlaneClassification ClassifyPoint( const D3DXVECTOR3& Point,			// point to classify
									const D3DXVECTOR3& VertexOnPlane,	// a vertex on the plane
									const D3DXVECTOR3& Normal );		// the normal of the plane

EPlaneClassification ClassifyPointForSphere( const D3DXVECTOR3& Point, 				// point to classify
											 const D3DXVECTOR3& VertexOnPlane, 		// a vertex on the plane
											 const D3DXVECTOR3& Normal, 			// the normal of the plane
											 float32 Radius );						// the radius of the sphere

float32			DistanceToPlane( const D3DXVECTOR3& RayOrigin,	
								const D3DXVECTOR3& RayVector,
								const D3DXVECTOR3& PlaneNormal,
								float32 PlaneD );

void FillGammaRamp( D3DGAMMARAMP& Ramp,	// ramp to fill
					uint32 Gamma );		// gamma between 0-255 (190 is about right )

#endif