/*
	skeleton.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#ifndef _SKELETON_H
#define _SKELETON_H

#include "d3dx8_shim.h"

#include <d3d8.h>
#include <d3dx8.h>
#include <string>
#include "bone.h"
#include "pose.h"
#include "skinnedanimation.h"

// a skeleton is simply a heirarchy of bones
class CSkeleton
{
public :

			CSkeleton( void )
			{
				D3DXMATRIX Identity;
				D3DXMatrixIdentity( &Identity );
				m_pRootBone = new CBoneNode( "SCRoot",
											 Identity );
			};

			~CSkeleton( void ) 
			{
				DELETE_SAFELY( m_pRootBone );
				m_pBoneMap.clear();
			};

	bool	BoneExists( const std::string& BoneName )	// name of bone to find
	{
		return m_pRootBone->BoneExists( BoneName );
	}

	void	AddBone( CBoneNode* pBone,					// bone to add
					 uint32 Index,						// index in bone map
					 const std::string& ParentName )	// name of parent bone ("" for root)
	{
		if( ParentName == "" )
		{
			m_pRootBone->AddChild( pBone );
		}
		else if( BoneExists( ParentName ) )
		{
			CBoneNode* pParent = m_pRootBone->GetBone( ParentName );
			if( pParent != NULL )
			{
				pParent->AddChild( pBone );
			}
		}
		if( Index >= m_pBoneMap.size() )
		{
			m_pBoneMap.resize( Index + 1 );
			m_BoneAnimationMap.resize( Index + 1 );
		}
		m_pBoneMap[Index] = pBone;
		m_BoneAnimationMap[Index] = Index;
	}

	int32		RootBoneIndex( void )
	{
		for( uint32 i = 0; i < m_pBoneMap.size(); i++ )
		{
			if( m_pBoneMap[i]->Parent() == m_pRootBone )
			{
				return i;
			}
		}
		return 0;
	}

	void MapToAnimation( CSkinnedAnimation* pAnimation )	// animation to map bones to
	{
		for( uint32 i = 0; i < Bones(); i++ )
		{
			int32 BoneIndex = pAnimation->IndexOfBone( m_pBoneMap[i]->Name() );
			if( BoneIndex != -1 )
			{
				m_BoneAnimationMap[i] = BoneIndex;
			}

		}
	}

	CBoneNode*	Bone( uint32 Index )		{	return m_pBoneMap[Index];	};

	uint32  AnimationMap( uint32 Bone )		{	return m_BoneAnimationMap[Bone];	};

	void	UpdateTransformation( void )
	{
		D3DXMATRIX Identity;
		D3DXMatrixIdentity( &Identity );
		m_pRootBone->UpdateTransformation( Identity );
	}

	uint32		Bones( void )				{	return m_pBoneMap.size();	};

	void	SetPose( CPose*	pPose,										// pose to set
					 std::vector< D3DXMATRIX >& BoneTransformations )	// transformations to apply
	{
		// the animation pose must match our bone map
		if( pPose->Bones() < Bones() )
		{
			return;
		}
		uint32 PoseSize = BoneTransformations.size();
		for( uint32 i = 0; i < Bones(); i++ )
		{
			if( i < PoseSize )
			{
				m_pBoneMap[i]->SetTransformation( BoneTransformations[m_BoneAnimationMap[i]] );
			}
		}
		UpdateTransformation();

		for( int32 i = 0; i < Bones(); i++ )
		{
			pPose->SetTransformation( m_BoneAnimationMap[i], m_pBoneMap[i]->WorldspaceTransformation() );
		}
	}

					 

private:

	CBoneNode*	m_pRootBone;

	std::vector< CBoneNode* >	m_pBoneMap;
	std::vector< uint32 >		m_BoneAnimationMap;	// maps bones to the counterparts in an animation
}; // CSkeleton Class

#endif