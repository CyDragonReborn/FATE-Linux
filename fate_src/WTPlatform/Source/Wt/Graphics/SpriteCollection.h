//
// SpriteCollection.h
//
// Copyright (C) 2005 WildTangent, Inc. 
// All Rights Reserved
//
// Overview:
//	The SpriteCollection class is derived from the RenderCollection abstract class.  It contains a collection of 
//	Sprite objects and implements the RenderObjects() method to render them.  Since this class is derived from 
//	RenderCollection class a single pre and post render state can be set for all sprites in the collection, thus
//	preventing redundant device render state changes.
//

#pragma once

#include <vector>
#include "WT/Graphics/RenderCollection.h"
#include "WT/Graphics/Sprite.h"


namespace WT
{
namespace Graphics
{

typedef std::vector<CSprite*> SpriteVector;

typedef System::SmartPointer<class CSpriteCollection> HSpriteCollection;
class CSpriteCollection : public CRenderCollection
{
public:
	WT_CLASS( CSpriteCollection, CRenderCollection );

	CSpriteCollection() {}
	~CSpriteCollection() {}

	SpriteVector& GetSprites()
	{
		return m_Sprites;
	}

protected:

	void RenderObjects( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

private:

	SpriteVector	m_Sprites;
};


} // namespace Graphics
} // WT
