/*
	text.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#ifndef _TEXT_H
#define _TEXT_H

#include <string>
#include "../UTILITIES/constants.h"
#include "fontmetric.h"

class CMaterial;
class CFontMetric;

// text justification possibilities
enum EJustification
{
	KJustifyLeft,
	KJustifyRight,
	KJustifyCenter
};

// CText objects are comprised of polygons, and use a CFontMetric object to calculate UVs
// and poly positioning
class CText
{
public:

							CText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
								   CMaterial* pMaterial,				// material of the font texture we want to use
								   CFontMetric* pFontMetric,			// font metric describing character widths/positioning
								   const std::string& Text,			// text to display
								   float32 X,							// x location onscreen (pixel coords from top left)
								   float32 Y,							// y location onscreen (pixel coords from top left)
								   float32 Scale,						// scale factor for the text (1.0f)
								   bool Bold,							// bolded?
								   EJustification Justification,		// justification for the text
								   uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								   uint32 ScreenHeight = 768 );			// screen height used in orthographic projection

							CText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
								   CMaterial* pMaterial,				// material of the font texture we want to use
								   CMaterial* pMaterial2,				// another font texture for accomodating multiple sheets
								   CFontMetric* pFontMetric,			// font metric describing character widths/positioning
								   const std::string& Text,			// text to display
								   float32 X,							// x location onscreen (pixel coords from top left)
								   float32 Y,							// y location onscreen (pixel coords from top left)
								   float32 Scale,						// scale factor for the text (1.0f)
								   bool Bold,							// bolded?
								   EJustification Justification,		// justification for the text
								   uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								   uint32 ScreenHeight = 768 );			// screen height used in orthographic projection
							~CText();

	void					Update( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									CMaterial* pMaterial,				// material of the font texture we want to use
								    CFontMetric* pFontMetric,			// font metric describing character widths/positioning
								    const std::string& Text,			// text to display
								    float32 X,							// x location onscreen (pixel coords from top left)
								    float32 Y,							// y location onscreen (pixel coords from top left)
								    float32 Scale,						// scale factor for the text (1.0f)
								    bool Bold,							// bolded?
								    EJustification Justification,		// justification for the text
								    uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								    uint32 ScreenHeight = 768 );		// screen height used in orthographic projection

		void					Update( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									CMaterial* pMaterial,				// material of the font texture we want to use
									CMaterial* pMaterial2,				// another font texture for accomodating multiple sheets
								    CFontMetric* pFontMetric,			// font metric describing character widths/positioning
								    const std::string& Text,			// text to display
								    float32 X,							// x location onscreen (pixel coords from top left)
								    float32 Y,							// y location onscreen (pixel coords from top left)
								    float32 Scale,						// scale factor for the text (1.0f)
								    bool Bold,							// bolded?
								    EJustification Justification,		// justification for the text
								    uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								    uint32 ScreenHeight = 768 );		// screen height used in orthographic projection

	void					Update( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
								    const std::string& Text,			// text to display
								    float32 X,							// x location onscreen (pixel coords from top left)
								    float32 Y,							// y location onscreen (pixel coords from top left)
								    float32 Scale,						// scale factor for the text (1.0f)
								    bool Bold,							// bolded?
								    EJustification Justification,		// justification for the text
								    uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								    uint32 ScreenHeight = 768 );		// screen height used in orthographic projection

	void					Render( LPDIRECT3DDEVICE8 pD3DDevice);				// the direct3d device
	void					RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice );	// the direct3d device

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	EJustification			Justification( void )		{	return m_Justification;		};

	bool					Bolded( void )				{	return m_Bold;				};

	bool					Wrapping( void )			{	return m_Wrapping;			};

	CMaterial*				Material( void )			{	return m_pMaterial;			};

	const std::string&		Text( void )				{	return m_LastText;			};

	float32					Scale( void )				{	return m_Scale;				};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void					SetPosition ( float32 X,							// x location onscreen (pixel coords from top left)
										  float32 Y,							// y location onscreen (pixel coords from top left)
										  uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
										  uint32 ScreenHeight = 768 );		// screen height used in orthographic projection

	void					EnableWrapping( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
											float32 WrapWidth );			// pixel wrap width;

	void					DisableWrapping( LPDIRECT3DDEVICE8 pD3DDevice );// our direct3d device

	void					SetBold( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
									 bool Bold );					// bold state

	void					SetScale( LPDIRECT3DDEVICE8 pD3DDevice, // our direct3d device
									  float32 Scale );				// scale factor for the text (1.0f)

	void					SetJustification( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
											  EJustification Justification );	// justification for the text

	void					SetText( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									 const std::string& Text );			// text to display

	const D3DXVECTOR3&		MinBounds( void )		{	return m_MinBounds;		};
	const D3DXVECTOR3&		MaxBounds( void )		{	return m_MaxBounds;		};

	std::string				WrapText( std::string Message,			// message to wrap
									  float32 Scale,				// scale of font
									  float32 MaxPixels );			// pixel width to wrap to


private:
	std::string				m_LastText;

	int32					m_TextCharacters;

	uint32					m_ScreenWidth;	// screen width in pixels
	uint32					m_ScreenHeight;	// screen height in pixels
	float32					m_ScreenX;	//7 screen coordinate
	float32					m_ScreenY;	// screen coordinate
	float32					m_X;		// worldspace coordinate
	float32					m_Y;		// worldspace coordinate
	float32					m_Scale;

	EJustification			m_Justification;
	
	bool					m_Bold;

	bool					m_Wrapping;

	bool					m_BufferExists;	// whether a vertex buffer already exists

	bool					m_Wrapped;

	float32					m_WrapWidth;

	int32					m_LastLength;

	CMaterial*				m_pMaterial;
	CMaterial*				m_pMaterial2;
	CFontMetric*			m_pFontMetric;

	LPDIRECT3DVERTEXBUFFER8	m_pVB;
	LPDIRECT3DINDEXBUFFER8	m_pIB;

	D3DXVECTOR3				m_MinBounds;
	D3DXVECTOR3				m_MaxBounds;
};


#endif