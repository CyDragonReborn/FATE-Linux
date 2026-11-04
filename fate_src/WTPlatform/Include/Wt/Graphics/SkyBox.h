/*
	SkyBox.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved
*/

#pragma once

#include "d3d9.h"
#include "WT/System/Object.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/Material.h"

namespace WT
{

namespace Graphics
{

typedef System::SmartPointer<class CSkyBox> HSkyBox;
class CSkyBox : public System::CObject, public CRenderableImpl
{
public:

	WT_BEGIN_CLASS( CSkyBox, CObject );
		WT_INHERITS_INTERFACE( IRenderable );
	WT_END_CLASS();

	CSkyBox();
	CSkyBox( IDirect3DDevice9* pDevice, CMaterial* pMaterial );
	CSkyBox( IDirect3DDevice9* pDevice, const std::string& CubeTextureName );
	virtual ~CSkyBox();

	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

protected:

	HRESULT InitVB( IDirect3DDevice9* pDevice , const std::string& CubeTextureName );

	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DIndexBuffer9* m_pIB;
	IDirect3DCubeTexture9* m_pTexture;
	CMaterial* m_pMaterial;
};

}
}
