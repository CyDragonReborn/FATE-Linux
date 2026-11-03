/***************************************************************************************
 *
 * MaterialMap.h
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

#include "WT/System/Serialization/ISerializable.h"
#include "WT/Graphics/IDeviceObject.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/Texture.h"

namespace WT
{
namespace Graphics
{

namespace EMaterialMap
{
	enum Type
	{
		KExplicit, ///Explicit texture mapping with UV coordinates
		KSphere, ///On-the-fly spherical mapping
		KCylinder, ///On-the-fly cylindrical mapping
		KShrinkWrap, ///"Shrink wrap" mapping
		KScreen, ///On-the-fly screen-based mapping
		KCount //Number of material map types
	};
}

//A material map. The material map owns its Texture object.
typedef System::SmartPointer<class CMaterialMap> HMaterialMap;
class CMaterialMap : public System::CObject, public System::Serialization::ISerializable, public IDeviceObject
{
public:
	WT_BEGIN_CLASS( CMaterialMap, System::CObject );
		WT_INHERITS_INTERFACE( System::Serialization::ISerializable );
		WT_INHERITS_INTERFACE( IDeviceObject );
	WT_END_CLASS();

	//
	//ISerializable methods
	//
	static uint32 WTCall GetClassFormatVersion();
	virtual uint32 GetObjectFormatVersion() const {return GetClassFormatVersion();}
	virtual void GetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext) const;
	virtual void SetObjectData(System::Serialization::CSerializationInfo* Info, System::Serialization::CSerializationContext* sContext);

	CMaterialMap();
	~CMaterialMap();
	CMaterialMap( CMaterialMap const& Map );

	virtual void InvalidateDeviceObjects();
	virtual void RestoreDeviceObjects( LPDIRECT3DDEVICE9 pDevice );
	virtual bool IsValid() const;

	//Return whether the maps are suitable for combining under operations such as per-texel
	//modulation. Properties such as name and texture are ignored for this check.
	bool IsCompatible( CMaterialMap const& Map ) const;

	virtual void PrepareRender( CRenderContext const& Context,
		LPDIRECT3DDEVICE9 pDevice, int32 SamplerIndex, float32 OffsetU = 0.0f, float32 OffsetV = 0.0f );

	std::string const& GetName() const {return m_Name;}
	void SetName( std::string const& Name ) {m_Name = Name;}

	//Texture path
	string GetTexturePath() const {return m_TexturePath;}
	void SetTexturePath( string const& TexturePath ) {m_TexturePath = TexturePath;}

	/// Get material map. This method returns a EMaterialMap::Type value.
	int32 GetMapType() const;

	/// Set material map type. This method accepts a EMaterialMap::Type value.
	void SetMapType( int32 MapType );

	/// Get texture coordinate index (applicable when GetMapping() == EMaterialMap::Explicit).
	int32 GetMapChannel() const;

	/// Set texture coordinate index (applicable when GetMapping() == EMaterialMap::Explicit).
	void SetMapChannel( int32 MapChannel );

	bool GetUTile() const;
	void SetUTile( bool bUTile );
	
	bool GetVTile() const;
	void SetVTile( bool bVTile );
	
	bool GetWTile() const;
	void SetWTile( bool bWTile );

	bool GetUMirror() const;
	void SetUMirror( bool bUMirror );

	bool GetVMirror() const;
	void SetVMirror( bool bVMirror );

	bool GetWMirror() const;
	void SetWMirror( bool bWMirror );

	bool GetUseLuminanceAsAlpha() const {return m_bUseLuminanceAsAlpha;}
	void SetUseLuminanceAsAlpha( bool bUseLuminanceAsAlpha ) {m_bUseLuminanceAsAlpha = bUseLuminanceAsAlpha;}

	Math::CVector3f GetScale() const;
	void SetScale( Math::CVector3f const& Scale );
	void SetScale( float32 ScaleX, float32 ScaleY, float32 ScaleZ );
	
	Math::CVector3f const& GetRotation() const;
	void SetRotation( Math::CVector3f const& Rotation );
	void SetRotation( float32 Pitch, float32 Yaw, float32 Roll );

	Math::CVector3f GetTranslation() const;
	void SetTranslation( Math::CVector3f const& Translation );
	void SetTranslation( float32 TranslationX, float32 TranslationY, float32 TranslationZ );

	void SetTexture( CTexture* pTexture )
	{
		m_hTexture = pTexture;
	}

	CTexture* GetTexture() {return m_hTexture;}

private:
	string					m_Name;
	string					m_TexturePath;

	EMaterialMap::Type		m_MapType;
	int32					m_MapChannel;

	Math::CVector3f			m_Scale;
	Math::CVector3f			m_Rotation; //Euler angles in radians
	Math::CVector3f			m_Translation;
	Math::CMatrix4f			m_Transform;

	HTexture				m_hTexture;

	bool					m_bTile[3];
	bool					m_bMirror[3];
	bool					m_ResourcesValid : 1;
	bool					m_bUseTransform : 1;
	bool					m_bUseLuminanceAsAlpha : 1;
    
	void UpdateTransform();
};

}
}
