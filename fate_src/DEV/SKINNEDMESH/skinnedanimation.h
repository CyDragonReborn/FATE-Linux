
/*
	skinnedanimation.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#ifndef _SKINNEDANIMATION_H
#define _SKINNEDANIMATION_H


#include <string>
#include <assert.h>

class CTag;
class CSkinnedGeometry;

// an skinned animation object maintains all the bone xform data for a single animation

class CSkinnedAnimation
{
public :

			CSkinnedAnimation( const std::string& Name,		// name of the animation
							   const std::string& Path,		// path to SMA animation file
							   CSkinnedGeometry* pParent,	// geometry loading the animation
							   float32 FPS );				// frames per second to play at

			~CSkinnedAnimation( void );


	bool	Load( std::string FileName,			// path to the SMA file
				  CSkinnedGeometry* pParent );	// geometry loading the animation


	const std::string&	Name( void )			{ return m_Name;	};

	uint32				Frames( void )					{ return m_Frames;				};
	uint32				CompressedFrames( void )		{ return m_CompressedFrames;	};

	float32				FPS( void )			{ return m_FPS; };
	float32				FrameLength( void )	{ return 1.0f / m_FPS; };

	float32				Length( void )		{ return (float32)CompressedFrames() * FrameLength();	};

	float32				FrameRatio( void )	{ return (float32)m_FPS / (float32)CompressedFrames();	};


	D3DXQUATERNION	GetTagQuaternion( uint32 Index,				// index of tag to get transformed info for
									  uint32 CurrentFrame,		// current frame
									  uint32 NextFrame,			// next frame
									  float32 Interpolation );	// interpolation between the frames

	D3DXMATRIX	GetTransformedTag( uint32 Index,				// index of tag to get transformed info for
								   uint32 CurrentFrame,			// current frame
								   uint32 NextFrame,			// next frame
								   float32 Interpolation );		// interpolation between the frames


	D3DXVECTOR3	GetTagPosition( uint32 Index,				// index of tag to get transformed info for
								uint32 CurrentFrame,		// current frame
								uint32 NextFrame,			// next frame
								float32 Interpolation );	// interpolation between the frames

	const std::string&	GetTagName( uint32 Index );					// index of tag name to retrieve

	
	float32	CompressionFactor( void )			{	return m_CompressionFactor;	};

	uint32	GetTagCount( void )					{	return m_TagList.size();	};

	bool	HasFrameMap( void )					{	return m_FrameMap.size() > 0;	};

	int32	NearestMappedFrame( int32 UnmappedIndex );	// unmapped frame to find frame mapping for

	uint32	MappedFrame( uint32 Index )			{	return m_FrameMap[Index];	};

	uint32	Bones( void )						{	return m_Bones;	};

	std::vector< D3DXMATRIX >&	TransformationSet( uint32 Frame )	{	return m_BoneTransforms[Frame];		};

	int32	IndexOfBone( const std::string& BoneName );	// name of bone to find

	void	MapToAnimation( CSkinnedAnimation* pAnimation );	// animation to map bone layout to

private:
	std::vector < CTag* >		m_TagList;

	std::vector< std::vector< D3DXMATRIX > >	m_BoneTransforms;
	std::vector< std::string >	m_BoneNames;

	std::vector< int32 >	m_FrameMap;	// keeps track of the original frame indices of compressed frames

	float32				m_CompressionFactor;

	std::string			m_Name;
	uint32				m_Frames;
	uint32				m_CompressedFrames;
	float32				m_FPS;
	uint32				m_Bones;


}; // CAnimation class

#endif