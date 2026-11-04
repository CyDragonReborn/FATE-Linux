/*
	NetException.h

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
namespace Net
{

WT_DECLARE_EXCEPTION_CLASS( CNetException, CSystemException, "Net Exception" )
WT_DECLARE_EXCEPTION_CLASS( CNoSuchData,	CNetException, "No such data" )


}	// IO
}	// System
}	// WT