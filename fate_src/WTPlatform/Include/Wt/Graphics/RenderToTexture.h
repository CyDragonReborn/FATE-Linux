/*
	RenderToTexture.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Author:	Gary Hinger
	12/15/2005
*/

#pragma once

#include "d3d9.h"
#include "WT/System/Object.h"
#include "WT/System/Callback.h"
#include "WT/Graphics/IDeviceObject.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/Camera.h"

namespace WT
{
namespace Graphics
{

/*
An object that assists in rendering to a texture. It creates a camera automatically. 
You may wish to call GetCamera().SetNearClip(), SetFarClip(), SetFOV(), SetVerticalFOV() 
and SetTransform() to set the camera's properties to different values than the defaults. 
Call Update() to update the texture, but first make sure you have the necessary callback 
attached to the OnUpdate() event so that your desired IRenderable's are rendered into the 
texture. Call SetTextureToDevice() to safely use the resultant texture for further rendering.
*/
class CRenderToTexture : public System::CObject, public IDeviceObject
{
public:
	WT_BEGIN_CLASS_NO_CREATOR( CRenderToTexture, System::CObject );
		WT_INHERITS_INTERFACE( IDeviceObject );
	WT_END_CLASS();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	//Constructs a render-to-texture object. The contents of the newly created texture are 
	//not initialized.
	CRenderToTexture( LPDIRECT3DDEVICE9 pDevice, int32 Width = 512, int32 Height = -1,
		D3DCOLOR BackgroundColor = 0xffffffffu,
		D3DFORMAT TextureFormat = D3DFMT_A8R8G8B8, 
		DWORD Usage = D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP,
		D3DFORMAT DepthStencilFormat = D3DFMT_D24S8 );
	virtual ~CRenderToTexture();

	LPDIRECT3DDEVICE9 GetDevice() const {return m_pDevice;}

	int32 GetWidth() const {return m_Width;}
	int32 GetHeight() const {return m_Height;}

	D3DCOLOR GetBackgroundColor() const {return m_BackgroundColor;}
	void SetBackgroundColor( D3DCOLOR BackgroundColor ) {m_BackgroundColor = BackgroundColor;}

	CCamera& GetCamera() {return m_Camera;}
	CCamera const& GetCamera() const {return m_Camera;}

	int32 GetEnabled() const {return m_Enabled;}
	void SetEnabled( bool Enabled ) {m_Enabled = Enabled;}

	//Create a new context that's suggested for use when rendering to this texture.
	virtual HRenderContext CreateRenderContext() const;
	
	//Update the texture. This performs its own D3D BeginScene() and EndScene() calls. You should attach 
	//the necessary callback(s) for rendering IRenderable's to the OnUpdate() event.
	virtual void Update();

	//Set the texture to be the specified color.
	void SetColor( D3DCOLOR Color );

	//An event that occurs when the texture needs to be rendered to. You should attach a callback that 
	//renders appropriate IRenderable's using the supplied CCamera.	You should not make your own
	//D3D BeginScene() and EndScene() calls.
	typedef System::CCallback1<CRenderToTexture const&>	UpdateCallback;
	typedef UpdateCallback::Event		UpdateEvent;
	UpdateEvent& OnUpdate() {return m_UpdateEvent;}

	//This sets the device to use the rendered-to-texture for the specified stage.
	//If the texture is being rendered to, then a NULL texture will be substituted
	//to avoid potential D3D warnings and corruption.
	void SetTextureToDevice( CRenderContext const& Context, DWORD Stage ) const;

	//Check if this render-to-texture is the current render target for the device.
	//This handles cases where the render-to-texture has become the render target for the 
	//device due to member functions such as Update() and SetColor(), not through manual 
	//manipulation of the D3D resources.
	bool InUse() const {return m_InUse;}

	//Get the texture. This texture should not be used while it is selected as the 
	//current render target.
	LPDIRECT3DTEXTURE9 GetTexture() {return m_hTexture;}

	//Get the surface. This surface should not be used while it is selected as the 
	//current render target.
	LPDIRECT3DSURFACE9 GetSurface() {return m_hSurface;}

	//Get the depth stencil surface. This surface should not be used while it is selected 
	//as the current render target.
	LPDIRECT3DSURFACE9 GetDepthStencilSurface() {return m_hDepthStencilSurface;}

	Math::CMatrix4f GetWorldToTexture() const {return m_Camera.GetWorldToScreen() * GetScreenToTexture();}
	Math::CMatrix4f GetViewToTexture() const {return m_Camera.GetViewToScreen() * GetScreenToTexture();}
	Math::CMatrix4f GetScreenToTexture() const;

protected:
	void InitDeviceObjects( LPDIRECT3DDEVICE9 pDevice );

	void UpdateInternal();

protected:
	int32				m_Width;
	int32				m_Height;
	D3DFORMAT			m_TextureFormat;
	DWORD				m_Usage;
	D3DFORMAT			m_DepthStencilFormat;
	D3DCOLOR			m_BackgroundColor;
	UpdateEvent			m_UpdateEvent;
	LPDIRECT3DDEVICE9	m_pDevice;
	LPDIRECT3DTEXTURE9	m_hTexture;
	LPDIRECT3DSURFACE9  m_hSurface;
	LPDIRECT3DSURFACE9	m_hDepthStencilSurface;
	CCamera				m_Camera;
	bool				m_ResourcesValid : 1;
	bool				m_Enabled : 1; //Whether this object updates
	bool				m_InUse   : 1;
};

} //Graphics
} //WT
