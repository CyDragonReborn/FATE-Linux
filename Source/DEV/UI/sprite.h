/*
	sprite.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#ifndef _SPRITE_H
#define _SPRITE_H

#include <string>
#include "../UTILITIES/constants.h"

class CMaterial;

// possible centerpoints for a sprite
enum ESpriteCenter
{
	KSpriteTopLeft,
	KSpriteCenter
};

// CSprite objects are comprised of two tris and are generally used as UI elements
class CSprite
{
public:

							CSprite( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
									 CMaterial* pMaterial,				// material of the sprite texture we want to use
									 float32 X,							// x location onscreen (pixel coords from top left)
									 float32 Y,							// y location onscreen (pixel coords from top left)
									 float32 Width,						// width of the sprite, in pixels
									 float32 Height,					// height of the sprite, in pixels
									 float32 TopLeftU,					// u texture coord at top left 
									 float32 TopLeftV,					// v texture coord at top left 
									 float32 BottomRightU,				// u texture coord at bottom right 
									 float32 BottomRightV,				// v texture coord at bottom right
									 ESpriteCenter CenterPoint,			// positioning of the sprite
								     uint32 ScreenWidth = 1024,			// screen width used in orthographic projection
								     uint32 ScreenHeight = 768 );		// screen height used in orthographic projection
							~CSprite( void );
	
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

	void					Render( LPDIRECT3DDEVICE8 pD3DDevice, 				// our direct3d device
									float32 X,									// screen x
									float32 Y,									// screen y position
								    uint32 ScreenWidth = 1024,					// screen width used in orthographic projection
								    uint32 ScreenHeight = 768 );				// screen height used in orthographic projection

	void					RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice, 				// our direct3d device
											 float32 X,									// screen x
											 float32 Y,									// screen y position
											 uint32 ScreenWidth = 1024,					// screen width used in orthographic projection
											 uint32 ScreenHeight = 768 );				// screen height used in orthographic projection

	void					RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice, 				// our direct3d device
											 const D3DXMATRIX& Transformation );			// transformation for the sprite

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	float32					GetX( void )		{	return m_X;	};
	float32					GetY( void )		{	return m_Y;	};

	// pixel-space coordinates
	float32					GetScreenX( void )	{	return m_ScreenX;	};
	float32					GetScreenY( void )	{	return m_ScreenY;	};

	float32					Width( void )		{	return m_Width;		};
	float32					Height( void )		{	return m_Height;	};

	CMaterial*				Material( void )	{	return m_pMaterial;		};

	const D3DXMATRIX&		Transformation( void )	{	return m_Transformation;	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void					SetMaterial( CMaterial* pMaterial )	{	m_pMaterial = pMaterial;	};

	void					SetPosition( float32 X,							// x location onscreen (pixel coords from top left)
										 float32 Y ); 						// y location onscreen (pixel coords from top left)

	void					SetPosition( float32 X,							// x location onscreen (pixel coords from top left)
										 float32 Y, 						// y location onscreen (pixel coords from top left)
										 uint32 ScreenWidth,				// screen width used in orthographic projection
										 uint32 ScreenHeight );				// screen height used in orthographic projection

	void					SetUVs( float32 TopLeftU,					// u texture coord at top left 
								    float32 TopLeftV,					// v texture coord at top left 
									float32 BottomRightU,				// u texture coord at bottom right 
									float32 BottomRightV );				// v texture coord at bottom right

	void					SetSize( float32 Width,						// width of the sprite, in pixels
									 float32 Height );					// height of the sprite, in pixels

	void					SetZRotation( float32 ZRotation )			{	m_ZRotation = ZRotation;		};

private:
	ESpriteCenter			m_CenterPoint;

	float32					m_ZRotation;

	float32					m_ScreenX;
	float32					m_ScreenY;

	float32					m_X;
	float32					m_Y;

	float32					m_Width;
	float32					m_Height;

	uint32					m_ScreenWidth;
	uint32					m_ScreenHeight;

	CMaterial*				m_pMaterial;
	LPDIRECT3DVERTEXBUFFER8	m_pVB;

	D3DXMATRIX				m_Transformation;
};


#endif