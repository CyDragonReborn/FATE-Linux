//
// Camera.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "d3d9.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Math/Math.h"
#include "WT/Math/LinearAlgebra/Vector3f.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Math/LinearAlgebra/Plane.h"
#include "WT/Math/Objects/AABox.h"
#include "WT/Math/Rects/Rect2i.h"
#include "WT/Math/MathSerialization.h"
#include "WT/Graphics/Frustum.h"
#include "WT/Graphics/IDeviceObject.h"
#include "WT/Graphics/IMovable.h"

namespace WT
{
namespace Graphics
{

namespace EPrepareForRenderFlags
{
	///Use this flags to prepare the device for rendering using the different options
	///You can combine multiple flags to set more than one thing at once
	enum Flags
	{
		Projection		= 1,	//Set only the projection matrix to the device
		View			= 2,	//Set only the view matrix
		Matrices		= 3,	//Set both projection and view matrices
		RenderTarget	= 4,	//Set the rendertarget on the device
		ViewRect		= 8,	//Set the view rect on the device
		All				= 15,	//Set all options at once
	};
}

/// A camera represetns a movable point in space with a directional viewing eye
/// includes a projection transformation and viewing transformation

typedef System::SmartPointer<class CCamera> HCamera;
class CCamera : public System::CObject, public IDeviceObject, public CMovableImpl, 
	public System::Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS( CCamera, System::CObject );
		WT_INHERITS_INTERFACE( IDeviceObject );
		WT_INHERITS_INTERFACE( IMovable );
		WT_INHERITS_INTERFACE( ISerializable );

		WT_BEGIN_PROPERTIES();
			WT_PROP_RW_BYREF( Name, string );
			WT_PROP_RW_BYREF( Rect, Math::CRect2i );
			WT_PROP_RW_BYVAL( FOV, float32 );
			WT_PROP_RW_BYVAL( VerticalFOV, float32 );
			WT_PROP_RW_BYVAL( NearClip, float32 );
			WT_PROP_RW_BYVAL( FarClip, float32 );
			WT_PROP_RW_BYVAL( Orthographic, bool );
		WT_END_PROPERTIES();

	WT_END_CLASS();

	/// Set the transform for the movable object
	virtual void SetTransform( Math::CMatrix4f const& Transform );

	std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	LPDIRECT3DSURFACE9 GetRenderTarget() const {return m_pRenderTarget;}

	/// Get the viewport rectangle.
	Math::CRect2i const& GetRect() const {return m_Rect;}

	/// Get the field of view in radians. The field of view is
	/// the angle formed by the left and right clip planes.
	float32 GetFOV() const {return m_FOV;}

	/// Get the vertical FOV in radians. The field of view is the angle
	/// formed by the top and bottom clip planes. Normally this will be computed
	/// based off of the rectangle dimensions and horizontal FOV, however the vertical
	/// FOV may be any value in the range (0, Pi) if explicitly set with SetFOV().
	float32 GetVerticalFOV() const;

	float32 GetNearClip() const {return m_NearClip;}
	float32 GetFarClip() const {return m_FarClip;}
	
	/// Get various transformation matrices. View space is physical space from the point of view of 
	/// the camera; with x right, y up, and z forwards. Screen space is projected space and 
	/// coordinates typically range from (-1,-1,0) for the bottom left corner of the near plane to
	/// (+1,+1,1) for the top right corner of the far plane. Pixel space has x and y coordinates
	/// within the viewport (GetRect()) range and the z coordinate is the same as in screen space.
	Math::CMatrix4f		   GetPixelToScreen() const;
	Math::CMatrix4f		   GetPixelToView() const;
	Math::CMatrix4f		   GetPixelToWorld() const;
	Math::CMatrix4f		   GetScreenToPixel() const;
	Math::CMatrix4f const& GetScreenToView() const {return m_ScreenToView;}
	Math::CMatrix4f const& GetScreenToWorld() const {return m_ScreenToWorld;}
	Math::CMatrix4f		   GetViewToPixel() const;
	Math::CMatrix4f const& GetViewToScreen() const {return m_ViewToScreen;}
	Math::CMatrix4f const& GetViewToWorld() const {return m_ViewToWorld;}
	Math::CMatrix4f		   GetWorldToPixel() const;
	Math::CMatrix4f const& GetWorldToScreen() const {return m_WorldToScreen;}
	Math::CMatrix4f const& GetWorldToView() const {return m_WorldToView;}
	/// Matrix access for Direct3D device direct access
	Math::CMatrix4f const& GetProjection() const {return m_Projection;}
	Math::CMatrix4f const& GetView() const { return m_WorldToView;}

	CFrustum const& GetFrustum() const {return m_Frustum;}

	/// Perform various transformations on a point's coordinates. These functions take into
	/// account perspective projection.
	Math::CVector3f TransformPixelToScreen( Math::CVector3f const& PixelPoint ) const;
	Math::CVector3f TransformPixelToView( Math::CVector3f const& PixelPoint ) const;
	Math::CVector3f TransformPixelToWorld( Math::CVector3f const& PixelPoint ) const;
	Math::CVector3f TransformPixelToScreen( float32 X, float32 Y, float32 Z = 0.0f ) const;
	Math::CVector3f TransformPixelToView( float32 X, float32 Y, float32 Z = 0.0f ) const;
	Math::CVector3f TransformPixelToWorld( float32 X, float32 Y, float32 Z = 0.0f ) const;

	Math::CVector3f TransformScreenToPixel( Math::CVector3f const& ScreenPoint ) const;
	Math::CVector3f TransformScreenToView( Math::CVector3f const& ScreenPoint ) const;
	Math::CVector3f TransformScreenToWorld( Math::CVector3f const& ScreenPoint ) const;

	Math::CVector3f TransformViewToPixel( Math::CVector3f const& ViewPoint ) const;
	Math::CVector3f TransformViewToScreen( Math::CVector3f const& ViewPoint ) const;
	Math::CVector3f TransformViewToWorld( Math::CVector3f const& ViewPoint ) const;

	Math::CVector3f TransformWorldToPixel( Math::CVector3f const& WorldPoint ) const;
	Math::CVector3f TransformWorldToScreen( Math::CVector3f const& WorldPoint ) const;
	Math::CVector3f TransformWorldToView( Math::CVector3f const& WorldPoint ) const;

	/// Sets the target device and surface of the camera. If no surface is specified, the current
	/// render target of the device is used.
	void SetRenderTarget( LPDIRECT3DDEVICE9 pDevice, LPDIRECT3DSURFACE9 pRenderTarget = NULL );

	/// Set the viewport rectangle.
	void SetRect( Math::CRect2i const& Rect );

	/// Set the horizontal field of view in radians.
	void SetFOV( float32 HorizontalAngleRadians );
	
	/// Set the vertical field of view in radians. If a value of -1 is specified for VerticalAngleRadians,
	/// then the vertical FOV will be comptued based off of the horizontal FOV and rectangle dimensions.
	/// Otherwise, the specified value for vertical FOV will be used.
	void SetVerticalFOV( float32 VerticalAngleRadians );

	/// Set clip near value. Non-positive values are rejected. If the clip near value is not 
	/// less than the clip far value, then safe default clip near and far values will be used internally.
	void SetNearClip( float32 NearClip );

	/// Set clip far value. Non-positive values are rejected. If the clip far value is not 
	/// greater than the clip near value, then safe default clip near and far values will be used internally.
	void SetFarClip( float32 FarClip );

	//Member functions for dealing with orthographic projection. If orthographic projection
	//is enabled, the frustum will be a box ranging from the near clip plane to the far
	//clip plane. The nearest and furthest faces of the boxes will be of equal size, calculated
	//based on the far clip plane distance and the field of view angles.
	bool GetOrthographic() const {return m_bOrthographic;}
	void SetOrthographic( bool bOrthographic );

	/// Prepare the device for rendering
	void PrepareRender(LPDIRECT3DDEVICE9 pDevice, EPrepareForRenderFlags::Flags Flags = EPrepareForRenderFlags::Matrices );

	CCamera();
	CCamera( const CCamera& Other );
	virtual ~CCamera();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	//Pick a triangle from a pixel location, if an intersection happens, the point is returned along
	//with the UV coordinates of it.
	bool PickTriangle ( int32 X , int32 Y,
	                    const CVector3f& Vert0, const CVector3f& Vert1, const CVector3f& Vert2,
                        float32& U, float32& V , CVector3f& Intersection );

	//Pick a rectangle from a pixel location, if an intersection happens, the point is returned along
	//with the UV coordinates of it.
	bool PickRectangle ( int32 X , int32 Y,
	                     const CVector3f& Vert0, const CVector3f& Vert1, const CVector3f& Vert2,
                         float32& U, float32& V , CVector3f& Intersection );


	//
	// ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private:
	void UpdateProjection();
	static Math::CVector3f Transform( Math::CMatrix4f const& Matrix, Math::CVector3f const& Input3D );

private:
	std::string m_Name;

	LPDIRECT3DSURFACE9 m_pRenderTarget;
	bool m_bDefaultRenderTarget : 1;
	bool m_ResourcesValid : 1;
	Math::CRect2i m_Rect;

	float32 m_FOV; //Horizontal FOV angle in radians
	float32 m_VerticalFOV;  //Vertical FOV angle in radians, or -1.0 to select a computed value based on 
							//horizontal FOV and rectange dimensions
	float32 m_NearClip; //Distance of frustum near clipping plane
	float32 m_FarClip; //Distance of frustum far clipping plane
	bool	m_bOrthographic; //Orthographic projection?

	Math::CMatrix4f m_ViewToWorld; //View-to-world matrix - product of all object transforms
	Math::CMatrix4f m_WorldToView; //World-to-view matrix; inverse of the view-to-world matrix
	Math::CMatrix4f m_Projection; //DirectX-like projection matrix
	Math::CMatrix4f m_ViewToScreen; //View-to-screen matrix
	Math::CMatrix4f m_ScreenToView; //Screen-to-view matrix
	Math::CMatrix4f m_WorldToScreen; //World-to-screen matrix
	Math::CMatrix4f m_ScreenToWorld; //Screen-to-world matrix

	CFrustum m_Frustum;
};

} //namespace Graphics
} //namespace WT
