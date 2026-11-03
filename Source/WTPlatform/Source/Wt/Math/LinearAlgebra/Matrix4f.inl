//
//	Matrix4f.inl
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//

__forceinline CMatrix4f::CMatrix4f(const CMatrix4f & Source)
	: RowX( Source.RowX ), RowY( Source.RowY ), RowZ( Source.RowZ ), RowT( Source.RowT )
{
}

__forceinline	CMatrix4f::CMatrix4f(float32 e11, float32 e12, float32 e13, float32 e14,
						   float32 e21, float32 e22, float32 e23, float32 e24,
						   float32 e31, float32 e32, float32 e33, float32 e34,
						   float32 e41, float32 e42, float32 e43, float32 e44) 
	: RowX( false )
	, RowY( false )
	, RowZ( false )
	, RowT( false )
{
	_11 = e11; _12 = e12; _13 = e13; _14 = e14;
	_21 = e21; _22 = e22; _23 = e23; _24 = e24;
	_31 = e31; _32 = e32; _33 = e33; _34 = e34;
	_41 = e41; _42 = e42; _43 = e43; _44 = e44;
}

__forceinline void __fastcall CMatrix4f::SetIdentity()
{
	*this = gm_Identity;
}

__forceinline CMatrix4f __fastcall CMatrix4f::operator+() const
{
	return CMatrix4f( *this );
}

__forceinline CMatrix4f __fastcall CMatrix4f::operator-() const
{
	CMatrix4f Result( false );
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		Result.e[i] = -Result.e[i];
	}
	return Result;
}

__forceinline bool __fastcall CMatrix4f::operator ==( const CMatrix4f& M ) const
{
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		if (e[i] != M.e[i])
		{
			return false;
		}
	}
	return true;
}

__forceinline bool __fastcall CMatrix4f::operator !=( const CMatrix4f& M ) const
{
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		if (e[i] != M.e[i])
		{
			return true;
		}
	}
	return false;
}

__forceinline CMatrix4f __fastcall  CMatrix4f::operator *( float32 S ) const
{
	CMatrix4f Result( false );
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		Result.e[i] = e[i] * S;
	}
	return Result;
}

__forceinline CMatrix4f __fastcall  CMatrix4f::operator /( float32 S ) const
{
	float32 const f = 1.0f / S;
	CMatrix4f Result( false );
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		Result.e[i] = e[i] * f;
	}
	return Result;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator *=( float32 S )
{
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		e[i] *= S;
	}
	return *this;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator /=( float32 S )
{
	float32 const f = 1.0f / S;
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		e[i] *= f;
	}
	return *this;
}

__forceinline CMatrix4f __fastcall  CMatrix4f::operator +( const CMatrix4f& M ) const
{
	CMatrix4f Result( false );
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		Result.e[i] = e[i] + M.e[i];
	}
	return Result;
}

__forceinline CMatrix4f __fastcall  CMatrix4f::operator -( const CMatrix4f& M ) const
{
	CMatrix4f Result( false );
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		Result.e[i] = e[i] - M.e[i];
	}
	return Result;
}

__forceinline CMatrix4f __fastcall  CMatrix4f::operator *( const CMatrix4f& M ) const
{
	CMatrix4f Result( false );
	Result.Multiply4x4C( *this, M );
	return Result;
}

__forceinline CMatrix4f __fastcall  CMatrix4f::operator /( const CMatrix4f& M ) const
{
	CMatrix4f Inverse( false );
	M.GetInverse4x4( &Inverse );
	CMatrix4f Result( false );
	Result.Multiply4x4C( *this, Inverse );
	return Result;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator +=( const CMatrix4f& M )
{
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		e[i] += M.e[i];
	}
	return *this;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator -=( const CMatrix4f& M )
{
	for (int32 i = 0; i < sizeof e / sizeof *e; ++i)
	{
		e[i] -= M.e[i];
	}
	return *this;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator *=( const CMatrix4f& M )
{
	CMatrix4f Result( false );
	Result.Multiply4x4C( *this, M );
	operator=( Result );
	return *this;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator /=( const CMatrix4f& M )
{
	CMatrix4f Inverse( false );
	M.GetInverse4x4( &Inverse );
	CMatrix4f Result( false );
	Result.Multiply4x4C( *this, Inverse );
	operator=( Result );
	return *this;
}
	
__forceinline CVector4f __fastcall  CMatrix4f::operator *( const CVector4f& V ) const
{
	CVector4f Result( false );
	Result.x = RowX.DotProduct( V );
	Result.y = RowY.DotProduct( V );
	Result.z = RowZ.DotProduct( V );
	Result.w = RowT.DotProduct( V );
	return Result;
}

__forceinline CVector3f __fastcall  CMatrix4f::operator *( const CVector3f& V ) const
{
	CVector3f Result( false );
	Result.x = RowX.CVector3f::DotProduct( V ) + RowX.w;
	Result.y = RowY.CVector3f::DotProduct( V ) + RowY.w;
	Result.z = RowZ.CVector3f::DotProduct( V ) + RowZ.w;
	return Result;
}

__forceinline void __fastcall CMatrix4f::SetTranslation(float32 x, float32 y, float32 z)
{
	RowT.x = x;
	RowT.y = y;
	RowT.z = z;
	
	RowX.x = RowY.y = RowZ.z = RowT.w = 1.0f;
	RowX.y = RowX.z = RowX.w = 0.0f;
	RowY.x = RowY.z = RowY.w = 0.0f;
	RowZ.x = RowZ.y = RowZ.w = 0.0f;
}

__forceinline void __fastcall CMatrix4f::SetTranslation(const CVector3f& V)
{
	RowT.x = V.x;
	RowT.y = V.y;
	RowT.z = V.z;
	
	RowX.x = RowY.y = RowZ.z = RowT.w = 1.0f;
	RowX.y = RowX.z = RowX.w = 0.0f;
	RowY.x = RowY.z = RowY.w = 0.0f;
	RowZ.x = RowZ.y = RowZ.w = 0.0f;
}

__forceinline void __fastcall CMatrix4f::SetScale(float32 x, float32 y, float32 z)
{
	assert( x * x >= 0.0f);
	assert( y * y >= 0.0f);
	assert( z * z >= 0.0f);

	RowX.x = x;
	RowY.y = y;
	RowZ.z = z;
	RowT.w = 1.0f;
	
	RowX.y = RowX.z = RowX.w = 0.0f;
	RowY.x = RowY.z = RowY.w = 0.0f;
	RowZ.x = RowZ.y = RowZ.w = 0.0f;
	RowT.x = RowT.y = RowT.z = 0.0f;
}

__forceinline void __fastcall CMatrix4f::SetScale(const CVector3f& V)
{
	assert( V.x * V.x >= 0.0f);
	assert( V.y * V.y >= 0.0f);
	assert( V.z * V.z >= 0.0f);

	RowX.x = V.x;
	RowY.y = V.y;
	RowZ.z = V.z;
	RowT.w = 1.0f;
	
	RowX.y = RowX.z = RowX.w = 0.0f;
	RowY.x = RowY.z = RowY.w = 0.0f;
	RowZ.x = RowZ.y = RowZ.w = 0.0f;
	RowT.x = RowT.y = RowT.z = 0.0f;
}

__forceinline void __fastcall CMatrix4f::PreRotateX(float32 RadianAngle)
{
	CMatrix4f R( false );
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	R.SetXRotation( RadianAngle );
	this->Multiply( R, *this );
}

__forceinline void __fastcall CMatrix4f::PreRotateY(float32 RadianAngle)
{
	CMatrix4f R( false );
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	R.SetYRotation( RadianAngle );
	this->Multiply( R, *this );
}

__forceinline void __fastcall CMatrix4f::PreRotateZ(float32 RadianAngle)
{
	CMatrix4f R( false );
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	R.SetZRotation( RadianAngle );
	this->Multiply( R, *this );
}

__forceinline void __fastcall CMatrix4f::PostRotateX(float32 RadianAngle)
{
	CMatrix4f R( false );
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	R.SetXRotation( RadianAngle );
	this->Multiply( *this, R );
}

__forceinline void __fastcall CMatrix4f::PostRotateY(float32 RadianAngle)
{
	CMatrix4f R( false );
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	R.SetYRotation( RadianAngle );
	this->Multiply( *this, R );
}

__forceinline void __fastcall CMatrix4f::PostRotateZ(float32 RadianAngle)
{
	CMatrix4f R( false );
	assert( RadianAngle * RadianAngle >= 0.0f );
	
	R.SetZRotation( RadianAngle );
	this->Multiply( *this, R );
}

__forceinline void __fastcall CMatrix4f::PreTranslate(float32 x, float32 y, float32 z)
{
	CVector3f V( x, y, z );
	CVector3f Result( false );
	RotateNoOverlap( Result, V );
	RowT.CVector3f::operator+=( Result );
}
	
__forceinline void __fastcall CMatrix4f::PreScale(float32 x, float32 y, float32 z)
{
	RowX.CVector3f::operator*=( x );
	RowY.CVector3f::operator*=( y );
	RowZ.CVector3f::operator*=( z );
}

__forceinline void __fastcall CMatrix4f::PreScale(const CVector3f& V)
{
	RowX.CVector3f::operator*=( V.x );
	RowY.CVector3f::operator*=( V.y );
	RowZ.CVector3f::operator*=( V.z );
}

__forceinline void __fastcall CMatrix4f::PreTranslate(const CVector3f& V)
{
	CVector3f Result( false );
	RotateNoOverlap( Result, V );
	RowT.CVector3f::operator+=( Result );
}

__forceinline void __fastcall CMatrix4f::PostTranslate(float32 x, float32 y, float32 z)
{
	RowT.x += x;
	RowT.y += y;
	RowT.z += z;
}

__forceinline void __fastcall CMatrix4f::PostTranslate(const CVector3f& V)
{
	RowT.x += V.x;
	RowT.y += V.y;
	RowT.z += V.z;
}

__forceinline void __fastcall CMatrix4f::PostScale(const CVector3f& S)
{
	RowX.x *= S.x;
	RowX.y *= S.y;
	RowX.z *= S.z;
	RowY.x *= S.x;
	RowY.y *= S.y;
	RowY.z *= S.z;
	RowZ.x *= S.x;
	RowZ.y *= S.y;
	RowZ.z *= S.z;
	RowT.x *= S.x;
	RowT.y *= S.y;
	RowT.z *= S.z;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::Multiply(const CMatrix4f& M1, const CMatrix4f& M2) // result = this
{
	if( this == &M1 || this == &M2)
	{
		CMatrix4f M( false );
		M.MultiplyC( M1, M2 );
		*this = M;
	}
	else
		this->MultiplyC( M1, M2 );

	return *this;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::Multiply4x4(const CMatrix4f& M1, const CMatrix4f& M2) // result = this
{
	if( this == &M1 || this == &M2)
	{
		CMatrix4f M( false );
		M.Multiply4x4C( M1, M2 );
		*this = M;
	}
	else
		this->Multiply4x4C( M1, M2 );

	return *this;
}

__forceinline CMatrix4f& __fastcall CMatrix4f::Multiply3x3(const CMatrix4f& M1, const CMatrix4f& M2) // result = this
{
	if( this == &M1 || this == &M2)
	{
		CMatrix4f M( false );
		M.Multiply3x3C( M1, M2 );
		*this = M;
	}
	else
		this->Multiply3x3C( M1, M2 );

	return *this;
}


__forceinline void __fastcall CMatrix4f::GetAxes	(CVector3f& X, CVector3f& Y, CVector3f& Z) const
{
	X = RowX;
	Y = RowY;
	Z = RowZ;
}

__forceinline void __fastcall CMatrix4f::SetFromAxes(const CVector3f& X, const CVector3f& Y, const CVector3f& Z)
{
	RowX = X;
	RowY = Y;
	RowZ = Z;
	RowT.x = RowT.y = RowT.z = 0;
	RowX.w = RowY.w = RowZ.w = 0;
	RowT.w = 1;
}

__forceinline float32 __fastcall CMatrix4f::ComputeDeterminant3x3() const
{
	CVector3f Cross;
	Cross.CrossProduct( RowX, RowY );
	return Cross.DotProduct( RowZ );
}

__forceinline CMatrix4f __fastcall CMatrix4f::GetTranspose() const
{
	CMatrix4f MInv( false );
	
	MInv.RowX.x = RowX.x;
	MInv.RowX.y = RowY.x;
	MInv.RowX.z = RowZ.x;
	MInv.RowX.w = 0.0f;
	
	MInv.RowY.x = RowX.y;
	MInv.RowY.y = RowY.y;
	MInv.RowY.z = RowZ.y;
	MInv.RowY.w = 0.0f;
	
	MInv.RowZ.x = RowX.z;
	MInv.RowZ.y = RowY.z;
	MInv.RowZ.z = RowZ.z;
	MInv.RowZ.w = 0.0f;
	
	MInv.RowT.x = 0.0f;
	MInv.RowT.y = 0.0f;
	MInv.RowT.z = 0.0f;
	MInv.RowT.w = 1.0f;
	
	MInv.PreTranslate( RowT.CVector3f::operator-() );

	return MInv;
}

__forceinline CMatrix4f __fastcall CMatrix4f::GetTranspose4x4() const
{
	CMatrix4f MTran( false );
	
	MTran.RowX.x = RowX.x;
	MTran.RowX.y = RowY.x;
	MTran.RowX.z = RowZ.x;
	MTran.RowX.w = RowT.x;
	
	MTran.RowY.x = RowX.y;
	MTran.RowY.y = RowY.y;
	MTran.RowY.z = RowZ.y;
	MTran.RowY.w = RowT.y;
	
	MTran.RowZ.x = RowX.z;
	MTran.RowZ.y = RowY.z;
	MTran.RowZ.z = RowZ.z;
	MTran.RowZ.w = RowT.z;

	MTran.RowT.x = RowX.w;
	MTran.RowT.y = RowY.w;
	MTran.RowT.z = RowZ.w;
	MTran.RowT.w = RowT.w;

	return MTran;
}



__forceinline void __fastcall CMatrix4f::Transform( CVector3f& Result, const CVector3f& V ) const
{
	if ( &V == &Result ) // optimizer should remove this compare for us !?
	{
		CVector3f VL = V;
		TransformNoOverlap( Result, VL );
	}
	else
		TransformNoOverlap( Result, V );
}

__forceinline void __fastcall CMatrix4f::Transform4d( CVector4f& Result, const CVector4f& V ) const
{
	if ( &V == &Result ) // optimizer should remove this compare for us !?
	{
		CVector4f VL = V;
		Transform4dNoOverlap( Result, VL );
	}
	else
		Transform4dNoOverlap( Result, V );
}

__forceinline void __fastcall CMatrix4f::Rotate( CVector3f& Result, const CVector3f& V ) const
{
	if ( &V == &Result ) // optimizer should remove this compare for us !?
	{
		CVector4f VL = V;
		RotateNoOverlap( Result, VL );
	}
	else
		RotateNoOverlap( Result, V );
}

__forceinline void __fastcall CMatrix4f::TransformVec3dArray( const CVector3f *Source, CVector3f *Dest, int32 Count ) const
{
	TransformVec3dArrayC( Source, Dest, Count );
}

__forceinline void __fastcall CMatrix4f::TransformVec4dArray( const CVector4f *Source, CVector4f *Dest, int32 Count ) const
{
	TransformVec4dArrayC( Source, Dest, Count );
}

__forceinline CVector3f __fastcall  CVector3f::operator *( const CMatrix4f& M ) const
{
	CVector3f Result( false );
	M.TransformNoOverlap( Result, *this );
	return Result;
}

__forceinline CVector3f __fastcall  CVector3f::operator /( const CMatrix4f& M ) const
{
	CMatrix4f Inverse = M.GetInverse();
	CVector3f Result( false );
	Inverse.TransformNoOverlap( Result, *this );
	return Result;
}

__forceinline CVector4f __fastcall  CVector4f::operator *( const CMatrix4f& M ) const
{
	CVector4f Result( false );
	M.TransformNoOverlap( Result, *this );
	return Result;
}

__forceinline CVector4f __fastcall  CVector4f::operator /( const CMatrix4f& M ) const
{
	CMatrix4f Inverse( false );
	M.GetInverse4x4( &Inverse );
	CVector4f Result( false );
	Inverse.TransformNoOverlap( Result, *this );
	return Result;
}

// private

__forceinline void __fastcall CMatrix4f::TransformNoOverlap( CVector3f& Result, const CVector3f& V ) const
{
	assert( &Result != &V );
	Result.x = (V.x * RowX.x) + (V.y * RowY.x) + (V.z * RowZ.x) + RowT.x;
	Result.y = (V.x * RowX.y) + (V.y * RowY.y) + (V.z * RowZ.y) + RowT.y;
	Result.z = (V.x * RowX.z) + (V.y * RowY.z) + (V.z * RowZ.z) + RowT.z;
}

__forceinline void __fastcall CMatrix4f::Transform4dNoOverlap( CVector4f& Result,	const CVector4f& V ) const
{
	assert( &Result != &V );
	Result.x = (V.x * RowX.x) + (V.y * RowY.x) + (V.z * RowZ.x) + (V.w * RowT.x);
	Result.y = (V.x * RowX.y) + (V.y * RowY.y) + (V.z * RowZ.y) + (V.w * RowT.y);
	Result.z = (V.x * RowX.z) + (V.y * RowY.z) + (V.z * RowZ.z) + (V.w * RowT.z);
	Result.w = (V.x * RowX.w) + (V.y * RowY.w) + (V.z * RowZ.w) + (V.w * RowT.w);
}

__forceinline void __fastcall CMatrix4f::RotateNoOverlap( CVector3f& Result, const CVector3f& V ) const
{
	assert( &Result != &V );
	Result.x = (V.x * RowX.x) + (V.y * RowY.x) + (V.z * RowZ.x);
	Result.y = (V.x * RowX.y) + (V.y * RowY.y) + (V.z * RowZ.y);
	Result.z = (V.x * RowX.z) + (V.y * RowY.z) + (V.z * RowZ.z);
}

__forceinline CMatrix4f operator*( float32 s, CMatrix4f const& m )
{
	return m * s;
}

__forceinline CMatrix4f operator/( float32 s, CMatrix4f const& m )
{
	CMatrix4f Inverse( false );
	m.GetInverse4x4( &Inverse );
	return Inverse * s;
}


// D3D support

__forceinline CMatrix4f::CMatrix4f(const D3DMATRIX& D3DSource)
	: RowX( false )
	, RowY( false )
	, RowZ( false )
	, RowT( false )
{
	_copyFrom(D3DSource);
}

__forceinline CMatrix4f& __fastcall CMatrix4f::operator=(const D3DMATRIX& rhs)
{
	_copyFrom(rhs);
	return *this;
}

__forceinline void __fastcall CMatrix4f::_copyFrom(const D3DMATRIX& D3DSource)
{
	_11 = D3DSource._11; _12 = D3DSource._12; _13 = D3DSource._13; _14 = D3DSource._14;
	_21 = D3DSource._21; _22 = D3DSource._22; _23 = D3DSource._23; _24 = D3DSource._24;
	_31 = D3DSource._31; _32 = D3DSource._32; _33 = D3DSource._33; _34 = D3DSource._34;
	_41 = D3DSource._41; _42 = D3DSource._42; _43 = D3DSource._43; _44 = D3DSource._44;
}

__forceinline D3DMATRIX* __fastcall CMatrix4f::GetD3DMatrixPtr()
{
	return reinterpret_cast<D3DMATRIX *>( e );
}

__forceinline D3DMATRIX const* __fastcall CMatrix4f::GetD3DMatrixPtr() const
{
	return reinterpret_cast<D3DMATRIX const*>( e );
}

__forceinline __fastcall CMatrix4f::operator D3DMATRIX*()
{
	return reinterpret_cast<D3DMATRIX *>( e );
}

__forceinline __fastcall CMatrix4f::operator const D3DMATRIX*() const
{
	return reinterpret_cast<const D3DMATRIX *>( e );
}

__forceinline D3DXMATRIX* __fastcall CMatrix4f::GetD3DXMatrixPtr()
{
	return reinterpret_cast<D3DXMATRIX *>( e );
}

__forceinline D3DXMATRIX const* __fastcall CMatrix4f::GetD3DXMatrixPtr() const
{
	return reinterpret_cast<D3DXMATRIX const*>( e );
}
