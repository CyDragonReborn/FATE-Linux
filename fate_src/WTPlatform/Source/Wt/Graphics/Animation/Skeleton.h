/*
	Skeleton.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	4/5/2005
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/ISerializable.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Graphics/Animation/Bone.h"
#include "WT/System/CRC.h"
#include <vector>

namespace WT
{
namespace Graphics
{
namespace Animation
{

typedef System::SmartPointer<class CSkeleton> HSkeleton;
class CSkeleton : public System::CObject, public System::Serialization::ISerializable
{
public:
	typedef std::vector<CBone> Bones;

	WT_BEGIN_CLASS( CSkeleton, System::CObject );
		WT_INHERITS_INTERFACE(System::Serialization::ISerializable);
	WT_END_CLASS();

	CSkeleton()
	{
	}

	virtual ~CSkeleton()
	{
	}

	//
	// Accessors
	//

	Bones& GetBones()
	{
		return m_Bones;
	}

	Bones const& GetBones() const
	{
		return m_Bones;
	}

	int32 GetBoneCount() const
	{
		return static_cast<int32>( m_Bones.size() );
	}

	void ClearBones()
	{
		m_Bones.clear();
	}

	void AddBone(const CBone& NewBone)
	{
		m_Bones.push_back(NewBone);
	}

	CBone& GetBone( int32 Index )
	{
		if (Index < 0 || Index >= static_cast<int32>( m_Bones.size() ))
		{
			throw std::out_of_range( "Index" );
		}

		return m_Bones[Index];
	}

	CBone const& GetBone( int32 Index ) const
	{
		if (Index < 0 || Index >= static_cast<int32>( m_Bones.size() ))
		{
			throw std::out_of_range( "Index" );
		}

		return m_Bones[Index];
	}

	int32 GetIndexOfBone(const CBone * pBone) const
	{
		Bones::const_iterator it = m_Bones.begin();
		int32 Index = 0;
		while (it != m_Bones.end())
		{
			if (&(*it) == pBone)
			{
				return Index;
			}

			++Index;
			++it;
		}

		return -1;
	}

	//
	// Methods
	//

	virtual bool DoesBoneExist(const string& BoneName)
	{
		Bones::const_iterator it = m_Bones.begin();
		while (it != m_Bones.end())
		{
			if ((*it).GetName() == BoneName)
			{
				return true;
			}

			++it;
		}

		return false;
	}

	virtual CBone * GetBoneByName(const string& BoneName)
	{
		Bones::iterator it = m_Bones.begin();
		while (it != m_Bones.end())
		{
			if ((*it).GetName() == BoneName)
			{
				return ( &(*it) );
			}

			++it;
		}

		return NULL;
	}

	virtual const CBone * GetBoneByName(const string& BoneName) const
	{
		Bones::const_iterator it = m_Bones.begin();
		while (it != m_Bones.end())
		{
			if ((*it).GetName() == BoneName)
			{
				return ( &(*it) );
			}

			++it;
		}

		return NULL;
	}

	//TODO: Implement property support
	/*
	WT_CLASS_PROPERTY_START2( cmSkeleton )
		WT_FUNC_HANDLE_RW_PROPERTY( Bones, m_Bones )
	WT_CLASS_PROPERTY_END2
	*/

	//
	// ISerializable interface
	//
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private: // data
	Bones			m_Bones;
};


} // Animation
} // Graphics
} // WT
