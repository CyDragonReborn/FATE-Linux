/*
	mousehandler.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/12/2004

*/
#define STRICT

#include <windows.h>
#include <assert.h>

#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "mousehandler.h"

CMouseHandler::CMouseHandler( void ) :
										m_MouseWheelDelta( 0 )
{
	ZeroMemory ( &m_VirtualMousePosition, sizeof( m_VirtualMousePosition ));
	ZeroMemory ( &m_MousePosition, sizeof ( m_MousePosition ));

	FlushAll();	
} // CMouseHandler::CMouseHandler()

CMouseHandler::~CMouseHandler( void )
{
} // CMouseHandler::~CMouseHandler()

// you must call update if you want a current mouse position
void CMouseHandler::Update( HWND hWnd )	// hwnd of the active window
{
	GetCursorPos( &m_MousePosition );
	ScreenToClient( hWnd, &m_MousePosition );
} // CMouseHandler::Update()

void CMouseHandler::Flush( void )
{
	m_MouseWheelDelta = 0;
	ZeroMemory( m_ButtonPressed, KMouseButtons );
	ZeroMemory( m_ButtonDoubleClick, KMouseButtons );
} // CMouseHandler::Flush()

void CMouseHandler::FlushAll( void )
{
	m_MouseWheelDelta = 0;
	ZeroMemory( m_ButtonHeld, KMouseButtons );
	ZeroMemory( m_ButtonPressed, KMouseButtons );
	ZeroMemory( m_ButtonDoubleClick, KMouseButtons );
} // CMouseHandler::FlushAll()

void CMouseHandler::MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONDOWN, etc.
							    WPARAM WMParam )	// the wparam which contains mousewheel info
{
	switch( WMMessage )
	{
	case WM_LBUTTONDBLCLK :
		m_ButtonDoubleClick[KButtonLeft] = 1;
		break;
	case WM_RBUTTONDBLCLK :
		m_ButtonDoubleClick[KButtonRight] = 1;
		break;
	case WM_LBUTTONDOWN :

		if ( !m_ButtonHeld[KButtonLeft] )
		{
			m_ButtonPressed[KButtonLeft] = 1;
		}
		m_ButtonHeld[KButtonLeft] = 1;

		break;

	case WM_LBUTTONUP :

		m_ButtonHeld[KButtonLeft] = 0;
		m_ButtonPressed[KButtonLeft] = 0;

		break;
	case WM_RBUTTONDOWN :

		if ( !m_ButtonHeld[KButtonRight] )
		{
			m_ButtonPressed[KButtonRight] = 1;
		}
		m_ButtonHeld[KButtonRight] = 1;

		break;

	case WM_RBUTTONUP :

		m_ButtonHeld[KButtonRight] = 0;
		m_ButtonPressed[KButtonRight] = 0;

		break;

	case WM_MBUTTONDOWN :

		if ( !m_ButtonHeld[KButtonMiddle] )
		{
			m_ButtonPressed[KButtonMiddle] = 1;
		}
		m_ButtonHeld[KButtonMiddle] = 1;

		break;

	case WM_MBUTTONUP :

		m_ButtonHeld[KButtonMiddle] = 0;
		m_ButtonPressed[KButtonMiddle] = 0;

		break;

	case WM_MOUSEWHEEL :
			m_MouseWheelDelta = GET_WHEEL_DELTA_WPARAM( WMParam );
		break;
	}

} // CMouseHandler::MouseEvent()

bool CMouseHandler::ButtonPressed( EMouseButton Button )	// button to check
{
	assert( Button < KMouseButtons );
	return m_ButtonPressed[Button] == 1;
} // CMouseHandler::ButtonPressed()

bool CMouseHandler::ButtonHeld( EMouseButton Button )	// button to check
{
	assert( Button < KMouseButtons );
	return m_ButtonHeld[Button] == 1;
} // CMouseHandler::ButtonHeld()

bool CMouseHandler::ButtonDoubleClick( EMouseButton Button )	// button to check
{
	assert( Button < KMouseButtons );
	return m_ButtonDoubleClick[Button] == 1;
} // CMouseHandler::ButtonDoubleClick()

const POINT& CMouseHandler::VirtualMousePosition( float32 ActualScreenWidth,		// width of the actual HWND
												  float32 ActualScreenHeight,	// height of the actual HWND
												  float32 VirtualScreenWidth,	// width of virtual screen
												  float32 VirtualScreenHeight )	// height of virtual screen
{
	float32 XAspect = VirtualScreenWidth / ActualScreenWidth;
	float32 YAspect = VirtualScreenHeight / ActualScreenHeight;
	m_VirtualMousePosition.x = (int32)( (float32)m_MousePosition.x * XAspect );
	m_VirtualMousePosition.y = (int32)( (float32)m_MousePosition.y * YAspect );

	return m_VirtualMousePosition;
} // CMouseHandler::MouseScreenPosition()