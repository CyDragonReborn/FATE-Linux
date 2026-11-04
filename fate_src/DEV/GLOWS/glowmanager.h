/*
	glowmanager.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/23/2004
	
*/

#ifndef _GLOWMANAGER_H
#define _GLOWMANAGER_H

#include "d3dx8_shim.h"

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"

class CSettings;
class CRefManager;
class CMaterial;
class CFrustum;
class CGlowSystem;
class CGlow;


// manager all the glow systems used, and their textures
class CGlowManager
{
public:


				CGlowManager( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
						      CRefManager& pRefManager,			// the reference manager, for tracking media we create
							  CSettings& pSettings, 			// the settings manager, for keeping track of card/machine capabilities
							  const std::string& Path ) ;		// path to glow definition file
				~CGlowManager( void );


	void	Update( float32 TimeElapsed );					// time elapsed in seconds

	CGlow* 	AddGlow( uint32 Type,						// type of glow to add
					 const D3DXVECTOR3& Position,		// position to create at
					 float32 Scale = 1 );							// scale of glow

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

	int32	GetGlowType( std::string Name );	// name of glow to look for

private:

	void	Load( const std::string& Path );		// path to template

	void	CreateGlowSystems( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void	DestroyGlowSystems( void );

	CSettings&			m_pSettings;
	CRefManager&		m_pRefManager;

	// glow systems

	std::vector< CGlowSystem* >			m_pGlowSystem;

	std::vector< CMaterial* >			m_pGlowTexture;

	uint32							m_GlowTypes;

	std::vector< std::string >		m_Name;
	std::vector< std::string >		m_Texture;
	std::vector< std::string >		m_TextureAlpha;
	std::vector< std::string >		m_TextureIllumination;
	std::vector< float32 >			m_Size;
	std::vector< float32 >			m_FadeSpeed;
	std::vector< uint32 >			m_Frames;
	std::vector< float32 >			m_FrameSpeed;
	std::vector< float32 >			m_ScaleSpeed;
	std::vector< float32 >			m_MinRotationSpeed;
	std::vector< float32 >			m_MaxRotationSpeed;
	std::vector< float32 >			m_MinScaleMagnitude;
	std::vector< float32 >			m_MaxScaleMagnitude;
	std::vector< float32 >			m_MinFadeMagnitude;
	std::vector< float32 >			m_MaxFadeMagnitude;
	std::vector< float32 >			m_MinStartingScale;
	std::vector< float32 >			m_MaxStartingScale;
	std::vector< float32 >			m_MinStartingZOrientation;
	std::vector< float32 >			m_MaxStartingZOrientation;
	std::vector< float32 >			m_MinStartingOpacity;
	std::vector< float32 >			m_MaxStartingOpacity;
	std::vector< uint32 >			m_MinStartingFrame;
	std::vector< uint32 >			m_MaxStartingFrame;
	std::vector< bool >				m_LightBlur;

};

#endif