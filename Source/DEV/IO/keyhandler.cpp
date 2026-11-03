/*
	keyhandler.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	5/16/2004

*/
#define STRICT

#include <windows.h>
#include <assert.h>

#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "keyhandler.h"

CKeyHandler::CKeyHandler( void ) :
						 m_CapsLock( kFalse )
{
	FlushAll();	

	GetKeyboardState( m_KeyStates );

	if ( m_KeyStates[VK_CAPITAL] )
	{
		m_CapsLock = kTrue;
	}
	else
	{
		m_CapsLock = kFalse;
	}

} // CKeyHandler::CKeyHandler()

CKeyHandler::~CKeyHandler( void )
{
} // CKeyHandler::~CKeyHandler())

void CKeyHandler::Flush( void )
{
	ZeroMemory( m_KeyPressed, 512 );
} // CKeyHandler::Flush()

void CKeyHandler::FlushAll( void )
{
	ZeroMemory( m_KeyHeld, 512 );
	ZeroMemory( m_KeyPressed, 512 );
} // CKeyHandler::FlushAll()

void CKeyHandler::KeyEvent( UINT WMMessage,	// windows message - WM_KEYUP or WM_KEYDOWN
							WPARAM KeyCode )	// the wparam which will be the keycode
{
	switch( WMMessage )
	{
	case WM_KEYDOWN :
	case WM_SYSKEYDOWN :

		if ( !m_KeyHeld[KeyCode] )
		{
			m_KeyPressed[KeyCode] = 1;
		}
		m_KeyHeld[KeyCode] = 1;

		break;

	case WM_KEYUP :
	case WM_SYSKEYUP :

		m_KeyHeld[KeyCode] = 0;
		m_KeyPressed[KeyCode] = 0;

		break;
	}

	GetKeyboardState( m_KeyStates );

	if ( m_KeyStates[VK_CAPITAL] )
	{
		m_CapsLock = kTrue;
	}
	else
	{
		m_CapsLock = kFalse;
	}
} // CKeyHandler::KeyEvent()

bool CKeyHandler::KeyPressed( uint32 Code )	// keycode to check
{
	assert( Code < 512 );
	return m_KeyPressed[Code] == 1;
} // CKeyHandler::KeyPressed()

bool CKeyHandler::KeyHeld( uint32 Code )	// keycode to check
{
	assert( Code < 512 );
	return m_KeyHeld[Code] == 1;
} // CKeyHandler::KeyHeld()

char CKeyHandler::ShiftCharacter( int32 Character,	// character to Shift
								  bool Shift	)	// state of Shift
{
	switch( Character )
	{
	case 32:
		return ' ';
	case 65:
		return Shift ? 'A' : 'a';
	case 66:
		return Shift ? 'B' : 'b';
	case 67:
		return Shift ? 'C' : 'c';
	case 68:
		return Shift ? 'D' : 'd';
	case 69:
		return Shift ? 'E' : 'e';
	case 70:
		return Shift ? 'F' : 'f';
	case 71:
		return Shift ? 'G' : 'g';
	case 72:
		return Shift ? 'H' : 'h';
	case 73:
		return Shift ? 'I' : 'i';
	case 74:
		return Shift ? 'J' : 'j';
	case 75:
		return Shift ? 'K' : 'k';
	case 76:
		return Shift ? 'L' : 'l';
	case 77:
		return Shift ? 'M' : 'm';
	case 78:
		return Shift ? 'N' : 'n';
	case 79:
		return Shift ? 'O' : 'o';
	case 80:
		return Shift ? 'P' : 'p';
	case 81:
		return Shift ? 'Q' : 'q';
	case 82:
		return Shift ? 'R' : 'r';
	case 83:
		return Shift ? 'S' : 's';
	case 84:
		return Shift ? 'T' : 't';
	case 85:
		return Shift ? 'U' : 'u';
	case 86:
		return Shift ? 'V' : 'v';
	case 87:
		return Shift ? 'W' : 'w';
	case 88:
		return Shift ? 'X' : 'x';
	case 89:
		return Shift ? 'Y' : 'y';
	case 90:
		return Shift ? 'Z' : 'z';
		// Numbers:
	case 48:
	case 96:
		return Shift ? ')' : '0';
	case 49:
	case 97:
		return Shift ? '!' : '1';
	case 50:
	case 98:
		return Shift ? '@' : '2';
	case 51:
	case 99:
		return Shift ? '#' : '3';
	case 52:
	case 100:
		return Shift ? '$' : '4';
	case 53:
	case 101:
		return Shift ? '%' : '5';
	case 54:
	case 102:
		return Shift ? '^' : '6';
	case 55:
	case 103:
		return Shift ? '&' : '7';
	case 56:
	case 104:
		return Shift ? '*' : '8';
	case 57:
	case 105:
		return Shift ? '(' : '9';
	case 19:
		return '*';

	// extended characters
	case 241:
		return Shift ? 'Ñ' : 'ñ';
		
	// MISC.:
	case 187:
		return Shift ? '+' : '=';
	case 189:
		return Shift ? '_' : '-';
	case 192:
		return Shift ? '~' : '~';

	case 219:
		return Shift ? '{' : '[';
	case 221:
		return Shift ? '}' : ']';
	case 220:
		return Shift ? '|' : '\\';

	case 186:
		return Shift ? ':' : ';';
	case 222:
		return Shift ? '"' : '\'';
	case 188:
		return Shift ? '<' : ',';
	case 190:
		return Shift ? '>' : '.';
	case 191:
		return Shift ? '?' : '/';

		// Keypad:
	case 110:
		return '.';
	case 107:
		return '+';
	case 109:
		return '-';
	case 106:
		return '*';
	case 111:
		return '/';

	default:
		return (char)Character;
	}
} // CKeyHandler::ShiftCharacter