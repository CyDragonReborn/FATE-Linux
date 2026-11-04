//
// ControlBehavior.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/Callback.h"
#include <d3d9.h>


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace System;

class CControl;
class CControlBehavior;

// Defines an event that is raised just before the behavior applies a control update or
// restores the original state of a control
// Returning false from the callback will signal the control to not apply the update
typedef CCallback3r<CControlBehavior*, CControl*, IDirect3DDevice9*, bool> ControlBehaviorCallback;
typedef ControlBehaviorCallback::Event ControlBehaviorEvent;


// Describes the current state of a control
// These are intended (but not limited) for use with control behaviors
namespace EControlState{
	enum State
	{
		Normal		= 1<<0,		// default/normal state
		Disabled	= 1<<2,		// the control is disabled
		Hidden		= 1<<3,		// the control is currently non-visible
		Focus		= 1<<4,		// the control currently has focus
		MouseOver	= 1<<5,		// the mouse cursor is currently over the control's bounding area
		Pressed		= 1<<6,		// the control is being pressed either via hotkey or a mouse button

		All			= 0xffff,	// mask of all available states
	};
}


class CControlBehavior : public System::CObject
{
public:

	WT_ABSTRACT_CLASS( CControlBehavior, System::CObject )

	CControlBehavior( ) : m_ControlStates( EControlState::Normal )
	{
	}
	CControlBehavior( const CControlBehavior& Other)
	{
		CopyPropertiesFrom( Other );
	}
	virtual ~CControlBehavior( )
	{
	}

	// Sets the control states the behavior is able to modify
	// The State parameter is expected to be one or more OR'ed cmEControlState::State values
	virtual void SetControlStates( uint32 States )
	{
		assert( AreValidControlStates( States ) );

		if( AreValidControlStates( States ) )
		{
			m_ControlStates = States;
		}
	}

	// Retrieves the control states the behavior is able to modify
	// The return value will be one or more OR'ed cmEControlState::State values
	virtual uint32 GetControlStates( ) const 
	{
		return m_ControlStates;
	}

	// Updates the state of the given control
	// This method will typically be called by controls before rendering is performed
	virtual void UpdateControl( CControl* pControl, IDirect3DDevice9* pDevice )
	{
		m_UpdateWasFiltered = false;
		if( FilterUpdate( pControl, pDevice ) )
		{
			m_UpdateWasFiltered = true;
			return;
		}
		if( m_UpdateEvent.GetCount() > 0 && false == m_UpdateEvent( this, pControl, pDevice ) )
		{
			m_UpdateWasFiltered = true;
			return;
		}

		UpdateControlInternal( pControl, pDevice );
	}

	// Restores the given control to its previous state 
	// This method will typically be called by controls after rendering is performed
	virtual void RestoreControl( CControl* pControl, IDirect3DDevice9* pDevice )
	{
		if( !m_UpdateWasFiltered )
		{
			if( m_RestoreEvent.GetCount( ) > 0 && false == m_RestoreEvent( this, pControl, pDevice ) )
			{
				return;
			}

			RestoreControlInternal( pControl, pDevice );
		}
	}

	// Retrieves the OnUpdate event for the control behavior
	virtual ControlBehaviorEvent& OnUpdateControl( )
	{
		return m_UpdateEvent;
	}

	// Retrieves the OnUpdate event for the control behavior
	virtual ControlBehaviorEvent& OnRestoreControl( )
	{
		return m_RestoreEvent;
	}

protected:	// methods

	// Determines whether the given value is a valid representation of one or more
	// cmEControlState::State values
	virtual bool AreValidControlStates( uint32 States )
	{
		using namespace EControlState;
				
		if( States & Normal )
			return true;
		if( States & Disabled )
			return true;
		if( States & Hidden )
			return true;
		if( States & Focus )
			return true;
		if( States & MouseOver )
			return true;
		if( States & Pressed )
			return true;

		return false;
	}

	// Internal update method called only after filtering is performed
	// Note that all derived classes must implement this method
	virtual void UpdateControlInternal( CControl* pControl, IDirect3DDevice9* pDevice ) = 0;

	// Internal restore method called only after filtering is performed
	// This is the trivial implementation; derived classes may override this method as needed
	virtual void RestoreControlInternal( CControl* pControl, IDirect3DDevice9* pDevice )
	{
		pControl;
		pDevice;
	}

	// Determines whether the given control should be filtered from update because its current
	// state is not a state this behavior modifier instance controls
	virtual bool FilterUpdate( CControl* pControl, IDirect3DDevice9* pDevice );

	// Copies Other's properties/state to this object
	virtual void CopyPropertiesFrom( const CControlBehavior& Other)
	{
		m_ControlStates = Other.m_ControlStates;
		m_UpdateWasFiltered = Other.m_UpdateWasFiltered;

		// Copy event handlers
		uint32 Count = 0;
		ControlBehaviorCallback** ppCallbacks = NULL;

		// Update events
		Count = Other.m_UpdateEvent.GetCount();
		ppCallbacks = Other.m_UpdateEvent.GetCallbacks();
		for ( uint32 i=0; i<Count; ++i )
		{
			m_UpdateEvent.Add( *ppCallbacks );
			++ppCallbacks;
		}

		// Restore events
		Count = Other.m_RestoreEvent.GetCount();
		ppCallbacks = Other.m_RestoreEvent.GetCallbacks();
		for ( uint32 i=0; i<Count; i++ )
		{
			m_RestoreEvent.Add( *ppCallbacks );
			++ppCallbacks;
		}
	}

protected: // data

	// User properties
	uint32	m_ControlStates;		// one or more cmEControlState::State values

	// Internal properties
	bool	m_UpdateWasFiltered;	// flags whether UpdateControl rejected the update so further calls to 
									// RestoreControl know whether or not to actually modify the control

protected: // events

	ControlBehaviorEvent	m_UpdateEvent;
	ControlBehaviorEvent	m_RestoreEvent;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
