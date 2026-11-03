//
// ParticleSystem.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#pragma once

#include "WT/Particles/ParticleSystemData.h"
#include "WT/Particles/ParticleBehaviors.h"
#include "WT/Math/Math.h"


namespace WT
{
namespace Particles
{

class ParticleSystem
{
public:

	// Single Using() method to initialize all types needed for the particle system.  Types must be
	// initialized before deserialization can work.
	static void Using()
	{
		WT::Using<CParticleSystemData>();
		WT::Using<CEmitParticleBehavior>();
		WT::Using<CAnimateParticleBehavior>();
		WT::Using<CBounceParticleBehavior>();
		WT::Using<CFadeParticleBehavior>();
		WT::Using<CGravityParticleBehavior>();
		WT::Using<CImageEmitParticleBehavior>();
		WT::Using<CKillInsideParticleBehavior>();
		WT::Using<CKillOldParticleBehavior>();
		WT::Using<CMoveParticleBehavior>();
		WT::Using<CPointAttractParticleBehavior>();
		WT::Using<CRandomMoveParticleBehavior>();
		WT::Using<CRotateParticleBehavior>();
		WT::Using<CScaleParticleBehavior>();
		WT::Using<CTargetSizeParticleBehavior>();

		WT::Using<Math::CPlaneShape>();
		WT::Using<Math::CDisc>();
		WT::Using<Math::CLine>();
		WT::Using<Math::CPoint3d>();
		WT::Using<Math::CRect>();
	}

protected:
	ParticleSystem();
};


} // namespace Particles
} // WT
