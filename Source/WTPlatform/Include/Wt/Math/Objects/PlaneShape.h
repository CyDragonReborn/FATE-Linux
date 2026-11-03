//
//	PlaneShape.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/Math/LinearAlgebra/Plane.h"
#include "WT/Math/Objects/Shape3d.h"


namespace WT
{
namespace Math
{


//
//	CPlaneShape
//		Class encapsulating data and functionality
//		related to the manipulation of a plane
//


//
//	CPlaneShape flags and enumerated values
//

//	EPlaneType just simply says what axis the plane is aligned on.
//	It does not say what direction it is facing on the aligned axis.
//	DON'T change these numbers!!!!  Doing so will result in BAD things!!!
//	The reason, is code uses these enums to index into the X,Y,Z elements:
//		Val = Vec3d_GetElement(&CPlane->Normal, CPlane->Type%Type_AnyX);
namespace EPlaneType
{
	enum Type
	{
		X=0,				// X aligned
		Y=1,				// Y aligned
		Z=2,				// Z aligned
		AnyX=3,				// Arbitrary, X dominent axis
		AnyY=4,				// Arbitrary, Y dominent axis
		AnyZ=5,				// Arbitrary, Z dominent axis
		Any=6,				// Arbitrary (Any axis)
	};
}

// ---------------------------------------------------
//	CPlaneShape class
// ---------------------------------------------------

typedef System::SmartPointer<class CPlaneShape> HPlaneShape;
class CPlaneShape : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CPlaneShape, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()
	
	CPlane m_Plane;

	// C'tors
	CPlaneShape() : m_Plane( CVector3f::gm_YAxis ) {};
	
	// Set from normal, position on plane
	CPlaneShape( const CVector3f& norm, const CVector3f& position ) : m_Plane( norm, position ) {}

	// Static method, get the alligned planed type matching this unit vector
	static EPlaneType::Type	__fastcall TypeFromUnitVector(const CVector3f *V1);

	__forceinline bool __fastcall IsInside ( const CVector3f& Point ) const;

	// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
	bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;

	__forceinline CVector3f __fastcall GenerateRandom();

	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

public:	// really protected, bug@@
	EPlaneType::Type	Type;				// EPlaneType

};

__forceinline bool __fastcall CPlaneShape::IsInside ( const CVector3f& Point ) const
{
	return m_Plane.IsInside( Point );
}

__forceinline CVector3f __fastcall CPlaneShape::GenerateRandom()
{
	return m_Plane.Normal;
}

} // namespace Math
} // namespace WT
