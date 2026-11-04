/***************************************************************************************
 *
 * IMovable.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          9/29/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/IInterface.h"
#include "WT/Math/Math.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Math/MathSerialization.h"

using namespace WT::Math;

namespace WT
{
namespace Graphics
{

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/*
IMovable is an interface for objects that are movable in world space and have
a 4x4 matrix describing the object's model-to-world transformation. "model-to-world" means 
that a coordinate in model space, multiplied by the transform in the usual order 
(vector times matrix), yields a worldspace coordinate.

IMovable also supports a component representation for the linear portion of the
transformation (i.e. excluding the translation). This representation is accessible
through the GetScaleComponent(), SetScaleComponent(), GetRotationComponent(), and 
SetRotationComponent() functions.

Calling SetScaleComponent() or SetRotationComponent() will cause the linear portion of the 
transformation to be set to the combination of the scale and rotation. However, most IMovable 
mutators will not update the scale and rotation components. Call ComputeTransformComponents() 
if you need to compute scale and rotation components based on the current matrix.
*/

class IMovable : public System::IInterface
{
public:
	WT_BEGIN_INTERFACE( IMovable );
		WT_BEGIN_PROPERTIES();
			WT_PROP_RW_BYREF( Transform, CMatrix4f );
			WT_PROP_RW_BYREF( ScaleComponent, CVector3f );
			WT_PROP_RW_BYREF( RotationComponent, CQuaternion );
		WT_END_PROPERTIES();
	WT_END_INTERFACE();

	/// Get the model-to-world transform.
	virtual Math::CMatrix4f const& GetTransform() const = 0;

	/// Set the explicit model-to-world transform.
	virtual void SetTransform( Math::CMatrix4f const& Transform ) = 0;

	virtual CVector3f const& GetScaleComponent() const = 0;
	virtual void SetScaleComponent( CVector3f const& Scale ) = 0;
	virtual void SetScaleComponent( float32 x, float32 y, float32 z ) = 0;

	virtual CQuaternion const& GetRotationComponent() const = 0;
	virtual void SetRotationComponent( CQuaternion const& Rotation ) = 0;
	virtual void SetRotationComponent( Math::CVector3f const& Axis, float32 Angle ) = 0;
	virtual void SetRotationComponent( CMatrix4f const& RotationMatrix ) = 0;
	virtual void SetRotationComponent( Math::CVector3f const& Forward, Math::CVector3f const& Up ) = 0;
	virtual void SetRotationComponent( CVector3f const& EulerAngles ) = 0;
	virtual void SetRotationComponent( float32 Pitch, float32 Yaw, float32 Roll ) = 0;

	CVector3f const& GetPositionComponent() const {return GetTransform().RowT;}
	inline void SetPositionComponent( Math::CVector3f const& Position );
	inline void SetPositionComponent( float32 x, float32 y, float32 z );

	//Compute a component representation based on the current
	//transform matrix. As a unique component representation does
	//not exist, one is chosen arbitrarily. This function may throw 
	//if the matrix is singular. Also, if the matrix has shear, then 
	//the resulting component representation will only be an 
	//approximation.
	virtual void ComputeTransformComponents() = 0;
	
	//Set the entire transformation
	inline void SetXRotation (float32 RadianAngle);
	inline void SetYRotation (float32 RadianAngle);
	inline void SetZRotation (float32 RadianAngle);
	inline void SetAxisRotation(const CVector3f& Axis, float32 RadianAngle);
	inline void SetTranslation(const CVector3f &V);
	inline void SetTranslation(float32 x, float32 y, float32 z);
	inline void SetScale(const CVector3f &V);
	inline void SetScale(float32 x, float32 y, float32 z);

	//Set the entire transformation. Place the object at the specified origin and rotate the object 
	//to point at the target. The specified Up direction is matched as closely as possible.
	inline void SetLookAt( const Math::CVector3f& Origin, const Math::CVector3f& Target, const Math::CVector3f& Up = CVector3f::gm_YAxis );

	//Model-space transformation. Pre-multiply by transformation T, i.e. M' = TM.
	inline void PreRotateX(float32 RadianAngle);  
	inline void PreRotateY(float32 RadianAngle);
	inline void PreRotateZ(float32 RadianAngle);
	inline void PreTranslate(float32 x, float32 y, float32 z);	
	inline void PreTranslate(const Math::CVector3f& V);
	inline void PreScale(float32 x, float32 y, float32 z);
	inline void PreScale(const Math::CVector3f& V);

	//World-space transformation. Post-multiply by transformation T, i.e. M' = MT.
	inline void PostRotateX(float32 RadianAngle);  
	inline void PostRotateY(float32 RadianAngle);
	inline void PostRotateZ(float32 RadianAngle);
	inline void PostTranslate(float32 x, float32 y, float32 z);
	inline void PostTranslate(const Math::CVector3f& V);
	inline void PostScale(const Math::CVector3f& S);

	//Rotate to point at the target. The specified Up direction is matched as closely as possible.
	//The current positional component is unchanged.
	inline void LookAt( const Math::CVector3f& Target, CVector3f const& Up = CVector3f::gm_YAxis );
};

class CMovableImpl : public IMovable
{
public:
	CMovableImpl()
		: m_ScaleComponent( Math::CVector3f::gm_UnitScale )
	{
	}

	/// Get the model-to-world transform.
	virtual Math::CMatrix4f const& GetTransform() const {return m_Transform;}

	/// Set the explicit model-to-world transform.
	virtual void SetTransform( Math::CMatrix4f const& Transform ) {m_Transform = Transform;}

	virtual CVector3f const& GetScaleComponent() const {return m_ScaleComponent;}
	virtual void SetScaleComponent( Math::CVector3f const& Scale );
	virtual void SetScaleComponent( float32 x, float32 y, float32 z );

	virtual CQuaternion const& GetRotationComponent() const {return m_RotationComponent;}
	virtual void SetRotationComponent( Math::CQuaternion const& Rotation );
	virtual void SetRotationComponent( Math::CVector3f const& Axis, float32 Angle );
	virtual void SetRotationComponent( Math::CMatrix4f const& RotationMatrix );
	virtual void SetRotationComponent( Math::CVector3f const& Forward, Math::CVector3f const& Up );
	virtual void SetRotationComponent( Math::CVector3f const& EulerAngles );
	virtual void SetRotationComponent( float32 Pitch, float32 Yaw, float32 Roll );

	//Compute a component representation based on the current
	//transform matrix. As a unique component representation does
	//not exist, one is chosen arbitrarily. This function may throw 
	//if the matrix is singular. Also, if the matrix has shear, then 
	//the resulting component representation will only be an 
	//approximation.
	virtual void ComputeTransformComponents();

private:
	Math::CMatrix4f     m_Transform;
	Math::CQuaternion	m_RotationComponent;
	Math::CVector3f		m_ScaleComponent;

	//Update m_Transform based on m_ScaleComponent and m_RotationComponent
	void ComputeTransform();
};

} //namespace Graphics
} //namespace WT

#include "WT/Graphics/IMovable.inl"
