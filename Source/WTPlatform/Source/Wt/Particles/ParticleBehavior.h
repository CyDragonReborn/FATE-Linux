//
// ParticleBehavior.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The CParticleBehavior class is an abstract class describing a single behavior of a particle (See the CParticle class).
//	This class exposes a single Update() method that is used to update the attributes of all CParticle objects in the 
//	provided CParticleSystemData (See the CParticleSystemData class) object.  
//	A concrete particle behavior object will derive from this class and implement the Update() method to modify the 
//	CParticleSystemData object collection of particle's based on the current frame time.
//	The CParticleSystemData class holds a collection of these behavior objects and more than one behavior can 
//	be applied to each particle.  For example there may be one behavior to define how/where/when a particle is
//	emitted, another to modify a particle's position based on velocity and another that governs the lifetime 
//	of a particle.
//

#pragma once

#include "WT/System/Object.h"

namespace WT
{
namespace Particles
{

class CParticleSystemData;

typedef System::SmartPointer<class CParticleBehavior> HParticleBehavior;
class CParticleBehavior : public System::CObject
{
public:

	WT_ABSTRACT_CLASS(CParticleBehavior, System::CObject)

	CParticleBehavior() : m_ID( -1 ) {}

	// Accessors
	int32 GetID()
	{
		return m_ID;
	}
	void SetID( int32 ID )
	{
		m_ID = ID;
	}

	virtual void Update ( CParticleSystemData* pData , float32 FrameTime ) = 0;

protected: // data

	int32		m_ID;
};

} //namespace Particles
} //namespace WT
