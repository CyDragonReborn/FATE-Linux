/*
	treadmanager.cpp

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	7/24/2004
	
*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"

#include "../UTILITIES/llist.h"

#include "../MEDIA/refmanager.h"
#include "../MEDIA/material.h"


#include "tread.h"
#include "treadmanager.h"

CTreadManager::CTreadManager( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
							  CRefManager& pRefManager,			// the reference manager, for tracking media we create
							  CSettings& pSettings,				// the settings manager, for keeping track of card/machine capabilities
							  const std::string& TexturePath,	// primary texture for treads
							  const std::string& AlphaPath ) :	// alpha mask for tread texture
							  m_pRefManager( pRefManager ),
							  m_pSettings( pSettings ),
							  m_ActiveTreads( 0 )
{
	m_MaximumTreads = KMaximumTreads;
	// create a templatized linked list for all active treads
	m_pTreadList = new CLList< CTread >();

	m_pTreadMaterial = m_pRefManager.AddMaterialAlpha( pD3DDevice, TexturePath, AlphaPath );
} // CTreadManager::CTreadManager()

CTreadManager::~CTreadManager( void )
{
	m_pRefManager.RemoveMaterial( m_pTreadMaterial );

	DELETE_SAFELY( m_pTreadList );
} // CTreadManager::~CTreadManager()

CTread*	CTreadManager::AddTread( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
								 const D3DXVECTOR3& Position,	// ground contact position of wheel
								 const D3DXMATRIX& Orientation, // orientation of wheel
								 ETreadType Type,				// type of tread
								 float32 Width )				// width of the tread
{
	if( m_MaximumTreads == 0 )
	{
		return NULL;
	}
	if( m_ActiveTreads > m_MaximumTreads &&
		!PruneInactiveTread() )
	{
		return NULL;
	}
	CTread* pTread = m_pTreadList->Queue( new CTread( pD3DDevice,
													  Position,
													  Orientation,
													  Type,
													  Width ) );
	m_ActiveTreads = m_pTreadList->Nodes();

	return pTread;
} // CTreadManager::AddTread()

// prunes one inactive tread from the list, if possible 
bool CTreadManager::PruneInactiveTread( void )
{
	CLLNode<CTread>*	pPruneTread( NULL );
	// go through our linked list of treads, and check for dead ones
	CLLNode<CTread>*	pTread	= m_pTreadList->GetFirst();
	while ( pTread != NULL )
	{
		if( !pTread->Data()->Active() )
		{
			pPruneTread = pTread;
		}
		pTread = pTread->Next();
	}
	if( pPruneTread != NULL )
	{
		m_pTreadList->Remove( pPruneTread );
		return kTrue;
	}

	return kFalse;
} // CTreadManager::PruneInactiveTread()

void CTreadManager::Render( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
							const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							CFrustum*	pFrustum )					// frustum to cull against (NULL for no culling)

{
	// make sure we're alpha blending, and turn on additive blending
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kTrue );
	pD3DDevice->SetRenderState( D3DRS_LIGHTING,  kFalse );
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kFalse );

	pD3DDevice->SetTexture( 0, m_pTreadMaterial->GetTexture() );

	// go through our linked list of treads, and check for dead ones
	CLLNode<CTread>*	pTread	= m_pTreadList->GetFirst();
	while ( pTread != NULL )
	{
		pTread->Data()->Render( pD3DDevice,
								ViewportMatrix,
								pFrustum );
		pTread = pTread->Next();
	}

	// back to standard settings
	pD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
	pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, kFalse );
	pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, kTrue );

} // CTreadManager::Render()
