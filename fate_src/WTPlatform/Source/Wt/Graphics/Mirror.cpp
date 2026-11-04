#include "WT/stdafx.h"

#include "WT/Graphics/Mirror.h"
#include "WT/System/Logging/Log.h"

using namespace WT::Math;

namespace WT
{
namespace Graphics
{

CMirror::CMirror( LPDIRECT3DDEVICE9 pDevice, int32 Resolution, D3DCOLOR BackgroundColor, Math::CPlane const& Plane )
	: CRenderToTexture( pDevice, Resolution, Resolution, BackgroundColor )
	, m_Plane( Plane )
	, m_bClipPlaneEnabled( false )
{
}

/*virtual*/ void CMirror::Update( CCamera const& RealCamera )
{
	if (!GetEnabled())
	{
		return;
	}

	UpdateInternal( RealCamera );
}

void CMirror::UpdateInternal( CCamera const& RealCamera )
{
	GetCamera().SetNearClip( RealCamera.GetNearClip() );
	GetCamera().SetFarClip( RealCamera.GetFarClip() );

	GetCamera().SetFOV( RealCamera.GetFOV() );
	GetCamera().SetVerticalFOV( RealCamera.GetVerticalFOV() );

	CMatrix4f VirtualTransform( RealCamera.GetTransform().PostReflect( m_Plane ) );
	//Flip camera about its x axis to yield a transform with positive determinant
	VirtualTransform.PreScale( -1.0f, 1.0f, 1.0f );

	GetCamera().SetTransform( VirtualTransform );

	LPDIRECT3DDEVICE9 pDevice = GetDevice();
	assert( pDevice != NULL );
	if (m_bClipPlaneEnabled)
	{
		CVector4f const Plane( m_Plane.Normal.x, m_Plane.Normal.y, m_Plane.Normal.z, m_Plane.Distance );
		pDevice->SetClipPlane( 0, Plane.e );
		pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, (1 << 0) );
	}

	_BaseClass::UpdateInternal();

	if (m_bClipPlaneEnabled)
	{
		pDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, 0 );
	}

	//Set texture transform.
	m_TextureTransform = RealCamera.GetViewToWorld() * GetWorldToTexture();
}

/*virtual*/ HRenderContext CMirror::CreateRenderContext() const
{
	return new CRenderContextMirror();
}

/*virtual*/ void CReflectionMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	assert( pDevice != NULL );

	if (PassIndex < 0 || PassIndex >= 1)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}

	m_Mirror.SetTextureToDevice( Context, 0 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );
	
	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_CAMERASPACEPOSITION );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED );

	pDevice->SetTransform( D3DTS_TEXTURE0, m_Mirror.GetTextureTransform().GetD3DMatrixPtr() );

	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
}

CBlendedReflectionMaterial::CBlendedReflectionMaterial( CMirror const& mirror, LPDIRECT3DBASETEXTURE9 pBlendTexture ) : 
	CReflectionMaterial( mirror )
{
	m_pBlendTexture = pBlendTexture;
	if (m_pBlendTexture != NULL)
	{
		m_pBlendTexture->AddRef();
	}

#ifndef NDEBUG
	LPDIRECT3DDEVICE9 pDevice = mirror.GetDevice();
	assert( pDevice != NULL );
	D3DCAPS9 Caps;
	HRESULT Result = pDevice->GetDeviceCaps( &Caps );
	assert( SUCCEEDED( Result ) );
	if (!(Caps.TextureOpCaps & D3DTEXOPCAPS_BLENDCURRENTALPHA))
	{
		LOG_RED( "Warning: this hardware cannot support D3DTOP_BLENDCURRENTALPHA." );
	}
#endif
}

/*virtual*/ void CBlendedReflectionMaterial::BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const
{
	assert( pDevice != NULL );

	if (PassIndex < 0 || PassIndex >= 1)
	{
		WT_THROW( System::CArgumentOutOfRangeException, "PassIndex" );
	}

	Context.SetTexture( pDevice, 0, m_pBlendTexture );

	pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT );

	pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 | D3DTSS_TCI_PASSTHRU );
	pDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );

	m_Mirror.SetTextureToDevice( Context, 1 );

	pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA );
	pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
	pDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
	pDevice->SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT );
	
	pDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 | D3DTSS_TCI_CAMERASPACEPOSITION );
	pDevice->SetTextureStageState( 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED );
	pDevice->SetTransform( D3DTS_TEXTURE1, m_Mirror.GetTextureTransform().GetD3DMatrixPtr() );

	pDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
}

} //Graphics
} //WT
