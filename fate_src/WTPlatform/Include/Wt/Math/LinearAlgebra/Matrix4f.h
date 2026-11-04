//
//	Matrix4f.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Vector4f.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Plane.h"
#include "WT/Math/Rects/Rect2f.h"
#include "windows.h" //Needed by d3d9types.h
#ifdef USE_DIRECTX_7
#include "d3dtypes.h"
#include "d3dxmath.h"
#elif defined USE_DIRECTX_8
#include "d3d8types.h"
#include "d3dx8math.h"
#else
#include "d3d9types.h"
#include "d3dx9math.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4201 ) //nonstandard extension used : nameless struct/union


namespace WT
{
namespace Math
{


//
//	CMatrix4f :
//		Data class encapsulating a 4x4 floating point
//		matrix. Uses CVector4f for internal storage.
//
//	Note: while CMatrix4f is a 4x4 matrix class, and
//	has all functionality necessary to do 4x4 operations
//	it is at heart optimized to be a 3x4 matrix class,
//	a 3x3 rotational matrix, with a fourth 3d translation
//	component. Thus by default it is assumed that when
//	you multiply 2 Matrix'4fs together it will perform
//	a 3x4 multiply, and so on for other operations.
//
//	In a 3x4 operation it is assumed that the 4th element
//  for the X,Y,Z basis rows is 0, and the 4th element for
//	the W (or Translation) row is 1.
//
//	Be carefull when using the basis rows directly. If you
//	modify the 4th element manually, and you wish it to
//	take effect, you will need to start using Multiply4x4,
//	or Transform4d and so on for it to take effect. Regular
//	Multiply and Transform commands will still assume
//	{0,0,0,1} for the 4th rows elements.
//
//	It is recommended you use the regular 3x4 matrix ops
//	in your program. Unless you start working in a true
//	4d space, or in projected space (w = 1/z), you will
//	never need true 4x4 operations, and their 3x4 equivalents
//	have been optimized to do the minimal ammount of work.
//
//	CMatrix4f also has SSE assembly optimized equivalents
//	of the more important operations. While you may call
//	these methods manually, currentely Chimera is lacking
//	an auto detecting CPU mechanism which will switch
//	the internal calls to start taking advantage of the SSE
//	code automatically when running on such a platform.
// ---------------------------------------------------


class CMatrix4f
{
public:
	// unioned data storage
	union
	{
		struct
		{
			float32        _11, _12, _13, _14;	//< x basis
            float32        _21, _22, _23, _24;	//< y basis
            float32        _31, _32, _33, _34;	//< z basis
            float32        _41, _42, _43, _44;	//< w basis
		};
		
		struct
		{
			CVector4f	RowX;	//< X basis row, x axis
			CVector4f	RowY;	//< Y basis row, y axis
			CVector4f	RowZ;	//< Z basis row, z axis
			CVector4f	RowT;	//< W basis row, translation
		};

		struct
		{
			float32			e[16];
		};
	};
	
	//Static member variables.
	//Note: these are defined after the non-static member variables in order to improve
	//Visual Studio tool tip appearance.
	static CMatrix4f const gm_Zero; //Zero matrix
	static CMatrix4f const gm_Identity; //Identity matrix

	// C'tors

	// Default C'tor initializes to identity
	__forceinline CMatrix4f() : RowX(1,0,0,0), RowY(0,1,0,0), RowZ(0,0,1,0), RowT(0,0,0,1) {}

	// Non initialzing c'tor (perf)
	__forceinline explicit	CMatrix4f(bool) 
		: RowX( false )
		, RowY( false )
		, RowZ( false )
		, RowT( false )
	{}

	// Assigning C'tors
    __forceinline CMatrix4f(const CMatrix4f& Source);
	__forceinline CMatrix4f(float32 e11, float32 e12, float32 e13, float32 e14,
						   float32 e21, float32 e22, float32 e23, float32 e24,
						   float32 e31, float32 e32, float32 e33, float32 e34,
						   float32 e41, float32 e42, float32 e43, float32 e44);

	// Euler C'Tor (euler vector holds the 3 rotations, constructs the matrix holding the rotations directly
	__forceinline explicit CMatrix4f( const CVector3f& euler ) 
		: RowX( false )
		, RowY( false )
		, RowZ( false )
		, RowT( false )
	{ 
		SetEulerAngles(euler);
	}

	// Reset to identity
	__forceinline	void __fastcall SetIdentity();

	// Unary operators
	__forceinline CMatrix4f __fastcall operator+() const;
	__forceinline CMatrix4f __fastcall operator-() const;

	// Comparison operators
	__forceinline bool __fastcall operator ==( const CMatrix4f& M ) const;
	__forceinline bool __fastcall operator !=( const CMatrix4f& M ) const;

	// Matrix/scalar operators

	__forceinline CMatrix4f __fastcall  operator *( float32 S ) const;
	// Divide by scalar S. The results are undefined if S is zero.
	__forceinline CMatrix4f __fastcall  operator /( float32 S ) const;
	__forceinline CMatrix4f& __fastcall operator *=( float32 S );
	// Divide by scalar S. The results are undefined if S is zero.
	__forceinline CMatrix4f& __fastcall operator /=( float32 S );

	// Matrix/matrix operators

	__forceinline CMatrix4f __fastcall  operator +( const CMatrix4f& M ) const;
	__forceinline CMatrix4f __fastcall  operator -( const CMatrix4f& M ) const;
    __forceinline CMatrix4f __fastcall  operator *( const CMatrix4f& M ) const;
	// Divide by matrix M. The results are undefined if M is not invertible.
	__forceinline CMatrix4f __fastcall  operator /( const CMatrix4f& M ) const;

	__forceinline CMatrix4f& __fastcall operator +=( const CMatrix4f& M );
	__forceinline CMatrix4f& __fastcall operator -=( const CMatrix4f& M );
	__forceinline CMatrix4f& __fastcall operator *=( const CMatrix4f& M );
	// Divide by matrix M. The results are undefined if M is not invertible.
	__forceinline CMatrix4f& __fastcall operator /=( const CMatrix4f& M );
	
	// Matrix/vector operators

	// Normally, one should multiply a vector and a matrix. This instead
	// multiplies a matrix and the vector. With a rotation matrix, this has 
	// the effect of performing an inverse rotation on the vector.
	__forceinline CVector4f __fastcall  operator *( const CVector4f& V ) const;

	// Nomally, one should multiply a vector and a matrix. This instead
	// multiplies a matrix and a 3-vector. The vector is treated as a 4-vector
	// with a fourth component of 1. The 4th component of the result is discarded.
	__forceinline CVector3f __fastcall  operator *( const CVector3f& V ) const;
	
	// State queries
	bool __fastcall IsInvertible3x3() const	{ return (0.0f != ComputeDeterminant3x3()); }
	bool __fastcall IsOrthonormal() const;
	bool __fastcall IsOrthogonal() const;
	bool __fastcall IsTranslation(float32 Tolerance) const;

	// Orthonormalize matrix, preseve current Y direction and translation
	void __fastcall Orthonormalize();
	// Orthonormalize matrix, preseve current Z direction and translation
	void __fastcall Orthonormalize_PreserveZ();
	
	// Reset the current state of the matrix to reflect ONLY this operation
	void __fastcall SetXRotation (float32 RadianAngle);
	void __fastcall SetYRotation (float32 RadianAngle);
	void __fastcall SetZRotation (float32 RadianAngle);
	void __fastcall SetAxisRotation(const CVector3f& Axis, float32 RadianAngle);
	__forceinline void	__fastcall SetTranslation(const CVector3f &V);
	__forceinline void	__fastcall SetTranslation(float32 x, float32 y, float32 z);
	__forceinline void	__fastcall SetScale(const CVector3f &V);
	__forceinline void	__fastcall SetScale(float32 x, float32 y, float32 z);
	
	// Pre-multiply by transformation T: M' = TM, so v' = vM' = vTM.
	__forceinline void	__fastcall PreRotateX(float32 RadianAngle);  
	__forceinline void	__fastcall PreRotateY(float32 RadianAngle);
	__forceinline void	__fastcall PreRotateZ(float32 RadianAngle);
	__forceinline void	__fastcall PreTranslate(float32 x, float32 y, float32 z);	
	__forceinline void	__fastcall PreTranslate(const CVector3f& V);
	__forceinline void	__fastcall PreScale(float32 x, float32 y, float32 z);
	__forceinline void	__fastcall PreScale(const CVector3f& V);

	// Post-multiply by transformation T: M' = MT, so v' = vM' = vMT.
	__forceinline void	__fastcall PostRotateX(float32 RadianAngle);  
	__forceinline void	__fastcall PostRotateY(float32 RadianAngle);
	__forceinline void	__fastcall PostRotateZ(float32 RadianAngle);
	__forceinline void	__fastcall PostTranslate(float32 x, float32 y, float32 z);
	__forceinline void	__fastcall PostTranslate(const CVector3f& V);
	__forceinline void	__fastcall PostScale(const CVector3f& S);
	
	// Matrix multiplication, This = Result
	__forceinline CMatrix4f& __fastcall Multiply(const CMatrix4f& M1, const CMatrix4f& M2); // result = this, assumed the 4 colum = (0,0,0,1)
	__forceinline CMatrix4f __fastcall Multiply(const CMatrix4f& M2) const { return CMatrix4f().Multiply(*this, M2); }
	
	__forceinline CMatrix4f& __fastcall Multiply4x4(const CMatrix4f& M1, const CMatrix4f& M2); // no assumptions
	__forceinline CMatrix4f	__fastcall Multiply4x4(const CMatrix4f& M2) const { return CMatrix4f().Multiply4x4(*this, M2); }

	__forceinline CMatrix4f& __fastcall Multiply3x3(const CMatrix4f& M1, const CMatrix4f& M2); // 3x3 multiply... basically rotations...
	__forceinline CMatrix4f __fastcall Multiply3x3(const CMatrix4f& M2) const { return CMatrix4f().Multiply3x3(*this, M2); }
	
	__forceinline void	__fastcall GetAxes(CVector3f& X, CVector3f& Y, CVector3f& Z) const;
	__forceinline void	__fastcall SetFromAxes(const CVector3f& X, const CVector3f& Y, const CVector3f& Z);
	
	// Misc. Ops
	__forceinline float32 __fastcall ComputeDeterminant3x3() const;
	
	CMatrix4f __fastcall GetInverse() const;	// quickie inverse of 3x3 with an appended translation
	bool __fastcall GetInverse4x4( CMatrix4f* pResult ) const; // Full inverse; returns false on failure

	// GetTranpose() transposes the 3x3 portion of a matrix. GetTranspose() will 
	// also negate the the translation portion of the matrix (RowT.x, RowT.y, and RowT.z)
	// and transform it with the transposed 3x3 matrix. Thus, GetTranspose() is suitable
	// for inverting a combined rotation and translation.
	__forceinline CMatrix4f	__fastcall GetTranspose () const;

	__forceinline CMatrix4f	__fastcall GetTranspose4x4 () const;	// full 4x4 transpose

	// Get Euler angles is not guaranteed to return a specific set of Euler
	// angles, since there are infinite valid combinations
	CVector3f __fastcall GetEulerAngles() const;
	
	// Euler angles are in radians and consist of pitch (x), yaw (y), and roll (z). The actual
	// rotation order is: about Z, then about X, and finally about Y.
	void __fastcall SetEulerAngles(const CVector3f& Angles);

	// Compute and return a matrix equal to the current transformation followed by a reflection through a plane
	CMatrix4f __fastcall PostReflect( class CPlane const& Plane ) const;

	// Get a Mean Squared Average differential between the 2 matrices
	float32 __fastcall Compare(const CMatrix4f& pXF2);

	//Compute a perspective projection matrix based on screen half-width and half-height 
	//(maximum X and Y in view space).
	void __fastcall MakeOrthoProjectionXY( float32 X, float32 Y, float32 NearClip, float32 FarClip );

	//Compute a perspective projection matrix based on screen half-width and half-height at 
	//the far clip plane (maximum X / Z and Y / Z in view space).
	void __fastcall MakePerspProjectionXY( float32 X, float32 Y, float32 NearClip, float32 FarClip );

	// Compute a perspective projection matrix based on horizontal FOV and aspect ratio
	//(screen width divided by height).
	void __fastcall MakePerspProjectionFOVAspect( float32 FOVRadians, float32 Aspect, float32 NearClip, float32 FarClip );

	// Compute a perspective projection matrix based on horizontal and vertical FOV.
	void __fastcall MakePerspProjectionFOV( float32 FOVXRadians, float32 FOVYRadians, float32 NearClip, float32 FarClip );

	// Make an orthonormal matrix with the Z axis pointing from Origin to Target. Specify a unit vector for Up; the
	// Y axis will be aligned as closely as possible with it. The matrix translation will be set to the Origin.
	void __fastcall SetLookAt( const CVector3f& Origin, const CVector3f& Target, const CVector3f& Up );

	// Vector Transformation
	__forceinline void __fastcall Transform( CVector3f& Result, const CVector3f& V ) const;
	__forceinline void __fastcall Transform4d( CVector4f& Result,	const CVector4f& V ) const;
	__forceinline void __fastcall Rotate( CVector3f& Result, const CVector3f& V ) const;

	// Vector transformation - arguments must not overlap
	__forceinline void __fastcall TransformNoOverlap( CVector3f& Result, const CVector3f& V ) const;
	__forceinline void __fastcall Transform4dNoOverlap( CVector4f& Result,	const CVector4f& V ) const;
	__forceinline void __fastcall RotateNoOverlap( CVector3f& Result, const CVector3f& V ) const;

	// Vector Array Transformation
	__forceinline void __fastcall TransformVec3dArray( const CVector3f *Source, CVector3f *Dest, int32 Count ) const;
	__forceinline void __fastcall TransformVec4dArray( const CVector4f *Source, CVector4f *Dest, int32 Count ) const;


	// D3D support
	__forceinline explicit CMatrix4f(const D3DMATRIX& D3DSource);
	__forceinline CMatrix4f& __fastcall operator=(const D3DMATRIX& rhs);
	__forceinline D3DMATRIX* __fastcall GetD3DMatrixPtr();
	__forceinline D3DMATRIX const* __fastcall GetD3DMatrixPtr() const;
	__forceinline __fastcall operator D3DMATRIX*();
	__forceinline __fastcall operator const D3DMATRIX*() const;

	// D3DX support
	__forceinline D3DXMATRIX* __fastcall GetD3DXMatrixPtr();
	__forceinline D3DXMATRIX const* __fastcall GetD3DXMatrixPtr() const;

private:

	__forceinline void __fastcall _copyFrom(const D3DMATRIX& D3DSource);

	// Assembly optimized routines, not supported, but knock yourself out
	void __fastcall MultiplyC(const CMatrix4f& M1, const CMatrix4f& M2); // result = this, assumed the 4 colum = (0,0,0,1)
	void MultiplySSE(const CMatrix4f& M1, const CMatrix4f& M2); // result = this, assumed the 4 colum = (0,0,0,1)
	void __fastcall Multiply4x4C(const CMatrix4f& M1, const CMatrix4f& M2); // no assumptions
	void __fastcall Multiply3x3C(const CMatrix4f& M1, const CMatrix4f& M2); // 3x3 multiply... basically rotations...

	void __fastcall TransformVec3dArrayC( const CVector3f *Source, CVector3f *Dest, int32 Count ) const;
	void __fastcall TransformVec4dArrayC( const CVector4f *Source, CVector4f *Dest, int32 Count ) const;

	void __fastcall TransformVec3dArraySSE( const CVector3f *Source, CVector3f *Dest, int32 Count ) const;
	void __fastcall TransformVec4dArraySSE( const CVector4f *Source, CVector4f *Dest, int32 Count ) const;

};

__forceinline CMatrix4f operator*( float32 s, CMatrix4f const& m );
__forceinline CMatrix4f operator/( float32 s, CMatrix4f const& m );

//
//		SKINNED MATRIX TRANSFORMS!!
//
//		NOTES:!:!:!:!:!
//		for speed purposes the functions below work on 4 single precision
//		values at once, reading processing and writing. now the vector data
//		is composed of only 3 values. this is okay. however you MUST ensure
//		that right past the 'nz' value in the last entry in dest array there
//		is an extra 4 bytes valid for read/write!!! Typically this will not be
//		a problem since people won't work directly with _FVECTOR_BLENDDST/SRC
//		but have similar memory compatible structures with extra data (colors eg)
//		and use the stride parameter to compesate for it.
//	
//		the STRIDE parameter refers to the stride of the SOURCE array. it is
//		assumed that the same ammount of EXTRA data (Stride - sizeof(_FVECTOR_BLENDSRC))
//		is also present in the DEST array  
//			(deststride = sizeof(_FVECTOR_BLENDDST) + (Stride - sizeof(_FVECTOR_BLENDSRC)))
//		the stride value MUST MUST MUST be a multiple of 4 (your extra data is DWORD aligned)
//


struct VertexBlendDst
{
	float32 x, y, z;
	float32 nx, ny, nz;
};

struct VertexBlendSrc
{
	float32 x, y, z;
	float32 w[3];
	uint8 b[4];
	float32 nx, ny, nz;
};

void __fastcall CMatrix4f_TranformVec3dMultiBlendC( const CMatrix4f * xforms, 
												   const VertexBlendSrc * vert, 
												   VertexBlendDst * destvert, 
												   int32 stride, int32 count);

void __fastcall CMatrix4f_TranformVec3dMultiBlendSSE( const CMatrix4f * xforms, 
													 const VertexBlendSrc * vert, 
													 VertexBlendDst * destvert, 
													 int32 stride, int32 count);

#define CMatrix4f_TranformVec3dMultiBlend CMatrix4f_TranformVec3dMultiBlendC


#include "Matrix4f.inl"


} // namespace Math
} // namespace WT

#pragma warning( pop )
