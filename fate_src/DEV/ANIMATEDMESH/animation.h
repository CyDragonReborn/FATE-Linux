
/*
	animation.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	3/28/2004

	Change History :

	Travis Baldree
	4/17/2004	- added support for animation keys, so that events can be triggered ad
				  specific moments in an animation

*/

#ifndef _ANIMATION_H
#define _ANIMATION_H


#include <string>
#include <assert.h>


// an animation object maintains information about a specific animation
// this is not an instance, but the actual description of the animation itself
class CAnimation
{
public :

			CAnimation( const std::string& Name,	// name of the animation
						uint32 Frames,				// frame count (uncompressed)
						uint32 FirstFrame,			// first frame within the master animation
						uint32 LastFrame,			// last frame within the master animation
						float32 FPS ) :				// frames per second to play at
						m_Frames( Frames ),
						m_Name( Name ),
						m_FirstFrame( FirstFrame ),
						m_LastFrame( LastFrame ),
						m_FPS( FPS )
			{
			};

			~CAnimation( void ) 
			{
			};

	void				AddKey( const std::string& Name,	// name of key to add
								float32 Frame )				// frame at which the key occurs
	{	
		m_KeyNames.push_back( Name );
		m_KeyFrames.push_back( Frame );
	};

	const std::string&	Name( void )		{ return m_Name;		};
	uint32				FirstFrame( void )	{ return m_FirstFrame;	};
	uint32				LastFrame( void )	{ return m_LastFrame;	};
	uint32				Frames( void )		{ return m_Frames;		};

	float32				Length( void )		{	return (float32)Frames() * FrameLength();	};

	float32				FPS( void )			{ return m_FPS; };
	float32				FrameLength( void )	{ return 1.0f / m_FPS; };

	float32				FrameRatio( void )	{ return (float32)m_FPS / (float32)m_Frames;	};

	uint32				Keys( void )		{ return m_KeyFrames.size();	};
	
	float32				KeyFrame( uint32 Index )	// index of key
	{
		assert( Index < m_KeyFrames.size() );

		return m_KeyFrames[Index];
	}
	const std::string&	KeyName( uint32 Index )		// index of key
	{
		assert( Index < m_KeyNames.size() );

		return m_KeyNames[Index];
	}
private:
	std::vector< std::string >	m_KeyNames;
	std::vector< float32 >		m_KeyFrames;

	std::string			m_Name;
	uint32				m_FirstFrame;
	uint32				m_LastFrame;
	uint32				m_Frames;
	float32				m_FPS;

}; // CAnimation class

#endif