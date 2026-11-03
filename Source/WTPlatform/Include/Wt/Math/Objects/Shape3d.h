//
//	Shape3d.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/System/Object.h"


namespace WT
{
namespace Math
{


//
//	CShape3d :
//		represents a 3d shape in space (virtual class)

typedef System::SmartPointer<class CShape3d> HShape3d;
class CShape3d : public System::CObject
{
public:

	WT_ABSTRACT_CLASS(CShape3d, System::CObject)

	///Checks if the supplied vector is inside the bounding volume of this object
	virtual bool __fastcall IsInside ( const CVector3f& V ) const = 0;

	///Generates a random vector for this bounding object
	virtual CVector3f __fastcall GenerateRandom () = 0;

	///checks if a point starting from Start will collide with the object traveling a the given velocity
	///within the given frame time. Will return true if collide as well as the normal velocity
	///and the tangential velocity.
	virtual bool __fastcall MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const = 0;

};

} // namespace Math
} // namespace WT
