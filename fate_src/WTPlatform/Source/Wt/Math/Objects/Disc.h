//
//	Disc.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Math/Utils/MathUtil.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/Objects/Shape3d.h"
#include <math.h>


namespace WT
{
namespace Math
{


//
// CDisc class, represents a CDisc (or a flat ring)
// in 3d space, using a center location, a unit normal and 2 radii
// Radius1 is the outside radius, and Radius2 is the inside, if
// Radius2 is 0 then it is a CDisc, otherwise it represents a ring
//

typedef System::SmartPointer<class CDisc> HDisc;
class CDisc : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CDisc, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CVector3f		Center;				/// Distance from origin
	CVector3f		Normal;				/// Unit Orientation
	float32			Radius;				///	CDisc radius
	float32			Radius2;			/// inside radius (mostly used for containment tests

	// Constructors

	__forceinline CDisc () : Radius(0) , Radius2(0) , RadiusSqr(0) , Radius2Sqr(0) , RealRadiusSqr(0) {}
	__forceinline CDisc ( const CVector3f& ACenter , const CVector3f& ANormal , float32 ARadius , float32 ARadius2 = 0 );

	// public methods

	__forceinline void __fastcall Set ( const CVector3f& ACenter , const CVector3f& ANormal , float32 ARadius , float32 ARadius2 = 0 );
	__forceinline bool __fastcall IsInside ( const CVector3f& Point ) const;
	__forceinline CVector3f __fastcall GenerateRandom ();
	bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;

	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private:

	__forceinline void __fastcall Update();

	float32 RadiusSqr;
	float32 Radius2Sqr;
	float32 RealRadiusSqr;
	CVector3f U;
	CVector3f V;
};

#include "Disc.inl"


} // namespace Math
} // namespace WT
