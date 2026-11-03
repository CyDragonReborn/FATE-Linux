/***************************************************************************************
 *
 * DxMaterial.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          10/14/2005
 * Notes:         
 *
 ***************************************************************************************/

#include "WT/stdafx.h"

#include "WT/Math/MathSerialization.h"
#include "WT/System/IO/IOException.h"
#include "WT/Image/Image.h"
#include "WT/Graphics/DxMaterial.h"
#include "WT/Graphics/Graphics.h"

#define USE_INITIAL_TECHNIQUE

#if 0 //This causes problems with creating shaders
#ifndef NDEBUG
#define DEBUG_VS
#define DEBUG_PS
#endif
#endif

using namespace WT::Math;
using namespace WT::System;
using namespace WT::System::Serialization;

namespace WT
{

namespace Graphics
{

WT_DEFINE_CLASS( CDxMaterial )

void CDxMaterial::SetEffectTextureParameters( LPDIRECT3DDEVICE9 pDevice ) const
{
#if 0 //Unnecessary?
	for (std::vector<Texture>::const_iterator it = m_Textures.begin(); it != m_Textures.end(); ++it)
	{
		int32 SamplerIndex = 0; //TBD
		assert( it->TexCoordIndex >= 0 );
		pDevice->SetTexture( SamplerIndex, it->hTexture == NULL ? NULL : it->hTexture->GetD3DTexture() );
		pDevice->SetTextureStageState( SamplerIndex, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | it->TexCoordIndex );
		pDevice->SetTextureStageState( SamplerIndex, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	}
#endif
}

void CDxMaterial::SetEffectTransformParameters( LPDIRECT3DDEVICE9 pDevice ) const
{
	assert( pDevice != NULL );
	assert( m_pEffect != NULL );

	CMatrix4f World( false );
	CMatrix4f View( false );
	CMatrix4f Projection( false );
	CMatrix4f WorldView( false );

	if (m_NeedWorldMatrix)
	{
		pDevice->GetTransform( D3DTS_WORLD, World.GetD3DMatrixPtr() );

		if (m_hEffectParameters[KParameterTypeWorld] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorld], World.GetTranspose4x4().GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeWorldT] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldT], World.GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeWorldI] != NULL || m_hEffectParameters[KParameterTypeWorldIT] != NULL)
		{
			CMatrix4f WorldI( false );
			World.GetInverse4x4( &WorldI );

			if (m_hEffectParameters[KParameterTypeWorldI] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldI], WorldI.GetTranspose4x4().GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeWorldIT] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldIT], WorldI.GetD3DXMatrixPtr() );
			}
		}
	}

	if (m_NeedViewMatrix)
	{
		pDevice->GetTransform( D3DTS_VIEW, View.GetD3DMatrixPtr() );

		if (m_hEffectParameters[KParameterTypeView] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeView], View.GetTranspose4x4().GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeViewT] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewT], View.GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeViewI] != NULL || m_hEffectParameters[KParameterTypeViewIT] != NULL)
		{
			CMatrix4f ViewI( false );
			View.GetInverse4x4( &ViewI );

			if (m_hEffectParameters[KParameterTypeViewI] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewI], ViewI.GetTranspose4x4().GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeViewIT] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewIT], ViewI.GetD3DXMatrixPtr() );
			}
		}

		if (m_hEffectParameters[KParameterTypeWorldCameraPosition] != NULL)
		{
			D3DXVECTOR4 WorldCameraPosition( -View.RowT.x, -View.RowT.y, -View.RowT.z, 1.0f );
			m_pEffect->SetVector( m_hEffectParameters[KParameterTypeWorldCameraPosition], &WorldCameraPosition );
		}
	}

	if (m_NeedProjectionMatrix)
	{
		pDevice->GetTransform( D3DTS_PROJECTION, Projection.GetD3DMatrixPtr() );

		if (m_hEffectParameters[KParameterTypeProjection] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeProjection], Projection.GetTranspose4x4().GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeProjectionT] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeProjectionT], Projection.GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeProjectionI] != NULL || m_hEffectParameters[KParameterTypeProjectionIT] != NULL)
		{
			CMatrix4f ProjectionI( false );
			Projection.GetInverse4x4( &ProjectionI );

			if (m_hEffectParameters[KParameterTypeProjectionI] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeProjectionI], ProjectionI.GetTranspose4x4().GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeProjectionIT] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeProjectionIT], ProjectionI.GetD3DXMatrixPtr() );
			}
		}
	}

	if (m_NeedWorldViewMatrix)
	{
		WorldView = World * View;

		if (m_hEffectParameters[KParameterTypeWorldView] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldView], WorldView.GetTranspose4x4().GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeWorldViewT] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewT], WorldView.GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeWorldViewI] != NULL || m_hEffectParameters[KParameterTypeWorldViewIT] != NULL)
		{
			CMatrix4f WorldViewI( false );
			WorldView.GetInverse4x4( &WorldViewI );

			if (m_hEffectParameters[KParameterTypeWorldViewI] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewI], WorldViewI.GetTranspose4x4().GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeWorldViewIT] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewIT], WorldViewI.GetD3DXMatrixPtr() );
			}
		}

		if (m_NeedWorldViewProjectionMatrix)
		{
			CMatrix4f WorldViewProjection( WorldView * Projection );

			if (m_hEffectParameters[KParameterTypeWorldViewProjection] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewProjection], WorldViewProjection.GetTranspose4x4().GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeWorldViewProjectionT] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewProjectionT], WorldViewProjection.GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeWorldViewProjectionI] != NULL || m_hEffectParameters[KParameterTypeWorldViewProjectionIT] != NULL)
			{
				CMatrix4f WorldViewProjectionI( false );
				WorldViewProjection.GetInverse4x4( &WorldViewProjectionI );

				if (m_hEffectParameters[KParameterTypeWorldViewProjectionI] != NULL)
				{
					m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewProjectionI], WorldViewProjectionI.GetTranspose4x4().GetD3DXMatrixPtr() );
				}
				if (m_hEffectParameters[KParameterTypeWorldViewProjectionIT] != NULL)
				{
					m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeWorldViewProjectionIT], WorldViewProjectionI.GetD3DXMatrixPtr() );
				}
			}
		}
	}

	if (m_NeedViewProjectionMatrix)
	{
		CMatrix4f ViewProjection = View * Projection;

		if (m_hEffectParameters[KParameterTypeViewProjection] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewProjection], ViewProjection.GetTranspose4x4().GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeViewProjectionT] != NULL)
		{
			m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewProjectionT], ViewProjection.GetD3DXMatrixPtr() );
		}
		if (m_hEffectParameters[KParameterTypeViewProjectionI] != NULL || m_hEffectParameters[KParameterTypeViewProjectionIT] != NULL)
		{
			CMatrix4f ViewProjectionI( false );
			ViewProjection.GetInverse4x4( &ViewProjectionI );

			if (m_hEffectParameters[KParameterTypeViewProjectionI] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewProjectionI], ViewProjectionI.GetTranspose4x4().GetD3DXMatrixPtr() );
			}
			if (m_hEffectParameters[KParameterTypeViewProjectionIT] != NULL)
			{
				m_pEffect->SetMatrixTranspose( m_hEffectParameters[KParameterTypeViewProjectionIT], ViewProjectionI.GetD3DXMatrixPtr() );
			}
		}
	}
}

void CDxMaterial::SetEffectTimeParameters( LPDIRECT3DDEVICE9 pDevice ) const
{
	assert( pDevice != NULL );
	assert( m_pEffect != NULL );

	if (m_hEffectParameters[KParameterTypeTime] != NULL)
	{
		m_pEffect->SetFloat( m_hEffectParameters[KParameterTypeTime], 
			static_cast<float32>( gSimulationTimeSeconds ) );
	}
}

/*virtual*/ void CDxMaterial::SetEffectParameters( LPDIRECT3DDEVICE9 pDevice ) const
{
	SetEffectTextureParameters( pDevice );
	SetEffectTransformParameters( pDevice );
	SetEffectTimeParameters( pDevice );
}

/*virtual*/ void CDxMaterial::Begin( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32& PassCount ) const
{
	assert( pDevice != NULL );
	
	if (m_pEffect != NULL)
	{
		SetEffectParameters( pDevice );

		UINT Passes;
		m_pEffect->Begin( &Passes, 0 );
		PassCount = Passes;
	} else if (m_hSoftwareMaterial != NULL) {
		m_hSoftwareMaterial->Begin( pDevice, Context, PassCount );
	} else {
		PassCount = 0;
	}
}

/*virtual*/ void CDxMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	assert( pDevice != NULL );

	if (m_pEffect != NULL)
	{
		m_pEffect->BeginPass( PassIndex );
	} else if (m_hSoftwareMaterial != NULL) {
		m_hSoftwareMaterial->BeginPass( pDevice, Context, PassIndex );
	} else {
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}
}

/*virtual*/ void CDxMaterial::EndPass() const
{
	if (m_pEffect != NULL)
	{
		m_pEffect->EndPass();
	} else if (m_hSoftwareMaterial != NULL) {
		m_hSoftwareMaterial->EndPass();
	}
}

/*virtual*/ void CDxMaterial::End() const
{
	if (m_pEffect != NULL)
	{
		m_pEffect->End();
	} else if (m_hSoftwareMaterial != NULL) {
		m_hSoftwareMaterial->End();
	}
}

void CDxMaterial::ClearParameters()
{
	std::fill( m_hEffectParameters, m_hEffectParameters + KParameterTypeCount, static_cast<D3DXHANDLE>( NULL ) );
	m_NeedWorldMatrix					= false;
	m_NeedViewMatrix					= false;
	m_NeedProjectionMatrix				= false;
	m_NeedWorldViewMatrix				= false;
	m_NeedViewProjectionMatrix			= false;
	m_NeedWorldViewProjectionMatrix		= false;
}

void CDxMaterial::LoadEffectFromFile( LPDIRECT3DDEVICE9 pDevice )
{
	ClearParameters();
	SafeRelease( m_pEffect );

	DWORD Flags = 0;

#ifdef DEBUG_VS
	Flags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	Flags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	LPD3DXBUFFER pCompilationErrors = NULL;
	HRESULT hResult = D3DXCreateEffectFromFile( pDevice, m_EffectPath.c_str(),
		NULL, NULL, Flags, NULL, &m_pEffect, &pCompilationErrors );

	if (pCompilationErrors != NULL)
	{
#ifdef WTLOG_ENABLED
		char* pBuffer = static_cast<char*>( pCompilationErrors->GetBufferPointer() );
		DWORD BufferSize = pCompilationErrors->GetBufferSize();
		std::string CompilationErrors;
		CompilationErrors.resize( BufferSize );
		std::copy( pBuffer, pBuffer + BufferSize, &CompilationErrors[0] );

		LOG_RED( "Compilation results for '%s':\n%s", m_EffectPath.c_str(), CompilationErrors.c_str() );
#endif

		pCompilationErrors->Release();
	}

	if (FAILED( hResult ))
	{
		WT_THROW( System::CRuntimeException, 
			std::string( "D3DXCreateEffectFromFile failure for '" ) + m_EffectPath
			+ '\'' );
	}

	if (m_Technique == -1)
	{	
#ifdef USE_INITIAL_TECHNIQUE
		D3DXHANDLE hCurTechnique = m_pEffect->GetCurrentTechnique();
#else
		//Some sources suggest finding and setting the technique,
		//instead of using the initially selected technique.
		D3DXHANDLE hCurTechnique;
		HRESULT Result = m_pEffect->FindNextValidTechnique( NULL, &hCurTechnique );
		if (FAILED( Result ))
		{
			WT_THROW( System::CRuntimeException, "FindNextValidTechnique failure" );
		}
#endif

		//Get the index of the technique handle by performing a linear search.
		if (hCurTechnique != NULL)
		{
			//Find the index of the technique that was selected
			for (m_Technique = 0; ; ++m_Technique)
			{
				D3DXHANDLE hTechnique = m_pEffect->GetTechnique( m_Technique );

				if (hTechnique == NULL)
				{
					//No more techniques left
					m_Technique = -1;
					break;
				}

				if (hCurTechnique == hTechnique)
				{
					break;
				}
			}
		}
	}
#ifdef USE_INITIAL_TECHNIQUE
	else
	{
		SetTechnique( m_Technique );
	}
#else
	SetTechnique( m_Technique );
#endif
	
	//Get effect global parameters based on known semantics

	//Try 3ds max 6 semantics (http://sparks.discreet.com/knowledgebase/techdocs/searchable/techdoc_DXMaterialFormat/DxMaterial_Effect_format.htm)
	m_hEffectParameters[KParameterTypeTime                  ] = m_pEffect->GetParameterBySemantic( NULL, "TIME" );
	m_hEffectParameters[KParameterTypeWorld					] = m_pEffect->GetParameterBySemantic( NULL, "WORLD" );
	m_hEffectParameters[KParameterTypeWorldI				] = m_pEffect->GetParameterBySemantic( NULL, "WORLDI" );
	m_hEffectParameters[KParameterTypeWorldIT				] = m_pEffect->GetParameterBySemantic( NULL, "WORLDIT" );
	m_hEffectParameters[KParameterTypeView					] = m_pEffect->GetParameterBySemantic( NULL, "VIEW" );
	m_hEffectParameters[KParameterTypeViewI					] = m_pEffect->GetParameterBySemantic( NULL, "VIEWI" );
	m_hEffectParameters[KParameterTypeViewIT				] = m_pEffect->GetParameterBySemantic( NULL, "VIEWIT" );
	m_hEffectParameters[KParameterTypeProjection			] = m_pEffect->GetParameterBySemantic( NULL, "PROJECTION" );
	m_hEffectParameters[KParameterTypeProjectionI			] = m_pEffect->GetParameterBySemantic( NULL, "PROJECTIONI" );
	m_hEffectParameters[KParameterTypeProjectionIT			] = m_pEffect->GetParameterBySemantic( NULL, "PROJECTIONIT" );
	m_hEffectParameters[KParameterTypeWorldView				] = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEW" );
	m_hEffectParameters[KParameterTypeWorldViewI			] = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWI" );
	m_hEffectParameters[KParameterTypeWorldViewIT			] = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWIT" );
	m_hEffectParameters[KParameterTypeWorldViewProjection	] = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJ" );
	m_hEffectParameters[KParameterTypeWorldViewProjectionI	] = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJI" );
	m_hEffectParameters[KParameterTypeWorldViewProjectionIT	] = m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJIT" );
	m_hEffectParameters[KParameterTypeWorldCameraPosition	] = m_pEffect->GetParameterBySemantic( NULL, "WORLD_CAMERA_POSITION" );
	if (m_hEffectParameters[KParameterTypeWorldCameraPosition] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldCameraPosition] = m_pEffect->GetParameterBySemantic( NULL, "WORLDCAMERAPOSITION" );
	}

	//Try additional NVIDIA semantics (i.e. C:\Program Files\NVIDIA Corporation\NVIDIA FX Composer\SDK\inc\nv_sys)
	//Some Max 8 sample effects seem to use these instead of Max's own standard.
	m_hEffectParameters[KParameterTypeWorldT]						= m_pEffect->GetParameterBySemantic( NULL, "WORLDTRANSPOSE" );
	if (m_hEffectParameters[KParameterTypeWorldI] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldI]					= m_pEffect->GetParameterBySemantic( NULL, "WORLDINVERSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldIT] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldIT]					= m_pEffect->GetParameterBySemantic( NULL, "WORLDINVERSETRANSPOSE" );
	}
	m_hEffectParameters[KParameterTypeViewT]					= m_pEffect->GetParameterBySemantic( NULL, "VIEWTRANSPOSE" );
	if (m_hEffectParameters[KParameterTypeViewI] == NULL)
	{
		m_hEffectParameters[KParameterTypeViewI]					= m_pEffect->GetParameterBySemantic( NULL, "VIEWINVERSE" );
	}
	if (m_hEffectParameters[KParameterTypeViewIT] == NULL)
	{
		m_hEffectParameters[KParameterTypeViewIT]					= m_pEffect->GetParameterBySemantic( NULL, "VIEWINVERSETRANSPOSE" );
	}
	m_hEffectParameters[KParameterTypeProjectionT]					= m_pEffect->GetParameterBySemantic( NULL, "PROJECTIONTRANSPOSE" );
	if (m_hEffectParameters[KParameterTypeProjectionI] == NULL)
	{
		m_hEffectParameters[KParameterTypeProjectionI]				= m_pEffect->GetParameterBySemantic( NULL, "PROJECTIONINVERSE" );
	}
	if (m_hEffectParameters[KParameterTypeProjectionIT] == NULL)
	{
		m_hEffectParameters[KParameterTypeProjectionIT]			= m_pEffect->GetParameterBySemantic( NULL, "PROJECTIONINVERSETRANSPOSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldViewT] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewT]				= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWTRANSPOSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldViewI] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewI]				= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWINVERSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldViewIT] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewIT]				= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWINVERSETRANSPOSE" );
	}
	m_hEffectParameters[KParameterTypeViewProjection]				= m_pEffect->GetParameterBySemantic( NULL, "VIEWPROJECTION" );
	m_hEffectParameters[KParameterTypeViewProjectionT]				= m_pEffect->GetParameterBySemantic( NULL, "VIEWPROJECTIONTRANSPOSE" );
	m_hEffectParameters[KParameterTypeViewProjectionI]				= m_pEffect->GetParameterBySemantic( NULL, "VIEWPROJECTIONINVERSE" );
	m_hEffectParameters[KParameterTypeViewProjectionIT]			= m_pEffect->GetParameterBySemantic( NULL, "VIEWPROJECTIONINVERSETRANSPOSE" );
	if (m_hEffectParameters[KParameterTypeWorldViewProjection] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewProjection]		= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJECTION" );
	}
	if (m_hEffectParameters[KParameterTypeWorldViewProjectionT] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewProjectionT]	= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJECTIONTRANSPOSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldViewProjectionI] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewProjectionI]	= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJECTIONINVERSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldViewProjectionIT] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldViewProjectionIT]	= m_pEffect->GetParameterBySemantic( NULL, "WORLDVIEWPROJECTIONINVERSETRANSPOSE" );
	}
	if (m_hEffectParameters[KParameterTypeWorldCameraPosition] == NULL)
	{
		m_hEffectParameters[KParameterTypeWorldCameraPosition]		= m_pEffect->GetParameterBySemantic( NULL, "CAMERAPOSITION" );
	}

	m_NeedWorldViewProjectionMatrix	=
		m_hEffectParameters[KParameterTypeWorldViewProjection] != NULL ||
		m_hEffectParameters[KParameterTypeWorldViewProjectionT] != NULL ||
		m_hEffectParameters[KParameterTypeWorldViewProjectionI] != NULL ||
		m_hEffectParameters[KParameterTypeWorldViewProjectionIT] != NULL;

	m_NeedWorldViewMatrix =
		m_NeedWorldViewProjectionMatrix ||
		m_hEffectParameters[KParameterTypeWorldView] != NULL ||
		m_hEffectParameters[KParameterTypeWorldViewT] != NULL ||
		m_hEffectParameters[KParameterTypeWorldViewI] != NULL ||
		m_hEffectParameters[KParameterTypeWorldViewIT] != NULL;

	m_NeedViewProjectionMatrix =
		m_hEffectParameters[KParameterTypeViewProjection] != NULL ||
		m_hEffectParameters[KParameterTypeViewProjectionT] != NULL ||
		m_hEffectParameters[KParameterTypeViewProjectionI] != NULL ||
		m_hEffectParameters[KParameterTypeViewProjectionIT] != NULL;

	m_NeedWorldMatrix = 
		m_NeedWorldViewMatrix ||
		m_hEffectParameters[KParameterTypeWorld] != NULL ||
		m_hEffectParameters[KParameterTypeWorldT] != NULL ||
		m_hEffectParameters[KParameterTypeWorldI] != NULL ||
		m_hEffectParameters[KParameterTypeWorldIT] != NULL;

	m_NeedViewMatrix = 
		m_NeedWorldViewMatrix ||
		m_NeedViewProjectionMatrix ||
		m_hEffectParameters[KParameterTypeView] != NULL ||
		m_hEffectParameters[KParameterTypeViewT] != NULL ||
		m_hEffectParameters[KParameterTypeViewI] != NULL ||
		m_hEffectParameters[KParameterTypeViewIT] != NULL ||
		m_hEffectParameters[KParameterTypeWorldCameraPosition] != NULL;

	m_NeedProjectionMatrix = 
		m_NeedWorldViewProjectionMatrix	||
		m_NeedViewProjectionMatrix ||
		m_hEffectParameters[KParameterTypeProjection] != NULL ||
		m_hEffectParameters[KParameterTypeProjectionT] != NULL ||
		m_hEffectParameters[KParameterTypeProjectionI] != NULL ||
		m_hEffectParameters[KParameterTypeProjectionIT] != NULL;
}

LPD3DXEFFECT CDxMaterial::GetEffect()
{
	return m_pEffect;
}

D3DXHANDLE CDxMaterial::GetEffectParameter( EParameterType ParameterType )
{
	if (ParameterType < 0 || ParameterType >= KParameterTypeCount)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "ParameterType" );
	}
	
	return m_hEffectParameters[ParameterType];
}

void CDxMaterial::SetEffectParameter( EParameterType ParameterType, D3DXHANDLE hParameter )
{
	if (ParameterType < 0 || ParameterType >= KParameterTypeCount)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "ParameterType" );
	}
	
	m_hEffectParameters[ParameterType] = hParameter;
}

std::vector<CDxMaterial::Texture>& CDxMaterial::GetTextures()
{
	return m_Textures;
}

std::vector<CDxMaterial::Texture> const& CDxMaterial::GetTextures() const
{
	return m_Textures;
}

CDxMaterial::CDxMaterial()
	: m_hParameters( new System::CUserPropertyCollection() )
	, m_pEffect( NULL )
	, m_Technique( -1 )
	, m_ResourcesValid( false )
{
	ClearParameters();
}

CDxMaterial::~CDxMaterial( )
{
	SafeRelease( m_pEffect );
}

int32 CDxMaterial::GetTechnique() const
{
	return m_Technique;
}

void CDxMaterial::SetTechnique( int32 Technique )
{
	if (m_pEffect != NULL && Technique != -1)
	{
		D3DXHANDLE hTechnique = m_pEffect->GetTechnique( m_Technique );
		if (hTechnique == NULL)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "Technique" );
		}

		HRESULT hResult = m_pEffect->SetTechnique( hTechnique );
		if (FAILED( hResult ))
		{
			WT_THROW( System::CRuntimeException, "SetTechnique failure" );
		}
	}
	m_Technique = Technique;
}

/*virtual*/ void CDxMaterial::GetObjectData(CSerializationInfo* Info, CSerializationContext* sContext) const
{
	Info->AddValue( "Name", m_Name );
	Info->AddValue( "EffectPath", m_EffectPath );
	Info->AddValue( "Technique", m_Technique );
	Info->AddValue( "SoftwareMaterial", m_hSoftwareMaterial.get() );
	Info->AddValue( "Parameters", m_hParameters.get() );
}

/*virtual*/ void CDxMaterial::SetObjectData( CSerializationInfo* Info, CSerializationContext* sContext )
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

	if (Info->GetObjectFormatVersion() < 2)
	{
		WT_THROW( System::Serialization::CSerializationFormatVersionError, "Unsupported version" );
	}

	Info->GetValue( "Name", m_Name );
	Info->GetValue( "EffectPath", m_EffectPath );
	Info->GetValue( "Technique", m_Technique );

	if (!m_EffectPath.empty())
	{
		std::string EffectPath = FindResourceFile( sContext, m_EffectPath, "EffectPaths" );
		if (!EffectPath.empty())
		{
			m_EffectPath = EffectPath;
			LoadEffectFromFile( pDevice );
		}
	}

	CMaterial* pSoftwareMaterial; 
	Info->GetValue( "SoftwareMaterial", pSoftwareMaterial ); 
	m_hSoftwareMaterial = pSoftwareMaterial;

	CUserPropertyCollection* pParameters; 
	Info->GetValue( "Parameters", pParameters ); 
	if (pParameters == NULL)
	{
		WT_THROW( System::CRuntimeException, "Parameters" );
	}
	m_hParameters = pParameters;
}

/*virtual*/ void CDxMaterial::FinalizeDeserialization( CSerializationInfo* SerialInfo,
									   CSerializationContext* sContext )
{
	//_BaseClass::FinalizeDeserialization( SerialInfo, sContext );

	LPDIRECT3D9 pD3D = reinterpret_cast<LPDIRECT3D9>( sContext->GetProperties()->GetValue<uint64>( "D3D" ) );
	assert( pD3D != NULL );

	LPDIRECT3DDEVICE9 pDevice = reinterpret_cast<LPDIRECT3DDEVICE9>( sContext->GetProperties()->GetValue<uint64>( "D3DDevice" ) );
	assert( pDevice != NULL );

	SerialInfo;
	sContext;
	pD3D;
	pDevice;

	//Set effect parameters based on properties
	CPropertyCollection::PropertyMap const& Props = m_hParameters->GetProperties();
	for (CPropertyCollection::PropertyMap::const_iterator it = Props.begin(); it != Props.end(); ++it)
	{
		std::string const& Name = it->first;
		CProperty const* const pProp = it->second;
		
		if (m_pEffect != NULL)
		{
			D3DXHANDLE hParam = m_pEffect->GetParameterByName( NULL, Name.c_str() );
			if (hParam == NULL)
			{
#ifdef WTLOG_ENABLED
				if (Name.size() >= 10 && std::string( "mapChannel" ) == Name.c_str() + (Name.size() - 10))
				{
					//This is a map channel parameter, which 3ds max uses to provide supplementary information
					//for bitmap parameters.
				} else {
					LOG_RED( "Parameter '%s' not found in effect '%s'.", Name.c_str(), m_EffectPath.c_str() );
				}
#endif
			} else {

				for (;;)
				{
					CExplicitTypedProperty<CVector4f> const* pVector4fProp = DynamicCast< CExplicitTypedProperty<CVector4f> >( pProp );
					if (pVector4fProp != NULL)
					{
						m_pEffect->SetVector( hParam, pVector4fProp->Get().GetD3DXVector4Ptr() );
						break;
					}

					CExplicitTypedProperty<bool> const* pBoolProp = DynamicCast< CExplicitTypedProperty<bool> >( pProp );
					if (pBoolProp != NULL)
					{
						m_pEffect->SetBool( hParam, pBoolProp->Get() );
						break;
					}

					CExplicitTypedProperty<int32> const* pIntProp = DynamicCast< CExplicitTypedProperty<int32> >( pProp );
					if (pIntProp != NULL)
					{
						m_pEffect->SetInt( hParam, pIntProp->Get() );
						break;
					}

					CExplicitTypedProperty<float32> const* pFloatProp = DynamicCast< CExplicitTypedProperty<float32> >( pProp );
					if (pFloatProp != NULL)
					{
						m_pEffect->SetFloat( hParam, pFloatProp->Get() );
						break;
					}

					CExplicitTypedProperty<string> const* pStringProp = DynamicCast< CExplicitTypedProperty<string> >( pProp );
					std::string const& String = pStringProp->Get();

					if (pStringProp != NULL)
					{
						//Check if the parameter is a texture parameter
						D3DXPARAMETER_DESC Desc;
						m_pEffect->GetParameterDesc( hParam, &Desc );
						if (Desc.Type >= D3DXPT_TEXTURE && Desc.Type <= D3DXPT_TEXTURECUBE)
						{
							HTexture hTexture = CTexture::Load( String, pDevice );
							if (hTexture != NULL)
							{
								m_pEffect->SetTexture( hParam, hTexture->GetD3DTexture() );

								//Check if the string is a texture filename; if so, look for a corresponding
								//mapChannel property (created by 3ds max) and then load the file.
								int32 Channel = 1;
								for (;;)
								{
									//Look for a "mapChannel" property.
									std::string ChannelPropName = Name + "mapChannel";
									CProperty* pChannelProp = m_hParameters->GetItem( ChannelPropName );
									if (pChannelProp == NULL)
									{
										LOG_RED( "Supporting property '%s' not found for texture property '%s'.", ChannelPropName.c_str(), Name.c_str() );
										break;
									}

									CExplicitTypedProperty<int32> const* pIntProp = DynamicCast< CExplicitTypedProperty<int32> >( pChannelProp );
									if (pIntProp == NULL)
									{
										LOG_RED( "Invalid type for map channel property '%s'.", ChannelPropName.c_str() );
										break;
									}

									Channel = pIntProp->Get();
									if (Channel < 1)
									{
										LOG_RED( "Invalid value %d for map channel property '%s'.", Channel, ChannelPropName.c_str() );
										Channel = 1;
									}
									break;
								}

								Texture Texture_;
								Texture_.hTexture = hTexture;
								Texture_.TexCoordIndex = Channel - 1;
								m_Textures.push_back( Texture_ );
							}
						} else {
							m_pEffect->SetString( hParam, String.c_str() );
						}

						break;
					}

					LOG_RED( "Unexpected type for property '%s' for material '%s'.", Name.c_str(), m_Name.c_str() );
					break;
				}
			}
		}
	}
}

uint32 WTCall CDxMaterial::GetClassFormatVersion() {return 2;}

void CDxMaterial::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;
	if (m_pEffect != NULL)
	{
		m_pEffect->OnLostDevice();
	}
}

void CDxMaterial::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	if (!m_ResourcesValid)
	{
		if (m_pEffect != NULL)
		{
			m_pEffect->OnResetDevice();
		}

		m_ResourcesValid = true;
	}
}

bool CDxMaterial::IsValid() const
{
	return m_ResourcesValid;
}

}
}
