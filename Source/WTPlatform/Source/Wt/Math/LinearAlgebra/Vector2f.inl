//
//	CVector2f.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline bool __fastcall CVector2f::operator == (const CVector2f& v) const
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

__forceinline bool __fastcall CVector2f::operator != (const CVector2f& v) const
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

__forceinline CVector2f& __fastcall CVector2f::operator +=(const CVector2f& v)
{
	x+=v.x; y+=v.y;
	return *this;
}

__forceinline CVector2f __fastcall CVector2f::operator + (const CVector2f& v) const
{
	return CVector2f(x+v.x, y+v.y);
}

__forceinline CVector2f& __fastcall CVector2f::operator -=(const CVector2f& v)
{
	x-=v.x; y-=v.y;
	return *this;
}

__forceinline CVector2f __fastcall CVector2f::operator - (const CVector2f& v) const
{
	return CVector2f(x-v.x, y-v.y);
}

__forceinline CVector2f& __fastcall CVector2f::operator +=(const float32 v)
{
	x+=v; y+=v;
	return *this;
}

__forceinline CVector2f __fastcall CVector2f::operator + (const float32 v) const
{
	return CVector2f(x+v, y+v);
}

__forceinline CVector2f& __fastcall CVector2f::operator -=(const float32 v)
{
	x-=v; y-=v;
	return *this;
}

__forceinline CVector2f __fastcall CVector2f::operator - (const float32 v) const
{
	return CVector2f(x-v, y-v);
}

__forceinline CVector2f& __fastcall CVector2f::operator *=(const float32 v)
{
	x*=v; y*=v; 
	return *this;
}

__forceinline CVector2f __fastcall CVector2f::operator * (const float32 v) const
{
	return CVector2f(x*v, y*v);
}

__forceinline CVector2f& __fastcall CVector2f::operator /=(const float32 v)
{
	return operator *=(1.0f/(float32)v);
}

__forceinline CVector2f __fastcall CVector2f::operator / (const float32 v) const
{
	return operator *(1.0f/v);
}


__forceinline float32 __fastcall CVector2f::operator [] (const int32 x) const
{
	return e[x];
}

__forceinline float32& __fastcall CVector2f::operator [] (const int32 x)
{
	return e[x];
}

__forceinline CVector2f __fastcall CVector2f::operator - () const
{
	return CVector2f(-x, -y);
}

__forceinline void __fastcall CVector2f::Set(float32 X, float32 Y)
{
	x=X; y=Y;
}

__forceinline void __fastcall CVector2f::Get(float32 *X, float32 *Y) const
{
	*X=x; *Y=y;
}

__forceinline float32 __fastcall CVector2f::DotProduct(const CVector2f& V2) const
{
	return x*V2.x + y*V2.y;
}

__forceinline float32 __fastcall CVector2f::CrossProduct(const CVector2f& V2)
{
	return x*V2.y - y*V2.x;
}

__forceinline float32 __fastcall CVector2f::LengthSquared() const
{
	return ( x*x + y*y );
}

__forceinline float32 __fastcall CVector2f::Length() const
{
	return (float32)(sqrtf((float32)LengthSquared()));
}

__forceinline float32 __fastcall CVector2f::Normalize()
{
	float32 OneOverDist;
	float32 Dist;
	
	Dist = Length();
	
	if (Dist == 0)
		return 0;
	
	OneOverDist = (1.0f/(float32)Dist);
	
	x = (float32)((float32)x * OneOverDist);
	y = (float32)((float32)y * OneOverDist);
	
	return Dist;
}

__forceinline void __fastcall CVector2f::Inverse()
{
	x = -x; y = -y;
}

__forceinline float32 __fastcall CVector2f::DistanceBetweenSquared(const CVector2f& V2) const
{
	float32 d,Nx;
	Nx = x - V2.x;
	d = Nx*Nx;
	Nx = y - V2.y;
	d+= Nx*Nx;
	return d;
}

__forceinline float32 __fastcall CVector2f::DistanceBetween(const CVector2f& V2) const
{
	return (float32)(sqrtf( (float32)DistanceBetweenSquared(V2) ));
}

__forceinline void __fastcall CVector2f::Clear()
{
	x = y = 0;
}


__forceinline bool __fastcall CVector2f::IsNormalized()
{
	float32	length;
	
	length = LengthSquared();
	
	if ( (( length) < 1 ) &&
		(( length) > 1 ) )
		return true;
	else
		return false;
}

__forceinline bool __fastcall CVector2f::Compare(const CVector2f& V2,float32 tolerance)
{
	if (fabs(V2.x - x) > tolerance)
		return false;
	if (fabs(V2.y - y) > tolerance)
		return false;

	return true;
}

__forceinline void __fastcall CVector2f::Rand()
{
	x = Math_Randf();
	y = Math_Randf();
}

__forceinline void __fastcall CVector2f::SetToHalf()
{
	x = y = 0.5f;
}

__forceinline CVector2f operator*( float32 s, CVector2f const& v )
{
	return v * s;
}


// D3DX support
__forceinline CVector2f::CVector2f(const D3DXVECTOR2& D3DXSource)
{
	x = D3DXSource.x;
	y = D3DXSource.y;
}

__forceinline CVector2f& __fastcall CVector2f::operator=(const D3DXVECTOR2& rhs)
{
	x = rhs.x;
	y = rhs.y;
	return *this;
}

__forceinline D3DXVECTOR2 * __fastcall CVector2f::GetD3DXVector2Ptr()
{
	return reinterpret_cast<D3DXVECTOR2 *>(this);
}
