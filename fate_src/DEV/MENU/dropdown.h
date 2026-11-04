/*
	dropdown.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/7/2004

*/


#ifndef _DROPDOWN_H
#define _DROPDOWN_H

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>
#include <math.h>

#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"

#include "../MEDIA/material.h"

#include "../UTILITIES/llist.h"

#include "../UI/sprite.h"
#include "../UI/button.h"
#include "../UI/text.h"


// dropdown menu object

class CDropdown
{
public:
	CDropdown( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
			   CMaterial* pSpriteMaterial,				// material for backdrop
			   CMaterial* pFontMaterial,				// material for sprite
			   CFontMetric* pFontMetric,				// font metric for items
			   float32 X,								// x position
			   float32 Y,								// y position
			   float32 Width,							// width of dropdown
			   float32 TopHeight,							// height of top item
			   float32 BottomHeight,						// height of bottom item
			   float32 ItemHeight,							// height of individual items
			   float32 TextScale,							// scale of text within
			   float32 TextOffsetX,							// text offset per button
			   float32 TextOffsetY,							// text offset per button
			   float32 TopU1,								// top u
			   float32 TopV1,								// top v
			   float32 TopU2,								// top u2
			   float32 TopV2,								// top v2
			   float32 MidU1,								// mid u
			   float32 MidV1,								// mid v
			   float32 MidU2,								// mid u2
			   float32 MidV2,								// mid v2
			   float32 BotU1,								// bot u
			   float32 BotV1,								// bot v
			   float32 BotU2,								// bot u2
			   float32 BotV2 );								// bot v2

			~CDropdown( void );

	void	Create( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	void	Update( const POINT& MousePosition );	// current screen mouse position 


	void	Render( LPDIRECT3DDEVICE8 pD3DDevice );		// d3d device

	int32	UpdateButtonPress( const POINT& MousePosition,	// current screen mouse position 
							   bool ButtonPressed );		// is the left mouse button pressed?

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool			IsOver( const POINT& MousePosition );	// current screen mouse position 

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	
	bool			IsOpen( void )							{	return m_Open;					};

	void			Close( void )							{	m_Open = kFalse;				};

	void			Open( void )							{	m_Open = kTrue;					};

	void			ToggleOpen( void )						{	m_Open = !m_Open;				};

	void			UpdateElements()						{	m_UpdateElements = kTrue;		};

	void			AddElement( const std::string& Entry,
								int32 Code )				
					{	
						m_EntryText.push_back( Entry );
						m_EntryCode.push_back( Code );
						m_UpdateElements = kTrue;
					}

	void			ClearElements( void )	
					{
						m_EntryText.clear();
						m_EntryCode.clear();
						m_UpdateElements = kTrue;
					}

protected:


	CButton*		AddButton( CButton* pButton );			// pointer to button to add to internal list

	void			RemoveButton( CButton* pButton );		// pointer to button to remove from internal list

	void			UpdateButtons( float32 TimeElapsed );		// time elapsed in seconds


	void			DestroyButtons();

	void			RenderButtons( LPDIRECT3DDEVICE8 pD3DDevice );	// d3d device


	CLList<CButton>*		m_pButtonList;

	CSprite*				m_pTopSprite;
	CSprite*				m_pBotSprite;
	CSprite*				m_pMidSprite;

	bool					m_UpdateElements;		// onscreen info has been changed, and text/ui should be recreated

	float32					m_X;
	float32					m_Y;

	float32					m_TextOffsetX;
	float32					m_TextOffsetY;

	float32					m_Width;
	float32					m_TopHeight;
	float32					m_BottomHeight;
	float32					m_ItemHeight;

	float32					m_TopU1;
	float32					m_TopV1;
	float32					m_TopU2;
	float32					m_TopV2;
	float32					m_MidU1;
	float32					m_MidV1;
	float32					m_MidU2;
	float32					m_MidV2;
	float32					m_BotU1;
	float32					m_BotV1;
	float32					m_BotU2;
	float32					m_BotV2;

	float32					m_TextScale;

	CFontMetric*			m_pFontMetric;
	CMaterial*				m_pFontMaterial;
	CMaterial*				m_pSpriteMaterial;

	std::vector< std::string >	m_EntryText;
	std::vector< int32 >		m_EntryCode;

	bool					m_Open;
};

#endif