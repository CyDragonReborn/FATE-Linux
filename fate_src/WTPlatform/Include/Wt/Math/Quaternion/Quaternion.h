//
//	Quaternion.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#ifdef USE_DIRECTX_7
#include "d3dxmath.h"
#elif defined USE_DIRECTX_8
#include "d3dx8math.h"
#else
#include "d3dx9math.h"
#endif


/***************************************************************************

	the quatern module contains basic support for a quaternion object.

	Unlike Hamilton's original quaternions; where ij = k, jk = i, and
	ki = j; WTPlatform uses ij = -k, jk = -i, and ki = -j. This makes 
	WTPlatform compatible with D3DXQUATERNION.

	quaternions are an extension of complex numbers that allows an
	expression for rotation that can be easily interpolated.  Quaternions are also 
	more numericaly stable for repeated rotations than matrices.

	A quaternion is a 4 element 'vector'  [w,x,y,z] where:

	q = w + xi + yj + zk
	i*i = -1
	j*j = -1
	k*k = -1
	i*j = -j*i = -k
	j*k = -k*j = -i
	k*i = -i*k = -j
	q' (conjugate) = w - xi - yj - zk
	||q|| (magnitude) = sqrt(q*q') = sqrt(w*w + x*x + y*y + z*z)
	unit quaternion ||q|| == 1; this implies  q' == qinverse 
	quaternions are associative (q1*q2)*q3 == q1*(q2*q3)
	quaternions are not commutative  q1*q2 != q2*q1
	qinverse (inverse (1/q) ) = q'/(q*q')
	
	q can be expressed by w + xi + yj + zk or [w,x,y,z] 
	or as in this implementation (s,v) where s=w, and v=[x,y,z]

	quaternions can represent a rotation.  The rotation is an angle t, around a 
	unit vector u.   q=(s,v);  s= cos(t/2);   v= u*sin(t/2).

	quaternions can apply the rotation to a point.  let the point be p [px,py,pz],
	and let P be a quaternion(0,p).  Protated = qinverse*P*q
	( Protated = q'*P*q if q is a unit quaternion)

	concatenation rotations is similar to matrix concatenation.  given two rotations
	q1 and q2,  to rotate by q1, then q2:  let qc = (q1*q2), then the combined 
	rotation is given by qcinverse*P*qc (= qc'*P*qc if q is a unit quaternion)

	multiplication: 
	q1 = w1 + x1i + y1j + z1k
	q2 = w2 + x2i + y2j + z2k
	q1*q2 = q3 =
			(w1*w2 - x1*x2 - y1*y2 - z1*z2)     {w3}
	        (w1*x2 + x1*w2 - y1*z2 + z1*y2)i	{x3}
			(w1*y2 + x1*z2 + y1*w2 - z1*x2)j    {y3}
			(w1*z2 - x1*y2 + y1*x2 + z1*w2)k	{z3}

	also, 
	q1 = (s1,v1) = [s1,(x1,y1,z1)]
	q2 = (s2,v2) = [s2,(x2,y2,z2)]
	q1*q2 = q3	=	(s1*s2 - dot_product(v1,v2),			{s3}
					(s1*v2 + s2*v1 - cross_product(v1,v2))	{v3}


	interpolation - it is possible (and sometimes reasonable) to interpolate between
	two quaternions by interpolating each component.  This does not quarantee a 
	resulting unit quaternion, and will result in an animation with non-linear 
	rotational velocity.

	spherical interpolation: (slerp) treat the quaternions as vectors 
	find the angle between them (w = arccos(q1 dot q2) ).
	given 0<=t<=1,  q(t) = q1*(sin((1-t)*w)/sin(w) + q2 * sin(t*w)/sin(w).
	since q == -q, care must be taken to rotate the proper way.  

	this implementation uses the notation quaternion q = (quatV,quatS) 
	  where quatS is a scalar, and quatV is a 3 element vector.

********************************************/


namespace WT
{
namespace Math
{


//
//	CQuaternion :
//		Data class encapsulating a right handed 
//		floating point quaternion. See above for
//		more documentation
//

class CQuaternion
{
public:
	float32 x,y,z;
	float32	w;

	// C'tors
	
	//	Clears to an identity quaternion
	__forceinline CQuaternion();

	// Explicit Initialize to nothing c'tor
	__forceinline explicit CQuaternion(bool) {}

	__forceinline CQuaternion(float32 xx, float32 yy, float32 zz, float32 ww);
	__forceinline CQuaternion(const CVector3f& vec, float32 ww);
	__forceinline CQuaternion(CVector3f const& EulerAngles );
	__forceinline CQuaternion(float32 Pitch, float32 Yaw, float32 Roll);

	// Comparison operators
	__forceinline bool		  __fastcall	operator == (const CQuaternion& q) const;
	__forceinline bool		  __fastcall	operator != (const CQuaternion& q) const;

	// Quat->Quat ops
	__forceinline CQuaternion& __fastcall	operator +=	(const CQuaternion& q);
	__forceinline CQuaternion  __fastcall	operator +	(const CQuaternion& q) const;
	__forceinline CQuaternion& __fastcall	operator -=	(const CQuaternion& q);
	__forceinline CQuaternion  __fastcall	operator -	(const CQuaternion& q) const;

	__forceinline CQuaternion& __fastcall	operator *= (const CQuaternion& q);
	__forceinline CQuaternion  __fastcall	operator *	(const CQuaternion& q) const;

	//Divide by a quaternion. These assume that the divisor is a unit quaternion.
	__forceinline CQuaternion& __fastcall	operator /=	(const CQuaternion& q);
	__forceinline CQuaternion  __fastcall	operator /	(const CQuaternion& q) const;

	// Quat->Scalar ops
	// These are essentially scaling operators, no more
	__forceinline CQuaternion& __fastcall	operator *=	(const float32 v);
	__forceinline CQuaternion  __fastcall	operator *	(const float32 v) const;
	__forceinline CQuaternion& __fastcall	operator /=	(const float32 v);
	__forceinline CQuaternion  __fastcall	operator /	(const float32 v) const;

	// This is NOT an inverse,  it is a scale by -1
	__forceinline CQuaternion  __fastcall	operator -	() const;       

	// Method style accessors
				  void		 __fastcall SetFromAxisAngle	(const CVector3f& Axis, float32 Theta);
				  void		 __fastcall SetFromEulerAngles	(const CVector3f& EulerAngles );
				  void		 __fastcall SetFromEulerAngles	(float32 Pitch, float32 Yaw, float32 Roll);
				  
				  //Set the quaternion to be the shortest rotation that changes one unit vector into another 
				  //unit vector.
				  void		 __fastcall SetAlign			( CVector3f const& Source, CVector3f const& Dest );

				  bool		 __fastcall GetAxisAngle		(CVector3f* Axis, float32* Theta) const;
				  void       __fastcall ComputeEulerAngles	(CVector3f& EulerAngles) const;
				  void       __fastcall ComputeEulerAngles	(float32* pPitch, float32* pYaw, float32* pRoll) const;
	
	// Matrix conversion
				  void		 __fastcall FromMatrix			(const CMatrix4f& RotationMatrix);
				  void		 __fastcall ToMatrix			(CMatrix4f* RotationMatrixDest) const;

	// Interpolations	(from Q0 -> Q1 by T, This = result)
				  void		 __fastcall Slerp				(const CQuaternion& Q0, const CQuaternion& Q1, float32 T);
				  void		 __fastcall SlerpNotShortest	(const CQuaternion& Q0, const CQuaternion& Q1, float32 T);
	
	// Vector By CQuaternion Transform
	__forceinline CVector3f	 __fastcall Rotate				(const CVector3f& V) const;

	// Scale by -1
	__forceinline void       __fastcall Negate              ();

	// Produce an opposite CQuaternion (Q * inverse.Q) == identity.Q
	// This assumes that the object is a unit quaternion.
	__forceinline CQuaternion __fastcall Inverse				() const;

    // Compute the "dot product" of two quaternions; that is the real component of (q1 times the conjugate of q2).
    // If dot >= 0, this suggests that q2 is in the hemi-hypersphere centered about q1.
    __forceinline float32    __fastcall Dot                 (CQuaternion const& Q) const;

	// Misc. Ops
				  CQuaternion __fastcall Ln					() const;
				  CQuaternion __fastcall Exp				() const;
	__forceinline bool		 __fastcall IsUnit				() const;

	//Check for problems such as NaN values
	__forceinline bool		 __fastcall IsValid				() const;

				  float32	 __fastcall Normalize			();
	__forceinline void		 __fastcall SetNoRotation		();
	__forceinline float32	 __fastcall MagnitudeSquared	() const;
				  bool		 __fastcall Compare				(const CQuaternion& Q2, float32 Tolerance ) const;

	// D3DX support
	__forceinline explicit CQuaternion(const D3DXQUATERNION& D3DXQuat);
	__forceinline CQuaternion& __fastcall operator=(const D3DXQUATERNION& rhs);
	__forceinline D3DXQUATERNION * __fastcall GetD3DXQuatPtr();
};

__forceinline CQuaternion operator/( float32 s, CQuaternion const& q );
__forceinline CQuaternion operator*( float32 s, CQuaternion const& q );

#include "Quaternion.inl"


} // namespace Math
} // namespace WT
