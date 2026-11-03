//
// Form.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Control.h"
#include "WT/Graphics/UI/Font.h"
#include "WT/Graphics/Material.h"
#include "WT/Graphics/UI/Button.h"
#include "WT/Graphics/UI/Slider.h"
#include "WT/Graphics/UI/Label.h"
#include "WT/Graphics/UI/CheckBox.h"
#include "WT/Graphics/UI/RadioButton.h"
#include <vector>
#include <d3d9.h>


namespace WT
{
namespace Graphics
{
namespace UI
{


class CForm : public CControl
{
public:

	WT_CLASS( CForm, CControl )

	CForm();
	CForm( IDirect3DDevice9* pDevice );
	CForm( const CForm& Other );
	virtual ~CForm();

	// Sets the font collection associated with the form
	// Although not required, this field serves as a collection of fonts shared among
	// multiple controls within a form
    virtual void SetFonts( const FontCollection& Fonts )
	{
		m_Fonts.clear();

		FontCollection::const_iterator it = Fonts.begin();
		FontCollection::const_iterator itend = Fonts.end();
		while ( it != itend )
		{
			m_Fonts.push_back( (*it) );
			++it;
		}
	}

	// Retrieves the font collection associated with the form
    virtual FontCollection& GetFonts( )
	{
		return m_Fonts;
	}

	virtual const FontCollection& GetFonts( ) const
	{
		return m_Fonts;
	}

	// Sets the texture collection associated with the form
	// Although not required, this field serves as a collection of textures shared among
	// multiple controls within a form
    virtual void SetTextures( const TextureCollection& Textures )
	{
		// First clear existing textures
		ReleaseTextures();

		TextureCollection::const_iterator it = Textures.begin();
		TextureCollection::const_iterator itend = Textures.end();
		while ( it != itend )
		{
			if ( (*it) )
				(*it)->AddRef();

			m_Textures.push_back( *it );
			++it;
		}
	}

	// Retrieves the texture collection associated with the form
    virtual TextureCollection& GetTextures( )
	{
		return m_Textures;
	}

	virtual const TextureCollection& GetTextures() const
	{
		return m_Textures;
	}

	// Requests focus activation of the given control
	// This method will typically be called by controls themselves
	virtual void RequestFocus( CControl* pControl );

	// Clears focus from the currently focused item
	virtual void ClearFocus( )
	{
		RequestFocus( NULL );
	}

	// Gives focus to the form's default control
	virtual void FocusDefaultControl( )
	{
		RequestFocus( m_pDefaultControl );
	}

	// Cycles focus to the next control in the form's control collection
	virtual void CycleFocus( );

	// Sets all radio buttons which are direct children of the form to the "unchecked" state
	void ClearRadioButtonGroup( uint32 Group );

	// Retrieves the OnFocusChanged event for the form
	virtual FocusChangedEvent& OnFocusChanged( )
	{
		return m_FocusChanged;
	}

	// TODO : Review - Remove?  This should never be done, right?  To change a sprite a new
	// material (texture) should be set, and CControl will update the internal sprite object.
	// Establishes a new internal sprite object used to render the form
	virtual void SetSpriteObject( CSprite* pSprite )
	{
		SafeDelete( m_pSprite );
		if( pSprite != NULL )
		{
			m_pSprite = new CSprite( *pSprite );
		}
	}

	// Retrieves the internal sprite object for finer-grained sprite attribute setting and placement
	virtual CSprite* GetSpriteObject( ) const
	{
		return m_pSprite;
	}


	//
	// Methods that operate on the ControlCollection
	//

	// Retrieves the default control associated with the collection
	virtual CControl* GetDefaultControl() const
	{
		return m_pDefaultControl;
	}

	// Sets the default control
	virtual void SetDefaultControl( CControl* pControl )
	{
		m_pDefaultControl = pControl;
	}

	// Retrieves the first control in the collection with the given numeric identifier
	CControl* GetControl( int32 ID ) const;

	// Retrieves the first control in the collection with the given numeric identifier and type
	CControl* GetControl( int32 ID, EControlType::Type ControlType ) const;

	// Retrieves the control in the collection at the given screen coordinates
	CControl* GetControlAtPoint( int32 X, int32 Y ) const;

	// Helper method for setting the enabled state of a control
	void SetControlEnabled( int32 ID, bool Enabled );

	// Helper method for retrieving the enabled state of a control
	bool GetControlEnabled( int32 ID ) const;

	// Removes the control with the given numeric identifier from the collection
	void RemoveControl( int32 ID );


	// Helper methods to create specific controls.  This form object is set as the parent of the created controls.
	virtual CLabel* CreateLabel( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, 
								float32 Width, float32 Height, uint32 TextColor, bool IsDefault = false );

	virtual CButton* CreateButton( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, float32 Width, 
								   float32 Height, CMaterialTextured* pMaterial, uint8 Hotkey = 0xff, 
								   bool IsDefault = false );

	virtual CCheckBox* CreateCheckBox( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, float32 Width, 
									   float32 Height, CMaterialTextured* pMaterial, bool Checked = false, 
									   uint8 Hotkey = 0xff, bool IsDefault = false );

	virtual CRadioButton* CreateRadioButton( int32 ID, CFont* pFont, const wstring& Text, float32 X, float32 Y, 
											float32 Width, float32 Height, CMaterialTextured* pMaterial, 
											uint32 ButtonGroup, bool Checked = false, uint8 Hotkey = 0xff, 
											bool IsDefault = false );

	virtual CSlider* CreateSlider( int32 ID, float32 X, float32 Y, float32 Width, float32 Height, 
								  CMaterialTextured* pMaterial, int32 Min = 0, int32 Max = 100, int32 Value = 50, 
								  bool IsDefault = false );

	virtual CForm* CreateForm( int32 ID, float32 X, float32 Y, float32 Width, float32 Height,
							   CMaterialTextured* pMaterial );

	// Handles the given mouse event, raising user events as needed
	virtual bool HandleMouseEvent( uint32 Event, uint32 wParam, uint32 lParam );

	// Handles the given keyboard event, raising user events as needed
	virtual bool HandleKeyboardEvent( uint32 Event, uint32 wParam, uint32 lParam );


protected: // methods

	// Performs actual rendering of the control with respect to the given camera object
	virtual bool RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	// Updates all visible element areas	
    virtual void UpdateLayout( );

	// Raises the OnFocusChanged event to registered callbacks
	void RaiseOnFocusChanged( CControl* pControl, bool HasFocus )
	{
        m_FocusChanged( pControl, HasFocus );
	}

private: // methods

	void ReleaseTextures()
	{
		TextureCollection::iterator it = m_Textures.begin();
		TextureCollection::const_iterator itend = m_Textures.end();
		while ( it != itend )
		{
			if ( (*it) )
			{
				(*it)->Release();
			}
			++it;
		}
		m_Textures.clear();
	}

private: // data
	
	FontCollection		m_Fonts;			// shared fonts for the form
	TextureCollection	m_Textures;			// shared textures for the form
	CControl*			m_pFocusControl;	// the currently focused control
	CControl*			m_pDefaultControl;

private: // events

	FocusChangedEvent	m_FocusChanged;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
