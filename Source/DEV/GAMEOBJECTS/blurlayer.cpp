/*
	blurlayer.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	6/23/2004

*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/settings.h"

#include "../MEDIA/material.h"

#include "blurlayer.h"

CBlurLayer::CBlurLayer( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device						  
					    CRefManager& pRefManager,				// reference manager
						CSettings& pSettings,					// the settings manager, for keeping track of card/machine capabilities
						const std::string& BlurMaskPath,		// path to a blur mask texture (can be "" - FrameFade will be used in stead)
						const std::string& BlurMaskPathAlpha,	// path to a blur mask texture alpha (can be "" - FrameFade will be used in stead)
						uint32 Size,							// dimensions of the texture
						float32 FrameFade ) :					// alpha amount to 'fade' the blur each frame ( invalid if using a blur mask )
												m_pRefManager( pRefManager ),
												m_pSettings( pSettings ),
												m_FrameFade( FrameFade ),
												m_pVertexBuffer( NULL ),
												m_pVertexBufferLeftHalf( NULL ),
												m_pVertexBufferRightHalf( NULL ),
		  										m_pTexture( NULL ),
												m_pBlurTexture( NULL ),
												m_TextureSize( Size ),
												m_pMaskVertexBuffer ( NULL ),
												m_ScreenWidth ( 0 ),
												m_ScreenHeight( 0 )
{

	pD3DDevice->CreateVertexBuffer( 4 * sizeof( BLURLAYERVERTEX ),
												D3DUSAGE_WRITEONLY,
												D3DFVF_BLURLAYERVERTEX,
												D3DPOOL_MANAGED,
												&m_pVertexBuffer ) ;

	pD3DDevice->CreateVertexBuffer( 4 * sizeof( BLURLAYERVERTEX ),
												D3DUSAGE_WRITEONLY,
												D3DFVF_BLURLAYERVERTEX,
												D3DPOOL_MANAGED,
												&m_pVertexBufferLeftHalf ) ;

	pD3DDevice->CreateVertexBuffer( 4 * sizeof( BLURLAYERVERTEX ),
												D3DUSAGE_WRITEONLY,
												D3DFVF_BLURLAYERVERTEX,
												D3DPOOL_MANAGED,
												&m_pVertexBufferRightHalf ) ;

	pD3DDevice->CreateVertexBuffer( 4 * sizeof( BLURLAYERVERTEX ),
												D3DUSAGE_WRITEONLY,
												D3DFVF_BLURLAYERVERTEX,
												D3DPOOL_MANAGED,
												&m_pMaskVertexBuffer ) ;

	if( BlurMaskPath.length() > 0 &&
		BlurMaskPathAlpha.length() > 0 )
	{
		m_pBlurTexture = m_pRefManager.AddMaterialAlpha( pD3DDevice,
														 BlurMaskPath,
														 BlurMaskPathAlpha );
	}

	// we're adding an EMPTY material with no texture.
	m_pTexture = m_pRefManager.AddMaterial( pD3DDevice, 
													   "",
													   "",
													   "",
													   "",
													   "",
													   kTrue );
	m_pTexture->EnableDynamicTexture( pD3DDevice, 
									  Size );

	RestoreDeviceObjects( pD3DDevice,
						  pSettings );
}


CBlurLayer::~CBlurLayer( void )
{
	m_pRefManager.RemoveMaterial( m_pTexture );
	m_pRefManager.RemoveMaterial( m_pBlurTexture );
	RELEASE_SAFELY( m_pVertexBuffer );
	RELEASE_SAFELY( m_pVertexBufferLeftHalf );
	RELEASE_SAFELY( m_pVertexBufferRightHalf );
	RELEASE_SAFELY( m_pMaskVertexBuffer );
} // CBlurLayer::~CBlurLayer()


void CBlurLayer::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
										CSettings& pSettings )			// so that we can tell the dimensions of the screen
{
	BLURLAYERVERTEX*	Vertex;
	m_ScreenWidth = (float32)pSettings.GetSettings( KSetScreenWidth );
	m_ScreenHeight = (float32)pSettings.GetSettings( KSetScreenHeight );
	
	m_pVertexBuffer->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].p = D3DXVECTOR4( 0, m_ScreenHeight, 0.0f, 1.0f );
	Vertex[1].p = D3DXVECTOR4( 0, 0, 0.0f, 1.0f );
	Vertex[2].p = D3DXVECTOR4( m_ScreenWidth, m_ScreenHeight, 0.0f, 1.0f );
	Vertex[3].p = D3DXVECTOR4( m_ScreenWidth, 0, 0.0f, 1.0f );

	Vertex[0].tu = 0;
	Vertex[0].tv = 1;

	Vertex[1].tu = 0;
	Vertex[1].tv = 0;

	Vertex[2].tu = 1;
	Vertex[2].tv = 1;

	Vertex[3].tu = 1;
	Vertex[3].tv = 0;

	Vertex[0].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[1].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[2].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[3].color = D3DXCOLOR( 0, 0, 0, 1 );
	m_pVertexBuffer->Unlock();

	m_pVertexBufferLeftHalf->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].p = D3DXVECTOR4( 0, m_ScreenHeight, 0.0f, 1.0f );
	Vertex[1].p = D3DXVECTOR4( 0, 0, 0.0f, 1.0f );
	Vertex[2].p = D3DXVECTOR4( m_ScreenWidth * .5f, m_ScreenHeight, 0.0f, 1.0f );
	Vertex[3].p = D3DXVECTOR4( m_ScreenWidth * .5f, 0, 0.0f, 1.0f );

	Vertex[0].tu = 0;
	Vertex[0].tv = 1;

	Vertex[1].tu = 0;
	Vertex[1].tv = 0;

	Vertex[2].tu = 1;
	Vertex[2].tv = 1;

	Vertex[3].tu = 1;
	Vertex[3].tv = 0;

	Vertex[0].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[1].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[2].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[3].color = D3DXCOLOR( 0, 0, 0, 1 );
	m_pVertexBufferLeftHalf->Unlock();

	m_pVertexBufferRightHalf->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].p = D3DXVECTOR4( m_ScreenWidth * .5f, m_ScreenHeight, 0.0f, 1.0f );
	Vertex[1].p = D3DXVECTOR4( m_ScreenWidth * .5f, 0, 0.0f, 1.0f );
	Vertex[2].p = D3DXVECTOR4( m_ScreenWidth, m_ScreenHeight, 0.0f, 1.0f );
	Vertex[3].p = D3DXVECTOR4( m_ScreenWidth, 0, 0.0f, 1.0f );

	Vertex[0].tu = 0;
	Vertex[0].tv = 1;

	Vertex[1].tu = 0;
	Vertex[1].tv = 0;

	Vertex[2].tu = 1;
	Vertex[2].tv = 1;

	Vertex[3].tu = 1;
	Vertex[3].tv = 0;

	Vertex[0].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[1].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[2].color = D3DXCOLOR( 0, 0, 0, 1 );
	Vertex[3].color = D3DXCOLOR( 0, 0, 0, 1 );
	m_pVertexBufferRightHalf->Unlock();

	m_pMaskVertexBuffer->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].p = D3DXVECTOR4( 0, (float32)m_TextureSize, 0.0f, 1.0f );
	Vertex[1].p = D3DXVECTOR4( 0, 0, 0.0f, 1.0f );
	Vertex[2].p = D3DXVECTOR4( (float32)m_TextureSize, (float32)m_TextureSize, 0.0f, 1.0f );
	Vertex[3].p = D3DXVECTOR4( (float32)m_TextureSize, 0, 0.0f, 1.0f );

	Vertex[0].tu = 0;
	Vertex[0].tv = 1;

	Vertex[1].tu = 0;
	Vertex[1].tv = 0;

	Vertex[2].tu = 1;
	Vertex[2].tv = 1;

	Vertex[3].tu = 1;
	Vertex[3].tv = 0;

	Vertex[0].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	Vertex[1].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	Vertex[2].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	Vertex[3].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	m_pMaskVertexBuffer->Unlock();


	if( m_pTexture->IsDynamicTextureValid() )
	{
		// we need to clear it to black to start with, and clear the zbuffer
		m_pTexture->BeginRender();

		pD3DDevice->Clear( 0,
						NULL,
						D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET,
						0x000000,
						1.0f,
						0 );

		m_pTexture->EndRender();
	}
} // CBlurLayer::RestoreDeviceObjects()

// change the color of the fill shadow
void CBlurLayer::SetFrameFade( float32 Alpha ) // alpha amount to fade each frame ( 0 is all, 1 is none )
{
	m_FrameFade = Alpha;

	BLURLAYERVERTEX*	Vertex;
	m_pMaskVertexBuffer->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	Vertex[1].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	Vertex[2].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	Vertex[3].color = D3DXCOLOR( 0, 0, 0, m_FrameFade );
	m_pMaskVertexBuffer->Unlock();
} // CBlurLayer::SetShadowColor()

void CBlurLayer::BeginRender( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{

	// projection matrix for the texture
	D3DXMATRIX ProjectionMatrix;

	float32	Aspect	= ( float32 )m_pSettings.GetSettings( KSetScreenWidth ) /
					  ( float32 )m_pSettings.GetSettings( KSetScreenHeight );

	float32 FOV		= ( float32 )( m_pSettings.GetSettings( KSetFOV ) * KDeg2Rad );


	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix,
								FOV,		// FOV degrees
								Aspect,
								(float32)m_pSettings.GetSettings( KSetNearClip ),	// near clip
								(float32)m_pSettings.GetSettings( KSetFarClip ) );	// far clip


	// start rendering to our road reflection texture
	m_pTexture->BeginRender();

	pD3DDevice->Clear( 0,
					   NULL,
					   D3DCLEAR_ZBUFFER,
					   0x000000,
					   1.0f,
					   0 );

	// first we fade out the last frame

	RenderFade( pD3DDevice );


	// set  our projection to the this projection matrix
	pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );


	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

} // void CBlurLayer::BeginRender( )
void CBlurLayer::BeginRenderHalfScreen( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{

	// projection matrix for the texture
	D3DXMATRIX ProjectionMatrix;

	float32	Aspect	= ( float32 )m_pSettings.GetSettings( KSetScreenWidth ) * .5f/
					  ( float32 )m_pSettings.GetSettings( KSetScreenHeight );

	float32 FOV		= ( float32 )( m_pSettings.GetSettings( KSetFOV ) * KDeg2Rad );


	D3DXMatrixPerspectiveFovLH( &ProjectionMatrix,
								FOV,		// FOV degrees
								Aspect,
								(float32)m_pSettings.GetSettings( KSetNearClip ),	// near clip
								(float32)m_pSettings.GetSettings( KSetFarClip ) );	// far clip


	// start rendering to our road reflection texture
	m_pTexture->BeginRender();

	pD3DDevice->Clear( 0,
					   NULL,
					   D3DCLEAR_ZBUFFER,
					   0x000000,
					   1.0f,
					   0 );

	// first we fade out the last frame

	RenderFade( pD3DDevice );


	// set  our projection to the this projection matrix
	pD3DDevice->SetTransform( D3DTS_PROJECTION, &ProjectionMatrix );


	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

} // void CBlurLayer::BeginRenderHalfScreen( )

void CBlurLayer::RenderFade( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if( m_pSettings.GetSettings( KSetScreenWidth ) != m_ScreenWidth ||
		m_pSettings.GetSettings( KSetScreenHeight ) != m_ScreenHeight )
	{
		RestoreDeviceObjects( pD3DDevice, m_pSettings );
	}
	// set up an othographic projection for rendering UI elements - we don't need depth!
	D3DXMATRIX Identity;
	D3DXMATRIX Orthographic;
	D3DXMatrixIdentity( &Identity );

	D3DXMatrixOrthoLH ( &Orthographic, 
						1024,
						768,
						1.0f,
						10.0f );

	pD3DDevice->SetTransform( D3DTS_PROJECTION, &Orthographic );
	pD3DDevice->SetTransform( D3DTS_WORLD, &Identity );
	pD3DDevice->SetTransform( D3DTS_VIEW, &Identity );


	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	if( m_pBlurTexture != NULL )
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTexture( 0, m_pBlurTexture->GetTexture() );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	}
	else
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	}


	// Draw a big alpha square to fade things out square
	pD3DDevice->SetVertexShader( D3DFVF_BLURLAYERVERTEX );
	pD3DDevice->SetStreamSource( 0, m_pMaskVertexBuffer, sizeof( BLURLAYERVERTEX ) );
	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	// Restore render states
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

} // CBlurLayer::RenderFade()

void CBlurLayer::EndRender( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{

	// fade out the last frame
	RenderFade( pD3DDevice );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	m_pTexture->EndRender();

} // CBlurLayer::EndRender()

void CBlurLayer::RenderBlur( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if( !m_pTexture->IsDynamicTextureValid() )
	{
		return;
	}
	// Set renderstates (disable z-buffering, enable stencil, disable fog, and
	// turn on alphablending)

	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	pD3DDevice->SetTexture( 0, m_pTexture->GetDynamicTexture() );

	// Draw a our blurred plane over the existing scene
	pD3DDevice->SetVertexShader( D3DFVF_BLURLAYERVERTEX );
	pD3DDevice->SetStreamSource( 0, m_pVertexBuffer, sizeof( BLURLAYERVERTEX ) );
	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	// Restore render states
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

} // CBlurLayer::Render()

void CBlurLayer::RenderBlurLeft( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if( !m_pTexture->IsDynamicTextureValid() )
	{
		return;
	}
	// Set renderstates (disable z-buffering, enable stencil, disable fog, and
	// turn on alphablending)

	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	pD3DDevice->SetTexture( 0, m_pTexture->GetDynamicTexture() );

	// Draw a our blurred plane over the existing scene
	pD3DDevice->SetVertexShader( D3DFVF_BLURLAYERVERTEX );
	pD3DDevice->SetStreamSource( 0, m_pVertexBufferLeftHalf, sizeof( BLURLAYERVERTEX ) );
	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	// Restore render states
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

} // CBlurLayer::RenderLeft()

void CBlurLayer::RenderBlurRight( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	if( !m_pTexture->IsDynamicTextureValid() )
	{
		return;
	}
	// Set renderstates (disable z-buffering, enable stencil, disable fog, and
	// turn on alphablending)

	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );

	pD3DDevice->SetTexture( 0, m_pTexture->GetDynamicTexture() );

	// Draw a our blurred plane over the existing scene
	pD3DDevice->SetVertexShader( D3DFVF_BLURLAYERVERTEX );
	pD3DDevice->SetStreamSource( 0, m_pVertexBufferRightHalf, sizeof( BLURLAYERVERTEX ) );
	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	// Restore render states
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );

} // CBlurLayer::RenderBlurRight()

bool CBlurLayer::IsValid( void )
{
	return m_pTexture->IsDynamicTextureValid();
} // CBlurLayer::IsValid()