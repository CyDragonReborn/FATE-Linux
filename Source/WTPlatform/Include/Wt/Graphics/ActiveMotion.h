/***************************************************************************************
 *
 * ActiveMotionBase.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          09/9/2005
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
#include "WT/Graphics/Animation/Motion.h"
#include "WT/System/SmartPointer.h"
#include "WT/Graphics/IUpdateable.h"

namespace WT
{
namespace Graphics
{

//CActiveMotionBase is the base class for all active motions.
//Active motions advance at a rate specified by SetRate(),
//which gives the ratio between the flow of "internal time"
//for this object and the flow of "external time". The rate may 
//be negative, in which case the animation plays in reverse.
//There may be many time scales if the active motion actually consists
//of several active motions in a hierarchy.
typedef System::SmartPointer<class CActiveMotionBase> HActiveMotionBase;
class CActiveMotionBase : public System::CObject, public IUpdateable
{
public:
	WT_BEGIN_CLASS( CActiveMotionBase, System::CObject );
		WT_INHERITS_INTERFACE( IUpdateable );
	WT_END_CLASS();

	//Create an active motion with the specified internal time.
	CActiveMotionBase( float32 InternalTimeSeconds = 0.0f, bool bStopped = false )
		: m_Time( InternalTimeSeconds )
		, m_Rate( 1.0f )
		, m_bStopped( bStopped )
		, m_ExpireTime( 
			m_Rate >= 0.0f ? std::numeric_limits<float32>::max()
						  : -std::numeric_limits<float32>::max() )
	    , m_ResidualTime( 0.0f )
		, m_bExpired( false )
	{
	}
	
	CActiveMotionBase( CActiveMotionBase const& base )
		: m_Time( base.m_Time )
		, m_Rate( base.m_Rate )
		, m_bStopped( base.m_bStopped )
		, m_ExpireTime( base.m_ExpireTime )
		, m_ResidualTime( base.m_ResidualTime )
		, m_bExpired( base.m_bExpired )
	{
	}
    
	virtual ~CActiveMotionBase() {}

	virtual HActiveMotionBase Clone() const {return new CActiveMotionBase( *this );}

	void Apply( class CActor &actor );

	//Sample this active motion for the specified actor and bone index. Store the results in the
	//specified PathComponents. Always set PathComponents (unless an exception occurs).
	//Return true if applicable to this actor and bone; false otherwise.
	virtual bool Sample( class CActor& /*actor*/, int32 /*BoneIndex*/, Animation::PathComponents& Components ) const
	{
		Components.SetIdentity();
		return true;
	}

	//Sample this active motion for the specified actor and morph target index. Store the results in the
	//specified float32 Factor. Always set Factor (unless an exception occurs). Return true
	//if applicable to this actor and morph target; false otherwise.
	virtual bool Sample( class CActor& /*actor*/, int32 /*MorphTargetIndex*/, float32& Factor ) const
	{
		Factor = 0.0f;
		return true;
	}

	//Update the active motion based on an elapsed amount of
	//external time.
	virtual void Update( float32 ElapsedTimeSeconds )
	{
		if (IsStopped())
		{
			return;
		}

		float32 const ElapsedInternalTimeSeconds = ElapsedTimeSeconds * GetRate();

		if (IsExpired())
		{
			SetResidualTime( GetResidualTime() + ElapsedInternalTimeSeconds );
			return;
		}

		m_Time += ElapsedInternalTimeSeconds;

		if ((GetRate() >= 0.0f) ^ (m_Time < GetExpireTimeSeconds()))
		{
			SetExpired( true );
			SetResidualTime( fabsf( m_Time - GetExpireTimeSeconds() ) );
			SetTimeSeconds( GetExpireTimeSeconds() );
		}
	}

	//Get the current internal time in seconds.
	float32 GetTimeSeconds() const {return m_Time;}
	void SetTimeSeconds( float32 TimeSeconds ) {m_Time = TimeSeconds;}

	//Get the rate of flow of internal time per external time. The default value 
	//is 1.0.
	float32 GetRate() const {return m_Rate;}
	void SetRate( float32 Rate ) {m_Rate = Rate;}

	bool IsStopped() const {return m_bStopped;}
	void SetStopped( bool bStopped ) {m_bStopped = bStopped;}

	//ExpireTime is the internal time at which the motion will expire.
	//The default is essentially infinity for Rate >= 0 and 
	//negative infinity for Rate < 0.
	float32 GetExpireTimeSeconds() const {return m_ExpireTime;}
	void SetExpireTimeSeconds( float32 ExpireTime ) {m_ExpireTime = ExpireTime;}

	bool IsExpired() const {return m_bExpired;}
	void SetExpired( bool bExpired ) {m_bExpired = bExpired;}

	//ResidualTime is the internal time accumulated
	//since the motion expired.
	float32 GetResidualTime() const {return m_ResidualTime;}
	void SetResidualTime( float32 ResidualTime ) {m_ResidualTime = ResidualTime;}

private:
	float32 m_Time; //Internal animation time
	float32 m_Rate; //Rate at which animation advances (default = 1.0)
	float32 m_ExpireTime; //Internal time of expiration in seconds
	float32 m_ResidualTime; //Internal time accumulated since expiration in seconds
	bool    m_bStopped : 1;
	bool	m_bExpired : 1;
};

//A simple active motion consisting of one animation.
typedef System::SmartPointer<class CActiveMotion> HActiveMotion;
class CActiveMotion : public CActiveMotionBase
{
public:
	WT_CLASS( CActiveMotion, CActiveMotionBase );

	CActiveMotion()
		: CActiveMotionBase( 0.0f, false )
		, m_Index( -1 )
		, m_bLooping( false )
	{
	}

	CActiveMotion( int32 Index, float32 InternalTimeSeconds = 0.0f, bool bStopped = false )
		: CActiveMotionBase( InternalTimeSeconds, bStopped )
		, m_Index( Index )
		, m_bLooping( false )
	{
	}

	CActiveMotion( CActiveMotion const& activeMotion )
		: CActiveMotionBase( activeMotion )
		, m_Index( activeMotion.m_Index )
		, m_bLooping( activeMotion.m_bLooping )
	{
	}

	virtual ~CActiveMotion() {}

	virtual HActiveMotionBase Clone() const {return new CActiveMotion( *this );}

	virtual bool Sample( CActor& actor, int32 BoneIndex, Animation::PathComponents& Components ) const;
	virtual bool Sample( class CActor& /*actor*/, int32 /*MorphTargetIndex*/, float32& Factor ) const;

	int32 GetIndex() const {return m_Index;}
	void SetIndex( int32 Index ) {m_Index = Index;}

	bool IsLooping() const {return m_bLooping;}
	void SetLooping( bool bLooping ) {m_bLooping = bLooping;}

private:
	int32	m_Index; //Index of motion - picks out a particular Motion* from an Actor
	bool	m_bLooping : 1;
};

//A composite of zero or more active sub-motions. The composite is intended for use
//when simultaneously playing animations control completely different parts of a skeleton. 
//Latter sub-motions override earlier sub-motions. The sub-motions you provide will be 
//managed internally with shared ownership.
typedef System::SmartPointer<class CActiveMotionComposite> HActiveMotionComposite;
class CActiveMotionComposite : public CActiveMotionBase
{
public:
	WT_CLASS( CActiveMotionComposite, CActiveMotionBase );
	
	CActiveMotionComposite()
	{
	}

	CActiveMotionComposite( CActiveMotionComposite const& Composite ) :
		CActiveMotionBase( Composite )
	{
		m_Motions = Composite.m_Motions;	}

	virtual ~CActiveMotionComposite() {}

	virtual HActiveMotionBase Clone() const {return new CActiveMotionComposite( *this );}

	virtual bool Sample( CActor& actor, int32 BoneIndex, Animation::PathComponents& Components ) const;
	virtual bool Sample( class CActor& /*actor*/, int32 /*MorphTargetIndex*/, float32& Factor ) const;

	void Clear() {m_Motions.clear();}
	
	void Add( CActiveMotionBase* pActiveMotion )
	{
		m_Motions.push_back( pActiveMotion );
	}

	virtual void Update( float32 ElapsedTimeSeconds )
	{
		float32 ElapsedInternalTimeSeconds = -GetTimeSeconds();
		CActiveMotionBase::Update( ElapsedTimeSeconds );
		ElapsedInternalTimeSeconds += GetTimeSeconds();

		if (ElapsedInternalTimeSeconds)
		{
			for (int32 i = 0; i < static_cast<int32>( m_Motions.size() ); ++i)
			{
				if (m_Motions[i] != NULL)
				{
					m_Motions[i]->Update( ElapsedInternalTimeSeconds );
				}
			}
		}
	}

private:
	typedef std::vector<HActiveMotionBase> Motions;
	Motions m_Motions;
};

//A mix of zero or more active sub-motions. The sub-motions you provide
//will be managed internally with shared ownership.
typedef System::SmartPointer<class CActiveMotionMix> HActiveMotionMix;
class CActiveMotionMix : public CActiveMotionBase
{
public:
	WT_CLASS( CActiveMotionMix, CActiveMotionBase );

	CActiveMotionMix()
	{
	}

	CActiveMotionMix( CActiveMotionMix const& Mix ) :
		CActiveMotionBase( Mix )
	{
		m_Elements = Mix.m_Elements;
	}

	virtual ~CActiveMotionMix() {}

	virtual HActiveMotionBase Clone() const {return new CActiveMotionMix( *this );}

	virtual bool Sample( CActor& actor, int32 BoneIndex, Animation::PathComponents& Components ) const;
	virtual bool Sample( class CActor& /*actor*/, int32 /*MorphTargetIndex*/, float32& Factor ) const;

	void Clear() {m_Elements.clear();}
	
	void Add( CActiveMotionBase* pActiveMotion, float32 m_Weight = 1.0f )
	{
		m_Elements.push_back( Element( pActiveMotion, m_Weight ) );
	}

	virtual void Update( float32 ElapsedTimeSeconds )
	{
		float32 ElapsedInternalTimeSeconds = -GetTimeSeconds();
		CActiveMotionBase::Update( ElapsedTimeSeconds );
		ElapsedInternalTimeSeconds += GetTimeSeconds();

		if (ElapsedInternalTimeSeconds)
		{
			for (int32 i = 0; i < static_cast<int32>( m_Elements.size() ); ++i)
			{
				if (m_Elements[i].m_hActiveMotion != NULL)
				{
					m_Elements[i].m_hActiveMotion->Update( ElapsedInternalTimeSeconds );
				}
			}
		}
	}

private:
	struct Element
	{
		Element( CActiveMotionBase* pActiveMotion, float32 Weight = 1.0f )
			: m_hActiveMotion( pActiveMotion )
			, m_Weight( Weight )
		{
		}

		Element& operator=( Element const& e )
		{
			m_hActiveMotion = e.m_hActiveMotion;
			m_Weight = e.m_Weight;
			return *this;
		}

		HActiveMotionBase m_hActiveMotion;
		float32      m_Weight;
	};

	typedef std::vector<Element> Elements;

	Elements const& GetElements() const {return m_Elements;}
	Elements& GetElements() {return m_Elements;}

	Elements m_Elements;
};

//An active motion that consists of two active sub-motions blended together with an
//interpolation factor that starts at 0 at the specified start time and rises linearly
//to 1 at a specified end time. The sub-motions you provide will be managed internally 
//with shared ownership.
typedef System::SmartPointer<class CActiveMotionBlend> HActiveMotionBlend;
class CActiveMotionBlend : public CActiveMotionBase
{
public:
	WT_CLASS( CActiveMotionBlend, CActiveMotionBase );

	CActiveMotionBlend()
		: m_InterpStartTime( 0.0f )
		, m_InterpEndTime( 0.0f )
	{
	}

	CActiveMotionBlend( CActiveMotionBase* pStartActiveMotion, CActiveMotionBase* pEndActiveMotion )
		: m_hStartActiveMotion( pStartActiveMotion )
		, m_hEndActiveMotion( pEndActiveMotion )
		, m_InterpStartTime( 0.0f )
		, m_InterpEndTime( 0.0f )
	{
	}

	CActiveMotionBlend( CActiveMotionBlend const& Blend )
		: CActiveMotionBase( Blend )
		, m_hStartActiveMotion( Blend.m_hStartActiveMotion != NULL ? Blend.m_hStartActiveMotion->Clone() : NULL )
		, m_hEndActiveMotion( Blend.m_hEndActiveMotion != NULL ? Blend.m_hEndActiveMotion->Clone() : NULL )
		, m_InterpStartTime( Blend.m_InterpStartTime )
		, m_InterpEndTime( Blend.m_InterpEndTime )
	{
	}

	virtual ~CActiveMotionBlend() {}

	virtual HActiveMotionBase Clone() const {return new CActiveMotionBlend( *this );}

	virtual void Update( float32 ElapsedTimeSeconds )
	{
		float32 ElapsedInternalTimeSeconds = -GetTimeSeconds();
		CActiveMotionBase::Update( ElapsedTimeSeconds );
		ElapsedInternalTimeSeconds += GetTimeSeconds();

		if (ElapsedInternalTimeSeconds)
		{
			if (m_hStartActiveMotion != NULL)
			{
				m_hStartActiveMotion->Update( ElapsedInternalTimeSeconds );
			}

			if (m_hEndActiveMotion != NULL)
			{
				m_hEndActiveMotion->Update( ElapsedInternalTimeSeconds );
			}
		}
	}

	virtual bool Sample( CActor& actor, int32 BoneIndex, Animation::PathComponents& Components ) const;
	virtual bool Sample( class CActor& /*actor*/, int32 /*MorphTargetIndex*/, float32& Factor ) const;

	CActiveMotionBase* GetStartActiveMotion() const {return m_hStartActiveMotion;}
	void SetStartActiveMotion( CActiveMotionBase* pStartActiveMotion )
	{
		m_hStartActiveMotion = pStartActiveMotion;
	}

	CActiveMotionBase* GetEndActiveMotion() const {return m_hEndActiveMotion;}
	void SetEndActiveMotion( CActiveMotionBase* pEndActiveMotion )
	{
		m_hEndActiveMotion = pEndActiveMotion;
	}

	float32 GetInterpStartTime() const {return m_InterpStartTime;}
	void SetInterpStartTime( float32 StartTimeSeconds ) {m_InterpStartTime = StartTimeSeconds;}

	float32 GetInterpEndTime() const {return m_InterpEndTime;}
	void SetInterpEndTime( float32 EndTimeSeconds ) {m_InterpEndTime = EndTimeSeconds;}

	//Return parameter in range [0.0,1.0] reflecting animation progress
	float32 GetParameter() const;

private:
	HActiveMotionBase m_hStartActiveMotion;
	HActiveMotionBase m_hEndActiveMotion;
	float32 m_InterpStartTime;
	float32 m_InterpEndTime;
};

//A queue of active sub-motions. The front of the queue is the current
//active motion. The sub-motions you provide will be managed internally 
//with shared ownership.
typedef System::SmartPointer<class CActiveMotionQueue> HActiveMotionQueue;
class CActiveMotionQueue : public CActiveMotionBase
{
public:
	WT_CLASS( CActiveMotionQueue, CActiveMotionBase );

	CActiveMotionQueue()
	{
	}

	CActiveMotionQueue( CActiveMotionQueue const& q )
		: CActiveMotionBase( q )
	{
		for (Queue::const_iterator it = q.m_Queue.begin(); it != q.m_Queue.end(); ++it)
		{
			HActiveMotionBase hMotion;
			if (*it != NULL)
			{
				hMotion = (*it)->Clone();
			}
			m_Queue.push_back( hMotion );
		}
	}

	virtual ~CActiveMotionQueue()
	{
		Clear();
	}

	virtual HActiveMotionBase Clone() const {return new CActiveMotionQueue( *this );}

	virtual bool Sample( CActor& actor, int32 BoneIndex, Animation::PathComponents& Components ) const;
	virtual bool Sample( class CActor& /*actor*/, int32 /*MorphTargetIndex*/, float32& Factor ) const;

	void Push( CActiveMotionBase* pActiveMotionBase )
	{
		m_Queue.push_front( pActiveMotionBase );
	}

	void Pop()
	{
		if (m_Queue.empty())
		{
			throw std::runtime_error( "popping empty motion queue" );
		}

		m_Queue.pop_front();
	}

	bool Empty() const {return m_Queue.empty();}

	CActiveMotionBase* GetFront() const {return m_Queue.front();}

	void Clear()
	{
		while (!m_Queue.empty())
		{
			Pop();
		}
	}

	virtual void Update( float32 ElapsedTimeSeconds )
	{
		float32 ElapsedInternalTimeSeconds = -GetTimeSeconds();
		CActiveMotionBase::Update( ElapsedTimeSeconds );
		ElapsedInternalTimeSeconds += GetTimeSeconds();

		//Only the active animation updates
		if (ElapsedInternalTimeSeconds && !m_Queue.empty())
		{
			CActiveMotionBase* pMotion = m_Queue.front();

			pMotion->Update( ElapsedInternalTimeSeconds );

			ElapsedInternalTimeSeconds = 0.0f;

			if (pMotion->IsExpired() && pMotion->GetRate())
			{
				ElapsedInternalTimeSeconds = pMotion->GetResidualTime() / pMotion->GetRate();
				assert( ElapsedInternalTimeSeconds >= 0.0f );
				Pop();
			}
		}
	}

private:
	typedef std::deque<HActiveMotionBase> Queue;
	Queue m_Queue;
};

}
}
