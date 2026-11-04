//
//	AABox.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#undef min
#undef max
#include <algorithm>

namespace WT
{
namespace Math
{

__forceinline CAABox::CAABox(const CVector3f& mmin, const CVector3f& mmax)
{
	Min = mmin;
	Max = mmax;
}

__forceinline CAABox::CAABox()
{
	Reset();
}

__forceinline bool  __fastcall  CAABox::operator==( CAABox const& Box ) const
{
	return Min == Box.Min && Max == Box.Max;
}

__forceinline bool  __fastcall  CAABox::operator!=( CAABox const& Box ) const
{
	return Min != Box.Min || Max != Box.Max;
}

__forceinline bool  __fastcall CAABox::IsEmpty() const
{
	return Min.x > Max.x || Min.y > Max.y || Min.z > Max.z;
}

__forceinline void __fastcall CAABox::Set(const CVector3f& mmin, const CVector3f& mmax)
{
	Set( mmin.x, mmin.y, mmin.z, mmax.x, mmax.y, mmax.z );
}

__forceinline void __fastcall CAABox::Set(float32 X1, float32 Y1, float32 Z1, float32 X2, float32 Y2, float32 Z2)
{
	if ( X1 > X2 )
	{	Max.x = X1;	Min.x = X2;	}
	else
	{	Max.x = X2; Min.x = X1; }
	
	if ( Y1 > Y2 )
	{	Max.y = Y1;	Min.y = Y2;	}
	else
	{	Max.y = Y2; Min.y = Y1; }
	
	if ( Z1 > Z2 )
	{	Max.z = Z1;	Min.z = Z2;	}
	else
	{	Max.z = Z2; Min.z = Z1; }
}

__forceinline void __fastcall CAABox::SetDirect(const CVector3f& mmin, const CVector3f& mmax)
{
	SetDirect( mmin.x, mmin.y, mmin.z, mmax.x, mmax.y, mmax.z );
}

__forceinline void __fastcall CAABox::SetDirect(float32 X1, float32 Y1, float32 Z1, float32 X2, float32 Y2, float32 Z2)
{
	Min.x = X1; Min.y = Y1; Min.z = Z1;
	Max.x = X2; Max.y = Y2; Max.z = Z2;
}

__forceinline void __fastcall CAABox::ExtendToEnclose( const CVector3f& Point )
{
	Max.x = std::max( Max.x,Point.x );
	Max.y = std::max( Max.y,Point.y );
	Max.z = std::max( Max.z,Point.z );
	
	Min.x = std::min( Min.x,Point.x );
	Min.y = std::min( Min.y,Point.y );
	Min.z = std::min( Min.z,Point.z );
}

__forceinline bool __fastcall CAABox::IsInside ( const CVector3f& Point ) const
{
	return Point.x >= Min.x && Point.y >= Min.y && Point.z >= Min.z &&
		   Point.x <= Max.x && Point.y <= Max.y && Point.z <= Max.z;
}

__forceinline CVector3f __fastcall CAABox::GenerateRandom()
{
	//TODO: fill this function
	return ( Min + Max ) * 0.5f;
}

}
}
