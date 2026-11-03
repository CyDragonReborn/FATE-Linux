//
//	Line.h
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
// CLine class, represents a CLine between 2 points
// in 3d space.
//

typedef System::SmartPointer<class CLine> HLine;
class CLine : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CLine, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CVector3f StartPoint;			// Start point
	CVector3f EndPoint;				// End point

	// Constructors

	__forceinline CLine () {}
	__forceinline CLine ( const CVector3f& AStartPoint , const CVector3f& AEndPoint );

	// public methods

	__forceinline void __fastcall Set ( const CVector3f& AStartPoint , const CVector3f& AEndPoint );
	__forceinline bool __fastcall IsInside ( const CVector3f& Point ) const;
	__forceinline CVector3f __fastcall GenerateRandom ();
	bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;


	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );
};

#include "Line.inl"


} // namespace Math
} // namespace WT
