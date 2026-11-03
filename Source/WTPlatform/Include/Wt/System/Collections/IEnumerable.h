/*
	IEnumerable.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	01/04/2005
*/

#pragma once

#include "WT/System/Collections/IEnumerator.h"

namespace WT
{
namespace System
{
namespace Collections
{
	
template<typename T>
class IEnumerable
{
public:
	
	// Returns an enumerator that can iterate through a collection
	virtual IEnumerator<T>* GetEnumerator() = 0;
};


}	// Collections
}	// System
}	// WT