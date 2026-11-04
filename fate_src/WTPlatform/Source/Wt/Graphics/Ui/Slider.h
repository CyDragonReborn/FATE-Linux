//
// Slider.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/UI/Control.h"
#include "WT/Graphics/UI/ControlEvents.h"
#include "WT/Graphics/UI/Button.h"
#include "WT/Graphics/Material.h"
#include "WT/Math/Math.h"
#include <d3d9.h>


namespace WT
{
namespace Graphics
{
namespace UI
{

using namespace Math;

class CSlider : public CControl
{
public:

	WT_CLASS( CSlider, CControl )


	CSlider();
	CSlider( IDirect3DDevice9* pDevice );
	CSlider( IDirect3DDevice9* pDevice, int32 ID, float32 X, float32 Y, float32 Width, float32 Height, 
			CMaterialTextured* pMaterial, int32 Min, int32 Max, int32 Value, CControl* pParent );
	CSlider( const CSlider& Other );
	virtual ~CSlider( );


	// Determines whether the control's boundaries contains the given point
	virtual bool ContainsPoint( int32 X, int32 Y ) const 
	{ 
		return CControl::ContainsPoint( X, Y ) || 
			( m_pButton != NULL ? m_pButton->ContainsPoint( X, Y ) : false );
	}

	// Handles the given mouse event, raising user events as needed
	virtual bool HandleMouseEvent( EControlMouseEvent::Event MouseEvent, uint32 VirtualKeyMask, bool ButtonPressed, 
								   const CVector2i& MousePos )
	{
		// TODO: Need to handle rotations, {Horizontal,Vertical,Free} cases

		if( CControl::HandleMouseEvent( MouseEvent, VirtualKeyMask, ButtonPressed, MousePos ) )
		{
			return true;
		}

		if( m_pButton != NULL )
		{
			return m_pButton->HandleMouseEvent( MouseEvent, VirtualKeyMask, ButtonPressed, MousePos );
		}
		return false;
	}

	// Sets the minimum logical value associated with the slider
	virtual void SetMin( int32 Min )
	{
		m_Min = Min;
		SetValueInternal( m_Value, false );
	}

	// Retrieves the minimum logical value associated with the slider
	virtual int32 GetMin( ) const
	{
		return m_Min;
	}

	// Sets the maximum logical value associated with the slider	
	virtual void SetMax( int32 Max )
	{
		m_Max = Max;
		SetValueInternal( m_Value, false );
	}

	// Retrieves the maximum logical value associated with the slider	
	virtual int32 GetMax( ) const
	{
		return m_Max;
	}

	// Helper method for associating both minimum and maximum logical slider values	
	virtual void SetRange( int32 Min, int32 Max )
	{
		m_Min = Min;
		m_Max = Max;
		SetValueInternal( m_Value, false );
	}
	
	// Sets the current logical value of the slider	
	virtual void SetValue( int32 Value )
	{
		SetValueInternal( Value, false );
	}

	// Retrieves the current logical value of the slider
	virtual int32 GetValue( ) const
	{
		return m_Value;
	}

	// Sets the value to increment the slider bar's current value by when the non-button
	// area of the slider's track bar is clicked
	// If this value is not set, it will be 10% of the overall range between the minimum and
	// maximum values of the slider
	virtual void SetIncrement( uint32 Increment )
	{
		m_Increment = Increment;
		m_IncrementSet = true;
	}

	// Retrieves the value to increment the slider bar's current value by when the non-button
	// area of the slider's track bar is clicked
	virtual uint32 GetIncrement( ) const
	{
		if( m_IncrementSet )
		{
			return m_Increment;
		}
		if( m_Min > m_Max )
		{
			return 0;
		}

		// Make sure return value is at least 1
		return Math_Clamp<uint32>( (m_Max-m_Min) / 10, 1, (m_Max-m_Min) / 10 );
	}
	
	// Retrieves the internal button element attached to the slider
	virtual CButton* GetButton( ) const
	{
		return m_pButton;
	}

	// Retrieves the OnValueChanged event associated with the slider control
	virtual SliderValueChangedEvent& OnValueChanged( )
	{
		return m_ValueChanged;
	}

protected: // methods

	// Internal method called from HandleMouseEvent when a mouse button is pressed over the control
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	virtual bool OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos );

	// Internal method called from HandleMouseEvent when the mouse position has changed
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	virtual bool OnMouseMove( const CVector2i& MousePos );

	// Updates all visible element areas	
    virtual void UpdateLayout( );

	// Performs actual rendering of the control with respect to the given camera object
	virtual bool RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

#if 0
	// Calculates the current slider value based on the sent horizontal position along the trackbar
	int32 ValueFromPosition( int32 X )
	{
		float32 u = ((float32)(m_Max - m_Min) / m_BoundingBox.Width( ));
		return static_cast<int32>( (0.5f + m_Min + u * (X - m_BoundingBox.left)) );
	}
#endif

private: // methods
    
	// Raises the ValueChanged event to registered callbacks
	void RaiseValueChanged( bool FromInput )
	{
		FromInput;	// reserved for potential later use

		m_ValueChanged( this, m_Value );
	}

	// Validates the sent value by clamping it between [min,max], flags the control
	// for layout, and raises the OnValueChanged event to registered callbacks
	void SetValueInternal( int32 NewValue, bool FromInput )
	{
		// Clamp to the range
		NewValue = Math_Clamp<int32>( NewValue, m_Min, m_Max );
		if( NewValue == m_Value )
		{
			// No change
			return;
		}

		// Update the value, the rects, then fire the events if necessary
		m_Value			= NewValue; 
		m_LayoutDirty	= true;
		RaiseValueChanged( FromInput );
	}

	// Performs initialization of internal properties
	void InitInternal( )
	{
		m_pButton->SetMaterial( m_pTexMaterial );

		m_MouseButtons[EMouseButtonType::LButton] = true;	// Enable mouse button 1
		m_MouseOffset	= CVector2f( 0.0f, 0.0f );
		m_ButtonPressed = false;
	}

private: // data

	// User properties
	int32					m_Min;
	int32					m_Max;
	int32					m_Value;
	int32					m_Increment;

	// Internal properties
	bool					m_IncrementSet;		// flags whether SetIncrement was called by the user
	CButton*				m_pButton;			// slider bar button
	bool					m_ButtonPressed;	// flags slider button as pressed
	CVector2i				m_MousePos;			// cached mouse position
	CVector2f				m_MouseOffset;		// offset from mouse position to slider button

private: // events

	SliderValueChangedEvent m_ValueChanged;
};


} // namespace UI
} // namespace Graphics
} // namespace WT
