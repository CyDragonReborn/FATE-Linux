
/*
	skinnedanimation.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/4/2004

*/

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/utilities.h"
#include "../ANIMATEDMESH/tag.h"
#include "skinnedgeometry.h"
#include "skinnedanimation.h"

CSkinnedAnimation::CSkinnedAnimation( const std::string& Name,	// name of the animation
									  const std::string& Path,	// path to SMA animation file
									  CSkinnedGeometry* pParent,// geometry loading the animation
									  float32 FPS ) :			// frames per second to play at
														m_Name( Name ),
														m_FPS( FPS ),
														m_CompressionFactor( 0 ),
														m_Frames( 0 ),
														m_CompressedFrames( 0 ),
														m_Bones( 0 )
{
	Load( Path, pParent );
}

CSkinnedAnimation::~CSkinnedAnimation( void )
{
	for( uint32 i = 0; i < m_TagList.size(); i++ )
	{
		DELETE_SAFELY( m_TagList[i] );
	}
	m_TagList.clear();
} // CSkinnedAnimation::~CSkinnedAnimation( )


// load a skinned geometry animation file
bool CSkinnedAnimation::Load( std::string FileName,			// path to the SMA file
							  CSkinnedGeometry* pParent )	// geometry loading the animation

{
	FILE *File = NULL;
	
	// Check if file exists
	if ( !FileExists( FileName ) )
	{
		return kFalse;
	}
	
	float32 ModelScale = 1.0f;
	float32 CompressionBias = 1 / 64.0f;

	File = fopen( FileName.c_str(), "rb" );

	std::string Version( "V1.0" );
	Version = ReadString( File );

	ModelScale = ReadFloat( File );	// scale of the entire model
	ModelScale = pParent->Scale();	// override with parent scale

	m_Frames = ReadInt( File );

	m_CompressionFactor = (float32)ReadInt( File );
	m_CompressionFactor = 100 - m_CompressionFactor;
	m_CompressionFactor /= 100.0f;
	if( m_CompressionFactor < .1f )
	{
		m_CompressionFactor = .1f;
	}
	if( m_CompressionFactor > 1.0f )
	{
		m_CompressionFactor = 1.0f;
	}

	m_FPS *= m_CompressionFactor;
	
	uint32 TagCount = ReadInt( File );

	m_CompressedFrames = ReadInt( File );


	// Read in the tags
	std::vector< std::string > TagNames;

	if( TagCount < pParent->GetTagCount() )
	{
		m_TagList.resize( pParent->GetTagCount() );
	}
	else
	{
		m_TagList.resize( TagCount );
	}

	// parse all the tags that this mesh will recognize
	for( uint32 i = 0; i < TagCount; i++ )
	{
		TagNames.push_back( ReadString( File ) );
	}
	std::vector< std::vector< D3DXMATRIX > >	TagXForm( TagCount );
	for( int32 i = 0; i < TagCount; i++ )
	{
		TagXForm[i].resize( m_CompressedFrames );
	}
	for( uint32 k = 0; k < m_CompressedFrames; k++ )
	{
		for( int32 i = 0; i < TagCount; i++ )
		{
			TagXForm[i][k]._41 = ReadFloat( File ) * ModelScale;
			TagXForm[i][k]._42 = ReadFloat( File ) * ModelScale;
			TagXForm[i][k]._43 = ReadFloat( File ) * ModelScale;

			TagXForm[i][k]._11 = -ReadShort( File ) / 32767.0f;
			TagXForm[i][k]._12 = -ReadShort( File ) / 32767.0f;
			TagXForm[i][k]._13 = -ReadShort( File ) / 32767.0f;

			TagXForm[i][k]._31 = ReadShort( File ) / 32767.0f;
			TagXForm[i][k]._32 = ReadShort( File ) / 32767.0f;
			TagXForm[i][k]._33 = ReadShort( File ) / 32767.0f;

			TagXForm[i][k]._21 = ReadShort( File ) / 32767.0f;
			TagXForm[i][k]._22 = ReadShort( File ) / 32767.0f;
			TagXForm[i][k]._23 = ReadShort( File ) / 32767.0f;

		}
	}
	int32* TagIndices = new int32[TagCount];

	for( int32 k = 0; k < m_TagList.size(); k++ )
	{
		m_TagList[k] = NULL;
	}
	// convert the tag Position/orientation data to usable vectors and quaternions,
	// for quick calculation later
	for( int32 k = 0; k < TagCount; k++ )
	{
		int32 index = pParent->TagIndex( TagNames[k] );
		if( index != -1 )
		{
			m_TagList[index] = ( new CTag( TagNames[k] ) );
		}
		else
		{
			index = k;
			m_TagList[index] = ( new CTag( TagNames[k] ) );
		}
		TagIndices[k] = index;
	}

	D3DXVECTOR3		EmptyPosition( 0, 0, 0 );
	D3DXQUATERNION	EmptyOrientation;
	// place empty tags in any vacant slots
	for( int32 k = 0; k < m_TagList.size(); k++ )
	{
		if( m_TagList[k] == NULL )
		{
			m_TagList[k] = ( new CTag( "EMPTY" ) );
			m_TagList[k]->AddPosition( EmptyPosition );
			m_TagList[k]->AddOrientation( EmptyOrientation );
		}
	}

	for( uint32 m = 0; m < TagCount; m++ )
	{
		if( TagIndices[m] != -1 )
		{
			for( uint32 l = 0; l < m_CompressedFrames; l++ )
			{
				m_TagList[TagIndices[m]]->AddPosition( D3DXVECTOR3( TagXForm[m][l]._41,
																	TagXForm[m][l]._42,
																	TagXForm[m][l]._43 ) );
				D3DXQUATERNION Quaternion;
				D3DXQuaternionRotationMatrix( &Quaternion,&TagXForm[m][l] );

				m_TagList[TagIndices[m]]->AddOrientation( Quaternion );
			}
		}
	}
	TagXForm.clear();

	DELETE_SAFELY( TagIndices );

	for( uint32 l = 0; l < m_CompressedFrames; l++ )
	{
		m_FrameMap.push_back( ReadInt( File ) );
	}


	// parse the skeleton information
	m_Bones = ReadInt( File );
	m_BoneTransforms.resize( m_CompressedFrames );

	std::vector< D3DXMATRIX >	BoneXForm( m_Bones );
	for( int32 k = 0; k < m_Bones; k++ )
	{
		m_BoneNames.push_back( ReadString( File ) );
	}


	for( int32 l = 0; l < m_CompressedFrames; l++ )
	{
		m_BoneTransforms[l].resize( m_Bones );

		for( uint32 m = 0; m < m_Bones; m++ )
		{
			D3DXMatrixIdentity( &m_BoneTransforms[l][m] );
			m_BoneTransforms[l][m]._41 = ReadFloat( File ) * ModelScale;
			m_BoneTransforms[l][m]._42 = ReadFloat( File ) * ModelScale;
			m_BoneTransforms[l][m]._43 = ReadFloat( File ) * ModelScale;

			m_BoneTransforms[l][m]._11 = ReadShort( File ) / 32767.0f;
			m_BoneTransforms[l][m]._12 = ReadShort( File ) / 32767.0f;
			m_BoneTransforms[l][m]._13 = ReadShort( File ) / 32767.0f;

			m_BoneTransforms[l][m]._21 = ReadShort( File ) / 32767.0f;
			m_BoneTransforms[l][m]._22 = ReadShort( File ) / 32767.0f;
			m_BoneTransforms[l][m]._23 = ReadShort( File ) / 32767.0f;

			m_BoneTransforms[l][m]._31 = ReadShort( File ) / 32767.0f;
			m_BoneTransforms[l][m]._32 = ReadShort( File ) / 32767.0f;
			m_BoneTransforms[l][m]._33 = ReadShort( File ) / 32767.0f;

			D3DXMATRIX& XForm = m_BoneTransforms[l][m];
		}
	}

	// we're done, close the file
	fclose(File);
	return kTrue;
		
} // CSkinnedAnimation::Load()

D3DXQUATERNION CSkinnedAnimation::GetTagQuaternion( uint32 Index,				// index of tag to get transformed info for
													uint32 CurrentFrame,		// current frame
													uint32 NextFrame,			// next frame
													float32 Interpolation )		// interpolation between the frames
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTagQuaternion( CurrentFrame, 
											   NextFrame, 
											   Interpolation );			
} // CSkinnedAnimation::GetTagQuaternion()

D3DXMATRIX CSkinnedAnimation::GetTransformedTag( uint32 Index,				// index of tag to get transformed info for
												 uint32 CurrentFrame,			// current frame
												 uint32 NextFrame,			// next frame
												 float32 Interpolation )		// interpolation between the frames
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTransformedTag( CurrentFrame, 
												NextFrame, 
												Interpolation );			
} // CSkinnedAnimation::GetTransformedTag()

D3DXVECTOR3	CSkinnedAnimation::GetTagPosition( uint32 Index,			// index of tag to get transformed info for
											   uint32 CurrentFrame,		// current frame
											   uint32 NextFrame,		// next frame
											   float32 Interpolation )	// interpolation between the frames
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->GetTagPosition( CurrentFrame,
											 NextFrame,
											 Interpolation );
} // CSkinnedAnimation::GetTagPosition()

const std::string& CSkinnedAnimation::GetTagName( uint32 Index )
{
	assert( Index < m_TagList.size() );

	return m_TagList[Index]->Name();
} // CSkinnedAnimation::GetTagName()


int32 CSkinnedAnimation::IndexOfBone( const std::string& BoneName )	// name of bone to find
{
	for( uint32 i = 0; i < Bones(); i++ )
	{
		if( m_BoneNames[i] == BoneName )
		{
			return i;
		}
	}
	return -1;
} // CSkinnedAnimation::IndexOfBone()

void CSkinnedAnimation::MapToAnimation( CSkinnedAnimation* pAnimation )	// animation to map to
{
	std::vector< std::vector< D3DXMATRIX > >	BoneTransforms;
	std::vector< std::string >	BoneNames;

	BoneTransforms.resize( m_CompressedFrames );
	for( uint32 i = 0; i < m_CompressedFrames; i++ )
	{
		BoneTransforms[i].resize( Bones() );
	}
	BoneNames.resize( Bones() );

	for( int32 i = 0; i < Bones(); i++ )
	{
		uint32 NewIndex = pAnimation->IndexOfBone( m_BoneNames[i] );
		if( NewIndex != -1 && NewIndex < Bones() )
		{
			for( uint32 j = 0; j < m_CompressedFrames; j++ )
			{
				BoneTransforms[j][NewIndex] = m_BoneTransforms[j][i];
			}
			BoneNames[NewIndex] = m_BoneNames[i];
		}
	}

	m_BoneTransforms = BoneTransforms;
	m_BoneNames = BoneNames;
} // CSkinnedAnimation::MapToAnimation()