/***************************************************************************************
 *
 * Graphics.cpp
 *
 * Copyright (C) 2005 WildTangent, Inc.
 * All Rights Reserved
 *
 *
 * Revision History
 *
 * 11/8/2005: Gary Hinger: created file 
 *
 ***************************************************************************************/

#include "WT/stdafx.h"
#include "WT/System/IO/File.h"
#include "WT/System/IO/FilePath.h"
#include "WT/System/Logging/Log.h"
#include "WT/Graphics/IRenderable.h"

using namespace WT::System;
using namespace WT::System::IO;
using namespace WT::System::Serialization;

namespace WT
{
namespace Graphics
{

string FindResourceFile( CSerializationContext const* sContext, std::string const& FilePath, 
					   std::string const& SearchPathsPropertyName )
{
	std::string FullPath = CFilePath::GetFullPath( FilePath );

	//first check if the file exists in the current working folder (if FilePath is relative)
	if (CFile::Exists( FullPath ))
	{
		return FullPath;
	}

	std::string const FileName = CFilePath::GetFileName( FilePath );
	std::string const DirectoryName = CFilePath::GetDirectoryName( FullPath );

	//could not find the file in relative or absolute locations, check the paths (if provided)
	std::vector<std::string> const SearchPaths = sContext->GetProperties()->GetValue<
		std::vector< string > >( SearchPathsPropertyName, std::vector<string>() );

	//if not paths provided log a descriptive error and return empty string (will cause not to load image)
	if (SearchPaths.empty())
	{
		LOG_PURPLE( "Could not find '%s' in '%s' and no alternative paths were provided", FileName.c_str(), DirectoryName.c_str() );
		return std::string();
	}

	//search our file in each of the givne folders
	std::string SearchPathsText;
	for (std::vector<string>::const_iterator it = SearchPaths.begin(); it != SearchPaths.end(); ++it)
	{
		FullPath = CFilePath::GetFullPath( *it );
		SearchPathsText += FullPath;
		SearchPathsText += ";";

		std::string const SearchDirectoryName = CFilePath::GetDirectoryName( FullPath );
		FullPath = CFilePath::Combine( SearchDirectoryName , FileName );
		if (CFile::Exists( FullPath ))
		{
			return FullPath;
		}
	}

	//didn't find it anywhere, log descriptive error
	LOG_PURPLE( "Could not find '%s' in '%s' and in the search paths: '%s'", FileName.c_str(), DirectoryName.c_str(), SearchPathsText.c_str() );
	return std::string();
}

HObject WTCall DeserializeObject( std::string const& FilePath,
	std::vector<std::string> const& TexturePaths,
	std::vector<std::string> const& EffectPaths,
	LPDIRECT3DDEVICE9 pDevice )
{
	CSerializationContext serializationContext;
	serializationContext.GetProperties()->SetValue< std::vector<string> >( "TexturePaths", TexturePaths );
	serializationContext.GetProperties()->SetValue< std::vector<string> >( "EffectPaths", EffectPaths );

	if (pDevice != NULL)
	{
		LPDIRECT3D9 pD3D9;
		HRESULT Result = pDevice->GetDirect3D( &pD3D9 );
		if (FAILED( Result ))
		{
			throw std::runtime_error( "GetDirect3D failure" );
		}

		serializationContext.GetProperties()->SetValue<uint64>( "D3D", reinterpret_cast<uint64>( pD3D9 ) );
		serializationContext.GetProperties()->SetValue<uint64>( "D3DDevice", reinterpret_cast<uint64>( pDevice ) );
	}

    HStream hStream = CFile::Open( FilePath, 
		FileMode::Open, FileAccess::Read, FileShare::Read, FileFlags::KSequentialScan );
	return CAgnosticFormatter().Deserialize( hStream, &serializationContext );
}

} //namespace Graphics
} //namespace WT
