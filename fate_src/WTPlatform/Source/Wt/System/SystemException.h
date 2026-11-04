/*
	SystemException.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/Exception.h"

namespace WT
{
namespace System
{

WT_DECLARE_EXCEPTION_CLASS( CSystemException, CException, "System Exception" )


WT_DECLARE_EXCEPTION_CLASS( CUnauthorizedAccessException,	CSystemException,	"Unauthorized Access" )
WT_DECLARE_EXCEPTION_CLASS( CArgumentException,				CSystemException,	"Invalid argument" )
WT_DECLARE_EXCEPTION_CLASS( CArgumentOutOfRangeException,	CArgumentException,	"Argument out of range" )
WT_DECLARE_EXCEPTION_CLASS( CNotSupportedException,			CSystemException,	"Not supported" )
WT_DECLARE_EXCEPTION_CLASS( COutOfMemoryException,			CSystemException,	"Out of Memory" ) 
WT_DECLARE_EXCEPTION_CLASS( CElementNotFoundException,		CSystemException,	"Element Not Found" ) 
WT_DECLARE_EXCEPTION_CLASS( CElementAlreadyExistsException,	CSystemException,	"Element Already Exists" ) 
WT_DECLARE_EXCEPTION_CLASS( CIndexOutOfRangeException,		CSystemException,	"Index out of range" )
WT_DECLARE_EXCEPTION_CLASS( CRuntimeException,				CSystemException,	"Runtime Exception" )


}	// System
}	// WT