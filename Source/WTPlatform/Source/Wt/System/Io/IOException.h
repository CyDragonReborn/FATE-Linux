/*
	IOException.h

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
namespace IO
{

WT_DECLARE_EXCEPTION_CLASS( CIOException, CSystemException, "IO Exception" )


WT_DECLARE_EXCEPTION_CLASS( CDirectoryNotFoundException,	CIOException, "Directory not found" )
WT_DECLARE_EXCEPTION_CLASS( CFileNotFoundException,			CIOException, "File not found" )
WT_DECLARE_EXCEPTION_CLASS( CPathTooLongException,			CIOException, "Path too long" )
WT_DECLARE_EXCEPTION_CLASS( CUnexpectedEndOfDataException,	CIOException, "End of stream encountered when more data was expected" )


}	// IO
}	// System
}	// WT