
#include "d3dx8_shim.h"
/*
	Quadtree.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	CHANGE HISTORY

	Travis Baldree
	6/29/2004		-	LARGE fix - there was a nasty memory leak!

*/


#ifndef _Quadtree_H
#define _Quadtree_H

#include <D3D8.h>
#include <d3dx8.h>
#include <stdio.h>

#include "utilities.h"
#include "macros.h"
#include "frustum.h"

enum EQuadtreeNode
{
	KNodeFrontLeft,
	KNodeFrontRight,
	KNodeBackLeft,
	KNodeBackRight
};

// An Quadtree Node
template <class T>
class CQuadtreeNode
{
public:
	CQuadtreeNode( const D3DXVECTOR3&  MinBounds,		// min bounds of space to be divided
				 const D3DXVECTOR3&  MaxBounds ) :	// max bounds of space to be divided		
				 m_HasChildren( kFalse ),
				 m_MinBounds( MinBounds ),
				 m_MaxBounds( MaxBounds )
	{
		for( uint32 i = 0; i < 4; i++ )
		{
			m_pChildNodes[i] = NULL;
		}

	} // CQuadtreeNode()

	~CQuadtreeNode( void )
	{
		for( uint32 i = 0; i < 4; i++ )
		{
			DELETE_SAFELY( m_pChildNodes[i] );
		}
	} // ~CQuadtreeNode()

	void	AddData( T pData,							// pointer to data to be added to the Quadtree
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
		
			

			m_pChildNodes[KNodeFrontLeft] = new CQuadtreeNode<T>( D3DXVECTOR3( m_MinBounds.x, 
																   m_MinBounds.y,
																   m_MinBounds.z + HalfSize ),
																   D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																   m_MaxBounds.y,
																   m_MaxBounds.z ) );
			
			m_pChildNodes[KNodeFrontRight] = new CQuadtreeNode<T>( D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																   m_MinBounds.y,
																   m_MinBounds.z + HalfSize ),
																   D3DXVECTOR3( m_MaxBounds.x, 
																   m_MaxBounds.y,
																   m_MaxBounds.z ) );
			
			m_pChildNodes[KNodeBackLeft] = new CQuadtreeNode<T>( D3DXVECTOR3( m_MinBounds.x, 
																   m_MinBounds.y,
																   m_MinBounds.z ),
																   D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																   m_MaxBounds.y,
																   m_MinBounds.z + HalfSize ) );
			
			m_pChildNodes[KNodeBackRight] = new CQuadtreeNode<T>( D3DXVECTOR3( m_MinBounds.x + HalfSize, 
																   m_MinBounds.y,
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
		m_pMemberDataMinBounds.push_back( MinBounds );
		m_pMemberDataMaxBounds.push_back( MaxBounds );
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

	bool			Intersects( const D3DXVECTOR3& MinBounds,		// minimum bounds of the object	being checked
								const D3DXVECTOR3& MaxBounds)		// maximum bounds of the object being checked
	{
		return BoundsIntersectXY( m_MinBounds, m_MaxBounds, MinBounds, MaxBounds );
	} // Intersects()
	
	EFrustumIntersect	IntersectsFrustum( CFrustum& pFrustum )			// frustum to intersect with
	{
		return pFrustum.Intersects( m_CullingBounds );
	} // IntersectsFrustum()

	EFrustumIntersect	IntersectsFrustumAABB( CFrustum& pFrustum )			// frustum to intersect with
	{
		return pFrustum.IntersectsAABBXY( m_MinBounds, m_MaxBounds );
	} // IntersectsFrustumAABB()


	void	Sort( CFrustum& pFrustum,					// frustum used to sort data
				  bool SortChildren,					// sort any child nodes, or just dump everything in?												// sort any child nodes, or just dump everything in?
				  std::vector <T>& pSortedData )		// vector to sort data into
	{
		for( uint32 j = 0; j < m_pMemberData.size(); j ++ )
		{
			if( pFrustum.IntersectsAABBXY( m_pMemberDataMinBounds[j], m_pMemberDataMaxBounds[j] ) )
			{
				pSortedData.push_back( m_pMemberData[j] );
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
											pSortedData );
				}
				else
				{
					EFrustumIntersect Result = m_pChildNodes[i]->IntersectsFrustumAABB( pFrustum );

					if( Result == KFrustumInside )
					{
					m_pChildNodes[i]->Sort( pFrustum, 
											kFalse,
											pSortedData );				
					}
					else if( Result == KFrustumIntersect )
					{
					m_pChildNodes[i]->Sort( pFrustum, 
											kTrue,
											pSortedData );				
					}
				}
			}
		}


	} // Sort()

	void	Sort( const D3DXVECTOR3& MinBounds,					// min bounds of area to sort
				  const D3DXVECTOR3& MaxBounds,					// max bounds of area to sort
				  bool SortChildren,					// sort any child nodes, or just dump everything in?												// sort any child nodes, or just dump everything in?
				  std::vector <T>& pSortedData )		// vector to sort data into
	{
		for( uint32 j = 0; j < m_pMemberData.size(); j ++ )
		{
			if( BoundsIntersectXY( MinBounds, MaxBounds, m_pMemberDataMinBounds[j], m_pMemberDataMaxBounds[j] ) )
			{
				pSortedData.push_back( m_pMemberData[j] );
			}
		}

		if( m_HasChildren )
		{
			for( uint32 i = 0; i < 4; i ++ )
			{
				if( !SortChildren )
				{
					m_pChildNodes[i]->Sort( MinBounds, 
											MaxBounds,
											kFalse,
											pSortedData );
				}
				else
				{

					if( m_pChildNodes[i]->ContainedIn( MinBounds, MaxBounds ) )
					{
						m_pChildNodes[i]->Sort( MinBounds, 
												MaxBounds,
												kFalse,
												pSortedData );				
					}
					else if( m_pChildNodes[i]->Intersects( MinBounds, MaxBounds ) )
					{
						m_pChildNodes[i]->Sort( MinBounds, 
												MaxBounds,
												kTrue,
												pSortedData );				
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



private:
	D3DXVECTOR3					m_MaxBounds;
	D3DXVECTOR3					m_MinBounds;
	
	bool						m_HasChildren;

	CQuadtreeNode*				m_pChildNodes[4];
	std::vector <T>				m_pMemberData;	
	std::vector < D3DXVECTOR3 >	m_pMemberDataMinBounds;
	std::vector < D3DXVECTOR3 >	m_pMemberDataMaxBounds;
}; 


// the parent Quadtree -	contains a tree of QuadtreeNodes
template <class T>
class CQuadtree
{
public:
	CQuadtree( D3DXVECTOR3  MinBounds,				// min bounds of space to be divided
			 D3DXVECTOR3  MaxBounds,				// max bounds of space to be divided
			 float32 SmallestNode ) : 				// smallest unit size for nodes
			 m_pParentNode( NULL ),
			 m_MinBounds( MinBounds ),
			 m_MaxBounds( MaxBounds ),
			 m_SmallestNode( SmallestNode )
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

		m_pParentNode = new CQuadtreeNode<T>( MinBounds, MaxBounds );
	} // Quadtree()

	~CQuadtree( void )	
	{
		DELETE_SAFELY( m_pParentNode );
	} // ~Quadtree()


	void	AddData( T pData,							// pointer to data to be added to the Quadtree
					 const D3DXVECTOR3&  MinBounds,		// minimum bounds of the object	being added
					 const D3DXVECTOR3&  MaxBounds )	// maximum bounds of the object being added
	{
		if( m_pParentNode != NULL )
		{
			m_pParentNode->AddData( pData, MinBounds, MaxBounds, m_SmallestNode );
		}
	} // AddData()

	void	Sort( CFrustum& pFrustum,					// frustum used to sort data
				  std::vector <T>& pSortedData )		// vector to sort data into
	{
		if( m_pParentNode != NULL )
		{
			m_pParentNode->Sort( pFrustum, 
								 kTrue,
								 pSortedData );
		}
	} // Sort()

	void	Sort( const D3DXVECTOR3& MinBounds,					// min bounds of area to sort
				  const D3DXVECTOR3& MaxBounds,					// max bounds of area to sort
				  std::vector <T>& pSortedData )				// vector to sort data into
	{
		if( m_pParentNode != NULL )
		{
			m_pParentNode->Sort( MinBounds, 
								 MaxBounds,
								 kTrue,
								 pSortedData );
		}
	} // Sort()

private:
	float32			m_SmallestNode;

	D3DXVECTOR3		m_MaxBounds;
	D3DXVECTOR3		m_MinBounds;

	CQuadtreeNode<T>*	m_pParentNode;
}; // end Quadtree


#endif