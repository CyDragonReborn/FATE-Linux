//
// CheckBox.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Button.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace Math;

class CCheckBox : public CButton
{
public:

	WT_CLASS( CCheckBox, CButton )
	
	CCheckBox( IDirect3DDevice9* pDevice )
		: CButton( pDevice, -1, NULL )
		, m_Checked( false )
	{
		m_Type = EControlType::CheckBox;		
	}

	CCheckBox( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent )
		: CButton( pDevice, ID, pParent )
		, m_Checked( false )
	{
		m_Type = EControlType::CheckBox;
	}

	virtual ~CCheckBox( )
	{
	}


	// Sets the texture coordinates associated with the control's base element layer
	virtual void SetTextureUVs( const CRect2i& TextureUVs )
	{
		// This implementation assumes that the specified texture UVs actually represent 
		// two images laid-out horizontally; each using 50% of the texture space

		int32 const Width  = TextureUVs.right-TextureUVs.left;

		m_TextureUVs = TextureUVs;
		m_TextureUVsChecked = TextureUVs;

		m_TextureUVs.right -= Width/2;
		m_TextureUVsChecked.left += Width/2;

		// Update sprite UVs
		UpdateSprite( false, true );
	}

	// Establishes the current checked state of the control
	virtual void SetChecked( bool Checked )
	{
		m_Checked = Checked;
	}

	// Determines whether the control is currently in the checked state
	virtual bool GetChecked( ) const
	{
		return m_Checked;
	}

protected: // methods

	CCheckBox() {}

	// Performs actual rendering of the control with respect to the given camera object
	virtual bool RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	// Updates the texture object and texture coordinates of the internal sprite object
	virtual void UpdateSprite( bool UpdateTexture, bool UpdateTextureUVs );

	// Updates all visible element areas
    virtual void UpdateLayout( );

	// Updates the button to be in the non-pressed state and raises the OnClick event
	// This method is called when a button is released from either the keyboard hotkey
	// or a registered mouse button
	virtual void DoPressUp( uint32 VirtualKeyMask )
	{
		CButton::DoPressUp( VirtualKeyMask );

		// Toggle checked state
		SetChecked( !GetChecked( ) );
	}

protected: // data

	// User properties
	bool		m_Checked;	
	
	// Internal properties
	CRect2i		m_TextureUVsChecked;	// UV bounds for checked state
	CVector2f	m_CheckedStartUV;		// cached upper-left checked UVs
	CVector2f	m_CheckedEndUV;			// cached lower-right checked UVs
};


} // namespace UI
} // namespace Graphics
} // namespace WT
