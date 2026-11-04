/*
	AudioException.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Thomas Petchel
	07/05/2005
*/

#pragma once

#include "WT/System/Exception.h"

namespace WT
{
namespace Audio
{

	class CAudioException : public System::CException
{
public:

	WT_CLASS( CAudioException, System::CException )

	explicit CAudioException( std::string const& Message = "Audio Exception",
		std::string const& File = "unknown", int32 Line = -1, std::string const& Function = "unknown" );

	int32 GetFMODError() const {return m_FMODError;}
	std::string const& GetFMODErrorString() const {return m_FMODErrorString;}

private:
	int32 m_FMODError;
	std::string m_FMODErrorString;
};

} //namespace Audio
} //namespace WT
