/*
	activeanimation.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	3/28/2004

	Change History :


*/

#ifndef _ACTIVEANIMATION_H
#define _ACTIVEANIMATION_H


#include <string>



// an ActiveAnimation maintains the current state of a playing animation, and its
// blend with other animations
class CActiveAnimation
{
public :

			CActiveAnimation(  uint32 Index,		// index of the original animation
							   uint32 FirstFrame,	// first frame of the animation
							   uint32 LastFrame,	// last frame of the animation
							   float32 FPS,			// frames per second
							   float32 FrameLength,	// frame length, in seconds
							   bool Looping ) :
												m_AnimationIndex( Index ),
												m_Interpolation( 0.0f ),
												m_FrameTick( 0.0f ),
												m_BlendInTime( 0.0f ),
												m_CurrentTime ( 0.0f ),
												m_TotalBlendInTime ( 0.0f ),
												m_BlendPercentage ( 0.0f ),
												m_FirstFrame( FirstFrame ),
												m_LastFrame( LastFrame ),
												m_CurrentFrame( FirstFrame ),
												m_NextFrame( FirstFrame ),
												m_FPS( FPS ),
												m_FrameLength( FrameLength ),
												m_AnimationLooping( Looping ),
												m_JustLooped( kFalse ),
												m_Playing( kFalse ),
												m_AnimationComplete( kFalse ),
												m_QueuedToBlend( kFalse ),
												m_CompletedBlend( kFalse )
			{
			};

			~CActiveAnimation( void ) 
			{
			};

			CActiveAnimation( CActiveAnimation& pAnimation )	// animation to copy
			{
				m_AnimationIndex = pAnimation.m_AnimationIndex;
				m_FirstFrame = pAnimation.m_FirstFrame;
				m_LastFrame = pAnimation.m_LastFrame;
				m_CurrentFrame = pAnimation.m_CurrentFrame;
				m_NextFrame = pAnimation.m_NextFrame;

				m_FPS = pAnimation.m_FPS;
				m_Interpolation = pAnimation.m_Interpolation;
				m_FrameLength = pAnimation.m_FrameLength;
				m_FrameTick = pAnimation.m_FrameTick;
				m_CurrentTime = pAnimation.m_CurrentTime;

				m_AnimationLooping = pAnimation.m_AnimationLooping;
				m_Playing = pAnimation.m_Playing;
				m_QueuedToBlend = pAnimation.m_QueuedToBlend;

				m_AnimationComplete = pAnimation.m_AnimationComplete;		
				m_CompletedBlend = pAnimation.m_CompletedBlend;
				m_JustLooped = pAnimation.m_JustLooped;

				m_BlendInTime = pAnimation.m_BlendInTime;
				m_TotalBlendInTime = pAnimation.m_TotalBlendInTime;
				m_BlendPercentage = pAnimation.m_BlendPercentage;
			};

	void				Play( void )		
						{	
							m_Playing = kTrue; 
							m_QueuedToBlend = kFalse;
							m_NextFrame = m_CurrentFrame + 1;
							if ( m_NextFrame >= m_LastFrame )
							{
								m_NextFrame = m_FirstFrame;
							}
						};

	void				Stop( void )		
						{	
							m_Playing = kFalse;	
							m_QueuedToBlend = kFalse;
						};

	void				BlendIn( float32 Time )				// time in seconds to blend in	
						{	
							Play();
							m_BlendInTime = Time;	
							m_TotalBlendInTime = Time;
							m_BlendPercentage = 1;
						};


	void				QueueBlendIn( float32 Time )		// time in seconds to blend in
						{
							m_Playing = kFalse;
							m_QueuedToBlend = kTrue;
							if( Time <= 0 )
							{
								Time = .0001f;
							}
							m_BlendInTime = Time;
							m_TotalBlendInTime = Time;
							m_BlendPercentage = 1;
						}

	void				SetFrame( uint32 Frame,				// frame to set
								  uint32 NextFrame,			// next frame
								  float32 Interpolation )	// interpolation between the two
						{	
							m_AnimationLooping = kFalse;
							m_Playing = kFalse;	
							m_CurrentFrame = Frame;
							m_NextFrame = NextFrame;
							m_Interpolation = Interpolation;
							m_FrameTick = m_Interpolation/m_FPS;
						};

	void				SetStartFrame( uint32 Frame,				// frame to set
									   uint32 NextFrame,			// next frame
									   float32 Interpolation )	// interpolation between the two
						{	
							m_CurrentFrame = Frame;
							m_NextFrame = NextFrame;
							m_Interpolation = Interpolation;
						};

	void				SetLooping( bool State )		{	m_AnimationLooping = State;	};

	bool				IsBlending( void )
						{
							if( m_BlendInTime > 0 )
							{
								return kTrue;
							}
							return kFalse;
						}

	void				Update( float32 TimeElapsed )	// time elapsed in seconds
						{
							if( m_Playing )
							{
								m_JustLooped = kFalse;


								if( m_BlendInTime > 0 )
								{
									m_BlendInTime -= TimeElapsed;
									m_BlendPercentage = 0;
									if( m_TotalBlendInTime != 0 )
									{
										m_BlendPercentage = m_BlendInTime / m_TotalBlendInTime;
									}
									if( m_BlendInTime <= 0 )
									{
										m_CompletedBlend = kTrue;
										m_BlendPercentage = 0;
										m_BlendInTime = 0;
										return;
									}
								}

								m_FrameTick += TimeElapsed;
								
								if( m_LastFrame == m_FirstFrame )
								{
									m_FrameTick = 0;
									m_CurrentFrame = m_FirstFrame;
									m_NextFrame = m_CurrentFrame;
								}
								else
								{
									while ( m_FrameTick >= m_FrameLength )
									{
										m_FrameTick -= m_FrameLength;
										m_CurrentFrame = m_NextFrame;
										m_NextFrame ++;
										
										if ( m_NextFrame >= m_LastFrame )
										{
											if( m_AnimationLooping )
											{
												m_NextFrame = m_FirstFrame;
												m_JustLooped = kTrue;
											}
											else
											{
												m_NextFrame = m_LastFrame;
												m_AnimationComplete = kTrue;
												m_Playing = kFalse;
												return;
											}
										}
									}
								}

								m_Interpolation = m_FPS*m_FrameTick;

							}
						}

	bool				Complete( void )		{	return m_AnimationComplete;	};
	void				SetComplete( void )		{	m_AnimationComplete = kTrue;};
	bool				QueuedToBlend( void )	{	return m_QueuedToBlend;		};
	bool				Playing( void )			{	return m_Playing;			};

	uint32				AnimationIndex( void )	{	return m_AnimationIndex;	};

	uint32				CurrentFrame( void )	{	return m_CurrentFrame;		};
	uint32				NextFrame( void )		{	return m_NextFrame;			};
	float32				Interpolation( void )	{	return m_Interpolation;		};
	float32				BlendPercentage( void )	{	return m_BlendPercentage;	};
	bool				CompletedBlend( void )	{	return m_CompletedBlend;	};
	float32				BlendSpan( void	)		{	return m_TotalBlendInTime;	};

	float32				TimeFromEnd( void )
						{
							if( m_JustLooped )
							{
								return 0;
							}
							float32 FrameDelta = (float32)m_LastFrame-m_CurrentFrame;
							FrameDelta += m_Interpolation;
							FrameDelta *= m_FrameLength;
							return FrameDelta;
						}

protected:
	uint32				m_AnimationIndex;
	int32				m_FirstFrame;
	int32				m_LastFrame;
	int32				m_CurrentFrame;
	int32				m_NextFrame;

	float32				m_FPS;
	float32				m_Interpolation;
	float32				m_FrameLength;
	float32				m_FrameTick;
	float32				m_CurrentTime;

	bool				m_AnimationLooping;
	bool				m_Playing;
	bool				m_QueuedToBlend;

	bool				m_AnimationComplete;		// are we ready to remove this animation?
	bool				m_CompletedBlend;
	bool				m_JustLooped;

	float32				m_BlendInTime;
	float32				m_TotalBlendInTime;
	float32				m_BlendPercentage;

}; // CActiveAnimation class

#endif