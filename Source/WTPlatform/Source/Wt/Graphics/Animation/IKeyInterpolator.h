/*
	IKeyInterpolator.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Ronen Borshack
	10/28/2004
*/
#pragma once

#include "WT/System/Platform.h"
#include "WT/Math/Math.h"
#include "WT/Graphics/Animation/IInterpolator.h"


namespace WT
{
namespace Graphics
{
namespace Animation
{

using namespace Math;

template<typename ValueType> class IKeyInterpolator;
typedef IKeyInterpolator<float32>			IFloat32KeyInterpolator;
typedef IKeyInterpolator<CVector3f>			IVector3fKeyInterpolator;
typedef IKeyInterpolator<CQuaternion>		IQuatKeyInterpolator;


template<typename ValueType>
class IKeyInterpolator : public IInterpolator
{
public:
	WT_BEGIN_INTERFACE( IKeyInterpolator<ValueType> )
		WT_INHERITS_INTERFACE( IInterpolator )
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

	//TODO the IKeyInterpolator should just have an IsLooping() flag that should
	//be serialized. Currently we call SetRecomputeAsAClosedLoop( true ) at
	//deserialization time because we don't know how the interpolator will be used.
	virtual void SetRecomputeAsAClosedLoop ( bool ClosedLoop ) = 0;
	virtual bool GetRecomputeAsAClosedLoop () const = 0;

	virtual void Recompute() = 0;

    virtual void Sample ( float32 Time , bool Looping , ValueType& Result ) const = 0;
};


} // namespace Animation
} // namespace Graphics
} // namespace WT
