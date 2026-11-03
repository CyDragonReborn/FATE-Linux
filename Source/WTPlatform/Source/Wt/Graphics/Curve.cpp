/***************************************************************************************
 *
 * Curve.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 1/5/2006: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Graphics/Curve.h"
#include "WT/System/Serialization/SerializationException.h"

using namespace WT::System;
using namespace WT::System::Serialization;
using namespace WT::Math;
using namespace WT::Graphics;

namespace WT
{
namespace Graphics
{

namespace
{

//The integrand in the cubic spline arc length calculation is the square root of the quartic 
//polynomial Coef[4] t^4 + Coef[3] t^3 + Coef[2] t^2 + Coef[1] t + Coef[0].
float32 WTCall SplineArcLengthIntegrand( float32 t, void const* pData )
{
	float32 const t2 = t * t;
	float32 const (&Coef)[5] = *static_cast<float32 const (*)[5]>( pData );
	return sqrtf( Coef[4] * t2 * t2 + Coef[3] * t2 * t + Coef[2] * t2 + Coef[1] * t + Coef[0] );
}

//Compute the arc length of the cubic spline in the interval [Param0, Param1].
float32 ComputeSplineArcLength( float32 Param0, float32 Param1, float32 const (&Coef)[5] )
{
	return AdaptiveIntegrate( SplineArcLengthIntegrand, Coef, Param0, Param1 );
}

//Get the polynomial coefficients of the cubic spline.
void GetSplinePolyCoefficients( CCurve::Knot const* pCur, CCurve::Knot const* pNext,
									  CVector3f (&PolyCoef)[4] )
{
/*
	The position of the Bezier curve function at parameter t is 
	
	f(t) = P0 * (1 - t)^3 + 3P1 * t(1 - t)^2 + 3P2 * t^2(1 - t) + P3 * t^3
	
	or

	f(t) = At^3 + Bt^2 + Ct + D
	
	where

	A = P3 - 3P2 + 3P1 - P0
	B = 3P2 - 6P1 + 3P0
	C = 3P1 - 3P0
	D = P0
*/
	CVector3f const& P0 = pCur->m_Position;
	CVector3f const& P1 = pCur->m_Right;
	CVector3f const& P2 = pNext->m_Left;
	CVector3f const& P3 = pNext->m_Position;
	PolyCoef[3] = P3 - P0 - 3 * (P2 - P1);
	PolyCoef[2] = 3 * (P2 - P1 - P1 + P0);
	PolyCoef[1] = 3 * (P1 - P0);
	PolyCoef[0] = P0;
}

void GetSplineArcLengthCoefficients( CCurve::Knot const* pCur, CCurve::Knot const* pNext,
									  float32 (&Coef)[5] )
{
	/*
	The position of the Bezier curve function at parameter t is 

	f(t) = At^3 + Bt^2 + Ct + D

	therefore

	f'(t) = 3At^2 + 2Bt + C

	and

	|f'(t)| = sqrt[ 9AAt^4 + 12ABt^3 + (6AC + 4BB)t^2 + 4BCt + CC ]
    = sqrt( Et^4 + Ft^3 + Gt^2 + Ht + I )

	where  

	E = 9AA
	F = 12AB
	G = 6AC + 4BB
	H = 4BC
	I = CC

	The arc length L(t) is the integral of |f'(t)|.
	This function returns coefficients E, F, G, H, and I.
	*/

	CVector3f PolyCoef[4];
	GetSplinePolyCoefficients( pCur, pNext, PolyCoef );

	Coef[4] = 9.0f * PolyCoef[3].LengthSquared();
	Coef[3] = 12.0f * PolyCoef[3].DotProduct( PolyCoef[2] );
	Coef[2] = 6.0f * PolyCoef[3].DotProduct( PolyCoef[1] ) + 4.0f * PolyCoef[2].LengthSquared();
	Coef[1] = 4.0f * PolyCoef[2].DotProduct( PolyCoef[1] );
	Coef[0] = PolyCoef[1].LengthSquared();
}

}

WT_DEFINE_CLASS( CCurve )

uint32 WTCall CCurve::GetClassFormatVersion() {return 1;}

/*virtual*/ void CCurve::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	GetClassType()->GetProperties()->SerializePropertyValues( this, Info, sContext);

	Info->AddValue( L"Knots", m_Knots );
}

/*virtual*/ void CCurve::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	if (pDevice == NULL)
	{
		throw std::invalid_argument( "NULL D3DDevice in serialization context" );
	}

	if (Info->GetObjectFormatVersion() < 1)
	{
		throw std::runtime_error( "unsupported CCurve data version" );
	}

	m_ArcLengthDirty = true;

	m_hMesh = NULL;
	m_Knots.clear();

	//Initialize property values
	CRenderableImpl::InitPropertyValues();
	SetTransform( CMatrix4f::gm_Identity );
	SetScaleComponent( Math::CVector3f::gm_UnitScale );
	SetRotationComponent( CQuaternion() );
	SetName( std::string() );
	SetClosed( false );
	SetDiffuse( 0xffffffffu );

	GetClassType()->GetProperties()->DeserializePropertyValues( this, Info, sContext);

	Info->GetValue( L"Knots", m_Knots );

	UpdateArcLengths();

	CreateMesh( pDevice );
}

/*virtual*/ void CCurve::FinalizeDeserialization(CSerializationInfo* Info, CSerializationContext* sContext)
{
	//_BaseClass::FinalizeDeserialization( Info, sContext );
}

/*virtual*/ void CCurve::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return;
	}

	if (Context.GetRenderCategory() != ERenderCategory::Opaque)
	{
		return;
	}

	if (m_hMesh == NULL)
	{
		CreateMesh( pDevice );
	}

	//Use curve's transform explicitly
	Context.SetWorldTransform( pDevice, GetTransform() );

	m_hMesh->Render( pDevice, Context, NULL );
}

//Add to the mesh a line segment approximating the cubic spline with
//the specified polynomial coefficients over the interval [L,R]. 
//The maximum tolerable error between the true spline and the proposed 
//line segment is Toler. If the error is intolerable, divide the proposed 
//segment at the parameter where error is greatest.
void AddSplineSegmentToMesh( CMesh* pMesh, CVector3f const (&PolyCoef)[4],
					 float32 L, float32 R, float32 Toler, bool bSkipLast )
{
	float32 const Interval = R - L;

	//Represent the polynomial in terms of s where t = (R - L)s + L.
	//This results in new vector coefficients A', B', C', and D':
	//f(t) = At^3 + Bt^2 + Ct + D = g(s) = A's^3 + B's^2 + C's + D'
	CVector3f NewCoef[4]; 
	{
		float32 const Interval2 = Interval * Interval;
		float32 const Interval3 = Interval2 * Interval;
		float32 const L2 = L * L;
		NewCoef[3] = PolyCoef[3] * Interval3;
		NewCoef[2] = PolyCoef[3] * Interval2 * 3.0f * L;
		NewCoef[1] = PolyCoef[3] * Interval * 3.0f * L2;
		NewCoef[0] = PolyCoef[3] * L2 * L;
		NewCoef[2] += PolyCoef[2] * Interval2;
		NewCoef[1] += PolyCoef[2] * Interval * 2.0f * L;
		NewCoef[0] += PolyCoef[2] * L2;
		NewCoef[1] += PolyCoef[1] * Interval;
		NewCoef[0] += PolyCoef[1] * L;
		NewCoef[0] += PolyCoef[0];
	}

	//Let E(s) be the vector difference between the spline g(s) and the 
	//interpolating line (A' + B' + C')s + D'.
	//
	//E(s) = g(s) - (A' + B' + C')s - D'.
	//
	//We are interested in knowing where E^2 (a positive scalar) is greatest.
	//We find the extrema of E^2 by finding the roots of dE^2/ds = 0.
	//The roots turn out to be 0, 1, and the three roots of
	//3A'A's^3 + (5A'B' + 3A'A')s^2 + (2B'B' - A'A' + A'B')s - (A' + B')^2.
	std::vector<float32> Roots;
	float32 AA = NewCoef[3].LengthSquared();
	float32 AB = NewCoef[3].DotProduct( NewCoef[2] );
	float32 BB = NewCoef[2].LengthSquared();

	//Get the real roots of the polynomial; there are up to three of them.
	{
		float32 AA3 = AA * 3.0f;
		ComputeRealPolynomialRoots( Roots, AA3, AA3 + 5.0f * AB, BB + BB - AA + AB, -2.0f * AB - AA - BB );
	}

	float32 MaxError2 = 0.0f;
	float32 WorstRoot = 0.0f;
	for (int32 i = 0; i < static_cast<int32>( Roots.size() ); ++i)
	{
		//Ignore the endpoints (minima where error is zero) and
		//values outside the interval of interest.
		if (Roots[i] <= 0.0f || Roots[i] >= 1.0f)
		{
			continue;
		}

		//Compute E^2
		CVector3f Error = (NewCoef[3] * (Roots[i] + 1.0f) + NewCoef[2]);
		float32 E2 = Error.LengthSquared() * Roots[i] * Roots[i] * 
			(Roots[i] - 1.0f) * (Roots[i] - 1.0f);

		if (MaxError2 < E2)
		{
			MaxError2 = E2;
			WorstRoot = Roots[i];
		}
	}

	//If the greatest error is unacceptable, then divide the segment at this parameter.
	if (MaxError2 > Toler * Toler)
	{
		float32 const Split = L + Interval * WorstRoot;
		AddSplineSegmentToMesh( pMesh, PolyCoef, L, Split, Toler, false );
		AddSplineSegmentToMesh( pMesh, PolyCoef, Split, R, Toler, bSkipLast );
		return;
	}

	for (int32 j = 0; j < 2; ++j)
	{
		//Ignore adding the last point if we are instructed to. Presumably
		//it will be added later as part of some other curve segment.
		if (j == 1 && bSkipLast)
		{
			break;
		}
	
		int32 const OutVertex = pMesh->GetVertexCount();
		pMesh->SetVertexCount( OutVertex + 1 );

		CVector3f Position = (NewCoef[3] + NewCoef[2] + NewCoef[1]) * 
			static_cast<float32>( j ) + NewCoef[0];

		pMesh->SetVertexPosition( OutVertex, Position );
		pMesh->GetParts()[0]->SetVertexCount( OutVertex + 1 );
		pMesh->GetParts()[0]->SetVertex( OutVertex, OutVertex );
	}
}

void CCurve::CreateMesh( LPDIRECT3DDEVICE9 pDevice )
{
	m_hMesh = new CMesh( pDevice );

	int32 const KnotCount = static_cast<int32>( m_Knots.size() );

	HMeshPart hPart = new CMeshPart( m_hMesh );
	m_hMesh->GetParts().push_back( hPart );
	hPart->SetPartType( CMeshPart::EType::LineStrip );

	int32 LastKnot = KnotCount - 1;
	if (m_Closed)
	{
		++LastKnot;
	}

	for (int32 i = 0; i < LastKnot; ++i)
	{
		//If this is not the last knot we are examining, the last sample
		//for this segment will be handled the next time we examine a knot.
		bool bSkipLast = (i != LastKnot - 1);

		Knot* pCur = &m_Knots[i % m_Knots.size()];
		Knot* pNext = &m_Knots[(i + 1) % m_Knots.size()];
		if (pCur->m_Type == Knot::KTypeBezier ||
			pNext->m_Type == Knot::KTypeBezier)
		{
			//Tolerate errors in distance of no more than a fraction of the arc length
			float32 const Toler = m_Knots[i].m_ArcLength * 0.005f;
			CVector3f PolyCoef[4];
			GetSplinePolyCoefficients( pCur, pNext, PolyCoef );
			AddSplineSegmentToMesh( m_hMesh, PolyCoef, 0.0f, 1.0f, Toler, bSkipLast );
		} else {
			for (int32 j = 0; j < 2; ++j)
			{
				if (j == 1 && bSkipLast)
				{
					break;
				}
			
				int32 const OutVertex = m_hMesh->GetVertexCount();
				m_hMesh->SetVertexCount( OutVertex + 1 );
				m_hMesh->SetVertexPosition( OutVertex, Evaluate( static_cast<float32>( i + j ) ) );
				hPart->SetVertexCount( OutVertex + 1 );
				hPart->SetVertex( OutVertex, OutVertex );
			}
		}
	}

	UpdateMeshDiffuse();
}

void CCurve::UpdateMeshDiffuse()
{
	assert( m_hMesh != NULL );

	//Set vertex color
	for (int32 i = m_hMesh->GetVertexCount() - 1; i >= 0; --i)
	{
		m_hMesh->SetVertexDiffuse( i, m_Diffuse );
	}
}

//If the arc-length estimated by linear interpolation over the specified
//parameter interval [L,R] has tolerable error (within specified Toler), 
//do nothing. Otherwise, add an arc length entry for the parameter value of 
//worst error and split the interval at this parameter value. PolyCoef contains 
//the coefficients of the quartic polynomial which is the square of the 
//arc-length integrand. LArcLength, RArcLength, and TotalArcLength are the arc-lengths 
//for the intervals [0.0,L], [0.0,R], and [0.0,1.0] respectively.
void FillArcLengthTable( std::vector<CCurve::Knot::ArcLengthEntry>& Table, 
	float32 const (&PolyCoef)[5], float32 L, float32 R, 
	float32 LArcLength, float32 RArcLength,
	float32 TotalArcLength,	float32 Toler )
{
	int32 iL = Float32ToInt32( L * 65535.0f );
	int32 iR = Float32ToInt32( R * 65535.0f );
	if (iL + 1 >= iR)
	{
		//Parameter values too close for the table to represent a division
		return;
	}

	float32 const Interval = R - L;
	float32 const IntervalArcLength = RArcLength - LArcLength;

	//Represent the polynomial in terms of s where t = (R - L)s + L.
	//This results in new polynomial coefficients. Furthermore,
	//there will be a (R-L) factor in the arc-length calculation: 
	//the sqrt(P(t)) dt integrand changes to sqrt(P'(s)) (R-L) ds.
	float32 NewCoef[5];
	{
		float32 Interval2 = Interval * Interval;
		float32 Interval3 = Interval2 * Interval;
		float32 Interval4 = Interval2 * Interval2;
		float32 const L2 = L * L;
		float32 const L3 = L2 * L;

		NewCoef[4] = PolyCoef[4] * Interval4;
		NewCoef[3] = PolyCoef[4] * Interval3 * 4.0f * L;
		NewCoef[2] = PolyCoef[4] * Interval2 * 6.0f * L2;
		NewCoef[1] = PolyCoef[4] * Interval * 4.0f * L3;
		NewCoef[0] = PolyCoef[4] * L2 * L2;
		NewCoef[3] += PolyCoef[3] * Interval3;
		NewCoef[2] += PolyCoef[3] * Interval2 * 3.0f * L;
		NewCoef[1] += PolyCoef[3] * Interval * 3.0f * L2;
		NewCoef[0] += PolyCoef[3] * L3;
		NewCoef[2] += PolyCoef[2] * Interval2;
		NewCoef[1] += PolyCoef[2] * Interval * 2.0f * L;
		NewCoef[0] += PolyCoef[2] * L2;
		NewCoef[1] += PolyCoef[1] * Interval;
		NewCoef[0] += PolyCoef[1] * L;
		NewCoef[0] += PolyCoef[0];
	}

	//Let E(s) be the vector difference between the true arc-length and the
	//interpolating line, ArcLength * s:
	//
	//E(s) = (R - L) Integral( sqrt( quartic polynomial ), 0, s ) - ArcLength * s
	//
	//The extrema (points of greatest error) are where dE/ds = 0:
	//
	//dE/ds = (R - L) sqrt( quartic polynomial ) - ArcLength
	//dE/ds = 0
	//(R - L) sqrt( quartic polynomial ) - ArcLength = 0
	//quartic polynomial - (ArcLength/(R-L))^2 = 0
	//
	//We can solve this directly.
	
	//Get the real roots of the polynomial; there are up to four of them.
	std::vector<float32> Roots;
	ComputeRealPolynomialRoots( Roots, NewCoef[4], NewCoef[3], NewCoef[2], NewCoef[1],
		NewCoef[0] - (IntervalArcLength * IntervalArcLength) / (Interval * Interval) );

	float32 MaxError = 0.0f;
	float32 WorstRoot = 0.0f;
	for (int32 i = 0; i < static_cast<int32>( Roots.size() ); ++i)
	{
		//Ignore the endpoints (minima where error is zero) and
		//values outside the interval of interest.
		if (Roots[i] <= 0.0f || Roots[i] >= 1.0f)
		{
			continue;
		}

		float32 const Error = fabsf( 
			Interval * ComputeSplineArcLength( 0.0f, Roots[i], NewCoef ) - Roots[i] * IntervalArcLength );

		if (MaxError < Error)
		{
			MaxError = Error;
			WorstRoot = Roots[i];
		}
	}

	//If the greatest error is unacceptable, then divide the segment at this parameter.
	if (MaxError > Toler)
	{
		float32 Split = L + Interval * WorstRoot;
		int32 iSplit = Float32ToInt32( Split * 65535.0f );

		if (iSplit <= iL) {iSplit = iL + 1;}
		if (iSplit >= iR) {iSplit = iR - 1;}
		assert( iSplit >= iL + 1 );
		assert( iSplit <= iR - 1 );
		assert( iSplit > 0 );
		assert( iSplit < 65535 );
		Split = iSplit * (1.0f / 65535.0f);

		float32 const SplitArcLength = ComputeSplineArcLength( 0.0f, Split, PolyCoef );

		FillArcLengthTable( Table, PolyCoef, L, Split, LArcLength, SplitArcLength, TotalArcLength, Toler );

		int32 iSplitArcLength = Float32ToInt32( SplitArcLength / TotalArcLength * 65535.0f );
		assert( iSplitArcLength >= 0 );
		assert( iSplitArcLength <= 65535 );
		
		CCurve::Knot::ArcLengthEntry E;
		E.m_Parameter = static_cast<uint16>( iSplit );
		E.m_ArcLength = static_cast<uint16>( iSplitArcLength );
		Table.push_back( E );

		FillArcLengthTable( Table, PolyCoef, Split, R, SplitArcLength, RArcLength, TotalArcLength, Toler );
	}
}

//Given a parameter on the curve, typically in the range [0, GetKnots.size() - 1] for an open
//curve, return the two knots that the referenced point is surrounded by (pCur and pNext) as well
//as an interpolation parameter in the range [0.0, 1.0] corresponding to the point. pPrev
//is used when the parameter references a knot; it is the knot preceding that knot.
void CCurve::GetInterpData( float32 CurveParam, Knot const*& pPrev, 
	Knot const*& pCur, Knot const*& pNext, float32& InterpParam ) const
{
	if (m_Closed)
	{
		if (m_Knots.empty())
		{
			throw std::domain_error( "empty curve" );
		}

		CurveParam = static_cast<float32>( 
			fmod( static_cast<double>( CurveParam ), static_cast<double>( m_Knots.size() ) ) );
		if (CurveParam < 0.0f)
		{
			CurveParam += m_Knots.size();
		}
		assert( CurveParam >= 0.0f );
		assert( CurveParam < m_Knots.size() );

	} else {
		if (CurveParam < 0.0f || CurveParam > m_Knots.size() - 1)
		{
			throw std::domain_error( "curve parameter" );
		}
		assert( !m_Knots.empty() );

		if (m_Knots.size() > 1 && CurveParam == m_Knots.size() - 1)
		{
			pPrev = NULL;
			pCur = &m_Knots[m_Knots.size() - 2];
			pNext = &m_Knots[m_Knots.size() - 1];
			InterpParam = 1.0f;
			return;
		}
	}

	int32 const Index = Float32ToInt32( floor( CurveParam ) );
	assert( Index >= 0 );
	assert( Index < static_cast<int32>( m_Knots.size() ) );

	pPrev = &m_Knots[(Index ? Index : m_Knots.size()) - 1];
	pCur  = &m_Knots[Index];
	pNext = &m_Knots[Index + 1 == static_cast<int32>( m_Knots.size() ) ? 0 : Index + 1 ];

	InterpParam = CurveParam - Index;
	assert( InterpParam >= 0.0f );
	assert( InterpParam <  1.0f );
}

CVector3f CCurve::EvaluateLocal( float32 CurveParam ) const
{
	Knot const *pPrev, *pCur, *pNext;
	float32 InterpParam;
	GetInterpData( CurveParam, pPrev, pCur, pNext, InterpParam );

	CVector3f Result( false );
	if (pCur->m_Type == Knot::KTypeBezier || pNext->m_Type == Knot::KTypeBezier)
	{
		BezierInterpolate( InterpParam, pCur->m_Position, pCur->m_Right, pNext->m_Left, pNext->m_Position, Result );
	} else {
		LinearInterpolate( InterpParam, pCur->m_Position, pNext->m_Position, Result );
	}
	return Result;
}

CVector3f CCurve::EvaluateDerivativeLocal( float32 CurveParam ) const
{
	if (!m_Closed && m_Knots.size() == 1)
	{
		throw std::runtime_error( "derivative does not exist" );
	}

	Knot const *pPrev, *pCur, *pNext;
	float32 InterpParam;
	GetInterpData( CurveParam, pPrev, pCur, pNext, InterpParam );

	CVector3f Result( false );
	if (pCur->m_Type == Knot::KTypeBezier || pNext->m_Type == Knot::KTypeBezier)
	{
		BezierInterpolateDerivative( InterpParam, pCur->m_Position, pCur->m_Right, pNext->m_Left, pNext->m_Position, Result );
	} else {
		Result = pNext->m_Position - pCur->m_Position;
	}

	//If exactly at a knot surrounded by two segments, then the derivative is discontinuous
	bool const bDiscontinuousDerivative = !InterpParam && (m_Closed || pCur != &m_Knots[0]);
	if (bDiscontinuousDerivative)
	{
		//Compute derivative on negative side and return the average
		if (pPrev->m_Type == Knot::KTypeBezier || pCur->m_Type == Knot::KTypeBezier)
		{
			CVector3f LeftResult( false );
			BezierInterpolateDerivative( 1.0f, pPrev->m_Position, pPrev->m_Right, pCur->m_Left, pCur->m_Position, LeftResult );
			Result += LeftResult;
		} else {
			Result = pNext->m_Position - pPrev->m_Position;
		}

		Result *= 0.5f;
	}

	return Result;
}

CVector3f CCurve::EvaluateCurvatureLocal( float32 CurveParam ) const
{
	if (!m_Closed && m_Knots.size() == 1)
	{
		throw std::runtime_error( "curvature does not exist" );
	}

	Knot const *pPrev, *pCur, *pNext;
	float32 InterpParam;
	GetInterpData( CurveParam, pPrev, pCur, pNext, InterpParam );

	CVector3f Curvature( false );

	//If exactly at a knot surrounded by two segments, then the derivative is discontinuous
	bool const bDiscontinuousDerivative = !InterpParam && (m_Closed || pCur != &m_Knots[0]);
	if (bDiscontinuousDerivative)
	{
		//Compute derivative on left and right sides
		CVector3f RightDerivative( false );
		if (pCur->m_Type == Knot::KTypeBezier || pNext->m_Type == Knot::KTypeBezier)
		{
			BezierInterpolateDerivative( 0.0f, pCur->m_Position, pCur->m_Right, pNext->m_Right, pNext->m_Position, RightDerivative );
		} else {
			RightDerivative = pNext->m_Position - pCur->m_Position;
		}

		CVector3f LeftDerivative( false );
		if (pPrev->m_Type == Knot::KTypeBezier || pCur->m_Type == Knot::KTypeBezier)
		{
			BezierInterpolateDerivative( 1.0f, pPrev->m_Position, pPrev->m_Right, pCur->m_Left, pCur->m_Position, LeftDerivative );
		} else {
			LeftDerivative = pCur->m_Position - pPrev->m_Position;
		}

		Curvature = RightDerivative - LeftDerivative;

		if (Curvature.x < 0.0f)
		{
			Curvature.x = -std::numeric_limits<float32>::infinity();
		} else if (Curvature.x > 0.0f) {
			Curvature.x = +std::numeric_limits<float32>::infinity();
		}

		if (Curvature.y < 0.0f)
		{
			Curvature.y = -std::numeric_limits<float32>::infinity();
		} else if (Curvature.y > 0.0f) {
			Curvature.y = +std::numeric_limits<float32>::infinity();
		}

		if (Curvature.z < 0.0f)
		{
			Curvature.z = -std::numeric_limits<float32>::infinity();
		} else if (Curvature.z > 0.0f) {
			Curvature.z = +std::numeric_limits<float32>::infinity();
		}
	} else {
		if (pCur->m_Type == Knot::KTypeBezier || pNext->m_Type == Knot::KTypeBezier)
		{
			BezierInterpolateCurvature( InterpParam, pCur->m_Position, pCur->m_Right, pNext->m_Left, pNext->m_Position, Curvature );
		} else {
			Curvature = CVector3f();
		}
	}

	return Curvature;
}

CVector3f CCurve::Evaluate( float32 CurveParam ) const
{
	CVector3f V = EvaluateLocal( CurveParam );
	CVector3f Result( false );
	GetTransform().TransformNoOverlap( Result, V );
	return Result;
}

CVector3f CCurve::EvaluateDerivative( float32 CurveParam ) const
{
	CVector3f V = EvaluateDerivativeLocal( CurveParam );
	CVector3f Result( false );
	GetTransform().RotateNoOverlap( Result, V );
	return Result;
}

CVector3f CCurve::EvaluateCurvature( float32 CurveParam ) const
{
	CVector3f V = EvaluateCurvatureLocal( CurveParam );
	CVector3f Result( false );
	GetTransform().RotateNoOverlap( Result, V );
	return Result;
}

CVector3f CCurve::EvaluateDerivativeWrtArcLengthLocal( float32 CurveParam ) const
{
	// df/dL = f' / L' = f' / |f'|
	CVector3f Result = EvaluateDerivativeLocal( CurveParam );
	Result.Normalize();
	return Result;
}

CVector3f CCurve::Evaluate2ndDerivativeWrtArcLengthLocal( float32 CurveParam ) const
{
	/*	d/dL (df/dL) = 1/|f'| d/dt [f' (f'.f')^(-0.5)]
		= 1/|f'| [f'' / |f'| - 0.5 f' (2f'.f'') (f'.f')^(-1.5)]
		= 1/(f'.f') [f'' - f' (f'.f'') / (f'.f')]	*/
	CVector3f Derivative = EvaluateDerivativeLocal( CurveParam );
	CVector3f Curvature = EvaluateCurvatureLocal( CurveParam );
	float32 InvDerivLenSq = 1.0f / Derivative.LengthSquared();
	CVector3f Result = (Curvature - Derivative * (Derivative.DotProduct( Curvature ) * InvDerivLenSq))
		* InvDerivLenSq;
	return Result;
}

CVector3f CCurve::EvaluateDerivativeWrtArcLength( float32 CurveParam ) const
{
	CVector3f V = EvaluateDerivativeWrtArcLengthLocal( CurveParam );
	CVector3f Result( false );
	GetTransform().RotateNoOverlap( Result, V );
	return Result;
}

CVector3f CCurve::Evaluate2ndDerivativeWrtArcLength( float32 CurveParam ) const
{
	CVector3f V = Evaluate2ndDerivativeWrtArcLengthLocal( CurveParam );
	CVector3f Result( false );
	GetTransform().RotateNoOverlap( Result, V );
	return Result;
}

//Compute the arc-length of the segment between knots Index and (Index + 1) % size().
//The portion on the segment from parameter 0.0 through SegmentParam
//(in the range [0.0,1.0]) is considered.
void CCurve::UpdateArcLength( int32 Index )
{
	if (Index < 0 || Index >= static_cast<int32>( m_Knots.size() ))
	{
		throw std::invalid_argument( "knot index" );
	}

	Knot* pCur  = &m_Knots[Index];
	pCur->m_ArcLengthTable.clear();

	if (!m_Closed && Index == static_cast<int32>( m_Knots.size() ) - 1)
	{
		pCur->m_ArcLength = 0.0f; //No segment
		return;
	}
	
	Knot const* pNext = &m_Knots[
		Index + 1 == static_cast<int32>( m_Knots.size() ) ? 0 : Index + 1 ];

	if (pCur->m_Type == Knot::KTypeCorner && pNext->m_Type == Knot::KTypeCorner)
	{
		pCur->m_ArcLength = (pNext->m_Position - pCur->m_Position).Length();
		return;
	}

	float32 Coef[5];
	GetSplineArcLengthCoefficients( pCur, pNext, Coef );

	pCur->m_ArcLength = ComputeSplineArcLength( 0.0f, 1.0f, Coef );

	float32 const Toler = pCur->m_ArcLength * 1.0e-3f;
	FillArcLengthTable( pCur->m_ArcLengthTable, Coef, 0.0f, 1.0f, 
		0.0f, pCur->m_ArcLength, pCur->m_ArcLength, Toler );

#ifndef NDEBUG
	for (int32 i = 1; i < static_cast<int32>( pCur->m_ArcLengthTable.size() ); ++i)
	{
		assert( pCur->m_ArcLengthTable[i].m_Parameter >  pCur->m_ArcLengthTable[i - 1].m_Parameter );
		assert( pCur->m_ArcLengthTable[i].m_ArcLength >= pCur->m_ArcLengthTable[i - 1].m_ArcLength );
	}

#if 0
	//Slow, thorough test for GetArcLength() accuracy.
	//This should only fail for extreme curves for which the 16-bit quantization of parameter
	//space is inadequate for piecewise linear approximation of a sharply curved arc length 
	//function.
	m_ArcLengthDirty = false;
	pCur->m_CumArcLength = pCur->m_ArcLength;
	for (int32 x = 0; x < 100000; ++x)
	{
		float32 InterpolatedResult = GetArcLength( Index + x * 1.0f / 100000.0f );
		float32 ComputedResult = ComputeSplineArcLength( 0.0f, x * 1.0f / 100000.0f, Coef );
		assert( fabsf( InterpolatedResult - ComputedResult ) <= Toler );
	}
#endif

#endif
}

void CCurve::UpdateArcLengths()
{
	if (!m_ArcLengthDirty)
	{
		return;
	}

	float32 CumArcLength = 0.0f;
	for (int32 i = 0; i < static_cast<int32>( m_Knots.size() ); ++i)
	{
		UpdateArcLength( i );
		CumArcLength += m_Knots[i].m_ArcLength;
		m_Knots[i].m_CumArcLength = CumArcLength;
	}

	m_ArcLengthDirty = false;

#ifndef DEBUG
#if 0 //Slow test for GetCurveParameter()
	for (int32 x = 0; x < 10000 * (m_Knots.size() - 1); ++x)
	{
		float32 CurveParam1 = x * 1.0f / 10000.0f;
		float32 ArcLength = GetArcLength( CurveParam1 );
		float32 CurveParam2 = GetCurveParameter( ArcLength );
		assert( fabsf( CurveParam2 - CurveParam1 ) < 0.01f );
	}
#endif
#endif
}

float32 CCurve::GetArcLength( float32 CurveParam ) const
{
	const_cast<CCurve*>( this )->UpdateArcLengths();

	float32 Cycles = 0.0f; //Number of full cycles around the curve

	if (m_Closed)
	{
		if (m_Knots.empty())
		{
			throw std::domain_error( "empty curve" );
		}

		Cycles = floorf( CurveParam / m_Knots.size() );

		CurveParam = static_cast<float32>( 
			fmod( static_cast<double>( CurveParam ), static_cast<double>( m_Knots.size() ) ) );
		if (CurveParam < 0.0f)
		{
			CurveParam += m_Knots.size();
		}
		assert( CurveParam >= 0.0f );
		assert( CurveParam < m_Knots.size() );

	} else {
		if (CurveParam < 0.0f || CurveParam > m_Knots.size() - 1)
		{
			throw std::domain_error( "curve parameter" );
		}
		assert( !m_Knots.empty() );
	}
	
	Knot const* pCur;
	float32 SegmentParam;
	if (!m_Closed && m_Knots.size() > 1 && CurveParam == m_Knots.size() - 1)
	{
		pCur = &m_Knots[m_Knots.size() - 2];
		SegmentParam = 1.0f;
	} else {
		int32 const Index = Float32ToInt32( floor( CurveParam ) );

		assert( Index >= 0 );
		assert( Index < static_cast<int32>( m_Knots.size() ) );
		pCur = &m_Knots[Index];

		SegmentParam = CurveParam - Index;
		assert( SegmentParam >= 0.0f );
		assert( SegmentParam <  1.0f );
	}
    
	//Find the segment parameter in the arc-length table
	int32 const TableSize = static_cast<int32>( pCur->m_ArcLengthTable.size() );
	int32 L = -1, R = TableSize;
	while (L + 1 < R)
	{
		int32 M = (L + R) >> 1;	
		
		float32 Param = 0.0f;
		
		if (M >= 0)
		{
			Param = (1.0f / 65535.0f) * pCur->m_ArcLengthTable[M].m_Parameter;
		}

		if (Param <= SegmentParam)
		{
			L = M;
		} else {
			R = M;
		}
	}

	//Get the two surrounding table entries. The are implicit entries with
	//arc length 0.0 at segment parameter 0.0, as well as full arc length at 
	//segment parameter 1.0.
	float32 const ParamL     = (L >= 0 && L < TableSize) ? 
		pCur->m_ArcLengthTable[L].m_Parameter * (1.0f / 65535.0f) : 0.0f;
	float32 const ArcLengthL = (L >= 0 && L < TableSize) ? 
		pCur->m_ArcLengthTable[L].m_ArcLength * (1.0f / 65535.0f) : 0.0f;
	float32 const ParamR	 = (R < TableSize) ? 
		pCur->m_ArcLengthTable[R].m_Parameter * (1.0f / 65535.0f) : 1.0f;
	float32 const ArcLengthR = (R < TableSize) ? 
		pCur->m_ArcLengthTable[R].m_ArcLength * (1.0f / 65535.0f) : 1.0f;
	assert( SegmentParam >= ParamL );
	assert( SegmentParam <= ParamR );

	//Linearly interpolate the result based on the two surrounding table
	//entries.
	return 
		Cycles * m_Knots[m_Knots.size() - 1].m_CumArcLength +
		pCur->m_CumArcLength - pCur->m_ArcLength + pCur->m_ArcLength * (
		ArcLengthL + (ArcLengthR - ArcLengthL) * (SegmentParam - ParamL) / (ParamR - ParamL) );
}

float32 CCurve::GetCurveParameter( float32 ArcLength ) const
{
	const_cast<CCurve*>( this )->UpdateArcLengths();

	float32 const CurveLength = m_Knots[m_Knots.size() - 1].m_CumArcLength;

	float32 Cycles = 0.0f; //Number of full cycles around the curve
	if (m_Closed)
	{
		if (m_Knots.empty())
		{
			throw std::domain_error( "empty curve" );
		}

		Cycles = floorf( ArcLength / CurveLength );
		ArcLength = fmodf( ArcLength, CurveLength );
		if (ArcLength < 0.0f)
		{
			ArcLength += m_Knots[m_Knots.size() - 1].m_CumArcLength;
		}

		assert( ArcLength >= 0.0f );
		assert( ArcLength < m_Knots[m_Knots.size() - 1].m_CumArcLength );

	} else {
		if (ArcLength < 0.0f || ArcLength > CurveLength)
		{
			throw std::domain_error( "curve parameter" );
		}
		assert( !m_Knots.empty() );
	}

	int32 Index;
	Knot const* pCur;
	float32 SegmentArcLengthParam;
	if (!m_Closed && m_Knots.size() > 1 && ArcLength == CurveLength)
	{
		Index = static_cast<int32>( m_Knots.size() ) - 2;
		SegmentArcLengthParam = 1.0f;
	} else {
		int32 L = 0;
		int32 R = static_cast<int32>( m_Knots.size() - 1 );
		int32 M = 0;
		for (;;)
		{
			assert( L <= R );
			M = (L + R) >> 1;
			if (ArcLength > m_Knots[M].m_CumArcLength)
			{
				L = M + 1;
			} else if (ArcLength < m_Knots[M].m_CumArcLength - m_Knots[M].m_ArcLength) {
				R = M - 1;
			} else {
				break;	
			}
		}
		
		Index = M;
		SegmentArcLengthParam = (ArcLength - (m_Knots[Index].m_CumArcLength - m_Knots[Index].m_ArcLength)) / 
			m_Knots[Index].m_ArcLength;
	}

	assert( Index >= 0 );
	assert( Index < static_cast<int32>( m_Knots.size() ) );
	pCur = &m_Knots[Index];
	assert( SegmentArcLengthParam >= 0.0f );
	assert( SegmentArcLengthParam <= 1.0f );
    
	//Find the arc length in the arc-length table
	int32 const TableSize = static_cast<int32>( pCur->m_ArcLengthTable.size() );
	int32 L = -1, R = TableSize;
	while (L + 1 < R)
	{
		int32 M = (L + R) >> 1;	
		
		float32 ArcLength = 0.0f;
		
		if (M >= 0)
		{
			ArcLength = (1.0f / 65535.0f) * pCur->m_ArcLengthTable[M].m_ArcLength;
		}

		if (ArcLength <= SegmentArcLengthParam)
		{
			L = M;
		} else {
			R = M;
		}
	}

	//Get the two surrounding table entries. The are implicit entries with
	//arc length 0.0 at segment parameter 0.0, as well as full arc length at 
	//segment parameter 1.0.
	float32 const ParamL     = (L >= 0 && L < TableSize) ? 
		pCur->m_ArcLengthTable[L].m_Parameter * (1.0f / 65535.0f) : 0.0f;
	float32 const ArcLengthL = (L >= 0 && L < TableSize) ? 
		pCur->m_ArcLengthTable[L].m_ArcLength * (1.0f / 65535.0f) : 0.0f;
	float32 const ParamR	 = (R < TableSize) ? 
		pCur->m_ArcLengthTable[R].m_Parameter * (1.0f / 65535.0f) : 1.0f;
	float32 const ArcLengthR = (R < TableSize) ? 
		pCur->m_ArcLengthTable[R].m_ArcLength * (1.0f / 65535.0f) : 1.0f;
	assert( SegmentArcLengthParam >= ArcLengthL );
	assert( SegmentArcLengthParam <= ArcLengthR );

	//Linearly interpolate the result based on the two surrounding table
	//entries.
	float32 InterpFactor = 0.5f;
	if (ArcLengthL != ArcLengthR)
	{
		InterpFactor = (SegmentArcLengthParam - ArcLengthL) / (ArcLengthR - ArcLengthL);
	}
	return Cycles * m_Knots.size() + Index + ParamL + (ParamR - ParamL) * InterpFactor;
}

} //namespace Graphics

} //namespace WT

template <> void WT::System::Serialization::CSerializationInfo::AddValue(
	wstring const& name, WT::Graphics::CCurve::Knot const& value )
{
	AddContainerBegin(name);
	AddValue( L"Type", static_cast<int32>( value.m_Type ) );
	AddValue( L"Position", value.m_Position );
	AddValue( L"Left", value.m_Left );
	AddValue( L"Right", value.m_Right );
	AddContainerEnd();
}

template <> void WT::System::Serialization::CSerializationInfo::GetValue(
	wstring const& name, WT::Graphics::CCurve::Knot& value )
{
	GetContainerBegin(name);

	int32 Type;
	GetValue( L"Type", Type );
	if (Type < 0 || Type >= WT::Graphics::CCurve::Knot::KTypeCount)
	{
		throw WT::System::Serialization::CSerializationException( "invalid knot type" );
	}
	value.m_Type = static_cast<WT::Graphics::CCurve::Knot::EType>( Type );

	GetValue( L"Position", value.m_Position );
	GetValue( L"Left", value.m_Left );
	GetValue( L"Right", value.m_Right );
	GetContainerEnd();
}
