/*
	cullingbounds.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	CHANGE HISTORY
	
	5/27/2004	-	worldspace culling bounds were slightly incorrect, leading to some popping.
					Fixed.

	6/18/2004	-	added access to local min/max vakyes

*/



#ifndef _CULLINGBOUNDS_H
#define _CULLINGBOUNDS_H


#include <D3D8.h>
#include <d3dx8.h>

#include "settings.h"
#include "math3d.h"

// maintains the 8 corners of a bounding box for use with a frustum for culling

class CCullingBounds
{
public:
	CCullingBounds( const D3DXVECTOR3& MinBounds,
					const D3DXVECTOR3& MaxBounds ) :
					m_MinBounds( MinBounds ),
					m_MaxBounds( MaxBounds ),
					m_WorldMinBounds( MinBounds ),
					m_WorldMaxBounds( MaxBounds )

	{
	} // CCullingBounds()

	~CCullingBounds( void )
	{
	} // ~CCullingBounds()


	void	Update( const D3DXMATRIX& Transformation )
	{
		D3DXVECTOR3 Point;
		
		D3DXVec3TransformCoord( &Point, &m_MinBounds, &Transformation );
		m_WorldMinBounds = Point;
		m_WorldMaxBounds = Point;
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );
		
		D3DXVec3TransformCoord( &Point, &m_MaxBounds, &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		D3DXVec3TransformCoord( &Point, &D3DXVECTOR3( m_MinBounds.x, m_MinBounds.y, m_MaxBounds.z ), &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		D3DXVec3TransformCoord( &Point, &D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MaxBounds.z ), &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		D3DXVec3TransformCoord( &Point, &D3DXVECTOR3( m_MinBounds.x, m_MaxBounds.y, m_MinBounds.z ), &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		D3DXVec3TransformCoord( &Point, &D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MinBounds.z ), &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		D3DXVec3TransformCoord( &Point, &D3DXVECTOR3( m_MaxBounds.x, m_MaxBounds.y, m_MinBounds.z ), &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		D3DXVec3TransformCoord( &Point, &D3DXVECTOR3( m_MaxBounds.x, m_MinBounds.y, m_MaxBounds.z ), &Transformation );
		ExpandBounds( m_WorldMinBounds, m_WorldMaxBounds, Point );

		CalculateBounds();
		
	}

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	D3DXVECTOR3*	GetBounds( void )		{	return m_CullingBounds;	};

	const D3DXVECTOR3&	GetMinBounds( void )	{	return m_WorldMinBounds;	};
	const D3DXVECTOR3&	GetMaxBounds( void )	{	return m_WorldMaxBounds;	};

	const D3DXVECTOR3&	GetLocalMinBounds( void )	{	return m_MinBounds;	};
	const D3DXVECTOR3&	GetLocalMaxBounds( void )	{	return m_MaxBounds;	};

////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////

	void		SetLocalMinBounds( const D3DXVECTOR3& MinBounds )	{	m_MinBounds = MinBounds;	};
	void		SetLocalMaxBounds( const D3DXVECTOR3& MaxBounds )	{	m_MaxBounds = MaxBounds;	};

private:

	// calculates all 8 corners based upon min and max bounds
	void	CalculateBounds( void )
	{
		m_CullingBounds[0] = D3DXVECTOR3( m_WorldMinBounds.x, m_WorldMinBounds.y, m_WorldMinBounds.z );
		m_CullingBounds[1] = D3DXVECTOR3( m_WorldMaxBounds.x, m_WorldMinBounds.y, m_WorldMinBounds.z );
		m_CullingBounds[2] = D3DXVECTOR3( m_WorldMinBounds.x, m_WorldMaxBounds.y, m_WorldMinBounds.z );
		m_CullingBounds[3] = D3DXVECTOR3( m_WorldMaxBounds.x, m_WorldMaxBounds.y, m_WorldMinBounds.z );
		m_CullingBounds[4] = D3DXVECTOR3( m_WorldMinBounds.x, m_WorldMinBounds.y, m_WorldMaxBounds.z );
		m_CullingBounds[5] = D3DXVECTOR3( m_WorldMaxBounds.x, m_WorldMinBounds.y, m_WorldMaxBounds.z );
		m_CullingBounds[6] = D3DXVECTOR3( m_WorldMinBounds.x, m_WorldMaxBounds.y, m_WorldMaxBounds.z );
		m_CullingBounds[7] = D3DXVECTOR3( m_WorldMaxBounds.x, m_WorldMaxBounds.y, m_WorldMaxBounds.z );

	} // CalculateBounds()

	// axial aligned min and max bounds
	D3DXVECTOR3	m_MinBounds;
	D3DXVECTOR3	m_MaxBounds;

	D3DXVECTOR3	m_WorldMinBounds;
	D3DXVECTOR3	m_WorldMaxBounds;

	D3DXVECTOR3		m_CullingBounds[8];
};


#endif