//
// ArcBall.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/Math/Math.h"
#include "d3d9.h"

namespace WT
{
using namespace Math;

namespace Graphics
{

typedef System::SmartPointer<class ArcBall> HArcBall;
class ArcBall : public System::CObject
{
public:

	WT_CLASS( ArcBall, System::CObject );

	ArcBall();

	void Reset(); 

	void SetTranslationRadius( float32 RadiusTranslation )
	{
		m_RadiusTranslation = RadiusTranslation;
	}

	void SetWindow( int32 Width, int32 Height, float32 Radius = 0.9f )
	{
		m_Width = Width;
		m_Height = Height;
		m_Radius = Radius;
		m_Center.Set ( m_Width*0.5f , m_Height*0.5f );
	}

	void SetOffset ( int32 X ,int32 Y )
	{
		m_Offset.x = X;
		m_Offset.y = Y;
	}

	// start the rotation (pass current mouse position)
	void OnBegin ( int32 X, int32 Y );

	// continue the rotation (pass current mouse position)
	void OnMove ( int32 X , int32 Y );

	// end the rotation 
	void OnEnd ();

	// call this to automatically handle left, middle, right buttons
	LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	// Functions to get/set state
	CMatrix4f GetRotationMatrix() const
	{
		CMatrix4f Result( false);
		m_Now.ToMatrix ( &Result );
		return Result;
	}

	const CMatrix4f& GetOrientationMatrix() const
	{
		return m_Rotation;
	}
	void SetOrientationMatrix( const CMatrix4f& Rotation )
	{
		m_Rotation = Rotation;
	}

	const CMatrix4f& GetTranslationMatrix() const
	{
		return m_Translation;
	}
	void SetTranslation( const CVector3f& Translation )
	{
		m_Translation.SetTranslation( Translation );
	}

	const CMatrix4f& GetTranslationDeltaMatrix() const
	{
		return m_TranslationDelta;
	}

	bool IsBeingDragged() const
	{
		return m_Drag;
	}

	const CQuaternion& GetQuatNow() const
	{
		return m_Now;
	}

	void SetQuatNow ( const CQuaternion& Now )
	{
		m_Now = Now;
	}

	static CQuaternion QuatFromBallPoints ( const CVector3f &From , const CVector3f &To );

private:

	CMatrix4f	m_Rotation;         // Matrix for arc ball's orientation
	CMatrix4f	m_Translation;      // Matrix for arc ball's position
	CMatrix4f	m_TranslationDelta; // Matrix for arc ball's position

	POINT		m_Offset;	// window offset, or upper-left corner of window
	int32		m_Width;	// arc ball's window width
	int32		m_Height;	// arc ball's window height
	CVector2f	m_Center;	// center of arc ball 
	float32		m_Radius;	// arc ball's radius in screen coords
	float32		m_RadiusTranslation; // arc ball's radius for translating the target

	CQuaternion	m_Down;	// CQuaternion before button down
	CQuaternion	m_Now;	// Composite quaternion for current drag
	bool		m_Drag;	// Whether user is dragging arc ball

	POINT		m_LastMouse;	// position of last mouse point
	CVector3f	m_DownPt;		// starting point of rotation arc
	CVector3f	m_CurrentPt;	// current point of rotation arc

	CVector3f ScreenToVector ( float32 ScreenPtX , float32 ScreenPtY );
};

} //namespace Graphics
} //namespace WT
