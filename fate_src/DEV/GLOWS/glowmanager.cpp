/*
	glowmanager.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/23/2004
	
	CHANGE HISTORY

	Travis Baldree
	9/14/2004	-	changed to support an external glow definition file
*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/datafile.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "glowsystem.h"
#include "glowmanager.h"

CGlowManager::CGlowManager( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
							CRefManager& pRefManager,		// the reference manager, for tracking media we create
							CSettings& pSettings,			// the settings manager, for keeping track of card/machine capabilities
							const std::string& Path ) :		// path to glow definition file
							m_pRefManager( pRefManager ),
							m_pSettings( pSettings ),
							m_GlowTypes( 0 )
{
	Load( Path );
	CreateGlowSystems( pD3DDevice );
} // CGlowManager::CGlowManager()

CGlowManager::~CGlowManager( void )
{
	DestroyGlowSystems();
} // CGlowManager::~CGlowManager()

void CGlowManager::Load( const std::string& Path )		// path to template
{

	CDataFile* pDataFile = new CDataFile( Path );

	uint32 GlowTypes = pDataFile->DataGroups( "GLOW" );

	for( uint32 i = 0; i < GlowTypes; i++ )
	{
		std::string Name( "" );
		bool		Blur( kFalse );
		std::string Texture( "" );
		std::string TextureAlpha( "" );
		std::string TextureIllumination( "" );
		float32		Size( 1 );
		uint32		Frames( 0 );
		float32		FrameSpeed( .1f );
		float32		ScaleSpeed( 0 );
		float32		FadeSpeed( 0 );
		float32		MinScaleMagnitude( 1 );
		float32		MaxScaleMagnitude( 1 );
		float32		MinFadeMagnitude( 1 );
		float32		MaxFadeMagnitude( 1 );
		float32		MinRotationSpeed( 0 );
		float32		MaxRotationSpeed( 0 );
		float32		MinStartingScale( 1 );
		float32		MaxStartingScale( 1 );
		float32		MinStartingZOrientation( 1 );
		float32		MaxStartingZOrientation( 1 );
		float32		MinStartingOpacity( 255 );
		float32		MaxStartingOpacity( 255 );
		uint32		MinStartingFrame( 0 );
		uint32		MaxStartingFrame( 0 );

		// parse tile widths
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "GLOW", i );

		CDataGroup* pGroup = pMasterGroup->GetSubDataGroup( "NAME" );
		if( pGroup != NULL )
		{
			Name = pGroup->Item( 0 );
		}
			
		pGroup = pMasterGroup->GetSubDataGroup( "TEXTURE" );
		if( pGroup != NULL )
		{
			Texture = pGroup->Item( 0 );
			if( StringUpper( Texture ) == "NONE" )
			{
				Texture = "";
			}
			TextureAlpha = pGroup->Item( 1 );
			if( StringUpper( TextureAlpha ) == "NONE" )
			{
				TextureAlpha = "";
			}
			TextureIllumination = pGroup->Item( 2 );
			if( StringUpper( TextureIllumination ) == "NONE" )
			{
				TextureIllumination = "";
			}
		}

		pGroup = pMasterGroup->GetSubDataGroup( "BLUR" );
		if( pGroup != NULL )
		{
			Blur = atoi( pGroup->Item( 0 ).c_str() ) == 1;
		}

		pGroup = pMasterGroup->GetSubDataGroup( "SIZE" );
		if( pGroup != NULL )
		{
			Size = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "FADE_SPEED" );
		if( pGroup != NULL )
		{
			FadeSpeed = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "FRAMES" );
		if( pGroup != NULL )
		{
			Frames = (uint32)atoi( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "FRAME_SPEED" );
		if( pGroup != NULL )
		{
			FrameSpeed = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "SCALE_SPEED" );
		if( pGroup != NULL )
		{
			ScaleSpeed = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_ROTATION_SPEED" );
		if( pGroup != NULL )
		{
			MinRotationSpeed = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_ROTATION_SPEED" );
		if( pGroup != NULL )
		{
			MaxRotationSpeed = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_SCALE_MAGNITUDE" );
		if( pGroup != NULL )
		{
			MinScaleMagnitude = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_SCALE_MAGNITUDE" );
		if( pGroup != NULL )
		{
			MaxScaleMagnitude = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_FADE_MAGNITUDE" );
		if( pGroup != NULL )
		{
			MinFadeMagnitude = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_FADE_MAGNITUDE" );
		if( pGroup != NULL )
		{
			MaxFadeMagnitude = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_STARTING_SCALE" );
		if( pGroup != NULL )
		{
			MinStartingScale = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_STARTING_SCALE" );
		if( pGroup != NULL )
		{
			MaxStartingScale = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_STARTING_ZORIENTATION" );
		if( pGroup != NULL )
		{
			MinStartingZOrientation = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_STARTING_ZORIENTATION" );
		if( pGroup != NULL )
		{
			MaxStartingZOrientation = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_STARTING_OPACITY" );
		if( pGroup != NULL )
		{
			MinStartingOpacity = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_STARTING_OPACITY" );
		if( pGroup != NULL )
		{
			MaxStartingOpacity = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_STARTING_FRAME" );
		if( pGroup != NULL )
		{
			MinStartingFrame = (uint32)atoi( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_STARTING_FRAME" );
		if( pGroup != NULL )
		{
			MaxStartingFrame = (uint32)atoi( pGroup->Item( 0 ).c_str() );
		}

		m_Name.push_back( StringUpper( Name ) );
		m_Texture.push_back( Texture );
		m_TextureAlpha.push_back( TextureAlpha );
		m_TextureIllumination.push_back( TextureIllumination );
		m_Size.push_back( Size );
		m_FadeSpeed.push_back( FadeSpeed );
		m_Frames.push_back( Frames );
		m_FrameSpeed.push_back( FrameSpeed );
		m_ScaleSpeed.push_back( ScaleSpeed );
		m_MinScaleMagnitude.push_back( MinScaleMagnitude );
		m_MaxScaleMagnitude.push_back( MaxScaleMagnitude );
		m_MinFadeMagnitude.push_back( MinFadeMagnitude );
		m_MaxFadeMagnitude.push_back( MaxFadeMagnitude );
		m_MinRotationSpeed.push_back( MinRotationSpeed );
		m_MaxRotationSpeed.push_back( MaxRotationSpeed );
		m_MinStartingScale.push_back( MinStartingScale );
		m_MaxStartingScale.push_back( MaxStartingScale );
		m_MinStartingZOrientation.push_back( MinStartingZOrientation );
		m_MaxStartingZOrientation.push_back( MaxStartingZOrientation );
		m_MinStartingOpacity.push_back( MinStartingOpacity );
		m_MaxStartingOpacity.push_back( MaxStartingOpacity );
		m_MinStartingFrame.push_back( MinStartingFrame );
		m_MaxStartingFrame.push_back( MaxStartingFrame );
		m_LightBlur.push_back( Blur );
	}

	m_GlowTypes = m_Name.size();

	DELETE_SAFELY( pDataFile );
} // CGlowManager::Load()

void CGlowManager::CreateGlowSystems( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	// initialize a glow system and the material we will use to render it
	for( uint32 i = 0; i < m_GlowTypes;  i++ )
	{
		if( m_Texture[i].length() > 0 ||
			m_TextureIllumination[i].length() > 0 )
		{
			m_pGlowTexture.push_back( m_pRefManager.AddMaterial( pD3DDevice, 
																	m_Texture[i],
																	m_TextureAlpha[i],
																	m_TextureIllumination[i],
																	"",
																	"" ) );
		}
		else
		{
			m_pGlowTexture.push_back( NULL );
		}

		CGlowSystem* pSystem = new CGlowSystem( pD3DDevice,
														  m_pSettings,
														  m_Size[i], // width of standard glow
														  m_ScaleSpeed[i],
														  m_FadeSpeed[i],
														  m_MinScaleMagnitude[i],
														  m_MaxScaleMagnitude[i],
														  m_MinFadeMagnitude[i],
														  m_MaxFadeMagnitude[i],
														  m_Frames[i],
														  m_FrameSpeed[i] );
			if( m_TextureIllumination[i].length() > 0 &&
				m_FadeSpeed[i] != 0 )
			{
				pSystem->SetAdditiveFading( kTrue );
			}

		m_pGlowSystem.push_back( pSystem );
	}

} // CGlowManager::CreateGlowSystems()

void CGlowManager::DestroyGlowSystems( void )
{
	for( uint32 i = 0; i < m_pGlowSystem.size();  i++ )
	{
		m_pRefManager.RemoveMaterial( m_pGlowTexture[i] );

		DELETE_SAFELY( m_pGlowSystem[i] );
	}
	m_pGlowSystem.clear();

} // CGlowManager::DestroyGlowSystems()

int32 CGlowManager::GetGlowType( std::string Name )	// name of glow to look for
{
	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_GlowTypes; i++ )
	{
		if( Name == m_Name[i] )
		{
			return i;
		}
	}
	return -1;
} // CGlowManager::GetGlowType()

CGlow* CGlowManager::AddGlow( uint32 Type,					// type of glow to add
							  const D3DXVECTOR3& Position,	// position to create at
							  float32 Scale )						// scale of glow
{



	return m_pGlowSystem[Type]->Add( new CGlow( Position,
												 RandomBetween( m_MinStartingScale[Type], m_MaxStartingScale[Type] ) * Scale,
												 RandomBetween( m_MinStartingZOrientation[Type], m_MaxStartingZOrientation[Type] ),
												 RandomBetween( m_MinRotationSpeed[Type], m_MaxRotationSpeed[Type] ),
												 RandomBetween( m_MinStartingScale[Type], m_MaxStartingScale[Type] ),
												 (uint32)RandomBetween( (float32)m_MinStartingFrame[Type], (float32)m_MaxStartingFrame[Type] ),
												 RandomBetween( m_MinStartingOpacity[Type], m_MaxStartingOpacity[Type] ) ) );
} // CGlowManager::AddGlow()

void CGlowManager::Clear( void )
{
	// update our glow systems
	for( uint32 i = 0; i < m_pGlowSystem.size(); i++ )
	{
		m_pGlowSystem[i]->Clear();
	}
} // CGlowManager::Update()

void CGlowManager::Update( float32 TimeElapsed )	// time elapsed in seconds
{
	// update our glow systems
	for( uint32 i = 0; i < m_pGlowSystem.size(); i++ )
	{
		m_pGlowSystem[i]->Update( TimeElapsed );
	}
} // CGlowManager::Update()

void CGlowManager::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
							   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							   CFrustum* pFrustum )						// frustum to cull against (NULL for no culling)

{
	// make sure we're alpha blending, and turn on additive blending
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING,  kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	/////////////////////////////////////////////////////////////////
	// ALPHA GLOWS
	/////////////////////////////////////////////////////////////////

	for( uint32 i = 0; i < m_pGlowSystem.size(); i++ )
	{
		if( m_pGlowTexture[i] != NULL &&
			m_pGlowTexture[i]->IsValid() &&
			m_pGlowSystem[i]->ActiveGlows() > 0 &&
			!m_pGlowTexture[i]->HasIllumination() )
		{
			// set our glow system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pGlowTexture[i]->GetTexture() );

			m_pGlowSystem[i]->Render( pD3DDevice, 
										  ViewportMatrix, 
										  pFrustum );
		}
	}

	// now we do additively blended glows
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	/////////////////////////////////////////////////////////////////
	// ADDITIVE GLOWS
	/////////////////////////////////////////////////////////////////

	for( i = 0; i < m_pGlowSystem.size(); i++ )
	{
		if( m_pGlowTexture[i] != NULL &&
			m_pGlowTexture[i]->IsValid() &&
			m_pGlowSystem[i]->ActiveGlows() > 0 &&
			m_pGlowTexture[i]->HasIllumination() )
		{
			// set our glow system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pGlowTexture[i]->GetIlluminationTexture() );

			m_pGlowSystem[i]->Render( pD3DDevice, 
										  ViewportMatrix, 
										  pFrustum );
		}
	}

	// back to standard settings
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );

} // CGlowManager::Render()

// render ONLY glows that are additively blended
void CGlowManager::RenderAdditive( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
									   const D3DXMATRIX& ViewportMatrix,		// viewport matrix
									   CFrustum* pFrustum )						// frustum to cull against (NULL for no culling)

{
	// make sure we're alpha blending, and turn on additive blending
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING,  kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	/////////////////////////////////////////////////////////////////
	// ADDITIVE GLOWS
	/////////////////////////////////////////////////////////////////

	for( uint32 i = 0; i < m_pGlowSystem.size(); i++ )
	{
		if( m_pGlowTexture[i] != NULL &&
			m_pGlowTexture[i]->IsValid() &&
			m_pGlowSystem[i]->ActiveGlows() > 0 &&
			m_pGlowTexture[i]->HasIllumination() &&
			m_LightBlur[i] )
		{
			// set our glow system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pGlowTexture[i]->GetIlluminationTexture() );

			m_pGlowSystem[i]->Render( pD3DDevice, 
										  ViewportMatrix, 
										  pFrustum );
		}
	}

	// back to standard settings
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );

} // CGlowManager::RenderAdditive()
