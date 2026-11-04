/***************************************************************************************
 *
 * CompositeMaterial.cpp
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
#include "WT/Graphics/CompositeMaterial.h"

using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CCompositeMaterial )

CCompositeMaterial::CCompositeMaterial()
	: m_ResourcesValid( false )
	, m_bTwoSided( false )
{
}

CCompositeMaterial::~CCompositeMaterial()
{
}

void CCompositeMaterial::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;

	for (int32 i = 0; i < static_cast<int32>( m_SubMaterials.size() ); ++i)
	{
		CMaterial* const pMaterial = m_SubMaterials[i].m_hMaterial;
		if (pMaterial != NULL)
		{
			pMaterial->InvalidateDeviceObjects();
		}
	}
}

void CCompositeMaterial::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	for (int32 i = 0; i < static_cast<int32>( m_SubMaterials.size() ); ++i)
	{
		CMaterial* const pMaterial = m_SubMaterials[i].m_hMaterial;
		if (pMaterial != NULL)
		{
			pMaterial->RestoreDeviceObjects( pDevice );
		}
	}

	m_ResourcesValid = true;
}

bool CCompositeMaterial::IsValid() const
{
	return m_ResourcesValid;
}

void CCompositeMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	if (pDevice == NULL)
	{
		throw std::invalid_argument( "null device" );
	}

	if (PassIndex < 0 || PassIndex >= 1)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}

	pDevice->SetMaterial( &gm_DefaultD3DMaterial );

	int32 i;
	for (i = 0; i < static_cast<int32>( m_SubMaterials.size() ); ++i)
	{
		CMaterialMap* pMap = NULL;
		
		if (m_SubMaterials[i].m_hMaterial != NULL)
		{
			pMap = m_SubMaterials[i].m_hMaterial->GetDiffuseMap();
		}

		if (pMap != NULL)
		{
			pMap->PrepareRender( Context, pDevice, i );
		} else {
			Context.SetTexture( pDevice, i, NULL );
		}

		//Convert composite amount to color
		int32 c = Float32ToInt32( m_SubMaterials[i].m_CompositeAmount * 255.0f );
		if (c < 0) c = 0; else if (c > 255) c = 255;
		pDevice->SetTextureStageState( i, D3DTSS_CONSTANT, D3DCOLOR_ARGB( c, c, c, c ) );

		pDevice->SetTextureStageState( i, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( i, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( i, D3DTSS_RESULTARG, D3DTA_CURRENT);

		if (i == 0)
		{
			pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_MODULATE );
			pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
			pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		} else {
			switch (m_SubMaterials[i].m_CompositeType)
			{
				case EComposite::Add:
					pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR );
					pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
					pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG1, D3DTA_CONSTANT );
					break;
				case EComposite::Subtract:
					//Composite amount not supported
					pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_SUBTRACT );
					pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
					pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
					break;
				case EComposite::Mix:
					pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_LERP );
					pDevice->SetTextureStageState( i, D3DTSS_COLORARG0, D3DTA_CONSTANT );
					pDevice->SetTextureStageState( i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
					pDevice->SetTextureStageState( i, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
					break;
				default:
					assert( 0 );
					break;
			}
		}
	}

	pDevice->SetTextureStageState( i, D3DTSS_COLOROP, D3DTOP_DISABLE );

	if (Context.GetTranslucent())
	{
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}
}

/*virtual*/ void CCompositeMaterial::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	Info->AddValue( "Name", m_Name );
	Info->AddValue( "TwoSided", m_bTwoSided );

	int32 const SubMaterialCount = static_cast<int32>( m_SubMaterials.size() );
	Info->AddContainerBeginWithCount( "SubMaterials", SubMaterialCount );
	
	for (int32 i = 0; i < SubMaterialCount; ++i)
	{
		Info->AddValue<CObject*>( "Material", m_SubMaterials[i].m_hMaterial.get() );
		Info->AddValue( "CompositeType", static_cast<int32>( m_SubMaterials[i].m_CompositeType ) );
		Info->AddValue( "CompositeAmount", m_SubMaterials[i].m_CompositeAmount );
	}

	Info->AddContainerEnd();
}

/*virtual*/ void CCompositeMaterial::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	Info->GetValue( "Name", m_Name );
	{bool bTwoSided; Info->GetValue( "TwoSided", bTwoSided ); m_bTwoSided = bTwoSided;}

	m_SubMaterials.clear();

	int32 const SubMaterialCount = Info->GetContainerBeginWithCount( "SubMaterials" );

	m_SubMaterials.resize( SubMaterialCount );

	for (int32 i = 0; i < SubMaterialCount; ++i)
	{
		{
			CObject* pObject;
			Info->GetValue( "Material", pObject );

			if (pObject != NULL)
			{
				CMaterial* pMaterial = DynamicCast<CMaterial>( pObject );

				if (pMaterial == NULL)
				{
					throw std::runtime_error( "corrupt stream" );
				}

				m_SubMaterials[i].m_hMaterial = pMaterial;
			}
		}

		int32 CompositeType;
		Info->GetValue( "CompositeType", CompositeType );
		SetCompositeType( i, static_cast<EComposite::Type>( CompositeType ) );

		Info->GetValue( "CompositeAmount", m_SubMaterials[i].m_CompositeAmount );

	}

	Info->AddContainerEnd();
}

}
}
