//
// Button.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Label.h"
#include "WT/Graphics/UI/ControlEvents.h"


namespace WT
{
namespace Graphics
{
namespace UI
{


class CButton : public CLabel
{
public:
	WT_CLASS( CButton, CLabel )

	CButton( IDirect3DDevice9* pDevice ) : CLabel( pDevice, -1, NULL )
	{
		m_Type = EControlType::Button;

		// Enable mouse button 1
		m_MouseButtons[EMouseButtonType::LButton] = true;
	}

	CButton( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent ) : CLabel( pDevice, ID, pParent )
	{
		m_Type = EControlType::Button;

		// Enable mouse button 1
		m_MouseButtons[EMouseButtonType::LButton] = true;
	}

	virtual ~CButton( )
	{
	}

	// Enables or disables mouse polling for the given mouse button
	// By default, EMouseButtonType::LButton is initially enabled; all other buttons are initially disabled
	virtual void SetMouseButtonEnable( EMouseButtonType::Type MouseButton, bool Enable )
	{
		m_MouseButtons[MouseButton] = Enable;
	}

	// Retrieves the whether mouse polling for the given mouse button is currently enabled
	virtual bool GetMouseButtonEnable( EMouseButtonType::Type MouseButton ) const
	{
		return m_MouseButtons[MouseButton];
	}

	// Retrieves the internal sprite object for finer-grained sprite attribute setting and placement
	virtual CSprite* GetSpriteObject( ) const
	{
		return m_pSprite;
	}

	// Retrieves the Click event associated with the mouse
	virtual MouseClickEvent& OnMouseClick( )
	{
		return m_ClickEvent;
	}

	
	// The following are for editor compatibility

	// Enables or disables mouse polling for the given mouse button
	// By default, EMouseButtonType::LButton is initally enabled; all other buttons are initially disabled
	void SetMouseButtonEnableEnum( uint32 MouseButton, bool Enable )
	{
		if ( MouseButton >= EMouseButtonType::EndOfTypes )
		{
			assert(false);
			return;
		}

		SetMouseButtonEnable( static_cast<EMouseButtonType::Type>( MouseButton ), Enable );
	}

	// Retrieves the whether mouse polling for the given mouse button is currently enabled
	bool GetMouseButtonEnableEnum( uint32 MouseButton ) const
	{
		if ( MouseButton >= EMouseButtonType::EndOfTypes )
		{
			assert(false);
			return false;
		}

		return GetMouseButtonEnable( static_cast<EMouseButtonType::Type>( MouseButton ) );
	}

	uint32 GetMouseButtonEnableEnumCount( ) const
	{
		return EMouseButtonType::EndOfTypes;
	}

	void SetMouseButtonEnableEnumCount( uint32 )
	{
		return;
	}

protected: // methods

	CButton() {}

	// Internal method called from HandleKeyboardEvent when a keyboard key is pressed
	// Note that HandleKeyboardEvent will filter out cases where the event should not be handled
	virtual bool OnKeyDown( uint8 Key )
	{
		if ( CControl::OnKeyDown(Key) )	// should this event be handled?
		{
			// Verify the hotkey was chosen
			if( m_Hotkey == Key )
			{
				// Update pressed state
				DoPressDown( );
				return true;
			}
		}

		return false;
	}

	// Internal method called from HandleKeyboardEvent when a keyboard key is released
	// Note that HandleKeyboardEvent will filter out cases where the event should not be handled
	virtual bool OnKeyUp( uint8 Key )
	{
		if ( CControl::OnKeyUp(Key) )		// should this event be handled?
		{
			// Verify the hotkey was chosen
			if( m_Hotkey == Key )
			{
				// Update pressed state
				// Note the DoPressUp will raise an event which requires a mouse button
				// Pass the null button to indicate the event was triggered from the keyboard
				DoPressUp( 0 );
				return true;
			}
		}

		return false;
	}

	// Performs actual rendering of the control with respect to the given camera object
	virtual bool RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
	{
		if (!Context.ShouldRender( *this ))
		{
			return true;
		}

		// Render the sprite image (if available), then overlay text caption
		if( m_pSprite != NULL )
		{
			m_pSprite->Render( pDevice, Context );
		}

		return CLabel::RenderInternal( pDevice, Context );
	}

	// Internal method called from HandleMouseEvent when a mouse button is pressed over the control
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	// Derived classes should override this method as needed 
	virtual bool OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos );

	// Updates all visible element areas
    virtual void UpdateLayout( );

	// Updates the button to be in the pressed state
	// This method is called when a button is pressed from either the keyboard hotkey
	// or a registered mouse button
	virtual void DoPressDown( );

	// Updates the button to be in the non-pressed state and raises the OnClick event
	// This method is called when a button is released from either the keyboard hotkey
	// or a registered mouse button
	virtual void DoPressUp( uint32 VirtualKeyMask )
	{
		if( m_Pressed )
		{
			m_Pressed = false;
			RaiseOnClickEvent( VirtualKeyMask );
		}
	}

	// Raises the OnClick event to registered callbacks
	virtual void RaiseOnClickEvent( uint32 VirtualKeyMask )
	{
		m_ClickEvent( this, VirtualKeyMask );
	}

protected: // events

	MouseClickEvent	m_ClickEvent;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
