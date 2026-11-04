/*
	skinnedmesh.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

	Change History :
	9/25/2004	-	Fixed some issues with cacheing previous animation states. See the skinnedsubmesh for changes
	
	11/1/2004	-	Added support to start playing/blending animations from a specific frame

	11/15/2004	-	Added methods at Paul's suggestion for getting the keyframes of specific keys

	1/18/2004	-	Added methods for getting/setting materials by index instead of material ID

*/


#include <d3dx8.h>
#include <assert.h>
#include <string>

#include "../MEDIA/refmanager.h"

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/textfile.h"
#include "../UTILITIES/settings.h"

#include "../ANIMATEDMESH/tag.h"
#include "skinnedgeometry.h"
#include "skinnedanimation.h"
#include "activeskinnedanimation.h"
#include "skinnedanimation.h"
#include "skinnedmesh.h"
#include "pose.h"

CSkinnedMesh::CSkinnedMesh( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
							CSettings& pSettings ):		// pointer to the settings manager, for keeping track of card/machine capabilities
													m_pRefManager( pRefManager ),
													m_pSettings( pSettings ),
													m_pSkinnedGeometry( NULL ),
													m_pBlendedPose( NULL ),
													m_CastsShadows( kFalse )
{

} // CSkinnedMesh::CSkinnedMesh()

CSkinnedMesh::~CSkinnedMesh()
{
	for( uint32 i = 0; i < m_Animations.size(); i ++ )
	{
		m_pRefManager.RemoveSkinnedAnimation( m_Animations[i] );
	}

	m_Animations.clear();

	ClearAnimations();

	m_pRefManager.RemoveSkinnedGeometry( m_pSkinnedGeometry );
	m_pSkinnedGeometry = NULL;

	DELETE_SAFELY( m_pBlendedPose );
	
} // CSkinnedMesh::~CSkinnedMesh()

void CSkinnedMesh::CopyAnimationState( CSkinnedMesh& pMesh )	// mesh to copy animation state from
{
	ClearAnimations();
	for( uint32 i = 0; i < pMesh.ActiveAnimations(); i++ )
	{
		m_ActiveAnimations.push_back( new CActiveSkinnedAnimation( *pMesh.ActiveAnimation( i ),
																   m_pSkinnedGeometry->Skeleton() ) );
	}
	GenerateBlendedPose();
} // CSkinnedMesh::CopyAnimationState()

void CSkinnedMesh::SetUserData( int32 Id,			// id of material to set user data on
								 int32 Value )		// value to set for user data
{
	if( m_pSkinnedGeometry != NULL )
	{
		m_pSkinnedGeometry->SetUserData( Id, Value );
	}

} // CMSkinnedMesh::SetUserData()

void CSkinnedMesh::SetUserData( std::string Name,	// name of material to set user data on
								 int32 Value )		// value to set for user data
{
	if( m_pSkinnedGeometry != NULL )
	{
		m_pSkinnedGeometry->SetUserData( Name, Value );
	}

} // CSkinnedMesh::SetUserData()

bool CSkinnedMesh::HasMaterial( std::string Name )	// name of material to check existence of
{
	if( m_pSkinnedGeometry != NULL )
	{
		return m_pSkinnedGeometry->HasMaterial( Name );
	}
	return kFalse;
} // CSkinnedMesh::HasMaterial()

void CSkinnedMesh::Load( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
						  std::string Path,						// path to the model
						  std::string MediaPath,				// path where materials/anim data should be loaded from
						  const std::string& AnimationListPath )	// path to animation data file
{
	DELETE_SAFELY( m_pSkinnedGeometry );

	// If no path for textures was provided, construct one
	// from the file path by removing the Path and using its path
	if ( MediaPath.length() == 0 )
	{
		int32	LastSlash	= Path.rfind( "/" );
		int32	LastBackSlash	= Path.rfind( "\\" );
		if( LastSlash > LastBackSlash )
		{
			MediaPath = Path.substr( 0, LastSlash + 1 );
		}
		else
		{
			MediaPath = Path.substr( 0, LastBackSlash + 1 );
		}
	}
	

	m_pSkinnedGeometry = m_pRefManager.AddSkinnedGeometry( pD3DDevice, Path, MediaPath );


	if( AnimationListPath.length() == 0 )
	{
		LoadAnimations( MediaPath + "animation.dat" );
	}
	else
	{
		LoadAnimations( AnimationListPath );
	}

	if( GetAnimationCount() > 0 )
	{
		m_pBlendedPose = new CPose( m_Animations[0]->Bones() );
		m_pSkinnedGeometry->Skeleton()->MapToAnimation( m_Animations[0] );
		for( uint32 i = 1; i < GetAnimationCount(); i++ )
		{
			m_Animations[i]->MapToAnimation( m_Animations[0] );
		}
	}
	else
	{
		m_pBlendedPose = new CPose( m_pSkinnedGeometry->Skeleton()->Bones() );
	}

	SetAnimationFrame( 0, 0 );

} // CSkinnedMesh::LoadMesh()

bool CSkinnedMesh::LoadAnimations( std::string Path )	// path to animation File
{


	if ( !FileExists( Path ) )
	{
		return kFalse;
	}

	CTextFile	File;
	File.Open( Path );

	std::string RootPath = RemoveFileName( Path );

	queue<string>	Tokens;

	bool			done	= false;
	while ( !done )
	{
		File.TokenizeNextLine( &Tokens, ":\n\t\r", '#' );	
		if ( File.IsEOF() )
		{
			break;
		}
		if ( Tokens.size() >= 3 )
		{

			std::string Name = Tokens.front();
			Tokens.pop();

			// this is an animation key!
			if( Name == "<KEY>" )
			{

				std::string AnimationName = Tokens.front();
				Tokens.pop();

				float32 KeyFrame = (float32)atof( Tokens.front().c_str() );
				Tokens.pop();

				std::string KeyName = Tokens.front();
				Tokens.pop();

				int32 AnimationIndex = GetAnimationIndex( AnimationName );
				
				// if this animation exists
				if( AnimationIndex != -1 )
				{
					AddKey( AnimationIndex, KeyName, KeyFrame * m_Animations[AnimationIndex]->CompressionFactor() );
				}
			}
			else // otherwise this is an animation description
			{

				std::string AnimationPath = Tokens.front();
				Tokens.pop();
				AnimationPath = AssembleAbsolutePath( RootPath, AnimationPath );

				float32 FPS = (float32)atof( Tokens.front().c_str() );
				Tokens.pop();

				CSkinnedAnimation* pAnim = m_pRefManager.AddSkinnedAnimation( Name,
																			  AnimationPath,
																			  m_pSkinnedGeometry,
																			  FPS );
				m_Animations.push_back( pAnim );

				if( m_KeyNames.size() < m_Animations.size() )
				{
					m_KeyNames.resize( m_Animations.size() );
					m_KeyFrames.resize( m_Animations.size() );
				}
			}
		}

		ClearQueue( Tokens );
	}
	File.Close();

	return kTrue;

} // CSkinnedMesh::LoadAnim()

// is any animation playing?
bool CSkinnedMesh::AnimationPlaying( void )
{
	if( m_ActiveAnimations.size() > 0 )
	{
		if( m_ActiveAnimations[0]->Playing() )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CSkinnedMesh::AnimationPlaying()

// find out if a specific animation is playing
bool CSkinnedMesh::AnimationPlaying( const std::string& Animation ) // name of animation to check for
{
	return AnimationPlaying( GetAnimationIndex( Animation ) );
} // CSkinnedMesh::AnimationPlaying()

// find out if a specific animation is playing
bool CSkinnedMesh::AnimationPlaying( int32 Index ) // index of animation to check for
{
	for( uint32 i = 0; i < m_ActiveAnimations.size(); i++ )
	{
		if( !m_ActiveAnimations[i]->QueuedToBlend() )
		{
			if( Index == m_ActiveAnimations[i]->AnimationIndex() &&
				m_ActiveAnimations[i]->Playing() )
			{
				return kTrue;
			}
		}
	}
	return kFalse;
} // CSkinnedMesh::AnimationPlaying()

// find out if a specific animation is queued to play
bool CSkinnedMesh::AnimationQueued( const std::string& Animation ) // name of animation to check for
{
	return AnimationQueued( GetAnimationIndex( Animation ) );
} // CSkinnedMesh::AnimationQueued()

// find out if a specific animation is queued to play
bool CSkinnedMesh::AnimationQueued( int32 Index ) // index of animation to check for
{
	for( uint32 i = 0; i < m_ActiveAnimations.size(); i++ )
	{
		if( m_ActiveAnimations[i]->QueuedToBlend() )
		{
			if( Index == m_ActiveAnimations[i]->AnimationIndex() )
			{
				return kTrue;
			}
		}
	}
	return kFalse;
} // CSkinnedMesh::AnimationQueued()

// retrieve the name of the primary playing animation
const std::string&	CSkinnedMesh::CurrentAnimation( void )
{
	int32 Index = -1;
	for( uint32 i = 0; i < m_ActiveAnimations.size(); i++ )
	{
		if( !m_ActiveAnimations[i]->QueuedToBlend() )
		{
			Index = m_ActiveAnimations[i]->AnimationIndex();
			break;
		}
	}
	assert( Index > 0 );
	return m_Animations[Index]->Name();
} // CSkinnedMesh::CurrentAnimation()

void CSkinnedMesh::ClearQueuedAnimations( void )
{
	for( int32 i = 0; i < (int32)m_ActiveAnimations.size(); i++ )
	{
		if( m_ActiveAnimations[i]->QueuedToBlend() )
		{
			DELETE_SAFELY( m_ActiveAnimations[i] );

			for( int32 j = i; j < (int32)m_ActiveAnimations.size() - 1; j++ )
			{
				m_ActiveAnimations[j] = m_ActiveAnimations[j + 1];
			}
			m_ActiveAnimations.pop_back();
			i--;
		}
	}
} // CSkinnedMesh::ClearQueuedAnimations()

const std::string&	CSkinnedMesh::GetAnimationName( uint32 Index )	// index of animation to retrieve name of
{
	assert( Index < m_Animations.size() );

	return m_Animations[Index]->Name();
} // GetAnimationName();

int32 CSkinnedMesh::GetAnimationIndex( const std::string& Name )	// name of animation to retrieve index of
{
	for( uint32 i = 0; i < m_Animations.size(); i++ )
	{
		if( m_Animations[i]->Name() == Name )
		{
			return i;
		}
	}

	return -1;

} // CSkinnedMesh::GetAnimationIndex()
															

bool CSkinnedMesh::AnimationExists( const std::string& Name )	// name to check the existence of
{
	if( GetAnimationIndex( Name ) > -1 )
	{
		return kTrue;
	}

	return kFalse;
} // CSkinnedMesh::AnimationExists()


void CSkinnedMesh::PlayAnimation( const std::string& Name,	// name of animation to play
								  bool Looping,				// loop the animation?
								  float32 Speed )			// defaults to 1
{

	PlayAnimation( GetAnimationIndex( Name ), Looping, Speed );
} // CSkinnedMesh::PlayAnimation()

void CSkinnedMesh::PlayAnimation( int32 Index,	// index of the animation to set
								  bool Looping,	// loop the animation?
								  float32 Speed )	// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	ClearAnimations();


	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength /= Speed;
	}

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																		 0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 Looping );

	NewAnimation->Play();

	m_ActiveAnimations.push_front( NewAnimation );

} // CSkinnedMesh::PlayAnimation()

void CSkinnedMesh::PlayAnimationFromFrame( const std::string& Name,	// name of animation to play
										   float32 Frame,				// frame to start from
										   bool Looping,				// loop the animation?
										   float32 Speed )			// defaults to 1
{

	PlayAnimationFromFrame( GetAnimationIndex( Name ), Frame, Looping, Speed );
} // CSkinnedMesh::PlayAnimation()

void CSkinnedMesh::PlayAnimationFromFrame( int32 Index,		// index of the animation to set
										   float32 Frame,	// frame to start from
										   bool Looping,	// loop the animation?
										   float32 Speed )	// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	ClearAnimations();


	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength /= Speed;
	}

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																		 0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 Looping );

	NewAnimation->Play();

	m_ActiveAnimations.push_front( NewAnimation );

	Frame *= m_Animations[Index]->CompressionFactor();

	while( Frame > m_Animations[Index]->CompressedFrames() )
	{
		Frame = (float32)m_Animations[Index]->CompressedFrames();
	}


	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > m_Animations[Index]->CompressedFrames() )
	{
		NextFrame = 0;
	}

	NewAnimation->SetStartFrame( ActualFrame, NextFrame, Interpolation );

} // CSkinnedMesh::PlayAnimationFromFrame()

uint32 CSkinnedMesh::GetAnimationLength( const std::string& Name )	// name of animation to retrieve frame length for
									   
{

	return GetAnimationLength( GetAnimationIndex( Name) );
} // CSkinnedMesh::GetAnimationLength()

uint32 CSkinnedMesh::GetAnimationLength( int32 Index )		// index of animation to retrieve frame length for
									   
{
	assert( (uint32)Index < m_Animations.size() && Index >= 0 );

	if( (uint32)Index >= m_Animations.size() || Index < 0 )
	{
		return 0;
	}

	return m_Animations[Index]->Frames();
} // CSkinnedMesh::GetAnimationLength()

float32 CSkinnedMesh::GetAnimationLengthSeconds( const std::string& Name )	// name of animation to retrieve frame length for
									   
{

	return GetAnimationLengthSeconds( GetAnimationIndex( Name) );
} // CSkinnedMesh::GetAnimationLengthSeconds()

float32 CSkinnedMesh::GetAnimationLengthSeconds( int32 Index )		// index of animation to retrieve frame length for
									   
{
	assert( (uint32)Index < m_Animations.size() && Index >= 0 );

	if( (uint32)Index >= m_Animations.size() || Index < 0 )
	{
		return 0;
	}

	return m_Animations[Index]->Length();
} // CSkinnedMesh::GetAnimationLengthSeconds()

void CSkinnedMesh::SetAnimationFrame( const std::string& Name,	// name of animation to play
									  float32 Frame )			// frame (floating point to interpolate to next frame )
{

	SetAnimationFrame( GetAnimationIndex( Name) , Frame );
} // CSkinnedMesh::SetAnimationFrame()

void CSkinnedMesh::SetAnimationFrame( int32 Index,		// index of the animation to set
									  float32 Frame )	// frame (floating point to interpolate to next frame )
									   
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	Frame *= m_Animations[Index]->CompressionFactor();

	ClearAnimations();

	while( Frame > m_Animations[Index]->CompressedFrames() )
	{
		Frame = (float32)m_Animations[Index]->CompressedFrames();
	}


	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > m_Animations[Index]->CompressedFrames() )
	{
		NextFrame = 0;
	}




	float32 FPS = m_Animations[Index]->FPS();
	float32 FrameLength = m_Animations[Index]->FrameLength();

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																	     0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 kFalse );

	NewAnimation->SetFrame( ActualFrame, NextFrame, Interpolation );

	m_ActiveAnimations.push_front( NewAnimation );

	UpdateAnimation( 0 );

} // CSkinnedMesh::SetAnimationFrame()

void CSkinnedMesh::QueueBlendAnimation( const std::string& Name,	// name of animation to play
										bool Looping,				// loop the animation?
										float32 BlendTime,			// time over which to blend
									    float32 Speed )				// defaults to 1
{

	QueueBlendAnimation( GetAnimationIndex( Name ), Looping, BlendTime, Speed );
} // CSkinnedMesh::QueueBlendAnimation()

void CSkinnedMesh::QueueBlendAnimation( int32 Index,	// index of the animation to set
										bool Looping,	// loop the animation?
										float32 BlendTime, // time over which to blend
		 								float32 Speed )	// defaults to 1

{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength /= Speed;
	}

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																	     0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 Looping );

	NewAnimation->QueueBlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

} // CSkinnedMesh::QueueBlendAnimation()

void CSkinnedMesh::BlendAnimation( const std::string& Name,// name of animation to play
								   bool Looping,			// loop the animation?
								   float32 BlendTime,		// time over which to blend
								   float32 Speed )			// defaults to 1
{

	BlendAnimation( GetAnimationIndex( Name ) , Looping, BlendTime, Speed );
} // CSkinnedMesh::BlendAnimation()

void CSkinnedMesh::BlendAnimation( int32 Index,			// index of the animation to set
								   bool Looping,		// loop the animation?
								   float32 BlendTime,   // time over which to blend
								   float32 Speed )		// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength /= Speed;
	}

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																		 0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 Looping );

	NewAnimation->BlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

} // CSkinnedMesh::BlendAnimation()

void CSkinnedMesh::StopAnimation( void )
{
	while( m_ActiveAnimations.size() > 1 )
	{
		DELETE_SAFELY( m_ActiveAnimations[0] );
		m_ActiveAnimations.pop_front();
	}

	if( m_ActiveAnimations.size() > 0 )
	{
		m_ActiveAnimations[0]->Stop();
	}

	for( int32 i = m_ActiveAnimations.size() - 1; i > 0; i -- )
	{
		DELETE_SAFELY( m_ActiveAnimations[i] );
		m_ActiveAnimations.pop_back();
	}

} // CSkinnedMesh::StopAnimation()

void CSkinnedMesh::QueueBlendAnimationFromFrame( const std::string& Name,	// name of animation to play
												 float32 Frame,				// frame to start from
												 bool Looping,				// loop the animation?
												 float32 BlendTime,			// time over which to blend
												 float32 Speed )				// defaults to 1
{

	QueueBlendAnimationFromFrame( GetAnimationIndex( Name ), Frame, Looping, BlendTime, Speed );
} // CSkinnedMesh::QueueBlendAnimation()

void CSkinnedMesh::QueueBlendAnimationFromFrame( int32 Index,	// index of the animation to set
												 float32 Frame,				// frame to start from
												 bool Looping,	// loop the animation?
												 float32 BlendTime, // time over which to blend
		 										 float32 Speed )	// defaults to 1

{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength /= Speed;
	}

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																	     0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 Looping );

	NewAnimation->QueueBlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

	Frame *= m_Animations[Index]->CompressionFactor();

	while( Frame > m_Animations[Index]->CompressedFrames() )
	{
		Frame = (float32)m_Animations[Index]->CompressedFrames();
	}


	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > m_Animations[Index]->CompressedFrames() )
	{
		NextFrame = 0;
	}

	NewAnimation->SetStartFrame( ActualFrame, NextFrame, Interpolation );
} // CSkinnedMesh::QueueBlendAnimationFromFrame()

void CSkinnedMesh::BlendAnimationFromFrame( const std::string& Name,// name of animation to play
											float32 Frame,				// frame to start from
											bool Looping,			// loop the animation?
											float32 BlendTime,		// time over which to blend
											float32 Speed )			// defaults to 1
{

	BlendAnimationFromFrame( GetAnimationIndex( Name ), Frame, Looping, BlendTime, Speed );
} // CSkinnedMesh::BlendAnimation()

void CSkinnedMesh::BlendAnimationFromFrame( int32 Index,			// index of the animation to set
											float32 Frame,				// frame to start from
											bool Looping,		// loop the animation?
											float32 BlendTime,   // time over which to blend
											float32 Speed )		// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength /= Speed;
	}

	CActiveSkinnedAnimation* NewAnimation = new CActiveSkinnedAnimation( Index,
																		 m_pSkinnedGeometry->Skeleton(),
																		 m_Animations[Index],
																		 0,
																		 m_Animations[Index]->CompressedFrames(),
																		 FPS,
																		 FrameLength,
																		 Looping );

	NewAnimation->BlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

	Frame *= m_Animations[Index]->CompressionFactor();

	while( Frame > m_Animations[Index]->CompressedFrames() )
	{
		Frame = (float32)m_Animations[Index]->CompressedFrames();
	}


	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > m_Animations[Index]->CompressedFrames() )
	{
		NextFrame = 0;
	}

	NewAnimation->SetStartFrame( ActualFrame, NextFrame, Interpolation );

} // CSkinnedMesh::BlendAnimationFromFrame()

void CSkinnedMesh::ClearAnimations( void )
{
	for( uint32 i = 0; i < m_ActiveAnimations.size(); i ++ )
	{
		DELETE_SAFELY( m_ActiveAnimations[i] );
	}

	m_ActiveAnimations.clear();

} // CSkinnedMesh::ClearAnimations()


void CSkinnedMesh::UpdateAnimation( float32 TimeElapsed )	// time elapsed in seconds
{
	// clear any past encountered keys
	m_KeysEncountered.clear();

	bool RemoveFollowingAnimations = kFalse;
	for( int32 i = 0; i < (int32)m_ActiveAnimations.size(); i++ )
	{
		if( m_ActiveAnimations[i]->QueuedToBlend())
		{
			// if this is next to a playing animation
			if( !( i < (int32)m_ActiveAnimations.size() - 1 &&
 				   m_ActiveAnimations[i+1]->QueuedToBlend() ) )
			{
				if( i == (int32)m_ActiveAnimations.size() - 1 ||
					!m_ActiveAnimations[i+1]->Playing() ||
					( m_ActiveAnimations[i+1]->TimeFromEnd() * 
					  m_ActiveAnimations[i+1]->Animation()->CompressionFactor() ) <= 
					m_ActiveAnimations[i]->BlendSpan() )
				{
					m_ActiveAnimations[i]->Play();
				}
			}
		}

		uint32 AnimationIndex = m_ActiveAnimations[i]->AnimationIndex();

		// frame before update
		float32 LastAnimationFrame = (float32)m_ActiveAnimations[i]->CurrentFrame();
		LastAnimationFrame += m_ActiveAnimations[i]->Interpolation();

		m_ActiveAnimations[i]->Update( TimeElapsed );

		// frame after update
		float32 AnimationFrame = (float32)m_ActiveAnimations[i]->CurrentFrame();
		AnimationFrame += m_ActiveAnimations[i]->Interpolation();

		if( m_ActiveAnimations[i]->Playing() )
		{
			// for all the animation keys, see if we passed an animation key during animation
			for( uint32 j = 0; j < Keys( AnimationIndex ); j ++ )
			{
				if( LastAnimationFrame <= KeyFrame( AnimationIndex, j ) &&
					AnimationFrame >= KeyFrame( AnimationIndex, j ) )
				{
					// put it on the key-encountered list
					m_KeysEncountered.push_back( KeyName( AnimationIndex, j ) );
				}
				else if( LastAnimationFrame <= KeyFrame( AnimationIndex, j ) &&
					AnimationFrame < LastAnimationFrame )
				{
					// put it on the key-encountered list
					m_KeysEncountered.push_back( KeyName( AnimationIndex, j ) );
				}
				else if( AnimationFrame >= KeyFrame( AnimationIndex, j ) &&
					AnimationFrame < LastAnimationFrame )
				{
					// put it on the key-encountered list
					m_KeysEncountered.push_back( KeyName( AnimationIndex, j ) );
				}
			}
		}
		

		if( RemoveFollowingAnimations )
		{
			m_ActiveAnimations[i]->SetComplete();
		}

		if( m_ActiveAnimations[i]->CompletedBlend() )
		{
			RemoveFollowingAnimations = kTrue;
		}
	}

	for( i = 1; i < (int32)m_ActiveAnimations.size(); i++ )
	{
		if( m_ActiveAnimations[i]->Complete() )
		{
			DELETE_SAFELY( m_ActiveAnimations[i] );
			m_ActiveAnimations[i] = m_ActiveAnimations[(int32)m_ActiveAnimations.size() - 1];
			m_ActiveAnimations.pop_back();
			i--;
		}
	}

	GenerateBlendedPose();

} // CSkinnedMesh::UpdateAnimation()

void CSkinnedMesh::GenerateBlendedPose( void )
{
	assert( m_pSkinnedGeometry != NULL );


	if( m_ActiveAnimations.size() == 1)
	{
		m_pBlendedPose->CopyFrom( m_ActiveAnimations[0]->InterpolatedPose() );
		return;
	}


	uint32 Animations = m_ActiveAnimations.size();
	uint32 StartAnimation = 0;

	for( uint32 k = 0; k < Animations; k++ )
	{
		if( !m_ActiveAnimations[k]->QueuedToBlend() ) 
		{
			StartAnimation = k;
			break;
		}
	}

	if( StartAnimation == Animations - 1 )
	{
		m_pBlendedPose->CopyFrom( m_ActiveAnimations[StartAnimation]->InterpolatedPose() );
		return;
	}
						

	m_pBlendedPose->CopyFrom( m_ActiveAnimations[StartAnimation]->InterpolatedPose() );

	float32 CurrentBlend = m_ActiveAnimations[StartAnimation]->BlendPercentage();

	for( uint32 step = StartAnimation + 1; step < Animations; step++ )
	{

		m_pBlendedPose->Interpolate( m_pBlendedPose,
									 m_ActiveAnimations[step]->InterpolatedPose(),
									 CurrentBlend );


		CurrentBlend *= m_ActiveAnimations[step]->BlendPercentage();
	}


} // CSkinnedMesh:GenerateBlendedPose()


void CSkinnedMesh::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
							bool UpdateVB )							// update vbs?
{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pSkinnedGeometry->Render( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, m_pBlendedPose, Light, UpdateVB );

} // CSkinnedMesh::Render()

// renders the model with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void CSkinnedMesh::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
								   const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix 
								   bool UpdateVB )							// update vbs?
									
{
	
	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pSkinnedGeometry->RenderGlowPass( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, m_pBlendedPose, UpdateVB );

} // CSkinnedMesh::Render()

// renders the model highlighted
void CSkinnedMesh::RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
								      const D3DXMATRIX& CameraSpaceMatrix,		// camera space matrix 
									  const D3DXVECTOR3& Light,	// direction of light for use in normal mapping
								      bool UpdateVB )							// update vbs?
									
{
	
	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pSkinnedGeometry->RenderIlluminated( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, m_pBlendedPose, Light, UpdateVB );

} // CSkinnedMesh::Render()

void CSkinnedMesh::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
							const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
							bool UpdateVB )							// update vbs?
{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pSkinnedGeometry->Render( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, m_pBlendedPose, pCubeMap, Light, UpdateVB );

} // CSkinnedMesh::Render()

void CSkinnedMesh::RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
									int32 UserDataValue,					// userdata value that each segment must have set to be rendered
									const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
									bool UpdateVB )							// update vbs?

{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pSkinnedGeometry->RenderUserData( pD3DDevice, 
										 CameraSpaceMatrix, 
										 m_ActiveAnimations, 
										 m_pBlendedPose, 
										 pCubeMap, 
										 UserDataValue, 
										 Light,
										 UpdateVB );

} // CSkinnedMesh::RenderUserData()

void CSkinnedMesh::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									bool UpdateVB )							// update vbs?
{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pSkinnedGeometry->RenderNoTexture( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, m_pBlendedPose, UpdateVB );

} // CSkinnedMesh::RenderNoTexture()

uint32 CSkinnedMesh::GetKeyCount( uint32 AnimationIndex ) // animation to get count for
{ 
        assert( AnimationIndex < m_KeyNames.size() ); 

        return m_KeyNames[AnimationIndex].size(); 
}// CSkinnedMesh::GetKeyCount() 

float32 CSkinnedMesh::GetKeyFrame( uint32 AnimationIndex, // animation key is for
								   uint32 KeyIndex ) 	  // index of key
{ 
        assert( AnimationIndex < m_KeyFrames.size() ); 
        assert( KeyIndex < m_KeyFrames[AnimationIndex].size() ); 
        return m_KeyFrames[AnimationIndex][KeyIndex] / 
			   m_Animations[AnimationIndex]->CompressionFactor(); 
}// CSkinnedMesh::GetKeyFrame() 

const std::string& CSkinnedMesh::GetKeyName( uint32 AnimationIndex, // animation key is for
											 uint32 KeyIndex ) 		// index of key
{ 
        assert( AnimationIndex < m_KeyNames.size() ); 
        assert( KeyIndex < m_KeyNames[AnimationIndex].size() ); 
        return m_KeyNames[AnimationIndex][KeyIndex]; 
}// CSkinnedMesh::GetKeyName() 


uint32 CSkinnedMesh::GetKeyIndex( uint32 AnimationIndex, // animation key is for
								  std::string KeyName )  // name of key
{ 
        for( uint32 i = 0; i < GetKeyCount( AnimationIndex ); i++ ) 
		{
                if( KeyName == GetKeyName( AnimationIndex, i ) ) 
				{
                       return i; 
				}
		}
        return -1; 
}// CSkinnedMesh::GetKeyIndex() 


D3DXMATRIX CSkinnedMesh::GetTransformedTag( uint32 Index )				// index of tag
{
	assert( m_pSkinnedGeometry != NULL );


	if( m_ActiveAnimations.size() == 1)
	{
		return m_ActiveAnimations[0]->Animation()->GetTransformedTag( Index,
																      m_ActiveAnimations[0]->CurrentFrame(),
																      m_ActiveAnimations[0]->NextFrame(),
																      m_ActiveAnimations[0]->Interpolation() );
	}


	uint32 Animations = m_ActiveAnimations.size();
	uint32 StartAnimation = 0;

	for( uint32 k = 0; k < Animations; k++ )
	{
		if( !m_ActiveAnimations[k]->QueuedToBlend() ) 
		{
			StartAnimation = k;
			break;
		}
	}

	if( StartAnimation == Animations-1 )
	{
		return m_ActiveAnimations[StartAnimation]->Animation()->GetTransformedTag( Index,
																				   m_ActiveAnimations[StartAnimation]->CurrentFrame(),
																				   m_ActiveAnimations[StartAnimation]->NextFrame(),
																				   m_ActiveAnimations[StartAnimation]->Interpolation() );
	}


						
	uint32 CurrentFrame = m_ActiveAnimations[StartAnimation]->CurrentFrame();
	uint32 NextFrame = m_ActiveAnimations[StartAnimation]->NextFrame();
	float32 Interpolation = m_ActiveAnimations[StartAnimation]->Interpolation();

	D3DXQUATERNION		FinalQuaternion;
	D3DXQUATERNION		CurrentQuaternion;
	D3DXVECTOR3			FinalPosition;
	D3DXVECTOR3			CurrentPosition;

	CurrentPosition = m_ActiveAnimations[StartAnimation]->Animation()->GetTagPosition( Index,
																					   CurrentFrame,
																					   NextFrame,
																					   Interpolation );

	CurrentQuaternion = m_ActiveAnimations[StartAnimation]->Animation()->GetTagQuaternion( Index,
																					       CurrentFrame,
																					       NextFrame,
																					       Interpolation );


	float32 CurrentBlend = m_ActiveAnimations[StartAnimation]->BlendPercentage();

	for( uint32 step = StartAnimation; step < Animations; step++ )
	{
		CurrentFrame = m_ActiveAnimations[step]->CurrentFrame();
		NextFrame = m_ActiveAnimations[step]->NextFrame();
		Interpolation = m_ActiveAnimations[step]->Interpolation();

		FinalPosition = m_ActiveAnimations[step]->Animation()->GetTagPosition( Index,
																			   CurrentFrame,
																			   NextFrame,
																			   Interpolation );

		FinalQuaternion = m_ActiveAnimations[step]->Animation()->GetTagQuaternion( Index,
																				   CurrentFrame,
																				   NextFrame,
																				   Interpolation );

		D3DXVec3Lerp( &CurrentPosition, 
					  &CurrentPosition, 
					  &FinalPosition,
					  CurrentBlend );

		D3DXQuaternionSlerp( &CurrentQuaternion, 
							 &CurrentQuaternion, 
							 &FinalQuaternion,
							 CurrentBlend );


		CurrentBlend *= m_ActiveAnimations[step]->BlendPercentage();
	}


	D3DXMATRIX CurrentTransformation;
	D3DXMatrixRotationQuaternion( &CurrentTransformation, &CurrentQuaternion );

	CurrentTransformation.m[3][0] = CurrentPosition.x;
	CurrentTransformation.m[3][1] = CurrentPosition.y;
	CurrentTransformation.m[3][2] = CurrentPosition.z;	
	CurrentTransformation.m[3][3] = 1.0f;

	return CurrentTransformation;
} // CSkinnedMesh::GetTransformedTag()

D3DXVECTOR3 CSkinnedMesh::GetTagPosition( uint32 Index )					// index of tag							
{
	assert( m_pSkinnedGeometry != NULL );


	if( m_ActiveAnimations.size() == 1 )
	{
		return m_ActiveAnimations[0]->Animation()->GetTagPosition( Index,
																   m_ActiveAnimations[0]->CurrentFrame(),
																   m_ActiveAnimations[0]->NextFrame(),
																   m_ActiveAnimations[0]->Interpolation() );

	}

	uint32 Animations = m_ActiveAnimations.size();
	uint32 StartAnimation = 0;

	for( uint32 k = 0; k < Animations; k++ )
	{
		if( !m_ActiveAnimations[k]->QueuedToBlend() ) 
		{
			StartAnimation = k;
			break;
		}
	}

	if( StartAnimation == Animations - 1 )
	{
		return m_ActiveAnimations[StartAnimation]->Animation()->GetTagPosition( Index,
																				m_ActiveAnimations[StartAnimation]->CurrentFrame(),
																				m_ActiveAnimations[StartAnimation]->NextFrame(),
																				m_ActiveAnimations[StartAnimation]->Interpolation() );
	}


						
	uint32 CurrentFrame = m_ActiveAnimations[StartAnimation]->CurrentFrame();
	uint32 NextFrame = m_ActiveAnimations[StartAnimation]->NextFrame();
	float32 Interpolation = m_ActiveAnimations[StartAnimation]->Interpolation();

	D3DXVECTOR3		FinalPosition;
	D3DXVECTOR3		CurrentPosition;

	CurrentPosition = m_ActiveAnimations[StartAnimation]->Animation()->GetTagPosition( Index,
																					   CurrentFrame,
																					   NextFrame,
																					   Interpolation );

	float32 CurrentBlend = m_ActiveAnimations[StartAnimation]->BlendPercentage();

	for( uint32 step = StartAnimation; step < Animations; step++ )
	{
		CurrentFrame = m_ActiveAnimations[step]->CurrentFrame();
		NextFrame = m_ActiveAnimations[step]->NextFrame();
		Interpolation = m_ActiveAnimations[step]->Interpolation();

		FinalPosition = m_ActiveAnimations[step]->Animation()->GetTagPosition( Index,
																		       CurrentFrame,
																			   NextFrame,
																			   Interpolation );

		D3DXVec3Lerp( &CurrentPosition, 
					  &CurrentPosition, 
					  &FinalPosition,
					  CurrentBlend );

		CurrentBlend *= m_ActiveAnimations[step]->BlendPercentage();
	}

	return CurrentPosition;

} // CSkinnedMesh::GetTagPosition()

uint32 CSkinnedMesh::GetTagCount( void )	
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->GetTagCount();
} // CSkinnedMesh::GetTagCount()

const std::string& CSkinnedMesh::GetTagName( uint32 Index )
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->GetTagName( Index );
} // CSkinnedMesh::GetTagName()

const std::string& CSkinnedMesh::GetTagUserData( uint32 Index )
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->GetTagUserData( Index );
} // CSkinnedMesh::GetTagUserData()

D3DXMATRIX CSkinnedMesh::GetTransformedBone( uint32 Index )				// index of tag
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pBlendedPose->Transformation( m_pSkinnedGeometry->Skeleton()->AnimationMap( Index ) );
} // CSkinnedMesh::GetTransformedBone()

D3DXVECTOR3 CSkinnedMesh::GetBonePosition( uint32 Index )					// index of tag							
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pBlendedPose->Position( m_pSkinnedGeometry->Skeleton()->AnimationMap( Index ) );

} // CSkinnedMesh::GetBonePosition()

uint32 CSkinnedMesh::GetBoneCount( void )	
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->Skeleton()->Bones();
} // CSkinnedMesh::GetBoneCount()

const std::string& CSkinnedMesh::GetBoneName( uint32 Index )
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->Skeleton()->Bone( Index )->Name();
} // CSkinnedMesh::GetBoneName()

bool CSkinnedMesh::CastsShadows( void )
{
	if( m_pSkinnedGeometry == NULL )
	{
		return kFalse;
	}

	return m_pSkinnedGeometry->CastsShadows();
} // CSkinnedMesh::CastsShadows()

void CSkinnedMesh::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if( m_pSkinnedGeometry == NULL )
	{
		return;
	}
	m_pSkinnedGeometry->EnableShadows( pD3DDevice );
} // CSkinnedMesh::EnableShadows()

// render shadows
void CSkinnedMesh::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									   const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									   const D3DXVECTOR3& Light )				// direction of light shadow is being cast from
{
	if( m_pSkinnedGeometry == NULL )
	{
		return;
	}
	m_pSkinnedGeometry->RenderShadows( pD3DDevice,
										CameraSpaceMatrix,
										Light );

} // CSkinnedMesh::RenderShadows()


const D3DXVECTOR3& CSkinnedMesh::GetMinBounds( void )
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->GetMinBounds();
} // CSkinnedMesh::GetMinBounds()

const D3DXVECTOR3&	CSkinnedMesh::GetMaxBounds( void )
{
	assert( m_pSkinnedGeometry != NULL );

	return m_pSkinnedGeometry->GetMaxBounds();
} // CSkinnedMesh::GetMaxBounds()