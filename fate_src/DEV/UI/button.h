/*
	button.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	7/12/2004
	
*/

#ifndef _BUTTON_H
#define _BUTTON_H

#include <string>
#include "../UTILITIES/constants.h"
#include "text.h"
#include "fmod.h"

class CText;
class CFontMetric;

// possible centerpoints for a sprite
enum EButtonCenter
{
	KButtonTopLeft,
	KButtonCenter
};

enum EButtonState
{
	KButtonOff,
	KButtonOver,
	KButtonRequiresUpdate,
	KButtonStates
};

// CButton objects are comprised of two tris and are UI elements
// which manage rollover states and can receive click events
class CButton
{
public:

							CButton( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									 CMaterial* pMaterial,				// material of the sprite texture we want to use
									 float32 X,							// x location onscreen (pixel coords from top left)
									 float32 Y,							// y location onscreen (pixel coords from top left)
									 float32 Width,						// width of the sprite, in pixels
									 float32 Height,					// height of the sprite, in pixels
									 float32 TopLeftU,					// u texture coord at top left 
									 float32 TopLeftV,					// v texture coord at top left 
									 float32 BottomRightU,				// u texture coord at bottom right 
									 float32 BottomRightV,				// v texture coord at bottom right
									 float32 OverTopLeftU,				// u texture coord at top left of rollover
									 float32 OverTopLeftV,				// v texture coord at top left of rollover
									 float32 OverBottomRightU,			// u texture coord at bottom right of rollover
									 float32 OverBottomRightV,			// v texture coord at bottom right of rollover
									 EButtonCenter CenterPoint,			// justification for the text
									 int32 ButtonCode,					// code that is returned on click ( default -1 )
								     uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								     uint32 ScreenHeight = 768 );		// screen height used in orthographic projection

							CButton( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									 CMaterial* pMaterial,				// material of the sprite texture we want to use
									 FSOUND_SAMPLE* pOverSound,			// over sound
									 FSOUND_SAMPLE* pClickSound,		// click sound
									 float32 X,							// x location onscreen (pixel coords from top left)
									 float32 Y,							// y location onscreen (pixel coords from top left)
									 float32 Width,						// width of the sprite, in pixels
									 float32 Height,					// height of the sprite, in pixels
									 float32 TopLeftU,					// u texture coord at top left 
									 float32 TopLeftV,					// v texture coord at top left 
									 float32 BottomRightU,				// u texture coord at bottom right 
									 float32 BottomRightV,				// v texture coord at bottom right
									 float32 OverTopLeftU,				// u texture coord at top left of rollover
									 float32 OverTopLeftV,				// v texture coord at top left of rollover
									 float32 OverBottomRightU,			// u texture coord at bottom right of rollover
									 float32 OverBottomRightV,			// v texture coord at bottom right of rollover
									 EButtonCenter CenterPoint,			// justification for the text
									 int32 ButtonCode,					// code that is returned on click ( default -1 )
								     uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								     uint32 ScreenHeight = 768 );		// screen height used in orthographic projection
							
							~CButton();
	
	bool					CreateVB( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									  float32 Width,					// width of the sprite, in pixels
									  float32 Height,					// height of the sprite, in pixels
									  float32 TopLeftU,					// u texture coord at top left 
									  float32 TopLeftV,					// v texture coord at top left 
									  float32 BottomRightU,				// u texture coord at bottom right 
									  float32 BottomRightV );			// v texture coord at bottom right


	void					Render( LPDIRECT3DDEVICE8 pD3DDevice);				// the direct3d device
	void					RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice );	// the direct3d device
	
	void					Render( LPDIRECT3DDEVICE8 pD3DDevice, 				// our direct3d device
									const D3DXMATRIX& Transformation );			// transformation for the sprite

	void					RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice, 				// our direct3d device
											 const D3DXMATRIX& Transformation );			// transformation for the sprite

	void					Update( const POINT& MousePosition );		// current screen mouse position 

	void					SetText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									 const std::string& Text );			// button text to add 

	void					SetOverText( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
										 const std::string& Text );		// button text to add 

	void					AddText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									 const std::string& Text,		// button text to add 
									 CMaterial* pMaterial,			// material to use when rendering text
									 CFontMetric* pFontMetric,		// font metric text is based upon
									 float32 X,						// x offset from button centerpoint
									 float32 Y,						// y offset from button centerpoint
									 float32 Scale,					// scale of text
									 bool Bold,						// is text bolded?
									 EJustification Justification,	// text justification
									 uint32 ScreenWidth = 1024,		// screen width of ui elements (default 1024)
									 uint32 ScreenHeight = 768 );	// screen height of ui elements (default 768)

	void					AddOverText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
										 const std::string& Text,		// button text to add 
										 CMaterial* pMaterial,			// material to use when rendering text
										 CFontMetric* pFontMetric,		// font metric text is based upon
										 float32 X,						// x offset from button centerpoint
										 float32 Y,						// y offset from button centerpoint
										 float32 Scale,					// scale of text
										 bool Bold,						// is text bolded?
										 EJustification Justification,	// text justification
										 uint32 ScreenWidth = 1024,		// screen width of ui elements (default 1024)
										 uint32 ScreenHeight = 768 );	// screen height of ui elements (default 768)

	void					ClearText( void );															

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	float32					GetX( void )		{	return m_X;	};
	float32					GetY( void )		{	return m_Y;	};

	// pixel-space coordinates
	float32					GetScreenX( void )	{	return m_ScreenX;	};
	float32					GetScreenY( void )	{	return m_ScreenY;	};

	int32					IsOver( const POINT& MousePosition );	// returns the button code if the
																	// position is over the button,
																	// or -1 if not

	CMaterial*				Material( void )	{	return m_pMaterial;		};

	const std::string&		ToolTip( void )		{	return m_ToolTipText;	};
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void					SetMaterial( CMaterial* pMaterial )			{	m_pMaterial = pMaterial;	};

	void					SetAllowClickOnEdge( bool State )			{	m_AllowClickOnEdge = State;	};

	void					SetButtonCode( int32 Code )					{	m_ButtonCode = Code;		};

	void					Press( void );

	void					SetPosition( float32 X,					// x location onscreen (pixel coords from top left)
										 float32 Y );				// y location onscreen (pixel coords from top left)

	void					SetPosition( float32 X,					// x location onscreen (pixel coords from top left)
										 float32 Y, 				// y location onscreen (pixel coords from top left)
										 uint32 ScreenWidth,		// screen width used in orthographic projection
										 uint32 ScreenHeight );		// screen height used in orthographic projection

	void					SetUVs( float32 TopLeftU,					// u texture coord at top left 
								    float32 TopLeftV,					// v texture coord at top left 
									float32 BottomRightU,				// u texture coord at bottom right 
									float32 BottomRightV );				// v texture coord at bottom right

	void					SetOverUVs( float32 TopLeftU,					// u texture coord at top left 
										float32 TopLeftV,					// v texture coord at top left 
										float32 BottomRightU,				// u texture coord at bottom right 
										float32 BottomRightV );				// v texture coord at bottom right

	void					SetSize( float32 Width,						// width of the sprite, in pixels
									 float32 Height );					// height of the sprite, in pixels

	void					SetToolTip( const std::string& Text )		{	m_ToolTipText = Text;		};

	void					SetOverSound( FSOUND_SAMPLE* pSample )		{	m_pOverSound = pSample;		};

	void					SetClickSound( FSOUND_SAMPLE* pSample )		{	m_pClickSound = pSample;	};

private:

	void					UpdateTextPosition( uint32 ScreenWidth = 1024,		// screen width of ui elements (default 1024)
												uint32 ScreenHeight = 768  );	// screen height of ui elements (default 768)

	void					UpdateUVs( void );

	void					UpdateScreenBounds( void );

	bool					OverButton( const POINT& Position );	// position to check

	EButtonCenter			m_CenterPoint;

	uint32					m_ScreenWidth;
	uint32					m_ScreenHeight;

	float32					m_X;
	float32					m_Y;
	float32					m_ScreenX;
	float32					m_ScreenY;
	float32					m_TextX;
	float32					m_TextY;
	float32					m_OverTextX;
	float32					m_OverTextY;
	float32					m_Width;
	float32					m_Height;

	CMaterial*				m_pMaterial;
	LPDIRECT3DVERTEXBUFFER8	m_pVB;

	float32					m_TopLeftU;
	float32					m_TopLeftV;
	float32					m_BottomRightU;
	float32					m_BottomRightV;

	float32					m_OverTopLeftU;
	float32					m_OverTopLeftV;
	float32					m_OverBottomRightU;
	float32					m_OverBottomRightV;

	float32					m_ScreenTopLeftX;
	float32					m_ScreenTopLeftY;
	float32					m_ScreenBottomRightX;
	float32					m_ScreenBottomRightY;

	EButtonState			m_ButtonState;		// state of the button
	EButtonState			m_SpriteButtonState;// state of the sprite

	int32					m_ButtonCode;		// return code when this button is pressed
												// defaults to -1

	CText*					m_pButtonText;		// text to be displayed within the button ( not requried )
	CText*					m_pButtonOverText;		// text to be displayed within the button when rolled over( not requried )

	std::string				m_ToolTipText;		// rollover text - blank by default

	FSOUND_SAMPLE*			m_pOverSound;
	FSOUND_SAMPLE*			m_pClickSound;

	bool					m_AllowClickOnEdge;
};


#endif