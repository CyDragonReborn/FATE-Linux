/***************************************************************************************
 *
 * TranslucentActor.cpp
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
#include "WT/Graphics/TranslucentActor.h"
#include "WT/Graphics/TranslucentStandardMaterial.h"

using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CTranslucentMaterial )
WT_DEFINE_CLASS( CTranslucentActor )

CTranslucentMaterial::CTranslucentMaterial( System::SmartPointer<float32> const& hOpacity )
	: m_hOpacity( hOpacity )
{
}

float32 CTranslucentMaterial::GetOpacity() const
{
	return *m_hOpacity;
}

/*virtual*/ void CTranslucentMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	if (PassIndex < 0 || PassIndex >= 1)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}

	int32 iOpacity = Float32ToInt32( GetOpacity() * 255.0f );

	if (iOpacity >= 255)
	{
		//Finished; no need to apply opacity.
		return;
	}

	iOpacity = std::max<int32>( iOpacity, 0 );
	D3DCOLOR TextureFactor = D3DCOLOR_RGBA( 0xff, 0xff, 0xff, iOpacity );
	pDevice->SetRenderState( D3DRS_TEXTUREFACTOR, TextureFactor );

	pDevice->SetMaterial( &gm_DefaultD3DMaterial );
	Context.SetTexture( pDevice, 0, NULL );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
}

CTranslucentActor::CTranslucentActor()
	: m_hOpacity( new float32( 1.0f ) )
{
}

float32 CTranslucentActor::GetOpacity() const
{
	return *m_hOpacity;
}

void CTranslucentActor::SetOpacity( float32 Opacity )
{
	if (Opacity < 0.0f || Opacity > 1.0f)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "Opacity" );
	}

	*m_hOpacity = Opacity;
}

System::SmartPointer<float32> const& CTranslucentActor::GetOpacityPointer()
{
	return m_hOpacity;
}

void CTranslucentActor::SetOpacityPointer( System::SmartPointer<float32> const& hOpacity )
{
	if (hOpacity == NULL)
	{
		WT_THROW( System::CArgumentException, "hOpacity" );
	}

	m_hOpacity = hOpacity;
}

void CTranslucentActor::SetFromActor( CActor const& Actor )
{
	CActor::operator=( Actor );

	//Parts without materials will be assigned a translucent material.
	//If we happen to create a translucent material, save the index for later for this 
	//purpose.
	int32 TranslucentMaterialIndex = -1;

	if (GetMaterials() != NULL)
	{
		//Clone the materials array. Replace all CStandardMaterial's with CTranslucentStandardMaterial's,
		//and all CTranslucentMaterial's or NULL's with CTranslucentMaterial's.
		System::SmartPointer< std::vector<HMaterial> > hMaterials = new std::vector<HMaterial>( *GetMaterials() );
		SetMaterials( hMaterials );
		int32 const MaterialCount = static_cast<int32>( hMaterials->size() );
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			CMaterial const* pMaterial = hMaterials->operator[]( i );

			if (pMaterial == NULL)
			{
				//If there is no material, replace it with a translucent material.
				goto LNeedTranslucentMaterial;
			}

			CStandardMaterial const* pStandardMaterial;

			CTranslucentMaterial const* pTranslucentMaterial =
				DynamicCast<CTranslucentMaterial const>( pMaterial );

			if (pTranslucentMaterial != NULL)
			{
				CTranslucentStandardMaterial const* pTranslucentStandardMaterial = 
					DynamicCast<CTranslucentStandardMaterial const>( pMaterial );
				
				if (pTranslucentStandardMaterial != NULL)
				{
					//This is a translucent standard material; we need to create a new translucent 
					//standard material based on the underlying standard material. This allows us
					//to have a separate opacity value.
					pStandardMaterial = pTranslucentStandardMaterial->GetStandardMaterial();
				} else {
					//This is a translucent material. Replace it with a translucent material
					//using our own own opacity value.
					goto LNeedTranslucentMaterial;
				}
			} else {
				pStandardMaterial = DynamicCast<CStandardMaterial const>( pMaterial );
			}

			//We can only do something at this point if there is a standard material to work with.
			if (pStandardMaterial != NULL)
			{
				hMaterials->operator[]( i ) = new CTranslucentStandardMaterial( pStandardMaterial, m_hOpacity );
			}
			continue;

LNeedTranslucentMaterial:
			//Replace the material with a translucent material.
			if (TranslucentMaterialIndex == -1)
			{
				HTranslucentMaterial hTranslucentMaterial = new CTranslucentMaterial( m_hOpacity );
				hMaterials->operator[]( i ) = hTranslucentMaterial.get();
				TranslucentMaterialIndex = i;
			} else {
				//Reuse the translucent material that was created earlier.
				hMaterials->operator[]( i ) = hMaterials->operator[]( TranslucentMaterialIndex );
			}
		}
	}

	if (GetMesh() != NULL)
	{
		//Clone the mesh and move all parts into the post-opaque category
		HMesh hMesh = GetMesh()->Clone();
		SetMesh( hMesh );
		CMesh::Parts& Dest = hMesh->GetParts( ERenderCategory::PostOpaque );
		CMesh::Parts& Source1 = hMesh->GetParts( ERenderCategory::PreOpaque );
		CMesh::Parts& Source2 = hMesh->GetParts( ERenderCategory::Opaque );
		Dest.resize( Dest.size() + Source1.size() + Source2.size() );
		std::copy( Source1.begin(), Source1.end(), Dest.begin() + (Dest.size() - Source1.size() - Source2.size()) );
		std::copy( Source2.begin(), Source2.end(), Dest.begin() + (Dest.size() - Source2.size()) );
		Source1.clear();
		Source2.clear();

		//Check for parts without materials; assign such parts to use a CTranslucentMaterial.
		std::vector<HMaterial>* pMaterials = GetMaterials().get();
		int32 const MaterialCount = pMaterials == NULL ? 0 : static_cast<int32>( pMaterials->size() );
		int32 const PartCount = static_cast<int32>( Dest.size() );
		for (int32 i = 0; i < PartCount; ++i)
		{
			CMeshPart* const pPart = Dest[i];
			if (pPart->GetMaterialID() < 0 || pPart->GetMaterialID() >= MaterialCount)
			{
				if (TranslucentMaterialIndex == -1)
				{
					if (!pMaterials)
					{
						System::SmartPointer< std::vector<HMaterial> > hMaterials = new std::vector<HMaterial>;
						SetMaterials( hMaterials );
						pMaterials = hMaterials.get();
					}

					HTranslucentMaterial hTranslucentMaterial = new CTranslucentMaterial( m_hOpacity );
					pMaterials->push_back( hTranslucentMaterial.get() );
					TranslucentMaterialIndex = MaterialCount;
				}

				pPart->SetMaterialID( TranslucentMaterialIndex );
			}
		}
	}
}

}
}
