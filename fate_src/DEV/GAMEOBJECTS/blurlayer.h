/*
	blurlayer.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	6/23/2004

*/
#ifndef _BLURLAYER_H
#define _BLURLAYER_H

#include "../UTILITIES/constants.h"
#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

class CSettings;

// the blur layer is used for fullscreen 'glow' trails. Each frame, the glows are rendered
// to this layer. This layer is then rendered over the top of the finished scene for a blurring effect
class CBlurLayer
{
public:

				CBlurLayer( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device						  
							CRefManager& pRefManager,			// reference manager
							CSettings& pSettings,				// the settings manager, for keeping track of card/machine capabilities
							const std::string& BlurMaskPath,		// path to a blur mask texture (can be "" - FrameFade will be used in stead)
							const std::string& BlurMaskPathAlpha,	// path to a blur mask texture alpha (can be "" - FrameFade will be used in stead)
							uint32 Size,						// dimensions of the texture
							float32 FrameFade );				// alpha amount to 'fade' the blur each frame
				~CBlurLayer( void );

	void		RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									  CSettings& pSettings );
	
	void		BeginRender( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	void		BeginRenderHalfScreen( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	void		EndRender( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	void		RenderBlur( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device

	void		RenderBlurLeft( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device

	void		RenderBlurRight( LPDIRECT3DDEVICE8 pD3DDevice );				// direct3d device

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void		SetFrameFade( float32 Alpha );	// alpha amount to fade each frame ( 0 is none, 1 is all )

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool		IsValid( void );
private:
	
	void		RenderFade( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

	LPDIRECT3DVERTEXBUFFER8	m_pVertexBuffer;
	LPDIRECT3DVERTEXBUFFER8	m_pVertexBufferLeftHalf;
	LPDIRECT3DVERTEXBUFFER8	m_pVertexBufferRightHalf;
	LPDIRECT3DVERTEXBUFFER8	m_pMaskVertexBuffer;

	// texture for the blur layer render
	CMaterial*				m_pTexture;
	// texture to mask the previous renders and control fading ( not required )
	CMaterial*				m_pBlurTexture;

	CSettings&				m_pSettings;		
	CRefManager&			m_pRefManager;

	float32					m_FrameFade;	// alpha amount to fade each frame ( 0 is none, 1 is all )

	float32					m_ScreenWidth;
	float32					m_ScreenHeight;

	uint32					m_TextureSize;	// size of the texture we will render the blur to
};

#endif