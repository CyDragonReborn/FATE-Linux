/***************************************************************************************
 *
 * Actor.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 4/11/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"

#include "WT/Graphics/ActiveMotion.h"
#include "WT/Graphics/Actor.h"

using namespace WT::System;
using namespace WT::Math;
using namespace WT::Graphics;
using namespace WT::Graphics::Animation;

namespace WT
{
namespace Graphics
{

void CActiveMotionBase::Apply( CActor &actor )
{
	if (actor.GetSkeleton() != NULL)
	{
		uint32 BoneCount = static_cast<uint32>( actor.GetSkeleton()->GetBones().size() );
		for (uint32 Bone = 0; Bone < BoneCount; ++Bone)
		{
			Sample( actor, Bone, actor.GetPose()->GetPathComponents()[Bone] );
		}

		actor.SetPoseDirty( true );
	}

    std::vector<MorphTarget>* pTargets = actor.GetMorphInfo().GetMorphTargets().get();
	if (pTargets != NULL)
	{
		uint32 MorphTargetCount = static_cast<uint32>( pTargets->size() );

		actor.GetMorphInfo().GetMorphState().resize( MorphTargetCount );

		for (uint32 i = 0; i < MorphTargetCount; ++i)
		{
			CMorphInfo::MorphContribution& Contribution = actor.GetMorphInfo().GetMorphState()[i];
			Contribution.m_Index = i;
			Sample( actor, i, Contribution.m_Factor );
		}

		actor.GetMorphInfo().SetMorphDirty( actor, true );
	}
}

/*virtual*/ bool CActiveMotion::Sample( CActor& actor, int32 BoneIndex, PathComponents& Components ) const
{
	if (m_Index < 0 ||
		m_Index >= static_cast<int32>( actor.GetMotions().size() ))
	{
		throw std::invalid_argument( "motion index" );
	}

	if (BoneIndex < 0)
	{
		throw std::invalid_argument( "bone index" );
	}

	if (actor.GetSkeleton() != NULL)
	{
		if (BoneIndex >= static_cast<int32>( actor.GetSkeleton()->GetBoneCount() ))
		{
			throw std::invalid_argument( "bone index" );
		}

		MotionInfo const& MotionSpec = actor.GetMotions()[m_Index];
		assert( MotionSpec.hMotion != NULL );

		int32 PathIndex = MotionSpec.BoneToPathMap[BoneIndex];
		if (PathIndex > -1)
		{
			CPath const* pPath = MotionSpec.hMotion->GetPath( PathIndex );
			if (pPath != NULL)
			{
				pPath->Sample( GetTimeSeconds(), IsLooping(), Components );
				return true;
			}
		}
	}

	Components.SetIdentity();
	return false;
}

/*virtual*/ bool CActiveMotion::Sample( CActor& actor, int32 MorphTargetIndex, float32& Factor ) const
{
	if (m_Index < 0 ||
		m_Index >= static_cast<int32>( actor.GetMotions().size() ))
	{
		throw std::invalid_argument( "motion index" );
	}

	if (MorphTargetIndex < 0)
	{
		throw std::invalid_argument( "bone index" );
	}

	CMorphInfo::MorphTargets* pTargets = actor.GetMorphInfo().GetMorphTargets().get();
	if (pTargets != NULL)
	{
		if (MorphTargetIndex >= static_cast<int32>( pTargets->size() ))
		{
			throw std::invalid_argument( "morph target index" );
		}

		MotionInfo const& MotionSpec = actor.GetMotions()[m_Index];
		assert( MotionSpec.hMotion != NULL );

		int32 PathIndex = MotionSpec.MorphTargetToPathMap[MorphTargetIndex];
		if (PathIndex > -1)
		{
			CMotion::MorphPath const* pPath = MotionSpec.hMotion->GetMorphPath( PathIndex );
			if (pPath != NULL)
			{
				pPath->Sample( GetTimeSeconds(), IsLooping(), Factor );
				return true;
			}
		}
	}

	Factor = 0.0f;
	return false;
}

bool CActiveMotionComposite::Sample( CActor& actor, int32 BoneIndex, PathComponents& Components ) const
{
	for (int32 i = static_cast<int32>( m_Motions.size() ) - 1; i >= 0; --i)
	{
		if (m_Motions[i] != NULL && m_Motions[i]->Sample( actor, BoneIndex, Components ))
		{
			return true;
		}
	}

	Components.SetIdentity();
	return false;
}

bool CActiveMotionComposite::Sample( CActor& actor, int32 MorphTargetIndex, float32& Factor ) const
{
	for (int32 i = static_cast<int32>( m_Motions.size() ) - 1; i >= 0; --i)
	{
		if (m_Motions[i] != NULL && m_Motions[i]->Sample( actor, MorphTargetIndex, Factor ))
		{
			return true;
		}
	}

	Factor = 0.0f;
	return false;
}

bool CActiveMotionMix::Sample( CActor& actor, int32 BoneIndex, PathComponents& Components ) const
{
	Components.SetIdentity();

	PathComponents TempComponents;
	float32 TotalWeight = 0.0f;
	for (int32 i = 0; i < static_cast<int32>( m_Elements.size() ); ++i)
	{
		CActiveMotionBase const* pActiveMotion = m_Elements[i].m_hActiveMotion;

		if (pActiveMotion == NULL)
		{
			throw std::runtime_error( "mixing NULL animation" );
		}

		pActiveMotion->Sample( actor, BoneIndex, TempComponents );

		float32 Weight = m_Elements[i].m_Weight;

		//Use linear interpolation of scale (it may be more natural to do logarithmic
		//interpolation in some applications).
		if (TempComponents.ApplyScale)
		{
			Components.Scale.x += (TempComponents.Scale.x - 1.0f) * Weight;
			Components.Scale.y += (TempComponents.Scale.y - 1.0f) * Weight;
			Components.Scale.z += (TempComponents.Scale.z - 1.0f) * Weight;
		}

		Components.Translation += TempComponents.Translation * Weight;

		//TODO method of mixing multiple quaternions TBD
		if (TotalWeight == 0.0f)
		{
			Components.Rotation = TempComponents.Rotation;
		} else {
			Components.Rotation.Slerp(
				Components.Rotation,
				TempComponents.Rotation, Weight / (TotalWeight + Weight) );
		}

		TotalWeight += Weight;
	}

	Components.Rotation = (Components.Rotation.Ln() * TotalWeight ).Exp();

	if (Components.Scale.x != 1.0f ||
		Components.Scale.y != 1.0f ||
		Components.Scale.z != 1.0f)
	{
		Components.ApplyScale = true;
	}

	return true;
}

bool CActiveMotionMix::Sample( CActor& actor, int32 MorphTargetIndex, float32& Factor ) const
{
	Factor = 0.0f;

	for (int32 i = 0; i < static_cast<int32>( m_Elements.size() ); ++i)
	{
		CActiveMotionBase const* pActiveMotion = m_Elements[i].m_hActiveMotion;

		if (pActiveMotion == NULL)
		{
			throw std::runtime_error( "mixing NULL animation" );
		}

		float32 TempFactor;
		pActiveMotion->Sample( actor, MorphTargetIndex, TempFactor );

		Factor += TempFactor * m_Elements[i].m_Weight;
	}

	return true;
}

float32 CActiveMotionBlend::GetParameter() const
{
	float32 const StartTime = GetInterpStartTime();
	float32 const EndTime = GetInterpEndTime();
	float32 Parameter = GetTimeSeconds();
	if (Parameter >= EndTime)
	{
		Parameter = 1.0f;
	} else if (Parameter <= StartTime) {
		Parameter = 0.0f;
	} else {
		Parameter = (Parameter - StartTime) / (EndTime - StartTime);
	}

	return Parameter;
}

bool CActiveMotionBlend::Sample( CActor& actor, int32 BoneIndex, PathComponents& Components ) const
{
	if (m_hStartActiveMotion == NULL || m_hEndActiveMotion == NULL)
	{
		throw std::runtime_error( "blending NULL animation" );
	}

	float32 Parameter = GetParameter();

	m_hStartActiveMotion->Sample( actor, BoneIndex, Components );
	PathComponents EndComponents;
	m_hEndActiveMotion->Sample( actor, BoneIndex, EndComponents );

	//Use linear interpolation of scale (it may be more natural to do logarithmic
	//interpolation in some applications).
	
	if (!Components.ApplyScale)
	{
		Components.Scale = CVector3f::gm_UnitScale;
	}
	if (!EndComponents.ApplyScale)
	{
		EndComponents.Scale = CVector3f::gm_UnitScale;
	}

	Components.Scale += (EndComponents.Scale - Components.Scale) * Parameter;

	Components.Translation += (EndComponents.Translation - Components.Translation) * Parameter;
	Components.Rotation.Slerp( Components.Rotation, EndComponents.Rotation, Parameter );

	Components.ApplyScale = false;
	if (Components.Scale.x != 1.0f ||
		Components.Scale.y != 1.0f ||
		Components.Scale.z != 1.0f)
	{
		Components.ApplyScale = true;
	}

	return true;
}

bool CActiveMotionBlend::Sample( CActor& actor, int32 MorphTargetIndex, float32& Factor ) const
{
	if (m_hStartActiveMotion == NULL || m_hEndActiveMotion == NULL)
	{
		throw std::runtime_error( "blending NULL animation" );
	}

	float32 Parameter = GetParameter();

	m_hStartActiveMotion->Sample( actor, MorphTargetIndex, Factor );
	float32 EndFactor = 0.0f;
	m_hEndActiveMotion->Sample( actor, MorphTargetIndex, EndFactor );

	Factor += (EndFactor - Factor) * Parameter;

	return true;
}

bool CActiveMotionQueue::Sample( CActor& actor, int32 BoneIndex, PathComponents& Components ) const
{
	if (m_Queue.empty())
	{
		Components.SetIdentity();
		return false;
	}

	CActiveMotionBase* pMotion = m_Queue.front();
	if (pMotion == NULL)
	{
		throw std::runtime_error( "NULL animation in queue" );
	}

	return pMotion->Sample( actor, BoneIndex, Components );
}

bool CActiveMotionQueue::Sample( CActor& actor, int32 MorphTargetIndex, float32& Factor ) const
{
	if (m_Queue.empty())
	{
		Factor = 0.0f;
		return false;
	}

	CActiveMotionBase* pMotion = m_Queue.front();
	if (pMotion == NULL)
	{
		throw std::runtime_error( "NULL animation in queue" );
	}

	return pMotion->Sample( actor, MorphTargetIndex, Factor );
}

} //namespace Graphics
} //namespace WT
