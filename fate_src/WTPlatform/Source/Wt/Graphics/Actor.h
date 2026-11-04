/***************************************************************************************
 *
 * Actor.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          04/11/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#undef min
#undef max
#include <limits>
#include "WT/System/SmartPointer.h"
#include "WT/Graphics/Model.h"
#include "WT/Graphics/ActiveMotion.h"
#include "WT/Graphics/Animation/Skeleton.h"
#include "WT/Graphics/Animation/Pose.h"
#include "WT/Graphics/Animation/Motion.h"
#include "WT/Graphics/IUpdateable.h"

namespace WT
{
namespace Graphics
{

class CActor;

struct MotionInfo
{
	Animation::HMotion	hMotion;
	std::vector<int32>	BoneToPathMap;
	std::vector<int32>	MorphTargetToPathMap;

	MotionInfo() 
	{
	}

	MotionInfo( Animation::CMotion* pMotion )
		: hMotion( pMotion )
	{
	}
};

typedef std::vector<MotionInfo>	MotionCollection;

struct MorphTargetVertex
{
	int32 m_Index; //Vertex index
	CMesh::GeometryData m_Perturbation;
};

struct MorphTarget
{
	std::string m_Name;
	std::vector<MorphTargetVertex> m_Vertices;
};

//A class encapsulating the storage of morph target data and morph state.
//Note: when using morph targets, resultant normal vectors are usually
//unnormalized, so using D3DRS_NORMALIZENORMALS is recommended.
class CMorphInfo
{
public:
	void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

	///
	/// Types
	///

	typedef std::vector<MorphTarget> MorphTargets;
	typedef System::SmartPointer<MorphTargets> HMorphTargets;

	//A morph target contribution. The amount of contribution typically ranges from 0.0 through 1.0.
	struct MorphContribution
	{
		int32 m_Index; //Morph target index
		float32 m_Factor; //Amount of contribution
	};

	//A morph state - a collection of morph target contributions.
	//Any sum of total contribution amounts is acceptable.
	//An empty MorphState disables morph calculations.
	//MorphState is considered to be run-time information
	//and is not persistent.
	typedef std::vector<MorphContribution> MorphState;

	///
	/// Methods
	///

	//Get the morph targets. If you change the morph targets,
	//then calling SetMorphDirty( Actor, true ) is recommended.
	HMorphTargets const& GetMorphTargets() const {return m_hMorphTargets;}

	//Set the morph targets. Calling SetMorphDirty( true ) is recommended.
	void SetMorphTargets( HMorphTargets const& hMorphTargets ) {m_hMorphTargets = hMorphTargets;}

	//Get the morph state. If you change the morph state,
	//then calling SetMorphDirty( true ) is recommended.
	MorphState& GetMorphState() {return m_MorphState;}

	//Get read-only access to the morph state.
	MorphState const& GetMorphState() const {return m_MorphState;}

	//Set whether the morph result is dirty. CActor::Update()
	//will compute updated morph results only if dirty is true.
	void SetMorphDirty( CActor& Actor, bool MorphDirty );
	bool GetMorphDirty() const {return m_bMorphDirty;}

	CMorphInfo()
		: m_bMorphDirty( false )
	{
	}

	void Update( CActor const& Actor );

	CMesh::GeometryData const* GetMorphGeoData( CActor const& Actor ) const;

protected:
	std::vector<CMesh::GeometryData>	m_MorphGeoData;
	HMorphTargets						m_hMorphTargets;
	MorphState							m_MorphState;
	bool								m_bMorphDirty;
};

/// An Actor is an animatable, morphable Model.
typedef System::SmartPointer<class CActor> HActor;
class CActor : public CModel, public IUpdateable
{
public:
	WT_BEGIN_CLASS( CActor, CModel );
		WT_INHERITS_INTERFACE( IUpdateable );
	WT_END_CLASS();

	virtual void SetTransform( CMatrix4f const& Transform );

	CActor();
	CActor( LPDIRECT3DDEVICE9 pDevice );
	CActor( Animation::CSkeleton* pSkeleton, CMesh* pMesh );
	virtual ~CActor();

	//Perform mesh optimizations.
	virtual void Optimize();

	//Perform needed simulation-time updates. This supports animation and morphing.
	virtual void Update( float32 ElapsedTimeSeconds );

	/// Render the actor. Update() is automatically invoked.
	void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	/// Get the skeleton used by this actor.
	Animation::CSkeleton* GetSkeleton() const;

	/// Set the skeleton used by this actor.
	void SetSkeleton( Animation::CSkeleton* pSkeleton );

	/// Get the pose used by this actor.
	Animation::CPose* GetPose();

	//Mark the actor's pose as dirty, for example when bones have
	//moved. This is called automatically by CActiveMotionBase::Apply().
	//The pose takes into account the actor's transform when hardware vertex 
	//blending is used; so changing the actor's transform may also automatically
	//call this.
	void SetPoseDirty( bool bPoseDirty );

	/// Morph-related methods

	CMorphInfo const& GetMorphInfo() const {return m_MorphInfo;}

	CMorphInfo& GetMorphInfo() {return m_MorphInfo;}
    
	///
	/// Motion-related methods
	///

	/// Adds a motion to the collection and creates a skeleton bone-path map
	int32 AddMotion( Animation::CMotion* pMotion );

	/// Finds a motion and returns its index, or -1 if it is not mapped yet
	int32 FindMotion( Animation::CMotion const* pMotion );

	/// Returns a motion from the collection based on the given index value
	MotionInfo const& GetMotion( int32 Index ) const;

	/// remove all assigned motions from this actor (does not actually delete them)
	void ClearMotions();

	MotionCollection const& GetMotions() const {return m_Motions;}

	CActiveMotionQueue const& GetActiveMotionQueue() const {return m_ActiveMotionQueue;}
	CActiveMotionQueue& GetActiveMotionQueue() {return m_ActiveMotionQueue;}
	
	//Applies an identity transform to all bones
	void ResetMotion();

	//
	// Motion helper functions
	//

	/// Returns a valid index for the motion, adding it to the actor if necessary.
	int32 GetMotionIndex( Animation::CMotion* pMotion );

	/// returns total motion count assigned to this actor
	uint32 GetMotionCount() {return static_cast<uint32>( m_Motions.size() );}

	//Manually apply a motion using the specified time value.
	void ApplyMotion( int32 Index, float32 const Time = 0.0f, bool bLooping = false )
	{
		CActiveMotion activeMotion( Index, Time );
		activeMotion.SetLooping( bLooping );
		activeMotion.Apply( *this );
	}

	//Manually apply a motion using the specified time value.
	void ApplyMotion( Animation::CMotion* pMotion, float32 const Time = 0.0f, bool bLooping = false )
	{
		ApplyMotion( GetMotionIndex( pMotion ), Time, bLooping );
	}

	//Submit a new active motion to the queue.
	void PlayMotion( int32 Index, float32 const StartTime = 0.0f, float32 const ExpireTime = 
		std::numeric_limits<float32>::max(), float32 const Rate = 1.0f, bool bLooping = false )
	{
		CActiveMotion* pMotion = new CActiveMotion( Index, StartTime );
		pMotion->SetRate( Rate );
		pMotion->SetExpireTimeSeconds( ExpireTime );
		pMotion->SetLooping( bLooping );
		GetActiveMotionQueue().Push( pMotion );
	}

	//Submit a new active motion to the queue.
	void PlayMotion( Animation::CMotion* pMotion, float32 const StartTime = 0.0f, float32 const ExpireTime = 
		std::numeric_limits<float32>::max(), float32 const Rate = 1.0f, bool bLooping = false )
	{
		PlayMotion( GetMotionIndex( pMotion ), StartTime, ExpireTime, Rate, bLooping );
	}

	//Halts the current active motion and removes it from the queue.
	void PopMotion() {m_ActiveMotionQueue.Pop();}

	/// Stop playing motions
	void StopMotion() {m_ActiveMotionQueue.SetStopped( true );}

	/// Start playing motions
	void StartMotion() {m_ActiveMotionQueue.SetStopped( false );}

	/// Clear the queued motions
	void ClearMotionQueue() {m_ActiveMotionQueue.Clear();}

	bool IsMotionQueueEmpty() const {return m_ActiveMotionQueue.Empty();}

	/// Check if a motion is currently playing
	bool IsMotionPlaying() const
	{
		return 
			!m_ActiveMotionQueue.Empty() && 
			m_ActiveMotionQueue.GetFront() != NULL &&
			!m_ActiveMotionQueue.GetFront()->IsStopped() &&
			!m_ActiveMotionQueue.GetFront()->IsExpired();
	}

	//Blend to the specified motion. Both the current motion and this new motion will be played
	//concurrently; interpolation will switch over to the new motion in the specified time.
	void BlendMotion( Animation::CMotion* pMotion, float32 const Rate, bool bLooping, 
		float32 InterpStartTime, float32 InterpEndTime )
	{
		BlendMotion( GetMotionIndex( pMotion ), Rate, bLooping, InterpStartTime, InterpEndTime );
	}

	//Blend to the specified motion. Both the current motion and this new motion will be played
	//concurrently; interpolation will switch over to the new motion in the specified time.
	void BlendMotion( int32 Index, float32 const Rate, bool bLooping, 
		float32 InterpStartTime, float32 InterpEndTime )
	{
		CActiveMotionQueue& Queue = GetActiveMotionQueue();
		if (Queue.Empty())
		{
			throw std::runtime_error( "no animation to blend from" );
		}

		//Deal with unusual cases such as InterpStartTime > InterpEndTime,
		//or InterpStartTime < InterpEndTime < 0.
		bool bReverse = false;
		float32 TrueStartTime, TrueEndTime;
		if (InterpStartTime > InterpEndTime)
		{
			//Assume that this animation is going to be run backwards (with rate < 0.0)
			bReverse = true;
			TrueStartTime = std::min( InterpStartTime, 0.0f );
			TrueEndTime = std::min( InterpEndTime, 0.0f );
		} else {
			TrueStartTime = std::max( InterpStartTime, 0.0f );
			TrueEndTime = std::max( InterpEndTime, 0.0f );
		}

		//Remove the original motion from the queue, but save
		//a copy of it for use later.
		HActiveMotionBase hOrigMotion = Queue.GetFront()->Clone();
		Queue.Pop();

		//There are three stages - the first stage with the initial motion only,
		//the second stage that blends between the motions, and the third stage
		//where only the final motion plays. Correspondingly, we insert up to
		//three active motions into the actor's queue. We have to insert in
		//reverse order since the last added motion is the first to play.

		//Insert the final motion
		HActiveMotion hFinalMotion = new CActiveMotion( Index, (TrueEndTime - InterpStartTime) * Rate );
		hFinalMotion->SetRate( Rate );
		hFinalMotion->SetLooping( bLooping );
		Queue.Push( hFinalMotion.get() );

		//Insert the stage that blends between the two motions
		if (bReverse ? TrueStartTime > InterpEndTime : TrueStartTime < InterpEndTime)
		{
			HActiveMotionBase hSourceMotion = hOrigMotion->Clone();
			
			hSourceMotion->SetTimeSeconds( hOrigMotion->GetTimeSeconds() + 
				(TrueStartTime + TrueStartTime - InterpStartTime) * hOrigMotion->GetRate() );

			//Insert a blended motion

			HActiveMotion hDestMotion( new CActiveMotion( Index ) );
			hDestMotion->SetRate( Rate );
			hDestMotion->SetLooping( bLooping );

			//Add the blended animation. Submit source and dest motions for ownership.
			HActiveMotionBlend hBlend = new CActiveMotionBlend( hSourceMotion.get(), hDestMotion.get() );

			hBlend->SetInterpStartTime( InterpStartTime - TrueStartTime );
			hBlend->SetInterpEndTime( InterpEndTime - TrueStartTime );
			hBlend->SetExpireTimeSeconds( InterpEndTime - TrueStartTime );

			Queue.Push( hBlend.get() );
		}		

		//Insert the original motion, if it is supposed to run for a time.
		if (bReverse ? InterpStartTime < 0.0f : InterpStartTime > 0.0f)
		{
			//Let the original motion run, but only for InterpStartTime

			hOrigMotion->SetExpireTimeSeconds( 
				hOrigMotion->GetTimeSeconds() + InterpStartTime * hOrigMotion->GetRate() );

			Queue.Push( hOrigMotion );
		}
	}

	//
	//ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);
	virtual void GetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* SerialInfo, System::Serialization::CSerializationContext* sContext);

private: // methods
	void DoSoftwareBlending();

private: // data

	Animation::HSkeleton					m_hSkeleton;
	Animation::CPose						m_Pose;
	MotionCollection						m_Motions;
	std::vector<CMesh::GeometryData>		m_BlendGeoData;

	CMorphInfo								m_MorphInfo;

	CActiveMotionQueue						m_ActiveMotionQueue;

	bool									m_bPoseDirty  : 1;
	bool									m_bBlendDirty : 1;
};

}

template<> inline void WTCall WT::Using<Graphics::CActor>()
{
	Graphics::CActor::GetClassType();
	WT::Using<Graphics::CModel>();
	WT::Using<Graphics::Animation::CSkeleton>();
	WT::Using<Graphics::Animation::CMotion>();
}

}

DECLARE_VALUETYPE_TYPETRAIT(WT::Graphics::MorphTargetVertex);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Graphics::MorphTargetVertex>);

template <>
inline void WT::System::Serialization::CSerializationInfo::AddValue(const wstring& name,
	const WT::Graphics::MorphTargetVertex& value)
{
	AddContainerBegin(name);
	AddValue(L"Index", value.m_Index);
	AddValue(L"Perturbation", value.m_Perturbation);
	AddContainerEnd();
}

template <>
inline void WT::System::Serialization::CSerializationInfo::GetValue(const wstring& name,
	WT::Graphics::MorphTargetVertex& value )
{
	GetContainerBegin(name);
	GetValue(L"Index", value.m_Index);
	GetValue(L"Perturbation", value.m_Perturbation);
	GetContainerEnd();
}

DECLARE_VALUETYPE_TYPETRAIT(WT::Graphics::MorphTarget);
DECLARE_VALUETYPE_TYPETRAIT(std::vector<WT::Graphics::MorphTarget>);

template <>
inline void WT::System::Serialization::CSerializationInfo::AddValue(const wstring& name,
	const WT::Graphics::MorphTarget& value)
{
	AddContainerBegin(name);
	AddValue(L"Name", value.m_Name);
	AddValue(L"Vertices", value.m_Vertices);
	AddContainerEnd();
}

template <>
inline void WT::System::Serialization::CSerializationInfo::GetValue(const wstring& name,
	WT::Graphics::MorphTarget& value )
{
	GetContainerBegin(name);
	GetValue(L"Name", value.m_Name);
	GetValue(L"Vertices", value.m_Vertices);

	for (int32 i = 0; i < static_cast<int32>( value.m_Vertices.size() ); ++i)
	{
		if (value.m_Vertices[i].m_Index < 0)
		{
			throw std::runtime_error( "morph target uses negative vertex index" );
		}
	}

	GetContainerEnd();
}
