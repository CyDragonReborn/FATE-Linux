/*
	State.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#include "WT/stdafx.h"
#include "WT/AI/State.h"
#include "WT/System/SystemException.h"

using namespace WT::System;


namespace WT
{
namespace AI
{

WT_DEFINE_CLASS(CState)

//
// Constructors / destructor
//

CState::CState()
	: m_ID()
	, m_pUserData( NULL )
{
}

CState::CState( const CState& Other )
{
	// Copy ID
	m_ID = Other.m_ID;

	// Copy user data pointer
	m_pUserData = Other.m_pUserData;

	// Copy transitions collection
	std::copy( Other.m_Transitions.begin(), Other.m_Transitions.end(), m_Transitions.begin() );
	assert( Other.m_Transitions.size() == m_Transitions.size() );

	// Copy callbacks
	uint32 Count = 0;
	StateChangedCallback** ppCallbacks = NULL;

	// Enter events
	Count = Other.m_EnterStateEvent.GetCount();
	ppCallbacks = Other.m_EnterStateEvent.GetCallbacks();
	for( uint32 i = 0; i < Count; ++i )
	{
		this->m_EnterStateEvent.Add( *ppCallbacks );
		++ppCallbacks;
	}

	// Leave events
	Count = Other.m_LeaveStateEvent.GetCount();
	ppCallbacks = Other.m_LeaveStateEvent.GetCallbacks();
	for( uint32 i = 0; i < Count; ++i )
	{
		this->m_LeaveStateEvent.Add( *ppCallbacks );
		++ppCallbacks;
	}
}

CState::CState( int32 ID )
	: m_ID( ID )
	, m_pUserData( NULL )
{
}

CState::CState( int32 ID, const std::string& Name )
	: m_ID( ID, Name )
	, m_pUserData( NULL )
{
}

CState::CState( const CStringOrID& ID )
	: m_ID( ID )
	, m_pUserData( NULL )
{
}

CState::~CState()
{
}

// Associates the given identifier with the state object
void CState::SetID( const CStringOrID& ID )
{
	m_ID = ID;
}

// Retrieves the identifier associated with the state object
const CStringOrID& CState::GetID() const
{
	return m_ID;
}

// Retrieves the set of the state transition objects this state is capable of moving to
std::list<CStringOrID>& CState::GetTransitions()
{
	return m_Transitions;
}

// Adds the given transition identifier to the back of the transition list
void CState::AddTransition( const CStringOrID& TransitionID )
{
	m_Transitions.push_back( TransitionID );
}

// Associates arbitrary user data with the state
void CState::SetUserData( void* pUserData )
{
	m_pUserData = pUserData;
}

// Retrieves any arbitrary user data associated with the state, or NULL if no
// user data is associated with the state
void* CState::GetUserData() const
{
	return m_pUserData;
}

// Notifies the state object that its logical state has either been entered or exited, which
// may trigger user events to be raised
void CState::NotifyEvent( bool Enter )
{
	if( Enter )
	{
		m_EnterStateEvent( this );
	}
	else
	{
		m_LeaveStateEvent( this );
	}
}

// Retrieves the OnEnterState event associated with the state object object
// This event is raised when the state object is activated as the current object
StateChangedEvent& CState::OnEnterState()
{
	return m_EnterStateEvent;
}

// Retrieves the OnLeaveState event associated with the state object object
// This event is raised when the state object is deactivated from being the current object
StateChangedEvent& CState::OnLeaveState()
{
	return m_LeaveStateEvent;
}

} // namespace AI
} // namespace WT
