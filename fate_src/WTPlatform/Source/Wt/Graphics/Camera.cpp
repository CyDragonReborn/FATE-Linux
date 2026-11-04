//
// Camera.cpp
//
// Copyright (C) 2005 WildTangent, Inc.
// All Rights Reserved
//

#include "WT/stdafx.h"

#include "WT/System/Platform.h"
#include "WT/System/Logging/Log.h"
#include "WT/Graphics/Camera.h"
#include "WT/Math/Utils/MathUtil.h"
#include "WT/Math/Collision/Intersections.h"

using namespace WT::System;
using namespace WT::Math;


namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CCamera )

namespace
{
	float32 const KCameraDefaultFOV( KPi * 0.25f );
	float32 const KCameraDefaultNearClip( 1.0f );
	float32 const KCameraDefaultFarClip( 100.0f );
} //anonymous namespace

CCamera::CCamera() 
	: m_Rect( 0, 0, 640, 480 )
	, m_FOV( KCameraDefaultFOV )
	, m_VerticalFOV( -1.0f )
	, m_NearClip( KCameraDefaultNearClip )
	, m_FarClip( KCameraDefaultFarClip )
	, m_bOrthographic( false )
	, m_bDefaultRenderTarget( true )
	, m_ResourcesValid( false )
	, m_pRenderTarget( NULL )
{
	UpdateProjection();
}

CCamera::CCamera( const CCamera& Other ) 
	: CMovableImpl( Other )
	, m_bDefaultRenderTarget( Other.m_bDefaultRenderTarget )
	, m_ResourcesValid( Other.m_ResourcesValid )
	, m_Rect( Other.m_Rect )
	, m_FOV( Other.m_FOV )
	, m_VerticalFOV( Other.m_VerticalFOV )
	, m_NearClip( Other.m_NearClip )
	, m_FarClip( Other.m_FarClip )
	, m_bOrthographic( Other.m_bOrthographic )
	, m_ViewToWorld( Other.m_ViewToWorld )
	, m_WorldToView( Other.m_WorldToView )
	, m_Projection( Other.m_Projection )
	, m_ViewToScreen( Other.m_ViewToScreen )
	, m_ScreenToView( Other.m_ScreenToView )
	, m_WorldToScreen( Other.m_WorldToScreen )
	, m_ScreenToWorld( Other.m_ScreenToWorld )
	, m_Frustum( Other.m_Frustum )
{
	UpdateProjection( );

	m_pRenderTarget = Other.m_pRenderTarget;
	if (m_pRenderTarget != NULL)
	{
		m_pRenderTarget->AddRef();
	}
}

CCamera::~CCamera()	
{
	InvalidateDeviceObjects();
}

/// Set the transform for the movable object
void CCamera::SetTransform( Math::CMatrix4f const& Transform )
{
	CMovableImpl::SetTransform( Transform );
	
	m_ViewToWorld = GetTransform();

    m_ViewToWorld.GetInverse4x4( &m_WorldToView );

	m_WorldToScreen.Multiply4x4( m_WorldToView, m_ViewToScreen );
	m_ScreenToWorld.Multiply4x4( m_ScreenToView, m_ViewToWorld );

	m_Frustum.Update( this );
}

void CCamera::UpdateProjection()
{
	float32 NearClip = m_NearClip, FarClip = m_FarClip;

	if (NearClip >= FarClip || (!m_bOrthographic && NearClip <= 0.0f))
	{
		LOG_PURPLE( "Warning: ignoring bad clip range [%f,%f] for camera '%s'.", NearClip, FarClip, m_Name.c_str() );

		//Default to safe values for our calculations
		NearClip = KCameraDefaultNearClip;
		FarClip = KCameraDefaultFarClip;
	}
	
	float32 Aspect;
	if (m_VerticalFOV == -1.0f)
	{
		Aspect = static_cast<float32>( m_Rect.Width() ) / m_Rect.Height();
	} else {
		Aspect = tanf( 0.5f * m_FOV ) * tanf( 0.5f * (KPi - m_VerticalFOV) );
	}

	if (m_bOrthographic)
	{
		float32 X = FarClip * tanf( 0.5f * m_FOV );
		m_Projection.MakeOrthoProjectionXY( X, X * Aspect, NearClip, FarClip );
	} else {
		m_Projection.MakePerspProjectionFOVAspect( m_FOV, Aspect, NearClip, FarClip );
	}

	//Our visible screen coordinates range from (-1,-1,0) to (1,1,1) as in DirectX clip
	//coordinates:
	m_ViewToScreen = m_Projection;

	m_ViewToScreen.GetInverse4x4( &m_ScreenToView );

	m_WorldToScreen.Multiply4x4( m_WorldToView, m_ViewToScreen );
	m_ScreenToWorld.Multiply4x4( m_ScreenToView, m_ViewToWorld );

	m_Frustum.Update( this );
}

float32 CCamera::GetVerticalFOV() const
{
	if (m_VerticalFOV != -1.0f)
	{
		return m_VerticalFOV;
	}

	//Compute the FOV based off of the horizontal FOV and rectangle dimensions
	float32 InvAspect = static_cast<float32>( m_Rect.bottom - m_Rect.top ) / (m_Rect.right - m_Rect.left);
	return 2.0f * atanf( tanf( 0.5f * m_FOV ) * InvAspect );
}

/*static*/ CVector3f CCamera::Transform( CMatrix4f const& Matrix, CVector3f const& Input3D )
{
	CVector4f const Input( Input3D.x, Input3D.y, Input3D.z, 1.0f );
	CVector4f Result( false );
	Matrix.Transform4dNoOverlap( Result, Input );

	assert( Result.w != NULL );

	//Convert to non-homogeneous coordinate
	return Result.CVector3f::operator*( 1.0f / Result.w );
}

CMatrix4f CCamera::GetPixelToScreen() const
{
	CMatrix4f Result;
	Result.RowX.x = 2.0f / (m_Rect.right - m_Rect.left);
	Result.RowY.y = 2.0f / (m_Rect.top - m_Rect.bottom);
	Result.RowT.x = -m_Rect.left * Result.RowX.x - 1.0f;
	Result.RowT.y = -m_Rect.top * Result.RowY.y + 1.0f;
	return Result;
}

CMatrix4f CCamera::GetPixelToView() const
{
	return GetPixelToScreen() * GetScreenToView();
}

CMatrix4f CCamera::GetPixelToWorld() const
{
	return GetPixelToScreen() * GetScreenToWorld();
}

CMatrix4f CCamera::GetScreenToPixel() const
{
	CMatrix4f Result;
	Result.RowX.x = 0.5f * (m_Rect.right - m_Rect.left);
	Result.RowY.y = 0.5f * (m_Rect.top - m_Rect.bottom);
	Result.RowT.x = m_Rect.left + Result.RowX.x;
	Result.RowT.y = m_Rect.top - Result.RowY.y;
	return Result;
}

CMatrix4f CCamera::GetViewToPixel() const
{
	return GetViewToScreen() * GetScreenToPixel();
}

CMatrix4f CCamera::GetWorldToPixel() const
{
	return GetWorldToScreen() * GetScreenToPixel();
}

CVector3f CCamera::TransformPixelToScreen( CVector3f const& PixelPoint ) const
{
	return CVector3f(
		2.0f * (PixelPoint.x - m_Rect.left) / (m_Rect.right - m_Rect.left) - 1.0f,
		2.0f * (PixelPoint.y - m_Rect.top) / (m_Rect.top - m_Rect.bottom) + 1.0f,
		PixelPoint.z );
}

CVector3f CCamera::TransformPixelToView( CVector3f const& PixelPoint ) const
{
	return TransformScreenToView( TransformPixelToScreen( PixelPoint ) );
}

CVector3f CCamera::TransformPixelToWorld( CVector3f const& PixelPoint ) const
{
	return TransformScreenToWorld( TransformPixelToScreen( PixelPoint ) );
}

CVector3f CCamera::TransformPixelToScreen( float32 X, float32 Y, float32 Z ) const
{
	return TransformPixelToScreen( CVector3f( X, Y, Z ) );
}

CVector3f CCamera::TransformPixelToView( float32 X, float32 Y, float32 Z ) const
{
	return TransformPixelToView( CVector3f( X, Y, Z ) );
}

CVector3f CCamera::TransformPixelToWorld( float32 X, float32 Y, float32 Z ) const
{
	return TransformPixelToWorld( CVector3f( X, Y, Z ) );
}

CVector3f CCamera::TransformScreenToPixel( Math::CVector3f const& ScreenPoint ) const
{
	return CVector3f(
		m_Rect.left + (ScreenPoint.x + 1.0f) * 0.5f * (m_Rect.right - m_Rect.left),
		m_Rect.top + (ScreenPoint.y - 1.0f) * 0.5f * (m_Rect.top - m_Rect.bottom),
		ScreenPoint.z );
}

CVector3f CCamera::TransformScreenToView( CVector3f const& ScreenPoint ) const
{
	return Transform( m_ScreenToView, ScreenPoint );
}

CVector3f CCamera::TransformScreenToWorld( CVector3f const& ScreenPoint ) const
{
	return Transform( m_ScreenToWorld, ScreenPoint );
}

CVector3f CCamera::TransformViewToPixel( Math::CVector3f const& ViewPoint ) const
{
	return TransformScreenToPixel( TransformViewToScreen( ViewPoint ) );
}

CVector3f CCamera::TransformViewToScreen( CVector3f const& ViewPoint ) const
{
	return Transform( m_ViewToScreen, ViewPoint );
}

CVector3f CCamera::TransformViewToWorld( CVector3f const& ViewPoint ) const
{
	return Transform( m_ViewToWorld, ViewPoint );
}

CVector3f CCamera::TransformWorldToPixel( CVector3f const& WorldPoint ) const
{
	return TransformScreenToPixel( TransformWorldToScreen( WorldPoint ) );
}

CVector3f CCamera::TransformWorldToScreen( CVector3f const& WorldPoint ) const
{
	return Transform( m_WorldToScreen, WorldPoint );
}

CVector3f CCamera::TransformWorldToView( CVector3f const& WorldPoint ) const
{
	return Transform( m_WorldToView, WorldPoint );
}

void CCamera::SetRenderTarget( LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DSURFACE9 pRenderTarget )
{
	if (pRenderTarget != NULL && m_pRenderTarget == pRenderTarget)
	{
		return;
	}

	if (m_pRenderTarget != NULL)
	{
		if (m_bDefaultRenderTarget)
		{
			SafeRelease( m_pRenderTarget );
			m_bDefaultRenderTarget = false;
		}
	}

	if (m_pRenderTarget != pRenderTarget)
	{
		if (pRenderTarget != NULL)
		{
			pRenderTarget->AddRef();
		}

		SafeRelease( m_pRenderTarget );
		m_pRenderTarget = pRenderTarget;
	}

	if (pDevice != NULL && pRenderTarget == NULL)
	{
		pDevice->GetRenderTarget( 0, &m_pRenderTarget );
		m_bDefaultRenderTarget = true;
	}

	m_ResourcesValid = true;
}

void CCamera::SetRect( CRect2i const& Rect )
{
	if (Rect.right <= Rect.left ||
		Rect.bottom <= Rect.top)
	{
		throw std::invalid_argument( "camera rect" );
	}

	m_Rect = Rect;
	UpdateProjection();
}

void CCamera::SetFOV( float32 HorizontalAngleRadians )
{
	if (HorizontalAngleRadians <= 0.0f ||
		HorizontalAngleRadians >= KPi)
	{
		throw std::out_of_range( "fov angle" );
	}

	if (m_FOV != HorizontalAngleRadians)
	{
		m_FOV = HorizontalAngleRadians;
		UpdateProjection();
	}
}

void CCamera::SetVerticalFOV( float32 VerticalAngleRadians )
{
	if ((VerticalAngleRadians != -1.0f &&
		VerticalAngleRadians <= 0.0f) ||
		VerticalAngleRadians >= KPi)
	{
		throw std::invalid_argument( "fov vertical angle" );
	}

	if (m_VerticalFOV != VerticalAngleRadians)
	{
		m_VerticalFOV = VerticalAngleRadians;
		UpdateProjection();
	}
}

void CCamera::SetNearClip( float32 NearClip )
{
	if (m_NearClip != NearClip)
	{
		m_NearClip = NearClip;
		UpdateProjection();
	}
}

void CCamera::SetFarClip( float32 FarClip )
{
	if (FarClip <= 0.0f)
	{
		throw std::domain_error( "far clip" );
	}

	if (m_FarClip != FarClip)
	{
		m_FarClip = FarClip;
		UpdateProjection();
	}
}

void CCamera::SetOrthographic( bool bOrthographic )
{
	if (m_bOrthographic != bOrthographic)
	{
		m_bOrthographic = bOrthographic;
		UpdateProjection();
	}
}

void CCamera::PrepareRender( LPDIRECT3DDEVICE9 pDevice, EPrepareForRenderFlags::Flags Flags  )
{
	if (pDevice == NULL)
	{
		return;
	}

	HRESULT Result;

	if (Flags & EPrepareForRenderFlags::RenderTarget)
	{
		Result = pDevice->SetRenderTarget( 0, m_pRenderTarget );
		if (FAILED( Result ))
		{
			throw std::runtime_error( "SetRenderTarget failure" );
		}
	}

	if (Flags & EPrepareForRenderFlags::ViewRect)
	{
		D3DVIEWPORT9 Viewport;
		Viewport.X = m_Rect.left;
		Viewport.Y = m_Rect.top;
		Viewport.Width = m_Rect.right - m_Rect.left;
		Viewport.Height = m_Rect.bottom - m_Rect.top;
		Viewport.MinZ = 0.0f;
		Viewport.MaxZ = 1.0f;
		pDevice->SetViewport( &Viewport );
	}

	if (Flags & EPrepareForRenderFlags::View)
	{
		Result = pDevice->SetTransform( D3DTS_VIEW, m_WorldToView );
		if (FAILED( Result ))
		{
			throw std::runtime_error( "SetTransform failure" );
		}
	}

	if (Flags & EPrepareForRenderFlags::Projection)
	{
		Result = pDevice->SetTransform( D3DTS_PROJECTION, m_Projection );
		if (FAILED( Result ))
		{
			throw std::runtime_error( "SetTransform failure" );
		}
	}
}

void CCamera::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;
	SafeRelease( m_pRenderTarget );
}

void CCamera::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	if (m_ResourcesValid)
	{
		return;
	}

	SetRenderTarget( pDevice );
	m_ResourcesValid = true;
}

bool CCamera::IsValid() const
{
	return m_ResourcesValid;
}

uint32 WTCall CCamera::GetClassFormatVersion() {return 2;}

/*virtual*/ void CCamera::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	GetClassType()->GetProperties()->SerializePropertyValues( this, Info, sContext);
}

/*virtual*/ void CCamera::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	if (Info->GetObjectFormatVersion() < 2)
	{
		throw std::runtime_error( "unsupported CCamera data version" );
	}

	GetClassType()->GetProperties()->DeserializePropertyValues( this, Info, sContext);
}

/*virtual*/ void CCamera::FinalizeDeserialization(CSerializationInfo* Info, CSerializationContext* sContext)
{
	//_BaseClass::FinalizeDeserialization( Info, sContext );
}

bool CCamera::PickTriangle ( int32 X , int32 Y,
                             const CVector3f& Vert0, const CVector3f& Vert1, const CVector3f& Vert2,
                             float32& U, float32& V , CVector3f& Intersection)
{
	using namespace Intersections;
	using namespace NIntersectLineAndPoly;

	CVector3f PickRayDir = TransformPixelToWorld( static_cast<float32>( X ), static_cast<float32>( Y ) );
	CVector3f PickRayOrig = GetPositionComponent();
	PickRayDir -= PickRayOrig;
	PickRayDir.Normalize();
	float32 T;
	
	if (IntersectLineAndPoly<KPolyTriangle | KLineRay>( PickRayOrig, PickRayDir, Vert0, Vert1, Vert2, U, V, &T ))
	{
		Intersection  = PickRayOrig + T*PickRayDir;
		return true;
	}

	return false;
}

bool CCamera::PickRectangle ( int32 X , int32 Y,
                              const CVector3f& Vert0, const CVector3f& Vert1, const CVector3f& Vert2,
                              float32& U, float32& V ,CVector3f& Intersection )
{
	using namespace Intersections;
	using namespace NIntersectLineAndPoly;

	CVector3f PickRayDir = TransformPixelToWorld( static_cast<float32>( X ), static_cast<float32>( Y ) );
	CVector3f PickRayOrig = GetPositionComponent();
	PickRayDir -= PickRayOrig;
	PickRayDir.Normalize();
	float32 T;

	if (IntersectLineAndPoly<KPolyRectangle | KLineRay>( PickRayOrig, PickRayDir, Vert0, Vert1, Vert2, U, V, &T ))
	{
		Intersection  = PickRayOrig + T*PickRayDir;
		return true;
	}
	
	return false;
}

} //namespace Graphics
} //namespace WT
