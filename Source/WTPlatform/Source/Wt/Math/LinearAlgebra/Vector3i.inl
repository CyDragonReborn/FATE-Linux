//
//	Vector3i.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline bool __fastcall CVector3i::operator == (const CVector3i& v) const
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

__forceinline bool __fastcall CVector3i::operator != (const CVector3i& v) const
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

__forceinline CVector3i& __fastcall CVector3i::operator +=(const CVector3i& v)
{
	x += v.x; y += v.y; z += v.z;
}

__forceinline CVector3i __fastcall CVector3i::operator + (const CVector3i& v) const
{
	return CVector3i( v.x+x, v.y+y, v.z+z );
}

__forceinline CVector3i& __fastcall CVector3i::operator -=(const CVector3i& v)
{
	x -= v.x; y -= v.y; z -= v.z;
}

__forceinline CVector3i __fastcall CVector3i::operator - (const CVector3i& v) const
{
	return CVector3i( x-v.x, y-v.y, z-v.z );
}

__forceinline CVector3i& __fastcall CVector3i::operator +=(const int32 v)
{
	x += v; y += v; z += v;
}

__forceinline CVector3i __fastcall CVector3i::operator + (const int32 v) const
{
	return CVector3i( x+v, y+v, z+v );
}

__forceinline CVector3i& __fastcall CVector3i::operator -=(const int32 v)
{
	x -= v; y -= v; z -= v;
}

__forceinline CVector3i __fastcall CVector3i::operator - (const int32 v) const
{
	return CVector3i( x-v, y-v, z-v );
}

__forceinline CVector3i& __fastcall CVector3i::operator *=(const int32 v)
{
	x *= v; y *= v; z *= v;
}

__forceinline CVector3i __fastcall CVector3i::operator * (const int32 v) const
{
	return CVector3i( x*v, y*v, z*v );
}

__forceinline CVector3i& __fastcall CVector3i::operator /=(const int32 v)
{
	x /= v; y /= v; z /= v;
}

__forceinline CVector3i __fastcall CVector3i::operator / (const int32 v) const
{
	return CVector3i( x/v, y/v, z/v );
}

__forceinline CVector3i& __fastcall CVector3i::operator *=(const float32 v)
{
	x = Float32ToInt32( x * v );
	y = Float32ToInt32( y * v );
	z = Float32ToInt32( z * v );
	return *this;
}

__forceinline CVector3i __fastcall CVector3i::operator * (const float32 v) const
{
	return CVector3i( Float32ToInt32( x * v ), Float32ToInt32( y * v ), Float32ToInt32( z * v ) );
}

__forceinline CVector3i& __fastcall CVector3i::operator /=(const float32 v)
{
	float32 recip = 1.0f / v;
	x = Float32ToInt32( x * recip );
	y = Float32ToInt32( y * recip );
	z = Float32ToInt32( z * recip );
}

__forceinline CVector3i __fastcall CVector3i::operator / (const float32 v) const
{
	float32 recip = 1.0f / v;
	return CVector3i( Float32ToInt32( x * recip ), Float32ToInt32( y * recip ), Float32ToInt32( z * recip ) );
}

__forceinline CVector3i __fastcall CVector3i::operator -() const
{
	return CVector3i( -x, -y, -z );
}

__forceinline int32 __fastcall CVector3i::operator [] (const int32 x) const
{
	return e[x];
}

__forceinline int32& __fastcall CVector3i::operator [] (const int32 x)
{
	return e[x];
}

__forceinline void __fastcall CVector3i::Set(int32 X, int32 Y, int32 Z)
{
	x = X; y = Y; z = Z;
}

__forceinline void __fastcall CVector3i::Get(int32 *X, int32 *Y, int32 *Z) const
{
	*X = x; *Y = y; *Z = z;
}

__forceinline int32 __fastcall CVector3i::DotProduct(const CVector3i& V2) const
{
	return x*V2.x + y*V2.y + z*V2.z;
}

__forceinline int32 __fastcall CVector3i::CrossProduct(const CVector3i& V1, const CVector3i& V2)
{
	if( this == &V1 || this == &V2 )
	{
		CVector3i Result;
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
}

__forceinline int32 __fastcall CVector3i::LengthSquared() const
{
	return x*x + y*y + z*z;
}

__forceinline int32 __fastcall CVector3i::Length() const
{
	return Float32ToInt32( sqrtf( static_cast<float32>( LengthSquared() ) ) );
}

__forceinline int32 __fastcall CVector3i::Normalize()
{
	int32 Len = Length();
	float32 ILen = 1.0f / Len;
	*this *= ILen;
	return Len;
}

__forceinline void __fastcall CVector3i::Inverse()
{
	*this = -*this;
}

__forceinline int32 __fastcall CVector3i::DistanceBetweenSquared(const CVector3i& V2) const
{
	return (V2-*this).LengthSquared();
}

__forceinline int32 __fastcall CVector3i::DistanceBetween(const CVector3i& V2) const
{
	return Float32ToInt32( sqrtf( static_cast<float32>( DistanceBetweenSquared(V2) ) ) );
}

__forceinline void __fastcall CVector3i::Clear()
{
	x = y = z = 0;
}

__forceinline bool __fastcall CVector3i::IsNormalized()
{
	return LengthSquared() == 1;
}

__forceinline bool __fastcall CVector3i::Compare(const CVector3i& V2,int32 tolerance)
{
	int32 val = DistanceBetweenSquared(V2);
	return val <= (tolerance*tolerance);
}

__forceinline CVector3i operator*( int32 s, CVector3i const& v )
{
	return v * s;
}
