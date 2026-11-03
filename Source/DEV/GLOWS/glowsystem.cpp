/*
	glowsystem.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	2/26/2004
	
*/

#include <assert.h>

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/llist.h"
#include "../UTILITIES/math3d.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/vertexdefinitions.h"
#include "../UTILITIES/frustum.h"
#include "glowsystem.h"

CGlowSystem::CGlowSystem( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
								  CSettings& pSettings,			// settings object, to let us know what we can do
								  float32 GlowWidthHeight,		// width and height of basic glows, in units
								  float32 ScaleSpeed,			// change in scale in units per second
								  float32 FadeSpeed,			// change in opacity per second - 0 means no fading - opacity is 0-255, so a value of -255 would be totally transparent in 1 second
								  float32 MinScaleMagnitude,	//	min scale
								  float32 MaxScaleMagnitude,	//	max scale
								  float32 MinFadeMagnitude,		//	min fade
								  float32 MaxFadeMagnitude,		//	max fade
								  uint32 FlipbookWidthHeight,		// if this is an animated glow, how many frames wide/high is our animation sheet
								  float32 SecondsPerFlipbookFrame ) : // how much time is spent on each frame of a flipbook animation
																		m_pSettings( pSettings ),
																		m_GlowWidthHeight( GlowWidthHeight ),
																		m_ScaleSpeed( ScaleSpeed ),
																		m_FadeSpeed( FadeSpeed ),
																		m_MinScaleMagnitude( MinScaleMagnitude ),
																		m_MaxScaleMagnitude( MaxScaleMagnitude ),
																		m_MinFadeMagnitude( MinFadeMagnitude ),
																		m_MaxFadeMagnitude( MaxFadeMagnitude ),
																		m_FlipbookWidthHeight( FlipbookWidthHeight ),
																		m_FlipbookFrames( FlipbookWidthHeight * FlipbookWidthHeight ),
																		m_SecondsPerFlipbookFrame( SecondsPerFlipbookFrame ),
																		m_IsAnimated( FlipbookWidthHeight > 1 ),
																		m_SystemValid( kFalse ),
																		m_AdditiveFading( kFalse ),
																		m_FlipbookFrameUVWidth( 1 ),
																		m_ActiveGlows( 0 ),
																		m_pGlowList ( new CLList<CGlow>() ),
																		m_pVB( NULL )
{
	ZeroMemory ( &m_Gravity, sizeof ( m_Gravity ));

	if( m_FlipbookFrames > 1 )
	{
		m_FlipbookFrameUVWidth = (float32)( 1.0f / m_FlipbookWidthHeight );
	}

	m_SystemValid = CreateVB( pD3DDevice );

} // CGlowSystem::CGlowSystem()



CGlowSystem::~CGlowSystem( void )
{
	RELEASE_SAFELY( m_pVB );
	DELETE_SAFELY( m_pGlowList );
} // CGlowSystem::~CGlowSystem()

CGlow* CGlowSystem::Add( CGlow* pGlow )	// glow to add
{
	assert( pGlow != NULL );

	if( !m_SystemValid )
	{
		return NULL;
	}

	// if it won't exceed our maximum, add this glow to the list
	m_pGlowList->Queue( pGlow );
	return pGlow;
} // CGlowSystem::Add()

void CGlowSystem::Clear( void )
{
	// remove all glows from the linked list
	m_pGlowList->Destroy();
} // CGlowSystem::Clear()

bool CGlowSystem::CreateVB( LPDIRECT3DDEVICE8 pD3DDevice )	// our direct3d device
{
	HRESULT hr;
	uint32 Verts( 4 );

	// set up the vert buffer for all sprites
    hr = pD3DDevice->CreateVertexBuffer( Verts * sizeof(SPRITEVERTEX),
										 D3DUSAGE_WRITEONLY, 
										 D3DFVF_SPRITEVERTEX,
										 D3DPOOL_MANAGED, 
										 &m_pVB );
	
	if( FAILED(hr) )
	{
		return kFalse;
	}

	float32 HalfWidth = m_GlowWidthHeight * .5f;

	SPRITEVERTEX* pVertices;
	if( FAILED( m_pVB->Lock( 0, 0, (BYTE**)&pVertices, NULL ) ) )
	{
		return kFalse;
	}

	pVertices[0].v.x	= -HalfWidth;
	pVertices[0].v.y	= HalfWidth;
	pVertices[0].v.z	= 0;
	pVertices[0].tu   = 0;
	pVertices[0].tv   = 0;
	pVertices[0].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	pVertices[1].v.x	= HalfWidth;
	pVertices[1].v.y	= HalfWidth;
	pVertices[1].v.z	= 0;
	pVertices[1].tu   = m_FlipbookFrameUVWidth;
	pVertices[1].tv   = 0;
	pVertices[1].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	pVertices[2].v.x	= HalfWidth;
	pVertices[2].v.y	= -HalfWidth;
	pVertices[2].v.z	= 0;
	pVertices[2].tu   = m_FlipbookFrameUVWidth;
	pVertices[2].tv   = m_FlipbookFrameUVWidth;
	pVertices[2].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	pVertices[3].v.x	= -HalfWidth;
	pVertices[3].v.y	= -HalfWidth;
	pVertices[3].v.z	= 0;
	pVertices[3].tu   = 0;
	pVertices[3].tv   = m_FlipbookFrameUVWidth;
	pVertices[3].color	= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );


	m_pVB->Unlock();	

	return kTrue;
} // CGlowSystem::CreateVB()

void CGlowSystem::Update( float32 TimeElapsed )					// time elapsed in seconds
{
	if( !m_SystemValid )
	{
		return;
	}

	// we're going to re-count our glows, so reset our tally
	m_ActiveGlows = 0;

	CLLNode<CGlow>* pCurrentGlow	= m_pGlowList->GetFirst();
	CLLNode<CGlow>* NextGlow	= NULL;
	CGlow*	pGlowData;

	float32 FadeChange		= m_FadeSpeed * TimeElapsed;
	float32 FrameIncrement	= TimeElapsed / m_SecondsPerFlipbookFrame;
	float32 FrameGate		= (float32)m_FlipbookFrames - .5f;

	bool CanFade( kFalse );
	if( m_FadeSpeed != 0 && m_pSettings.GetSettings( KSetAlphaFade ) )
	{
		CanFade = kTrue;
	}

	while( pCurrentGlow != NULL )
	{

		m_ActiveGlows++;

		NextGlow = pCurrentGlow->Next();
		pGlowData = pCurrentGlow->Data();

		if( pGlowData->m_RequestingRemoval )
		{
			m_pGlowList->Remove( pCurrentGlow );
		}
		else
		{

			pGlowData->m_OpacityOffset += FadeChange;

			pGlowData->m_Scale = ( sin( pGlowData->m_OpacityOffset ) * .5f + .5f ) *
								( m_MaxFadeMagnitude - m_MinFadeMagnitude ) + 
								m_MinFadeMagnitude;

			if( pGlowData->m_Opacity > 255 )
			{
				pGlowData->m_Opacity = 255;
			}

			if( pGlowData->m_Opacity < 0  )
			{
				pGlowData->m_Opacity = 0;
			}

			// if glows are flipbook animated
			if( m_FlipbookFrames > 1 )
			{
				pGlowData->m_FlipbookFrame	+= FrameIncrement;
				if( pGlowData->m_FlipbookFrame >= FrameGate )
				{
					pGlowData->m_FlipbookFrame -= (float32)m_FlipbookFrames;
				}
			}

			// rotate around the z axis
			pGlowData->m_ZRotation += pGlowData->m_ZRotationSpeed * TimeElapsed;


			if( m_ScaleSpeed != 0 )
			{
				pGlowData->m_ScaleOffset += m_ScaleSpeed * TimeElapsed;
				pGlowData->m_Scale = ( sin( pGlowData->m_ScaleOffset ) * .5f + .5f ) *
									( m_MaxScaleMagnitude - m_MinScaleMagnitude ) + 
									m_MinScaleMagnitude;
			}
		}
		pCurrentGlow = NextGlow;
	}
} // CGlowSystem::Update()

void CGlowSystem::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// our direct3d device
							  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							  CFrustum*	pFrustum )					// frustum to cull against (NULL for no culling)

{
	if( !m_SystemValid )
	{
		return;
	}

	D3DXMATRIX UVTransformation;
	D3DXMatrixIdentity(&UVTransformation);

	D3DXMATRIX Translation;
	D3DXMATRIX Scale;
	D3DXMATRIX Rotation;

	D3DXVECTOR3 Start( 0, 0, 0 );
	D3DXVECTOR3 Up( 0, 1, 0 );
	D3DXVECTOR3 Forward( 0, 0, 1 );

	D3DXMATRIX Orientation;
	D3DXMatrixTranspose( &Orientation, &ViewportMatrix);
	Orientation._41=0;
	Orientation._42=0;
	Orientation._43=0;
	Orientation._14=0;
	Orientation._24=0;
	Orientation._34=0;


	
    pD3DDevice->SetVertexShader( D3DFVF_SPRITEVERTEX );
    pD3DDevice->SetStreamSource( 0, m_pVB, sizeof(SPRITEVERTEX) );

	CLLNode<CGlow>* pCurrentGlow	= m_pGlowList->GetFirst();
	CGlow*			pGlowData	= NULL;

	int32	LastSetFrame	= -1;
	uint32	Fade			= 0;
	int32	Frame			= 0;
	int32	Row				= 0;
	int32	Col				= 0;
	bool	CanFade			= kFalse;
	bool	IsScaling		= kFalse;

	// we only fade if this glow system has animated opacity and the machine supports it
	// expensive stuff, so be careful!
	if( m_FadeSpeed != 0 && m_pSettings.GetSettings( KSetAlphaFade ) )
	{
		CanFade = kTrue;

		// turn on fading
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_BLENDFACTORALPHA );
		if( m_AdditiveFading )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
		}
		else
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
		}
	}

	if( m_ScaleSpeed != 0 )
	{
		IsScaling = kTrue;
	}

	// if we are animated, set up a texture transformation
	if( m_FlipbookFrames > 1)
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
	}

	while( pCurrentGlow != NULL )
	{
		pGlowData = ( CGlow* )pCurrentGlow->Data();

		if( pFrustum == NULL || 
			pFrustum->SphereIntersectsAABB( pGlowData->m_Position, 
											m_GlowWidthHeight * pGlowData->m_Scale ) )
		{
			// if this glow system has a flipbook animation
			if( m_FlipbookFrames > 1 )
			{
				Frame = (uint32)floor( pGlowData->m_FlipbookFrame + .5f );

				// no sense changing the UV transform if the frame is the same
				if( Frame != LastSetFrame )
				{
					Row	=	(uint32)floor( (float32)Frame / (float32)m_FlipbookWidthHeight );
					Col	=	Frame - ( ( Row ) * m_FlipbookWidthHeight );
					UVTransformation._31 = Col * m_FlipbookFrameUVWidth;
					UVTransformation._32 = Row * m_FlipbookFrameUVWidth;
					pD3DDevice->SetTransform( D3DTS_TEXTURE0, &UVTransformation );

					LastSetFrame = Frame;
				}
			}

			if( CanFade )
			{
				uint32 Fade = (uint32)floor( pGlowData->m_Opacity + .5f );
				pD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 
											D3DCOLOR_RGBA(0, 0, 0, Fade));			
			}

			D3DXMatrixTranslation( &Translation, 
								   pGlowData->m_Position.x,
								   pGlowData->m_Position.y,
								   pGlowData->m_Position.z );

			D3DXMatrixScaling( &Scale,
							   pGlowData->m_Scale,
							   pGlowData->m_Scale,
							   0 );


			D3DXMatrixRotationZ( &Rotation, 
								 (float32)( pGlowData->m_ZRotation * KDeg2Rad ) );

			Translation = Scale * Rotation * Orientation *
						  Translation *
						  ViewportMatrix;

			pD3DDevice->SetTransform( D3DTS_VIEW, &Translation );
			
			pD3DDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,  0, 2 );
		}
		else	// not in the frustum, so we mark this glow for 
				// immediate removal
		{
			pGlowData->m_Opacity = -1;
		}
		pCurrentGlow = pCurrentGlow->Next();
	}
	// if we are animated, turn off the texture transformation
	if( m_FlipbookFrames > 1)
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
	}

	// turn off fading
	if( CanFade )
	{
		pD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		if( m_AdditiveFading )
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
		}
		else
		{
			pD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_CURRENT );
			pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
		}
	}

} // CGlowSystem::Render()



