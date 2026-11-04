/*
	Motion.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/

#include "WT/stdafx.h"
#include "WT/Graphics/Animation/Motion.h"

namespace WT
{
namespace Graphics
{
namespace Animation
{

WT_DEFINE_CLASS( CMotion )

void CMotion::GetObjectData( System::Serialization::CSerializationInfo* pSerialInfo, 
					System::Serialization::CSerializationContext* pContext ) const
{
	pContext;

	// Create the Motion serializer object
	CMotionSerializer Serializer;

	// Serialize this Motion object
	pSerialInfo->AddContainerBegin( "item" );
	CStream* pStream = pSerialInfo->BeginStreamBlock( "Motion" );
	if (pStream != NULL)
	{
		Serializer.Encode( pStream, this );
		pSerialInfo->EndStreamBlock();
	}
	pSerialInfo->AddContainerEnd();

	pSerialInfo->AddContainerBeginWithCount( "MorphPaths", static_cast<uint32>( m_MorphPaths.size() ) );
	for (int32 i = 0; i < static_cast<int32>( m_MorphPaths.size() ); ++i)
	{
		pSerialInfo->AddValue( L"item", m_MorphPaths[i].get() );
	}
	pSerialInfo->AddContainerEnd();
}

void CMotion::SetObjectData( System::Serialization::CSerializationInfo* pSerialInfo, 
					System::Serialization::CSerializationContext* pContext )
{
	pContext;

	// Create the Motion serializer object
	CMotionSerializer Serializer;
	Serializer.SetVersion( pSerialInfo->GetObjectFormatVersion() );

	// Deserialize this Motion object
	pSerialInfo->GetContainerBegin( "item" );
	CStream* pStream = pSerialInfo->BeginStreamBlock( "Motion" );
	if ( pStream )
	{
		Serializer.Decode( pStream, this );
		pSerialInfo->EndStreamBlock();
	}
	pSerialInfo->GetContainerEnd();

	if (pSerialInfo->GetObjectFormatVersion() >= 3)
	{
		int32 const MorphPathCount = pSerialInfo->GetContainerBeginWithCount( "MorphPaths" );
		
		m_MorphPaths.resize( MorphPathCount );

		for (int32 i = 0; i < MorphPathCount; ++i)
		{
			MorphPath* pPath;
			pSerialInfo->GetValue( L"item", pPath );
			m_MorphPaths[i] = HMorphPath( pPath );
		}

		pSerialInfo->GetContainerEnd();
	}
}

} // namespace Animation
} // namespace Graphics
} // namespace WT
