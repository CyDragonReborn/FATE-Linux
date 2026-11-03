/*
	pose.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#ifndef _POSE_H
#define _POSE_H

#include <d3d8.h>
#include <d3dx8.h>
#include <string>



// a pose is a transformation snapshot of a skeleton, used in animations
class CPose
{
public :

			CPose( uint32 Bones )	// number of bones per pose
			{
				m_Transformations.resize( Bones );
				m_Orientations.resize( Bones );
				m_Positions.resize( Bones );
			};

			~CPose( void ) 
			{
				m_Transformations.clear();
				m_Orientations.clear();
				m_Positions.clear();
			};

	const D3DXMATRIX&		Transformation( uint32 Index )	{	return m_Transformations[Index];	};

	const D3DXQUATERNION&	Orientation( uint32 Index )		{	return m_Orientations[Index];	};

	const D3DXVECTOR3&		Position( uint32 Index )		{	return m_Positions[Index];	};

	uint32	Bones( void )									{	return m_Transformations.size();	};

	void	SetTransformation( uint32 Index,						// bone index
							   const D3DXMATRIX& Transformation )	// transformation to set	
			{	
				m_Transformations[Index] = Transformation;	
				// store position and quaternion orientation for quick
				// interpolation later
				m_Positions[Index].x = Transformation._41;
				m_Positions[Index].y = Transformation._42;
				m_Positions[Index].z = Transformation._43;
				D3DXQuaternionRotationMatrix( &m_Orientations[Index], &Transformation );

			};

	void	Interpolate( CPose* pStartingPose,		// pose to start from
					     CPose* pEndingPose,		// pose to interpolate to
						 float32 Interpolation )	// between 0-1
	{
		for( uint32 i = 0; i < m_Transformations.size() ; i++ )
		{
			D3DXQuaternionSlerp( &m_Orientations[i],
								 &pStartingPose->Orientation( i ), 
								 &pEndingPose->Orientation( i ),
								 Interpolation );			


			D3DXVec3Lerp( &m_Positions[i], 
						  &pStartingPose->Position( i ), 
						  &pEndingPose->Position( i ),
						  Interpolation );

			D3DXMatrixRotationQuaternion( &m_Transformations[i], &m_Orientations[i] );

			m_Transformations[i]._41 = m_Positions[i].x;
			m_Transformations[i]._42 = m_Positions[i].y;
			m_Transformations[i]._43 = m_Positions[i].z;	

		}
	}

	void	CopyFrom( CPose* pStartingPose )		// pose to copy from
	{
		for( uint32 i = 0; i < m_Transformations.size() ; i++ )
		{
			m_Transformations[i] = pStartingPose->Transformation( i );
			m_Orientations[i] = pStartingPose->Orientation( i );
			m_Positions[i] = pStartingPose->Position( i );
		}
	}

	bool IsEqual( CPose* pStartingPose )		// pose to check equality to
	{
		if( pStartingPose->Bones() != Bones() )
		{
			return kFalse;
		}
		for( uint32 i = 0; i < m_Transformations.size() ; i++ )
		{
			if( m_Transformations[i] != pStartingPose->Transformation( i ) )
			{
				return kFalse;
			}
		}
		return kTrue;
	}

private:

	std::vector< D3DXMATRIX >		m_Transformations;
	std::vector< D3DXQUATERNION >	m_Orientations;
	std::vector< D3DXVECTOR3 >		m_Positions;
}; // CPose Class

#endif