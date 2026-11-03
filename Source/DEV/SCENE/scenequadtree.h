/*
	SceneQuadtree.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	CHANGE HISTORY

	Travis Baldree
	6/29/2004		-	LARGE fix - there was a nasty memory leak!

*/


#ifndef _SceneQuadtree_H
#define _SceneQuadtree_H

#include <D3D8.h>
#include <d3dx8.h>
#include <stdio.h>

#include "../MEDIA/materialconstants.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/quadtree.h"
#include "subscenesegment.h"

// The primary purpose of the scene octree, and the reason it differs from the template octree in 
// the utilities folder, is that it can sort mesh segments into buckets with unique texture properties
// this is VITAL for speedy scene rendering, so that we don't go through so many texture
// state changes.


// A Scene Octree Node
class CSceneQuadtreeNode
{
public:
	CSceneQuadtreeNode( const D3DXVECTOR3&  MinBounds,		// min bounds of space to be divided
						const D3DXVECTOR3&  MaxBounds ) :			// max bounds of space to be divided		
															m_HasChildren( kFalse ),
															m_MinBounds( MinBounds ),
															m_MaxBounds( MaxBounds )
	{
		CalculateBounds();

		for( uint32 i = 0; i < 4; i++ )
		{
			m_pChildNodes[i] = NULL;
		}

	} // CSceneQuadtreeNode()

	~CSceneQuadtreeNode( void )
	{
		for( uint32 i = 0; i < 4; i++ )
		{
			DELETE_SAFELY( m_pChildNodes[i] );
		}
	} // ~CSceneQuadtreeNode()

	void	AddData( CSubSceneSegment* pData,							// pointer to data to be added to the octree
					 const D3DXVECTOR3& MinBounds,		// minimum bounds of the object	being added
					 const D3DXVECTOR3& MaxBounds,		// maximum bounds of the object being added
					 float32 SmallestNode )				// smallest size node that can be created
	{
		float32	Size = m_MaxBounds.x - m_MinBounds.x;
		float32 HalfSize = Size * .5f;

		// this is the smallest we can go, so we have no children
		if( !HasChildren() &&
			Size > SmallestNode &&
			InBounds( MinBounds, MaxBounds ) )
		{
			// we're going to have child nodes
			m_HasChildren = kTrue;
		

			m_pChildNodes[KNodeFrontLeft] = new CSceneQuadtreeNode( D3DXVECTOR3( m_MinBounds.x, 
																	 m_MinBounds.y + HalfSize,
																	 m_MinBounds.z + HalfSize ),
																	 D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																	 m_MaxBounds.y,
																	 m_MaxBounds.z ) );
			
			m_pChildNodes[KNodeFrontRight] = new CSceneQuadtreeNode( D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																	 m_MinBounds.y + HalfSize,
																	 m_MinBounds.z + HalfSize ),
																	 D3DXVECTOR3( m_MaxBounds.x, 
																	 m_MaxBounds.y,
																	 m_MaxBounds.z ) );
			
			m_pChildNodes[KNodeBackLeft] = new CSceneQuadtreeNode( D3DXVECTOR3( m_MinBounds.x, 
																	 m_MinBounds.y + HalfSize,
																	 m_MinBounds.z ),
																	 D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																	 m_MaxBounds.y,
																	 m_MinBounds.z + HalfSize ) );
			
			m_pChildNodes[KNodeBackRight] = new CSceneQuadtreeNode( D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																	 m_MinBounds.y + HalfSize,
																	 m_MinBounds.z ),
																	 D3DXVECTOR3( m_MaxBounds.x, 
																	 m_MaxBounds.y,
																	 m_MinBounds.z + HalfSize ) );
			
			
		}

		
		// if the data can go in any of the child nodes, place it there
		if( m_HasChildren )
		{
			for( uint32 i = 0; i < 4; i ++ )
			{
				if( m_pChildNodes[i]->InBounds( MinBounds, MaxBounds ) )
				{
					m_pChildNodes[i]->AddData( pData, MinBounds, MaxBounds, SmallestNode );
					return;
				}
			}
		}

		// otherwise it goes in the parent node
		m_pMemberData.push_back( pData );
	} // AddData()

	bool			InBounds( const D3DXVECTOR3& MinBounds,		// minimum bounds of the object	being checked
							  const D3DXVECTOR3& MaxBounds)		// maximum bounds of the object being checked
	{
		return BoundsContainsXY( m_MinBounds, m_MaxBounds, MinBounds, MaxBounds );
	} // InBounds()
	
	bool			ContainedIn( const D3DXVECTOR3& MinBounds,		// minimum bounds of the object	being checked
								 const D3DXVECTOR3& MaxBounds)		// maximum bounds of the object being checked
	{
		return BoundsContainsXY( MinBounds, MaxBounds, m_MinBounds, m_MaxBounds );
	} // ContainedIn()

	EFrustumIntersect	IntersectsFrustum( CFrustum& pFrustum )			// frustum to intersect with
	{
		return pFrustum.Intersects( m_CullingBounds );
	} // IntersectsFrustum()

	EFrustumIntersect	IntersectsFrustumAABB( CFrustum& pFrustum )			// frustum to intersect with
	{
		return pFrustum.IntersectsAABBXY( m_MinBounds, m_MaxBounds );
	} // IntersectsFrustumAABB()

	void	Sort( CFrustum& pFrustum,																	// frustum used to sort data
				  bool SortChildren,																	// sort any child nodes, or just dump everything in?
				  std::vector <std::vector <std::vector <CSubSceneSegment*> > >& pMaterialSegments )
	{

		for( uint32 i = 0; i < m_pMemberData.size(); i++ )
		{
			CSubSceneSegment*	Segment			= m_pMemberData[i];	
			
			if(	!SortChildren ||
				pFrustum.Intersects( Segment->GetCullingBounds() ) != KFrustumOutside )
			{
				uint32				SubMaterialIndex	= m_pMemberData[i]->SubIndex();
				
				pMaterialSegments[Segment->MaterialType()][SubMaterialIndex].push_back( Segment );
			}
		}


		if( m_HasChildren )
		{
			for( uint32 i = 0; i < 4; i ++ )
			{
				if( !SortChildren )
				{
					m_pChildNodes[i]->Sort( pFrustum, 
											kFalse,
											pMaterialSegments );
				}
				else
				{
					EFrustumIntersect Result = m_pChildNodes[i]->IntersectsFrustumAABB( pFrustum );

					if( Result == KFrustumInside )
					{
					m_pChildNodes[i]->Sort( pFrustum, 
											kFalse,
											pMaterialSegments );				
					}
					else if( Result == KFrustumIntersect )
					{
					m_pChildNodes[i]->Sort( pFrustum, 
											kTrue,
											pMaterialSegments );				
					}
				}


			}
		}


	} // Sort()

	bool			HasChildren( void )			{	return m_HasChildren;			};

	bool			HasData( void )
	{
		if( m_pMemberData.size() > 0 )
		{
			return kTrue;
		}

		return kFalse;
	} // HasData()


	void				CalculateBounds( void )
	{
		m_CullingBounds[0] = D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MinBounds.z );
		m_CullingBounds[1] = D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MinBounds.z );
		m_CullingBounds[2] = D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MinBounds.z );
		m_CullingBounds[3] = D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MinBounds.z );
		m_CullingBounds[4] = D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MaxBounds.z );
		m_CullingBounds[5] = D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MaxBounds.z );
		m_CullingBounds[6] = D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MaxBounds.z );
		m_CullingBounds[7] = D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MaxBounds.z );

	} // CalculateBounds()

private:
	D3DXVECTOR3		m_MaxBounds;
	D3DXVECTOR3		m_MinBounds;
	D3DXVECTOR3		m_CullingBounds[8];
	
	bool			m_HasChildren;

	CSceneQuadtreeNode*					m_pChildNodes[4];
	std::vector <CSubSceneSegment*>		m_pMemberData;	
}; 


// the parent Octree -	contains a tree of OctreeNodes
class CSceneQuadtree
{
public:
	CSceneQuadtree( D3DXVECTOR3  MinBounds,				// min bounds of space to be divided
				  D3DXVECTOR3  MaxBounds,				// max bounds of space to be divided
				  float32 SmallestNode ) : 				// smallest unit size for nodes
											m_MinBounds( MinBounds ),
											m_MaxBounds( MaxBounds ),
											m_SmallestNode( SmallestNode ),
											m_pParentNode( NULL )
	{
		// first we need to reshape these bounds into a cube,
		// so we find the length of every axis
		float32 XAxis = (float32)fabs( MaxBounds.x - MinBounds.x );
		float32 ZAxis = (float32)fabs( MaxBounds.z - MinBounds.z );

		// pick the largest axis, so we can expand the others to match
		float32 LargestAxis = XAxis;

		if( ZAxis > LargestAxis )
		{
			LargestAxis = ZAxis;
		}

		// find the difference in size between the largest axis and the others,
		// and expand them by the difference to make a cube

		if( XAxis < LargestAxis )
		{
			float32 HalfDifference = ( LargestAxis - XAxis ) * .5f;

			MinBounds.x -= HalfDifference;
			MaxBounds.x += HalfDifference;
		}

		if( ZAxis < LargestAxis )
		{
			float32 HalfDifference = ( LargestAxis - ZAxis ) * .5f;

			MinBounds.z -= HalfDifference;
			MaxBounds.z += HalfDifference;
		}

		// now it's time to start generating nodes, and dividing space!

		m_pParentNode = new CSceneQuadtreeNode( MinBounds, MaxBounds );
	} // Octree()

	~CSceneQuadtree( void )	
	{
		DELETE_SAFELY( m_pParentNode );
	} // ~Octree()


	void	AddData( CSubSceneSegment* pData,							// pointer to data to be added to the octree
					 const D3DXVECTOR3&  MinBounds,		// minimum bounds of the object	being added
					 const D3DXVECTOR3&  MaxBounds )	// maximum bounds of the object being added
	{
		if( m_pParentNode != NULL )
		{
			m_pParentNode->AddData( pData, MinBounds, MaxBounds, m_SmallestNode );
		}
	} // AddData()

	void	Sort( CFrustum& pFrustum,									// frustum used to sort data
				  std::vector <std::vector <std::vector <CSubSceneSegment*> > >& pMaterialSegments )

	{
		if( m_pParentNode != NULL )
		{
			m_pParentNode->Sort( pFrustum, 
								 kTrue,
								 pMaterialSegments );
		}
	} // Sort()

private:
	float32				m_SmallestNode;

	D3DXVECTOR3			m_MaxBounds;
	D3DXVECTOR3			m_MinBounds;

	CSceneQuadtreeNode*	m_pParentNode;
}; // end SceneQuadtree


#endif