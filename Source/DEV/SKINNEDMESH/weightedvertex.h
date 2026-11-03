/*
	weightedvertex.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#ifndef _WEIGHTEDVERTEX_H
#define _WEIGHTEDVERTEX_H


#include <d3d8.h>
#include <d3dx8.h>
#include <string>

// a weighted vertex contains offsets for all of the influencing bones
class CWeightedVertex
{
public :

			CWeightedVertex( void )
			{
			};

			~CWeightedVertex( void ) 
			{
			};


			CWeightedVertex( CWeightedVertex* pVertex )
			{
				for( uint32 i = 0; i < pVertex->Influences(); i++ )
				{
					m_BoneIndices.push_back( pVertex->BoneIndex( i ) );
					m_Offsets.push_back( pVertex->Offset( i ) );
					m_NormalOffsets.push_back( pVertex->NormalOffset( i ) );
					m_Weights.push_back( pVertex->Weight( i ) );
				}
			};

	void	AddInfluence( int32 BoneIndex,				// index of influencing bone
						  float32 Weight,				// weight (0-1) of influence of bone
						  const D3DXVECTOR3& Offset,	// offset of vertex from bone
						  const D3DXVECTOR3& Normal )	// normal offset from bone
	{
		m_BoneIndices.push_back( BoneIndex );
		m_Offsets.push_back( Offset );
		m_NormalOffsets.push_back( Normal );
		m_Weights.push_back( Weight );
	}


	uint32				Influences( void )				{	return m_BoneIndices.size();	};

	int32				BoneIndex( uint32 Index )		{	return m_BoneIndices[Index];	};
	const D3DXVECTOR3&	Offset( uint32 Index )			{	return m_Offsets[Index];		};
	const D3DXVECTOR3&	NormalOffset( uint32 Index )	{	return m_NormalOffsets[Index];	};
	float32				Weight( uint32 Index )			{	return m_Weights[Index];		};

private:

	std::vector < int32 >		m_BoneIndices;
	std::vector < D3DXVECTOR3 > m_Offsets;
	std::vector < D3DXVECTOR3 > m_NormalOffsets;
	std::vector < float32 >		m_Weights;

}; // CWeightedVertex Class

#endif