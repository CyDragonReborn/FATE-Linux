/*
	lightmanager.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/

#ifndef _LIGHTMANAGER_H
#define _LIGHTMANAGER_H

#include "../UTILITIES/llist.h"

class CDataLight;
class CSettings;

// this is just a simple class for maintaining location, orientation, and model references
// of an object in 3d space
class CLightManager
{
public:

								CLightManager( CSettings& pSettings );	// pointer to the settings manager, for keeping track of card/machine capabilities
								~CLightManager( void );

		void					UpdateLights( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
											  const D3DXMATRIX& Transformation );	// world space transformation of object

		CDataLight*				AddLight( CDataLight* pLight );	// light to add

		void					DisableLights( LPDIRECT3DDEVICE8 pD3DDevice );		// direct3d device

		void					Clear( void );

		void					SetSceneAmbient( float32 R,		// red component, 0-1
												 float32 G, 	// green component, 0-1
												 float32 B );	// blue component, 0-1

		void					SetSceneDiffuse( float32 R,		// red component, 0-1
												 float32 G, 	// green component, 0-1
												 float32 B );	// blue component, 0-1

		void					SetSceneSpecular( float32 R,	// red component, 0-1
												  float32 G, 	// green component, 0-1
												  float32 B );	// blue component, 0-1

		void					SetSceneSpecularPower( float32 Power );	// specular power

		const D3DXVECTOR3&		LightDirection( void );

private:
	uint32					m_AmbientR;
	uint32					m_AmbientG;
	uint32					m_AmbientB;

	D3DMATERIAL8*			m_pBaseMaterial;

	CSettings&				m_pSettings;

	CLList<CDataLight>*		m_pLightList;

	D3DXVECTOR3				m_LightDirection;

};

#endif