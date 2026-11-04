//
// Control.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/UI/Control.h"
#include "WT/Graphics/UI/ControlBehavior.h"

#include "WT/Graphics/UI/Label.h"
#include "WT/Graphics/UI/Button.h"
#include "WT/Graphics/UI/CheckBox.h"
#include "WT/Graphics/UI/RadioButton.h"
#include "WT/Graphics/UI/Slider.h"
#include "WT/Graphics/UI/Form.h"

using namespace WT::Math;


namespace WT
{
namespace Graphics
{
namespace UI
{


// Handles the given keyboard event, raising user events as needed
bool CControl::HandleKeyboardEvent( EControlKeyState::State KeyState, uint8 Key )
{
	// Ignore the event if
	// 1. This is not the focused control
	// 2. The control is hidden
	// 3. The control is disabled
	// 4. The control is flagged to not handle keyboard events
	if (!m_HasFocus || !GetVisible() || !m_Enabled || !UsingKeyboardEvents())
	{
		return false;
	}

	// Fire either a key pressed or key released event
	switch ( KeyState )
	{
	case EControlKeyState::KeyDown:
		return OnKeyDown( Key );

	case EControlKeyState::KeyUp:
		return OnKeyUp( Key );

	default:
		assert( false );
		return false;
	}
}

// Handles the given mouse event, raising user events as needed
bool CControl::HandleMouseEvent( EControlMouseEvent::Event MouseEvent, uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos )
{
	// Ignore the event if
	// 1. The control is hidden
	// 2. The control is disabled
	// 3. The control is flagged to not handle mouse events
	if (!GetVisible() || ! m_Enabled || ! UsingMouseEvents( ))
	{
		return false;
	}

	switch ( MouseEvent )
	{
	case EControlMouseEvent::MouseMove:
		return OnMouseMove( MousePos );

	case EControlMouseEvent::MouseButton:
		return OnMouseButton( VirtualKeyMask, ButtonPressed, MousePos );

	default:
		assert( false );
		return false;
	}
}


void CControl::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (pDevice == NULL)
	{
		assert(0);
		return;
	}

	assert( pDevice == m_pDevice );

	if (!Context.ShouldRender( *this ))
	{
		return;
	}

	// Prepare the control for rendering
	PreRender( pDevice );

	// Render the control
	// Note we take different rendering paths depending on whether there is an effect attached to the control
	bool Result = m_pEffect != NULL ? RenderEffectInternal( pDevice, Context ) : RenderInternal( pDevice, Context );
	if ( !Result )
	{					
		return;
	}

	// Restore control state
	PostRender( pDevice );

	// Render children
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = (*it);
		if ( pControl )
		{
			pControl->Render( pDevice, Context );
		}
		++it;
	}
}

template<typename T>
static T* GetControlHelper( const ControlCollection& Controls, int32 ID )
{
	ControlCollection::const_iterator it = Controls.begin();
	while( it != Controls.end() )
	{
		CControl* pControl = *it;
		if( pControl != NULL && pControl->GetID() == ID )
		{
			T* pT = DynamicCast<T>( pControl );
			if( pT != NULL )
			{
				return pT;
			}
		}
		++it;
	}
	return NULL;
}

// Helper method to retrieve the Label with the given numeric identifier from the controls's
// internal collection of sub-controls
CLabel* CControl::GetLabel( int32 ID ) const
{
	return GetControlHelper<CLabel>( m_Controls, ID ); 
}

// Helper method to retrieve the Button with the given numeric identifier from the controls's
// internal collection of sub-controls
CButton* CControl::GetButton( int32 ID ) const
{
	return GetControlHelper<CButton>( m_Controls, ID ); 
}

// Helper method to retrieve the CheckBox with the given numeric identifier from the controls's
// internal collection of sub-controls
CCheckBox* CControl::GetCheckBox( int32 ID ) const
{
	return GetControlHelper<CCheckBox>( m_Controls, ID ); 
}

// Helper method to retrieve the RadioButton with the given numeric identifier from the controls's
// internal collection of sub-controls
CRadioButton* CControl::GetRadioButton( int32 ID ) const
{
	return GetControlHelper<CRadioButton>( m_Controls, ID ); 
}

// Helper method to retrieve the Slider with the given numeric identifier from the controls's
// internal collection of sub-controls
CSlider* CControl::GetSlider( int32 ID ) const
{
	return GetControlHelper<CSlider>( m_Controls, ID ); 
}

// Helper method to retrieve the Form with the given numeric identifier from the controls's
// internal collection of sub-controls
CForm* CControl::GetForm( int32 ID ) const
{
	return GetControlHelper<CForm>( m_Controls, ID );
}

//
// Protected methods
//

// Renders the control using the attached high-level effect object
// This is just a default implementation; implementing classes may override this method as needed
bool CControl::RenderEffectInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	assert( m_pEffect != NULL );

	UINT Passes;
	HRESULT hr = m_pEffect->Begin( &Passes, 0 );
	if( !SUCCEEDED(hr) )
	{
		return false;
	}

	bool Result = true;
	for( UINT i = 0; i < Passes; ++i ) 
	{
		m_pEffect->BeginPass( i );
		Result = RenderInternal( pDevice, Context );
		m_pEffect->EndPass( );

		if( !Result )
		{
			break;
		}
	}

	m_pEffect->End( );
	return Result;
}

// Prepares the control for rendering
void CControl::PreRender( LPDIRECT3DDEVICE9 pDevice )
{		
	// Apply behavior modifiers to the control
	ControlBehaviorCollection::const_iterator it = m_Behaviors.begin();
	ControlBehaviorCollection::const_iterator itend = m_Behaviors.end();
	while ( it != itend )
	{
		CControlBehavior* pBehavior = (*it);
		if ( pBehavior )
		{
			pBehavior->UpdateControl( this, pDevice );
		}
		++it;
	}

	// Ensure any pending layout items are taken care of 
	if ( m_LayoutDirty )
	{
		// Reset the dirty flag before calling UpdateLayout, as UpdateLayout
		// may need to reset the flag back to true if not enough data is yet available
		m_LayoutDirty = false;
		UpdateLayout( );			
	}
}

// Performs post-rendering logic
void CControl::PostRender( LPDIRECT3DDEVICE9 pDevice )
{
	// Allow each behavior modifier to restore control state
	ControlBehaviorCollection::const_iterator it = m_Behaviors.begin();
	ControlBehaviorCollection::const_iterator itend = m_Behaviors.end();
	while ( it != itend )
	{
		CControlBehavior* pBehavior = (*it);
		if ( pBehavior != NULL )
		{
			pBehavior->RestoreControl( this, pDevice );
		}
		++it;
	}
}

// Updates all visible element areas
void CControl::UpdateLayout()
{
	// Update bounding box
	UpdateBounds( );

	// Refresh all child controls
	ControlCollection::const_iterator it = m_Controls.begin();
	ControlCollection::const_iterator itend = m_Controls.end();
	while ( it != itend )
	{
		CControl* pControl = (*it);
		if ( pControl )
		{
			pControl->Refresh();
		}
		++it;
	}
}

// Updates the texture object and texture coordinates of the internal sprite object
void CControl::UpdateSprite( bool UpdateTexture, bool UpdateTextureUVs )
{
	if ( m_pSprite == NULL || m_pTexMaterial == NULL )
	{
		return;
	}

	// Update texture if needed
	if( UpdateTexture )
	{
		m_pSprite->SetMaterial( m_pTexMaterial );
	}

	// Update UVs if needed
	if( UpdateTextureUVs )
	{
		int32 TexWidth, TexHeight;
		if ( !m_pTexMaterial->GetTextureSize(0, TexWidth, TexHeight) )
		{
			assert(0);
			return;
		}

		if ( m_TextureUVs.right != 0 && m_TextureUVs.bottom != 0 &&
			TexWidth != 0 && TexHeight != 0 )	// Avoid divide-by-zero
		{

			float32 const OneDivWidth  = 1.0f / static_cast<float32>( TexWidth );
			float32 const OneDivHeight = 1.0f / static_cast<float32>( TexHeight );

			m_pSprite->SetTopLeftUV( static_cast<float32>( m_TextureUVs.left ) * OneDivWidth, 
				static_cast<float32>( m_TextureUVs.top ) * OneDivHeight );

			m_pSprite->SetBottomRightUV( static_cast<float32>( m_TextureUVs.right )  * OneDivWidth, 
				static_cast<float32>( m_TextureUVs.bottom ) * OneDivHeight );
		}
	}
}


} // namespace UI
} // namespace Graphics
} // namespaceWT
