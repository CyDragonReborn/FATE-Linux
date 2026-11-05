/*
	tread.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/24/2004

*/

#ifndef _TREAD_H
#define _TREAD_H

#include "d3dx8_shim.h"

class	CCullingBounds;
class	CFrustum;

// number of segments long a tread is
const uint32 KTreadLength( 40 );

// how much the treads automatically 'push' up from the ground
// to avoid Z-Fighting
const float32 KNormalBias( .5f );	

// how many seconds until we advance to updating the next segment of the tread
const float32 KTimePerSegment( .05f );	

// tread types should match the tread texture used by the tread manager,
// the texture should, left to right, match the tread types.
// None does not count, but it must ALWAYS exist
enum ETreadType
{
	KTreadNone,
	KTreadPavement,
	KTreadDirt,
	KTreadTypes
};

// the tread is a strip of polys that are updated to follow a tire, presumably.
class CTread
{

public :

				CTread( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
						const D3DXVECTOR3& Position,	// ground contact position of wheel
						const D3DXMATRIX& Orientation,	// orientation of wheel
						ETreadType Type,						// type of tread
						float32 Width );				// width of the tread

				~CTread();

	bool		CreateVB( LPDIRECT3DDEVICE8 pD3DDevice );	// direct3d device

	void		Update( LPDIRECT3DDEVICE8 pD3DDevice,	// d3d device
						const D3DXVECTOR3& Position,	// ground contact position of wheel
						const D3DXMATRIX& Orientation,	// orientation of wheel
						float32 TimeElapsed );			// time elapsed in seconds
	
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,		// direct3d device
						const D3DXMATRIX& ViewportMatrix,	// viewport matrix
						CFrustum* pFrustum );				// frustum to cull with - NULL for none

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	bool		Active( void )				{	return m_Active;	};

	ETreadType	Type( void )				{	return m_Type;		};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	void		SetActive( bool Active )	{	m_Active = Active;	};

private :
	
	bool					m_Active;

	uint32					m_CurrentIndex;		// current index of tread to update

	float32					m_CurrentTime;		// counter to increment updating index

	float32					m_Width;			// width of tread, in inches

	LPDIRECT3DVERTEXBUFFER8	m_pVB;

	D3DXVECTOR3				m_Position;
	D3DXMATRIX				m_TransformedMatrix;

	D3DXVECTOR3				m_MinBounds;
	D3DXVECTOR3				m_MaxBounds;

	// culling bounds of model
	CCullingBounds*		m_pCullingBounds;

	ETreadType			m_Type;
};


#endif