/*
	keyhandler.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	5/16/2004

*/

#ifndef _KEYHANDLER_H
#define _KEYHANDLER_H


#include "../UTILITIES/constants.h"


class CKeyHandler
{
public:
			CKeyHandler( void );
			~CKeyHandler( void );

	void	KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
					  WPARAM KeyCode );	// the wparam which will be the keycode

	void	Flush( void );				// should be called each frame to flush key pressed events
	void	FlushAll( void );			// should be called when you want to flush key held events as well
										


	bool	KeyPressed( uint32 Code );	// keycode to check

	bool	KeyHeld( uint32 Code );	// keycode to check

	bool	CapsLock( void )	{	return m_CapsLock;		};

	char	ShiftCharacter( int32 Character,	// character to Shift
							bool Shift	);	// state of Shift
private:

	bool					m_CapsLock;

	// Key map
	BYTE					m_KeyPressed[512];
	BYTE					m_KeyHeld[512];

	BYTE					m_KeyStates[256];
};


#endif