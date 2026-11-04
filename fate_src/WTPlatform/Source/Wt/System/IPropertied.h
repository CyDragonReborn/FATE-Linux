/*
	IPropertied.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Gary Hinger
	04/17/2006
*/

#pragma once

#include "WT/System/IInterface.h"

namespace WT
{
namespace System
{

class IPropertied : public IInterface
{
public:
	WT_INTERFACE( IPropertied )

	virtual IPropertyCollection& GetProperties() = 0;
	virtual IPropertyCollection const& GetProperties() const = 0;
};

} //namespace System
} //namespace WT
