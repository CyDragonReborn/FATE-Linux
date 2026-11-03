//
// Cursor.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include <vector>
#include <map>
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/Callback.h"
#include "WT/Math/Math.h"
#include "WT/Mesh/Mesh.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/Sprite.h"

namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace WT::Math;
using namespace WT::System;

namespace
{
	// Functor for sorting CRect2f structures within a std::map
	struct Rect2fLess
	{
		// Returns true if the either component of the first rect's center is less than the second
		bool operator( )( const CRect2f& lRect, const CRect2f& rRect ) const
		{			
			const CVector2f lCenter( lRect.left+((lRect.right-lRect.left)*0.5f), lRect.top+((lRect.bottom-lRect.top)*0.5f) );
			const CVector2f rCenter( rRect.left+((rRect.right-rRect.left)*0.5f), rRect.top+((rRect.bottom-rRect.top)*0.5f) );
		
			return lCenter.x < rCenter.x || lCenter.y < rCenter.y;
		}
	};
}

class CCursor;

// Callback/event types for tracking cursor movement changes 
typedef CCallback2<CCursor*, const CVector2f&>	CursorPositionCallback;
typedef CursorPositionCallback::Event			CursorPositionEvent;

// Callback/event types for tracking when a cursor enters and leaves a given rectangle's bounds
typedef CCallback2<CCursor*, const CRect2f&>	CursorFocusCallback;
typedef CursorFocusCallback::Event				CursorFocusEvent;

typedef std::vector<CSprite *> SpriteCollection;

class CCursor : public System::CObject, public CRenderableImpl
{
public:

	WT_BEGIN_CLASS( CCursor, System::CObject );
		WT_INHERITS_INTERFACE( IRenderable );
	WT_END_CLASS();

	CCursor( );
	virtual ~CCursor( );

	// Sets the cursor's current position
	// All attached objects' (e.g. sprites or meshes) positions will be relative to this value
	virtual void SetPosition( const CVector2f& Position )
	{
		m_Position = Position;
	}

	// Retrieves the cursor's current position
	virtual CVector2f GetPosition( ) const
	{
		return m_Position;
	}

	// Sets the cursor's current hotspot position
	// This value is relative to the cursor's current position
	virtual void SetHotSpot( const CVector2f& HotSpot )
	{
		m_HotSpot = HotSpot;
	}

	// Retrieves the cursor's current hotspot position
	virtual CVector2f GetHotSpot( ) const
	{
		return m_HotSpot;
	}

	// Retrieves the current collection of sprites maintained by the cursor
	virtual SpriteCollection& GetSprites( )
	{
		return m_Sprites;
	}

	// Sets the currently active sprite representing the cursor image
	// The passed sprite does not need to be a member of the cursor's internal sprite collection
	// Pass NULL to invalidate the current sprite
	virtual void SetCurrentSprite( CSprite* pSprite )
	{
		m_pCurrentSprite = pSprite;
	}

	// Retrieves the currently active sprite representing the cursor image
	// This method returns NULL if no sprite is currently active
	virtual CSprite* GetCurrentSprite( ) const
	{
		return m_pCurrentSprite;
	}
	
	// Sets the current mesh that represents the cursor image
	// Pass NULL to invalidate the current mesh
	virtual void SetMesh( CMesh* pMesh )
	{
		m_pMesh = pMesh;

		// Update bounds
		if ( pMesh != NULL )
		{
			CMeshHelper::CalculateSphereBounds( *pMesh, m_MeshBounds );
		}
		else
		{
			m_MeshBounds = CSphere();
		}
	}

	// Gets the current mesh that represents the cursor image
	// This method returns NULL if no mesh is currently active
	virtual CMesh* GetMesh( ) const
	{
		return m_pMesh;
	}

	// Retrieves the CursorPositionEvent object responsible for raising OnCursorMoved events
	// This event is raised each time the cursor's position has changed
	virtual CursorPositionEvent& OnMoved( )
	{
		return m_MovedEvent;
	}

	// Retrieves the CursorFocusEvent object responsible for raising OnEnterRect events
	// This event is raised whenever the cursor enters the specified rect
	virtual CursorFocusEvent& OnEnterRect( const CRect2f& Rect )
	{
		return OnRectEvent( Rect, true );
	}

	// Retrieves the CursorFocusEvent object responsible for raising OnLeaveRect events
	// This event is raised whenever the cursor leaves the specified rect
	virtual CursorFocusEvent& OnLeaveRect( const CRect2f& Rect )
	{
		return OnRectEvent( Rect, false );
	}

	// Render the cursor from the perspective of the given camera
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );


private: // methods

	// Determines whether the given rect contains the specified point
	static inline bool RectContainsPoint( const CRect2f& Rect, const CVector2f& Point );

	// Retrieves either the OnEnterRect or OnLeaveRect event associated with the given rectangle
	CursorFocusEvent& OnRectEvent( const CRect2f& Rect, bool EnterEvent );

	// Raises all movement events associated with the cursor
	void RaiseEvents( const CVector2f& Position );

	// Translates the given screen point to device coordinates, where device coordinates are defined
	// to be within the range [0.0, 1.0]
	static inline void TransformScreenToDevice( const CVector2f& ScreenPoint, const CRect2i& CameraRect, CVector3f& DevicePoint );

private: // data
	
	SpriteCollection	m_Sprites;
	CSprite*			m_pCurrentSprite;

	CMesh*				m_pMesh;
	CSphere				m_MeshBounds;

	CVector2f			m_Position;			// current position
	CVector2f			m_PrevPosition;		// previously known (sprite + cursor) position 

	CVector2f			m_HotSpot;

private: // events

	// Structure for keeping track of when the cursor enters and leaves
	// a given rect
	struct CursorFocusState
	{
		CursorFocusState() : Inside( false )
		{
		}

		CursorFocusEvent	EnterEvent;		// OnEnterEvent
		CursorFocusEvent	LeaveEvent;		// OnLeaveEvent
		bool				Inside;			// flags whether the cursor is currently inside a given rect
	};

	typedef std::map<CRect2f, CursorFocusState, Rect2fLess> RectEventMap;
	RectEventMap			m_RectEvents;

	CursorPositionEvent		m_MovedEvent;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
