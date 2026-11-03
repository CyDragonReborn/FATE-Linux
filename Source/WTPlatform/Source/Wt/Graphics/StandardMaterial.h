/***************************************************************************************
 *
 * StandardMaterial.h
 *
 * Copyright (C) 2006 WildTangent, Inc. 
 * All Rights Reserved
 *
 *
 * Description:  
 *
 * Author:        Gary Hinger
 * Date:          05/3/2006
 * Notes:         
 *
 ***************************************************************************************/

#pragma once

#include "WT/Graphics/Material.h"

namespace WT
{
namespace Graphics
{

typedef System::SmartPointer<class CStandardMaterial> HStandardMaterial;
class CStandardMaterial : public CMaterial, public System::Serialization::ISerializable
{
public:
	enum EMapType
	{
		KMapTypeDiffuse,
		KMapTypeOpacity,
		KMapTypeReflection,
		KMapTypeBump,
		KMapTypeSelfIllumination,
		KMapTypeCount
	};

	WT_BEGIN_CLASS( CStandardMaterial, CMaterial )
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable )
	WT_END_CLASS()

	//
	//ISerializable methods
	//
	virtual void GetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext);
	virtual void FinalizeDeserialization( System::Serialization::CSerializationInfo* SerialInfo,
		System::Serialization::CSerializationContext* sContext );

	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion( ) const {return GetClassFormatVersion();}

	CStandardMaterial();
	~CStandardMaterial();

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	virtual void BeginPass( LPDIRECT3DDEVICE9 pDevice, CRenderContext const& Context, int32 PassIndex ) const;

	virtual bool IsTwoSided() const {return m_bTwoSided;}

	void SetTwoSided( bool bTwoSided ) {m_bTwoSided = bTwoSided;}

	virtual std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	/// Sets a value that specifies the ambient color of the material
	void SetAmbient( D3DCOLORVALUE const& Ambient );

	/// Retrieves the value that specifies the ambient color of the material
	D3DCOLORVALUE GetAmbient() const;

	/// Sets a value that specifies the diffuse color of the material
	void SetDiffuse( D3DCOLORVALUE const& Diffuse );

	/// Retrieves the value that specifies the diffuse color of the material
	D3DCOLORVALUE GetDiffuse() const;
	
	/// Sets a value that specifies the emissive color of the material
	void SetEmissive( D3DCOLORVALUE const& Emissive );

	/// Retrieves the value that specifies the emissive color of the material
	D3DCOLORVALUE GetEmissive() const;

	/// Sets a value that specifies the specular color of the material
	void SetSpecular( D3DCOLORVALUE const& Specular );

	/// Retrieves the value that specifies the specular color of the material
	D3DCOLORVALUE GetSpecular() const;

	/// Sets a value that specifies the sharpness of the material's specular highlights
	/// In 3ds max, "shininess" is the specular power divided by 100.
	void SetSpecularPower( float32 Power );

	/// Retrieves the value that specifies the sharpness of the material's specular highlights
	float32 GetSpecularPower() const;

	float32 GetHorizVelocity()    const {return m_HorizVelocity;}
	float32 GetVertVelocity()     const {return m_VertVelocity;}
	int32   GetHorizFrameCount()  const {return m_HorizFrameCount;}
	int32   GetVertFrameCount()   const {return m_VertFrameCount;}
	float32 GetFrameTimeSeconds() const {return m_FrameTimeSeconds;}

	void SetHorizVelocity( float32 HorizVelocity )       {m_HorizVelocity = HorizVelocity;}
	void SetVertVelocity( float32 VertVelocity )         {m_VertVelocity = VertVelocity;}
	void SetHorizFrameCount( int32 HorizFrameCount )     {m_HorizFrameCount = HorizFrameCount;}
	void SetVertFrameCount( int32 VertFrameCount )       {m_VertFrameCount = VertFrameCount;}
	void SetFrameTimeSeconds( float32 FrameTimeSeconds ) {m_FrameTimeSeconds = FrameTimeSeconds;}

	virtual CMaterialMap* GetDiffuseMap() const {return GetMap( KMapTypeDiffuse );}

	/// Get the map.
	CMaterialMap* GetMap( EMapType MapType ) const;

	/// Set the map.
	void SetMap( EMapType MapType, CMaterialMap* pMaterialMap );

	/// Get the map amount.
	float32 GetMapAmount( EMapType MapType ) const;

	/// Set the map amount.
	void SetMapAmount( EMapType MapType, float32 MapAmount );

private:
	std::string		m_Name;
	D3DMATERIAL9	m_Material;
	float32			m_HorizVelocity;
	float32			m_VertVelocity;
	int32			m_HorizFrameCount;
	int32			m_VertFrameCount;
	float32			m_FrameTimeSeconds;
	HMaterialMap	m_pMaps[KMapTypeCount];
	float32			m_MapAmounts[KMapTypeCount];
	bool			m_ResourcesValid : 1;
	bool			m_bTwoSided : 1;
};

}
}
