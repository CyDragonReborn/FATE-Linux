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

#include "WT/Graphics/Actor.h"
#include "xmmintrin.h"
#include <set>

using namespace WT::System;
using namespace WT::System::Serialization;
using namespace WT::Math;
using namespace WT::Graphics;
using namespace WT::Graphics::Animation;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CActor )

CActor::CActor() 
: m_bPoseDirty( true )
	, m_bBlendDirty( true )
{
}

CActor::CActor( Animation::CSkeleton* pSkeleton, CMesh* pMesh )
	: CModel( pMesh )
	, m_hSkeleton( pSkeleton )
	, m_bPoseDirty( true )
	, m_bBlendDirty( true )
{
	if (pSkeleton != NULL)
	{
		m_Pose.SetSkeleton( pSkeleton );
	}
}

CActor::~CActor()
{
}

void CActor::SetPoseDirty( bool bPoseDirty )
{
	if (m_bPoseDirty != bPoseDirty)
	{
		CMesh* const pMesh = GetMesh();
		if (pMesh != NULL)
		{
			pMesh->SetStatic( false );
		}
		m_bPoseDirty = bPoseDirty;
	}
}

CSkeleton* CActor::GetSkeleton() const
{
	return m_hSkeleton;
}

void CActor::SetSkeleton( CSkeleton* pSkeleton )
{
	m_hSkeleton = pSkeleton;
	m_Pose.SetSkeleton( m_hSkeleton );
}

CPose* CActor::GetPose()
{
	return &m_Pose;
}

/*virtual*/ void CActor::SetTransform( CMatrix4f const& Transform )
{
	CModel::SetTransform( Transform );

	//If hardware blending is used, then mark bones as dirty.
	CMesh* const pMesh = GetMesh();
	if (pMesh != NULL && pMesh->UsesHardwareBlending() && m_hSkeleton != NULL)
	{
		m_bPoseDirty = true;
	}
}

CMesh::GeometryData const* CMorphInfo::GetMorphGeoData( CActor const& Actor ) const
{
	if (m_MorphGeoData.empty())
	{
		return NULL;
	}

	assert( static_cast<int32>( m_MorphGeoData.size() ) == 
		Actor.GetMesh()->GetVertexCount() + 1 /*pad for unaligned SSE access*/);
	return &*m_MorphGeoData.begin();
}

void CMorphInfo::SetMorphDirty( CActor& Actor, bool MorphDirty )
{
	CMesh* const pMesh = Actor.GetMesh();
	if (pMesh != NULL)
	{
		pMesh->SetStatic( false );
	}

	m_bMorphDirty = MorphDirty;
}

void CMorphInfo::Update( CActor const& Actor )
{
	bool bUseMorphData = false;

	if (m_bMorphDirty)
	{
		CMesh const* const pMesh = Actor.GetMesh();
		if (!m_MorphState.empty() && pMesh != NULL && 
			m_hMorphTargets != NULL && !m_hMorphTargets->empty())
		{
			int32 const VertexCount = pMesh->GetVertexCount();

			CMesh::GeometryData* pGeoData = NULL;

			for (MorphState::iterator it = m_MorphState.begin(); it != m_MorphState.end(); ++it)
			{
				MorphContribution& Contrib = *it;
				if (Contrib.m_Index < 0 || Contrib.m_Index >= static_cast<int32>( m_hMorphTargets->size() ))
				{
					throw std::invalid_argument( "morph target index" );
				}

				float32 const Factor = Contrib.m_Factor;
				if (Factor == 0.0f)
				{
					continue;
				}

				if (!bUseMorphData)
				{
					m_MorphGeoData.resize( VertexCount + 1 /*pad for unaligned SSE access*/ );
					pGeoData = &m_MorphGeoData[0];
					memcpy( pGeoData, pMesh->GetGeoData(), VertexCount * sizeof( CMesh::GeometryData ) );
					bUseMorphData = true;
				}

				MorphTarget* pMorphTarget = &m_hMorphTargets->operator[]( Contrib.m_Index );
				int32 const MorphTargetVertexCount = static_cast<int32>( pMorphTarget->m_Vertices.size() );
				for (int32 i = 0; i < MorphTargetVertexCount; ++i)
				{
					MorphTargetVertex const& Vertex = pMorphTarget->m_Vertices[i];

					int32 const Index = Vertex.m_Index;
					assert( Index >= 0 && Index < VertexCount );
					pGeoData[Index].m_Position += Factor * Vertex.m_Perturbation.m_Position;
					pGeoData[Index].m_Normal   += Factor * Vertex.m_Perturbation.m_Normal;
				}
			}
		}

		m_bMorphDirty = false;
	}

	if (!bUseMorphData)
	{
		m_MorphGeoData.clear();
	}
}

void CActor::Update( float32 ElapsedTimeSeconds )
{
	CMesh* const pMesh = GetMesh();

	//Update model-space vertex positions and normals
	if (m_MorphInfo.GetMorphDirty())
	{
		m_bBlendDirty = true;
		m_MorphInfo.Update( *this );
	}

	//Update bone transforms
	m_ActiveMotionQueue.Update( ElapsedTimeSeconds );
	if (!m_ActiveMotionQueue.Empty())
	{
		m_ActiveMotionQueue.Apply( *this );
	}

	if (m_bPoseDirty)
	{
		m_Pose.SetPathComponentsAreDirty( true );

		if (pMesh != NULL && pMesh->UsesHardwareBlending() && m_hSkeleton != NULL)
		{
			//Use actor's transform explicitly
			CMatrix4f Transform( GetTransform() );
			m_Pose.UpdateBoneTransforms( Transform );
		} else {
			//Pass actor transform through world matrix 0
			//Animate with identity matrix
			m_Pose.UpdateBoneTransforms( CMatrix4f::gm_Identity );
		}

		m_bBlendDirty = true;
		m_bPoseDirty = false;
	}

	//Update world-space vertex positions and normals
	DoSoftwareBlending();
}

void CActor::Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	if (!Context.ShouldRender( *this ))
	{
		return;
	}

	CMesh* const pMesh = GetMesh();
	if (pMesh == NULL)
	{
		return;
	}

	//If the mesh has no parts corresponding to the render category we are
	//rendering, then quit.
	//Note that to be object-oriented we should really have a function
	//like Mesh::NeedsRender( renderCategory ), we are using detailed
	//knowledge about how meshes work here.
	if (pMesh->GetParts( Context.GetRenderCategory() ).empty())
	{
		return;
	}

	//Set world matrices needed for software / hardware blending.
	if (pMesh->UsesHardwareBlending() && m_hSkeleton != NULL)
	{
		//Hardware blending
		uint32 const BoneCount = m_hSkeleton->GetBoneCount();

		//D3DTS_WORLDMATRIX() only accepts bone indices 0 through 255
		//TODO check for this possibility beforehand and disable hardware vertex blending
		assert( BoneCount < 256 );

		CMatrix4f* SkinningPalette = m_Pose.GetSkinningPalette();
		for (uint32 i = 0; i < BoneCount; i++)
		{
			pDevice->SetTransform( D3DTS_WORLDMATRIX( i ), SkinningPalette[i] );
		}
	}
	else
	{
		//No blending or software blending - use actor's transform explicitly
		CMatrix4f Transform( GetTransform() );
		Context.SetWorldTransform( pDevice, GetTransform() );
	}

	pMesh->Render( pDevice, Context, GetMaterials().get() );
}

int32 CActor::AddMotion( CMotion* pMotion )
{
	if (pMotion == NULL)
	{
		return -1;
	}

	//Create a bone to (motion) path map for this new motion
	//TODO consider the optimization of not using a map - in some situations
	//it may be possible to rearrange the Motion's paths or the Skeleton's 
	//bones instead.
	MotionInfo MotionSpec( pMotion );

	if (m_hSkeleton != NULL)
	{
		std::vector<int32>& BoneToPathMap = MotionSpec.BoneToPathMap;

		uint32 const BoneCount = m_hSkeleton->GetBoneCount();
		BoneToPathMap.resize( BoneCount );

		uint32 const PathCount = pMotion->GetPathCount();
		for (uint32 BoneIndex = 0; BoneIndex < BoneCount; ++BoneIndex)
		{
			string const& BoneName = m_hSkeleton->GetBone( BoneIndex ).GetName();

			uint32 PathIndex;
			for (PathIndex = 0; PathIndex < PathCount; ++PathIndex)
			{
				if (BoneName == pMotion->GetPath( PathIndex )->GetName())
				{
					break;
				}
			}

			if (PathIndex < PathCount)
			{
				BoneToPathMap[BoneIndex] = PathIndex;
			}
			else
			{
				// Possible to have no mapping of Bone index to Path index, -1 indicates this
				BoneToPathMap[BoneIndex] = -1;
			}
		}
	}

	CMorphInfo::MorphTargets* pTargets = m_MorphInfo.GetMorphTargets().get();
	if (pTargets != NULL)
	{
		std::vector<int32>& MorphTargetToPathMap = MotionSpec.MorphTargetToPathMap;

		int32 const MorphTargetCount = static_cast<int32>( pTargets->size() );
		MorphTargetToPathMap.resize( MorphTargetCount );

		uint32 const PathCount = pMotion->GetMorphPathCount();
		for (int32 MorphTargetIndex = 0; MorphTargetIndex < MorphTargetCount; ++MorphTargetIndex)
		{
			std::string const& MorphTargetName = pTargets->operator[]( MorphTargetIndex ).m_Name;
			uint32 PathIndex;
			for (PathIndex = 0; PathIndex < PathCount; ++PathIndex)
			{
				if (MorphTargetName == pMotion->GetMorphPath( PathIndex )->GetName())
				{
					break;
				}
			}

			if (PathIndex < PathCount)
			{
				MorphTargetToPathMap[MorphTargetIndex] = PathIndex;
			}
			else
			{
				// Possible to have no mapping of MorphTarget index to Path index, -1 indicates this
				MorphTargetToPathMap[MorphTargetIndex] = -1;
			}
		}
	}

	int32 Result = static_cast<int32>( m_Motions.size() );
	m_Motions.push_back( MotionSpec );
	return Result;
}

int32 CActor::FindMotion( CMotion const* pMotion )
{
	if (pMotion == NULL)
	{
		return -1;
	}

	for (int32 i = 0; i < static_cast<int32>( m_Motions.size() ); ++i)
	{
		if (m_Motions[i].hMotion == pMotion)
		{
			return i;
		}
	}

	return -1;
}

int32 CActor::GetMotionIndex( CMotion* pMotion )
{
	int32 Result = FindMotion( pMotion );
	if (Result != -1)
	{
		return Result;
	}

	return AddMotion( pMotion );
}

MotionInfo const& CActor::GetMotion( int32 Index ) const
{
	if (Index < 0 || Index >= static_cast<int32>( m_Motions.size() ))
	{
		throw std::out_of_range( "Index" );
	}

	return m_Motions[Index];
}

void CActor::ClearMotions()
{
	m_Motions.clear();
}

void CActor::ResetMotion()
{
	if (m_hSkeleton == NULL)
	{
		return;
	}

	PathComponents* pComponents = m_Pose.GetPathComponents();

	uint32 BoneCount = static_cast<uint32>( m_hSkeleton->GetBones().size() );
	for (uint32 Bone = 0; Bone < BoneCount; ++Bone)
	{
		pComponents[Bone].SetIdentity();
	}

	m_bPoseDirty = true;
}

/*virtual*/ void CActor::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	Info->AddValue( L"skeleton", m_hSkeleton.get() );

	Info->AddBaseClass( L"model", *this );

	//Write morph target data
	
	//TODO store min and max position and normal x/y/z for each morph target and use bit stream
	//to store each value. Also, optimize by taking advantage of case where a morph target is
	//equivalent to current mesh geometry data. Also, detect similar morph targets and store as
	//differential morph targets with low bit precision. Consider enforcing similar error
	//tolerance for all morph targets so that small-valued (e.g. differential) morph
	//targets can use reduced bit precision.

	m_MorphInfo.GetObjectData( Info, sContext );

	Info->AddContainerBeginWithCount( "Motions", static_cast<uint32>( m_Motions.size() ) );
	for (int32 i = 0; i < static_cast<int32>( m_Motions.size() ); ++i)
	{
		Info->AddValue<CObject*>( "item", const_cast<CMotion*>( m_Motions[i].hMotion.get() ) );
	}
	Info->AddContainerEnd();
}

void CMorphInfo::GetObjectData( CSerializationInfo* Info, CSerializationContext* sContext ) const
{
	if (m_hMorphTargets == NULL)
	{
		Info->AddValue( "HaveMorphData", false );
	} else {
		Info->AddValue( "HaveMorphData", true );
		Info->AddValue( "MorphTargets", *m_hMorphTargets );
	}
}

void CMorphInfo::SetObjectData( CSerializationInfo* Info, CSerializationContext* sContext )
{
	m_bMorphDirty = true;

	bool b;
	Info->GetValue( "HaveMorphData", b );
	if (!b)
	{
		m_hMorphTargets = NULL;
		return;
	}

	if (Info->GetObjectFormatVersion() < 4)
	{
		throw std::runtime_error( "unsupported actor version" );
	}

	m_hMorphTargets = new MorphTargets();
	Info->GetValue( "MorphTargets", *m_hMorphTargets );
}

uint32 WTCall CActor::GetClassFormatVersion() {return 4;}

/*virtual*/ void CActor::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	if (Info->GetObjectFormatVersion() >= 3)
	{
		//Read the skeleton
		CSkeleton* pSkeleton;
		Info->GetValue( L"skeleton", pSkeleton );
		m_hSkeleton = HSkeleton( pSkeleton );

		Info->GetBaseClass( L"model", *this );
	} else {
		throw std::runtime_error( "Obsolete CActor format" );
	}

	//Read the morph target data
	m_MorphInfo.SetObjectData( Info, sContext );

	ClearMotions();
	int32 MotionCount = Info->GetContainerBeginWithCount( "Motions" );
	m_Motions.reserve( MotionCount );
    for (int32 i = 0; i < MotionCount; ++i)
	{
		CObject* pObject;
		Info->GetValue( "item", pObject );
		CMotion* pMotion = DynamicCast<CMotion>( pObject );

		if (pMotion == NULL)
		{
			throw std::runtime_error( "corrupt actor data" );
		}

		//Store the motions into the m_Motions vector. Later,
		//in FinalizeDeserialization(), we will add them properly.
		m_Motions.push_back( pMotion );
	}
	Info->GetContainerEnd();
}

/*virtual*/ void CActor::FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext)
{
	CMesh* const pMesh = GetMesh();

	if (pMesh != NULL && !SerialInfo->ObjectIsFinalized( pMesh ) )
	{
		throw std::runtime_error( "ObjectIsFinalized() failure - possibly due to circular references" );
	}

	_BaseClass::FinalizeDeserialization( SerialInfo, sContext );

	//Validate morph target vertex references
	int32 const VertexCount = pMesh != NULL ? pMesh->GetVertexCount() : 0;
	CMorphInfo::MorphTargets* pMorphTargets = m_MorphInfo.GetMorphTargets().get();
	if (pMorphTargets != NULL )
	{
		for (int32 i = 0; i < static_cast<int32>( pMorphTargets->size() ); ++i)
		{
			MorphTarget& MorphTarget = pMorphTargets->operator[]( i );
			for (int32 j = 0; j < static_cast<int32>( MorphTarget.m_Vertices.size() ); ++j)
			{
				assert( MorphTarget.m_Vertices[j].m_Index >= 0 );
				if (MorphTarget.m_Vertices[j].m_Index >= VertexCount)
				{
					throw std::runtime_error( "morph target refers to non-existent vertex" );
				}
			}
		}
	}

	SetSkeleton( m_hSkeleton );

	int32 MotionCount = static_cast<int32>( m_Motions.size() );
	std::vector<HMotion> Temp( MotionCount );

	for (int32 i = 0; i < MotionCount; ++i)
	{
		Temp[i] = m_Motions[i].hMotion;
	}

	m_Motions.clear();
	m_Motions.reserve( MotionCount );

	for (int32 i = 0; i < MotionCount; ++i)
	{
		AddMotion( Temp[i] );
	}

	//Apply an initial animation if there is one
	//TODO this should be removed; it's arbitrary to select the zeroth animation. This is only done 
	//here because the exporter typically generates an excess bone with the fixed transform 
	//information in an animation. So, static objects require this to be transformed correctly.
	if (!m_Motions.empty())
	{
		CActiveMotion( 0 ).Apply( *this );

		//If there is only one motion, with only one path (matrix), and the mesh
		//uses the default bones, then try to apply the zeroth bone to the model transform.
		//Skeleton and animation data can then be deleted.
		if (m_Motions.size() == 1 && m_Motions[0].hMotion->GetPathCount() == 1 &&
			(pMesh == NULL || !pMesh->GetMatrixCount()))
		{
			Update( 0.0f ); //Update pose skinning palette

			if (m_hSkeleton->GetBoneCount())
			{
				SetTransform( GetTransform() * m_Pose.GetSkinningPalette()[0] );
			}

			ClearMotions();
			SetSkeleton( NULL );
		}
	}
}

void CActor::DoSoftwareBlending()
{
	CMesh::GeometryData const* pMorphGeoData = GetMorphInfo().GetMorphGeoData( *this );

	CMesh* const pMesh = GetMesh();
	if (pMesh == NULL || !pMesh->UsesSoftwareBlending() || m_hSkeleton == NULL)
	{
		m_BlendGeoData.clear();
		pMesh->SetRenderGeoData( pMorphGeoData );
		m_bBlendDirty = false;
		return;
	}

	if (!m_bBlendDirty)
	{
		return;
	}

	//Set world matrices.
	CMatrix4f const* pSoftwareBlendingMatrices = m_Pose.GetSkinningPalette();

	m_BlendGeoData.resize( pMesh->GetVertexCount() + 1 /*pad for unaligned SSE access*/ );

	uint8 const* pSourceVertex = pMesh->GetVertexPtr( 0 );
	CMesh::GeometryData* pBlendResults = &m_BlendGeoData[0];

	if (pMorphGeoData == NULL)
	{
		pMorphGeoData = pMesh->GetGeoData();
	}

	//Get the number of bytes devoted to beta (bone weights) in the vertex structure.
	int32 const BetaBytes = pMesh->GetBetaBytes();

	//If blending, the final matrix is always blended in with a weight that
	//brings the total weight to 1.0. Other than this final matrix, there is
	//the following number of weighted matrices.
	int32 const MaxCount = std::min( pMesh->GetBetaCount(), pMesh->GetMatrixCount() - 1 );

	int32 VertexCount = pMesh->GetVertexCount();
	int32 SourceVertexSize = pMesh->m_VertexSize;

	if (MaxCount <= 0)
	{
		for (int32 i = 0; i < VertexCount; ++i, pSourceVertex += SourceVertexSize)
		{
			uint8 const* const pMatrices = reinterpret_cast<uint8 const*>( pSourceVertex ) + BetaBytes;

			//Add in last matrix (0 == m_MatrixCount - 1) using a weight of FinalWeight
			assert( pMatrices[0] < m_hSkeleton->GetBoneCount() );

			CMatrix4f const& SourceMatrix = pSoftwareBlendingMatrices[pMatrices[0]];

			//Transform the position and normal with the matrix.
			//Normal will be un-normalized; using D3DRS_NORMALIZENORMALS is suggested.
			SourceMatrix.TransformNoOverlap( pBlendResults[i].m_Position, pMorphGeoData[i].m_Position );
			SourceMatrix.RotateNoOverlap( pBlendResults[i].m_Normal, pMorphGeoData[i].m_Normal );
		}
	} else if (IsSSESupported()) {
		//Require SSE-alignment for pose matrices
		assert( IsSSEAligned( pSoftwareBlendingMatrices ) );

		for (int32 i = 0; i < VertexCount; ++i, pSourceVertex += SourceVertexSize)
		{
			float32 const* const pBeta = reinterpret_cast<float32 const*>( pSourceVertex );
			uint8 const* const pMatrices = reinterpret_cast<uint8 const*>( pBeta ) + BetaBytes;

			//Bring the the total weight to 1 by adding in this final weight.
			//Use a value slightly less than 1 so that we can tolerate numerical error
			//when trying to detect when it is reduced to zero.
			float32 FinalWeight = 0.999999f;

			float32 const Weight = pBeta[0];

			assert( pMatrices[0] < m_hSkeleton->GetBoneCount() );
			CMatrix4f const& SourceMatrix = pSoftwareBlendingMatrices[pMatrices[0]];

//			BlendMatrix = SourceMatrix * Weight;
			__m128 WeightVec = _mm_load_ps1( &Weight );
			__m128 BlendX = _mm_mul_ps( _mm_load_ps( SourceMatrix.RowX.e ), WeightVec );
			__m128 BlendY = _mm_mul_ps( _mm_load_ps( SourceMatrix.RowY.e ), WeightVec );
			__m128 BlendZ = _mm_mul_ps( _mm_load_ps( SourceMatrix.RowZ.e ), WeightVec );
			__m128 BlendT = _mm_mul_ps( _mm_load_ps( SourceMatrix.RowT.e ), WeightVec );

			//If total weight is 1 (or greater) then we are finished.
			if (FinalWeight <= Weight)
			{
				goto DoneSSE;
			}

			FinalWeight -= Weight;

			int32 j;
			for (j = 1; j < MaxCount; ++j)
			{
				float32 const Weight = pBeta[j];

				//if (Weight == 0.0f) continue; //Not effective optimization

				assert( pMatrices[j] < m_hSkeleton->GetBoneCount() );
				CMatrix4f const& SourceMatrix = pSoftwareBlendingMatrices[pMatrices[j]];

	//			BlendMatrix += SourceMatrix * Weight;
				WeightVec = _mm_load_ps1( &Weight );
				BlendX = _mm_add_ps( BlendX, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowX.e ), WeightVec ) );
				BlendY = _mm_add_ps( BlendY, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowY.e ), WeightVec ) );
				BlendZ = _mm_add_ps( BlendZ, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowZ.e ), WeightVec ) );
				BlendT = _mm_add_ps( BlendT, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowT.e ), WeightVec ) );

				//If total weight is 1 (or greater) then we are finished.
				if (FinalWeight <= Weight)
				{
					goto DoneSSE;
				}

				FinalWeight -= Weight;
			}

			//Add in last matrix (j == m_MatrixCount - 1) using a weight of FinalWeight
			{
		//		BlendMatrix += SourceMatrix * FinalWeight;
				assert( pMatrices[j] < m_hSkeleton->GetBoneCount() );
				CMatrix4f const& SourceMatrix = pSoftwareBlendingMatrices[pMatrices[j]];

				__m128 WeightVec = _mm_load_ps1( &FinalWeight );
				BlendX = _mm_add_ps( BlendX, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowX.e ), WeightVec ) );
				BlendY = _mm_add_ps( BlendY, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowY.e ), WeightVec ) );
				BlendZ = _mm_add_ps( BlendZ, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowZ.e ), WeightVec ) );
				BlendT = _mm_add_ps( BlendT, _mm_mul_ps( _mm_load_ps( SourceMatrix.RowT.e ), WeightVec ) );
			}

	DoneSSE:
			//Transform the position and normal with the blended matrix.
			//Normal will be un-normalized; using D3DRS_NORMALIZENORMALS is suggested.

			__m128 Prod0;
			__m128 Prod1;
			__m128 Prod2;
			//__m128 Prod3;

			//For position, assume w == 1
			Prod0 = _mm_loadu_ps( pMorphGeoData[i].m_Position.e );
			Prod1 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(1,1,1,1) );
			Prod2 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(2,2,2,2) );
			//Prod3 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(3,3,3,3) );
			Prod0 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(0,0,0,0) );
			Prod0 = _mm_mul_ps( BlendX, Prod0 );
			Prod1 = _mm_mul_ps( BlendY, Prod1 );
			Prod2 = _mm_mul_ps( BlendZ, Prod2 );
			//Prod3 = _mm_mul_ps( BlendT, Prod3 );
			Prod0 = _mm_add_ps( Prod0, Prod1 );
			Prod2 = _mm_add_ps( Prod2, BlendT /*Prod3*/ );
			Prod0 = _mm_add_ps( Prod0, Prod2 );
			_mm_storeu_ps( pBlendResults[i].m_Position.e, Prod0 );

			//For normal, assume w == 0
			Prod0 = _mm_loadu_ps( pMorphGeoData[i].m_Normal.e );
			Prod1 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(1,1,1,1) );
			Prod2 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(2,2,2,2) );
			//Prod3 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(3,3,3,3) );
			Prod0 = _mm_shuffle_ps( Prod0, Prod0, _MM_SHUFFLE(0,0,0,0) );
			Prod0 = _mm_mul_ps( BlendX, Prod0 );
			Prod1 = _mm_mul_ps( BlendY, Prod1 );
			Prod2 = _mm_mul_ps( BlendZ, Prod2 );
			//Prod3 = _mm_mul_ps( BlendT, Prod3 );
			Prod0 = _mm_add_ps( Prod0, Prod1 );
			//Prod2 = _mm_add_ps( Prod2, Prod3 );
			Prod0 = _mm_add_ps( Prod0, Prod2 );
			_mm_storeu_ps( pBlendResults[i].m_Normal.e, Prod0 );
		}

	} else {

		CMatrix4f BlendMatrix;

		for (int32 i = 0; i < VertexCount; ++i, pSourceVertex += SourceVertexSize)
		{
			float32 const* const pBeta = reinterpret_cast<float32 const*>( pSourceVertex );
			uint8 const* const pMatrices = reinterpret_cast<uint8 const*>( pBeta ) + BetaBytes;

			//Initialize the blend matrix to the zero matrix.
			BlendMatrix._11 = 0.0f;
			BlendMatrix._12 = 0.0f;
			BlendMatrix._13 = 0.0f;
			BlendMatrix._21 = 0.0f;
			BlendMatrix._22 = 0.0f;
			BlendMatrix._23 = 0.0f;
			BlendMatrix._31 = 0.0f;
			BlendMatrix._32 = 0.0f;
			BlendMatrix._33 = 0.0f;
			BlendMatrix._41 = 0.0f;
			BlendMatrix._42 = 0.0f;
			BlendMatrix._43 = 0.0f;

			//Bring the the total weight to 1 by adding in this final weight.
			//Use a value slightly less than 1 so that we can tolerate errors
			//when trying to detect when it is reduced to zero.
			float32 FinalWeight = 0.999999f;

			int32 j = 0;
			for (; j < MaxCount; ++j)
			{
				float32 const Weight = pBeta[j];

				assert( pMatrices[j] < m_hSkeleton->GetBoneCount() );
				CMatrix4f const& SourceMatrix = pSoftwareBlendingMatrices[pMatrices[j]];

	//			BlendMatrix += SourceMatrix * Weight;
				BlendMatrix._11 += SourceMatrix._11 * Weight;
				BlendMatrix._12 += SourceMatrix._12 * Weight;
				BlendMatrix._13 += SourceMatrix._13 * Weight;
				BlendMatrix._21 += SourceMatrix._21 * Weight;
				BlendMatrix._22 += SourceMatrix._22 * Weight;
				BlendMatrix._23 += SourceMatrix._23 * Weight;
				BlendMatrix._31 += SourceMatrix._31 * Weight;
				BlendMatrix._32 += SourceMatrix._32 * Weight;
				BlendMatrix._33 += SourceMatrix._33 * Weight;
				BlendMatrix._41 += SourceMatrix._41 * Weight;
				BlendMatrix._42 += SourceMatrix._42 * Weight;
				BlendMatrix._43 += SourceMatrix._43 * Weight;

				//If total weight is 1 (or greater) then we are finished.
				if (FinalWeight <= Weight)
				{
					goto Done;
				}

				FinalWeight -= Weight;
			}

			//Add in last matrix (j == m_MatrixCount - 1) using a weight of FinalWeight
			assert( pMatrices[j] < m_hSkeleton->GetBoneCount() );
			CMatrix4f const& SourceMatrix = pSoftwareBlendingMatrices[pMatrices[j]];

	//		BlendMatrix += SourceMatrix * FinalWeight;
			BlendMatrix._11 += SourceMatrix._11 * FinalWeight;
			BlendMatrix._12 += SourceMatrix._12 * FinalWeight;
			BlendMatrix._13 += SourceMatrix._13 * FinalWeight;
			BlendMatrix._21 += SourceMatrix._21 * FinalWeight;
			BlendMatrix._22 += SourceMatrix._22 * FinalWeight;
			BlendMatrix._23 += SourceMatrix._23 * FinalWeight;
			BlendMatrix._31 += SourceMatrix._31 * FinalWeight;
			BlendMatrix._32 += SourceMatrix._32 * FinalWeight;
			BlendMatrix._33 += SourceMatrix._33 * FinalWeight;
			BlendMatrix._41 += SourceMatrix._41 * FinalWeight;
			BlendMatrix._42 += SourceMatrix._42 * FinalWeight;
			BlendMatrix._43 += SourceMatrix._43 * FinalWeight;

	Done:
			//Transform the position and normal with the blended matrix.
			//Normal will be un-normalized; using D3DRS_NORMALIZENORMALS is suggested.
			BlendMatrix.TransformNoOverlap( pBlendResults[i].m_Position, pMorphGeoData[i].m_Position );
			BlendMatrix.RotateNoOverlap( pBlendResults[i].m_Normal, pMorphGeoData[i].m_Normal );
		}
	}

	pMesh->SetRenderGeoData( &m_BlendGeoData[0] );

	m_bBlendDirty = false;
}

struct PerVertexMorphTargetInfo
{
	int32 m_TargetIndex; //Morph target index
	CMesh::GeometryData m_Perturbation;
	
	bool operator==( PerVertexMorphTargetInfo const& Info ) const
	{
		return m_TargetIndex == Info.m_TargetIndex &&
			m_Perturbation == Info.m_Perturbation;
	}

	uint32 ComputeHash() const
	{
		uint32 Hash = m_TargetIndex;
		Hash ^= *reinterpret_cast<uint32 const*>( &m_Perturbation.m_Position.x );
		Hash ^= *reinterpret_cast<uint32 const*>( &m_Perturbation.m_Position.y );
		Hash ^= *reinterpret_cast<uint32 const*>( &m_Perturbation.m_Position.z );
		Hash ^= *reinterpret_cast<uint32 const*>( &m_Perturbation.m_Normal.x );
		Hash ^= *reinterpret_cast<uint32 const*>( &m_Perturbation.m_Normal.y );
		Hash ^= *reinterpret_cast<uint32 const*>( &m_Perturbation.m_Normal.z );
		return Hash;
	}

	static uint32 WTCall ComputeHash( void const* pAux, int32 i );
	static bool WTCall Equals( void const* pAux, int32 i, int32 j );
};

uint32 WTCall PerVertexMorphTargetInfo::ComputeHash( void const* pAux, int32 i )
{
	std::map<int32, std::vector<PerVertexMorphTargetInfo> > const* pInfos =
		static_cast< std::map<int32, std::vector<PerVertexMorphTargetInfo> > const* >( pAux );
	
	std::map<int32, std::vector<PerVertexMorphTargetInfo> >::const_iterator it = pInfos->find( i );
	if (it == pInfos->end())
	{
		return 0;
	}

	std::vector<PerVertexMorphTargetInfo> const& Infos = it->second;
	
	uint32 Hash = 0;
	for (int32 i = 0; i < static_cast<int32>( Infos.size() ); ++i)
	{
		Hash ^= Infos[i].ComputeHash();
	}
	return Hash;
}

bool WTCall PerVertexMorphTargetInfo::Equals( void const* pAux, int32 i, int32 j )
{
	std::map<int32, std::vector<PerVertexMorphTargetInfo> > const* pInfos =
		static_cast< std::map<int32, std::vector<PerVertexMorphTargetInfo> > const* >( pAux );

	std::map<int32, std::vector<PerVertexMorphTargetInfo> >::const_iterator it = pInfos->find( i );
	std::map<int32, std::vector<PerVertexMorphTargetInfo> >::const_iterator jt = pInfos->find( j );

	if ((it == pInfos->end()) != (jt == pInfos->end()))
	{
		return false;
	}

	if (it == pInfos->end())
	{
		return true;
	}

	return it->second == jt->second;
}

/*virtual*/ void CActor::Optimize()
{
	if (m_MorphInfo.GetMorphTargets() == NULL ||
		m_MorphInfo.GetMorphTargets()->empty())
	{
		return CModel::Optimize();
	}
	
	CMesh* const pMesh = GetMesh();
	if (pMesh == NULL)
	{
		return;
	}

	//Optimize without merging similar vertices.
	pMesh->Optimize( false );
	
	std::vector<int32> VertexRemap;
	std::vector<MorphTarget>& MorphTargets = *m_MorphInfo.GetMorphTargets();

	{
		//Collect auxiliary per-vertex data.
		std::map<int32, std::vector<PerVertexMorphTargetInfo> > Infos;

		for (int32 i = 0; i < static_cast<int32>( MorphTargets.size() ); ++i)
		{
			MorphTarget const& Target = MorphTargets[i];
			for (int32 j = 0; j < static_cast<int32>( Target.m_Vertices.size() ); ++j)
			{
				PerVertexMorphTargetInfo Info;
				Info.m_TargetIndex = i;
				Info.m_Perturbation = Target.m_Vertices[j].m_Perturbation;
				Infos[Target.m_Vertices[j].m_Index].push_back( Info );
			}
		}

		pMesh->MergeSimilarVertices( &Infos, PerVertexMorphTargetInfo::ComputeHash, 
			PerVertexMorphTargetInfo::Equals, VertexRemap );
	}

	//Replace vertex indices referred to morph targets. Also, delete duplicates.

	for (int32 i = 0; i < static_cast<int32>( MorphTargets.size() ); ++i)
	{
		MorphTarget& Target = MorphTargets[i];
		std::set<int32> UsedVertices;
	
		int32 Dest = 0;
		for (int32 j = 0; j < static_cast<int32>( Target.m_Vertices.size() ); ++j)
		{
			int32 NewIndex = VertexRemap[Target.m_Vertices[j].m_Index];
			if (!UsedVertices.insert( NewIndex ).second)
			{
				//Vertex not inserted; must already be used
				continue;
			}

			Target.m_Vertices[Dest].m_Index = NewIndex;
			Target.m_Vertices[Dest].m_Perturbation = Target.m_Vertices[j].m_Perturbation;
			++Dest;
		}

		Target.m_Vertices.resize( Dest );
	}
}

} //namespace Graphics

} //namespace WT
