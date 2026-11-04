/*
	levelprop.h

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	7/19/2004

*/

#ifndef _LEVELPROP_H
#define _LEVELPROP_H

#include "d3dx8_shim.h"

#include "template.h"
#include "fmod.h"

class CRefManager;
class CSettings;
class CGenericModel;
class CFrustum;
class CCullingBounds;
class CPropTemplate;
class CPropInstance;
class CMaterial;
class CParticleManager;
class CPositionalAudioManager;
class CPositionalAudio;
class CGlowManager;
class CGlow;

// a level prop contains a model, which is its visual representation,
// as well as data about contents and passability.

class CLevelProp
{


public:

	CLevelProp( LPDIRECT3DDEVICE8 pD3DDevice,	// direct3d device
				CRefManager& pRefManager,		// pointer to the reference manager, for tracking media
				CSettings& pSettings,			// pointer to the settings manager, for keeping track of card/machine capabilities
				CParticleManager& pParticleManager,	// particle manager
			    CGlowManager& pGlowManager,			// glow manager
				CPositionalAudioManager& pPositionalAudioManager,	// positional audio manager
				CPropTemplate& pTemplate,		// prop template
				const D3DXVECTOR3& Position,	// worlddspace position
				float32 Scale,					// scale of prop
				float32 Angle );				// angle of prop
	~CLevelProp( void );
	
	void		Render( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
						CLightManager* pLightManager,			// light manager - null for no light manager
					    const D3DXMATRIX& ViewportMatrix,		// viewport matrix
						CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		RenderShadow( LPDIRECT3DDEVICE8 pD3DDevice,			// direct3d device
							  const D3DXMATRIX& ViewportMatrix,		// viewport matrix
							  CFrustum* pFrustum );					// frustum to cull with - NULL for none

	void		Update( float32 TimeElapsed );	// time elapsed in seconds

	void		UpdateProximity( const D3DXVECTOR3& Position );	// position of nearby mobile object

	////////////////////////////////////////////////////////////////////////////////////////////
	//ACCESSORS
	////////////////////////////////////////////////////////////////////////////////////////////

	bool				Structural( void )			{	return m_Structural;		};

	bool				Onscreen( void )			{	return m_Onscreen;			};

	bool				CastsShadow( void )			{	return m_Shadow;			};

	CCullingBounds*		CullingBounds( void )		{	return m_pCullingBounds;	};

	CGenericModel*		Model( void )				{	return m_pModel;			};

	const D3DXVECTOR3&	Position( void ) const		{	return m_Position;			};

	const D3DXMATRIX&	Orientation( void ) const	{	return m_OrientationMatrix;	};

	const D3DXMATRIX&	Transformation( void ) const{	return m_TransformationMatrix;	};

	bool				Collideable( void )			{	return m_Collideable;		};

	bool				HasLightPool( void )		{	return m_HasLightPool;		};

	CMaterial*			LightPoolMaterial( void )	{	return m_pLightPoolMaterial;	};

	float32				LightPoolScale( void )		{	return m_LightPoolScale;		};
	float32				LightPoolAngle( void )		{	return m_LightPoolAngle;		};

	CFaceList*			FaceList( void )			
	{
		if( !Collideable() )
		{
			return NULL;
		}
		if( m_pCollisionModel != NULL )
		{
			return m_pCollisionModel->FaceList();	
		}
		else
		{
			return m_pModel->FaceList();	
		}
	};

	////////////////////////////////////////////////////////////////////////////////////////////
	//MUTATORS
	////////////////////////////////////////////////////////////////////////////////////////////

	void	SetTransformation( const D3DXMATRIX& Transform );	// transformation matrix

	void	CalculateLocalLight( bool Torchlit,					//  is the level torchlit?
								 const D3DXVECTOR3& FocalPoint,		// point where the camera is directed
							     float32 MaximumDistance );			// maximum distance from camera

	void				SetOnscreen( bool State )		{	m_Onscreen = State;		};

	bool				InFrustum( CFrustum& pFrustum );	// frustum to cull against

private:
	void				BlendRandomIdleAnimation( void );

	void				FindParticles( void );

	void				EmitParticles( void );


	D3DXVECTOR3			m_Position;

	D3DXVECTOR3			m_LocalLight;

	D3DXMATRIX			m_OrientationMatrix;
	D3DXMATRIX			m_TransformationMatrix;

	CCullingBounds*		m_pCullingBounds;

	CRefManager&		m_pRefManager;
	CSettings&			m_pSettings;

	CParticleManager&	m_pParticleManager;
	CGlowManager&		m_pGlowManager;

	CPositionalAudioManager&	m_pPositionalAudioManager;

	CGenericModel*		m_pModel;
	CModel*				m_pCollisionModel;

	bool				m_Collideable;
	bool				m_Shadow;

	CMaterial*			m_pLightPoolMaterial;
	bool				m_HasLightPool;
	ELightPoolType		m_LightPoolType;
	float32				m_LightPoolRadius;
	float32				m_LightPoolScale;
	float32				m_LightPoolAngle;

	float32				m_TorchLightFlicker;

	float32				m_ActivationRadius;
	float32				m_DeactivationRadius;

	bool				m_Onscreen;

	bool				m_CanOpen;
	bool				m_CanClose;
	bool				m_Open;

	bool				m_ObjectNearby;

	bool				m_Structural;

	std::vector< CGlow* >		m_pGlowList;
	std::vector< CGlow* >		m_pModelGlowList;
	std::vector< uint32 >		m_GlowIndex;
	std::vector< D3DXVECTOR3 >	m_GlowPosition;

	std::vector< D3DXVECTOR3 >	m_ParticlePosition;
	std::vector< D3DXMATRIX >	m_ParticleOrientation;
	std::vector< int32 >		m_ParticleType;

	std::vector< uint32 >		m_TagParticleIndex;
	std::vector< int32 >		m_TagParticleType;

	CPositionalAudio*		m_pAmbientPositional;

	FSOUND_SAMPLE*			m_pActivationSound;
	FSOUND_SAMPLE*			m_pDeactivationSound;

	uint32				m_BaseSoundVolume;
	float32				m_BaseSoundRadius;

	std::vector< uint32 >		m_IdleAnimations;

};

#endif