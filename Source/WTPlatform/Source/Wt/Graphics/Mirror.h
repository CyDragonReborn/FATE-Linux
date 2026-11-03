/*
	Mirror.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	8/29/2005
*/
#pragma once

#include "WT/Graphics/RenderToTexture.h"
#include "WT/Graphics/Material.h"

namespace WT
{
namespace Graphics
{

class CScene;

//An object that generates a texture showing the scene from the point of view of a virtual camera.
//The resulting texture can be used to render a planar reflection. Inter-reflections between
//mirrors on different planes are not supported.
typedef System::SmartPointer<class CMirror> HMirror;
class CMirror : public CRenderToTexture
{
public:
	WT_CLASS_NO_CREATOR( CMirror, CRenderToTexture );

	CMirror( LPDIRECT3DDEVICE9 pDevice, int32 Resolution, D3DCOLOR BackgroundColor, Math::CPlane const& Plane );
	virtual ~CMirror() {}

	virtual HRenderContext CreateRenderContext() const;

	Math::CPlane const& GetPlane() const {return m_Plane;}
	void SetPlane( Math::CPlane const& Plane ) {m_Plane = Plane;}

	//Update the texture to be the reflection of the specified real camera's view.
	virtual void Update( CCamera const& RealCamera );

	//Get texture transform - useful for applying the resultant render texture
	//in combination with a D3DTSS_TEXCOORDINDEX of D3DTSS_TCI_CAMERASPACEPOSITION
	//and D3DTSS_TEXTURETRANSFORMFLAGS of D3DTTFF_COUNT3 | D3DTTFF_PROJECTED.
	Math::CMatrix4f const& GetTextureTransform() const {return m_TextureTransform;}

	//Get and Set whether a clip plane is used to prevent objects located behind the mirror
	//from appearing in the reflected texture. The initial value for new CMirror's is false. This 
	//flag should be enabled ideally but it does lower performance.
	bool GetClipPlaneEnabled() const {return m_bClipPlaneEnabled;}
	void SetClipPlaneEnabled( bool bClipPlaneEnabled ) {m_bClipPlaneEnabled = bClipPlaneEnabled;}

protected:
	void UpdateInternal( CCamera const& RealCamera );

protected:
	Math::CPlane		m_Plane;
	Math::CMatrix4f		m_TextureTransform;
	bool				m_bClipPlaneEnabled : 1;
};

typedef System::SmartPointer<class CRenderContextMirror> HRenderContextMirror;
class CRenderContextMirror : public CRenderContext
{
public:
	WT_CLASS_NO_CREATOR( CRenderContextMirror, CRenderContext );

	CRenderContextMirror( CCamera* pCamera = NULL, ERenderCategory::Type RenderCategory = ERenderCategory::Opaque, 
		bool Translucent = false )
		: CRenderContext( pCamera, RenderCategory, Translucent )
	{
	}

	virtual bool ShouldRender( IRenderable const& Renderable ) const
	{
		return Renderable.GetCastsReflections();
	}
};

//A material that renders a base reflection texture.
typedef System::SmartPointer<class CReflectionMaterial> HReflectionMaterial;
class CReflectionMaterial : public CMaterial
{
public:
	WT_CLASS_NO_CREATOR( CReflectionMaterial, CMaterial );

	CReflectionMaterial( CMirror const& mirror )
		: m_Mirror( mirror )
	{
	}

	~CReflectionMaterial()
	{
	}

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;

protected:
	CMirror const& m_Mirror;

private:
	void operator=( CReflectionMaterial const& ); //Do not use
};

//A material that renders a base reflection texture and, on top of this, a blend texture that obscures
//the reflection texture based on alpha.
typedef System::SmartPointer<class CBlendedReflectionMaterial> HBlendedReflectionMaterial;
class CBlendedReflectionMaterial : public CReflectionMaterial
{
public:
	WT_CLASS_NO_CREATOR( CBlendedReflectionMaterial, CReflectionMaterial );

	CBlendedReflectionMaterial( CMirror const& mirror, LPDIRECT3DBASETEXTURE9 pBlendTexture ); 

	~CBlendedReflectionMaterial()
	{
		SafeRelease( m_pBlendTexture );
	}

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& /*Context*/, int32 PassIndex ) const;

private:
	LPDIRECT3DBASETEXTURE9 m_pBlendTexture;

private:
	void operator=( CBlendedReflectionMaterial const& ); //Do not use
};

} //Graphics
} //WT

#include "WT/Graphics/Scene.h"
