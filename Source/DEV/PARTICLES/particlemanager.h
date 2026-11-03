/*
	particlemanager.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/23/2004
	
*/

#ifndef _PARTICLEMANAGER_H
#define _PARTICLEMANAGER_H

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"

class CSettings;
class CRefManager;
class CMaterial;
class CFrustum;
class CParticleSystem;
class CParticle;

const float32 KFrequencyMultiplier[] = { .25f, .75f, 1.0f };

// manager all the glow systems used, and their textures
class CParticleManager
{
public:


				CParticleManager( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
						      CRefManager& pRefManager,			// the reference manager, for tracking media we create
							  CSettings& pSettings, 			// the settings manager, for keeping track of card/machine capabilities
							  const std::string& Path ) ;		// path to particle definition file
				~CParticleManager( void );


	void	Update( float32 TimeElapsed );					// time elapsed in seconds

	void	CParticleManager::AddParticle( uint32 Type,								// type of particle to add
										   const D3DXMATRIX& Transformation,		// transformation
										   const D3DXVECTOR3& Velocity = D3DXVECTOR3( 0, 0, 0 ), // velocity of particle
										   float32 Scale = 1);							// scale of particle

	void	CParticleManager::AddParticle( uint32 Type,						// type of particle to add
										   const D3DXVECTOR3& Position,		// position to create at
										   const D3DXVECTOR3& Velocity = D3DXVECTOR3( 0, 0, 0 ), // velocity of particle
										   float32 Scale = 1 );							// scale of particle

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
					const D3DXMATRIX& ViewportMatrix,	// camera's viewport matrix
					CFrustum* Frustum = NULL );			// frustum to cull to

	void	RenderAdditive( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
							const D3DXMATRIX& ViewportMatrix,	// camera's viewport matrix
							CFrustum* Frustum = NULL );			// frustum to cull to

	void	Clear( void );

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	int32	GetParticleType( std::string Name );	// name of particle to look for

private:

	void	Load( const std::string& Path );		// path to template

	void	CreateParticleSystems( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	DestroyParticleSystems( void );

	CSettings&			m_pSettings;
	CRefManager&		m_pRefManager;

	// glow systems

	std::vector< CParticleSystem* >			m_pParticleSystem;

	std::vector< CMaterial* >			m_pParticleTexture;

	uint32							m_ParticleTypes;

	std::vector< std::string >		m_Name;
	std::vector< float32 >			m_Frequency;
	std::vector< std::string >		m_Texture;
	std::vector< std::string >		m_TextureAlpha;
	std::vector< std::string >		m_TextureIllumination;
	std::vector< bool >				m_Multiply;
	std::vector< float32 >			m_Size;
	std::vector< uint32 >			m_MaxParticles;
	std::vector< float32 >			m_FadeSpeed;
	std::vector< uint32 >			m_Frames;
	std::vector< float32 >			m_FrameSpeed;
	std::vector< D3DXVECTOR3 >		m_Gravity;
	std::vector< float32 >			m_ScaleSpeed;
	std::vector< float32 >			m_MinRotationSpeed;
	std::vector< float32 >			m_MaxRotationSpeed;
	std::vector< D3DXVECTOR3 >		m_MinStartingOffset;
	std::vector< D3DXVECTOR3 >		m_MaxStartingOffset;
	std::vector< D3DXVECTOR3 >		m_MinStartingVelocity;
	std::vector< D3DXVECTOR3 >		m_MaxStartingVelocity;
	std::vector< float32 >			m_MinStartingScale;
	std::vector< float32 >			m_MaxStartingScale;
	std::vector< float32 >			m_MinStartingZOrientation;
	std::vector< float32 >			m_MaxStartingZOrientation;
	std::vector< float32 >			m_MinStartingOpacity;
	std::vector< float32 >			m_MaxStartingOpacity;
	std::vector< float32 >			m_MinLifetime;
	std::vector< float32 >			m_MaxLifetime;
	std::vector< uint32 >			m_MinAnimationLifetime;
	std::vector< uint32 >			m_MaxAnimationLifetime;
	std::vector< uint32 >			m_MinStartingFrame;
	std::vector< uint32 >			m_MaxStartingFrame;
	std::vector< bool >				m_LightBlur;
	std::vector< bool >				m_DoubleSided;
	std::vector< bool >				m_Directed;

};

#endif