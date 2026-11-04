/*
	particlesystem.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#ifndef _PARTICLESYSTEM_H
#define _PARTICLESYSTEM_H

#include "d3dx8_shim.h"

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"

class CSettings;

class CParticle
{
public:
	CParticle( const D3DXVECTOR3& Position,		// starting position of the particle in space
			   const D3DXVECTOR3& Velocity,		// starting velocity of the particle 
			   float32 Scale,					// scale of the particle (multiplied by the particle system's base scale)
			   float32 MaxLifeTime,				// how long, in seconds, the particle should exist
			   float32 ZRotation,				// starting rotation about z axis in degree
			   float32 ZRotationSpeed,			// speed in degrees per second to rotate about z axis
			   uint32 StartingFlipbookFrame,	// starting animation frame (if applicable
			   uint32 AnimationCycleLifetime,	// 0 to ignore, or a uint to ignore lifetime, and destroy after specified number of animation cycles
			   uint32 StartingOpacity ) :		// starting particle opacity (if system supports) from 0-255
											m_Position( Position ),
											m_Velocity( Velocity ),
											m_Scale( Scale ),
											m_MaxLifeTime( MaxLifeTime ),
											m_ZRotation( ZRotation ),
											m_ZRotationSpeed( ZRotationSpeed ),
											m_FlipbookFrame( (float32)StartingFlipbookFrame ),
											m_AnimationCycleLifetime( AnimationCycleLifetime ),
											m_Opacity( (float32)StartingOpacity ),
											m_AnimationLoops( 0 ),
											m_LifeTime( 0 ),
											m_IsMoving( kFalse )
	{
		if( D3DXVec3Length( &m_Velocity ) > 0 )
		{
			m_IsMoving = kTrue;
		}
	};
	D3DXVECTOR3	m_Position;
	D3DXVECTOR3 m_Velocity;

	float32		m_Scale;
	float32		m_LifeTime;
	float32		m_MaxLifeTime;
	float32		m_ZRotation;
	float32		m_ZRotationSpeed;
	float32		m_Opacity;

	float32		m_FlipbookFrame;
	uint32		m_AnimationCycleLifetime;
	uint32		m_AnimationLoops;

	bool		m_IsMoving;

};

// a particle system manages the motion and data of individual quads in space
// it does NOT manage texture or material data. The particle system is rendered
// at once, and any material or blending settings should be done before the system
// is rendered. For this reason, each system supports only ONE texture.
// Therefore, for each type of particle, you would have a particle system.
class CParticleSystem
{
public:


				CParticleSystem( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
								 CSettings& pSettings,				// settings object, to let us know what we can do
								 uint32 MaximumParticles,			// maximum particles the system can hold
								 float32 ParticleWidthHeight,		// width and height of basic particles, in units
								 const D3DXVECTOR3& Gravity,		// Direction and speed of fall, in units per second
								 float32 ScaleSpeed,				// change in scale in units per second
								 float32 FadeSpeed,					// change in opacity per second - 0 means no fading - opacity is 0-255, so a value of -255 would be totally transparent in 1 second
								 uint32 FlipbookWidthHeight,		// if this is an animated particle, how many frames wide/high is our animation sheet
								 float32 SecondsPerFlipbookFrame,	// how much time is spent on each frame of a flipbook animation
								 bool Multiply,						// multiply blend?
								 bool DoubleSided = kFalse );		// double sided sprite?
				~CParticleSystem( void );

	void		Add( CParticle* pParticle );		// particle to add
	void		Clear( void );

	void		Update( float32 TimeElapsed );		// time elapsed in seconds
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
						const D3DXMATRIX& ViewportMatrix,		// viewport matrix
					    CFrustum* pFrustum );					// frustum to cull against (NULL for no culling)

    bool		CreateVB( LPDIRECT3DDEVICE8 pD3DDevice );	// our direct3d device

	// by default, fading particles fade their alpha value - 
	// if you want to fade out an additive particle, you need to setadditivefading to true
	// additive fading has to fade out the color value
	void		SetAdditiveFading( bool AdditiveFading )	{	m_AdditiveFading = AdditiveFading;	};

	// directed particles don't point at the camera, but along their heading
	void		SetDirected( bool State )					{	m_Directed = State;				};

	uint32		ActiveParticles( void )						{	return m_ActiveParticles;		};

protected:
	CSettings&			m_pSettings;

	D3DXVECTOR3			m_Gravity;
	
	uint32				m_ActiveParticles;
	uint32				m_MaximumParticles;
	uint32				m_FlipbookWidthHeight;
	uint32				m_FlipbookFrames;

	float32				m_ParticleWidthHeight;
	float32				m_FlipbookFrameUVWidth;
	float32				m_ScaleSpeed;
	float32				m_FadeSpeed;
	float32				m_SecondsPerFlipbookFrame;

	bool				m_SystemValid;
	bool				m_IsAnimated;
	bool				m_HasGravity;

	bool				m_AdditiveFading;

	bool				m_Multiply;


	CLList<CParticle>*	m_pParticleList;

	
	bool				m_Directed;
	bool				m_DoubleSided;

	LPDIRECT3DVERTEXBUFFER8 m_pVB;
};

#endif