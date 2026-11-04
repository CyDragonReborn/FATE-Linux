/*
	Scene.h

	Copyright (C) 2005 WildTangent, Inc. 
	All Rights Reserved

	Ira Small
	10/20/2005
*/

#pragma once

// -------------------------------------------------------------------------------------
// Include Files
// -------------------------------------------------------------------------------------
#include "WT/System/Collections/ObjectCollection.h"
#include "WT/Graphics/IUpdateable.h"
#include "WT/Graphics/Camera.h"
#include "WT/Graphics/Mirror.h"
#include "WT/Graphics/ShadowMap.h"

namespace WT
{
namespace Graphics
{

// -------------------------------------------------------------------------------------
// Interfaces
// -------------------------------------------------------------------------------------

typedef System::SmartPointer<class CScene> HScene;
class CScene : public WT::System::Collections::CObjectCollection, public WT::System::Serialization::ISerializable, 
	public CRenderableImpl, public IUpdateable, public IDeviceObject
{
public:
	WT_BEGIN_CLASS( CScene, WT::System::Collections::CObjectCollection );
		WT_INHERITS_INTERFACE(WT::System::Serialization::ISerializable);
		WT_INHERITS_INTERFACE( IRenderable );
		WT_INHERITS_INTERFACE( IUpdateable );
		WT_INHERITS_INTERFACE( IDeviceObject );
	WT_END_CLASS();

	CScene();
	CScene( LPDIRECT3DDEVICE9 pDevice );
	virtual ~CScene();

	LPDIRECT3DDEVICE9 GetDevice() const {return m_pDevice;}

	virtual void Render( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context );

	virtual void Update( float32 ElapsedTimeSeconds );

	virtual void AddObject( System::CObject& Object );

	virtual void SetActorDefaultMotion( uint32 Index, string MotionName );
	virtual string GetActorDefaultMotion( uint32 Index ) const;

	virtual void SetActorDefaultLooping( uint32 Index, bool Looping );
	virtual bool GetActorDefaultLooping( uint32 Index ) const;

	virtual void SetActorTimeIntoMotion( uint32 Index, float32 TimeIntoMotion );
	virtual float32 GetActorTimeIntoMotion( uint32 Index ) const;

	//Update all of the reflection managed by the scene. The update reflection textures
	//are applicable to the specified camera.
	void RenderToTextures( CCamera const& RealCamera );

	//Get the mirror update event object. This event occurs whenever the reflection textures need updating.
	//Typically, this is used as follows:
	//You should have a class such as CMyGame with a member function
	//void CMyGame::RenderToTexture( CRenderToTexture& RenderToTexture ).
	//Then you attach your callback to the scene using
	//m_pScene->OnRenderToTexture() += CRenderToTexture::UpdateCallback::Create( *m_pMyApp, &CMyGame::RenderToTexture );
	//Your RenderToTexture callback is responsible for rendering all game objects (e.g. opaque scene 
	//elements, translucent scene elements, special effects, etc.).
	CRenderToTexture::UpdateEvent& OnRenderToTexture() {return m_RenderToTextureEvent;}

	//Apply this scene's render-to-texture event to the specified render-to-texture object.
	//This is used internally for render-to-texture objects managed by the scene, but can also 
	//be added as a callback to render-to-texture objects that aren't managed directly by the scene.
	void RenderToTexture( CRenderToTexture const& RenderToTexture );

	//Get the number of mirrors managed by the scene object.
	uint32 GetMirrorCount() const {return static_cast<uint32>( m_hMirrors.size() );}
	CMirror* GetMirror( int32 i ) const {return m_hMirrors[i];}

	uint32 GetShadowMapCount() const {return static_cast<uint32>( m_hShadowMaps.size() );}
	CShadowMap* GetShadowMap( int32 i ) const {return m_hShadowMaps[i];}
	
	void AddShadowMap( CShadowMap* pShadowMap );
	void ClearShadowMaps();

	//Get an existing mirror for the specified plane, or create one if necessary.
	//The mirror will be owned and managed by the scene object.
	CMirror* AddMirror( CPlane const& Plane );
	void ClearMirrors();

	D3DCOLOR const& GetBackgroundColor() {return m_BackgroundColor;}

	void SetBackgroundColor( D3DCOLOR const& BackgroundColor );

	//
	// IDeviceObject interface
	//
	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	//
	// ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization( WT::System::Serialization::CSerializationInfo* SerialInfo, WT::System::Serialization::CSerializationContext* sContext );
	virtual void GetObjectData( WT::System::Serialization::CSerializationInfo* SerialInfo, WT::System::Serialization::CSerializationContext* sContext ) const;
	virtual void SetObjectData( WT::System::Serialization::CSerializationInfo* SerialInfo, WT::System::Serialization::CSerializationContext* sContext );

private:
	void Clear();

private:
	vector< string >	m_ActorDefaultMotions;
	vector< bool >		m_ActorDefaultLooping;
	vector< float32 >	m_ActorDefaultTimeIntoMotion;

	LPDIRECT3DDEVICE9 m_pDevice;

	std::vector<HMirror>			m_hMirrors;
	std::vector<HShadowMap>			m_hShadowMaps;
	CRenderToTexture::UpdateEvent	m_RenderToTextureEvent;

	D3DCOLOR m_BackgroundColor;

	bool m_ResourcesValid : 1;
};

}
}
