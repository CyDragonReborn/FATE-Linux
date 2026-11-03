//
//	ICloneable.h
//
//	Copyright (C) 2005 WildTangent, Inc. 
//	All Rights Reserved
//
#pragma once

#include "WT/System/IInterface.h"
#include "WT/System/Object.h"


namespace WT
{
namespace System
{

// Supports creation of a new instance of a concrete class with the same values as an existing instance
class ICloneable : public IInterface
{
public:

	// Creates a new object that is a copy of the current instance
	virtual Object * Clone( ) const = 0;

};

} // namespace System
} // namespace WT
