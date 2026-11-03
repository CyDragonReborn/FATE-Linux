//
// ParticleSystemCollection.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Particles/ParticleSystemCollection.h"


namespace WT
{
namespace Particles
{


void CParticleSystemCollection::Update( float32 FrameTime )
{
	ParticleSystemVector::const_iterator it = m_ParticleSystems.begin();
	while ( it != m_ParticleSystems.end() )
	{
		(*it)->Update( FrameTime );
		++it;
	}
}

void CParticleSystemCollection::RenderObjects( LPDIRECT3DDEVICE9 pDevice, Graphics::CRenderContext const& Context )
{
	ParticleSystemVector::const_iterator it = m_ParticleSystems.begin();
	while ( it != m_ParticleSystems.end() )
	{
		(*it)->Render( pDevice, Context );
		++it;
	}
}


} // namespace Particles
} // namespace WT
