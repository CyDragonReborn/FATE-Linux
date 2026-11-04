//
// FontUtil.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/System/Platform.h"
#include <string>

namespace WT
{
namespace Graphics
{
namespace UI
{


class CFontUtil
{
public:

	// reorder the given string to a visual order if the string is non LTR reading order
	static void WTCall ReorderString ( std::wstring& Text );
};


} // namespace UI
} // namespace Graphics
} // namespace WT
