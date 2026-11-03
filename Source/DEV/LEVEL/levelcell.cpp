/*
	levelprop.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/17/2004

*/
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/utilities.h"
#include "../UTILITIES/constants.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "../UTILITIES/cullingbounds.h"

#include "../MEDIA/refmanager.h"

#include "../MODEL/model.h"

#include "../GAMEOBJECTS/lightmanager.h"

#include "levelprop.h"

CLevelProp::CLevelProp( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					    CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
						CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
						const D3DXVECTOR3& Position,	// worlddspace position
						const std::string& Path ) :		// path to visual model
						m_pRefManager( pRefManager ),
						m_pSettings( pSettings ),
						m_Position( Position ),
						m_pModel( NULL ),
					    m_pCullingBounds( NULL )
{
	m_pModel = m_pRefManager.AddModel( pD3DDevice,
									   Path,
									   "",
									   kTrue,
									   kTrue );

	m_pCullingBounds = new CCullingBounds( m_pModel->GetMinBounds(),
										   m_pModel->GetMaxBounds() );

	D3DXMatrixIdentity( &m_OrientationMatrix );
	
	
	D3DXMATRIX TransformedMatrix;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &TransformedMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	TransformedMatrix = m_OrientationMatrix * TransformedMatrix;

	m_pCullingBounds->Update( TransformedMatrix );

} // CLevelProp::CLevelProp()

CLevelProp::~CLevelProp( void )
{
	m_pRefManager.RemoveModel( m_pModel );


	DELETE_SAFELY( m_pCullingBounds );
} // CLevelProp::~CLevelProp()

void CLevelProp::Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
					 CLightManager* pLightManager,			// light manager - NULL for no light manager
					 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
					 CFrustum* pFrustum )					// frustum to cull with - NULL for none
{
	// no model, no render
	if( m_pModel == NULL )
	{
		return;
	}
		
	D3DXMATRIX TransformedMatrix;

	// calculate the translation matrix based upon our location
	D3DXMatrixTranslation( &TransformedMatrix,
						   m_Position.x,
						   m_Position.y,
						   m_Position.z );

	TransformedMatrix = m_OrientationMatrix * TransformedMatrix;


	if( pFrustum != NULL )
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum->Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return;
		}
	}

	D3DXVECTOR3 NormalLight( 0, -1, 0 );

	if( pLightManager != NULL )
	{
		pLightManager->UpdateLights( pD3DDevice, TransformedMatrix );

		D3DXMATRIX InverseTransformation;
		D3DXMatrixInverse( &InverseTransformation, NULL, &TransformedMatrix );

		D3DXVec3TransformNormal( &NormalLight, &pLightManager->LightDirection(), &InverseTransformation );

	}

	TransformedMatrix *= ViewportMatrix;

	m_pModel->Render( pD3DDevice, TransformedMatrix, NormalLight );

} // CLevelProp::Render()

bool CLevelProp::InFrustum( CFrustum& pFrustum )	// frustum to cull against					// frustum to cull with - NULL for none

{
	// no model, no render
	if( m_pModel == NULL )
	{
		return kFalse;
	}

	// if we're not in the frustum, don't render at all
	if( pFrustum.Intersects( *m_pCullingBounds ) == KFrustumOutside )
	{
		return kFalse;
	}

	return kTrue;
} // CLevelProp::InFrustum()