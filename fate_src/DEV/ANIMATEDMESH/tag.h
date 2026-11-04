/*
	tag.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	CHANGE HISTORY

	Travis Baldree
	11/15/2004	-	Transformation data was not returned correctly if animation had only
					one frame - fixed (thanks Paul!)

*/

#ifndef _TAG_H
#define _TAG_H


#include <d3d8.h>
#include <d3dx8.h>
#include <string>
#include "activeanimation.h"

// a tag is used as an attachment point, and maintains its
// animated position and orientation in sync with the rest of the mesh
class CTag
{
public :

			CTag( const std::string& Name ) :		// name of the tag
												m_Name( Name ),
												m_UserData( "" )
			{
				ZeroMemory ( &m_CurrentQuaternion, sizeof ( m_CurrentQuaternion ));
				ZeroMemory ( &m_CurrentTransformation, sizeof ( m_CurrentTransformation ));
				ZeroMemory ( &m_CurrentPosition, sizeof ( m_CurrentPosition ));
			};

			~CTag( void ) 
			{
			};

	const std::string&	Name( void )				{	return m_Name;				};

	const std::string&	UserData( void )						{	return m_UserData;	};
	void				SetUserData( const std::string& Data )	{	m_UserData = Data;	};


	uint32				Frames( void )				{	return m_Positions.size();	};

	void				AddOrientation( const D3DXQUATERNION& Orientation )	{ m_Orientations.push_back( Orientation );	};
	
	void				AddPosition( const D3DXVECTOR3& Position )		{ m_Positions.push_back( Position );	};

	D3DXMATRIX	GetTransformedTag( uint32 CurrentFrame,			// current frame
								   uint32 NextFrame,			// next frame
								   float32 Interpolation )		// interpolation between the frames
	{

		if( Frames() == 0 )
		{
			return m_CurrentTransformation;
		}
		if( CurrentFrame >= Frames() )
		{
			CurrentFrame = Frames() - 1;
		}
		if( NextFrame >= Frames() )
		{
			NextFrame = Frames() - 1;
		}
		D3DXVec3Lerp( &m_CurrentPosition, 
					  &m_Positions[CurrentFrame], 
					  &m_Positions[NextFrame],
					  Interpolation );

		D3DXQuaternionSlerp( &m_CurrentQuaternion, 
							 &m_Orientations[CurrentFrame], 
							 &m_Orientations[NextFrame],
							 Interpolation );

		D3DXMatrixRotationQuaternion( &m_CurrentTransformation, &m_CurrentQuaternion );

		m_CurrentTransformation.m[3][0] = m_CurrentPosition.x;
		m_CurrentTransformation.m[3][1] = m_CurrentPosition.y;
		m_CurrentTransformation.m[3][2] = m_CurrentPosition.z;	
		m_CurrentTransformation.m[3][3] = 1.0f;

		return m_CurrentTransformation;
	}

	D3DXQUATERNION	GetTagQuaternion( uint32 CurrentFrame,		// current frame
									  uint32 NextFrame,			// next frame
									  float32 Interpolation )	// interpolation between the frames
	{
		if( Frames() == 0 )
		{
			return m_CurrentQuaternion;
		}
		if( CurrentFrame >= Frames() )
		{
			CurrentFrame = Frames() - 1;
		}
		if( NextFrame >= Frames() )
		{
			NextFrame = Frames() - 1;
		}

		D3DXVec3Lerp( &m_CurrentPosition, 
					  &m_Positions[CurrentFrame], 
					  &m_Positions[NextFrame],
					  Interpolation );

		D3DXQuaternionSlerp( &m_CurrentQuaternion, 
							 &m_Orientations[CurrentFrame], 
							 &m_Orientations[NextFrame],
							 Interpolation );


		return m_CurrentQuaternion;
	}

	D3DXMATRIX	GetTransformedTag( const std::deque < CActiveAnimation* >& ActiveAnimations ) // animations to blend
	{
		if( ActiveAnimations.size() == 1)
		{
			return GetTransformedTag( ActiveAnimations[0]->CurrentFrame(),
							   ActiveAnimations[0]->NextFrame(),
							   ActiveAnimations[0]->Interpolation() );

		}

		uint32 Animations = ActiveAnimations.size();
		uint32 StartAnimation = 0;

		for( uint32 k = 0; k < Animations; k++ )
		{
			if( !ActiveAnimations[k]->QueuedToBlend() ) 
			{
				StartAnimation = k;
				break;
			}
		}

		if( StartAnimation == Animations-1 )
		{
			return GetTransformedTag( ActiveAnimations[StartAnimation]->CurrentFrame(),
							   ActiveAnimations[StartAnimation]->NextFrame(),
							   ActiveAnimations[StartAnimation]->Interpolation() );
		}


						
		uint32 CurrentFrame = ActiveAnimations[StartAnimation]->CurrentFrame();
		uint32 NextFrame = ActiveAnimations[StartAnimation]->NextFrame();
		float32 Interpolation = ActiveAnimations[StartAnimation]->Interpolation();

		if( Frames() == 0 )
		{
			return m_CurrentTransformation;
		}
		if( CurrentFrame >= Frames() )
		{
			CurrentFrame = Frames() - 1;
		}
		if( NextFrame >= Frames() )
		{
			NextFrame = Frames() - 1;
		}

		D3DXQUATERNION		FinalQuaternion;
		D3DXVECTOR3			FinalPosition;

		D3DXVec3Lerp( &m_CurrentPosition, 
					  &m_Positions[CurrentFrame], 
					  &m_Positions[NextFrame],
					  Interpolation );

		D3DXQuaternionSlerp( &m_CurrentQuaternion, 
							 &m_Orientations[CurrentFrame], 
							 &m_Orientations[NextFrame],
							 Interpolation );



		float32 CurrentBlend = ActiveAnimations[StartAnimation]->BlendPercentage();

		for( uint32 step = StartAnimation; step < Animations; step++ )
		{
				CurrentFrame = ActiveAnimations[step]->CurrentFrame();
				NextFrame = ActiveAnimations[step]->NextFrame();
				Interpolation = ActiveAnimations[step]->Interpolation();

				if( CurrentFrame >= Frames() ||
					NextFrame >= Frames() )
				{
					return m_CurrentTransformation;
				}

				D3DXVec3Lerp( &FinalPosition, 
							  &m_Positions[CurrentFrame], 
							  &m_Positions[NextFrame],
							  Interpolation );

				D3DXQuaternionSlerp( &FinalQuaternion, 
									 &m_Orientations[CurrentFrame], 
									 &m_Orientations[NextFrame],
									 Interpolation );

				D3DXVec3Lerp( &m_CurrentPosition, 
							  &m_CurrentPosition, 
							  &FinalPosition,
							  CurrentBlend );

				D3DXQuaternionSlerp( &m_CurrentQuaternion, 
									 &m_CurrentQuaternion, 
									 &FinalQuaternion,
									 CurrentBlend );


				CurrentBlend *= ActiveAnimations[step]->BlendPercentage();
		}


		D3DXMatrixRotationQuaternion( &m_CurrentTransformation, &m_CurrentQuaternion );

		m_CurrentTransformation.m[3][0] = m_CurrentPosition.x;
		m_CurrentTransformation.m[3][1] = m_CurrentPosition.y;
		m_CurrentTransformation.m[3][2] = m_CurrentPosition.z;	
		m_CurrentTransformation.m[3][3] = 1.0f;

		return m_CurrentTransformation;
	}

	D3DXVECTOR3	GetTagPosition( uint32 CurrentFrame,			// current frame
								uint32 NextFrame,			// next frame
								float32 Interpolation )		// interpolation between the frames
	{
		if( Frames() == 0 )
		{
			return m_CurrentPosition;
		}
		if( CurrentFrame >= Frames() )
		{
			CurrentFrame = Frames() - 1;
		}
		if( NextFrame >= Frames() )
		{
			NextFrame = Frames() - 1;
		}

		D3DXVec3Lerp( &m_CurrentPosition, 
					  &m_Positions[CurrentFrame], 
					  &m_Positions[NextFrame],
					  Interpolation );

		return m_CurrentPosition;
	}

	D3DXVECTOR3	GetTagPosition( const std::deque < CActiveAnimation* >& ActiveAnimations ) // animations to blend
	{

		if( ActiveAnimations.size() == 1)
		{
			return GetTagPosition( ActiveAnimations[0]->CurrentFrame(),
								   ActiveAnimations[0]->NextFrame(),
								   ActiveAnimations[0]->Interpolation() );

		}

		uint32 Animations = ActiveAnimations.size();
		uint32 StartAnimation = 0;

		for( uint32 k = 0; k < Animations; k++ )
		{
			if( !ActiveAnimations[k]->QueuedToBlend() ) 
			{
				StartAnimation = k;
				break;
			}
		}

		if( StartAnimation == Animations-1 )
		{
			return GetTagPosition( ActiveAnimations[StartAnimation]->CurrentFrame(),
								   ActiveAnimations[StartAnimation]->NextFrame(),
								   ActiveAnimations[StartAnimation]->Interpolation() );
		}


						
		uint32 CurrentFrame = ActiveAnimations[StartAnimation]->CurrentFrame();
		uint32 NextFrame = ActiveAnimations[StartAnimation]->NextFrame();
		float32 Interpolation = ActiveAnimations[StartAnimation]->Interpolation();

		if( Frames() == 0 )
		{
			return m_CurrentPosition;
		}
		if( CurrentFrame >= Frames() )
		{
			CurrentFrame = Frames() - 1;
		}
		if( NextFrame >= Frames() )
		{
			NextFrame = Frames() - 1;
		}

		D3DXVECTOR3			FinalPosition;

		D3DXVec3Lerp( &m_CurrentPosition, 
					  &m_Positions[CurrentFrame], 
					  &m_Positions[NextFrame],
					  Interpolation );


		float32 CurrentBlend = ActiveAnimations[StartAnimation]->BlendPercentage();

		for( uint32 step = StartAnimation; step < Animations; step++ )
		{
				CurrentFrame = ActiveAnimations[step]->CurrentFrame();
				NextFrame = ActiveAnimations[step]->NextFrame();
				Interpolation = ActiveAnimations[step]->Interpolation();

				if( Frames() == 0 )
				{
					return m_CurrentPosition;
				}
				if( CurrentFrame >= Frames() )
				{
					CurrentFrame = Frames() - 1;
				}
				if( NextFrame >= Frames() )
				{
					NextFrame = Frames() - 1;
				}

				D3DXVec3Lerp( &FinalPosition, 
							  &m_Positions[CurrentFrame], 
							  &m_Positions[NextFrame],
							  Interpolation );

				D3DXVec3Lerp( &m_CurrentPosition, 
							  &m_CurrentPosition, 
							  &FinalPosition,
							  CurrentBlend );

				CurrentBlend *= ActiveAnimations[step]->BlendPercentage();
		}

		return m_CurrentPosition;
	}

private:
	D3DXQUATERNION		m_CurrentQuaternion;
	D3DXVECTOR3			m_CurrentPosition;
	D3DXMATRIX			m_CurrentTransformation;
	std::string			m_Name;
	std::string			m_UserData;

	std::vector < D3DXQUATERNION > m_Orientations;
	std::vector < D3DXVECTOR3 > m_Positions;

}; // CTag Class

#endif