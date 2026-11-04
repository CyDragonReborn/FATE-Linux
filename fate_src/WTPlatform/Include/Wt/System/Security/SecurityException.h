/*
	SecurityException.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/SystemException.h"

namespace WT
{
namespace System
{
namespace Security
{

WT_DECLARE_EXCEPTION_CLASS( CSecurityException, SystemException, "Security Exception" )

}	// Security
}	// System
}	// WT