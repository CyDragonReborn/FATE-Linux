/*
	levelprop.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/ 
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/cullingbounds.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"

#include "../PARTICLES/particlemanager.h"
#include "../GLOWS/glowmanager.h"
#include "../GLOWS/glowsystem.h"

#include "../POSITIONALAUDIO/positionalaudiomanager.h"

#include "../GAMEOBJECTS/genericmodel.h"

#include "../GAMEOBJECTS/lightmanager.h"

#include "template.h"
#include "levelprop.h"

CLevelProp::CLevelProp( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					    CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
						CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
						CParticleManager& pParticleManager,	// particle manager
						CGlowManager& pGlowManager,			// glow manager
						CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
						CPropTemplate& pTemplate,		// prop template
						const D3DXVECTOR3& Position,	// worlddspace position
						float32 Scale,					// scale of prop
						float32 Angle ) :				// angle of prop
											m_pRefManager( pRefManager ),
											m_pSettings( pSettings ),
											m_pParticleManager( pParticleManager ),
											m_pGlowManager( pGlowManager ),
											m_pPositionalAudioManager( pPositionalAudioManager ),
											m_Position( Position ),
											m_pModel( NULL ),
											m_Onscreen( kFalse ),
											m_pCullingBounds( NULL ),
											m_pLightPoolMaterial( NULL ),
											m_pCollisionModel( NULL ),
											m_pAmbientPositional( NULL ),
											m_pActivationSound( NULL ),
											m_pDeactivationSound( NULL ),
											m_Collideable( kTrue ),
											m_Shadow( kTrue ),
											m_HasLightPool( kFalse ),
											m_CanOpen( kFalse ),
											m_CanClose( kFalse ),
											m_ObjectNearby( kFalse ),
											m_Open( kFalse ),
											m_Structural( kFalse ),
											m_TorchLightFlicker( 0 ),
											m_LightPoolAngle( 0 ),
											m_ActivationRadius( 16 ),
											m_DeactivationRadius( 16 ),
											m_LocalLight ( 0, 0, 0 ),
											m_LightPoolType ( KLightFlicker ),
											m_LightPoolRadius ( 0.0f ),
											m_LightPoolScale ( 0.0f ),
											m_BaseSoundVolume ( 0 ),
											m_BaseSoundRadius ( 0.0f )
{
	ZeroMemory ( &m_OrientationMatrix, sizeof ( m_OrientationMatrix ));
	ZeroMemory ( &m_TransformationMatrix, sizeof ( m_TransformationMatrix ));

	if( pTemplate.HasLightPool() )
	{
		m_HasLightPool = kTrue;
		m_pLightPoolMaterial = m_pRefManager.AddMaterial( pD3DDevice, pTemplate.LightPoolPath() );
		m_LightPoolRadius = pTemplate.LightPoolRadius();
		m_LightPoolType = pTemplate.LightPoolType();
	}

	m_Collideable = pTemplate.Collideable();

	m_Shadow = pTemplate.Shadow();

	m_Structural = pTemplate.Structural();

	std::string AnimationPath = RemoveFileName( pTemplate.ModelPath() );
	AnimationPath = GetLocalPath() + AnimationPath + "animation.dat";
	m_pModel = new CGenericModel( pD3DDevice, 
								  m_pRefManager, 
								  m_pSettings,
								  pTemplate.ModelPath(),
								  pTemplate.TexturePath(),
								  AnimationPath );


	for( uint32 i = 0; i < m_pModel->GetAnimationCount(); i++ )
	{
		std::string Name = StringUpper( m_pModel->GetAnimationName( i ) );
		if( Name.length() >= 4 && Name.substr( 0, 4 ) == "IDLE" )
		{
			m_IdleAnimations.push_back( i );
		}
	}

	if( m_IdleAnimations.size() > 0 )
	{
		uint32 Index = RandomIntegerBetween( 0, m_IdleAnimations.size() - 1 );
		m_pModel->PlayAnimation( Index, kTrue );
	}
	else if( m_pModel->GetAnimationCount() == 0 )
	{
		m_pModel->PlayAnimation( 0 , kTrue );
	}

	// door-like motions 
	if( m_pModel->AnimationExists( "OPEN" ) )
	{
		m_CanOpen = kTrue;
	}
	if( m_pModel->AnimationExists( "CLOSE" ) )
	{
		m_CanClose = kTrue;
	}

	D3DXVECTOR3 MinBounds = m_pModel->GetMinBounds();
	D3DXVECTOR3 MaxBounds = m_pModel->GetMaxBounds();

	if( pTemplate.CollisionPath().length() > 0 )
	{
		m_pCollisionModel = m_pRefManager.AddModel( pD3DDevice,
													pTemplate.CollisionPath(),
													"",
													kTrue,
													kFalse );
		MinBounds = m_pCollisionModel->GetMinBounds();
		MaxBounds = m_pCollisionModel->GetMaxBounds();

	}
	ExpandHorizontalCubicBounds( MinBounds, MaxBounds, D3DXVECTOR3( MinBounds.x, 0, MinBounds.z ) );
	ExpandHorizontalCubicBounds( MinBounds, MaxBounds, D3DXVECTOR3( MaxBounds.x, 0, MaxBounds.z ) );


	// opening doors need larger culling bounds
	if( m_CanOpen )
	{
		MinBounds *= 2;
		MaxBounds *= 2;
	}
	m_pCullingBounds = new CCullingBounds( MinBounds,
										   MaxBounds );


	D3DXMatrixIdentity( &m_OrientationMatrix );

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &m_TransformationMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	D3DXMATRIX RotationMatrix;

	D3DXMatrixRotationY( &RotationMatrix, (float32)KDeg2Rad * Angle );


	D3DXMATRIX ScaleMatrix;
	D3DXMatrixScaling( &ScaleMatrix, 
					   Scale,
					   Scale,
					   Scale );

	m_OrientationMatrix = ScaleMatrix * RotationMatrix * m_OrientationMatrix;

	m_TransformationMatrix = m_OrientationMatrix * m_TransformationMatrix;

	m_pCullingBounds->Update( m_TransformationMatrix );

	if( m_pCollisionModel == NULL &&
		!m_pModel->HasCollisionData() )
	{
		m_Collideable = kFalse;
	}

	m_ActivationRadius = pTemplate.ActivationRadius();
	m_DeactivationRadius = pTemplate.DeactivationRadius();

	m_BaseSoundVolume = pTemplate.BaseSoundVolume();
	m_BaseSoundRadius = pTemplate.BaseSoundRadius();

	if( pTemplate.AmbientSoundPath().length() > 0 )
	{
		m_pAmbientPositional = m_pPositionalAudioManager.AddAudio( m_Position,
																	pTemplate.AmbientSoundPath(),
																	m_BaseSoundVolume,
																	m_BaseSoundRadius );

	}
	if( pTemplate.ActivationSoundPath().length() > 0 )
	{
		m_pActivationSound = m_pRefManager.AddSample( pTemplate.ActivationSoundPath(), kFalse );
	}
	if( pTemplate.DeactivationSoundPath().length() > 0 )
	{
		m_pDeactivationSound = m_pRefManager.AddSample( pTemplate.DeactivationSoundPath(), kFalse );
	}

	FindParticles();
	EmitParticles();

} // CLevelProp::CLevelProp()

CLevelProp::~CLevelProp( void )
{
	m_pPositionalAudioManager.RemoveAudio( m_pAmbientPositional );

	m_pRefManager.RemoveSample( m_pActivationSound );
	m_pRefManager.RemoveSample( m_pDeactivationSound );

	m_pRefManager.RemoveMaterial( m_pLightPoolMaterial );
	m_pRefManager.RemoveModel( m_pCollisionModel );

	DELETE_SAFELY( m_pModel );

	DELETE_SAFELY( m_pCullingBounds );

	for( uint32 i = 0; i < m_pGlowList.size(); i++ )
	{
		m_pGlowList[i]->m_RequestingRemoval = kTrue;
	}
	for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
	{
		m_pModelGlowList[i]->m_RequestingRemoval = kTrue;
	}

} // CLevelProp::~CLevelProp()

void CLevelProp::SetTransformation( const D3DXMATRIX& Transform )	// transformation matrix
{
	m_OrientationMatrix = Transform;
	m_TransformationMatrix = Transform;
	m_OrientationMatrix._41 = 0;
	m_OrientationMatrix._42 = 0;
	m_OrientationMatrix._43 = 0;

	m_pCullingBounds->Update( m_TransformationMatrix );

} // CLevelProp::SetTransformation()

void CLevelProp::BlendRandomIdleAnimation( void )
{
	if( m_IdleAnimations.size() > 0 )
	{
		uint32 Index = RandomIntegerBetween( 0, m_IdleAnimations.size() - 1 );
		m_pModel->QueueBlendAnimation( Index, kTrue, 0.2f );
	}
} // CLevelProp::BlendRandomIdleAnimation()

void CLevelProp::FindParticles( void )
{
	for( uint32 i = 0; i < m_pModel->GetPointCount(); i++ )
	{
		CDataPoint& Point = m_pModel->GetPoint( i );
		std::string Name = StringUpper( Point.Name() );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( Point.UserData() );
			if( ParticleIndex != -1 )
			{
				D3DXVECTOR3 Position = Point.Position();
				m_ParticlePosition.push_back( Position );
				m_ParticleOrientation.push_back( Point.Orientation() );
				m_ParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGlowManager.GetGlowType( Point.UserData() );
			D3DXVECTOR3 Position = Point.Position();
			if( GlowIndex != -1 )
			{
				m_GlowPosition.push_back( Position );
				m_pModelGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, Point.Position() ) );					
			}
		}
	}

	for( int32 i = 0; i < m_pModel->GetTagCount(); i++ )
	{
		std::string Name = StringUpper( m_pModel->GetTagName( i ) );
		if( Name.length() >= 8 && Name.substr( 0, 8 ) == "PARTICLE" )
		{
			int32 ParticleIndex = m_pParticleManager.GetParticleType( m_pModel->GetTagUserData( i )  );
			if( ParticleIndex != -1 )
			{
				m_TagParticleIndex.push_back( i );
				m_TagParticleType.push_back( ParticleIndex );
			}
		}
		else if( Name.length() >= 4 && Name.substr( 0, 4 ) == "GLOW" )
		{
			int32 GlowIndex = m_pGlowManager.GetGlowType( m_pModel->GetTagUserData( i ) );
			if( GlowIndex != -1 )
			{
				m_pGlowList.push_back( m_pGlowManager.AddGlow( GlowIndex, D3DXVECTOR3( 0, -10000, 0 ) ) );					
				m_GlowIndex.push_back( i );
			}
		}
	}
} // CLevelProp::FindParticles()

void CLevelProp::EmitParticles( void )
{
	for( uint32 i = 0; i < m_ParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXVec3TransformCoord( &Position,  &m_ParticlePosition[i], &m_TransformationMatrix );
		D3DXMATRIX Transform = m_ParticleOrientation[i] * m_OrientationMatrix;
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pParticleManager.AddParticle( m_ParticleType[i], Transform );
	}

	for( int32 i = 0; i < m_TagParticleType.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXMATRIX Transform = m_pModel->GetTransformedTag( m_TagParticleIndex[i] );
		Transform *= m_TransformationMatrix;
		m_pParticleManager.AddParticle( m_TagParticleType[i], Transform );
	}

	for( int32 i = 0; i < m_pGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position = m_pModel->GetTagPosition( m_GlowIndex[i] );
		D3DXVec3TransformCoord( &Position, &Position, &m_TransformationMatrix );
		m_pGlowList[i]->m_Position = Position;
	}
	for( int32 i = 0; i < m_pModelGlowList.size(); i++ )
	{
		D3DXVECTOR3 Position;
		D3DXVec3TransformCoord( &Position,  &m_GlowPosition[i], &m_TransformationMatrix );
		D3DXMATRIX Transform = m_OrientationMatrix;
		Transform._41 = Position.x;
		Transform._42 = Position.y;
		Transform._43 = Position.z;
		m_pModelGlowList[i]->m_Position = Position;
	}

} // CLevelProp::EmitParticles()

void CLevelProp::UpdateProximity( const D3DXVECTOR3& Position )	// position of nearby mobile object
{
	// if the object can perform no actions, we don't care about proximity
	if( !m_CanOpen && !m_CanClose )
	{
		return;
	}
	D3DXVECTOR3 Delta = Position - m_Position;
	Delta.y = 0;
	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance <= m_ActivationRadius )
	{
		m_ObjectNearby = kTrue;
	}
	else if ( Distance <= m_DeactivationRadius && m_Open )
	{
		m_ObjectNearby = kTrue;
	}


} // CLevelProp::UpdateProximity()

void CLevelProp::Update( float32 TimeElapsed )		// time elapsed in seconds
{
	if( Onscreen() )
	{
		if( !m_CanOpen &&
			 m_pModel->ActiveAnimations() == 1 )
		{
			if( rand() % 1000 < 5 )
			{
				BlendRandomIdleAnimation();
			}
		}

		EmitParticles();
	}
	if( Onscreen() || m_CanOpen )
	{
		m_pModel->UpdateAnimation( TimeElapsed );
	}

	if( m_HasLightPool )
	{
		if( m_LightPoolType == KLightFlicker )
		{
			m_TorchLightFlicker += TimeElapsed * RandomBetween( 2, 5 );
			if( RandomBetween( 0, 10 ) > 7 )
			{
				m_TorchLightFlicker += RandomBetween( -.2f, .2f );
			}
		}
		else
		{
			m_TorchLightFlicker += TimeElapsed * .5f;
		}
		while( m_TorchLightFlicker >= 2.0f * (float32)KPI )
		{
			m_TorchLightFlicker -= 2.0f * (float32)KPI;
		}

		if( m_LightPoolType == KLightFlicker )
		{
			m_LightPoolScale = m_LightPoolRadius * 2.0f + fabs( sin( m_TorchLightFlicker ) ) * 10;
		}
		else
		{
			m_LightPoolScale = m_LightPoolRadius * 2.0f + fabs( sin( m_TorchLightFlicker ) ) * 15;
		}


		if( m_LightPoolType == KLightFlicker )
		{
			m_LightPoolAngle = (float32)KDeg2Rad * sin( sin( m_TorchLightFlicker ) * 10.0f );
		}
	}

	// if there is something in proximity
	if( m_ObjectNearby )
	{
		if( !m_Open && 
			m_CanOpen &&
			!m_pModel->AnimationQueued( "OPEN" ) )
		{
			m_Open = kTrue;
			m_pModel->QueueBlendAnimation( "OPEN", kFalse, 0.0f );
			m_pModel->QueueBlendAnimation( "OPENED", kTrue, 0.0f );
			if( m_pActivationSound != NULL )
			{
				m_pPositionalAudioManager.PlaySample( m_pActivationSound,
													  m_Position,
													  m_BaseSoundVolume,
													  m_BaseSoundRadius );
			}
		}
	}
	else
	{
		if( m_Open && 
			m_CanClose  &&
			!m_pModel->AnimationQueued( "CLOSE" ) )
		{
			m_Open = kFalse;
			m_pModel->QueueBlendAnimation( "CLOSE", kFalse, 0.0f );
			m_pModel->QueueBlendAnimation( "IDLE", kTrue, 0.0f );
			if( m_pDeactivationSound != NULL )
			{
				m_pPositionalAudioManager.PlaySample( m_pDeactivationSound,
													  m_Position,
													  m_BaseSoundVolume,
													  m_BaseSoundRadius );
			}
		}
	}

	m_ObjectNearby = kFalse;
} // CLevelProp::Update()

void CLevelProp::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						 CLightManager* pLightManager,			// light manager - NULL for no light manager
						 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						 CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	// no model, no render
	if( m_pModel == NULL )
	{
		return;
	}
		

	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

	D3DXMATRIX TransformedMatrix;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &TransformedMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	TransformedMatrix = m_OrientationMatrix * TransformedMatrix;


	SetOnscreen( kTrue );

	// set local ambient light, based upon distance from camera
	for( uint32 i = 0; i < m_pModel->GetMaterials(); i++ )
	{
		CMaterial*	Material = m_pModel->GetMaterial( i );
		if( Material != NULL )
		{
			Material->D3DMaterial().Diffuse.r = m_LocalLight.x;
			Material->D3DMaterial().Diffuse.g = m_LocalLight.y;
			Material->D3DMaterial().Diffuse.b = m_LocalLight.z;
		}
	}


	D3DXVECTOR3 NormalLight( 0, -1, 0 );

	if( pLightManager != NULL )
	{
		pLightManager->UpdateLights( pD3DDevice, TransformedMatrix );

		D3DXMATRIX InverseTransformation;
		D3DXMatrixInverse( &InverseTransformation, NULL, &TransformedMatrix );

		D3DXVec3TransformNormal( &NormalLight, &pLightManager->LightDirection(), &InverseTransformation );

	}

	TransformedMatrix *= ViewportMatrix;

	m_pModel->Render( pD3DDevice, TransformedMatrix, NormalLight );

} // CLevelProp::Render()

void CLevelProp::RenderShadow( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							   const D3DXMATRIX& ViewportMatrix,	// viewport matrix
							   CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	// no model, no render
	if( m_pModel == NULL )
	{
		return;
	}

	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

		
	D3DXMATRIX TransformedMatrix;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &TransformedMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	TransformedMatrix = m_OrientationMatrix * TransformedMatrix;

	TransformedMatrix *= ViewportMatrix;

	m_pModel->RenderNoTexture( pD3DDevice, TransformedMatrix );

} // CLevelProp::Render()


bool CLevelProp::InFrustum( CFrustum& pFrustum )	// frustum to cull against					// frustum to cull with - NULL for none

{
	// no model, no render
	if( m_pModel == NULL )
	{
		return kFalse;
	}

	// if we're not in the frustum, don't render at all
	if( pFrustum.Intersects( *m_pCullingBounds ) == KFrustumOutside )
	{
		return kFalse;
	}

	return kTrue;
} // CLevelProp::InFrustum()

void CLevelProp::CalculateLocalLight( bool Torchlit,					//  is the level torchlit?
									  const D3DXVECTOR3& FocalPoint,	// point where the camera is directed
									  float32 MaximumDistance )			// maximum distance from camera

{
	if( !Torchlit || Structural() )
	{
		m_LocalLight = D3DXVECTOR3( 1, 1, 1 );
		return;
	}

	D3DXVECTOR3 Delta = FocalPoint - m_Position;
	Delta.y = 0;
	float32 Distance = D3DXVec3Length( &Delta );

	if( Distance > MaximumDistance )
	{
		Distance = MaximumDistance;
	}

	// bring to the range of 0-1
	Distance /= MaximumDistance;

	// attenuate a little
	Distance *= Distance;

	// and invert
	Distance = 1.0f - Distance;

	m_LocalLight = D3DXVECTOR3( Distance, Distance, Distance );

} // CLevelProp::CalculateLocalLight()
