//
// Input.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

//
// Data structures to contain keyboard and mouse input events, reflecting Windows mouse/keyboard handling routines
//


// Keyboard

namespace EKeyState
{
	enum State
	{
		Invalid = -1,
		Down,
		Up
	};
}

struct KeyboardData
{
	EKeyState::State	KeyState;			// State of key event
	uint32				VKeycode;			// Windows Virtual Key Code of key event
	uint16				RepeatCount;		// Keystroke autorepeat number (always one for EKeyState::Up)
	uint16				ScanCode;			// OEM scan code
	bool				IsExtendedKey:1;	// True if key is an extended key
	bool				IsAltKeyDown:1;		// True if Alt key is down
	bool				PrevDownState:1;	// True if EKeyState::Down and previous key state was EKeyState::Down

	KeyboardData()
	{
		KeyState = EKeyState::Invalid;
		VKeycode = 0xffffffff;
		RepeatCount = 0;
		ScanCode = 0;
		IsExtendedKey = false;
		IsAltKeyDown = false;
		PrevDownState = false;
	}
};


// Mouse

namespace EMouseButtonState
{
	enum State
	{
		Invalid = -1,
		Down,								// Button depressed
		Up,									// Button released
		DblClick,							// Button double clicked
		Rotate,								// Mousewheel rotated
		Move								// Mouse moved
	};
}

namespace EMouseButtonType
{
	enum Type
	{
		None = 0,
		LButton,							// Left mouse button
		MButton,							// Middle mouse button
		RButton,							// Right mouse button
		X1Button,							// First X button
		X2Button,							// Second X button
		Wheel,								// Mouse wheel
		EndOfTypes
	};
}

struct MouseData
{
	EMouseButtonState::State		ButtonState;		// State of mouse button
	EMouseButtonType::Type			ButtonType;			// Mouse button pressed in this event
	uint32							ButtonModifier;		// Windows Mouse button modifier (MK_CONTROL, MK_SHIFT, etc.)
	Math::Vector2i					Position;			// Position of mouse in (Windows) screen coordinates
	int32							WheelRotation;		// Wheel rotation (positive for forward, negative for backward)

	MouseData()
	{
		ButtonState = EMouseButtonState::Invalid;
		ButtonType = EMouseButtonType::None;
		ButtonModifier = 0;
		WheelRotation = 0;
	}
};


} // namespace UI
} // namespace Graphics
} // namespace WT
