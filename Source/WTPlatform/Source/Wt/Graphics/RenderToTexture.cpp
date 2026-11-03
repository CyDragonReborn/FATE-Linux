/*
	RenderToTexture.cpp

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	12/15/2005
*/

#include "WT/stdafx.h"

#include "WT/Graphics/RenderToTexture.h"

using namespace WT::Math;

namespace WT
{
namespace Graphics
{

CRenderToTexture::CRenderToTexture( LPDIRECT3DDEVICE9 pDevice, int32 Width, int32 Height, D3DCOLOR BackgroundColor,
								   D3DFORMAT TextureFormat, DWORD Usage, D3DFORMAT DepthStencilFormat )
	: m_pDevice( pDevice )
	, m_hTexture( NULL )
	, m_hSurface( NULL )
	, m_hDepthStencilSurface( NULL )
	, m_Width( Width )
	, m_Height( Height )
	, m_BackgroundColor( BackgroundColor )
	, m_TextureFormat( TextureFormat )
	, m_Usage( Usage )
	, m_DepthStencilFormat( DepthStencilFormat )
	, m_ResourcesValid( false )
	, m_Enabled( true )
	, m_InUse( false )
{
	if (m_Height == -1)
	{
		m_Height = m_Width;
	}

	if (m_Width < 0 || m_Height < 0)
	{
		throw std::invalid_argument( "render-to-texture dimensions" );
	}

	if (m_pDevice == NULL)
	{
		throw std::invalid_argument( "NULL device pointer" );
	}

	InitDeviceObjects( m_pDevice );

	m_Camera.SetRect( CRect2i( 0, 0, m_Width, m_Height ) );
}

/*virtual*/ CRenderToTexture::~CRenderToTexture()
{
	SafeRelease( m_hDepthStencilSurface );
	SafeRelease( m_hSurface );
	SafeRelease( m_hTexture );
}

/*virtual*/ HRenderContext CRenderToTexture::CreateRenderContext() const
{
	return new CRenderContext( &m_Camera );
}

class CDeviceRenderTargetGuard
{
public:
	CDeviceRenderTargetGuard( LPDIRECT3DDEVICE9 pDevice )
		: m_pDevice( pDevice )
	{
		assert( m_pDevice != NULL );
		m_pDevice->GetRenderTarget( 0, &m_hOrigRenderTarget );
	}

	~CDeviceRenderTargetGuard()
	{
		m_pDevice->SetRenderTarget( 0, m_hOrigRenderTarget );
		SafeRelease( m_hOrigRenderTarget );
	}

private:
	LPDIRECT3DDEVICE9 m_pDevice;
	LPDIRECT3DSURFACE9 m_hOrigRenderTarget;
};

class CDeviceDepthStencilSurfaceGuard
{
public:
	CDeviceDepthStencilSurfaceGuard( LPDIRECT3DDEVICE9 pDevice )
		: m_pDevice( pDevice )
	{
		assert( m_pDevice != NULL );
		m_pDevice->GetDepthStencilSurface( &m_hOrigDepthStencilSurface );
	}

	~CDeviceDepthStencilSurfaceGuard()
	{
		m_pDevice->SetDepthStencilSurface( m_hOrigDepthStencilSurface );
		SafeRelease( m_hOrigDepthStencilSurface );
	}

private:
	LPDIRECT3DDEVICE9 m_pDevice;
	LPDIRECT3DSURFACE9 m_hOrigDepthStencilSurface;
};

/*virtual*/ void CRenderToTexture::Update()
{
	if (!m_Enabled)
	{
		return;
	}

	UpdateInternal();
}

void CRenderToTexture::UpdateInternal()
{
	assert( !m_InUse );
	m_InUse = true;

	CDeviceRenderTargetGuard DeviceRenderTargetGuard( m_pDevice );
	CDeviceDepthStencilSurfaceGuard DeviceDepthStencilSurfaceGuard( m_pDevice );

	m_Camera.PrepareRender( m_pDevice, EPrepareForRenderFlags::All );

	m_pDevice->SetDepthStencilSurface( m_hDepthStencilSurface );

	m_pDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_BackgroundColor, 1.0f, 0 );
	m_pDevice->BeginScene();

	//TODO for CMirror, fill in Z buffer in areas outside of the object of interest - this
	//may help speed up rendering.
	OnUpdate()( *this );

	m_pDevice->EndScene();

	m_InUse = false;
}

void CRenderToTexture::SetColor( D3DCOLOR Color )
{
	assert( !m_InUse );
	m_InUse = true;

	CDeviceRenderTargetGuard DeviceRenderTargetGuard( m_pDevice );

	m_pDevice->SetRenderTarget( 0, m_hSurface );
	m_pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, Color, 1.0f, 0 );

	m_InUse = false;
}

void CRenderToTexture::InvalidateDeviceObjects()
{
	m_ResourcesValid = false;

	m_Camera.SetRenderTarget( m_pDevice, NULL );
	SafeRelease( m_hDepthStencilSurface );
	SafeRelease( m_hSurface );
	SafeRelease( m_hTexture );
	m_Camera.InvalidateDeviceObjects();
}

void CRenderToTexture::RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	assert( m_pDevice == pDevice );

	if (m_ResourcesValid)
	{
		return;
	}

	InitDeviceObjects( pDevice );

	m_ResourcesValid = true;
}

bool CRenderToTexture::IsValid() const
{
	return m_ResourcesValid;
}

void CRenderToTexture::InitDeviceObjects( LPDIRECT3DDEVICE9 pDevice )
{
	assert( m_pDevice == pDevice );

	assert( m_hTexture == NULL );
	pDevice->CreateTexture( m_Width, m_Height, 1, m_Usage, 
	                        m_TextureFormat, D3DPOOL_DEFAULT, &m_hTexture, NULL );
	if (m_hTexture == NULL)
	{
		throw std::runtime_error( "CreateTexture failure" );
	}

	assert( m_hSurface == NULL );
	m_hTexture->GetSurfaceLevel( 0, &m_hSurface );
	if (m_hTexture == NULL)
	{
		throw std::runtime_error( "GetSurfaceLevel failure" );
	}
	m_Camera.SetRenderTarget( pDevice, m_hSurface );

	assert( m_hDepthStencilSurface == NULL );
	pDevice->CreateDepthStencilSurface( m_Width, m_Height, m_DepthStencilFormat,
	                                    D3DMULTISAMPLE_NONE, 0, TRUE, &m_hDepthStencilSurface, NULL );
	if (m_hDepthStencilSurface == NULL)
	{
		throw std::runtime_error( "CreateDepthStencilSurface failure" );
	}

	m_ResourcesValid = true;
}

void CRenderToTexture::SetTextureToDevice( CRenderContext const& Context, DWORD Stage ) const
{
	assert( m_pDevice != NULL );

	assert( m_hTexture != NULL );
	LPDIRECT3DTEXTURE9 pTexture = m_hTexture;

	//If we're rendering to this texture, then do not render with this
	//texture at the same time.
	if (m_InUse)
	{
		pTexture = NULL;
	}

	Context.SetTexture( m_pDevice, Stage, pTexture );
}

Math::CMatrix4f CRenderToTexture::GetScreenToTexture() const
{
	CMatrix4f Result;
	Result.RowX.x = 0.5f;
	Result.RowY.y = -0.5f;
	Result.RowZ.z = 0.0f;
	Result.RowT.x = 0.5f + 0.5f / m_Width; //Texel adjustment
	Result.RowT.y = 0.5f + 0.5f / m_Height; //Texel adjustment
	Result.RowT.z = 1.0f;
	return Result;
}

} //Graphics
} //WT
