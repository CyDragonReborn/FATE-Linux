//
//	AABox.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Math/Objects/AABox.h"
#include "WT/Math/Objects/Sphere.h"
#include "WT/Math/MathSerialization.h"
#include <limits>

using namespace WT::Math;


#define EB_CLAMP(x, low, high) (((x)<(low))?(low):(((x)>(high))?(high):(x)))


namespace WT
{
namespace Math
{

WT_DEFINE_CLASS(CAABox)

void __fastcall CAABox::Reset()
{
	Min.Set( +std::numeric_limits<float32>::max(), +std::numeric_limits<float32>::max(), +std::numeric_limits<float32>::max() );
	Max.Set( -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max() );
}

// Set box Min and Max to the passed point
void __fastcall CAABox::SetToPoint( const CVector3f& Point )
{
	Max = Point;
	Min = Point;
}

CVector3f __fastcall CAABox::ComputeCenter( ) const
{
	return (Max + Min) * 0.5f;
}

bool __fastcall CAABox::Intersects( const CAABox& B2 ) const
{
	if ((Min.x > B2.Max.x) || (Max.x < B2.Min.x)) return false;
	if ((Min.y > B2.Max.y) || (Max.y < B2.Min.y)) return false;
	if ((Min.z > B2.Max.z) || (Max.z < B2.Min.z)) return false;

	return true;
}

	
bool __fastcall CAABox::Intersection(const CAABox& B1, const CAABox& B2)
{
	bool rslt;
	
	rslt = B1.Intersects( B2 );
	if( rslt != false )
	{
		Set ( std::max (B1.Min.x, B2.Min.x),
			  std::max (B1.Min.y, B2.Min.y),
			  std::max (B1.Min.z, B2.Min.z),
			  std::min (B1.Max.x, B2.Max.x),
			  std::min (B1.Max.y, B2.Max.y),
			  std::min (B1.Max.z, B2.Max.z) );
	}
	return rslt;
}

void __fastcall CAABox::Union( const CAABox& B1, const CAABox& B2 )
{
	Min.x = std::min(B1.Min.x, B2.Min.x);
	Min.y = std::min(B1.Min.y, B2.Min.y);
	Min.z = std::min(B1.Min.z, B2.Min.z);
	Max.x = std::max(B1.Max.x, B2.Max.x);
	Max.y = std::max(B1.Max.y, B2.Max.y);
	Max.z = std::max(B1.Max.z, B2.Max.z);
}

bool __fastcall CAABox::ContainsPoint( const CVector3f& Point ) const
{
	if ((Point.x >= Min.x) && (Point.x <= Max.x) &&
		(Point.y >= Min.y) && (Point.y <= Max.y) &&
		(Point.z >= Min.z) && (Point.z <= Max.z))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool __fastcall CAABox::ContainsBox( const CAABox& b2 ) const
{
	assert( b2.Min.x <= b2.Max.x );
	assert( b2.Min.y <= b2.Max.y );
	assert( b2.Min.z <= b2.Max.z );
	
	return b2.Min.x >= Min.x && b2.Min.y >= Min.y && b2.Min.z >= Min.z
		&& b2.Max.x <= Max.x && b2.Max.y <= Max.y && b2.Max.z <= Max.z;
}

void __fastcall CAABox::Translate( float32 DX,  float32 DY,  float32 DZ )
{
	CVector3f VecDelta(DX, DY, DZ);

	Max += VecDelta;
	Min += VecDelta;
}

void __fastcall CAABox::SetCenter( const CVector3f& NewCenter )
{
	CVector3f Center = ComputeCenter();
	Translate( NewCenter.x - Center.x, NewCenter.y - Center.y, NewCenter.z - Center.z );
}

CVector3f __fastcall CAABox::GetScaling( ) const
{
	return Max - Min;
}

void __fastcall CAABox::Scale( float32 ScaleX, float32 ScaleY, float32 ScaleZ )
{
	float32 DX,DY,DZ;

	assert (ScaleX >= 0.0f );
	assert (ScaleY >= 0.0f );
	assert (ScaleZ >= 0.0f );
	assert (ScaleX * ScaleX >= 0.0f );		// check for NANS
	assert (ScaleY * ScaleY >= 0.0f );
	assert (ScaleZ * ScaleZ >= 0.0f );

	CVector3f Center = ComputeCenter( );
	CVector3f Scale = GetScaling( );
	
	DX = ScaleX * Scale.x * 0.5f;
	DY = ScaleY * Scale.y * 0.5f;
	DZ = ScaleZ * Scale.z * 0.5f;

	Min.x = Center.x - DX;
	Min.y = Center.y - DY;
	Min.z = Center.z - DZ;
	
	Max.x = Center.x + DX;
	Max.y = Center.y + DY;
	Max.z = Center.z + DZ;
}

void __fastcall CAABox::SetScaling( const CVector3f& Scale )
{
	float32 DX,DY,DZ;

	assert (Scale.x >= 0.0f );
	assert (Scale.y >= 0.0f );
	assert (Scale.z >= 0.0f );
	
	CVector3f Center = ComputeCenter( );
	
	DX = Scale.x * 0.5f;
	DY = Scale.y * 0.5f;
	DZ = Scale.z * 0.5f;
	
	Min.x = Center.x - DX;
	Min.y = Center.y - DY;
	Min.z = Center.z - DZ;
	
	Max.x = Center.x + DX;
	Max.y = Center.y + DY;
	Max.z = Center.z + DZ;
}

void __fastcall CAABox::LinearSweep( const CAABox& BoxToSweep, 
								    const CVector3f& StartPoint, 
								    const CVector3f& EndPoint )
{
	*this = BoxToSweep;
	
	if (EndPoint.x > StartPoint.x)
	{
		Min.x += StartPoint.x; 
		Max.x += EndPoint.x; 
	}
	else
	{
		Min.x += EndPoint.x; 
		Max.x += StartPoint.x; 
	}
	
	if (EndPoint.y > StartPoint.y)
	{
		Min.y += StartPoint.y; 
		Max.y += EndPoint.y; 
	}
	else
	{
		Min.y += EndPoint.y; 
		Max.y += StartPoint.y; 
	}
	
	if (EndPoint.z > StartPoint.z)
	{
		Min.z += StartPoint.z; 
		Max.z += EndPoint.z; 
	}
	else
	{
		Min.z += EndPoint.z; 
		Max.z += StartPoint.z; 
	}
}

bool __fastcall CAABox::XFaceDist( const CVector3f& Start, const CVector3f& Delta, float32 *T, float32 X) const
{
	float32 t;
	float32 Y,Z;
	assert( T     != NULL );
	
	//if ( (Start.x <= X) && (X <= Delta.x + Start.x) )
	{
		t = (X - Start.x)/Delta.x;
		Y  = Start.y + Delta.y * t;
		if ( ( Min.y <= Y) && (Y <= Max.y) )
		{
			Z = Start.z + Delta.z * t;
			if ( ( Min.z <= Z) && (Z <= Max.z) )
			{
				*T = t;
				return true;
			}
		}
	}
	return false;
}

bool __fastcall CAABox::YFaceDist( const CVector3f& Start, const CVector3f& Delta, float32 *T, float32 Y) const
{
	float32 t;
	float32 X,Z;
	assert( T     != NULL );
	
	//if ( (Start.y <= Y) && (Y <= Delta.y + Start.y) )
	{
		t = (Y - Start.y)/Delta.y;
		Z  = Start.z + Delta.z * t;
		if ( ( Min.z <= Z) && (Z <= Max.z) )
		{
			X = Start.x + Delta.x * t;
			if ( ( Min.x <= X) && (X <= Max.x) )
			{
				*T = t;
				return true;
			}
		}
	}
	return false;
}


bool __fastcall CAABox::ZFaceDist( const CVector3f& Start, const CVector3f& Delta, float32 *T, float32 Z) const
{
	float32 t;
	float32 X,Y;
	assert( T     != NULL );
	
	//if ( (Start.z <= Z) && (Z <= Delta.z + Start.z) )
	{
		t = (Z - Start.z)/Delta.z;
		X  = Start.x + Delta.x * t;
		if ( ( Min.x <= X) && (X <= Max.x) )
		{
			Y = Start.y + Delta.y * t;
			if ( ( Min.y <= Y) && (Y <= Max.y) )
			{
				*T = t;
				return true;
			}
		}
	}
	return false;
}


bool __fastcall CAABox::RayCollision( const CVector3f& Start, const CVector3f& End, float32* T, CVector3f& Normal ) const
{
	// only detects rays going 'in' to the box
	float32 t;
	CVector3f Delta = End - Start;
	float32 LocalT;
	
	if (T == NULL)
		T = &LocalT;
	
	// test x end of box, facing away from ray direction.
	if (Delta.x > 0.0f)
	{
		if ( (Start.x <= Min.x) && (Min.x <= End.x) &&
			 (XFaceDist( Start, Delta, &t, Min.x ) != false) )
		{
			Normal.x = -1.0f;
			Normal.y = 0.0f;
			Normal.z = 0.0f;
			*T = t;
			return true;
		}
	}
	else if (Delta.x < 0.0f)
	{
		if ( (End.x <= Max.x) && (Max.x <= Start.x) &&
			 (XFaceDist( Start, Delta, &t, Max.x ) != false) )
		{
			Normal.x = 1.0f;
			Normal.y = 0.0f;
			Normal.z = 0.0f;
			*T = t;
			return true;
		}
	}
	
	// test y end of box, facing away from ray direction.
	if (Delta.y > 0.0f)
	{	
		if ( (Start.y <= Min.y) && (Min.y <= End.y) &&
			 (YFaceDist( Start, Delta, &t, Min.y ) != false) )
		{
			Normal.x = 0.0f;
			Normal.y = -1.0f;
			Normal.z = 0.0f;
			*T = t;
			return true;
		}
	}
	else if (Delta.y < 0.0f)
	{
		if ( (End.y <= Max.y) && (Max.y <= Start.y) &&
			 (YFaceDist( Start, Delta, &t, Max.y ) != false) )
		{
			Normal.x = 0.0f;
			Normal.y = 1.0f;
			Normal.z = 0.0f;
			*T = t;
			return true;
		}
	}
	
	// test z end of box, facing away from ray direction.
	if (Delta.z > 0.0f)
	{	
		if ( (Start.z <= Min.z) && (Min.z <= End.z) &&
			 (ZFaceDist( Start, Delta, &t, Min.z ) != false) )
		{
			Normal.x = 0.0f;
			Normal.y = 0.0f;
			Normal.z = -1.0f;
			*T = t;
			return true;
		}
	}
	else if (Delta.z < 0.0f)
	{			
		if ( (End.z <= Max.z) && (Max.z <= Start.z) &&
			 (ZFaceDist( Start, Delta, &t, Max.z ) != false) )
		{
			Normal.x = 0.0f;
			Normal.y = 0.0f;
			Normal.z = 1.0f;
			*T = t;
			return true;
		}
	}
	return false;	
}

void  __fastcall CAABox::Transform( const CMatrix4f& XF )
{
	// Ignore translation until end

	CVector3f Corners[8];

	Corners[0].Set( Min.x, Min.y, Min.z );
	Corners[1].Set( Min.x, Max.y, Min.z );
	Corners[2].Set( Max.x, Max.y, Min.z );
	Corners[3].Set( Max.x, Min.y, Min.z );

	Corners[4].Set( Min.x, Min.y, Max.z );
	Corners[5].Set( Min.x, Max.y, Max.z );
	Corners[6].Set( Max.x, Max.y, Max.z );
	Corners[7].Set( Max.x, Min.y, Max.z );

	Reset( );
	CVector3f Temp;
	for( int32 v =0; v < 8; v++ )
	{
		XF.Transform( Temp, Corners[v] );
		ExtendToEnclose( Temp );
	}
}

void  __fastcall CAABox::Rotate( const CMatrix4f& XF )
{
	CVector3f LocalMin( false ), LocalMax( false );
	float32 tmpmin, tmpmax;

	// Ignore translation until end

	tmpmin = Min.x * XF.RowX.x;
	tmpmax = Max.x * XF.RowX.x;
	if ( tmpmin < tmpmax ) { LocalMin.x = tmpmin; LocalMax.x = tmpmax; }
	else { LocalMin.x = tmpmax; LocalMax.x = tmpmin; }
	tmpmin = Min.y * XF.RowX.y;
	tmpmax = Max.y * XF.RowX.y;
	if ( tmpmin < tmpmax ) { LocalMin.x += tmpmin; LocalMax.x += tmpmax; }
	else { LocalMin.x += tmpmax; LocalMax.x += tmpmin; }
	tmpmin = Min.z * XF.RowX.z;
	tmpmax = Max.z * XF.RowX.z;
	if ( tmpmin < tmpmax ) { LocalMin.x += tmpmin; LocalMax.x += tmpmax; }
	else { LocalMin.x += tmpmax; LocalMax.x += tmpmin; }

	tmpmin = Min.x * XF.RowY.x;
	tmpmax = Max.x * XF.RowY.x;
	if ( tmpmin < tmpmax ) { LocalMin.y = tmpmin; LocalMax.y = tmpmax; }
	else { LocalMin.y = tmpmax; LocalMax.y = tmpmin; }
	tmpmin = Min.y * XF.RowY.y;
	tmpmax = Max.y * XF.RowY.y;
	if ( tmpmin < tmpmax ) { LocalMin.y += tmpmin; LocalMax.y += tmpmax; }
	else { LocalMin.y += tmpmax; LocalMax.y += tmpmin; }
	tmpmin = Min.z * XF.RowY.z;
	tmpmax = Max.z * XF.RowY.z;
	if ( tmpmin < tmpmax ) { LocalMin.y += tmpmin; LocalMax.y += tmpmax; }
	else { LocalMin.y += tmpmax; LocalMax.y += tmpmin; }

	tmpmin = Min.x * XF.RowZ.x;
	tmpmax = Max.x * XF.RowZ.x;
	if ( tmpmin < tmpmax ) { LocalMin.z = tmpmin; LocalMax.z = tmpmax; }
	else { LocalMin.z = tmpmax; LocalMax.z = tmpmin; }
	tmpmin = Min.y * XF.RowZ.y;
	tmpmax = Max.y * XF.RowZ.y;
	if ( tmpmin < tmpmax ) { LocalMin.z += tmpmin; LocalMax.z += tmpmax; }
	else { LocalMin.z += tmpmax; LocalMax.z += tmpmin; }
	tmpmin = Min.z * XF.RowZ.z;
	tmpmax = Max.z * XF.RowZ.z;
	if ( tmpmin < tmpmax ) { LocalMin.z += tmpmin; LocalMax.z += tmpmax; }
	else { LocalMin.z += tmpmax; LocalMax.z += tmpmin; }

	Min = LocalMin;
	Max = LocalMax;
}

float32 __fastcall CAABox::DistanceSquaredToPoint( const CVector3f& Pos ) const
{
	CVector3f Closest( false );

	Closest.x = EB_CLAMP(Pos.x, Min.x, Max.x);
	Closest.y = EB_CLAMP(Pos.y, Min.y, Max.y);
	Closest.z = EB_CLAMP(Pos.z, Min.z, Max.z);

	return Pos.DistanceBetweenSquared( Closest );
}

void  __fastcall CAABox::GetExtSphere(CSphere& Sphere) const
{
	CVector3f center = ComputeCenter( );

	Sphere.Set( center.x, center.y, center.z, Min.DistanceBetween(Max) * 0.5f );
}

bool __fastcall CAABox::MovingPointCollision ( const CVector3f& Start , const CVector3f& Velocity , float32 FrameTime , CVector3f& TangentVelocity , CVector3f& NormalVelocity ) const
{
	throw std::runtime_error( "not implemented" );

	Start;
	Velocity;
	FrameTime;
	TangentVelocity;
	NormalVelocity;
	
	//TODO: fill this function;

	return false;
}

// ISerializable

void CAABox::GetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						    System::Serialization::CSerializationContext* pContext ) const
{
	pInfo->AddValue( "Min", Min );
	pInfo->AddValue( "Max", Max );
}

void CAABox::SetObjectData( System::Serialization::CSerializationInfo* pInfo, 
						    System::Serialization::CSerializationContext* pContext )
{
	pInfo->GetValue( "Min", Min );
	pInfo->GetValue( "Max", Max );
}


} // namespace Math
} // namespace WT
