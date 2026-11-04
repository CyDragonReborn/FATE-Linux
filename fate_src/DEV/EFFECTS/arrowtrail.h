/*
	arrowtrail.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	9/15/2004

*/

#ifndef _ARROWTRAIL_H
#define _ARROWTRAIL_H

#include "d3dx8_shim.h"


#include "../UTILITIES/constants.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/cullingbounds.h"


// position and fade information for arrow trails

class CArrowTrail
{

public:

	CArrowTrail( const D3DXMATRIX& Transformation, 	// worldspace transformation
				 float32 Length ) :					// length of trail
				 m_Transformation( Transformation ),
				 m_Length( Length ),
				 m_Opacity( 255 ),
				 m_pCullingBounds( NULL )
	{
		m_pCullingBounds = new CCullingBounds( D3DXVECTOR3( -1, -1, -1 ),
											   D3DXVECTOR3( 1, 1, Length + 1 ) );
		m_pCullingBounds->Update( m_Transformation );

	}


	~CArrowTrail( void )
	{
		DELETE_SAFELY( m_pCullingBounds );
	}

	
////////////////////////////////////////////////////////////////////////
// MUTATORS
////////////////////////////////////////////////////////////////////////
	const D3DXMATRIX&		Transformation( void )		{	return m_Transformation;	};


	void					Update( float32 TimeElapsed )	// time elapsed in seconds
	{
		m_Opacity -= TimeElapsed * 300;
		if( m_Opacity <= 0 )
		{
			m_Opacity = 0;
		}
	}

////////////////////////////////////////////////////////////////////////
// ACCESSORS
////////////////////////////////////////////////////////////////////////

	float32		Opacity( void )			{	return m_Opacity;		};

	bool		InFrustum( CFrustum& pFrustum )			// frustum to cull to
	{
		// if we're not in the frustum, don't render at all
		if( pFrustum.Intersects( *m_pCullingBounds ) == KFrustumOutside )
		{
			return kFalse;
		}

		return kTrue;
	} 
private:

	float32		m_Length;
	float32		m_Opacity;
	D3DXMATRIX	m_Transformation;

	CCullingBounds*		m_pCullingBounds;

};
#endif