/*
	Pose.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	4/5/2005
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/System/SSE.h"
#include "WT/Graphics/Animation/Skeleton.h"
#include "WT/Graphics/Animation/Path.h"
#include "WT/Math/Math.h"
#include <vector>

namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

typedef System::SmartPointer<class CPose> HPose;
class CPose : public System::CObject
{
public:
	WT_CLASS(CPose, System::CObject)

	CPose() : m_SkinningPaletteIsDirty(true), m_PathComponentsAreDirty(true), m_pSkeleton(NULL)
	{
	}
	CPose(const CSkeleton * pSkeleton) : m_SkinningPaletteIsDirty(true), m_PathComponentsAreDirty(true)
	{
		SetSkeleton(pSkeleton);
	}
	virtual ~CPose()
	{
	}

	void Clear()
	{
		if (m_pSkeleton == NULL)
		{
			return;
		}

		int32 const BoneCount = m_pSkeleton->GetBoneCount();
		for (int32 i = 0; i < BoneCount; ++i)
		{
			m_BoneTransforms[i].SetIdentity();
		}
	}

	CMatrix4f* GetSkinningPalette()
	{
		if (m_SkinningPaletteIsDirty && m_pSkeleton != NULL)
		{
			int32 BoneCount = m_pSkeleton->GetBoneCount();
			
			for (int32 i = 0; i < BoneCount; ++i)
			{
				m_SkinningPalette[i].Multiply(
					m_pSkeleton->GetBone( i ).GetSkinToBoneTransform(), m_BoneTransforms[i] );
			}

			m_SkinningPaletteIsDirty = false;
		}

		return &m_SkinningPalette[0];
	}

	CMatrix4f* GetBoneTransforms()
	{
		return &m_BoneTransforms[0];
	}

    CSkeleton const* GetSkeleton() const
	{
		return m_pSkeleton;
	}

	void SetSkeleton(const CSkeleton * pSkeleton)
	{
		m_pSkeleton = pSkeleton;

		if (m_pSkeleton == NULL)
		{
			return;
		}

		int32 BoneCount = m_pSkeleton->GetBoneCount();
		m_SkinningPalette.resize(BoneCount);
		m_BoneTransforms.resize(BoneCount);
		m_PathComponents.resize(BoneCount);
		for (int32 i = 0; i < BoneCount; ++i)
		{
			m_SkinningPalette[i].SetIdentity();
		}

		m_SkinningPaletteIsDirty = false;
	}

    PathComponents * GetPathComponents()
	{
		return &m_PathComponents[0];
	}

    void SetSkinningPaletteIsDirty(bool Dirty)
	{
		m_SkinningPaletteIsDirty = Dirty;
	}
    bool GetSkinningPaletteIsDirty() const
	{
		return m_SkinningPaletteIsDirty;
	}

    void SetPathComponentsAreDirty(bool Dirty)
	{
		m_PathComponentsAreDirty = Dirty;
	}
	bool GetPathComponentsAreDirty() const
	{
		return m_PathComponentsAreDirty;
	}

	CMatrix4f * UpdateBoneTransforms(const CMatrix4f& RootTransform)
	{
		if (m_PathComponentsAreDirty && m_pSkeleton)
		{
			int32 BoneCount = m_pSkeleton->GetBoneCount();

			CMatrix4f* pTransform = &m_BoneTransforms[0];
			for (int32 i = 0; i < BoneCount; ++i, pTransform++)
			{
				CBone const& Bone = m_pSkeleton->GetBone(i);

				m_PathComponents[i].ToMatrix4f(pTransform);

				int32 BoneParent = Bone.GetParent();
				if (BoneParent >= 0)
				{
					pTransform->Multiply(*pTransform, m_BoneTransforms[BoneParent]);
				}
				else
				{
					pTransform->Multiply(*pTransform, RootTransform);
				}
			}

			m_PathComponentsAreDirty = false;
			m_SkinningPaletteIsDirty = true;
		}

		return &m_BoneTransforms[0];
	}

	//TODO: Implement property support
	/*
	WT_CLASS_PROPERTY_START2( cmPose )
		WT_FUNC_RW_PROPERTY( bool, SkinningPaletteIsDirty )
		WT_FUNC_RW_PROPERTY( bool, PathComponentsAreDirty )
	WT_CLASS_PROPERTY_END2
	*/


private: // data
	std::vector<CMatrix4f, System::SSEAllocator<CMatrix4f> >
								m_SkinningPalette;		// skinning transforms
	std::vector<CMatrix4f>		m_BoneTransforms;		// bone positions
	std::vector<PathComponents>	m_PathComponents;		// components of the bone positions (relative to their parent)
    const CSkeleton*			m_pSkeleton;
	bool						m_SkinningPaletteIsDirty:1;
	bool						m_PathComponentsAreDirty:1;
};


} // Animation
} // Graphics
} // WT
