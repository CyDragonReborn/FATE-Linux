//
// Label.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Label.h"

using namespace WT::Math;

namespace WT
{
namespace Graphics
{
namespace UI
{


CLabel::CLabel()
{
	m_Type = EControlType::Label;
	m_pFont = NULL;
	m_pText = NULL;
}

CLabel::CLabel( IDirect3DDevice9* pDevice ) : CControl( pDevice, -1, NULL )
{
	m_Type = EControlType::Label;
	m_pFont = NULL;
	m_pText = NULL;
}

CLabel::CLabel( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent ) : CControl( pDevice, ID, pParent )
{
	m_Type = EControlType::Label;
	m_pFont = NULL;
	m_pText = NULL;
}

CLabel::CLabel( const CLabel& Other ) : CControl( Other )
{
	this->m_pFont		= Other.m_pFont;			// copy pointer only; font resources are shared
	this->m_Text		= Other.m_Text;
	this->m_TextColor	= Other.m_TextColor;
	this->m_pText		= new CText( *(Other.m_pText) );
}

CLabel::~CLabel()
{
	SafeDelete( m_pText );
}


// Updates all visible element areas
void CLabel::UpdateLayout()
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

	// Forward centering, rotation to sub-objects
	if( m_pText != NULL )
	{
		m_pText->SetRotationCenter( m_CenterX, m_CenterY );
		m_pText->SetRotationZ( m_RotationZ );
		m_pText->SetPosition( X, Y );
	}
}

// Updates text properties related to the label
void CLabel::UpdateText( bool bSetText, bool bSetTextColor )
{
	if( m_pText == NULL )
	{
		if( m_pFont == NULL )
		{
			UpdateLayout( );
			return;
		}

		assert( m_pDevice != NULL );
		m_pText = new CText( m_pDevice, m_pFont );
		if( m_pText == NULL )
		{
			return;
		}

		m_pText->SetAutoSize( true );
		
		bSetText = true;
		bSetTextColor = true;	
	}

	if( bSetTextColor )
	{
		m_pText->SetColor( m_TextColor );
	}
	if( bSetText )
	{
		m_pText->SetText( m_Text );
	}

	UpdateLayout( );
}

// Internal method called from HandleMouseEvent when the mouse position has changed
// Note that HandleMouseEvent will filter out cases where the event should not be handled
// Derived classes should override this method as needed 
bool CLabel::OnMouseMove( const CVector2i& MousePos )
{
	bool const WasMouseOver = m_MouseOver;

	// Determine whether the mouse is currently over the control
	m_MouseOver = ContainsPoint( MousePos );

	// Potentially raise the OnLeave or OnEnter event if we've just entered
	// or just left the control
	if( !m_MouseOver )
	{
		if( WasMouseOver )
		{
			// Raise the OnLeave event
			RaiseMouseLeaveEvent( MousePos );
			return true;
		}
	}
	else 
	{ 
		if( !WasMouseOver )
		{
			// Raise the OnEnter event
			RaiseMouseEnterEvent( MousePos );
			return true; 
		}
	}

	return false;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
