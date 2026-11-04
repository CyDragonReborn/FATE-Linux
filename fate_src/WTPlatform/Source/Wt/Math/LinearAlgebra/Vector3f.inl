//
//	CVector3f.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline bool __fastcall CVector3f::operator == (const CVector3f& v) const
{
	if (x != v.x || y != v.y || z != v.z)
	{
		return false;
	} 
	else 
	{
		return true;
	}
}

__forceinline bool __fastcall CVector3f::operator != (const CVector3f& v) const
{
	if (x != v.x || y != v.y || z != v.z)
	{
		return true;
	} 
	else 
	{
		return false;
	}
}

__forceinline CVector3f& __fastcall CVector3f::operator +=(const CVector3f& v)
{
	x+=v.x; y+=v.y; z+=v.z;
	return *this;
}

__forceinline CVector3f __fastcall CVector3f::operator +	(const CVector3f& v) const
{
	return CVector3f(x+v.x, y+v.y, z+v.z);
}

__forceinline CVector3f& __fastcall CVector3f::operator -=(const CVector3f& v)
{
	x-=v.x; y-=v.y; z-=v.z;
	return *this;
}

__forceinline CVector3f __fastcall CVector3f::operator - (const CVector3f& v) const
{
	return CVector3f(x-v.x, y-v.y, z-v.z);
}


__forceinline CVector3f& __fastcall CVector3f::operator +=(const float32 v)
{
	x+=v; y+=v; z+=v;
	return *this;
}

__forceinline CVector3f __fastcall CVector3f::operator + (const float32 v) const
{
	return CVector3f(x+v, y+v, z+v);
}

__forceinline CVector3f& __fastcall CVector3f::operator -=(const float32 v)
{
	x-=v; y-=v; z-=v;
	return *this;
}

__forceinline CVector3f __fastcall CVector3f::operator - (const float32 v) const
{
	return CVector3f(x-v, y-v, z-v);
}

__forceinline CVector3f& __fastcall CVector3f::operator *=(const float32 v)
{
	x*=v; y*=v; z*=v;
	return *this;
}

__forceinline CVector3f __fastcall CVector3f::operator * (const float32 v) const
{
	return CVector3f(x*v, y*v, z*v);
}

__forceinline CVector3f& __fastcall CVector3f::operator /=(const float32 v)
{
	return operator *=(1.0f/v);
}

__forceinline CVector3f __fastcall CVector3f::operator / (const float32 v) const
{
	return operator *(1.0f/v);
}

__forceinline float32 const& __fastcall CVector3f::operator [] (const int32 x) const
{
	return e[x];
}

__forceinline float32& __fastcall CVector3f::operator [] (const int32 x)
{
	return e[x];
}

__forceinline CVector3f __fastcall CVector3f::operator - () const
{
	return CVector3f(-x, -y, -z);
}

__forceinline void __fastcall CVector3f::Set(float32 X, float32 Y, float32 Z)
{
	x=X; y=Y; z=Z;
}

__forceinline void __fastcall CVector3f::Get(float32 *X, float32 *Y, float32 *Z) const
{
	*X=x; *Y=y; *Z=z;
}

__forceinline float32 __fastcall CVector3f::DotProduct(const CVector3f& V2) const
{
	return (x * V2.x + y * V2.y + z * V2.z);
}

__forceinline CVector3f& __fastcall CVector3f::CrossProduct(const CVector3f& V1, const CVector3f& V2)
{
	if( this == &V1 || this == &V2 )
	{
		CVector3f Result;
		Result.x = V1.y * V2.z - V1.z * V2.y;
		Result.y = V1.z * V2.x - V1.x * V2.z;
		Result.z = V1.x * V2.y - V1.y * V2.x;
		x = Result.x; y = Result.y; z = Result.z;
	}
	else
	{
		x = V1.y * V2.z - V1.z * V2.y;
		y = V1.z * V2.x - V1.x * V2.z;
		z = V1.x * V2.y - V1.y * V2.x;
	}

	return *this;
}

__forceinline void __fastcall CVector3f::Scale3d( float32 Scale )
{	
	x *= Scale; y *= Scale; z *= Scale;
}

__forceinline void __fastcall CVector3f::Subtract3d(const CVector3f& V1, const CVector3f& V2)
{
	x = V1.x - V2.x;
	y = V1.y - V2.y;
	z = V1.z - V2.z;
}

__forceinline void __fastcall CVector3f::Add3d(const CVector3f& V1, const CVector3f& V2 )
{
	x = V1.x + V2.x;
	y = V1.y + V2.y;
	z = V1.z + V2.z;
}

__forceinline void __fastcall CVector3f::AddScaled3d(const CVector3f& V1, const CVector3f& V2, float32 V2Scale)
{
	x = V1.x + V2.x * V2Scale;
	y = V1.y + V2.y * V2Scale;
	z = V1.z + V2.z * V2Scale;
}

__forceinline float32 __fastcall CVector3f::RayDotProduct(const CVector3f& RayStart, const CVector3f& RayEnd) // this * (S - E)
{
	return (x * (RayEnd.x - RayStart.x) +
			y * (RayEnd.y - RayStart.y) +
			z * (RayEnd.z - RayStart.z));
}

__forceinline float32 __fastcall CVector3f::LengthSquared() const
{
	return ( x * x + y * y + z * z );
}

__forceinline float32 __fastcall CVector3f::Length() const
{
	return sqrtf( LengthSquared() );
}

__forceinline float32 __fastcall CVector3f::Normalize()
{
	float32 Dist = Length();
	if (Dist == 0.0f)
	{
		return 0.0f;
	}
	
	float32 OneOverDist = 1.0f / Dist;
	x *= OneOverDist;
	y *= OneOverDist;
	z *= OneOverDist;
	
	return Dist;
}

__forceinline void __fastcall CVector3f::Inverse()
{
	x = -x;
	y = -y;
	z = -z;
}

__forceinline float32 __fastcall CVector3f::DistanceBetweenSquared(const CVector3f& V2) const
{
	float32 d,s;
	s = (x - V2.x);
	d = s * s;
	s = (y - V2.y);
	d += s * s;
	s = (z - V2.z);
	d += s * s;
	return d;
}

__forceinline float32 __fastcall CVector3f::DistanceBetween(const CVector3f& V2) const
{
	return sqrtf( DistanceBetweenSquared(V2) );
}

__forceinline void __fastcall CVector3f::Clear()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

__forceinline void __fastcall CVector3f::Rand()
{
	x = Math_Randf();
	y = Math_Randf();
	z = Math_Randf();
}

__forceinline CVector3f operator*( float32 s, CVector3f const& v )
{
	return v * s;
}

// D3D support
__forceinline CVector3f::CVector3f(const D3DVECTOR& D3DSource)
{
	x = D3DSource.x;
	y = D3DSource.y;
	z = D3DSource.z;
}

__forceinline CVector3f& __fastcall CVector3f::operator=(const D3DVECTOR& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

__forceinline D3DVECTOR * __fastcall CVector3f::GetD3DVectorPtr()
{
	return reinterpret_cast<D3DVECTOR *>( e );
}

__forceinline D3DVECTOR const* __fastcall CVector3f::GetD3DVectorPtr() const
{
	return reinterpret_cast<D3DVECTOR const*>( e );
}

__forceinline __fastcall CVector3f::operator D3DVECTOR*()
{
	return reinterpret_cast<D3DVECTOR *>( e );
}

__forceinline __fastcall CVector3f::operator const D3DVECTOR*() const
{
	return reinterpret_cast<const D3DVECTOR *>( e );
}

// D3DX support
__forceinline CVector3f::CVector3f(const D3DXVECTOR3& D3DXSource)
{
	x = D3DXSource.x;
	y = D3DXSource.y;
	z = D3DXSource.z;
}

__forceinline CVector3f& __fastcall CVector3f::operator=(const D3DXVECTOR3& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	return *this;
}

__forceinline D3DXVECTOR3* __fastcall CVector3f::GetD3DXVector3Ptr()
{
	return reinterpret_cast<D3DXVECTOR3*>( e );
}

__forceinline D3DXVECTOR3 const* __fastcall CVector3f::GetD3DXVector3Ptr() const
{
	return reinterpret_cast<D3DXVECTOR3 const*>( e );
}
