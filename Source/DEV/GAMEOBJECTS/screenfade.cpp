/*
	ScreenFade.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/settings.h"
#include "screenfade.h"

CScreenFade::CScreenFade( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
						  CSettings& pSettings ) :			// the settings manager, for keeping track of card/machine capabilities
						  m_Color( 0, 0, 0, .8f ),
						  m_pVertexBuffer( NULL )
{
	pD3DDevice->CreateVertexBuffer( 4 * sizeof( FILLSHADOWVERTEX ),
												D3DUSAGE_WRITEONLY,
												D3DFVF_FILLSHADOWVERTEX,
												D3DPOOL_MANAGED,
												&m_pVertexBuffer ) ;
	RestoreDeviceObjects( pD3DDevice,
						  pSettings );
}


CScreenFade::~CScreenFade( void )
{
	RELEASE_SAFELY( m_pVertexBuffer );
} // CScreenFade::~CScreenFade()


void CScreenFade::RestoreDeviceObjects( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
										CSettings& pSettings )			// so that we can tell the dimensions of the screen
{
	FILLSHADOWVERTEX*	Vertex;
	float32			SX	= ( float32 ) pSettings.GetSettings( KSetScreenWidth );
	float32			SY	= ( float32 ) pSettings.GetSettings( KSetScreenHeight );
	m_pVertexBuffer->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].p = D3DXVECTOR4( 0, SY, 0.0f, 1.0f );
	Vertex[1].p = D3DXVECTOR4( 0, 0, 0.0f, 1.0f );
	Vertex[2].p = D3DXVECTOR4( SX, SY, 0.0f, 1.0f );
	Vertex[3].p = D3DXVECTOR4( SX, 0, 0.0f, 1.0f );
	Vertex[0].color = D3DXCOLOR( m_Color.x, m_Color.y, m_Color.z, m_Color.w );
	Vertex[1].color = D3DXCOLOR( m_Color.x, m_Color.y, m_Color.z, m_Color.w );
	Vertex[2].color = D3DXCOLOR( m_Color.x, m_Color.y, m_Color.z, m_Color.w );
	Vertex[3].color = D3DXCOLOR( m_Color.x, m_Color.y, m_Color.z, m_Color.w );
	m_pVertexBuffer->Unlock();
} // CScreenFade::RestoreDeviceObjects()

// change the color/opacity of the fade
void CScreenFade::SetColor( float32 R,	// red component
							float32 G,	// green component
							float32 B,	// blue component
							float32 A )	// alpha component
{
	m_Color = D3DXVECTOR4( R, G, B, A );

	FILLSHADOWVERTEX*	Vertex;
	m_pVertexBuffer->Lock( 0, 0, ( BYTE * * ) &Vertex, 0 );
	Vertex[0].color = D3DXCOLOR( R, G, B, A );
	Vertex[1].color = D3DXCOLOR( R, G, B, A );
	Vertex[2].color = D3DXCOLOR( R, G, B, A );
	Vertex[3].color = D3DXCOLOR( R, G, B, A );
	m_pVertexBuffer->Unlock();
} // CScreenFade::SetShadowColor()

void CScreenFade::Render( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	// turn off lighting
	pD3DDevice->SetRenderState( D3DRS_LIGHTING, kFalse );

	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );


	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );

	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );


	// Draw a big square
	pD3DDevice->SetVertexShader( D3DFVF_FILLSHADOWVERTEX );
	pD3DDevice->SetStreamSource( 0, m_pVertexBuffer, sizeof( FILLSHADOWVERTEX ) );
	pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

	// Restore render states
	pD3DDevice->SetRenderState( D3DRS_ZENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );

	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

} // CScreenFade::Render()