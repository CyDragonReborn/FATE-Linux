/*
	ITypedInterpolator.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Graphics/Animation/IKeyInterpolator.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{


template<typename ValueType, typename KeyType>
class ITypedInterpolator : public IKeyInterpolator<ValueType>
{
public:
	typedef ITypedInterpolator<ValueType, KeyType> ITypedInterpolatorType;
	WT_BEGIN_INTERFACE( ITypedInterpolatorType )
		WT_INHERITS_INTERFACE( IKeyInterpolator<ValueType> )
	WT_END_INTERFACE()

	//
	// IInterpolator interface
	//

	virtual void Update ( float32 Time ) = 0;

	//
	// IKeyInterpolatory interface
	//

	virtual bool GetTimeExtents ( float32& StartTime , float32& EndTime ) const = 0;
	virtual bool IsSorted () const = 0;
	virtual void Sort() = 0;

	virtual uint32 GetKeyCount() const = 0;
	virtual void SetKeyCount( uint32 Count ) = 0;
	virtual float32 GetKeyTime ( uint32 Index ) const = 0;
	virtual void GetValueByIndex ( uint32 Index , ValueType* Value ) const = 0;
	virtual void ClearKeys() = 0;
	virtual bool IsEmpty() const = 0;
	virtual void RemoveKeyByIndex ( uint32 Index ) = 0;
	virtual void RemoveKeyByTime ( float32 Time ) = 0;
    virtual bool InsertKey ( float32 Time, const ValueType& Value ) = 0;
    virtual void AddKey ( float32 Time, const ValueType& Value ) = 0;

	virtual void SetRecomputeAsAClosedLoop ( bool ClosedLoop ) = 0;
	virtual bool GetRecomputeAsAClosedLoop () const = 0;

	virtual void Recompute() = 0;

    virtual void Sample ( float32 Time , bool Looping , ValueType& Result ) const = 0;

	//
	// ITypedInterpolator interface
	//

	virtual bool InsertKey ( const KeyType& Key ) = 0;
    virtual void AddKey ( const KeyType& Key ) = 0;
};


} // namespace Animation
} // namespace Graphics
} // namespace WT
