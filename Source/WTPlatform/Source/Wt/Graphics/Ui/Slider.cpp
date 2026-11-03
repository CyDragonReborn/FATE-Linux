//
// Slider.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Slider.h"
#include "WT/Graphics/UI/UIException.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace Math;


CSlider::CSlider()
	: m_Min( 0 )
	, m_Max( 100 )
	, m_Value( 50 )	
	, m_IncrementSet( false )
	, m_pButton( NULL )
{
	m_Type = EControlType::Slider;
}

CSlider::CSlider( IDirect3DDevice9* pDevice )
	: CControl( pDevice, -1, NULL )
	, m_Min( 0 )
	, m_Max( 100 )
	, m_Value( 50 )	
	, m_IncrementSet( false )
	, m_pButton( NULL )
{
	m_pButton = new CButton( pDevice, m_ID, this );
	if( m_pButton == NULL )
	{
		throw CUIException( "CSlider::CSlider - Out of memory creating Button object" );
	}
	InitInternal( );
	
	m_Type = EControlType::Slider;
}

CSlider::CSlider( IDirect3DDevice9* pDevice, int32 ID, float32 X, float32 Y, float32 Width, float32 Height, 
				CMaterialTextured* pMaterial, int32 Min, int32 Max, int32 Value, CControl* pParent )
	: CControl( pDevice, ID, pParent )
	, m_Min( 0 )
	, m_Max( 100 )
	, m_Value( 50 )	
	, m_IncrementSet( false )
	, m_pButton( NULL )
{
	SetPosition( X, Y );
	SetSize( Width, Height );
	SetMaterial( pMaterial );
	SetRange( Min, Max );
	SetValue( Value );

	m_pButton = new CButton( pDevice, m_ID, this );
	if( m_pButton == NULL )
	{
		throw CUIException( "CSlider::CSlider - Out of memory creating Button object" );
	}
	InitInternal( );

	m_Type = EControlType::Slider;
}

CSlider::CSlider( const CSlider& Other )
	: CControl( Other.m_pDevice, Other.m_ID, Other.m_pParent )	
	, m_Min( Other.m_Min )
	, m_Max( Other.m_Max )
	, m_Value( Other.m_Value )
	, m_pButton( NULL )
{	
	m_pButton = Other.m_pButton != NULL ? new CButton(*Other.m_pButton) : NULL;
	if( m_pButton == NULL )
	{
		throw CUIException( "CSlider::CSlider - Out of memory creating Button object" );
	}
	InitInternal( );

	m_Type = EControlType::Slider;

	UpdateLayout( );
}

CSlider::~CSlider( )
{
	SafeDelete( m_pButton );
}


bool CSlider::OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos )
{
	EMouseButtonType::Type ButtonType = CControl::GetMouseButtonType( VirtualKeyMask );

	if ( ButtonType == EMouseButtonType::None )
	{
		assert(false);
		return false;
	}

	if ( m_MouseButtons[ButtonType] )	// do we handle this button?
	{
		// Is the button pressed?
		if( ButtonPressed )
		{
			// If the mouse is pressed over the slider button, remember the offset 
			// from the button to the mouse position to prevent 'snapping' during layout
			if( m_pButton != NULL && m_pButton->ContainsPoint( MousePos ) )
			{
				CVector2f const ButtonPos = m_pButton->GetAbsolutePosition( );
				CVector2f const ButtonSz  = m_pButton->GetSize( );

				m_MouseOffset = CVector2f( 
					ButtonPos.x + (0.5f*ButtonSz.x) - static_cast<float32>( MousePos.x ), 
					0.0f );
			}

			// Early-out if the slider button has the mouse captured, as from this point
			// forward we're only concerned with the non-button client area
			if( m_pButton == NULL || m_pButton->HasMouseOver( ) )
			{
				return false;
			}

			// Trivial rejection: the slider does not contain the mouse position
			// Note we don't call 'this->ContainsPoint' since we're no longer interested
			// in the slider button
			if( !CControl::ContainsPoint( MousePos ) )
			{
				return false;
			}

			// We've established that the trackbar contains the button position
			// Increment or decrement the trackbar value by the current 'increment' 
			// value depending on whether the trackbar was clicked to the left or the right
			// of the button

			CVector2f const ButtonPos = m_pButton->GetAbsolutePosition( );

            int32 NewValue = m_Value;
			if( m_MousePos.x < ButtonPos.x )
			{
				NewValue -= GetIncrement( );
			}
			else
			{
				NewValue += GetIncrement( );
			}

			SetValueInternal( NewValue, true );

			return true;
		}			
	}

	return false;
}

bool CSlider::OnMouseMove( const CVector2i& MousePos )
{
	if( !ContainsPoint( MousePos.x, MousePos.y ) )
	{
		// Neither slider button nor trackbar contain the mouse position
		return false;
	}

	m_MousePos = MousePos;

	// Calculate the new slider value based on the button position relative to the trackbar layout 
	if( m_pButton != NULL && m_pButton->IsPressed( ) && m_pButton->HasMouseOver( ) )
	{
		float32 const Width = (m_BoundingBox.right-m_BoundingBox.left);
		float32 const XPos	= static_cast<float32>( m_MousePos.x ) - m_BoundingBox.left + m_MouseOffset.x;

		float32 const u = Width != 0.0f ? (XPos / Width) : 0.0f;
		int32 const NewValue = static_cast<int32>(u*(m_Max-m_Min));
		
		SetValueInternal( NewValue, true );

		return true;
	}

	return false;
}

void CSlider::UpdateLayout( )
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

	// TODO: Need to handle rotations

	float32 SpriteOffset = 0.0f;

	// Update sprite object
	if( m_pSprite != NULL )
	{
		SpriteOffset = m_pSprite->GetSize( ).x;

		m_pSprite->SetRotationCenter( m_CenterX, m_CenterY );
		m_pSprite->SetZRotation( m_RotationZ );

		// Set sprite position to upper-left of bounds
		m_pSprite->SetPosition( X, Y );

		m_pSprite->SetSize( m_Width, m_Height );
	}

	if( m_pButton != NULL )
	{
		float32 XOffset = -m_pButton->GetSize( ).x * 0.5f;
		XOffset += (float32)((m_Value - m_Min) * m_BoundingBox.Width( ) / (m_Max - m_Min) );
		m_pButton->SetPosition( (float32)(int32)XOffset, 0.0f );

		// Make button size square (height*height)
		m_pButton->SetSize( m_Height, m_Height );
	}
}

bool CSlider::RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if( m_pButton == NULL || m_pSprite == NULL )
	{
		// Nothing to render
		// This is bad, as it implies something failed to create
		assert(false);
		return false;
	}

	if (!Context.ShouldRender( *this ))
	{
		return true;
	}

	m_pSprite->Render( pDevice, Context );
	m_pButton->Render( pDevice, Context );

	return true;
}


} // namespace UI
} // namespace Graphics
} // namespace WT
