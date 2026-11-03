/*
	Motion.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Object.h"
#include "WT/System/CRC.h"
#include "WT/System/SmartPointer.h"
#include "WT/Math/Math.h"
#include "WT/Graphics/Animation/Path.h"
#include "WT/Graphics/Animation/MotionSerializer.h"
#include "WT/Graphics/Animation/AnimationException.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/IO/Stream.h"
#include <vector>


namespace WT
{
namespace Graphics
{
namespace Animation
{

// Constants

static const uint32 KMotionVersion = 3;

typedef System::SmartPointer<class CMotion> HMotion;
class CMotion : public System::CObject, public System::Serialization::ISerializable
{
public:
	typedef std::vector<CPath*> Paths;
	typedef CSimplePath<float32> MorphPath;
	typedef System::SmartPointer<MorphPath> HMorphPath;

	WT_BEGIN_CLASS(CMotion, System::CObject)
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable )
	WT_END_CLASS()

	CMotion()
	{
	}

	virtual ~CMotion()
	{
	}

	//
	// Accessors
	//

	string const& GetName() const
	{
		return m_Name;
	}

	string& GetName()
	{
		return m_Name;
	}

	void SetName( string const& Name )
	{
		m_Name = Name;
	}

	int32 GetPathCount() const
	{
		return static_cast<int32>( m_hPaths.size() );
	}
	
	void SetPathCount( int32 Count )
	{
		m_hPaths.resize( Count );
	}

	CPath* GetPath( int32 Index )
	{
		if (Index < 0 || Index >= static_cast<int32>( m_hPaths.size() ))
		{
			throw std::invalid_argument( "path index" );
		}

		return m_hPaths[Index];
	}

	CPath const* GetPath( int32 Index ) const
	{
		if (Index < 0 || Index >= static_cast<int32>( m_hPaths.size() ))
		{
			throw std::invalid_argument( "path index" );
		}

		return m_hPaths[Index];
	}

	void SetPath( int32 Index, CPath& Path )
	{
		if (Index < 0 || Index >= static_cast<int32>( m_hPaths.size() ))
		{
			throw std::invalid_argument( "path index" );
		}

		m_hPaths[Index] = &Path;
	}

	void AddPath( CPath& Path )
	{
		m_hPaths.push_back( &Path );
	}

	int32 GetMorphPathCount() const
	{
		return static_cast<int32>( m_MorphPaths.size() );
	}

	MorphPath* GetMorphPath( int32 Index )
	{
		if (Index < 0 || Index >= static_cast<int32>( m_MorphPaths.size() ))
		{
			throw std::invalid_argument( "morph path index" );
		}

		return m_MorphPaths[Index];
	}

	MorphPath const* GetMorphPath( int32 Index ) const
	{
		if (Index < 0 || Index >= static_cast<int32>( m_MorphPaths.size() ))
		{
			throw std::invalid_argument( "morph path index" );
		}

		return m_MorphPaths[Index];
	}

	void SetMorphPathCount( int32 Count )
	{
		m_MorphPaths.resize( Count, NULL );
	}

	void SetMorphPath( int32 Index, MorphPath* pPath )
	{
		if (Index < 0 || Index >= static_cast<int32>( m_MorphPaths.size() ))
		{
			throw std::invalid_argument( "morph path index" );
		}

		m_MorphPaths[Index] = pPath;
	}

	// Methods

	bool GetTimeExtents(float32& StartTime, float32& EndTime) const
	{
		if (!m_hPaths.empty())
		{
			return m_hPaths[0]->GetTimeExtents( StartTime, EndTime );
		}

		if (!m_MorphPaths.empty())
		{
			return m_MorphPaths[0]->GetTimeExtents( StartTime, EndTime );
		}

		StartTime = 0.0f;
		EndTime = 0.0f;
		return false;
	}

	void RecomputeCheckSum()
	{
		m_BoneCheckSum.Clear();
		for (int32 i = 0; i < GetPathCount(); ++i)
		{
			m_BoneCheckSum.Add( m_hPaths[i]->GetName().c_str(), true );
		}
 	}

	uint32 GetCheckSum() const
	{
		return m_BoneCheckSum.GetCRC();
	}

	static uint32 WTCall GetClassFormatVersion() {return KMotionVersion;}
	virtual uint32 GetObjectFormatVersion( ) const {return GetClassFormatVersion();}

	//
	// ISerializable interface
	//
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private: // data
	System::CRC			m_BoneCheckSum;
	string				m_Name;
	vector<HPath>		m_hPaths;
	vector<HMorphPath>	m_MorphPaths;
};

} // Animation
} // Graphics
} // WT
