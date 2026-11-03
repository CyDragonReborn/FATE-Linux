//
// StateMachine.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The StateMachine class defines a collection of individual State nodes 
//  along with the logic to transition between nodes.
//  This class is useful for setting up complex state diagrams with 
//  well-defined transitions between states.
//  In addition to containing a pool of State nodes, the StateMachine class
//  also contains a stack of 'active' nodes.  The top of the active stack
//  indicates the 'current' node in the state diagram. This stacking
//  mechanism allows the state machine to remember previous states that can
//  be re-entered by popping the active stack.
//  After seeding a StateMachine object with a collection of possible state
//  nodes, an initial state should be chosen.  Users may also register a 
//  custom event callback that is raised when the state machine is evaluated.
//  During the user callback, the state of the application can be evaluated, 
//  and the state machine can be potentially signaled to transition to another
//  state.
//


#pragma once

#include "WT/System/Callback.h"
#include "WT/System/StringOrID.h"
#include "WT/AI/State.h"

namespace WT
{
	using namespace WT::System;

namespace AI
{

class CStateMachine;

// Event that can be raised when a state machine requests user state evaluation
typedef CCallback2<CStateMachine*, CState*>	EvaluateStateCallback;
typedef EvaluateStateCallback::Event		EvaluateStateEvent;


typedef System::SmartPointer<class CStateMachine> HStateMachine;
class CStateMachine : public System::CObject
{
public:
	WT_CLASS( CStateMachine , System::CObject )
	//TODO Add all derived interface base classes when supported in Type

	//
	// Constructors / destructor
	//

	CStateMachine();
	CStateMachine( const CStateMachine& Other );
	virtual ~CStateMachine();


	//
	// CStateMachine methods
	//

	// Adds a new state object to the collection with the given object identifier
	CState* AddState( const CStringOrID& ID );

	// Retrieves the state object within the collection with the given object identifier
	CState* GetState( const CStringOrID& ID ) const;

	// Pushes the sent state object to the top of the active state object stack
	// This allows historical state information to be embedded into the collection itself
	void PushState( CState* pState );

	// Retrieves the currently active state object from the collection (e.g. the top of the active state object stack)
	CState* GetCurrent() const;

	// Pops the top item of the active state object stack
	void PopState();

	// Clears the the active state object stack, effectively setting the active state object to NULL
	void ClearStateStack();

	// Associates the given identifier with the state machine
	void SetID( const CStringOrID& ID );

	// Retrieves the identifier associated with the state machine
	const CStringOrID& GetID() const;

	// Retrieves the collection of state objects associated with the state machine
	const std::list<CState*>& GetStates() const;

	// Associates arbitrary user data with the state machine
    void SetUserData( void* pUserData );

	// Retrieves any arbitrary user data associated with the state machine, or NULL if no
	// user data is associated with the state machine
    void* GetUserData() const;

	// Evaluates the current state in the state collection, firing user callbacks as needed
	void Evaluate();

	// Attempts to transition the state machine from one state to the next using the given message
	// This method returns true if the current state node was able to transition to the given state
	bool FireMessage( const CStringOrID& Message );

	// Attempts to transition the state machine from one state to the next using the given message
	// This method returns true if the current state node was able to transition to the given state
	bool FireMessage( int32 Message );

	// Attempts to transition the state machine from one state to the next using the given message
	// This method returns true if the current state node was able to transition to the given state
	bool FireMessage( const std::string& Message );

	// Retrieves the OnEvaluate event associated with the state machine
	// This event is raised during calls to Evaluate
	EvaluateStateEvent& OnEvaluate();
	
private:	// methods

	bool InternalFireMessage( const CStringOrID& QueryObject );

private:	// data

	// User properties
	CStringOrID					m_ID;
	std::list<CState*>			m_StateList;
	void*						m_pUserData;

	// Internal properties
	bool						m_InEvaluate;	// used to flag for recursive external calls to Evaluate
	CStringOrID					m_QueryObject;
	std::list<CState*>			m_ActiveStack;
	
private:	// events

	EvaluateStateEvent			m_EvaluateEvent;
};


} // namespace AI
} // namespace WT
