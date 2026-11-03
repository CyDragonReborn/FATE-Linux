/***************************************************************************************
 *
 * DxMaterial.h
 *
 * Copyright (C) 2005 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          10/14/2005
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

#include "WT/Graphics/Material.h"
#include "WT/System/UserPropertyCollection.h"

namespace WT
{
namespace Graphics
{

typedef System::SmartPointer<class CDxMaterial> HDxMaterial;
class CDxMaterial : public CMaterial, public System::Serialization::ISerializable
{
public:
	WT_BEGIN_CLASS( CDxMaterial, CMaterial )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable )
	WT_END_CLASS()

	//Enumeration of values corresponding to typical D3DX effect global parameters
	enum EParameterType
	{
		KParameterTypeTime,
		KParameterTypeWorld,
		KParameterTypeWorldT,
		KParameterTypeWorldI,
		KParameterTypeWorldIT,
		KParameterTypeView,
		KParameterTypeViewT,
		KParameterTypeViewI,
		KParameterTypeViewIT,
		KParameterTypeProjection,
		KParameterTypeProjectionT,
		KParameterTypeProjectionI,
		KParameterTypeProjectionIT,
		KParameterTypeWorldView,
		KParameterTypeWorldViewT,
		KParameterTypeWorldViewI,
		KParameterTypeWorldViewIT,
		KParameterTypeViewProjection,
		KParameterTypeViewProjectionT,
		KParameterTypeViewProjectionI,
		KParameterTypeViewProjectionIT,
		KParameterTypeWorldViewProjection,
		KParameterTypeWorldViewProjectionT,
		KParameterTypeWorldViewProjectionI,
		KParameterTypeWorldViewProjectionIT,
		KParameterTypeWorldCameraPosition,
		KParameterTypeCount, //Number of parameter types
	};

	CDxMaterial();
	~CDxMaterial();

	virtual void Begin( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32& PassCount ) const;
	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;
	virtual void EndPass() const;
	virtual void End() const;

	void LoadEffectFromFile( LPDIRECT3DDEVICE9 pDevice );

	//Get the D3DX effect.
	LPD3DXEFFECT GetEffect();

	//Get the D3DX effect global parameter corresponding to the specified value.
	D3DXHANDLE GetEffectParameter( EParameterType ParameterType );

	//Set the D3DX effect global parameter corresponding to the specified value.
	void SetEffectParameter( EParameterType ParameterType, D3DXHANDLE hParameter );

	int32 GetTechnique() const;
	void SetTechnique( int32 Technique );

	virtual bool IsTwoSided() const {return false;}

	std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	std::string const& GetEffectPath() const {return m_EffectPath;}
	void SetEffectPath( std::string const EffectPath ) {m_EffectPath = EffectPath;}

	CMaterial const* GetSoftwareMaterial() const {return m_hSoftwareMaterial;}
	CMaterial* GetSoftwareMaterial() {return m_hSoftwareMaterial;}
	void SetSoftwareMaterial( CMaterial* pMaterial ) {m_hSoftwareMaterial = pMaterial;}

	struct Texture
	{
		HTexture hTexture;
		int32	 TexCoordIndex;
	};

	std::vector<Texture>& GetTextures();
	std::vector<Texture> const& GetTextures() const;

	//Get user-defined properties
	System::CUserPropertyCollection& GetParameters() {return *m_hParameters;}
	System::CUserPropertyCollection const& GetParameters() const {return *m_hParameters;}

	//Update the D3DX effect global texture parameters. The D3DX effect pointer must not be NULL.
	void SetEffectTextureParameters( LPDIRECT3DDEVICE9 pDevice ) const;

	//Update the D3DX effect global parameters related to transforms. The D3DX effect pointer must not be NULL.
	void SetEffectTransformParameters( LPDIRECT3DDEVICE9 pDevice ) const;

	//Update the D3DX effect global parameters related to time. The D3DX effect pointer must not be NULL.
	void SetEffectTimeParameters( LPDIRECT3DDEVICE9 pDevice ) const;

	//Update all D3DX effect global parameters. This is always called before ID3DXEffect::Begin().
	//This function is considered const because its purpose is only to set dynamic parameters 
	//that don't reflect the logical state of the D3DX effect.
	virtual void SetEffectParameters( LPDIRECT3DDEVICE9 pDevice ) const;

	//
	//ISerializable interface
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion( ) const {return GetClassFormatVersion();}
	virtual void FinalizeDeserialization( System::Serialization::CSerializationInfo* SerialInfo,
		System::Serialization::CSerializationContext* sContext );
	virtual void GetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext);

	//
	//IDeviceObject interface
	//
	void InvalidateDeviceObjects();
	void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	bool IsValid() const;

private:

    void ClearParameters();

	std::string						m_Name;
	std::string						m_EffectPath;
	HMaterial						m_hSoftwareMaterial;
	std::vector<Texture>			m_Textures;
	System::HUserPropertyCollection m_hParameters;
	LPD3DXEFFECT					m_pEffect;
	int32							m_Technique;

	D3DXHANDLE	m_hEffectParameters[KParameterTypeCount];
	bool		m_NeedWorldMatrix				: 1;
	bool		m_NeedViewMatrix				: 1;
	bool		m_NeedProjectionMatrix			: 1;
	bool		m_NeedWorldViewMatrix			: 1;
	bool		m_NeedViewProjectionMatrix		: 1;
	bool		m_NeedWorldViewProjectionMatrix	: 1;
	bool		m_ResourcesValid : 1;
};

}

template <> inline void WTCall WT::Using<Graphics::CDxMaterial>()
{
	Graphics::CDxMaterial::GetClassType();
	WT::Using<System::CExplicitTypedProperty<int32> >();
	WT::Using<System::CExplicitTypedProperty<float32> >();
	WT::Using<System::CExplicitTypedProperty<bool> >();
	WT::Using<System::CExplicitTypedProperty<string> >();
	WT::Using<System::CExplicitTypedProperty<System::CObject*> >();
	WT::Using<System::CExplicitTypedProperty<std::vector<int32> > >();
	WT::Using<System::CExplicitTypedProperty<std::vector<float32> > >();
	WT::Using<System::CExplicitTypedProperty<std::vector<bool> > >();
	WT::Using<System::CExplicitTypedProperty<std::vector<string> > >();
	WT::Using<System::CExplicitTypedProperty<std::vector<System::CObject*> > >();
	WT::Using<System::CExplicitTypedProperty<Math::CMatrix4f> >();
	WT::Using<System::CExplicitTypedProperty<Math::CVector4f> >();
	WT::Using<System::CExplicitTypedProperty<Math::CVector3f> >();
	WT::Using<System::CExplicitTypedProperty<std::vector<Math::CMatrix4f> > >();
	WT::Using<System::CExplicitTypedProperty<std::vector<Math::CVector4f> > >();
	WT::Using<System::CExplicitTypedProperty<std::vector<Math::CVector3f> > >();
}

}
