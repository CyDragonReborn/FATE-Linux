//
//	CVector4f.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline bool __fastcall CVector4f::operator == (const CVector4f& v) const
{
	if (x != v.x || y != v.y || z != v.z || w != v.w)
	{
		return false;
	} 
	else 
	{
		return true;
	}
}

__forceinline bool __fastcall CVector4f::operator != (const CVector4f& v) const
{
	if (x != v.x || y != v.y || z != v.z || w != v.w)
	{
		return true;
	} 
	else 
	{
		return false;
	}
}

__forceinline CVector4f& __fastcall CVector4f::operator += (const CVector3f& v)
{
	x+=v.x;	y+=v.y;	z+=v.z; w = w;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator + (const CVector3f& v) const
{
	return CVector4f(x+v.x, y+v.y, z+v.z, w);
}

__forceinline CVector4f& __fastcall CVector4f::operator -= (const CVector3f& v)
{
	x-=v.x;	y-=v.y;	z-=v.z; w = w;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator - (const CVector3f& v) const
{
	return CVector4f(x-v.x, y-v.y, z-v.z, w);
}

__forceinline CVector4f& __fastcall CVector4f::operator += (const CVector4f& v)
{
	x+=v.x;	y+=v.y;	z+=v.z; w+=v.w;
	return *this;

}

__forceinline CVector4f __fastcall CVector4f::operator + (const CVector4f& v) const
{
	return CVector4f(x+v.x, y+v.y, z+v.z, w+v.w);
}

__forceinline CVector4f& __fastcall CVector4f::operator -= (const CVector4f& v)
{
	x-=v.x;	y-=v.y;	z-=v.z; w-=v.w;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator - (const CVector4f& v) const
{
	return CVector4f(x-v.x, y-v.y, z-v.z, w-v.w);
}

__forceinline CVector4f& __fastcall CVector4f::operator += (const float32 v)
{
	x+=v; y+=v; z+=v; w+=v;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator + (const float32 v) const
{
	return CVector4f(x+v, y+v, z+v, w+v);
}

__forceinline CVector4f& __fastcall CVector4f::operator -= (const float32 v)
{
	x-=v; y-=v; z-=v; w-=v;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator - (const float32 v) const
{
	return CVector4f(x-v, y-v, z-v, w-v);
}

__forceinline CVector4f& __fastcall CVector4f::operator *= (const float32 v)
{
	x*=v; y*=v; z*=v; w*=v;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator * (const float32 v) const
{
	return CVector4f(x*v, y*v, z*v, w*v);
}

__forceinline CVector4f& __fastcall CVector4f::operator /= (const float32 v)
{
	float32 iv = 1.0f/v;
	x*=iv; y*=iv; z*=iv; w*=iv;
	return *this;
}

__forceinline CVector4f __fastcall CVector4f::operator / (const float32 v) const
{
	float32 iv = 1.0f/v;
	return CVector4f(x*iv, y*iv, z*iv, w*iv);
}

__forceinline CVector4f __fastcall CVector4f::operator - () const
{
	return CVector4f(-x, -y, -z, -w);
}

__forceinline void __fastcall CVector4f::Set(float32 X, float32 Y, float32 Z, float32 W)
{
	x = X; y = Y; z = Z; w = W;
}

__forceinline void __fastcall CVector4f:: Get(float32 *X, float32 *Y, float32 *Z, float32 *W) const
{
	*X = x; *Y = y; *Z = z; *W = w;
}

__forceinline void __fastcall CVector4f::Set(float32 X, float32 Y, float32 Z)
{
	x = X; y = Y; z = Z;
}

__forceinline void __fastcall CVector4f:: Get(float32 *X, float32 *Y, float32 *Z) const
{
	*X = x; *Y = y; *Z = z;
}

__forceinline float32 __fastcall CVector4f::DotProduct(const CVector4f& V2) const
{
	return x * V2.x + y * V2.y + z * V2.z + w * V2.w;
}

__forceinline CVector4f operator*( float32 s, CVector4f const& v )
{
	return v * s;
}


// D3DX support
__forceinline CVector4f::CVector4f(const D3DXVECTOR4& D3DXSource)
{
	x = D3DXSource.x;
	y = D3DXSource.y;
	z = D3DXSource.z;
	w = D3DXSource.w;
}

__forceinline CVector4f& __fastcall CVector4f::operator=(const D3DXVECTOR4& rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
	return *this;
}

__forceinline D3DXVECTOR4* __fastcall CVector4f::GetD3DXVector4Ptr()
{
	return reinterpret_cast<D3DXVECTOR4*>( e );
}

__forceinline D3DXVECTOR4 const* __fastcall CVector4f::GetD3DXVector4Ptr() const
{
	return reinterpret_cast<D3DXVECTOR4 const*>( e );
}
