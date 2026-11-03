/*
	templatemanifest.cpp

	Copyright (C) 2004 WildTangent, Inc. 
	All Rights Reserved

	Travis Baldree
	8/12/2004

*/
#define STRICT

#include <windows.h>
#include <assert.h>

#include "../UTILITIES/utilities.h"
#include "../UTILITIES/macros.h"
#include "../UTILITIES/settings.h"
#include "../UTILITIES/datafile.h"

#include "templatemanifest.h"

CTemplateManifest::CTemplateManifest(  const std::string& ManifestPath )// path to manifest.dat file
{
	LoadManifest( ManifestPath );
} // CTemplateManifest::CTemplateManifest()

CTemplateManifest::~CTemplateManifest( void )
{
} // CTemplateManifest::~CTemplateManifest()

void CTemplateManifest::LoadManifest(  const std::string& ManifestPath )	// path to manifest.dat file
{
	m_TemplatePaths.clear();

	// get the root path to the vehicle
	std::string RootPath = AssembleAbsolutePath( GetLocalPath(),
												 ManifestPath );

	std::string RootDirectory = RemoveFileName( RootPath );

	CDataFile*	pDataFile = new CDataFile( RootPath );

	// parse all vehicle and sub-manifest paths from the manifest data file
	uint32 Vehicles = pDataFile->DataGroups( "TEMPLATE" );
	for( uint32 i = 0; i < Vehicles; i++ )
	{
		CDataGroup* pGroup = pDataFile->GetDataGroup( "TEMPLATE", i );
		m_TemplatePaths.push_back( pGroup->Item( 0 ) );
	}

	DELETE_SAFELY( pDataFile );

} // CTemplateManifest::LoadManifest()
