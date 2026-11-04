/*
	camera.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

*/


#ifndef _CAMERA_H
#define _CAMERA_H

#include "d3dx8_shim.h"

#include "../UTILITIES/frustum.h"

class	CSettings;

const enum EViewType
{
	KViewCovered,
	KViewLeft,
	KViewRight,
	KViewFull
};

// this is just a simple object for maintaining a camera's position, orientation,
// view frustum, and attendant matrices
class CCamera
{
public:
				CCamera( CSettings& pSettings );	//we need this for calculating the projection matrix
				~CCamera( void );

	void		Update( float32 TimeElapsed );		// time elapsed, in seconds
	
	void		UpdateFixed( float32 TimeElapsed );		// time elapsed, in seconds

	CFrustum&			Frustum( void )				{	return	m_ViewFrustum;	};

	const D3DXMATRIX&	ProjectionMatrix( void )	{	return	m_ProjectionMatrix;	};
	const D3DXMATRIX&	ViewMatrix( void )			{	return	m_ViewMatrix;	};
	const D3DXMATRIX&	Orientation( void )			{	return	m_Orientation;	};

	const D3DXVECTOR3&	Position( void )			{	return	m_Position;		};
	const D3DXVECTOR3&	LookTarget( void )			{	return	m_LookTarget;	};
	const D3DXVECTOR3&	Offset( void )				{	return	m_CameraOffset;	};
	const D3DXVECTOR3&	Up( void )					{	return	m_Up;			};

	void		SetPosition( const D3DXVECTOR3& Position )		{	m_Position = Position;		};
	void		SetLookTarget( const D3DXVECTOR3& Target )		{	m_LookTarget = Target;		};
	void		SetCameraOffset( const D3DXVECTOR3& Offset )	{	m_CameraOffset = Offset;	};
	void		SetUp( const D3DXVECTOR3& Up )					{	m_Up = Up;					};

	void		SetDolly( float32 Dolly )						{	m_Dolly = Dolly;			};

	void		SetTargetOrbit( float32 Orbit )					{	m_TargetOrbit = Orbit;		};

	void		SetViewType( EViewType ViewType )				{	m_ViewType = ViewType;		};

	EViewType	ViewType( void )								{	return m_ViewType;			};

	void		UpdateViewMatrix( void );

	void		UpdateProjection( void );

private:
	void		CalculateTransformationMatrix( void );
	
	CFrustum	m_ViewFrustum;
	CSettings&	m_pSettings;


	D3DXMATRIX	m_ViewMatrix;
	D3DXMATRIX	m_ProjectionMatrix;
	D3DXMATRIX	m_Orientation;

	D3DXMATRIX  m_SwingOrientation;

	D3DXVECTOR3	m_Position;
	D3DXVECTOR3	m_LookTarget;
	D3DXVECTOR3 m_CameraOffset;
	D3DXVECTOR3	m_Up;

	float32		m_ProjectionAspect;
	float32		m_ProjectionNearClip;
	float32		m_ProjectionFarClip;
	float32		m_ProjectionFOV;

	float32		m_Dolly;

	float32		m_TargetOrbit;

	EViewType	m_ViewType;
};

#endif