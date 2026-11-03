//
// ParticleSystemCollection.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The CParticleSystemCollection class is derived from the RenderCollection abstract class.  It contains
//	a collection of CParticleSystemData objects and provides methods for animating and rendering each
//	particle system.  Since this is derived from RenderCollection, a pre-render and post-render device render
//	state can be defined that will apply to all particle system objects in the collection.
//

#pragma once

#include <vector>
#include "WT/Graphics/RenderCollection.h"
#include "WT/Graphics/IUpdateable.h"
#include "WT/Particles/ParticleSystemData.h"


namespace WT
{
namespace Particles
{

typedef std::vector<HParticleSystemData> ParticleSystemVector;

typedef System::SmartPointer<class CParticleSystemCollection> HParticleSystemCollection;
class CParticleSystemCollection : public Graphics::CRenderCollection, public Graphics::IUpdateable
{
public:
	WT_BEGIN_CLASS( CParticleSystemCollection, Graphics::CRenderCollection );
		WT_INHERITS_INTERFACE( Graphics::IUpdateable );
	WT_END_CLASS();

	CParticleSystemCollection() {}
	virtual ~CParticleSystemCollection() {}

	ParticleSystemVector& GetParticleSystems()
	{
		return m_ParticleSystems;
	}

	virtual void Update( float32 ElapsedTimeSeconds );

protected: // methods

	void RenderObjects( LPDIRECT3DDEVICE9 pDevice, Graphics::CRenderContext const& Context );

private: // data

	ParticleSystemVector	m_ParticleSystems;
};


} // namespace Particles
} // namespace WT
