//
//	Point3d.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CPoint3d::CPoint3d(const CVector3f& center)
{
	Center = center;
}

__forceinline CPoint3d::CPoint3d()
{
}

__forceinline void __fastcall CPoint3d::Set(const CVector3f& center)
{
	Center = center;
}

__forceinline void __fastcall CPoint3d::Set(float32 X, float32 Y, float32 Z )
{
	Center.Set( X, Y, Z );
}

__forceinline CVector3f __fastcall CPoint3d::GenerateRandom ()
{
	return Center;
}

// Intersection tests
__forceinline bool __fastcall CPoint3d::IsInside( const CVector3f& Point ) const
{
	Point;

	return false;
}

// Translation
__forceinline CVector3f	__fastcall	CPoint3d::GetTranslation ( ) const
{
	return Center;
}
__forceinline void 		__fastcall	CPoint3d::SetTranslation ( const CVector3f& NewCenter )
{
	Center = NewCenter;
}
__forceinline void 		__fastcall	CPoint3d::Translate      ( float32 DX, float32 DY, float32 DZ )
{
	Center += CVector3f ( DX , DY , DZ );
}

// check for collision for a moving point with the sphere within a given timeframe, the point is moving from start at velocity
__forceinline bool __fastcall CPoint3d::MovingPointCollision( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	Start;
	Velocity;
	FrameTime;
	TangentVelocity;
	NormalVelocity;

	return false;
}
