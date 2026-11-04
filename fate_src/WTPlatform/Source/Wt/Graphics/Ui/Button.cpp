//
// Button.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Button.h"
#include "WT/Graphics/UI/Form.h"
#include "WT/Math/Math.h"

using namespace WT::Math;


namespace WT
{
namespace Graphics
{
namespace UI
{


bool CButton::OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos )
{
	EMouseButtonType::Type ButtonType = CControl::GetMouseButtonType( VirtualKeyMask );

	if ( ButtonType == EMouseButtonType::None )
	{
		assert( false );
		return false;
	}

	if ( m_MouseButtons[ButtonType] )	// do we handle this button?
	{
		// Is the button pressed?
		if ( ButtonPressed )
		{
			if ( ContainsPoint( MousePos ) )
			{
				// Update pressed state
				DoPressDown();
				return true;
			}
		}
		else
		{
			if ( m_Pressed && m_MouseOver )
			{
				DoPressUp( VirtualKeyMask );
				// Reset pressed state
				m_Pressed = false;
				return true;
			}

			// Reset pressed state
			m_Pressed = false;
			return false;
		}
	}

	return false;
}

void CButton::UpdateLayout( )
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

	// Update the sprite object to fit our current dimensions
	// Then update the text object to be centered within the button

	// TODO: Rotations are probably not handled correctly 

	if( m_pSprite != NULL )
	{
		m_pSprite->SetRotationCenter( m_CenterX, m_CenterY );
		m_pSprite->SetZRotation( m_RotationZ );
		m_pSprite->SetPosition( X, Y );
		m_pSprite->SetSize( m_Width, m_Height );
	}

	if( m_pText != NULL )
	{
		m_pText->SetRotationCenter( m_CenterX, m_CenterY );
		m_pText->SetRotationZ( m_RotationZ );

		CVector2f const TextSize = m_pText->GetSize( );
		float32 const XOffset = 0.5f * (m_Width - TextSize.x);
		float32 const YOffset = 0.5f * (m_Height- TextSize.y);
		m_pText->SetPosition( (float32)(int32)(X + XOffset), (float32)(int32)(Y + YOffset) );
	}
}

void CButton::DoPressDown( )
{
	m_Pressed = true;

	// Attempt to obtain focus from a parent form
	if( !m_HasFocus )
	{
		CControl* pParent = m_pParent;
		while( pParent != NULL )
		{
			CForm* pForm = DynamicCast<CForm>( pParent );
			if( pForm != NULL )
			{
				pForm->RequestFocus( this );
				break;
			}

			pParent = pParent->GetParent( );
		}
	}					
}


} // namespace UI
} // namespace Graphics
} // namespace WT
