//
// Control.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include <vector>
#include <d3d9.h>
#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/Graphics/UI/UIException.h"
#include "WT/Graphics/UI/ControlEvents.h"
#include "WT/Math/Math.h"
#include "WT/Graphics/Sprite.h"
#include "WT/Graphics/Material.h"


namespace WT
{
namespace Graphics
{
namespace UI
{

class CControl;
class CControlBehavior;

class CLabel;
class CButton;
class CCheckBox;
class CRadioButton;
class CSlider;
class CForm;

typedef std::vector<CControl *>						ControlCollection;
typedef std::vector<CControlBehavior *>				ControlBehaviorCollection;

// Describes various well-known control types
// CControl types can be accessed via the CControl.GetType method
// These values can be used for convenience and to reduce the number of calls to QueryInterface
namespace EControlType
{ 
	enum Type
	{
		Label = 0,		// static text label
		Button,			// button		
		CheckBox,		// check box
		RadioButton,	// radio button
		ComboBox,		// combo box
		Slider,			// slider
		EditBox,		// edit box
		ImeEditBox,		// IME-compatibile edit box
		ListBox,		// list box 
		ScrollBar,		// scrollbar
		Form,			// form comprising of child controls
		Composite,		// control consisting of various sub-controls
		Custom			// custom user control
	};
}

// Description of keyboard key state
namespace EControlKeyState
{
	enum State
	{
		Invalid = -1,
		KeyDown,
		KeyUp
	};
}

// Description of a mouse event
namespace EControlMouseEvent
{
	enum Event
	{
		Unknown = -1,
		MouseMove,
		MouseButton
	};
}

// Mouse button types for controls
namespace EMouseButtonType
{
	enum Type
	{
		None = 0,
		LButton,							// Left mouse button
		MButton,							// Middle mouse button
		RButton,							// Right mouse button
		X1Button,							// First X button
		X2Button,							// Second X button
		Wheel,								// Mouse wheel
		EndOfTypes
	};
}


using namespace Math;

class CControl : public System::CObject, public CRenderableImpl
{
public:

	WT_BEGIN_ABSTRACT_CLASS( CControl, System::CObject );
		WT_INHERITS_INTERFACE( IRenderable );
	WT_END_CLASS();

	CControl()
	{
		InitializeData();
	}
		
	CControl( IDirect3DDevice9* pDevice )
	{
		InitializeData();
		m_pDevice = pDevice;
	}

	CControl( IDirect3DDevice9* pDevice, int32 ID, CControl* pParent )
	{
		InitializeData();

		m_pDevice = pDevice;
		m_ID = ID;
		m_pParent = pParent;
	}

	CControl( const CControl& Other )
	{
		InitializeData();

		m_pParent = Other.m_pParent;
		m_ID = Other.m_ID;
		m_pDevice = Other.m_pDevice;
		m_Type = Other.m_Type;
		m_X = Other.m_X;
		m_Y = Other.m_Y;
		m_Width = Other.m_Width;
		m_Height = Other.m_Height;
		m_CenterX = Other.m_CenterX;
		m_CenterY = Other.m_CenterY;
		m_RotationZ = Other.m_RotationZ;
		m_BoundingBox = Other.m_BoundingBox;
		m_TextureUVs = Other.m_TextureUVs;
		SetEffect( Other.m_pEffect );
		SetMaterial( Other.m_pTexMaterial );
	}

	virtual ~CControl()
	{
		SafeRelease( m_pEffect );
		SafeDelete( m_pSprite );
	}


	// Initialize the CControl with a device object
	virtual void Init ( IDirect3DDevice9* pDevice )
	{
		m_pDevice = pDevice;
	}

	// Handles the given keyboard event, raising user events as needed
	virtual bool HandleKeyboardEvent( EControlKeyState::State KeyState, uint8 Key );

	// Handles the given mouse event, raising user events as needed
	virtual bool HandleMouseEvent( EControlMouseEvent::Event MouseEvent, uint32 VirtualKeyMask, bool ButtonPressed, 
								   const CVector2i& MousePos );

	// Renders the object from the perspective of the given camera
	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );


	// Set the size in pixels.
	virtual void SetSize( CVector2f Size )
	{
		SetSize( Size.x, Size.y );		
	}

	// Set the size in pixels.
	virtual void SetSize( float32 Width, float32 Height )
	{
		if( m_Width != Width || m_Height != Height )
		{
			m_Width = Width;
			m_Height = Height;

			// Flag layout as dirty
			m_LayoutDirty = true;
		}
	}

	// Get the size in pixels.
	virtual CVector2f GetSize( ) const
	{
		return CVector2f( m_Width, m_Height );
	}

	// Set the position in pixels.
	virtual void SetPosition( CVector2f Position )
	{
		SetPosition( Position.x, Position.y );
	}

	// Set the position in pixels.
	virtual void SetPosition( float32 X, float32 Y )
	{
		if( m_X != X || m_Y != Y )
		{
			m_X = X;
			m_Y = Y;
			
			// Flag layout as dirty
			m_LayoutDirty = true;

			ControlCollection::iterator it = m_Controls.begin();
			while( it != m_Controls.end() )
			{
				(*it)->TagDirtyRec();
				++it;
			}
		}
	}
	
	// Get the position in pixels.
	virtual CVector2f GetPosition() const
	{
		return CVector2f( m_X, m_Y );
	}

	// Set the offset of the center in pixels.
	virtual void SetRotationCenter( CVector2f Center )
	{
		SetRotationCenter( Center.x, Center.y );		
	}

	// Set the offset of the center in pixels.
	virtual void SetRotationCenter( float32 CenterX, float32 CenterY )
	{
		if( m_CenterX != CenterX || m_CenterY != CenterY )
		{
			m_CenterX = CenterX;
			m_CenterY = CenterY;

			// Flag layout as dirty
			m_LayoutDirty = true;
		}
	}

	// Get the offset of the center in pixels.
	virtual CVector2f GetRotationCenter( ) const 
	{
		return CVector2f( m_CenterX, m_CenterY );
	}

	// Set the rotation about the screen Z-axis in radians. Positive angles yield rotations that appear
	// counterclockwise to the user.
	virtual void SetRotationZ( float32 RotationZ )
	{
		if( m_RotationZ != RotationZ )
		{
			m_RotationZ = RotationZ;

			// Flag layout as dirty
			m_LayoutDirty = true;
		}
	}

	// Get the rotation about the screen Z-axis in radians. Positive angles yield rotations that appear
	// counterclockwise to the user.
	virtual float32 GetRotationZ( ) const
	{
		return m_RotationZ;
	}


	// Retrieves the parent control of the control
	virtual CControl* GetParent( ) const
	{
		return m_pParent;
	}
	
	// Retrieves the collection of child controls attached to this control
	virtual ControlCollection& GetControls( )
	{
		return m_Controls;
	}

	virtual ControlCollection const& GetControls( ) const
	{
		return m_Controls;
	}

	// Retrieves single control specified by collection index
	virtual CControl* GetControl( uint32 index ) const
	{
		if ( index < static_cast<uint32>( m_Controls.size() ) )
			return m_Controls[index];
		else
			return NULL;
	}

	uint32 GetControlCount() const
	{
		return static_cast<uint32>( m_Controls.size() );
	}

	void SetControlCount( uint32 Count )
	{
		m_Controls.resize( Count );
	}

	// Helper method to retrieve the Label with the given numeric identifier from the controls's
	// internal collection of sub-controls
	CLabel* GetLabel( int32 ID ) const;

	// Helper method to retrieve the Button with the given numeric identifier from the controls's
	// internal collection of sub-controls
	CButton* GetButton( int32 ID ) const;

	// Helper method to retrieve the CheckBox with the given numeric identifier from the controls's
	// internal collection of sub-controls
	CCheckBox* GetCheckBox( int32 ID ) const;

	// Helper method to retrieve the RadioButton with the given numeric identifier from the controls's
	// internal collection of sub-controls
	CRadioButton* GetRadioButton( int32 ID ) const;

	// Helper method to retrieve the Slider with the given numeric identifier from the controls's
	// internal collection of sub-controls
	CSlider* GetSlider( int32 ID ) const;

	// Helper method to retrieve the Form with the given numeric identifier from the controls's
	// internal collection of sub-controls
	CForm* GetForm( int32 ID ) const;
	
	// Determines whether the control can obtain focus
	virtual bool CanHaveFocus( ) const
	{
		return GetVisible() && m_Enabled; 
	}

	// Sets the current focus state of the control
    virtual void SetFocus( bool Focus ) 
	{ 
		m_HasFocus = Focus;
	}

	// Determines whether the control currently has focus
	virtual bool GetFocus( ) const
	{
		return m_HasFocus;
	}

	// Determines whether the control's boundaries contains the given point
	virtual bool ContainsPoint( int32 X, int32 Y ) const 
	{ 
		return ( X >= m_BoundingBox.left && X <= m_BoundingBox.right &&
				 Y >= m_BoundingBox.top  && Y <= m_BoundingBox.bottom );
	}

	// Determines whether the control's boundaries contains the given point
    virtual bool ContainsPoint( const CVector2i& Point ) const
	{ 
		return ContainsPoint( Point.x, Point.y );		
	}

	// Sets the current enabled state of the control
    virtual void SetEnabled( bool Enabled )
	{ 
		m_Enabled = Enabled; 
	}

	// Determines whether the control is currently enabled
    virtual bool GetEnabled( ) const
	{
		return m_Enabled; 
	}

	// Retrieves the type of the control
    virtual EControlType::Type GetControlType( ) const
	{
		return m_Type; 
	}

	// Retrieves the controls numeric identifier
    virtual int32 GetID( ) const 
	{ 
		return m_ID; 
	}
    
	// Sets the keyboard hotkey associated with the control
	virtual void SetHotkey( uint8 Hotkey ) 
	{ 
		m_Hotkey = Hotkey; 
	}

	// Retrieves the keyboard hotkey associated with the control
    virtual uint32 GetHotkey( ) const
	{ 
		return m_Hotkey; 
	}

	// Associates arbitrary user data with the control
    virtual void SetUserData( void* pUserData ) 
	{ 
		m_pUserData = pUserData; 
	}

	// Retrieves any arbitrary user data associated with the control, or NULL if no
	// user data is associated with the control
    virtual void* GetUserData( ) const 
	{ 
		return m_pUserData; 
	}

	// Sets the texture associated with the control's visual elements
	virtual void SetMaterial( CMaterialTextured* pMaterial )
	{
		m_pTexMaterial = pMaterial;

		if ( m_pTexMaterial )
		{
			UpdateSprite( true, true );
		}
	}

	// Retrieves the texture associated with the control's visual elements
	virtual CMaterialTextured* GetMaterial( ) const
	{
		return m_pTexMaterial;
	}

	// Sets the texture coordinates associated with the control's base element layer
	virtual void SetTextureUVs( const CRect2i& TextureUVs )
	{
		m_TextureUVs = TextureUVs;
		UpdateSprite( false, true );
	}

	// Retrieves the texture coordinates associated with the control's base element layer
	virtual CRect2i GetTextureUVs( ) const
	{
		return m_TextureUVs;
	}

	// Associates the given high-level effect object with the control
	virtual void SetEffect( ID3DXEffect* pEffect )
	{
		if ( pEffect )
			pEffect->AddRef();

		SafeRelease( m_pEffect );
		m_pEffect = pEffect;
	}

	// Retrieves the high-level effect object associated with the control
	virtual ID3DXEffect* GetEffect( ) const
	{
		return m_pEffect;
	}

	// Retrieves the collection of behavior modifiers associated with the control
	virtual ControlBehaviorCollection& GetBehaviors()
	{
		return m_Behaviors;
	}

	virtual ControlBehaviorCollection const& GetBehaviors() const
	{
		return m_Behaviors;
	}

	CControlBehavior* GetBehavior( uint32 Index ) const
	{
		if ( Index < static_cast<uint32>(m_Behaviors.size()) )
			return m_Behaviors[Index];
		else
			return NULL;
	}

	void SetBehavior( uint32 Index, CControlBehavior* pChild )
	{
		if ( Index < static_cast<uint32>(m_Behaviors.size()) )
		{
			m_Behaviors[Index] = pChild;
		}
		else 
		{
			throw WT::System::CIndexOutOfRangeException();
		}
	}

	uint32 GetBehaviorCount() const
	{
		return static_cast<uint32>( m_Behaviors.size() );
	}

	void SetBehaviorCount( uint32 Count )
	{
		m_Behaviors.resize( Count );
	}

	// Retrieves the KeyDown keyboard event associated with the control
	virtual KeyPressedEvent& OnKeyDown( )
	{
		return m_KeyDown;
	}

	// Retrieves the KeyUp keyboard event associated with the control
	virtual KeyPressedEvent& OnKeyUp( )
	{
		return m_KeyUp;
	}

	virtual MouseClickEvent& OnMouseClick( )
	{
		return m_MouseClick;
	}

	virtual MouseMoveEvent& OnMouseMove( )
	{
		return m_MouseMove;
	}
	
	// Enables or disables the control from processing calls to HandleKeyboardEvent
	// However, if the control has a valid parent, the parent's value is used to determine this value
	// By default, this value is set to kTrue
	virtual void SetKeyboardInputEnabled( bool Enable )
	{
		m_KeyboardInput = Enable; 
	}

	// Determines whether the form is set to process calls to HandleKeyboardEvent
	virtual bool GetKeyboardInputEnabled( ) const
	{
		return m_KeyboardInput;		
	}

	// Enables or disables the control from processing calls to HandleMouseEvent
	// However, if the control has a valid parent, the parent's value is used to determine this value
	// By default, this value is set to true
    virtual void SetMouseInputEnabled( bool Enable )
	{
		m_MouseInput = Enable;
	}

	// Determines whether the form is set to process calls to HandleMouseEvent
    virtual bool GetMouseInputEnabled( ) const
	{
		return m_MouseInput;		
	}

	// Determines whether the mouse cursor is currently hovering the control
	virtual bool HasMouseOver( ) const
	{
		return m_MouseOver;
	}

	// Determines whether the button is in the "pressed" state
	virtual bool IsPressed( ) const
	{
		return m_Pressed;
	}

	// Forces the control to refresh its internal layout
	virtual void Refresh( )
	{
		UpdateLayout( );
	}

	// Calculates the absolute position of the control
	// This is the sum of the control's current position and its parent's absolute position
	virtual CVector2f GetAbsolutePosition( ) const
	{
		CVector2f const Position( m_X, m_Y );
		if( m_pParent != NULL )
		{
			return Position + m_pParent->GetAbsolutePosition( );
		}
		return Position;
	}


	// The following methods are for Editor compatibility

	void SetParent( CControl* pParent )
	{
		m_pParent = pParent;
	}

	void SetControls( const ControlCollection& Controls )
	{
		m_Controls.clear();

		ControlCollection::const_iterator it = Controls.begin();
		while ( it != Controls.end() )
		{
			m_Controls.push_back( *it );
			++it;
		}
	}

	void SetControl( uint32 Index, CControl* pControl )
	{
		if ( Index < static_cast<uint32>(m_Controls.size()) )
			m_Controls[Index] = pControl;
		else
			throw WT::System::CIndexOutOfRangeException();
	}

	void SetTypeEnum( int32 TypeEnum )
	{
		m_Type = static_cast<EControlType::Type>( TypeEnum );
	}
	int32 GetTypeEnum( ) const
	{
		return static_cast<int32>( m_Type );
	}

	void SetID( int32 ID )
	{
		m_ID = ID;
	}


	static inline EMouseButtonType::Type GetMouseButtonType( uint32 VirtualKeyMask )
	{
		if ( (VirtualKeyMask & MK_LBUTTON) == MK_LBUTTON )
		{
			return EMouseButtonType::LButton;
		}
		else if ( (VirtualKeyMask & MK_MBUTTON) == MK_MBUTTON )
		{
			return EMouseButtonType::MButton;
		}
		else if ( (VirtualKeyMask & MK_RBUTTON) == MK_RBUTTON )
		{
			return EMouseButtonType::RButton;
		}
		/*
		else if ( (VirtualKeyMask & MK_XBUTTON1) == MK_XBUTTON1 )
		{
			return EMouseButtonType::X1Button;
		}
		else if ( (VirtualKeyMask & MK_XBUTTON2) == MK_XBUTTON2 )
		{
			return EMouseButtonType::X2Button;
		}
		*/
		else
		{
			return EMouseButtonType::None;
		}
	}


protected: // methods

	// Prepares the control for rendering
	virtual void PreRender( LPDIRECT3DDEVICE9 pDevice );

	// Renders the control using the attached high-level effect object
	// This is just a default implementation; implementing classes may override this method as needed
    virtual bool RenderEffectInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	// Performs actual rendering of the control with respect to the given camera object
	// Note that derived classes must implement this method
	virtual bool RenderInternal( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context ) = 0;

	// Performs post-rendering logic
	virtual void PostRender( LPDIRECT3DDEVICE9 pDevice );

	// Determines whether the control implementation is using visual sprite elements
	// By default, this implementation returns kTrue. Derived classes may override this method as needed
	virtual bool UsingSpriteElements( )
	{
		return true;
	}

	// Determines whether the control is flagged to possibly consume keyboard events
	// Note that the parent setting may override the internal value
	virtual bool UsingKeyboardEvents( )
	{
		return m_pParent != NULL ? (m_KeyboardInput && m_pParent->GetKeyboardInputEnabled()) : m_KeyboardInput;
	}

	// Internal method called from HandleKeyboardEvent when a keyboard key is pressed
	// Note that HandleKeyboardEvent will filter out cases where the event should not be handled
	virtual bool OnKeyDown( uint8 Key )
	{
		if( m_Hotkey == Key )
		{
			// Raise the event to registered callbacks
			m_KeyDown( this, Key );
			return true;
		}

		return false;
	}

	// Internal method called from HandleKeyboardEvent when a keyboard key is released
	// Note that HandleKeyboardEvent will filter out cases where the event should not be handled
	virtual bool OnKeyUp( uint8 Key )
	{
		if( m_Hotkey == Key )
		{
			// Raise the event to registered callbacks
			m_KeyUp( this, Key );
			return true;
		}

		return false;
	}

	// Determines whether the control is flagged to possibly consume mouse events
	// Note that the parent setting may override the internal value
	virtual bool UsingMouseEvents( )
	{
		return m_pParent != NULL ? (m_MouseInput && m_pParent->GetMouseInputEnabled( )) : m_MouseInput;
	}

	// Internal method called from HandleMouseEvent when a mouse button is pressed over the control
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	// Derived classes should override this method as needed 
	virtual bool OnMouseButton( uint32 VirtualKeyMask, bool ButtonPressed, const CVector2i& MousePos )
	{
		ButtonPressed;
		if ( ContainsPoint( MousePos ) )
		{
			m_MouseClick( this, VirtualKeyMask );
			return true;
		}

		return false;
	}

	// Internal method called from HandleMouseEvent when the mouse position has changed
	// Note that HandleMouseEvent will filter out cases where the event should not be handled
	// Derived classes should override this method as needed 
	virtual bool OnMouseMove( const CVector2i& MousePos )
	{
		m_MouseMove( this, MousePos );
		return true;
	}

	// Updates the bounding area of the control according to its size and absolute position
	virtual void UpdateBounds( ) 
	{
		CVector2f const AbsPosition = GetAbsolutePosition( );
		float32 const& X = AbsPosition.x;
		float32 const& Y = AbsPosition.y;
		
		m_BoundingBox = CRect2f( X, Y, X + m_Width, Y + m_Height );
	}

	// Updates all visible element areas
    virtual void UpdateLayout();

	// Updates the texture object and texture coordinates of the internal sprite object
	virtual void UpdateSprite( bool UpdateTexture, bool UpdateTextureUVs );

	virtual void TagDirtyRec()
	{
		m_LayoutDirty = true;

        ControlCollection::iterator it = m_Controls.begin();
		while( it != m_Controls.end() )
		{
			(*it)->TagDirtyRec();
			++it;
		}
	}

private: // methods

	// Initializes data members to their default values
	// This class is rather data-heavy, so this method was added to reduce code needed
	// for each overloaded constructor
	void InitializeData()
	{
		m_ID			= -1; 
		m_pParent		= NULL;
		m_HasFocus		= false;
		m_IsDefault		= false;
		m_MouseInput	= true;
		m_KeyboardInput = true;
		m_X				= 0;
		m_Y				= 0;
		m_Width			= 0;
		m_Height		= 0;		
		m_CenterX		= 0.0f;
		m_CenterY		= 0.0f;
		m_RotationZ		= 0.0f;
		m_Index			= 0;
		m_Type			= EControlType::Custom;
		m_Hotkey		= 0xFF;
		m_pUserData		= NULL;
		m_Enabled		= true;
		m_BoundingBox	= CRect2f( 0.0f,0.0f,0.0f,0.0f );
		m_MouseOver		= false;
		m_Pressed		= false;
		m_LayoutDirty	= true;

		m_pTexMaterial	= NULL;
		m_pEffect		= NULL;
		m_pSprite		= NULL;
		m_pUserData		= NULL;
		m_pParent		= NULL;
		m_pDevice		= NULL;

		memset( m_MouseButtons, 0, sizeof(m_MouseButtons) );
		
		if( UsingSpriteElements() )
		{
			m_pSprite = new CSprite();
			if( !m_pSprite )
			{
				throw CUIException( "CControl::InitializeData - Out of memory" );
			}
		}
	}

protected:	// data








	// Identifier attributes
	uint32					m_Index;		// Index within the control list, set by the container
    int32					m_ID;			// ID number
	EControlType::Type		m_Type;			// Control type, set once in constructor  
    void*					m_pUserData;	// Data associated with this control that is set by user
	
    // Size, scale, and positioning members
    float32					m_X;
	float32					m_Y;
    float32					m_Width;
	float32					m_Height;
	float32					m_CenterX;
	float32					m_CenterY;
	float32					m_RotationZ;
	CRect2f					m_BoundingBox;  // Rectangle defining the active region of the control

	// Hierarchy attributes
    CControl*				m_pParent;		// Parent control, set by the container
	ControlCollection		m_Controls;

	// Input attributes
	uint8					m_Hotkey;		// Virtual key code for this control's hotkey


	KeyPressedEvent			m_KeyDown;
	KeyPressedEvent			m_KeyUp;
	MouseClickEvent			m_MouseClick;
	MouseMoveEvent			m_MouseMove;


	bool					m_MouseButtons[EMouseButtonType::EndOfTypes];

	// Rendering/behavior attributes
	CSprite*				m_pSprite;		// internal sprite object built at runtime
	CMaterialTextured*		m_pTexMaterial;
	CRect2i					m_TextureUVs;
	ID3DXEffect*			m_pEffect;
	ControlBehaviorCollection m_Behaviors;

	IDirect3DDevice9*		m_pDevice;

	// State attributes
	bool					m_Enabled     : 1;      // Enabled/disabled flag
	bool					m_HasFocus    : 1;     // Control has input focus
	bool					m_IsDefault   : 1;	// Is the default control
	bool					m_LayoutDirty : 1;	// flags need to rebuild layout

	//Input flags
	bool					m_MouseInput	: 1;
	bool					m_KeyboardInput : 1;
	bool					m_MouseOver		: 1;
	bool					m_Pressed		: 1;
};


} // namespace UI
} // namespace Graphics
} // WT
