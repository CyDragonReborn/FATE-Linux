//
// Cursor.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Cursor.h"

using namespace WT::Math;


namespace WT
{
namespace Graphics
{
namespace UI
{


CCursor::CCursor( )
	: m_Position( 0.0f, 0.0f )
	, m_HotSpot( 0.0f, 0.0f )
	, m_PrevPosition( 0.0f, 0.0f )
	, m_pCurrentSprite( NULL )
	, m_pMesh( NULL )
{
}

CCursor::~CCursor( )
{	
}


//
// Private Methods
//

// Determines whether the given rect contains the specified point
/*static*/ inline bool CCursor::RectContainsPoint( const CRect2f& Rect, const CVector2f& Point )
{
	// TODO : Review - Move this functionality to CRect2f class?
	return ( Point.x >= Rect.left && Point.x <= Rect.right && Point.y >= Rect.top && Point.y <= Rect.bottom );
}

// Retrieves either the OnEnterRect or OnLeaveRect event associated with the given rectangle
CursorFocusEvent& CCursor::OnRectEvent( const CRect2f& Rect, bool EnterEvent )
{
	RectEventMap::iterator it = m_RectEvents.find( Rect );
	if( it == m_RectEvents.end( ) )
	{
		m_RectEvents.insert( std::pair<CRect2f, CursorFocusState>( Rect, CursorFocusState() ) );
		it = m_RectEvents.find( Rect );
	}

	return EnterEvent ? it->second.EnterEvent : it->second.LeaveEvent;
}

// Raises all movement events associated with the cursor
void CCursor::RaiseEvents( const CVector2f& Position )
{
	if( m_MovedEvent.GetCount( ) == 0 && m_RectEvents.empty( ) )
	{
		return;
	}

	// Raise the OnMovedEvent to all registered callback listeners
	m_MovedEvent( this, Position );

	// Did the new position enter or leave any mapped rects?
	RectEventMap::iterator it = m_RectEvents.begin( );
	while( it != m_RectEvents.end() )
	{
		const CRect2f& Rect = it->first;
		CursorFocusState& FocusState = it->second;

		// Determine if the current rect contains the provided position
		const bool ContainsPoint = RectContainsPoint( Rect, Position );

		// Raise the OnEnterRect event if this is the first time we calcluated the point to 
		// be inside the rect
		if( ContainsPoint && !FocusState.Inside )
		{
			FocusState.EnterEvent( this, Rect );
			FocusState.Inside = true;
		}

		// Or raise the OnLeaveRect event if this is the first time we calcluated the point to 
		// have left the rect
		else if( !ContainsPoint && FocusState.Inside )
		{
			FocusState.LeaveEvent( this, Rect );
			FocusState.Inside = false;
		}

		++it;
	}
}

// Translates the given screen point to device coordinates, where device coordinates are defined
// to be within the range [0.0, 1.0]
/*static*/ inline void CCursor::TransformScreenToDevice( const CVector2f& ScreenPoint, const CRect2i& CameraRect, CVector3f& DevicePoint )
{
	const float32 InvWidth  = 1.0f / static_cast<float32>(CameraRect.right-CameraRect.left);
	const float32 InvHeight = 1.0f / static_cast<float32>(CameraRect.bottom-CameraRect.top);

	DevicePoint.x = ScreenPoint.x * InvWidth;
	DevicePoint.y = ScreenPoint.y * InvHeight;
	DevicePoint.z = 0.0f;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
