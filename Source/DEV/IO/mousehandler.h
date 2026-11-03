/*
	mousehandler.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/12/2004

*/

#ifndef _MOUSEHANDLER_H
#define _MOUSEHANDLER_H

#ifndef WM_MOUSEWHEEL
    #define WM_MOUSEWHEEL                   0x020A
    #define WHEEL_DELTA                     120     /* Value for rolling one detent */
    #define WHEEL_PAGESCROLL                ( UINT_MAX ) /* Scroll one page */
#endif 


#ifndef GET_WHEEL_DELTA_WPARAM
	#define GET_WHEEL_DELTA_WPARAM( wparam ) ( (short)HIWORD ( wparam ) )
#endif

#include "../UTILITIES/constants.h"


enum EMouseButton
{
	KButtonLeft,
	KButtonRight,
	KButtonMiddle,
	KMouseButtons
};

class CMouseHandler
{
public:
			CMouseHandler( void );
			~CMouseHandler( void );

	void	MouseEvent( UINT WMMessage,		// windows message - WM_LBUTTONDOWN, etc.
						WPARAM WMParam );	// the wparam which contains mousewheel info

	void	Flush( void );					// should be called each frame to flush button pressed events
	void	FlushAll( void );				// should be called when you want to flush button held events as well
					
	// you must call update if you want a current mouse position
	void	Update( HWND hWnd );			// hwnd of the active window

	bool	ButtonPressed( EMouseButton Button );	// button to check

	bool	ButtonHeld( EMouseButton Button );	// button to check

	bool	ButtonDoubleClick( EMouseButton Button );	// button to check

	const POINT&	MousePosition( void )	{	return m_MousePosition;		};

	// if your UI elements are using a different screen resolution
	// than your actual screen resolution, you can translate mouse
	// coordinates to your 'virtual' screen resolution by passing
	// in the width and height of your UI elements assumed resolution;
	const POINT&	VirtualMousePosition( float32 ActualScreenWidth,		// width of the actual HWND
										  float32 ActualScreenHeight,	// height of the actual HWND
										  float32 VirtualScreenWidth,	// width of virtual screen
										  float32 VirtualScreenHeight );	// height of virtual screen

	int32			MouseWheelDelta( void )	{	return m_MouseWheelDelta;	};
private:

	// Key map
	BYTE					m_ButtonPressed[KMouseButtons];
	BYTE					m_ButtonHeld[KMouseButtons];
	BYTE					m_ButtonDoubleClick[KMouseButtons];


	POINT		m_MousePosition;
	POINT		m_VirtualMousePosition;

	int32		m_MouseWheelDelta;
};


#endif