/*
	camera.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	1/27/2004

	Change History :

	Pete Siamidis
	5/27/2004	-	added 'elasticity' for camera movement

*/

#include <D3D8.h>
#include <d3dx8.h>
#include <assert.h>

#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/frustum.h"
#include "camera.h"

CCamera::CCamera( CSettings& pSettings ) :	//we need this for calculating the projection matrix
				  m_pSettings( pSettings ),
				  m_ProjectionAspect( 0 ),
				  m_ProjectionNearClip( 0 ),
				  m_ProjectionFarClip( 0 ),
				  m_Position( 0, 1, -1 ),
				  m_LookTarget( 0, 0, 0 ),
				  m_CameraOffset( 0, 1, -1 ),
				  m_Up( 0, 1, 0 ),
				  m_Dolly( 100 ),
				  m_TargetOrbit( 0 ),
				  m_ViewType( KViewFull ),
				  m_ProjectionFOV ( 0 )
{
	ZeroMemory ( &m_ViewFrustum,		sizeof( m_ViewFrustum ));
	ZeroMemory ( &m_ViewMatrix,			sizeof( m_ViewMatrix ));
	ZeroMemory ( &m_ProjectionMatrix,	sizeof( m_ProjectionMatrix ));
	ZeroMemory ( &m_Orientation,		sizeof( m_Orientation ));

	D3DXMatrixIdentity( &m_SwingOrientation );
	// keep the projection matrix in sync with the settings
	UpdateProjection();

	UpdateViewMatrix();

} // CCamera::CCamera()

CCamera::~CCamera( void )
{
} // CCamera::~CCamera()

// keeps the view matrix in sync with position and look target
void CCamera::UpdateViewMatrix( void )
{
	D3DXMatrixLookAtLH( &m_ViewMatrix,
						&m_Position,
						&m_LookTarget,
						&m_Up );

	CalculateTransformationMatrix();
} // CCamera::UpdateViewMatrix()

// calculate an orientation matrix for the camera (treating it like an object)
// from its view matrix
void CCamera::CalculateTransformationMatrix( void )
{
	D3DXMatrixTranspose( &m_Orientation, &m_ViewMatrix );
	// remove translation information (position)
	m_Orientation._41 = 0;
	m_Orientation._42 = 0;
	m_Orientation._43 = 0;
	m_Orientation._14 = 0;
	m_Orientation._24 = 0;
	m_Orientation._34 = 0;

} // CCamera::CalculateTransformationMatrix()

void CCamera::Update( float32 TimeElapsed ) // time elapsed, in seconds
{
	D3DXVECTOR3 Offset( m_CameraOffset );
	D3DXVec3Normalize( &Offset, &Offset );
	Offset *= m_Dolly;

	D3DXMATRIX TargetOrientation;
	D3DXMatrixRotationY( &TargetOrientation, (float32)KDeg2Rad * m_TargetOrbit );

	D3DXMATRIX InverseMatrix;
	D3DXMatrixInverse( &InverseMatrix, NULL, &m_SwingOrientation );

	// bring our target orientation into local space of the current orientation
	D3DXMATRIX LocalTargetMatrix = InverseMatrix * TargetOrientation;

	// now we can calculate a rotation quaternion to get to this new orientation
	D3DXQUATERNION Current;
	D3DXQuaternionIdentity( &Current );

	D3DXQUATERNION Target;
	D3DXQuaternionRotationMatrix( &Target, &LocalTargetMatrix );

	float32 Distance = D3DXQuaternionLength( &Target );

	Distance = 10.0f * TimeElapsed 
				/ fabs( Distance );
	if( Distance > 1 )
	{
		Distance = 1;
	}

	// slerp it by how far we want to turn this frame
	D3DXQuaternionSlerp( &Current, 
						 &Current, 
						 &Target,
						 Distance );

	// bring it back into matrix form
	D3DXMatrixRotationQuaternion( &LocalTargetMatrix, &Current );

	// and apply it to our current orientation to swing toward the target
	m_SwingOrientation = m_SwingOrientation * LocalTargetMatrix;

	D3DXVec3TransformNormal( &Offset, &Offset, &m_SwingOrientation );

	m_Position = Offset + m_LookTarget;


	// keep the projection matrix in sync with the settings
	UpdateProjection();

	// calculate the view matrix, based upon location, target, and up vector
	UpdateViewMatrix();

	m_ViewFrustum.Update( m_ViewMatrix, m_ProjectionMatrix );

} // CCamera::Update()

void CCamera::UpdateFixed( float32 TimeElapsed )
{
	// keep the projection matrix in sync with the settings
	UpdateProjection();

	// calculate the view matrix, based upon location, target, and up vector
	UpdateViewMatrix();

	m_ViewFrustum.Update( m_ViewMatrix, m_ProjectionMatrix );
} // CCamera::UpdateFixed()

void CCamera::UpdateProjection( void )
{
	float32	Aspect	= ( float32 )m_pSettings.GetSettings( KSetScreenWidth ) /
					  ( float32 )m_pSettings.GetSettings( KSetScreenHeight );

	if( m_ViewType != KViewFull )
	{
		Aspect	= ( float32 )m_pSettings.GetSettings( KSetScreenWidth ) * .5f /
				  ( float32 )m_pSettings.GetSettings( KSetScreenHeight );
	}

	float32 FOV		= ( float32 )( m_pSettings.GetSettings( KSetFOV ) * KDeg2Rad );

	float32 Near	= (float32)m_pSettings.GetSettings( KSetNearClip );
	float32 Far		= (float32)m_pSettings.GetSettings( KSetFarClip );

	if( m_ProjectionAspect != Aspect ||
		m_ProjectionNearClip != Near ||
		m_ProjectionFarClip != Far ||
		m_ProjectionFOV != FOV )
	{
		m_ProjectionAspect = Aspect;
		m_ProjectionNearClip = Near;
		m_ProjectionFarClip = Far;
		m_ProjectionFOV = FOV;

		D3DXMatrixPerspectiveFovLH( &m_ProjectionMatrix,
									m_ProjectionFOV,
									m_ProjectionAspect,
									m_ProjectionNearClip,
									m_ProjectionFarClip );
	}
}
