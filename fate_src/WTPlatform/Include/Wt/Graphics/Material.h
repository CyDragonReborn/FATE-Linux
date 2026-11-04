/***************************************************************************************
 *
 * Material.h
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

#pragma once

#include "d3d9.h"
#include "WT/Math/Math.h"
#include "WT/Image/Image.h"
#include "WT/Graphics/MaterialMap.h"

namespace WT
{

//
//Forward declarations
//
namespace Graphics
{
	class CMaterial;
}
template<> struct CCanCastPointer<Graphics::CMaterial const, System::CReferenceCounted const> {typedef TrueType Result;};


namespace Graphics
{

typedef System::SmartPointer<class CMaterial> HMaterial;
class CMaterial : public System::CObject, public IDeviceObject
{
public:
	WT_BEGIN_ABSTRACT_CLASS( CMaterial, System::CObject );
		WT_INHERITS_INTERFACE( IDeviceObject );
	WT_END_CLASS();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	virtual std::string const& GetName() const {static std::string Result; return Result;}

	virtual bool IsTwoSided() const {return false;}

	virtual CMaterialMap* GetDiffuseMap() const {return NULL;}

	virtual void Begin( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32& PassCount ) const;
	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;
	virtual void EndPass() const;
	virtual void End() const;

	//
	// Cached material methods - used to minimize redundant Begin() and End() calls calls.
	//

	// Set the cached material. If the cached material has changed, then the
	// device will be updated with the new material. If a NULL material is specified,
	// then a default, white material will be used instead.
	static void WTCall SetCachedMaterial( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, CMaterial const* pMaterial );

	// Resets the cached material and forces an update of the device.
	// ResetCachedMaterial() should be called before rendering each collection of materials,
	// or when a cached material needs to be deleted.
	static void WTCall ResetCachedMaterial( CRenderContext const& Context, LPDIRECT3DDEVICE9 pDevice );

	// Get the currently cached material.
	static CMaterial const* WTCall GetCachedMaterial() {return gm_hCachedMaterial;}

	// Get the number of passes used by the currently cached material.
	static int32 WTCall GetCachedPassCount() {return gm_CachedPassCount;}

private:
	// Most recently rendered material.
	static System::SmartPointer<CMaterial const>	gm_hCachedMaterial;
	static System::SmartPointer<CMaterial const>	gm_hDefaultMaterial;
	static System::SmartPointer<CMaterial const>	gm_hNullMaterial;
	static int32									gm_CachedPassCount;

protected:
	static D3DMATERIAL9 const gm_DefaultD3DMaterial;
};

//A material which does not change rendering settings.
typedef System::SmartPointer<class CNullMaterial> HNullMaterial;
class CNullMaterial : public CMaterial
{
public:
	WT_CLASS( CNullMaterial, CMaterial );

	virtual void Begin( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32& PassCount ) const
	{
		pDevice;
		Context;
		PassCount = 0;
	}

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
	{
		pDevice;
		Context;
		PassIndex;
		assert( pDevice != NULL );
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}
};

//The default, white material.
typedef System::SmartPointer<class CDefaultMaterial> HDefaultMaterial;
class CDefaultMaterial : public CMaterial
{
public:
	WT_CLASS( CDefaultMaterial, CMaterial );

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
	{
		assert( pDevice != NULL );

		if (PassIndex < 0 || PassIndex >= 1)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
		}

		pDevice->SetMaterial( &gm_DefaultD3DMaterial );
		Context.SetTexture( pDevice, 0, NULL );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
		pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}
};

//A material using vertex diffuse values.
typedef System::SmartPointer<class CMaterialDiffuse> HMaterialDiffuse;
class CMaterialDiffuse : public CMaterial
{
public:
	WT_CLASS( CMaterialDiffuse, CMaterial );

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
	{
		assert( pDevice != NULL );

		if (PassIndex < 0 || PassIndex >= 1)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
		}

		pDevice->SetMaterial( &gm_DefaultD3DMaterial );
		Context.SetTexture( pDevice, 0, NULL );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}
};

//A textured material.
typedef System::SmartPointer<class CMaterialTextured> HMaterialTextured;
class CMaterialTextured : public CMaterial
{
public:
	WT_CLASS( CMaterialTextured, CMaterial );

	CMaterialTextured()
		: m_pTexture( NULL )
	{
	}

	CMaterialTextured( LPDIRECT3DBASETEXTURE9 pTexture )
		: m_pTexture( pTexture )
	{
		if (m_pTexture != NULL)
		{
			m_pTexture->AddRef();
		}
	}

	CMaterialTextured( LPDIRECT3DDEVICE9 pDevice, std::string const& FileName, CImageSerializer* pSerializer = NULL )
	{
		HImage hImage = CImage::CreateImage( FileName, pSerializer );
		m_pTexture = hImage->CreateD3DTexture( pDevice );
	}

	CMaterialTextured( LPDIRECT3DDEVICE9 pDevice, std::string const& FileName, EImageFormat::Type Format )
	{
		HImage hImage = CImage::CreateImage( FileName, Format );
		m_pTexture = hImage->CreateD3DTexture( pDevice );
	}

	virtual ~CMaterialTextured()
	{
		SafeRelease( m_pTexture );
	}

	LPDIRECT3DBASETEXTURE9 GetTexture() const
	{
		return m_pTexture;
	}

	virtual void SetTexture( LPDIRECT3DBASETEXTURE9 pTexture )
	{
		if (pTexture != NULL)
		{
			pTexture->AddRef();
		}

		SafeRelease( m_pTexture );
		m_pTexture = pTexture;
	}

	bool GetTextureSize( uint32 Level, int32& Width, int32& Height ) const
	{
		Width = -1;
		Height = -1;
		if ( !m_pTexture )
			return false;

		// IDirect3DTexture9 interface needed to get width/height
		IDirect3DTexture9* pTexture = NULL;
		HRESULT hr = m_pTexture->QueryInterface(IID_IDirect3DTexture9, (void **)&pTexture);
		if ( SUCCEEDED(hr) )
		{
			D3DSURFACE_DESC TextureDesc;
			pTexture->GetLevelDesc(Level, &TextureDesc);	// Get top level texture description

			Width = TextureDesc.Width;
			Height = TextureDesc.Height;

			SafeRelease(pTexture);
			return true;
		}
		
		return false;
	}

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
	{
		assert( pDevice != NULL );

		if (PassIndex < 0 || PassIndex >= 1)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
		}

		pDevice->SetMaterial( &gm_DefaultD3DMaterial );
		Context.SetTexture( pDevice, 0, m_pTexture );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}

protected:
	LPDIRECT3DBASETEXTURE9 m_pTexture;
};

//A textured material, modulated with vertex diffuse values.
typedef System::SmartPointer<class CMaterialTexturedDiffuse> HMaterialTexturedDiffuse;
class CMaterialTexturedDiffuse : public CMaterialTextured
{
public:
	WT_CLASS( CMaterialTextured, CMaterial );

	CMaterialTexturedDiffuse() : CMaterialTextured() {}
	CMaterialTexturedDiffuse( LPDIRECT3DBASETEXTURE9 pTexture ) : CMaterialTextured( pTexture ) {}

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
	{
		assert( pDevice != NULL );

		if (PassIndex < 0 || PassIndex >= 1)
		{
			WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
		}

		pDevice->SetMaterial( &gm_DefaultD3DMaterial );
		Context.SetTexture( pDevice, 0, m_pTexture );
		pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
		pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
		pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
		pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
	}
};

}
}
