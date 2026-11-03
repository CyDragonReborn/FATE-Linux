/*
	datalight.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	2/26/2004

	CHANGE HISTORY

	7/6/2004	-	Copy constructor added.
					Range is now a parameter - all media files updated as a result.

*/


#ifndef _DATALIGHT_H
#define _DATALIGHT_H


#include <d3dx8.h>

#include "../UTILITIES/utilities.h"

const enum ELightType
{
	KLightOmni,
	KLightTargetSpot,
	KLightDirectional,
	KLightFreeSpot,
	KLightTargetDirectional
};

// a named light  models and scenes may have 'lights' inside them
// these are usually used for actual light placement in the d3d scene.

class CDataLight
{
public:
	CDataLight( std::string Name,			// name of light
			ELightType	LightType,			// type of light
			float32 Range,					// range ( only applicable for points and spots )
			const D3DXVECTOR3& Position,			// position in space
			const D3DXVECTOR3& Direction,			// direction of light
			const D3DXVECTOR3& Color ) :			// color of the light RGB range of 0-1
											m_Name( Name ),
											m_LightType( LightType ),
											m_Range ( Range ),
											m_Position( Position ),
											m_Direction( Direction ),
											m_Color( Color ),
											m_UserData ( "" )
	{
		ZeroMemory ( &m_D3DLight, sizeof ( m_D3DLight ));
		Create();
	}; // CDataLight()

	CDataLight( CDataLight* pLight ) : // light to copy
										m_Name( pLight->Name() ),
										m_Range( pLight->Range() ),
										m_LightType( pLight->Type() ),
										m_Position( pLight->Position() ),
										m_Direction( pLight->Direction() ),
										m_Color( pLight->Color() ),
										m_UserData ( "" )
	{
		ZeroMemory ( &m_D3DLight, sizeof ( m_D3DLight ));
		Create();
	}; // CDataLight()

	~CDataLight( void )
	{
	}
	
	const std::string&	Name( void )		{	return m_Name;		};

	ELightType			Type( void )		{	return m_LightType;		};
	
	const D3DXVECTOR3&	Position( void )	{	return m_Position;	};
	const D3DXVECTOR3&	Direction( void )	{	return m_Direction;	};
	const D3DXVECTOR3&	Color( void )		{	return m_Color;		};

	void				SetPosition( const D3DXVECTOR3& Value )		{	m_Position = Value;		};
	void				SetDirection( const D3DXVECTOR3& Value )	{	m_Direction = Value; D3DXVec3Normalize( &m_Direction, &m_Direction );	};
	void				SetColor( const D3DXVECTOR3& Value )		{	m_Color = Value;		};

	float32				Range( void )		{	return m_D3DLight.Range;		};
	
	D3DLIGHT8&			D3DLight( void )	{	return m_D3DLight;	};

	void				SetD3DLight( const D3DLIGHT8& Light )	{	m_D3DLight = Light;	};

	const std::string&	UserData( void )						{	return m_UserData;	};
	void				SetUserData( const std::string& Data )	{	m_UserData = Data;	};

private:

	void			Create( void )
	{
		ZeroMemory( &m_D3DLight, sizeof( D3DLIGHT8 ) );

		switch( m_LightType )
		{
		default:
		case KLightOmni :
			m_D3DLight.Type = D3DLIGHT_POINT;
			m_D3DLight.Range = m_Range;// * 1000;
			m_D3DLight.Position = m_Position;
			m_D3DLight.Attenuation0 = 1.0f;
			break;
		case KLightFreeSpot :
			m_D3DLight.Type = D3DLIGHT_SPOT;
			m_D3DLight.Range = m_Range;
			m_D3DLight.Position = m_Position;
			D3DXVec3Normalize( &m_Direction, &m_Direction );
			m_D3DLight.Direction = m_Direction;
            m_D3DLight.Theta        = 0.5f;
            m_D3DLight.Phi          = 1.0f;
            m_D3DLight.Falloff      = 1.0f;
            m_D3DLight.Attenuation0 = 1.0f;
			break;
		case KLightTargetSpot :
			m_D3DLight.Type = D3DLIGHT_SPOT;
			m_D3DLight.Range = m_Range;
			m_D3DLight.Position = m_Position;
			D3DXVec3Normalize( &m_Direction, &m_Direction );
			m_D3DLight.Direction = m_Direction;
            m_D3DLight.Theta        = 0.5f;
            m_D3DLight.Phi          = 1.0f;
            m_D3DLight.Falloff      = 1.0f;
            m_D3DLight.Attenuation0 = 1.0f;
			break;
		case KLightDirectional :
		case KLightTargetDirectional :
			m_LightType = KLightDirectional;
			m_D3DLight.Type = D3DLIGHT_DIRECTIONAL;
			D3DXVec3Normalize( &m_Direction, &m_Direction );
			m_D3DLight.Direction = m_Direction;
			break;

		}

		m_D3DLight.Falloff	= 1.0f;

		m_D3DLight.Diffuse.r = m_Color.x;
		m_D3DLight.Diffuse.g = m_Color.y;
		m_D3DLight.Diffuse.b = m_Color.z;

		m_D3DLight.Specular.r = m_Color.x;
		m_D3DLight.Specular.g = m_Color.y;
		m_D3DLight.Specular.b = m_Color.z;
	}

	std::string		m_UserData;

	D3DLIGHT8		m_D3DLight;

	ELightType		m_LightType;

	std::string		m_Name;

	D3DXVECTOR3		m_Position;
	D3DXVECTOR3		m_Direction;
	D3DXVECTOR3		m_Color;

	float32			m_Range;
};


#endif