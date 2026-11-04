//
// Form.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Form.h"
#include "WT/Math/Math.h"
#include <windows.h>

using namespace WT::Math;


namespace WT
{
namespace Graphics
{
namespace UI
{


CForm::CForm()
	: m_pFocusControl( NULL )
	, m_pDefaultControl( NULL )
{
	m_Type = EControlType::Form;
}
	
CForm::CForm( IDirect3DDevice9* pDevice )
	: CControl( pDevice, -1, NULL )
	, m_pFocusControl( NULL )
	, m_pDefaultControl( NULL )
{
	m_Type = EControlType::Form;
}

CForm::CForm( const CForm& Other )
	: CControl( Other.m_pDevice, -1, NULL )
	, m_pFocusControl( NULL )
	, m_pDefaultControl( NULL )
{
	m_Type = EControlType::Form;

	SetFonts( Other.m_Fonts );
	SetTextures( Other.m_Textures );
}

CForm::~CForm( )
{
	ReleaseTextures();
}


// Requests focus activation of the given control
// This method will typically be called by the controls themselves
void CForm::RequestFocus( CControl* pControl )
{
	if( pControl == m_pFocusControl )
	{
		// Already focused
		return;
	}

	if( pControl != NULL )
	{
		if( !pControl->CanHaveFocus() )
		{
			// Can't have focus
			return;
		}
	}

    // Unfocus current focus control 
    if( m_pFocusControl != NULL )
	{
		m_pFocusControl->SetFocus( false );

		// Raise 'unfocus' event to registered callbacks
		RaiseOnFocusChanged( m_pFocusControl, false );
	}

    if( pControl != NULL )
	{
		// Set the control focus now
        pControl->SetFocus( true );

		// Raise 'focus' event to registered callbacks
		RaiseOnFocusChanged( pControl, true );
	}        

	m_pFocusControl = pControl;
}

// Cycles focus to the next control in the form's control collection
void CForm::CycleFocus( )
{
	if ( m_Controls.size() == 0 )
		return;

	// If there is a current focus control, attempt to retrieve the next one in the list,
	// wrapping around if necessary
	// Otherwise, the 'next' control will be the first control in the list
	uint32 NextControlIndex = 0; 
	if( m_pFocusControl != NULL )
	{
		ControlCollection::const_iterator it = m_Controls.begin();
		ControlCollection::const_iterator itend = m_Controls.end();
		while ( it != itend )
		{
			CControl* pControl = *it;
			if ( pControl == m_pFocusControl )
			{
				++NextControlIndex;
				break;
			}

			++NextControlIndex;
			++it;
		}

		if ( NextControlIndex >= static_cast<uint32>(m_Controls.size()) )
		{
			NextControlIndex = 0;
		}
	}

	RequestFocus( m_Controls[NextControlIndex] );
}

// Sets all radio buttons which are direct children of the form to the "unchecked" state
void CForm::ClearRadioButtonGroup( uint32 Group )
{
	// Trivial rejection: check for an empty control collection
	// or the -1 group (which means the button is not attached to any particular group)
	if ( m_Controls.size() == 0 || Group == 0xffffffff )
	{
		return;
	}

	// Enumerate through all direct children and reset the checked state of all
	// radio buttons we encounter from the requested button group
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl && pControl->GetControlType() == EControlType::RadioButton )
		{
			CRadioButton* pRadioButton = DynamicCast<CRadioButton>( pControl );
			assert( pRadioButton != NULL );

			if ( pRadioButton->GetButtonGroup() == Group )
			{
				pRadioButton->SetChecked( false );
			}
		}

		++it;
	}
}


//
// Protected methods
//

// Handles the given mouse event, raising user events as needed
// Based on Windows mouse event messages
bool CForm::HandleMouseEvent( uint32 Event, uint32 wParam, uint32 lParam )
{
	// Ignore the event if
	// 1. The control is hidden
	// 2. The control is disabled
	// 3. The control is flagged to not handle mouse events
	if (!GetVisible() || !m_Enabled || !UsingMouseEvents( ) )
	{
		return false;
	}

	// Find out what kind of mouse event this is and assemble mouse data
	EControlMouseEvent::Event MouseEvent = EControlMouseEvent::Unknown;
	uint32 VirtualKeyMask = wParam;
	CVector2i MousePos;
	bool ButtonPressed = false;

	switch ( Event )
	{
		case WM_MOUSEMOVE:
			MouseEvent = EControlMouseEvent::MouseMove;
			break;

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			MouseEvent = EControlMouseEvent::MouseButton;
			ButtonPressed = true;
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			if ( Event==WM_LBUTTONUP ) VirtualKeyMask = MK_LBUTTON;
			if ( Event==WM_MBUTTONUP ) VirtualKeyMask = MK_MBUTTON;
			if ( Event==WM_RBUTTONUP ) VirtualKeyMask = MK_RBUTTON;
			MouseEvent = EControlMouseEvent::MouseButton;
			break;

		default:
			return false;
	}

	MousePos = CVector2i( LOWORD(lParam), HIWORD(lParam) );

	// If a control is in focus, it belongs to this form, and it's enabled, then give
	// it the first chance at handling the message
    if( m_pFocusControl != NULL && 
        m_pFocusControl->GetParent( ) == this && 
		m_pFocusControl->GetEnabled( ) )
	{
		// If the control handles it, then we don't
		if( m_pFocusControl->HandleMouseEvent( MouseEvent, VirtualKeyMask, ButtonPressed, MousePos ) )
		{
			return true;
		}
	}

	// Query all containing elements for mouse handling before forwarding the call to the 
	// base implementation
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl )
		{
			if ( pControl->HandleMouseEvent( MouseEvent, VirtualKeyMask, ButtonPressed, MousePos ) )
			{
				return true;
			}
		}
		++it;
	}

	return CControl::HandleMouseEvent( MouseEvent, VirtualKeyMask, ButtonPressed, MousePos );
}

// Handles the given keyboard event, raising user events as needed
// Based on Windows mouse event messages
bool CForm::HandleKeyboardEvent( uint32 Event, uint32 wParam, uint32 lParam )
{
	// Ignore the event if
	// 1. The control is hidden
	// 2. The control is disabled
	// 3. The control is flagged to not handle keyboard events
	if (!GetVisible() || !m_Enabled || !UsingKeyboardEvents( ))
	{
		return false;
	}

	// Find out what kind of keyboard event this is and assembly key data
	EControlKeyState::State KeyState;
	uint8 Key = static_cast<uint8>( wParam );
	switch ( Event )
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		KeyState = EControlKeyState::KeyDown;
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		KeyState = EControlKeyState::KeyUp;
		break;

	default:
		return false;
	}

	// If a control is in focus, it belongs to this form, and it's enabled, then give
	// it the first chance at handling the message.
    if( m_pFocusControl != NULL && 
        m_pFocusControl->GetParent( ) == this && 
		m_pFocusControl->GetEnabled( ) )
	{
		// If the control handles it, then we don't
		if( m_pFocusControl->HandleKeyboardEvent( KeyState, Key ) )
		{
			return true;
		}
	}

	// Query all containing elements for mouse handling before forwarding the call to the 
	// base implementation
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl )
		{
			if ( pControl->HandleKeyboardEvent( KeyState, Key ) )
			{
				return true;
			}
		}
		++it;
	}

	return CControl::HandleKeyboardEvent( KeyState, Key );
}

// Performs actual rendering of the control with respect to the given camera object
bool CForm::RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return true;
	}

	// Render the background sprite if we have one
	if( m_pSprite != NULL )
	{
		m_pSprite->Render( pDevice, Context );
	}
	
	return true;	// base class will handle child rendering
}

// Updates all visible element areas	
void CForm::UpdateLayout( )
{
	CControl::UpdateLayout( );

	// If the bounding box is still 0 in size, defer the layout until later
	if( m_BoundingBox.Width() == 0.0f || m_BoundingBox.Height() == 0.0f )
	{
		m_LayoutDirty = true;
		return;
	}

	CVector2f const AbsPosition = GetAbsolutePosition( );
	float32 const& X = AbsPosition.x;
	float32 const& Y = AbsPosition.y;

	// Update sprite object to fill the form's bounds
	if( m_pSprite != NULL )
	{
		m_pSprite->SetRotationCenter( m_CenterX, m_CenterY );
		m_pSprite->SetZRotation( m_RotationZ );
		m_pSprite->SetPosition( X, Y );
		m_pSprite->SetSize( m_Width, m_Height );
	}
}


//
// Methods that operate on the ControlCollection
//

// Retrieves the first control in the collection with the given numeric identifier
CControl* CForm::GetControl( int32 ID ) const
{
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl )
		{
			if ( pControl->GetID() == ID )
			{
				return pControl;
			}
		}
		++it;
	}

	return NULL;
}

// Retrieves the first control in the collection with the given numeric identifier and type
CControl* CForm::GetControl( int32 ID, EControlType::Type ControlType ) const
{
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl )
		{
			if ( pControl->GetControlType() == ControlType && pControl->GetID() == ID )
			{
				return pControl;
			}
		}
		++it;
	}

	return NULL;
}

// Retrieves the control in the collection at the given screen coordinates
CControl* CForm::GetControlAtPoint( int32 X, int32 Y ) const
{
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl )
		{
			if ( pControl->ContainsPoint( X, Y ) )
			{
				return pControl;
			}
		}
		++it;
	}

	return NULL;
}

// Helper method for setting the enabled state of a control
void CForm::SetControlEnabled( int32 ID, bool Enabled )
{
	CControl* pControl = GetControl( ID );
	if ( pControl )
	{
		pControl->SetEnabled( Enabled );
	}
}

// Helper method for retrieving the enabled state of a control
bool CForm::GetControlEnabled( int32 ID ) const
{
	CControl* pControl = GetControl( ID );
	if ( pControl )
	{
		return pControl->GetEnabled();
	}

	return false;
}

// Removes the control with the given numeric identifier from the collection
void CForm::RemoveControl( int32 ID )
{
	ControlCollection::iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = *it;
		if ( pControl )
		{
			if ( pControl->GetID() == ID )
			{
				m_Controls.erase( it );
				break;
			}
		}
		++it;
	}
}

// Helper methods to create specific controls.  This form object is set as the parent of the created controls.

CLabel* CForm::CreateLabel( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, 
						    float32 Width, float32 Height, uint32 TextColor, bool IsDefault )
{
	try
	{
		CLabel* pNewLabel = new CLabel( m_pDevice, ID, this );
		if ( !pNewLabel )
		{
			return NULL;
		}

		pNewLabel->SetTextAttributes( pFont, Text, TextColor );
		pNewLabel->SetPosition( X, Y );
		pNewLabel->SetSize( Width, Height );

		if ( IsDefault )
		{
			SetDefaultControl( pNewLabel );
		}

		return pNewLabel;
	}
	catch(...)
	{
		assert(false);
		return NULL;
	}
}

CButton* CForm::CreateButton( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y,
						     float32 Width, float32 Height, CMaterialTextured* pMaterial, uint8 Hotkey, 
							 bool IsDefault )
{
	try
	{
		CButton* pNewButton = new CButton( m_pDevice, ID, this );
		if ( !pNewButton )
		{
			return NULL;
		}

		pNewButton->SetFont( pFont );
		pNewButton->SetText( Text );

		pNewButton->SetMaterial( pMaterial );

		pNewButton->SetPosition( X, Y );
		pNewButton->SetSize( Width, Height );
		pNewButton->SetHotkey( Hotkey );

		if ( IsDefault )
		{
			SetDefaultControl( pNewButton );
		}

		return pNewButton;
	}
	catch(...)
	{
		assert(false);
		return NULL;
	}
}

CCheckBox* CForm::CreateCheckBox( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, float32 Width, 
								 float32 Height, CMaterialTextured* pMaterial, bool Checked, uint8 Hotkey, 
								 bool IsDefault )
{
	try
	{
		CCheckBox* pNewCheckBox = new CCheckBox( m_pDevice, ID, this );
		if ( !pNewCheckBox )
		{
			return NULL;
		}

		pNewCheckBox->SetFont( pFont );
		pNewCheckBox->SetText( Text );

		pNewCheckBox->SetMaterial( pMaterial );

		pNewCheckBox->SetChecked( Checked );
		pNewCheckBox->SetHotkey( Hotkey );
		pNewCheckBox->SetPosition( X, Y );
		pNewCheckBox->SetSize( Width, Height );

		if ( IsDefault )
		{
			SetDefaultControl( pNewCheckBox );
		}

		return pNewCheckBox;
	}
	catch(...)
	{
		assert(false);
		return NULL;
	}
}

CRadioButton* CForm::CreateRadioButton( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, float32 Width, 
									  float32 Height, CMaterialTextured* pMaterial, uint32 ButtonGroup, bool Checked, 
									  uint8 Hotkey, bool IsDefault )
{
	try
	{
		CRadioButton* pNewRadioButton = new CRadioButton( m_pDevice, ID, this );
		if ( !pNewRadioButton )
		{
			return NULL;
		}

		pNewRadioButton->SetFont( pFont );
		pNewRadioButton->SetText( Text );

		pNewRadioButton->SetMaterial( pMaterial );

		pNewRadioButton->SetChecked( Checked );
		pNewRadioButton->SetHotkey( Hotkey );
		pNewRadioButton->SetPosition( X, Y );
		pNewRadioButton->SetSize( Width, Height );
		pNewRadioButton->SetButtonGroup( ButtonGroup );

		if ( IsDefault )
		{
			SetDefaultControl( pNewRadioButton );
		}

		return pNewRadioButton;
	}
	catch(...)
	{
		assert(false);
		return NULL;
	}
}

CSlider* CForm::CreateSlider( int32 ID, float32 X, float32 Y, float32 Width, float32 Height, 
							  CMaterialTextured* pMaterial, int32 Min, int32 Max, int32 Value, bool IsDefault )
{
	try
	{
		CSlider* pNewSlider = new CSlider( m_pDevice, ID, X, Y, Width, Height, pMaterial, Min, Max, Value, this );
		if ( !pNewSlider )
		{
			return NULL;
		}

		if ( IsDefault )
		{
			SetDefaultControl( pNewSlider );
		}

		return pNewSlider;
	}
	catch(...)
	{
		assert(false);
		return NULL;
	}
}

CForm* CForm::CreateForm( int32 ID, float32 X, float32 Y, float32 Width, float32 Height,
				  CMaterialTextured* pMaterial )
{
	CForm* pForm = new CForm( m_pDevice );
	pForm->SetParent( this );
	pForm->SetID( ID );
	pForm->SetPosition( X, Y );
	pForm->SetSize( Width, Height );
	pForm->SetMaterial( pMaterial );	

	return pForm;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
