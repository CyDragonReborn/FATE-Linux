//
//	Vector2i.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "math.h"

namespace WT
{
namespace Math
{

__forceinline bool __fastcall CVector2i::operator == (const CVector2i& v) const
{
	if (x != v.x || y != v.y)
	{
		return false;
	} 
	else 
	{
		return true;
	}
}

__forceinline bool __fastcall CVector2i::operator != (const CVector2i& v) const
{
	if (x != v.x || y != v.y)
	{
		return true;
	} 
	else 
	{
		return false;
	}
}

__forceinline CVector2i& __fastcall CVector2i::operator +=(const CVector2i& v)
{
	x+=v.x; y+=v.y;
	return *this;
}

__forceinline CVector2i  __fastcall CVector2i::operator +	(const CVector2i& v) const
{
	return CVector2i(x+v.x, y+v.y);
}

__forceinline CVector2i& __fastcall CVector2i::operator -=(const CVector2i& v)
{
	x-=v.x; y-=v.y;
	return *this;
}

__forceinline CVector2i  __fastcall CVector2i::operator -	(const CVector2i& v) const
{
	return CVector2i(x-v.x, y-v.y);
}


__forceinline CVector2i& __fastcall CVector2i::operator +=(const int32 v)
{
	x+=v; y+=v;
	return *this;
}

__forceinline CVector2i __fastcall CVector2i::operator +	(const int32 v) const
{
	return CVector2i(x+v, y+v);
}

__forceinline CVector2i& __fastcall CVector2i::operator -=(const int32 v)
{
	x-=v; y-=v;
	return *this;
}

__forceinline CVector2i __fastcall CVector2i::operator -	(const int32 v) const
{
	return CVector2i(x-v, y-v);
}

__forceinline CVector2i& __fastcall CVector2i::operator *=(const int32 v)
{
	x*=v; y*=v; 
	return *this;
}

__forceinline CVector2i __fastcall CVector2i::operator *	(const int32 v) const
{
	return CVector2i(x*v, y*v);
}

__forceinline CVector2i& __fastcall CVector2i::operator /=(const int32 v)
{
	x /= v;
	y /= v;
	return *this;
}

__forceinline CVector2i __fastcall CVector2i::operator /	(const int32 v) const
{
	return CVector2i( x / v, y / v );
}


__forceinline CVector2i& __fastcall CVector2i::operator *=(const float32 v)
{
	x = Float32ToInt32( x * v );
	y = Float32ToInt32( y * v ); 
	return *this;
}

__forceinline CVector2i __fastcall CVector2i::operator *	(const float32 v) const
{
	return CVector2i( Float32ToInt32( x * v ), Float32ToInt32( y * v ) );
}

__forceinline CVector2i& __fastcall CVector2i::operator /=(const float32 v)
{
	return operator *=(1.0f/(float32)v);
}

__forceinline CVector2i __fastcall CVector2i::operator /	(const float32 v) const
{
	return operator *(1.0f/v);
}

__forceinline int32 __fastcall CVector2i::operator [] (const int32 x) const
{
	return e[x];
}

__forceinline int32& __fastcall CVector2i::operator [] (const int32 x)
{
	return e[x];
}

__forceinline CVector2i __fastcall CVector2i::operator -	() const
{
	return CVector2i(-x, -y);
}

__forceinline void __fastcall CVector2i::Set(int32 X, int32 Y)
{
	x=X; y=Y;
}

__forceinline void __fastcall CVector2i::Get(int32 *X, int32 *Y) const
{
	*X=x; *Y=y;
}

__forceinline int32 __fastcall CVector2i::DotProduct(const CVector2i& V2) const
{
	return x*V2.x + y*V2.y;
}

__forceinline int32 __fastcall CVector2i::CrossProduct(const CVector2i& V2)
{
	return x*V2.y - y*V2.x;
}

__forceinline int32 __fastcall CVector2i::LengthSquared() const
{
	return ( x*x + y*y );
}

__forceinline int32 __fastcall CVector2i::Length() const
{
	return Float32ToInt32( sqrtf( static_cast<float32>( LengthSquared() ) ) );
}

__forceinline int32 __fastcall CVector2i::Normalize()
{
	float32 OneOverDist;
	int32 Dist;
	
	Dist = Length();
	
	if (Dist == 0)
		return 0;
	
	OneOverDist = 1.0f / Dist;
	
	x = Float32ToInt32( x * OneOverDist );
	y = Float32ToInt32( y * OneOverDist );
	
	return Dist;
}

__forceinline void __fastcall CVector2i::Inverse()
{
	x = -x; y = -y;
}

__forceinline int32 __fastcall CVector2i::DistanceBetweenSquared(const CVector2i& V2) const
{
	int32 d,Nx;
	Nx = x - V2.x;
	d = Nx*Nx;
	Nx = y - V2.y;
	d+= Nx*Nx;
	return d;
}

__forceinline int32 __fastcall CVector2i::DistanceBetween(const CVector2i& V2) const
{
	return Float32ToInt32( sqrtf( static_cast<float32>( DistanceBetweenSquared(V2) ) ) );
}

__forceinline void __fastcall CVector2i::Clear()
{
	x = y = 0;
}


__forceinline bool __fastcall CVector2i::IsNormalized()
{
	return LengthSquared() == 1;
}

__forceinline bool __fastcall CVector2i::Compare(const CVector2i& V2,int32 tolerance)
{
	if (abs(V2.x - x) > tolerance)
		return false;
	if (abs(V2.y - y) > tolerance)
		return false;

	return true;
}

__forceinline CVector2i operator*( int32 s, CVector2i const& v )
{
	return v * s;
}

} //namespace Math
} //namespace WT
