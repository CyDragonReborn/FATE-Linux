/*
	Bone.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#pragma once

#include "WT/System/Platform.h"
#include "WT/System/Serialization/SerializationInfo.h"
#include "WT/Math/LinearAlgebra/Matrix4f.h"
#include "WT/Math/MathSerialization.h"

namespace WT
{

namespace Graphics
{
namespace Animation
{

class CBone
{
public:
	static int32 const KNoParent = -1;

	CBone() 
		: m_Parent( KNoParent )
	{
	}

	CBone(const string& Name)
		: m_Name( Name )
		, m_Parent( KNoParent )
	{
	}

	CBone(const string& Name, int32 Parent)
		: m_Name( Name )
		, m_Parent( Parent )
	{
	}

	CBone(const string& Name, int32 Parent, 
		 const Math::CMatrix4f& SkinToBoneX)
		: m_Name( Name )
		, m_Parent( Parent )
		, m_SkinToBoneTransform( SkinToBoneX )
	{
	}

	void SetName(const string& Name)
	{
		m_Name = Name;
	}
	string GetName() const
	{
		return m_Name;
	}

	void SetParent(int32 Parent)
	{
		m_Parent = Parent;
	}
	int32 GetParent() const
	{
		return m_Parent;
	}

	void SetSkinToBoneTransform (const Math::CMatrix4f& Transform)
	{
		m_SkinToBoneTransform = Transform;
	}

	Math::CMatrix4f& GetSkinToBoneTransform()
	{
		return m_SkinToBoneTransform;
	}
	
	Math::CMatrix4f const& GetSkinToBoneTransform() const
	{
		return m_SkinToBoneTransform;
	}

private:
	string				m_Name;
	int32				m_Parent;					// KNoParent if none. (parent index < child index)
	Math::CMatrix4f		m_SkinToBoneTransform;		// mesh space (skin) into this bone space  == 1/(Reference pose matrix)
};

} //namespace Animation
} //namespace Graphics

DECLARE_VALUETYPE_TYPETRAIT(WT::Graphics::Animation::CBone);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Graphics::Animation::CBone>);

namespace System
{
namespace Serialization
{

template <>
inline void CSerializationInfo::AddValue(const wstring& name, Graphics::Animation::CBone const& value)
{
	AddContainerBegin( name );
	AddValue( "BoneName", value.GetName() );
	AddValue( "BoneParent", value.GetParent() );
	AddValue( "SkinToBoneTransform", value.GetSkinToBoneTransform() );
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, Graphics::Animation::CBone& value )
{
	std::string BoneName;
	int32 BoneParent;
	Math::CMatrix4f M( false );

	GetContainerBegin( name );
	GetValue( "BoneName", BoneName );
	GetValue( "BoneParent", BoneParent );
	GetValue( "SkinToBoneTransform", M );
	GetContainerEnd();

	value.SetName( BoneName );
	value.SetParent( BoneParent );
	value.SetSkinToBoneTransform( M );
}

} //namespace Serialization
} //namespace System

} //namespace WT
