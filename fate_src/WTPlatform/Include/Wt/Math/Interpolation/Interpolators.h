//
//	Interpolators.h
//	
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/Platform.h"

namespace WT
{
namespace Math
{


// y = a*x + b;
// y = val;
// a = (val_end-val_start)/(tick_end-tick_start)
// x = tick_current;
// b = val_start;
template<typename V, typename T>
class InterpolateTimedVal
{
public:
	V	m_A;
	V	m_val_start;
	V	m_val_end;
	T	m_tick_end;
	T	m_tick_start;

	T	m_tick_current;
	V	m_val_current;

public:
	__forceinline void __fastcall Reset();
	__forceinline void __fastcall Reset(V targetval);
	__forceinline void __fastcall Move(T tickdelta);
	__forceinline void __fastcall Start(V startval, T maxtick, T mintick);
	__forceinline T	   __fastcall Status();
};

template<typename V, typename T>
class InterpolateLoopTimedVal : public InterpolateTimedVal<V,T>
{
public:

public:
	__forceinline void __fastcall Start(V startval, T maxtick, T mintick);
	__forceinline void __fastcall Move(T tickdelta);
};

template<typename V, typename T>
class InterpolateBounceTimedVal : public InterpolateTimedVal<V,T>
{
public:
	bool	m_increasing;

public:
	__forceinline void __fastcall Start(V startval, T maxtick, T mintick);
	__forceinline void __fastcall Move(T tickdelta);
};

#include "Interpolators.inl"


} // namespace Math
} // namespace WT
