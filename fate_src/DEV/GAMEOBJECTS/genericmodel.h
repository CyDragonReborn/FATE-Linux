/*
	genericmodel.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _GENERICMODEL_H
#define _GENERICMODEL_H

#include "d3dx8_shim.h"

#include "../UTILITIES/constants.h"
#include "../MODEL/model.h"

class CAnimatedMesh;
class CModel;
class CSkinnedMesh;
class CRefManager;
class CSettings;
class CLightManager;
class CFrustum;
class CCubeMap;
class CFaceList;

// this wraps all model types up so that they can be treated as a single model object
class CGenericModel
{
public:

				CGenericModel( LPDIRECT3DDEVICE8 pD3DDevice,// direct3d device
							   CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
							   CSettings& pSettings,		// pointer to the settings manager, for keeping track of card/machine capabilities
							   std::string ModelPath,		// path to the model we want to load
							   std::string TexturePath,		// path to textures ("" to use model path)
							   std::string AnimationPath = "" );	// path to animation file ( optional
				~CGenericModel( void );

	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,						// direct3d device
						const D3DXMATRIX& TransformedMatrix,				// transformed matrix
						const D3DXVECTOR3& NormalLight = D3DXVECTOR3( 0, -1, 0 ),		// normal mapping direction
						bool UpdateVB = kTrue );							// update vbs for skinned model?

	void		RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,									// direct3d device
								const D3DXMATRIX& TransformedMatrix,							// transformed matrix
								int32 UserDataValue,											// userdata value that each segment must have set to be rendered
								const D3DXVECTOR3& NormalLight = D3DXVECTOR3( 0, -1, 0 ),		// normal mapping direction
								bool UpdateVB = kTrue );							// update vbs for skinned model?

	void		RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								const D3DXMATRIX& TransformedMatrix,		// transformed matrix
								bool UpdateVB = kTrue );					// update vbs for skinned model?

	void		RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								   const D3DXMATRIX& TransformedMatrix,		// transformed matrix
								   const D3DXVECTOR3& NormalLight = D3DXVECTOR3( 0, -1, 0 ),		// normal mapping direction
								   bool UpdateVB = kTrue );					// update vbs for skinned model?

	void		RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
								 const D3DXMATRIX& TransformedMatrix,		// transformed matrix
								 bool UpdateVB = kTrue );					// update vbs for skinned model?

	void		RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
							   const D3DXMATRIX& TransformedMatrix,			// transformed matrix
							   const D3DXVECTOR3& LocalLightDirection );	// direction the light is coming from

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void		SetMaterial( uint32 MaterialID,			// od of material to overwrite
							 CMaterial* pMaterial );	// material to replace it with
	
	void		SetMaterialByIndex( uint32 Index,				// index of material to overwrite
									CMaterial* pMaterial );	// material to replace it with

	void		EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device


	void		UpdateAnimation( float32 TimeElapsed );		// time elapsed in seconds

	void		QueueBlendAnimation( const std::string& AnimationName,	//  animation to check for
									 bool Looping,						// looping anim?
									 float32 BlendTime,					// time to blend
									 float32 Speed = 1 );				// speed, defaults to 1

	void		BlendAnimation( const std::string& AnimationName,	//  animation to check for
								 bool Looping,						// looping anim?
								 float32 BlendTime,					// time to blend
								 float32 Speed = 1 );				// speed, defaults to 1

	void		PlayAnimation( const std::string& AnimationName,	//  animation to check for
							   bool Looping,						// looping anim?
							   float32 Speed = 1 );					// speed, defaults to 1

	void		QueueBlendAnimation( uint32 Index,						//  index of animation
									 bool Looping,						// looping anim?
									 float32 BlendTime,					// time to blend
									 float32 Speed = 1 );				// speed, defaults to 1

	void		BlendAnimation( uint32 Index,						//  index of animation
								bool Looping,						// looping anim?
								float32 BlendTime,					// time to blend
								float32 Speed = 1 );				// speed, defaults to 1

	void		PlayAnimation( uint32 Index,						//  index of animation
							   bool Looping,						// looping anim?
							   float32 Speed = 1 );					// speed, defaults to 1

	void		ClearQueuedAnimations( void );

	void		ClearAnimations( void );

	void		CopyAnimationState( CGenericModel& pModel );	// model to copy state from
////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	const std::string&	ModelPath( void )			{	return m_ModelPath;			};

	const std::string&	TexturePath( void )			{	return m_TexturePath;		};

	uint32		GetPointCount( void );

	CDataPoint&	GetPoint( uint32 Index );	// index of point to retrieve

	bool		CastsShadows( void );

	/// check for the existence of the named material on this mesh
	bool		HasMaterial( const std::string& Name );	// name of material to check existence of

	uint32		GetMaterials( void );

	CMaterial*	GetMaterial( uint32 MaterialID );	// id of material to retrieve

	CMaterial*	GetMaterialByIndex( uint32 Index );	// index of material to retrieve

	int32	GetMaterialIDByIndex( uint32 Index );	// index of material to retrieve

	std::string	GetMaterialName( uint32 ID );	// index to retrieve


	const D3DXVECTOR3&	GetMinBounds( void );

	const D3DXVECTOR3&	GetMaxBounds( void );

	int32				GetAnimationIndex( const std::string& Name );	// name of animation to retrieve index of															// returns -1 if it doesn't exist

	uint32				GetAnimationCount( void );

	bool				AnimationPlaying( void );

	bool				AnimationPlaying( const std::string& AnimationName );	//  animation to check for

	bool				AnimationQueued( const std::string& AnimationName );	//  animation to check for

	bool				AnimationExists( const std::string& AnimationName );	//  animation to check for

	bool				AnimationPlaying( uint32 Index );			//  index of animation

	bool				AnimationQueued( uint32 Index );			//  index of animation

	const std::string&	GetAnimationName( uint32 Index );		// index of animation to retrieve name of

	uint32				GetAnimationLength( const std::string& Name );	// name of animation to retrieve frame length for

	uint32				GetAnimationLength( int32 Index );				// index of animation to retrieve frame length for

	float32				GetAnimationLengthSeconds( const std::string& Name );	// name of animation to retrieve frame length for

	float32				GetAnimationLengthSeconds( int32 Index );				// index of animation to retrieve frame length for

	uint32				ActiveAnimations( void );

	bool				HasReflection( void );

	bool				HasCubeMap( void );

	CCubeMap*			CubeMap( void )					{	return m_pCubeMap;		};

	D3DXMATRIX			GetTransformedTag( uint32 Index );			// index of tag

	D3DXVECTOR3			GetTagPosition( uint32 Index );				// index of tag

	uint32				GetTagCount( void );

	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	const std::string&	GetTagUserData( uint32 Index );				// index of tag name to retrieve

	D3DXMATRIX			GetTransformedBone( uint32 Index );			// index of bone

	D3DXVECTOR3			GetBonePosition( uint32 Index );				// index of bone

	uint32				GetBoneCount( void );

	const std::string&	GetBoneName( uint32 Index );					// index of bone name to retrieve

	void				ClearKeys( void );

	uint32				KeysEncountered( void );

	const std::string&	KeyEncounteredName( uint32 Index );

	bool				IsAnimated( void )			{	return ( m_pSkinnedModel != NULL || m_pAnimatedModel != NULL );		};

	bool				IsSkinned( void )			{	return m_pSkinnedModel != NULL;		};

	bool				IsTweened( void )			{	return m_pAnimatedModel != NULL;	};

	bool				HasCollisionData( void )	{	return m_pModel != NULL;		};

	CFaceList*			FaceList( void )			{	return m_pModel->FaceList();	};

protected:
	void		LoadModel( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						   std::string ModelPath,			// path to the model we want to load
						   std::string TexturePath,			// path to textures ("" to use model path)	
						   std::string AnimationPathPath = "" );	// path to animation file ( optional )

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CSkinnedMesh*		m_pSkinnedModel;
	CAnimatedMesh*		m_pAnimatedModel;	
	CModel*				m_pModel;
	CCubeMap*			m_pCubeMap;

	std::string			m_ModelPath;
	std::string			m_TexturePath;

};

#endif