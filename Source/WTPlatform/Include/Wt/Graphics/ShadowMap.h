/***************************************************************************************
 *
 * ShadowMap.h
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          1/24/2006
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/Mesh/Mesh.h"
#include "WT/Graphics/RenderToTexture.h"
#include "WT/Graphics/Camera.h"

namespace WT
{
namespace Graphics
{

class CScene;

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

/*
A CShadowMap is a render-to-texture object used for shadow mapping techniques.
*/
typedef System::SmartPointer<class CShadowMap> HShadowMap;
class CShadowMap : public CRenderToTexture
{
public:
	WT_CLASS_NO_CREATOR( CShadowMap, CRenderToTexture );

	static int32 const KTextureStage = 7; //Texture stage used to apply shadow map to scene

protected:
	CShadowMap( LPDIRECT3DDEVICE9 pDevice, int32 Width = 1024, int32 Height = -1 );

public:
	virtual ~CShadowMap();

	//Check whether the explicitly specified hull is used (instead of an
	//automatically computed hull). The default is false.
	bool GetUseExplicitHull() const {return m_UseExplicitHull;}

	//Set whether the explicitly specified hull is used (instead of an
	//automatically computed hull).
	void SetUseExplicitHull( bool UseExplicitHull ) {m_UseExplicitHull = UseExplicitHull;}

	//Get the explicit hull, which can be selected for use by calling SetUseExplicitHull( true ).
	//The explicit hull is in world coordinates and is used to focus the shadow map.
	std::vector<CVector3f> const& GetExplicitHull() const {return m_ExplicitHull;}
	std::vector<CVector3f>& GetExplicitHull() {return m_ExplicitHull;}
    
	virtual void Update( CCamera const& RealCamera, CScene const& Scene );

	virtual void BeginShadows( CCamera const& RealCamera ) const;
	virtual void EndShadows() const;

	//Render an overlay onto the screen showing the shadow map contents
	void RenderOverlay( CRenderContext const& Context );

protected:
	void UpdateInternal( CCamera const& RealCamera, CScene const& Scene );

private:
	//Per-device shared CShadowMap data
	struct SharedData
	{
		SharedData( LPDIRECT3DDEVICE9 pDevice );
		~SharedData();

		int32 m_RefCount; //Number of CShadowMap objects depending on this data

		LPDIRECT3DDEVICE9		m_pDevice;
		LPDIRECT3DPIXELSHADER9	m_pShowMapPS;
	};

	std::vector<CVector3f>	m_ExplicitHull;
	HMesh					m_hOverlay;
	SharedData*				m_pSharedData;
	bool					m_UseExplicitHull : 1;

	typedef std::map<LPDIRECT3DDEVICE9, SharedData*> SharedDataMap; 
	static SharedDataMap gm_SharedDataMap;
};

//CShadowMapDirectional is a render-to-texture object used to compute and apply shadow maps
//based on a directional light.
typedef System::SmartPointer<class CShadowMapDirectional> HShadowMapDirectional;
class CShadowMapDirectional : public CShadowMap
{
public:
	WT_CLASS_NO_CREATOR( CShadowMapDirectional, CShadowMap );

	CShadowMapDirectional( LPDIRECT3DDEVICE9 pDevice, int32 Width = 1024, int32 Height = -1 );

	virtual ~CShadowMapDirectional();

	//Create a new context that's suggested for use when rendering to this texture.
	virtual HRenderContext CreateRenderContext() const;

	virtual void Update( CCamera const& RealCamera, CScene const& Scene );

	virtual void BeginShadows( CCamera const& RealCamera ) const;

	CVector3f const& GetLightDir() const {return m_LightDir;}
	void SetLightDir( CVector3f const& LightDir ) {m_LightDir = LightDir;}

	float32 GetZBias() const {return m_ZBias;}
	void SetZBias( float32 ZBias ) {m_ZBias = ZBias;}

	CVector4f const& GetLightDiffuse() const {return m_LightDiffuse;}

	//Set the light diffuse 4-vector. The components are typically in the [0.0,1.0] range.
	//The w component, representing alpha, should be large (e.g. 1.0) to avoid ALPHATEST
	//failures.
	void SetLightDiffuse( CVector4f const& LightDiffuse ) {m_LightDiffuse = LightDiffuse;}

	void SetShadowWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CMatrix4f const& ModelToWorld ) const;
	void SetSceneWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CCamera const& RealCamera, CMatrix4f const& ModelToWorld ) const;
	void SetSceneTextured( bool Textured ) const;

	static CVector3f const& gm_DefaultLightDir;
	static float32 const gm_DefaultZBias;

protected:
	void UpdateInternal( CCamera const& RealCamera, CScene const& Scene );

private:
	//Per-device shared CShadowMap data
	struct SharedData
	{
		SharedData( LPDIRECT3DDEVICE9 pDevice );
		~SharedData();

		int32 m_RefCount; //Number of CShadowMapDirectional objects depending on this data

		LPDIRECT3DDEVICE9		m_pDevice;

		LPDIRECT3DVERTEXSHADER9	m_pShadowMapVS;
		LPDIRECT3DPIXELSHADER9	m_pShadowMapPS;

		//Shader for rendering diffuse light contributions to the scene
		LPDIRECT3DVERTEXSHADER9	m_pSceneVS;
		LPDIRECT3DPIXELSHADER9	m_pScenePS;
		LPDIRECT3DVERTEXSHADER9	m_pSceneTexVS; //Textured
		LPDIRECT3DPIXELSHADER9	m_pSceneTexPS; //Textured
	};

	//Update the shadow camera. Use uniform shadow projection, based on vertices of the convex 
	//hull of interest.
	void UpdateCameraUniform( std::vector<CVector3f> const& Hull );

	//Update the shadow camera, using the LiSPSM (light space perspective shadow maps) method.
	void UpdateCameraLiSPSM( std::vector<CVector3f> const& Hull, CCamera const& RealCamera );

	//Adjust transform and set optimal roll, FOV angles, far clip, and transform on shadow camera
	void FocusCamera( std::vector<CVector3f> const& Hull, CMatrix4f& ShadowCameraTransform );

	CVector3f				m_LightDir;
	float32					m_ZBias;
	CVector4f				m_LightDiffuse; //Light diffuse coefficients; typically in [0.0,1.0] range
	SharedData*				m_pSharedData;

	typedef std::map<LPDIRECT3DDEVICE9, SharedData*> SharedDataMap;
	static SharedDataMap gm_SharedDataMap;
};

typedef System::SmartPointer<class CRenderContextShadowed> HRenderContextShadowed;
class CRenderContextShadowed : public CRenderContext
{
public:
	WT_CLASS_NO_CREATOR( CRenderContextShadowed, CRenderContext );

	CRenderContextShadowed( CCamera const* pCamera,
		ERenderCategory::Type RenderCategory = ERenderCategory::Opaque, bool Translucent = false )
		: CRenderContext( pCamera, RenderCategory, Translucent )
	{
	}

	virtual bool ShouldRender( IRenderable const& Renderable ) const
	{
		return _BaseClass::ShouldRender( Renderable ) && Renderable.GetReceivesShadows();
	}
};

typedef System::SmartPointer<class CRenderContextUnshadowed> HRenderContextUnshadowed;
class CRenderContextUnshadowed : public CRenderContext
{
public:
	WT_CLASS_NO_CREATOR( CRenderContextUnshadowed, CRenderContext );

	CRenderContextUnshadowed( CCamera const* pCamera,
		ERenderCategory::Type RenderCategory = ERenderCategory::Opaque, bool Translucent = false )
		: CRenderContext( pCamera, RenderCategory, Translucent )
	{
	}

	virtual bool ShouldRender( IRenderable const& Renderable ) const
	{
		return _BaseClass::ShouldRender( Renderable ) && !Renderable.GetReceivesShadows();
	}
};

//A render context for rendering a shadowed scene, using the shadow map
typedef System::SmartPointer<class CRenderContextShadowMapDirectionalScene> HRenderContextShadowMapDirectionalScene;
class CRenderContextShadowMapDirectionalScene : public CRenderContext
{
public:
	WT_CLASS_NO_CREATOR( CRenderContextShadowMapDirectionalScene, CRenderContext );

	CRenderContextShadowMapDirectionalScene( CShadowMapDirectional const& ShadowMap, CCamera const& RealCamera, 
		ERenderCategory::Type RenderCategory = ERenderCategory::Opaque, bool Translucent = false )
		: CRenderContext( &RealCamera, RenderCategory, Translucent )
		, m_ShadowMap( ShadowMap )
	{
	}

	virtual bool ShouldRender( IRenderable const& Renderable ) const
	{
		return _BaseClass::ShouldRender( Renderable ) && Renderable.GetReceivesShadows();
	}

	virtual void SetWorldTransform( LPDIRECT3DDEVICE9 pD3DDevice, CMatrix4f const& Transform ) const;
	virtual void SetTexture( LPDIRECT3DDEVICE9 pDevice, int32 Stage, LPDIRECT3DBASETEXTURE9 pTexture ) const;

	void BeginShadows() const
	{
		m_ShadowMap.BeginShadows( *GetCamera() );
	}

	void EndShadows() const
	{
		m_ShadowMap.EndShadows();
	}

private:
	CShadowMapDirectional const& m_ShadowMap;
};

}
}
