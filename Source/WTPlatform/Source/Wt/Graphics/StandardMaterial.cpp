/***************************************************************************************
 *
 * StandardMaterial.cpp
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
#include "WT/Graphics/StandardMaterial.h"
#include "WT/Graphics/Graphics.h"

using namespace WT::System;

namespace WT
{
namespace Graphics
{

WT_DEFINE_CLASS( CStandardMaterial )

void CStandardMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	//Note: in this function, the final stage should have D3DTSS_ALPHAOP, D3DTOP_SELECTARG1.
	//This makes it easy for CTranslucentStandardMaterial to override the final stage's alpha operation.

	if (pDevice == NULL)
	{
		return;
	}

	if (PassIndex < 0 || PassIndex >= 1)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}

	float32 OffsetU = 0.0f;
	float32 OffsetV = 0.0f;

	if (m_HorizVelocity || m_VertVelocity || m_FrameTimeSeconds)
	{
		OffsetU = static_cast<float32>( fmod( m_HorizVelocity * gSimulationTimeSeconds, 1000.0 ) );
		OffsetV = static_cast<float32>( fmod( m_VertVelocity * gSimulationTimeSeconds, 1000.0 ) );
		
		if (m_FrameTimeSeconds)
		{

			int32 Frames = /*round to zero*/ static_cast<int32>( gSimulationTimeSeconds / m_FrameTimeSeconds );
			
			if (m_HorizFrameCount > 1)
			{
				OffsetU += static_cast<float32>( Frames % m_HorizFrameCount ) / m_HorizFrameCount;
			}

			if (m_VertFrameCount > 1)
			{
				int32 VFrames = Frames;
				if (m_HorizFrameCount > 1)
				{
					VFrames /= m_HorizFrameCount;
				}

				OffsetV += static_cast<float32>( VFrames % m_VertFrameCount ) / m_VertFrameCount;
			}
		}
	}

	pDevice->SetMaterial( &m_Material );

	int32 StageIndex = 0;
	if (m_pMaps[KMapTypeDiffuse] != NULL)
	{
		m_pMaps[KMapTypeDiffuse]->PrepareRender( Context, pDevice, StageIndex, OffsetU, OffsetV );

		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );

		++StageIndex;
	}

	if (m_pMaps[KMapTypeBump] != NULL)
	{
		m_pMaps[KMapTypeBump]->PrepareRender( Context, pDevice, StageIndex, OffsetU, OffsetV );

		pDevice->SetTextureStageState( StageIndex, D3DTSS_TEXCOORDINDEX, 0 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_BUMPENVMAPLUMINANCE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT);

		//TODO expose interfaces for controlling bump mapping properties
		float32 const fZero = 0.0f;
		float32 const fOne = 1.0f;
		DWORD const& Zero = *reinterpret_cast<DWORD const*>( &fZero );
		DWORD const& One = *reinterpret_cast<DWORD const*>( &fOne );

		pDevice->SetTextureStageState( StageIndex, D3DTSS_BUMPENVMAT00, One );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_BUMPENVMAT01, Zero );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_BUMPENVMAT10, Zero );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_BUMPENVMAT11, One );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_BUMPENVLSCALE, One );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_BUMPENVLOFFSET, Zero );

		//Specular/diffuse environment map
		++StageIndex;

		LPDIRECT3DBASETEXTURE9 pDiffuseTexture = NULL;
		if (m_pMaps[KMapTypeDiffuse] != NULL && m_pMaps[KMapTypeDiffuse]->GetTexture() != NULL)
		{
			pDiffuseTexture = m_pMaps[KMapTypeDiffuse]->GetTexture()->GetD3DTexture();
		}
		Context.SetTexture( pDevice, StageIndex, pDiffuseTexture );

		pDevice->SetTextureStageState( StageIndex, D3DTSS_TEXCOORDINDEX, 0 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_ADD );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );

		++StageIndex;
	}

	if (m_pMaps[KMapTypeReflection] != NULL)
	{
 		m_pMaps[KMapTypeReflection]->PrepareRender( Context, pDevice, StageIndex, OffsetU, OffsetV );
	
		pDevice->SetTextureStageState( StageIndex, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR );

		int32 iConstant = Float32ToInt32( m_MapAmounts[KMapTypeReflection] * 255.0f );
		if (iConstant < 0) iConstant = 0; else if (iConstant > 255) {iConstant = 255;}
		iConstant += iConstant << 8;
		iConstant += iConstant << 16;

		if (StageIndex == 0)
		{
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_MODULATE );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );
			if (m_MapAmounts[KMapTypeReflection] != 1.0f)
			{
				++StageIndex;
				pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_MODULATE );
				pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_CURRENT );
				pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_CONSTANT );
				pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
				pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
				pDevice->SetTextureStageState( StageIndex, D3DTSS_CONSTANT, iConstant );
				pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );
			}
		} else {
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_MODULATE );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_TEMP );
			++StageIndex;
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_MODULATEALPHA_ADDCOLOR );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_CURRENT );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_TEMP );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CONSTANT );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_CONSTANT, iConstant );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );
		}

		++StageIndex;
	}

	if (m_pMaps[KMapTypeSelfIllumination] != NULL)
	{
		m_pMaps[KMapTypeSelfIllumination]->PrepareRender( Context, pDevice, StageIndex, OffsetU, OffsetV );
		
		if (m_MapAmounts[KMapTypeSelfIllumination] == 1.0f)
		{
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_ADD );
		} else {
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD );
			pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG0, D3DTA_CONSTANT );

			int32 iConstant = Float32ToInt32( m_MapAmounts[KMapTypeSelfIllumination] * 255.0f );
			if (iConstant < 0) iConstant = 0; else if (iConstant > 255) {iConstant = 255;}
			iConstant += iConstant << 8;
			iConstant += iConstant << 16;
			pDevice->SetTextureStageState( StageIndex, D3DTSS_CONSTANT, iConstant );
		}

		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG2, D3DTA_TEXTURE );

		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );

		++StageIndex;
	}

	if (m_pMaps[KMapTypeOpacity] != NULL)
	{
		m_pMaps[KMapTypeOpacity]->PrepareRender( Context, pDevice, StageIndex, OffsetU, OffsetV );
		
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_COLORARG1, D3DTA_CURRENT );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( StageIndex, D3DTSS_RESULTARG, D3DTA_CURRENT );

		++StageIndex;
	}


	if (StageIndex == 0)
	{
		Context.SetTexture( pDevice, 0, NULL );
	}

	pDevice->SetTextureStageState( StageIndex, D3DTSS_COLOROP, D3DTOP_DISABLE );

	if (Context.GetTranslucent())
	{
		if (m_pMaps[KMapTypeSelfIllumination] != NULL &&
			m_pMaps[KMapTypeDiffuse] == NULL)
		{
			//Self-illumination map, but no diffuse map
			//Use additive transparency
			pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
		} else {
			pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		}
	}
}

/// Sets a value that specifies the ambient color of the material
void CStandardMaterial::SetAmbient( D3DCOLORVALUE const& Ambient )
{
	m_Material.Ambient = Ambient;
}

/// Retrieves the value that specifies the ambient color of the material
D3DCOLORVALUE CStandardMaterial::GetAmbient( ) const
{
	return m_Material.Ambient;
}

/// Sets a value that specifies the diffuse color of the material
void CStandardMaterial::SetDiffuse( D3DCOLORVALUE const& Diffuse )
{
	m_Material.Diffuse = Diffuse;
}

/// Retrieves the value that specifies the diffuse color of the material
D3DCOLORVALUE CStandardMaterial::GetDiffuse( ) const
{
	return m_Material.Diffuse;
}

/// Sets a value that specifies the emissive color of the material
void CStandardMaterial::SetEmissive( D3DCOLORVALUE const& Emissive )
{
	m_Material.Emissive = Emissive;
}

/// Retrieves the value that specifies the emissive color of the material
D3DCOLORVALUE CStandardMaterial::GetEmissive( ) const
{
	return m_Material.Emissive;
}

/// Sets a value that specifies the specular color of the material
void CStandardMaterial::SetSpecular( D3DCOLORVALUE const& Specular )
{
	m_Material.Specular = Specular;
}

/// Retrieves the value that specifies the specular color of the material
D3DCOLORVALUE CStandardMaterial::GetSpecular( ) const
{
	return m_Material.Specular;
}

/// Sets a value that specifies the sharpness of the material's specular highlights
void CStandardMaterial::SetSpecularPower( float32 Power )
{
	m_Material.Power = Power;
}

/// Retrieves the value that specifies the sharpness of the material's specular highlights
float32 CStandardMaterial::GetSpecularPower( ) const
{
	return m_Material.Power;
}


CMaterialMap* CStandardMaterial::GetMap( EMapType MapType ) const
{
	if (MapType < 0 || MapType > KMapTypeCount) throw std::invalid_argument( "map type" );
	return m_pMaps[ MapType ];
}

void CStandardMaterial::SetMap( EMapType MapType, CMaterialMap* pMaterialMap )
{
	if (MapType < 0 || MapType > KMapTypeCount) throw std::invalid_argument( "map type" );
	m_pMaps[ MapType ] = pMaterialMap;
}

float32 CStandardMaterial::GetMapAmount( EMapType MapType ) const
{
	if (MapType < 0 || MapType > KMapTypeCount) throw std::invalid_argument( "map type" );
	return m_MapAmounts[ MapType ];
}

void CStandardMaterial::SetMapAmount( EMapType MapType, float32 MapAmount )
{
	if (MapType < 0 || MapType > KMapTypeCount) throw std::invalid_argument( "map type" );
	m_MapAmounts[ MapType ] = MapAmount;
}

CStandardMaterial::CStandardMaterial()
	: m_ResourcesValid( false )
	, m_bTwoSided( false )
{
	m_Material.Diffuse.r = 1.0f;
	m_Material.Diffuse.g = 1.0f;
	m_Material.Diffuse.b = 1.0f;
	m_Material.Diffuse.a = 1.0f;
	m_Material.Ambient.r = 1.0f;
	m_Material.Ambient.g = 1.0f;
	m_Material.Ambient.b = 1.0f;
	m_Material.Ambient.a = 1.0f;
	m_Material.Specular.r = 0.0f;
	m_Material.Specular.g = 0.0f;
	m_Material.Specular.b = 0.0f;
	m_Material.Specular.a = 0.0f;
	m_Material.Emissive.r = 0.0f;
	m_Material.Emissive.g = 0.0f;
	m_Material.Emissive.b = 0.0f;
	m_Material.Emissive.a = 0.0f;
	m_Material.Power = 0.0f;

	m_HorizVelocity = 0.0f;
	m_VertVelocity = 0.0f;
	m_HorizFrameCount = 1;
	m_VertFrameCount = 1;
	m_FrameTimeSeconds = 0.0f;

	for (int32 i = 0; i < KMapTypeCount; ++i)
	{
		m_MapAmounts[i] = 1.0f;
	}
}

CStandardMaterial::~CStandardMaterial( )
{
}

void CStandardMaterial::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;

	for (int32 i = 0; i < KMapTypeCount; ++i)
	{
		if (m_pMaps[i] != NULL)
		{
			m_pMaps[i]->InvalidateDeviceObjects();
		}
	}
}

void CStandardMaterial::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	for (int32 i = 0; i < KMapTypeCount; ++i)
	{
		if (m_pMaps[i] != NULL)
		{
			m_pMaps[i]->RestoreDeviceObjects( pDevice );
		}
	}

	m_ResourcesValid = true;
}

bool CStandardMaterial::IsValid() const
{
	return m_ResourcesValid;
}

float32 const KShininessConversionFactor = 100.0f;

/*virtual*/ void CStandardMaterial::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	Info->AddValue( "Name", m_Name );
	Info->AddValue( "TwoSided", m_bTwoSided );

	Info->AddValue( "DiffuseMap", m_pMaps[KMapTypeDiffuse].get() );
	Info->AddValue( "OpacityMap", m_pMaps[KMapTypeOpacity].get() );

	//TODO 
	//Bump map

	Info->AddValue( "Ambient", *reinterpret_cast<CVector4f const*>( &m_Material.Ambient ) );
	Info->AddValue( "Diffuse", *reinterpret_cast<CVector4f const*>( &m_Material.Diffuse ) );
	Info->AddValue( "Specular", *reinterpret_cast<CVector4f const*>( &m_Material.Specular ) );
	Info->AddValue( "Shininess", m_Material.Power / KShininessConversionFactor);

	Info->AddValue( "HorizVelocity", m_HorizVelocity );
	Info->AddValue( "VertVelocity", m_VertVelocity );
	Info->AddValue( "HorizFrameCount", m_HorizFrameCount );
	Info->AddValue( "VertFrameCount", m_VertFrameCount );
	Info->AddValue( "FrameTimeSeconds", m_FrameTimeSeconds );

	Info->AddValue( "ReflectionMap", m_pMaps[KMapTypeReflection].get() );
	Info->AddValue( "ReflectionMapAmount", m_MapAmounts[KMapTypeReflection] );

	Info->AddValue( "SelfIlluminationMap", m_pMaps[KMapTypeSelfIllumination].get() );
	Info->AddValue( "SelfIlluminationMapAmount", m_MapAmounts[KMapTypeSelfIllumination] );
}

/*virtual*/ void CStandardMaterial::SetObjectData(CSerializationInfo* Info, CSerializationContext* sContext)
{
	LPDIRECT3D9 pD3D = reinterpret_cast<LPDIRECT3D9>( 
		sContext->GetProperties()->GetValue<uint64>( "D3D" ) );
	if (pD3D == NULL)
	{
		throw std::invalid_argument( "NULL D3D in serialization context" );
	}

	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( 
		sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	if (pDevice == NULL)
	{
		throw std::invalid_argument( "NULL D3DDevice in serialization context" );
	}

	Info->GetValue( "Name", m_Name );
	{bool bTwoSided; Info->GetValue( "TwoSided", bTwoSided ); m_bTwoSided = bTwoSided;}

	for (int32 i = 0; i < KMapTypeCount; ++i)
	{
		m_pMaps[i] = NULL;
		m_MapAmounts[i] = 1.0f;
	}

	{
		CMaterialMap* pMaterialMap;
		Info->GetValue( "DiffuseMap", pMaterialMap );
		m_pMaps[KMapTypeDiffuse] = pMaterialMap;
	}

	{
		CMaterialMap* pMaterialMap;
		Info->GetValue( "OpacityMap", pMaterialMap );
		m_pMaps[KMapTypeOpacity] = pMaterialMap;
	}

	//TODO 
	//Bump map should use texture format D3DFMT_X8L8V8U8

	Info->GetValue( "Ambient", *reinterpret_cast<CVector4f*>( &m_Material.Ambient ) );
	Info->GetValue( "Diffuse", *reinterpret_cast<CVector4f*>( &m_Material.Diffuse ) );
	Info->GetValue( "Specular", *reinterpret_cast<CVector4f*>( &m_Material.Specular ) );
	Info->GetValue( "Shininess", m_Material.Power );
	m_Material.Power *= KShininessConversionFactor;

	Info->GetValue( "HorizVelocity", m_HorizVelocity );
	Info->GetValue( "VertVelocity", m_VertVelocity );
	Info->GetValue( "HorizFrameCount", m_HorizFrameCount );
	Info->GetValue( "VertFrameCount", m_VertFrameCount );
	Info->GetValue( "FrameTimeSeconds", m_FrameTimeSeconds );

	{
		CMaterialMap* pMaterialMap;
		Info->GetValue( "ReflectionMap", pMaterialMap );
		m_pMaps[KMapTypeReflection] = pMaterialMap;
	}

	Info->GetValue( "ReflectionMapAmount", m_MapAmounts[KMapTypeReflection] );

	if (Info->GetObjectFormatVersion() >= 4)
	{
		CMaterialMap* pMaterialMap;
		Info->GetValue( "SelfIlluminationMap", pMaterialMap );
		m_pMaps[KMapTypeSelfIllumination] = pMaterialMap;

		Info->GetValue( "SelfIlluminationMapAmount", m_MapAmounts[KMapTypeSelfIllumination] );
	}
}

/*virtual*/ void CStandardMaterial::FinalizeDeserialization( CSerializationInfo* SerialInfo,
									   CSerializationContext* sContext )
{
	//_BaseClass::FinalizeDeserialization( SerialInfo, sContext );

	for (int32 i = 0; i < KMapTypeCount; ++i)
	{
		if (m_pMaps[i] != NULL && !SerialInfo->ObjectIsFinalized( m_pMaps[i].get() ) )
		{
			throw std::runtime_error( "ObjectIsFinalized() failure - possibly due to circular references" );
		}
	}

	LPDIRECT3D9 pD3D = reinterpret_cast<LPDIRECT3D9>( sContext->GetProperties()->GetValue<uint64>( "D3D" ) );
	assert( pD3D != NULL );

	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( 
		sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	assert( pDevice != NULL );

	TexturePaths Paths;
	if (m_pMaps[KMapTypeDiffuse] != NULL && !m_pMaps[KMapTypeDiffuse]->GetTexturePath().empty())
	{
		Paths.Diffuse = StringLower( FindResourceFile( sContext, m_pMaps[KMapTypeDiffuse]->GetTexturePath(), "TexturePaths" ) );
	}
	if (m_pMaps[KMapTypeOpacity] != NULL && !m_pMaps[KMapTypeOpacity]->GetTexturePath().empty())
	{
		Paths.Opacity = StringLower( FindResourceFile( sContext, m_pMaps[KMapTypeOpacity]->GetTexturePath(), "TexturePaths" ) );
	}

	bool bDiffuseAndOpacityCompatible = false;
	if (m_pMaps[KMapTypeDiffuse] != NULL && m_pMaps[KMapTypeOpacity] != NULL)
	{
		bDiffuseAndOpacityCompatible = m_pMaps[KMapTypeDiffuse]->IsCompatible( *m_pMaps[KMapTypeOpacity] );
	}

	//Check the caches to see if textures have already been created for these
	//paths.
	if (bDiffuseAndOpacityCompatible)
	{
		CTexture::CombinedCache::iterator CombinedIt = CTexture::gm_CombinedCache.find( Paths );
		if (CombinedIt != CTexture::gm_CombinedCache.end())
		{
			m_pMaps[KMapTypeOpacity] = NULL;
			m_pMaps[KMapTypeDiffuse]->SetTexture( CombinedIt->second );
			return;
		}
	}

	HImage pDiffuseImage;
	if (!Paths.Diffuse.empty())
	{
		m_pMaps[KMapTypeDiffuse]->SetTexture( CTexture::Load( Paths.Diffuse, pDevice, false, &pDiffuseImage ) );
	}

	HImage pOpacityImage;
	if (!Paths.Opacity.empty())
	{
		m_pMaps[KMapTypeOpacity]->SetTexture( CTexture::Load( Paths.Opacity, pDevice, true, &pOpacityImage ) );
	}

	//If the diffuse and opacity maps and images are compatible,
	//we can multiply the opacity alpha into the diffuse map texture
	//and discard the entire opacity map.
	//TODO support scaling the smaller image up by an integral multiple
	if (bDiffuseAndOpacityCompatible &&
		pDiffuseImage != NULL && pOpacityImage != NULL &&
		pDiffuseImage->GetWidth() == pOpacityImage->GetWidth() &&
		pDiffuseImage->GetHeight() == pOpacityImage->GetHeight() &&
		pDiffuseImage->GetDepth() == pOpacityImage->GetDepth())
	{
		HImage pNewImage;

		if (m_pMaps[KMapTypeOpacity]->GetUseLuminanceAsAlpha())
		{
			pNewImage = ImageCombineRGBLuminance( pDiffuseImage, pOpacityImage );
		} else {
			pNewImage = ImageCombineRGBAlpha( pDiffuseImage, pOpacityImage );
		}

		LPDIRECT3DBASETEXTURE9 pTexture = pNewImage->CreateD3DTexture( pDevice, D3DUSAGE_AUTOGENMIPMAP,
				D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, 0 );

		HSimpleTexture hSimpleTexture = new CSimpleTexture();
		hSimpleTexture->SetD3DTexture( pTexture );
		if (pTexture != NULL)
		{
			pTexture->Release();
		}

		CTexture::gm_CombinedCache[Paths] = hSimpleTexture;

		m_pMaps[KMapTypeOpacity] = NULL;
		m_pMaps[KMapTypeDiffuse]->SetTexture( hSimpleTexture );
	}

	{
		std::string SelfIlluminationPath;
		if (m_pMaps[KMapTypeSelfIllumination] != NULL && !m_pMaps[KMapTypeSelfIllumination]->GetTexturePath().empty())
		{
			SelfIlluminationPath = StringLower( FindResourceFile( sContext, m_pMaps[KMapTypeSelfIllumination]->GetTexturePath(), "TexturePaths" ) );
			m_pMaps[KMapTypeSelfIllumination]->SetTexture( CTexture::Load( SelfIlluminationPath, pDevice ) );
		}
	}
}

uint32 WTCall CStandardMaterial::GetClassFormatVersion() {return 4;}

}
}
