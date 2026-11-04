/*
	animatedmesh.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	3/28/2004



*/
#ifndef _ANIMATEDMESH_H
#define _ANIMATEDMESH_H

#include "d3dx8_shim.h"

#include <d3dx8.h>
#include "../MEDIA/refmanager.h"
#include "AnimatedGeometry.h"
#include "animation.h"
#include "activeanimation.h"
#include <string>
#include <assert.h>

// an animated mesh maintains the animation data for an animatedgeometry object
// it maintains a reference to an animatedgeometry object, and feeds it frame data
// at render time.
class CAnimatedMesh
{

public:

					CAnimatedMesh( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
								   CSettings& pSettings );		// pointer to the settings manager, for keeping track of card/machine capabilities
					~CAnimatedMesh();

	void			Load( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
						  std::string Path,						// path to the model
						  std::string MediaPath,				// path where materials/anim data should be loaded from
						  const std::string& AnimationListPath = "" );	// path to animation data file

	bool			LoadAnimations( std::string Path );	// path to animation file

	void			PlayAnimation( const std::string& Name,	// name of animation to play
								   bool Looping,			// loop the animation?
								   float32 Speed = 1 );		// speed at which to play

	void			PlayAnimation( int32 Index,		// index of the animation to set
								   bool Looping,	// loop the animation?
								   float32 Speed = 1 );		// speed at which to play

	void			PlayAnimationFromFrame( const std::string& Name,	// name of animation to play
											float32 Frame,				// frame to start from
											bool Looping,				// loop the animation?
											float32 Speed = 1 );		// speed at which to play

	void			PlayAnimationFromFrame( int32 Index,		// index of the animation to set
											float32 Frame,		// frame to start from
											bool Looping,	// loop the animation?
											float32 Speed = 1 );		// speed at which to play

	void			StopAnimation( void );

	void			ClearAnimations( void );
	
	void			QueueBlendAnimation( const std::string& Name,	// name of animation to play
										 bool Looping,				// loop the animation?
										 float32 BlendTime,			// time over which to blend, from the end of the preceding animation
										 float32 Speed = 1 );		// speed at which to play

	void			QueueBlendAnimation( int32 Index,			// index of the animation to set
										 bool Looping,			// loop the animation?
										 float32 BlendTime,		// time over which to blend, from the end of the preceding animation
										 float32 Speed = 1 );	// speed at which to play

	void			BlendAnimation( const std::string& Name,	// name of animation to play
									bool Looping,				// loop the animation?
									float32 BlendTime,			// time over which to blend
								    float32 Speed = 1 );		// speed at which to play

	void			BlendAnimation( int32 Index,			// index of the animation to set
									bool Looping,			// loop the animation?
									float32 BlendTime,		// time over which to blend
								    float32 Speed = 1 );	// speed at which to play

	void			QueueBlendAnimationFromFrame( const std::string& Name,	// name of animation to play
												  float32 Frame,		// frame to start from
												  bool Looping,				// loop the animation?
												  float32 BlendTime,			// time over which to blend, from the end of the preceding animation
												  float32 Speed = 1 );		// speed at which to play

	void			QueueBlendAnimationFromFrame( int32 Index,			// index of the animation to set
												  float32 Frame,		// frame to start from
												  bool Looping,			// loop the animation?
												  float32 BlendTime,		// time over which to blend, from the end of the preceding animation
												  float32 Speed = 1 );	// speed at which to play

	void			BlendAnimationFromFrame( const std::string& Name,	// name of animation to play
											 float32 Frame,		// frame to start from
											 bool Looping,				// loop the animation?
											 float32 BlendTime,			// time over which to blend
											 float32 Speed = 1 );		// speed at which to play

	void			BlendAnimationFromFrame( int32 Index,			// index of the animation to set
											 float32 Frame,		// frame to start from
											 bool Looping,			// loop the animation?
											 float32 BlendTime,		// time over which to blend
											 float32 Speed = 1 );	// speed at which to play

	void			Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
							bool UpdateVB = kTrue );					// update the vertex buffer?


	void			Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
							bool UpdateVB = kTrue );					// update the vertex buffer?


	void			RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
									int32 UserDataValue,					// userdata value that each segment must have set to be rendered
									const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
									bool UpdateVB = kTrue );					// update the vertex buffer?

	void			RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									const D3DXMATRIX& ViewportMatrix,			// viewport matrix
									bool UpdateVB = kTrue );					// update the vertex buffer?

	void			RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
										const D3DXMATRIX& ViewportMatrix,			// viewport matrix
										const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
										bool UpdateVB = kTrue );					// update the vertex buffer?

	void			RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									 bool UpdateVB = kTrue );					// update the vertex buffer?

	void			UpdateAnimation( float32 TimeElapsed );	// time elapsed in seconds

	void	EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from


	void	ClearQueuedAnimations( void );

	void	CopyAnimationState( CAnimatedMesh& pMesh );				// mesh to copy animation state from


////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	uint32			GetAnimationLength( const std::string& Name );	// name of animation to retrieve frame length for

	uint32			GetAnimationLength( int32 Index );				// index of animation to retrieve frame length for

	float32			GetAnimationLengthSeconds( const std::string& Name );	// name of animation to retrieve frame length for

	float32			GetAnimationLengthSeconds( int32 Index );				// index of animation to retrieve frame length for

	const std::string&		GetAnimationName( uint32 Index );		// index of animation to retrieve name of

	int32			GetAnimationIndex( const std::string& Name );	// name of animation to retrieve index of															// returns -1 if it doesn't exist

	bool			AnimationExists( const std::string& Name );	// name to check the existence of

	/// check for the existence of the named material on this mesh
	bool				HasMaterial( std::string Name );	// name of material to check existence of

	uint32		GetMaterials( void )			{	return m_pAnimatedGeometry->GetMaterials();	};

	CMaterial*	GetMaterial( uint32 MaterialID )		{	return m_pAnimatedGeometry->GetMaterial( MaterialID );	};

	CMaterial*	GetMaterialByIndex( uint32 Index )		{	return m_pAnimatedGeometry->GetMaterialByIndex( Index );	};

	int32		GetMaterialIDByIndex( uint32 Index )	{	return m_pAnimatedGeometry->GetMaterialIDByIndex( Index );	};

	void		SetMaterial( uint32 MaterialID,			// id of material to overwrite
							 CMaterial* pMaterial )		// material to set
				{
					m_pAnimatedGeometry->SetMaterial( MaterialID, pMaterial );
				};

	void		SetMaterialByIndex( uint32 Index,				// index of material to overwrite
									CMaterial* pMaterial )		// material to set
				{
					m_pAnimatedGeometry->SetMaterialByIndex( Index, pMaterial );
				};


	std::string	GetMaterialName( uint32 Index )	{	return m_pAnimatedGeometry->GetMaterialName( Index );	};

	const std::string&	CurrentAnimation( void );

	bool				AnimationPlaying( void );
	bool				AnimationPlaying( const std::string& Name );	// name to check the play state of
	bool				AnimationPlaying( int32 Index );				// index of animation to check for

	bool				AnimationQueued( const std::string& Name );	// name to check the play state of
	bool				AnimationQueued( int32 Index );				// index of animation to check for

	uint32				ActiveAnimations( void )			{	return m_ActiveAnimations.size();		};

	CActiveAnimation*	ActiveAnimation( uint32 Index )		{	return m_ActiveAnimations[Index];		};

	uint32				GetAnimationCount( void )			{	return m_Animations.size();		};

	D3DXMATRIX			GetTransformedTag( uint32 Index );				// index of tag
	D3DXVECTOR3			GetTagPosition( uint32 Index );				// index of tag

	uint32				GetTagCount( void );

	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	const std::string&	GetTagUserData( uint32 Index );				// index of tag name to retrieve

	bool	CastsShadows( void );

	const D3DXVECTOR3&		GetMinBounds( void );
	const D3DXVECTOR3&		GetMaxBounds( void );

	uint32              GetKeyCount( uint32 AnimationIndex );	// animation to get count for
    
	float32             GetKeyFrame( uint32 AnimationIndex,		// animation key is for
									 uint32 KeyIndex );			// index of key
    
	const std::string&  GetKeyName( uint32 AnimationIndex,		// animation key is for
									uint32 KeyIndex );			// index of key

	uint32              GetKeyIndex( uint32 AnimationIndex, // animation key is for
									 std::string KeyName );	// name of key
    
	void				ClearKeys( void )			{	m_KeysEncountered.clear();			};

	uint32				KeysEncountered( void )		{	return m_KeysEncountered.size();	};
	
	const std::string&	KeyEncounteredName( uint32 Index )
	{
		assert( Index < m_KeysEncountered.size()	);

		return m_KeysEncountered[Index];
	}

	bool				HasReflection( void )
	{
		assert( m_pAnimatedGeometry != NULL );

		return m_pAnimatedGeometry->HasReflection();
	}
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void			SetUserData( int32 Id,			// id of material to set user data on
								 int32 Value );		// value to set for user data

	/// set a user data value on a specific material name
	void			SetUserData( std::string Name,		// name of material to set user data on
								 int32 Value );			// value to set for user data

	void			SetAnimationFrame( const std::string& Name,	// name of animation to play
									   float32 Frame );			// frame (floating point to interpolate to next frame )

	void			SetAnimationFrame( int32 Index,		// index of the animation to set
									   float32 Frame );	// frame (floating point to interpolate to next frame )

private:
	bool						m_CastsShadows;

	CRefManager&				m_pRefManager;
	CSettings&					m_pSettings;

	CAnimatedGeometry*			m_pAnimatedGeometry;

	// Animation properties
	std::vector < CAnimation* >	m_Animations;

	std::deque < CActiveAnimation* > m_ActiveAnimations;

	std::vector < std::string >	m_KeysEncountered;

};	// CAnimatedMesh class


#endif
