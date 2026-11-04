//
//	CRect.h
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


namespace WT
{
namespace Math
{


//
// CRect class, represents a CRect in 3D space, using the origin
// and two edges (widht and Height)
//

typedef System::SmartPointer<class CRect> HRect;
class CRect : public CShape3d, public System::Serialization::ISerializable
{
public:

	WT_BEGIN_CLASS( CRect, CShape3d )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
	WT_END_CLASS()

	CVector3f	Origin;				// Distance from origin
	CVector3f	Width;				// Width
	CVector3f	Height;				// height

	// Constructors

	__forceinline CRect () : Distance(0) , WidthSqr(0) , HeightSqr(0) {}
	__forceinline CRect ( const CVector3f& AOrigin , const CVector3f& AWidth , const CVector3f& AHeight );

	// public methods

	__forceinline void __fastcall Set ( const CVector3f& AOrigin , const CVector3f& AWidth , const CVector3f& AHeight );
	__forceinline bool __fastcall IsInside ( const CVector3f& Point ) const;
	__forceinline CVector3f __fastcall GenerateRandom ();
	bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const;
	__forceinline CVector3f GetPointOn ( float32 U , float32 V ) const;


	// ISerializable

	void GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext ) const;

	void SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						System::Serialization::CSerializationContext* pContext );

private:

	__forceinline void __fastcall Update();

	CVector3f	Normal;
	float32		Distance;
	float32		WidthSqr;
	float32		HeightSqr;
};

#include "Rect.inl"


} // namespace Math
} // namespace WT
