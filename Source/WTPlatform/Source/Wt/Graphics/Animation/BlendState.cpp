/*
	BlendState.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	4/14/2005
*/
#include "WT/stdafx.h"
#include "WT/Graphics/Animation/BlendState.h"

using namespace WT::System;


namespace WT
{
namespace Graphics
{
namespace Animation
{

WT_DEFINE_CLASS(CBlendState)

CBlendState::CBlendState()
{
}

CBlendState::CBlendState(CBlendState const& Other)
	: m_Indices(Other.m_Indices), m_Weights(Other.m_Weights)
{
}

CBlendState::~CBlendState()
{
}


//
//	CBlendState methods
//

uint32 CBlendState::GetIndexCount() const
{
	return static_cast<uint32>(m_Indices.size());
}
void CBlendState::SetIndexCount(uint32 Count)
{
	m_Indices.resize(Count, -1);
}

int32 CBlendState::GetIndex(uint32 Index) const
{
	if (Index >= m_Indices.size())
	{
		return -1;
	}

	return m_Indices[Index];
}
void CBlendState::SetIndex(uint32 Index, int32 Value)
{
	if (Index >= m_Indices.size())
	{
		return;
	}

	m_Indices[Index] = Value;
}

uint32 CBlendState::GetWeightCount() const
{
	return static_cast<uint32>(m_Weights.size());
}
void CBlendState::SetWeightCount(uint32 Count)
{
	m_Weights.resize(Count, 0.0f);
}

float32 CBlendState::GetWeight(uint32 Index) const
{
	if (Index >= m_Indices.size())
	{
		return 0.0f;
	}

	return m_Weights[Index];
}
void CBlendState::SetWeight(uint32 Index, float32 Value)
{
	if (Index >= m_Indices.size())
	{
		return;
	}

	m_Weights[Index] = Value;
}


} // namespace Animation
} // namespace Graphics
} // namespace WT
