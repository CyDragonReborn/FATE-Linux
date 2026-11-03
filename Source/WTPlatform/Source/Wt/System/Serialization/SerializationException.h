
/*
	SerializationException.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

*/

#pragma once

#include "WT/System/SystemException.h"

namespace WT
{
namespace System
{
namespace Serialization
{

WT_DECLARE_EXCEPTION_CLASS( CSerializationException, CSystemException, "Serialization Exception" )

WT_DECLARE_EXCEPTION_CLASS( CSerializationTypeUnknownException, CSerializationException, "Encountered an unknown type when deserializing" )

WT_DECLARE_EXCEPTION_CLASS( CSerializationFormatVersionError, CSerializationException, "Attempting to deserialize a stream with a formatter that is older than the formatter that generated it." )

}	// Serialization
}	// System
}	// WT
