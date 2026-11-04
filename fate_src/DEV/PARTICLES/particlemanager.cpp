/*
	particlemanager.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/23/2004
	
	CHANGE HISTORY

	Travis Baldree
	9/14/2004	-	changed to support an external particle definition file
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

#include "particlesystem.h"
#include "particlemanager.h"

CParticleManager::CParticleManager( LPDIRECT3DDEVICE8 pD3DDevice,	// our direct3d device
									CRefManager& pRefManager,		// the reference manager, for tracking media we create
									CSettings& pSettings,			// the settings manager, for keeping track of card/machine capabilities
									const std::string& Path ) :		// path to particle definition file
																m_pRefManager( pRefManager ),
																m_pSettings( pSettings ),
																m_ParticleTypes( 0 )
{
	Load( Path );
	CreateParticleSystems( pD3DDevice );
} // CParticleManager::CParticleManager()

CParticleManager::~CParticleManager( void )
{
	DestroyParticleSystems();
} // CParticleManager::~CParticleManager()

void CParticleManager::Load( const std::string& Path )		// path to template
{

	CDataFile* pDataFile = new CDataFile( Path );

	uint32 ParticleTypes = pDataFile->DataGroups( "PARTICLE" );

	for( uint32 i = 0; i < ParticleTypes; i++ )
	{
		std::string Name( "" );
		bool		Blur( kFalse );
		float32		Frequency( .25f );
		std::string Texture( "" );
		std::string TextureAlpha( "" );
		std::string TextureIllumination( "" );
		bool		Multiply( kFalse );
		float32		Size( 1 );
		uint32		MaxParticles( 200 );
		float32		FadeSpeed( 0 );
		uint32		Frames( 0 );
		float32		FrameSpeed( .1f );
		D3DXVECTOR3 Gravity( 0, -10, 0 );
		float32		ScaleSpeed( 0 );
		float32		MinRotationSpeed( 0 );
		float32		MaxRotationSpeed( 0 );
		D3DXVECTOR3	MinStartingVelocity( 0, 0, 0 );
		D3DXVECTOR3	MaxStartingVelocity( 0, 0, 0 );
		D3DXVECTOR3	MinStartingOffset( 0, 0, 0 );
		D3DXVECTOR3	MaxStartingOffset( 0, 0, 0 );
		float32		MinStartingScale( 1 );
		float32		MaxStartingScale( 1 );
		float32		MinStartingZOrientation( 1 );
		float32		MaxStartingZOrientation( 1 );
		float32		MinStartingOpacity( 255 );
		float32		MaxStartingOpacity( 255 );
		float32		MinLifetime( 1 );
		float32		MaxLifetime( 1 );
		uint32		MinAnimationLifetime( 0 );
		uint32		MaxAnimationLifetime( 0 );
		uint32		MinStartingFrame( 0 );
		uint32		MaxStartingFrame( 0 );
		bool		DoubleSided( 0 );
		bool		Directed( 0 );

		// parse tile widths
		CDataGroup* pMasterGroup = pDataFile->GetDataGroup( "PARTICLE", i );

		CDataGroup* pGroup = pMasterGroup->GetSubDataGroup( "NAME" );
		if( pGroup != NULL )
		{
			Name = pGroup->Item( 0 );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "FREQUENCY" );
		if( pGroup != NULL )
		{
			Frequency = (float32)atof( pGroup->Item( 0 ).c_str() );
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
			if( pGroup->Items() >= 4 )
			{
				TextureIllumination = pGroup->Item( 3 );
				if( StringUpper( TextureIllumination ) == "NONE" )
				{
					TextureIllumination = "";
				}
				else
				{
					Multiply = kTrue;
				}
			}
		}

		pGroup = pMasterGroup->GetSubDataGroup( "BLUR" );
		if( pGroup != NULL )
		{
			Blur = atoi( pGroup->Item( 0 ).c_str() ) == 1;
		}

		pGroup = pMasterGroup->GetSubDataGroup( "DOUBLESIDED" );
		if( pGroup != NULL )
		{
			DoubleSided = atoi( pGroup->Item( 0 ).c_str() ) == 1;
		}

		pGroup = pMasterGroup->GetSubDataGroup( "DIRECTED" );
		if( pGroup != NULL )
		{
			Directed = atoi( pGroup->Item( 0 ).c_str() ) == 1;
		}

		pGroup = pMasterGroup->GetSubDataGroup( "SIZE" );
		if( pGroup != NULL )
		{
			Size = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAXIMUM_PARTICLES" );
		if( pGroup != NULL )
		{
			MaxParticles = (uint32)atoi( pGroup->Item( 0 ).c_str() );
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

		pGroup = pMasterGroup->GetSubDataGroup( "GRAVITY" );
		if( pGroup != NULL )
		{
			Gravity.x = (float32)atof( pGroup->Item( 0 ).c_str() );
			Gravity.y = (float32)atof( pGroup->Item( 1 ).c_str() );
			Gravity.z = (float32)atof( pGroup->Item( 2 ).c_str() );
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

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_STARTING_OFFSET" );
		if( pGroup != NULL )
		{
			MinStartingOffset.x = (float32)atof( pGroup->Item( 0 ).c_str() );
			MinStartingOffset.y = (float32)atof( pGroup->Item( 1 ).c_str() );
			MinStartingOffset.z = (float32)atof( pGroup->Item( 2 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_STARTING_OFFSET" );
		if( pGroup != NULL )
		{
			MaxStartingOffset.x = (float32)atof( pGroup->Item( 0 ).c_str() );
			MaxStartingOffset.y = (float32)atof( pGroup->Item( 1 ).c_str() );
			MaxStartingOffset.z = (float32)atof( pGroup->Item( 2 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_STARTING_VELOCITY" );
		if( pGroup != NULL )
		{
			MinStartingVelocity.x = (float32)atof( pGroup->Item( 0 ).c_str() );
			MinStartingVelocity.y = (float32)atof( pGroup->Item( 1 ).c_str() );
			MinStartingVelocity.z = (float32)atof( pGroup->Item( 2 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_STARTING_VELOCITY" );
		if( pGroup != NULL )
		{
			MaxStartingVelocity.x = (float32)atof( pGroup->Item( 0 ).c_str() );
			MaxStartingVelocity.y = (float32)atof( pGroup->Item( 1 ).c_str() );
			MaxStartingVelocity.z = (float32)atof( pGroup->Item( 2 ).c_str() );
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

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_LIFETIME" );
		if( pGroup != NULL )
		{
			MinLifetime = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_LIFETIME" );
		if( pGroup != NULL )
		{
			MaxLifetime = (float32)atof( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MIN_ANIMATION_LIFETIME" );
		if( pGroup != NULL )
		{
			MinAnimationLifetime = (uint32)atoi( pGroup->Item( 0 ).c_str() );
		}

		pGroup = pMasterGroup->GetSubDataGroup( "MAX_ANIMATION_LIFETIME" );
		if( pGroup != NULL )
		{
			MaxAnimationLifetime = (uint32)atoi( pGroup->Item( 0 ).c_str() );
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
		m_Frequency.push_back( Frequency );
		m_Texture.push_back( Texture );
		m_TextureAlpha.push_back( TextureAlpha );
		m_TextureIllumination.push_back( TextureIllumination );
		m_Multiply.push_back( Multiply );
		m_Size.push_back( Size );

		m_MaxParticles.push_back( MaxParticles );
		m_FadeSpeed.push_back( FadeSpeed );
		m_Frames.push_back( Frames );
		m_FrameSpeed.push_back( FrameSpeed );
		m_Gravity.push_back( Gravity );
		m_ScaleSpeed.push_back( ScaleSpeed );
		m_MinRotationSpeed.push_back( MinRotationSpeed );
		m_MaxRotationSpeed.push_back( MaxRotationSpeed );
		m_MinStartingOffset.push_back( MinStartingOffset );
		m_MaxStartingOffset.push_back( MaxStartingOffset );
		m_MinStartingVelocity.push_back( MinStartingVelocity );
		m_MaxStartingVelocity.push_back( MaxStartingVelocity );
		m_MinStartingScale.push_back( MinStartingScale );
		m_MaxStartingScale.push_back( MaxStartingScale );
		m_MinStartingZOrientation.push_back( MinStartingZOrientation );
		m_MaxStartingZOrientation.push_back( MaxStartingZOrientation );
		m_MinStartingOpacity.push_back( MinStartingOpacity );
		m_MaxStartingOpacity.push_back( MaxStartingOpacity );
		m_MinLifetime.push_back( MinLifetime );
		m_MaxLifetime.push_back( MaxLifetime );
		m_MinAnimationLifetime.push_back( MinAnimationLifetime );
		m_MaxAnimationLifetime.push_back( MaxAnimationLifetime );
		m_MinStartingFrame.push_back( MinStartingFrame );
		m_MaxStartingFrame.push_back( MaxStartingFrame );
		m_LightBlur.push_back( Blur );
		m_DoubleSided.push_back( DoubleSided );
		m_Directed.push_back( Directed );
	}

	m_ParticleTypes = m_Name.size();

	DELETE_SAFELY( pDataFile );
} // CParticleManager::Load()

void CParticleManager::CreateParticleSystems( LPDIRECT3DDEVICE8 pD3DDevice )	// direct3d device
{
	// initialize a particle system and the material we will use to render it
	for( uint32 i = 0; i < m_ParticleTypes;  i++ )
	{
		if( m_Texture[i].length() > 0 ||
			m_TextureIllumination[i].length() > 0 )
		{
			CMaterial* pMaterial = m_pRefManager.AddMaterial( pD3DDevice, 
																	m_Texture[i],
																	m_TextureAlpha[i],
																	m_TextureIllumination[i],
																	"",
																	"" );
			m_pParticleTexture.push_back( pMaterial );
			if( m_Multiply[i] )
			{
				pMaterial->SetIlluminationType( KMaterialMultiply );
			}
		}
		else
		{
			m_pParticleTexture.push_back( NULL );
		}

		CParticleSystem* pSystem = new CParticleSystem( pD3DDevice,
														  m_pSettings,
														  (uint32)floor( (float32)m_MaxParticles[i] ),
														  m_Size[i], // width of standard particle
														  m_Gravity[i],
														  m_ScaleSpeed[i],
														  m_FadeSpeed[i],
														  m_Frames[i],
														  m_FrameSpeed[i],
														  m_Multiply[i],
														  m_DoubleSided[i] );
			if( m_TextureIllumination[i].length() > 0 &&
				m_FadeSpeed[i] != 0 )
			{
				pSystem->SetAdditiveFading( kTrue );
			}
			if( m_Directed[i] )
			{
				pSystem->SetDirected( kTrue );
			}

		m_pParticleSystem.push_back( pSystem );
	}

} // CParticleManager::CreateParticleSystems()

void CParticleManager::DestroyParticleSystems( void )
{
	for( uint32 i = 0; i < m_pParticleSystem.size();  i++ )
	{
		m_pRefManager.RemoveMaterial( m_pParticleTexture[i] );

		DELETE_SAFELY( m_pParticleSystem[i] );
	}
	m_pParticleSystem.clear();

} // CParticleManager::DestroyParticleSystems()

int32 CParticleManager::GetParticleType( std::string Name )	// name of particle to look for
{
	Name = StringUpper( Name );
	for( uint32 i = 0; i < m_ParticleTypes; i++ )
	{
		if( Name == m_Name[i] )
		{
			return i;
		}
	}
	return -1;
} // CParticleManager::GetParticleType()

void CParticleManager::AddParticle( uint32 Type,						// type of particle to add
									const D3DXMATRIX&  Transformation,	// transformation to use
									const D3DXVECTOR3& Velocity,		// velocity of particle
									float32 Scale )						// scale of particle
{
	// check the frequency for this particle to see if we should in fact create it
	if( ( rand()%1000 ) / 1000.0f > m_Frequency[Type] * KFrequencyMultiplier[ m_pSettings.GetSettings( KSetParticleDetail ) ] )
	{
		return;
	}

	D3DXVECTOR3 Position( Transformation._41, Transformation._42, Transformation._43 );

	D3DXVECTOR3 Offset( 0, 0, 0 );
	Offset.x = RandomBetween( m_MinStartingOffset[Type].x, m_MaxStartingOffset[Type].x );
	Offset.y = RandomBetween( m_MinStartingOffset[Type].y, m_MaxStartingOffset[Type].y );
	Offset.z = RandomBetween( m_MinStartingOffset[Type].z, m_MaxStartingOffset[Type].z );

	D3DXVECTOR3 StartingVelocity( 0, 0, 0 );
	StartingVelocity.x = RandomBetween( m_MinStartingVelocity[Type].x, m_MaxStartingVelocity[Type].x );
	StartingVelocity.y = RandomBetween( m_MinStartingVelocity[Type].y, m_MaxStartingVelocity[Type].y );
	StartingVelocity.z = RandomBetween( m_MinStartingVelocity[Type].z, m_MaxStartingVelocity[Type].z );
	StartingVelocity += Velocity;

	D3DXVec3TransformNormal( &StartingVelocity, &StartingVelocity, &Transformation );
	D3DXVec3TransformNormal( &Offset, &Offset, &Transformation );


	Offset += Position;


	m_pParticleSystem[Type]->Add( new CParticle( Offset,
												 StartingVelocity,
												 RandomBetween( m_MinStartingScale[Type], m_MaxStartingScale[Type] ) * Scale,
												 RandomBetween( m_MinLifetime[Type], m_MaxLifetime[Type] ),
												 RandomBetween( m_MinStartingZOrientation[Type], m_MaxStartingZOrientation[Type] ),
												 RandomBetween( m_MinRotationSpeed[Type], m_MaxRotationSpeed[Type] ),
												 (uint32)RandomBetween( (float32)m_MinStartingFrame[Type], (float32)m_MaxStartingFrame[Type] ),
												 (uint32)RandomBetween( (float32)m_MinAnimationLifetime[Type], (float32)m_MaxAnimationLifetime[Type] ),
												 (uint32)RandomBetween( m_MinStartingOpacity[Type], m_MaxStartingOpacity[Type] ) ) );
} // CParticleManager::AddParticle()

void CParticleManager::AddParticle( uint32 Type,					// type of particle to add
									const D3DXVECTOR3& Position,	// position to create at
									const D3DXVECTOR3& Velocity,		// velocity of particle
									float32 Scale )						// scale of particle
{
	// check the frequency for this particle to see if we should in fact create it
	if( ( rand()%1000 ) / 1000.0f > m_Frequency[Type] )
	{
		return;
	}

	D3DXVECTOR3 Offset( 0, 0, 0 );
	Offset.x = RandomBetween( m_MinStartingOffset[Type].x, m_MaxStartingOffset[Type].x );
	Offset.y = RandomBetween( m_MinStartingOffset[Type].y, m_MaxStartingOffset[Type].y );
	Offset.z = RandomBetween( m_MinStartingOffset[Type].z, m_MaxStartingOffset[Type].z );
	Offset += Position;

	D3DXVECTOR3 StartingVelocity( 0, 0, 0 );
	StartingVelocity.x = RandomBetween( m_MinStartingVelocity[Type].x, m_MaxStartingVelocity[Type].x );
	StartingVelocity.y = RandomBetween( m_MinStartingVelocity[Type].y, m_MaxStartingVelocity[Type].y );
	StartingVelocity.z = RandomBetween( m_MinStartingVelocity[Type].z, m_MaxStartingVelocity[Type].z );
	StartingVelocity += Velocity;




	m_pParticleSystem[Type]->Add( new CParticle( Offset,
												 StartingVelocity,
												 RandomBetween( m_MinStartingScale[Type], m_MaxStartingScale[Type] ) * Scale,
												 RandomBetween( m_MinLifetime[Type], m_MaxLifetime[Type] ),
												 RandomBetween( m_MinStartingZOrientation[Type], m_MaxStartingZOrientation[Type] ),
												 RandomBetween( m_MinRotationSpeed[Type], m_MaxRotationSpeed[Type] ),
												 (uint32)RandomBetween( (float32)m_MinStartingFrame[Type], (float32)m_MaxStartingFrame[Type] ),
												 (uint32)RandomBetween( (float32)m_MinAnimationLifetime[Type], (float32)m_MaxAnimationLifetime[Type] ),
												 (uint32)RandomBetween( m_MinStartingOpacity[Type], m_MaxStartingOpacity[Type] ) ) );
} // CParticleManager::AddParticle()

void CParticleManager::Update( float32 TimeElapsed )	// time elapsed in seconds
{
	// update our particle systems
	for( uint32 i = 0; i < m_pParticleSystem.size(); i++ )
	{
		m_pParticleSystem[i]->Update( TimeElapsed );
	}
} // CParticleManager::Update()

void CParticleManager::Clear( void )
{
	// update our glow systems
	for( uint32 i = 0; i < m_pParticleSystem.size(); i++ )
	{
		m_pParticleSystem[i]->Clear();
	}
} // CParticleManager::Update()

void CParticleManager::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
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
	// ALPHA PARTICLES
	/////////////////////////////////////////////////////////////////

	for( uint32 i = 0; i < m_pParticleSystem.size(); i++ )
	{
		if( m_pParticleTexture[i] != NULL &&
			m_pParticleTexture[i]->IsValid() &&
			m_pParticleSystem[i]->ActiveParticles() > 0 &&
			!m_pParticleTexture[i]->HasIllumination() )
		{
			// set our particle system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pParticleTexture[i]->GetTexture() );

			m_pParticleSystem[i]->Render( pD3DDevice, 
										  ViewportMatrix, 
										  pFrustum );
		}
	}
	// do multiply materials
	// render multiply materials
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

	/////////////////////////////////////////////////////////////////
	// MULTIPLY PARTICLES
	/////////////////////////////////////////////////////////////////

	for( i = 0; i < m_pParticleSystem.size(); i++ )
	{
		if( m_pParticleTexture[i] != NULL &&
			m_pParticleTexture[i]->IsValid() &&
			m_pParticleSystem[i]->ActiveParticles() > 0 &&
			m_pParticleTexture[i]->HasIllumination() &&
			m_pParticleTexture[i]->GetIlluminationType() == KMaterialMultiply )
		{
			// set our particle system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pParticleTexture[i]->GetIlluminationTexture() );

			m_pParticleSystem[i]->Render( pD3DDevice, 
										  ViewportMatrix, 
										  pFrustum );
		}
	}

	// now we do additively blended particles
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	/////////////////////////////////////////////////////////////////
	// ADDITIVE PARTICLES
	/////////////////////////////////////////////////////////////////

	for( i = 0; i < m_pParticleSystem.size(); i++ )
	{
		if( m_pParticleTexture[i] != NULL &&
			m_pParticleTexture[i]->IsValid() &&
			m_pParticleSystem[i]->ActiveParticles() > 0 &&
			m_pParticleTexture[i]->HasIllumination() &&
			m_pParticleTexture[i]->GetIlluminationType() != KMaterialMultiply )
		{
			// set our particle system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pParticleTexture[i]->GetIlluminationTexture() );

			m_pParticleSystem[i]->Render( pD3DDevice, 
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

} // CParticleManager::Render()

// render ONLY particles that are additively blended
void CParticleManager::RenderAdditive( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
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
	// ADDITIVE PARTICLES
	/////////////////////////////////////////////////////////////////

	for( uint32 i = 0; i < m_pParticleSystem.size(); i++ )
	{
		if( m_pParticleTexture[i] != NULL &&
			m_pParticleTexture[i]->IsValid() &&
			m_pParticleTexture[i]->HasIllumination() &&
			m_pParticleSystem[i]->ActiveParticles() > 0 &&
			m_pParticleTexture[i]->GetIlluminationType() != KMaterialMultiply &&
			m_LightBlur[i] )
		{
			// set our particle system texture at layer 0
			pD3DDevice->SetTexture( 0, m_pParticleTexture[i]->GetIlluminationTexture() );

			m_pParticleSystem[i]->Render( pD3DDevice, 
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

} // CParticleManager::RenderAdditive()
