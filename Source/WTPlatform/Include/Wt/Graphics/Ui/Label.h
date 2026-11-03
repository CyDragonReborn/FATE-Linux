//
// Label.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Control.h"
#include "WT/Graphics/UI/Font.h"
#include "WT/Graphics/UI/Text.h"
#include "WT/Math/Math.h"
#include <d3d9.h>


namespace WT
{
namespace Graphics
{
namespace UI
{


class CLabel : public CControl
{
public:

	WT_CLASS( CLabel, CControl )

	CLabel();
	CLabel( IDirect3DDevice9* pDevice );
	CLabel( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent );
	CLabel( const CLabel& Other );
	virtual ~CLabel();


	// Determines whether the control implementation is using visual sprite elements
	virtual bool UsingSpriteElements( )
	{
		// Labels contain only text; sprite elements are not used
		return false;
	}

	// Associates the label's internal text object with the given font
	virtual void SetFont( CFont* pFont )
	{
		m_pFont = pFont;
		SafeDelete( m_pText );		// font has changed, so text must be rebuilt

		UpdateText( true, true );
	}

	// Retrieves the font associated with the the label's internal text object
	virtual CFont* GetFont( ) const
	{
		return m_pFont;
	}

	// Sets the label's current caption
	virtual void SetText( const wstring& Text )
	{
		m_Text = Text;
		UpdateText( true, false );
	}

	// Retrieves the label's current caption
	virtual wstring GetText( ) const
	{
		return m_Text;
	}

	// Sets the drawing color of the label's caption
	virtual void SetTextColor( uint32 TextColor )
	{
		m_TextColor = TextColor;
		UpdateText( false, true );
	}

	// Retrieves the drawing color of the label's caption
	virtual uint32 GetTextColor( ) const
	{
		return m_TextColor;
	}

	// Helper method for associating a font, caption, and drawing color with a label
	virtual void SetTextAttributes( CFont* pFont, const wstring& Text, uint32 TextColor )
	{
		bool bSetText  = false;
		bool bSetColor = false;

		if( pFont != m_pFont )
		{
			m_pFont = pFont;

			SafeDelete( m_pText );	// font has changed, so text must be rebuilt

			bSetText  = true;		// font has changed, so all properties must be set
			bSetColor = true;
		}

		if( Text != m_Text )
		{
			m_Text = Text;
			bSetText = true;
		}
		if( TextColor != m_TextColor )
		{
			m_TextColor = TextColor;
			bSetColor = true;
		}

		UpdateText( bSetText, bSetColor );
	}

	// Retrieves the internal text object for finer-grained text attribute setting and placement
	virtual CText* GetTextObject( ) const
	{
		return m_pText;
	}

	// Retrieves the Enter event associated with the mouse
	virtual MouseMoveEvent& OnMouseEnter( )
	{
		return m_EnterEvent;
	}

	// Retrieves the Leave event associated with the mouse
	virtual MouseMoveEvent& OnMouseLeave( )
	{
		return m_LeaveEvent;
	}

protected: // methods

	// Render the object from the perspective of the given device
	virtual bool RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
	{
		if (!Context.ShouldRender( *this ))
		{
			return true;
		}

		if( m_pText != NULL )
		{
			m_pText->Render( pDevice, Context );
		}

		return true;
	}

	// Updates all visible element areas
    virtual void UpdateLayout();

	// Updates text properties related to the label
	virtual void UpdateText( bool bSetText, bool bSetTextColor );

	// Internal method called from HandleMouseEvent when the mouse position has changed
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	// Derived classes should override this method as needed 
	virtual bool OnMouseMove( const CVector2i& MousePos );

	virtual bool OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos )
	{
		VirtualKeyMask;
		ButtonPressed;
		MousePos;

		// Labels don't currently respond to mouse clicks
		return false;
	}

	// Raises the OnEnter event to registered callbacks
	virtual void RaiseMouseEnterEvent( const CVector2i& MousePos ) 
	{
		m_EnterEvent( this, MousePos );
	}

	// Raises the OnLeave event to registered callbacks
	virtual void RaiseMouseLeaveEvent( const CVector2i& MousePos )
	{
		m_LeaveEvent( this, MousePos );
	}

protected: // data

	// User properties
	CFont*				m_pFont;
	wstring				m_Text;
	uint32				m_TextColor;
	
	// Internal properties
	CText*				m_pText;		// built at runtime using above user properties

protected: // events

	MouseMoveEvent		m_EnterEvent;
	MouseMoveEvent		m_LeaveEvent;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
