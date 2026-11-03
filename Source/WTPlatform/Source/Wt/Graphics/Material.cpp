/***************************************************************************************
 *
 * Material.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          07/13/2005
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"

#include "WT/System/IO/IOException.h"
#include "WT/System/IO/File.h"
#include "WT/System/Logging/Log.h"
#include "WT/Graphics/Material.h"
#include "WT/Image/Image.h"
#include "WT/Math/MathSerialization.h"

#ifdef _DEBUG
#pragma comment( lib, "d3dx9d" )
#pragma comment( lib, "d3dx9dt" )
#else
#pragma comment( lib, "d3dx9" )
#endif

using namespace WT::Math;
using namespace WT::System;
using namespace WT::System::Serialization;
using namespace WT::System::IO;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CMaterial )
WT_DEFINE_CLASS( CNullMaterial )
WT_DEFINE_CLASS( CDefaultMaterial )
WT_DEFINE_CLASS( CMaterialDiffuse )
WT_DEFINE_CLASS( CMaterialTextured )
WT_DEFINE_CLASS( CMaterialTexturedDiffuse )

/*static*/ System::SmartPointer<CMaterial const>	CMaterial::gm_hCachedMaterial;
/*static*/ System::SmartPointer<CMaterial const>	CMaterial::gm_hDefaultMaterial;
/*static*/ System::SmartPointer<CMaterial const>	CMaterial::gm_hNullMaterial;
/*static*/ int32									CMaterial::gm_CachedPassCount = 0;

D3DMATERIAL9 const CMaterial::gm_DefaultD3DMaterial = 
	{{ 1.0f,1.0f,1.0f,1.0f,},{ 1.0f,1.0f,1.0f,1.0f,}, { 0.0f,0.0f,0.0f,0.0f,},{ 0.0f,0.0f,0.0f,0.0f,}, 0.0f,};

/*static*/ void WTCall CMaterial::SetCachedMaterial( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, CMaterial const* pMaterial )
{
	if (!Context.GetUseMaterials())
	{
		if (gm_hNullMaterial == NULL)
		{
			gm_hNullMaterial = new CNullMaterial();
		}

		pMaterial = gm_hNullMaterial;
	} else if (pMaterial == NULL) {
		if (gm_hDefaultMaterial == NULL)
		{
			gm_hDefaultMaterial = new CDefaultMaterial();
		}

		pMaterial = gm_hDefaultMaterial;
	}

	assert( pMaterial != NULL );

	if (gm_hCachedMaterial == pMaterial)
	{
		return;
	}

	if (gm_hCachedMaterial != NULL)
	{
		gm_hCachedMaterial->End();
	}

	pMaterial->Begin( pDevice, Context, gm_CachedPassCount );

	gm_hCachedMaterial = pMaterial;
}

// Resets the cached material and forces an update of the device.
// ResetCachedMaterial() should be called before rendering each collection of materials,
// or after a cached material has been deleted.
/*static*/ void WTCall CMaterial::ResetCachedMaterial( CRenderContext const& Context, LPDIRECT3DDEVICE9 pDevice )
{
	if (gm_hCachedMaterial != NULL)
	{
		gm_hCachedMaterial->End();
	}

	gm_hCachedMaterial = NULL;
	gm_hDefaultMaterial = NULL;
	gm_hNullMaterial = NULL;

	pDevice->SetMaterial( &gm_DefaultD3DMaterial );
	Context.SetTexture( pDevice, 0, NULL );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
}

/*virtual*/ void CMaterial::Begin( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32& PassCount ) const
{
	pDevice;
	Context;
	PassCount = 1;
}

/*virtual*/ void CMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	if (PassIndex < 0 || PassIndex >= 1)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}
	pDevice;
	Context;
}

/*virtual*/ void CMaterial::EndPass() const
{
}

/*virtual*/ void CMaterial::End() const
{
}

void CMaterial::InvalidateDeviceObjects()
{
}

void CMaterial::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	pDevice;
}

bool CMaterial::IsValid() const
{
	return true;
}

}
}
