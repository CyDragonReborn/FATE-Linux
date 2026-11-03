/***************************************************************************************
 *
 * Curve.h
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          1/5/2006
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Object.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Graphics/IMovable.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Mesh/Mesh.h"

namespace WT
{
namespace Graphics
{

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/*
A CCurve is curve defined by a sequence of knots and segments connecting them.
Segments may be linear or cubic splines. A CCurve may be open or closed (looping).
Note that zero-length segments are currently not fully supported.

CCurve's are parameterized in a way such that knots correspond to integral values
of the parameter, with adjacent knots having a difference in parameter of 1. When
dealing with open curves, the parameter must be in the range [0, KnotCount - 1].
Closed curves permit parameters of any real value.

CCurve supports conversions to and from an alternative parameterization: the arc-length 
of the portion of the curve measured from the origin of the curve through the current 
point. For open curves, this arc length parameter is restricted to the range [0, GetArcLength()]. 
The arc length parameter is measured in the curve's coordinate system; a scaled
transform matrix is not taken into account.

A CCurve results from exporting a 3ds max spline.
*/
typedef System::SmartPointer<class CCurve> HCurve;
class CCurve : public System::CObject, public CMovableImpl, public System::Serialization::ISerializable,
	public CRenderableImpl
{
public:
	WT_BEGIN_CLASS( CCurve, System::CObject );
		WT_INHERITS_INTERFACE( IMovable );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( IRenderable );

		WT_BEGIN_PROPERTIES();
			WT_PROP_RW_BYREF( Name, string );
			WT_PROP_RW_BYVAL( Closed, bool );
			WT_PROP_RW_BYVAL( Diffuse, D3DCOLOR );
		WT_END_PROPERTIES();

	WT_END_CLASS();

	struct Knot
	{
		//Type of knot - if at least one of two adjacent knots is of type KTypeBezier,
		//then the joining segment will be a cubic Bezier spline.
		enum EType
		{
			KTypeCorner, //For piecewise linear curves
			KTypeBezier, //For cubic splines
			KTypeCount
		};

		CVector3f	m_Position;
		CVector3f   m_Left; //Preceding control point
		CVector3f   m_Right; //Subsequent control point
		EType		m_Type;

		//
		//Computed quantities - these should not be set manually; call CCurve::UpdateArcLengths() instead.
		//
		float32		m_ArcLength;
		float32		m_CumArcLength;

		struct ArcLengthEntry
		{
			uint16 m_Parameter; //Segment parameter * 65535
			uint16 m_ArcLength; //Partial segment arc length / total segment arc length * 65535
		};
		std::vector<ArcLengthEntry> m_ArcLengthTable;

		//Construct uninitialized
		Knot( bool )
			: m_Position( false )
			, m_Left( false )
			, m_Right( false )
		{
		}

		Knot()
			: m_Type( EType::KTypeCorner )
			, m_ArcLength( 0.0f )
			, m_CumArcLength( 0.0f )
		{
		}

		Knot( EType Type, CVector3f const& Position, CVector3f const& Left, CVector3f const& Right )
			: m_Type( Type )
			, m_Position( Position )
			, m_Left( Left )
			, m_Right( Right )
			, m_ArcLength( 0.0f )
			, m_CumArcLength( 0.0f )
		{
		}
	};

	CCurve()
		: m_Closed( false )
		, m_Diffuse( 0xffffffffu )
		, m_ArcLengthDirty( false )
	{
	}

	std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	bool GetClosed() const {return m_Closed;}
	void SetClosed( bool Closed ) {m_Closed = Closed;}

	D3DCOLOR GetDiffuse() const {return m_Diffuse;}
	void SetDiffuse( D3DCOLOR Diffuse )
	{
		m_Diffuse = Diffuse;
		if (m_hMesh != NULL)
		{
			UpdateMeshDiffuse();
		}
	}

	vector<Knot>& GetKnots() {return m_Knots;}
	vector<Knot> const& GetKnots() const {return m_Knots;}

	//Evaluate P(t) - a position in curve's coordinate system based on knots 
	//floor( CurveParam ) and floor( CurveParam ) + 1. Open curves allow evaluation 
	//at values in the range [0, KnotCount - 1]. Closed non-empty curves allow 
	//evaluation at any real value.
	CVector3f EvaluateLocal( float32 CurveParam ) const;

	//Evaluate dP/dt - the derivative of position in the curve's coordinate system at the 
	//specified parameter. Note that the derivative may be discontinous at knots.
	CVector3f EvaluateDerivativeLocal( float32 CurveParam ) const;

	//Evaluate d^2P/dt^2 - the second derivative of position in the curve's coordinate system 
	//at the specified parameter. At knots, where the derivative may be discontinuous, each 
	//component of the curvature vector will be set to 0.0f, 
	//-std::numeric_limits<float32>::infinity(), or +std::numeric_limits<float32>::infinity().
	CVector3f EvaluateCurvatureLocal( float32 CurveParam ) const;

	//Evaluate dP/dL - the derivative of position in the curve's coordinate system, with respect to 
	//arc length, at the specified parameter. This is the direction of the tangent of the curve.
	CVector3f EvaluateDerivativeWrtArcLengthLocal( float32 CurveParam ) const;

	//Evaluate d^2P/dL^2 - the second derivative in the curve's coordinate system, with respect to 
	//arc length, at the specified parameter.
	CVector3f Evaluate2ndDerivativeWrtArcLengthLocal( float32 CurveParam ) const;

	//Evaluate the position or various derivatives in the world coordinate system.
	CVector3f Evaluate( float32 CurveParam ) const;
	CVector3f EvaluateDerivative( float32 CurveParam ) const;
	CVector3f EvaluateCurvature( float32 CurveParam ) const;
	CVector3f EvaluateDerivativeWrtArcLength( float32 CurveParam ) const;
	CVector3f Evaluate2ndDerivativeWrtArcLength( float32 CurveParam ) const;

	//Get the arc length of the entire curve.
	float32 GetArcLength() const
	{
		const_cast<CCurve*>( this )->UpdateArcLengths();
		return m_Knots.empty() ? 0.0f : m_Knots[m_Knots.size() - 1].m_CumArcLength;
	}

	//Get the arc-length parameter L corresponding to the specified curve parameter.
	//For open curves, the curve parameter must be in the range [0, KnotCount - 1].
	float32 GetArcLength( float32 CurveParam ) const;

	//Get the curve parameter t corresponding to the specified arc-length parameter.
	//For open curves, the arc length parameter must be in the range [0, total arc-length].
	float32 GetCurveParameter( float32 ArcLength ) const;

	//Mark the curve as dirty. This invalidates the arc length data and
	//resets the internal mesh used for rendering.
	void SetDirty()
	{
		m_hMesh = NULL;

		for (int32 i = 0; i < static_cast<int32>( m_Knots.size() ); ++i)
		{
			m_Knots[i].m_ArcLength = 0.0f;
			m_Knots[i].m_CumArcLength = 0.0f;
			m_Knots[i].m_ArcLengthTable.clear();
		}

		m_ArcLengthDirty = true;
	}

	//Update internal arc length information for the curve. Repeated calls have
	//no effect unless SetDirty() is called.
	void UpdateArcLengths();

	//
	// ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private:
	std::string				m_Name;
	bool					m_Closed  : 1;
	bool					m_ArcLengthDirty : 1;
	D3DCOLOR				m_Diffuse;
	vector<Knot>			m_Knots;
	HMesh					m_hMesh; //Mesh used for rendering

	void CreateMesh( LPDIRECT3DDEVICE9 pDevice );
	void UpdateMeshDiffuse();
	void GetInterpData( float32 CurveParam, Knot const*& pPrev,
		Knot const*& pCur, Knot const*& pNext, float32& InterpParam ) const;

	//Update arc length information for the specified knot
	void UpdateArcLength( int32 Index );
};

}
}

DECLARE_VALUETYPE_TYPETRAIT(WT::Graphics::CCurve::Knot);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Graphics::CCurve::Knot>);

template <> void WT::System::Serialization::CSerializationInfo::AddValue(
	wstring const& name, WT::Graphics::CCurve::Knot const& value);

template <> void WT::System::Serialization::CSerializationInfo::GetValue(
	wstring const& name, WT::Graphics::CCurve::Knot& value );
