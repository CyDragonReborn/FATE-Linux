/***************************************************************************************
 *
 * Graphics.h
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

#include "WT/Graphics/ActiveMotion.h"
#include "WT/Graphics/Actor.h"
#include "WT/Graphics/ArcBall.h"
#include "WT/Graphics/AVITexture.h"
#include "WT/Graphics/Camera.h"
#include "WT/Graphics/CompositeMaterial.h"
#include "WT/Graphics/Curve.h"
#include "WT/Graphics/DxMaterial.h"
#include "WT/Graphics/FlashRenderer.h"
#include "WT/Graphics/Frustum.h"
#include "WT/Graphics/HTMLRenderer.h"
#include "WT/Graphics/IDeviceObject.h"
#include "WT/Graphics/IMovable.h"
#include "WT/Graphics/IRenderable.h"
#include "WT/Graphics/IUpdateable.h"
#include "WT/Graphics/Level.h"
#include "WT/Graphics/Light.h"
#include "WT/Graphics/Material.h"
#include "WT/Graphics/MaterialMap.h"
#include "WT/Graphics/Mirror.h"
#include "WT/Graphics/Model.h"
#include "WT/Graphics/PointHelper.h"
#include "WT/Graphics/RenderCollection.h"
#include "WT/Graphics/RenderToTexture.h"
#include "WT/Graphics/Scene.h"
#include "WT/Graphics/ShadowMap.h"
#include "WT/Graphics/SkyBox.h"
#include "WT/Graphics/Sprite.h"
#include "WT/Graphics/SpriteCollection.h"
#include "WT/Graphics/StandardMaterial.h"
#include "WT/Graphics/Texture.h"
#include "WT/Graphics/TranslucentActor.h"
#include "WT/Graphics/TranslucentStandardMaterial.h"

namespace WT
{
namespace Graphics
{

//Deserialization helper functions.

//Form the actual path to a file, given a tentative path and zero or more search paths.
//The path is tried as-is before the search paths are used. The search paths are specified as 
//a std::vector<string>-valued property in the serialization context with the property name 
//given by SearchPathsPropertyName. An empty string is returned on failure.
string FindResourceFile( System::CSerializationContext const* sContext, std::string const& FilePath, 
					   std::string const& SearchPathsPropertyName );

System::HObject WTCall DeserializeObject( std::string const& FilePath, std::vector< std::string > const& TexturePaths, 
	std::vector<std::string> const& EffectPaths, LPDIRECT3DDEVICE9 pDevice = NULL );

template<typename T> System::SmartPointer<T> Deserialize( std::string const& FilePath,
	std::vector< std::string > const& TexturePaths, 
	std::vector< std::string > const& EffectPaths, 
	LPDIRECT3DDEVICE9 pDevice = NULL )
{
	System::HObject hObject = DeserializeObject( FilePath, TexturePaths, EffectPaths, pDevice );
	T* pResult = WT::DynamicCast<T>( hObject.get() );
	return pResult;
}

template<typename T> System::SmartPointer<T> Deserialize( std::string const& FilePath, std::vector< std::string > const& TexturePaths, 
	LPDIRECT3DDEVICE9 pDevice = NULL )
{
	std::vector<std::string> EffectPaths;
	return Deserialize<T>( FilePath, TexturePaths, EffectPaths, pDevice );
}

template<typename T> System::SmartPointer<T> Deserialize( std::string const& FilePath, std::string const& TexturePath, 
	LPDIRECT3DDEVICE9 pDevice = NULL )
{
	std::vector<std::string> TexturePaths;
	TexturePaths.push_back( TexturePath );
	std::vector<std::string> EffectPaths;
	return Deserialize<T>( FilePath, TexturePaths, EffectPaths, pDevice );
}

template<typename T> System::SmartPointer<T> Deserialize( std::string const& FilePath, LPDIRECT3DDEVICE9 pDevice = NULL )
{
	std::vector<std::string> ResourcePaths;
	return Deserialize<T>( FilePath, ResourcePaths, ResourcePaths, pDevice );
}

}
}
