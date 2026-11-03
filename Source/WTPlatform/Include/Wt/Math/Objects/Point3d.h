//
//	Point3d.h
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


//	CPoint3d : represents a 3d point in space

typedef System::SmartPointer<class CPoint3d> HPoint3d;
class CPoint3d : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CPoint3d, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	// data members
	CVector3f	Center;		// location of the point

	// C'tors
	__forceinline CPoint3d(const CVector3f& center);
	__forceinline CPoint3d();

	__forceinline void __fastcall Set(const CVector3f& center);
	__forceinline void __fastcall Set(float32 X, float32 Y, float32 Z);

	// Intersection tests
	__forceinline bool __fastcall IsInside( const CVector3f& Point ) const;

	// generate a random vector inside the point (the point itself)
	__forceinline CVector3f __fastcall GenerateRandom ();

	// Translation
	__forceinline CVector3f	__fastcall	GetTranslation ( ) const;
	__forceinline void 		__fastcall	SetTranslation ( const CVector3f& NewCenter );
	__forceinline void 		__fastcall	Translate      ( float32 DX, float32 DY, float32 DZ );

	// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
	__forceinline bool __fastcall MovingPointCollision( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;


	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );
};

#include "Point3d.inl"


} // namespace Math
} // namespace WT
