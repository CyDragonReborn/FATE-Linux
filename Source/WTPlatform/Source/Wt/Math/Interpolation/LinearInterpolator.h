//
//	LinearInterpolator.h
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


//
// A linear interpolation
//

template< typename VT>
__forceinline void WTCall LinearInterpolate ( float32 T, VT const& P0, VT const& P1, VT& Result )
{
	Result = P0 + (P1 - P0) * T;
}


} // namespace Math
} // WT
