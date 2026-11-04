/***************************************************************************************
 *
 * IMovable.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 9/29/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"

#include "WT/Graphics/IMovable.h"
#include "WT/Math/MathSerialization.h"

using namespace WT::Math;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( IMovable )

void CMovableImpl::ComputeTransformComponents()
{
	float32 const Det = m_Transform.ComputeDeterminant3x3();
	if (Det == 0.0f)
	{
		throw std::invalid_argument( "singular matrix" );
	}

	m_ScaleComponent.x = m_Transform.RowX.Length();
	m_ScaleComponent.y = m_Transform.RowY.Length();
	m_ScaleComponent.z = m_Transform.RowZ.Length();

	//Check for negative determinant - this indicates a reflection, i.e. negative scaling.
	if (Det < 0.0f)
	{
		//We need to negate an odd number of scale factors (one or all three).
		//Different choices will result in different rotations below, but the
		//reconstructed transform matrix will be essentially equivalent.
		m_ScaleComponent = -m_ScaleComponent;
	}

	CMatrix4f Rotation( false );
	Rotation.RowX = m_Transform.RowX / m_ScaleComponent.x;
	Rotation.RowY = m_Transform.RowY / m_ScaleComponent.y;
	Rotation.RowZ = m_Transform.RowZ / m_ScaleComponent.z;
	m_RotationComponent.FromMatrix( Rotation );
}

void CMovableImpl::SetScaleComponent( CVector3f const& Scale )
{
	m_ScaleComponent = Scale;
	ComputeTransform();
}

void CMovableImpl::SetScaleComponent( float32 x, float32 y, float32 z )
{
	m_ScaleComponent.x = x;
	m_ScaleComponent.y = y;		
	m_ScaleComponent.z = z;
	ComputeTransform();
}

void CMovableImpl::SetRotationComponent( CQuaternion const& Rotation )
{
	m_RotationComponent = Rotation; ComputeTransform();
}
	
void CMovableImpl::SetRotationComponent( Math::CVector3f const& Axis, float32 Angle )
{
	m_RotationComponent.SetFromAxisAngle( Axis, Angle );
	ComputeTransform();
}

void CMovableImpl::SetRotationComponent( CMatrix4f const& RotationMatrix )
{
	m_RotationComponent.FromMatrix( RotationMatrix );
	ComputeTransform();
}

void CMovableImpl::SetRotationComponent( Math::CVector3f const& Forward, Math::CVector3f const& Up )
{
	CMatrix4f RotationMatrix( false );
	RotationMatrix.SetLookAt( CVector3f::gm_Zero, Forward, Up );
	SetRotationComponent( RotationMatrix );
}

void CMovableImpl::SetRotationComponent( CVector3f const& EulerAngles )
{
	m_RotationComponent.SetFromEulerAngles( EulerAngles );
	ComputeTransform();
}

void CMovableImpl::SetRotationComponent( float32 Pitch, float32 Yaw, float32 Roll )
{
	m_RotationComponent.SetFromEulerAngles( Pitch, Yaw, Roll );
	ComputeTransform();
}

void CMovableImpl::ComputeTransform()
{
	CMatrix4f Transform( false );
	m_RotationComponent.ToMatrix( &Transform );
	Transform.PreScale( m_ScaleComponent );
	Transform.RowT = GetTransform().RowT;
	SetTransform( Transform );
}

}
}
