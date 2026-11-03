/*
	KeyInterpolatorImpl.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/ITypedInterpolator.h"
#include "WT/Math/Interpolation/LinearInterpolator.h"
#include <stdlib.h>
#include <vector>


namespace WT
{
namespace Graphics
{
namespace Animation
{

const static float32 KeyCutTime = 0.001f;
const static float32 KeyTimeEqualTolerance = 0.0001f;

template <typename ValueType, typename KeyType>
class CKeyInterpolator
	: public System::CObject
	, public ITypedInterpolator<ValueType, KeyType>
	// TODO  Include property Interface derivation for serialization
{
public:
	typedef CKeyInterpolator<ValueType, KeyType> KeyInterpolatorClassType;
	typedef ITypedInterpolator<ValueType, KeyType> ITypedInterpolatorType;
	WT_BEGIN_CLASS( KeyInterpolatorClassType, System::CObject )
		WT_INHERITS_INTERFACE( ITypedInterpolatorType )
	WT_END_CLASS()

	//
	// Constructors / destructor
	//

	CKeyInterpolator()
		: m_Index1( -1 )
		, m_Index2( -1 )
		, m_RecomputeAsAClosedLoop( false )
	{
		ResetCachedTimes();
	}

	CKeyInterpolator(const CKeyInterpolator& Other)
	{
		ReplaceWith(Other);
	}

	virtual ~CKeyInterpolator()
	{
	}

	// Assignment operator
	CKeyInterpolator& operator=(const CKeyInterpolator& Other)
	{
		ReplaceWith(Other);
		return *this;
	}

	//
	// IInterpolator interfaces
	//

	virtual bool GetTimeExtents( float32& StartTime , float32& EndTime ) const
	{
		if ( IsEmpty() )
		{
			StartTime = 0;
			EndTime = 0;
			return false;
		}
		else
		{
			StartTime = m_Keys[0].Time;
			EndTime = m_Keys[GetKeyCount()-1].Time;
			return true;
		}
	}

	virtual uint32 GetKeyCount() const
	{
		return static_cast<uint32>(m_Keys.size());
	}

	virtual void SetKeyCount( uint32 Count )
	{
		m_Keys.resize( Count );
	}

	virtual void ClearKeys()
	{
		m_Keys.clear();
		ResetCachedTimes();
	}

	virtual bool IsEmpty() const
	{
		return m_Keys.empty() ? true : false;
	}

	virtual void RemoveKeyByIndex( uint32 Index )
	{
		m_Keys.erase ( m_Keys.begin() + Index );
		ResetCachedTimes();
	}

	virtual void RemoveKeyByTime( float32 Time )
	{
		uint32 Index = SearchForKey (Time ); 
		if ( Index >= 0 )
			RemoveKeyByIndex ( Index );
	}

	virtual void GetValueByIndex( uint32 Index , ValueType* Value ) const
	{
		*Value = m_Keys[Index].Value;
	}

	virtual bool IsSorted() const
	{
		int32 Size = static_cast<int32>(m_Keys.size());

		if ( Size > 1 )
		{
			float32 LastTime = m_Keys[0].Time;
			for ( int32 i=1 ; i<Size ; i++ )
			{
				float32 T = m_Keys[i].Time;
				if ( LastTime > T )  
					return false;
				LastTime = T;
			}
		}

		return true;
	}

	static int __cdecl CompareTimeValues( const void *Arg1, const void *Arg2 )
	{
		const KeyType* Time1 = (const KeyType*) Arg1;
		const KeyType* Time2 = (const KeyType*) Arg2;

		if ( Time1->Time < Time2->Time )
			return -1;
		else if ( Time1->Time > Time2->Time  )
			return 1;
		else
			return 0;
	}

	virtual void Sort()
	{
		KeyType* Start = &(m_Keys[0]);
		qsort( Start, GetKeyCount(), sizeof(KeyType), CompareTimeValues );
		ResetCachedTimes();
	}

	virtual void AddKey( const KeyType& Key )
	{
		m_Keys.push_back ( Key );
	}

	inline static bool TimesAreEqual( float32 T1 , float32 T2 )
	{
		return ( T1 >= T2 - KeyTimeEqualTolerance && T1 <= T2 + KeyTimeEqualTolerance ) ? true : false;
	}

    virtual bool InsertKey( float32 Time, const ValueType& Value )
	{
		KeyType NewKey;
		NewKey.Time = Time;
		NewKey.Value = Value;
		return InsertKey ( NewKey );
	}

    virtual void AddKey( float32 Time, const ValueType& Value )
	{
		KeyType NewKey;
		NewKey.Time = Time;
		NewKey.Value = Value;
		AddKey ( NewKey );
	}

	virtual bool InsertKey( const KeyType& Key )
	{
		int32 Index = SearchForKey (Key.Time ); 

		// if there's an exact match - fail
		if ( Index >= 0 )  // if index is negative, there can't be an exact match
		{
			if ( TimesAreEqual(m_Keys[Index].Time , Key.Time) )
				return false;
		}

		// insert AFTER index
		if ( Index == (static_cast<int32>(m_Keys.size())) - 1 )
			m_Keys.push_back ( Key );
		else
		{
			m_Keys.insert( m_Keys.begin() + (Index+1) , Key ); 
		}

		ResetCachedTimes();

		return true;
	}

	virtual float32 GetKeyTime( uint32 Index ) const
	{
		return m_Keys[Index].Time;
	}

	virtual KeyType GetKey( uint32 Index ) const
	{
		return m_Keys[Index];
	}

	int32 SearchForKey( float32 Time ) const
	{
		int32 Low , Hi , Mid;
		float32 Test;

		Low = 0;
		Hi = static_cast<int32>(m_Keys.size()) - 1;

		while ( Low <= Hi )
		{
			assert( Low >= 0 );
			assert( Hi >= 0 );
			Mid = ( Low + Hi ) >> 1;
			Test = m_Keys[Mid].Time; 
			if ( Time > Test )
			{
				Low = Mid + 1;
			}
			else if ( Time < Test )
			{
				Hi = Mid - 1;
			}
			else
			{
				return Mid;
			}
		}

		return Hi;
	}

	static inline float32 ConstrainTimeToRange(float32 Time, float32 StartTime, float32 EndTime)
	{
		if ( Time >= EndTime )
		{
			if ( EndTime - StartTime < KeyTimeEqualTolerance )
				return StartTime;
			else
				return EndTime - KeyTimeEqualTolerance;
		}
		else if ( Time < StartTime )
		{
			return StartTime;
		}

		return Time;
	}

	static inline float32 ConstrainTimeToLoopRange(float32 Time, float32 StartTime, float32 EndTime)
	{
		if ( Time >= EndTime )
		{
			if ( EndTime - StartTime < KeyTimeEqualTolerance )
				return StartTime;
			else
				return fmodf( Time - StartTime , EndTime - StartTime) + StartTime;
		}
		if ( Time < StartTime )
		{
			float32 Result = fmodf( Time - StartTime , EndTime - StartTime ) + EndTime;
			if ( Result >= EndTime )
				Result = EndTime - KeyTimeEqualTolerance;
			return Result;
		}

		return Time;
	}

	void Sample(float32 Time, bool Looping, ValueType& Result) const
	{
		int32 Index1 , Index2;    		// index of keyframe just before and after Time
		float32 Time1 , Time2;	// Times in those keyframes	
		float32 BlendFactor;			// 0..1 blending factor
		float32 AdjTime;		// Time (possibly adjusted for looping)

		int32 const Length = static_cast<int32>(m_Keys.size());

		// Trap 0 and 1 key cases

		if ( Length < 2 )
		{
			if ( Length == 0 )
			{
				Result = ValueType();
				return;
			}

			if ( Length == 1 )
			{
				Interpolate( 0.0f , m_Keys[0] , m_Keys[0] , Result );
				return;
			}
		}

		if ( Looping )
			AdjTime = ConstrainTimeToLoopRange(Time , m_Keys[0].Time , m_Keys[Length-1].Time);
		else
			AdjTime = Time;	

		if ( AdjTime < m_Time2 && m_Time1 <= AdjTime )
		{
			Index1 = m_Index1;
			Index2 = m_Index2;
			Time1  = m_Time1;
			Time2  = m_Time2;
		}
		else
		{
			Index1 = SearchForKey ( AdjTime );
			Index2 = Index1 + 1;

			if ( Index2 == 0 )	
				Index1 = 0;

			if ( Index2 == Length )
				Index2 = Length - 1;

			m_Index1 = Index1;
			m_Index2 = Index2;
			m_Time1 = Time1 = m_Keys[Index1].Time;
			m_Time2 = Time2 = m_Keys[Index2].Time;
		}

		if ( Index1 == Index2 )
			BlendFactor = 0.0f;
		else
		{
			if ( (Time2 - Time1) < KeyCutTime )
				BlendFactor = 0.0f;
			else	
				BlendFactor = ( AdjTime - Time1 ) / ( Time2 - Time1 );
		}

		Interpolate(BlendFactor , m_Keys[Index1] , m_Keys[Index2] , Result);
	}

	virtual void SetRecomputeAsAClosedLoop( bool ClosedLoop )
	{
		m_RecomputeAsAClosedLoop = ClosedLoop;
	}

	virtual bool GetRecomputeAsAClosedLoop () const
	{
		return m_RecomputeAsAClosedLoop;
	}

	virtual void Recompute()
	{
		InternalRecompute();
	}

	virtual void Update( float32 Time )
	{
		ValueType Result;
		Sample( Time, false, Result );

		// TODO : Add property support
		/*
		if ( m_hProperty )
			m_hProperty->set( Result );
		*/
	}

	// TODO : Implement when property system is available
	/*
	virtual void SetProperty( TypedProperty<ValueType>* Property )
	{
		m_hProperty = Property;
	}

	virtual TypedProperty<ValueType>* GetProperty ( )
	{
		return m_hProperty;
	}
	*/

private:	// methods

	inline void Interpolate(float32 BlendFactor, const KeyType& Key1, const KeyType& Key2, ValueType& Result) const
	{
		Math::LinearInterpolate( BlendFactor, Key1.Value, Key2.Value, Result );
	}

	inline void InternalRecompute()
	{
	}

    inline void ResetCachedTimes()
	{
		m_Time1 = 1.0f;
		m_Time2 = 0.0f;
	}

	inline void ReplaceWith(const CKeyInterpolator& Other)
	{
		m_Index1 = Other.m_Index1;
		m_Index2 = Other.m_Index2;
		m_Time1 = Other.m_Time1;
		m_Time2 = Other.m_Time2;
		m_RecomputeAsAClosedLoop = Other.m_RecomputeAsAClosedLoop;

		this->m_Keys.clear();
		std::vector<KeyType>::const_iterator it = Other.m_Keys.begin();
		while (it != Other.m_Keys.end())
		{
			this->m_Keys.push_back(*it);
			++it;
		}
	}

private:	// data

	/*
	TODO consider moving these member variables to a seperate class,
	e.g. such as CKeyInterpolatorSampler. Currently, these will cause bugs 
	with multi-threaded sampling which might be useful (e.g. update each 
	actor using shared animation in seperate thread). Also, with many
	actors using shared animation with different time offsets, these
	may actually slow down sampling rather than speed it up.
	*/
	mutable int32			m_Index1;
	mutable int32			m_Index2;
	mutable float32			m_Time1;
	mutable float32			m_Time2;
	std::vector<KeyType>	m_Keys;
	bool					m_RecomputeAsAClosedLoop;
	// cmOwnerHandle<cmTypedProperty<ValueType> > m_hProperty;	//TODO
};


} // namespace Animation
} // namespace Graphics
} // namespace WT
