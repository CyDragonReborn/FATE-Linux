/***************************************************************************************
 *
 * TranslucentStandardMaterial.cpp
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          05/3/2006
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/Graphics/TranslucentStandardMaterial.h"

using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CTranslucentStandardMaterial );

CTranslucentStandardMaterial::CTranslucentStandardMaterial( CStandardMaterial const* pStandardMaterial, System::SmartPointer<float32> const& hOpacity )
	: CTranslucentMaterial( hOpacity )
	, m_hStandardMaterial( pStandardMaterial )
{
	if (pStandardMaterial == NULL)
	{
		WT_THROW( System::CArgumentException, "pStandardMaterial" );
	}
}

CStandardMaterial const* CTranslucentStandardMaterial::GetStandardMaterial() const
{
	return m_hStandardMaterial;
}

/*virtual*/ void CTranslucentStandardMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	assert( pDevice != NULL );

	m_hStandardMaterial->BeginPass( pDevice, Context, PassIndex );

	assert( PassIndex == 0 );

	int32 iOpacity = Float32ToInt32( GetOpacity() * 255.0f );

	if (iOpacity >= 255)
	{
		//Finished; no need to apply opacity.
		return;
	}

	//Find the first disabled texture stage 
	int32 Stage = 0;
	for (; Stage <= 7; ++Stage)
	{
		DWORD Value;
		HRESULT Result = pDevice->GetTextureStageState( Stage, D3DTSS_COLOROP, &Value );
		if (FAILED( Result ))
		{
			WT_THROW( System::CRuntimeException, "GetTextureStageState failure" );
		}

		if (Value == D3DTOP_DISABLE)
		{
			break;
		}
	}

	if (Stage == 0)
	{
		CTranslucentMaterial::BeginPass( pDevice, Context, PassIndex );
	} else {
		iOpacity = std::max<int32>( iOpacity, 0 );
		D3DCOLOR TextureFactor = D3DCOLOR_RGBA( 0xff, 0xff, 0xff, iOpacity );
		pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, TextureFactor );

		--Stage;

#ifndef NDEBUG
		{
			DWORD Value;
			HRESULT Result = pDevice->GetTextureStageState( Stage, D3DTSS_ALPHAOP, &Value );
			assert( SUCCEEDED( Result ) );
			assert( Value == D3DTOP_SELECTARG1 );
		}
#endif

		//Note: this tends not to work properly if SetTexture( NULL ) is called
		//and D3DTSS_ALPHAARG1 is D3DTA_TEXTURE.
		pDevice->SetTextureStageState( Stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		pDevice->SetTextureStageState( Stage, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
	}
}

}
}
