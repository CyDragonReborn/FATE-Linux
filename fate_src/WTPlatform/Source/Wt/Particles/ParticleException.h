//
// ParticleException.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The ParticleException class is the exception base class used for all exceptions thrown by the particle system.
//

#pragma once

#include "WT/System/SystemException.h"

namespace WT
{
namespace Particles
{

	WT_DECLARE_EXCEPTION_CLASS( CParticleException, System::CSystemException, "Particle Exception" )

} // Particles
} // WT
