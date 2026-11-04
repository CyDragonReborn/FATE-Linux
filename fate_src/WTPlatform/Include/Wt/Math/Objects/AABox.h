//
//	AABox.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Math/Objects/Shape3d.h"

namespace WT
{
namespace Math
{

class CSphere;

// Data class encapsulating an axially alligned bounding box

typedef System::SmartPointer<class CAABox> HAABox;
class CAABox : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CAABox, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	// data members
	CVector3f Min;
	CVector3f Max;

	// C'tors
	__forceinline CAABox(const CVector3f& mmin, const CVector3f& mmax);
	__forceinline CAABox();

	__forceinline bool  __fastcall  operator==( CAABox const& Box ) const;
	__forceinline bool  __fastcall  operator!=( CAABox const& Box ) const;

				  void	__fastcall	Reset();

    __forceinline bool  __fastcall  IsEmpty() const;

	/// Set corner values, ensures validity
	__forceinline void	__fastcall	Set(const CVector3f& mmin, const CVector3f& mmax);
	__forceinline void	__fastcall	Set(float32 X1, float32 Y1, float32 Z1, float32 X2, float32 Y2, float32 Z2);

	/// Set corner values, no validity check
	__forceinline void	__fastcall	SetDirect(const CVector3f& mmin, const CVector3f& mmax);
	__forceinline void	__fastcall	SetDirect(float32 X1, float32 Y1, float32 Z1, float32 X2, float32 Y2, float32 Z2);

	/// Modify to enclose point
	__forceinline void	__fastcall	ExtendToEnclose( const CVector3f& Point );

	/// Reset the box to a point
	void		__fastcall	SetToPoint( const CVector3f& Point);

	CVector3f	__fastcall	ComputeCenter() const;
	
	/// Does the box intersect another box
	bool		__fastcall	Intersects( const CAABox& B2 ) const;

	/// Generate a box representing the volume of space which is the intersection
	///	 of 2 other boxes
	bool		__fastcall	Intersection( const CAABox& B1, const CAABox& B2 );

	/// Generate a box representing the volume of space which is the union
	///	 of 2 other boxes
	void		__fastcall	Union( const CAABox& B1, const CAABox& B2 );

	/// Containment tests
	bool		__fastcall	ContainsPoint( const CVector3f& Point ) const;
	bool		__fastcall	ContainsBox( const CAABox& b2) const;
	
	/// Reset the center of the box while preserving volume
	void  		__fastcall	SetCenter( const CVector3f& Center );
	void  		__fastcall	Translate      ( float32 DX, float32 DY, float32 DZ );

	/// Get/Set the volume of the box while preserving center
	CVector3f	__fastcall	GetScaling     ( ) const;
	void  		__fastcall	SetScaling     ( const CVector3f& Scale );
	void  		__fastcall	Scale          ( float32 DX, float32 DY,float32 DZ );

	/// Generate a box representing the space swept by BoxToSweep from Start to End
	void		__fastcall	LinearSweep ( const CAABox& BoxToSweep, const CVector3f& StartPoint, const CVector3f& EndPoint );

	/// Collide a ray against the box. If collision has occured, T receives the distance allong the ray collision occured,
	///  and Normal receives the normal of the collision point
	bool  	__fastcall	RayCollision( const CVector3f& Start, const CVector3f& End, float32 *T, CVector3f& Normal ) const;
	
	/// Shortest distance from a point to any side of the box
	float32 	__fastcall	DistanceSquaredToPoint ( const CVector3f& Pos ) const;

	/// Matrix transformation Ops
	void	  	__fastcall	Transform		( const CMatrix4f& XF );
	void		__fastcall	Rotate			( const CMatrix4f& XF );

	/// Convert the ext box into an ext sphere
	void		__fastcall	GetExtSphere(CSphere& Sphere) const;

	// check if the provided point is inside the box
	__forceinline bool __fastcall IsInside ( const CVector3f& Point ) const;

	// check for collision for a moving point with the box within a given timeframe, the point is moving from start at velocity
	bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;

	// generate a evenly distributed random point within the box
	__forceinline CVector3f __fastcall GenerateRandom();

	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private:

	bool		__fastcall XFaceDist( const CVector3f& Start, const CVector3f& Delta, float32 *T, float32 X) const;
	bool		__fastcall YFaceDist( const CVector3f& Start, const CVector3f& Delta, float32 *T, float32 X) const;
	bool		__fastcall ZFaceDist( const CVector3f& Start, const CVector3f& Delta, float32 *T, float32 X) const;

};

} // namespace Math
} // namespace WT

#include "AABox.inl"
