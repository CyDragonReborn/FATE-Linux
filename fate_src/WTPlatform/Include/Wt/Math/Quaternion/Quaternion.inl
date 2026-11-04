//
//	Quaternion.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CQuaternion::CQuaternion()
{
    SetNoRotation();
}

__forceinline CQuaternion::CQuaternion(float32 xx, float32 yy, float32 zz, float32 ww)
{
	x = xx; y = yy; z = zz; w = ww;
	assert( IsValid() );
}

__forceinline CQuaternion::CQuaternion(const CVector3f& vec, float32 ww)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = ww;
	assert( IsValid() );
}

__forceinline CQuaternion::CQuaternion(const CVector3f& EulerAngles)
{
	SetFromEulerAngles( EulerAngles );
}

__forceinline CQuaternion::CQuaternion(float32 Pitch, float32 Yaw, float32 Roll)
{
	SetFromEulerAngles( Pitch, Yaw, Roll );
}

__forceinline bool     __fastcall    CQuaternion::operator == (const CQuaternion& q) const
{
	if (x != q.x || y != q.y || z != q.z || w != q.w)
	{
		return false;
	} 
	else
	{
		return true;
	}
}

__forceinline bool     __fastcall    CQuaternion::operator != (const CQuaternion& q) const
{
	if (x != q.x || y != q.y || z != q.z || w != q.w)
	{
		return true;
	} 
	else 
	{
		return false;
	}
}

__forceinline CQuaternion& __fastcall	CQuaternion::operator +=	(const CQuaternion& q)
{
	x+=q.x;	y+=q.y;	z+=q.z;	w+=q.w;
	return *this;
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator +	(const CQuaternion& q) const
{
	return CQuaternion(x+q.x, y+q.y, z+q.z, w+q.w);
}

__forceinline CQuaternion& __fastcall	CQuaternion::operator -=	(const CQuaternion& q)
{
	x-=q.x;	y-=q.y;	z-=q.z;	w-=q.w;
	return *this;
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator -	(const CQuaternion& q) const
{
	return CQuaternion(x-q.x, y-q.y, z-q.z, w-q.w);
}

__forceinline CQuaternion&  __fastcall	CQuaternion::operator *=(const CQuaternion& q)
{
	operator=( *this * q );
	return *this;
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator *	(const CQuaternion& q) const
{
	assert( IsValid() );
	assert( q.IsValid() );

	CQuaternion Result( false );
	Result.x = w * q.x + x * q.w - y * q.z + z * q.y;
	Result.y = w * q.y + x * q.z + y * q.w - z * q.x;
	Result.z = w * q.z - x * q.y + y * q.x + z * q.w;
    Result.w = w * q.w - x * q.x - y * q.y - z * q.z;

	assert( Result.IsValid() );
	return Result;
}

__forceinline CQuaternion&  __fastcall	CQuaternion::operator /=(const CQuaternion& q)
{
	operator=( *this / q );
	return *this;
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator /	(const CQuaternion& q) const
{
	assert( IsValid() );
	assert( q.IsValid() );

	CQuaternion Result( false );
	Result.x = x * q.w + y * q.z - z * q.y - w * q.x;
	Result.y = y * q.w + z * q.x - x * q.z - w * q.y;
	Result.z = x * q.y - y * q.x + z * q.w - w * q.z;
    Result.w = x * q.x + y * q.y + z * q.z + w * q.w;

	assert( Result.IsValid() );
	return Result;
}

__forceinline CQuaternion& __fastcall	CQuaternion::operator *=	(const float32 v)
{
	x *= v; y *= v; z *= v; w *= v;
	return *this;
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator *	(const float32 v) const
{
	return CQuaternion( x * v, y * v, z * v, w * v );
}

__forceinline CQuaternion& __fastcall	CQuaternion::operator /=	(const float32 v)
{
	operator*=( 1.0f / v );
	return *this;
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator /	(const float32 v) const
{
	return operator*( 1.0f / v );
}

__forceinline CQuaternion  __fastcall	CQuaternion::operator -	() const
{
	return CQuaternion( -x, -y, -z, -w );
}

__forceinline CVector3f  __fastcall CQuaternion::Rotate(const CVector3f& V) const
{
	CMatrix4f M( false );
	ToMatrix( &M );
	CVector3f Result( false );
	M.RotateNoOverlap( Result, V );
	return Result;
}

__forceinline bool __fastcall CQuaternion::IsValid() const
{
	if (w * w < 0.0f)
		return false;
	if (x * x  < 0.0f)
		return false;
	if (y * y < 0.0f)
		return false;
	if (z * z < 0.0f)
		return false;
	return true;
}

__forceinline bool		 __fastcall CQuaternion::IsUnit() const
{
	float32 const KEpsilon = 0.000173f;
	float32 const MagnitudeSquared = x * x + y * y + z * z + w * w;
	float32 const Error = MagnitudeSquared - 1.0f;
	
	return fabsf( Error ) <= 2.0f * KEpsilon;
}

__forceinline void		 __fastcall CQuaternion::SetNoRotation()
{
	x = y = z = 0.0f;
	w = 1.0f;
}

__forceinline void       __fastcall CQuaternion::Negate()
{
    x = -x;
    y = -y; 
    z = -z;
    w = -w;
}

__forceinline CQuaternion __fastcall CQuaternion::Inverse() const 
{
	return CQuaternion( -x, -y, -z, w );
}

__forceinline float32	 __fastcall CQuaternion::MagnitudeSquared() const
{
	return x * x + y * y + z * z + w * w;
}

__forceinline float32	 __fastcall CQuaternion::Dot(CQuaternion const& q) const
{
    return x * q.x + y * q.y + z * q.z + w * q.w;
}

__forceinline CQuaternion operator*( float32 s, CQuaternion const& q )
{
	return q * s;
}

__forceinline CQuaternion operator/( float32 s, CQuaternion const& q )
{
	CQuaternion Result( false );
	Result.w = s * q.w;
	s = -s;
	Result.x = s * q.x;
	Result.y = s * q.y;
	Result.z = s * q.z;
	return Result;
}

// D3DX support
__forceinline CQuaternion::CQuaternion(const D3DXQUATERNION& D3DXQuat)
{
	x = D3DXQuat.x;
	y = D3DXQuat.y;
	z = D3DXQuat.z;
	w = D3DXQuat.w;
}

__forceinline CQuaternion& __fastcall CQuaternion::operator=(const D3DXQUATERNION& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
	return *this;
}

__forceinline D3DXQUATERNION * __fastcall CQuaternion::GetD3DXQuatPtr()
{
	return reinterpret_cast<D3DXQUATERNION *>(this);
}
