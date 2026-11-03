//
//	Plane.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline bool __fastcall CPlane::IsInside ( const CVector3f& Point ) const
{
	return PointDistance( Point ) <= 0.0f;
}


// D3DX support
__forceinline CPlane::CPlane(const D3DXPLANE& D3DXPlane)
{
	Normal.x = D3DXPlane.a;
	Normal.y = D3DXPlane.b;
	Normal.z = D3DXPlane.c;
	Distance = D3DXPlane.d;
}

__forceinline CPlane& __fastcall CPlane::operator=(const D3DXPLANE& rhs)
{
	Normal.x = rhs.a;
	Normal.y = rhs.b;
	Normal.z = rhs.c;
	Distance = rhs.d;
	return *this;
}

__forceinline D3DXPLANE * __fastcall CPlane::GetD3DXPlanePtr()
{
	return reinterpret_cast<D3DXPLANE *>( e );
}
