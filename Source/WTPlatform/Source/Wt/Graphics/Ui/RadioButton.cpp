//
// RadioButton.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/RadioButton.h"
#include "WT/Graphics/UI/Form.h"


namespace WT
{
namespace Graphics
{
namespace UI
{


CRadioButton::CRadioButton()
{
	m_ButtonGroup = 0xffffffff;
	m_Type = EControlType::RadioButton;
}

CRadioButton::CRadioButton( IDirect3DDevice9* pDevice )
	: CCheckBox( pDevice, -1, NULL )
	, m_ButtonGroup( 0xffffffff )
{
	m_Type = EControlType::RadioButton;
}

CRadioButton::CRadioButton( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent )
	: CCheckBox( pDevice, ID, pParent )
	, m_ButtonGroup( 0xffffffff )
{
	m_Type = EControlType::RadioButton;
}

CRadioButton::CRadioButton( const CRadioButton& Other )
	: CCheckBox( Other.m_pDevice, Other.m_ID, Other.m_pParent )
{
	m_Type = EControlType::RadioButton;

	this->m_pText	= Other.m_pText;
	this->m_pSprite	= Other.m_pSprite;
	this->m_ButtonGroup = Other.m_ButtonGroup;
}

CRadioButton::~CRadioButton( )
{	
}


bool CRadioButton::OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos )
{
	if( m_Checked ) 
	{
		// Don't allow 'unchecking' from mouse input
		return false;
	}

	// Check whether the base implemenation handled the message
	if( CButton::OnMouseButton( VirtualKeyMask, ButtonPressed, MousePos ) )
	{
		// Did we become checked?
		if( m_Checked )
		{
			OnChecked( );
		}
		return true;
	}

	return false;
}

void CRadioButton::OnChecked( )
{
	// Inform the parent form of the change by having it clear this button's group
	// Note we may need to propagate upward in order to find the parent form
	CControl* pParent = GetParent( );
	while( pParent != NULL )
	{
		CForm* pParentForm = DynamicCast<CForm>( pParent );
		if( pParentForm != NULL )
		{
			pParentForm->ClearRadioButtonGroup( m_ButtonGroup );
			break;
		}
		pParent = pParent->GetParent( );
	}

	// Re-set the Checked flag, as the above call to ClearRadioButtonGroup will have cleared it
	m_Checked = true;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
