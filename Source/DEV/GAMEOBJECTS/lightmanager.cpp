/*
	lightmanager.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

	CHANGE HISTORY

	Travis Baldree
	7/6/2004	-	Re-enabled spotlight support

*/

#include <D3D8.h>
#include <d3dx8.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/llist.h"
#include "../UTILITIES/math3d.h"
#include "../OBJECTS/datalight.h"
#include "lightmanager.h"


CLightManager::CLightManager( CSettings& pSettings ) :		// settings class to maintain info about light capabilities
							  m_pSettings( pSettings ),
							  m_AmbientR( 20 ),
							  m_AmbientG( 20 ),
							  m_AmbientB( 20 ),
							  m_pLightList ( new CLList< CDataLight >() ),
							  m_pBaseMaterial ( new D3DMATERIAL8() )
{
	ZeroMemory ( &m_LightDirection, sizeof ( m_LightDirection ));

	m_pBaseMaterial->Diffuse.r	= 1.0f;					
	m_pBaseMaterial->Diffuse.g	= 1.0f;
	m_pBaseMaterial->Diffuse.b	= 1.0f;
	m_pBaseMaterial->Diffuse.a	= 1.0f;

	m_pBaseMaterial->Specular.r = 0.4F;
	m_pBaseMaterial->Specular.g = 0.4F;
	m_pBaseMaterial->Specular.b = 0.4F;

	m_pBaseMaterial->Power		= 10.0f;

	m_pBaseMaterial->Ambient.r	= 0.15f;						
	m_pBaseMaterial->Ambient.g	= 0.15f;
	m_pBaseMaterial->Ambient.b	= 0.15f;
	m_pBaseMaterial->Ambient.a	= 1.0f;



} // CLightManager::CLightManager()

CLightManager::~CLightManager( void )
{
	DELETE_SAFELY( m_pBaseMaterial );
	DELETE_SAFELY( m_pLightList );

} // CLightManager::~CLightManager()

void CLightManager::SetSceneAmbient( float32 R,		// red component, 0-1
									 float32 G, 	// green component, 0-1
									 float32 B )	// blue component, 0-1
{
	m_pBaseMaterial->Ambient.r	= R;						
	m_pBaseMaterial->Ambient.g	= G;
	m_pBaseMaterial->Ambient.b	= B;

	m_AmbientR = (uint32) ( R*255 );
	m_AmbientG = (uint32) ( R*255 );
	m_AmbientB = (uint32) ( R*255 );
} // CLightManager::SetSceneAmbient()


void CLightManager::SetSceneSpecular( float32 R,		// red component, 0-1
									  float32 G, 	// green component, 0-1
									  float32 B )	// blue component, 0-1
{
	m_pBaseMaterial->Specular.r	= R;						
	m_pBaseMaterial->Specular.g	= G;
	m_pBaseMaterial->Specular.b	= B;
} // CLightManager::SetSceneSpecular()

void CLightManager::SetSceneSpecularPower( float32 Power )	// specular power
{
	m_pBaseMaterial->Power		= Power;
} // CLightManager::SetSceneSpecularPower()

void CLightManager::SetSceneDiffuse( float32 R,		// red component, 0-1
									 float32 G, 	// green component, 0-1
									 float32 B )	// blue component, 0-1
{
	m_pBaseMaterial->Diffuse.r	= R;						
	m_pBaseMaterial->Diffuse.g	= G;
	m_pBaseMaterial->Diffuse.b	= B;
} // CLightManager::SetSceneDiffuse()


// add a light to our scene
CDataLight* CLightManager::AddLight( CDataLight* pLight )	// light to add
{
	m_pLightList->Queue( pLight );

	return pLight;
} // CLightManager::AddLight()

// remove all lights from the scene
void CLightManager::Clear( void )
{
	m_pLightList->Clear();
} // CLightManager::Clear()

void CLightManager::DisableLights( LPDIRECT3DDEVICE8 pD3DDevice )		// direct3d device
{
	for ( int32 i = 0; i < m_pSettings.GetSettings( KSetMaxLights ) ; i++ )
	{
		pD3DDevice->LightEnable( i, kFalse );
	}
} // CLightManager::DisableLights()


void CLightManager::UpdateLights( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								  const D3DXMATRIX& Transformation )	// world space transformation of object
{
	pD3DDevice->SetMaterial( m_pBaseMaterial );
	// give us some ambient light
	pD3DDevice->SetRenderState( D3DRS_AMBIENT,  RGB( m_AmbientR,
													 m_AmbientG,
													 m_AmbientB ) );

	D3DXMATRIX InverseTransformation;
	D3DXMatrixInverse( &InverseTransformation, NULL, &Transformation );

	uint32	MaxLights	= m_pSettings.GetSettings( KSetMaxLights ) ;
	if ( MaxLights <= 0 )
	{
		return;
	}

	uint32		Index = 0;

	CLLNode< CDataLight > *Light	= m_pLightList->GetFirst();

	D3DXVECTOR3	ScratchVector;
	D3DXVECTOR3	ScratchDirection;

	// we go for all directional lights first, 'cos they're the most important
	// we dont' want to run out of available lights on pointlights
	while ( Light != NULL )
	{

		if( Light->Data()->Type() == KLightDirectional )
		{
			
			ScratchVector = Light->Data()->Direction();
			D3DXVec3TransformNormal( &ScratchVector, &ScratchVector, &InverseTransformation );

			if ( Index < MaxLights )
			{
					
				// set the active direction of the light to this transformed vector
				Light->Data()->D3DLight().Direction = ScratchVector;

				pD3DDevice->SetLight( Index, &Light->Data()->D3DLight() );
				pD3DDevice->LightEnable( Index, kTrue );
					
				Index++;
			}
		}
		Light = Light->Next();
	}

	Light	= m_pLightList->GetFirst();

	// now we can pick up any pointlights/spotlights
	while ( Light != NULL )
	{
		if( Light->Data()->Type() == KLightOmni ||
			Light->Data()->Type() == KLightFreeSpot ||
			Light->Data()->Type() == KLightTargetSpot )
		{
			
			ScratchVector = Light->Data()->Position();
			D3DXVec3TransformCoord( &ScratchVector, &ScratchVector, &InverseTransformation );
			
			float32 Length = D3DXVec3Length( &ScratchVector );
			//if ( Length < Light->Data()->Range() )
			{		
				if ( Index < MaxLights )
				{

					// set its active position to the locally transformed position
					Light->Data()->D3DLight().Position = ScratchVector;

					ScratchVector = Light->Data()->Direction();
					D3DXVec3TransformNormal( &ScratchVector, &ScratchVector, &InverseTransformation );

					// set the active direction of the light to this transformed vector
					//Light->Data()->D3DLight().Direction = ScratchVector;

					pD3DDevice->SetLight( Index, &Light->Data()->D3DLight() );
					pD3DDevice->LightEnable( Index, kTrue );
					
					Index++;
				}
			}
		}

		Light = Light->Next();
	}

	// disable any lights that are no longer in range
	for ( ; Index <= MaxLights; Index++ )
	{
		pD3DDevice->LightEnable( Index, kFalse );
	}

} // CLightManager::UpdateLights()

const D3DXVECTOR3& CLightManager::LightDirection( void )
{
	CLLNode< CDataLight > *Light	= m_pLightList->GetFirst();


	// we go for all directional lights first, 'cos they're the most important
	// we dont' want to run out of available lights on pointlights
	while ( Light != NULL )
	{

		if( Light->Data()->Type() == KLightDirectional )
		{
			m_LightDirection = Light->Data()->Direction();
		}
		Light = Light->Next();
	}

	return m_LightDirection;
} // CLightManager::LightDirection()
