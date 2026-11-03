//
//	Average.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"
#include <deque>


namespace WT
{
namespace Math
{

using std::deque;

//
// Classes
//

template< class T >
class CAverage
{
public:
	CAverage( int32 OperandCount = 32 );

    void __fastcall PushOperand( const T& Operand );
	void __fastcall GetAverage( T& Avg );

	int32		DataCount;
	deque<T>	DataQueue;
};


template< class T >
CAverage<T>::CAverage( int32 OperandCount )
{
	DataCount = OperandCount;
}

template< class T >
void __fastcall CAverage<T>::PushOperand( const T& Operand )
{
	if( DataQueue.size() == DataCount )
		DataQueue.pop_front();

	DataQueue.push_back( Operand );
}

template< class T >
void __fastcall CAverage<T>::GetAverage( T& Avg )
{
	deque<T>::const_iterator it = DataQueue.begin();

	for(; it != DataQueue.end(); ++it )
	{
		if( it == DataQueue.begin() ) 
			Avg = *it;
		else
			Avg += *it;
	}

	Avg *= 1.0f/(float32)DataCount;
}


} // namespace Math
} // namespace WT
