/*
	skinnedmesh.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004


*/
#ifndef _SKINNEDMESH_H
#define _SKINNEDMESH_H



#include "skinnedanimation.h"
#include "activeskinnedanimation.h"
#include "skinnedanimation.h"
#include "skinnedgeometry.h"

class CPose;

// a skinned mesh maintains the animation data for an skinnedgeometry object
// it maintains a reference to an skinnedgeometry object, and feeds it frame data
// at render time.
class CSkinnedMesh
{

public:

					CSkinnedMesh( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
								   CSettings& pSettings );		// pointer to the settings manager, for keeping track of card/machine capabilities
					~CSkinnedMesh();

	void			Load( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
						  std::string Path,						// path to the model
						  std::string MediaPath,				// path where materials/anim data should be loaded from
						  const std::string& AnimationListPath = "" );	// path to animation data file

	bool			LoadAnimations( std::string Path );	// path to animation file

	void			PlayAnimation( const std::string& Name,	// name of animation to play
								   bool Looping,			// loop the animation?
								   float32 Speed = 1 );		// defaults to 1

	void			PlayAnimation( int32 Index,				// index of the animation to set
								   bool Looping,			// loop the animation?
								   float32 Speed = 1 );		// defaults to 1

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
							bool UpdateVB = kTrue );							// update vbs for skinned model?


	void			Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d pD3DDevice 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
							const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
							bool UpdateVB = kTrue );							// update vbs for skinned model?


	void			RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
									int32 UserDataValue,					// userdata value that each segment must have set to be rendered
									const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
									bool UpdateVB = kTrue );							// update vbs for skinned model?

	void			RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,				// direct3d device
									const D3DXMATRIX& ViewportMatrix ,			// viewport matrix
									bool UpdateVB = kTrue );					// update vbs for skinned model?

	void			RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									   const D3DXMATRIX& ViewportMatrix ,		// viewport matrix
									   const D3DXVECTOR3& Light = D3DXVECTOR3( 0, -1, 0 ),	// direction of light for use in normal mapping
									   bool UpdateVB = kTrue );					// update vbs for skinned model?

	void			RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									 bool UpdateVB = kTrue );				// update vbs for skinned model?

	void			UpdateAnimation( float32 TimeElapsed );	// time elapsed in seconds

	void	EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice );			// direct3d device

	void	RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix
						   const D3DXVECTOR3& Light );				// direction of light shadow is being cast from


	void	ClearQueuedAnimations( void );

	void	CopyAnimationState( CSkinnedMesh& pMesh );				// mesh to copy animation state from

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

	uint32		GetMaterials( void )			{	return m_pSkinnedGeometry->GetMaterials();	};

	CMaterial*	GetMaterial( uint32 MaterialID )		{	return m_pSkinnedGeometry->GetMaterial( MaterialID );	};

	CMaterial*	GetMaterialByIndex( uint32 Index )		{	return m_pSkinnedGeometry->GetMaterialByIndex( Index );	};

	int32		GetMaterialIDByIndex( uint32 Index )	{	return m_pSkinnedGeometry->GetMaterialIDByIndex( Index );	};

	void		SetMaterial( uint32 MaterialID,			// id of material to overwrite
							 CMaterial* pMaterial )		// material to set
				{
					m_pSkinnedGeometry->SetMaterial( MaterialID, pMaterial );
				};

	void		SetMaterialByIndex( uint32 Index,				// index of material to overwrite
									CMaterial* pMaterial )		// material to set
				{
					m_pSkinnedGeometry->SetMaterialByIndex( Index, pMaterial );
				};

	std::string	GetMaterialName( uint32 Index )	{	return m_pSkinnedGeometry->GetMaterialName( Index );	};

	const std::string&	CurrentAnimation( void );

	bool				AnimationPlaying( void );
	bool				AnimationPlaying( const std::string& Name );	// name to check the play state of
	bool				AnimationPlaying( int32 Index );				// index of animation to check for

	bool				AnimationQueued( const std::string& Name );	// name to check the play state of
	bool				AnimationQueued( int32 Index );				// index of animation to check for

	uint32				GetAnimationCount( void )				{	return m_Animations.size();			};

	uint32				ActiveAnimations( void )				{	return m_ActiveAnimations.size();	};

	CActiveSkinnedAnimation* ActiveAnimation( uint32 Index )	{	return m_ActiveAnimations[Index];	};

	uint32              GetKeyCount( uint32 AnimationIndex );	// animation to get count for
    
	float32             GetKeyFrame( uint32 AnimationIndex,		// animation key is for
									 uint32 KeyIndex );			// index of key
    
	const std::string&  GetKeyName( uint32 AnimationIndex,		// animation key is for
									uint32 KeyIndex );			// index of key

	uint32              GetKeyIndex( uint32 AnimationIndex, // animation key is for
									 std::string KeyName );	// name of key
    

	D3DXMATRIX			GetTransformedTag( uint32 Index );				// index of tag
	D3DXVECTOR3			GetTagPosition( uint32 Index );				// index of tag

	uint32				GetTagCount( void );

	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	const std::string&	GetTagUserData( uint32 Index );				// index of tag name to retrieve

	D3DXMATRIX			GetTransformedBone( uint32 Index );				// index of bone
	D3DXVECTOR3			GetBonePosition( uint32 Index );				// index of bone

	uint32				GetBoneCount( void );

	const std::string&	GetBoneName( uint32 Index );					// index of bone name to retrieve


	bool	CastsShadows( void );

	const D3DXVECTOR3&		GetMinBounds( void );
	const D3DXVECTOR3&		GetMaxBounds( void );

	void				ClearKeys( void )			{	m_KeysEncountered.clear();			};

	uint32				KeysEncountered( void )		{	return m_KeysEncountered.size();	};
	
	const std::string&	KeyEncounteredName( uint32 Index )
	{
		assert( Index < m_KeysEncountered.size()	);

		return m_KeysEncountered[Index];
	}

	bool				HasReflection( void )
	{
		assert( m_pSkinnedGeometry != NULL );

		return m_pSkinnedGeometry->HasReflection();
	}

	CPose*				CurrentPose( void )			{	return m_pBlendedPose;				};

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

	void	AddKey( uint32 Animation,			// animation the key is for
					const std::string& Name,	// name of key to add
					float32 Frame )				// frame at which the key occurs
	{	
		if( m_KeyNames.size() <= Animation )
		{
			m_KeyNames.resize( Animation + 1 );
			m_KeyFrames.resize( Animation + 1 );
		}
		m_KeyNames[Animation].push_back( Name );
		m_KeyFrames[Animation].push_back( Frame );
	};

	uint32				Keys( uint32 Animation )		{ return m_KeyFrames[Animation].size();	};
	
	float32				KeyFrame( uint32 Animation,	// animation index
								  uint32 Index )	// index of key
	{
		assert( Index < m_KeyFrames[Animation].size() );

		return m_KeyFrames[Animation][Index];
	}
	const std::string&	KeyName( uint32 Animation,	// animation index
								 uint32 Index )		// index of key
	{
		assert( Index < m_KeyNames[Animation].size() );

		return m_KeyNames[Animation][Index];
	}

	void			GenerateBlendedPose( void );

	bool						m_CastsShadows;

	CRefManager&				m_pRefManager;
	CSettings&					m_pSettings;

	CSkinnedGeometry*			m_pSkinnedGeometry;

	CPose*						m_pBlendedPose;

	// Animation properties
	std::vector < CSkinnedAnimation* >	m_Animations;

	std::deque < CActiveSkinnedAnimation* > m_ActiveAnimations;

	std::vector < std::string >	m_KeysEncountered;

	std::vector< std::vector< std::string > >	m_KeyNames;
	std::vector< std::vector< float32 > >		m_KeyFrames;

};	// CSkinnedMesh class


#endif
