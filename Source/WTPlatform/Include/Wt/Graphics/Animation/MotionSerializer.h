/*
	MotionSerializer.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	6/7/2005
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/IO/Stream.h"
#include "WT/Graphics/Animation/Skeleton.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace System::IO;

class CMotion;

typedef System::SmartPointer<class CMotionSerializer> HMotionSerializer;
class CMotionSerializer : public System::CObject
{
public:
	WT_CLASS(CMotionSerializer, System::CObject)

	CMotionSerializer() : m_pStream(NULL), m_MotionVersion(0)
	{
	}
	virtual ~CMotionSerializer() {}

	//
	// Accessors
	//

	virtual void SetVersion( uint32 Version )
	{
		m_MotionVersion = Version;
	}

	virtual uint32 GetVersion() const
	{
		return m_MotionVersion;
	}

	virtual void SetStream(CStream * pStream)
	{
		m_pStream = pStream;
	}

	virtual CStream * GetStream()
	{
		return m_pStream;
	}


	// Main methods to Read/Write Motion object to stream
	bool Decode(System::IO::CStream * pStream, CMotion * pMotion);
	int32 Encode(System::IO::CStream * pStream, const CMotion * pMotion);

	// ISerialize interface methods
	void ReadObject(CMotion * pMotion);
	void WriteObject(const CMotion * pMotion);

	//
	// Helper methods
	//

	template <typename T> 
	void Read(T Data[], int32 Count = 1)
	{
		for (int32 i=0; i<Count; ++i)
		{
			if ( m_pStream->Read(reinterpret_cast<uint8*>(Data + i), sizeof(Data[i])) != sizeof(Data[i]) )
			{
				throw WT::System::CArgumentException( "corrupt Motion object stream" );
			}

			//TODO endian correction
		}
	}

	template <typename T>
	void Write(T const Data[], int32 Count = 1)
	{
		for (int32 i = 0; i < Count; ++i)
		{
			//TODO endian correction
			m_pStream->Write( reinterpret_cast<uint8 const*>(Data + i), sizeof(Data[i]) );
		}
	}

	template <typename T>	
	int32 Peek(T Data[], int32 Count = 1)
	{
		int32 TotalBytes = 0;

		int32 i = 0;
		for (; i < Count; ++i)
		{
			int32 BytesRead = m_pStream->Read( reinterpret_cast<uint8*>(Data + i), sizeof(Data[i]) );
			TotalBytes += BytesRead;

			if (BytesRead != sizeof Data[i])
			{
				break;
			}
		}

		//TODO endian correction

		m_pStream->Seek(-TotalBytes, SeekOrigin::Current);
		return i;
	}

	int32 Skip(int32 Bytes)
	{
		int32 OldPosition = m_pStream->GetPosition();
		return m_pStream->Seek(Bytes, System::IO::SeekOrigin::Current) - OldPosition;
	}

private:
	System::IO::CStream *	m_pStream;
	uint32					m_MotionVersion;
};


} // Animation
} // Graphics
} // WT
