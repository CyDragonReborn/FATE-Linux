//
//	Intersections.inl
//
//	Copyright (C) 2006 WildTangent, Inc. 
//	All Rights Reserved
//

#define EPSILON 0.00001

#pragma warning( push )
#pragma warning( disable: 4127 ) //warning C4127: conditional expression is constant
template<uint32 Flags> 
	inline bool WTCall IntersectLineAndPoly(
		const CVector3f& LinePoint, const CVector3f& LineDirection,
		const CVector3f& Vert0, const CVector3f& Vert1, const CVector3f& Vert2,
		float32& rU, float32& rV, float32* pT )
{
	using namespace NIntersectLineAndPoly;

	assert( LineDirection.IsNormalized() );

	CVector3f pvec(false), qvec(false), tvec(false);

	//find vectors for two edges sharing Vert0
	CVector3f const edge1 ( Vert1 - Vert0 );
	CVector3f const edge2 ( Vert2 - Vert0 );

	//begin calculating determinant - also used to calculate U parameter
	pvec.CrossProduct( LineDirection, edge2 );

	// if determinant is near zero, line lies in plane of poly
	float32 det = edge1.DotProduct ( pvec );

	float32 U, V, T;

	if (det > EPSILON)
	{
		//calculate distance from Vert0 to line origin
		tvec = LinePoint - Vert0;
	    
		//calculate U parameter and test bounds
		U = tvec.DotProduct( pvec );
		if ( U < 0.0 || U > det )
		{
			return false;
		}
	    
		//prepare to test V parameter
		qvec.CrossProduct( tvec, edge1 );
	    
		//calculate V parameter and test bounds
		V = LineDirection.DotProduct ( qvec );

		if ((Flags & KPolyMask) == KPolyTriangle)
		{
			if ( V < 0.0 || U + V > det )
			{
				return false;
			}
		} else {
			assert( (Flags & KPolyMask) == KPolyRectangle );
			if ( V < 0.0 || V > det )
			{
				return false;
			}
		}

		if (Flags & KLineMask)
		{
			T = edge2.DotProduct( qvec );
			if ((Flags & KLineBound0) && T < 0.0f)
			{
				return false;
			}
			if ((Flags & KLineBound1) && T > det)
			{
				return false;
			}
		}
	}
	else if ( det < -EPSILON )
	{
		if (!(Flags & KDoubleSided))
		{
			return false;
		}

		//calculate distance from Vert0 to line origin
		tvec = LinePoint - Vert0;
	    
		//calculate U parameter and test bounds
		U = tvec.DotProduct ( pvec );
		if ( U > 0.0 || U < det )
		{
			return false;
		}
	    
		//prepare to test V parameter
		qvec.CrossProduct( tvec, edge1 );

		//calculate V parameter and test bounds
		V = LineDirection.DotProduct ( qvec );

		if ((Flags & KPolyMask) == KPolyTriangle)
		{
			if ( V > 0.0 || U + V < det )
			{
				return false;
			}
		} else {
			assert( (Flags & KPolyMask) == KPolyRectangle );
			if ( V > 0.0 || V < det )
			{
				return false;
			}
		}
		
		if (Flags & KLineMask)
		{
			T = edge2.DotProduct( qvec );
			if ((Flags & KLineBound0) && T > 0.0f)
			{
				return false;
			}
			if ((Flags & KLineBound1) && T < det)
			{
				return false;
			}
		}

	} else {
		//line lies in the plane of the poly
		return false;
	}

	float32 inv_det = 1.0f / det;

	//calculate T, line intersects poly
	rU = U * inv_det;
	rV = V * inv_det;

	if (pT != NULL)
	{
		if (Flags & KLineMask)
		{
			*pT = T * inv_det;
		} else {
			*pT = edge2.DotProduct( qvec ) * inv_det;
		}
	}

	return true;
}
#pragma warning( pop )

// Line/sphere intersections:
// distance( p + tD, C ) = R
// Let V = C - p
// (tD - V)^2 = R^2
// DDtt - 2tD.V + V.V - R^2 = 0
// D^2 = 1
// Let b = D.V
// Let c = V.V - R^2
// t^2 - 2bt + c = 0
// t = b + s sqrt( b^2 - c ), where s^2 = 1
inline bool WTCall IntersectLineAndSphere( const CVector3f& Center, float32 RadiusSquared,
	const CVector3f& LinePoint, const CVector3f& LineDirection, std::vector<float32>* Results )
{
	assert( LineDirection.IsNormalized() );

	CVector3f const V = Center - LinePoint;
	float32 const b = LineDirection.DotProduct( V );
	float32 const c = V.LengthSquared() - RadiusSquared;

	float32 bb = b * b;
	if (bb < c)
	{
		return false;
	}

	if (Results != NULL)
	{
		float32 rad = sqrtf( bb - c );
		Results->push_back( b - rad );
		if (rad)
		{
			Results->push_back( b + rad );
		}
	}

	return true;
}
