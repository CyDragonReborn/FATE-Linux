/*
	StateMachine.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#include "WT/stdafx.h"
#include "WT/AI/StateMachine.h"
#include "WT/System/SystemException.h"

using namespace WT::System;


namespace WT
{
namespace AI
{

WT_DEFINE_CLASS(CStateMachine)

//
// Constructors / destructor
//

CStateMachine::CStateMachine()
	: m_ID()
	, m_InEvaluate( false )
	, m_QueryObject()
	, m_pUserData( NULL )
{
}

CStateMachine::CStateMachine( const CStateMachine& Other )
	: m_InEvaluate( Other.m_InEvaluate )
	, m_QueryObject()
	, m_pUserData( Other.m_pUserData )
{
	m_ID = Other.m_ID;

	std::copy( Other.m_StateList.begin(), Other.m_StateList.end(), m_StateList.begin() );
	assert( Other.m_StateList.size() == m_StateList.size() );

	// Copy callbacks
	uint32 Count = 0;
	EvaluateStateCallback** ppCallbacks = NULL;

	// EvaluateState Event
	Count = Other.m_EvaluateEvent.GetCount();
	ppCallbacks = Other.m_EvaluateEvent.GetCallbacks();
	for( uint32 i = 0; i < Count; ++i )
	{
		this->m_EvaluateEvent.Add( *ppCallbacks );
		++ppCallbacks;
	}
}

CStateMachine::~CStateMachine()
{
	std::list<CState*>::iterator it = m_StateList.begin();
	while( it != m_StateList.end() )
	{
		CState* pState = *it;
		delete pState;
		++it;
	}
}


//
// CStateMachine methods
//

// Adds a new state object to the collection with the given object identifier
CState* CStateMachine::AddState( const CStringOrID& ID )
{
	CState* pState = new CState( ID );
	m_StateList.push_back( pState );
	return pState;
}

// Retrieves the state object within the collection with the given object identifier
CState* CStateMachine::GetState( const CStringOrID& ID ) const
{
	std::list<CState*>::const_iterator it = m_StateList.begin();
    while( it != m_StateList.end() )
	{
		CState* pState = *it;
		if( pState != NULL && pState->GetID() == ID )
		{
			return pState;
		}
		++it;
	}
	return NULL;
}

// Pushes the sent state object to the top of the active state object stack
// This allows historical state information to be embedded into the collection itself
void CStateMachine::PushState( CState* pState )
{
	if( pState != NULL )
	{
		// Notify new current state it is being entered
		pState->NotifyEvent( true );
		m_ActiveStack.push_back( pState );
	}
}

// Retrieves the currently active state object from the collection (e.g. the top of the active state object stack)
CState* CStateMachine::GetCurrent() const
{
	if( ! m_ActiveStack.empty() )
	{
		return m_ActiveStack.back();
	}
	return NULL;
}

// Pops the top item of the active state object stack
void CStateMachine::PopState()
{
	if( ! m_ActiveStack.empty() )
	{
		// Notify current state it is being exited
		m_ActiveStack.back()->NotifyEvent( false );
		m_ActiveStack.pop_back();
	}
}

// Clears the the active state object stack, effectively setting the active state object to NULL
void CStateMachine::ClearStateStack()
{
	m_ActiveStack.clear();
}

// Associates the given identifier with the state machine
void CStateMachine::SetID( const CStringOrID& ID )
{
	m_ID = ID;
}

// Retrieves the identifier associated with the state machine
const CStringOrID& CStateMachine::GetID() const
{
	return m_ID;
}

// Retrieves the collection of state objects associated with the state machine
const std::list<CState*>& CStateMachine::GetStates() const
{
	return m_StateList;
}

// Associates arbitrary user data with the state machine
void CStateMachine::SetUserData( void* pUserData )
{
	m_pUserData = pUserData;
}

// Retrieves any arbitrary user data associated with the state machine, or NULL if no
// user data is associated with the state machine
void* CStateMachine::GetUserData() const
{
	return m_pUserData;
}

// Evaluates the current state in the state collection, firing user callbacks as needed
void CStateMachine::Evaluate()
{
	// Don't allow recursive evaluations
	if( m_InEvaluate )
	{
		return;
	}

	m_InEvaluate = true;

	m_EvaluateEvent( this, GetCurrent() );

	m_InEvaluate = false;
}

// Attempts to transition the state machine from one state to the next using the given message
// This method returns true if the current state node was able to transition to the given state
bool CStateMachine::FireMessage( const CStringOrID& Message )
{
	m_QueryObject.Invalidate();

	if( Message.HasValidID() )
	{
		m_QueryObject.SetID( Message.GetID() );
	}
	if( Message.HasValidString() )
	{
		m_QueryObject.SetString( Message.GetString() );
	}

	return InternalFireMessage( m_QueryObject );
}

// Attempts to transition the state machine from one state to the next using the given message
// This method returns true if the current state node was able to transition to the given state
bool CStateMachine::FireMessage( int32 Message )
{
	m_QueryObject.Invalidate();
	m_QueryObject.SetID( Message );

	return InternalFireMessage( m_QueryObject );
}

// Attempts to transition the state machine from one state to the next using the given message
// This method returns true if the current state node was able to transition to the given state
bool CStateMachine::FireMessage( const std::string& Message )
{
	m_QueryObject.Invalidate();
	m_QueryObject.SetString( Message );

	return InternalFireMessage( m_QueryObject );
}

// Retrieves the OnEvaluate event associated with the state machine
// This event is raised during calls to Evaluate
EvaluateStateEvent& CStateMachine::OnEvaluate()
{
	return m_EvaluateEvent;
}

bool CStateMachine::InternalFireMessage( const CStringOrID& QueryObject )
{
    CState* pCurrent = GetCurrent();
	if( pCurrent == NULL )
	{
		// There is no active state to send the message to
		return false;
	}

	// Attempt to find a transition state matching the sent identifiers				
	std::list<CStringOrID>& Transitions = pCurrent->GetTransitions();
	std::list<CStringOrID>::iterator id_it = Transitions.begin();
	bool ContainsTransition = false;
	while( id_it != Transitions.end() )
	{
		CStringOrID& ID = *id_it;
		if( ID == QueryObject )
		{
			ContainsTransition = true;
			break;
		}
		++id_it;
	}
	if( ! ContainsTransition )
	{
		// Current state cannot transition to query state
		return false;
	}

	CState* pTargetState = GetState( QueryObject );
	if( pTargetState == NULL )
	{
		// Could not find a state matching the given identifier
		return false;
	}

	// Pop the previous state and push the new state onto the state collection stack
	PopState();
	PushState( pTargetState );

	// Evaluate the newly activated node
	// Note it is safe to reset the InEvaluate flag because we are controlling it internally
	// The InEvaluate flag is to protect from external calls to Evaluate
	m_InEvaluate = false;
	Evaluate();

	return true;
}


} // namespace AI
} // namespace WT
