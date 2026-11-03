/*
	animatedmesh.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	3/28/2004

	Change History :

	Travis Baldree
	4/17/2004	- added support for animation keys, so that events can be triggered ad
				  specific moments in an animation

	Travis Baldree
	6/23/2004	- stopping animation was not stopping queued animations. Fixed
				  GetAnimationLength now available to find frame counts of animations by index or name


	Travis Baldree
	10/08/2004	-	ams files did not properly load and render meshes with no mesh data (only tags)
					Fixed

	11/1/2004	-	Added support to start playing/blending animations from a specific frame

	11/15/2004	-	Added methods at Paul's suggestion for getting the keyframes of specific keys

	1/18/2004	-	Added methods for getting/setting materials by index instead of material ID

*/
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/textfile.h"
#include "../UTILITIES/settings.h"
#include "AnimatedMesh.h"
#include "animation.h"
#include "tag.h"

CAnimatedMesh::CAnimatedMesh( CRefManager& pRefManager,	// pointer to the reference manager, for tracking submaterials
							  CSettings& pSettings ):		// pointer to the settings manager, for keeping track of card/machine capabilities
							  m_pRefManager( pRefManager ),
							  m_pSettings( pSettings ),
							  m_pAnimatedGeometry( NULL ),
							  m_CastsShadows( kFalse )
{

} // CAnimatedMesh::CAnimatedMesh()

CAnimatedMesh::~CAnimatedMesh()
{
	for( uint32 i = 0; i < m_Animations.size(); i ++ )
	{
		DELETE_SAFELY( m_Animations[i] );
	}

	m_Animations.clear();

	ClearAnimations();

	m_pRefManager.RemoveAnimatedGeometry( m_pAnimatedGeometry );
	m_pAnimatedGeometry = NULL;
	
} // CAnimatedMesh::~CAnimatedMesh()


void CAnimatedMesh::CopyAnimationState( CAnimatedMesh& pMesh )	// mesh to copy animation state from
{
	ClearAnimations();
	for( uint32 i = 0; i < pMesh.ActiveAnimations(); i++ )
	{
		m_ActiveAnimations.push_back( new CActiveAnimation( *pMesh.ActiveAnimation( i ) ) );
	}
} // CAnimatedMesh::CopyAnimationState()

void CAnimatedMesh::SetUserData( int32 Id,			// id of material to set user data on
								 int32 Value )		// value to set for user data
{
	if( m_pAnimatedGeometry != NULL )
	{
		m_pAnimatedGeometry->SetUserData( Id, Value );
	}

} // CMAnimatedMesh::SetUserData()

void CAnimatedMesh::SetUserData( std::string Name,	// name of material to set user data on
								 int32 Value )		// value to set for user data
{
	if( m_pAnimatedGeometry != NULL )
	{
		m_pAnimatedGeometry->SetUserData( Name, Value );
	}

} // CAnimatedMesh::SetUserData()

bool CAnimatedMesh::HasMaterial( std::string Name )	// name of material to check existence of
{
	if( m_pAnimatedGeometry != NULL )
	{
		return m_pAnimatedGeometry->HasMaterial( Name );
	}
	return kFalse;
} // CAnimatedMesh::HasMaterial()

void CAnimatedMesh::Load( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
						  std::string Path,						// path to the model
						  std::string MediaPath,				// path where materials/anim data should be loaded from
						  const std::string& AnimationListPath )	// path to animation data file
{
	DELETE_SAFELY( m_pAnimatedGeometry );

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
	

	m_pAnimatedGeometry = m_pRefManager.AddAnimatedGeometry( pD3DDevice, Path, MediaPath );


	if( AnimationListPath.length() == 0 )
	{
		LoadAnimations( MediaPath + "animation.dat" );
	}
	else
	{
		LoadAnimations( AnimationListPath );
	}

	SetAnimationFrame( 0, 0 );

} // CAnimatedMesh::LoadMesh()

bool CAnimatedMesh::LoadAnimations( std::string Path )	// path to animation File
{


	if ( !FileExists( Path ) )
	{
		return kFalse;
	}

	CTextFile	File;
	File.Open( Path );

	queue<string>	Tokens;

	bool			done	= false;
	while ( !done )
	{
		File.TokenizeNextLine( &Tokens, ":\n\t\r", '#' );	
		if ( File.IsEOF() )
		{
			break;
		}
		if ( Tokens.size() >= 4 )
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
					if( m_pAnimatedGeometry->HasFrameMap() )
					{
						uint32 BestFrame = m_pAnimatedGeometry->NearestMappedFrame( (uint32)KeyFrame );
						KeyFrame -= (float32)m_pAnimatedGeometry->MappedFrame( BestFrame );
						KeyFrame = m_pAnimatedGeometry->CompressionFactor() * (float32)KeyFrame;
						KeyFrame += BestFrame;
					}
					else
					{
						KeyFrame = m_pAnimatedGeometry->CompressionFactor() * KeyFrame;
					}

					m_Animations[AnimationIndex]->AddKey( KeyName, (float32)KeyFrame );
				}
			}
			else // otherwise this is an animation description
			{

				uint32 FirstFrame = atoi( Tokens.front().c_str() );
				Tokens.pop();

				uint32 LastFrame = atoi( Tokens.front().c_str() );
				Tokens.pop();

				float32 FPS = (float32)atof( Tokens.front().c_str() );
				Tokens.pop();
				
				uint32 Frames = LastFrame - FirstFrame;
				// frame mapping is simpler for the artist, and more accurate,
				// - only available in version 3.0
				if( m_pAnimatedGeometry->HasFrameMap() )
				{
					uint32 BestFrame = m_pAnimatedGeometry->NearestMappedFrame( FirstFrame );
					FirstFrame -= m_pAnimatedGeometry->MappedFrame( BestFrame );
					FirstFrame = (int32)floor( m_pAnimatedGeometry->CompressionFactor() * (float32)FirstFrame + .5f);
					FirstFrame += BestFrame;

					BestFrame = m_pAnimatedGeometry->NearestMappedFrame( LastFrame );
					LastFrame -= m_pAnimatedGeometry->MappedFrame( BestFrame );
					LastFrame = (int32)floor( m_pAnimatedGeometry->CompressionFactor() * (float32)LastFrame + .5f);
					LastFrame += BestFrame;
				}
				else
				{
					FirstFrame = (int32)floor( m_pAnimatedGeometry->CompressionFactor() * (float32)FirstFrame + .5f);
					LastFrame = (int32)floor( m_pAnimatedGeometry->CompressionFactor() * (float32)LastFrame + .5f);
				}
				FPS = m_pAnimatedGeometry->CompressionFactor() * FPS;
		
				if( FirstFrame < 0 )
				{
					FirstFrame = 0;
				}
				if( LastFrame < 0 )
				{
					LastFrame = 0;
				}

				if( LastFrame >= m_pAnimatedGeometry->AnimationFrames() )
				{
					LastFrame = m_pAnimatedGeometry->AnimationFrames()-1;
				}

				if( FirstFrame >= m_pAnimatedGeometry->AnimationFrames() )
				{
					FirstFrame = m_pAnimatedGeometry->AnimationFrames()-1;
				}

				m_Animations.push_back( new CAnimation( Name,
														Frames,
														FirstFrame,
														LastFrame,
														FPS ) );
			}
		}

		ClearQueue( Tokens );
	}
	File.Close();

	if( m_Animations.size() == 0 )
	{
		m_Animations.push_back( new CAnimation( "NONE",
												0,
												0,
												0,
												30 ) );
	}

	return kTrue;

} // CAnimatedMesh::LoadAnim()

// is any animation playing?
bool CAnimatedMesh::AnimationPlaying( void )
{
	if( m_ActiveAnimations.size() > 0 )
	{
		if( m_ActiveAnimations[0]->Playing() )
		{
			return kTrue;
		}
	}
	return kFalse;
} // CAnimatedMesh::AnimationPlaying()

// find out if a specific animation is playing
bool CAnimatedMesh::AnimationPlaying( const std::string& Animation ) // name of animation to check for
{
	return AnimationPlaying( GetAnimationIndex( Animation ) );
} // CAnimatedMesh::AnimationPlaying()

// find out if a specific animation is playing
bool CAnimatedMesh::AnimationPlaying( int32 Index ) // index of animation to check for
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
} // CAnimatedMesh::AnimationPlaying()

// find out if a specific animation is Queued
bool CAnimatedMesh::AnimationQueued( const std::string& Animation ) // name of animation to check for
{
	return AnimationQueued( GetAnimationIndex( Animation ) );
} // CAnimatedMesh::AnimationQueued()

// find out if a specific animation is Queued
bool CAnimatedMesh::AnimationQueued( int32 Index ) // index of animation to check for
{
	for( uint32 i = 0; i < m_ActiveAnimations.size(); i++ )
	{
		if( m_ActiveAnimations[i]->QueuedToBlend() )
		{
			if( Index == m_ActiveAnimations[i]->AnimationIndex()  )
			{
				return kTrue;
			}
		}
	}
	return kFalse;
} // CAnimatedMesh::AnimationQueued()

// retrieve the name of the primary playing animation
const std::string&	CAnimatedMesh::CurrentAnimation( void )
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
} // CAnimatedMesh::CurrentAnimation()

void CAnimatedMesh::ClearQueuedAnimations( void )
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
} // CAnimatedMesh::ClearQueuedAnimations()

const std::string&	CAnimatedMesh::GetAnimationName( uint32 Index )	// index of animation to retrieve name of
{
	assert( Index < m_Animations.size() );

	return m_Animations[Index]->Name();
} // GetAnimationName();

int32 CAnimatedMesh::GetAnimationIndex( const std::string& Name )	// name of animation to retrieve index of
{
	for( uint32 i = 0; i < m_Animations.size(); i++ )
	{
		if( m_Animations[i]->Name() == Name )
		{
			return i;
		}
	}

	return -1;

} // CAnimatedMesh::GetAnimationIndex()
															

bool CAnimatedMesh::AnimationExists( const std::string& Name )	// name to check the existence of
{
	if( GetAnimationIndex( Name ) > -1 )
	{
		return kTrue;
	}

	return kFalse;
} // CAnimatedMesh::AnimationExists()


void CAnimatedMesh::PlayAnimation( const std::string& Name,	// name of animation to play
								   bool Looping,			// loop the animation?
								   float32 Speed )			// defaults to 1
{

	PlayAnimation( GetAnimationIndex( Name ) , Looping, Speed );
} // CAnimatedMesh::PlayAnimation()

void CAnimatedMesh::PlayAnimation( int32 Index,	// index of the animation to set
								   bool Looping,	// loop the animation?
								   float32 Speed )	// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	ClearAnimations();

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength = m_Animations[Index]->FrameLength() / Speed;
	}

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   Looping );

	NewAnimation->Play();

	m_ActiveAnimations.push_front( NewAnimation );

} // CAnimatedMesh::PlayAnimation()

void CAnimatedMesh::PlayAnimationFromFrame( const std::string& Name,	// name of animation to play
										    float32 Frame,			// frame to begin at
											bool Looping,			// loop the animation?
											float32 Speed )			// defaults to 1
{

	PlayAnimationFromFrame( GetAnimationIndex( Name ), Frame, Looping, Speed );
} // CAnimatedMesh::PlayAnimationFromFrame()

void CAnimatedMesh::PlayAnimationFromFrame( int32 Index,	// index of the animation to set
										    float32 Frame,			// frame to begin at
											bool Looping,	// loop the animation?
											float32 Speed )	// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	ClearAnimations();

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength = m_Animations[Index]->FrameLength() / Speed;
	}

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   Looping );

	NewAnimation->Play();

	m_ActiveAnimations.push_front( NewAnimation );

	Frame *= m_pAnimatedGeometry->CompressionFactor();

	uint32 Frames = LastFrame - FirstFrame;

	Frame += FirstFrame;

	while( Frame > LastFrame )
	{
		Frame -= Frames;
	}

	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > LastFrame )
	{
		NextFrame = FirstFrame;
	}

	NewAnimation->SetStartFrame( ActualFrame, NextFrame, Interpolation );

} // CAnimatedMesh::PlayAnimationFromFrame()

uint32 CAnimatedMesh::GetAnimationLength( const std::string& Name )	// name of animation to retrieve frame length for
									   
{

	return GetAnimationLength( GetAnimationIndex( Name) );
} // CAnimatedMesh::GetAnimationLength()

uint32 CAnimatedMesh::GetAnimationLength( int32 Index )		// index of animation to retrieve frame length for
									   
{
	assert( (uint32)Index < m_Animations.size() && Index >= 0 );

	if( (uint32)Index >= m_Animations.size() || Index < 0 )
	{
		return 0;
	}

	return m_Animations[Index]->Frames();
} // CAnimatedMesh::GetAnimationLength()

float32 CAnimatedMesh::GetAnimationLengthSeconds( const std::string& Name )	// name of animation to retrieve frame length for
									   
{

	return GetAnimationLengthSeconds( GetAnimationIndex( Name) );
} // CAnimatedMesh::GetAnimationLengthSeconds()

float32 CAnimatedMesh::GetAnimationLengthSeconds( int32 Index )		// index of animation to retrieve frame length for
									   
{
	assert( (uint32)Index < m_Animations.size() && Index >= 0 );

	if( (uint32)Index >= m_Animations.size() || Index < 0 )
	{
		return 0;
	}

	return m_Animations[Index]->Length();
} // CAnimatedMesh::GetAnimationLengthSeconds()

void CAnimatedMesh::SetAnimationFrame( const std::string& Name,	// name of animation to play
									   float32 Frame )			// frame (floating point to interpolate to next frame )
{

	SetAnimationFrame( GetAnimationIndex( Name) , Frame );
} // CAnimatedMesh::SetAnimationFrame()

void CAnimatedMesh::SetAnimationFrame( int32 Index,		// index of the animation to set
									   float32 Frame )	// frame (floating point to interpolate to next frame )
									   
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}
	Frame *= m_pAnimatedGeometry->CompressionFactor();

	ClearAnimations();

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	uint32 Frames = LastFrame - FirstFrame;

	Frame += FirstFrame;

	while( Frame > LastFrame )
	{
		Frame -= Frames;
	}


	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > LastFrame )
	{
		NextFrame = FirstFrame;
	}




	float32 FPS = m_Animations[Index]->FPS();
	float32 FrameLength = m_Animations[Index]->FrameLength();

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   kFalse );

	NewAnimation->SetFrame( ActualFrame, NextFrame, Interpolation );

	m_ActiveAnimations.push_front( NewAnimation );

} // CAnimatedMesh::SetAnimationFrame()

void CAnimatedMesh::QueueBlendAnimation( const std::string& Name,	// name of animation to play
										 bool Looping,	// loop the animation?
										 float32 BlendTime, // time over which to blend
		 								 float32 Speed )	// defaults to 1
{

	QueueBlendAnimation( GetAnimationIndex( Name ) , Looping, BlendTime, Speed );
} // CAnimatedMesh::QueueBlendAnimation()

void CAnimatedMesh::QueueBlendAnimation( int32 Index,	// index of the animation to set
										 bool Looping,	// loop the animation?
										 float32 BlendTime, // time over which to blend
		 								 float32 Speed )	// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength = m_Animations[Index]->FrameLength() / Speed;
	}

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   Looping );

	NewAnimation->QueueBlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

} // CAnimatedMesh::QueueBlendAnimation()

void CAnimatedMesh::BlendAnimation( const std::string& Name,// name of animation to play
									bool Looping,			// loop the animation?
									float32 BlendTime,		// time over which to blend
								    float32 Speed )			// defaults to 1
{

	BlendAnimation( GetAnimationIndex( Name ) , Looping, BlendTime, Speed );
} // CAnimatedMesh::BlendAnimation()

void CAnimatedMesh::BlendAnimation( int32 Index,		// index of the animation to set
								    bool Looping,		// loop the animation?
							  	    float32 BlendTime,  // time over which to blend
								    float32 Speed )		// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength = m_Animations[Index]->FrameLength() / Speed;
	}

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   Looping );

	NewAnimation->BlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

} // CAnimatedMesh::BlendAnimation()

void CAnimatedMesh::QueueBlendAnimationFromFrame( const std::string& Name,	// name of animation to play
												  float32 Frame,	// frame to start from
												  bool Looping,	// loop the animation?
												  float32 BlendTime, // time over which to blend
		 										  float32 Speed )	// defaults to 1
{

	QueueBlendAnimationFromFrame( GetAnimationIndex( Name ), Frame, Looping, BlendTime, Speed );
} // CAnimatedMesh::QueueBlendAnimationFromFrame()

void CAnimatedMesh::QueueBlendAnimationFromFrame( int32 Index,	// index of the animation to set
												  float32 Frame,	// frame to start from
												  bool Looping,	// loop the animation?
												  float32 BlendTime, // time over which to blend
		 										  float32 Speed )	// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength = m_Animations[Index]->FrameLength() / Speed;
	}

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   Looping );

	m_ActiveAnimations.push_front( NewAnimation );

	Frame *= m_pAnimatedGeometry->CompressionFactor();

	uint32 Frames = LastFrame - FirstFrame;

	Frame += FirstFrame;

	while( Frame > LastFrame )
	{
		Frame -= Frames;
	}

	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > LastFrame )
	{
		NextFrame = FirstFrame;
	}

	NewAnimation->SetStartFrame( ActualFrame, NextFrame, Interpolation );

	NewAnimation->QueueBlendIn( BlendTime );

} // CAnimatedMesh::QueueBlendAnimationFromFrame()

void CAnimatedMesh::BlendAnimationFromFrame( const std::string& Name,// name of animation to play
										     float32 Frame,			 // frame to start from
											 bool Looping,			// loop the animation?
											 float32 BlendTime,		// time over which to blend
											 float32 Speed )			// defaults to 1
{

	BlendAnimationFromFrame( GetAnimationIndex( Name ), Frame, Looping, BlendTime, Speed );
} // CAnimatedMesh::BlendAnimationFromFrame()

void CAnimatedMesh::BlendAnimationFromFrame( int32 Index,		// index of the animation to set
											 float32 Frame,		// frame to start from
											 bool Looping,		// loop the animation?
							  				 float32 BlendTime,  // time over which to blend
											 float32 Speed )		// defaults to 1
{
	if( (uint32)Index >= m_Animations.size() || Index < 0)
	{
		return;
	}

	uint32 FirstFrame = m_Animations[Index]->FirstFrame();
	uint32 LastFrame = m_Animations[Index]->LastFrame();

	float32 FPS = m_Animations[Index]->FPS() * Speed;
	float32 FrameLength = m_Animations[Index]->FrameLength();
	if( Speed != 0 )
	{
		FrameLength = m_Animations[Index]->FrameLength() / Speed;
	}

	CActiveAnimation* NewAnimation = new CActiveAnimation( Index,
														   FirstFrame,
														   LastFrame,
														   FPS,
														   FrameLength,
														   Looping );

	NewAnimation->BlendIn( BlendTime );

	m_ActiveAnimations.push_front( NewAnimation );

	Frame *= m_pAnimatedGeometry->CompressionFactor();

	uint32 Frames = LastFrame - FirstFrame;

	Frame += FirstFrame;

	while( Frame > LastFrame )
	{
		Frame -= Frames;
	}

	uint32 ActualFrame = (uint32) floor( Frame );
	float32 Interpolation = Frame-ActualFrame;

	uint32 NextFrame = ActualFrame + 1;
	if( NextFrame > LastFrame )
	{
		NextFrame = FirstFrame;
	}

	NewAnimation->SetStartFrame( ActualFrame, NextFrame, Interpolation );

} // CAnimatedMesh::BlendAnimationFromFrame()

void CAnimatedMesh::StopAnimation( void )
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

} // CAnimatedMesh::StopAnimation()

void CAnimatedMesh::ClearAnimations( void )
{
	for( uint32 i = 0; i < m_ActiveAnimations.size(); i ++ )
	{
		DELETE_SAFELY( m_ActiveAnimations[i] );
	}

	m_ActiveAnimations.clear();

} // CAnimatedMesh::ClearAnimations()


void CAnimatedMesh::UpdateAnimation( float32 TimeElapsed )	// time elapsed in seconds
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
					m_ActiveAnimations[i+1]->TimeFromEnd() * m_pAnimatedGeometry->CompressionFactor() <= m_ActiveAnimations[i]->BlendSpan() )
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

		// for all the animation keys, see if we passed an animation key during animation
		for( uint32 j = 0; j < m_Animations[AnimationIndex]->Keys(); j ++ )
		{
			if( LastAnimationFrame <= m_Animations[AnimationIndex]->KeyFrame( j ) &&
				AnimationFrame >= m_Animations[AnimationIndex]->KeyFrame( j ) )
			{
				// put it on the key-encountered list
				m_KeysEncountered.push_back( m_Animations[AnimationIndex]->KeyName( j ) );
			}
			else if( LastAnimationFrame <= m_Animations[AnimationIndex]->KeyFrame( j ) &&
				AnimationFrame < LastAnimationFrame )
			{
				// put it on the key-encountered list
				m_KeysEncountered.push_back( m_Animations[AnimationIndex]->KeyName( j ) );
			}
			else if( AnimationFrame >= m_Animations[AnimationIndex]->KeyFrame( j ) &&
				AnimationFrame < LastAnimationFrame )
			{
				// put it on the key-encountered list
				m_KeysEncountered.push_back( m_Animations[AnimationIndex]->KeyName( j ) );
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

} // CAnimatedMesh::UpdateAnimation()



void CAnimatedMesh::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
							bool UpdateVB )							// update the vertex buffer?
{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pAnimatedGeometry->Render( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, Light, UpdateVB );

} // CAnimatedMesh::Render()

// renders the model with all non-glowing objects as invisible, but written to the zbuffer.
// all glowing objects are visible, but do NOT write to the zbuffer.
// this is used for post effects like light-streaking.
void CAnimatedMesh::RenderGlowPass( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									bool UpdateVB )							// update the vertex buffer?
									
{
	
	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pAnimatedGeometry->RenderGlowPass( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, UpdateVB );

} // CAnimatedMesh::Render()

// renders the model highlighted
void CAnimatedMesh::RenderIlluminated( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
										const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
										const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
										bool UpdateVB )							// update the vertex buffer?
									
{
	
	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pAnimatedGeometry->RenderIlluminated( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, Light, UpdateVB );

} // CAnimatedMesh::RenderIlluminated()

void CAnimatedMesh::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
							const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
							LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use - NULL if none
							const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
							bool UpdateVB )							// update the vertex buffer?
{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pAnimatedGeometry->Render( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, pCubeMap, Light, UpdateVB );

} // CAnimatedMesh::Render()

void CAnimatedMesh::RenderUserData( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									LPDIRECT3DCUBETEXTURE8 pCubeMap,		// cubemap to use for reflection maps - NULL if none
									int32 UserDataValue,					// userdata value that each segment must have set to be rendered
									const D3DXVECTOR3& Light,				// direction of light for use in normal mapping
									bool UpdateVB )							// update the vertex buffer?

{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pAnimatedGeometry->RenderUserData( pD3DDevice, 
										 CameraSpaceMatrix, 
										 m_ActiveAnimations, 
										 pCubeMap, 
										 UserDataValue, 
										 Light,
										 UpdateVB );

} // CAnimatedMesh::RenderUserData()

void CAnimatedMesh::RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// the direct3d device 
									 const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix 
									 bool UpdateVB )						// update the vertex buffer?
{
	

	D3DXMATRIX matParent;
	D3DXMatrixIdentity(&matParent);
	// Render the character

	m_pAnimatedGeometry->RenderNoTexture( pD3DDevice, CameraSpaceMatrix, m_ActiveAnimations, UpdateVB );

} // CAnimatedMesh::RenderNoTexture()

uint32 CAnimatedMesh::GetKeyCount( uint32 AnimationIndex ) // animation to get count for
{ 
        assert( AnimationIndex < m_Animations.size() ); 

        return m_Animations[AnimationIndex]->Keys(); 
}// CAnimatedMesh::GetKeyCount() 

float32 CAnimatedMesh::GetKeyFrame( uint32 AnimationIndex, // animation key is for
								    uint32 KeyIndex ) 	  // index of key
{ 
        assert( AnimationIndex < m_Animations.size() ); 
        assert( KeyIndex < m_Animations[AnimationIndex]->Keys() ); 
        return m_Animations[AnimationIndex]->KeyFrame(KeyIndex) / 
			   m_pAnimatedGeometry->CompressionFactor(); 
}// CAnimatedMesh::GetKeyFrame() 

const std::string& CAnimatedMesh::GetKeyName( uint32 AnimationIndex, // animation key is for
											  uint32 KeyIndex ) 		// index of key
{ 
        assert( AnimationIndex < m_Animations.size() ); 
        assert( KeyIndex < m_Animations[AnimationIndex]->Keys() ); 
        return m_Animations[AnimationIndex]->KeyName(KeyIndex); 
}// CAnimatedMesh::GetKeyName() 

uint32 CAnimatedMesh::GetKeyIndex( uint32 AnimationIndex, // animation key is for
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
}// CAnimatedMesh::GetKeyIndex() 

D3DXMATRIX CAnimatedMesh::GetTransformedTag( uint32 Index )				// index of tag
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetTransformedTag( Index,
												   m_ActiveAnimations );
} // CAnimatedMesh::GetTransformedTag()

D3DXVECTOR3 CAnimatedMesh::GetTagPosition( uint32 Index )					// index of tag							
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetTagPosition( Index,
												m_ActiveAnimations );
} // CAnimatedMesh::GetTagPosition()

uint32 CAnimatedMesh::GetTagCount( void )	
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetTagCount();
} // CAnimatedMesh::GetTagCount()

const std::string& CAnimatedMesh::GetTagName( uint32 Index )
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetTagName( Index );
} // CAnimatedMesh::GetTagName()

const std::string& CAnimatedMesh::GetTagUserData( uint32 Index )
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetTagUserData( Index );
} // CAnimatedMesh::GetTagUserData()

bool CAnimatedMesh::CastsShadows( void )
{
	if( m_pAnimatedGeometry == NULL )
	{
		return kFalse;
	}

	return m_pAnimatedGeometry->CastsShadows();
} // CAnimatedMesh::CastsShadows()

void CAnimatedMesh::EnableShadows( LPDIRECT3DDEVICE8 pD3DDevice )			// direct3d device
{
	if( m_pAnimatedGeometry == NULL )
	{
		return;
	}
	m_pAnimatedGeometry->EnableShadows( pD3DDevice );
} // CAnimatedMesh::EnableShadows()

// render shadows
void CAnimatedMesh::RenderShadows( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
									   const D3DXMATRIX& CameraSpaceMatrix,	// camera space matrix
									   const D3DXVECTOR3& Light )				// direction of light shadow is being cast from
{
	if( m_pAnimatedGeometry == NULL )
	{
		return;
	}
	m_pAnimatedGeometry->RenderShadows( pD3DDevice,
										CameraSpaceMatrix,
										Light );

} // CAnimatedMesh::RenderShadows()


const D3DXVECTOR3& CAnimatedMesh::GetMinBounds( void )
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetMinBounds();
} // CAnimatedMesh::GetMinBounds()

const D3DXVECTOR3&	CAnimatedMesh::GetMaxBounds( void )
{
	assert( m_pAnimatedGeometry != NULL );

	return m_pAnimatedGeometry->GetMaxBounds();
} // CAnimatedMesh::GetMaxBounds()