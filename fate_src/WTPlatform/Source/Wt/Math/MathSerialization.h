//
//	MathSerialization.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once


#include "WT/Math/Math.h"
#include "WT/System/Serialization/SerializationInfo.h"

namespace WT
{

DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CMatrix4f);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CQuaternion);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CVector4f);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CVector3i);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CVector3f);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CVector2i);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CVector2f);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CPlane);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CRect3i);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CRect2i);
DECLARE_VALUETYPE_TYPETRAIT(WT::Math::CRect2f);

DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CMatrix4f>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CQuaternion>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CVector4f>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CVector3i>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CVector3f>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CVector2i>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CVector2f>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CPlane>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CRect3i>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CRect2i>);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Math::CRect2f>);

namespace System
{
namespace Serialization
{

using namespace WT::Math;


// This implements by-value serialization for all non-WT::Object derived clases.

// CRect2f
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CRect2f& value)
{
	AddContainerBegin(name);
	AddValue(L"Left", value.left);
	AddValue(L"Top", value.top);
	AddValue(L"Right", value.right);
	AddValue(L"Bottom", value.bottom);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CRect2f& value )
{
	GetContainerBegin(name);
	GetValue( L"Left", value.left );
	GetValue( L"Top", value.top );
	GetValue( L"Right", value.right );
	GetValue( L"Bottom", value.bottom );
	GetContainerEnd();
}


// CRect2i
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CRect2i& value)
{
	AddContainerBegin(name);
	AddValue(L"Left", value.left);
	AddValue(L"Top", value.top);
	AddValue(L"Right", value.right);
	AddValue(L"Bottom", value.bottom);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CRect2i& value )
{
	GetContainerBegin(name);
	GetValue( L"Left", value.left );
	GetValue( L"Top", value.top );
	GetValue( L"Right", value.right );
	GetValue( L"Bottom", value.bottom );
	GetContainerEnd();
}

// CRect3i
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CRect3i& value)
{
	AddContainerBegin(name);
	AddValue(L"Left", value.left);
	AddValue(L"Top", value.top);
	AddValue(L"Front", value.front);
	AddValue(L"Right", value.right);
	AddValue(L"Bottom", value.bottom);
	AddValue(L"Back", value.back);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CRect3i& value )
{
	GetContainerBegin(name);
	GetValue( L"Left", value.left );
	GetValue( L"Top", value.top );
	GetValue( L"Front", value.front );
	GetValue( L"Right", value.right );
	GetValue( L"Bottom", value.bottom );
	GetValue( L"Back", value.back );
	GetContainerEnd();
}

// CVector2f
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CVector2f& value)
{
	AddContainerBegin(name);
	AddValue(L"x", value.x);
	AddValue(L"y", value.y);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CVector2f& value )
{
	GetContainerBegin(name);
	GetValue( L"x", value.x );
	GetValue( L"y", value.y );
	GetContainerEnd();
}


// Vector2i
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CVector2i& value)
{
	AddContainerBegin(name);
	AddValue(L"x", value.x);
	AddValue(L"y", value.y);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CVector2i& value )
{
	GetContainerBegin(name);
	GetValue( L"x", value.x );
	GetValue( L"y", value.y );
	GetContainerEnd();
}


// CVector3f
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CVector3f& value)
{
	AddContainerBegin(name);
	AddValue(L"x", value.x);
	AddValue(L"y", value.y);
	AddValue(L"z", value.z);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CVector3f& value )
{
	GetContainerBegin(name);
	GetValue( L"x", value.x );
	GetValue( L"y", value.y );
	GetValue( L"z", value.z );
	GetContainerEnd();
}


// Vector3i
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CVector3i& value)
{
	AddContainerBegin(name);
	AddValue(L"x", value.x);
	AddValue(L"y", value.y);
	AddValue(L"z", value.z);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CVector3i& value )
{
	GetContainerBegin(name);
	GetValue( L"x", value.x );
	GetValue( L"y", value.y );
	GetValue( L"z", value.z );
	GetContainerEnd();
}


// CVector4f
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CVector4f& value)
{
	AddContainerBegin(name);
	AddValue(L"x", value.x);
	AddValue(L"y", value.y);
	AddValue(L"z", value.z);
	AddValue(L"w", value.w);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CVector4f& value )
{
	GetContainerBegin(name);
	GetValue( L"x", value.x );
	GetValue( L"y", value.y );
	GetValue( L"z", value.z );
	GetValue( L"w", value.w );
	GetContainerEnd();
}

// CQuaternion
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CQuaternion& value)
{
	AddContainerBegin(name);
	AddValue(L"x", value.x);
	AddValue(L"y", value.y);
	AddValue(L"z", value.z);
	AddValue(L"w", value.w);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CQuaternion& value )
{
	GetContainerBegin(name);
	GetValue( L"x", value.x );
	GetValue( L"y", value.y );
	GetValue( L"z", value.z );
	GetValue( L"w", value.w );
	GetContainerEnd();
}

// CMatrix4f
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CMatrix4f& value)
{
	AddContainerBegin(name);
	AddValue(L"RowX", value.RowX);
	AddValue(L"RowY", value.RowY);
	AddValue(L"RowZ", value.RowZ);
	AddValue(L"RowT", value.RowT);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CMatrix4f& value )
{
	GetContainerBegin(name);
	GetValue( L"RowX", value.RowX );
	GetValue( L"RowY", value.RowY );
	GetValue( L"RowZ", value.RowZ );
	GetValue( L"RowT", value.RowT );
	GetContainerEnd();
}

// CPlane
template <>
inline void CSerializationInfo::AddValue(const wstring& name, const CPlane& value)
{
	AddContainerBegin(name);
	AddValue(L"Normal", value.Normal);
	AddValue(L"Distance", value.Distance);
	AddContainerEnd();
}

template <>
inline void CSerializationInfo::GetValue(const wstring& name, CPlane& value )
{
	GetContainerBegin(name);
	GetValue( L"Normal", value.Normal );
	GetValue( L"Distance", value.Distance );
	GetContainerEnd();
}

} // namespace Serialization
} // namespace System
} // namespace WT
