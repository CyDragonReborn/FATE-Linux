//
// SpriteCollection.cpp
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//

#include "WT/stdafx.h"
#include "WT/Graphics/SpriteCollection.h"


namespace WT
{
namespace Graphics
{


void CSpriteCollection::RenderObjects( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context )
{
	// Render sprites
	SpriteVector::const_iterator it = m_Sprites.begin();
	while ( it != m_Sprites.end() )
	{
		(*it)->Render( pDevice, Context );
		++it;
	}
}


} // namespace Graphics
} // WT
