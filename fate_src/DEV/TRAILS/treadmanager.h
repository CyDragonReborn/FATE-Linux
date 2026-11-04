/*
	treadmanager.h

  Copyright (C) 2004 WildTangent, Inc. 
  All Rights Reserved
  
	Travis Baldree
	6/24/2004
	
*/

#ifndef _TREADMANAGER_H
#define _TREADMANAGER_H

#include "../UTILITIES/llist.h"
#include "../UTILITIES/constants.h"

class CSettings;
class CRefManager;
class CMaterial;
class CFrustum;
class CTread;

// maximum number of treadmarks allowed in a scene
const uint32	KMaximumTreads( 100 );

// manager all the treads in a scene
class CTreadManager
{
public:


				CTreadManager( LPDIRECT3DDEVICE8 pD3DDevice,		// our direct3d device
							   CRefManager& pRefManager,			// the reference manager, for tracking media we create
							   CSettings& pSettings,				// the settings manager, for keeping track of card/machine capabilities
							   const std::string& TexturePath,		// primary texture for treads
							   const std::string& AlphaPath );		// alpha mask for tread texture

				~CTreadManager( void );

	CTread*	AddTread( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
					  const D3DXVECTOR3& Position,	// ground contact position of wheel
					  const D3DXMATRIX& Orientation,// orientation of wheel
					  ETreadType Type,				// type of tread
					  float32 Width );				// width of the tread

	void	Render( LPDIRECT3DDEVICE8 pD3DDevice,		// d3d device
					const D3DXMATRIX& ViewportMatrix,	// camera's viewport matrix
					CFrustum* Frustum = NULL );			// frustum to cull to

	void	SetMaximumTreads( uint32 Treads )		{	m_MaximumTreads = Treads;		};

private:

	bool				PruneInactiveTread( void );

	uint32				m_MaximumTreads;
	uint32				m_ActiveTreads;	// number of treads currently in the manager

	CLList<CTread>*		m_pTreadList;	// list of pointers to all active treads
	
	CMaterial*			m_pTreadMaterial;

	CSettings&			m_pSettings;
	CRefManager&		m_pRefManager;


};

#endif