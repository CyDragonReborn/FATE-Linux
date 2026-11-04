/*
	ScreenFade.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/
#ifndef _SCREENFADE_H
#define _SCREENFADE_H

#include "d3dx8_shim.h"

#include "../UTILITIES/constants.h"
class CSettings;

// used as an overlay to fade out/in
class CScreenFade
{
public:

				CScreenFade( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							 CSettings& pSettings );				// the settings manager, for keeping track of card/machine capabilities
				~CScreenFade( void );

	void		RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									  CSettings& pSettings );
	
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice );					// direct3d device

	void		SetColor( float32 R,	// red component
						  float32 G,	// green component
						  float32 B,	// blue component
						  float32 A );	// alpha component
private:
	LPDIRECT3DVERTEXBUFFER8	m_pVertexBuffer;

	D3DXVECTOR4	m_Color;

};

#endif