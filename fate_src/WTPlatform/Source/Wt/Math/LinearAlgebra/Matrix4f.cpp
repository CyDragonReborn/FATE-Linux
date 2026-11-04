//
//	Matrix4f.cpp
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

#include "WT/stdafx.h"
#include "Matrix4f.h"
#include "WT/Math/Utils/MathUtil.h"
#include "WT/Math/Quaternion/Quaternion.h"

#include <memory.h>


namespace WT
{
namespace Math
{


namespace
{
	float32 const EPSILON = 0.000173f;
} //anonymous namespace


CMatrix4f const CMatrix4f::gm_Zero    ( 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 );
CMatrix4f const CMatrix4f::gm_Identity( 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 );

bool __fastcall Matrix4f_IsValid(const CMatrix4f *M)
// returns true if M is 'valid'  
// 'valid' means that M is non NULL, and there are no NAN's in the matrix.
{
	
	if (M == NULL)
		return false;
	
	if ((M->RowX.x * M->RowX.x) < 0.0f) 
		return false;
	if ((M->RowX.y * M->RowX.y) < 0.0f) 
		return false;
	if ((M->RowX.z * M->RowX.z) < 0.0f) 
		return false;
	
	if ((M->RowY.x * M->RowY.x) < 0.0f) 
		return false;
	if ((M->RowY.y * M->RowY.y) < 0.0f) 
		return false;
	if ((M->RowY.z * M->RowY.z) < 0.0f) 
		return false;
	
	if ((M->RowZ.x * M->RowZ.x) < 0.0f) 
		return false;
	if ((M->RowZ.y * M->RowZ.y) < 0.0f) 
		return false;
	if ((M->RowZ.z * M->RowZ.z) < 0.0f) 
		return false;
	
	return true;
}

void __fastcall CMatrix4f::SetXRotation(float32 RadianAngle)
// sets up a transform that rotates RadianAngle about X axis
{
	float Cos,Sin;
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	Cos = cosf(RadianAngle);
	Sin = sinf(RadianAngle);
	RowY.y =  Cos;
	RowY.z =  Sin;
	RowZ.y = -Sin;
	RowZ.z =  Cos;
	RowX.x = 1.0f;
	RowX.y = RowX.z = RowY.x = RowZ.x = 0.0f;
	RowT.x = RowT.y = RowT.z = 0.0f;
	RowX.w = RowY.w = RowZ.w = 0.0f;
	RowT.w = 1.0f;

	assert( IsOrthonormal() );
} 

void __fastcall CMatrix4f::SetYRotation(float32 RadianAngle)
// sets up a transform that rotates RadianAngle about Y axis
{
	float Cos,Sin;
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	Cos = cosf(RadianAngle);
	Sin = sinf(RadianAngle);
	
	RowX.x =  Cos;
	RowX.z = -Sin;
	RowZ.x =  Sin;
	RowZ.z =  Cos;
	RowY.y = 1.0f;
	RowX.y = RowY.x = RowY.z = RowZ.y = 0.0f;
	RowT.x = RowT.y = RowT.z = 0.0f;
	RowX.w = RowY.w = RowZ.w = 0.0f;
	RowT.w = 1.0f;
	
	assert( IsOrthonormal() );
}

void __fastcall CMatrix4f::SetZRotation(float32 RadianAngle)
// sets up a transform that rotates RadianAngle about Y axis
{
	float Cos,Sin;
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	Cos = cosf(RadianAngle);
	Sin = sinf(RadianAngle);
	
	RowX.x =  Cos;
	RowX.y =  Sin;
	RowY.x = -Sin;
	RowY.y =  Cos;
	RowZ.z = 1.0f;
	RowX.z = RowY.z = RowZ.x = RowZ.y = 0.0f;
	RowT.x = RowT.y = RowT.z = 0.0f;
	RowX.w = RowY.w = RowZ.w = 0.0f;
	RowT.w = 1.0f;
	
	assert( IsOrthonormal() );
}

void __fastcall CMatrix4f::SetAxisRotation(const CVector3f& Axis, float32 RadianAngle)
{
	assert( Axis.IsNormalized() );

	float32 c = cosf( RadianAngle );
	float32 s = sinf( RadianAngle );
	float32 vercos = 1.0f - c;

	float32 const sx = s * Axis.x;
	float32 const sy = s * Axis.y;
	float32 const sz = s * Axis.z;
	float32 const vx = vercos * Axis.x;
	float32 const vy = vercos * Axis.y;
	float32 const vxy = vx * Axis.y;
	float32 const vyz = vy * Axis.z;
	float32 const vzx = vx * Axis.z;

	// x axis
	RowX.x = vx * Axis.x + c;
	RowX.y = vxy + sz;
	RowX.z = vzx - sy;
	RowX.w = 0;
	
	// y axis
	RowY.x = vxy - sz;
	RowY.y = vy * Axis.y + c;
	RowY.z = vyz + sx;
	RowY.w = 0;
	
	// z axis
	RowZ.x = vzx + sy;
	RowZ.y = vyz - sx;
	RowZ.z = Axis.z * Axis.z * vercos + c;
	RowZ.w = 0;

	RowT.x = RowT.y = RowT.z = 0.0f;
	RowT.w = 1;

	//assert( IsOrthonormal() );

#if 0 && !defined(NDEBUG)
	{
		CQuaternion Q;
		Q.SetFromAxisAngle( Axis, RadianAngle );
		CMatrix4f M( false );
		Q.ToMatrix( &M );

		float32 const diff = Compare( M );
		assert( diff < 0.01f );
	}
#endif
}

CMatrix4f __fastcall CMatrix4f::GetInverse() const
{
	CMatrix4f MInv( false );
	CVector3f U( false ), V( false ), W( false ), T( false );

	//Get the columns
	U.x = RowX.x;	V.x = RowX.y;	W.x = RowX.z;
	U.y = RowY.x;	V.y = RowY.y;	W.y = RowY.z;
	U.z = RowZ.x;	V.z = RowZ.y;	W.z = RowZ.z;
	
	//RowU = (V x W) / ( U * (V x W)) , and so on
	
	T.CrossProduct( V, W );
	MInv.RowX = T / U.DotProduct( T );
	MInv.RowX.w = 0.0f;
	
	T.CrossProduct( U, W );
	MInv.RowY = T / V.DotProduct( T );
	MInv.RowY.w = 0.0f;
	
	T.CrossProduct( U, V );
	MInv.RowZ = T / W.DotProduct( T );
	MInv.RowZ.w = 0.0f;
	
	// InvTranslation = - (MInv3x3) (RowT)
	// do the 3x3 matrix multiply right here custom:
	
	MInv.RowT.x = -(RowT.x * MInv.RowX.x + RowT.y * MInv.RowY.x + RowT.z * MInv.RowZ.x);
    MInv.RowT.y = -(RowT.x * MInv.RowX.y + RowT.y * MInv.RowY.y + RowT.z * MInv.RowZ.y);
    MInv.RowT.z = -(RowT.x * MInv.RowX.z + RowT.y * MInv.RowY.z + RowT.z * MInv.RowZ.z);
	MInv.RowT.w = 1.0f;

#ifndef NDEBUG // make sure it really is the inverse!
	{
		CMatrix4f Mul;
		Mul.Multiply( MInv, *this );
		float32 diff = Mul.Compare( gm_Identity );
		assert( diff < 0.1f );
	}
#endif

	return MInv;
}

bool __fastcall CMatrix4f::GetInverse4x4( CMatrix4f* pResult ) const
{
#if 0 
	//More numerically stable method, but roughly 8x slower...
	//Compute full inverse using Gauss-Jordan elimination

	CMatrix4f Source = *this;
	CMatrix4f Result;

	for (uint32 Cur = 0; Cur < 4; ++Cur)
	{
		float32* SourceCol = &Source.e[Cur];

		//Find element of greatest magnitude in this column
		uint32 BestRow = Cur;
		for (uint32 Row = Cur + 1; Row < 4; ++Row)
		{
			if (fabsf( SourceCol[Row * 4] ) > fabsf( SourceCol[BestRow * 4] ))
			{
				BestRow = Row;
			}
		}

		float32 Value = SourceCol[BestRow * 4];
		if (Value == 0.0f)
		{
			//Not invertible
			return false;
		}
		
		float32* SourceRow = &Source.e[Cur * 4];
		float32* ResultRow = &Result.e[Cur * 4];

		//Swap rows
		if (BestRow != Cur)
		{
			float32 t;

			for (uint32 Col = Cur; Col < 4; ++Col)
			{
				t = SourceRow[Col];
				SourceRow[Col] = Source.e[BestRow * 4 + Col];
				Source.e[BestRow * 4 + Col] = t;
			}

			for (uint32 Col = 0; Col < 4; ++Col)
			{
				t = ResultRow[Col];
				ResultRow[Col] = Result.e[BestRow * 4 + Col];
				Result.e[BestRow * 4 + Col] = t;
			}
		}

		//Scale rows
		float32 InvValue = 1.0f / Value;
		SourceRow[Cur] = 1.0f;
		for (uint32 Col = Cur + 1; Col < 4; ++Col)
		{
			SourceRow[Col] *= InvValue;
		}
	
		for (uint32 Col = 0; Col < 4; ++Col)
		{
			ResultRow[Col] *= InvValue;
		}

		//Elimination
		for (uint32 Row = Cur + 1; Row < 4; ++Row)
		{
			float32 Scale = Source.e[Row * 4 + Cur];
			for (uint32 Col = Cur + 1; Col < 4; ++Col)
			{
				Source.e[Row * 4 + Col] -= Scale * SourceRow[Col];
			}

			for (uint32 Col = 0; Col < 4; ++Col)
			{
				Result.e[Row * 4 + Col] -= Scale * ResultRow[Col];
			}
		}
	}

	//Back elimination
	for (uint32 Cur = 1; Cur < 4; ++Cur)
	{
		for (uint32 Row = 0; Row < Cur; ++Row)
		{
			float32 Scale = Source.e[Row * 4 + Cur];
		
			for (uint32 Col = Cur + 1; Col < 4; ++Col)
			{
				Source.e[Row * 4 + Col] -= Scale * Source.e[Cur * 4 + Col];
			}

			for (uint32 Col = 0; Col < 4; ++Col)
			{
				Result.e[Row * 4 + Col] -= Scale * Result.e[Cur * 4 + Col];
			}
		}
	}

	if (pResult != NULL)
	{
		*pResult = Result;
	}
#else
	CMatrix4f Result( false );

	float32 const D1324 = _13 * _24 - _23 * _14;
	float32 const D1334 = _13 * _34 - _33 * _14;
	float32 const D1344 = _13 * _44 - _43 * _14;
	float32 const D2334 = _23 * _34 - _33 * _24;
	float32 const D2344 = _23 * _44 - _43 * _24;
	float32 const D3344 = _33 * _44 - _43 * _34;

	Result._11 = _22 * D3344 - _32 * D2344 + _42 * D2334;
	Result._12 = _32 * D1344 - _42 * D1334 - _12 * D3344;
	Result._13 = _12 * D2344 - _22 * D1344 + _42 * D1324;
	Result._14 = _22 * D1334 - _32 * D1324 - _12 * D2334;

	float32 Determinant = _11 * Result._11 + _21 * Result._12 + _31 * Result._13 + _41 * Result._14;
	if (Determinant == 0.0f)
	{
		return false;
	}

	if (pResult == NULL)
	{
		return true;
	}

	Determinant = 1.0f / Determinant;

	Result._21 = _31 * D2344 - _41 * D2334 - _21 * D3344;
	Result._22 = _11 * D3344 - _31 * D1344 + _41 * D1334;
	Result._23 = _21 * D1344 - _41 * D1324 - _11 * D2344;
	Result._24 = _11 * D2334 - _21 * D1334 + _31 * D1324;
			
	float32 const D1122 = _11 * _22 - _21 * _12;
	float32 const D1132 = _11 * _32 - _31 * _12;
	float32 const D1142 = _11 * _42 - _41 * _12;
	float32 const D2132 = _21 * _32 - _31 * _22;
	float32 const D2142 = _21 * _42 - _41 * _22;
	float32 const D3142 = _31 * _42 - _41 * _32;

	Result._31 = _44 * D2132 - _34 * D2142 + _24 * D3142;
	Result._32 = _34 * D1142 - _14 * D3142 - _44 * D1132;
	Result._33 = _44 * D1122 - _24 * D1142 + _14 * D2142;
	Result._34 = _24 * D1132 - _14 * D2132 - _34 * D1122;
	Result._41 = _33 * D2142 - _23 * D3142 - _43 * D2132;
	Result._42 = _43 * D1132 + _13 * D3142 - _33 * D1142;
	Result._43 = _23 * D1142 - _13 * D2142 - _43 * D1122;
	Result._44 = _33 * D1122 - _23 * D1132 + _13 * D2132;

	Result *= Determinant;

	*pResult = Result;
#endif

#ifndef NDEBUG // make sure it really is the inverse!
	{
		CMatrix4f Product( false );
		Product.Multiply4x4( *this, Result );
		float32 Diff = Product.Compare( gm_Identity );
		assert( Diff < 0.1f );
	}
#endif

    return true;
}

CVector3f __fastcall CMatrix4f::GetEulerAngles() const
{
	//assert( IsOrthonormal() );

	CVector3f Result( false );

	//Pitch
	Result.x = RowZ.y <= -1.0f ? KPi * 0.5f :
			   RowZ.y >= +1.0f ? KPi * -0.5f :
		       -asinf( RowZ.y );

	if (fabsf( RowZ.y ) <= 1.0f - EPSILON)
	{
		Result.y = atan2f( RowZ.x, RowZ.z ); //Yaw
		Result.z = atan2f( RowX.y, RowY.y ); //Roll
	} else {
		Result.y = atan2f( RowY.x, RowY.z ); //Yaw
		Result.z = 0.0f; //Roll
	}

	return Result;
}


void __fastcall CMatrix4f::SetEulerAngles(const CVector3f& Angles)
// order of angles pitch, yaw, roll
{
	const float32 sinx = sinf( Angles.x );
	const float32 cosx = cosf( Angles.x );
	const float32 siny = sinf( Angles.y );
	const float32 cosy = cosf( Angles.y );
	const float32 sinz = sinf( Angles.z );
	const float32 cosz = cosf( Angles.z );

	const float32 sxsy = sinx * siny;
	const float32 sxcy = sinx * cosy;

	_11 = cosy * cosz + sxsy * sinz;
	_12 = sinz * cosx;
	_13 = sxcy * sinz - siny * cosz;
	_14 = 0.0f;

	_21 = sxsy * cosz - cosy * sinz;
	_22 = cosz * cosx;
	_23 = siny * sinz + sxcy * cosz;
	_24 = 0.0f;

	_31 = cosx * siny;
	_32 = -sinx;
	_33 = cosx * cosy;
	_34 = 0.0f;

	_41 = 0.0f;
	_42 = 0.0f;
	_43 = 0.0f;
	_44 = 1.0f;

#if 0 && !defined(NDEBUG)
	{
		CMatrix4f XM, YM, ZM;							            
		XM.SetXRotation( Angles.x );
		YM.SetYRotation( Angles.y );
		ZM.SetZRotation( Angles.z );
	
		CMatrix4f Out( false );
		Out.Multiply( ZM, XM );
		Out.Multiply( Out, YM );
		float32 const diff = Compare( Out );
		assert( diff < 0.01f );
	}
#endif

	assert( IsOrthonormal() );
}


bool __fastcall CMatrix4f::IsOrthonormal() const
// returns true if M is orthonormal 
// (if the rows and columns are all normalized (transform has no scaling or shearing)
// and is orthogonal (row1 cross row2 = row3 & col1 cross col2 = col3)
{
    static float kOrthoEpsilon = 2.0f*EPSILON;
	CVector3f Col1,Col2,Col3;
	CVector3f Col1CrossCol2;
	bool IsOrthonormal;
		
	assert( Matrix4f_IsValid(this) );
	
	Col1.x = RowX.x;
	Col1.y = RowY.x;
	Col1.z = RowZ.x;
	
	if ( ! Col1.IsNormalized() )
		return false;
	
	Col2.x = RowX.y;
	Col2.y = RowY.y;
	Col2.z = RowZ.y;
	
	if ( ! Col2.IsNormalized() )
		return false;
	
	Col3.x = RowX.z;
	Col3.y = RowY.z;
	Col3.z = RowZ.z;
	
	if ( ! Col3.IsNormalized() )
		return false;
	
	Col1CrossCol2.CrossProduct( Col1, Col2 );
	
	IsOrthonormal = Col1CrossCol2.Compare( Col3, kOrthoEpsilon );
	if ( ! IsOrthonormal )
	{
		Col3.Inverse( );
		IsOrthonormal = Col1CrossCol2.Compare( Col3, kOrthoEpsilon );
        if ( !IsOrthonormal )
        {
            // This does not chan the value--it's just a place to hang a breakpoint
            IsOrthonormal = false;
        }
	}
	
	return IsOrthonormal;
}

void __fastcall CMatrix4f::Orthonormalize_PreserveZ( )
{
	RowZ.Normalize( );
	RowX.CrossProduct( RowY, RowZ );
	RowX.Normalize( );
	RowY.CrossProduct( RowZ, RowX );

	RowX.w = RowY.w = RowZ.w = 0.0f;
	RowT.w = 1.0f;
	
	assert( IsOrthonormal() );
}


void __fastcall CMatrix4f::Orthonormalize()
// essentially removes scaling (or other distortions) from 
// an orthogonal (or nearly orthogonal) matrix, preserves Y
{
	RowY.Normalize( );
	RowZ.CrossProduct( RowX, RowY );
	RowZ.Normalize( );
	RowX.CrossProduct( RowY, RowZ );

	RowX.w = RowY.w = RowZ.w = 0.0f;
	RowT.w = 1.0f;
	
	assert( IsOrthonormal() );
}


bool __fastcall CMatrix4f::IsOrthogonal() const
// returns true if M is orthogonal
// (row1 cross row2 = row3 & col1 cross col2 = col3)
{
	CVector3f Col1,Col2,Col3;
	CVector3f Col1CrossCol2;
	bool IsOrthogonal;

	assert( Matrix4f_IsValid(this) );
	
	Col1.x = RowX.x;
	Col1.y = RowY.x;
	Col1.z = RowZ.x;
	
	Col2.x = RowX.y;
	Col2.y = RowY.y;
	Col2.z = RowZ.y;
	
	Col1CrossCol2.CrossProduct( Col1, Col2 );
	Col1CrossCol2.Normalize( );
	
	Col3.x = RowX.z;
	Col3.y = RowY.z;
	Col3.z = RowZ.z;
	
	Col3.Normalize( );
	
	IsOrthogonal = Col1CrossCol2.Compare( Col3, EPSILON );
	if ( ! IsOrthogonal )
	{
		Col3.Inverse( );
		IsOrthogonal = Col1CrossCol2.Compare( Col3, EPSILON );
	}
	
	return IsOrthogonal;
}

CMatrix4f __fastcall CMatrix4f::PostReflect( CPlane const& Plane ) const
{
	CMatrix4f		Dest( *this );
	float32			Dist;
	
	Dist = RowX.DotProduct( Plane.Normal );
	Dest.RowX.AddScaled3d( RowX, Plane.Normal, Dist*-2.0f);

	Dist = RowY.DotProduct( Plane.Normal );
	Dest.RowY.AddScaled3d( RowY, Plane.Normal, Dist*-2.0f);

	Dist = RowZ.DotProduct( Plane.Normal );
	Dest.RowZ.AddScaled3d( RowZ, Plane.Normal, Dist*-2.0f);

	// Mirror the translation portion of the matrix
	Dist = RowT.DotProduct( Plane.Normal ) + Plane.Distance;
	Dest.RowT.AddScaled3d( RowT, Plane.Normal, Dist*-2.0f);

	return Dest;
}



float32 __fastcall CMatrix4f::Compare(const CMatrix4f& XF)
{
	float D;
	D  = RowX.DistanceBetweenSquared(XF.RowX);
	D += RowY.DistanceBetweenSquared(XF.RowY);
	D += RowZ.DistanceBetweenSquared(XF.RowZ);
	D += RowT.DistanceBetweenSquared(XF.RowT); 

	return D;
}

void __fastcall CMatrix4f::MakeOrthoProjectionXY( float32 X, float32 Y, float32 NearClip, float32 FarClip )
{
	_11 = 1.0f / X;
	_12 = 0.0f;
	_13 = 0.0f;
	_14 = 0.0f;
	_21 = 0.0f;
	_22 = 1.0f / Y;
	_23 = 0.0f;
	_24 = 0.0f;
	_31 = 0.0f;
	_32 = 0.0f;
	_33 = 1.0f / (FarClip - NearClip);
	_34 = 0.0f;
	_41 = 0.0f;
	_42 = 0.0f;
	_43 = _33 * -NearClip;
	_44 = 1.0f;
}

void __fastcall CMatrix4f::MakePerspProjectionXY( float32 X, float32 Y, float32 NearClip, float32 FarClip )
{
    float32 const ZBufQ = FarClip / (FarClip - NearClip);
    float32 const ZBufQZN = -NearClip * ZBufQ;

	_11 = 1.0f / X;
	_12 = 0.0f;
	_13 = 0.0f;
	_14 = 0.0f;
	_21 = 0.0f;
	_22 = 1.0f / Y;
	_23 = 0.0f;
	_24 = 0.0f;
	_31 = 0.0f;
	_32 = 0.0f;
	_33 = ZBufQ;
	_34 = 1.0f;
	_41 = 0.0f;
	_42 = 0.0f;
	_43 = ZBufQZN;
	_44 = 0.0f;
}

void __fastcall CMatrix4f::MakePerspProjectionFOVAspect( float32 FOVRadians, float32 Aspect, float32 NearClip, float32 FarClip )
{
    float32 const ZBufQ = FarClip / (FarClip - NearClip);
    float32 const ZBufQZN = -NearClip * ZBufQ;
	float32 const iX = tanf( (KPi - FOVRadians) * 0.5f);

	_11 = iX;
	_12 = 0.0f;
	_13 = 0.0f;
	_14 = 0.0f;
	_21 = 0.0f;
	_22 = iX * Aspect;
	_23 = 0.0f;
	_24 = 0.0f;
	_31 = 0.0f;
	_32 = 0.0f;
	_33 = ZBufQ;
	_34 = 1.0f;
	_41 = 0.0f;
	_42 = 0.0f;
	_43 = ZBufQZN;
	_44 = 0.0f;
}

void __fastcall CMatrix4f::MakePerspProjectionFOV( float32 FOVXRadians, float32 FOVYRadians, float32 NearClip, float32 FarClip )
{
    float32 const ZBufQ = FarClip / (FarClip - NearClip);
    float32 const ZBufQZN = -NearClip * ZBufQ;
	float32 const iX = tanf( (KPi - FOVXRadians) * 0.5f);
	float32 const iY = tanf( (KPi - FOVYRadians) * 0.5f);

	_11 = iX;
	_12 = 0.0f;
	_13 = 0.0f;
	_14 = 0.0f;
	_21 = 0.0f;
	_22 = iY;
	_23 = 0.0f;
	_24 = 0.0f;
	_31 = 0.0f;
	_32 = 0.0f;
	_33 = ZBufQ;
	_34 = 1.0f;
	_41 = 0.0f;
	_42 = 0.0f;
	_43 = ZBufQZN;
	_44 = 0.0f;
}

namespace
{

//Choose an output vector perpendicular to the input vector.
//The output vector may have any magnitude, but nonzero if possible.
__forceinline void Perp( CVector3f const& Input, CVector3f* pOutput )
{
	//Find the smallest component of Input and use the other two components
	//to make a perpendicular vector.
	CVector3f Abs( fabsf( Input.x ), fabsf( Input.y ), fabsf( Input.z ) );

	if (Abs.x <= Abs.y && Abs.x <= Abs.z)
	{
		pOutput->x = 0.0f;
		pOutput->y = Input.z;
		pOutput->z = -Input.y;
	} else if (Abs.y <= Abs.z && Abs.y <= Abs.x) {
		pOutput->x = -Input.z;
		pOutput->y = 0.0f;
		pOutput->z = Input.x;
	} else {
		pOutput->x = Input.y;
		pOutput->y = -Input.x;
		pOutput->z = 0.0f;
	}
}

} //anonymous namespace

void __fastcall CMatrix4f::SetLookAt( const CVector3f& Origin, const CVector3f& Target, const CVector3f& Up )
{
	//Set RowT
	RowT = Origin;
	RowT.w = 1.0f;

	//Set RowZ
	RowZ = Target - Origin;
	if (!RowZ.Normalize()) //If Origin and Target are nearly the same point
	{
		Perp( Up, &RowZ );
		RowZ.Normalize();
	}
	RowZ.w = 0.0f;
    
	//
	//Set RowX
	//
	RowX.CrossProduct( Up, RowZ );

	//If RowX's magnitude is sufficiently large, it is safe to normalize
	if (fabsf( RowX.x ) + fabsf( RowX.y ) + fabsf( RowX.z ) < EPSILON)
	{
		//RowZ points nearly Up or down.
		//Normalization of RowX may not be possible (if RowX is the zero vector)
		//or may result in a vector not perpendicular to RowZ. Find an
		//arbitrary vector perpendicular to RowZ.
		Perp( RowZ, &RowX );
	}

	RowX.Normalize();
	RowX.w = 0.0f;

	//
	//Set RowY
	//
	RowY = RowZ.CrossProduct( RowX );
	RowY.w = 0.0f;
}

bool __fastcall CMatrix4f::IsTranslation(float32 Tolerance) const
{
	CMatrix4f Test;
	Test.RowT = RowT;
	return ( Test.Compare(*this) <= Tolerance );
}


void __fastcall CMatrix4f::TransformVec3dArrayC( const CVector3f *Source, CVector3f *Dest, int32 Count ) const
{
	for(int c = 0; c < Count; c++)
		Transform(*(Dest+c), *(Source+c));
}


void __fastcall CMatrix4f::TransformVec4dArrayC( const CVector4f *Source, CVector4f *Dest, int32 Count ) const
{
	for(int c = 0; c < Count; c++)
		Transform4d(*(Dest+c), *(Source+c));
}

void __fastcall CMatrix4f::MultiplyC(const CMatrix4f& M1, const CMatrix4f& M2)
{
	RowX.x = M1.RowX.x * M2.RowX.x + M1.RowX.y * M2.RowY.x + M1.RowX.z * M2.RowZ.x;
	RowX.y = M1.RowX.x * M2.RowX.y + M1.RowX.y * M2.RowY.y + M1.RowX.z * M2.RowZ.y;
	RowX.z = M1.RowX.x * M2.RowX.z + M1.RowX.y * M2.RowY.z + M1.RowX.z * M2.RowZ.z;
	RowX.w = 0;
	
	RowY.x = M1.RowY.x * M2.RowX.x + M1.RowY.y * M2.RowY.x + M1.RowY.z * M2.RowZ.x;
	RowY.y = M1.RowY.x * M2.RowX.y + M1.RowY.y * M2.RowY.y + M1.RowY.z * M2.RowZ.y;
	RowY.z = M1.RowY.x * M2.RowX.z + M1.RowY.y * M2.RowY.z + M1.RowY.z * M2.RowZ.z;
	RowY.w = 0;
	
	RowZ.x = M1.RowZ.x * M2.RowX.x + M1.RowZ.y * M2.RowY.x + M1.RowZ.z * M2.RowZ.x;
	RowZ.y = M1.RowZ.x * M2.RowX.y + M1.RowZ.y * M2.RowY.y + M1.RowZ.z * M2.RowZ.y;
	RowZ.z = M1.RowZ.x * M2.RowX.z + M1.RowZ.y * M2.RowY.z + M1.RowZ.z * M2.RowZ.z;
	RowZ.w = 0;
	
	RowT.x =  M2.RowX.x * M1.RowT.x
							 + M2.RowY.x * M1.RowT.y
							 + M2.RowZ.x * M1.RowT.z
							 + M2.RowT.x;
	
	RowT.y =  M2.RowX.y * M1.RowT.x
							 + M2.RowY.y * M1.RowT.y
							 + M2.RowZ.y * M1.RowT.z
							 + M2.RowT.y;
	
	RowT.z =  M2.RowX.z * M1.RowT.x
							 + M2.RowY.z * M1.RowT.y
							 + M2.RowZ.z * M1.RowT.z
							 + M2.RowT.z;

	RowT.w = 1;
}

void __fastcall CMatrix4f::Multiply4x4C(const CMatrix4f& M1, const CMatrix4f& M2)
{
	RowX.x = M1.RowX.x * M2.RowX.x + M1.RowX.y * M2.RowY.x + M1.RowX.z * M2.RowZ.x + M1.RowX.w * M2.RowT.x;
	RowX.y = M1.RowX.x * M2.RowX.y + M1.RowX.y * M2.RowY.y + M1.RowX.z * M2.RowZ.y + M1.RowX.w * M2.RowT.y;
	RowX.z = M1.RowX.x * M2.RowX.z + M1.RowX.y * M2.RowY.z + M1.RowX.z * M2.RowZ.z + M1.RowX.w * M2.RowT.z;
	RowX.w = M1.RowX.x * M2.RowX.w + M1.RowX.y * M2.RowY.w + M1.RowX.z * M2.RowZ.w + M1.RowX.w * M2.RowT.w;
	
	RowY.x = M1.RowY.x * M2.RowX.x + M1.RowY.y * M2.RowY.x + M1.RowY.z * M2.RowZ.x + M1.RowY.w * M2.RowT.x;
	RowY.y = M1.RowY.x * M2.RowX.y + M1.RowY.y * M2.RowY.y + M1.RowY.z * M2.RowZ.y + M1.RowY.w * M2.RowT.y;
	RowY.z = M1.RowY.x * M2.RowX.z + M1.RowY.y * M2.RowY.z + M1.RowY.z * M2.RowZ.z + M1.RowY.w * M2.RowT.z;
	RowY.w = M1.RowY.x * M2.RowX.w + M1.RowY.y * M2.RowY.w + M1.RowY.z * M2.RowZ.w + M1.RowY.w * M2.RowT.w;
	
	RowZ.x = M1.RowZ.x * M2.RowX.x + M1.RowZ.y * M2.RowY.x + M1.RowZ.z * M2.RowZ.x + M1.RowZ.w * M2.RowT.x;
	RowZ.y = M1.RowZ.x * M2.RowX.y + M1.RowZ.y * M2.RowY.y + M1.RowZ.z * M2.RowZ.y + M1.RowZ.w * M2.RowT.y;
	RowZ.z = M1.RowZ.x * M2.RowX.z + M1.RowZ.y * M2.RowY.z + M1.RowZ.z * M2.RowZ.z + M1.RowZ.w * M2.RowT.z;
	RowZ.w = M1.RowZ.x * M2.RowX.w + M1.RowZ.y * M2.RowY.w + M1.RowZ.z * M2.RowZ.w + M1.RowZ.w * M2.RowT.w;
	
	RowT.x = M1.RowT.x * M2.RowX.x + M1.RowT.y * M2.RowY.x + M1.RowT.z * M2.RowZ.x + M1.RowT.w * M2.RowT.x;
	RowT.y = M1.RowT.x * M2.RowX.y + M1.RowT.y * M2.RowY.y + M1.RowT.z * M2.RowZ.y + M1.RowT.w * M2.RowT.y;
	RowT.z = M1.RowT.x * M2.RowX.z + M1.RowT.y * M2.RowY.z + M1.RowT.z * M2.RowZ.z + M1.RowT.w * M2.RowT.z;
	RowT.w = M1.RowT.x * M2.RowX.w + M1.RowT.y * M2.RowY.w + M1.RowT.z * M2.RowZ.w + M1.RowT.w * M2.RowT.w;
}

void __fastcall CMatrix4f::Multiply3x3C(const CMatrix4f& M1, const CMatrix4f& M2)
{
	RowX.x = M1.RowX.x * M2.RowX.x + M1.RowX.y * M2.RowY.x + M1.RowX.z * M2.RowZ.x;
	RowX.y = M1.RowX.x * M2.RowX.y + M1.RowX.y * M2.RowY.y + M1.RowX.z * M2.RowZ.y;
	RowX.z = M1.RowX.x * M2.RowX.z + M1.RowX.y * M2.RowY.z + M1.RowX.z * M2.RowZ.z;
	RowX.w = 0;
	
	RowY.x = M1.RowY.x * M2.RowX.x + M1.RowY.y * M2.RowY.x + M1.RowY.z * M2.RowZ.x;
	RowY.y = M1.RowY.x * M2.RowX.y + M1.RowY.y * M2.RowY.y + M1.RowY.z * M2.RowZ.y;
	RowY.z = M1.RowY.x * M2.RowX.z + M1.RowY.y * M2.RowY.z + M1.RowY.z * M2.RowZ.z;
	RowY.w = 0;
	
	RowZ.x = M1.RowZ.x * M2.RowX.x + M1.RowZ.y * M2.RowY.x + M1.RowZ.z * M2.RowZ.x;
	RowZ.y = M1.RowZ.x * M2.RowX.y + M1.RowZ.y * M2.RowY.y + M1.RowZ.z * M2.RowZ.y;
	RowZ.z = M1.RowZ.x * M2.RowX.z + M1.RowZ.y * M2.RowY.z + M1.RowZ.z * M2.RowZ.z;
	RowZ.w = 0;
	
	RowT.x = 0;
	RowT.y = 0;
	RowT.z = 0;
	RowT.w = 1;
}

void __fastcall Matrix4f_TranformVec3dMultiBlendC(	const CMatrix4f * xforms, const VertexBlendSrc * vert, VertexBlendDst * destvert, int stride, int count)
{
	int ExtraData = stride - sizeof(VertexBlendSrc);
	int SrcStride = stride;
	int DestStride = sizeof(VertexBlendDst) + ExtraData;

	CMatrix4f blendxf;
	float	 weights[4];

	for(int c = 0; c < count; c++)
	{
		memset(&blendxf, 0, sizeof(blendxf));
		weights[0] = vert->w[0]; weights[1] = vert->w[1]; weights[2] = vert->w[2];
		weights[3] = 1.0f - ( weights[0] + weights[1] + weights[2] );

		for( int m = 0; m < 4; m++ )
		{
			const CMatrix4f& xf = xforms[ vert->b[m] ];
			blendxf.RowX += xf.RowX * weights[m];
			blendxf.RowY += xf.RowY * weights[m];
			blendxf.RowZ += xf.RowZ * weights[m];
			blendxf.RowT += xf.RowT * weights[m];
		}

		blendxf.Transform( *((CVector3f*)&(destvert->x)), *((CVector3f*)&(vert->x)) );
		blendxf.Rotate( *((CVector3f*)&(destvert->nx)), *((CVector3f*)&(vert->nx)) );
		memcpy( ((char*)destvert) + sizeof(VertexBlendDst), ((char*)vert) + sizeof(VertexBlendSrc), ExtraData );

		vert = (VertexBlendSrc*)( (char*)vert + SrcStride );
		destvert = (VertexBlendDst*)( (char*)destvert + DestStride );
	}
}


} // namespace Math
} // WT
