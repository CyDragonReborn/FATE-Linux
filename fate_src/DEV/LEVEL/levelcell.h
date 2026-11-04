/*
	levelcell.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVELCELL_H
#define _LEVELCELL_H

#include "d3dx8_shim.h"


class CRefManager;
class CSettings;
class CModel;
class CFrustum;
class CCullingBounds;

// a levelcell contains a model, which is its visual representation,
// as well as data about contents and passability.

class CLevelCell
{


public:

	CLevelCell( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
				CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
				CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
				const D3DXVECTOR3& Position,	// worlddspace position
				const std::string& Path );		// path to visual model
	~CLevelCell( void );
	
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						CLightManager* pLightManager,			// light manager - null for no light manager
					    const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderNoTexture( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
								 CLightManager* pLightManager,			// light manager - null for no light manager
								 const D3DXMATRIX& ViewportMatrix,		// viewport matrix
								 CFrustum* pFrustum );					// frustum to cull with - NULL for none

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	CCullingBounds*	CullingBounds( void )		{	return m_pCullingBounds;	};

	CModel*			Model( void )				{	return m_pModel;			};

	const D3DXVECTOR3&	Position( void ) const		{	return m_Position;			};

	const D3DXMATRIX&	Orientation( void ) const	{	return m_OrientationMatrix;	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////


private:

	
	bool		InFrustum( CFrustum& pFrustum );	// frustum to cull against

	D3DXVECTOR3			m_Position;

	D3DXMATRIX			m_OrientationMatrix;

	CCullingBounds*		m_pCullingBounds;

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CModel*				m_pModel;
};

#endif