//
// ControlEvents.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Callback.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace Math;

class CControl;
class CSlider;

// Defines an event that is raised when a control consumes a keyboard event
typedef System::CCallback2<CControl*, uint8>			KeyPressedCallback;
typedef KeyPressedCallback::Event						KeyPressedEvent;

// Defines an event that is raised when a control is mouse-clicked 
typedef System::CCallback2<CControl*, uint32>			MouseClickCallback;
typedef MouseClickCallback::Event						MouseClickEvent;

// Defines an event that is raised when the mouse cursor enters or leaves the area of a control
typedef System::CCallback2<CControl*, const CVector2i&>	MouseMoveCallback;
typedef MouseMoveCallback::Event						MouseMoveEvent;

// Defines an event that is raised when a control's focus has changed as a result of a 
// call to the Form::RequestFocus method
typedef System::CCallback2<CControl*, bool>				FocusChangedCallback;
typedef FocusChangedCallback::Event						FocusChangedEvent;

// Defines an event that is raised when a slider bar's value has changed
typedef System::CCallback2<CSlider*, int32>				SliderValueChangedCallback;
typedef SliderValueChangedCallback::Event				SliderValueChangedEvent;


} // namespace UI
} // namespace Graphics
} // namespace WT
