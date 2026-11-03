/*
	genericmodel.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/14/2004

	3/16/2005	-	Added bone querying support, with methods equivalent to tags

*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/cubemap.h"

#include "../MODEL/model.h"

#include "../ANIMATEDMESH/animatedmesh.h"

#include "../SKINNEDMESH/skinnedmesh.h"

#include "lightmanager.h"

#include "genericmodel.h"


CGenericModel::CGenericModel( LPDIRECT3DDEVICE8 pD3DDevice, // direct3d device
							  CRefManager& pRefManager,		// pointer to the reference manager, for tracking submaterials
							  CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
							  std::string ModelPath,		// path to the model we want to load
							  std::string TexturePath,		// path to textures ("" to use model path)
							  std::string AnimationPath ):	// path to animation file ( optional
															m_pRefManager( pRefManager ),
															m_pSettings( pSettings ),
															m_pModel( NULL ),
															m_pAnimatedModel( NULL ),
															m_pSkinnedModel( NULL ),
															m_pCubeMap( NULL ),
															m_ModelPath( "" ),
															m_TexturePath( "" )
{
	LoadModel( pD3DDevice, ModelPath, TexturePath, AnimationPath );
} // CGenericModel::CGenericModel()

CGenericModel::~CGenericModel( void )
{
	if( HasCubeMap() )
	{
		m_pRefManager.RemoveCubeMap( m_pCubeMap );
	}

	// clear out any models we already had loaded
	if( m_pModel != NULL )
	{
		m_pRefManager.RemoveModel( m_pModel );
		m_pModel = NULL;
	}
	// clear out any models we already had loaded
	if( m_pAnimatedModel != NULL )
	{
		m_pRefManager.RemoveAnimatedMesh( m_pAnimatedModel );
		m_pAnimatedModel = NULL;
	}
	// clear out any models we already had loaded
	if( m_pSkinnedModel != NULL )
	{
		m_pRefManager.RemoveSkinnedMesh( m_pSkinnedModel );
		m_pSkinnedModel = NULL;
	}
} // CGenericModel::~CGenericModel()


void CGenericModel::UpdateAnimation( float32 TimeElapsed )	// time elapsed in seconds
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->UpdateAnimation( TimeElapsed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->UpdateAnimation( TimeElapsed );
	}


} // CGenericModel::UpdateAnimation()

uint32 CGenericModel::GetAnimationCount( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationCount();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetAnimationCount();
	}
	return 0;
} // CGenericModel::GetAnimationCount()

bool CGenericModel::AnimationExists( const std::string& AnimationName ) //  animation to check for
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->AnimationExists( AnimationName );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->AnimationExists( AnimationName );
	}
	return kFalse;
} // CGenericModel::AnimationExists()

bool CGenericModel::AnimationPlaying( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->AnimationPlaying( );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->AnimationPlaying( );
	}
	return kFalse;
} // CGenericModel::AnimationPlaying()

bool CGenericModel::AnimationPlaying( const std::string& AnimationName ) //  animation to check for
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->AnimationPlaying( AnimationName );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->AnimationPlaying( AnimationName );
	}
	return kFalse;
} // CGenericModel::AnimationPlaying()

bool CGenericModel::AnimationQueued( const std::string& AnimationName ) //  animation to check for
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->AnimationQueued( AnimationName );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->AnimationQueued( AnimationName );
	}
	return kFalse;
} // CGenericModel::AnimationQueued()

bool CGenericModel::AnimationPlaying( uint32 Index )			//  index of animation
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->AnimationPlaying( Index );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->AnimationPlaying( Index );
	}
	return kFalse;
} // CGenericModel::AnimationPlaying()

bool CGenericModel::AnimationQueued( uint32 Index )			//  index of animation
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->AnimationQueued( Index );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->AnimationQueued( Index );
	}
	return kFalse;
} // CGenericModel::AnimationQueued()

void CGenericModel::BlendAnimation( const std::string& AnimationName,	//  animation to check for
								    bool Looping,						// looping anim?
								    float32 BlendTime,					// time to blend
									float32 Speed )						// defaults to 1
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->BlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
} // CGenericModel::BlendAnimation()

void CGenericModel::QueueBlendAnimation( const std::string& AnimationName,	//  animation to check for
										 bool Looping,						// looping anim?
										 float32 BlendTime,					// time to blend
										 float32 Speed )					// defaults to 1
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->QueueBlendAnimation( AnimationName, Looping, BlendTime, Speed );
	}
} // CGenericModel::QueueBlendAnimation()

void CGenericModel::PlayAnimation( const std::string& AnimationName,	//  animation to check for
								   bool Looping,						// looping anim?
								   float32 Speed )						// defaults to 1
{								    
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->PlayAnimation( AnimationName, Looping, Speed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->PlayAnimation( AnimationName, Looping, Speed );
	}
} // CGenericModel::PlayAnimation()

void CGenericModel::BlendAnimation( uint32 Index,						//  index of animation
								    bool Looping,						// looping anim?
								    float32 BlendTime,					// time to blend
									float32 Speed )						// defaults to 1
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->BlendAnimation( Index, Looping, BlendTime, Speed );
	}
} // CGenericModel::BlendAnimation()

void CGenericModel::QueueBlendAnimation( uint32 Index,						//  index of animation
										 bool Looping,						// looping anim?
										 float32 BlendTime,					// time to blend
										 float32 Speed )					// defaults to 1
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->QueueBlendAnimation( Index, Looping, BlendTime, Speed );
	}
} // CGenericModel::QueueBlendAnimation()

void CGenericModel::PlayAnimation( uint32 Index,						//  index of animation
								   bool Looping,						// looping anim?
								   float32 Speed )						// defaults to 1
{								    
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->PlayAnimation( Index, Looping, Speed );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->PlayAnimation( Index, Looping, Speed );
	}
} // CGenericModel::PlayAnimation()

void CGenericModel::ClearAnimations( void )
{								    
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->ClearAnimations();
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->ClearAnimations();
	}
} // CGenericModel::ClearAnimations()

void CGenericModel::ClearQueuedAnimations( void )
{								    
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->ClearQueuedAnimations();
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->ClearQueuedAnimations();
	}
} // CGenericModel::ClearQueuedAnimations()

void CGenericModel::CopyAnimationState( CGenericModel& pModel )	// model to copy state from
{			
	// only animated models can copy, obviously
	if( !IsAnimated() ||
		!pModel.IsAnimated() ||
		pModel.IsTweened() != 
		IsTweened() ||
		pModel.IsSkinned() !=
		IsSkinned() )
	{
		return;
	}
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->CopyAnimationState( *pModel.m_pAnimatedModel );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->CopyAnimationState( *pModel.m_pSkinnedModel );
	}
} // CGenericModel::CopyAnimationState()

void CGenericModel::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							const D3DXMATRIX& TransformedMatrix,	// transformed matrix
							const D3DXVECTOR3& NormalLight,			// normal mapping direction
							bool UpdateVB )							// update vbs for skinned models?

{
	// no model, no render
	if( m_pModel == NULL && m_pSkinnedModel == NULL && m_pAnimatedModel == NULL )
	{
		return;
	}

	// OK, now we have our Character in camera space - we can render it now
	if( HasCubeMap() )
	{
		if( m_pSkinnedModel != NULL )
		{
			m_pSkinnedModel->Render( pD3DDevice, TransformedMatrix, m_pCubeMap->GetTexture(), NormalLight, UpdateVB );
		}
		else if( m_pAnimatedModel != NULL )
		{
			m_pAnimatedModel->Render( pD3DDevice, TransformedMatrix, m_pCubeMap->GetTexture(), NormalLight, UpdateVB );
		}
		else
		{
			m_pModel->Render( pD3DDevice, TransformedMatrix, m_pCubeMap->GetTexture(), NormalLight );
		}
	}
	else
	{
		if( m_pSkinnedModel != NULL )
		{
			m_pSkinnedModel->Render( pD3DDevice, TransformedMatrix, NormalLight, UpdateVB );
		}
		else if( m_pAnimatedModel != NULL )
		{
			m_pAnimatedModel->Render( pD3DDevice, TransformedMatrix, NormalLight, UpdateVB );
		}
		else 
		{
			m_pModel->Render( pD3DDevice, TransformedMatrix, NormalLight );
		}
	}
} // CGenericModel::Render()

void CGenericModel::RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									const D3DXMATRIX& TransformedMatrix,	// transformed matrix
									int32 UserDataValue,					// userdata value that each segment must have set to be rendered
									const D3DXVECTOR3& NormalLight,			// normal mapping direction
									bool UpdateVB )							// update vbs for skinned models?

{
	// no model, no render
	if( m_pModel == NULL && m_pSkinnedModel == NULL && m_pAnimatedModel == NULL )
	{
		return;
	}

	// OK, now we have our Character in camera space - we can render it now
	if( HasCubeMap() )
	{
		if( m_pSkinnedModel != NULL )
		{
			m_pSkinnedModel->RenderUserData( pD3DDevice, TransformedMatrix, m_pCubeMap->GetTexture(), UserDataValue, NormalLight, UpdateVB );
		}
		else if( m_pAnimatedModel != NULL )
		{
			m_pAnimatedModel->RenderUserData( pD3DDevice, TransformedMatrix, m_pCubeMap->GetTexture(), UserDataValue, NormalLight, UpdateVB );
		}
		else
		{
			m_pModel->RenderUserData( pD3DDevice, TransformedMatrix, m_pCubeMap->GetTexture(), UserDataValue, NormalLight );
		}
	}
	else
	{
		if( m_pSkinnedModel != NULL )
		{
			m_pSkinnedModel->RenderUserData( pD3DDevice, TransformedMatrix, NULL, UserDataValue, NormalLight, UpdateVB );
		}
		else if( m_pAnimatedModel != NULL )
		{
			m_pAnimatedModel->RenderUserData( pD3DDevice, TransformedMatrix, NULL, UserDataValue, NormalLight, UpdateVB );
		}
		else 
		{
			m_pModel->RenderUserData( pD3DDevice, TransformedMatrix, NULL, UserDataValue, NormalLight );
		}
	}
} // CGenericModel::RenderUserData()

void CGenericModel::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									 const D3DXMATRIX& TransformedMatrix,	// transformed matrix
									 bool UpdateVB )						// update vbs for skinned model?

{
	// no model, no render
	if( m_pModel == NULL && m_pSkinnedModel == NULL && m_pAnimatedModel == NULL )
	{
		return;
	}


	// OK, now we have our Character in camera space - we can render it now
	if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->RenderGlowPass( pD3DDevice, TransformedMatrix, UpdateVB );
	}
	else if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->RenderGlowPass( pD3DDevice, TransformedMatrix, UpdateVB );
	}
	else
	{
		m_pModel->RenderGlowPass( pD3DDevice, TransformedMatrix );
	}
} // CGenericModel::RenderGlowPass()

void CGenericModel::RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
									   const D3DXMATRIX& TransformedMatrix,	// transformed matrix
									   const D3DXVECTOR3& NormalLight,			// normal mapping direction
									   bool UpdateVB )						// update vbs for skinned model?

{
	// no model, no render
	if( m_pModel == NULL && m_pSkinnedModel == NULL && m_pAnimatedModel == NULL )
	{
		return;
	}


	// OK, now we have our Character in camera space - we can render it now
	if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->RenderIlluminated( pD3DDevice, TransformedMatrix, NormalLight, UpdateVB );
	}
	else if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->RenderIlluminated( pD3DDevice, TransformedMatrix, NormalLight, UpdateVB );
	}
	else
	{
		m_pModel->RenderIlluminated( pD3DDevice, TransformedMatrix, NormalLight );
	}
} // CGenericModel::RenderIlluminated()

void CGenericModel::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									 const D3DXMATRIX& TransformedMatrix,	// transformed matrix
									 bool UpdateVB )						// update vbs for skinned model?

{
	// no model, no render
	if( m_pModel == NULL && m_pSkinnedModel == NULL && m_pAnimatedModel == NULL )
	{
		return;
	}


	// OK, now we have our Character in camera space - we can render it now
	if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->RenderNoTexture( pD3DDevice, TransformedMatrix, UpdateVB );
	}
	else if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->RenderNoTexture( pD3DDevice, TransformedMatrix, UpdateVB );
	}
	else
	{
		m_pModel->RenderNoTexture( pD3DDevice, TransformedMatrix );
	}
} // CGenericModel::RenderNoTexture()


void CGenericModel::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if( m_pModel != NULL )
	{
		m_pModel->EnableShadows( pD3DDevice );
	}
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->EnableShadows( pD3DDevice );
	}
	if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->EnableShadows( pD3DDevice );
	}
} // CGenericModel::EnableShadows()

void CGenericModel::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								   const D3DXMATRIX& TransformedMatrix,		// viewport matrix
								   const D3DXVECTOR3& LocalLightDirection )	// direction the light is coming from
{
	// no model, no render
	if( m_pModel == NULL && m_pSkinnedModel == NULL && m_pAnimatedModel == NULL)
	{
		return;
	}
	if( m_pModel != NULL && !m_pModel->CastsShadows() )
	{
		return;
	}
	if( m_pSkinnedModel != NULL && !m_pSkinnedModel->CastsShadows() )
	{
		return;
	}
	if( m_pAnimatedModel != NULL && !m_pAnimatedModel->CastsShadows() )
	{
		return;
	}
		

	// OK, now we have our Character in camera space - we can render it now
	if( m_pModel != NULL )
	{
		m_pModel->RenderShadows( pD3DDevice, TransformedMatrix, LocalLightDirection );
	}
	else if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->RenderShadows( pD3DDevice, TransformedMatrix, LocalLightDirection );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->RenderShadows( pD3DDevice, TransformedMatrix, LocalLightDirection );
	}
} // CGenericModel::RenderShadows()

bool CGenericModel::HasReflection( void )
{
	if( m_pModel != NULL )
	{
		return m_pModel->HasReflection();
	}
	else if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->HasReflection();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->HasReflection();
	}
	return kFalse;
} // CGenericModel::HasReflection()

bool CGenericModel::CastsShadows( void )
{
	if( m_pModel != NULL )
	{
		return m_pModel->CastsShadows();
	}
	else if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->CastsShadows();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->CastsShadows();
	}
	return kFalse;
} // CGenericModel::CastsShadows()

bool CGenericModel::HasCubeMap( void )
{
	if( m_pCubeMap != NULL )
	{
		return kTrue;
	}
	return kFalse;
} // CGenericModel::HasCubeMap()


void CGenericModel::LoadModel( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
							   std::string ModelPath,				// path to the model we want to load
							   std::string TexturePath,				// path to textures ("" to use model path)
							   std::string AnimationPath )			// path to animation file ( optional )
{
	std::string		Extension( "" );
	
	m_ModelPath = ModelPath;
	m_TexturePath = TexturePath;

	// parse the extension of the filename
	if (  ModelPath.length() > 3 )
	{
		Extension = StringUpper( ModelPath.substr(  ModelPath.length() - 3, 3 ) );
	}
	if( Extension == "MDL" )
	{
		m_pModel = m_pRefManager.AddModel( pD3DDevice, 
										   ModelPath, 
										   TexturePath,
										   kTrue,
										   kTrue );
	}
	else if( Extension == "AMS" )
	{
		m_pAnimatedModel = m_pRefManager.AddAnimatedMesh( pD3DDevice, 
														ModelPath, 
														TexturePath,
														AnimationPath );
	}
	else if( Extension == "SMS" )
	{
		m_pSkinnedModel = m_pRefManager.AddSkinnedMesh( pD3DDevice, 
														ModelPath, 
														TexturePath,
														AnimationPath );
	}

	if( HasReflection() && m_pSettings.GetSettings( KSetCubeMapping ) )
	{
		// create a cube map for reflective models
		// this is a little game-specific setting so we can specify dynamic or static reflections
		//if( m_pSettings.GetSettings( KSetDynamicReflections ) )
		//{
		//	m_pCubeMap = m_pRefManager.AddCubeMap( pD3DDevice, 128, KCubeRenderTarget );
		//}
		//else
		//{		
			m_pCubeMap = m_pRefManager.AddCubeMap( pD3DDevice, "CUBEMAPS\\envmap.dds" );
		//}
	}
} // CGenericModel::LoadModel()


uint32 CGenericModel::GetTagCount( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetTagCount();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetTagCount();
	}

	return 0;
} // CGenericModel::GetTagCount()

const std::string& CGenericModel::GetTagName( uint32 Index )
{
	assert( m_pAnimatedModel != NULL || m_pSkinnedModel != NULL );

	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetTagName( Index );
	}
	else
	{
		return m_pSkinnedModel->GetTagName( Index );
	}


} // CGenericModel::GetTagName()

const std::string& CGenericModel::GetTagUserData( uint32 Index )
{
	assert( m_pAnimatedModel != NULL || m_pSkinnedModel != NULL );

	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetTagUserData( Index );
	}
	else
	{
		return m_pSkinnedModel->GetTagUserData( Index );
	}


} // CGenericModel::GetTagUserData()

D3DXMATRIX CGenericModel::GetTransformedTag( uint32 Index )					// index of tag
{
	assert( m_pAnimatedModel != NULL || m_pSkinnedModel != NULL);

	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetTransformedTag( Index );
	}
	else
	{
		return m_pSkinnedModel->GetTransformedTag( Index );
	}

} // CGenericModel::GetTransformedTag()
D3DXVECTOR3 CGenericModel::GetTagPosition( uint32 Index )					// index of tag							
{
	assert( m_pAnimatedModel != NULL || m_pSkinnedModel != NULL);

	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetTagPosition( Index );
	}
	else
	{
		return m_pSkinnedModel->GetTagPosition( Index );
	}

} // CGenericModel::GetTagPosition()

uint32 CGenericModel::GetBoneCount( void )
{
	if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetBoneCount();
	}

	return 0;
} // CGenericModel::GetBoneCount()

const std::string& CGenericModel::GetBoneName( uint32 Index )
{
	assert( m_pSkinnedModel != NULL );

	return m_pSkinnedModel->GetBoneName( Index );
} // CGenericModel::GetBoneName()

D3DXMATRIX CGenericModel::GetTransformedBone( uint32 Index )					// index of tag
{
	assert( m_pSkinnedModel != NULL);
	return m_pSkinnedModel->GetTransformedBone( Index );


} // CGenericModel::GetTransformedBone()
D3DXVECTOR3 CGenericModel::GetBonePosition( uint32 Index )					// index of tag							
{
	assert( m_pSkinnedModel != NULL);
	return m_pSkinnedModel->GetBonePosition( Index );

} // CGenericModel::GetBonePosition()

const D3DXVECTOR3& CGenericModel::GetMinBounds( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMinBounds();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMinBounds();
	}
	else
	{
		return m_pModel->GetMinBounds();
	}
} // CGenericModel::MinBounds()

const D3DXVECTOR3& CGenericModel::GetMaxBounds( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMaxBounds();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMaxBounds();
	}
	else
	{
		return m_pModel->GetMaxBounds();
	}
} // CGenericModel::MaxBounds()

uint32 CGenericModel::KeysEncountered( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->KeysEncountered();
	}
	else
	{
		return m_pSkinnedModel->KeysEncountered();
	}
} // CGenericModel::KeysEncountered()

void CGenericModel::ClearKeys( void )
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->ClearKeys();
	}
	else
	{
		m_pSkinnedModel->ClearKeys();
	}
} // CGenericModel::ClearKeys()

const std::string& CGenericModel::KeyEncounteredName( uint32 Index )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->KeyEncounteredName( Index );
	}
	else
	{
		return m_pSkinnedModel->KeyEncounteredName( Index );
	}
} // CGenericModel::KeyEncounteredName()

void CGenericModel::SetMaterial( uint32 MaterialID,		// id of material to overwrite
								 CMaterial* pMaterial )	// material to replace it with
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->SetMaterial( MaterialID, pMaterial );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->SetMaterial( MaterialID, pMaterial );
	}
	else
	{
		m_pModel->SetMaterial( MaterialID, pMaterial );
	}
} // CGenericModel::SetMaterial()

void CGenericModel::SetMaterialByIndex( uint32 Index,				// index of material to overwrite
										CMaterial* pMaterial )	// material to replace it with
{
	if( m_pAnimatedModel != NULL )
	{
		m_pAnimatedModel->SetMaterialByIndex( Index, pMaterial );
	}
	else if( m_pSkinnedModel != NULL )
	{
		m_pSkinnedModel->SetMaterialByIndex( Index, pMaterial );
	}
	else
	{
		m_pModel->SetMaterialByIndex( Index, pMaterial );
	}
} // CGenericModel::SetMaterialByIndex()

bool CGenericModel::HasMaterial( const std::string& Name )	// name of material to check existence of
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->HasMaterial( Name );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->HasMaterial( Name );
	}
	else
	{
		return m_pModel->HasMaterial( Name );
	}
} // CGenericModel::HasMaterial()

uint32 CGenericModel::GetMaterials( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMaterials();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMaterials();
	}
	else
	{
		return m_pModel->GetMaterials();
	}
} // CGenericModel::GetMaterials()

CMaterial* CGenericModel::GetMaterial( uint32 MaterialID )	// id of material to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMaterial( MaterialID );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMaterial( MaterialID );
	}
	else
	{
		return m_pModel->GetMaterial( MaterialID );
	}
} // CGenericModel::GetMaterial()

CMaterial* CGenericModel::GetMaterialByIndex( uint32 Index )	// index of material to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMaterialByIndex( Index );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMaterialByIndex( Index );
	}
	else
	{
		return m_pModel->GetMaterialByIndex( Index );
	}
} // CGenericModel::GetMaterialByIndex()

int32 CGenericModel::GetMaterialIDByIndex( uint32 Index )	// index of material to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMaterialIDByIndex( Index );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMaterialIDByIndex( Index );
	}
	else
	{
		return m_pModel->GetMaterialIDByIndex( Index );
	}
} // CGenericModel::GetMaterialIDByIndex()

std::string	CGenericModel::GetMaterialName( uint32 Index )	// index to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetMaterialName( Index );
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->GetMaterialName( Index );
	}
	else
	{
		return m_pModel->GetMaterialName( Index );
	}
} // CGenericModel::GetMaterialName()

uint32 CGenericModel::GetPointCount( void )
{
	if( m_pModel != NULL )
	{
		return m_pModel->GetPointCount();
	}
	return 0;
} // CGenericModel::GetPointCount()

CDataPoint&	CGenericModel::GetPoint( uint32 Index )	// index of point to retrieve
{
	return m_pModel->GetPoint( Index );
} // CGenericModel::GetPoint()

const std::string&	CGenericModel::GetAnimationName( uint32 Index )	// index to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationName( Index );
	}
	else
	{
		return m_pSkinnedModel->GetAnimationName( Index );
	}

} // CGenericModel::GetAnimationName()

uint32 CGenericModel::ActiveAnimations( void )
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->ActiveAnimations();
	}
	else if( m_pSkinnedModel != NULL )
	{
		return m_pSkinnedModel->ActiveAnimations();
	}
	return 0;
} // CGenericModel::ActiveAnimations()

uint32	CGenericModel::GetAnimationLength( int32 Index )	// index to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationLength( Index );
	}
	else
	{
		return m_pSkinnedModel->GetAnimationLength( Index );
	}
	return 0;
} // CGenericModel::GetAnimationLength()

uint32	CGenericModel::GetAnimationLength( const std::string& Name )	// name of material to check existence of
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationLength( Name );
	}
	else
	{
		return m_pSkinnedModel->GetAnimationLength( Name );
	}
	return 0;
} // CGenericModel::GetAnimationLength()

float32	CGenericModel::GetAnimationLengthSeconds( int32 Index )	// index to retrieve
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationLengthSeconds( Index );
	}
	else
	{
		return m_pSkinnedModel->GetAnimationLengthSeconds( Index );
	}
	return 0;
} // CGenericModel::GetAnimationLengthSeconds()

float32	CGenericModel::GetAnimationLengthSeconds( const std::string& Name )	// name ofanimation to check length of
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationLengthSeconds( Name );
	}
	else
	{
		return m_pSkinnedModel->GetAnimationLengthSeconds( Name );
	}
	return 0;
} // CGenericModel::GetAnimationLengthSeconds()

int32	CGenericModel::GetAnimationIndex( const std::string& Name )	// name of animation to check index of
{
	if( m_pAnimatedModel != NULL )
	{
		return m_pAnimatedModel->GetAnimationIndex( Name );
	}
	else
	{
		return m_pSkinnedModel->GetAnimationIndex( Name );
	}
	return 0;
} // CGenericModel::GetAnimationIndex()