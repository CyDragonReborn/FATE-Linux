//
//	IMovable.inl
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//

#pragma once

#include "WT/Graphics/IMovable.h"

namespace WT
{
namespace Graphics
{

void IMovable::SetPositionComponent( Math::CVector3f const& Translation )
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.RowT.CVector3f::operator=( Translation );
	SetTransform( Transform );
}

void IMovable::SetPositionComponent( float32 x, float32 y, float32 z )
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.RowT.x = x;
	Transform.RowT.y = y;
	Transform.RowT.z = z;
	SetTransform( Transform );
}

void IMovable::SetXRotation (float32 RadianAngle)
{
	Math::CMatrix4f Transform( false );
	Transform.SetXRotation( RadianAngle );
	SetTransform( Transform );
}

void IMovable::SetYRotation (float32 RadianAngle)
{
	Math::CMatrix4f Transform( false );
	Transform.SetYRotation( RadianAngle );
	SetTransform( Transform );
}

void IMovable::SetZRotation (float32 RadianAngle)
{
	Math::CMatrix4f Transform( false );
	Transform.SetZRotation( RadianAngle );
	SetTransform( Transform );
}

void IMovable::SetAxisRotation(const CVector3f& Axis, float32 RadianAngle)
{
	Math::CMatrix4f Transform( false );
	Transform.SetAxisRotation( Axis, RadianAngle );
	SetTransform( Transform );
}

void IMovable::SetTranslation(const CVector3f &V)
{
	Math::CMatrix4f Transform( false );
	Transform.SetTranslation( V );
	SetTransform( Transform );
}

void IMovable::SetTranslation(float32 x, float32 y, float32 z)
{
	Math::CMatrix4f Transform( false );
	Transform.SetTranslation( x, y, z );
	SetTransform( Transform );
}

void IMovable::SetScale(const CVector3f &V)
{
	Math::CMatrix4f Transform( false );
	Transform.SetScale( V );
	SetTransform( Transform );
}

void IMovable::SetScale(float32 x, float32 y, float32 z)
{
	Math::CMatrix4f Transform( false );
	Transform.SetScale( x, y, z );
	SetTransform( Transform );
}

void IMovable::PreRotateX(float32 RadianAngle)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreRotateX( RadianAngle );
	SetTransform( Transform );
}

void IMovable::PreRotateY(float32 RadianAngle)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreRotateY( RadianAngle );
	SetTransform( Transform );
}

void IMovable::PreRotateZ(float32 RadianAngle)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreRotateZ( RadianAngle );
	SetTransform( Transform );
}

void IMovable::PreTranslate(float32 x, float32 y, float32 z)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreTranslate( x, y, z );
	SetTransform( Transform );
}

void IMovable::PreTranslate(const Math::CVector3f& V)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreTranslate( V.x, V.y, V.z );
	SetTransform( Transform );
}

void IMovable::PreScale(float32 x, float32 y, float32 z)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreScale( x, y, z );
	SetTransform( Transform );
}

void IMovable::PreScale(const Math::CVector3f& V)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PreScale( V.x, V.y, V.z );
	SetTransform( Transform );
}

void IMovable::PostRotateX(float32 RadianAngle)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PostRotateX( RadianAngle );
	SetTransform( Transform );
}

void IMovable::PostRotateY(float32 RadianAngle)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PostRotateY( RadianAngle );
	SetTransform( Transform );
}

void IMovable::PostRotateZ(float32 RadianAngle)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PostRotateZ( RadianAngle );
	SetTransform( Transform );
}

void IMovable::PostTranslate(float32 x, float32 y, float32 z)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PostTranslate( x, y, z );
	SetTransform( Transform );
}

void IMovable::PostTranslate(const Math::CVector3f& V)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PostTranslate( V );
	SetTransform( Transform );
}

void IMovable::PostScale(const Math::CVector3f& S)
{
	Math::CMatrix4f Transform( GetTransform() );
	Transform.PostScale( S );
	SetTransform( Transform );
}

void IMovable::SetLookAt( const Math::CVector3f& Origin, const Math::CVector3f& Target, const Math::CVector3f& Up )
{
	Math::CMatrix4f Transform( false );
	Transform.SetLookAt( Origin, Target, Up );
	SetTransform( Transform );
}

void IMovable::LookAt( const Math::CVector3f& Target, CVector3f const& Up )
{
	Math::CMatrix4f CurTransform( GetTransform() );
	Math::CMatrix4f Transform( false );
	Transform.SetLookAt( CurTransform.RowT, Target, Up );
	SetTransform( Transform );
}

}
}
