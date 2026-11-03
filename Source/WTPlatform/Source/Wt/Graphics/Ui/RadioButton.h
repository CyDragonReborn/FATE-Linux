//
// RadioButton.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/CheckBox.h"
#include <d3d9.h>


namespace WT
{
namespace Graphics
{
namespace UI
{


class CRadioButton : public CCheckBox
{
public:
	
	WT_CLASS( CRadioButton, CCheckBox )

	CRadioButton();
	CRadioButton( IDirect3DDevice9* pDevice );
	CRadioButton( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent );
	CRadioButton( const CRadioButton& Other );
	virtual ~CRadioButton( );


	// Associates the given numeric group identifier with the radio button
	// Setting this value to -1 (which is also the default) will unassociate it with all other groups
	virtual void SetButtonGroup( uint32 ButtonGroup )
	{
		m_ButtonGroup = ButtonGroup;
	}

	// Retrieves the numeric group identifier associated with the radio button
	virtual uint32 GetButtonGroup( ) const
	{
		return m_ButtonGroup;
	}

protected: // methods

	// Internal method called from HandleMouseEvent when a mouse button is pressed over the control
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	// Derived classes should override this method as needed 
	virtual bool OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos );

	// Updates the button to be in the non-pressed state and raises the OnClick event
	// This method is called when a button is released from either the keyboard hotkey
	// or a registered mouse button
	virtual void DoPressUp( uint32 VirtualKeyMask )
	{
		CCheckBox::DoPressUp( VirtualKeyMask );

		// Ensure that the control is always checked, as we do not allow
		// checked radio buttons to "uncheck" themselves via hotkey
		OnChecked( );			
	}

	// Handles the radio button entering the 'checked' state
	void OnChecked( );

private: // data

	uint32	m_ButtonGroup;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
