//
// ArcBall.cpp
//
// Copyright (C) 2005 WildTangent, Inc.
// All Rights Reserved
//

#include "WT/stdafx.h"

#include "WT/System/Platform.h"
#include "WT/Graphics/ArcBall.h"

using namespace WT::System;
using namespace WT::Math;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( ArcBall )

ArcBall::ArcBall()
{
	Reset();
	m_Offset.x = m_Offset.y = 0;
	RECT rc;
	GetClientRect( GetForegroundWindow(), &rc );
	SetWindow( rc.right, rc.bottom );
}

void ArcBall::Reset()
{
	m_Down.SetNoRotation();
	m_Now.SetNoRotation();
	m_Rotation.SetIdentity();
	m_Translation.SetIdentity();
	m_TranslationDelta.SetIdentity();
    m_Drag = false;
    m_RadiusTranslation = 1.0f;
    m_Radius = 1.0f;
}

CVector3f ArcBall::ScreenToVector( float32 ScreenPtX , float32 ScreenPtY )
{
	// Scale to screen
	float32 x = -(ScreenPtX - m_Offset.x - m_Width/2)  / (m_Radius*m_Width/2);
	float32 y =  (ScreenPtY - m_Offset.y - m_Height/2) / (m_Radius*m_Height/2);

	float32 z   = 0.0f;
	float32 mag = x*x + y*y;

	if( mag > 1.0f )
	{
		float32 scale = 1.0f/sqrtf(mag);
		x *= scale;
		y *= scale;
	}
	else
		z = sqrtf( 1.0f - mag );

	// Create vector
	CVector3f ArcPoint(x, y, z);

	// Adjust vector based on Arcball orientation
	return (m_Rotation * ArcPoint);
}

CQuaternion ArcBall::QuatFromBallPoints(const CVector3f &From, const CVector3f& To )
{
	float32 Dot = From.DotProduct ( To );
	CVector3f Part = From.CrossProduct ( To );

	return CQuaternion ( Part.x, Part.y, Part.z, Dot );
}

void ArcBall::OnBegin( int32 X, int32 Y )
{
	// Only enter the drag state if the click falls
	// inside the click rectangle.
	if( X >= m_Offset.x && X < m_Offset.x + m_Width && Y >= m_Offset.y && Y < m_Offset.y + m_Height )
	{
		m_Drag = true;
		m_Down = m_Now;
		m_DownPt = ScreenToVector( (float32)X, (float32)Y );
	}
}

void ArcBall::OnMove( int32 X, int32 Y )
{
	if (m_Drag) 
	{ 
		m_CurrentPt = ScreenToVector( (float32)X, (float32)Y );
		m_Now = m_Down * QuatFromBallPoints( m_DownPt, m_CurrentPt );
	}
}

void ArcBall::OnEnd()
{
	m_Drag = false;
}

LRESULT ArcBall::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// Current mouse position
	int32 MouseX = (short)LOWORD(lParam);
	int32 MouseY = (short)HIWORD(lParam);

	switch( uMsg )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
			SetCapture( hWnd );
			OnBegin( MouseX, MouseY );
			return TRUE;

		case WM_LBUTTONUP:
			ReleaseCapture();
			OnEnd();
			return TRUE;

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
			SetCapture( hWnd );
			// Store off the position of the cursor when the button is pressed
			m_LastMouse.x = MouseX;
			m_LastMouse.y = MouseY;
			return TRUE;

		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			ReleaseCapture();
			return TRUE;

		case WM_MOUSEWHEEL:
			{
				short Delta = GET_WHEEL_DELTA_WPARAM(wParam);
				CVector3f TranslationVector( 0.0f, 0.0f, Delta*0.05f );
				
				// Adjust translation for ArcBall orientation
				TranslationVector = m_Rotation * TranslationVector;

				m_TranslationDelta.SetTranslation( TranslationVector );
				m_Translation = m_TranslationDelta * m_Translation;
			}
			return TRUE;

		case WM_MOUSEMOVE:
			if( MK_LBUTTON & wParam )
			{
				OnMove( MouseX, MouseY );
			}
			else if( (MK_RBUTTON & wParam) || (MK_MBUTTON & wParam) )
			{
				// Normalize based on size of window and bounding sphere radius
				float32 DeltaX = ( m_LastMouse.x-MouseX ) * m_RadiusTranslation / m_Width;
				float32 DeltaY = ( m_LastMouse.y-MouseY ) * m_RadiusTranslation / m_Height;

				CVector3f TranslationVector;

				if( wParam & MK_RBUTTON )
				{
					TranslationVector.Set( -2*DeltaX, 2*DeltaY, 0.0f );

					// Adjust translation for ArcBall orientation
					TranslationVector = m_Rotation * TranslationVector;

					m_TranslationDelta.SetTranslation ( TranslationVector );
					m_Translation = m_TranslationDelta * m_Translation;
				}
				else  // wParam & MK_MBUTTON
				{
					TranslationVector.Set( 0.0f, 0.0f, 5*DeltaY );

					// Adjust translation for ArcBall orientation
					TranslationVector = m_Rotation * TranslationVector;

					m_TranslationDelta.SetTranslation ( TranslationVector );
					m_Translation = m_TranslationDelta * m_Translation;
				}

				// Store mouse coordinate
				m_LastMouse.x = MouseX;
				m_LastMouse.y = MouseY;
			}
			return TRUE;
	}

	return FALSE;
}

} //namespace Graphics
} //namespace WT
