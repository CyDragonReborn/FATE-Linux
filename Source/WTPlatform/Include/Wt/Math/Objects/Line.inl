//
//	Line.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CLine::CLine ( const CVector3f& AStartPoint , const CVector3f& AEndPoint ) :
	StartPoint ( AStartPoint ) , EndPoint ( AEndPoint )
{
}

__forceinline void __fastcall CLine::Set ( const CVector3f& AStartPoint , const CVector3f& AEndPoint )
{
	StartPoint = AStartPoint;
	EndPoint = AEndPoint;
}

__forceinline bool __fastcall CLine::IsInside ( const CVector3f& Point ) const
{
	Point;

	return false;
}

__forceinline CVector3f __fastcall CLine::GenerateRandom ()
{
	return StartPoint + ( EndPoint - StartPoint ) * Math_Randf();
}

__forceinline bool __fastcall CLine::MovingPointCollision( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	Start;
	Velocity;
	FrameTime;
	TangentVelocity;
	NormalVelocity;

	return false;
}
