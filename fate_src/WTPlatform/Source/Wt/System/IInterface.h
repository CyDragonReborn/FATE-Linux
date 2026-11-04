/*
	IInterface.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Paul Higinbotham
	7/12/2005
*/
#pragma once

#include "WT/System/Type.h"

namespace WT
{
namespace System
{

//
// Base class for all interface classes
//
class IInterface
{
public:
	WT_BEGIN_INTERFACE( IInterface );
    WT_END_INTERFACE()

	virtual ~IInterface() = 0 {};

	virtual const CType* GetType() const = 0;
	virtual CObject const * GetObjectPtr() const = 0;
	virtual CObject * GetObjectPtr() = 0;
};


} // namespace System
} // namespace WT
