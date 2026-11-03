//
// State.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The State class defines a single node within a state machine.
//  This class is useful for setting up complex state diagrams with 
//  well-defined transitions between states.
//  The State class contains a single CStringOrID object used to uniquely
//  identify the state within its parent state machine.
//  State machines will generally contain a collection of State nodes, as well
//  as the 'current' node.  Each State object contains a collection of nodes
//  it is able to transition to.  
//  State objects also contain a multicast Event member capable of firing user
//  callbacks when a state machine transitions from one state to another.
//  The State class can be thought of as merely a container of state node 
//  information.  The actual transitioning from one state to another is handled 
//  exclusively by its parent StateMachine object.
//

#pragma once

#include "WT/System/Callback.h"
#include "WT/System/StringOrID.h"

namespace WT
{
	using namespace WT::System;

namespace AI
{


// Defines an event that can be raised when a state object has changed state
class CState;
typedef CCallback1<CState*>				StateChangedCallback;
typedef StateChangedCallback::Event		StateChangedEvent;


typedef System::SmartPointer<class CState> HState;
class CState : public System::CObject
{
public:

	WT_CLASS( CState, System::CObject )
	//TODO Add all derived interface base classes when supported in Type

	//
	// Constructors / destructor
	//

	CState();
	CState( const CState& Other );
	CState( int32 ID );
	CState( int32 ID, const std::string& Name );
	CState( const CStringOrID& ID );
	virtual ~CState();

	//
    // State methods
    //

	// Associates the given identifier with the state object
	void SetID( const CStringOrID& ID );

	// Retrieves the identifier associated with the state object
	const CStringOrID& GetID() const;

	// Retrieves the set of the state transition objects this state is capable of moving to
	std::list<CStringOrID>& GetTransitions();

	// Adds the given transition identifier to the back of the transition list
	void AddTransition( const CStringOrID& TransitionID );

	// Associates arbitrary user data with the state
    void SetUserData( void* pUserData );

	// Retrieves any arbitrary user data associated with the state, or NULL if no
	// user data is associated with the state
    void* GetUserData() const;

	// Notifies the state object that its logical state has either been entered or exited, which
	// may trigger user events to be raised
	void NotifyEvent( bool Enter );

	// Retrieves the OnEnterState event associated with the state object object
	// This event is raised when the state object is activated as the current object
	StateChangedEvent& OnEnterState();

	// Retrieves the OnLeaveState event associated with the state object object
	// This event is raised when the state object is deactivated from being the current object
	StateChangedEvent& OnLeaveState();

private:	// data

	CStringOrID				m_ID;
	std::list<CStringOrID>	m_Transitions;
	void*					m_pUserData;

private:	// events

	StateChangedEvent		m_EnterStateEvent;
	StateChangedEvent		m_LeaveStateEvent;
};


} // namespace AI
} // namespace WT
