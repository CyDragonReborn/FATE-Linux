//
// IControlBehavior.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Callback.h"
#include "WT/System/IInterface.h"
#include "d3d9.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace System;

class IControlBehavior;
class Control;

// Defines an event that is raised just before the behavior applies a control update or
// restores the original state of a control
// Returning false from the callback will signal the control to not apply the update
typedef Callback3r<IControlBehavior*, Control*, IDirect3DDevice9*, bool> ControlBehaviorCallback;
typedef ControlBehaviorCallback::Event ControlBehaviorEvent;


class IControlBehavior : public IInterface
{
public:

	WT_BEGIN_INTERFACE( IControlBehavior )
		WT_INHERITS_INTERFACE( IInterface )
	WT_END_INTERFACE()

	// Sets the control states the behavior is able to modify
	// The State parameter is expected to be one or more OR'ed cmEControlState::State values
	virtual void SetControlStates( uint32 States ) = 0;

	// Retrieves the control states the behavior is able to modify
	// The return value will be one or more OR'ed cmEControlState::State values
	virtual uint32 GetControlStates( ) const = 0;

	// Updates the state of the given control
	// This method will typically be called by controls before rendering is performed
	virtual void UpdateControl( Control* pControl, IDirect3DDevice9* pDevice ) = 0;

	// Restores the given control to its previous state 
	// This method will typically be called by controls after rendering is performed
	virtual void RestoreControl( Control* pControl, IDirect3DDevice9* pDevice ) = 0;

	// Retrieves the OnUpdate event for the control behavior
	virtual ControlBehaviorEvent& OnUpdateControl( ) = 0;

	// Retrieves the OnUpdate event for the control behavior
	virtual ControlBehaviorEvent& OnRestoreControl( ) = 0;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
