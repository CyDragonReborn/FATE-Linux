/*
	glowsystem.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#ifndef _GLOWSYSTEM_H
#define _GLOWSYSTEM_H

#include "d3dx8_shim.h"

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"

class CSettings;

class CGlow
{
public:
	CGlow( const D3DXVECTOR3& Position,		// starting position of the glow in space
		   float32 Scale,					// scale of the glow (multiplied by the glow system's base scale)
		   float32 ZRotation,				// starting rotation about z axis in degree
		   float32 ZRotationSpeed,			// speed in degrees per second to rotate about z axis
		   float32 ScaleOffset,				// scale offset
		   uint32 StartingFlipbookFrame,	// starting animation frame (if applicable
		   float32 StartingOpacity ) :		// starting glow opacity (if system supports) from 0-1
										m_Position( Position ),
										m_Scale( Scale ),
										m_ScaleOffset( ScaleOffset ),
										m_ZRotation( ZRotation ),
										m_ZRotationSpeed( ZRotationSpeed ),
										m_FlipbookFrame( (float32)StartingFlipbookFrame ),
										m_RequestingRemoval( kFalse ),
										m_Opacity( 255 ),
										m_OpacityOffset( StartingOpacity )
	{
	};
	D3DXVECTOR3	m_Position;

	float32		m_Scale;
	float32		m_ScaleOffset;
	float32		m_ZRotation;
	float32		m_ZRotationSpeed;
	float32		m_Opacity;
	float32		m_OpacityOffset;

	float32		m_FlipbookFrame;	
	bool		m_RequestingRemoval;

};

// a glow system manages the motion and data of individual quads in space
// it does NOT manage texture or material data. The glow system is rendered
// at once, and any material or blending settings should be done before the system
// is rendered. For this reason, each system supports only ONE texture.
// Therefore, for each type of glow, you would have a glow system.
class CGlowSystem
{
public:


				CGlowSystem( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
								 CSettings& pSettings,			// settings object, to let us know what we can do
								 float32 GlowWidthHeight,		// width and height of basic glows, in units
								 float32 ScaleSpeed,			// change in scale in units per second
								 float32 FadeSpeed,				// change in opacity per second - 0 means no fading - opacity is 0-255, so a value of -255 would be totally transparent in 1 second
								 float32 MinScaleMagnitude,		//	min scale
								 float32 MaxScaleMagnitude,		//	max scale
								 float32 MinFadeMagnitude,		//	min fade
								 float32 MaxFadeMagnitude,		//	max fade
								 uint32 FlipbookWidthHeight,		// if this is an animated glow, how many frames wide/high is our animation sheet
								 float32 SecondsPerFlipbookFrame );	// how much time is spent on each frame of a flipbook animation
				~CGlowSystem( void );

	CGlow*		Add( CGlow* pGlow );		// glow to add
	void		Clear( void );

	void		Update( float32 TimeElapsed );		// time elapsed in seconds
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
						const D3DXMATRIX& ViewportMatrix,		// viewport matrix
					    CFrustum* pFrustum );					// frustum to cull against (NULL for no culling)

    bool		CreateVB( LPDIRECT3DDEVICE8 pD3DDevice );	// our direct3d device

	// by default, fading glows fade their alpha value - 
	// if you want to fade out an additive glow, you need to setadditivefading to true
	// additive fading has to fade out the color value
	void		SetAdditiveFading( bool AdditiveFading )	{	m_AdditiveFading = AdditiveFading;	};


	uint32		ActiveGlows( void )							{	return m_ActiveGlows;		};

protected:
	CSettings&			m_pSettings;

	D3DXVECTOR3			m_Gravity;
	
	uint32				m_ActiveGlows;
	uint32				m_FlipbookWidthHeight;
	uint32				m_FlipbookFrames;

	float32				m_MinScaleMagnitude;
	float32				m_MaxScaleMagnitude;
	float32				m_MinFadeMagnitude;
	float32				m_MaxFadeMagnitude;

	float32				m_GlowWidthHeight;
	float32				m_FlipbookFrameUVWidth;
	float32				m_ScaleSpeed;
	float32				m_FadeSpeed;
	float32				m_SecondsPerFlipbookFrame;

	bool				m_SystemValid;
	bool				m_IsAnimated;

	bool				m_AdditiveFading;

	CLList<CGlow>*	m_pGlowList;

	
	LPDIRECT3DVERTEXBUFFER8 m_pVB;
};

#endif