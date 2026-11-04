//
//	Sphere.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Math/Objects/Shape3d.h"

namespace WT
{
namespace Math
{


//
//	CSphere :
//		represent a math sphere in space - center and
//      radius
//

typedef System::SmartPointer<class CSphere> HSphere;
class CSphere : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CSphere, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	// data members
	CVector3f	Center;		// Center of the sphere
	float32		Radius;		// Outside radius
	float32		Radius2;	// Inside radius (mostly used to caluclate containment

	// C'tors
	__forceinline CSphere(const CVector3f& center, const float32 radius, const float32 radius2 = 0);
	__forceinline CSphere();

	__forceinline void __fastcall Set(const CVector3f& center, const float32 radius, const float32 radius2 = 0);
	__forceinline void __fastcall Set(float32 X, float32 Y, float32 Z, float32 R, const float32 R2 = 0);

	/// Reset the sphere to a point
	void		__fastcall	SetToPoint( const CVector3f& Point);
	CVector3f	__fastcall	ComputeCenter( ) const;


	/// Extend the sphere to enclose a point 
	void		__fastcall	ExtendToEnclose( const CVector3f& Point );

	/// Intersection tests
	bool		__fastcall	Intersects( const CSphere& B2 ) const;
	bool		__fastcall	IsInside( const CVector3f& Point ) const;

	// generate a random point inside the sphere with normal distribution
	__forceinline CVector3f __fastcall GenerateRandom ();

	// Modify the sphere to represent a sphere containing the volume of space
	//	 reflecting the intersection between 2 other spheres
	// If no intersection, returns FALSE and This is not modified.
	// If intersection, returns TRUE and fills This with the intersected sphere,
	// Result may be one of b1 or b2.
	bool		__fastcall	Intersection( const CSphere& B1, const CSphere& B2 );

	// Computes union of space between b1 and b2 and returns in this.
	void		__fastcall	Union( const CSphere& B1, const CSphere& B2 );

	// Translation
	CVector3f	__fastcall	GetTranslation ( ) const;
	void  		__fastcall	SetTranslation ( const CVector3f& NewCenter );
	void  		__fastcall	Translate      ( float32 DX, float32 DY, float32 DZ );

	// Scaling
	CVector3f	__fastcall	GetScaling  ( ) const;
	void  		__fastcall	SetScaling  ( const CVector3f& Scale );
	void  		__fastcall	SetScaling	( float32 Scale );
	void  		__fastcall	Scale		( float32 DX, float32 DY,float32 DZ );
	void  		__fastcall	Scale		( float32 Scale );
	
	//  Creates a sphere that encloses the entire area of a box(B1) that moves along linear path
	void		__fastcall	LinearSweep ( const CSphere& B1, const CVector3f& StartPoint, const CVector3f& EndPoint );

	// Collides a ray with sphere B.  The ray is directed, from Start to End.  
	//   Only returns a ray hitting the outside of the box.  
	//     on success, TRUE is returned, and 
	//       if T is non-NULL, T is returned as 0..1 where 0 is a collision at Start, and 1 is a collision at End
	//       if Normal is non-NULL, Normal is the surface normal of the sphere where the collision occured.
	bool  	__fastcall	RayCollision( const CVector3f& Start, const CVector3f& End, float32 *T, CVector3f* Normal ) const;

	// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
	bool __fastcall MovingPointCollision( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;

	// return the shortest distance between a point and the spheres surface
	float32 		__fastcall	DistanceSquaredToPoint ( const CVector3f& Pos ) const;

	// Transform the sphere by various matrix op's
	void		 	__fastcall	Transform( const CMatrix4f& XF );


	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );
};

#include "Sphere.inl"


} // namespace Math
} // namespace WT
