//
//	Sphere.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CSphere::CSphere(const CVector3f& center, const float32 radius, const float32 radius2 )
{
	Center = center;
	Radius = radius;
	Radius2 = radius2;
}

__forceinline CSphere::CSphere()
{
	Center.Set(0,0,0);
	Radius = 0;
	Radius2 = 0;
}

__forceinline void __fastcall CSphere::Set(const CVector3f& center, const float32 radius, const float32 radius2 )
{
	Center = center;
	Radius = radius;
	Radius2 = radius2;
}

__forceinline void __fastcall CSphere::Set(float32 X, float32 Y, float32 Z, float32 R, const float32 R2 )
{
	Center.Set( X, Y, Z );
	Radius = R;
	Radius2 = R2;
}

__forceinline CVector3f __fastcall CSphere::GenerateRandom ()
{
	CVector3f V;
	V.Rand();
	V -= 0.5f;
	V.Normalize();
	if ( Radius == Radius2 )
		return Center + V * Radius;
	else
		return Center + V * Math_Randf ( Radius2 , Radius );
}
