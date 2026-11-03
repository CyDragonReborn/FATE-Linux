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
namespace Xml
{

WT_DECLARE_EXCEPTION_CLASS( CXMLException, CSystemException, "XML Exception" );

WT_DECLARE_EXCEPTION_CLASS( CXMLReaderException, CXMLException, "XML Reader Exception" );
WT_DECLARE_EXCEPTION_CLASS( CXMLWriterException, CXMLException, "XML Writer Exception" );


}	// XML
}	// System
}	// WT
